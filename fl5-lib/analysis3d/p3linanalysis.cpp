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

#include <thread>
#include <iostream>
#include <QString>



#include <geom_global.h>
#include <gqtriangle.h>
#include <p3linanalysis.h>
#include <panel3.h>
#include <polar3d.h>


#if defined ACCELERATE
  #include <Accelerate/Accelerate.h>
  #define lapack_int int
#elif defined INTEL_MKL
    #include <mkl.h>
#elif defined OPENBLAS
    #include <openblas/lapacke.h>
#endif

P3LinAnalysis::P3LinAnalysis() : P3Analysis()
{
    Panel3::makeGQCoeffs();
}


void P3LinAnalysis::makeMatrixBlock(int iBlock)
{
    int N = nPanels()*3;
    double sp[]={0,0,0,0,0,0,0,0,0};
    // for each panel
    int blockSize = int(nPanels()/m_nBlocks)+1; // add one to compensate for rounding errors
    int iStart = iBlock*blockSize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blockSize, maxRows);

    for(int i3=iStart; i3<iMax; i3++)
    {
        Panel3 const &p3i = m_Panel3.at(i3);

        for(int k3=0; k3<nPanels(); k3++)
        {
            Panel3 const &p3k = m_Panel3.at(k3);

            if(m_pPolar3d->bNeumann() || p3i.isMidPanel())
                p3i.scalarProductDoubletVelocity(p3k, sp);
            else
                p3i.scalarProductDoubletPotential(p3k, i3==k3, sp);

            if(std::isnan(sp[0]) || std::isnan(sp[1]) || std::isnan(sp[2]))
            {
                QString strange;
                strange = QString::asprintf("      *** numerical error when calculating the influence of panel %d on panel %d ***\n", k3, i3);
                traceLog(strange);
                m_bMatrixError = true;
                return;
            }

            for(int iBasis=0; iBasis<3; iBasis++)
            {
                int row = 3*i3 + iBasis;
                for(int kBasis=0; kBasis<3; kBasis++)
                {
                    int col = 3*k3+kBasis;
                    if(s_bDoublePrecision)
                        m_aijd[uint(row*N+col)] = sp[3*iBasis+kBasis];// * p3i->orientationSign();
                    else
                        m_aijf[uint(row*N+col)] = float(sp[3*iBasis+kBasis]);// * p3i->orientationSign();
                }
            }

            if(m_pPolar3d->bGroundEffect() || m_pPolar3d->bFreeSurfaceEffect())
            {
                double coef = m_pPolar3d->bGroundEffect() ? 1.0 : -1.0;
                // add the contribution of the symmetric panel below the water's surface
                // This is done slightly differently than for the uniform methods:
                // make the symmetric panel and reverse its orientation
                Vector3d S[3];
                for(int in=0; in<3; in++)  S[in].set(p3k.node(in).x, p3k.node(in).y, -p3k.node(in).z-2.0*m_pPolar3d->groundHeight());
                Panel3 p3kG(S[0], S[2], S[1]);

                if(m_pPolar3d->bNeumann() || p3i.isMidPanel()) p3i.scalarProductDoubletVelocity(p3kG, sp);
                else                                           p3i.scalarProductDoubletPotential(p3kG, false, sp);

                for(int iBasis=0; iBasis<3; iBasis++)
                {
                    int row = 3*i3 + iBasis;
                    for(int kBasis=0; kBasis<3; kBasis++)
                    {
                        int col = 3*k3+kBasis;
                        if(s_bDoublePrecision) m_aijd[uint(row*N+col)] += sp[3*iBasis+kBasis]*coef;
                        else                   m_aijf[uint(row*N+col)] += float(sp[3*iBasis+kBasis]*coef);
                    }
                }
            }
            if(isCancelled() || m_bMatrixError) break;
        }
        if(isCancelled() || m_bMatrixError) break;
    }
}


