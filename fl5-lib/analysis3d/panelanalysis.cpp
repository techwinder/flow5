/****************************************************************************

    flow5 application
    Copyright (C) 2025 André Deperrois 
    
    This file is part of flow5.

    flow5 is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    flow5 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with flow5.
    If not, see <https://www.gnu.org/licenses/>.


*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include <iostream>
#include <thread>
#include <QString>


#include <panelanalysis.h>

#include <gaussquadrature.h>
#include <objects_global.h>
#include <panel.h>
#include <panel3.h>
#include <polar3d.h>
#include <stabderivatives.h>



#if defined ACCELERATE
  #include <Accelerate/Accelerate.h>
  #define lapack_int int
#elif defined INTEL_MKL
    #include <mkl.h>
#elif defined OPENBLAS
    #include <openblas/lapacke.h>
#endif

#include <lapack_fortran.h>

/*#else
  #define lapack_complex_float std::complex<float>
  #define lapack_complex_double std::complex<double>
  #include <lapacke.h>
#endif*/

bool PanelAnalysis::s_bDoublePrecision(true);
bool PanelAnalysis::s_bMultiThread(true);
int PanelAnalysis::s_MaxThreads(1);

std::vector<Vector3d> PanelAnalysis::s_DebugPts;
std::vector<Vector3d> PanelAnalysis::s_DebugVecs;


PanelAnalysis::PanelAnalysis()
{
    m_AnalysisStatus = xfl::PENDING;

    m_bCancel      = false;
    m_bMatrixError = false;
    m_bSequence    = false;
    m_bWarning     = false;

    m_nBlocks     = s_MaxThreads;

    m_nStations = 0;

    m_pPolar3d = nullptr;

    tmp_coreradius=0.0;
    tmp_bWakeOnly=false;
    tmp_Mu=nullptr;
    tmp_Sigma=nullptr;

}


/**
 * Reserves the memory necessary to matrix arrays.
 * @return true if the memory could be allocated, false otherwise.
 */
bool PanelAnalysis::allocateMatrix(int N)
{
    uint matSize = uint(N);

    uint size2 = matSize * matSize;
    double gb=0;

    try
    {
        if(s_bDoublePrecision)
        {
            m_aijd.resize(size2);
            memset(m_aijd.data(), 0, size2 * sizeof(double));
            m_aijf.clear();

            gb = size2 * sizeof(double) /1024/1024;
        }
        else
        {
            m_aijf.resize(size2);
            memset(m_aijf.data(), 0, size2 * sizeof(float));
            m_aijd.clear();

            gb = size2 * sizeof(float) /1024/1024;
        }
    }
    catch(std::bad_alloc &exception)
    {
        QString strong(exception.what());
        strong += ": error allocating memory for the influence matrix\n";
        strong += QString::asprintf("Request for %7.2f Mb failed\n", gb);
        traceLog(strong);
        return false;
    }
    catch(...)
    {
        QString strong("Undetermined error during memory allocation for the influence matrix\n");
        strong += QString::asprintf("Request for %7.2f Mb failed\n", gb);
        traceLog(strong);
        return false;
    }

    gb += matSize * sizeof(int)/1024/1024;

    return true;
}

void PanelAnalysis::traceLog(QString const &str) const
{
    traceStdLog(str.toStdString());
}

void PanelAnalysis::traceStdLog(std::string const &str) const
{
    m_ErrorLog.append(str);
}


bool PanelAnalysis::initializeAnalysis(Polar3d const *pPolar3d, int )
{
    if(!pPolar3d) return false;
    m_pPolar3d = pPolar3d;
    return true;
}


void PanelAnalysis::makeSourceStrengths(Vector3d const &VInf)
{
    for (int i3=0; i3<nPanels(); i3++)
    {
        Panel const *panel = panelAt(i3);
        if(isCancelled()) return;
        if(panel->isMidPanel()) m_Sigma[i3] =  0.0;
        else                    m_Sigma[i3] = sourceStrength(panel->normal(), VInf);
    }
}


void PanelAnalysis::makeSourceStrengths(std::vector<Vector3d> const &VInf)
{
    for (int i3=0; i3<nPanels(); i3++)
    {
        Panel const *panel = panelAt(i3);
        if(isCancelled()) return;
        if(panel->isMidPanel()) m_Sigma[i3] =  0.0;
        else                    m_Sigma[i3] = sourceStrength(panel->normal(), VInf.at(i3));
    }
}


void PanelAnalysis::releasePanelArrays()
{
    m_uRHS.clear();
    m_wRHS.clear();
}


/**
 * Solves the linear system for the unit RHS, using LU decomposition
 */
bool PanelAnalysis::LUfactorize()
{
#ifdef INTEL_MKL
    if(s_bMultiThread)
        MKL_Set_Num_Threads_Local(s_MaxThreads);
    else
        MKL_Set_Num_Threads_Local(1);
#endif

    lapack_int n = matSize();
    lapack_int lda = matSize();
    m_ipiv.resize(matSize());
    lapack_int info = -1;

    if(s_bDoublePrecision)
    {
        dgetrf_(&n, &n, m_aijd.data(), &lda, m_ipiv.data(), &info);
    }
    else
    {
        //solve single precision
        sgetrf_(&n, &n, m_aijf.data(), &lda, m_ipiv.data(), &info);
    }

    if(info>0 || info <0)
    {
        traceStdLog("         Singular Matrix.... Aborting calculation...\n");
        return false;
    }

    return true;
}


/**
* Solves the linear system for the unit RHS, using LU decomposition
*/
void PanelAnalysis::backSubUnitRHS(double *uRHS, double *vRHS, double *wRHS, double *pRHS, double *qRHS, double *rRHS)
{
#ifdef INTEL_MKL
    if(s_bMultiThread)
        mkl_set_num_threads(s_MaxThreads);
    else
        mkl_set_num_threads(1);
#endif
    char trans = 'T';
    lapack_int n = matSize();
    lapack_int lda = matSize();

    //LU solve
    lapack_int nrhs = 1;
    lapack_int ldb = n;
    lapack_int info = 0;

    if(s_bDoublePrecision)
    {
#ifdef OPENBLAS
        if(uRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), uRHS, &ldb, &info, 1);
        if(vRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), vRHS, &ldb, &info, 1);
        if(wRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), wRHS, &ldb, &info, 1);
        if(pRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), pRHS, &ldb, &info, 1);
        if(qRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), qRHS, &ldb, &info, 1);
        if(rRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), rRHS, &ldb, &info, 1);
