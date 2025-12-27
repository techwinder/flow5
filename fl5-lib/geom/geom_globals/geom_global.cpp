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


#include <thread>
#include <future>

#include <QDebug>

#include <geom_global.h>

#include <constants.h>
#include <nurbssurface.h>
#include <quaternion.h>
#include <segment2d.h>
#include <spline.h>
#include <triangle3d.h>
#include <node.h>

bool geom::intersectXYPlane(Vector3d const &A,  Vector3d const &U,  Vector3d &I)
{
    Vector3d N(0.0,0.0,1.0);

    double r = (-A.x)*N.x + (-A.y)*N.y + (-A.z)*N.z ;
    double s = U.x*N.x + U.y*N.y + U.z*N.z;

    double dist = 10000.0;

    if(fabs(s)>0.0)
    {
        dist = r/s;

        //inline operations to save time
        I.x = A.x + U.x * dist;
        I.y = A.y + U.y * dist;
        I.z = A.z + U.z * dist;

        return true;
    }
    return false;
}


/**
* Returns the intersection of a ray with the object's panels
* The ray is defined by a position and a direction
*    A is the ray's origin,
*    U is the ray's direction
*    LA, LB, TA, TB define a quadrangle in 3d space.
*    N is the normal to the quadrangle
*    I is the resulting intersection point of the ray and the quadrangle, if inside the quadrangle
*    dist = |AI|
*    The return value is true if the intersection inside the quadrangle, false otherwise
*
* if the flag bDirOnly is set to true, the intersection must be in the direction of the unit vector
**/
bool geom::intersectQuad3d(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB,
                     Vector3d const &A,  Vector3d const &U,  Vector3d &I, bool bDirOnly)
{
    Vector3d P, W, V, T;
    Vector3d N; // TATB * LATA
    N.x =  (LB.y-TA.y) * (LA.z-TB.z) - (LB.z-TA.z) * (LA.y-TB.y);
    N.y = -(LB.x-TA.x) * (LA.z-TB.z) + (LB.z-TA.z) * (LA.x-TB.x);
    N.z =  (LB.x-TA.x) * (LA.y-TB.y) - (LB.y-TA.y) * (LA.x-TB.x);

    double precision=1.e-10;

    double r = (LA.x-A.x)*N.x + (LA.y-A.y)*N.y + (LA.z-A.z)*N.z ;
    double s = U.x*N.x + U.y*N.y + U.z*N.z;

    double dist = 10000.0;

    if(fabs(s)>0.0)
    {
        dist = r/s;
        if(bDirOnly)
        {
            if (dist<LENGTHPRECISION) return false;
        }
        //inline operations to save time
        P.x = A.x + U.x * dist;
        P.y = A.y + U.y * dist;
        P.z = A.z + U.z * dist;

        // P is inside the panel if on the left side of each panel side

        // first edge is TATB
        V.x = TB.x - TA.x;      W.x = P.x - TA.x;
        V.y = TB.y - TA.y;      W.y = P.y - TA.y;
        V.z = TB.z - TA.z;      W.z = P.z - TA.z;
        T.x =  V.y * W.z - V.z * W.y;
        T.y = -V.x * W.z + V.z * W.x;
        T.z =  V.x * W.y - V.y * W.x;
        if(T.x*N.x+T.y*N.y+T.z*N.z>=-precision)
        {
            //second edge is TBLB
            V.x = LB.x - TB.x;      W.x = P.x - TB.x;
            V.y = LB.y - TB.y;      W.y = P.y - TB.y;
            V.z = LB.z - TB.z;      W.z = P.z - TB.z;

            T.x =  V.y * W.z - V.z * W.y;
            T.y = -V.x * W.z + V.z * W.x;
            T.z =  V.x * W.y - V.y * W.x;
            if(T.x*N.x+T.y*N.y+T.z*N.z>=-precision)
            {
                //third edge is LBLA
                V.x = LA.x - LB.x;      W.x = P.x - LB.x;
                V.y = LA.y - LB.y;      W.y = P.y - LB.y;
                V.z = LA.z - LB.z;      W.z = P.z - LB.z;
                T.x =  V.y * W.z - V.z * W.y;
                T.y = -V.x * W.z + V.z * W.x;
                T.z =  V.x * W.y - V.y * W.x;
                if(T.x*N.x+T.y*N.y+T.z*N.z>=-precision)
                {
                    //last edge is LATA
                    V.x = TA.x - LA.x;      W.x = P.x - LA.x;
                    V.y = TA.y - LA.y;      W.y = P.y - LA.y;
                    V.z = TA.z - LA.z;      W.z = P.z - LA.z;

                    T.x =  V.y * W.z - V.z * W.y;
                    T.y = -V.x * W.z + V.z * W.x;
                    T.z =  V.x * W.y - V.y * W.x;
                    if(T.x*N.x+T.y*N.y+T.z*N.z>=-precision)
                    {
                        I.set(P.x, P.y, P.z);
                        return true;

                    }
                }
            }
        }
    }
    return false;
}


