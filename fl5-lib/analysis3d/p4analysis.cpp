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

#include <QString>
#include <QDebug>

#include <thread>
#include <iostream>


#include <p4analysis.h>

#include <matrix.h>
#include <objects2d.h>
#include <panel4.h>
#include <polar3d.h>
#include <stabderivatives.h>
#include <vortex.h>



P4Analysis::P4Analysis() : PanelAnalysis()
{
    m_pRefQuadMesh = nullptr;
}


bool P4Analysis::initializeAnalysis(Polar3d const *pPolar3d, int nRHS)
{
    if(!PanelAnalysis::initializeAnalysis(pPolar3d, nRHS)) return false;

    std::string strange;

    int N = nPanels();
    int memsize = 0;

    if(nRHS>0)
    {
        if(!allocateMatrix(N)) return false;

        memsize += allocateRHS4(nRHS);
    }

//    memsize += allocateSailResultsArray(nRHS),
    if(memsize<0)
    {
        strange = "Could not allocate memory for panel arrays";
        traceStdLog(strange+"\n\n");
        return false;
    }

    m_bWarning  = false;
    m_bMatrixError = false;

    return true;
}


void P4Analysis::makeMu(int qrhs)
{
    for(int i4=0; i4<nPanels(); i4++)
    {
        m_Mu[qrhs*nPanels() +i4] = m_uRHS[i4];
    }
}


void P4Analysis::makeLocalVelocities(std::vector<double> const &uRHS, const std::vector<double> &vRHS, std::vector<double> const &wRHS,
                                     std::vector<Vector3d> &uVLocal, std::vector<Vector3d> &vVLocal, std::vector<Vector3d> &wVLocal,
                                     Vector3d const &WindDirection) const
{
    double Cp(0);

    for (int i4=0; i4<nPanels(); i4++)
    {
        if(!m_Panel4.at(i4).isMidPanel() || !m_pPolar3d->isVLM())
        {
            getDoubletDerivative(i4, uRHS.data(), Cp, uVLocal[i4], WindDirection);
        }
        if(isCancelled()) return;
    }

    for (uint i4=0; i4<vRHS.size(); i4++)
    {
        if(!m_Panel4.at(i4).isMidPanel() || !m_pPolar3d->isVLM())
        {
            getDoubletDerivative(i4, vRHS.data(), Cp, vVLocal[i4], WindDirection);
        }
        if(isCancelled()) return;
    }

    for (uint i4=0; i4<wRHS.size(); i4++)
    {
        if(!m_Panel4.at(i4).isMidPanel() || !m_pPolar3d->isVLM())
        {
            getDoubletDerivative(i4, wRHS.data(), Cp, wVLocal[i4], WindDirection);
        }
        if(isCancelled()) return;
    }
}


void P4Analysis::combineLocalVelocities(double alpha, double beta, std::vector<Vector3d> &VLocal) const
{
    VLocal.resize(nPanels());
    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);
    double cosb = cos(-beta*PI/180.0); //change of beta sign introduced in v7.24 to be consistent with AVL
    double sinb = sin(-beta*PI/180.0); //change of beta sign introduced in v7.24 to be consistent with AVL
    for(uint i=0; i<VLocal.size(); i++)
    {
        VLocal[i].x = cosa*cosb* m_uVLocal.at(i).x + sinb*m_vVLocal.at(i).x + sina*cosb* m_wVLocal.at(i).x;
        VLocal[i].y = cosa*cosb* m_uVLocal.at(i).y + sinb*m_vVLocal.at(i).y + sina*cosb* m_wVLocal.at(i).y;
        VLocal[i].z = cosa*cosb* m_uVLocal.at(i).z + sinb*m_vVLocal.at(i).z + sina*cosb* m_wVLocal.at(i).z;
    }
}


/**
 * Reserves the memory necessary to RHS arrays.
 * @return the size of the allocated memory, in bytes.
 */
int P4Analysis::allocateRHS4(int nRHS)
{
    int memsize = 0;
    int nPanel4 = nPanels();
    int size = nPanel4 * nRHS;
    if(size==0) return -1;
    try
    {
//        m_RHS.resize(size);
        m_Sigma.resize(size);
        m_Mu.resize(size);
        m_Cp.resize(size);

        memsize += 5*uint(size)*sizeof(double);

        memsize += uint(size)*sizeof(Vector3d);
    }
    catch(std::exception &e)
    {
        traceStdLog(e.what());
        return -1;
    }

    m_uRHS.resize(nPanel4);
    m_vRHS.resize(nPanel4);
    m_wRHS.resize(nPanel4);
    m_pRHS.resize(nPanel4);
    m_qRHS.resize(nPanel4);
    m_rRHS.resize(nPanel4);
    m_cRHS.resize(nPanel4);

    memsize += 14 * uint(nPanel4) * sizeof(double);

    std::fill(m_uRHS.begin(), m_uRHS.end(), 0);
    std::fill(m_vRHS.begin(), m_vRHS.end(), 0);
    std::fill(m_wRHS.begin(), m_wRHS.end(), 0);
    std::fill(m_pRHS.begin(), m_pRHS.end(), 0);
    std::fill(m_qRHS.begin(), m_qRHS.end(), 0);
    std::fill(m_rRHS.begin(), m_rRHS.end(), 0);
    std::fill(m_cRHS.begin(), m_cRHS.end(), 0);

    m_uVLocal.resize(nPanel4);
    m_vVLocal.resize(nPanel4);
    m_wVLocal.resize(nPanel4);
    memsize += 2 * uint(nPanel4) * sizeof(Vector3d);

    return memsize;
}


/**
 * Alpha is used in the case of Neumann BC for thick surfaces to transfer the knwo part of the doublet densities to the RHS
 * Neumann BC are only active for control polars. T123 and T7 polars should use Dirichlet BC.
*/
void P4Analysis::makeInfluenceMatrix()
{
    m_bMatrixError = false;

    s_DebugPts.clear();
    s_DebugVecs.clear();

    if(s_bMultiThread)
    {
        std::vector<std::thread> threads;

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            threads.push_back(std::thread(&P4Analysis::makeMatrixBlock, this, iBlock));
        }

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            threads[iBlock].join();
        }
    }
    else
    {
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            if(m_bMatrixError) break; // no point to continue
            makeMatrixBlock(iBlock);
        }
    }

    if(m_bMatrixError)
    {
        traceStdLog("      Error building the influence matrix\n");
    }
}


void P4Analysis::makeMatrixBlock(int iBlock)
{
    int N = nPanels();
    Vector3d C, V;

    double phi=0.0;

    // for each panel
    int blocksize = int(double(nPanels())/double(m_nBlocks))+1; // add one to compensate for rounding errors
    int iStart = iBlock*blocksize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blocksize, maxRows);

    // for each panel
    for(int i4=iStart; i4<iMax; i4++)
    {
        Panel4 const &p4i = m_Panel4.at(i4);
        //for each Boundary Condition point

        if(p4i.isMidPanel() && m_pPolar3d->isVLM())
            C = p4i.m_CtrlPt;
        else
            C = p4i.m_CollPt;

        for(int k4=0; k4<nPanels(); k4++)
        {
            Panel4 const &p4k = m_Panel4.at(k4);
            //for each panel, get the unit doublet or vortex influence at the boundary condition pt
            if(m_pPolar3d->bNeumann() || p4i.isMidPanel())
            {
                getDoubletVelocity(C, p4k, V, 0.000, true, true);

                if(std::isnan(V.x) || std::isnan(V.y) || std::isnan(V.z))
                {
                    QString strange;
                    strange = QString::asprintf("      *** numerical error when calculating the influence of panel %d on panel %d ***\n", k4, i4);
                    traceLog(strange);
                    m_bMatrixError = true;
                    return;
                }

                double d =  V.dot(p4i.normal());

                if(s_bDoublePrecision) m_aijd[uint(i4*N+k4)] = d;
                else                   m_aijf[uint(i4*N+k4)] = float(d);

/*                if(!p4i.isMidPanel())
                {
                    // thick body with Neumann BC - should always be a T6 polar and a single Opp, no linear combinations
                    m_uRHS[i4] -=  d*freeStreamPotential(p4k.CoG(), alpha, beta, 1.0);
                }*/
            }
            else if(m_pPolar3d->bDirichlet())
            {
                getDoubletPotential(C, i4==k4, p4k, phi, 0.0, true, true);

                if(std::isnan(phi))
                {
                    QString strange;
                    strange = QString::asprintf("      *** numerical error when calculating the influence of panel %d on panel %d ***\n", k4, i4);
                    traceLog(strange);
                    m_bMatrixError = true;
                    return;
                }

                if(s_bDoublePrecision)  m_aijd[uint(i4*N+k4)] = phi;
                else                    m_aijf[uint(i4*N+k4)] = float(phi);
            }

            if(isCancelled()) break;
        }
        if(isCancelled()) break;
    }
//    display_mat(m_aijf.data(), nPanels(), nPanels());
}


