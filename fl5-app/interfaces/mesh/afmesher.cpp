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

/**
 * Advancing front mesher
 *
 * for each Shell
 *    for each Face of the shell
 *       make the array of pslg3d in g-space from the face's wires
 *       order the outer PSLG's segments and vertices so that they form a continuous ordered line
 *       proceed with the meshing front until it encounters an opposite front
*/




#include <QtGlobal> // need to include qglobal.h before testing OS macros

#ifdef Q_OS_LINUX
    #  include <unistd.h>
#endif
#ifdef Q_OS_WIN
  #include <windows.h>
//  #include <synchapi.h>
  #define sleep(s) Sleep(s)
#endif

#ifdef Q_OS_MAC
    #  include <unistd.h>
#endif

#include <algorithm>

#include <BOPTools_AlgoTools3D.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <Geom2d_Curve.hxx>
#include <GeomLProp_SLProps.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>



#include <QtConcurrent/QtConcurrent>

#include "afmesher.h"

#include <api/constants.h>
#include <api/occ_globals.h>
#include <api/pslg2d.h>
#include <api/sail.h>
#include <api/triangle3d.h>
#include <api/units.h>
#include <api/xflmesh.h>
#include <api/sail.h>

#include <interfaces/mesh/meshevent.h>
//#include <interfaces/mesh/slg3d.h>



SLG3d AFMesher::s_SLG;
std::vector<Triangle3d> AFMesher::s_Triangles;
QVector<Vector3d> AFMesher::s_DebugPts;
int AFMesher::s_TraceFaceIdx  = -1;
int AFMesher::s_MaxIterations = -1;

bool AFMesher::s_bCancel = false;
double AFMesher::s_SearchRadiusFactor = 0.7; // best by test (Fischer)
double AFMesher::s_GrowthFactor = 1.0;

double AFMesher::s_MaxEdgeLength = 0.25;
int AFMesher::s_MaxPanelCount = 1000;
bool AFMesher::s_bDelaunay = true;

int AFMesher::s_AnimationPause = 30;
bool AFMesher::s_bIsAnimating = false;


AFMesher::AFMesher() : QObject()
{
    m_pParent = nullptr;
    s_DebugPts.clear();
    m_pSail = nullptr;

    m_bSplittableInnerPSLG = false;
    m_bError = false;
}


void AFMesher::triangulateShells()
{
    m_bError = false;
    int iShell=0;
    m_Triangles.clear();

    for(TopTools_ListIteratorOfListOfShape ShellIt(m_Shapes); ShellIt.More(); ShellIt.Next())
    {
        std::vector<Triangle3d> shelltriangles;
        QString logmsg;

        try
        {
            TopoDS_Shell const &theShell = TopoDS::Shell(ShellIt.Value());
            logmsg = QString::asprintf("   Triangulating shell %d\n", iShell);
            postMessageEvent(logmsg);
            bool bRes = triangulateShell(theShell, shelltriangles, logmsg);
            if(!bRes) m_bError = true;
            m_Triangles.insert(m_Triangles.end(), shelltriangles.begin(), shelltriangles.end());
            postMessageEvent(logmsg);
            iShell++;
        }
        catch (Standard_TypeMismatch &)
        {
            logmsg  = QString::asprintf("   Error casting shape %d to a SHELL\n", iShell);
            postMessageEvent(logmsg);
            continue;
        }
    }
    emit meshFinished();

    thread()->exit(0); // exit event loop so that finished() is emitted
}


bool AFMesher::triangulateShell(TopoDS_Shell const &shell, std::vector<Triangle3d> &triangles, QString &logmsg)
{
    QString strange;

    s_Triangles.clear();
    s_SLG.clear();

    logmsg.clear();

    TopExp_Explorer ShellExplorer;
    int nFace=0;

    for(ShellExplorer.Init(shell,TopAbs_FACE); ShellExplorer.More(); ShellExplorer.Next())
    {
        nFace++;
    }
    m_FaceTriangles.resize(nFace);
    m_Segs.resize(nFace);
    for(int o=0; o<nFace; o++)
    {
        m_FaceTriangles[o].clear();
        m_Segs[o].clear();
    }

/*    if(s_bMultiThread && !s_bIsAnimating)
    {
        QFutureSynchronizer<void> futureSync;
        int iFace = 0;
        for(ShellExplorer.Init(shell, TopAbs_FACE); ShellExplorer.More(); ShellExplorer.Next())
        {
            if(s_TraceFaceIdx<0 || iFace==s_TraceFaceIdx)
            {
                TopoDS_Face const &aFace = TopoDS::Face(ShellExplorer.Current());
                futureSync.addFuture(QtConcurrent::run(this, &AFMesher::triangulateFace,
                                                       aFace, iFace));
            }
            iFace++;
        }
        futureSync.waitForFinished();
    }
    else*/
    {
        int iFace = 0;
        for(ShellExplorer.Init(shell, TopAbs_FACE); ShellExplorer.More(); ShellExplorer.Next())
        {
            if(s_TraceFaceIdx<0 || iFace==s_TraceFaceIdx)
            {
                TopoDS_Face const &aFace = TopoDS::Face(ShellExplorer.Current());
                strange.clear();
                if(!triangulateFace(aFace, iFace))
                {
                }
            }
            iFace++;
        }
    }

    s_SLG.clear();
    for(int iFace=0; iFace<nFace; iFace++)
    {
        triangles.insert(triangles.end(), m_FaceTriangles[iFace].begin(), m_FaceTriangles[iFace].end());
        s_SLG.append(m_Segs[iFace]);
    }
    s_Triangles = triangles;

    return true;
}