/**
* @return true and intersection point M if AB and CD intersect inside, false and intersection point M if AB and CD intersect outside
*/
bool geom::intersect(Vector2d const &A, Vector2d const &B, Vector2d const &C, Vector2d const &D, Vector2d &M)
{
    double Det{0}, Det1{0}, Det2{0}, t{0}, u{0};
    Vector2d AB, CD;

    M.x = 0.0;
    M.y = 0.0;

    AB.set(B.x-A.x, B.y-A.y);
    CD.set(D.x-C.x, D.y-C.y);

    //Cramer's rule

    Det  = -AB.x * CD.y + CD.x * AB.y;
    if(Det==0.0)
    {
        //vectors are parallel, no intersection
        return false;
    }
    Det1 = -(C.x-A.x)*CD.y + (C.y-A.y)*CD.x;
    Det2 = -(C.x-A.x)*AB.y + (C.y-A.y)*AB.x;

    t = Det1/Det;
    u = Det2/Det;

    M.x = A.x + t*AB.x;
    M.y = A.y + t*AB.y;

    if (0.0<=t && t<=1.0 && 0.0<=u && u<=1.0) return true;//M is between A and B
    else                                      return false;//M is outside
}


/**
 * Returns true if [A0,A1] intersects [B0,B1], false otherwise
 * Parallel overlapping segments are considered to intersect.
 * The default precision is set to 1/100 mmm because the max error returned by OCC on test cases is ~1.e-6
 */
