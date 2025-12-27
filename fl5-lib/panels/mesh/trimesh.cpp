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

#include <QString>

#include <string>
#include <iostream>
#include <thread>


#include <trimesh.h>

#include <units.h>
#include <utils.h>

bool TriMesh::s_bCancel = false;

TriMesh::TriMesh() : XflMesh()
{
    m_nBlocks = 1;
    m_nWakeColumns = 0;
}


void TriMesh::meshInfo(std::string &) const
{

}


void TriMesh::rotate(double alpha, double beta, double phi)
{
//    auto t0 = std::chrono::high_resolution_clock::now();

    Vector3d Origin;

    for(uint in=0; in<m_Node.size(); in++)
    {
        Node &nd = m_Node[in];
        if(fabs(phi)>ANGLEPRECISION)            nd.rotateX(Origin, phi);
        if(fabs(beta)>ANGLEPRECISION)           nd.rotateZ(Origin, beta);
        if(fabs(alpha)>ANGLEPRECISION)          nd.rotateY(Origin, alpha);
    }

    rebuildPanelsFromNodes(m_Panel3, m_Node);

/*    auto t1 = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    qDebug("TriMesh::rotateMesh  %gms",  double(duration)/1000.0);*/
}



void TriMesh::rotatePanels(Vector3d const &O, Vector3d const &axis, double theta)
{
    if(fabs(theta)<ANGLEPRECISION) return;
    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 &p3 = m_Panel3[i3];
        p3.rotate(O, axis, theta);
    }

    for(uint in=0; in<m_Node.size(); in++)
    {
        m_Node[in].rotate(O, axis, theta);
    }
}


void TriMesh::translatePanels(Vector3d const &T)
{
    translatePanels(T.x, T.y, T.z);
}


void TriMesh::translatePanels(double tx, double ty, double tz)
{
    if(fabs(tx)<PRECISION && fabs(ty)<PRECISION && fabs(tz)<PRECISION) return;
    for(int i3=0; i3<nPanels(); i3++)
    {
        m_Panel3[i3].translate(tx, ty, tz);
    }
    for(uint in=0; in<m_Node.size(); in++)
    {
        m_Node[in].translate(tx, ty, tz);
    }
}


/** makes the array of nodes and sets the vertex indices */
int TriMesh::makeNodeArrayFromPanels(int firstnodeindex, std::string &logmsg, std::string const &prefix)
{
    clearNodes();
    m_Node.reserve(nPanels()*2);
    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 &p3 = m_Panel3[i3];

        for(int iv=0; iv<3; iv++)
        {
            int iNode = isNode(p3.vertexAt(iv), s_NodeMergeDistance);

            if(iNode>=0)
            {
                // if trailing edge node, do not merge with node of opposite surface
                if(p3.isTrailingEdgeNode(iv) && p3.surfacePosition()!=m_Node.at(iNode).surfacePosition())
                {
                    // continue with a new node instead
                    iNode = -1;
                }
            }

            if(iNode<0)
            {
                iNode = nodeCount();
                addNode(p3.vertexAt(iv));
                lastNode().setIndex(firstnodeindex + nodeCount()-1);
                lastNode().setSurfacePosition(p3.surfacePosition());
                p3.setVertex(iv, node(iNode));
            }
            else
            {
                p3.setVertex(iv, node(iNode));
                p3.setFrame();
            }
            m_Node[iNode].addTriangleIndex(i3);
        }
    }

    QString strong;
    strong = QString::asprintf("Extracted %d nodes from the array of triangles\n", int(m_Node.size()));
    logmsg += prefix + strong.toStdString();
    return int(m_Node.size());
}


void TriMesh::setNodePanels()
{
    for(uint in=0; in<m_Node.size(); in++)
    {
        m_Node[in].clearTriangles();
    }

    // set panel references for each node
    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);
        for(int ivtx=0; ivtx<3; ivtx++)
        {
            int ind = p3.nodeIndex(ivtx);
            if(ind>=0 && ind <int(m_Node.size()))
            {
                Node &nd = m_Node[p3.nodeIndex(ivtx)];
                nd.addTriangleIndex(i3);
            }
        }
    }
}


void TriMesh::getMeshInfo(std::string &logmsg) const
{
    QString strong;

    strong = QString::asprintf("    Nbr. of triangles = %d\n"
                               "    Nbr. of nodes     = %d", nPanels(), nNodes());
    QString log = strong +"\n";
    double minarea = 1.e10;
    double maxarea = 0.0;
    int minareaindex = -1, maxareaindex = -1;
    double r=0.0, e=0.0;
    double minquality=1.e10, maxquality=0.0;
    int minqualityindex = -1, maxqualityindex = -1;

    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);
        if(p3.area()<minarea)
        {
            minareaindex=i3;
            minarea = p3.area();
        }
        if(p3.area()>maxarea)
        {
            maxareaindex=i3;
            maxarea = p3.area();
        }

        double q = p3.qualityFactor(r,e);
        if(q<minquality)
        {
            minqualityindex = i3;
            minquality = q;
        }
        if(q>maxquality)
        {
            maxqualityindex = i3;
            maxquality = q;
        }
        maxquality = std::max(maxquality, p3.qualityFactor(r,e));
    }
    strong = QString::asprintf("    min. panel area = %9.3g ", minarea*Units::m2toUnit());
    strong += Units::areaUnitQLabel();
    log += strong;
    strong = QString::asprintf(" for panel %d\n", minareaindex);
    log += strong;
    strong = QString::asprintf("    max. panel area = %9.3g ", maxarea*Units::m2toUnit());
    strong += Units::areaUnitQLabel();
    log += strong;
    strong = QString::asprintf(" for panel %d\n", maxareaindex);
    log += strong;

    strong = QString::asprintf("    min. quality factor = %7.3f for panel %d\n", minquality, minqualityindex);
    log += strong;
    strong = QString::asprintf("    max. quality factor = %7.3f for panel %d\n", maxquality, maxqualityindex);
    log += strong;

    logmsg = log.toStdString();
}