#elif defined INTEL_MKL
        if(uRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), uRHS, &ldb, &info);
        if(vRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), vRHS, &ldb, &info);
        if(wRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), wRHS, &ldb, &info);
        if(pRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), pRHS, &ldb, &info);
        if(qRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), qRHS, &ldb, &info);
        if(rRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), rRHS, &ldb, &info);
#elif defined ACCELERATE
        if(uRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), uRHS, &ldb, &info);
        if(vRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), vRHS, &ldb, &info);
        if(wRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), wRHS, &ldb, &info);
        if(pRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), pRHS, &ldb, &info);
        if(qRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), qRHS, &ldb, &info);
        if(rRHS) dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), rRHS, &ldb, &info);
#endif
        if(info!=0)
        {
            traceStdLog("Error back-substituting\n");
        }
    }
    else
    {
        //solve single precision
        std::vector<float> srhs(n);
        if(uRHS)
        {
            for(int i=0; i<n; i++) srhs[i] = float(uRHS[i]);
#ifdef OPENBLAS
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info, 1);
#elif defined INTEL_MKL
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info);
#elif defined ACCELERATE
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info);
#endif
            for(int i=0; i<n; i++) uRHS[i] = double(srhs.at(i));
        }
        if(vRHS)
        {
            for(int i=0; i<n; i++) srhs[i] = float(vRHS[i]);
#ifdef OPENBLAS
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info, 1);
#elif defined INTEL_MKL
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info);
#elif ACCELERATE
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info);
#endif
            for(int i=0; i<n; i++) vRHS[i] = double(srhs.at(i));
        }
        if(wRHS)
        {
            for(int i=0; i<n; i++) srhs[i] = float(wRHS[i]);
#ifdef OPENBLAS
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info, 1);
#elif defined INTEL_MKL
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info);
#elif defined ACCELERATE
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info);
#endif
            for(int i=0; i<n; i++) wRHS[i] = double(srhs.at(i));
        }
        if(pRHS)
        {
            for(int i=0; i<n; i++) srhs[i] = float(pRHS[i]);
#ifdef OPENBLAS
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info, 1);
#elif defined INTEL_MKL
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info);
#elif defined ACCELERATE
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info);
#endif
            for(int i=0; i<n; i++) pRHS[i] = double(srhs.at(i));
        }
        if(qRHS)
        {
            for(int i=0; i<n; i++) srhs[i] = float(qRHS[i]);
#ifdef OPENBLAS
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info, 1);
#elif defined INTEL_MKL
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info);
#elif defined ACCELERATE
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info);
#endif
            for(int i=0; i<n; i++) qRHS[i] = double(srhs.at(i));
        }
        if(rRHS)
        {
            for(int i=0; i<n; i++) srhs[i] = float(rRHS[i]);
#ifdef OPENBLAS
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info, 1);
#elif defined INTEL_MKL
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info);
#elif defined ACCELERATE
            sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), srhs.data(), &ldb, &info);
#endif
            for(int i=0; i<n; i++) rRHS[i] = double(srhs.at(i));
        }
    }
}


bool PanelAnalysis::backSubRHS(std::vector<double> &RHS)
{
    int matsize = int(RHS.size());

    char trans = 'T';
    lapack_int lda=matsize, n=matsize, nrhs=1, ldb=n;
    lapack_int info = 0;
    if(s_bDoublePrecision)
    {
#ifdef OPENBLAS
        dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), RHS.data(), &ldb, &info, 1);
#elif INTEL_MKL
        dgetrs_(&trans, &n, &nrhs, m_aijd.data(), &lda, m_ipiv.data(), RHS.data(), &ldb, &info);
#endif
    }
    else
    {
        std::vector<float> cf(matsize,0);
        for(int i=0; i<n; i++)  cf[i] = float(RHS.at(i));
#ifdef OPENBLAS
        sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), cf.data(), &n, &info, 1);
#elif INTEL_MKL
        sgetrs_(&trans, &n, &nrhs, m_aijf.data(), &lda, m_ipiv.data(), cf.data(), &n, &info);
#endif
        for(int i=0; i<n; i++) RHS[i] = double(cf.at(i));
    }
    if(info!=0)
    {
        traceStdLog("      Error back-solving the RHS\n");
        return false;
    }
    return true;
}


/** Combines the unit RHS or unit solution vector to make respectively a unit RHS or solution vector */
void PanelAnalysis::combineUnitRHS(std::vector<double> &RHS, Vector3d const &VInf, Vector3d const &Omega)
{
    for(uint i=0; i<RHS.size(); i++)
    {
        RHS[i]  = VInf.x  * m_uRHS[i] + VInf.y *m_vRHS[i] + VInf.z *m_wRHS[i];
        RHS[i] += Omega.x * m_pRHS[i] + Omega.y*m_qRHS[i] + Omega.z*m_rRHS[i];
    }
}


void PanelAnalysis::makeUnitRHSVectors()
{
    if(s_bMultiThread)
    {
        std::vector<std::thread> threads;

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
//            futureSync.addFuture(QtConcurrent::run(&PanelAnalysis::makeUnitRHSBlock, this, iBlock));
            threads.push_back(std::thread(&PanelAnalysis::makeUnitRHSBlock, this, iBlock));
        }

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            threads[iBlock].join();
        }
//        std::cout << "PanelAnalysis::makeUnitRHSVectors joined all " << m_nBlocks << " threads" <<std::endl;
    }
    else
    {
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            makeUnitRHSBlock(iBlock);
        }
    }
}


/** Case where the velocity field is not a solid body movement, e.g. in the case of
 * control polars, virtual twist and vorton wake */
