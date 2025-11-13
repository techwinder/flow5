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




#include "triangle3d.h"

#include <xflgeom/geom3d/segment3d.h>
#include <xflgeom/geom_globals/geom_global.h>
#include <xflgeom/geom_globals/ruppertmesher.h>
#include <xflmath/constants.h>
#include <xflmath/matrix.h>


Triangle3d::Triangle3d()
{
    initialize();
}


/** The copy constructor, slow and useless */
/*Triangle3d::Triangle3d(Triangle3d const &triangle)
{
    initialize();
    m_S[0] = triangle.vertexAt(0);
    m_S[1] = triangle.vertexAt(1);
    m_S[2] = triangle.vertexAt(2);

    setTriangle();
}*/


Triangle3d::Triangle3d(Vector3d const *vertex)
{
    initialize();
    m_S[0] = vertex[0];
    m_S[1] = vertex[1];
    m_S[2] = vertex[2];
    setTriangle();
}


/** constructs the triangle's properties from its vertices; leaves the orientation unchanged */
Triangle3d::Triangle3d(Node const& vtx0, Node const& vtx1, Node const& vtx2)
{
    initialize();
    m_S[0] = vtx0;
    m_S[1] = vtx1;
    m_S[2] = vtx2;
    setTriangle();
}


void Triangle3d::setTriangle(Node const& vtx0, Node const& vtx1, Node const& vtx2)
{
    m_S[0].setNode(vtx0);
    m_S[1].setNode(vtx1);
    m_S[2].setNode(vtx2);
    setTriangle();
}


void Triangle3d::setTriangle()
{
    S01.x = m_S[1].x-m_S[0].x;
    S01.y = m_S[1].y-m_S[0].y;
    S01.z = m_S[1].z-m_S[0].z;
    S02.x = m_S[2].x-m_S[0].x;
    S02.y = m_S[2].y-m_S[0].y;
    S02.z = m_S[2].z-m_S[0].z;
    S12.x = m_S[2].x-m_S[1].x;
    S12.y = m_S[2].y-m_S[1].y;
    S12.z = m_S[2].z-m_S[1].z;

    m_Edge[0].setNodes(m_S[1],m_S[2]);
    m_Edge[1].setNodes(m_S[2],m_S[0]);
    m_Edge[2].setNodes(m_S[0],m_S[1]);

    m_Normal = m_Edge[1].segment() * m_Edge[2].segment();
    m_Normal.normalize();

    m_CoG_g.x = (m_S[0].x+m_S[1].x+m_S[2].x)/3.0;
    m_CoG_g.y = (m_S[0].y+m_S[1].y+m_S[2].y)/3.0;
    m_CoG_g.z = (m_S[0].z+m_S[1].z+m_S[2].z)/3.0;

    // set the origin at the CoG
    // to compare integrals with MC's method, set the origin at vertex 0
    O.x = m_CoG_g.x;
    O.y = m_CoG_g.y;
    O.z = m_CoG_g.z;

    if(m_Edge[0].length()<LENGTHPRECISION || m_Edge[1].length()<LENGTHPRECISION || m_Edge[2].length()<LENGTHPRECISION)
    {
        // one null side
        m_SignedArea = 0.0;
        m_bNullTriangle = true;
        return;
    }

    double sinthet0 = (S01*S02).norm() / S01.norm()/S02.norm();
    double sinthet1 = (S12*S01).norm() / S12.norm()/S01.norm();
    double sinthet2 = (S02*S12).norm() / S02.norm()/S12.norm();
    double anglemin = 0.1/180*PI;

    if(sinthet0<anglemin || sinthet1<anglemin || sinthet2<anglemin)
    {
        // the three sides are colinear, flat triangle
        m_SignedArea = 0.0;
        m_bNullTriangle = true;
        return;
    }

    m_bNullTriangle = false;


    //define the local frame of reference
    // S01 defines the x-axis
    l = m_Edge[2].segment().normalized();
    m = m_Normal*l;

    m_CF.setOrigin(O);
    m_CF.setIJK(l,m,m_Normal);

    m_Sl[0] = m_CF.globalToLocal(m_S[0]-m_CoG_g);
    m_Sl[1] = m_CF.globalToLocal(m_S[1]-m_CoG_g);
    m_Sl[2] = m_CF.globalToLocal(m_S[2]-m_CoG_g);

    m_CoG_l  = globalToLocal(m_CoG_g-m_CoG_g);

    m_triangle2d.setTriangle(Vector2d(m_Sl[0].x, m_Sl[0].y), Vector2d(m_Sl[1].x, m_Sl[1].y), Vector2d(m_Sl[2].x, m_Sl[2].y));

    // calculate the matrix to transform local coordinates in homogeneous barycentric coordinates
    gmat[0] = 1.0;     gmat[1] = m_Sl[0].x;     gmat[2] = m_Sl[0].y;
    gmat[3] = 1.0;     gmat[4] = m_Sl[1].x;     gmat[5] = m_Sl[1].y;
    gmat[6] = 1.0;     gmat[7] = m_Sl[2].x;     gmat[8] = m_Sl[2].y;
    transpose33(gmat);
    invert33(gmat);

    m_S01l = globalToLocal(m_Edge[2].segment());
    m_S02l = globalToLocal(m_Edge[1].segment());
    m_S12l = globalToLocal(m_Edge[0].segment());

    m_SignedArea = (m_Edge[1].segment()*m_Edge[2].segment()).dot(m_Normal)/2.0;
}


