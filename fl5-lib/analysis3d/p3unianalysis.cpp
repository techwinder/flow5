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

#include <QDebug>
#include <QString>


#include <cubicinterpolation.h>
#include <geom_global.h>
#include <mathelem.h>
#include <polar3d.h>



#if defined ACCELERATE
  #include <Accelerate/Accelerate.h>
  #define lapack_int int
#elif defined INTEL_MKL
    #include <mkl.h>
#elif defined OPENBLAS
    #include <openblas/lapacke.h>
#endif


#include <p3unianalysis.h>


P3UniAnalysis::P3UniAnalysis() : P3Analysis()
{
}


void P3UniAnalysis::makeMatrixBlock(int iBlock)
{
    int N = nPanels();

    // for each panel
    int blocksize = int(nPanels()/m_nBlocks)+1; // add one to compensate for rounding errors
    int iStart = iBlock*blocksize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blocksize, maxRows);

    Vector3d Vb[3];
    double phiNasa(0);
    Vector3d vel, velG;
    // for each panel
    for(int i3=iStart; i3<iMax; i3++)
    {
        Panel3 const &p3i = m_Panel3.at(i3);

        for(int k3=0; k3<nPanels(); k3++)
        {
            Panel3 const &p3k = m_Panel3.at(k3);

            if(m_pPolar3d->bNeumann() || p3i.isMidPanel())
            {
                p3k.doubletBasisVelocity(p3i.CoG(), Vb);

                vel = Vb[0]+Vb[1]+Vb[2];
                if(s_bDoublePrecision) m_aijd[uint(i3*N+k3)] = vel.dot(p3i.normal()); // change sign to be consistent with VLM
                else                   m_aijf[uint(i3*N+k3)] = float(vel.dot(p3i.normal()));

                if(m_pPolar3d->bGroundEffect() || m_pPolar3d->bFreeSurfaceEffect())
                {
                    double coef = m_pPolar3d->bGroundEffect() ? 1.0 : -1.0;

                    // add the contribution of the symmetric panel below the water's surface,
                    // which is the opposite of the contribution of this panel to the symmetric point
                    Vector3d  CG(p3i.CoG().x, p3i.CoG().y, -p3i.CoG().z-2.0*m_pPolar3d->groundHeight());
                    p3k.doubletBasisVelocity(CG, Vb);
                    velG = Vb[0]+Vb[1]+Vb[2];
                    velG.z = -velG.z;
                    if(s_bDoublePrecision) m_aijd[uint(i3*N+k3)] += velG.dot(p3i.normal()) * coef;
                    else                   m_aijf[uint(i3*N+k3)] += float(velG.dot(p3i.normal()))  * coef;
                }
            }
            else if(m_pPolar3d->bDirichlet())
            {
                // Dirichlet B.C.
                double phib[]{0,0,0};
                p3k.doubletBasisPotential(p3i.CoG(), i3==k3, phib, true);
                phiNasa = phib[0]+phib[1]+phib[2];

                if(s_bDoublePrecision) m_aijd[uint(i3*N+k3)] = phiNasa;
                else                   m_aijf[uint(i3*N+k3)] = float(phiNasa);

                if(m_pPolar3d->bGroundEffect() || m_pPolar3d->bFreeSurfaceEffect())
                {
                    double coef = m_pPolar3d->bGroundEffect() ? 1.0 : -1.0;

                    // add the contribution of the symmetric panel below the water's surface,
                    // which is the opposite of the contribution of this panel to the symmetric point
                    Vector3d CG(p3i.CoG().x, p3i.CoG().y, -p3i.CoG().z-2.0*m_pPolar3d->groundHeight());

                    p3k.doubletBasisPotential(CG, false, phib, true);
                    phiNasa = phib[0]+phib[1]+phib[2];

                    if(s_bDoublePrecision) m_aijd[uint(i3*N+k3)] += phiNasa * coef;
                    else                   m_aijf[uint(i3*N+k3)] += float(phiNasa) *coef;
                }
            }

            bool bError = false;
            if(s_bDoublePrecision)  bError = std::isnan(m_aijd[uint(i3*N+k3)]);
            else                    bError = std::isnan(m_aijf[uint(i3*N+k3)]);
            if(bError)
            {
                QString strange;
                strange = QString::asprintf("      *** numerical error when calculating the influence of panel %d on panel %d ***\n", k3, i3);
                traceLog(strange);
                m_bMatrixError = true;
                return;
            }

            if(isCancelled()) break;
        }
        if(isCancelled()) break;
    }
}