void PanelAnalysis::makeRHS(const std::vector<Vector3d> &VField, std::vector<double> &RHS, Vector3d const*normals)
{
    if(s_bMultiThread)
    {
        std::vector<std::thread> threads;

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
//            futureSync.addFuture(QtConcurrent::run(&PanelAnalysis::makeRHSBlock, this, iBlock, RHS.data(), VField, normals));
            threads.push_back(std::thread(&PanelAnalysis::makeRHSBlock, this, iBlock, RHS.data(), VField, normals));
        }

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            threads[iBlock].join();
        }
//        std::cout << "PanelAnalysis::makeRHS joined all " << m_nBlocks << " threads" <<std::endl;
    }
    else
    {
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            makeRHSBlock(iBlock, RHS.data(), VField, normals);
        }
    }
}


/**
 * Adds the contribution of the wake columns to the coefficients of the influence matrix
 * Method:
 *     - follow the method described in NASA 4023 eq. (44)
 *    - add the wake's doublet contribution to the matrix
 *    - add the difference in potential at the trailing edge panels to the RHS
 * Only a flat wake is considered. Wake roll-up has been tested but did not prove robust enough for implementation.
 */
void PanelAnalysis::addWakeContribution()
{
    makeWakeContribution();
}


/**
* In the case of a panel analysis, adds the contribution of the wake columns to the coefficients of the influence matrix
* Method :
*     - follow the method described in NASA 4023 eq. (44)
*    - add the wake's doublet contribution to the matrix
*    - add the potential difference at the trailing edge panels to the RHS ; the potential's origin
*     is set arbitrarily to the geometrical orgin so that phi = V.dot(WindDirection) x point_position
*/
void PanelAnalysis::makeWakeContribution()
{
    if(s_bMultiThread)
    {
        std::vector<std::thread> threads;

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
//            futureSync.addFuture(QtConcurrent::run(&PanelAnalysis::makeWakeMatrixBlock, this, iBlock));
            threads.push_back(std::thread(&PanelAnalysis::makeWakeMatrixBlock, this, iBlock));

        }

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            threads[iBlock].join();
        }
//        std::cout << "PanelAnalysis::makeWakeContribution joined all " << m_nBlocks << " threads" <<std::endl;

    }
    else
    {
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            makeWakeMatrixBlock(iBlock);
        }
    }
}


/**
 * Make the velocity field induced by the Vorton Wake
 */
void PanelAnalysis::makeRHSVWVelocities(std::vector<Vector3d> &VPW, bool bVLM)
{
    Vector3d C;
    if(s_bMultiThread)
    {
        int nThreads=std::thread::hardware_concurrency();

        std::vector<std::thread> threads;

        for(int iblock=0; iblock<nThreads; iblock++)
        {
            int ifirst = (nPanels()/nThreads) *  iblock;
            int ilast  = (nPanels()/nThreads) * (iblock+1);

            if(iblock==nThreads-1) ilast=nPanels();

//            futureSync.addFuture(QtConcurrent::run(&PanelAnalysis::makeRHSVWVelocitiesBlock, this, ifirst, ilast, bVLM, VPW.data()));
            threads.push_back(std::thread(&PanelAnalysis::makeRHSVWVelocitiesBlock, this, ifirst, ilast, bVLM, VPW.data()));
        }

        for(int iBlock=0; iBlock<nThreads; iBlock++)
        {
            threads[iBlock].join();
        }
//        std::cout << "PanelAnalysis::makeRHSVWVelocities joined all " << m_nBlocks << " threads" <<std::endl;

    }
    else
    {
        double vtncorelength = m_pPolar3d->vortonCoreSize()*m_pPolar3d->referenceChordLength();

        for(int p=0; p<nPanels(); p++)
        {
            Panel const *pPanel = panelAt(p);
            C.set(pPanel->ctrlPt(bVLM));
            getVortonVelocity(C, vtncorelength, VPW[p], false);
        }
    }
}


void PanelAnalysis::makeRHSVWVelocitiesBlock(int iFirst, int iLast, bool bVLM, Vector3d *Vpanel)
{
    Vector3d C;
    double vtncorelength = m_pPolar3d->vortonCoreSize()*m_pPolar3d->referenceChordLength();

    for(int p=iFirst; p<iLast; p++)
    {
        Panel const *pPanel = panelAt(p);
        C.set(pPanel->ctrlPt(bVLM));
        getVortonVelocity(C, vtncorelength, *(Vpanel+p), false);
    }
}


void PanelAnalysis::computeStabilityDerivatives(double alphaeq, double u0, Vector3d const &CoG, bool bFuseMi,
                                                StabDerivatives &SD, Vector3d &Force0, Vector3d &Moment0)
{
    SD.reset();
    computeTranslationDerivatives(alphaeq, u0, CoG, bFuseMi, SD, Force0, Moment0);
    computeAngularDerivatives(alphaeq, u0, CoG, bFuseMi, SD);
}