void P4Analysis::makeUnitRHSBlock(int iBlock)
{
    int blockSize = int(nPanels()/m_nBlocks) +1;
    int iStart = iBlock*blockSize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blockSize, maxRows);

    Vector3d Vx(1,0,0), Vy(0,1,0), Vz(0,0,1);
    Vector3d Omx(1,0,0), Omy(0,1,0), Omz(0,0,1);

    double  phi=0;
    Vector3d V, C;
    Vector3d leverarm_i4, leverarm_k4;

    for (int i4=iStart; i4<iMax; i4++)
    {
        Panel4 const &p4i = m_Panel4.at(i4);
        C.set(p4i.ctrlPt(m_pPolar3d->isVLM()));

        leverarm_i4.set(C-m_pPolar3d->CoG());

        if(p4i.isMidPanel() || m_pPolar3d->bNeumann())
        {
            // first term of RHS is -V.n
            m_uRHS[i4] = -Vx.dot(p4i.normal());
            m_vRHS[i4] = -Vy.dot(p4i.normal());
            m_wRHS[i4] = -Vz.dot(p4i.normal());
            m_pRHS[i4] = - (leverarm_i4 * Omx).dot(p4i.normal());
            m_qRHS[i4] = - (leverarm_i4 * Omy).dot(p4i.normal());
            m_rRHS[i4] = - (leverarm_i4 * Omz).dot(p4i.normal());
        }
        else if(m_pPolar3d->bDirichlet())
        {
            m_uRHS[i4] = 0;
            m_vRHS[i4] = 0;
            m_wRHS[i4] = 0;
            m_pRHS[i4] = 0;
            m_qRHS[i4] = 0;
            m_rRHS[i4] = 0;
        }

        for (int k4=0; k4<nPanels(); k4++)
        {
            Panel4 const &p4k = m_Panel4.at(k4);
            // Consider only the panels positioned on thick surfaces,
            // since the source strength is zero on thin surfaces
            if(p4k.isMidPanel())
            {
                // no source singularity on thin surfaces
            }
            else
            {
                // Add to RHS the source influence of panel k4 on panel i4
                leverarm_k4.set(p4k.CoG()-m_pPolar3d->CoG());

                if(m_pPolar3d->bNeumann() || p4i.isMidPanel())
                {
                    // Using the zero perturbation inside condition.
                    // In the case of thick Neumann surfaces, the velocity is evaluated on the inside point
                    // to ensure Vi = Vinf inside
                    // In the case of thin Neumann surfaces, the source component is zero anyway on the panel

                    getSourceVelocity(C, i4==k4, p4k, V);
                    m_uRHS[i4] -= V.dot(p4i.normal()) * sourceStrength(p4k.normal(), Vx);
                    m_vRHS[i4] -= V.dot(p4i.normal()) * sourceStrength(p4k.normal(), Vy);
                    m_wRHS[i4] -= V.dot(p4i.normal()) * sourceStrength(p4k.normal(), Vz);
                    m_pRHS[i4] -= V.dot(p4i.normal()) * sourceStrength(p4k.normal(), leverarm_k4*Omx);
                    m_qRHS[i4] -= V.dot(p4i.normal()) * sourceStrength(p4k.normal(), leverarm_k4*Omy);
                    m_rRHS[i4] -= V.dot(p4i.normal()) * sourceStrength(p4k.normal(), leverarm_k4*Omz);
                }
                else if(m_pPolar3d->bDirichlet())
                {
                    //NASA4023 eq. (20)
                    getSourcePotential(C, p4k, phi);
                    m_uRHS[i4] -= phi * sourceStrength(p4k.normal(), Vx);
                    m_vRHS[i4] -= phi * sourceStrength(p4k.normal(), Vy);
                    m_wRHS[i4] -= phi * sourceStrength(p4k.normal(), Vz);
                    m_pRHS[i4] -= phi * sourceStrength(p4k.normal(), leverarm_k4*Omx);
                    m_qRHS[i4] -= phi * sourceStrength(p4k.normal(), leverarm_k4*Omy);
                    m_rRHS[i4] -= phi * sourceStrength(p4k.normal(), leverarm_k4*Omz);
                }
            }
        }

        if(isCancelled()) return;
    }
}


void P4Analysis::makeRHSBlock(int iBlock, double *RHS, std::vector<Vector3d> const &VField, Vector3d const*normals) const
{
    int blockSize = int(nPanels()/m_nBlocks) +1;
    int iStart = iBlock*blockSize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blockSize, maxRows);

    double  phi(0), sigma(0);
    Vector3d V, C, VPanel, Normal;

    for (int i4=iStart; i4<iMax; i4++)
    {
        Panel4 const &p4i = m_Panel4.at(i4);
        Normal = normals ? normals[i4] : p4i.normal();

        VPanel = VField.at(i4);

        if(p4i.isMidPanel())
        {
            // first term of RHS is -V.n
            RHS[i4] = - VPanel.dot(Normal);
        }
        else if(m_pPolar3d->bNeumann() && !p4i.isMidPanel())
        {
            assert(false); // invalid option - Cf. 'Neumann BC - implementation notes"
            RHS[i4] = - VPanel.dot(Normal);
        }
        else if(m_pPolar3d->bDirichlet()) RHS[i4] = 0.0;


        if(p4i.isMidPanel() && m_pPolar3d->isVLM()) C = p4i.m_CtrlPt;
        else                                              C = p4i.m_CollPt;

        for (int k4=0; k4<nPanels(); k4++)
        {
            Panel4 const &p4k = m_Panel4.at(k4);
            // Consider only the panels positioned on thick surfaces,
            // since the source strength is zero on thin surfaces
            if(p4k.isMidPanel())
            {
                // no source singularity on thin surfaces
            }
            else
            {
                // Define the source strength on panel pp

                sigma = sourceStrength(normals ? normals[k4] : p4k.normal(), VPanel);

                // Add to RHS the source influence of panel k4 on panel i4

                if(m_pPolar3d->bNeumann() || p4i.isMidPanel())
                {
                    // Using the zero perturbation inside condition.
                    // In the case of thick Neumann surfaces, the velocity is evaluated on the inside point
                    // to ensure Vi = Vinf inside
                    // In the case of thin Neumann surfaces, the source component is zero anyway on the panel

                    getSourceVelocity(C, i4==k4, p4k, V);
                    RHS[i4] -= V.dot(Normal) * sigma;
                }
                else if(m_pPolar3d->bDirichlet())
                {
                    //NASA4023 eq. (20)
                    getSourcePotential(C, p4k, phi);
                    RHS[i4] -= phi * sigma;
                }
            }
        }

        if(isCancelled()) return;
    }
}


void P4Analysis::makeWakeMatrixBlock(int iBlock)
{
    int blockSize = int(double(nPanels())/double(m_nBlocks)) +1;
    int iStart = iBlock*blockSize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blockSize, maxRows);

    Vector3d V, C, TrPt;
    double phi(0);
    std::vector<double>   PHC(m_nStations, 0);
    std::vector<Vector3d> VHC(m_nStations);


    int Size = nPanels();

    for(int i4=iStart; i4<iMax; i4++)
    {
        Panel4 const &p4i = m_Panel4.at(i4);
        C = p4i.m_CollPt; // VLM does not use the wake contribution

        //____________________________________________________________________________
        //build the contributions of each wake column at point C
        //we have m_NWakeColum to consider
        int pw=0;
        for (int kw=0; kw<m_nStations; kw++)
        {
            PHC[kw] = 0.0;
            VHC[kw].set(0.0,0.0,0.0);

            do
            {
                if(m_pPolar3d->bNeumann() || p4i.isMidPanel())
                {
                    getDoubletVelocity(C, m_WakePanel4.at(pw), V, 0.0, false, true);
                    VHC[kw] += V;
                }
                else
                {
                    getDoubletPotential(C, false, m_WakePanel4.at(pw), phi, 0.0, false, true);
                    PHC[kw] += phi;
                }

                if(m_WakePanel4.at(pw).iPD()<0)
                {
                    pw++; // set the index to the first wake panel of the next wake column
                    break; // the way out
                }
                pw++; // move downstream one
            }
            while(pw<nWakePanels()); // limit
        }

        //____________________________________________________________________________
        //Add the contributions to the matrix coefficients and to the RHS

        for(int k4=0; k4<nPanels(); k4++) //for each matrix column
        {
            Panel4 const &p4k = m_Panel4.at(k4);
//            if(pMatWakeContrib) pMatWakeContrib[i4*Size+pp] = 0.0;

            // Is the panel pp shedding a wake?
            if(p4k.isTrailing())
            {
                // If so, we need to add the contributions of the wake column
                // shedded by this panel to the RHS and to the Matrix

                // Get trailing point where the jump in potential is evaluated
//                TrPt = (p4k.m_Node[1]+p4k.m_Node[2])/2.0;
                TrPt = p4k.CoG();

                double sign = p4k.isBotPanel() ? -1.0 : 1.0;
                double MatWakeContrib = 0.0;

                //The panel shedding a wake is on a thin surface
                if(m_pPolar3d->bNeumann() || p4i.isMidPanel())
                {
                    //then add the velocity contribution of the wake column to the matrix coefficient
                    MatWakeContrib += sign * VHC.at(p4k.iWakeColumn()).dot(p4i.normal());
                    //we do not add the term Phi_inf_KWPUM - Phi_inf_KWPLM (eq. 44) since it is 0, thin edge
                }
                else if(m_pPolar3d->bDirichlet())
                {
                    //then add the potential contribution of the wake column to the matrix coefficient
                    MatWakeContrib += sign * PHC.at(p4k.iWakeColumn());
                    //we do not add the term Phi_inf_KWPUM - Phi_inf_KWPLM (eq. 44) since it is 0, thin edge
                }

                if(s_bDoublePrecision) m_aijd[uint(i4*Size+k4)] += MatWakeContrib;
                else                   m_aijf[uint(i4*Size+k4)] += float(MatWakeContrib);
            }
            if(isCancelled()) return;
        }
    }
}


void P4Analysis::getDoubletPotential(Vector3d const &C, bool bSelf, Panel4 const &p4, double &phi,
                                     double coreradius, bool bUseRFF, bool bIncludingBoundVortex) const
{
    if(p4.isMidPanel())
    {
        VLMGetVortexInfluence(p4, C, &phi, nullptr, bIncludingBoundVortex, m_pPolar3d->TrefftzDistance());
//        phi = 0.0;
    }
    else
    {
        p4.doubletN4023Potential(C, bSelf, phi, coreradius, bUseRFF);
    }

    if(m_pPolar3d->bGroundEffect() || m_pPolar3d->bFreeSurfaceEffect())
    {
        double coef = m_pPolar3d->bGroundEffect() ? 1.0 : -1.0;
        Vector3d CG;
        double phiG=0.0;
        CG.set(C.x, C.y, -C.z-2.0*m_pPolar3d->groundHeight());

        p4.doubletN4023Potential(CG, false, phiG, coreradius);
        phi += phiG*coef;
        return;
    }
}