void P3LinAnalysis::makeWakeMatrixBlock(int iBlock)
{
    int N = nPanels()*3;

    // for each panel
    int blockSize = int(nPanels()/m_nBlocks) +1;
    int iStart = iBlock*blockSize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blockSize, maxRows);

    int col1=0, col2=0;
    double scalarLeft[]{0,0,0};
    double scalarRight[]{0,0,0};
    double LeftContrib[]{0,0,0}, RightContrib[]{0,0,0};

    for(int i3=iStart; i3<iMax; i3++)
    {
        Panel3 const &p3i = m_Panel3.at(i3);

        for(int k3=0; k3<nPanels(); k3++)
        {
            Panel3 const &p3k = m_Panel3.at(k3);
            if(p3k.isTrailing() && (p3k.isBotPanel() || p3k.isMidPanel()))
            {
                // whether p3k is on the left or right wing,
                // node 1 is its left trailing node and node 2 is its right trailing node
                // node 0 is the panel3's leading edge and doesn't get any contribution

                // make the wake contributions for the three rows: they are built once and used twice
                // calculate the contribution of the wake column shed by p3k
                if(!scalarProductWake(p3i, p3k.iWake(), scalarLeft, scalarRight))
                {
                    QString strange;
                    strange = QString::asprintf("*** numerical error calculating the scalar products of panel %d with the wake contribution of TE panel %d\n", i3, k3);
                    strange += "*** aborting calculation \n";
                    m_bMatrixError = true;
                    m_bWarning = true;
                    traceLog(strange);
                    return;
                }
                for(int ib=0; ib<3; ib++)
                {
                    LeftContrib[ib]  = scalarLeft[ib];
                    RightContrib[ib] = scalarRight[ib];
                }

                if(p3k.isMidPanel())
                {
                    double sign = +1.0;
                    // add wake contribution to mid panel's contribution
                    for(int ib=0; ib<3; ib++)
                    {
                        int row = 3*i3 + ib;
    //                    col0 = 3*k3;
                        col1 = 3*k3+1;
                        col2 = 3*k3+2;
                        if(s_bDoublePrecision)
                        {
                            // add the wake's left contribution to basis function 1
                            m_aijd[uint(row*N + col1)] += sign * LeftContrib[ib];

                            // add the wake's right contribution to basis function 2
                            m_aijd[uint(row*N + col2)] += sign * RightContrib[ib];
                        }
                        else
                        {
                            // add the wake's left contribution to basis function 1
                            m_aijf[uint(row*N + col1)] += float(sign * LeftContrib[ib]);

                            // add the wake's right contribution to basis function 2
                            m_aijf[uint(row*N + col2)] += float(sign * RightContrib[ib]);
                        }
                    }
                }
                else if(p3k.isBotPanel())
                {
                    double sign = -1.0;
                    // add wake contribution to bottom panel's contribution
                    for(int ib=0; ib<3; ib++)
                    {
                        int row = 3*i3 + ib;
    //                    col0 = 3*k3;
                        col1 = 3*k3+1;
                        col2 = 3*k3+2;
                        if(s_bDoublePrecision)
                        {
                            // add the wake's left contribution to basis function 1
                            m_aijd[uint(row*N + col1)] += sign * LeftContrib[ib];

                            // add the wake's right contribution to basis function 2
                            m_aijd[uint(row*N + col2)] += sign * RightContrib[ib];
                        }
                        else
                        {
                            // add the wake's left contribution to basis function 1
                            m_aijf[uint(row*N + col1)] += float(sign * LeftContrib[ib]);

                            // add the wake's right contribution to basis function 2
                            m_aijf[uint(row*N + col2)] += float(sign * RightContrib[ib]);
                        }
                    }

                    // add opposite wake contribution to opposite top TE panel's contribution
                    int k3t = p3k.oppositeIndex();
                    sign = 1.0;
                    for(int ib=0; ib<3; ib++)
                    {
                        int row = 3*i3 + ib;
    //                    col0 = 3*k3;
                        col1 = 3*k3t+1;
                        col2 = 3*k3t+2;
                        if(s_bDoublePrecision)
                        {
                            // add the wake's left contribution to basis function 1
                            m_aijd[uint(row*N + col1)] += sign * LeftContrib[ib];

                            // add the wake's right contribution to basis function 2
                            m_aijd[uint(row*N + col2)] += sign * RightContrib[ib];
                        }
                        else
                        {
                            // add the wake's left contribution to basis function 1
                            m_aijf[uint(row*N + col1)] += float(sign * LeftContrib[ib]);

                            // add the wake's right contribution to basis function 2
                            m_aijf[uint(row*N + col2)] += float(sign * RightContrib[ib]);
                        }
                    }
                }
            }

            if(isCancelled() || m_bMatrixError) return;
        }

        if(isCancelled() || m_bMatrixError) break;
    }
}


/**
 * @brief wakeScalarProduct calculates the scalar product of the potential induced by the left and right sides of a
 * wake column with a basis function of the given wing's panel.
 * @param pPanel0 a pointer to the wing's panel over which the scalar product is to be performed
 * @param ib the index of the basis function with which to perform the scalar product
 * @param iWake the index of the first wake element in the column
 * @param scalarLeft  the scalar product of the left  side of the wake column with the wing panel's basis function
 * @param scalarRight the scalar product of the right side of the wake column with the wing panel's basis function
 */