void TriMesh::checkPanels(std::string &logmsg,
                          bool bSkinny, bool bMinAngle, bool bMinArea, bool bMinSize,
                          std::vector<int> &skinnylist, std::vector<int> &minanglelist, std::vector<int>&minarealist, std::vector<int>&minsizelist,
                          double qualityfactor, double minangle, double minarea, double minsize)
{
    QString strong;
    QString log;

    //list skinny triangles
    int count(0);
    double q(0),r(0),e(0);

    if(bSkinny)
    {
        Panel3::setQualityFactor(qualityfactor);
        for(int i3=0; i3<nPanels(); i3++)
        {
            Panel3 const &p3 = m_Panel3.at(i3);
            q = p3.qualityFactor(r,e);
            if(p3.isSkinny())
            {
                skinnylist.push_back(p3.index());
                count++;
                strong = QString::asprintf("   Panel %4d is skinny:  qual.=%5.2f", p3.index(), q);
                log += strong;
                strong = QString::asprintf(", CC radius=%5.2f", r*Units::mtoUnit());
                log += strong + Units::lengthUnitQLabel();
                strong = QString::asprintf(", min. edge=%5.2f", e*Units::mtoUnit());
                log += strong + Units::lengthUnitQLabel() + "\n";
            }
        }
        if(!skinnylist.size())
        {
            log = "   No skinny panel found\n\n";
        }
        else
        {
            strong = QString::asprintf("   Found %d skinny triangles\n\n", count);
            log += strong;
        }
    }

    // list triangles with min angle above threshold
    if(bMinAngle)
    {
        count=0;

        for(int i3=0; i3<nPanels(); i3++)
        {
            Panel3 const &p3 = m_Panel3.at(i3);
            if(p3.isLowAngle(minangle))
            {
                minanglelist.push_back(p3.index());
                count++;
                strong = QString::asprintf("   Panel %4d has min angle %5.1f", p3.index(), p3.minAngle());
                strong += DEGch + "\n";
                log += strong;
            }
        }

        if(!minanglelist.size())
        {
            strong = QString::asprintf("   No panel found with vertex angle less than %.2f", minangle);
            strong += DEGch + "\n\n";
            log += strong + "\n\n";
        }
        else
        {
            strong = QString::asprintf("   Found %d panels with a vertex angle less than %.2f", count, minangle);
            strong += DEGch;
            log += strong + "\n\n";
        }
    }

    if(bMinArea)
    {
        // list triangles with area below threshold
        count=0;

        for(int i3=0; i3<nPanels(); i3++)
        {
            Panel3 const &p3 = m_Panel3.at(i3);
            if(fabs(p3.area())<minarea)
            {
                minarealist.push_back(p3.index());
                count++;
                strong = QString::asprintf("   Panel %4d has area %9g ", p3.index(), p3.area()*Units::m2toUnit());
                log += strong + Units::areaUnitQLabel() +"\n";
            }
            //                else m_PanelHightlight.insert(i3, false);
        }


        if(!minarealist.size())
        {
            log = "   No triangle with low area found\n";
        }
        else
        {
            strong = QString::asprintf("   Found %d panels with area less than %.3g ", count, minarea*Units::m2toUnit());
            log += strong + Units::areaUnitQLabel() + "\n\n";
        }
    }

    if(bMinSize)
    {
        // list triangles with edge length below threshold
        count=0;

        for(int i3=0; i3<nPanels(); i3++)
        {
            Panel3 const &p3 = m_Panel3.at(i3);
            double length(LARGEVALUE);
            for(int iedge=0; iedge<3; iedge++)
            {
                if(p3.edge(iedge).length()<length)
                {
                    length = p3.edge(iedge).length();
                }
            }
            if(length<minsize)
            {
                minsizelist.push_back(p3.index());
                count++;
                strong = QString::asprintf("   Panel %4d has edge length %9g ", p3.index(), length*Units::mtoUnit());
                log += strong + Units::lengthUnitQLabel() +"\n";
            }
        }


        if(!minsizelist.size())
        {
            log = "   No triangle with low edge length found\n";
        }
        else
        {
            strong = QString::asprintf("   Found %d panels with edge length less than %.g ", count, minsize*Units::mtoUnit());
            log += strong + Units::lengthUnitQLabel() + "\n\n";
        }
    }

    logmsg = log.toStdString();
}


void TriMesh::listPanels(bool bConnections)
{
    QString strange;
    if(bConnections)
        qDebug(" Panel   neigh0  neigh1   neigh2  Surf");
    else
        qDebug(" Panel     PU    PD    PL    PR  Surf");
    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);
        if(bConnections)
            strange = QString::asprintf(" %4d:  %4d  %4d  %4d", p3.index(), p3.neighbour(0), p3.neighbour(1), p3.neighbour(2));
        else
            strange = QString::asprintf(" %4d:  %4d  %4d  %4d  %4d", p3.index(), p3.m_iPU, p3.m_iPD, p3.m_iPL, p3.m_iPR);

        switch(p3.surfacePosition())
        {
            case xfl::BOTSURFACE:
                strange += "  bottom";
                break;
            case xfl::MIDSURFACE:
                strange += "  mid";
                break;
            case xfl::TOPSURFACE:
                strange += "  top";
                break;
            case xfl::SIDESURFACE:
                strange += "  side";
                break;
            case xfl::FUSESURFACE:
                strange += "  fuse";
                break;
            case xfl::WAKESURFACE:
                strange += "  wake";
                break;
            case xfl::NOSURFACE:
                strange += "  none";
                break;
        }
    }
}


void TriMesh::clearConnections()
{
    for(int it=0; it<nPanels(); it++)
    {
        m_Panel3[it].clearConnections();
    }
    for(uint in=0; in<m_Node.size(); in++)
    {
        m_Node[in].clearTriangles();
        m_Node[in].clearNeighbourNodes();
    }
}


/** Duplicates the connections, typically from the ref trimesh to the active trimesh;
 * Faster than recalculating;
 * For display information only */
void TriMesh::copyConnections(TriMesh const &mesh3)
{
    if(mesh3.panelCount()!=panelCount()) return; // something went wrong
    for(int i3=0; i3<panelCount(); i3++)
    {
        Panel3 const &refp3 = mesh3.panelAt(i3);
        Panel3 &p3 = m_Panel3[i3];
        p3.copyConnections(refp3);
    }

    for(uint in=0; in<m_Node.size(); in++)
    {
        Node const &refnode = mesh3.nodeAt(in);
        m_Node[in].setNeighbourNodeIndexes(refnode.nodeNeighbourIndexes());
        m_Node[in].setTriangleIndexes(refnode.triangleIndexes());
    }
}


/**
 * Connects the triangles of block [i0, i0+n0] to those of block [i1, i1+n1]
 * using node position.
 * Used to connect wing nodes to fuselage nodes.
 */
void TriMesh::connectMeshes(int i0, int n0, int i1, int n1)
{
    double const MAXDISTANCE = 1.e-4; // =0.1mm; consider that nodes node closer than this length are identical

    for(int it0=i0; it0<i0+n0; it0++)
    {
        Panel3 &p30 = m_Panel3[it0];
        if(p30.neighbourCount()==3) continue; // no further connections for this panel

        for(int it1=i1; it1<i1+n1; it1++)
        {
            Panel3 &p31 = m_Panel3[it1];
            if(p31.neighbourCount()==3) continue; // no further connections for this panel

            for(int ie=0; ie<3; ie++)
            {
                int nEdge0 = p30.edgeIndex(p31.edge(ie), MAXDISTANCE);
                if(nEdge0>=0)
                {
                    p30.setNeighbour(it1, nEdge0);

                    int nEdge1 = p31.edgeIndex(p30.edge(nEdge0), MAXDISTANCE);
                    if(nEdge1>=0) p31.setNeighbour(it0, nEdge1);
                    break;
                }
            }
        }

        if(s_bCancel) break;
    }
}


void TriMesh::makeConnectionsFromNodePosition2(int i0, int np0, double MERGEDISTANCE)
{
    if(i0+np0>nPanels())
    {
        QString err = QString::asprintf("TriMesh::makeConnectionsFromNodePosition error: %d %d %d", i0, np0, int(nPanels()));
        std::cerr << err.toStdString() << std::endl;
        return;
    }

    for(int it0=i0; it0<i0+np0; it0++)
    {
        Panel3 &p0 = panel(it0);
        // search starting from the base panel's index
        // since neighbours are likely to have a close index
        if(p0.neighbourCount()==3) continue;  // no further connections for this panel

        for(int it1=i0; it1<i0+np0; it1++)
        {
            if(it0==it1) continue; // do not connect the panel to itself
            Panel3 &p1 = panel(it1);

            if(p1.neighbourCount()==3) continue;  // no further connections for this panel

            if(p0.isFlapPanel() || p1.isFlapPanel())
            {
                if(p0.surfaceIndex() != p1.surfaceIndex()) continue; // avoid connecting flap side panels
            }

            if(p0.isTrailing())
            {
                if((p1.surfacePosition()==p0.surfacePosition()))
                {
                    for(int iEdge=0; iEdge<3; iEdge++)
                    {
                        int nEdge0 = p0.edgeIndex(p1.edge(iEdge), MERGEDISTANCE);
                        if(nEdge0>=0)
                        {
                            p0.setNeighbour(it1, nEdge0);
                            int nEdge1 = p1.edgeIndex(p0.edge(nEdge0), MERGEDISTANCE);
                            if(nEdge1>=0) // should alawys be true
                            {
                                p1.setNeighbour(it0, nEdge1);
                            }
                            break;
                        }
                    }
                }
                else if(p1.isFusePanel())
                {
                    assert(false);
                }
            }
            else
            {
                for(int ie=0; ie<3; ie++)
                {
                    int nEdge0 = p0.edgeIndex(p1.edge(ie), MERGEDISTANCE);
                    if(nEdge0>=0)
                    {
                        p0.setNeighbour(it1, nEdge0);
                        int nEdge1 = p1.edgeIndex(p0.edge(nEdge0), MERGEDISTANCE);
                        if(nEdge1>=0) // should always be true
                        {
                            p1.setNeighbour(it0, nEdge1);
                        }
                        break;
                    }
                }
            }
            if(p0.neighbourCount()==3) break;
         }
        
        if(s_bCancel) break;
    }
}


