/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include "ruppertmesher.h"

#include <xflgeom/geom_globals/geom_global.h>
#include <xflmath/constants.h>


bool RuppertMesher::s_bCancel = false;

RuppertMesher::RuppertMesher()
{
    m_MaxMeshIter = 100;
    m_MaxPanelCount = 3000;

    m_MinEdgeLength = 0.05;
    m_MaxEdgeLength = 1.0;
}


RuppertMesher::~RuppertMesher()
{
}


void RuppertMesher::clearAll()
{
    m_LastPSLG.clear();
    m_LastTriangles.clear();
}


/** Two points P and Q are visible to each other if the open line segment ]PQ[ (leaving out P and Q)
 does not intersect any segment of the input PSLG */
bool RuppertMesher::areVisible(Vector2d vtx0, Vector2d vtx1, PSLG2d const & PSLG)
{
    Vector2d IPt;
    for(int ipslg=0; ipslg<PSLG.size(); ipslg++)
    {
        if(PSLG.at(ipslg).intersects(vtx0, vtx1, IPt, false, 1.e-5)) return true;
    }
    return false;
}


void RuppertMesher::makeConnections()
{
    for(int it=0; it<m_LastTriangles.size(); it++)
    {
        Triangle2d  &t2d = m_LastTriangles[it];
        t2d.clearConnections();

        for(int it2=0; it2<m_LastTriangles.size(); it2++)
        {
            Triangle2d const& t2n = m_LastTriangles.at(it2);
            int iEdge=-1;
            if     (it!=it2 && t2d.isEdge(t2n.edge(0), iEdge))
            {
                t2d.setNeighbour(iEdge, it2);
            }
            else if(it!=it2 && t2d.isEdge(t2n.edge(1), iEdge))
            {
                t2d.setNeighbour(iEdge, it2);
            }
            else if(it!=it2 && t2d.isEdge(t2n.edge(2), iEdge))
            {
                t2d.setNeighbour(iEdge, it2);
            }
        }
    }
}


/**
 * Include the PSLG vertices in the array.
 * Note:  Occ error on vertex position can be greater 1.e-6, but seems to be less than 1.e-5
 *        so default precision is 1.e-5
 */
void RuppertMesher::addPSLGVertices(QVector<Vector2d> &VertexList, double precision)
{
    // add the vertices, checking for potential double points
    for(int i=0; i<m_LastPSLG.size(); i++)
    {
        for(int j=0; j<2; j++)
        {
            Vector2d vtxPSLG = m_LastPSLG.at(i).vertexAt(j);
            bool bFound = false;
            for(int k=0; k<VertexList.size(); k++)
            {
                if(VertexList.at(k).isSame(vtxPSLG, precision))
                {
                    bFound = true;
                    break;
                }
            }
            if(!bFound)
                VertexList.push_back(vtxPSLG);
        }
    }
}


bool RuppertMesher::checkDelaunayProperty(QVector<Triangle2d> const &TriangleList) const
{
    bool bDelaunay = true;
    // for each triangle
    for(int i=0; i<TriangleList.size(); i++)
    {
        //for each vertex of this triangle
        for(int j=0; j<3; j++)
        {
            Vector2d vtx = TriangleList.at(i).vertex(j);
            // check if the point is inside the circumcircle of another triangle
            for(int l=0; l<TriangleList.size(); l++)
            {
                if(l!=i)
                {
                    if(TriangleList[l].hasInCircumCircle(vtx) && ! TriangleList[l].hasVertex(vtx))
                    {
                        bDelaunay = false;
                    }
                }
            }
        }
    }
    return bDelaunay;
}



/** returns an array of unique edges of a list of triangles
 * i.e. edges common to two triangles are discarded */
PSLG2d RuppertMesher::uniqueEdges(QVector<Triangle2d> const &panelList) const
{
    PSLG2d edgeList;
    for(int i=0; i<panelList.size(); i++)
    {
        for(int j=0; j<3; j++)
        {
            Segment2d edge = panelList.at(i).edge(j);
            bool bFound = false;
            for(int k=0; k<edgeList.size(); k++)
            {
                if(edge.isSame(edgeList.at(k)))
                {
                    bFound = true;
                    edgeList.remove(k);
                    break;
                }
            }
            if(!bFound)
            {
                edgeList.appendSegment(edge);
//                displayDouble("appending ", edge.vertex(0).x, edge.vertex(0).y, edge.vertex(1).x, edge.vertex(1).y);
            }
        }
    }
    return edgeList;
}