void PanelAnalysis::computeTranslationDerivatives(double alphaeq, double u0, Vector3d const &CoG, bool bFuseMi,
                                                  StabDerivatives &SD, Vector3d &Force0, Vector3d &Moment0)
{
#ifdef INTEL_MKL
    if(s_bMultiThread)
        mkl_set_num_threads(s_MaxThreads);
    else
        mkl_set_num_threads(1);
#endif

    Vector3d Forcem, Momentm;
    Vector3d Forcep, Momentp;
    Vector3d V0, is, js, ks, WindDirection, WindNormal;

    Vector3d Vim, Vjm, Vkm, Vip, Vjp, Vkp;

    double beta(0.0);
    int N = nPanels();
    int rhssize = 0;
    if(m_pPolar3d->isQuadMethod())
        rhssize = nPanels();
    else
        rhssize = m_pPolar3d->isTriUniformMethod() ? nPanels() : nPanels()*3;

    std::vector<double> m_UmRHS(rhssize), m_VmRHS(rhssize), m_WmRHS(rhssize);
    std::vector<double> m_UpRHS(rhssize), m_VpRHS(rhssize), m_WpRHS(rhssize);
    std::vector<double> notanrhs(rhssize);

    std::vector<double> Sigma(7*N, 0);
    std::vector<std::vector<Vector3d>> VField(7);
    for(uint i=0; i<VField.size(); i++)
    {
        VField[i].resize(N); // uses the default constructor
    }

    double deltaspeed    = 0.001;         //  m/s   for forward difference estimation

    // Define the stability axes
    double cosa = cos(alphaeq*PI/180.0);
    double sina = sin(alphaeq*PI/180.0);
    WindDirection = objects::windDirection(alphaeq, 0.0);
    WindNormal = objects::windNormal(alphaeq, 0.0);

    is.set(-cosa, 0.0, -sina);
    js.set(  0.0, 1.0,   0.0);
    ks.set( sina, 0.0, -cosa);

    V0 = is * (-u0); //is the steady state velocity vector, if no sideslip

    //______________________________________________________________________________
    // RHS for unit speed vectors
    // The change in wind velocity is opposite to the change in plane velocity
    Vim = V0 - is * deltaspeed; //a positive increase in axial speed is a positive increase in wind speed
    Vjm = V0 - js * deltaspeed; //a plane movement to the right is a wind flow to the left, i.e. negative y
    Vkm = V0 - ks * deltaspeed; //a plane movement downwards (Z_stability>0) is a positive increase of V in geometry axes
    Vip = V0 + is * deltaspeed; //a positive increase in axial speed is a positive increase in wind speed
    Vjp = V0 + js * deltaspeed; //a plane movement to the right is a wind flow to the left, i.e. negative y
    Vkp = V0 + ks * deltaspeed; //a plane movement downwards (Z_stability>0) is a positive increase of V in geometry axes

    for (int p=0; p<N; p++)
    {
        Panel const *pPanel = panelAt(p);

        VField[0][p] = Vip;
        VField[1][p] = Vjp;
        VField[2][p] = Vkp;

        VField[3][p] = Vim;
        VField[4][p] = Vjm;
        VField[5][p] = Vkm;
        VField[6][p] = V0;

        if(!pPanel->isMidPanel())
        {
            Sigma[p]     = -1.0/4.0/PI * VField.at(0).at(p).dot(pPanel->normal());
            Sigma[p+  N] = -1.0/4.0/PI * VField.at(1).at(p).dot(pPanel->normal());
            Sigma[p+2*N] = -1.0/4.0/PI * VField.at(2).at(p).dot(pPanel->normal());
            Sigma[p+3*N] = -1.0/4.0/PI * VField.at(3).at(p).dot(pPanel->normal());
            Sigma[p+4*N] = -1.0/4.0/PI * VField.at(4).at(p).dot(pPanel->normal());
            Sigma[p+5*N] = -1.0/4.0/PI * VField.at(5).at(p).dot(pPanel->normal());
            Sigma[p+6*N] = -1.0/4.0/PI * VField.at(6).at(p).dot(pPanel->normal());
        }
    }

    Vector3d Omega;

    combineUnitRHS(m_UpRHS, Vip, Omega);
    combineUnitRHS(m_VpRHS, Vjp, Omega);
    combineUnitRHS(m_WpRHS, Vkp, Omega);
    combineUnitRHS(m_UmRHS, Vim, Omega);
    combineUnitRHS(m_VmRHS, Vjm, Omega);
    combineUnitRHS(m_WmRHS, Vkm, Omega);

    // cRHS used to calculate force0 and moment0 used later as the reference for control derivatives
    combineUnitRHS(m_cRHS, V0, Omega);

    //________________________________________________
    // reference force and moment

    double alpha = alphaeq;
    std::vector<Vector3d> VInf(nPanels(), objects::windDirection(alphaeq, 0.0) * u0);

    // make node vertex array
    // only needed for triuniform method to align with TriLinAnalysis
    std::vector<double> uRHSVertex, pRHSVertex;
    double *mup = nullptr, *mum=nullptr;
    if(m_pPolar3d->isTriUniformMethod())
    {
        uRHSVertex.resize(3*N);
        pRHSVertex.resize(3*N);
    }

    if(m_pPolar3d->isQuadMethod())
    {
        mup = m_cRHS.data();
        forces(mup, Sigma.data()+6*N, alpha, beta, CoG, bFuseMi, VField.at(6), Force0, Moment0);
    }
    else if(m_pPolar3d->isTriangleMethod())
    {
//        displayArray(m_cRHS);
        // Make the Cp coefficients necessary to calculate the moments
        // Re-use the methods implemented for unit calculations,
        // but this time for the delta+ and delta- solutions
        makeLocalVelocities(m_cRHS, notanrhs, notanrhs, m_uVLocal, m_vVLocal, m_wVLocal, objects::windDirection(alphaeq, 0.0));

        if(m_pPolar3d->isTriUniformMethod())
        {
            makeVertexDoubletDensities(m_cRHS, uRHSVertex);
            mup = uRHSVertex.data();
        }
        else if(m_pPolar3d->isTriLinearMethod())
        {
            mup = m_cRHS.data();
        }

        computeOnBodyCp(VInf, m_uVLocal, m_Cp);
        forces(mup, Sigma.data()+6*N, alpha, beta, CoG, bFuseMi, VField.at(6), Force0, Moment0);
    }

    //________________________________________________
    // 1st ORDER STABILITY DERIVATIVES
    // x-derivatives________________________

//    alpha = atan2(Vim.z, Vim.x) * 180.0/PI;// =alphaeq....
    alpha = alphaeq;

    if(m_pPolar3d->isTriUniformMethod())
    {
        uRHSVertex.resize(3*N);
        pRHSVertex.resize(3*N);
    }

    if(m_pPolar3d->isQuadMethod())
    {
        mup = m_UpRHS.data();
        forces(mup, Sigma.data()+0*N, alpha, beta, CoG, bFuseMi, VField.at(0), Forcep, Momentp);
        mum = m_UmRHS.data();
        forces(mum, Sigma.data()+3*N, alpha, beta, CoG, bFuseMi, VField.at(3), Forcem, Momentm);
    }
    else if(m_pPolar3d->isTriangleMethod())
    {
        // Make the Cp coefficients necessary to calculate the moments
        // Re-use the methods implemented for unit calculations,
        // but this time for the delta+ and delta- solutions
        makeLocalVelocities(m_UpRHS, notanrhs, m_UmRHS, m_uVLocal, m_vVLocal, m_wVLocal, objects::windDirection(alphaeq, 0.0)); // two at a time

        if(m_pPolar3d->isTriUniformMethod())
        {
            makeVertexDoubletDensities(m_UpRHS, uRHSVertex);
            makeVertexDoubletDensities(m_UmRHS, pRHSVertex);
            mup = uRHSVertex.data();
            mum = pRHSVertex.data();
        }
        else if(m_pPolar3d->isTriLinearMethod())
        {
            mup = m_UpRHS.data();
            mum = m_UmRHS.data();
        }

        computeOnBodyCp(VField.at(0), m_uVLocal, m_Cp);
        forces(mup, Sigma.data()+0*N, alpha, beta, CoG, bFuseMi, VField.at(0), Forcep, Momentp);

        computeOnBodyCp(VField.at(3), m_wVLocal, m_Cp);
        forces(mum, Sigma.data()+3*N, alpha, beta, CoG, bFuseMi, VField.at(3), Forcem, Momentm);
    }

    SD.Xu = (Forcem - Forcep ).dot(is) /deltaspeed/2.0;
    SD.Zu = (Forcem - Forcep ).dot(ks) /deltaspeed/2.0;
    SD.Mu = (Momentm- Momentp).dot(js) /deltaspeed/2.0;


    // y-derivatives________________________
//    alpha = atan2(Vjm.z, Vjm.x)*180.0/PI;// =alphaeq....
    alpha = alphaeq;
    if(m_pPolar3d->isQuadMethod())
    {
        mup = m_VpRHS.data();
        forces(mup, Sigma.data()+1*N, alpha, beta, CoG, bFuseMi, VField.at(1), Forcep, Momentp);
        mum = m_VmRHS.data();
        forces(mum, Sigma.data()+4*N, alpha, beta, CoG, bFuseMi, VField.at(4), Forcem, Momentm);
    }
    else if(m_pPolar3d->isTriangleMethod())
    {
        makeLocalVelocities(m_VpRHS, notanrhs, m_VmRHS, m_uVLocal, m_vVLocal, m_wVLocal, objects::windDirection(alphaeq, 0.0));

        if(m_pPolar3d->isTriUniformMethod())
        {
            makeVertexDoubletDensities(m_VpRHS, uRHSVertex);
            makeVertexDoubletDensities(m_VmRHS, pRHSVertex);
            mup = uRHSVertex.data();
            mum = pRHSVertex.data();
        }
        else if(m_pPolar3d->isTriLinearMethod())
        {
            mup = m_VpRHS.data();
            mum = m_VmRHS.data();
        }

        computeOnBodyCp(VField.at(1), m_uVLocal, m_Cp);
        forces(mup, Sigma.data()+1*N, alpha, beta, CoG, bFuseMi, VField.at(1), Forcep, Momentp);

        computeOnBodyCp(VField.at(4), m_wVLocal, m_Cp);
        forces(mum, Sigma.data()+4*N, alpha, beta, CoG, bFuseMi, VField.at(4), Forcem, Momentm);
    }
    SD.Yv = (Forcem - Forcep).dot(js)   /deltaspeed/2.0;
    SD.Lv = (Momentm - Momentp).dot(is) /deltaspeed/2.0;
    SD.Nv = (Momentm - Momentp).dot(ks) /deltaspeed/2.0;

    // z-derivatives________________________
//    alpha = atan2(Vkm.z, Vkm.x)* 180.0/PI;
    alpha = alphaeq;
    if(m_pPolar3d->isQuadMethod())
    {
        mup = m_WpRHS.data();
        forces(mup, Sigma.data()+2*N, alpha, beta, CoG, bFuseMi, VField.at(2), Forcep, Momentp);
        mum = m_WmRHS.data();
        forces(mum, Sigma.data()+5*N, alpha, beta, CoG, bFuseMi, VField.at(5), Forcem, Momentm);
    }
    else if(m_pPolar3d->isTriangleMethod())
    {
        makeLocalVelocities(m_WpRHS, notanrhs, m_WmRHS, m_uVLocal, m_vVLocal, m_wVLocal, objects::windDirection(alphaeq, 0.0));
        if(m_pPolar3d->isTriUniformMethod())
        {
            makeVertexDoubletDensities(m_WpRHS, uRHSVertex);
            makeVertexDoubletDensities(m_WmRHS, pRHSVertex);
            mup = uRHSVertex.data();
            mum = pRHSVertex.data();
        }
        else if(m_pPolar3d->isTriLinearMethod())
        {
            mup = m_WpRHS.data();
            mum = m_WmRHS.data();
        }

        computeOnBodyCp(VField.at(2), m_uVLocal, m_Cp);
        forces(mup, Sigma.data()+2*N, alpha, beta, CoG, bFuseMi, VField.at(2), Forcep, Momentp);

        computeOnBodyCp(VField.at(5), m_wVLocal, m_Cp);
        forces(mum, Sigma.data()+5*N, alpha, beta, CoG, bFuseMi, VField.at(5), Forcem, Momentm);
    }
    SD.Xw = (Forcem - Forcep).dot(is)   /deltaspeed/2.0;
    SD.Zw = (Forcem - Forcep).dot(ks)   /deltaspeed/2.0;
    SD.Mw = (Momentm - Momentp).dot(js) /deltaspeed/2.0;
}