/**
 * Makes the connections between the triangles which share a common edge.
 * Untested in multithread mode.
 * Slow.
 */
void TriMesh::makeConnectionsFromNodePosition(bool bConnectTE, bool bMultiThread)
{
    s_bCancel = false;

    if(bMultiThread)
    {
        int nThreads = std::thread::hardware_concurrency();

        m_nBlocks = nThreads;
    }
    else
        m_nBlocks = 1;

    // Find neighbours with common edge.
    // If the panel is trailing, do not connect to the opposite surface to prevent incorrect Cp calculations
    clearConnections();

    if(bMultiThread)
    {
        std::vector<std::thread> threads;
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            threads.push_back(std::thread(&TriMesh::connectPanelBlock, this, iBlock, bConnectTE));
        }

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            threads[iBlock].join();
        }
//        std::cout << "TriMesh::makeConnectionsFromNodePosition joined all " << m_nBlocks << " threads" <<std::endl;

    }
    else
    {
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            connectPanelBlock(iBlock, bConnectTE);
        }
    }

//    listMesh(true);
}


void TriMesh::connectPanelBlock(int iBlock, bool bConnectTE)
{
    double const MAXDISTANCE = 1.e-4;
    int blockSize = int(double(nPanels())/double(m_nBlocks)) +1;
    int iStart = iBlock*blockSize;
    int maxRows = nPanels();
    int iMax = std::min(iStart+blockSize, maxRows);

    // set panel neighbours
    for(int it0=iStart; it0<iMax; it0++)
    {
        Panel3 &p3 = m_Panel3[it0];
        // search starting from the base panel's index
        // since neighbour indexes are likely to be close

        int it2m = it0;
        int it2p = it0;

        do
        {
            it2m--;
            it2p++;
//            if(p3.neighbourCount()==3) break;
            if(it2p<nPanels())
            {
                Panel3 const & p2 = m_Panel3.at(it2p);

                if(p3.isTrailing() && !bConnectTE)
                {
                    if(!p2.isOppositeSurface(p3.surfacePosition())) // do not connect opposite Trailing edge panels
                    {
                        for(int iEdge=0; iEdge<3; iEdge++)
                        {
                            int nEdge = p3.edgeIndex(p2.edge(iEdge), MAXDISTANCE);
                            if(nEdge>=0)
                            {
                                p3.setNeighbour(it2p, nEdge);
                                break;
                            }
                        }
                    }
                }
                else
                {
                    for(int ie=0; ie<3; ie++)
                    {
                        int nEdge = p3.edgeIndex(p2.edge(ie), MAXDISTANCE);
                        if(nEdge>=0)
                        {
                            p3.setNeighbour(it2p, nEdge);
                            break;
                        }
                    }
                }
            }

            if(it2m>=0)
            {
                Panel3 const & p2 = m_Panel3.at(it2m);

                if(p3.isTrailing() && !bConnectTE)
                {
                    if(!p2.isOppositeSurface(p3.surfacePosition())) // do not connect opposite Trailing edge panels
                    {
                        for(int iEdge=0; iEdge<3; iEdge++)
                        {
                            int nEdge = p3.edgeIndex(p2.edge(iEdge), MAXDISTANCE);
                            if(nEdge>=0)
                            {
                                p3.setNeighbour(it2m, nEdge);
                                break;
                            }
                        }
                    }
                }
                else
                {
                    for(int ie=0; ie<3; ie++)
                    {
                        int nEdge = p3.edgeIndex(p2.edge(ie), MAXDISTANCE);
                        if(nEdge>=0)
                        {
                            p3.setNeighbour(it2m, nEdge);
                            break;
                        }
                    }
                }
            }
        }
        while(p3.neighbourCount()<3 && (it2p<nPanels() || it2m>=0));

        if(s_bCancel) break;
    }
}


/** connects the panels if they share two vertices with the same indexes */
void TriMesh::makeConnectionsFromNodeIndexes(int i0, int n0, int i1, int n1)
{
    int ni[]{0,0,0};
    int nj[]{0,0,0};
    bool bConnected(false);
    int iEdge(-1);

    for(int it0=i0; it0<i0+n0; it0++)
    {
        Panel3 &p3i = m_Panel3[it0];
        if(p3i.neighbourCount()==3) continue;  // no further connections for this panel

        for(int k=0; k<3; k++) ni[k] = p3i.nodeIndex(k);

        for(int it1=i1; it1<i1+n1; it1++)
        {
            if(it0==it1) continue; // do not connect the triangle to itself

            Panel3 &p3j = m_Panel3[it1];
            if(p3j.neighbourCount()==3) continue;  // no further connections for this panel

            for(int k=0; k<3; k++) nj[k] = p3j.nodeIndex(k);

            // connect the two panels if they share two vertices
            bConnected = false;
            for(int k=0; k<3; k++)
            {
                for(int l=0; l<3; l++)
                {
                    if(ni[k]==nj[l])
                    {
                        // we have one common vertex
                        // check for another
                        for(int kk=0; kk<3; kk++)
                        {
                            for(int ll=0; ll<3; ll++)
                            {
                                if(kk!=k && ll!=l && ni[kk]==nj[ll])
                                {
                                    // Success, two common vertices
                                    // process p3i first
                                    // find the edge
                                    if (k==0)
                                    {
                                        if     (kk==1) iEdge=2;
                                        else if(kk==2) iEdge=1;
                                    }
                                    else if(k==1)
                                    {
                                        if     (kk==0) iEdge=2;
                                        else if(kk==2) iEdge=0;
                                    }
                                    else if(k==2)
                                    {
                                        if     (kk==0) iEdge=1;
                                        else if(kk==1) iEdge=0;
                                    }
                                    p3i.setNeighbour(it1, iEdge);

                                    // process p3k first
                                    // find the edge

                                    if (l==0)
                                    {
                                        if     (ll==1) iEdge=2;
                                        else if(ll==2) iEdge=1;
                                    }
                                    else if(l==1)
                                    {
                                        if     (ll==0) iEdge=2;
                                        else if(ll==2) iEdge=0;
                                    }
                                    else if(l==2)
                                    {
                                        if     (ll==0) iEdge=1;
                                        else if(ll==1) iEdge=0;
                                    }
                                    p3j.setNeighbour(it0, iEdge);

                                    bConnected=true;
                                }
                            }
                            if(bConnected) break; // there can be only one, so move on to the next panel
                        }
                        if(bConnected) break; // there can be only one, so move on to the next panel
                    }
                    if(bConnected) break; // there can be only one, so move on to the next panel
                }
                if(bConnected) break; // there can be only one, so move on to the next panel
            }
        }
    }
}


void TriMesh::connectNodes()
{
    for(int in=0; in<nNodes(); in++)
    {
        Node& nd = m_Node[in];
        nd.clearNeighbourNodes();
        for(int i3=0; i3<nPanels(); i3++)
        {
            Panel3 const &p3 = panel(i3);
            if(p3.hasVertex(in))
            {
                if(p3.isTrailingEdgeNode(in))
                {
                    // connect only if triangle is on same side as node
                    if(p3.surfacePosition()!=nd.surfacePosition())  continue;
                }
                // add all the panel's vertices as neighbours.
                nd.addTriangleIndex(p3.index());
                nd.addNeighbourIndex(p3.nodeIndex(0));
                nd.addNeighbourIndex(p3.nodeIndex(1));
                nd.addNeighbourIndex(p3.nodeIndex(2));
            }
        }
        if(nd.neighbourNodeCount()==0)
        {
            // hanging node
            // leave in the array so as to not mess up the indexes
            // but set as NOSURFACE so that it is discarded in the analysis and the display
            nd.setSurfacePosition(xfl::NOSURFACE);
        }
    }
}