void RuppertMesher::clearSuperTriangle(QVector<Triangle2d> &TriangleList, Triangle2d super) const
{
    for(int i=TriangleList.size()-1; i>=0; i--)
    {
        if     (TriangleList.at(i).hasVertex(super.vertex(0))) TriangleList.remove(i);
        else if(TriangleList.at(i).hasVertex(super.vertex(1))) TriangleList.remove(i);
        else if(TriangleList.at(i).hasVertex(super.vertex(2))) TriangleList.remove(i);
    }
}


int RuppertMesher::clearTrianglesInInnerPSLG(QVector<PSLG2d> const &innerPSLG)
{
    int removed = 0;
    for(int i=0; i<innerPSLG.size(); i++)
    {
        removed += clearTrianglesInPSLG(innerPSLG.at(i));
    }
    return removed;
}


int RuppertMesher::clearTrianglesInPSLG(PSLG2d const &pslg)
{
    int removed = 0;
    for(int it=m_LastTriangles.size()-1; it>=0; it--)
    {
        Vector2d const &cog = m_LastTriangles.at(it).CoG();
        if(pslg.contains(cog))
        {
            m_LastTriangles.removeAt(it);
            removed++;
        }
    }
    return removed;
}


int RuppertMesher::clearTrianglesOutsidePSLG(PSLG2d const &pslg, QVector<Triangle2d> &triangles) const
{
    int removed = 0;
    for(int it=triangles.size()-1; it>=0; it--)
    {
        Triangle2d const &t2d = triangles.at(it);
        if(!pslg.contains(t2d.CoG()))
        {
            triangles.removeAt(it);
            removed++;
        }
    }
    return removed;
}


void RuppertMesher::insertVertex(int t2d_index, Vector2d const &newvtx, QVector<Triangle2d> &triangles,
                                 QVector<int> &CClist, QVector<int> &addedlist,
                                 PSLG2d &PSLG, bool bCheckPSLGIntersections) const
{
    addedlist.clear();
    CClist.clear();
    Triangle2d tsrc = triangles.at(t2d_index); // make a copy before it is removed
    CClist.append(t2d_index);
    QVector<Triangle2d> modtris;

    //  step 1: list all the CONNECTED NEIGHBOUR triangles with the circumcircle containing the vertex:
    //          this prevents including triangles on the opposite side of a concavity
    for(int it=triangles.size()-1; it>=0; it--)
    {
        Triangle2d const &t2d = triangles.at(it);
        if(t2d.hasInCircumCircle(newvtx) && it!=t2d_index)
        {
            CClist.append(it);
        }
    }

    // keep only the triangles connected directly or indirectly neighbours of the initial triangle
    bool bAdded=false;
    int iter = 0;
    QVector<int> reducedlist = {t2d_index}; // the list of connected triangles
    PSLG2d perimeter;
    tsrc.edges(perimeter);
    do
    {
        bAdded = false;
        //stop at the second triangle, the first being the source triangle
        for(int it=CClist.size()-1; it>=1; it--)
        {
            int idx = CClist.at(it);
            Triangle2d const &tmod = triangles.at(idx);
            for(int ip=0; ip<perimeter.size(); ip++)
            {
                int iCommonEdge=-1;
                Segment2d const &seg = perimeter.at(ip);
                if(tmod.isEdge(seg, iCommonEdge))
                {
                    //extend the perimeter to the edges of the triangle
                    perimeter.remove(ip); //this edge is no longer on the perimeter
                    for(int ie=0; ie<3; ie++)
                    {
                        // add the two other edges to the perimeter
                        if(ie!=iCommonEdge) perimeter.appendSegment(tmod.edge(ie));
                    }
                    reducedlist.append(idx);// add the triangle's index to the remove list
                    CClist.removeAt(it); // no need to check again this triangle
                    bAdded = true; //the perimeter has been changed, we'll need another loop
                    break; // no need to go through the rest of the perimeter
                }
            }
        }
        iter++;
    }
    while(bAdded && iter<500);
    Q_ASSERT(iter<500);

    // remove the triangles
    std::sort(reducedlist.begin(), reducedlist.end());
    for(int i=reducedlist.size()-1; i>=0; i--)
    {
        int idx=reducedlist.at(i);
        modtris.append(triangles.at(idx));
        triangles.removeAt(idx);
    }


    // step 2: make the array of unique edges
    PSLG2d edgeList = uniqueEdges(modtris);

    // step 3: insert the new triangles
    for(int i=0; i<edgeList.size(); i++)
    {
        Vector2d AB = newvtx-edgeList.at(i).vertexAt(0);
        Vector2d AC = newvtx-edgeList.at(i).vertexAt(1);
        double det = AB.x*AC.y-AB.y*AC.x;
        Triangle2d t3;
        if(fabs(det)>PRECISION)
        {
            if(det>0.0) t3.setTriangle(newvtx, edgeList.at(i).vertexAt(0), edgeList.at(i).vertexAt(1));
            else        t3.setTriangle(newvtx, edgeList.at(i).vertexAt(1), edgeList.at(i).vertexAt(0));
            if(!t3.isNullTriangle())
            {
                // check whether the edges intersect a PSLG segment
                bool bSplit = false;
                if(bCheckPSLGIntersections)
                {
                    Vector2d I;
                    for(int iedge=0; iedge<3; iedge++)
                    {
                        Segment2d const edge = t3.edge(iedge);
                        int ip = intersectPSLGSegment(edge, I);
                        if(ip>=0)
                        {
                            // split the pslg segment at I
                            Segment2d const &seg = m_LastPSLG.at(ip);
                            PSLG.appendSegment({seg.vertexAt(0), I});
                            PSLG.appendSegment({I, seg.vertexAt(1)});
                            PSLG.removeAt(ip);

                            // split the triangle edge at I to make two triangles
                            // the index of an edge is the index of the opposite vertex
                            Triangle2d t3a = {t3.vertex(iedge), edge.vertexAt(0), I};
                            Triangle2d t3b = {t3.vertex(iedge), edge.vertexAt(1), I};

                            // push the new triangles on the stack
                            addedlist.append(triangles.size());
                            triangles.push_back(t3a);

                            addedlist.append(triangles.size());
                            triangles.push_back(t3b);

                            bSplit = true;
                            break;
                        }
                    }
                }

                if(!bSplit)
                {
                    // the new triangle does not intersect the pslg, push it back on the triangle stack
                    addedlist.append(triangles.size());
                    triangles.push_back(t3);
                }
            }
        }
    }
}