QString Triangle3d::properties(bool bLong) const
{
    QString props;
    QString strange;

    if(m_bNullTriangle) props +="   ***** NULL triangle *****\n";

    strange = QString::asprintf("  Area    = %11g ", m_SignedArea);
    props += strange + "\n";

    strange = QString::asprintf("  Max. edge length = %9g ", maxEdgeLength());
    strange += + "\n";
    props += strange;

    strange = QString::asprintf("  Min. edge length = %9g ", minEdgeLength());
    strange +=+ "\n";
    props += strange;

    if(bLong)
    {
        strange = QString::asprintf("  CoG     = (%7g, %7g, %7g) ", CoG_g().x, CoG_g().y, CoG_g().z);
        strange += "\n";
        props += strange;

        for(int in=0; in<3; in++)
        {
            strange = QString::asprintf("  Node(%d) = (%7g, %7g, %7g) ", in,
                            m_S[in].x, m_S[in].y, m_S[in].z);
            strange +=  + "\n";
            props += strange;
        }

        strange = QString::asprintf("  Vertex indexes:  %4d  %4d  %4d\n", m_S[0].index(), m_S[1].index(), m_S[2].index());
        props += strange;

        strange = QString::asprintf("  Neighbours:      %4d  %4d  %4d\n", m_Neighbour[0], m_Neighbour[1], m_Neighbour[2]);
        props += strange;
    }

    return props;
}


void Triangle3d::flipXZ()
{
    for(int i=0; i<3; i++)
    {
        m_S[i].y = -m_S[i].y;
    }
    setTriangle();
}


void Triangle3d::scale(double xscalefactor, double yscalefactor, double zscalefactor)
{
    for(int i=0; i<3; i++)
    {
        m_S[i].x *=xscalefactor;
        m_S[i].y *=yscalefactor;
        m_S[i].z *=zscalefactor;

    }
    setTriangle();
}


void Triangle3d::translate(double tx, double ty, double tz)
{
    for(int i=0; i<3; i++)
    {
        m_S[i].x += tx;
        m_S[i].y += ty;
        m_S[i].z += tz;
    }
    setTriangle();
}


void Triangle3d::rotate(Vector3d const &center, Vector3d const& axis, double angle)
{
    for(int i=0; i<3; i++)
    {
        m_S[i].rotate(center, axis, angle);
    }
    setTriangle();
}


