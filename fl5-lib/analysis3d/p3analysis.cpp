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

#include <thread>


#include <p3analysis.h>

#include <mathelem.h>
#include <matrix.h>
#include <gaussquadrature.h>
#include <polar3d.h>
#include <objects2d.h>
#include <stabderivatives.h>
#include <vortex.h>
#include <vorton.h>



P3Analysis::P3Analysis() : PanelAnalysis()
{
    m_nStations = 0;
    m_pRefTriMesh = nullptr;
}


/** The Analysis has finished, the results have been stored, and the arrays are not needed anymore. */
void P3Analysis::releasePanelArrays()
{
    PanelAnalysis::releasePanelArrays();
    m_Mu.clear();
    m_Cp.clear();
    m_Sigma.clear();
    m_uRHSVertex.clear();
    m_vRHSVertex.clear();
    m_wRHSVertex.clear();
    m_pRHSVertex.clear();
    m_qRHSVertex.clear();
    m_rRHSVertex.clear();
    m_Panel3.clear();
    m_WakePanel3.clear();
}


void P3Analysis::setTriMesh(TriMesh const &trimesh)
{
    m_pRefTriMesh = &trimesh;
    m_refPanel3 = trimesh.panels();

    m_Panel3 = trimesh.panels();
    m_WakePanel3 = trimesh.wakePanels();
}


bool P3Analysis::initializeAnalysis(Polar3d const *pPolar3d, int nRHS)
{
    PanelAnalysis::initializeAnalysis(pPolar3d, nRHS);

    int N = nPanels();

    if(m_pPolar3d->isTriLinearMethod()) N*=3;

    if(nRHS>0)
    {
        if(!allocateMatrix(N)) return false;

        int memsize =     allocateRHS3(nRHS);
        QString strange = QString::asprintf("Allocating %2f Mb for %d RHS vectors", double(memsize)/1024.0/1024.0, nRHS);
        traceLog(strange+"\n\n");
    }

    m_bWarning  = false;

    return true;
}


int P3Analysis::allocateRHS3(int nRHS)
{
    if(!m_pPolar3d) return 0;

    int memsize = 0;

    int N = m_pPolar3d->isTriUniformMethod() ? nPanels() : nPanels()*3;

    // the new elements are initialized with a default-constructed value,
    // but for primitive types like int and double, as well as for pointer types,
    // the C++ language doesn't specify any initialization;
    // in those cases, Qt's containers automatically initialize the value to 0.

    // nRHS independent
    m_uRHSVertex.resize(nPanels()*3);
    m_vRHSVertex.resize(nPanels()*3);
    m_wRHSVertex.resize(nPanels()*3);

    memsize += 7 * uint(nPanels())*3 * sizeof(double);

    m_uRHS.resize(N);
    m_vRHS.resize(N);
    m_wRHS.resize(N);
    m_pRHS.resize(N);
    m_qRHS.resize(N);
    m_rRHS.resize(N);
    m_cRHS.resize(N);

    memsize += 14 * nPanels() * sizeof(double);

    m_uVLocal.resize(nPanels()*3); //the new elements are initialized with a default-constructed value
    m_vVLocal.resize(nPanels()*3); //the new elements are initialized with a default-constructed value
    m_wVLocal.resize(nPanels()*3);
    memsize += 2 * nPanels()*3 * sizeof(Vector3d);

    // nRHS dependent

    m_Cp.resize(nPanels()*3);
    m_Mu.resize(nPanels()*3);     // also independent of polarType
    memsize +=  2*nPanels()*3* int(sizeof(double));

    int sizeCst = nPanels() * nRHS;
    m_Sigma.resize(sizeCst);
    memsize +=  3*sizeCst* int(sizeof(double));

    return memsize;
}


/**
 * @brief Debug only, get the potential at any given field point
 */
void P3Analysis::getDebugPotential(Vector3d const &C, bool bSelf, double const *Mu, double const *Sigma,
                                   double &phi, bool bSource, bool bDoublet, bool bWake) const
{
    Vector3d *Vs = nullptr;
    Vector3d *Vd = nullptr;

    double phiD[]={0,0,0};
    double phiS=0, sign=1.0;
    phi = 0.0;
    for (int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const & p3 = m_Panel3[i3];
        if(bSource)
        {
            if(!p3.isMidPanel() && !p3.isWakePanel())
            {
                getSourceInfluence(m_pPolar3d, C, p3, bSelf, Vs, &phiS);
                phi += phiS* Sigma[i3];
            }
        }

        if(bDoublet)
        {
            getDoubletInfluence(C, p3, Vd, phiD, 0.0, true);

            if(m_pPolar3d->isTriUniformMethod()) phi += (phiD[0]+phiD[1]+phiD[2]) * Mu[i3]; // weight 1.0 for constant density
            else                                 phi += (phiD[0]*Mu[3*i3] + phiD[1]*Mu[3*i3+1] + phiD[2]*Mu[3*i3+2]);
        }

        if(bWake)
        {
            // Is the panel shedding a wake?
            if(p3.isTrailing())
            {
                //If so, we need to add the contribution of the wake column shedded by this panel
                if(p3.isBotPanel()) sign=-1.0; else sign=1.0;

                assert(m_Panel3.at(i3).iWake()>=0);

                Panel3 const * p3w = &m_WakePanel3[p3.iWake()];
                do
                {
                    // do not use RFF approximation for wake panels
                    getDoubletInfluence(C, *p3w, Vd, phiD, 0.0, false);

                    if(m_pPolar3d->isTriUniformMethod())
                    {
                        phi += (phiD[0]+phiD[1]+phiD[2]) * Mu[i3]*sign; // weight 1.0 for constant density
                    }
                    else if(m_pPolar3d->isTriLinearMethod())
                    {
                        // whether p3 is on the left or right wing, node 1 is its left trailing node and node 2 is its right trailing node
                        // if p3w is a left  wake panel, node 0 and 2 are left  side and node 1 is right side
                        if(p3w->isLeftSidePanel())  phi += ((phiD[0]+phiD[2])*Mu[3*i3+1] +  phiD[1]         *Mu[3*i3+2]) *sign;
                        // if p3w is a right wake panel, node 0 and 1 are right side and node 2 is left side
                        else                        phi += ( phiD[2]         *Mu[3*i3+1] + (phiD[0]+phiD[1])*Mu[3*i3+2]) *sign;
                    }
                    //is there another wake panel downstream?
                    if(p3w->m_iPD>=0) p3w = m_WakePanel3.data() + p3w->m_iPD;
                    else              p3w = nullptr;
                }
                while(p3w);
            }
        }
    }
}