int RuppertMesher::intersectPSLGSegment(Segment2d const &seg2d, Vector2d &I) const
{
    for(int ip=0; ip<m_LastPSLG.size(); ip++)
    {
        Segment2d const &pseg = m_LastPSLG.at(ip);

        // exclude end point case, since end points are existing vertices
        if(pseg.isEndPoint(seg2d.vertexAt(0))) return -1;
        if(pseg.isEndPoint(seg2d.vertexAt(1))) return -1;

        //test if endpoint is on the PSLG, but not an endpoint
        if(seg2d.isOnSegment(pseg.vertexAt(0)))
        {
            I = pseg.vertexAt(0);
            return ip;
        }
        if(seg2d.isOnSegment(pseg.vertexAt(1)))
        {
            I = pseg.vertexAt(1);
            return ip;
        }
        // test for full segment intersection
        if(pseg.intersects(seg2d, I, false, 1.e-5))
        {
            return ip;
        }
    }
    return -1;
}


int RuppertMesher::makeDelaunayTriangulation(QVector<Vector2d> const &vertices, QVector<Triangle2d> &triangles,
                                             Triangle2d &supertriangle, bool bClearSuperTriangles) const
{
    if(vertices.size()<3) return triangles.size();
/*    if(vertices.size()==3)
    {
        triangles.push_back({vertices.at(0), vertices.at(1), vertices.at(2)});
        return triangles.size();
    } */

    //make the supertriangle
    double xmin =  1.e100;
    double xmax = -1.e100;
    double ymin =  1.e100;
    double ymax = -1.e100;

    for(int it=0; it<vertices.size(); it++)
    {
        xmin = std::min(xmin, vertices.at(it).x);
        xmax = std::max(xmax, vertices.at(it).x);
        ymin = std::min(ymin, vertices.at(it).y);
        ymax = std::max(ymax, vertices.at(it).y);
    }
    double cx = (xmax+xmin)/2.0;
    double cy = (ymax+ymin)/2.0;
    double lx = xmax-cx;
    double ly = ymax-cy;

    supertriangle.setTriangle(Vector2d(cx-5.0*lx, cy-5.0*ly), Vector2d(cx-5.0*lx, cy+5.0*ly), Vector2d(cx+10.0*lx, cy));

    triangles.clear();
    triangles.push_back(supertriangle); // needed becuse each vertex is added INSIDE an existing triangle

    PSLG2d pslg;//dummy argument
    QVector<int> removedlist, addedlist;
    for(int iv=0; iv<vertices.size(); iv++)
    {
        if(supertriangle.contains(vertices.at(iv), false))
        {
            for(int it=0; it<triangles.size(); it++)
            {
                if(triangles.at(it).contains(vertices.at(iv), true))
                {
                    insertVertex(it, vertices.at(iv), triangles, removedlist, addedlist, pslg, false);
                    break;
                }
            }
        }
    }

    if(bClearSuperTriangles) clearSuperTriangle(triangles, supertriangle);

    checkDelaunayProperty(triangles);
    return triangles.size();
}