void TriMesh::cleanMesh(std::string &logmsg)
{
    // clean double triangles
    // clean coincident nodes
    std::string prefix;

    cleanDoubleNodes(m_Panel3, m_Node, s_NodeMergeDistance, logmsg, prefix);
    // clean small triangles: <min vertex angle
    // clean small triangles: <min area
    // clean small triangles: < min edge length
}


int TriMesh::cleanNullTriangles()
{
    int nRemoved = 0;
    for(int i3=nPanels()-1; i3>=0; i3--)
    {
        if(m_Panel3.at(i3).isNullTriangle())
        {
            m_Panel3.erase(m_Panel3.begin()+i3);
            nRemoved++;
        }
    }
    return nRemoved;
}


int TriMesh::cleanDoubleNodes(double precision, std::string &logmsg, std::string const &prefx)
{
    return cleanDoubleNodes(m_Panel3, m_Node, precision, logmsg, prefx);
}


int TriMesh::cleanDoubleNodes(std::vector<Panel3> &panel3, std::vector<Node> &nodes,
                              double precision, std::string &logmsg, std::string const &prefx)
{
    QString log;
    QString prefix = QString::fromStdString(prefx);

    std::vector<int> doublenodes;
    std::vector<int> modpanels;
    for(uint in0=0; in0<nodes.size(); in0++)
    {
        for(int in1=in0+1; in1<int(nodes.size()); in1++)
        {
            if(nodes.at(in1).isSame(nodes.at(in0), precision))
            {
                // make a middle node
                nodes[in1].setPosition(nodes.at(in0));
                // re-index the triangles with node in1
                for(int i3=0; i3<int(panel3.size()); i3++)
                {
                    Panel3 &p3 = panel3[i3];
                    if(p3.nodeIndex(0)==in1)
                    {
                        p3.setVertex(0, nodes[in0]);
                        if(std::find(modpanels.begin(), modpanels.end(), p3.index()) == modpanels.end())
                            modpanels.push_back(i3);
                    }
                    if(p3.nodeIndex(1)==in1)
                    {
                        p3.setVertex(1, nodes[in0]);
                        if(std::find(modpanels.begin(), modpanels.end(), p3.index()) == modpanels.end())
                            modpanels.push_back(i3);
                    }
                    if(p3.nodeIndex(2)==in1)
                    {
                        p3.setVertex(2, nodes[in0]);
                        if(std::find(modpanels.begin(), modpanels.end(), p3.index()) == modpanels.end())
                            modpanels.push_back(i3);
                    }
                }
                // add in1 to the list of nodes to remove
                if((std::find(doublenodes.begin(), doublenodes.end(), in1) == doublenodes.end()))
                    doublenodes.push_back(in1);
            }
        }
    }

    std::sort(doublenodes.begin(), doublenodes.end());
    // remove the duplicate nodes
    for(int in=int(doublenodes.size())-1; in>=0; in--)
    {
        nodes.erase(nodes.begin() + doublenodes.at(in));
    }

    log = prefix + QString::asprintf("Removed %d nodes\n", int(doublenodes.size()));
    log += prefix + QString::asprintf("New node count is %d \n", int(nodes.size()));

    if(modpanels.size())
    {
        for(int i3=int(modpanels.size())-1; i3>=0; i3--)
        {
            Panel3 &p3 = panel3[modpanels.at(i3)];
            p3.setFrame();
            if(p3.isNullTriangle())
            {
                panel3.erase(panel3.begin()+modpanels.at(i3));
                log += prefix + QString::asprintf("removing null panel %d\n", p3.index());
            }
            else
            {
                log += prefix + QString::asprintf("modifying panel %d\n", p3.index());
            }
        }
        log += prefix + QString::asprintf("New panel count is %d\n", int(panel3.size()));
    }
    log += "\n";

    logmsg = log.toStdString();
    return int(doublenodes.size());
}


int TriMesh::mergeFuseToWingNodes(double precision, std::string &logmsg, const std::string &prefx)
{
    QString prefix = QString::fromStdString(prefx);
    QString logg;
    std::vector<int> doublenodes;
    std::vector<int> modpanels;
    for(int in0=0; in0<int(m_Node.size()); in0++)
    {
        if(m_Node.at(in0).isFuseNode())
        {
            for(uint in1=0; in1<m_Node.size(); in1++)
            {
                if(m_Node.at(in1).isWingNode())
                {
                    if(m_Node.at(in1).isSame(m_Node.at(in0), precision))
                    {
                        // move the fuse node and merge it with the wing node
                        m_Node[in0].setPosition(m_Node.at(in1));

                        // re-index the triangles with node in0
                        for(int i3=0; i3<nPanels(); i3++)
                        {
                            Panel3 &p3 = m_Panel3[i3];
                            if(p3.nodeIndex(0)==in0)
                            {
                                p3.setVertex(0, m_Node[in1]);
                                if(std::find(modpanels.begin(), modpanels.end(), p3.index()) == modpanels.end())
                                    modpanels.push_back(i3);
                            }
                            if(p3.nodeIndex(1)==in0)
                            {
                                p3.setVertex(1, m_Node[in1]);
                                if(std::find(modpanels.begin(), modpanels.end(), p3.index()) == modpanels.end())
                                    modpanels.push_back(i3);
                            }
                            if(p3.nodeIndex(2)==in0)
                            {
                                p3.setVertex(2, m_Node[in1]);
                                if(std::find(modpanels.begin(), modpanels.end(), p3.index()) == modpanels.end())
                                    modpanels.push_back(i3);
                            }
                        }
                        // add in0 to the list of nodes to remove
                        if(std::find(doublenodes.begin(), doublenodes.end(), in0) == doublenodes.end())
                            doublenodes.push_back(in0);
                    }
                }
            }
        }
    }

    std::sort(doublenodes.begin(), doublenodes.end());
    // remove the duplicate nodes
    for(int in=int(doublenodes.size())-1; in>=0; in--)
    {
        m_Node.erase(m_Node.begin() + doublenodes.at(in));
    }

    logg = prefix + QString::asprintf("Removed %d nodes\n", int(doublenodes.size()));
    logg += prefix + QString::asprintf("New node count is %d \n", int(m_Node.size()));

    if(modpanels.size())
    {
        for(int i3=int(modpanels.size())-1; i3>=0; i3--)
        {
            Panel3 &p3 = m_Panel3[modpanels.at(i3)];
            p3.setFrame();
            if(p3.isNullTriangle())
            {
                m_Panel3.erase(m_Panel3.begin()+modpanels.at(i3));
                logg += prefix + QString::asprintf("removing null panel %d\n", p3.index());
            }
            else
            {
                logg += prefix + QString::asprintf("modifying panel %d\n", p3.index());
            }
        }
        logg += prefix + QString::asprintf("New panel count is %d\n", nPanels());
    }
    logg += "\n";

    logmsg = logg.toStdString();
    return int(doublenodes.size());
}


void TriMesh::scale(double sx, double sy, double sz)
{
    for(int in=0; in<nodeCount(); in++)
    {
        m_Node[in].x *= sx;
        m_Node[in].y *= sy;
        m_Node[in].z *= sz;
    }

    for(int i3=0; i3<nPanels(); i3++)
    {
        m_Panel3[i3].scalePanel(sx,sy,sz);
    }
}


void TriMesh::makeNodeValues(std::vector<Node> const & nodes, std::vector<Panel3> const &panels,
                             std::vector<double> const &VertexValues, std::vector<double> &NodeValues,
                             double &valmin, double &valmax,
                             double coef)
{
    NodeValues.resize(nodes.size());

    valmin =  1e10;
    valmax = -1e10;
//    bool bTrace = false;
    int nVal=0;
    for(uint in=0; in<nodes.size(); in++)
    {
        Node const &nd = nodes.at(in);
        //average the density over the node's panels
        double value = 0.0;
        nVal=0;

        for(int i=0; i<nd.triangleCount(); i++)
        {
            int i3 = nd.triangleIndex(i);
            Panel3 const &p3 = panels.at(i3);
            int vertexindex = p3.vertexIndex(nd.index());
            if(vertexindex>=0 && 3*i3+vertexindex<int(VertexValues.size()))
            {
//                if(bTrace) qDebug(" node %3d    %13g", nd.index(), VertexValues[3*i3+vertexindex]);
                value += VertexValues[3*i3+vertexindex];
                nVal++;
            }
        }
//        assert(nVal==nd.triangleCount()); // don't: some nodes have been left dangling or with incorrect connections

        if(nd.triangleCount()>0)
        {
            value /= nd.triangleCount();
            NodeValues[in] = value * coef;
        }
        else
        {
            NodeValues[in] = 0;
        }
        valmin = std::min(valmin, NodeValues[in]);
        valmax = std::max(valmax, NodeValues[in]);

    }
    (void)nVal;
}