void P3Analysis::getVelocityVector(Vector3d const &C,
                                   double const *Mu, double const *Sigma,
                                   Vector3d &VT, double coreradius, bool bWakeOnly, bool bMultiThread) const
{
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
            threads.push_back(std::thread(&P3Analysis::velocityVectorBlock, this, iBlock, C, &VBlock[iBlock]));
        }

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            threads[iBlock].join();
        }
//        std::cout << "P3Analysis::getVelocityVector joined all " << m_nBlocks << " threads" <<std::endl;
    }
    else
    {
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            velocityVectorBlock(iBlock, C, &VBlock[iBlock]);
        }
    }

    VT.reset(); // total induced velocity
    for(int ib=0; ib<m_nBlocks; ib++) VT += VBlock[ib];

    if(m_pPolar3d->bVortonWake())
    {
        double vtncorelength = m_pPolar3d->vortonCoreSize()*m_pPolar3d->referenceChordLength();

        Vector3d VVtn;
        getVortonVelocity(C, vtncorelength, VVtn, bMultiThread);
        VT += VVtn;
    }
}


void P3Analysis::velocityVectorBlock(int iBlock, Vector3d const &C, Vector3d *VT) const
{
    int blockSize = int(nPanels()/m_nBlocks) +1;
    int iStart = iBlock*blockSize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blockSize, maxRows);

    Vector3d Vd[3], Vs;
    double sign=0;

    for (int i3=iStart; i3<iMax; i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);

        if(!tmp_bWakeOnly)
        {
            if(tmp_Sigma && fabs(tmp_Sigma[i3])>0.0)
            {
                getSourceInfluence(m_pPolar3d, C, p3, C.isSame(p3.CoG()), &Vs, nullptr);
                *VT += Vs * tmp_Sigma[i3];
            }

            getDoubletInfluence(C, p3, Vd, nullptr, tmp_coreradius, true);
            VT->x += Vd[0].x*tmp_Mu[3*i3+0] + Vd[1].x*tmp_Mu[3*i3+1] + Vd[2].x*tmp_Mu[3*i3+2];
            VT->y += Vd[0].y*tmp_Mu[3*i3+0] + Vd[1].y*tmp_Mu[3*i3+1] + Vd[2].y*tmp_Mu[3*i3+2];
            VT->z += Vd[0].z*tmp_Mu[3*i3+0] + Vd[1].z*tmp_Mu[3*i3+1] + Vd[2].z*tmp_Mu[3*i3+2];

        }

        // Is the panel shedding a wake?
        if(p3.isTrailing())
        {
            //If so, we need to add the contribution of the wake column shedded by this panel
            if(p3.isBotPanel()) sign=-1.0; else sign=1.0;
            if(p3.iWake()<0) continue; // requesting the velocity before the wake has been set - usually the consequence of a repaint signal for streamlines

            //in the case of wake roll-up, only the panels upstream of row nWakeRow
//            int iRow=0;
            Panel3 const *p3w = &m_WakePanel3[p3.iWake()];

            // whether p3 is on the left or right wing, node 1 is its left trailing node and node 2 is its right trailing node
            //Mu3[3*i3+1] is the doublet density at the wing's panel left trailing node, and Mu3[3*i3+3] at the right trailing node
            double mu3left  = tmp_Mu[3*i3+1];
            double mu3right = tmp_Mu[3*i3+2];

            while(p3w)
            {
                // do not use RFF approximation for wake panels?
                getDoubletInfluence(C, *p3w, Vd, nullptr, tmp_coreradius, false);

                if(p3w->isLeftSidePanel())
                {
                    VT->x += ((Vd[0].x+Vd[2].x)*mu3left +  Vd[1].x       *mu3right) *sign;
                    VT->y += ((Vd[0].y+Vd[2].y)*mu3left +  Vd[1].y       *mu3right) *sign;
                    VT->z += ((Vd[0].z+Vd[2].z)*mu3left +  Vd[1].z       *mu3right) *sign;
                    // if p3w is a left wake panel, node 0 and 2 are left side and node 1 is right side - cf. TriMesh::makeWakePanels()
                }
                else
                {
                    // if p3w is a right wake panel, node 2 is left side and node 0 and 1 are right side
                    VT->x += ( Vd[2].x       *mu3left + (Vd[0].x+Vd[1].x)*mu3right) *sign;
                    VT->y += ( Vd[2].y       *mu3left + (Vd[0].y+Vd[1].y)*mu3right) *sign;
                    VT->z += ( Vd[2].z       *mu3left + (Vd[0].z+Vd[1].z)*mu3right) *sign;
                }
                // is there another wake panel downstream?
                if(p3w->m_iPD>=0) p3w = m_WakePanel3.data() + p3w->m_iPD;
                else              p3w = nullptr;

//                iRow++;
            }
        }
        if(isCancelled()) return;
    }
}


/**
 * Returns the perturbation velocity vector far downstream using a line vortex model for the wake
 * irrespective of the analysis method.
 */
void P3Analysis::getFarFieldVelocity(const Vector3d &C, const std::vector<Panel3> &panel3, const double *Mu,
                                     Vector3d &VT, double coreradius) const
{
    if(isCancelled()) return;

    VT.set(0.0,0.0,0.0);

    Vector3d VL, VR, A, B;
    double sign = 1.0;
    double fardist = m_pPolar3d->TrefftzDistance();
    for (int i3=0; i3<int(panel3.size()); i3++)
    {
        if(isCancelled()) return;
        Panel3 const &p3 = panel3.at(i3);
        if(p3.isTrailing())
        {
            sign = p3.isBotPanel() ? -1.0 : 1.0;
            A.set(p3.leftTrailingNode());
            A.x += fardist;
            VL = vortexInducedVelocity(p3.leftTrailingNode(), A, C, coreradius);
            VT +=  VL*(+Mu[3*i3+1] * sign);

            B.set(p3.rightTrailingNode());
            B.x += fardist;
            VR = vortexInducedVelocity(p3.rightTrailingNode(), B, C, coreradius);
            // circulations of each seg are oppposite
            VT +=  VR*(-Mu[3*i3+2] * sign);
        }
    }
}