bool AFMesher::triangulateFace(TopoDS_Face const &aFace, int iFace)
{
    QString strange, logmsg;

    logmsg = QString::asprintf("      triangulating face %d\n", iFace);

    // need to build first the pslg in 2d p-space, because parametric coordinates
    // are needed to build the 3d normals at each node
    PSLG2d contourpslg2d;
    QVector<PSLG2d> innerpslg2d;
    makeFacePSLG2d(aFace, innerpslg2d, contourpslg2d, strange);

    logmsg += strange;

    if(contourpslg2d.size()>3000)
    {
        logmsg += "Excessive number of edge nodes - check element size vs. part length.\nAborting mesh.\n";
        return false;
    }

    // build the 3d pslg from the 2d pslg

    SLG3d contourslg;
    QVector<SLG3d> innerslg(innerpslg2d.size());

    // ensure that the pslg is ordered positively around the face normal
    if(aFace.Orientation()==TopAbs_REVERSED) contourpslg2d.reverse();
    makeFaceSLG3dFrom2d(aFace, contourpslg2d, contourslg, strange);

    for(int i=0; i<innerpslg2d.size(); i++)
    {
        // ensure that the pslg is ordered positively around the face normal
        if(aFace.Orientation()==TopAbs_REVERSED) innerpslg2d[i].reverse();
        makeFaceSLG3dFrom2d(aFace, innerpslg2d.at(i), innerslg[i], strange);
    }

    m_Segs[iFace] = contourslg;
    for(int i=0; i<innerslg.size(); i++)
        m_Segs[iFace].append(innerslg[i]);

    if(!makeTriangles(aFace, m_Segs[iFace], m_FaceTriangles[iFace], s_MaxEdgeLength, s_MaxPanelCount, strange))
    {
        logmsg += strange;
        postMessageEvent(logmsg);
        return false;
    }

    logmsg += QString::asprintf("         made %d triangles\n", int(m_FaceTriangles[iFace].size()));

    int nFaceFlips=0, nIter=0;
    if(s_bDelaunay)
    {
        makeDelaunayFlips(m_FaceTriangles[iFace], nFaceFlips, nIter);
        logmsg += QString::asprintf("         made %d Delaunay flips in %d iterations\n", nFaceFlips, nIter);
    }

    postMessageEvent(logmsg);
    return true;
}


void AFMesher::makeFaceSLG3dFrom2d(TopoDS_Face const &aFace, PSLG2d const &pslg2d, SLG3d &slg, QString &logmsg) const
{
    slg.resize(pslg2d.size());
    if(pslg2d.isEmpty()) return;

    double umin=0, umax=0, vmin=0, vmax=0;
    BRepTools::UVBounds(aFace,umin, umax, vmin, vmax);
    double du = umax-umin;
    double dv = vmax-vmin;

    Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace);

    // convert the pslg
    gp_Pnt Pt;
    gp_Dir Dir;
    Node nd[2];
    int iseg=0;
    double insidefrac = 0.05;
    try
    {
        for(auto it=pslg2d.begin(); it!=pslg2d.end(); it++)
        {
            Segment3d &seg3d = slg[iseg];
            for(int ivtx=0; ivtx<2; ivtx++)
            {
                double u = it->vertexAt(ivtx).x;
                double v = it->vertexAt(ivtx).y;

                GeomLProp_SLProps props(aSurface, u, v, 1, 1.e-6); // max 1 derivation
                Pt = props.Value();

                u = qMax(u, umin+insidefrac*du);    u=qMin(u, umax-insidefrac*du);  // it can occur that the normal is undefined on the edge itself
                v = qMax(v, vmin+insidefrac*dv);    v=qMin(v, vmax-insidefrac*dv);  // it can occur that the normal is undefined on the edge itself
                props.SetParameters(u,v);

                if(props.IsNormalDefined())
                {
                    Dir = props.Normal();
                }
                else
                {
                    QString strange;
                    strange = QString::asprintf("   SLG3dFrom2d:: surface normal on edge is undefined at location (%7g, %7g, %7g) ",
                                                Pt.X()*Units::mtoUnit(), Pt.Y()*Units::mtoUnit(), Pt.Z()*Units::mtoUnit());
                    strange += Units::lengthUnitQLabel() + "\n";
                    logmsg += strange;
                }
                nd[ivtx].set(Pt.X(), Pt.Y(), Pt.Z());
                nd[ivtx].setNormal(Dir.X(), Dir.Y(), Dir.Z());
            }
            seg3d.setNodes(nd);
            iseg++;
            if(s_bCancel) return;
        }
    }
    catch (Standard_Failure &failure)
    {
        logmsg += "   SLG3dFrom2d:: standard exception" + QString(failure.GetMessageString());
    }
    catch(...)
    {
        logmsg += "   SLG3dFrom2d:: other exception";
    }

    // clean small segments
    double minlength = XflMesh::nodeMergeDistance();
    for(int is=int(slg.size()-1); is>=1; is--)
    {
        if(slg.at(is).length()<minlength)
        {
            slg[is-1].setVertex(1, slg.at(is).vertexAt(1));
            slg.removeAt(is);
        }
        if(s_bCancel) return;
    }
    // beta 06
    if(slg.front().length()<minlength)
    {
        slg[0].setVertex(0, slg.front().vertexAt(0));
        slg.removeAt(0);
    }

    // for closed slgs - required condition for the advancing front method
    // merge nodes between adjacent segments
    for(uint is=1; is<slg.size(); is++)
    {
//        double dist = slg[is].vertex(0).distanceTo(slg.at(is-1).vertexAt(1));
//        if(dist>0 && dist<XflMesh::nodeMergeDistance())
        {
            slg[is].setNodes(slg.at(is-1).vertexAt(1), slg.at(is).vertexAt(1));
        }
    }
//    double dist = slg.front().vertex(0).distanceTo(slg.back().vertexAt(1));
//    if(dist>0 && dist<XflMesh::nodeMergeDistance())
    if(slg.size())
    {
        slg.front().setNodes(slg.back().vertexAt(1), slg.front().vertexAt(1));
    }
}


/**
 * Builds the Face's SLG3d in geom-space.
 * Only the outer SLG is split with the input length.
 * The inner pslg are not split to match the foil's vertices.
 * @returns the number of the PSLG segments
 * UNUSED
 */
void AFMesher::makeFaceSLG3d(const TopoDS_Face &aFace,
                             QVector<SLG3d> &innerslg, SLG3d &contourpslgUV,
                             QString &logmsg) const
{
    std::string strange;
    TopoDS_ListOfShape theinnerwires;
    TopoDS_Wire theouterwire;
    double curvemin=0, curvemax=0;

//    double eps = 0.0001;
    double umin=0, umax=0, vmin=0, vmax=0;
    BRepTools::UVBounds(aFace,umin, umax, vmin, vmax);

    Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace);