void P3UniAnalysis::makeWakeMatrixBlock(int iBlock)
{
    int N = nPanels();

    int blockSize = int(nPanels()/m_nBlocks) +1;
    int iStart = iBlock*blockSize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blockSize, maxRows);

    Vector3d Vb[3];
    Vector3d vel;
//    double phiNasa=0.0;
    double phiB[]{0,0,0};

    for(int i3=iStart; i3<iMax; i3++)
    {
        Panel3 const &p3i = m_Panel3.at(i3);

        // Add the contributions to the influence of the top trailing panels
        // and subtract them from the influence of the bottom trailing panels

        for(int k3=0; k3<nPanels(); k3++)
        {
            Panel3 const &p3k = m_Panel3.at(k3);
            if(p3k.isTrailing() && (p3k.isBotPanel() || p3k.isMidPanel()))
            {
                double MatWakeContrib = 0.0;
                int jWake=p3k.iWake();

                do
                {
                    if(m_pPolar3d->bNeumann() || p3i.isMidPanel())
                    {
//                        m_WakePanel3.at(jWake).doubletN4023Velocity(p3i.CoG(), false, vel, 0.0, false);
                        getDoubletInfluence(p3i.CoG(), m_WakePanel3.at(jWake), Vb, nullptr,  0.0, false);
                        vel = Vb[0]+Vb[1]+Vb[2];
                        MatWakeContrib += vel.dot(p3i.normal());
                    }
                    else if(m_pPolar3d->bDirichlet())
                    {
//                        m_WakePanel3.at(jWake).doubletN4023Potential(p3i.CoG(), false, phiNasa, 0.0, false);
                        getDoubletInfluence(p3i.CoG(), m_WakePanel3.at(jWake), nullptr, phiB,  0.0, false);
                        MatWakeContrib += phiB[0]+phiB[1]+phiB[2];
                    }

                    if(m_WakePanel3.at(jWake).iPD()<0)
                    {
                        // no downstream panel
                        break; // the way out
                    }
                    jWake++; // move downstream one
                }
                while(jWake<nWakePanels()); // set a limit

                if(p3k.isMidPanel())
                {
                    // add contribution to bot panel
                    if(s_bDoublePrecision) m_aijd[uint(i3*N+k3)] += MatWakeContrib;
                    else                   m_aijf[uint(i3*N+k3)] += float(MatWakeContrib);
                }
                else if(p3k.isBotPanel())
                {
                    // add contribution to bot panel
                    if(s_bDoublePrecision) m_aijd[uint(i3*N+k3)] += MatWakeContrib * (-1);
                    else                   m_aijf[uint(i3*N+k3)] += float(MatWakeContrib) * (-1.0f);

                    // add opposite contribution to opposite top TE panel's contribution
                    int k3t = p3k.oppositeIndex();
                    assert(k3t>=0 && k3t<nPanels());
                    if(s_bDoublePrecision) m_aijd[uint(i3*N+k3t)] += MatWakeContrib;
                    else                   m_aijf[uint(i3*N+k3t)] += float(MatWakeContrib);
                }
            }
        }

        if(isCancelled()) break;
    }
}


/**
 * For a uniform doublet density, calculate the unit velocity vectors
 * at each panel's CoG as the derivative of the doublet density over this panel
 * and its neighbours
 * The method depends on the number of neighbours.
 *   - if one, the velocity is the slope of the line joining the two CoG
 *   - if two or three, the velocity is the slope of the plane best fitting
 *     all the doublet vlaues at the CoG of this panel and its neighbours
 */