/**
* Returns the perturbation potential at a given point, induced by the distribution of source and doublet/circulation strengths.
* @param C the point where the influence is to be evaluated
* @param Mu a pointer to the array of doublet strength or vortex circulations
* @param sigma a pointer to the array of source strengths
* @param VT the resulting perturbation velocity
* @param nWakeRow in the case of wake roll-up, only the panels upstream of row nWakeRow will be taken into account
*/
double P3Analysis::getPotential(Vector3d const &C, double const *mu, double const*sigma) const
{
    double phiT=0, phiSource=0;
    double phiBasis[]{0,0,0};
    double sign=0;
    for (int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);
        getSourceInfluence(m_pPolar3d, C, p3, false, nullptr, &phiSource);
        phiT += phiSource * sigma[i3];

        getDoubletInfluence(C, p3, nullptr, phiBasis, 0.0, true);
//        strange += QString::asprintf("   %9g", phiBasis[0]+phiBasis[1]+phiBasis[2]);
        phiT += phiBasis[0]*mu[3*i3] + phiBasis[1]*mu[3*i3+1] + phiBasis[2]*mu[3*i3+2];

        // Is the panel shedding a wake?
        if(p3.isTrailing())
        {
            //If so, we need to add the contribution of the wake column shedded by this panel
            if(p3.isBotPanel()) sign=-1.0; else sign=1.0;
//            assert(m_Panel3.at(i3).m_iWake>=0);

            //in the case of wake roll-up, only the panels upstream of row nWakeRow
//            int iRow=0;
            Panel3 const *p3w = &m_WakePanel3[p3.iWake()];
            while(p3w )
            {
                // do not use RFF approximation for wake panels
                getDoubletInfluence(C, *p3w, nullptr, phiBasis, 0.0, false);

                // whether p3 is on the left or right wing, node 1 is its left trailing node and node 2 is its right trailing node
                // if p3w is a left  wake panel, node 0 and 2 are left  side and node 1 is right side
                if(p3w->isLeftSidePanel())
                    phiT += ((phiBasis[0]+phiBasis[2])*mu[3*i3+1] + phiBasis[1] *mu[3*i3+2]) *sign;
                // if p3w is a right wake panel, node 0 and 1 are right side and node 2 is right side
                else
                    phiT += ( phiBasis[2]*mu[3*i3+1] + (phiBasis[0]+phiBasis[1])*mu[3*i3+2]) *sign;

                //is there another wake panel downstream?
                if(p3w->m_iPD>=0) p3w = m_WakePanel3.data() + p3w->m_iPD;
                else              p3w = nullptr;

 //               iRow++;
            }
        }
    }
//qDebug("%s", strange.toStdString().c_str());
    return phiT;
}


/**
* Returns the influence at point C of a uniform source distribution on the panel pPanel
* The panel is necessarily located on a thick surface, else the source strength is zero
* @param C the point where the influence is to be evaluated
* @param pPanel a pointer to the Panel with the doublet strength
* @param V the perturbation velocity at point C
* @param phi the potential at point C
*/
void P3Analysis::getSourceInfluence(Polar3d const *pPolar3d, Vector3d const &C, Panel3 const &p3, bool bSelf, Vector3d *V, double *phi) const
{
    if(phi) p3.sourcePotential(C, bSelf, *phi);
    if(V)
        p3.sourceN4023Velocity(C, bSelf, *V, Vortex::coreRadius());

    if(pPolar3d->bGroundEffect() || pPolar3d->bFreeSurfaceEffect())
    {
        double coef = pPolar3d->bGroundEffect() ? 1.0 : -1.0;

        // To enforce the condition Vz=0 on the ground, add the contribution
        // of a symmetric panel oppposiwte the ground surface, so that the z
        // contributions cancel each other
        // To avoid building a symmetric panel, calculate the influence of this panel
        // at a symmetric point below the surface, and add its z-opposite contribution

        Vector3d VG, CG;
        double phiG;
        CG.set(C.x, C.y, -C.z-2.0*m_pPolar3d->groundHeight());
        if(phi)
        {
            p3.sourcePotential(CG, false, phiG);
            *phi += phiG * coef;
        }
        if(V)
        {
            p3.sourceVelocity(CG, false, VG);
            V->x += VG.x * coef;
            V->y += VG.y * coef;
            V->z -= VG.z * coef;
        }
    }
}


/**
 * @brief For a uniform strength analysis, makes the doublet density at the nodes from the values at the panel's center of any two arrays
 */
void P3Analysis::makeNodeAverage(int iNode, std::vector<double> const &muPanel, std::vector<double> &muLin) const
{
    Node const &nd = m_pRefTriMesh->nodeAt(iNode);
    if(!nd.triangleCount()) return;

    //average the density over the panels that have been listed
    double muNode = 0.0;
    for(int i3=0; i3<nd.triangleCount(); i3++)
    {
        muNode += muPanel.at(nd.triangleIndex(i3));
    }
    assert(nd.triangleCount()>0);
    muNode /= nd.triangleCount();

    //store the result
    for(int i3=0; i3<nd.triangleCount(); i3++)
    {
        int idx3 = nd.triangleIndex(i3);
        Panel3 const &p3 = m_Panel3.at(idx3);
        for(int k=0; k<3; k++)
        {
            if(p3.nodeIndex(k)==iNode)
            {
                muLin[3*idx3+k]    = muNode;
            }
        }
    }
}


/**
 * This method calculates the Cp coefficient on a panel based on the distribution of doublet strengths.
 * Finds the plane best fitting the doublet values at the three vertices
 * and calculates it slope.
 * The local velocity is the slope of the plane.
 * @param p the index of the panel for which the calculation is performed
 * @param Mu the array of doublet strength values
 * @param VLocal a reference to the calculated local velocity on the panel
 */