//    GeomLProp_SLProps props(aSurface, 1, eps/1000.0); // max 1 derivation

    for(int in=0; in<innerslg.size(); in++) innerslg[in].clear();
    innerslg.clear();

    // find the Face's wire
    occ::findWires(aFace, theouterwire, theinnerwires, strange, "      ");

    if(theouterwire.IsNull())
    {
        logmsg += "   no outer wire found: aborting\n";
        return;
    }

    // build the PSLG in the parametric space from the edges of the wires
    gp_Pnt2d p2d, p1;
    gp_Pnt P0, P1;

    gp_Dir Dir;
    Node nd0, nd1;
    Standard_Real First=0, Last=0;
    double c=0;

    for(TopTools_ListIteratorOfListOfShape WireIt(theinnerwires); WireIt.More(); WireIt.Next())
    {
        if(WireIt.Value()!=theouterwire) innerslg.push_back({});

        TopExp_Explorer WireExplorer;


        for (WireExplorer.Init(WireIt.Value(), TopAbs_EDGE); WireExplorer.More(); WireExplorer.Next())
        {
            TopoDS_Edge const &anEdge = TopoDS::Edge(WireExplorer.Current());

            if(!anEdge.IsNull())
            {
                //get the curve in the face's parametric space
                Handle(Geom_Curve) edgecurve = BRep_Tool::Curve(anEdge, First, Last);
                Handle(Geom2d_Curve) hCurve = BRep_Tool::CurveOnSurface(anEdge, aFace, curvemin, curvemax);

                if(WireIt.Value()==theouterwire)
                {
                    std::vector<double> uval;
                    occ::makeEdgeUniformSplitList(anEdge, s_MaxEdgeLength, uval);

                    if(anEdge.Orientation()==TopAbs_REVERSED)     std::reverse(uval.begin(), uval.end());

                    c = uval.at(0);
                    edgecurve->D0(c, P0);
                    nd0.set(P0.X(), P0.Y(), P0.Z());

//                    BOPTools_AlgoTools3D::GetNormalToFaceOnEdge(anEdge, aFace, c, Dir);
                    BOPTools_AlgoTools3D::GetApproxNormalToFaceOnEdge(anEdge, aFace, c, P0, Dir, 0.01);
                    nd0.setNormal(Dir.X(), Dir.Y(), Dir.Z());

                    int nSegs=0;
                    for(uint k=1; k<uval.size(); k++)
                    {
                        c = uval.at(k);
                        edgecurve->D0(c, P1);
                        nd1.set(P1.X(), P1.Y(), P1.Z());

//                        BOPTools_AlgoTools3D::GetNormalToFaceOnEdge(anEdge, aFace, c, Dir);
                        BOPTools_AlgoTools3D::GetApproxNormalToFaceOnEdge(anEdge, aFace, c, P1, Dir, 0.01);
                        nd1.setNormal(Dir.X(), Dir.Y(), Dir.Z());

                        contourpslgUV.push_back({nd0, nd1});

                        nd0 = nd1;
                        nSegs++;
                    }
                    (void)nSegs;
                }
                else
                {
                    // don't split the inner wire edges to keep the split made for the foil edges
                    edgecurve->D0(curvemin, P0);
                    nd0.set(P0.X(), P0.Y(), P0.Z());
                    BOPTools_AlgoTools3D::GetNormalToFaceOnEdge(anEdge, aFace, curvemin, Dir);
                    nd0.setNormal(Dir.X(), Dir.Y(), Dir.Z());

                    edgecurve->D0(curvemax, P1);
                    nd1.set(P1.X(), P1.Y(), P1.Z());
                    BOPTools_AlgoTools3D::GetNormalToFaceOnEdge(anEdge, aFace, curvemax, Dir);
                    nd1.setNormal(Dir.X(), Dir.Y(), Dir.Z());

                    if  (anEdge.Orientation()==TopAbs_FORWARD)
                        innerslg.back().push_back({nd0, nd1});
                    else if(anEdge.Orientation()==TopAbs_REVERSED)
                        innerslg.back().push_back({nd1, nd0});
                }

                logmsg += QString::fromStdString(strange);
            }

        }
    }
}


/**
 * Mesh the face using the advancing front method.
 * It is not required that the front is continuous. It can the union
 * of the contour SLG and the inner SLG, e.g. the foil's edges.
 * All segments must be oriented positively around the surface normal.
 */