bool geom::intersectSegment(Vector2d const &A0, Vector2d const &A1,
                      Vector2d const &B0, Vector2d const &B1,
                      Vector2d &IPt, bool bPointsInside, double vtx_precision)
{
    double rx = A1.x-A0.x;
    double ry = A1.y-A0.y;

    double sx = B1.x-B0.x;
    double sy = B1.y-B0.y;

    double rXs = rx*sy - ry*sx; // two dimensional cross product

    double sinTheta = rXs / sqrt(rx*rx+ry*ry) / sqrt(sx*sx+sy*sy);

    if(A0.isSame(B0, vtx_precision)) return bPointsInside;
    if(A0.isSame(B1, vtx_precision)) return bPointsInside;
    if(A1.isSame(B0, vtx_precision)) return bPointsInside;
    if(A1.isSame(B1, vtx_precision)) return bPointsInside;

    if(fabs(sinTheta)<1.e-4)
    {
        // the two lines are parallel
        double qpXr = (B0.x-A0.x)*ry - (B0.y-A0.y)*rx;
        if(fabs(qpXr)<LENGTHPRECISION)
        {
            //the two lines are co-linear
            double r2 = rx*rx + ry*ry;
            double t0 = ((B0.x-A0.x)*rx + (B0.y-A0.y)*ry )/r2;
            double t1 = t0 + (sx*rx+sy*ry)/r2;
            if(t0>vtx_precision && t0<1.0-vtx_precision)
            {
                return false; // overlapping
            }
            if(t1>vtx_precision && t1<1.0-vtx_precision)
            {
                return true; // overlapping
            }
            // Colinear not overlapping;
            return false;
        }
        else
        {
            //parallel;
            return false;
        }
    }
    else
    {
        double qpXr = (B0.x-A0.x)*ry - (B0.y-A0.y)*rx;
        double qpXs = (B0.x-A0.x)*sy - (B0.y-A0.y)*sx;
        double t = qpXs / rXs;
        double u = qpXr / rXs;

        IPt.x = A0.x + t*rx;
        IPt.y = A0.y + t*ry;
        // note: using t>precision etc. excludes coincident endpoints
        //       and also endpoints on segment
        // --> handle limit cases in calling functions

        if (vtx_precision<t && t<1.0-vtx_precision && vtx_precision<u && u<1.0-vtx_precision)
        {
            return true; // clearly inside on both segments
        }
        if(fabs(t)<vtx_precision || fabs(t-1.0)<vtx_precision)
        {
            // the segments meet at an end point
            // check on the other segment
            if(vtx_precision<u && u<1.0-vtx_precision)
            {
                return bPointsInside;
            }
            return false;
        }
        if(fabs(u)<vtx_precision || fabs(u-1.0)<vtx_precision)
        {
            // the segments meet at an end point
            // check on the other segment
            if(vtx_precision<t && t<1.0-vtx_precision)
            {
                return bPointsInside;
            }
            return false;
        }
        return false;
    }
}


/**
 * Returns the distance of point Pt to line AB
 */
double geom::distanceToLine2d(Vector2d const &A, Vector2d const &B, Vector2d const &P)
{
    double dotproduct = (B.x-A.x)*(P.x-A.x) + (B.y-A.y)*(P.y-A.y);
    if(fabs(dotproduct)<LENGTHPRECISION) return 0.0;

    double AB = sqrt((B.x-A.x)*(B.x-A.x)+(B.y-A.y)*(B.y-A.y));
    double AP = sqrt((P.x-A.x)*(P.x-A.x)+(P.y-A.y)*(P.y-A.y));

    if(AP<LENGTHPRECISION) return 0.0;
    if(AB<LENGTHPRECISION) return 0.0;
    double cosa = dotproduct/AB/AP;

    double AH = AP * cosa;
    return sqrt(AP*AP-AH*AH);
}



/**
 * Returns the distance of point Pt to line AB
 */
double geom::distanceToLine3d(Vector3d const &A, Vector3d const &B, Vector3d const &P)
{
    double dotproduct = (B.x-A.x)*(P.x-A.x) + (B.y-A.y)*(P.y-A.y) + (B.z-A.z)*(P.z-A.z);

    double AB = sqrt((B.x-A.x)*(B.x-A.x)+(B.y-A.y)*(B.y-A.y)+(B.z-A.z)*(B.z-A.z));
    double AP = sqrt((P.x-A.x)*(P.x-A.x)+(P.y-A.y)*(P.y-A.y)+(P.z-A.z)*(P.z-A.z));

    if(AP<LENGTHPRECISION)
    {
        return 0.0;
    }
    if(AB<LENGTHPRECISION)
    {
        return 0.0;
    }
    double cosa = dotproduct/AB/AP;

    double AH = AP * cosa;
    return sqrt(AP*AP-AH*AH);
}


/**
 * Returns true if [A0,A1] intersects the ray with origin O and direction U, false otherwise
 * Parallel overlapping segments are considered to intersect
 */