bool P3LinAnalysis::scalarProductWake(Panel3 const &panel0, int iWake, double *scalarLeft, double *scalarRight) const
{
    double sum_Left[]={0.0,0.0,0.0};
    double sum_Right[]={0.0,0.0,0.0};
    double value_Left=0.0, value_Right=0.0;
    double integrandL=0.0, integrandR=0.0;
    double phiW[]={0.0,0.0,0.0};
    double phiWG[]={0.0,0.0,0.0};
    Vector3d VW[3], VWG[3];
    Vector3d S[3];


    Vector3d ptGlobal;

    GQTriangle gq(Panel3::quadratureOrder());

    for(uint igq=0; igq<gq.points().size(); igq++)
    {
        double x = panel0.m_Sl[0].x*(1.0-gq.points().at(igq).x-gq.points().at(igq).y) + panel0.m_Sl[1].x*gq.points().at(igq).x + panel0.m_Sl[2].x*gq.points().at(igq).y;
        double y = panel0.m_Sl[0].y*(1.0-gq.points().at(igq).x-gq.points().at(igq).y) + panel0.m_Sl[1].y*gq.points().at(igq).x + panel0.m_Sl[2].y*gq.points().at(igq).y;

        panel0.localToGlobalPosition(x,y,0.0, ptGlobal.x, ptGlobal.y, ptGlobal.z);

        value_Left = value_Right = 0.0;
        int iter=0;
        int iWakeinitial=iWake;
        do
        {
            Panel3 const &p3w = m_WakePanel3.at(iWakeinitial);
//            m_WakePanel3.at(iWakeinitial);
            if(m_pPolar3d->bNeumann() || panel0.isMidPanel())
            {
                p3w.doubletBasisVelocity(ptGlobal, VW, false);
            }
            else
            {
                p3w.doubletBasisPotential(ptGlobal, false, phiW, false);
                for(int ia=0; ia<3; ia++)
                    if(std::isnan(phiW[ia]) || std::isinf(phiW[ia])) return false;
            }

            if(m_pPolar3d->bGroundEffect() || m_pPolar3d->bFreeSurfaceEffect())
            {
                double coef = m_pPolar3d->bGroundEffect() ? 1.0 : -1.0;

                // add the contribution of the symmetric panel below the water's surface
                // make the symmetric panel and reverse its orientation
                for(int in=0; in<3; in++)  S[in].set(p3w.node(in).x, p3w.node(in).y, -p3w.node(in).z-2.0*m_pPolar3d->groundHeight());
                Panel3 p3wG(S[0], S[2], S[1]);

                if(m_pPolar3d->bNeumann() || panel0.isMidPanel()) p3wG.doubletBasisVelocity(ptGlobal, VWG, false);
                else                                              p3wG.doubletBasisPotential(ptGlobal, false, phiWG, false);

                for(int k=0; k<3; k++) VW[k] += VWG[k]*coef;
                phiW[0] += phiWG[0]*coef;
                phiW[1] += phiWG[1]*coef;
                phiW[2] += phiWG[2]*coef;
            }

            if(p3w.isLeftSidePanel())
            {
                // left  panel, nodes 0&2 contribute to left side, node 1 contributes to right side
                if(m_pPolar3d->bNeumann()|| panel0.isMidPanel())
                {
                    value_Left  += VW[0].dot(panel0.normal()) + VW[2].dot(panel0.normal());
                    value_Right += VW[1].dot(panel0.normal());
                }
                else
                {
                    value_Left  += phiW[0] + phiW[2];
                    value_Right += phiW[1];
                }
            }
            else
            {
                // right panel, node 2 contributes to left side, nodes 0&1 contribute to right side
                if(m_pPolar3d->bNeumann()|| panel0.isMidPanel())
                {
                    value_Left  += VW[2].dot(panel0.normal());
                    value_Right += VW[0].dot(panel0.normal()) + VW[1].dot(panel0.normal());
                }
                else
                {
                    value_Left  += phiW[2];
                    value_Right += phiW[0] + phiW[1];
                }
            }

            if(p3w.iPD()<0) break;
            iWakeinitial = p3w.iPD();
        } while(true && iter++<1000);
//            assert(iter+1==m_pWPolar->m_nXWakePanel4*2);

        for(int ib=0; ib<3; ib++)
        {
            integrandL = gq.weights().at(igq) * value_Left  * panel0.basis(x,y,ib);
            sum_Left[ib]  += integrandL;
            if(std::isnan(integrandL) || std::isinf(integrandL)) return false;
            integrandR = gq.weights().at(igq) * value_Right  * panel0.basis(x,y,ib);
            if(std::isnan(integrandR) || std::isinf(integrandR)) return false;
            sum_Right[ib]  += integrandR;
        }
    }

    for(int ib=0; ib<3; ib++)
    {
        scalarLeft[ib]  = sum_Left[ib]  * panel0.area();
        scalarRight[ib] = sum_Right[ib] * panel0.area();
    }
    return true;
}


void P3LinAnalysis::backSubUnitRHS(double *uRHS, double *vRHS, double*wRHS, double *pRHS, double *qRHS, double *rRHS)
{
    PanelAnalysis::backSubUnitRHS(uRHS, vRHS, wRHS, pRHS, qRHS, rRHS);

    int size = nPanels()*3 * sizeof(double);

    if(uRHS)
    {
        m_uRHSVertex.resize(nPanels()*3);
        memcpy(m_uRHSVertex.data(), uRHS, size);
    }

    if(vRHS)
    {
        m_vRHSVertex.resize(nPanels()*3);
        memcpy(m_vRHSVertex.data(), vRHS, size);
    }

    if(wRHS)
    {
        m_wRHSVertex.resize(nPanels()*3);
        memcpy(m_wRHSVertex.data(), wRHS, size);
    }

    if(pRHS)
    {
        m_pRHSVertex.resize(nPanels()*3);
        memcpy(m_pRHSVertex.data(), pRHS, size);
    }

    if(qRHS)
    {
        m_qRHSVertex.resize(nPanels()*3);
        memcpy(m_qRHSVertex.data(), qRHS, size);
    }

    if(rRHS)
    {
        m_rRHSVertex.resize(nPanels()*3);
        memcpy(m_rRHSVertex.data(), rRHS, size);
    }
}