void P3UniAnalysis::makeLocalVelocities(std::vector<double> const &uRHS, std::vector<double> const &vRHS, std::vector<double> const &wRHS,
                                        std::vector<Vector3d> &uLocal, std::vector<Vector3d> &vLocal, std::vector<Vector3d> &wLocal,
                                        Vector3d const &) const
{
    std::vector<int> SingleNeighbourPanels;

    bool bRegu(false);
    double au(0), bu(0);
    double x[]{0,0,0};
    double yu[]{0,0,0};

    double theta(0);

    Vector2d V01, V02, U;
    Vector3d CoGn, CoGnl, crossP, H;
    Vector3d localvel[3];

    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);

        localvel[0].reset();
        localvel[1].reset();
        localvel[2].reset();

        if(p3.neighbourCount()<1)
        {
//            assert(false); // should never occur; each panel has at least one neighbour - except for messy STL meshes
        }
        else if (p3.neighbourCount()<=1)  // one
        {
            // one neighbour
            // local velocities are later set equal to that of the neighbour panel
            SingleNeighbourPanels.push_back(i3);
        }
        else
        {
            // There are two or three neighbours and three or four values
            // Calculate the plane best fitting these 3 or 4 doublet values
            // First value is the panel's doublet density at its CoG

            int N = p3.neighbourCount();
            assert(N>=2 && N<=3);
            int rows = 1 + p3.neighbourCount();
            std::vector<double> A(rows*3);
            std::vector<double> Mu(rows*3);

            A[0] = 1.0;
            A[4] = p3.m_CoG_l.x; // is zero by definition
            A[8] = p3.m_CoG_l.y; // is zero by definition
            Mu[0*rows+0] = i3<int(uRHS.size()) ? uRHS[i3] : 0.0;
            Mu[1*rows+0] = i3<int(vRHS.size()) ? vRHS[i3] : 0.0;
            Mu[2*rows+0] = i3<int(wRHS.size()) ? wRHS[i3] : 0.0;

            // add the neighbours densities at their CoG
            // neighbour triangles are developed in the panel's plane by rotation around the shared edge
            int m=1;
            for(int in=0; in<3; in++)
            {
                int i3n = p3.neighbour(in);
                if(i3n>=0)
                {
                    // get a reference to the neighbour panel
                    Panel3 const & p3n = m_Panel3.at(i3n);
                    CoGn = p3n.CoG();

                    // rotate the neighbour's CoG around the common edge so that it lies in the panel's plane
                    Segment3d const &edge = p3.neighbourEdge(in);
                    crossP = p3n.normal() * p3.normal();

                    // check alignment of edge vector and cross product of normals
                    H = crossP.normalized();
                    double sinT = crossP.dot(H);
                    double cosT = p3.normal().dot(p3n.normal());
                    theta = atan2(sinT, cosT) * 180.0/PI;
                    assert(!qIsNaN(theta));
                    if(fabs(theta)>ANGLEPRECISION && crossP.norm()>LENGTHPRECISION)
                    {
                        CoGn.rotate(edge.vertexAt(0), H, theta);
                    }

                    // convert to local coordinates
                    CoGnl = p3.globalToLocalPosition(CoGn);

                    A[0*rows+m] = 1.0;
                    A[1*rows+m] = CoGnl.x;
                    A[2*rows+m] = CoGnl.y;
                    Mu[0*rows+m] = i3n<int(uRHS.size()) ? uRHS[i3n] : 0.0;
                    Mu[1*rows+m] = i3n<int(vRHS.size()) ? vRHS[i3n] : 0.0;
                    Mu[2*rows+m] = i3n<int(wRHS.size()) ? wRHS[i3n] : 0.0;

                    m++;
                }
            }
            assert(m==p3.neighbourCount()+1);

            if(N==2)
            {
                // If 2 neighbours only, need to check the case where the three CoG points are aligned.
                // Make a line regression instead of the plane regression which will be unstable

                V01 = {A[1*m+1]-p3.m_CoG_l.x, A[2*m+1]-p3.m_CoG_l.y};
                V02 = {A[1*m+2]-p3.m_CoG_l.x, A[2*m+2]-p3.m_CoG_l.y};

                double sintheta = (V01.x*V02.y-V01.y*V02.x)/V01.norm()/V02.norm();

                // if the CoG of the 3 panels are aligned whithin less than a thereshold angle, make a regression line out of three points
//                if(fabs(sintheta)<15.0*PI/180.0) //--> beta17
                if(fabs(sintheta)<fabs(sin(35.0*PI/180.0)))
                {
                    U = V02-V01;
                    U.normalize();

                    x[0]=-V01.norm();  x[1]=0.0;  x[2]=V02.norm();

                    for(int j=0; j<3; j++)
                    {
                        yu[0]=Mu[j*rows+1];  yu[1]=Mu[j*rows+0];  yu[2]=Mu[j*rows+2];

                        au=0, bu=0;
                        bRegu = linearRegression(3,x,yu,au,bu);
                        if(bRegu)
                        {
                            localvel[j].x = -4.0*PI*au*U.x;
                            localvel[j].y = -4.0*PI*au*U.y;
                            localvel[j].z = 0.0;
                        }
                        else localvel[j].reset();

                    }

                    uLocal[3*i3] = uLocal[3*i3+1] = uLocal[3*i3+2] = localvel[0];
                    vLocal[3*i3] = vLocal[3*i3+1] = vLocal[3*i3+2] = localvel[1];
                    wLocal[3*i3] = wLocal[3*i3+1] = wLocal[3*i3+2] = localvel[2];

                    continue;
                }
            }

            assert(m==3||m==4);

            lapack_int info=0, n=3, nrhs=3;
            lapack_int lwork = -1; // start with query
            lapack_int lda = std::max(1,m); // LDA >= max(1,M)
            lapack_int ldb = std::max(1,m); // LDB >= MAX(1,M,N)
            double wkopt=0;
            char trans = 'N';

#ifdef OPENBLAS
            dgels_(&trans, &m, &n, &nrhs, A.data(), &lda, Mu.data(), &ldb, &wkopt, &lwork, &info,1);
#elif defined INTEL_MKL
            dgels_(&trans, &m, &n, &nrhs, A.data(), &lda, Mu.data(), &ldb, &wkopt, &lwork, &info);
#elif defined ACCELERATE
            dgels_(&trans, &m, &n, &nrhs, A.data(), &lda, Mu.data(), &ldb, &wkopt, &lwork, &info);
#endif
            lwork = int(wkopt);
            std::vector<double> work(lwork);

            if(lwork>0 && info==0)
            {
                work.resize(lwork);

#ifdef OPENBLAS
                dgels_(&trans, &m, &n, &nrhs, A.data(), &lda, Mu.data(), &ldb, work.data(), &lwork, &info,1);
#elif defined INTEL_MKL
                dgels_(&trans, &m, &n, &nrhs, A.data(), &lda, Mu.data(), &ldb, work.data(), &lwork, &info);
#elif defined ACCELERATE
                dgels_(&trans, &m, &n, &nrhs, A.data(), &lda, Mu.data(), &ldb, work.data(), &lwork, &info);
#endif
            }
            else info = 1;

            if(info!=0)
            {
                QString strange;
                strange = QString::asprintf("         error making doublet derivative for panel %d\n", i3);
                traceLog(strange);
                continue;
            }

            //    double µ = -(a*sl[0].x + b*sl[0].y + c*mu0);
            // the gradient is (dµ/dx, dµ/dy)

            for(int j=0; j<3; j++)
            {
                localvel[j].x = -4.0*PI * (Mu[j*rows+1]);
                localvel[j].y = -4.0*PI * (Mu[j*rows+2]);
                localvel[j].z = 0.0;
            }
        }
        uLocal[3*i3] = uLocal[3*i3+1] = uLocal[3*i3+2] = localvel[0];
        vLocal[3*i3] = vLocal[3*i3+1] = vLocal[3*i3+2] = localvel[1];
        wLocal[3*i3] = wLocal[3*i3+1] = wLocal[3*i3+2] = localvel[2];
    }

    Vector3d ug,vg, wg;
    for(uint i3=0; i3<SingleNeighbourPanels.size(); i3++)
    {
        int index = SingleNeighbourPanels.at(i3);
        Panel3 const &p3 = m_Panel3.at(index);
        int in = -1;
        for(int k=0; k<3; k++)
        {
            // the neighbour can be at any of the three edges - find it
            if(p3.neighbour(k)>=0)
            {
                in = p3.neighbour(k);
                break;
            }
        }
        Panel3 const &p3n = m_Panel3.at(in);

        // convert the neighbour's local velocities to global
        p3n.localToGlobal(uLocal.at(3*in), ug);
        p3n.localToGlobal(vLocal.at(3*in), vg);
        p3n.localToGlobal(wLocal.at(3*in), wg);

        //convert the velocities to the local system of the base panel
        uLocal[3*index] = uLocal[3*index+1] = uLocal[3*index+2] = p3.globalToLocal(ug);
        vLocal[3*index] = vLocal[3*index+1] = vLocal[3*index+2] = p3.globalToLocal(vg);
        wLocal[3*index] = wLocal[3*index+1] = wLocal[3*index+2] = p3.globalToLocal(wg);
    }
}