bool geom::intersectRay(Vector2d const &A0, Vector2d const &A1, Vector2d const &O, Vector2d const &U, Vector2d &IPt)
{
    Vector2d r, s;
    Vector2d B0, B1;

    r.x = A1.x-A0.x;
    r.y = A1.y-A0.y;

    B0.x = O.x;
    B0.y = O.y;
    B1.x = O.x + U.x*100.0;
    B1.y = O.y + U.y*100.0;
    s.x = B1.x-B0.x;
    s.y = B1.y-B0.y;

    double rXs = r.x*s.y - r.y*s.x; // two dimensional cross product

    if(fabs(rXs)<LENGTHPRECISION)
    {
        // the two lines are parallel
        double qpXr = (B0.x-A0.x)*r.y - (B0.y-A0.y)*r.x;
        if(fabs(qpXr)<LENGTHPRECISION)
        {
            //the two lines are co-linear
            double r2 = r.x*r.x + r.y*r.y;
            double t0 = ((B0.x-A0.x)*r.x + (B0.y-A0.y)*r.y )/r2;
            double t1 = t0 + (s.x*r.x+s.y*r.y)/r2;
            if(t0>LENGTHPRECISION && t0<1.0-LENGTHPRECISION)
            {
                return true; // overlapping
            }
            if(t1>LENGTHPRECISION && t1<1.0-LENGTHPRECISION)
            {
                return true; // overlapping
            }
            // Colinear not overlapping;
            return false;
        }
        else
        {
            //parallel;
        }
    }
    else
    {
        double qpXr = (B0.x-A0.x)*r.y - (B0.y-A0.y)*r.x;
        double qpXs = (B0.x-A0.x)*s.y - (B0.y-A0.y)*s.x;
        double t = qpXs / rXs;
        double u = qpXr / rXs;
        IPt = A0 + r*t;
        return (0.0<t && t<1.0 && 0.0<u && u<1.0);
    }

    return false;
}


/**
 * Checks if the two Triangle3d intersect, and returns their intersection segment
 * Warning: not symmetric; all intersecting cases are not covered
*/
bool geom::intersectTriangles3d(Triangle3d const &t0, Triangle3d const &t1, Segment3d &seg, double precision)
{
    if ( (t0.normal() * t1.normal()).norm()<1.e-6)
    {
        // the triangle planes are parallel
        // check if they are identical
        Vector3d GG = (t1.CoG_g()-t0.CoG_g());
        GG.normalize();
        if(fabs(GG.dot(t0.normal()))>1.e-6)
        {
            return false; // not in the same plane
        }
        else
        {
            // in the same plane
            double g[] = {0,0,0};
            bool bIntersect = false;
            for(int iv=0; iv<3; iv++)
            {
                Vector3d ptl = t0.globalToLocalPosition(t1.vertexAt(iv));
                t0.barycentricCoords(ptl, g);
                if(g[0]>=0 && g[0]<=1 && g[1]>=0 && g[1]<=1 && g[2]>=0 && g[2]<=1 )
                {
                    bIntersect =true;
                    break;
                }
            }
            if(bIntersect)
            {
            }
        }
    }
    else
    {
        Vector3d I1[2],I2[2];

        for(int it=0; it<2; it++)
        {
            Triangle3d const *pt0 = nullptr, *pt1 = nullptr;
            if(it==0) { pt0=&t0;  pt1=&t1;}
            else      { pt0=&t1;  pt1=&t0;}
            // check if triangle t1 intersects the plane P0 of triangle t0
            // this occurs if its vertices are on opposite sides of P0

            double sign[] = {1,1,1};
            for(int iv=0; iv<3; iv++)
            {
                sign[iv] = pt0->normal().dot(pt1->vertexAt(iv)-pt0->CoG_g());
            }

            int idx0 = -1; // the index of the vertex opposite the other two
            int idx1 = -1, idx2=-1;
            // 8 cases to check - could probably write this in a more compact way
            if(sign[0]>0.0)
            {
                if(sign[1]>0.0)
                {
                    if(sign[2]>0.0)  return false;// the three vertices have the same sign, no intersection
                    else {idx0=2; idx1=0; idx2=1;}
                }
                else //sign1<0, 0 and 1 are on opposite sides of P0
                {
                    if(sign[2]>0.0) {idx0=1; idx1=0; idx2=2;}//vtx1 is opposite the other two
                    else            {idx0=0; idx1=1; idx2=2;}//vtx0 is opposite the other two
                }
            }
            else if(sign[0]<0.0)
            {
                if(sign[1]<0.0)
                {
                    if(sign[2]<0.0)  return false;// the three vertices have the same sign, no intersection
                    else {idx0=2; idx1=0; idx2=1;} //vtx2 is opposite the other two
                }
                else //sign1>0, 0 and 1 are on opposite sides of P0
                {
                    if(sign[2]<0.0) {idx0=1; idx1=0; idx2=2;}//vtx1 is opposite the other two
                    else            {idx0=0; idx1=1; idx2=2;}//vtx0 is opposite the other two
                }
            }

            // vertex idx0 is on the other side of P0 than idx1 and idx2
            // find the intersection points of segments V0V1 and V0V2 with plane P0
            Segment3d const &seg1 = pt1->edge(idx0,idx1);
            Segment3d const &seg2 = pt1->edge(idx0,idx2);
            if(!pt0->intersectRayPlane(pt1->vertexAt(idx1), seg1.unitDir(), I1[it]))
                return false; // messed up somewhere
            if(!pt0->intersectRayPlane(pt1->vertexAt(idx2), seg2.unitDir(), I2[it]))
                return false; // messes up somewhere

/*            I1[0].displayCoords("I10");
            I2[0].displayCoords("I20");
            I1[1].displayCoords("I11");
            I2[1].displayCoords("I21");*/
        }

        // lastly check if the two intersection segments overlap
        return overlapSegments3d(Segment3d(I1[0], I2[0]), Segment3d(I1[1], I2[1]), seg, precision);
    }

    return false;
}