void P4Analysis::getDoubletVelocity(Vector3d const &C, Panel4 const &p4, Vector3d &V,
                                    double coreradius, bool bUseRFF, bool bIncludingBoundVortex) const
{
    if(m_pPolar3d->isVLM() && p4.isMidPanel())
    {
        VLMGetVortexInfluence(p4, C, nullptr, &V, bIncludingBoundVortex, m_pPolar3d->TrefftzDistance());
    }
    else
    {
        // use either method
//        p4.doubletN4023Velocity(C, V, coreradius, bUseRFF);
        p4.doubletVortexVelocity(C, V, coreradius, bUseRFF);
    }

    if(m_pPolar3d->bGroundEffect() || m_pPolar3d->bFreeSurfaceEffect())
    {
        double coef = m_pPolar3d->bGroundEffect() ? 1.0 : -1.0;
        Vector3d VG, CG;

        CG.set(C.x, C.y, -C.z-2.0*m_pPolar3d->groundHeight());

        if(m_pPolar3d->isVLM() && p4.isMidPanel())
        {
            VLMGetVortexInfluence(p4, CG, nullptr, &VG, bIncludingBoundVortex, m_pPolar3d->TrefftzDistance());
        }
        else
        {
//            p4.doubletN4023Velocity(CG, VG, coreradius, bUseRFF);
            p4.doubletVortexVelocity(CG, VG, coreradius, bUseRFF);
        }

        V.x += VG.x * coef;
        V.y += VG.y * coef;
        V.z -= VG.z * coef;
    }
}


/**
 * Returns the influence at point C of a uniform source distribution on the panel pPanel
 * The panel is necessarily located on a thick surface, else the source strength is zero
 * @param C the point where the influence is to be evaluated
 * @param pPanel a pointer to the Panel with the doublet strength
 * @param V the perturbation velocity at point C
 * @param phi the potential at point C
 */
void P4Analysis::getSourcePotential(Vector3d const &C, Panel4 const &p4, double &phi) const
{
    // pass argument core radius = 0.0 since wake panels do not have source density
    p4.sourceN4023Potential(C, phi, 0.0);


    if(m_pPolar3d->bGroundEffect() || m_pPolar3d->bFreeSurfaceEffect())
    {
        double coef = m_pPolar3d->bGroundEffect() ? 1.0 : -1.0;

        double phiG = 0;

        Vector3d CG(C.x, C.y, -C.z-2.0*m_pPolar3d->groundHeight());
        p4.sourceN4023Potential(CG, phiG, 0.0);
        phi += phiG * coef;
    }
}


void P4Analysis::getSourceVelocity(Vector3d const &C, bool bSelf, Panel4 const &p4, Vector3d &V) const
{
    // pass argument core radius = 0.0 since wake panels do not have source density
    p4.sourceN4023Velocity(C, bSelf, V, 0.0);

    if(m_pPolar3d->bGroundEffect() || m_pPolar3d->bFreeSurfaceEffect())
    {
        double coef = m_pPolar3d->bGroundEffect() ? 1.0 : -1.0;

        Vector3d VG, CG;

        CG.set(C.x, C.y, -C.z-2.0*m_pPolar3d->groundHeight());
        p4.sourceN4023Velocity(CG, false, VG, 0.0);
        V.x += VG.x * coef;
        V.y += VG.y * coef;
        V.z -= VG.z * coef;
    }
}


void P4Analysis::getVelocityVector(Vector3d const &C,
                                   double const *Mu, double const *Sigma, Vector3d &VT, double coreradius,
                                   bool bWakeOnly, bool bMultiThread) const
{
    if(isCancelled()) return;
    // only 5 parameters max are allowed in the sub call to the parallized process, so make the others global
    tmp_coreradius = coreradius;
    tmp_bWakeOnly = bWakeOnly;
    tmp_Mu = Mu;
    tmp_Sigma = Sigma;

    std::vector<Vector3d> VBlock(m_nBlocks);

    if(bMultiThread)
    {
        std::vector<std::thread> threads;

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
//            futureSync.addFuture(QtConcurrent::run(&P4Analysis::velocityVectorBlock, this, iBlock, C, &VBlock[iBlock]));
            threads.push_back(std::thread(&P4Analysis::velocityVectorBlock, this, iBlock, C, &VBlock[iBlock]));
        }

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            threads[iBlock].join();
        }
    }
    else
    {
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            velocityVectorBlock(iBlock, C, &VBlock[iBlock]);
        }
    }

    VT.set(0.0,0.0,0.0); // total induced velocity
    for(int ib=0; ib<m_nBlocks; ib++) VT += VBlock[ib];

    if(m_pPolar3d->bVortonWake())
    {
        double vtncorelength = m_pPolar3d->vortonCoreSize()*m_pPolar3d->referenceChordLength();

        Vector3d VVtn;
        getVortonVelocity(C, vtncorelength, VVtn, bMultiThread);
        VT += VVtn;
    }
}


void P4Analysis::velocityVectorBlock(int iBlock, Vector3d const &C, Vector3d *VT) const
{
    // for each panel
    int blocksize = int(double(nPanels())/double(m_nBlocks))+1; // add one to compensate for rounding errors
    int iStart = iBlock*blocksize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blocksize, maxRows);

    Vector3d V, Vs, VV;
    double sign = 0;

    for (int i4=iStart; i4<iMax; i4++)
    {
        if(isCancelled()) return;
        Panel4 const &p4 = m_Panel4.at(i4);

        if(m_pPolar3d->isVLM())
        {
            getDoubletVelocity(C, p4, V, tmp_coreradius, true, !tmp_bWakeOnly);
            VT->x += V.x * tmp_Mu[i4];
            VT->y += V.y * tmp_Mu[i4];
            VT->z += V.z * tmp_Mu[i4];
        }
        else
        {
            if(!tmp_bWakeOnly)
            {
                if(!p4.isMidPanel()) //otherwise Sigma[pp] =0.0, so contribution is zero also
                {
                    getSourceVelocity(C, false, p4, V);
                    VT->x += V.x * tmp_Sigma[i4];
                    VT->y += V.y * tmp_Sigma[i4];
                    VT->z += V.z * tmp_Sigma[i4];
                }
                getDoubletVelocity(C, p4, V, tmp_coreradius, true, true);
                VT->x += V.x * tmp_Mu[i4];
                VT->y += V.y * tmp_Mu[i4];
                VT->z += V.z * tmp_Mu[i4];
            }

            // Is the panel pp shedding a wake?
            if(p4.isTrailing())
            {
                //If so, add the contribution of the wake column shedded by this panel
                if(p4.isBotPanel()) sign=-1.0; else sign=1.0;
                int iw4 = p4.iWake();
//                int irow=0;
                while(iw4>=0 /* && iw4<wakepanel4.size() */)
                {
                    assert(iw4<nWakePanels());
                    Panel4 const &p4w = m_WakePanel4.at(iw4);
                    // do not use RFF approximation for wake panels
                    getDoubletVelocity(C, p4w, V, tmp_coreradius, false, true);

                    VT->x += V.x * tmp_Mu[i4]*sign;
                    VT->y += V.y * tmp_Mu[i4]*sign;
                    VT->z += V.z * tmp_Mu[i4]*sign;

                    iw4 = p4w.m_iPD;
//                    irow++;
                }
            }
        }
    }
}


/**
 * Make the velocity field induced by the Vorton wake
 * Includes the influence of the cancelling vortices at the wake panels trailing edges
 */
/*void P4Analysis::makeRHSVWVelocities(std::vector<Vector3d>& VPW, bool bVLM)
{
    Vector3d C;
    for(int p=0; p<nPanels(); p++)
    {
        Panel4 const &p4 = m_Panel4.at(p);
        C.set(p4.ctrlPt(bVLM));

        VPW[p].set(0,0,0);

        getVortonVelocity(C, VPW[p]);
    }
}*/


/**
 * Returns the perturbation velocity vector far downstream using a line vortex model for the wake
 * irrespective of the analysis method.
 */
void P4Analysis::getFarFieldVelocity(const Vector3d &C, const std::vector<Panel4> &panel4, const double *Mu,
                                     Vector3d &VT, double coreradius) const
{
    if(isCancelled()) return;

    VT.set(0.0,0.0,0.0);

    Vector3d VL, VR, A, B;
    double fardist = m_pPolar3d->TrefftzDistance();
    for (uint i4=0; i4<panel4.size(); i4++)
    {
        if(isCancelled()) return;
        Panel4 const &p4 = panel4.at(i4);
        if(p4.isTrailing())
        {
            A = p4.TA();
            A.x += fardist;
            VL = vortexInducedVelocity(p4.TA(), A, C, coreradius);
            VT +=  VL*(+Mu[i4]);

            B = p4.TB();
            B.x += fardist;
            VR = vortexInducedVelocity(p4.TB(), B, C, coreradius);
            // circulations of each seg are oppposite
            VT +=  VR*(-Mu[i4]);

//if(bTrace) qDebug("  %3d  %13g ---  %13g   %13g   %13g  ---  %13g   %13g   %13g", i4,  Mu[i4], VL.x, VL.y, VL.z, VR.x, VR.y, VR.z);
        }
    }
    VT *=  4.0*PI;
}


double P4Analysis::getPotential(Vector3d const &C, double const *mu, double const *sigma) const
{
    double phiT=0;
    double phi=0, sign=0;
    double coreradius=0.0;
    Vector3d V;

    phiT = 0.0;
    for (int i4=0; i4<nPanels(); i4++)
    {
        Panel4 const &p4 = m_Panel4.at(i4);

        if(!p4.isMidPanel()) //otherwise Sigma[pp] =0.0, so contribution is zero also
        {
            getSourcePotential(C, p4, phi);
            phiT += phi * sigma[i4];
        }
        getDoubletPotential(C, false, p4, phi, 0.0, true, true);
        phiT += phi * mu[i4];

        // Is the panel pp shedding a wake?
        if(p4.isTrailing() && !p4.isMidPanel())
        {
            //If so, we need to add the contribution of the wake column shedded by this panel
            if(p4.isBotPanel()) sign=-1.0; else sign=1.0;
            int iw4 = p4.iWake();
//            int irow=0;
            while(iw4>=0)
            {
                // do not use RFF approximation for wake panels
                getDoubletPotential(C, false, m_WakePanel4.at(iw4), phi, coreradius, false, true);
                phiT += phi * mu[i4]*sign;

                iw4 = m_WakePanel4.at(iw4).m_iPD;
//                irow++;
            }
        }
    }
    return phiT;
}