void PanelAnalysis::computeAngularDerivatives(double alphaeq, double u0, Vector3d const &CoG, bool bFuseMi, StabDerivatives &SD)
{
#ifdef INTEL_MKL
    if(s_bMultiThread)
        mkl_set_num_threads(s_MaxThreads);
    else
        mkl_set_num_threads(1);
#endif

    Vector3d Forcem, Momentm, Rism, Rjsm, Rksm;
    Vector3d Forcep, Momentp, Risp, Rjsp, Rksp;
    Vector3d V0, is, js, ks, CGM, WindDirection, WindNormal;

    double beta(0.0);
    int N = nPanels();
    int rhssize = 0;
    if(m_pPolar3d->isQuadMethod())
        rhssize = nPanels();
    else
        rhssize = m_pPolar3d->isTriUniformMethod() ? nPanels() : nPanels()*3;

    std::vector<double> m_UmRHS(rhssize), m_VmRHS(rhssize), m_WmRHS(rhssize);
    std::vector<double> m_UpRHS(rhssize), m_VpRHS(rhssize), m_WpRHS(rhssize);
    std::vector<double> notanrhs(rhssize);

    std::vector<double> Sigma(6*N);
    std::vector<std::vector<Vector3d>> VField(6);
    for(uint i=0; i<VField.size(); i++)
    {
        VField[i].resize(N);
        std::fill(VField[i].begin(), VField[i].end(), Vector3d());
    }

    double rotationrate = 0.01;         //  rad/s for difference estimation

    // Define the stability axes
    double cosa = cos(alphaeq*PI/180);
    double sina = sin(alphaeq*PI/180);
    WindDirection = objects::windDirection(alphaeq, 0.0);
    WindNormal = objects::windNormal(alphaeq, 0.0);

    is.set(-cosa, 0.0, -sina);
    js.set(  0.0, 1.0,   0.0);
    ks.set( sina, 0.0, -cosa);

    V0 = is * (-u0); //is the steady state velocity vector, if no sideslip

    //______________________________________________________________________________
    // RHS for unit rotation vectors around Stability axis
    // stability axis origin is CoG


    for (int p=0; p<N; p++)
    {
        Panel const* pPanel = panelAt(p);
//        if(p3->m_Pos==MIDSURFACE) CGM = p3->VortexPos - m_CoG;
//        else
        CGM = pPanel->CoG() - CoG;

        // a rotation of the plane about a vector is the opposite of a rotation of the freestream about this vector
        Risp = is*CGM * (+rotationrate) + V0;
        Rjsp = js*CGM * (+rotationrate) + V0;
        Rksp = ks*CGM * (+rotationrate) + V0;

        Rism = is*CGM * (-rotationrate) + V0;
        Rjsm = js*CGM * (-rotationrate) + V0;
        Rksm = ks*CGM * (-rotationrate) + V0;

        VField[0][p] = Risp;
        VField[1][p] = Rjsp;
        VField[2][p] = Rksp;

        VField[3][p] = Rism;
        VField[4][p] = Rjsm;
        VField[5][p] = Rksm;

        if(!pPanel->isMidPanel())
        {
            Sigma[p+0*N] = -1.0/4.0/PI * Risp.dot(pPanel->normal());
            Sigma[p+1*N] = -1.0/4.0/PI * Rjsp.dot(pPanel->normal());
            Sigma[p+2*N] = -1.0/4.0/PI * Rjsp.dot(pPanel->normal());

            Sigma[p+3*N] = -1.0/4.0/PI * Rism.dot(pPanel->normal());
            Sigma[p+4*N] = -1.0/4.0/PI * Rjsm.dot(pPanel->normal());
            Sigma[p+5*N] = -1.0/4.0/PI * Rksm.dot(pPanel->normal());
        }
    }

    combineUnitRHS(m_UpRHS, V0, is*rotationrate);
    combineUnitRHS(m_VpRHS, V0, js*rotationrate);
    combineUnitRHS(m_WpRHS, V0, ks*rotationrate);
    combineUnitRHS(m_UmRHS, V0, is*(-rotationrate));
    combineUnitRHS(m_VmRHS, V0, js*(-rotationrate));
    combineUnitRHS(m_WmRHS, V0, ks*(-rotationrate));

    //________________________________________________
    // 1st ORDER STABILITY DERIVATIVES
    double const *mup = nullptr;
    double const *mum = nullptr;
    std::vector<double> uRHSVertex, pRHSVertex;
    if(m_pPolar3d->isTriUniformMethod())
    {
        uRHSVertex.resize(3*N);
        pRHSVertex.resize(3*N);
    }

//    std::vector<Vector3d> VInf(nPanels(), windDirection(alphaeq, 0.0) * u0);

    // p-derivatives
    if(m_pPolar3d->isQuadMethod())
    {
        mup = m_UpRHS.data();
        forces(mup, Sigma.data()+0*N, alphaeq, beta, CoG, bFuseMi, VField.at(0), Forcep, Momentp);
        mum = m_UmRHS.data();
        forces(mum, Sigma.data()+3*N, alphaeq, beta, CoG, bFuseMi, VField.at(3), Forcem, Momentm);
    }
    else if(m_pPolar3d->isTriangleMethod())
    {
        // Make the Cp coefficients necessary to calculate the moments
        // Re-use the methods implemented for unit calculations,
        // but this time for the delta+ and delta- solutions
        makeLocalVelocities(m_UpRHS, notanrhs, m_UmRHS, m_uVLocal, m_vVLocal, m_wVLocal, objects::windDirection(alphaeq, 0.0));

        if(m_pPolar3d->isTriUniformMethod())
        {
            makeVertexDoubletDensities(m_UpRHS, uRHSVertex);
            makeVertexDoubletDensities(m_UmRHS, pRHSVertex);
            mup = uRHSVertex.data();
            mum = pRHSVertex.data();
        }
        else if(m_pPolar3d->isTriLinearMethod())
        {
            mup = m_UpRHS.data();
            mum = m_UmRHS.data();
        }
        computeOnBodyCp(VField.at(0), m_uVLocal, m_Cp);
        forces(mup, Sigma.data()+0*N, alphaeq, beta, CoG, bFuseMi, VField.at(0), Forcep, Momentp);
        computeOnBodyCp(VField.at(3), m_wVLocal, m_Cp);
        forces(mum, Sigma.data()+3*N, alphaeq, beta, CoG, bFuseMi, VField.at(3), Forcem, Momentm);
    }
    SD.Yp = (Forcep -Forcem ).dot(js) /rotationrate/2.0;
    SD.Lp = (Momentp-Momentm).dot(is) /rotationrate/2.0;
    SD.Np = (Momentp-Momentm).dot(ks) /rotationrate/2.0;

    // q-derivatives
    if(m_pPolar3d->isQuadMethod())
    {
        mup = m_VpRHS.data();
        forces(mup, Sigma.data()+1*N, alphaeq, beta, CoG, bFuseMi, VField.at(1), Forcep, Momentp);
        mum = m_VmRHS.data();
        forces(mum, Sigma.data()+4*N, alphaeq, beta, CoG, bFuseMi, VField.at(4), Forcem, Momentm);
    }
    else if(m_pPolar3d->isTriangleMethod())
    {
        makeLocalVelocities(m_VpRHS, notanrhs, m_VmRHS, m_uVLocal, m_vVLocal, m_wVLocal, objects::windDirection(alphaeq, 0.0));

        if(m_pPolar3d->isTriUniformMethod())
        {
            makeVertexDoubletDensities(m_VpRHS, uRHSVertex);
            makeVertexDoubletDensities(m_VmRHS, pRHSVertex);
            mup = uRHSVertex.data();
            mum = pRHSVertex.data();
        }
        else if(m_pPolar3d->isTriLinearMethod())
        {
            mup = m_VpRHS.data();
            mum = m_VmRHS.data();
        }
        computeOnBodyCp(VField.at(1), m_uVLocal, m_Cp);
        forces(mup, Sigma.data()+1*N, alphaeq, beta, CoG, bFuseMi, VField.at(1), Forcep, Momentp);
        computeOnBodyCp(VField.at(4), m_wVLocal, m_Cp);
        forces(mum, Sigma.data()+4*N, alphaeq, beta, CoG, bFuseMi, VField.at(4), Forcem, Momentm);
    }
    SD.Xq = (Forcep -Forcem ).dot(is) /rotationrate/2.0;
    SD.Zq = (Forcep -Forcem ).dot(ks) /rotationrate/2.0;
    SD.Mq = (Momentp-Momentm).dot(js) /rotationrate/2.0;

    // r-derivatives
    if(m_pPolar3d->isQuadMethod())
    {
        mup = m_WpRHS.data();
        forces(mup, Sigma.data()+2*N, alphaeq, beta, CoG, bFuseMi, VField.at(2), Forcep, Momentp);
        mum = m_WmRHS.data();
        forces(mum, Sigma.data()+5*N, alphaeq, beta, CoG, bFuseMi, VField.at(5), Forcem, Momentm);
    }
    else if(m_pPolar3d->isTriangleMethod())
    {
        makeLocalVelocities(m_WpRHS, notanrhs, m_WmRHS, m_uVLocal, m_vVLocal, m_wVLocal, objects::windDirection(alphaeq, 0.0));

        if(m_pPolar3d->isTriUniformMethod())
        {
            makeVertexDoubletDensities(m_WpRHS, uRHSVertex);
            makeVertexDoubletDensities(m_WmRHS, pRHSVertex);
            mup = uRHSVertex.data();
            mum = pRHSVertex.data();
        }
        else if(m_pPolar3d->isTriLinearMethod())
        {
            mup = m_WpRHS.data();
            mum = m_WmRHS.data();
        }
        computeOnBodyCp(VField.at(2), m_uVLocal, m_Cp);
        forces(mup, Sigma.data()+2*N, alphaeq, beta, CoG, bFuseMi, VField.at(2), Forcep, Momentp);
        computeOnBodyCp(VField.at(5), m_wVLocal, m_Cp);
        forces(mum, Sigma.data()+5*N, alphaeq, beta, CoG, bFuseMi, VField.at(5), Forcem, Momentm);
    }

    SD.Yr = (Forcep -Forcem ).dot(js) /rotationrate/2.0;
    SD.Lr = (Momentp-Momentm).dot(is) /rotationrate/2.0;
    SD.Nr = (Momentp-Momentm).dot(ks) /rotationrate/2.0;
}