/**
 * Changes the orientation of the triangle by swapping two vertices and recalculating
 * the normal and all other properties.
 */
void Triangle3d::reverseOrientation()
{
    Node tmp = m_S[1];
    m_S[1].setNode(m_S[2]);
    m_S[2].setNode(tmp);
    setTriangle();
}


void Triangle3d::displayNodes(const QString &msg) const
{
    QString strong, str;
    str = QString::asprintf("Vec( %9.5f, %9.5f, %9.5f ), Vec( %9.5f, %9.5f, %9.5f ), Vec( %9.5f, %9.5f, %9.5f )",
                m_S[0].x, m_S[0].y, m_S[0].z, m_S[1].x, m_S[1].y, m_S[1].z, m_S[2].x, m_S[2].y, m_S[2].z);
    strong = msg+ " " +str;
    qDebug("%s", strong.toStdString().c_str());
}


/**
* Finds the intersection point of a line segment with the triangle.
* The segement is defined by its two endpoints
* @param A the segment's first end point
* @param B the segment's second end point
* @param I the intersection point
* @return true if the intersection point exists and lies inside the panel false otherwise
*/
bool Triangle3d::intersectSegmentInside(Vector3d const &A, Vector3d const &B, Vector3d &I, bool bEdgesInside) const
{
    if(isNull()) return false;

    Vector3d U = B-A;

    double r = (m_CoG_g.x-A.x)*m_Normal.x + (m_CoG_g.y-A.y)*m_Normal.y + (m_CoG_g.z-A.z)*m_Normal.z ;
    double s = U.x*m_Normal.x + U.y*m_Normal.y + U.z*m_Normal.z;

    if(fabs(s)>0.0)
    {
        double dist = r/s;

        if(dist>1 || dist<0) return false; // the intersection point is not in the segment [A,B]

        // intersection of ray with plane
        I.x = A.x + U.x * dist;
        I.y = A.y + U.y * dist;
        I.z = A.z + U.z * dist;

        // check if inside triangle
        double g[] = {0,0,0};
        double xl=0, yl=0, zl=0;
        globalToLocalPosition(I.x, I.y, I.z, xl, yl, zl);
        barycentricCoords(xl, yl, g);
        double gmin = bEdgesInside ?    -1.0e-6 : 0.0;
        double gmax = bEdgesInside ? 1.0+1.0e-6 : 1.0;
        return gmin<g[0] && g[0]<gmax && gmin<g[1] && g[1]<gmax && gmin<g[2] && g[2]<gmax;
    }

    return false;
}


/**
 * Tests if the projection of the segment on the triangle's plane intersects, or if this segments lies inside.
 * Not interested in the intersection point or segment itself
*/
bool Triangle3d::intersectSegmentProjection(Segment3d const &seg) const
{
    Vector3d A, B;
    Vector2d Al, Bl, I;
    // convert everything to 2d - segments don't intersect in 3d
    normalProjection(seg.vertexAt(0), A);
    normalProjection(seg.vertexAt(1), B);
    A = globalToLocalPosition(A);
    B = globalToLocalPosition(B);
    Al.set(A.x, A.y);
    Bl.set(B.x, B.y);
    Segment2d seg2d(Al, Bl);
    for(int i=0; i<3; i++)
        if(seg2d.intersects({m_Sl[i].x, m_Sl[i].y}, {m_Sl[(i+1)%3].x, m_Sl[(i+1)%3].y}, I, false, 1.e-5)) return true;

    /** @todo for completeness, test if the projected segment lies entirely inside the triangle */
    return false;
}