void P3UniAnalysis::makeUnitRHSBlock(int iBlock)
{
    int blockSize = int(nPanels()/m_nBlocks) +1;
    int iStart = iBlock*blockSize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blockSize, maxRows);

    Vector3d Vx(1,0,0), Vy(0,1,0), Vz(0,0,1);
    Vector3d Omx(1,0,0), Omy(0,1,0), Omz(0,0,1);

    double phi = 0.0;
    Vector3d V;
    Vector3d leverarm_i3, leverarm_k3;

    int iStation = 0;
    for(int i3=iStart; i3<iMax; i3++)
    {
        Panel3 const &p3i = m_Panel3.at(i3);
        leverarm_i3.set(p3i.CoG()-m_pPolar3d->CoG());
        int row = i3;

        if(m_pPolar3d->bNeumann() || p3i.isMidPanel())
        {
            // first term of RHS is -V.n
            m_uRHS[row] = -Vx.dot(p3i.normal());
            m_vRHS[row] = -Vy.dot(p3i.normal());
            m_wRHS[row] = -Vz.dot(p3i.normal());
            m_pRHS[row] = - (leverarm_i3 * Omx).dot(p3i.normal());
            m_qRHS[row] = - (leverarm_i3 * Omy).dot(p3i.normal());
            m_rRHS[row] = - (leverarm_i3 * Omz).dot(p3i.normal());
        }
        else
        {
            m_uRHS[row] = 0;
            m_vRHS[row] = 0;
            m_wRHS[row] = 0;
            m_pRHS[row] = 0;
            m_qRHS[row] = 0;
            m_rRHS[row] = 0;
        }

        for(int k3=0; k3<nPanels(); k3++)
        {
            Panel3 const &p3k = m_Panel3.at(k3);
            leverarm_k3.set(p3i.CoG()-m_pPolar3d->CoG());

            if(p3k.isMidPanel())
            {
                // no source singularity on thin surfaces
            }
            else
            {
                if(m_pPolar3d->bNeumann() || p3i.isMidPanel())
                {
                    p3k.sourceVelocity(p3i.CoG(), i3==k3, V);
                    m_uRHS[i3] -= V.dot(p3i.normal()) * sourceStrength(p3k.normal(), Vx);
                    m_vRHS[i3] -= V.dot(p3i.normal()) * sourceStrength(p3k.normal(), Vy);
                    m_wRHS[i3] -= V.dot(p3i.normal()) * sourceStrength(p3k.normal(), Vz);
                    m_pRHS[i3] -= V.dot(p3i.normal()) * sourceStrength(p3k.normal(), leverarm_k3*Omx);
                    m_qRHS[i3] -= V.dot(p3i.normal()) * sourceStrength(p3k.normal(), leverarm_k3*Omy);
                    m_rRHS[i3] -= V.dot(p3i.normal()) * sourceStrength(p3k.normal(), leverarm_k3*Omz);
                }
                else //if(m_pWPolar->bDirichlet())
                {
                    p3k.sourcePotential(p3i.CoG(), i3==k3, phi);
                    m_uRHS[i3] -= phi * sourceStrength(p3k.normal(), Vx);
                    m_vRHS[i3] -= phi * sourceStrength(p3k.normal(), Vy);
                    m_wRHS[i3] -= phi * sourceStrength(p3k.normal(), Vz);
                    m_pRHS[i3] -= phi * sourceStrength(p3k.normal(), leverarm_k3*Omx);
                    m_qRHS[i3] -= phi * sourceStrength(p3k.normal(), leverarm_k3*Omy);
                    m_rRHS[i3] -= phi * sourceStrength(p3k.normal(), leverarm_k3*Omz);
                }
            }
            if(isCancelled()) break;
        }

        if(m_pPolar3d->bNeumann() || p3i.isMidPanel())
        {
            if(p3i.isLeading()) iStation++; // move on to the next strip
        }
        else
        {
            if(p3i.isTopPanel() && p3i.isTrailing()) iStation++; // move on to the next strip
        }
        if(isCancelled()) break;
    }
    (void)iStation;
}