/** Sets the vortons from a pre-calculated PlaneOpp.
 * Used when calculating streamlines and surface velocities */
void PanelAnalysis::setVortons(std::vector<std::vector<Vorton>> const &vortons)
{
    m_Vorton = vortons;
}


/**
 * Returns the velocity vector induced by the array of vortons
 * Very fast, multithreading slows the calculation
*/
void PanelAnalysis::getVortonVelocity(Vector3d const &C, double vtncorelength, Vector3d &VelVtn, bool bMultiThread) const
{
    VelVtn.reset();
    Vector3d VG, CG;
    bMultiThread = false;
    if(bMultiThread)
    {
        // slower than single thread?

        std::vector<std::thread> threads;
        std::vector<Vector3d> VRow(nVortonRows());
        for(int iRow=0; iRow<nVortonRows(); iRow++)
        {
//            futureSync.addFuture(QtConcurrent::run(&PanelAnalysis::getVortonRowVelocity, this, iRow, C, vtncorelength, &VRow[iRow]));
            threads.push_back(std::thread(&PanelAnalysis::getVortonRowVelocity, this, iRow, C, vtncorelength, &VRow[iRow]));
        }

        for(int iBlock=0; iBlock<nVortonRows(); iBlock++)
        {
            threads[iBlock].join();
        }
        std::cout << "PanelAnalysis::getVortonVelocity joined all " << m_nBlocks << " threads" <<std::endl;

        for(uint iRow=0; iRow<VRow.size(); iRow++)
            VelVtn += VRow.at(iRow);
    }
    else
    {
        for(int irow=0; irow<nVortonRows(); irow++)
        {
            getVortonRowVelocity(irow, C, vtncorelength, &VelVtn);
        }
    }


    Vector3d VNeg, V;
    for(uint iv=0; iv<m_VortexNeg.size(); iv++)
    {
        m_VortexNeg.at(iv).getInducedVelocity(C, V, Vortex::coreRadius(), Vortex::vortexModel());
        VNeg += V;
        if(m_pPolar3d->bGroundEffect())
        {
            CG.set(C.x, C.y, -C.z-2.0*m_pPolar3d->groundHeight());
            m_VortexNeg.at(iv).getInducedVelocity(CG, VG, Vortex::coreRadius(), Vortex::vortexModel());
            VNeg.x += VG.x;
            VNeg.y += VG.y;
            VNeg.z -= VG.z;
        }
    }

    VelVtn += VNeg;
}