/**
 * Check if the two segments overlap, and returns their overlapped part
 * Only valid if the two segments are colinear
 * Colinearity is assumed to save times if flag is set to false
*/
bool geom::overlapSegments3d(Segment3d const &s0, Segment3d const &s1, Segment3d &seg, double precision, bool bCheckColinearity)
{
    if(s0.length()<LENGTHPRECISION)
    {
        if(s1.length()<LENGTHPRECISION)
        {
            if(s0.vertexAt(0).isSame(s1.vertexAt(0)))
            {
                seg = s0; // return a zero-length segment
                return true;
            }
            else return false;
        }
        else
        {
            if(s1.isOnSegment(s0.vertexAt(0), precision))
            {
                seg=s0;  // return the zero-length segment
                return true;
            }
            else return false;
        }
    }

    if(s1.length()<LENGTHPRECISION)
    {
        if(s0.isOnSegment(s1.vertexAt(0), precision))
        {
            seg=s1; // return the zero-length segment
            return true;
        }
        else return false;
    }

    //make s1 the same direction as s0 to facilitate dot product comparisons
    Segment3d s1p(s1);
    if(s0.unitDir().dot(s1.unitDir())<0) s1p.setNodes(s1.vertexAt(1), s1.vertexAt(0));
    assert(s1p.unitDir().dot(s0.unitDir())>0);

    Vector3d V00 = s1p.vertexAt(0)-s0.vertexAt(0);
    Vector3d V01 = s1p.vertexAt(1)-s0.vertexAt(0);
    Vector3d V10 = s1p.vertexAt(0)-s0.vertexAt(1);
    Vector3d V11 = s1p.vertexAt(1)-s0.vertexAt(1);

    if(bCheckColinearity)
    {
        if(fabs((s0.unitDir() * s1p.unitDir()).norm())>1.e-6) return false; // the two segments are not parallel
        if(fabs((s0.unitDir() * V00).norm()/V00.norm())>1.e-6) return false; // one point of s1 does not lie on the line of s0
    }

    double dot00 = V00.dot(s0.unitDir());
    double dot01 = V01.dot(s0.unitDir());
    double dot10 = V10.dot(s0.unitDir());
    double dot11 = V11.dot(s0.unitDir());


    // --------> 0 positive direction
    if(dot00>0)
    {
        //  00_____10
        if(dot10>0) return false; //  00___01____10______11
        else
        {
            if(dot11<0)
            {
                //  00____10______11____01
                seg = s1; // s1 is included in s0
                return true;
            }
            else
            {
                //  00____10______01_____11
                seg.setNodes(s1p.vertexAt(0), s0.vertexAt(1));
                return true;
            }
        }
    }
    else
    {
        //  10_____00
        if(dot01<0) return false; //  10_____11____00_____01
        else
        {
            //  10____00_____11_____01
            seg.setNodes(s0.vertexAt(0), s1p.vertexAt(1));
            return true;
        }
    }
}