/**
* Finds the intersection point of a ray with the triangle.
* The ray is defined by a point and a direction vector.
* @param A the ray's origin
* @param U the ray's direction, normalization is not necessary
* @param I the intersection point
* @return true if the intersection point lies inside the panel false otherwise
*/
bool Triangle3d::intersectRayInside(Vector3d const &A, Vector3d const &U, Vector3d &I) const
{
    if(isNull()) return false;

/*    double Nx =  (m_Vertex[1].y-m_Vertex[0].y) * (m_Vertex[2].z-m_Vertex[0].z) - (m_Vertex[1].z-m_Vertex[0].z) * (m_Vertex[2].y-m_Vertex[0].y);
    double Ny = -(m_Vertex[1].x-m_Vertex[0].x) * (m_Vertex[2].z-m_Vertex[0].z) + (m_Vertex[1].z-m_Vertex[0].z) * (m_Vertex[2].x-m_Vertex[0].x);
    double Nz =  (m_Vertex[1].x-m_Vertex[0].x) * (m_Vertex[2].y-m_Vertex[0].y) - (m_Vertex[1].y-m_Vertex[0].y) * (m_Vertex[2].x-m_Vertex[0].x);*/
    double Nx = m_Normal.x;
    double Ny = m_Normal.y;
    double Nz = m_Normal.z;

    double r = (m_CoG_g.x-A.x)*Nx + (m_CoG_g.y-A.y)*Ny + (m_CoG_g.z-A.z)*Nz ;
    double s = U.x*Nx + U.y*Ny + U.z*Nz;

    double dist = 10000.0;

    if(fabs(s)>0.0)
    {
        dist = r/s;
        I.x = A.x + U.x * dist;
        I.y = A.y + U.y * dist;
        I.z = A.z + U.z * dist;

        double g[] = {0,0,0};
        double xl=0, yl=0, zl=0;

        globalToLocalPosition(I.x, I.y, I.z, xl, yl, zl);
        barycentricCoords(xl, yl, g);
        return 0<=g[0] && g[0]<=1.0 && 0<=g[1] && g[1]<=1.0 && 0<=g[2] && g[2]<=1.0;
    }
    return false;
}