/** Applicable when the velocity field is not a solid body movement, i.e. with virtual twist */
void P3LinAnalysis::makeRHSBlock(int iBlock, double *RHS, std::vector<Vector3d> const &VField, Vector3d const*normals) const
{
    // for each panel
    int blockSize = int(nPanels()/m_nBlocks) +1;
    int iStart = iBlock*blockSize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blockSize, maxRows);

    double SP[] = {0,0,0}; // the 3 scalar products for a given panel
    Vector3d VPanel3, Normal;

    for(int i3=iStart; i3<iMax; i3++)
    {
        Panel3 const &p3i = m_Panel3.at(i3);
        Normal = normals ? normals[i3] : p3i.normal();

        VPanel3 = VField.at(i3);

        int row = 3*i3;

        if(m_pPolar3d->bNeumann() || p3i.isMidPanel())
        {
            // first term of RHS is -V.n
            /** @todo if VPanel3 is not uniform on the panel, replace by a scalar product? */
            RHS[row] = RHS[row+1] = RHS[row+2] = - VPanel3.dot(Normal) * p3i.area()/3.0;
        }
        else // Dirichlet or thick surface
        {
            RHS[row] = RHS[row+1] = RHS[row+2] = 0.0;
        }

        // add the scalar products of influences of source panels with the basis functions of panel i3
        for(int k3=0; k3<nPanels(); k3++)
        {
            Panel3 const &p3k = m_Panel3.at(k3);
            if(p3k.isMidPanel())
            {
                // no source singularity on thin surfaces
            }
            else
            {
                if(m_pPolar3d->bNeumann() || p3i.isMidPanel())
                {
                    p3i.scalarProductSourceVelocity(p3k, i3==k3, SP);
                }
                else
                {
                    p3i.scalarProductSourcePotential(p3k, i3==k3, SP);
                }

                double sigma = sourceStrength(p3k.normal(), VPanel3);
                RHS[row]   -= sigma * SP[0];
                RHS[row+1] -= sigma * SP[1];
                RHS[row+2] -= sigma * SP[2];
            }
        }

        if(isCancelled()) break;
    }
}


void P3LinAnalysis::sourceToRHS(std::vector<double> const &sigma, std::vector<double> &RHS)
{
    int ncols = int(m_Panel3.size());
    int nrows = 3*int(m_Panel3.size());

    // the source influence matrix is in signle precision to save space, so make the conversions from double to float and vice versa
    std::vector<float> sig(ncols);
    std::vector<float> rhs(nrows);

    for(int i=0; i<ncols; i++) sig[i] = float(sigma.at(i));
//    auto t0 = std::chrono::high_resolution_clock::now();
//    matVecMult(m_bijf.data(), sig.constData(), rhs.data(), nrows, ncols);
    matrix::matVecMultLapack(m_bijf.data(), sig.data(), rhs.data(), ncols, nrows, std::thread::hardware_concurrency());

/*    auto t1 = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    qDebug("P3LinAnalysis::sourceToRHS:matmult %g ms", double(duration)/1000.0);*/

    for(int i=0; i<nrows; i++) RHS[i] = double(rhs.at(i));
}


/**
 * @brief Makes the unit local velocity vectors from the unit doublet strengths.
 * Used for moment calculations in stability analysis of 3d panels.
 * Also used to make the Cp coefficients for each aoa.
 * Unlike in uniform methods, vVelocity vectors are in
 * __global__ coordinates because there are no reference frames at the nodes.
 */
void P3LinAnalysis::makeLocalVelocities(std::vector<double> const &uRHS, const std::vector<double> &vRHS, const std::vector<double> &wRHS,
                                        std::vector<Vector3d> &uVLocal, std::vector<Vector3d> &vVLocal, std::vector<Vector3d> &wVLocal, Vector3d const &) const
{
    // build the values at the nodes to calculate local velocities
    double valmin=0, valmax=0, coef=1.0;
    std::vector<double> uNodes(nNodes());
    std::vector<double> vNodes, wNodes;
    std::vector<Vector3d> uGlobal(nNodes());
    std::vector<Vector3d> vGlobal, wGlobal;

    /** @todo should use modified node array instead in case of T67 polars*/
    TriMesh::makeNodeValues(m_pRefTriMesh->nodes(), m_Panel3, uRHS, uNodes, valmin, valmax, coef);
    if(vRHS.size())
    {
        vNodes.resize(nNodes());
        vGlobal.resize(nNodes());
        TriMesh::makeNodeValues(m_pRefTriMesh->nodes(), m_Panel3, vRHS, vNodes, valmin, valmax, coef);
    }

    if(vRHS.size())
    {
        vNodes.resize(nNodes());
        vGlobal.resize(nNodes());
        TriMesh::makeNodeValues(m_pRefTriMesh->nodes(), m_Panel3, vRHS, vNodes, valmin, valmax, coef);
    }

    if(wRHS.size())
    {
        wNodes.resize(nNodes());
        wGlobal.resize(nNodes());
        TriMesh::makeNodeValues(m_pRefTriMesh->nodes(), m_Panel3, wRHS, wNodes, valmin, valmax, coef);
    }

    for(int in=0; in<nNodes(); in++)
    {
        makeNodeDoubletSurfaceVelocity(in, uNodes, vNodes, wNodes, uGlobal[in], vGlobal[in], wGlobal[in]);
    }
    // rebuild the array of doublet densities at the panel's nodes
    for(int ip=0; ip<nPanels(); ip++)
    {
        Panel3 const &p3 = m_Panel3.at(ip);
        int idx = p3.index();
        for(int k=0; k<3; k++)
        {
            int nidx = p3.nodeIndex(k);

            uVLocal[3*idx+k] = uGlobal.at(nidx);
            vVLocal[3*idx+k] = vGlobal.at(nidx);
            wVLocal[3*idx+k] = wGlobal.at(nidx);
        }
    }
}