void TriMesh::makeWakePanels(int nxWakePanels, double wakepanelfactor, double TotalWakeLength, Vector3d const &WindDir, bool bAlignWakeTE)
{
    makeWakePanels(m_Panel3, nxWakePanels, wakepanelfactor, TotalWakeLength, WindDir,
                   m_WakePanel3, m_nWakeColumns, bAlignWakeTE);

//    connectWakePanels();
}


/**
 * @param alpha the aoa - non zero only for the streamlines calculation
 * @param beta
 */
int TriMesh::makeWakePanels(std::vector<Panel3> &Panel3List,
                            int nxWakePanel4, double wakepanelfactor, double TotalWakeLength,
                            Vector3d const &winddir,
                            std::vector<Panel3>&WakePanel3, int &nWakeColumn, bool bAlignWakeTE)
{
    // The nodes and basis functions are numbered such that:
    //   - for a left  panel, nodes 0&2 map to the left  node of the quad panel, and node 1 maps to the right node
    //   - for a right panel, nodes 0&1 map to the right node of the quad panel, and node 2 maps to the left  node
    //
    //     left wing                 right wing
    //   2           1              2          1
    //   _____________             _____________
    //   | up       /|1           2|\    up    |
    //   | left   /  |             |  \  right |
    //   |      /    |             |    \      |
    //   |    /      |             |      \    |
    //   |  /  right |             | left   \  |
    //   |/    down  |             | down     \| 0
    // 0 _____________             _____________
    //   2           0             0           1
    //
    // Construction method to align the wake panels with the wind direction (0,beta):
    //  (i)   rotate the TE nodes by angle beta
    //  (ii)  make the panels so that all FF nodes are in the same plane
    //  (iii) rotate wake panels by angle -beta


    WakePanel3.clear();

    nWakeColumn = 0;

    double series=0.0, r=1.0;
    for(int p=0; p<nxWakePanel4; p++)
    {
        series +=r;
        r*=wakepanelfactor;
    }

    Vector3d Tl, Tr, Tl1, Tr1;
    int mw=0;
    double l0l(0), l0r(0);
    for(int i3=0; i3<int(Panel3List.size()); i3++)
    {
        Panel3 &p3 = Panel3List[i3];

        if(p3.isTrailing())
        {
            if(p3.isBotPanel() || p3.isMidPanel())
            {
                p3.setiWake(mw);
                p3.setWakeColumn(nWakeColumn);

                //start at the trailing edge
                Tl = p3.leftTrailingNode();
                Tr = p3.rightTrailingNode();

                // step (i)
                if(bAlignWakeTE)
                {
                    l0l = (TotalWakeLength-Tl.x)/series;
                    l0r = (TotalWakeLength-Tr.x)/series;
                }
                else
                {
                    l0l = TotalWakeLength/series;
                    l0r = TotalWakeLength/series;
                }

                for(int nx=0; nx<nxWakePanel4; nx++)
                {
                    WakePanel3.push_back({});
                    WakePanel3.push_back({});
                    Tl1 = Tl + winddir * l0l;
                    Tr1 = Tr + winddir * l0r;
                    l0l *= wakepanelfactor;
                    l0r *= wakepanelfactor;
                    Panel3* p3wU = WakePanel3.data()+mw;
                    Panel3* p3wD = WakePanel3.data()+mw+1;

                    Vector3d Bl, Br, Bl1, Br1;
                    Bl  = Tl;
                    Br  = Tr;
                    Bl1 = Tl1;
                    Br1 = Tr1;

                    if(p3.isLeftWingPanel())
                    {
                        //     left wing
                        //   2           1
                        //   _____________
                        //   | up       /|1
                        //   | left   /  |
                        //   |      /    |
                        //   |    /      |
                        //   |  /  right |
                        //   |/    down  |
                        // 0 _____________
                        //   2           0
                        p3wU->setFrame(Bl1, Br, Bl);
                        p3wU->setIndex(mw);
                        p3wU->setLeftWingPanel(true);
                        p3wU->setLeftSidePanel(true);
                        p3wU->setAsWakePanel();
                        p3wU->m_iPU = nx>0? mw-1 : -1;
                        p3wU->m_iPD = mw+1;

                        p3wD->setFrame(Br1, Br, Bl1);
                        p3wD->setIndex(mw+1);
                        p3wD->setLeftWingPanel(true);
                        p3wD->setLeftSidePanel(false);
                        p3wD->setAsWakePanel();
                        p3wD->m_iPU = mw;
                        p3wD->m_iPD = nx<nxWakePanel4-1? mw+2 : -1;
                    }
                    else
                    {
                        //     right wing
                        //     2          1
                        //     _____________
                        //    2|\    up    |
                        //     |  \  right |
                        //     |    \      |
                        //     |      \    |
                        //     | left   \  |
                        //     | down     \| 0
                        //     _____________
                        //     0           1

                        p3wU->setFrame(Br1, Br, Bl);
                        p3wU->setIndex(mw);
                        p3wU->setLeftWingPanel(false);
                        p3wU->setLeftSidePanel(false);
                        p3wU->setAsWakePanel();
                        p3wU->m_iPU = nx>0? mw-1 : -1;
                        p3wU->m_iPD = mw+1;

                        p3wD->setFrame(Bl1, Br1, Bl);
                        p3wD->setIndex(mw+1);
                        p3wD->setLeftWingPanel(false);
                        p3wD->setLeftSidePanel(true);
                        p3wD->setAsWakePanel();
                        p3wD->m_iPU = mw;
                        p3wD->m_iPD = nx<nxWakePanel4-1? mw+2 : -1;
                    }

                    Tl = Tl1;
                    Tr = Tr1;
                    mw += 2;
                }
                nWakeColumn++;
            }
        }
    }

    //connect the top trailing panels to the wake columns

    int mwBot=0;
    int iWakeColumn=0;
    for(int i3=0; i3<int(Panel3List.size()); i3++)
    {
        Panel3 &p3 = Panel3List[i3];

        if(p3.isTrailing())
        {
            if(p3.isBotPanel())
            {
                mwBot = p3.iWake();
                iWakeColumn = p3.iWakeColumn();
                assert(p3.oppositeIndex()>=0  && p3.oppositeIndex()<int(Panel3List.size()));
                Panel3 &p3opposite = Panel3List[p3.oppositeIndex()];
                p3opposite.setiWake(mwBot); //same wake index as last trailing bot panel
                p3opposite.setWakeColumn(iWakeColumn);
            }
/*            else if(p3.isTopPanel())
            {
                p3.m_iWake = mwBot; //same wake index as last trailing bot panel
                p3.m_iWakeColumn = iWakeColumn;
            }*/
        }
    }
    return mw;
}


void TriMesh::connectWakePanels()
{
    double const MAXDISTANCE = 1.e-6; // =1µm; consider that nodes node closer than this length are identical

    // set panel neighbours
    for(int it0=0; it0<nWakePanels(); it0++)
    {
        Panel3 &p3 = m_WakePanel3[it0];
        p3.clearConnections();
        // search starting from the base panel's index
        // since neighbours are likely to have a close index

        int it2m = it0;
        int it2p = it0;

        do
        {
            it2m--;
            it2p++;

            if(it2p<nWakePanels())
            {
                Panel3 const & p2 = wakePanelAt(it2p);

                for(int ie=0; ie<3; ie++)
                {
                    int nEdge = p3.edgeIndex(p2.edge(ie), MAXDISTANCE);
                    if(nEdge>=0)
                    {
                        p3.setNeighbour(it2p, nEdge);
                        break;
                    }
                }
            }


            if(it2m>=0)
            {
                Panel3 const & p2 = wakePanelAt(it2m);
                for(int ie=0; ie<3; ie++)
                {
                    int nEdge = p3.edgeIndex(p2.edge(ie), MAXDISTANCE);
                    if(nEdge>=0)
                    {
                        p3.setNeighbour(it2m, nEdge);
                        break;
                    }
                }
            }
        }
        while(p3.neighbourCount()<3 && (it2p<nPanels() || it2m>=0));
    }
}