/**
 * @brief Make a constrained Delaunay Triangulation
 * @param vertexList the list of free vertices, may be empty
 * @param PSLG the list of PSLG segments
 * @param nPSLGInsertions debug only, to control the number of insertions
 * @param bClearSuperTriangle debug only to avoid deleting the super triangle
 * @return the list of generated triangles
 */
int RuppertMesher::makeConstrainedDelaunay(QVector<Vector2d> const &vertices, PSLG2d &PSLG,
                                           QVector<Triangle2d> &triangles,
                                           int nPSLGInsertions, bool bClearSuperTriangle) const
{
    //-----------------------------------------------------------------------
    // STEP 1. Make an unconstrained Delaunay triangulation of the vertices,
    //         including those of the PSLG

    Triangle2d supertriangle;
    triangles.clear();
    makeDelaunayTriangulation(vertices, triangles, supertriangle, false);

    //-----------------------------------------------------------------------
    // STEP 2. List the triangles with an edge intersecting a constraint, and
    //         remesh the union polygon of these triangles
    QVector<Triangle2d> intersectedPanels;
    int iInsertions=0;
    for (int ipslg=PSLG.size()-1; ipslg>=0; ipslg--)
    {
        Segment2d seg = PSLG.at(ipslg);

        // List the triangles with an edge intersecting the constraint
        intersectedPanels.clear();
        Vector2d IPt;
        for(int it=triangles.size()-1; it>=0; it--)
        {
            Triangle2d const& t2d = triangles.at(it);
            if(t2d.segmentIntersects(seg, IPt))
            {
                intersectedPanels.append(t2d);
                triangles.removeAt(it);
            }
        }

        if(intersectedPanels.size()>0)
        {
            // extract unique edges from the intersected triangles
            PSLG2d edgelist = uniqueEdges(intersectedPanels);

            // extract the sequence of vertices to make a closed polygon
            QVector<Vector2d> polygon;
            // push the first vertex of the constraint in the polygon
            bool bIsClosed = false;
            int iter=0;
            int max_iter = edgelist.size();
            Vector2d firstVtx = seg.vertexAt(0);
            polygon.push_back(seg.vertexAt(0));
            do
            {
                for(int i=0; i<edgelist.size(); i++)
                {
                    Vector2d lastVtx = polygon.last();
                    if(edgelist.at(i).vertexAt(0).isSame(lastVtx))
                    {
                        polygon.push_back(edgelist.at(i).vertexAt(1));
                        if(edgelist.at(i).vertexAt(1).isSame(firstVtx))
                            bIsClosed = true;
                        edgelist.remove(i);
                        break;
                    }
                    else if(edgelist.at(i).vertexAt(1).isSame(lastVtx))
                    {
                        polygon.push_back(edgelist.at(i).vertexAt(0));
                        if(edgelist.at(i).vertexAt(0).isSame(firstVtx))
                            bIsClosed = true;
                        edgelist.remove(i);
                        break;
                    }
                }
                iter++;
            } while (edgelist.size()>0 && !bIsClosed && iter<max_iter);

            // insert a mid-point on the PSLG segment
            Vector2d midpoint = seg.midPoint();

            // make a fan of triangles joining this mid point
            for(int i=0; i<polygon.count()-1; i++)
            {
                triangles.push_back({polygon.at(i), polygon.at(i+1), midpoint});
            }
            // update the pslg
            PSLG.removeAt(ipslg);
            PSLG.push_back({seg.vertexAt(0), midpoint});
            PSLG.push_back({seg.vertexAt(1), midpoint});


            // make Delaunay sub-triangulation;
            // wrong because the triangles may intersect the PSLG segment
/*            QVector<Vector2d> subvertices = {seg.vertex(0), seg.midPoint(), seg.vertex(1)};
            for(int ie=0; ie<edgelist.size(); ie++)
            {
                Vector2d const& vtx0 = edgelist.at(ie).vertex(0);
                Vector2d const& vtx1 = edgelist.at(ie).vertex(1);

                bool bFound=false;
                for(int iv=0; iv<subvertices.size(); iv++)
                {
                    if(subvertices.at(iv).isSame(vtx0)) {bFound=true;    break;}
                }
                if(!bFound) subvertices.append(vtx0);

                bFound=false;
                for(int iv=0; iv<subvertices.size(); iv++)
                {
                    if(subvertices.at(iv).isSame(vtx1)) {bFound=true;    break;}
                }
                if(!bFound) subvertices.append(vtx1);
            }
            QVector<Triangle2d> newtriangles;
            Triangle2d subsupertriangle;
            makeDelaunayTriangulation(subvertices, newtriangles, subsupertriangle, true);
            clearTrianglesOutsidePSLG(edgelist, newtriangles);
            triangles.append(newtriangles);*/

            iInsertions++;
            if(iInsertions>=nPSLGInsertions) break;
        }
    }

    if(bClearSuperTriangle) clearSuperTriangle(triangles, supertriangle);
    return triangles.size();
}