/** Splits the edges with the section delimited by the rays */
void Triangle3d::splitTriangle(QVector<Segment3d> const & raylist,
                               QVector<Segment3d> &polygon3d, QVector<Segment3d>&PSLG3d,
                               QVector<Triangle3d> &trianglelist, bool bLeftSide) const
{
    // find the intersection points of the rays with the triangle's plane, and make the PSLG
    Vector3d I3d;
    QVector<Vector3d> pointlist;
    for(int i=0; i<raylist.count(); i++)
    {
        Segment3d ray = raylist.at(i);
        if(intersectRayPlane(ray.vertexAt(0), ray.unitDir(), I3d)) pointlist.push_back(I3d);
    }

    // convert the PSLG to 2d in the triangle's plane
    Vector2d I2d;
    PSLG2d PSLG;

    if(pointlist.count()>0)
    {
        Vector3d I0 = globalToLocalPosition(pointlist.at(0));
        for(int i=1; i<pointlist.count(); i++)
        {
            I3d = globalToLocalPosition(pointlist.at(i));
            PSLG.push_back({I0.x, I0.y, I3d.x, I3d.y});
            I0 = I3d;
        }
    }

    // from here onwards, work in 2d in the local reference frame
    PSLG2d refPSLG = PSLG;

    // split the pslg at the reference edges
    /*    for(int iedge=0; iedge<3; iedge++)
    {
        Segment2d const & edge = m_triangle2d.edge(iedge);
        for(int k=PSLG.count()-1; k>=0; k--)
        {
            Segment2d const & seg = PSLG.at(k);
            if(edge.intersects(seg, I2d))
            {
                PSLG.push_back({PSLG.at(k).vertex(0), I2d});
                PSLG.push_back({I2d, PSLG.at(k).vertex(1)});
                PSLG.removeAt(k);
            }
        }
    }*/

    // split the triangle edges with the reference PSLG
    PSLG2d polygon;
    for(int i=0; i<3; i++) polygon.push_back(m_triangle2d.edge(i));

    for(int k=0; k<refPSLG.count(); k++)
    {
        Segment2d const &seg = refPSLG.at(k);
        for(int ip=polygon.size()-1; ip>=0; ip--)
        {
            Segment2d &edge = polygon[ip];
            if(seg.intersects(edge, I2d, false, 1.e-5))
            {
                // move I2d to the closest point of the segment
                double d02 = (I2d.x-seg.vertexAt(0).x)*(I2d.x-seg.vertexAt(0).x) + (I2d.y-seg.vertexAt(0).y)*(I2d.y-seg.vertexAt(0).y);
                double d12 = (I2d.x-seg.vertexAt(1).x)*(I2d.x-seg.vertexAt(1).x) + (I2d.y-seg.vertexAt(1).y)*(I2d.y-seg.vertexAt(1).y);
                if(sqrt(d02)<PRECISION || sqrt(d12)<PRECISION)
                {
                    // no need to do anything, we already have coincident points;
                }
                else
                {
                    if(d02<d12) I2d = seg.vertexAt(0);
                    else        I2d = seg.vertexAt(1);

                    // split the edge
                    Segment2d half0(edge.vertexAt(0), I2d);
                    Segment2d half1(I2d, edge.vertexAt(1));

                    // one of these two half-edges is inside the pslg, so don't add it
                    //                    if(!isInPolygon(refPSLG, edge.vertex(0)) || !isInPolygon(refPSLG, I2d))
                    polygon.push_back(half0);
                    //                    if(!isInPolygon(refPSLG, edge.vertex(1)) || !isInPolygon(refPSLG, I2d))
                    polygon.push_back(half1);

                    // remove the old edge
                    polygon.removeAt(ip);

                }
            }
        }
    }

    //    for(int pol=0; pol<polygon.count(); pol++) polygon.at(pol).displayNodes(" poly= ");

    // remove pslg segments which have at least one vertex outside the polygon=former triangle
    for(int i=PSLG.size()-1; i>=0; i--)
    {
        if(!polygon.contains(PSLG.at(i).vertexAt(0)) || !polygon.contains(PSLG.at(i).vertexAt(1)))
        {
            PSLG.removeAt(i);
        }
    }


    //mesh the 2d triangle with the 2d PSLG
    RuppertMesher mesher;
    Triangle2d::setQualityBound(sqrt(2.0));
    mesher.setMaxIter(2);
    mesher.setMaxEdgeLength(1.0);

    // add the triangle perimeter to the ray's PSLG
    for(int i=0; i<polygon.size(); i++) PSLG.push_back(polygon.at(i));

    QString logmsg;
    bool bConverged=false;
    mesher.doAllMeshSteps(PSLG, PSLG, QVector<PSLG2d>());
    QVector<Triangle2d> triangles2d = mesher.doRuppert(PSLG, 50, 100, logmsg, bConverged);

    //clean all triangles inside the PSLG
    for(int it2=triangles2d.count()-1; it2>=0; it2--)
    {
        if(refPSLG.contains(triangles2d.at(it2).CoG())) triangles2d.removeAt(it2);
    }

    // the triangle's perimeter may have become concave, so remove also all triangles outside the polygon
    for(int it2=triangles2d.count()-1; it2>=0; it2--)
    {
        if(!polygon.contains(triangles2d.at(it2).CoG())) triangles2d.removeAt(it2);
    }

    //make and return the 3d triangle list
    Vector3d m_Vertex[3];

    for(int it2=0; it2<triangles2d.size(); it2++)
    {
        for(int iv=0; iv<3; iv++)
        {
            localToGlobalPosition(triangles2d.at(it2).vertex(iv).x, triangles2d.at(it2).vertex(iv).y, 0.0,
                                  m_Vertex[iv].x, m_Vertex[iv].y, m_Vertex[iv].z);

            // connect to rays
            for(int i=0; i<raylist.size(); i++)
            {
                if(distanceToLine3d(raylist.at(i).vertexAt(0), raylist.at(i).vertexAt(1), m_Vertex[iv])<PRECISION)
                {
                    m_Vertex[iv] = raylist.at(i).vertexAt(1);
                    break;
                }
            }
        }
        if(bLeftSide) trianglelist.push_back({m_Vertex[0], m_Vertex[1], m_Vertex[2]});
        else          trianglelist.push_back({m_Vertex[0], m_Vertex[2], m_Vertex[1]});
    }



    //convert PSLG and Polygon to 3d, for display information only
    PSLG3d.clear();
    for(int i=0; i<PSLG.size(); i++)
    {
        Vector3d v0(PSLG.at(i).vertexAt(0).x, PSLG.at(i).vertexAt(0).y, 0.0);
        Vector3d v1(PSLG.at(i).vertexAt(1).x, PSLG.at(i).vertexAt(1).y, 0.0);
        Vector3d V0 =localToGlobalPosition(v0);
        Vector3d V1 =localToGlobalPosition(v1);
        PSLG3d.push_back({V0, V1});
    }

    polygon3d.clear();
    for(int i=0; i<polygon.size(); i++)
    {
        Vector3d v0(polygon.at(i).vertexAt(0).x, polygon.at(i).vertexAt(0).y, 0.0);
        Vector3d v1(polygon.at(i).vertexAt(1).x, polygon.at(i).vertexAt(1).y, 0.0);
        Vector3d V0 =localToGlobalPosition(v0);
        Vector3d V1 =localToGlobalPosition(v1);
        polygon3d.push_back({V0, V1});
    }
}