bool TriMesh::setPanel(int index, Panel3 const &p3)
{
    if(index<0 || index>nPanels()) return false;
    m_Panel3[index] = p3;
    return true;
}


void TriMesh::rebuildPanelsFromNodes(std::vector<Panel3> &panel3, std::vector<Node> const &node)
{
    for(int i3=0; i3<int(panel3.size()); i3++)
    {
        Panel3 &p3 = panel3[i3];
        p3.setFrame(node[p3.nodeIndex(0)], node[p3.nodeIndex(1)], node[p3.nodeIndex(2)]);
    }
}


void TriMesh::rebuildPanels()
{
    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 &p3 = m_Panel3[i3];
        p3.setFrame(m_Node[p3.nodeIndex(0)], m_Node[p3.nodeIndex(1)], m_Node[p3.nodeIndex(2)]);
    }
}


/**
 * Converts the triangulation to a triangular mesh*/
void TriMesh::makeMeshFromTriangles(std::vector<Triangle3d> const &triangulation, int firstindex, xfl::enumSurfacePosition pos,
                                    std::string &logmsg, std::string const &prefix)
{
    QString strong;
    clearMesh();

    bool bDiscard = false;
    int nDiscard = 0;

    m_Panel3.reserve(triangulation.size()); // may be a bit less if triangles are discarded

    for(uint it=0; it<triangulation.size(); it++)
    {
        Triangle3d t3 = triangulation.at(it);
        Node &S0 = t3.vertex(0);
        Node &S1 = t3.vertex(1);
        Node &S2 = t3.vertex(2);
        // force nodes in the xz symmetry plane to ensure that left and right panels are connected
        if(fabs(S0.y)<SYMMETRYPRECISION)
        {
            S0.y=0.0;
            t3.setTriangle();
        }
        if(fabs(S1.y)<SYMMETRYPRECISION)
        {
            S1.y=0.0;
            t3.setTriangle();
        }
        if(fabs(S2.y)<SYMMETRYPRECISION)
        {
            S2.y=0.0;
            t3.setTriangle();
        }
        bDiscard = false;
        if(S0.isSame(S1, XflMesh::nodeMergeDistance()))
            bDiscard = true; // null triangle
        if(S1.isSame(S2, XflMesh::nodeMergeDistance()))
            bDiscard = true; // null triangle
        if(S2.isSame(S0, XflMesh::nodeMergeDistance()))
            bDiscard = true; // null triangle

        if(t3.isNull())
            bDiscard = true;
        if(t3.area()<MINREFAREA)
            bDiscard = true;

        if(bDiscard)
        {
            nDiscard++;
            continue;
        }

        m_Panel3.push_back({t3, pos});
        m_Panel3.back().setIndex(firstindex + nPanels()-1);
    }

    strong = QString::asprintf("Discarded %d null triangles\n", nDiscard);
    logmsg += prefix+strong.toStdString();

    strong = QString::asprintf("Converted %d triangles to %d panels\n", int(triangulation.size()), nPanels());
    logmsg += prefix+strong.toStdString();

    makeNodeArrayFromPanels(0, logmsg, prefix);
}


/**
 * Assumes connections have been made
 * Sets the node normals
 */
void TriMesh::makeNodeNormals(bool bReversed)
{
//    auto t0 = std::chrono::high_resolution_clock::now();
    for(uint iNode=0; iNode<m_Node.size(); iNode++)
    {
        Node &node = m_Node[iNode];
        Vector3d normal(0.0,0.0,0.0);
        //what  is an average of 3d vectors???
        for(int it=0; it<node.triangleCount(); it++)
        {
            Panel3 const &p3 = m_Panel3.at(node.triangleIndex(it));
            normal += p3.normal();
        }
        double norm = normal.norm();
        if(fabs(norm)<LENGTHPRECISION)
        {
            // usually a flat surface, node normals cancel each other
            if(node.triangleCount()>0) normal.set(m_Panel3.at(node.triangleIndex(0)).normal());
            else
            {
                //error somewhere
                normal.set(0,0,1);
            }
            norm = normal.norm();
        }
        normal *= 1/norm;
        if(bReversed) normal.reverse();
        node.setNormal(normal);
    }

    //set the normals at the triangle nodes
    for(int it=0; it<nPanels(); it++)
    {
        Panel3 &p3 = m_Panel3[it];
        for(int iv=0; iv<3; iv++)
        {
            int iNode = p3.nodeIndex(iv);
            if(iNode>=0 && iNode<int(m_Node.size()))
                p3.setNodeNormal(iv, m_Node.at(iNode).normal());
        }
    }
/*    auto t1 = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    qDebug("TriMesh::makeNodeNormals: %gms", double(duration)/1000.0);*/
}


/**
 * Moves the first node to the second location, replaces indexes in the triangles,
 * and finally deletes the node.
 * Assumes that the connections have been made.
 */
bool TriMesh::mergeNodes(int srcindex, int destindex, bool bDiscardNullPanels, std::string &logmsg, std::string prefx)
{
    QString prefix = QString::fromStdString(prefx);
    QString strange,logg;
    if(srcindex<0  || srcindex>=nNodes())
    {
        strange = QString::asprintf("Invalid source index %d\n", srcindex);
        logg = prefix + strange +prefix+"cancelling move operation\n\n";
        return false;
    }
    if(destindex<0 || destindex>=nNodes())
    {
        strange = QString::asprintf("Invalid destination index %d\n", destindex);
        logg = prefix + strange +prefix+"cancelling move operation\n\n";
        return false;
    }

    if(srcindex==destindex)
    {
        strange = QString::asprintf("Source and destination indexes are identical %d\n", srcindex);
        logg = prefix + strange +prefix+" cancelling move operation\n\n";
        return false;
    }

    Node const &src  = m_Node.at(srcindex);
    Node const &dest = m_Node.at(destindex);
    // Merge only if the two nodes are part of an existing triangle
    bool bCommon = false;
    for(int is=0; is<src.triangleCount(); is++)
    {
        int idx0 = src.triangleIndex(is);
        for(int id=0; id<dest.triangleCount(); id++)
        {
            if(idx0==dest.triangleIndex(id))
            {
                bCommon = true;
                break;
            }
        }
        if(bCommon) break;
    }
    if (!bCommon)
    {
        logg = prefix + "The nodes do not form an edge of an existing triangle\n"+prefix+"cancelling move operation\n\n";
        return false;
    }

    std::vector<int> modpanels;
    for(int i3=0; i3<src.triangleCount(); i3++)
    {
        Panel3 &p3 = m_Panel3[src.triangleIndex(i3)];
        for(int in=0; in<3; in++)
        {
            if(p3.nodeIndex(in)==srcindex)
            {
                p3.setNode(in, dest);
                p3.setFrame();
                modpanels.push_back(p3.index());
                break;
            }
        }
    }
    std::sort(modpanels.begin(), modpanels.end());

    logg += prefix + "Modifying triangles ";
    for(int it=int(modpanels.size())-1; it>=0; it--)
    {
        strange = QString::asprintf(" %d", modpanels.at(it));
        logg += strange;
    }
    logg += "\n";

    if(bDiscardNullPanels)
    {
        // this breaks node connections
        for(int it=int(modpanels.size())-1; it>=0; it--)
        {
            if(modpanels.at(it)<nPanels())
            {
                Panel3 &p3 = m_Panel3[modpanels.at(it)];
                if(p3.isNullTriangle())
                {
                    m_Panel3.erase(m_Panel3.begin()+modpanels.at(it));
                    strange = QString::asprintf("Discarding null triangle %d", modpanels.at(it));
                    logg += prefix + strange + "\n";
                }
            }
        }
    }

    logmsg = logg.toStdString();

    return true;
}