/**
 * assumes that the inpu pslg is a an ordered closed polygon
 * the difficulty is for concave polygons, for which the fan technique may cause self-intersections*/
bool RuppertMesher::triangulateClosedPolygon(PSLG2d const&pslg, QVector<Triangle2d> triangles)
{
    triangles.clear();

    if(pslg.size()<3) return false;

    PSLG2d wkpslg = pslg; // make a work copy
    int iStart = 0; // start at vertex 0
    while (wkpslg.size()>3)
    {
        Vector2d vtx0 = wkpslg.at(iStart+0).vertexAt(iStart+0);
        Vector2d vtx1 = wkpslg.at(iStart+0).vertexAt(iStart+1);
        Vector2d vtx2 = wkpslg.at(iStart+1).vertexAt(iStart+1);
        Triangle2d t2d = {vtx0, vtx1, vtx2};
        if(wkpslg.contains(t2d.CoG()))
        {
            // convex, keep the triangle
            triangles.push_back(t2d);
            // remove the mid vertex and keep only the segment linking pts 0 and2 to make a smaller polygon
            // need to keep wkpalsg closed and ordered, so modify one segment and discard the other
            wkpslg.removeAt(iStart);
            wkpslg[iStart].setVertices(vtx0, vtx2);
        }
        else {
            // concavity, discard this triangle and start the iterations at the next vertex
            iStart++;
        }
    }

    // add the triangle made of the remaining 3 vertices
    if(pslg.size()==3)
    {
        triangles.push_back({wkpslg.at(0).vertexAt(0), wkpslg.at(0).vertexAt(1), wkpslg.at(1).vertexAt(1)});
        return true;
    }

    return true;
}


/**
 * Ruppert's algorithm starts with a CDT.
 * This methods
 *    - splits the PSLG so that each segment has size greater than m_MaxEdgeLength
 *    - builds the CDT
 *    - removes triangles outside the contour PSLG
 *    - splits oversized triangles
 */
void RuppertMesher::doAllMeshSteps(PSLG2d const &inPSLG, PSLG2d const &contourPSLG, QVector<PSLG2d> innerPSLG)
{
    m_LastTriangles.clear();
    m_LastPSLG = inPSLG;
    m_LastPSLG.cleanNullSegments();

    m_InnerPSLG = innerPSLG;

//    splitOversizePSLG(); //don't, scale is only approximative

    QVector<Vector2d> Vertices;
    addPSLGVertices(Vertices);

    makeConstrainedDelaunay(Vertices, m_LastPSLG, m_LastTriangles, 200, true);

    clearTrianglesInInnerPSLG(innerPSLG);
    clearTrianglesOutsidePSLG(contourPSLG, m_LastTriangles);

    //split oversized triangles
    int iter = 0;
    QVector<int>longtris;
    listLongTriangles(m_LastTriangles, longtris);
    while(longtris.size()>0 && iter<100)
    {
        splitOversizedTriangles(false);
        listLongTriangles(m_LastTriangles, longtris);
        iter++;
    }

    QString log;
    bool bConverged=false;
    doRuppert(contourPSLG, m_MaxMeshIter, m_MaxPanelCount, log, bConverged);
}