void geom::makeSphere(double radius, int nSplit, std::vector<Triangle3d> &triangles)
{
    // make vertices
    std::vector<Node> vtx(12);
    vtx[10].set(0,0, radius);   // North pole
    vtx[11].set(0,0,-radius);   // South pole
    double x=0,y=0,z=0;
    double atn= atan(0.5);
    double di=0.0;
    for(int i=0; i<5; i++)
    {
        di = double(i);
        x = radius * cos(atn)*cos(72.0*di*PI/180.0);
        y = radius * cos(atn)*sin(72.0*di*PI/180.0);
        z = radius * sin(atn);
        vtx[i].set(x,y,z);
        x =  radius * cos(atn)*cos((36+72.0*di)*PI/180.0);
        y =  radius * cos(atn)*sin((36+72.0*di)*PI/180.0);
        z = -radius * sin(atn);
        vtx[i+5].set(x,y,z);
    }

    for(uint i=0; i<vtx.size(); i++) vtx[i].setNormal(vtx[i]);

    triangles.resize(20);


    //make the top five triangles from the North pole to the northern hemisphere latitude

    for(int i=0; i<5; i++)
    {
        int i1 = i;
        int i2 = (i+1)%5;
        triangles[i].setTriangle(vtx[10], vtx[i1], vtx[i2]);
    }
    //make the bottom five triangles from the South pole to the southern hemisphere latitude
    for(int i=0; i<5; i++)
    {
        int i1 = 5+i;
        int i2 = 5+(i+1)%5;
        triangles[5+i].setTriangle(vtx[11], vtx[i2], vtx[i1]);
    }

    // make the equatorial belt
    for(int i=0; i<5; i++)
    {
        int i1 = i;
        int i2 = (i+1)%5;
        triangles[10+i].setTriangle(vtx[i1], vtx[i1+5], vtx[i2]);
    }
    for(int i=0; i<5; i++)
    {
        int i1 = 5+i;
        int i2 = 5+(i+1)%5;
        triangles[15+i].setTriangle(vtx[i1], vtx[i2], vtx[(i1+1)%5]);
    }

    std::vector<Triangle3d> triangle, split(4);

    Node M0, M1, M2; //edge mid points
    for(int iter=0; iter<nSplit; iter++)
    {
        triangle.clear();
        for(uint it=0; it<triangles.size(); it++)
        {
            Triangle3d const& t3d = triangles.at(it);

            M0.set(t3d.edge(0).midPoint());
            M1.set(t3d.edge(1).midPoint());
            M2.set(t3d.edge(2).midPoint());

            // project radially the mid points onto the unit sphere
            M0.normalize();
            M1.normalize();
            M2.normalize();

            M0.setNormal(M0);
            M1.setNormal(M1);
            M2.setNormal(M2);

            M0 *= radius;
            M1 *= radius;
            M2 *= radius;

            // build the subset of 4 triangles
            split[0].setTriangle(M1, t3d.vertexAt(0), M2);
            split[1].setTriangle(M2, t3d.vertexAt(1), M0);
            split[2].setTriangle(M0, t3d.vertexAt(2), M1);
            split[3].setTriangle(M2, M0, M1);

            triangle.insert(triangle.end(), split.begin(), split.end());
        }

        triangles = triangle;
    }
}