/**
 * This method calculates the Cp coefficient at a node based on the distribution of doublet strengths.
 *  (i)   Use the plane defined by the node's normal
 *  (ii)  rotate=project/expand the neighbour nodes on the plane;
 *  (iii) build the plane best fitting the doublet values on the projected nodes - linear or quadratic regression tbd
 * @param p the index of the node
 * @param Mu the array of doublet strength values
 * @param VLocal a reference to the calculated local velocity on the panel
 */
void P3LinAnalysis::makeNodeDoubletSurfaceVelocity(int iNode, std::vector<double> const &uRHS, std::vector<double> const &vRHS, std::vector<double> const &wRHS,
                                                   Vector3d &uNode, Vector3d &vNode, Vector3d &wNode) const
{
    Node const &node = m_pRefTriMesh->nodeAt(iNode);
    // Some wing tip nodes are left unconnected during the mesh constructions
    // ignore them
    // All other nodes should have at least two neighbours
    if(node.neighbourNodeCount()==0)
    {
        uNode.reset();
        vNode.reset();
        wNode.reset();
        return;
    }
    assert(node.neighbourNodeCount()>=2);

    Vector3d const &nnormal = node.normal();
    int nnodes = node.neighbourNodeCount() + 1;

    // build a local Cartesian frame at the node's location
    CartesianFrame frame;

    Vector3d Xrnd, I, J;

    // find a random direction not aligned with the normal
    int iter=0;
    do
    {
        double phi   = xfl::randomfloat(PIf);
        double theta = xfl::randomfloat(PIf);
        Xrnd.set(cos(theta)*cos(phi), sin(theta)*cos(phi), sin(phi));
        iter++;
    }
    while(Xrnd.dot(nnormal)>0.25 && Xrnd.dot(nnormal)<0.75 && iter<50);
    if(iter>=50)
    {
        traceLog(QString::asprintf("      ***** Error making doublet derivative at node %d\n", iNode));
        uNode.set(0,0,0);
        wNode.set(0,0,0);
        return;
    }

    geom::rotateOnPlane(nnormal, Xrnd, I);
    J = node.normal()*I;
    frame.setFrame(node, I, J, nnormal);

    // find the plane which best fits the values of Mu at all local nodes

    Vector3d pt, ptl, neighl;
    std::vector<double> A(nnodes*3);
    std::vector<double> Mu(nnodes*3);

    A[0*nnodes] = 1.0;
    A[1*nnodes] = 0.0; // =node.x to local
    A[2*nnodes] = 0.0; // =node.y to lacal
    Mu[0*nnodes] = uRHS.at(iNode);
    Mu[1*nnodes] = iNode < int(vRHS.size()) ? vRHS.at(iNode) : 0.0;
    Mu[2*nnodes] = iNode < int(wRHS.size()) ? wRHS.at(iNode) : 0.0;

    int m=1;
    for(int in=0; in<node.neighbourNodeCount(); in++)
    {
        Node const &neigh = m_pRefTriMesh->nodeAt(node.neigbourNodeIndex(in));

        // rotate the neighbour node so that it lies in the plane defined by the origin node and its normal
        geom::rotateOnPlane(node, nnormal, neigh, pt);

        // convert to local coordinates
        frame.globalToLocalPosition(pt, ptl);

        A[0*nnodes+m]  = 1.0;
        A[1*nnodes+m]  = ptl.x;
        A[2*nnodes+m]  = ptl.y;
        Mu[0*nnodes+m] = uRHS.at(neigh.index());
        Mu[1*nnodes+m] = neigh.index()<int(vRHS.size()) ? vRHS.at(neigh.index()) : 0.0;
        Mu[2*nnodes+m] = neigh.index()<int(wRHS.size()) ? wRHS.at(neigh.index()) : 0.0;

        m++;
    }

    lapack_int info=0, n=3, lda=3, ldb=2, nrhs=3;

    char trans = 'N';
    lapack_int lwork = -1; // start with query
    lda = std::max(1,nnodes); // LDA >= max(1,M)
    ldb = std::max(1,nnodes); // LDB >= MAX(1,M,N)

    double wkopt=0;
#ifdef OPENBLAS
    dgels_(&trans, &nnodes, &n, &nrhs, A.data(), &lda, Mu.data(), &ldb, &wkopt, &lwork, &info,1);
#elif defined INTEL_MKL
    dgels_(&trans, &nnodes, &n, &nrhs, A.data(), &lda, Mu.data(), &ldb, &wkopt, &lwork, &info);
#elif defined ACCELERATE
    dgels_(&trans, &nnodes, &n, &nrhs, A.data(), &lda, Mu.data(), &ldb, &wkopt, &lwork, &info);
#endif

    lwork = int(wkopt);
    if(lwork>0 && info==0)
    {
        std::vector<double> work(lwork);
#ifdef OPENBLAS
        dgels_(&trans, &nnodes, &n, &nrhs, A.data(), &lda, Mu.data(), &ldb, work.data(), &lwork, &info,1);
#elif defined INTEL_MKL
        dgels_(&trans, &nnodes, &n, &nrhs, A.data(), &lda, Mu.data(), &ldb, work.data(), &lwork, &info);
#elif ACCELERATE
        dgels_(&trans, &nnodes, &n, &nrhs, A.data(), &lda, Mu.data(), &ldb, work.data(), &lwork, &info);
#endif
    }
    else info = 1;

    if(info!=0)
    {
        std::string strange = "      NodeDoubletSurfaceVelocity: Error making doublet derivative\n";
        traceStdLog(strange);
        uNode.reset();
        vNode.reset();
        wNode.reset();
        return;
    }

    //    double µ = -(a*sl[0].x + b*sl[0].y + c*mu0);
    // the gradient is (dµ/dx, dµ/dy)
    Vector3d uLocal,vLocal, wLocal;

    uLocal.x = -4.0*PI * (Mu[0*nnodes+1]);
    uLocal.y = -4.0*PI * (Mu[0*nnodes+2]);
    uLocal.z = 0.0;
    vLocal.x = -4.0*PI * (Mu[1*nnodes+1]);
    vLocal.y = -4.0*PI * (Mu[1*nnodes+2]);
    vLocal.z = 0.0;
    wLocal.x = -4.0*PI * (Mu[2*nnodes+1]);
    wLocal.y = -4.0*PI * (Mu[2*nnodes+2]);
    wLocal.z = 0.0;

    // convert back to global frame
    frame.localToGlobal(uLocal, uNode);
    if(vRHS.size()) frame.localToGlobal(vLocal, vNode);
    if(wRHS.size()) frame.localToGlobal(wLocal, wNode);
}