QVector<Triangle2d> RuppertMesher::doRuppert(PSLG2d const &contourpslg, int maxiters, int maxpanels,
                                             QString &log, bool &bConverged)
{
    QVector<Triangle2d> triangles;
//    PSLG2d pslg;
    QVector<int>encroachedlist, skinnylist, longTlist;

    QVector<int> removedlist, addedlist; // dummy arguments
    double r=0;
    Vector2d CC;

    triangles = m_LastTriangles;

//    listNonCompliant(triangles, m_LastPSLG, encroachedlist, skinnylist, longTlist);
    listSkinnyTriangles(triangles, skinnylist);

    // refine the PSLG until there are no more encroached PSLG segments
    // nor any skinny triangles,
    // or until skinny triangles have reached the minimum specified size
    int iter = 0;
    do
    {
        //-----------------------------------------------------------------------
        //STEP 1: process encroached PSLG edges
        std::sort(encroachedlist.begin(), encroachedlist.end());
        for(int idx = encroachedlist.size()-1; idx>=0; idx--)
        {
            int ipslg = encroachedlist.at(idx);
            Segment2d seg = m_LastPSLG.at(ipslg);
            Vector2d midpt = seg.midPoint();
            // find a triangle with midpt on its edge
            int idxt=0;
            for(; idxt<triangles.size(); idxt++)
            {
                int iEdge=-1;
                if(triangles.at(idxt).isEdge(seg, iEdge)) break;
            }
            if(idxt<triangles.size())
            {
                m_LastPSLG.remove(ipslg);
                insertVertex(idxt,midpt, triangles,removedlist, addedlist, m_LastPSLG, false);
                m_LastPSLG.appendSegments(seg.split());
            }
            else
            {
            }
            encroachedlist.removeAt(idx);
        }
        if(triangles.size()>m_MaxPanelCount) break;

        //-----------------------------------------------------------------------
        //STEP 2: process skinny triangle
        int iter_sk=0;

        while(skinnylist.size()>0 && iter_sk<1)
        {
            int jdx = skinnylist.last();
            Triangle2d const &t2d = triangles.at(jdx);
            t2d.circumCenter(r, CC);

            //list the segments of the PSLG that this new point encroaches
            bool bEncroaches=false;
            for(int ipslg=0; ipslg<m_LastPSLG.size(); ipslg++)
            {
                Segment2d const &segenc = m_LastPSLG.at(ipslg);
                if(segenc.isEncroachedBy(CC) && segenc.isSplittable())
                {
                    bEncroaches=true;
                    if(!encroachedlist.contains(ipslg))
                    {
                        encroachedlist.push_back(ipslg);
                    }
                }
            }
            if(!bEncroaches)
            {
                // insert the circumcenter of t2d into the triangulation
                // Addition to Ruppert's algorithm:
                //   Before inserting, check that the new point lies inside some existing
                //   triangle because we don't want to extend the mesh outside the closed PSLG.

                bool bInserted = false;
                for(int it=0; it<triangles.size(); it++)
                {
                    if(triangles.at(it).contains(CC, true))
                    {
                        insertVertex(it, CC, triangles, removedlist, addedlist, m_LastPSLG, false);
                        bInserted = true;
                        break;
                    }
                }

                // A special case is when the CC is so far outside the contour PSLG that it does not
                // encroach a segment; find the PSLG segment which is intersected by the segment
                // from the triangle's CoG to the CC and add it to the list of encroached segments
                if(!bInserted)
                {
                    if(!contourpslg.contains(CC))
                    {
                        QVector<int> intersectlist;
                        m_LastPSLG.listIntersected(t2d.CoG(), CC, intersectlist);
                        for(int i=0; i<intersectlist.size(); i++)
                        {
                            int idx = intersectlist.at(i);
                            if(!encroachedlist.contains(idx)) encroachedlist.append(idx);
                        }
                    }
                }
            }

            // update the skinny list;
            // once a skinny triangle has been processed, the whole skinny list is invalid:
            // some triangles in the list may have been deleted, and indexes have changed
//            listSkinnyTriangles(triangles, skinnylist);
            iter_sk++;
            if(s_bCancel) break;
        }
        iter++;

        // check if the new triangles are skinny
        // need to re-check them all becauses indices have changed
        listSkinnyTriangles(triangles, skinnylist);

        if(s_bCancel) break;
        if(triangles.size()>maxpanels) break;
    }
    while((!encroachedlist.isEmpty() || !skinnylist.isEmpty()) && iter<maxiters);

    QString strange;
    if(encroachedlist.isEmpty() && skinnylist.isEmpty())
    {
        bConverged = true;
        strange = QString::asprintf("         mesh has converged in %d iters\n", iter);
    }
    else
    {
        bConverged = false;
        strange = QString::asprintf("          mesh is unconverged after %d iters\n", iter);
    }
    log += strange;
    strange = QString::asprintf("         added %d triangles\n", int(triangles.size()-m_LastTriangles.size()));
    log += strange;

    m_LastTriangles = triangles;
    listNonCompliant(triangles, m_LastPSLG, encroachedlist, skinnylist, longTlist);

    return triangles;
}