void P3UniAnalysis::makeUnitDoubletStrengths(double alpha, double beta)
{
    //______________________________________________________________________________________
    //    reconstruct all results from cosine and sine unit vectors
    int N = nPanels();

    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);
    double cosb = cos(-beta*PI/180.0); //change of beta sign introduced in v7.24 to be consistent with AVL
    double sinb = sin(-beta*PI/180.0); //change of beta sign introduced in v7.24 to be consistent with AVL

    for(int i3=0; i3<N; i3++)
    {
        m_Mu[3*i3]   = cosa*cosb*m_uRHS.at(i3) + sinb*m_vRHS.at(i3) + sina*cosb*m_wRHS.at(i3);
        m_Mu[3*i3+1] = cosa*cosb*m_uRHS.at(i3) + sinb*m_vRHS.at(i3) + sina*cosb*m_wRHS.at(i3);
        m_Mu[3*i3+2] = cosa*cosb*m_uRHS.at(i3) + sinb*m_vRHS.at(i3) + sina*cosb*m_wRHS.at(i3);
    }
}


/** Applicable when the velocity field is not a solid body movement, i.e. with virtual twist */
void P3UniAnalysis::makeRHSBlock(int iBlock, double *RHS, std::vector<Vector3d> const &VField, const Vector3d *normals) const
{
    int blockSize = int(nPanels()/m_nBlocks) +1;
    int iStart = iBlock*blockSize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blockSize, maxRows);

    double phiSource(0);
    Vector3d vNASA, VPanel, Normal;

    int iStation = 0;
    for(int i3=iStart; i3<iMax; i3++)
    {
        Panel3 const &p3i = m_Panel3.at(i3);
        Normal = normals ? normals[i3] : p3i.normal();
        int row = i3;

        VPanel = VField.at(i3);

        if(m_pPolar3d->bNeumann() || p3i.isMidPanel())
        {
            // first term of RHS is -V.n
            RHS[row] = - VPanel.dot(Normal);
        }
        else RHS[row] = 0.0;

        for(int k3=0; k3<nPanels(); k3++)
        {
            Panel3 const &p3k = m_Panel3.at(k3);
            if(p3k.isMidPanel())
            {
                // no source singularity on thin surfaces
            }
            else
            {
                double sigma = sourceStrength(p3k.normal(), VPanel);

                if(m_pPolar3d->bNeumann() || p3i.isMidPanel())
                {
                    p3k.sourceVelocity(p3i.CoG(), i3==k3, vNASA);
                    RHS[row] -= vNASA.dot(Normal) * sigma;
                }
                else //if(m_pWPolar->bDirichlet())
                {
                    p3k.sourcePotential(p3i.CoG(), i3==k3, phiSource);
                    RHS[row] -= phiSource * sigma;
                }
            }
            if(isCancelled()) break;
        }

        if(m_pPolar3d->bNeumann() || p3i.isMidPanel())
        {
            if(p3i.isLeading()) iStation++; // move on to the next strip
        }
        else
        {
            if(p3i.isTopPanel() && p3i.isTrailing()) iStation++; // move on to the next strip
        }
        if(isCancelled()) break;
    }
    (void)iStation;
}