void Triangle3d::splitIn4Triangles(QVector<Triangle3d> &trianglelist) const
{
    trianglelist.resize(4);
    trianglelist[0].setTriangle(m_Edge[1].midPoint(), m_S[0], m_Edge[2].midPoint());
    trianglelist[1].setTriangle(m_Edge[2].midPoint(), m_S[1], m_Edge[0].midPoint());
    trianglelist[2].setTriangle(m_Edge[0].midPoint(), m_S[2], m_Edge[1].midPoint());
    trianglelist[3].setTriangle(m_Edge[2].midPoint(), m_Edge[0].midPoint(), m_Edge[1].midPoint());
}


void Triangle3d::splitIn3Triangles(Vector3d const &ptinside, QVector<Triangle3d> &trianglelist) const
{
    trianglelist.resize(3);
    trianglelist[0].setTriangle(ptinside, m_S[0], m_S[1]);
    trianglelist[0].setTriangle(ptinside, m_S[1], m_S[2]);
    trianglelist[0].setTriangle(ptinside, m_S[2], m_S[0]);
}


double Triangle3d::minEdgeLength() const
{
    double l=1.e10;
    for (int ie=0; ie<3; ie++)
    {
        l = std::min(l, edge(ie).length());
    }
    return l;
}


double Triangle3d::maxEdgeLength() const
{
    double l=0;
    for (int ie=0; ie<3; ie++)
    {
        l = std::max(l, edge(ie).length());
    }
    return l;
}


double Triangle3d::qualityFactor(double &r, double &shortestEdge) const
{
    double a = m_Edge[0].length();
    double b = m_Edge[1].length();
    double c = m_Edge[2].length();
    shortestEdge = a;
    shortestEdge = std::min(shortestEdge, b);
    shortestEdge = std::min(shortestEdge, c);

    // find circumradius
    r = a*b*c / sqrt((a+b+c)*(b+c-a)*(c+a-b)*(a+b-c));

    return r/shortestEdge;
}


int Triangle3d::edgeIndex(Segment3d const &seg, double precision) const
{
    for(int iEdge=0; iEdge<3; iEdge++)
    {
        if(m_Edge[iEdge].isSame(seg, precision)) return iEdge;
    }
    return -1;
}