void P3Analysis::makePanelDoubletSurfaceVelocity(int p, double const *Mu, Vector3d &VLocal)
{
    Vector3d const* sl = m_Panel3.at(p).m_Sl; // use local reference frame

    assert(p*3+2<3*nPanels());

    //find the gradient of steepest descent from the three values at the vertices
    double mu0 = Mu[p*3];
    double mu1 = Mu[p*3+1];
    double mu2 = Mu[p*3+2];

    // the three points which define the plane are A(s0.x, s0.y, µ0), B(s1.x, s1.y, µ1), C(s2.x, s2.y, µ2)
    // the plane equation is ax+by+cµ+d=0, or as a function: µ(x,y) = -(ax+by+d)/c
    double a = (sl[1].y-sl[0].y) * (mu2-mu0)         - (sl[2].y-sl[0].y)*(mu1-mu0);
    double b = (sl[2].x-sl[0].x) * (mu1-mu0)         - (sl[1].x-sl[0].x)*(mu2-mu0);
    double c = (sl[1].x-sl[0].x) * (sl[2].y-sl[0].y) - (sl[2].x-sl[0].x)*(sl[1].y-sl[0].y);
//  double d = -(a*sl[0].x + b*sl[0].y + c*mu0);  // Unused

    // the gradient is (dµ/dx, dµ/dy) =  (-a/c, -b/c)
    VLocal.x = -4.0*PI * (-a/c);
    VLocal.y = -4.0*PI * (-b/c);
    VLocal.z = 0.0;
//    VLocal.z = VinfL.z +4.0*PI * m_Sigma[p]; // is zero
}


double P3Analysis::stripArea(Panel3 const &p3, bool bThinSurfaces) const
{
    double striparea = 0.0;
    //sum panel areas of bottom strip
    int index = p3.index();
    do
    {
        Panel3 const &p3k = m_Panel3.at(index);
        striparea += p3k.area();
        if(p3k.m_iPU>=0) index = p3k.m_iPU;
        else             break;
    }
    while (index>=0); //  = while(true)
    if(!bThinSurfaces) striparea /= 2;

    return striparea;
}


int P3Analysis::nextTopTrailingPanelIndex(Panel3 const &p3) const
{
    if(!p3.isBotPanel()) return -1;

/*    int index = p3.index();
    do
    {
        Panel3 const &p3k = m_Panel3.at(index);
        if(p3k.m_iPU>=0) index = p3k.m_iPU;
        else             return index;
    }
    while (index>=0); //  = while(true)

    return -1;*/

    return p3.oppositeIndex();
}


void P3Analysis::trailingWakePanels(const Panel3 *pWakePanel, Panel3 &p3WU, Panel3 &p3WD) const
{
    do
    {
        if(pWakePanel->m_iPD<0)
        {
            break;
        }
        else pWakePanel = m_WakePanel3.data() + pWakePanel->m_iPD;
    }
    while (pWakePanel);

    if(pWakePanel)
    {
        p3WD = m_WakePanel3[pWakePanel->index()];
        p3WU = m_WakePanel3[pWakePanel->m_iPU];
    }
}

/**
 * Returns the center point of the of the wake column to which the panel belongs
 * Assumes that pWakePanel points to the leading panel of the wake column
 */
void P3Analysis::midWakePoint(Panel3 const*pWakePanel, Vector3d &midleft, Vector3d &midright) const
{
    Vector3d LA, LB, TA, TB;
    if(!pWakePanel) return;
    LA = pWakePanel->vertexAt(2);
    LB = pWakePanel->vertexAt(1);

    do
    {
        if(pWakePanel->m_iPD<0)
        {
            TA.set(pWakePanel->leftTrailingNode());
            TB.set(pWakePanel->rightTrailingNode());
            break;
        }
        else pWakePanel = m_WakePanel3.data() + pWakePanel->m_iPD;
    }
    while (pWakePanel);

    midleft  = (LA+TA)*0.5;
    midright = (LB+TB)*0.5;
}


void P3Analysis::trailingWakePoint(Panel3 const*pWakePanel, Vector3d &left, Vector3d &right) const
{
    do
    {
        if(pWakePanel->m_iPD<0)
        {
            left = pWakePanel->leftTrailingNode();
            right = pWakePanel->rightTrailingNode();
            break;
        }
        else pWakePanel = m_WakePanel3.data() + pWakePanel->m_iPD;
    }
    while (pWakePanel);
}


int P3Analysis::makeWakePanels(const Vector3d &WindDirection, bool bVortonWake)
{
    if(!bVortonWake)
    {
        Vector3d pt;
        m_pRefTriMesh->getLastTrailingPoint(pt);
        TriMesh::makeWakePanels(m_Panel3,
                                m_pPolar3d->NXWakePanel4(), m_pPolar3d->wakePanelFactor(), pt.x + m_pPolar3d->wakeLength(),
                                WindDirection, m_WakePanel3, m_nStations, true);
    }
    else
    {
        TriMesh::makeWakePanels(m_Panel3,
                                m_pPolar3d->NXBufferWakePanels(), 1.0, m_pPolar3d->bufferWakeLength(),
                                WindDirection, m_WakePanel3, m_nStations, false);
    }

    return nWakePanels();
}


int P3Analysis::matSize() const
{
    if(!m_pPolar3d) return nPanels();
    if(m_pPolar3d->isTriUniformMethod()) return nPanels();
    if(m_pPolar3d->isTriLinearMethod())  return nPanels()*3;
    return nPanels();
}


void P3Analysis::savePanels()
{
    m_refPanel3     = m_Panel3;
    m_refWakePanel3 = m_WakePanel3;
}


void P3Analysis::restorePanels()
{
    m_Panel3     = m_refPanel3;
    m_WakePanel3 = m_refWakePanel3;
}


void P3Analysis::scaleResultsToSpeed(double ratio)
{
    //______________________________________________________________________________________
    // Scale RHS and Sigma i.a.w. speeds (so far we have unit doublet and source strengths)
    for(int pp=0; pp<3*nPanels(); pp++)
    {
        m_Mu[pp] *= ratio;
    }
    for(int pp=0; pp<nPanels(); pp++)
    {
        m_Sigma[pp] *= ratio;
    }
}


/**
 * Evaluates the cross-flow forces by application of the Kutta-Joukowski theorem
 * Drela § 5.7.
 * The induced drag is evaluated separately in the Trefftz plane or in the vorton wake
 * */