/**
 * @brief makeNodeDoubletDensities converts the uniform doublet density resulting from a constant-strength analysis
 * into densities at the nodes. This allows the calculation of the doublet derivative at the panel's center as the
 * slope of the plane defined by the three node densities.
 * Also offers consistency with linear analysis which produces results at the nodes.
 */
void P3UniAnalysis::makeVertexDoubletDensities(std::vector<double> const &muPanel, std::vector<double> &muNode) const
{
/*    for(int iNode=0; iNode<m_pRefTriMesh->nNodes(); iNode++)
    {
        makeNodeAverage(iNode, muPanel, muNode); // something wrong in the method?
    }*/
    int N = nPanels();

    for(int i3=0; i3<N; i3++)
    {
        muNode[3*i3]   = muPanel.at(i3);
        muNode[3*i3+1] = muPanel.at(i3);
        muNode[3*i3+2] = muPanel.at(i3);
    }
}


void P3UniAnalysis::checkThinSurfaceSolution()
{
    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const &p3i = m_Panel3.at(i3);
        Vector3d Vsum;
        for(int k3=0; k3<nPanels(); k3++)
        {
            Panel3 const &p3k = m_Panel3.at(k3);
            Vector3d Vk;
            p3k.doubletN4023Velocity(p3i.CoG(), i3==k3, Vk, 0.0, true);
            Vsum += Vk *m_Mu.at(3*k3);
        }
        qDebug("V.n(%2d)=%13.7f", i3, Vsum.dot(p3i.normal()));
    }
}