/**
 * @brief Calculates the Cp coefficients at the panel node.
 * Assumes that the unit velocity vectors induced by doublet densities
 * at the panel nodes have been calculated at the previous stage.
 *
 * UNLIKE IN UNIFORM METHODS, CALCULATIONS ARE MADE IN GLOBAL COORDINATES
 * VLOCAL IS IN GLOBAL COORDINATES
 */
void P3LinAnalysis::computeOnBodyCp(const std::vector<Vector3d> &VInf, std::vector<Vector3d> const &VGLOBAL, std::vector<double>&Cp) const
{
    double QInf(0), Speed2(0), CpSup(0), CpInf(0);
    Vector3d VPanel;
    Vector3d Vtotsup, Vtotinf;
    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);
        for(int ivtx=0; ivtx<3; ivtx++)
        {
            Vector3d const &n0 = p3.vertexAt(ivtx).normal();

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

                Vtotsup = VInf.at(i3) + VGLOBAL.at(3*i3+ivtx) *0.5;
                Speed2 = Vtotsup.x*Vtotsup.x + Vtotsup.y*Vtotsup.y;
                CpSup = 1.0-Speed2/QInf/QInf;

                Vtotinf = VInf.at(i3) - VGLOBAL.at(3*i3+ivtx) *0.5;
                Speed2 = Vtotinf.x*Vtotinf.x + Vtotinf.y*Vtotinf.y;
                CpInf = 1.0-Speed2/QInf/QInf;

                if(p3.vertexAt(ivtx).surfacePosition()!=xfl::NOSURFACE)
                    Cp[3*i3+ivtx] = CpSup-CpInf;
                else
                    Cp[3*i3+ivtx] = CpSup-CpInf;
            }
            else
            {
                VPanel.set(VInf.at(i3) + VGLOBAL.at(3*i3+ivtx));

                // remove the part of the velocity normal to the surface at the node
                VPanel -= n0*VPanel.dot(n0);

                if(p3.vertexAt(ivtx).surfacePosition()!=xfl::NOSURFACE)
                    Cp[3*i3+ivtx] = 1.0-(VPanel.x*VPanel.x + VPanel.y*VPanel.y + VPanel.z*VPanel.z)/QInf/QInf;
                else
                    Cp[3*i3+ivtx] = 0.0;
            }
        }
    }
}