void P3Analysis::inducedForce(int nPanel3, double QInf, double alpha, double beta,
                              int pos3, Vector3d &ForceBodyAxes, SpanDistribs &SpanResFF) const
{
    Vector3d VInf;
    double gLeft=0, gRight=0, gMid=0;
    //dynamic pressure, kg/m³

    //   Define wind axis
    Vector3d winddir = objects::windDirection(alpha, beta);
    VInf = winddir * QInf;

    double q = 0.5 * m_pPolar3d->density() * QInf * QInf;
    Vector3d StripForce; // body axes

    double const *mu3    = m_Mu.data();
    int m=0;
    for(int i3=0; i3<nPanel3; i3++)
    {
        int index = i3+pos3;
        Panel3 const &p3 = m_Panel3.at(index);
        Vector3d const &surfacenormal = p3.surfaceNormal();

        if(p3.isTrailing() && (p3.isBotPanel() || p3.isMidPanel()))
        {
            if(p3.isMidPanel())
            {
                int idxM = p3.index();
                gLeft  = -mu3[3*idxM+1] *4.0*PI;
                gRight = -mu3[3*idxM+2] *4.0*PI;
            }
            else if(p3.isBotPanel())
            {
                int idxB = p3.index();
                int idxU = nextTopTrailingPanelIndex(p3);
                assert(idxU>=0);

                // the trailing nodes have indexes 1 & 2
                gLeft  = (-mu3[3*idxU+1] + mu3[3*idxB+1])*4.0*PI;
                gRight = (-mu3[3*idxU+2] + mu3[3*idxB+2])*4.0*PI;
            }

            gMid = (gLeft+gRight)/2.0;

            SpanResFF.m_Gamma[m] = gMid;

            StripForce = VInf * p3.trailingVortex();
            StripForce *= gMid * m_pPolar3d->density();     // N
            StripForce *=  1.0/q;      // N/q

            SpanResFF.m_Cl[m]  = StripForce.dot(surfacenormal) /SpanResFF.stripArea(m);
            SpanResFF.m_F[m]   = StripForce * q;                        // N, body axes

            m++;
        }
    }

    for(uint m=0; m<SpanResFF.m_F.size(); m++)
        ForceBodyAxes += SpanResFF.m_F.at(m);
    ForceBodyAxes *= 1.0/q;    // N/q, body axes
}


void P3Analysis::trefftzDrag(int nPanel3, double QInf, double alpha, double beta, int pos3,
                             Vector3d &Drag, SpanDistribs &SpanResFF) const
{
    Vector3d left, right;
    Vector3d Wg_l, Wg_m, Wg_r;
    Vector3d u, mid;
    Vector3d ForceBodyAxes; // Strip and global forces, body axes
    double gLeft(0), gRight(0), gMid(0);

    //   Define wind axis
    Vector3d winddir = objects::windDirection(alpha, beta);

    //dynamic pressure, kg/m³
    double qDyn = 0.5 * m_pPolar3d->density() * QInf * QInf;

    Vector3d StripForce; // body axes
    Vector3d theforce_l, theforce_m, theforce_r;

    //dynamic pressure, kg/m³

    double const *mu3    = m_Mu.data();
    double const *sigma3 = m_Sigma.data();

//    clearDebugPts();

    // Note: parallelization fails, incompatibility with std::vectors of SpanDistribs
    int m=0;
    for(int i3=0; i3<nPanel3; i3++)
    {
        int index = i3+pos3;
        Panel3 const &p3 = m_Panel3.at(index);
        Vector3d const &surfacenormal = p3.surfaceNormal();
        StripForce.set(0,0,0);
        if(p3.isTrailing() && (p3.isBotPanel() || p3.isMidPanel()))
        {
            // get the last triangle of the wake column
            assert(p3.iWake()>=0 && p3.iWake()<nWakePanels());
            Panel3 const *p3W = m_WakePanel3.data() + p3.iWake();
//            trailingWakePoint(p3W, left, right);

            // evaluate the induced drag at the half of the wake's length to avoid end effects
            // exact position is not significant, result is not affected by panel side singularities

            midWakePoint(p3W, left, right);
            mid.set((left + right)/2.0);

//            getVelocityVector(left,  mu3, sigma3, Wg_l, 0.0001, true, s_bMultiThread);
            getVelocityVector(mid,   mu3, sigma3, Wg_m, 0.0001, true, s_bMultiThread);
//            getVelocityVector(right, mu3, sigma3, Wg_r, 0.0001, true, s_bMultiThread);

//            Wg_l *= 0.5;
            Wg_m *= 0.5;
//            Wg_r *= 0.5;

//s_DebugPts.push_back(mid);
//s_DebugVecs.push_back(Wg_m);


            if(p3.isMidPanel())
            {
                int idxM = p3.index();
                gLeft  = -mu3[3*idxM+1] *4.0*PI;
                gRight = -mu3[3*idxM+2] *4.0*PI;
                gMid   = (gLeft+gRight)/2.0;
            }
            else if(p3.isBotPanel())
            {
                int idxB = p3.index();
                int idxU = nextTopTrailingPanelIndex(p3);
                assert(idxU>=0);

                // the trailing nodes have indexes 1 & 2
                gLeft  = (-mu3[3*idxU+1] + mu3[3*idxB+1])*4.0*PI;
                gRight = (-mu3[3*idxU+2] + mu3[3*idxB+2])*4.0*PI;
                gMid = (gLeft+gRight)/2.0;
            }

            u.set(p3.trailingVortex());
            u.normalize();

//            theforce_l = Wg_l * u * gLeft;
            theforce_m = Wg_m * u * gMid;
//            theforce_r = Wg_r * u * gRight;

/*            StripForce += (theforce_l + theforce_m)/2.0;
            StripForce += (theforce_m + theforce_r)/2.0;*/

             StripForce += theforce_m*2.0;

            StripForce *= p3.trailingVortex().norm()/2.0; // two half segments
            StripForce *= m_pPolar3d->density() / qDyn;      // N/q
            ForceBodyAxes += StripForce;      // N/q


            SpanResFF.m_Vd[m]  = Wg_m;
            SpanResFF.m_Ai[m]  = atan2(Wg_m.dot(surfacenormal), QInf)*180.0/PI;
            SpanResFF.m_ICd[m] = StripForce.dot(winddir) /SpanResFF.stripArea(m);

#ifdef QT_DEBUG
//qDebug("Wg[%d]  %11g  %11g  %11g", m, SpanResFF.m_Vd[m].x, SpanResFF.m_Vd[m].y, SpanResFF.m_Vd[m].z);
//qDebug("Wg[%d]  %11g  %11g", m, p3.trailingVortex().norm(), Wg_m.norm());

#endif
            m++;
        }
    }

    Drag.set(ForceBodyAxes);    // N/q, body axes
//    Drag.listCoords("Drag: ");
}