bool AFMesher::makeTriangles(TopoDS_Face const &aFace,
                             SLG3d &slg3d,
                             std::vector<Triangle3d> &facetriangles,
                             double MaxEdgeLength, double MaxPanelCount,
                             QString &logmsg) const
{
    std::vector<Triangle3d> starttriangles = s_Triangles; // for animation

    SLG3d &front = slg3d;

    std::vector<Node> frontnodes;
    front.makeNodes(frontnodes);
    Triangle3d triangle;

    int iseg=0;
    int iter=0;
    if(s_MaxIterations==0)
    {
        slg3d = front;
        if(s_bIsAnimating)
        {
            s_SLG.clear();
            s_Triangles.clear();
            s_SLG = front;
            s_Triangles = starttriangles;
            s_Triangles.insert(s_Triangles.end(), facetriangles.begin(), facetriangles.end());
            postAnimateEvent();
        }
        return true;
    }

    while(front.size()>0)
    {
        iter++;

        if(s_MaxIterations>0 && iter>=s_MaxIterations)
        {
            int nada=0; (void)nada;
        }

        // process the segment
        Segment3d baseseg = front.at(iseg); // don't take a reference from a changing array

        // find the previous adjacent segment which makes the minimal angle with this segment
        int iprevious = -1;
        double theta_prev = 2.0*PI;
        front.previous(iseg, theta_prev, iprevious);

        // find the next adjacent segment which makes the minimal angle with this segment
        int inext = -1;
        double theta_next = 2.0*PI;
        front.next(iseg, theta_next, inext);

        if(iprevious>=0 && inext>=0)
        {
            // address the case where {previous.seg.next} form a closed triangle
            // this eliminates some special cases at the outset
            Segment3d previous = front.at(iprevious);  // don't take a reference on a changing array
            Segment3d next     = front.at(inext);      // don't take a reference on a changing array

            if(previous.vertexAt(0).isSame(next.vertexAt(1), 1.e-6))
            {
                triangle.setTriangle(baseseg.vertexAt(0), baseseg.vertexAt(1), next.vertexAt(1));
                facetriangles.push_back(triangle);
                //remove the three segments
                front.removeSegments(next);
                front.removeSegments(previous);
                front.removeSegments(baseseg);
                // move on
                iseg++;
                if(front.size()) iseg = iseg%front.size();

                if(s_bIsAnimating)
                {
                    s_SLG = front;
                    s_Triangles = starttriangles;
                    s_Triangles.insert(s_Triangles.end(), facetriangles.begin(), facetriangles.end());
                    postAnimateEvent();
                }

                if(s_MaxIterations>=0 && iter>=s_MaxIterations) break;

                continue;
            }
        }

        // To avoid a flat panel at the nose, don't build the first triangle using the last segment.

        bool bCheckCloseNodes = true;
        // If the angle with the adjacent segment is less than 90°
        if(theta_prev<PI/3 && theta_prev<theta_next)
        {
            // use the previous segment to build the triangle
            Segment3d previous = front.at(iprevious); // don't take a reference on a changing array
            triangle.setTriangle(baseseg.vertexAt(0), baseseg.vertexAt(1), previous.vertexAt(0));
            bCheckCloseNodes = false;
        }
        //  next segment can be used if the angle is less than PI/2
        else if(theta_next<PI/3)
        {
            // use the next segment to build the triangle
            Segment3d next = front.at(inext);
            triangle.setTriangle(baseseg.vertexAt(0), baseseg.vertexAt(1), next.vertexAt(1));
            bCheckCloseNodes = false;
        }
        else
        {
            // no success with the previous and next segments
            // make the ideal triangle

            if(!occ::makeEquiTriangle(aFace, baseseg, MaxEdgeLength, s_GrowthFactor, triangle))
            {
                //face is unmeshable
                QString strange;
                strange = QString::asprintf("***Unconverged: Could not build triangle at iteration %d\n", iter);
                logmsg += strange;
                slg3d = front;
                return false;
            }

            bCheckCloseNodes = true; // there may be some more suitable nodes nearby
        }

        // make the new segments
        Segment3d newsegs[] = {{triangle.vertexAt(0), triangle.vertexAt(2)}, {triangle.vertexAt(2), triangle.vertexAt(1)}};

        // Before adding the triangle,
        //   - check that there are no internal nodes
        //   - check that the new segments do not intersect existing ones
        //   - check if there are available nodes within the search radius

        std::vector<Node> insidenodes, closenodes;
        front.nodesInTriangle(triangle, insidenodes);

        if(bCheckCloseNodes)
        {
            // the search radius shouldn't be too large to maintain a smooth progression
            // of triangle sizes, and not too small to avoid filling gaps with small triangles
            double searchradius = qMin(MaxEdgeLength, baseseg.length() * s_SearchRadiusFactor);
            front.nodesAroundCenter(triangle.vertexAt(2), searchradius, closenodes);
        }

        std::vector<int> intersected;
        std::vector<Vector3d> I;
        front.intersect(newsegs[0], intersected, I, XflMesh::nodeMergeDistance());
        front.intersect(newsegs[1], intersected, I, XflMesh::nodeMergeDistance());
//        front.intersect(newsegs[0], intersected, I, 0.005);
//        front.intersect(newsegs[1], intersected, I, 0.005);

        // add the vertices of the intersected segments to the close nodes
        for(uint i=0; i<intersected.size(); i++)
        {
            Segment3d const &seg = front.at(intersected.at(i));
            // the vertices may already be in the close nodes
            closenodes.push_back(seg.vertexAt(0));
            closenodes.push_back(seg.vertexAt(1));
        }

        // link preferably to the inside nodes
        if(insidenodes.size())
        {
            // pick the one which minimizes the distances to the base segment's vertices?
            double dmax = 1e10;
            for(uint in=0; in<insidenodes.size(); in++)
            {
                Node const &nd = insidenodes.at(in);
                double d0 = nd.distanceTo(baseseg.vertexAt(0));
                double d1 = nd.distanceTo(baseseg.vertexAt(1));
                // rule out the base segment's nodes
                if(d0<1.e-6 || d1<1.e-6) continue;

                double dist = d0 + d1;
                if(dist<dmax)
                {
                    triangle = {baseseg.vertexAt(0), baseseg.vertexAt(1), nd};
                    dmax = dist;
                }
            }
        }
        else if(closenodes.size())
        {
            Triangle3d t3d = triangle;
            double dmax = 1.e10;
            for(uint in=0; in<closenodes.size(); in++)
            {
                Node const &nd = closenodes.at(in);
                if(nd.isSame(baseseg.vertexAt(0), 1.e-6) || nd.isSame(baseseg.vertexAt(1), 1.e-6) )
                    continue;

                double d0 = nd.distanceTo(baseseg.vertexAt(0));
                double d1 = nd.distanceTo(baseseg.vertexAt(1));
                // reject the base segment's nodes
                if(d0<1.e-6 || d1<1.e-6) continue;

                // reject if intersecting adjacent segments
                Segment3d seg1(baseseg.vertexAt(0), nd);
                Segment3d seg2(baseseg.vertexAt(1), nd);
                Node In;
                if(iprevious>=0)
                {
                    Segment3d const &previous = front.at(iprevious);
                    if(seg2.intersectsProjected(previous, In, XflMesh::nodeMergeDistance())) continue;
                }
                if(inext>=0)
                {
                    Segment3d const &next = front.at(inext);
                    if(seg1.intersectsProjected(next,     In, XflMesh::nodeMergeDistance())) continue;
                }

                Triangle3d t3d_test = {baseseg.vertexAt(0), baseseg.vertexAt(1), nd};
                double dist = d0 + d1;
                // reject inverted
                if(dist<dmax && baseseg.averageNormal().dot(t3d_test.normal())>0)
                {
                    t3d = t3d_test;
                    dmax = dist;
                }
            }

            // It can occur that new nodes have been included in the modified triangle,
            // so check again
            insidenodes.clear();
            front.nodesInTriangle(t3d, insidenodes);

            if(insidenodes.size())
            {
                // pick the one which minimizes the distances to the base segment's vertices?
                double dmax = 1e10;
                for(uint in=0; in<insidenodes.size(); in++)
                {
                    Node const &nd = insidenodes.at(in);
                    double d0 = nd.distanceTo(baseseg.vertexAt(0));
                    double d1 = nd.distanceTo(baseseg.vertexAt(1));
                    // rule out the base segment's nodes
                    if(d0>1.e-6 && d1>1.e-6)
                    {
                        double dist = d0 + d1;
                        if(dist<dmax)
                        {
                            t3d = {baseseg.vertexAt(0), baseseg.vertexAt(1), nd};
                            dmax = dist;
                        }
                    }
                }
            }
            // if everything has been checked, keep this triangle
            triangle = t3d;
        }

        if(!triangle.isNull())
        {
            facetriangles.push_back({triangle});
        }

        // update segments
        newsegs[0] = {triangle.vertexAt(0), triangle.vertexAt(2)};
        newsegs[1] = {triangle.vertexAt(2), triangle.vertexAt(1)};

        // the new triangle has been inserted
        // remove the base segment
        front.removeAt(iseg); // same as triangle.edge(0)

        // check if the other two edges need to be added if non existent,
        // or removed if the front has merged

        for(int is=0; is<2; is++)
        {
            int iExists = front.isSegment(newsegs[is], 1.e-6);
            if(iExists>=0)
            {
                // the front has merged, delete the existing segment, and don't add the new one
                front.removeAt(iExists);
                if(iExists<iseg) iseg--;
            }
            else
            {
                front.insertAt(iseg, newsegs[is]);
                iseg++;
            }
        }

        if(front.size()) iseg = iseg%front.size();
        if(facetriangles.size()>MaxPanelCount) break;

        if(s_MaxIterations>=0 && iter>=s_MaxIterations) break;

        if(s_bIsAnimating)
        {
            s_SLG = front;
            s_Triangles = starttriangles;
            s_Triangles.insert(s_Triangles.end(), facetriangles.begin(), facetriangles.end());
            postAnimateEvent();
        }

//        if(iter%30==0) 

        if(s_bCancel)
            break;
    }

    if(aFace.Orientation()==TopAbs_REVERSED)
    {
        for(uint it=0; it<facetriangles.size(); it++)
            facetriangles[it].reverseOrientation();
    }


    logmsg.clear();
    if(facetriangles.size()>=MaxPanelCount)
    {
        logmsg = "      The maximum number of triangles has been reached for this FACE\n";
    }

/*    if(front.size()==0)
    {
        QString strange;
        strange = QString::asprintf("      Made %d triangles in %d iterations\n", facetriangles.size(), iter);
        logmsg += strange;
    }*/
    if(front.size()!=0)
    {
        QString strange;
        strange = QString::asprintf("      ***Unconverged: made %d triangles in %d iterations***\n"
                        "                      remains %d unprocessed segments\n",
                        int(facetriangles.size()), iter, int(front.size()));
        logmsg += strange;
    }

    slg3d = front;
    return true;
}