/** assumes that the connections have been made */
void TriMesh::getFreeEdges(std::vector<Segment3d> &freeedges) const
{
    freeedges.clear();
    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);
        for(int i=0; i<3; i++)
        {
//            if(p3.neighbourEdgeIndex(0)!=i && p3.neighbourEdgeIndex(1)!=i && p3.neighbourEdgeIndex(2)!=i)
            if(p3.neighbour(i)<0)
            {
                // this edge is a free edge
                Segment3d const &seg = p3.edge(i);
                freeedges.push_back(seg);
            }
        }
    }
}


void TriMesh::getLastTrailingPoint(Vector3d &pt) const
{
    pt.set(-100,0,0);
    for(uint in=0; in<m_Node.size(); in++)
    {
        if(m_Node.at(in).x>pt.x) pt.set(m_Node.at(in));
    }
}


void TriMesh::serializePanelsFl5(QDataStream &ar, bool bIsStoring)
{
    if(bIsStoring) savePanels(ar);
    else           loadPanels(ar);
}


void TriMesh::savePanels(QDataStream &ar)
{
    int nIntSpares=0;
    int nDbleSpares=0;
    int n=0;
    double dble=0.0;

    // 500001: new v7 format
    int ArchiveFormat = 500001;

    ar << ArchiveFormat;
    ar << panelCount();

    for(int i3=0; i3<panelCount(); i3++)
    {
        Panel3 const &p3 = panel(i3);
        for(int in=0; in<3; in++)
        {
            ar << p3.node(in).xf() << p3.node(in).yf() << p3.node(in).zf();
        }
        ar << p3.isPositiveOrientation();
    }

    // dynamic space allocation for the future storage of more data, without need to change the format
    nIntSpares=0;
    ar << nIntSpares;
    n=0;
    for (int i=0; i<nIntSpares; i++) ar << n;
    nDbleSpares=0;
    ar << nDbleSpares;
    for (int i=0; i<nDbleSpares; i++) ar << dble;
}


void TriMesh::loadPanels(QDataStream &ar)
{
    int nIntSpares=0;
    int nDbleSpares=0;
    int n=0;
    double dble=0.0;
    int ArchiveFormat=0;// identifies the format of the file
    bool bPositiveOrientation=false;
    int n3=0;
    float f0=0,f1=0,f2=0;
    Vector3d S[3];
    ar >> ArchiveFormat;
    ar >> n3;

    clearMesh();
    for(int i3=0; i3<n3; i3++)
    {
        for(int in=0; in<3; in++)
        {
            ar >> f0 >> f1 >> f2;
            S[in].set(double(f0), double(f1), double(f2));
        }
        ar >> bPositiveOrientation;
        addPanel({S[0], S[1], S[2]});
        lastPanel().setSurfacePosition(xfl::FUSESURFACE);
    }

    // space allocation
    ar >> nIntSpares;
    for (int i=0; i<nIntSpares; i++) ar >> n;
    ar >> nDbleSpares;
    for (int i=0; i<nDbleSpares; i++) ar >> dble;

    std::string logmsg;

//    auto t0 = std::chrono::high_resolution_clock::now();

    makeNodeArrayFromPanels(0, logmsg, "   "); //node and panel indexes are set later at plane assembly time
/*    auto t1 = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    qDebug("TriMesh::loadTriMesh::makeNodeArrayFromPanels: %gms", double(duration)/1000.0);*/
}


/** serializes the triangles, the nodes, and the connections */
void TriMesh::serializeMeshFl5(QDataStream &ar, bool bIsStoring)
{
    if(bIsStoring) saveMesh(ar);
    else           loadMesh(ar);
}


void TriMesh::saveMesh(QDataStream &ar)
{
    // 500001: new fl5 format
    // 500002: addded TE opposite indexes
    int ArchiveFormat = 500002;


    ar << ArchiveFormat;

    ar << int(m_Node.size());
    for(int in=0; in<nNodes(); in++)
    {
        Node const &nd = m_Node.at(in);
        ar << nd.index();

        ar << nd.xf() << nd.yf() << nd.zf();
        ar << nd.normal().xf() << nd.normal().yf() << nd.normal().zf();
        ar << nd.isTrailing();      
        ar << int(nd.neighbourNodeCount());
        for(int ine=0; ine<nd.neighbourNodeCount(); ine++)
            ar << nd.nodeNeighbourIndex(ine);
        ar << int(nd.triangleCount());
        for(int ine=0; ine<nd.triangleCount(); ine++)
            ar << nd.triangleIndex(ine);
    }

    ar << panelCount();
    for(int i3=0; i3<panelCount(); i3++)
    {
        Panel3 const &p3 = panel(i3);
        ar << p3.index();
        ar << p3.nodeIndex(0) << p3.nodeIndex(1) << p3.nodeIndex(2);

        ar << p3.neighbour(0) << p3.neighbour(1) << p3.neighbour(2);
        ar << p3.isPositiveOrientation();
        ar << p3.isTrailing();
        ar << p3.oppositeIndex();
        switch(p3.surfacePosition())
        {
            case xfl::BOTSURFACE:   ar<<0;     break;
            case xfl::MIDSURFACE:   ar<<1;     break;
            case xfl::TOPSURFACE:   ar<<2;     break;
            case xfl::SIDESURFACE:  ar<<3;     break;
            case xfl::FUSESURFACE:  ar<<4;     break;
            case xfl::WAKESURFACE:  ar<<5;     break;
            case xfl::NOSURFACE:    ar<<6;     break;
        }
    }
}


void TriMesh::loadMesh(QDataStream &ar)
{
    bool boolean(false);
    int n(0), ne(0), k(0);
    int ArchiveFormat(0);// identifies the format of the file
    int n3(0);
    int i0(0), i1(0), i2(0);
    float f0(0),f1(0),f2(0);

    clearMesh();

    ar >> ArchiveFormat;

    ar >>n;
    m_Node.resize(n);
    for(int in=0; in<n; in++)
    {
        Node &nd = m_Node[in];
        ar >> k;                     nd.setIndex(k);
        ar >> f0 >> f1 >> f2;        nd.set(double(f0), double(f1), double(f2));
        ar >> f0 >> f1 >> f2;        nd.setNormal(double(f0), double(f1), double(f2));
        ar >> boolean;               nd.setTrailing(boolean);

        ar >> ne;
        nd.resizeNodeNeighbours(ne);
        for(int j=0; j<ne; j++)
        {
            ar >> k;                 nd.setNodeNeighbourIndex(j,k);
        }

        ar >> ne;
        nd.resizeTriangles(ne);
        for(int j=0; j<ne; j++)
        {
            ar >> k;                 nd.setTriangleIndex(j,k);
        }
    }

    ar >> n3;
    m_Panel3.resize(n3);
    for(int i3=0; i3<n3; i3++)
    {
        Panel3 &p3 = m_Panel3[i3];
        ar >> k;  p3.setIndex(k);
        ar >> i0 >> i1 >> i2;
        p3.setVertex(0, m_Node.at(i0));
        p3.setVertex(1, m_Node.at(i1));
        p3.setVertex(2, m_Node.at(i2));

        ar >> i0 >> i1 >> i2;
        p3.setNeighbour(0, i0);
        p3.setNeighbour(1, i1);
        p3.setNeighbour(2, i2);

        ar >> boolean; // p3.m_bPositiveOrientation;
        ar >> p3.m_bIsTrailing;
        if(ArchiveFormat>=500002)
        {
            ar >> k;
            p3.setOppositeIndex(k);
        }

        ar >> k;
        switch(k)
        {
            case 0:   p3.setSurfacePosition(xfl::BOTSURFACE);   break;
            case 1:   p3.setSurfacePosition(xfl::MIDSURFACE);   break;
            case 2:   p3.setSurfacePosition(xfl::TOPSURFACE);   break;
            case 3:   p3.setSurfacePosition(xfl::SIDESURFACE);  break;
            case 4:   p3.setSurfacePosition(xfl::FUSESURFACE);  break;
            case 5:   p3.setSurfacePosition(xfl::WAKESURFACE);  break;
            default:
            case 6:   p3.setSurfacePosition(xfl::NOSURFACE);    break;
        }

        p3.setFrame();
    }
    if(ArchiveFormat<500002)
    {
        // cleaning up past errors
        std::vector<int> errorlist;
        connectTrailingEdges(errorlist);
    }
}