/**
* Calculates the forces using a far-field method.
* Calculates the moments by a near field method, i.e. direct summation on the panels.
* The downwash is evaluated far downstream (i.e. where the inluence of the bounded vortices is negligible)
*/
void P3Analysis::forces(double const *Mu3, double const *Sigma3, double alpha, double beta, Vector3d const &CoG, bool bFuseMi,
                        std::vector<Vector3d> const &VInf, Vector3d &Force, Vector3d &Moment)
{
    if(!m_pPolar3d) return;

    double Cp=0.0, viscousDrag=0.0;
    double QInf=1.0;
    Vector3d left, mid, right;
    Vector3d WindDirection, WindNormal, PanelLeverArm, Wg;
    Vector3d Velocity, stripforce, viscousMoment, PanelForce;

    //   Define the wind axis
    WindNormal = objects::windNormal(alpha, beta);
    WindDirection = objects::windDirection(alpha, beta);

    Force.set( 0.0, 0.0, 0.0);
    Moment.set(0.0, 0.0, 0.0);
    viscousDrag = 0.0;
    viscousMoment.set(0.0,0.0,0.0);

//    int m=0;

    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);

        if(p3.isMidPanel())
        {
            if(p3.isTrailing())
            {
                // get the last triangle of the wake column
                assert(p3.iWake()>=0 && p3.iWake()<nWakePanels());
                Panel3 *p3W = m_WakePanel3.data() + p3.iWake();
                trailingWakePoint(p3W, left, right);
                mid = (left + right)/2.0;

                //    s_PointDbg.push_back(C);
                getVelocityVector(mid, Mu3, Sigma3, Wg, 0.00001, true, true);

                int idxM = p3.index();
                double GammaStrip = -(Mu3[3*idxM+1] + Mu3[3*idxM+2])/2.0 *4.0*PI;
                //qDebug("m=%3d  %13.5f  %13.5f  %13.5f  %13.5f", m, GammaStrip, Wg.x, Wg.y, Wg.z);

                Wg += VInf.at(i3);

                Vector3d vortex = p3.trailingVortex();
                stripforce  =  Wg * vortex;
                stripforce *= GammaStrip * m_pPolar3d->density();     // N
                Force += stripforce;

 //               m++;
            }
        }
        else if(p3.isTrailing() && p3.isBotPanel())
        {
            int idxB = p3.index();
            int idxU = nextTopTrailingPanelIndex(p3);
            assert(idxU>=0);

            //Get the strip's lifting force

            // get the last triangle of the wake column
            Panel3 *p3W = m_WakePanel3.data() + p3.iWake();
            trailingWakePoint(p3W, left, right);
            mid = (left + right)/2.0;

            /** @todo used to be bWakeOnly=false up to beta07 */
            getVelocityVector(mid, Mu3, Sigma3, Wg, Vortex::coreRadius(), true, true);

            Wg += VInf.at(i3);

            // the trailing nodes have indexes 1 & 2
            double gU = (Mu3[3*idxU+1] + Mu3[3*idxU+2])/2.0;
            double gB = (Mu3[3*idxB+1] + Mu3[3*idxB+2])/2.0;
            double GammaStrip = (-gU + gB) *4.0*PI;

            Vector3d vortex = p3.trailingVortex()*(-1.0);
            stripforce  = vortex * Wg;
            stripforce *= GammaStrip * m_pPolar3d->density();     // N

            Force += stripforce;
//            m++;
        }
    }

    //On-Body moment

    Moment.set(0.0,0.0,0.0);

    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);
        if(!p3.isFusePanel() || bFuseMi)
        {
            Velocity = VInf.at(i3);
            QInf = Velocity.norm();
            Cp = (m_Cp[3*i3]+m_Cp[3*i3+1]+m_Cp[3*i3+2])/3.0;
            PanelForce = p3.normal() * (-Cp) * p3.area() *1/2.*QInf*QInf;      // Newtons/rho
            PanelLeverArm = p3.CoG() - CoG;
            Moment += PanelLeverArm * PanelForce;                     // N.m/rho
        }
    }

    if(m_pPolar3d->isViscous())
    {
        Force += WindDirection * viscousDrag;
        Moment += viscousMoment;
    }

    // add fuse drag and extra drag
/*    double frictiondrag = m_pPolar3d->extraDragTotal();
    if(m_pPlane->fuse(0) && pWPolar->isFuseDragIncluded())
    {
        Re = m_pPlane->fuse(0)->length() * m_QInfMin / m_pPolar3d->viscosity();
        double fusedrag = pWPolar->fuseDragCoef(Re) * m_pPlane->fuse(0)->wettedArea() * m_pPlane->fuse(0)->formFactor();
        frictiondrag += fusedrag;
    }
    frictiondrag *= 1./2.*m_pPolar3d->density()*Vinc.norm()*Vinc.norm();   // N

    Force += WindDirection*frictiondrag;
*/
    Moment *= m_pPolar3d->density();                          // N.m
}


/**
 * Calculates the forces using a far-field method.
 * Calculates the moments by a near field method, i.e. direct summation on the panels.
 * Downwash is evaluated at a distance 100 times the span downstream (i.e. infinite)
 * @param Mu a pointer to the array of doublet strengths or vortex circulations
 * @param Sigma a pointer to the array of source strengths
 * @param *VInf a pointer to the array of the velocity vectors on the panels
 * @param Force the resulting force vector
 * @param Moment the resulting moment vector
 */