/**
* Returns the perturbation velocity created at a point C by a horseshoe or quad vortex with unit circulation located on a panel pPanel
* @param pPanel a pointer to the Panel where the vortex is located
* @param C the point where the perrturbation is evaluated
* @param V a reference to the resulting perturbation velocity vector
* @param bIncludingBound true if the influence of the bound vector should be included. Not necessary in the case of a far-field evaluation.
*/
void P4Analysis::VLMGetVortexInfluence(Panel4 const &p4, Vector3d const &C, double *phi, Vector3d *V,
                                       bool bIncludingBoundVortex, double fardist) const
{
    if(!m_pPolar3d) return;
    Vector3d AA1, BB1, VT;
    int p = p4.index();

    if(V)
    {
        V->x = V->y = V->z = 0.0;
    }

    if(m_pPolar3d->isVLM1())
    {
        // get the horseshoe vortex's influence
        if(V)   objects::VLMCmnVelocity(p4.m_VA, p4.m_VB, C, *V, bIncludingBoundVortex, fardist);
        if(phi)
        {
            // Create far field points
            Vector3d FarA, FarB;
            FarA.x = p4.m_VA.x +  fardist;
            FarA.y = p4.m_VA.y;
            FarA.z = p4.m_VA.z;// + (Far_x-A.x) * tan(m_Alpha*PI/180.0);
            FarB.x = p4.m_VB.x +  fardist;
            FarB.y = p4.m_VB.y;
            FarB.z = p4.m_VB.z;// + (Far_x-B.x) * tan(m_Alpha*PI/180.0);
            objects::VLMQmnPotential(p4.m_VA, p4.m_VB, FarA, FarB, C, *phi);
        }
    }
    else
    {
        // quad vortices
        if(!p4.m_bIsTrailing)
        {
            if(bIncludingBoundVortex)
            {
                if(V)   objects::VLMQmnVelocity( p4.m_VA, p4.m_VB, m_Panel4.at( p-1).m_VA, m_Panel4.at(p-1).m_VB, C, *V);
                if(phi) objects::VLMQmnPotential(p4.m_VA, p4.m_VB, m_Panel4.at( p-1).m_VA, m_Panel4.at(p-1).m_VB, C, *phi);
            }
        }
        else // is trailing
        {
            // then panel p is shedding a wake
            // since Panel p+1 does not exist...
            // define the points AA=A+1 and BB=B+1
            AA1.x = p4.TA().x + (p4.TA().x-p4.m_VA.x)/3.0;
            AA1.y = p4.TA().y;
            AA1.z = p4.TA().z;
            // error corrected in beta 08
//            double BB1x = p4.m_Node[2].x + (p4.m_Node[2].x-p4.VA.x)/3.0;
//            BB1.x = p4.m_Node[2].x + (p4.m_Node[2].x-p4.VA.x)/3.0;
             BB1.x = p4.TB().x + (p4.TB().x-p4.m_VB.x)/3.0;
//             qDebug("  %13g  %13g", BB1x, BB1.x);
            BB1.y = p4.TB().y;
            BB1.z = p4.TB().z;

            // get the quad vortex's influence
            if (bIncludingBoundVortex)
            {
                if(phi) objects::VLMQmnPotential(p4.m_VA, p4.m_VB, AA1, BB1, C, *phi);
                if(V)   objects::VLMQmnVelocity( p4.m_VA, p4.m_VB, AA1, BB1, C, *V);
            }

            if(!m_pPolar3d->bVortonWake())
            {
                // add a trailing horseshoe vortex's influence to simulate the wake
                if(V)
                {
                    objects::VLMCmnVelocity(AA1,BB1,C,VT,bIncludingBoundVortex, fardist);
                    V->x += VT.x;
                    V->y += VT.y;
                    V->z += VT.z;
                }
                if(phi)
                {
                    // Create far field points
                    Vector3d FarA, FarB;
                    FarA.x = AA1.x +  fardist;
                    FarA.y = AA1.y;
                    FarA.z = AA1.z;// + (Far_x-A.x) * tan(m_Alpha*PI/180.0);
                    FarB.x = BB1.x +  fardist;
                    FarB.y = BB1.y;
                    FarB.z = BB1.z;// + (Far_x-B.x) * tan(m_Alpha*PI/180.0);
                    double phiT = 0;
                    objects::VLMQmnPotential(AA1, BB1, FarA, FarB, C, phiT);
                    *phi += phiT;
                }
            }
        }
    }
}


/**
* Calculates the forces using a far-field method.
* Calculates the moments by a near field method, i.e. direct summation on the panels.
* @param Mu a pointer to the array of doublet strengths or vortex circulations
* @param Sigma a pointer to the array of source strengths
* @param VInf a reference to the array of the velocity vectors on the panels
* @param Force the resulting force vector
* @param Moment the resulting moment vector
*/
void P4Analysis::forces(double const *Mu4, double const *Sigma4, double alpha, double beta, Vector3d const &CoG, bool ,
                        std::vector<Vector3d> const &VInf, Vector3d &Force, Vector3d &Moment)
{
    if(!m_pPolar3d) return;

    double Cp(0.0), viscousDrag(0.0);
    double QInf(0.0);
    Vector3d  C, WindDirection, WindNormal, PanelLeverArm, Wg, vortex;
    Vector3d Velocity, viscousMoment, PanelForce;

    //   Define the wind axis
    WindNormal = objects::windNormal(alpha, beta);
    WindDirection = objects::windDirection(alpha, beta);


    Force.set( 0.0, 0.0, 0.0);
    Moment.set(0.0, 0.0, 0.0);
    viscousDrag = 0.0;
    viscousMoment.set(0.0,0.0,0.0);

//    int m=0;
    for(int i4=0; i4<nPanels(); i4++)
    {
        Panel4 &p4 = m_Panel4[i4];

        if(!m_pPolar3d->isVLM())
        {
            if(p4.isTrailing() && (p4.isBotPanel()||p4.isMidPanel()))
            {
                // get the last quad of the wake column
//                assert(p4.m_iWake>=0 && p4.m_iWake<m_WakePanel4.size());
//                Panel4 const *p4W = m_WakePanel4.data() + p4.m_iWake;
//                C = trailingWakePoint(p4W);
                // modified in 7.01 beta09 to use vortex lines rather than wake panels
                C = (p4.TA() + p4.TB())/2.0;
                C.x = m_pPolar3d->TrefftzDistance()/2.0;
//                getVelocityVector(C, m_Panel4, m_WakePanel4, Mu4, Sigma4, Wg, 0.00001, true);
                getFarFieldVelocity(C, m_Panel4, Mu4, Wg, Vortex::coreRadius());
                Wg *=  4.0*PI;
                Wg *= 1.0/2.0;

                int idxB = p4.index();
                int idxU = nextTopTrailingPanelIndex(p4);

                double GammaStrip = 0.0;
                if(!p4.isMidPanel())
                {
                    // the trailing nodes have indexes 1 & 2
                    double gU = Mu4[idxU];
                    double gB = Mu4[idxB];
                    GammaStrip = -(gU - gB) *4.0*PI;
                    vortex = p4.trailingVortex() * (-1.0);
                }
                else
                {
                    GammaStrip = -Mu4[idxB] *4.0*PI;
                    vortex = p4.trailingVortex();
                }

                Wg += VInf.at(i4);

                Vector3d stripforce  = Wg * vortex;
                stripforce *= GammaStrip;      // N/rho

                Force += stripforce;

//                m++;
            }
        }
        else if (m_pPolar3d->isVLM())
        {
            if(p4.isTrailing())
            {
                assert(p4.isMidPanel());
                int pp = i4;

                do
                {
                    Panel4 const &pp4 = m_Panel4.at(pp);
                    if(m_pPolar3d->isVLM1() || pp4.isTrailing())
                    {
                        C = p4.m_CtrlPt;
                        // evaluate at half the ff distance, so that we get influence of upstream and downstream parts of the vortices
                        // then divide the influence by 2.0
                        C.x = m_pPolar3d->TrefftzDistance()/2.0;

                        getVelocityVector(C, Mu4, Sigma4, Wg, Vortex::coreRadius(), true, s_bMultiThread);

                        // The trailing point sees both the upstream and downstream parts of the trailing vortices
                        // Hence it sees twice the downwash.
                        // So divide by 2 to account for this.
                        Wg *= 1.0/2.0;

                        Wg += VInf.at(i4);

                        //induced force
                        Vector3d vortex = p4.trailingVortex();
                        Vector3d dF = Wg * vortex;
                        dF *= Mu4[i4];           // N/rho
                        Force += dF;    // N/rho
                    }
                    if(pp4.isLeading()) break; // is the next strip
                    pp++;
                    if(pp>=nPanels()) break; // safety break
                }while(true);

//                m++;
            }
        }
    }

    //On-Body moment
    Vector3d VLocal, ForcePt;
    Moment.set(0.0,0.0,0.0);
    for(int i4=0; i4<nPanels(); i4++)
    {
        Panel4 const &p4 = m_Panel4.at(i4);
        Velocity = VInf.at(i4);
        QInf = Velocity.norm();

        if(!m_pPolar3d->isVLM())
        {
            getDoubletDerivative(i4, Mu4, Cp, VLocal,  Velocity);
            PanelForce = p4.normal() * (-Cp) * p4.area() *1/2.*QInf*QInf;      // Newtons/rho
            PanelLeverArm = p4.CoG() - CoG;
        }
        else
        {
            // for each panel along the chord, add the lift coef
            ForcePt = p4.vortexPosition();
            PanelForce  = WindDirection * p4.trailingVortex();
            PanelForce *= 2.0 * Mu4[i4] /QInf;                                 //Newtons/q

            if(m_pPolar3d->isVLM() && m_pPolar3d->isVLM2() && !p4.isLeading())
            {
                Vector3d Force = WindDirection * p4.trailingVortex();
                Force      *= 2.0 * Mu4[i4+1] /QInf;                          //Newtons/q
                PanelForce -= Force;
            }
            PanelForce *= 0.5*QInf*QInf;  // N/rho
            PanelLeverArm = ForcePt - CoG;
        }
        Moment += PanelLeverArm * PanelForce ;                     // N.m/rho
    }

    if(m_pPolar3d->isViscous())
    {
        Force += WindDirection * viscousDrag;
        Moment += viscousMoment;
    }

    Force  *= m_pPolar3d->density();                          // N
    Moment *= m_pPolar3d->density();                          // N.m
}