int RuppertMesher::splitOversizePSLG()
{
    int nSplit = 0;
    for(int ip=m_LastPSLG.size()-1; ip>=0; ip--)
    {
        Segment2d seg = m_LastPSLG.at(ip);
        double l = seg.length();
        if(l>m_MaxEdgeLength)
        {
            int nsegs = int(std::ceil(l/m_MaxEdgeLength));
            Vector2d vtx0 = seg.vertexAt(0);
            Vector2d vtx1;
            for(int is=1; is<=nsegs; is++)
            {
                vtx1 = seg.vertexAt(0) + seg.unitDir()*l * double(is)/double(nsegs);
                m_LastPSLG.push_back({vtx0, vtx1});
                vtx0 = vtx1;
            }
            m_LastPSLG.removeAt(ip);
        }
    }
    return nSplit;
}


void RuppertMesher::splitPSLGSegment(int index)
{
    Segment2d seg = m_LastPSLG.at(index);
    m_LastPSLG.remove(index);
    PSLG2d splitsegs = seg.split();
    m_LastPSLG.appendSegments(splitsegs);
}


void RuppertMesher::splitTriangle(int index)
{
    if(index<0 || index>=m_LastTriangles.size()) return;

    QVector<int> removedlist, addedlist; // dummy arguments
    Triangle2d t2d = m_LastTriangles.at(index);
    insertVertex(index, t2d.CoG(), m_LastTriangles, removedlist, addedlist, m_LastPSLG, false);
}


int RuppertMesher::splitOversizedTriangles(bool bSplitSPLG)
{
    QVector<int> removedlist, addedlist; // dummy arguments

    if(bSplitSPLG)
    {
        for(int idx=m_LastPSLG.size()-1; idx>=0; idx--)
        {
            Segment2d seg = m_LastPSLG.at(idx);
            double l = seg.length();
            if(l>=m_MaxEdgeLength)
            {
                m_LastPSLG.remove(idx);
                PSLG2d splitsegs = seg.split();
                bool bCheckPSLGIntersections = false;
                for(int it=0; it<m_LastTriangles.size(); it++)
                {
                    if(m_LastTriangles.at(it).contains(seg.midPoint(), true))
                    {
                        insertVertex(it, seg.midPoint(), m_LastTriangles, removedlist, addedlist, m_LastPSLG, bCheckPSLGIntersections);
                        break;
                    }
                }
                m_LastPSLG.appendSegments(splitsegs);
            }
            if(s_bCancel) break;
        }
    }

    int nSplit=0;
    for(int it=m_LastTriangles.size()-1; it>=0; it--)
    {
        Triangle2d const &t2d = m_LastTriangles.at(it);

        int idx_l = -1;
        double l = 0;
        t2d.longestEdge(idx_l, l);

        if(l>m_MaxEdgeLength)
        {
            // In this case, the triangles surrounding the new point
            // may fall either side of a PSLG segment.
            // Need to check if any of the new triangles edges intersects the pslg
            bool bCheckPSLGIntersections = true;

            Vector2d newpt = t2d.edge(idx_l).midPoint();
//                Vector2d newpt = t2d.CoG();

            insertVertex(it, newpt, m_LastTriangles, removedlist, addedlist, m_LastPSLG, bCheckPSLGIntersections);
            nSplit++;
        }
        if(m_LastTriangles.size()>m_MaxPanelCount) return -1;
    }
    return nSplit;
}