/** Rotates the vector so that it lies on the plane defined by PlaneNormal*/
void geom::rotateOnPlane(Vector3d const &PlaneNormal, const Vector3d &Vin, Vector3d &VOut)
{
    assert(fabs(PlaneNormal.norm()-1.0)<LENGTHPRECISION);
    double thenorm = Vin.norm();
    if(thenorm<LENGTHPRECISION) return;

    //project on the plane and restore the norm - no issues with angle directions
    double dotprod = PlaneNormal.dot(Vin);
    VOut.x = Vin.x - dotprod*PlaneNormal.x;
    VOut.y = Vin.y - dotprod*PlaneNormal.y;
    VOut.z = Vin.z - dotprod*PlaneNormal.z;

    double projectednorm = VOut.norm();
    if(projectednorm<LENGTHPRECISION) return; // vectors are aligned

    VOut *= thenorm/projectednorm;
}


/** Rotates the point so that it lies on the plane defined by Origin and PlaneNormal*/
void geom::rotateOnPlane(Vector3d const &Origin, Vector3d const &PlaneNormal, Vector3d const &PtIn, Vector3d &PtOut)
{
    assert(fabs(PlaneNormal.norm()-1.0)<LENGTHPRECISION);
    double thenorm = PtIn.distanceTo(Origin);
    if(thenorm<LENGTHPRECISION) return;

    //project on the plane and restore the norm - no issues with angle directions
    Vector3d Vin(PtIn-Origin);
    double dotprod = PlaneNormal.dot(Vin);
    PtOut.x = PtIn.x - dotprod*PlaneNormal.x;
    PtOut.y = PtIn.y - dotprod*PlaneNormal.y;
    PtOut.z = PtIn.z - dotprod*PlaneNormal.z;

    double projectednorm = PtOut.distanceTo(Origin);
    if(projectednorm<LENGTHPRECISION) return; // vectors are aligned

    PtOut.x = Origin.x + (PtOut.x-Origin.x)*thenorm/projectednorm;
    PtOut.y = Origin.y + (PtOut.y-Origin.y)*thenorm/projectednorm;
    PtOut.z = Origin.z + (PtOut.z-Origin.z)*thenorm/projectednorm;
}


double geom::basis(int i, int deg, double t, double const *knots)
{
    double sum = 0.0;
    if(deg==0)
    {
        if(knots[i]<=t && t<knots[i+1]) return 1.0;
        else                            return 0.0;
    }

    if(fabs(knots[i+deg]-knots[i])>KNOTPRECISION)
    {
        sum += (t-knots[i])/(knots[i+deg]-knots[i]) * basis(i, deg-1, t, knots);
    }

    if(fabs(knots[i+deg+1]-knots[i+1])>KNOTPRECISION)
    {
        sum += (knots[i+deg+1]-t)/(knots[i+deg+1]-knots[i+1]) * basis(i+1, deg-1, t, knots);
    }
    return sum;
}


double geom::basisDerivative(int i, int deg, double t, double const *knots)
{
    double der = 0.0;

    if(fabs(knots[i+deg]-knots[i])>KNOTPRECISION)
    {
        der += double(deg)/(knots[i+deg]  -knots[i])   * basis(i, deg-1, t, knots);
    }
    if(fabs(knots[i+deg+1]-knots[i+1])>KNOTPRECISION)
    {
        der -= double(deg)/(knots[i+deg+1]-knots[i+1]) * basis(i+1, deg-1, t, knots);
    }
    return der;
}


void geom::makeNurbsTriangulation(NURBSSurface const &nurbs, int nx, int nh, std::vector<Triangle3d> &triangles)
{
    Vector3d S00, S01, S10, S11;

    // make the left side
    triangles.resize(nx*nh*2);

    int it=0;
    for (int k=0; k<nx; k++)
    {
        double uk  = double(k)   /double(nx);
        double uk1 = double(k+1) /double(nx);

        double vl=0.0;
        nurbs.getPoint(uk,  vl, S00);
        nurbs.getPoint(uk1, vl, S10);

        for (int l=0; l<nh; l++)
        {
            double vl1 = double(l+1) / double(nh);
            nurbs.getPoint(uk,  vl1, S01);
            nurbs.getPoint(uk1, vl1, S11);

            if(!S00.isSame(S01) && !S01.isSame(S11) && !S11.isSame(S00))
                triangles[it++] = {S00, S01, S11};
            if(!S00.isSame(S11) && !S11.isSame(S10) && !S10.isSame(S00))
                triangles[it++] = {S00, S11, S10};
            S00 = S01;
            S10 = S11;
//            vl = vl1;
        }
    }
    //squeeze
    triangles.resize(it);
}