void P4Analysis::getDoubletDerivative(int p, double const *Mu, double &Cp, Vector3d &VLocal, Vector3d const &VInf) const
{
    double DELQ=0, DELP=0;
    double mu0=0, mu1=0, mu2=0, x0=0, x1=0, x2=0;

    Vector3d VTot;//total local panel speed
    Vector3d S2, Sl2;

    Panel4 const &p4 = m_Panel4.at(p);

    int PL = p4.m_iPL;
    int PR = p4.m_iPR;
    int PU = p4.m_iPU;
    int PD = p4.m_iPD;

    if(PL>=0 && PR>=0)
    {
        //we have two side neighbours
        x1  = 0.0;
        x0  = x1 - p4.SMQ - m_Panel4.at(PL).SMQ;
        x2  = x1 + p4.SMQ + m_Panel4.at(PR).SMQ;

        if(fabs(x0-x1)>LENGTHPRECISION && fabs(x1-x2)>LENGTHPRECISION && fabs(x2-x0)>LENGTHPRECISION)
        {
            mu0 = Mu[PL];
            mu1 = Mu[p];
            mu2 = Mu[PR];
            DELQ =      mu0 *(x1-x2)       /(x0-x1)/(x0-x2)
                    + mu1 *(2.0*x1-x0-x2)/(x1-x0)/(x1-x2)
                    + mu2 *(x1-x0)       /(x2-x0)/(x2-x1);
        }
    }
    else if(PL>=0 && PR<0)
    {
        // no right neighbour
        // do we have two left neighbours?
        if(m_Panel4.at(PL).m_iPL>=0)
        {
            x2  = 0.0;
            x1  = x2 - p4.SMQ  - m_Panel4.at(PL).SMQ;
            x0  = x1 - m_Panel4.at(PL).SMQ - m_Panel4[m_Panel4.at(PL).m_iPL].SMQ;
            if(fabs(x0-x1)>LENGTHPRECISION && fabs(x1-x2)>LENGTHPRECISION && fabs(x2-x0)>LENGTHPRECISION)
            {
                mu0 = Mu[m_Panel4.at(PL).m_iPL];
                mu1 = Mu[PL];
                mu2 = Mu[p];
                DELQ =      mu0 *(x2-x1)       /(x0-x1)/(x0-x2)
                        + mu1 *(x2-x0)       /(x1-x0)/(x1-x2)
                        + mu2 *(2.0*x2-x0-x1)/(x2-x0)/(x2-x1);
            }
        }
        else
        {
            //calculate the derivative on two panels only
            DELQ = -(Mu[PL]-Mu[p])/(p4.SMQ  + m_Panel4.at(PL).SMQ);
        }
    }
    else if(PL<0 && PR>=0)
    {
        // no left neighbour
        // do we have two right neighbours?
        if(m_Panel4[PR].m_iPR>=0)
        {
            x0  = 0.0;
            x1  = x0 + p4.SMQ  + m_Panel4[PR].SMQ;
            x2  = x1 + m_Panel4[PR].SMQ + m_Panel4[m_Panel4[PR].m_iPR].SMQ;
            if(fabs(x0-x1)>LENGTHPRECISION && fabs(x1-x2)>LENGTHPRECISION && fabs(x2-x0)>LENGTHPRECISION)
            {
                mu0 = Mu[p];
                mu1 = Mu[PR];
                mu2 = Mu[m_Panel4[PR].m_iPR];
                DELQ =      mu0 *(2.0*x0-x1-x2)/(x0-x1)/(x0-x2)
                        + mu1 *(x0-x2)       /(x1-x0)/(x1-x2)
                        + mu2 *(x0-x1)       /(x2-x0)/(x2-x1);
            }
        }
        else
        {
            //calculate the derivative on two panels only
            DELQ = (Mu[PR]-Mu[p])/(p4.SMQ  + m_Panel4[PR].SMQ);
        }
    }
    else
    {
        DELQ = 0.0;
        //Cannot calculate a derivative on one panel only
    }

    if(PU>=0 && PD>=0)
    {
        //we have one upstream and one downstream neighbour
        x1  = 0.0;
        x0  = x1 - p4.SMP - m_Panel4.at(PU).SMP;
        x2  = x1 + p4.SMP + m_Panel4.at(PD).SMP;
        if(fabs(x0-x1)>LENGTHPRECISION && fabs(x1-x2)>LENGTHPRECISION && fabs(x2-x0)>LENGTHPRECISION)
        {
            mu0 = Mu[PU];
            mu1 = Mu[p];
            mu2 = Mu[PD];
            DELP =      mu0 *(x1-x2)       /(x0-x1)/(x0-x2)
                    + mu1 *(2.0*x1-x0-x2)/(x1-x0)/(x1-x2)
                    + mu2 *(x1-x0)       /(x2-x0)/(x2-x1);
        }
    }
    else if(PU>=0 && PD<0)
    {
        // no downstream neighbour
        // do we have two upstream neighbours?
        if(m_Panel4.at(PU).m_iPU>=0)
        {
            x2  = 0.0;
            x1  = x2 - p4.SMP  - m_Panel4.at(PU).SMP;
            x0  = x1 - m_Panel4.at(PU).SMP  - m_Panel4[m_Panel4.at(PU).m_iPU].SMP;
            if(fabs(x0-x1)>LENGTHPRECISION && fabs(x1-x2)>LENGTHPRECISION && fabs(x2-x0)>LENGTHPRECISION)
            {
                mu0 = Mu[m_Panel4.at(PU).m_iPU];
                mu1 = Mu[PU];
                mu2 = Mu[p];
                DELP =      mu0 *(x2-x1)       /(x0-x1)/(x0-x2)
                        + mu1 *(x2-x0)       /(x1-x0)/(x1-x2)
                        + mu2 *(2.0*x2-x0-x1)/(x2-x0)/(x2-x1);
            }
        }
        else
        {
            //calculate the derivative on two panels only
            DELP = -(Mu[PU]-Mu[p])/(p4.SMP  + m_Panel4.at(PU).SMP);
        }
    }
    else if(PU<0 && PD>=0)
    {
        // no upstream neighbour
        // do we have two downstream neighbours?
        if(m_Panel4.at(PD).m_iPD>=0)
        {
            x0  = 0.0;
            x1  = x0 + p4.SMP  + m_Panel4.at(PD).SMP;
            x2  = x1 + m_Panel4.at(PD).SMP + m_Panel4[m_Panel4.at(PD).m_iPD].SMP;
            if(fabs(x0-x1)>LENGTHPRECISION && fabs(x1-x2)>LENGTHPRECISION && fabs(x2-x0)>LENGTHPRECISION)
            {
                mu0 = Mu[p];
                mu1 = Mu[PD];
                mu2 = Mu[m_Panel4.at(PD).m_iPD];
                DELP =      mu0 *(2.0*x0-x1-x2)/(x0-x1)/(x0-x2)
                        + mu1 *(x0-x2)       /(x1-x0)/(x1-x2)
                        + mu2 *(x0-x1)       /(x2-x0)/(x2-x1);
            }
        }
        else
        {
            //calculate the derivative on two panels only
            DELP = (Mu[PD]-Mu[p])/(p4.SMP  + m_Panel4.at(PD).SMP);
        }
    }
    else
    {
        DELP = 0.0;
    }

    if(p4.isTopPanel()) DELP = -DELP; // changed the connection order in v7

    //find middle of side 2
//    S2 = (m_Node[p4.m_iTA] + m_Node[p4.m_iTB])/2.0 - p4.CollPt;
    S2 = (p4.m_Node[1]+p4.m_Node[2])/2.0 - p4.m_CollPt;

    //convert to local coordinates
    Sl2   = p4.globalToLocal(S2);
    VTot  = p4.globalToLocal(VInf);

    //in panel referential
    VLocal.x = -4.0*PI*(p4.SMP*DELP - Sl2.y*DELQ)/Sl2.x;
    VLocal.y = -4.0*PI*DELQ;
//    Vl.z =  4.0*PI*Sigma[p];

    VTot +=VLocal;

    VTot.z = 0.0;

    double Speed2 = VTot.x*VTot.x + VTot.y*VTot.y;
    double QInf = VInf.norm();
    Cp  = 1.0-Speed2/QInf/QInf;
}

/**
 * Evaluates the cross-flow forces by application of the Kutta-Joukowski theorem
 * Drela § 5.7.
 * The induced drag is evaluated separately in the Trefftz plane or in the vorton wake
 * */