/**
 * @brief Calculates the Cp coefficients at the panel nodes, whether the analysis was of the uniform or linear type.
 * Assumes that the unit velocity vectors induced by doublet densities at the panel nodes have been calculated at
 * the previous stage.
 */
void P3UniAnalysis::computeOnBodyCp(std::vector<Vector3d> const &VInf,
                                    std::vector<Vector3d> const &VLocal, std::vector<double>&Cp) const
{
    double QInf(0), Speed2(0), CpSup(0), CpInf(0);
    Vector3d VStream, VPanel0, VPanel1, VPanel2;
    Vector3d Vtotsup, Vtotinf;
    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);

        VStream = p3.globalToLocal(VInf.at(i3));
        QInf = VInf.at(i3).norm();

        if(p3.isMidPanel())
        {
        /* Correction made in v7.12 May 2021
         * NASA TN4023 paragraph 3.4 p.56:
         * Patches with IDENT=3 include both upper and lower surface velocity and pressure output.
         * On these patches the doublet gradient (Eq. (49)) provides the jump in tangential velocity
         * component between the upper and lower surfaces.
         * The program first computes the mean velocity (i.e., excluding the local panel's contribution)
         * at the panel's center then adds to this one half of the tangential velocity jump for the upper
         * surface and similarly subtracts for the lower surface.*/

            for(int k=0; k<3; k++)
            {
                Vtotsup = VStream + VLocal.at(3*i3+k) *0.5;
                Speed2 = Vtotsup.x*Vtotsup.x + Vtotsup.y*Vtotsup.y;
                CpSup = 1.0-Speed2/QInf/QInf;

                Vtotinf = VStream - VLocal.at(3*i3+k) *0.5;
                Speed2 = Vtotinf.x*Vtotinf.x + Vtotinf.y*Vtotinf.y;
                CpInf = 1.0-Speed2/QInf/QInf;

                Cp[3*i3+k] = CpSup-CpInf;
            }
        }
        else
        {
            VPanel0 = VStream + VLocal.at(3*i3);
            VPanel1 = VStream + VLocal.at(3*i3+1);
            VPanel2 = VStream + VLocal.at(3*i3+2);
            Cp[3*i3]   = 1.0-(VPanel0.x*VPanel0.x + VPanel0.y*VPanel0.y)/QInf/QInf;
            Cp[3*i3+1] = 1.0-(VPanel1.x*VPanel1.x + VPanel1.y*VPanel1.y)/QInf/QInf;
            Cp[3*i3+2] = 1.0-(VPanel2.x*VPanel2.x + VPanel2.y*VPanel2.y)/QInf/QInf;
        }
    }
}


/**
 * For a tri-uniform analysis, builds a row of vortons located at the wake panels streamwise edges.
 * The vorticity at the edges is the jump in doublet density between two adjacent panels x4.PI
 * The vorticity at the two tips is equal to the panel's doublet strength, i.e. zero vorticity
 * outside the wake.*/