void P3Analysis::moments(double *Mu3, double alpha, double beta, Vector3d CoG,
                         double *VInf, Vector3d &Force, Vector3d &Moment)
{
    if(!m_pPolar3d) return;

    double Cp=0.0, viscousDrag=0.0;
    double QInf=0.0;
    Vector3d WindDirection, WindNormal, PanelLeverArm, Wg;
    Vector3d Velocity, viscousMoment, PanelForce;

    //   Define the wind axis
    WindNormal = objects::windNormal(alpha, beta);
    WindDirection = objects::windDirection(alpha, beta);

    Force.set( 0.0, 0.0, 0.0);
    Moment.set(0.0, 0.0, 0.0);
    viscousDrag = 0.0;
    viscousMoment.set(0.0,0.0,0.0);


    //On-Body moment
    Vector3d Vp3;
    Moment.set(0.0,0.0,0.0);

    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 *p3 = m_Panel3.data()+i3;
        Velocity.x = *(VInf               +i3);
        Velocity.y = *(VInf +   nPanels() +i3);
        Velocity.z = *(VInf + 2*nPanels() +i3);
        QInf = Velocity.norm();
        makePanelDoubletSurfaceVelocity(i3, Mu3, Vp3);

        Vp3 += p3->globalToLocal(Velocity);
        Cp = 1.0 - (Vp3.x*Vp3.x + Vp3.y*Vp3.y + Vp3.z*Vp3.z)/QInf/QInf;

        PanelForce = p3->normal() * (-Cp) * p3->area() *1/2.*QInf*QInf;      // Newtons/rho
        PanelLeverArm = p3->CoG() - CoG;

        Force += PanelForce;
        Moment += PanelLeverArm * PanelForce;                     // N.m/rho
    }

    if(m_pPolar3d->isViscous())
    {
        Force += WindDirection * viscousDrag;
        Moment += viscousMoment;
    }

    Force  *= m_pPolar3d->density();                          // N
    Moment *= m_pPolar3d->density();                          // N.m
}


/**
 * Returns the geometric pitching moment coefficient for the specified angle of attack
 * The effect of the viscous drag is not included.
 * @param Alpha the aoa for which Cm is calculated
 */
double P3Analysis::computeCm(Vector3d const &CoG, double Alpha, bool bFuseMi)
{
    Vector3d PanelLeverArm, ForcePt, PanelForce, WindDirection;
    double Cp=0;

    // Define the wind axis
    double beta=0;
    double cosa = cos(Alpha*PI/180.0);
    double sina = sin(Alpha*PI/180.0);
    WindDirection.set(cosa, 0.0, sina);

    std::vector<Vector3d> VLocal(nPanels()*3);
    std::vector<Vector3d> VInf(nPanels(), WindDirection);
    combineLocalVelocities(Alpha, beta, VLocal);
    computeOnBodyCp(VInf, VLocal, m_Cp);

    bool bTrace = false;
    double Cm = 0.0;
    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);

        if(!p3.isFusePanel() || bFuseMi)
        {

    /*        //first calculate Cp for this angle
            p3.globalToLocal(VInf, VLocal);
            VLocal += m_uVlNode.at(3*i3)*cosa + m_wVlNode.at(3*i3)*sina;
            Speed2 = VLocal.x*VLocal.x + VLocal.y*VLocal.y;// + VLocal.z*VLocal.z;
            Cp  = 1.0-Speed2; // QInf=unit, /1.0/1.0;
    */
            Cp = (m_Cp.at(3*i3)+m_Cp.at(3*i3+1)+m_Cp.at(3*i3+2))/3.0;

            //next calculate the force acting on the panel
            ForcePt = p3.CoG();
            PanelForce = p3.normal() * (-Cp) * p3.area();      // Newtons/q
if(bTrace) qDebug("  %3d  %13g  %13g  %13g  %13g  %13g  %13g",
                  i3, ForcePt.x, ForcePt.y, ForcePt.z, PanelForce.x, PanelForce.y, PanelForce.z);

            PanelLeverArm.x = ForcePt.x - CoG.x;
            PanelLeverArm.y = ForcePt.y - CoG.y;
            PanelLeverArm.z = ForcePt.z - CoG.z;
            Cm += -PanelLeverArm.x * PanelForce.z + PanelLeverArm.z*PanelForce.x; //N.m/rho
        }
    }

    Cm *= m_pPolar3d->density();
    return Cm;
}


/**
* Performs the summation of on-body forces to calculate the total lift and drag forces
* @param Cp a pointer to the array of previously calculated Cp coefficients
* @param Alpha the aoa for which this calculation is performed
* @param Lift the resulting lift force
* @param Drag the resulting drag force
*/
void P3Analysis::sumPanelForces(double *Cp, Vector3d &F)
{
    for(int i3=0; i3<nPanels(); i3++)
    {
        F += m_Panel3.at(i3).normal() * (-Cp[3*i3]) * m_Panel3.at(i3).area();
    }
}


void P3Analysis::makeMu(int )
{
/*    for(int i3=0; i3<nPanels(); i3++)
    {
        m_Mu[qrhs*nPanels() +i3] = m_uRHS[i3];
    }*/
}