void P4Analysis::inducedForce(int nPanels, double QInf, double alpha, double beta, int pos,
                               Vector3d &ForceBodyAxes, SpanDistribs &SpanResFF) const
{
    if(!m_pPolar3d) return;
    double GammaStrip(0);
    Vector3d VInf;
    Vector3d vortex, stripforce; // strip and global forces, in body axes

    //   Define wind axes
    Vector3d winddir = objects::windDirection(alpha, beta);

    VInf = winddir * QInf;

    //dynamic pressure, kg/m³
    double qDyn = 0.5 * m_pPolar3d->density() * QInf * QInf;
    double const *Mu4 = m_Mu.data();

    int m=0;
    for(int i4=0; i4<nPanels; i4++)
    {
        Panel4 const &p4 = m_Panel4.at(i4+pos);

        Vector3d surfacenormal = p4.surfaceNormal();

        if(!m_pPolar3d->isVLM())
        {
            if(p4.isTrailing() && (p4.isBotPanel()||p4.isMidPanel()))
            {
                int idxB = p4.index();
                int idxU = nextTopTrailingPanelIndex(p4);

                if(!p4.isMidPanel())
                {
                    // the trailing nodes have indexes 1 & 2
                    double gU = Mu4[idxU];
                    double gB = Mu4[idxB];
                    GammaStrip = -(gU - gB) *4.0*PI;
                    vortex = p4.trailingVortex() * (-1.0);
                }
                else
                {
                    GammaStrip = -Mu4[idxB] *4.0*PI;
                    vortex = p4.trailingVortex();
                }
                SpanResFF.m_Gamma[m] = GammaStrip;

                stripforce  = VInf * vortex;
                stripforce *= GammaStrip * m_pPolar3d->density();     // N
                stripforce *= 1.0/ qDyn;     // N/q

                //____________________________
                // Project on wind axes
                SpanResFF.m_Cl[m]  = stripforce.dot(surfacenormal) /SpanResFF.stripArea(m); // is just (Vinf x Gamma).dot(surfacenormal)
                ForceBodyAxes     += stripforce;                                 // N/q
                SpanResFF.m_F[m]   = stripforce * qDyn;                             // N, body axes
                m++;
            }
        }
        else if (m_pPolar3d->isVLM())
        {
            if(p4.isTrailing())
            {
                stripforce.set(0,0,0);
                assert(p4.isMidPanel());
                int pp = i4;
                SpanResFF.m_Gamma[m] = 0;
                do
                {
                    Panel4 const &pp4 = m_Panel4.at(pp+pos);
                    if(m_pPolar3d->isVLM1() || pp4.isTrailing())
                    {
                        // The trailing point sees both the upstream and downstream parts of the trailing vortices
                        // Hence it sees twice the downwash.
                        // So divide by 2 to account for this.

                        SpanResFF.m_Gamma[m] += Mu4[pp+pos];

                        //induced force
                        Vector3d dF  = VInf * p4.trailingVortex();
                        dF *= Mu4[pp+pos];       // N/rho
                        stripforce += dF;        // N/rho
                    }
                    if(pp4.isLeading()) break; // is the next strip
                    pp++;
                    if(pp>=nPanels) break; // safety break
                }
                while(true);

                stripforce *= 2./QInf/QInf; //N/q
                //____________________________
                // Project on wind axes
                //qDebug(" %2d  %13.7f  %13.7f  %13.7f  %13.7f",m, C.y, stripforce.x,stripforce.y,stripforce.z);
                SpanResFF.m_Cl[m]  = stripforce.dot(surfacenormal) /SpanResFF.stripArea(m);
                ForceBodyAxes     += stripforce;                           // N/q
                SpanResFF.m_F[m]   = stripforce * qDyn;                       // N,  V=1
                m++;
            }
        }
    }
}


/** Calculates the induced drag in the Trefftz plane
 * The Trefttz plane is half-way down the wake panels to avoid end-effects
 */
void P4Analysis::trefftzDrag(int nPanels, double QInf, double alpha, double beta, int pos,
                               Vector3d &FFForce, SpanDistribs &SpanResFF) const
{
    double inducedAngle(0);
    double GammaStrip(0);
    Vector3d C, Wg;
    Vector3d vortex, stripforce, ForceBodyAxes; // strip and global forces, in body axes

    //   Define wind axes
    Vector3d winddir = objects::windDirection(alpha, beta);

    //dynamic pressure, kg/m³
    double qDyn = 0.5 * m_pPolar3d->density() * QInf * QInf;

    double const *Mu4    = m_Mu.data();
    double const *Sigma4 = m_Sigma.data();

    int m=0;
    for(int i4=0; i4<nPanels; i4++)
    {
        Panel4 const &p4 = m_Panel4.at(i4+pos);

        Vector3d surfacenormal = p4.surfaceNormal();

        if(!m_pPolar3d->isVLM())
        {
            if(p4.isTrailing() && (p4.isBotPanel()||p4.isMidPanel()))
            {
                Panel4 const *p4w = m_WakePanel4.data() + p4.iWake();
                // modified in 7.01 beta 09 to use vortex lines rather than wake panels
//                C = (p4.TA() + p4.TB())/2.0;
//                C += winddir*m_pPolar3d->TrefftzDistance()/2.0;
                // modified in 7.01 beta 12 to use the mid wake point
                C = midWakePoint(p4w);

                getVelocityVector(C, Mu4, Sigma4, Wg, Vortex::coreRadius(), true, s_bMultiThread);
//                getFarFieldVelocity(C, m_Panel4, Mu4, Wg, Vortex::coreRadius());
                Wg *= 1.0/2.0;
//                Wg += winddir;

                SpanResFF.m_Vd[m] = Wg;
                inducedAngle = atan2(Wg.dot(surfacenormal), QInf);
                SpanResFF.m_Ai[m] = inducedAngle*180.0/PI;

                int idxB = p4.index();
                int idxU = nextTopTrailingPanelIndex(p4);

                if(!p4.isMidPanel())
                {
                    double gU = Mu4[idxU];
                    double gB = Mu4[idxB];
                    GammaStrip = -(gU - gB) *4.0*PI;
                    vortex = p4.trailingVortex() * (-1.0);
                }
                else
                {
                    GammaStrip = -Mu4[idxB] *4.0*PI;
                    vortex = p4.trailingVortex();
                }

                stripforce  = Wg * vortex;
                stripforce *= GammaStrip;     // N/rho

                stripforce *= m_pPolar3d->density() / qDyn;     // N/q

                //____________________________
                // Project on wind axes
//                SpanResFF.m_Cl[m]  = stripforce.dot(surfacenormal) /SpanResFF.stripArea(m);
                SpanResFF.m_ICd[m] = stripforce.dot(winddir) /SpanResFF.stripArea(m);
                SpanResFF.m_F[m]  += stripforce * qDyn;                        // N, body axes
                ForceBodyAxes     += stripforce;                            // N/q
                m++;
            }
        }
        else if (m_pPolar3d->isVLM())
        {
            if(p4.isTrailing())
            {
                stripforce.set(0,0,0);
                assert(p4.isMidPanel());
                int pp = i4;
                SpanResFF.m_Gamma[m] = 0.0;
                do
                {
                    Panel4 const &pp4 = m_Panel4.at(pp+pos);
                    if(m_pPolar3d->isVLM1() || pp4.isTrailing())
                    {
                        C = p4.ctrlPt(true);

                        // evaluate at half the ff distance, so that we get influence of upstream and downstream parts of the vortices
                        // then divide the influence by 2.0 since point ought to be at infinity with no downstream wake
                        C.x = m_pPolar3d->TrefftzDistance()/2.0;

                        getVelocityVector(C, Mu4, Sigma4, Wg, Vortex::coreRadius(), true, s_bMultiThread);


                        // The trailing point sees both the upstream and downstream parts of the trailing vortices
                        // Hence it sees twice the downwash.
                        // So divide by 2 to account for this.
                        Wg *= 1.0/2.0;
//                        Wg += winddir;

                        if(pp4.isTrailing())
                        {
                            SpanResFF.m_Vd[m] = Wg;
                            inducedAngle = atan2(Wg.dot(surfacenormal), QInf);
                            SpanResFF.m_Ai[m] = inducedAngle*180.0/PI;
                        }

                        //induced force
                        Vector3d dF  = Wg * p4.trailingVortex();
                        dF *= Mu4[pp+pos];       // N/rho
                        stripforce += dF;        // N/rho
                    }
                    if(pp4.isLeading()) break; // is the next strip
                    pp++;
                    if(pp>=nPanels) break; // safety break
                }
                while(true);

                stripforce *= 2./QInf/QInf; //N/q
                //____________________________
                // Project on wind axes
//                SpanResFF.m_Cl[m]  = stripforce.dot(surfacenormal) /SpanResFF.stripArea(m);
                SpanResFF.m_ICd[m] = stripforce.dot(winddir) /SpanResFF.stripArea(m);
                SpanResFF.m_F[m]  += stripforce * qDyn;                        // N, body axes
                ForceBodyAxes     += stripforce;                           // N/q
                m++;
            }
        }
    }

    FFForce.set(ForceBodyAxes);    // N/q, body axes
}


/**
* Calculates the Cp coefficient on each panel, using the vortex circulations or the doublet strengths,
* depending on the analysis method.
* @param V0 the first value in the sequence, either aoa for type 1 & 2 polars or velocity for type 4
* @param VDelta the increment value of the input parameter, either aoa for type 1 & 2 polars or velocity for type 4
* @param nval the number of values in the sequence
*/
void P4Analysis::computeOnBodyCp(const std::vector<Vector3d> &VInf,
                                 std::vector<Vector3d> const &VLocal, std::vector<double>&Cp) const
{
    double QInf(0), Speed2(0), CpSup(0), CpInf(0);
    Vector3d Vl, Vtot, Vtotsup, Vtotinf;
    for (int i4=0; i4<nPanels(); i4++)
    {
        Panel4 const &p4 = m_Panel4.at(i4);

        Vl.x = VLocal.at(i4).x;
        Vl.y = VLocal.at(i4).y;
        Vl.z = m_Sigma.at(i4)*4.0*PI;
        Vtot.set(p4.globalToLocal(VInf.at(i4)));

        QInf = VInf.at(i4).norm();
        if(p4.isMidPanel())
        {
        /* Correction made in v7.12 May 2021
         * NASA TN4023 paragraph 3.4 p.56:
         * Patches with IDENT=3 include both upper and lower surface velocity and pressure output.
         * On these patches the doublet gradient (Eq. (49)) provides the jump in tangential velocity
         * component between the upper and lower surfaces.
         * The program first computes the mean velocity (i.e., excluding the local panel's contribution)
         * at the panel's center then adds to this one half of the tangential velocity jump for the upper
         * surface and similarly subtracts for the lower surface.*/

            Vtotsup = Vtot + Vl*0.5;
            Speed2 = Vtotsup.x*Vtotsup.x + Vtotsup.y*Vtotsup.y;
            CpSup = 1.0-Speed2/QInf/QInf;

            Vtotinf = Vtot - Vl*0.5;
            Speed2 = Vtotinf.x*Vtotinf.x + Vtotinf.y*Vtotinf.y;
            CpInf = 1.0-Speed2/QInf/QInf;

            Cp[i4] = CpSup-CpInf;
        }
        else
        {
            Vtot  += Vl;
            Speed2 = Vtot.x*Vtot.x + Vtot.y*Vtot.y;
            Cp[i4] = 1.0-Speed2/QInf/QInf;
        }
    }
}