int geom::isVector3d(std::vector<Vector3d> const &Nodes, Vector3d &Pt)
{
    for (int in=int(Nodes.size())-1; in>=0; in--)
    {
        if(Pt.isSame(Nodes.at(in))) return in;
    }
    return -1;
}



/** return the point closest to A among the intersected triangles*/
bool geom::blockIntersect(int iBlock, int m_nBlocks, const std::vector<Triangle3d> &triangulation, Vector3d A, Vector3d B, Node &I)
{
    Vector3d Int;

    double dmax = LARGEVALUE;

    int maxRows = int(triangulation.size());
    int blocksize = int(maxRows/m_nBlocks)+1; // add one to compensate for rounding errors
    int iStart = iBlock*blocksize;
    int iMax = std::min(iStart+blocksize, maxRows);

    bool bIntersect = false;

    for(int it=iStart; it<iMax; it++)
    {
        Triangle3d const &t3 = triangulation.at(it);
        if(t3.intersectSegmentInside(A, B, Int, true))
        {
            double d = sqrt((Int.x-A.x)*(Int.x-A.x)+(Int.y-A.y)*(Int.y-A.y)+(Int.z-A.z)*(Int.z-A.z));
            if(d<dmax)
            {
                I = Int;
                I.setNormal(t3.normal());
                dmax=d;
            }
            bIntersect = true;
        }
    }
    return bIntersect;
}


#define MAX_THREADS 100

bool geom::intersectTriangles(std::vector<Triangle3d> const &triangles, const Vector3d &A, const Vector3d &B, Node &I, bool bMultiThreaded)
{
    bool boolean[MAX_THREADS];
    memset(boolean, 0, MAX_THREADS*sizeof(bool));

    std::future<bool> bIntersect[MAX_THREADS];
    Node S[MAX_THREADS];

    int nThreads=std::thread::hardware_concurrency();
    nThreads = std::min(nThreads, MAX_THREADS);


    if(bMultiThreaded)
    {
//        std::vector<std::thread> threads;

        for (int iblock=0; iblock<nThreads; iblock++)
        {
//            threads.push_back(std::thread(blockIntersect, iblock, nThreads, std::ref(triangles), A, B, std::ref(S[iblock]), std::ref(bIntersect[iblock])));
            bIntersect[iblock] = std::async(geom::blockIntersect, iblock, nThreads, std::ref(triangles), A, B, std::ref(S[iblock]));
        }

        for (int iblock=0; iblock<nThreads; iblock++)
        {
//            threads[iblock].join();
            boolean[iblock] = bIntersect[iblock].get();
//            qDebug(" got future %d", iblock);
        }
//        qDebug()<<"Got all futures";
    }
    else
    {
        for(int iblock=0; iblock<nThreads; iblock++)
        {
            boolean[iblock] = blockIntersect(iblock, nThreads, triangles, A, B, S[iblock]);
        }
    }

    bool bResult = false;
    double dmax = LARGEVALUE;
    for(int i=0; i<nThreads; i++)
    {
        if(boolean[i])
        {
            bResult = true;
            double d = sqrt((S[i].x-A.x)*(S[i].x-A.x)+(S[i].y-A.y)*(S[i].y-A.y)+(S[i].z-A.z)*(S[i].z-A.z));
            if(d<dmax)
            {
                I = S[i];
                dmax=d;
            }
        }
    }

    return bResult;
}