void AFMesher::postAnimateEvent() const
{
    MeshEvent *pMeshEvent = new MeshEvent();
    qApp->postEvent(m_pParent, pMeshEvent);
    
#ifdef Q_OS_WIN
        Sleep(s_AnimationPause);
#endif
#ifdef Q_OS_LINUX
        usleep(uint(s_AnimationPause*1000));
#endif
#ifdef Q_OS_MAC
        usleep(uint(s_AnimationPause*1000));
#endif
}


void AFMesher::postMessageEvent(QString const & msg) const
{
    MessageEvent * pMsgEvent = new MessageEvent(msg.toStdString());
    qApp->postEvent(m_pParent, pMsgEvent);
    
}


/**
 * Builds the Face's PSLG2d in parametric space.
 * Only the outer PSLG is split with the input length.
 * The inner pslg are not split to match the foil's vertices.
 * @returns the number of the PSLG segments
 */
void AFMesher::makeFacePSLG2d(const TopoDS_Face &aFace,
                              QVector<PSLG2d> &innerpslgUV, PSLG2d &contourpslgUV,
                              QString &logmsg) const
{
    std::string strange;
    TopoDS_ListOfShape allwires;
    TopoDS_Wire theouterwire;
    double curvemin(0), curvemax(0);

    for(int in=0; in<innerpslgUV.size(); in++) innerpslgUV[in].clear();
    innerpslgUV.clear();

    // find the Face's wire
    occ::findWires(aFace, theouterwire, allwires, strange, "      ");

    if(theouterwire.IsNull())
    {
        logmsg += "   no outer wire found aborting\n";
        return;
    }

    // build the PSLG in the parametric space from the edges of the wires
    std::vector<double> uval;
    gp_Pnt2d P0,P1;
    int nWire = 0;
    for(TopTools_ListIteratorOfListOfShape WireIt(allwires); WireIt.More(); WireIt.Next())
    {
        if(WireIt.Value()!=theouterwire)
            innerpslgUV.push_back({});

        TopExp_Explorer WireExplorer;

        int nEdge=0;
        for (WireExplorer.Init(WireIt.Value(), TopAbs_EDGE); WireExplorer.More(); WireExplorer.Next())
        {
            TopoDS_Edge const &anEdge = TopoDS::Edge(WireExplorer.Current());

            if(!anEdge.IsNull())
            {
                //get the curve in the face's parametric space
                Handle(Geom2d_Curve) hCurve = BRep_Tool::CurveOnSurface(anEdge, aFace, curvemin, curvemax);

                if(WireIt.Value()==theouterwire)
                {
                    getEdgeSplit(anEdge, uval);
                    if(!uval.size())
                        occ::makeEdgeUniformSplitList(anEdge, s_MaxEdgeLength, uval);

                    if(!uval.size())
                    {
                        logmsg += QString::asprintf("   error splitting edge %d of wire %d\n", nEdge, nWire);
                        return;
                    }

                    if(anEdge.Orientation()==TopAbs_REVERSED)
                    {
                        std::reverse(uval.begin(), uval.end());
                    }
//                    double deltaC = curvemax-curvemin;
//                    double c = curvemin+uval.at(0)*deltaC;
                    double c = uval.at(0);
                    hCurve->D0(c, P0);
                    int nSegs=0;
                    for(uint k=1; k<uval.size(); k++)
                    {
//                        c = curvemin+uval.at(k)*deltaC;
                        c = uval.at(k);
                        hCurve->D0(c, P1);

                        contourpslgUV.push_back({{P0.X(), P0.Y()}, {P1.X(), P1.Y()}});

                        P0 = P1;
                        nSegs++;
                    }
                    (void)nSegs;
                }
                else
                {
                    if(!m_bSplittableInnerPSLG)
                    {
                        // don't split the inner wire edges to keep the split made with the foil's edges
                        hCurve->D0(curvemin, P0);
                        hCurve->D0(curvemax, P1);
                        if  (anEdge.Orientation()==TopAbs_FORWARD)
                            innerpslgUV.back().push_back({{P0.X(), P0.Y()}, {P1.X(), P1.Y()}});
                        else if(anEdge.Orientation()==TopAbs_REVERSED)
                            innerpslgUV.back().push_back({{P1.X(), P1.Y()}, {P0.X(), P0.Y()}});
                    }
                    else
                    {
                        occ::makeEdgeUniformSplitList(anEdge, s_MaxEdgeLength, uval);

                        if(anEdge.Orientation()==TopAbs_REVERSED)
                        {
                            std::reverse(uval.begin(), uval.end());
                        }
//                        double deltaC = curvemax-curvemin;
//                        double c = curvemin+uval.at(0)*deltaC;
                        double c = uval.at(0);
                        hCurve->D0(c, P0);
                        int nSegs=0;
                        for(uint k=1; k<uval.size(); k++)
                        {
//                            c = curvemin+uval.at(k)*deltaC;
                            c = uval.at(k);
                            hCurve->D0(c, P1);

                            innerpslgUV.back().push_back({{P0.X(), P0.Y()}, {P1.X(), P1.Y()}});

                            P0 = P1;
                            nSegs++;
                        }
                        (void)nSegs;
                    }
                }
                logmsg += QString::fromStdString(strange);
            }

            nEdge++;
        }
        nWire++;
    }

    contourpslgUV.setSplittable(false); // for p-space mesher only, to keep same number of vertices with common edges
}