void P3UniAnalysis::makeVortons(double dl, double const *mu3Vertex, int pos3, int nPanel3, int nStations, int nVtn0,
                                std::vector<Vorton> &vortons, std::vector<Vortex> &vortexneg) const
{
    double const *Mu3 = mu3Vertex;

    vortons.resize(2*nStations);
    vortexneg.resize(nStations);
    double gamma=0;

    int m=0;
    for(int i3=0; i3<nPanel3; i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3+pos3);
        if(p3.isTrailing())
        {
            assert(p3.iWake()>=0 && p3.iWake()<nWakePanels());
            Panel3 p3WU, p3WD;
            trailingWakePanels(m_WakePanel3.data() + p3.iWake(), p3WU, p3WD);

            if(p3.isMidPanel() || p3.isBotPanel())
            {
                if(p3.isMidPanel())
                {
                    int idxM = p3.index();
                    gamma = Mu3[3*idxM+2] *4.0*PI; // is the same as gLeft for this panel
                }
                else  if(p3.isBotPanel())
                {
                    int idxB = p3.index();
                    int idxU = nextTopTrailingPanelIndex(p3);
                    assert(idxU>=0);

                    gamma = (Mu3[3*idxU+2] - Mu3[3*idxB+2])*4.0*PI;
                }

                // Willis 2005 III.D p.9
                // A standard result is that the change in dipole strength along a surface is equivalent to vorticity oriented
                // in the surface tangential direction normal to the dipole gradient.
                // In the case of constant dipole panels, the vortex analogue is a vortex ring around the perimeter of the
                // given panel. Hence, the strength of the vortex line segment between two adjacent constant strength panels
                // is merely the difference in dipole strengths.

                if(p3.isLeftWingPanel())
                {
                    Segment3d const &leftedge  = p3WU.edge(1);
                    Segment3d const &rightedge = p3WD.edge(2);
                    vortons[2*m  ].setPosition(p3WU.vertexAt(0) + leftedge.unitDir()            *dl/2.0);
                    vortons[2*m+1].setPosition(p3WD.vertexAt(0) + rightedge.unitDir().reversed()*dl/2.0);
                    vortons[2*m  ].setVortex(leftedge.unitDir(),              gamma*dl);
                    vortons[2*m+1].setVortex(rightedge.unitDir().reversed(), -gamma*dl);
                    vortexneg[m].setNodes(p3WD.vertexAt(2), p3WD.vertexAt(0));
                }
                else
                {
                    Segment3d const &leftedge  = p3WD.edge(1);
                    Segment3d const &rightedge = p3WU.edge(2);
                    vortons[2*m  ].setPosition(p3WD.vertexAt(0) + leftedge.unitDir()            *dl/2.0);
                    vortons[2*m+1].setPosition(p3WD.vertexAt(1) + rightedge.unitDir().reversed()*dl/2.0);
                    vortons[2*m  ].setVortex(leftedge.unitDir(),              gamma*dl);
                    vortons[2*m+1].setVortex(rightedge.unitDir().reversed(), -gamma*dl);
                    vortexneg[m].setNodes(p3WD.vertexAt(0), p3WD.vertexAt(1));
                }

                vortexneg[m].setNodeIndex(0, nVtn0+2*m);
                vortexneg[m].setNodeIndex(1, nVtn0+2*m+1);
                vortexneg[m].setCirculation(-gamma); // negating circulation 4.PI Mu4
                m++;
            }
        }
    }
}


void P3UniAnalysis::testResults(double alpha, double beta, double QInf) const
{
    // check that perturbation potential & velocity are zero inside the body
    // check that residual is zero for each basis function, by recalculating the scalar products

    double const *mu = m_Mu.data();
    double const *sigma = m_Sigma.data();

    Vector3d VInf = objects::windDirection(alpha, beta)*QInf;
    Vector3d Vel;
//    double phi=0;

    if(nPanels()<25) return;

    Panel3 const & p3 = m_Panel3.at(12);
s_DebugPts.clear();
s_DebugVecs.clear();
    double Z = 0.05;
    double n = 10.0;
    for(int id=0; id<int(n); id++)
    {
        double d = -Z +double(id)*2.0*Z/n;
        Vector3d C = p3.CoG()+ p3.normal() * d;

        getVelocityVector(C, mu, sigma, Vel, 0.0, false, false);
        Vel += VInf;
        s_DebugPts.push_back(C);
        s_DebugVecs.push_back(Vel);

        qDebug(" %13g  %13g", d, Vel.dot(p3.normal()));
    }
}