int RuppertMesher::flipDelaunay(QVector<int> &fliplist)
{
    fliplist.clear();
    int nFlips = 0;
//    makeConnections(); // no use, connections change at each flip

    for(int it=0; it<m_LastTriangles.size(); it++)
    {
        Triangle2d &t2d = m_LastTriangles[it];
        for(int it2=it+1; it2<m_LastTriangles.size(); it2++)
        {
            Triangle2d &t2n = m_LastTriangles[it2];
            for(int ie0=0; ie0<3; ie0++)
            {
                Segment2d testedge = t2d.edge(ie0);

                int ien = -1;
                if(t2n.isEdge(testedge, ien))
                {
                    if(ien<0) break; // some error somewhere

                    // compare opposite angles
                    double alpha0 = t2d.angle(ie0);
                    double alphan = t2n.angle(ien);
                    if(alpha0+alphan>180.0)
                    {
                        //perform Delaunay flip
                        fliplist.append(it);     // info only
                        fliplist.append(it2);    // info only
                        Vector2d vtx0 = t2d.vertex(ie0);
                        Vector2d vtxn = t2n.vertex(ien);
                        t2d.setTriangle(vtx0, vtxn, testedge.vertexAt(0));
                        t2n.setTriangle(vtx0, testedge.vertexAt(1), vtxn);
                        nFlips++;
                    }
                }
            }
        }
    }
    return nFlips;
}


void RuppertMesher::listNonCompliant(QVector<int>&encroachedSegs, QVector<int> &skinnyTris, QVector<int> &longTris) const
{
    listNonCompliant(m_LastTriangles, m_LastPSLG, encroachedSegs, skinnyTris, longTris);
}


void RuppertMesher::listNonCompliant(QVector<Triangle2d> const &triangles, PSLG2d const &PSLG,
                                     QVector<int>&encroachedSegs, QVector<int> &skinnytris, QVector<int> &longtris) const
{
    encroachedSegs.clear();
    skinnytris.clear();
    longtris.clear();

    listEncroachedSegments(PSLG, triangles, encroachedSegs);

    listSkinnyTriangles(triangles, skinnytris);

    listLongTriangles(triangles, longtris);
}


/** skinny triangles with length less than the specified size are not counted as skinny */
int RuppertMesher::listSkinnyTriangles(QVector<Triangle2d> const &triangles, QVector<int> &skinnylist) const
{
    skinnylist.clear();
    //list skinny triangles
    for(int it=0; it<triangles.size(); it++)
    {
        Triangle2d const &t2d = triangles.at(it);
        if(t2d.isSkinny())
        {
            // to prevent further splitting, a triangle is not counted as skinny if
            //    - it is smaller than the min. size
            //    - its CC encroaches a non-splittable PSLG segment
            if(t2d.maxEdgeLength()>m_MinEdgeLength)
            {
            }
            else
            {
                double r=0;
                Vector2d CC;
                t2d.circumCenter(r, CC);
                bool bTriSplittable = true;
                for(int is=0; is<m_LastPSLG.size(); is++)
                {
                    Segment2d const &seg =  m_LastPSLG.at(is);
                    if(!seg.isSplittable() && seg.isEncroachedBy(CC))
                    {
                        bTriSplittable = false;
                        break;
                    }
                }
                if(bTriSplittable && !skinnylist.contains(it))
                {
                    //this skinny triangle is splittable
                    skinnylist.push_back(it);
                }
            }
        }
    }
    // sort them so that we can remove them by decrescending order at the next step
    std::sort(skinnylist.begin(), skinnylist.end());
    return skinnylist.size();
}


/** list long triangles */
int RuppertMesher::listLongTriangles(QVector<Triangle2d> const &triangles, QVector<int> &longtris) const
{
    longtris.clear();
    for(int it=0; it<triangles.size(); it++)
    {
        if(triangles.at(it).isLong(m_MaxEdgeLength))
        {
            if(!longtris.contains(it)) longtris.push_back(it);
        }
    }
    std::sort(longtris.begin(), longtris.end());
    return longtris.size();
}


/** Lists PSLG segments encroached by skinny triangles */
int RuppertMesher::listEncroachedSegments(PSLG2d const &PSLG, QVector<Triangle2d>const &triangles, QVector<int>&encroachedSegs) const
{
    encroachedSegs.clear();
    Vector2d CC;
    double r=0;
    for(int ipslg = PSLG.size()-1; ipslg>=0; ipslg--)
    {
        Segment2d seg = PSLG.at(ipslg);
        for(int it=0; it<triangles.size(); it++)
        {
            Triangle2d const &t2d = triangles.at(it);
            if(t2d.isSkinny())
            {
                t2d.circumCenter(r, CC);
                if(seg.isEncroachedBy(CC))
                {
                    if(!encroachedSegs.contains(ipslg))
                        encroachedSegs.push_back(ipslg);
                    // the segment is encroached, no need to continue checking other triangles
                    break;
                }
            }
        }
    }
    // sort the segements so that we can remove them by decrescending order at the next step
    std::sort(encroachedSegs.begin(), encroachedSegs.end());
    return encroachedSegs.size();
}