void AFMesher::makeDelaunayFlips(std::vector<Triangle3d> &triangles, int &nFlips, int &nIter) const
{
    nFlips = 0;
//    makeConnections(); // no use, connections change at each flip
    nIter = 0;
    int iterflips=0;

    do
    {
        iterflips=0;
        for(uint it=0; it<triangles.size(); it++)
        {
            Triangle3d &t2d = triangles[it];
            for(uint it2=it+1; it2<triangles.size(); it2++)
            {
                Triangle3d &t2n = triangles[it2];
                for(int ie0=0; ie0<3; ie0++)
                {
                    Segment3d testedge = t2d.edge(ie0); // copy to modify in-place

                    int ien = t2n.edgeIndex(testedge, 1.e-6);
                    if(ien>=0)
                    {
                        // compare opposite angles
                        double alpha0 = t2d.angle(ie0);
                        double alphan = t2n.angle(ien);

                        if(alpha0+alphan>181.0) // +1° in case of two 90° angles which cause infinite flips
                        {
                            //perform Delaunay flip
                            Node vtx0 = t2d.vertexAt(ie0); // copy to modify in-place
                            Node vtxn = t2n.vertexAt(ien); // copy to modify in-place
                            t2d.setTriangle(vtx0, testedge.vertexAt(0), vtxn);
                            t2n.setTriangle(vtx0, vtxn, testedge.vertexAt(1));
                            nFlips++;
                            iterflips++;
                            break;
                        }
                    }
                }
                if(s_bCancel) break;
            }
            if(it%10==0)
            {
                
            }
            if(s_bCancel) break;
        }

        QString strange = QString::asprintf("     %2d flips at iteration %2d\n", iterflips, nIter+1);
        postMessageEvent(strange);

        nIter++;

        if(s_bCancel) break;
    }
    while(iterflips>0 && nIter<10);
}


/** Builds the edge SLG for the free mesh.
    Only used for thin sails of the NURBS or SPLINE types.*/
void AFMesher::makeEdgeSLG(Sail *pSail, SLG3d & slg) const
{
//    int nVtx = (m_NXPanels+1)*2+(m_NZPanels+1)*2 - 3;
    int nVtx = 1;
    std::vector<EdgeSplit> const& es = pSail->edgeSplit().front();

    for(int i=0; i<4; i++) nVtx += es[i].nSegs();

    slg.resize(nVtx-1);

    QVector<Node> Vtx(nVtx);
    int ivtx=0;
    Vtx[ivtx++].set(pSail->edgeNode(0,0));

    Node P0, P1;
    int nSegs(0);
    double umin(0), umax(0),vmin(0), vmax(0);
    double u0(0), u1(0), v0(0), v1(0);
    double tl(0);
    double length(0);
    double l(0);

    double eps = 0.0001;
    std::vector<double> psplit;

    int iter=0;

    for(int iSide=0; iSide<4; iSide++)
    {
        nSegs = es[iSide].nSegs();
        xfl::getPointDistribution(psplit, nSegs, es[iSide].distrib());
        if(iSide==0)
        {
            umin=0, umax=0;
            vmin=0, vmax=1;
        }
        else if(iSide==1)
        {
            umin=0, umax=1;
            vmin=1, vmax=1;
        }
        else if(iSide==2)
        {
            umin=1, umax=1;
            vmin=1, vmax=0;
        }
        else if(iSide==3)
        {
            umin=1, umax=0;
            vmin=0, vmax=0;
        }

        l = pSail->edgeLength(umin, vmin, umax, vmax);

        // proceed using dichotomy
        for(int i=1; i<nSegs; i++)
        {
            tl = l * psplit.at(i); // target length
            u0 = umin;
            u1 = umax;
            v0 = vmin;
            v1 = vmax;

            P0 = pSail->edgeNode(u0, v0);
            P1 = pSail->edgeNode(u1, v1);
            iter = 0;
            do
            {
                length = pSail->edgeLength(umin, vmin, (u0+u1)/2.0, (v0+v1)/2.0);
                if(length<tl)
                {
                    u0 = (u0+u1)/2.0;
                    v0 = (v0+v1)/2.0;
                }
                else
                {
                    u1 = (u0+u1)/2.0;
                    v1 = (v0+v1)/2.0;
                }
            }
            while(fabs(length-tl)>eps && iter++<20);

            Vtx[ivtx++] = pSail->edgeNode((u0+u1)/2.0, (v0+v1)/2.0);
        }
        Vtx[ivtx++] = pSail->edgeNode(umax, vmax);
    }

    for(int i=0; i<Vtx.size()-1; i++)
    {
        slg[i].setNodes(Vtx[i], Vtx[i+1]);
    }
}