void P3LinAnalysis::makeUnitRHSBlock(int iBlock)
{
    int blockSize = int(nPanels()/m_nBlocks) +1;
    int iStart = iBlock*blockSize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blockSize, maxRows);

    double SP[]{0,0,0}; // the 3 scalar products for a given panel

    Vector3d Vx(1,0,0), Vy(0,1,0), Vz(0,0,1);
    Vector3d Omx(1,0,0), Omy(0,1,0), Omz(0,0,1);


    Vector3d leverarm_i3, leverarm_k3;
    double sigma=0;

    int iStation = 0;
    for(int i3=iStart; i3<iMax; i3++)
    {
        Panel3 const &p3i = m_Panel3.at(i3);
        leverarm_i3.set(p3i.CoG()-m_pPolar3d->CoG());

        int row = 3*i3;
        if(m_pPolar3d->bNeumann() || p3i.isMidPanel())
        {
            // first term of RHS is -V.n
            /** @todo if VPanel3 is not uniform on the panel, replace by a scalar product */
            // assign values one at a time otherwise issues when multithreading

            m_uRHS[row] = m_uRHS[row+1] = m_uRHS[row+2] = -Vx.dot(p3i.normal()) * p3i.area()/3.0;
            m_vRHS[row] = m_vRHS[row+1] = m_vRHS[row+2] = -Vy.dot(p3i.normal()) * p3i.area()/3.0;
            m_wRHS[row] = m_wRHS[row+1] = m_wRHS[row+2] = -Vz.dot(p3i.normal()) * p3i.area()/3.0;
            m_pRHS[row] = m_pRHS[row+1] = m_pRHS[row+2] = - (leverarm_i3 * Omx).dot(p3i.normal()) * p3i.area()/3.0;
            m_qRHS[row] = m_qRHS[row+1] = m_qRHS[row+2] = - (leverarm_i3 * Omy).dot(p3i.normal()) * p3i.area()/3.0;
            m_rRHS[row] = m_rRHS[row+1] = m_rRHS[row+2] = - (leverarm_i3 * Omz).dot(p3i.normal()) * p3i.area()/3.0;
        }
        else // Dirichlet or thick surface
        {
            m_uRHS[row] = m_uRHS[row+1] = m_uRHS[row+2] = 0;
            m_vRHS[row] = m_vRHS[row+1] = m_vRHS[row+2] = 0;
            m_wRHS[row] = m_wRHS[row+1] = m_wRHS[row+2] = 0;
            m_pRHS[row] = m_pRHS[row+1] = m_pRHS[row+2] = 0;
            m_qRHS[row] = m_qRHS[row+1] = m_qRHS[row+2] = 0;
            m_rRHS[row] = m_rRHS[row+1] = m_rRHS[row+2] = 0;
        }

        // add the scalar products of influences of source panels with the basis functions of panel i3
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
                    p3i.scalarProductSourceVelocity(p3k, i3==k3, SP);
                }
                else
                {
                    p3i.scalarProductSourcePotential(p3k, i3==k3, SP);
                }

                sigma = sourceStrength(p3k.normal(), Vx);
                for(int ib=0; ib<3; ib++)  m_uRHS[row+ib] -= sigma * SP[ib];

                sigma = sourceStrength(p3k.normal(), Vy);
                for(int ib=0; ib<3; ib++)  m_vRHS[row+ib] -= sigma * SP[ib];

                sigma = sourceStrength(p3k.normal(), Vz);
                for(int ib=0; ib<3; ib++)  m_wRHS[row+ib] -= sigma * SP[ib];

                // corrected in v7.09
                sigma = sourceStrength(p3k.normal(), leverarm_k3*Omx);
//                for(int ib=0; ib<3; ib++)  m_pRHS[row] -= sigma * SP[ib];
                for(int ib=0; ib<3; ib++)  m_pRHS[row+ib] -= sigma * SP[ib];

                sigma = sourceStrength(p3k.normal(), leverarm_k3*Omy);
//               for(int ib=0; ib<3; ib++)  m_qRHS[row] -= sigma * SP[ib];
                for(int ib=0; ib<3; ib++)  m_qRHS[row+ib] -= sigma * SP[ib];

                sigma = sourceStrength(p3k.normal(), leverarm_k3*Omz);
//                for(int ib=0; ib<3; ib++)  m_rRHS[row] -= sigma * SP[ib];
                for(int ib=0; ib<3; ib++)  m_rRHS[row+ib] -= sigma * SP[ib];
            }
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

/** unused, untested in multithread mode  */
void P3LinAnalysis::makeSourceInfluenceMatrix()
{
    int ncols = int(m_Panel3.size());
    int nrows = 3*int(m_Panel3.size());

    m_bijf.resize(ncols*nrows);
    memset(m_bijf.data(), 0, m_bijf.size()*sizeof(float));

    if(s_bMultiThread)
    {

        std::vector<std::thread> threads;

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
//            futureSync.addFuture(QtConcurrent::run(&P3LinAnalysis::makeSourceMatrixBlock, this, iBlock));
            threads.push_back(std::thread(&P3LinAnalysis::makeSourceMatrixBlock, this, iBlock));
        }

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            threads[iBlock].join();
        }
        std::cout << "P3LinAnalysis::makeSourceInfluenceMatrix joined all " << m_nBlocks << " threads" <<std::endl;
    }
    else
    {
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            makeWakeMatrixBlock(iBlock);
        }
    }
}


void P3LinAnalysis::makeSourceMatrixBlock(int iBlock)
{
    // for each panel
    int blockSize = int(nPanels()/m_nBlocks) +1;
    int iStart = iBlock*blockSize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blockSize, maxRows);


    double SP[]{0,0,0}; // the 3 scalar products for a given panel

    int ncols = int(m_Panel3.size());

    for(int i3=iStart; i3<iMax; i3++)
    {
        Panel3 const &p3i = m_Panel3.at(i3);
        int row0 = 3*i3;

        for(int k3=0; k3<nPanels(); k3++)
        {
            Panel3 const &p3k = m_Panel3.at(k3);

            if(m_pPolar3d->bNeumann() || p3i.isMidPanel())
            {
                p3i.scalarProductSourceVelocity(p3k, i3==k3, SP);
            }
            else
            {
                p3i.scalarProductSourcePotential(p3k, i3==k3, SP);
            }
            for(int ib=0; ib<3; ib++)
                m_bijf[(row0+ib)*ncols + k3] = -float(SP[ib]); // minus sign to include the move to the equation's RHS

            if(isCancelled() || m_bMatrixError) return;
        }

        if(isCancelled() || m_bMatrixError) break;
    }
}