#define CM_ITER_MAX 50
/**
* Finds the zero-pitching-moment aoa such that Cm=0.
* Proceeds by iteration between -PI/4 and PI/4
* @return true if an equlibrium angle was found false otherwise.
*/
bool P3Analysis::getZeroMomentAngle(Vector3d const &CoG, double &alphaeq, bool bFuseMi)
{
    double tmp=0;
    double eps = 1.e-7;

    int iter = 0;
    double a0 = -PI/4.0;
    double a1 =  PI/4.0;

    double a = 0.0;
    double Cm0 = computeCm(CoG, a0*180.0/PI, bFuseMi);
    double Cm1 = computeCm(CoG, a1*180.0/PI, bFuseMi);
    double Cm = 1.0;

    //are there two initial values of opposite signs?
    int itermax = 50;
    while(Cm0*Cm1>0.0 && iter <=itermax)
    {
        a0 *=0.9;
        a1 *=0.9;
        Cm0 = computeCm(CoG, a0*180.0/PI, bFuseMi);
        Cm1 = computeCm(CoG, a1*180.0/PI, bFuseMi);
//        qDebug(" iter=%3d,  %11g   %11g,  %11g   %11g", iter, a0*180.0/PI, a1*180.0/PI, Cm0, Cm1);
        iter++;
        if(isCancelled()) break;
    }
    if(iter>=itermax || isCancelled()) return false;

    iter = 0;

    //Cm0 and Cm1 are of opposite sign
    if(Cm0>Cm1)
    {
        tmp = Cm1;
        Cm1 = Cm0;
        Cm0 = tmp;
        tmp = a0;
        a0  = a1;
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

    computeCm(CoG, a*180.0/PI, bFuseMi);
    alphaeq = a*180.0/PI;

    return true;
}


bool P3Analysis::computeTrimmedConditions(double mass, Vector3d const &CoG, double &alphaeq, double &u0, bool bFuseMi)
{
    Vector3d VInf = objects::windDirection(alphaeq, 0.0);
    Vector3d WindNormal = objects::windNormal(alphaeq, 0.0);

    if(isCancelled()) return false;

    if(!getZeroMomentAngle(CoG, alphaeq, bFuseMi))
    {
        traceStdLog("      no zero-moment angle found...     \n");
        return false;
    }


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

    // so far we have a unit Vortex Strength
    // find the speeds which will create a lift equal to the weight

    u0 = 1.0;

    //extra drag is unused when calculating lifting velocity
//displayArray(m_Mu);

    Vector3d Force, Moment;
    std::vector<Vector3d> VField(nPanels(), VInf);
    forces(m_Mu.data(), m_Sigma.data(), alphaeq, 0.0, CoG, bFuseMi, VField, Force, Moment);

    double Lift   = Force.dot(WindNormal);        //N/rho ; bank effect not included
//    VerticalCl = Lift*2.0/m_pWPolar->referenceArea() * cos(phi)/m_pWPolar->density();
    if(Lift<=PRECISION)
    {
        u0 = -100.0;
        QString strong;
        strong = "        Found a negative lift for " + ALPHAch;
        strong = QString::asprintf("=%.5f", alphaeq) + DEGch;
        strong = ".... skipping the angle...\n";
        m_bWarning = true;
        traceLog("\n"+strong);
        return false;
    }
    else
    {
        u0 = sqrt(9.81 * mass / Force.z);

        if(fabs(m_pPolar3d->phi())>PRECISION)
        {
        }
    }
    return true;
}


void P3Analysis::makeInfluenceMatrix()
{

    m_bMatrixError = false;

    if(s_bMultiThread)
    {
        std::vector<std::thread> threads;

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            threads.push_back(std::thread(&P3Analysis::makeMatrixBlock, this, iBlock));
        }
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            threads[iBlock].join();
        }
//        std::cout << "P3Analysis::makeInfluenceMatrix joined all " << m_nBlocks << " threads" <<std::endl;

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


/**
 * UNUSED
 * Makes the array of negating vortices at the downstream end of the wake panels.
 * Used to cancel the effect of the transverse vortex between the last wake panel and the array of vortons.
 * The circulation of the vortices is stored in the SpanDistrib results
 * Cf. Willis 2005 fig.3
 */
void P3Analysis::makeNegatingVortices(std::vector<Vortex> &negvortices)
{
/*    s_DebugPts.clear();
    s_DebugVecs.clear();*/
    Vector3d left, right;
    negvortices.clear();
    for(int iw=0; iw<nWakePanels(); iw++)
    {
        Panel3 const &p3w = m_WakePanel3.at(iw);
        if(p3w.m_iPU<0)
        {
            // start of the wake column
            trailingWakePoint(&p3w, left, right);
            negvortices.push_back({left, right});
/*            s_DebugPts.push_back(left);
            s_DebugVecs.push_back(negvortices.back().segment());*/
        }
    }
}


void P3Analysis::getDoubletInfluence(Vector3d const &C, Panel3 const &p3, Vector3d *V, double *phi,
                                     double , bool bUseRFF) const
{
    if(!phi && !V) return;

    if(phi) p3.doubletBasisPotential(C, p3.isPointInTriangle(C), phi, bUseRFF);
    if(V)
    {
        if(m_pPolar3d->isTriLinearMethod())
            p3.doubletBasisVelocity(C, V, bUseRFF);
        else
        {
            // faster
            p3.doubletN4023Velocity(C, false, *V, Vortex::coreRadius(), bUseRFF);
            V[0] = V[1] = V[2] = V[0]/3.0;
        }
    }

    if(m_pPolar3d->bHPlane())
    {
        double coef = m_pPolar3d->bGroundEffect() ? 1.0 : -1.0;

        double phiG[]{0,0,0};
        Vector3d VG[3];
        Vector3d CG(C.x, C.y, -C.z-2.0*m_pPolar3d->groundHeight());

        if(phi) p3.doubletBasisPotential(CG, false, phiG, bUseRFF);
        if(V)
        {
            if(m_pPolar3d->isTriLinearMethod())
                p3.doubletBasisVelocity(CG, VG, bUseRFF);
            else
            {
                // faster
                p3.doubletN4023Velocity(CG, false, *VG, Vortex::coreRadius(), bUseRFF);
                VG[0] = VG[1] = VG[2] = VG[0]/3.0;
            }
        }


        for(int ig=0; ig<3; ig++)
        {
            if(phi) phi[ig] += phiG[ig] * coef;
            if(V)
            {
                V[ig].x += VG[ig].x * coef;
                V[ig].y += VG[ig].y * coef;
                V[ig].z -= VG[ig].z * coef;
            }
        }
    }
}


void P3Analysis::combineLocalVelocities(double alpha, double beta, std::vector<Vector3d> &VLocal) const
{
    VLocal.resize(nPanels()*3);
    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);
    double cosb = cos(-beta*PI/180.0); //change of beta sign introduced in v7.24 to be consistent with AVL
    double sinb = sin(-beta*PI/180.0); //change of beta sign introduced in v7.24 to be consistent with AVL
    for(int i=0; i<int(VLocal.size()); i++)
    {
        VLocal[i].x = cosa*cosb* m_uVLocal.at(i).x + sinb*m_vVLocal.at(i).x + sina*cosb* m_wVLocal.at(i).x;
        VLocal[i].y = cosa*cosb* m_uVLocal.at(i).y + sinb*m_vVLocal.at(i).y + sina*cosb* m_wVLocal.at(i).y;
        VLocal[i].z = cosa*cosb* m_uVLocal.at(i).z + sinb*m_vVLocal.at(i).z + sina*cosb* m_wVLocal.at(i).z;
    }

//    for(int i=0; i<VLocal.size(); i++)        qDebug("  %11g  %11g  %11g", VLocal.at(i).x, VLocal.at(i).y, VLocal.at(i).z);
}