bool AFMesher::makeThinSailMesh()
{
    if(!m_pSail)
    {
        emit meshFinished();
        return false;
    }
    s_SLG.clear();

    double MaxEdgeLength = m_pSail->maxElementSize();
    double MaxPanelCount = s_MaxPanelCount;

    QString logmsg, strange;

    if(!m_pSail->edgeSplit().size() || !m_pSail->edgeSplit().front().size())
    {
        emit meshFinished();
        return false;
    }

    SLG3d slg3d;
    makeEdgeSLG(m_pSail, slg3d);

    // clean null segments
    for(int i=int(slg3d.size()-1); i>=0; i--)
    {
        if(slg3d.at(i).isNull()) slg3d.removeAt(i);
    }

    logmsg = QString::asprintf("   made %d free edges\n", int(slg3d.size()));

    SLG3d front = slg3d;
    std::vector<int> intersected;
    std::vector<Vector3d> I;
    std::vector<Node> frontnodes, insidenodes, closenodes;
    front.makeNodes(frontnodes);
    Triangle3d triangle, t3d, t3d_test;
    Segment3d baseseg, previous, next, seg1, seg2;
    int iseg=0, inext=0, iprevious=0, iter=0;
    double theta_next=0, theta_prev=0;
    bool bCheckCloseNodes = true;
    Node In;
    if(s_MaxIterations==0)
    {
        slg3d = front;
        if(s_bIsAnimating)
        {
            s_SLG.clear();
            s_Triangles.clear();
            s_SLG = front;
            s_Triangles = m_Triangles;
            postAnimateEvent();
        }
        emit meshFinished();
        return true;
    }

    while(front.size()>0)
    {
        iter++;

        if(s_MaxIterations>0 && iter>=s_MaxIterations)
        {
            int nada=0; (void)nada;
        }

        // process the segment
        baseseg = front.at(iseg); // don't take a reference from a changing array

        // find the previous adjacent segment which makes the minimal angle with this segment
        iprevious = -1;
        theta_prev = 2.0*PI;
        front.previous(iseg, theta_prev, iprevious);

        // find the next adjacent segment which makes the minimal angle with this segment
        inext = -1;
        theta_next = 2.0*PI;
        front.next(iseg, theta_next, inext);

        if(iprevious>=0 && inext>=0)
        {
            // address the case where {previous.seg.next} form a closed triangle
            // this eliminates some special cases at the outset
            previous = front.at(iprevious);  // don't take a reference on a changing array
            next     = front.at(inext);      // don't take a reference on a changing array

            if(previous.vertexAt(0).isSame(next.vertexAt(1), 1.e-6))
            {
                triangle.setTriangle(baseseg.vertexAt(0), baseseg.vertexAt(1), next.vertexAt(1));
                m_Triangles.push_back(triangle);
                //remove the three segments
                front.removeSegments(next);
                front.removeSegments(previous);
                front.removeSegments(baseseg);
                // move on
                iseg++;
                if(front.size()) iseg = iseg%front.size();

                if(s_MaxIterations>=0 && iter>=s_MaxIterations) break;

                continue;
            }
        }

        // To avoid a flat panel at the nose, don't build the first triangle using the last segment.

        bCheckCloseNodes = true;
        // If the angle with the adjacent segment is less than 90°
        if(theta_prev<PI/3 && theta_prev<theta_next)
        {
            // use the previous segment to build the triangle
            previous = front.at(iprevious); // don't take a reference on a changing array
            triangle.setTriangle(baseseg.vertexAt(0), baseseg.vertexAt(1), previous.vertexAt(0));
            bCheckCloseNodes = false;
        }
        //  next segment can be used if the angle is less than PI/2
        else if(theta_next<PI/3)
        {
            // use the next segment to build the triangle
            next = front.at(inext);
            triangle.setTriangle(baseseg.vertexAt(0), baseseg.vertexAt(1), next.vertexAt(1));
            bCheckCloseNodes = false;
        }
        else
        {
            // no success with the previous and next segments
            // make the ideal triangle

            if(!makeEquiTriangleOnSail(m_pSail, baseseg, MaxEdgeLength, s_GrowthFactor, triangle))
            {
                //face is unmeshable
                slg3d = front;

                strange = QString::asprintf("***Unconverged: Could not build triangle at iteration %d\n", iter);
                logmsg += strange;
                postMessageEvent(logmsg);

                // send troubleshooting info
                s_Triangles = m_Triangles;
                s_SLG = front;

                emit meshFinished();

                return false;
            }

            bCheckCloseNodes = true; // there may be some more suitable nodes nearby
        }

        // make the new segments
        Segment3d newsegs[] = {{triangle.vertexAt(0), triangle.vertexAt(2)}, {triangle.vertexAt(2), triangle.vertexAt(1)}};

        // Before adding the triangle,
        //   - check that there are no internal nodes
        //   - check that the new segments do not intersect existing ones
        //   - check if there are available nodes within the search radius

        insidenodes.clear();
        closenodes.clear();
        front.nodesInTriangle(triangle, insidenodes);

        if(bCheckCloseNodes)
        {
            // the search radius shouldn't be too large to maintain a smooth progression
            // of triangle sizes, and not too small to avoid filling gaps with small triangles
            double searchradius = qMin(MaxEdgeLength, baseseg.length() * s_SearchRadiusFactor);
            front.nodesAroundCenter(triangle.vertexAt(2), searchradius, closenodes);
        }

        intersected.clear();
        I.clear();

        front.intersect(newsegs[0], intersected, I, 0.005);
        front.intersect(newsegs[1], intersected, I, 0.005);

        // add the vertices of the intersected segments to the close nodes
        for(uint i=0; i<intersected.size(); i++)
        {
            Segment3d const &seg = front.at(intersected.at(i));
            // the vertices may already be in the close nodes
            closenodes.push_back(seg.vertexAt(0));
            closenodes.push_back(seg.vertexAt(1));
        }
#ifdef QT_DEBUG
        if(s_MaxIterations>0 && iter>=s_MaxIterations)
        {
            s_DebugPts.clear();
            s_DebugPts.append(triangle.vertexAt(2));
            for(uint i=0; i<closenodes.size(); i++)  s_DebugPts.append(closenodes.at(i));
        }
#endif
        // link preferably to the inside nodes
        if(insidenodes.size())
        {
            // pick the one which minimizes the distances to the base segment's vertices?
            double dmax = 1e10;
            for(uint in=0; in<insidenodes.size(); in++)
            {
                Node const &nd = insidenodes.at(in);
                double d0 = nd.distanceTo(baseseg.vertexAt(0));
                double d1 = nd.distanceTo(baseseg.vertexAt(1));
                // rule out the base segment's nodes
                if(d0<1.e-6 || d1<1.e-6) continue;

                double dist = d0 + d1;
                if(dist<dmax)
                {
                    triangle = {baseseg.vertexAt(0), baseseg.vertexAt(1), nd};
                    dmax = dist;
                }
            }
        }
        else if(closenodes.size())
        {
            t3d = triangle;
            double dmax = 1.e10;
            for(uint in=0; in<closenodes.size(); in++)
            {
                Node const &nd = closenodes.at(in);
                if(nd.isSame(baseseg.vertexAt(0), 1.e-6) || nd.isSame(baseseg.vertexAt(1), 1.e-6) )
                    continue;

                double d0 = nd.distanceTo(baseseg.vertexAt(0));
                double d1 = nd.distanceTo(baseseg.vertexAt(1));
                // reject the base segment's nodes
                if(d0<1.e-6 || d1<1.e-6) continue;

                // reject if intersecting adjacent segments
                seg1.setNodes(baseseg.vertexAt(0), nd);
                seg2.setNodes(baseseg.vertexAt(1), nd);

                if(iprevious>=0)
                {
                    Segment3d const &previous = front.at(iprevious);
                    if(seg2.intersectsProjected(previous, In, XflMesh::nodeMergeDistance())) continue;
                }
                if(inext>=0)
                {
                    Segment3d const &next = front.at(inext);
                    if(seg1.intersectsProjected(next,     In, XflMesh::nodeMergeDistance())) continue;
                }

                t3d_test.setTriangle(baseseg.vertexAt(0), baseseg.vertexAt(1), nd);
                double dist = d0 + d1;
                // reject inverted
                if(dist<dmax && baseseg.averageNormal().dot(t3d_test.normal())>0)
                {
                    t3d = t3d_test;
                    dmax = dist;
                }
            }

            // It can occur that new nodes have been included in the modified triangle,
            // so check again
            insidenodes.clear();
            front.nodesInTriangle(t3d, insidenodes);

            if(insidenodes.size())
            {
                // pick the one which minimizes the distances to the base segment's vertices?
                double dmax = 1e10;
                for(uint in=0; in<insidenodes.size(); in++)
                {
                    Node const &nd = insidenodes.at(in);
                    double d0 = nd.distanceTo(baseseg.vertexAt(0));
                    double d1 = nd.distanceTo(baseseg.vertexAt(1));
                    // rule out the base segment's nodes
                    if(d0>1.e-6 && d1>1.e-6)
                    {
                        double dist = d0 + d1;
                        if(dist<dmax)
                        {
                            t3d = {baseseg.vertexAt(0), baseseg.vertexAt(1), nd};
                            dmax = dist;
                        }
                    }
                }
            }
            // if everything has been checked, keep this triangle
            triangle = t3d;
        }

        if(!triangle.isNull())
        {
            m_Triangles.push_back({triangle});
        }


        // update segments
        newsegs[0] = {triangle.vertexAt(0), triangle.vertexAt(2)};
        newsegs[1] = {triangle.vertexAt(2), triangle.vertexAt(1)};

        // the new triangle has been inserted
        // remove the base segment
        front.removeAt(iseg); // same as triangle.edge(0)

        // check if the other two edges need to be added if non existent,
        // or removed if the front has merged

        for(int is=0; is<2; is++)
        {
            int iExists = front.isSegment(newsegs[is], 1.e-6);
            if(iExists>=0)
            {
                // the front has merged, delete the existing segment, and don't add the new one
                front.removeAt(iExists);
                if(iExists<iseg) iseg--;
            }
            else
            {
                front.insertAt(iseg, newsegs[is]);
                iseg++;
            }
        }

        if(front.size()) iseg = iseg%front.size();
        if(m_Triangles.size()>MaxPanelCount) break;

        if(s_MaxIterations>=0 && iter>=s_MaxIterations) break;

        if(s_bCancel)
        {
            logmsg = "   Mesh operation cancelled\n";
            postMessageEvent(logmsg);

            MeshEvent *pMeshEvent = new MeshEvent(m_Triangles);
            qApp->postEvent(m_pParent, pMeshEvent);
            
            emit meshFinished();

            return false;
        }
    }

    logmsg += QString::asprintf("   made %d triangles\n", int(m_Triangles.size()));

    postMessageEvent(logmsg);

    
    logmsg.clear();

    if(m_Triangles.size()>=MaxPanelCount || (s_MaxIterations>=0 &&iter>=s_MaxIterations))
    {
        postMessageEvent("   The maximum number of triangles has been reached for this FACE\n");

        // send troubleshooting info
        s_Triangles = m_Triangles;
        s_SLG = front;
        MeshEvent *pMeshEvent = new MeshEvent();
        qApp->postEvent(m_pParent, pMeshEvent);
        
        emit meshFinished();

        return false;
    }

    if(front.size()!=0)
    {
        QString strange;
        strange = QString::asprintf("   ***Unconverged: made %d triangles in %d iterations***\n"
                                    "                   remains %d unprocessed segments\n",
                                    int(m_Triangles.size()), iter, int(front.size()));
        logmsg += strange;
        postMessageEvent(logmsg);

        // send troubleshooting info
        s_Triangles = m_Triangles;
        s_SLG = front;
        MeshEvent *pMeshEvent = new MeshEvent();
        qApp->postEvent(m_pParent, pMeshEvent);
        
        emit meshFinished();
        return false;
    }

    slg3d = front;

    if(s_bDelaunay)
    {
        int nFaceFlips=0;
        int nIter=0;
        postMessageEvent("   making Delaunay flips...\n");

        makeDelaunayFlips(m_Triangles, nFaceFlips, nIter);
        logmsg = QString::asprintf("      made %d flips in %d iterations\n", nFaceFlips, nIter);
        postMessageEvent(logmsg);

        logmsg.clear();
    }
    postMessageEvent(logmsg);

    s_SLG = front;
    MeshEvent *pMeshEvent = new MeshEvent(m_Triangles);
    qApp->postEvent(m_pParent, pMeshEvent);

    emit meshFinished();

    thread()->exit(0); // exit event loop so that finished() is emitted

    return true;
}