void PanelAnalysis::getVortonRowVelocity(int iRow, Vector3d const &C, double vtncorelength, Vector3d *VelVtn) const
{
    Vector3d VVtn, VG, CG;
    std::vector<Vorton> const &vortons = m_Vorton.at(iRow);


    double coef(0);
    if     (m_pPolar3d->bGroundEffect())      coef=1.0;
    else if(m_pPolar3d->bFreeSurfaceEffect()) coef=-1.0;

    for(uint iv=0; iv<vortons.size(); iv++)
    {
        Vorton const & vtn = vortons.at(iv);

        if(vtn.isActive())
        {
            vtn.inducedVelocity(C, vtncorelength, VVtn);
            VelVtn->x += VVtn.x;
            VelVtn->y += VVtn.y;
            VelVtn->z += VVtn.z;

            if(m_pPolar3d->bHPlane())
            {
                CG.set(C.x, C.y, -C.z-2.0*m_pPolar3d->groundHeight());
                vtn.inducedVelocity(CG, vtncorelength, VG);
                VelVtn->x += VG.x* coef;
                VelVtn->y += VG.y* coef;
                VelVtn->z -= VG.z* coef;
            }
        }
    }
}


/**
 * Returns the velocity gradient G of the velocity vector induced by the array of vortons
 * G is a 3x3 tensor such that g_ij = dV_j/dx_i. */