void P4Analysis::makeUnitDoubletStrengths(double alpha, double beta)
{
    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);
    double cosb = cos(-beta *PI/180.0); //change of beta sign introduced in v7.24 to be consistent with AVL
    double sinb = sin(-beta *PI/180.0); //change of beta sign introduced in v7.24 to be consistent with AVL

    for(int p=0; p<nPanels(); p++)
    {
        m_Mu[p] = cosa*cosb*m_uRHS.at(p) + sinb*m_vRHS.at(p) + sina*cosb* m_wRHS.at(p);
    }
}


int P4Analysis::makeWakePanels(const Vector3d &WindDirection, bool bVortonWake)
{
    Vector3d pt;
    if(!m_pRefQuadMesh) return 0;

    m_pRefQuadMesh->getLastTrailingPoint(pt);

    if(!bVortonWake)
    {
        QuadMesh::makeWakePanels(m_Panel4,
                                 m_pPolar3d->NXWakePanel4(), m_pPolar3d->wakePanelFactor(), pt.x + m_pPolar3d->wakeLength(),
                                 WindDirection, m_RefWakePanel4, m_WakeNode, m_nStations, true);
    }
    else
    {
        QuadMesh::makeWakePanels(m_Panel4, 3, 1.0, m_pPolar3d->bufferWakeLength(), WindDirection, m_RefWakePanel4, m_WakeNode, m_nStations, false);
    }

    m_WakePanel4 = m_RefWakePanel4;
    return int(m_RefWakePanel4.size());
}


void P4Analysis::setQuadMesh(QuadMesh const &quadmesh)
{
    m_pRefQuadMesh = &quadmesh;
    m_Panel4 = quadmesh.panels();
    m_WakePanel4 = quadmesh.wakePanels(); // used by gl velocity makers
}


/**
 * Releases the memory allocated to the Panel and node arrays.
 * Sets the pointers to NULL and the matrixsize to 0.
 */
void P4Analysis::releasePanelArrays()
{
    PanelAnalysis::releasePanelArrays();

    m_Sigma.clear();
    m_Mu.clear();
    m_Cp.clear();
    m_Panel4.clear();
//    m_WakePanel4.clear();
    m_RefWakePanel4.clear();
    m_WakeNode.clear();
    m_RefWakeNode.clear();

}


void P4Analysis::savePanels()
{
    if(!m_pRefQuadMesh) return;
    //restore the panels and nodes;
    m_Panel4 = m_pRefQuadMesh->panels();
    m_RefWakePanel4 = m_WakePanel4;

    m_RefWakeNode = m_WakeNode;
}


void P4Analysis::restorePanels()
{
    if(!m_pRefQuadMesh) return;
    //restore the panels and nodes;
    m_Panel4 = m_pRefQuadMesh->panels();
    m_WakePanel4 = m_RefWakePanel4;

    m_WakeNode = m_RefWakeNode;
}


/**
* Performs the summation of on-body forces to calculate the total lift and drag forces
* @param Cp a pointer to the array of previously calculated Cp coefficients
* @param Alpha the aoa for which this calculation is performed
* @param Lift the resulting lift force
* @param Drag the resulting drag force
*/
void P4Analysis::sumPanelForces(double *Cp, double Alpha, double &Lift, double &Drag)
{
    Vector3d PanelForce;

    for(int p=0; p<nPanels(); p++)
    {
        PanelForce += m_Panel4.at(p).normal() * (-Cp[p]) * m_Panel4.at(p).area();
    }

    Lift = PanelForce.z * cos(Alpha*PI/180.0) - PanelForce.x * sin(Alpha*PI/180.0);
    Drag = PanelForce.x * cos(Alpha*PI/180.0) + PanelForce.z * sin(Alpha*PI/180.0);
}


/**
* Returns the estimation of the panel's lift coeficient based on the vortex circulation.
* @param p the index of the panel
* @param Gamma the pointer to the array of vortex circulations
* @param Cp a pointer to the array of resulting panel lift coefficients
* @param VInf the freestream velocity vector
*/
void P4Analysis::getVortexCp(const int &p, double *Gamma, double *Cp, Vector3d &VInf)
{
    Vector3d PanelForce, Force;
    // for each panel along the chord, add the lift coef
    PanelForce  = VInf * m_Panel4.at(p).trailingVortex();
    PanelForce *= Gamma[p] * m_pPolar3d->density();                 //Newtons

    if(!m_pPolar3d->isVLM1() && !m_Panel4.at(p).isLeading())
    {
        Force       = VInf* m_Panel4.at(p).trailingVortex();
        Force      *= Gamma[p+1] * m_pPolar3d->density();           //Newtons
        PanelForce -= Force;
    }

    Cp[p]  = -2.0 * PanelForce.dot(m_Panel4.at(p).normal()) /m_Panel4.at(p).area()/m_pPolar3d->density();
}


/**
 * Resets the panel frames from the array of nodes
*/
void P4Analysis::rebuildPanelsFromNodes(std::vector<Vector3d> const &node)
{
    // rebuild the panels from the rotated nodes
    for(int i4=0; i4<nPanels(); i4++)
    {
        Panel4 &p4 = m_Panel4[i4];
        p4.setPanelFrame(node.at(p4.m_iLA), node.at(p4.m_iLB), node.at(p4.m_iTA), node.at(p4.m_iTB));
    }
}


/**
 * Returns the center point of the of the wake column to which the panel belongs
 * Assumes that pWakePanel points to the leading panel of the wake column
 */
Vector3d P4Analysis::midWakePoint(Panel4 const*pWakePanel) const
{
    Vector3d Leading, Trailing, Mid;
    if(!pWakePanel) return Mid;
    Leading = (pWakePanel->LA()+pWakePanel->LB())/2.0;
    do
    {
        if(pWakePanel->m_iPD<0)
        {
            Trailing.set((pWakePanel->leftTrailingNode() + pWakePanel->rightTrailingNode())/2.0);
            break;
        }
        else pWakePanel = m_WakePanel4.data() + pWakePanel->m_iPD;
    }
    while (pWakePanel);

    Mid = (Leading+Trailing)*0.5;
    return Mid;
}


/**
 * Returns the center point of the trailing edge of the wake column to which the panel belongs
 */
Vector3d P4Analysis::trailingWakePoint(Panel4 const*pWakePanel) const
{
    Vector3d C;
    do
    {
        if(pWakePanel->m_iPD<0)
        {
            C.set((pWakePanel->leftTrailingNode() + pWakePanel->rightTrailingNode())/2.0);
            break;
        }
        else pWakePanel = m_WakePanel4.data() + pWakePanel->m_iPD;
    }
    while (pWakePanel);
    return C;
}


/**
 * Returns returns a pointer to the last wake panel in the column starting with pWakePanel
 */
Panel4 const *P4Analysis::trailingWakePanel(Panel4 const *pWakePanel) const
{
    do
    {
        if(pWakePanel->m_iPD<0)
        {
            break; // pWakePanel is now the last panel in the column
        }
        else pWakePanel = &m_WakePanel4[pWakePanel->m_iPD];
    }
    while (pWakePanel);

    return pWakePanel;
}



int P4Analysis::nextTopTrailingPanelIndex(Panel4 const &p4) const
{
    if(!p4.isBotPanel()) return -1;

    int index = p4.index();
    do
    {
        Panel4 const &p4k = m_Panel4.at(index);
        if(p4k.m_iPU>=0)
            index = p4k.m_iPU;
        else
            return index;
    }
    while (index>=0); //  = while(true)

    return -1;
}


void P4Analysis::scaleResultsToSpeed(double ratio)
{
    //______________________________________________________________________________________
    // Scale RHS and Sigma i.a.w. speeds (so far we have unit doublet and source strengths)
    for(int i4=0; i4<nPanels(); i4++)
    {
        m_Mu[i4]    *= ratio;
        m_Sigma[i4] *= ratio;
    }
}


/**
 * Computes the trimmed condition for a stability analysis
 * Method :
 *   - For level flight, find the a.o.a. such that Cm=0
 *   - Set trimmed parameters for level flight or other
*/
bool P4Analysis::computeTrimmedConditions(double mass, Vector3d const &CoG, double &alphaeq, double &u0, bool bFuseMi)
{
    double beta = 0.0;

    if(!getZeroMomentAngle(CoG, alphaeq, bFuseMi))
    {
        traceStdLog("      no zero-moment angle found...     \n");
        return false;
    }

    Vector3d VInf = objects::windDirection(alphaeq, beta);
    Vector3d WindNormal = objects::windNormal(alphaeq, beta);
    std::vector<Vector3d> tmpVField(nPanels(), VInf);

    makeSourceStrengths(objects::windDirection(alphaeq, 0.0));
    if (isCancelled()) return true;

    //reconstruct doublet strengths from unit cosine and sine vectors
    makeUnitDoubletStrengths(alphaeq, 0.0);
    if(isCancelled()) return false;

    //______________________________________________________________________________________
    // Calculate the trimmed conditions for this control setting and calculated Alpha_eq
/*        phi = bank angle
        V   = sqrt(2 m g / rho S CL cos(phi))    (airspeed)
        R   = V^2 / g tan(phi)        (turn radius, positive for right turn)
        W   = V / R                   (turn rate, positive for right turn)
        p   = 0                       (roll rate, zero for steady turn)
        q   = W sin(phi)              (pitch rate, positive nose upward)
        r   = W cos(phi)              (yaw rate, positive for right turn) */

    //so far we have a unit Vortex Strength
    // find the speeds which will create a lift equal to the weight

    double Lift = 0.0;

    u0 = 1.0;

    //extra drag is useless when calculating lifting velocity
    Vector3d Force, Moment;
    forces(m_Mu.data(), m_Sigma.data(), alphaeq, 0.0, CoG, bFuseMi, tmpVField, Force, Moment);


//    phi = m_pPolar3d->phi() *PI/180.0;
    Lift   = Force.dot(WindNormal);        //N/rho ; bank effect not included
    if(Lift<=PRECISION)
    {
        u0 = -100.0;
        QString strong = QString::asprintf("        Found a negative lift for Alpha=%.3f.... skipping the angle...\n", alphaeq);
        traceLog("\n"+strong);
        m_bWarning = true;
        return false;
    }
    else
    {
        u0 = sqrt(9.81 * mass / Force.z);
    }

    return true;
}