void P3LinAnalysis::makeUnitDoubletStrengths(double alpha, double beta)
{
    int N = 3*nPanels();
    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);
    double cosb = cos(-beta *PI/180.0); //change of beta sign introduced in v7.24 to be consistent with AVL
    double sinb = sin(-beta *PI/180.0); //change of beta sign introduced in v7.24 to be consistent with AVL

    for(int p=0; p<N; p++)
    {
        m_Mu[p] = cosa*cosb*m_uRHSVertex.at(p) + sinb*m_vRHSVertex.at(p) + sina*cosb* m_wRHSVertex.at(p);
    }
}


/**
 * For a tri-linear analysis, builds a row of vortons.
 * Firstly, an approximate piecewise linear distribution of the doublet density at the TE is built.
 * Secondly, this continuous doublet profile is converted in the specified number of vortons
 * located at the panel edges.
 * The vorticity at the two tips is equal to the panel's doublet strength at the edge,
 * i.e. zero vorticity outside the wake.*/
void P3LinAnalysis::makeVortons(double dl, double const *mu3Vertex, int pos3, int nPanel3, int nStations, int nVtn0,
                                std::vector<Vorton> &vortons, std::vector<Vortex> &vortexneg) const
{
    double const *Mu3   = mu3Vertex;

    // lump the ciculation into vortons
    vortons.resize(2*nStations);
    vortexneg.resize(nStations);

    double gLeft=0, gRight=0, gamma=0;
    Segment3d RightEdge;
    Vector3d RightNode;
    Vector3d U;

    int m=0;

    for(int i3=0; i3<nPanel3; i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3+pos3);
        if(p3.isTrailing())
        {
            assert(p3.iWake()>=0 && p3.iWake()<int(m_WakePanel3.size()));
            Panel3 p3WU, p3WD;
            trailingWakePanels(m_WakePanel3.data() + p3.iWake(), p3WU, p3WD);

            if(p3.isMidPanel() || p3.isBotPanel()) // skip top panels
            {
                if(p3.isMidPanel())
                {
                    int idxM = p3.index();
                    gLeft  = Mu3[3*idxM+1] *4.0*PI;
                    gRight = Mu3[3*idxM+2] *4.0*PI;
                }
                else  if(p3.isBotPanel())
                {
                    int idxB = p3.index();
                    int idxU = nextTopTrailingPanelIndex(p3);
                    assert(idxU>=0);

                    gLeft  = (Mu3[3*idxU+1] - Mu3[3*idxB+1])*4.0*PI;
                    gRight = (Mu3[3*idxU+2] - Mu3[3*idxB+2])*4.0*PI;
                }
                gamma = (gLeft+gRight)/2.0;

                // Willis 2005 III.D p.9
                // A standard result is that the change in dipole strength along a surface is equivalent to vorticity oriented
                // in the surface tangential direction normal to the dipole gradient.
                // In the case of constant dipole panels, the vortex analogue is a vortex ring around the perimeter of the
                // given panel. Hence, the strength of the vortex line segment between two adjacent constant strength panels is merely
                // the difference in dipole strengths.

                // --> lump the difference of mid doublet densities into the edge vorton
                // this accounts both for the doublet gradient and the jump at the edge


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

                // the negating vortex has a linear circulation, approximated here by a uniform average
                vortexneg[m].setCirculation(-gamma); // negating circulation 4.PI Mu4

                m++;
            }
        }
    }

    assert(m==nStations);
}


void P3LinAnalysis::testResults(double alpha, double beta, double QInf) const
{
    // check that perturbation potential & velocity are zero inside the body
    // check that residual is zero for each basis function, by recalculating the scalar products

    double const *mu = m_Mu.data();
    double const *sigma = m_Sigma.data();

    Vector3d VInf = objects::windDirection(alpha, beta)*QInf;
    Vector3d Vel;
//    double phi=0;

    if(m_Panel3.size()<157) return;

    int iTrace = 169;
    s_DebugPts.clear();
    s_DebugVecs.clear();

    for(int i3=0; i3<int(m_Panel3.size()); i3++)
    {
        Panel3 const & p3 = m_Panel3.at(i3);

        Vector3d VW[3], VWG[3];

        Vector3d ptGlobal;

        GQTriangle gq(Panel3::quadratureOrder());

        double average = 0;
        for(uint igq=0; igq<gq.points().size(); igq++)
        {
            double x = p3.m_Sl[0].x*(1.0-gq.points().at(igq).x-gq.points().at(igq).y) + p3.m_Sl[1].x*gq.points().at(igq).x + p3.m_Sl[2].x*gq.points().at(igq).y;
            double y = p3.m_Sl[0].y*(1.0-gq.points().at(igq).x-gq.points().at(igq).y) + p3.m_Sl[1].y*gq.points().at(igq).x + p3.m_Sl[2].y*gq.points().at(igq).y;

            p3.localToGlobalPosition(x,y,0.0, ptGlobal.x, ptGlobal.y, ptGlobal.z);
            ptGlobal += p3.normal()*0.0001;
            getVelocityVector(ptGlobal, mu, sigma, Vel, 0.0, false, false);
            Vel += VInf;
            average += Vel.dot(p3.normal());
            if(i3==iTrace)
            {
                s_DebugPts.push_back(ptGlobal);
                s_DebugVecs.push_back(Vel);
//                qDebug(" pt_%d: %13g", igq, Vel.dot(p3.normal()));
            }
        }
        qDebug(" average[%d]=%13g", i3, average/gq.points().size());
    }

}