/** STL type mesh only; assumes connections have been made */
void TriMesh::checkTrailingEdges(std::vector<int> &errorlist)
{
    errorlist.clear();
    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 &p3 = m_Panel3[i3];
        if(p3.isTrailing())
        {
            bool bChecked = false;
            for(int in=0; in<3; in++)
            {
                if(p3.neighbour(in)>=0 && p3.neighbour(in)<nPanels())
                {
                    Panel3 &p3n = m_Panel3[p3.neighbour(in)];
                    if(p3n.isTrailing())
                    {
                        if ((p3.isTopPanel() && p3n.isBotPanel()) || (p3.isBotPanel() && p3n.isTopPanel()))
                        {
                            p3.setOppositeIndex(p3n.index());
                            p3n.setOppositeIndex(p3.index());
                            bChecked=true;
                            break;
                        }
                    }
                }
            }
            if(!bChecked) errorlist.push_back(p3.index());
        }
    }
}


/**
 * STL type mesh only; assumes TE top and bottom panels have been identified
 * Assigns opposite panel indexes to the top and bottom panels.
 * Reorders the vertices so that vertices 1 and 2 are trailing
*/
bool TriMesh::connectTrailingEdges(std::vector<int> &errorlist)
{
    errorlist.clear();
    double const MAXDISTANCE = 1.e-4; // =0.1mm; consider that nodes node closer than this length are identical

    // make top and bottom lists
    std::vector<int> toplist, botlist;
    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);
        if(p3.isTrailing())
        {
            if(p3.isTopPanel()) toplist.push_back(i3);
            if(p3.isBotPanel()) botlist.push_back(i3);
        }
    }
//    if(toplist.size()!=botlist.size()) return false;
//qDebug()<<"NTE ="<<toplist.size()<<botlist.size();
    for(int i=0; i<int(toplist.size()); i++)
    {
        Panel3 &p3t = m_Panel3[toplist.at(i)];

        bool bChecked = false;
        for(int j=0; j<int(botlist.size()); j++)
        {
            Panel3 &p3b = m_Panel3[botlist.at(j)];

            for(int ie=0; ie<3; ie++)
            {
                int nEdge = p3t.edgeIndex(p3b.edge(ie), MAXDISTANCE);
                if(nEdge>=0)
                {
                    // re-order the vertices of the top panel so that vertex 1 is left trailing and vertex 2 is right trailing
                    if(nEdge==0)
                    {
                        // All good
                    }
                    else if(nEdge==1)
                    {
                        //vertices 0 and 2 are trailing
                        Node tmp = p3t.vertexAt(1);
                        p3t.setVertex(1, p3t.vertexAt(2));
                        p3t.setVertex(2, p3t.vertexAt(0));
                        p3t.setVertex(0, tmp);
                        p3t.setFrame();
                    }
                    else if(nEdge==2)
                    {
                        //vertices 0 and 1 are trailing
                        Node tmp = p3t.vertexAt(2);
                        p3t.setVertex(2, p3t.vertexAt(1));
                        p3t.setVertex(1, p3t.vertexAt(0));
                        p3t.setVertex(0, tmp);

                        p3t.setFrame();
                    }
                    p3t.setOppositeIndex(p3b.index());

                    bChecked = true;
                    break;
                }
            }
            if(bChecked) break;
        }
        if(!bChecked) errorlist.push_back(p3t.index());
    }

    for(int i=0; i<int(botlist.size()); i++)
    {
        Panel3 &p3bot = m_Panel3[botlist.at(i)];
        bool bChecked = false;
        for(int j=0; j<int(toplist.size()); j++)
        {
            Panel3 &p3top = m_Panel3[toplist.at(j)];

            for(int ie=0; ie<3; ie++)
            {
                int nEdge = p3bot.edgeIndex(p3top.edge(ie), MAXDISTANCE);
                if(nEdge>=0)
                {
                    // re-order the vertices of the bot panel so that vertex 2 is left trailing and vertex 1 is right trailing
                    if(nEdge==0)
                    {
                        // All good
                    }
                    else if(nEdge==1)
                    {
                        //vertices 0 and 2 are trailing
                        Node tmp = p3bot.vertexAt(1);
                        p3bot.setVertex(1, p3bot.vertexAt(2));
                        p3bot.setVertex(2, p3bot.vertexAt(0));
                        p3bot.setVertex(0, tmp);
                        p3bot.setFrame();
                    }
                    else if(nEdge==2)
                    {
                        //vertices 0 and 1 are trailing
                        Node tmp = p3bot.vertexAt(2);
                        p3bot.setVertex(2, p3bot.vertexAt(1));
                        p3bot.setVertex(1, p3bot.vertexAt(0));
                        p3bot.setVertex(0, tmp);

                        p3bot.setFrame();
                    }
                    p3bot.setOppositeIndex(p3top.index());
                    bChecked = true;
                    break;
                }
            }
            if(bChecked) break;
        }
        if(!bChecked) errorlist.push_back(p3bot.index());
    }
    return errorlist.size()==0;
}


void TriMesh::addPanels(std::vector<Panel3>const &panel3list, const Vector3d &position)
{
    for(uint i3=0; i3<panel3list.size(); i3++)
    {
        m_Panel3.push_back(panel3list.at(i3));
        m_Panel3.back().translate(position.x, position.y, position.z);
    }
}


/** append, move to position, and shift indexes */
void TriMesh::appendMesh(TriMesh const &mesh)
{
    // append the nodes and shift their indexes
    int n0 = int(m_Node.size());
    m_Node.resize(n0+mesh.nNodes());

    for(uint in=0; in<mesh.m_Node.size(); in++)
    {
        Node &nd = m_Node[n0+in];
        nd = mesh.m_Node.at(in);
        nd.setIndex(n0+nd.index());
    }

    int np0 = nPanels();
    m_Panel3.resize(np0+mesh.nPanels());
    for(int i3=0; i3<mesh.nPanels(); i3++)
    {
        m_Panel3[np0+i3] = mesh.panelAt(i3);
        Panel3 &p3 = m_Panel3[np0+i3];
        p3.setNodeIndexes(p3.nodeIndex(0)+n0, p3.nodeIndex(1)+n0, p3.nodeIndex(2)+n0);
        for(int ine=0; ine<3; ine++)            if(p3.neighbour(ine)>=0) p3.m_Neighbour[ine] += np0; // only if not equal to -1
//            if(p3.neighbour(ine)>=0) p3.setNeighbour(ine, p3.neighbour(ine) + np0); // only if not equal to -1
    }
}


void TriMesh::addPanel(Panel3 const &p3)
{
    m_Panel3.push_back(p3);
    m_Panel3.back().setIndex(nPanels()-1);
}


void TriMesh::removePanelAt(int index)
{
    if(index>=0 && index<nPanels())
    {
        m_Panel3.erase(m_Panel3.begin()+index);
        //shift indexes
        for(int i=index; i<nPanels(); i++)
        {
            m_Panel3[i].setIndex(m_Panel3.at(i).index()-1);
        }
    }
}


void TriMesh::checkElementSize(double minsize, std::vector<int> &elements, std::vector<double> &size)
{
    elements.clear();
    size.clear();
    for(int i3=0; i3<nPanels(); i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);
        for(int iedge=0; iedge<3; iedge++)
        {
            double length = p3.edge(iedge).length();
            if(length<minsize*50.0)
            {
                elements.push_back(p3.index());
                size.push_back(length);
                break;
            }
        }
    }
}