#define CM_ITER_MAX 50
/**
 * Finds the zero-pitching-moment aoa such that Cm=0.
 * Proceeds by iteration between -PI/4 and PI/4
 * @return true if an equlibrium angle was found false otherwise.
 */
bool P4Analysis::getZeroMomentAngle(Vector3d const &CoG, double &alphaeq, bool bFuseMi)
{
    double tmp=0;
    double eps = 1.e-7;

    int iter = 0;
    double a0 = -PI/6.0;
    double a1 =  PI/6.0;

    double a = 0.0;
    double Cm0 = computeCm(CoG, a0*180.0/PI, bFuseMi);
    double Cm1 = computeCm(CoG, a1*180.0/PI, bFuseMi);
    double Cm = 1.0;

    //are there two initial values of opposite signs?
    while(Cm0*Cm1>0.0 && iter <=20)
    {
        a0 *=0.9;
        a1 *=0.9;
        Cm0 = computeCm(CoG, a0*180.0/PI, bFuseMi);
        Cm1 = computeCm(CoG, a1*180.0/PI, bFuseMi);
        iter++;
        if(isCancelled()) break;
    }
    if(iter>=100 || isCancelled()) return false;

    iter = 0;

    //Cm0 and Cm1 are of opposite sign
    if(Cm0>Cm1)
    {
        tmp = Cm1;
        Cm1 = Cm0;
        Cm0 = tmp;
        tmp = a0;
        a0  =  a1;
        a1  = tmp;
    }

    while (fabs(Cm)>eps && iter<=CM_ITER_MAX)
    {
        a = a0 - (a1-a0) * Cm0/(Cm1-Cm0);
        Cm = computeCm(CoG, a*180.0/PI, bFuseMi);
        if(Cm>0.0)
        {
            a1  = a;
            Cm1 = Cm;
        }
        else
        {
            a0  = a;
            Cm0 = Cm;
        }
        iter++;
        if(isCancelled()) break;
    }

    if(iter>=CM_ITER_MAX || isCancelled()) return false;

    alphaeq = a*180.0/PI;

    return true;
}


/**
* Returns the geometric pitching moment coefficient for the specified angle of attack
* The effect of the viscous drag is not included.
*@param Alpha the aoa for which Cm is calculated
*/
double P4Analysis::computeCm(Vector3d const &CoG, double Alpha, bool bFuseMi)
{
    double Cm=0.0, cosa=0.0, sina=0.0, Gamma=0.0, Gammap1=0.0;
    Vector3d VInf, Force, PanelLeverArm, ForcePt, PanelForce, WindDirection, VLocal;
    double Speed2=0.0, Cp=0.0;

    // Define the wind axis
    cosa = cos(Alpha*PI/180.0);
    sina = sin(Alpha*PI/180.0);
    WindDirection.set( cosa, 0.0, sina);
    VInf.set(cosa, 0.0, sina);

    Cm = 0.0;
    for(int p=0; p<nPanels(); p++)
    {
        //write vector operations in-line, more efficient
        Panel4 const &p4 = m_Panel4.at(p);
        if(!p4.isFusePanel() || bFuseMi)
        {
            if(!m_pPolar3d->isVLM())
            {
                //first calculate Cp for this angle
                p4.globalToLocal(VInf, VLocal);
                VLocal += m_uVLocal.at(p)*cosa + m_wVLocal.at(p)*sina;
                Speed2 = VLocal.x*VLocal.x + VLocal.y*VLocal.y;
                Cp  = 1.0-Speed2; // QInf=unit, /1.0/1.0;
                m_Cp[p] = Cp;

                //next calculate the force acting on the panel
                ForcePt = p4.m_CollPt;
                PanelForce = p4.normal() * (-Cp) * p4.area();      // Newtons/q
            }
            else
            {
                // for each panel along the chord, add the lift coef
                Gamma   = m_uRHS.at(p)*cosa + m_wRHS.at(p)*sina;
                ForcePt = p4.vortexPosition();
                PanelForce  = WindDirection * p4.trailingVortex();
                PanelForce *= 2.0 * Gamma;                                       //Newtons/q   (QInf = unit)
                if(!m_pPolar3d->isVLM1() && !p4.isLeading())
                {
                    Gammap1     = m_uRHS.at(p+1)*cosa + m_wRHS.at(p+1)*sina;
                    Force       = WindDirection * p4.trailingVortex();
                    Force      *= 2.0 * Gammap1;       //Newtons/q/QInf
                    PanelForce -= Force;
                }
//                else Gammap1=0.0;
//                m_Cp4[p] = PanelForce.dot(p4.m_Normal)/p4.m_Area;
            }
            PanelLeverArm.x = ForcePt.x - CoG.x;
            PanelLeverArm.y = ForcePt.y - CoG.y;
            PanelLeverArm.z = ForcePt.z - CoG.z;
            Cm += -PanelLeverArm.x * PanelForce.z + PanelLeverArm.z*PanelForce.x; //N.m/rho
        }
    }

    return Cm;
}

void P4Analysis::testResults(double alpha, double beta, double QInf) const
{
    // check that perturbation potential & velocity are zero inside the body
    // check that residual is zero for each basis function, by recalculating the scalar products

    double const *mu = m_Mu.data();
    double const *sigma = m_Sigma.data();

    Vector3d VInf = objects::windDirection(alpha, beta)*QInf;
    Vector3d Vel;

    Panel4 const &p4 = m_Panel4.at(0);

    double Z=0.0005;
    double n = 50.0;
    for(int id=0; id<int(n); id++)
    {
        double d=-Z + 2.0*double(id)*Z/n;
        Vector3d C = p4.CoG()+ p4.normal() * d;
        getVelocityVector(C, mu, sigma, Vel, 0.0, false, s_bMultiThread);
        Vel += VInf;
        qDebug(" %13g  %13g", d, Vel.dot(p4.normal()));
    }
}


/**
 * Builds a row of vortons located at the wake panel's streamwise edges.
 * The vorticity at the edges is the jump in doublet density between two adjacent panels x4.PI
 * The vorticity at the two tips is equal to the panel's doublet strength, i.e. zero vorticity
 * outside the wake panels.
 * In the case of a VLM2 analysis, the vorticity at the panel's edges is the sum of the left and
 * right vorticities.
 */
void P4Analysis::makeVortons(double dl, double const *Mu4,
                             int pos4, int nPanel4, int nStations, int nVtn0,
                             std::vector<Vorton> &vortons, std::vector<Vortex>&vortexneg) const
{
    vortons.resize(2*nStations); // one vorton on each of the wake panel's side; vortons at the same location are merged and one is discarded
    vortexneg.resize(nStations);

    double gamma=0, gLeft=0, gRight=0;
    int m=0;

    for(int i4=0; i4<nPanel4; i4++)
    {
        Panel4 const &p4 = m_Panel4.at(i4+pos4);
        if(p4.isTrailing())
        {
            assert(p4.iWake()>=0 && p4.iWake()<nWakePanels());

            Panel4 const *pP4w = &m_WakePanel4.at(p4.iWake());
            pP4w = trailingWakePanel(pP4w);

            if(!m_pPolar3d->isVLM())
            {
                if(p4.isMidPanel() || p4.isBotPanel())
                {
                    if(p4.isMidPanel())
                    {
                        int idxM = p4.index();
                        gamma = Mu4[idxM] *4.0*PI; // is the same as gLeft for this panel
                    }
                    else  if(p4.isBotPanel())
                    {
                        int idxB = p4.index();
                        int idxU = nextTopTrailingPanelIndex(p4);
                        gamma = (Mu4[idxU] - Mu4[idxB])*4.0*PI;
                    }

                    Segment3d leftedge = pP4w->leftEdge();
                    Segment3d rightedge = pP4w->rightEdge();
                    vortons[2*m  ].setPosition(leftedge.vertexAt(1) +leftedge.unitDir() *dl/2.0);
                    vortons[2*m+1].setPosition(rightedge.vertexAt(1)+rightedge.unitDir()*dl/2.0);
                    vortons[2*m  ].setVortex(leftedge.unitDir(),   gamma*dl);
                    vortons[2*m+1].setVortex(rightedge.unitDir(), -gamma*dl);

                    vortexneg[m].setNodes(pP4w->TA(), pP4w->TB());
                    // A panel with doublet density mu is equivalent to a vortex ring with circulation 4.0.PI.mu
                    vortexneg[m].setCirculation(-gamma); // negating circulation 4.PI Mu4
                    vortexneg[m].setNodeIndex(0, nVtn0+2*m);
                    vortexneg[m].setNodeIndex(1, nVtn0+2*m+1);

                    gLeft = gRight;
                    m++;
                }
            }
            else
            {
                assert(p4.isMidPanel());

                gRight = -Mu4[p4.index()];

                // extend the rear points;
                vortons[m].setPosition(pP4w->leftEdge().midPoint());
                vortons[m].setVortex(pP4w->leftEdge().unitDir(), (gRight-gLeft)*dl);

                gLeft = gRight;
                m++;
            }
        }
    }

    assert(m==nStations);
}


/**
 * UNUSED
 * Makes the array of negating vortices at the downstream end of the wake panels.
 * Used to cancel the effect of the transverse vortex between the last wake panel and the array of vortons.
 * The circulation of the vortices is stored in the SpanDistrib results
 * Cf. Willis 2005 fig.3
 */
void P4Analysis::makeNegatingVortices(std::vector<Vortex> &negvortices)
{
/*    s_DebugPts.clear();
    s_DebugVecs.clear();*/

    negvortices.clear();
    for(int iw=0; iw<nWakePanels(); iw++)
    {
        Panel4 const &p4w = m_WakePanel4.at(iw);
        if(p4w.m_iPU<0)
        {
            // start of the wake column
            Panel4 const *p4w_t = trailingWakePanel(&p4w);
            negvortices.push_back({p4w_t->TA(), p4w_t->TB()});
/*            s_DebugPts.append(p4w_t->TA());
            s_DebugVecs.append(negvortices.back().segment());*/
        }
    }
}