void PanelAnalysis::getVortonVelocityGradient(Vector3d const &C, double *G) const
{
    double g[9];
    memset(G, 0, 9*sizeof(double));
    memset(g, 0, 9*sizeof(double));
//    bool bTrace = false;
    double vtncoresize = m_pPolar3d->vortonCoreSize()*m_pPolar3d->referenceChordLength();

    for(int ic=0; ic<nVortonRows(); ic++)
    {
        std::vector<Vorton> const &vtnrow = m_Vorton.at(ic);
        for(uint iv=0; iv<vtnrow.size(); iv++)
        {
            Vorton const & vtn = vtnrow.at(iv);
            if(vtn.isActive())
            {
                vtn.velocityGradient(C, vtncoresize, g);
    //            if(bTrace)  qDebug("  %13g  %13g  %13g  %13g  %13g  %13g  %13g  %13g  %13g", -g[0], g[1], g[2], g[3], g[4], g[5], g[6], g[7], g[8]);

                for(int i=0; i<9; i++) G[i] += g[i];
    //            if(bTrace)   qDebug("  %13G  %13G  %13G  %13G  %13G  %13G  %13G  %13G  %13G", -G[0], G[1], G[2], G[3], G[4], G[5], G[6], G[7], G[8]);
            }
        }
    }
}

void PanelAnalysis::vortonDrag(double alpha, double beta, double QInf, int n0, int nStations,
                               Vector3d &Drag, SpanDistribs &SpanResFF) const
{
    if(nVortonRows()<2) return;

    //dynamic pressure, kg/m³
    double qDyn = 0.5 * m_pPolar3d->density() * QInf * QInf;

    Vector3d Wg, P;
    Vector3d stripforce; // Strip forces, body axes

    //   Define wind axes
    Vector3d winddir = objects::windDirection(alpha, beta);

    int iRow = nVortonRows() / 2;
//    int iRow = nVortonRows() -1;
    Drag.reset();

    double vtncorelength = m_pPolar3d->vortonCoreSize()*m_pPolar3d->referenceChordLength();

//    vtncorelength = Vortex::coreRadius();

    std::vector<Vorton> const &vorton = m_Vorton.at(iRow);
    int m=0;
    for(int ic=n0; ic<n0+nStations; ic++)
    {
        assert(ic<int(m_VortexNeg.size()));

        Vortex const &vortexneg = m_VortexNeg.at(ic);
        int idx0 = vortexneg.nodeIndex(0);
        int idx1 = vortexneg.nodeIndex(1);

        Vorton const &v0 = vorton.at(idx0);
        Vorton const &v1 = vorton.at(idx1);

        Vector3d const &P0 = v0.position();
        Vector3d const &P1 = v1.position();

        Vector3d vortex = (P1-P0);
        Vector3d vortexN = winddir * vortex.normalized();

        stripforce.set(0,0,0);

        double s = 0.5;
        P.set(P0 *(1.0-s) + P1*s);
        getVortonVelocity(P, vtncorelength, Wg);

        if(iRow!=nVortonRows() -1)  Wg *= 1.0/2.0;
#ifdef QT_DEBUG
        s_DebugPts.push_back(P);
        s_DebugVecs.push_back(Wg*1.0/QInf);
#endif
//        s_DebugPts.push_back(P0);
//        s_DebugVecs.push_back(P1-P0);

        stripforce = Wg * vortex * SpanResFF.m_Gamma.at(m);   // N/rho

        stripforce *= 2.0/QInf/QInf;      // N/q
        Drag += stripforce; // N/q, body axes


        SpanResFF.m_ICd[m] = stripforce.dot(winddir) /SpanResFF.stripArea(m);

        SpanResFF.m_F[m] += stripforce * qDyn; // N
        SpanResFF.m_Vd[m] = Wg;
        SpanResFF.m_Ai[m] = atan2(Wg.dot(vortexN), QInf)*180.0/PI;

#ifdef QT_DEBUG
//        qDebug("Vg[%d]  %11g  %11g", m, vortex.norm(), Wg.norm());

/*        for(int k=0; k<5; k++)
        {
            s = double(k)/5.0;
            P.set(P0 *(1.0-s) + P1*s);
            getVortonVelocity(P, vtncorelength, Wg);
            qDebug(" %13g  %13g  %13g", Wg.x, Wg.y, Wg.z);
        }*/
#endif

        m++;

    }
}