/**
 * Builds an ideal triangle on the surface, with first edge seg
 */
bool AFMesher::makeEquiTriangleOnSail(Sail const*pSail, Segment3d const &baseseg, double maxedgelength, double growthfactor, Triangle3d &triangle)
{
    // make the segment's average normal
    // could also request the face normal at the mid-point, but may take longer
    Vector3d N = baseseg.vertexAt(0).normal() + baseseg.vertexAt(1).normal();
    N.normalize();
    Vector3d Nt = N * baseseg.unitDir();
    Node nd;

    double height = qMin(baseseg.length()*sin(PI/3), maxedgelength) * growthfactor;
    int iter = 0;

    Vector3d V3, A, B, I;

    do
    {
        iter++;
        V3 = baseseg.midPoint() + Nt*height;
        A = V3 + N*10.0;
        B = V3 + N*(-10.0);

        if(pSail->intersect(A, B, I, N))
        {
            nd.setPosition(I);
            nd.setNormal(N);
            triangle = {baseseg.vertexAt(0), baseseg.vertexAt(1), nd}; // make it the same orientation as the PSLG
            return true;
        }
        else
        {
            // The projection falls outside the surface.
            // Reduce the triangle's height and try again
            height *= 0.9;
        }
    }
    while(iter<10);
s_DebugPts.clear();
s_DebugPts.append(A);
s_DebugPts.append(B);
    return false;
}


void AFMesher::getEdgeSplit(TopoDS_Edge const &edge, std::vector<double> &uval) const
{
    if(!m_Shapes.Size()) return;
    if(!m_EdgeSplit.size())
    {
        //default
        occ::makeEdgeUniformSplitList(edge, s_MaxEdgeLength, uval);
        return;
    }

    TopExp_Explorer FaceExplorer;
    TopExp_Explorer EdgeExplorer;
    int iFace = 0;
    int iEdge = 0;

    for(FaceExplorer.Init(m_Shapes.First(), TopAbs_FACE); FaceExplorer.More(); FaceExplorer.Next())
    {
        TopoDS_Face const &face = TopoDS::Face(FaceExplorer.Current());
        if(iFace>=int(m_EdgeSplit.size()))
            break; //error somewhere, can't continue

        iEdge = 0;
        for(EdgeExplorer.Init(face, TopAbs_EDGE); EdgeExplorer.More(); EdgeExplorer.Next())
        {
            TopoDS_Edge const &anEdge = TopoDS::Edge(EdgeExplorer.Current());
            if(occ::isSameEdge(anEdge, edge))
            {
                if(iEdge>=int(m_EdgeSplit.at(iFace).size()))
                    break; //error somewhere, can't continue

                EdgeSplit const &es = m_EdgeSplit.at(iFace).at(iEdge);
                uval = es.split();
                return;
            }
            iEdge++;
        }
        iFace++;
    }
}







