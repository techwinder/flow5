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



#include <quad3d.h>
#include <constants.h>
#include <matrix.h>

#include <segment2d.h>
#include <geom_global.h>


Quad3d::Quad3d()
{
    Area=0;
    m_length = 0.0;
    m_nu = m_nv = 1;
    memset(lij,0, 9*sizeof(double));
}


/**
 * Constructs the quad from the vertices in circular order
 */
Quad3d::Quad3d(Vector3d const&vtx0, Vector3d const&vtx1, Vector3d const&vtx2, Vector3d const&vtx3)
{
    m_nu = m_nv = 1;
    setQuad(vtx0, vtx1, vtx2, vtx3);
}


/**
* Defines the vortex and panel geometrical properties necessary for the VLM and panel calculations.
* pNode = LA - TA - TB - LB
*/
void Quad3d::setQuad(Vector3d const &S0, Vector3d const &S1, Vector3d const&S2, Vector3d const&S3)
{
    S[0]=S0;  S[1]=S1;  S[2]=S2;  S[3]=S3;

    Vector3d TALB, LATB, MidA, MidB;
    Vector3d smp, smq;

    LATB.x = S[2].x - S[0].x;
    LATB.y = S[2].y - S[0].y;
    LATB.z = S[2].z - S[0].z;
    TALB.x = S[3].x - S[1].x;
    TALB.y = S[3].y - S[1].y;
    TALB.z = S[3].z - S[1].z;

    Normal = LATB * TALB;
    Area = Normal.norm()/2.0;
    m_length = (LATB.norm() + TALB.norm())/2.0;
    Normal.normalize();


    MidA.x = (S[0].x + S[1].x)/2.0;
    MidA.y = (S[0].y + S[1].y)/2.0;
    MidA.z = (S[0].z + S[1].z)/2.0;

    MidB.x = (S[3].x + S[2].x)/2.0;
    MidB.y = (S[3].y + S[2].y)/2.0;
    MidB.z = (S[3].z + S[2].z)/2.0;


    // the panel's centroid, not the CoG
    CoG_G.x = (S[0].x + S[3].x + S[1].x + S[2].x)/4.0;
    CoG_G.y = (S[0].y + S[3].y + S[1].y + S[2].y)/4.0;
    CoG_G.z = (S[0].z + S[3].z + S[1].z + S[2].z)/4.0;

    O=CoG_G;

    m.x = (S[3].x + S[2].x) *0.5 - CoG_G.x;
    m.y = (S[3].y + S[2].y) *0.5 - CoG_G.y;
    m.z = (S[3].z + S[2].z) *0.5 - CoG_G.z;

    m.normalize();

    l.x =  m.y * Normal.z - m.z * Normal.y;
    l.y = -m.x * Normal.z + m.z * Normal.x;
    l.z =  m.x * Normal.y - m.y * Normal.x;

    smq.x  = (S[3].x + S[2].x) * 0.5 - CoG_G.x;
    smq.y  = (S[3].y + S[2].y) * 0.5 - CoG_G.y;
    smq.z  = (S[3].z + S[2].z) * 0.5 - CoG_G.z;
    smp.x  = (S[2].x + S[1].x) * 0.5 - CoG_G.x;
    smp.y  = (S[2].y + S[1].y) * 0.5 - CoG_G.y;
    smp.z  = (S[2].z + S[1].z) * 0.5 - CoG_G.z;


    //create the transformation matrix
    lij[0]=l.x;        lij[1]=m.x;       lij[2]=Normal.x;
    lij[3]=l.y;        lij[4]=m.y;       lij[5]=Normal.y;
    lij[6]=l.z;        lij[7]=m.z;       lij[8]=Normal.z;

    matrix::invert33(lij);


    Sl[0] = globalToLocal(S[0]-CoG_G);
    Sl[1] = globalToLocal(S[1]-CoG_G);
    Sl[2] = globalToLocal(S[2]-CoG_G);
    Sl[3] = globalToLocal(S[3]-CoG_G);
    CoG_L  = globalToLocal(CoG_G-CoG_G);

    m_quad2d.setPanelFrame(Vector2d(Sl[0].x, Sl[0].y), Vector2d(Sl[1].x, Sl[1].y), Vector2d(Sl[2].x, Sl[2].y), Vector2d(Sl[3].x, Sl[3].y));
}


/**
* Converts the global coordinates of the input vector in local panel coordinates.
*@param  V the global coordinates
*@param  V the calculated local coordinates
*/
void Quad3d::globalToLocal(Vector3d const &V, Vector3d &VLocal) const
{
    VLocal.x = lij[0]*V.x +lij[1]*V.y +lij[2]*V.z;
    VLocal.y = lij[3]*V.x +lij[4]*V.y +lij[5]*V.z;
    VLocal.z = lij[6]*V.x +lij[7]*V.y +lij[8]*V.z;
}


/**
 * Converts the global coordinates of the input vector in local panel coordinates.
 * @param  V the vector in the global reference frame
 * @return The vector in the local reference frame
 */
Vector3d Quad3d::globalToLocal(Vector3d const &V) const
{
    Vector3d L;
    L.x = lij[0]*V.x +lij[1]*V.y +lij[2]*V.z;
    L.y = lij[3]*V.x +lij[4]*V.y +lij[5]*V.z;
    L.z = lij[6]*V.x +lij[7]*V.y +lij[8]*V.z;
    return L;
}


/**
 * Converts the global coordinates of the input vector in local panel coordinates.
 * @param  V the vector in the global reference frame
 * @return The vector in the local reference frame
 */
Vector3d Quad3d::globalToLocal(double const &Vx, double const &Vy, double const &Vz) const
{
    Vector3d L;
    L.x = lij[0]*Vx +lij[1]*Vy +lij[2]*Vz;
    L.y = lij[3]*Vx +lij[4]*Vy +lij[5]*Vz;
    L.z = lij[6]*Vx +lij[7]*Vy +lij[8]*Vz;
    return L;
}


/**
 * Converts the local coordinates of the input vector in the global referential
 * @param  V the local coordinates
 * @return The Vector3d holding the global coordinates
 */
Vector3d Quad3d::localToGlobal(Vector3d const &V) const
{
    Vector3d L;
    L.x = V.x * l.x + V.y * m.x + V.z * Normal.x;
    L.y = V.x * l.y + V.y * m.y + V.z * Normal.y;
    L.z = V.x * l.z + V.y * m.z + V.z * Normal.z;
    return L;
}

/**
 * Converts the global coordinates of the input vector in the local referential
 * @todo return a reference to save memory allocation times
 * @param  P the position in the global reference frame
 * @return The position in the local reference frame
 */
Vector3d Quad3d::globalToLocalPosition(Vector3d const &P) const
{
    Vector3d V(P.x-O.x, P.y-O.y, P.z-O.z);
    Vector3d L;
    L.x = lij[0]*V.x +lij[1]*V.y +lij[2]*V.z;
    L.y = lij[3]*V.x +lij[4]*V.y +lij[5]*V.z;
    L.z = lij[6]*V.x +lij[7]*V.y +lij[8]*V.z;
    return L;
}


void Quad3d::globalToLocalPosition(double const&XG, double const&YG, double const&ZG, double &xl, double &yl, double &zl) const
{
    double vx = XG-O.x;
    double vy = YG-O.y;
    double vz = ZG-O.z;
    xl = lij[0]*vx +lij[1]*vy +lij[2]*vz;
    yl = lij[3]*vx +lij[4]*vy +lij[5]*vz;
    zl = lij[6]*vx +lij[7]*vy +lij[8]*vz;
}


/**
 * Converts the local coordinates of the input vector in the global referential
 * @param  Pl the position in the local reference frame
 * @return The position in the global reference frame
 */
Vector3d Quad3d::localToGlobalPosition(Vector3d const &Pl) const
{
    Vector3d L;
    L.x = O.x + Pl.x*l.x + Pl.y*m.x + Pl.z*Normal.x;
    L.y = O.y + Pl.x*l.y + Pl.y*m.y + Pl.z*Normal.y;
    L.z = O.z + Pl.x*l.z + Pl.y*m.z + Pl.z*Normal.z;
    return L;
}


void Quad3d::localToGlobalPosition(double const&xl, double const &yl, double const &zl, double &XG, double &YG, double &ZG) const
{
    XG = O.x + xl*l.x + yl*m.x + zl*Normal.x;
    YG = O.y + xl*l.y + yl*m.y + zl*Normal.y;
    ZG = O.z + xl*l.z + yl*m.z + zl*Normal.z;
}



bool Quad3d::hasVertex(Vector3d vtx) const
{
    if     (S[0].isSame(vtx)) return true;
    else if(S[1].isSame(vtx)) return true;
    else if(S[2].isSame(vtx)) return true;
    else if(S[3].isSame(vtx)) return true;
    return false;
}


Segment3d Quad3d::edge(int i) const
{
    if (i==0)     return Segment3d(S[0], S[1]);
    else if(i==1) return Segment3d(S[1], S[2]);
    else if(i==2) return Segment3d(S[2], S[3]);
    else if(i==3) return Segment3d(S[3], S[0]);
    else
    {
//        qDebug("Quad3d::edge   Problem");
        Segment3d anEdge;
        return anEdge;
    }
}


/**
 * Finds and returns the iso parametric coordinates of the local point (x2d, y2d)
 * using the Newton-Raphson iteration method.
 */
bool Quad3d::isoParamCoords(double x2d, double y2d, double &s, double &t) const
{
    double err_max = 1.e-5;
    double err=1.0;
    int iter = 0;

    s = t = 0.5;
    double x = Sl[0].x*(1-s)*(1-t)/4 + Sl[1].x*(1-s)*(1+t)/4 +  Sl[2].x*(1+s)*(1+t)/4 + Sl[3].x*(1+s)*(1-t)/4;
    double y = Sl[0].y*(1-s)*(1-t)/4 + Sl[1].y*(1-s)*(1+t)/4 +  Sl[2].y*(1+s)*(1+t)/4 + Sl[3].y*(1+s)*(1-t)/4;
    x -=x2d;
    y -=y2d;

    do
    {
        double dxds = Sl[0].x * (-1+t)/4.0 + Sl[1].x * (-1-t)/4.0 + Sl[2].x * (1+t)/4.0 + Sl[3].x * (1-t)/4.0;
        double dyds = Sl[0].y * (-1+t)/4.0 + Sl[1].y * (-1-t)/4.0 + Sl[2].y * (1+t)/4.0 + Sl[3].y * (1-t)/4.0;

        double dxdt = Sl[0].x * (-1+s)/4.0 + Sl[1].x * ( 1-s)/4.0 + Sl[2].x * (1+s)/4.0 + Sl[3].x * (-1-s)/4.0;
        double dydt = Sl[0].y * (-1+s)/4.0 + Sl[1].y * ( 1-s)/4.0 + Sl[2].y * (1+s)/4.0 + Sl[3].y * (-1-s)/4.0;

        //invert matrix
        double a[4];
        double det = dxds*dydt - dyds*dxdt;
        if(fabs(det)<PRECISION) return false;
        a[0] =  dydt/det;
        a[1] = -dxdt/det;
        a[2] = -dyds/det;
        a[3] =  dxds/det;
        s -= x*a[0] + y*a[1];
        t -= x*a[2] + y*a[3];

        x = Sl[0].x*(1-s)*(1-t)/4 + Sl[1].x*(1-s)*(1+t)/4 +  Sl[2].x*(1+s)*(1+t)/4 + Sl[3].x*(1+s)*(1-t)/4;
        y = Sl[0].y*(1-s)*(1-t)/4 + Sl[1].y*(1-s)*(1+t)/4 +  Sl[2].y*(1+s)*(1+t)/4 + Sl[3].y*(1+s)*(1-t)/4;
        x -=x2d;
        y -=y2d;

        err = sqrt(x*x+y*y);
        iter++;
    }while (err>err_max && iter<50);

    return err<err_max;
}



/** returns a half triangle */
Triangle3d Quad3d::triangle(int iTriangle) const
{
    if(iTriangle<0 || iTriangle>1) return Triangle3d();
    if(iTriangle==0) return Triangle3d(S[0], S[1], S[3]);
    else             return Triangle3d(S[1], S[2], S[3]);
}

#define NSUBQUADS 10
#define ERRMAX 1.e-3
/**
* Finds the intersection point of a ray with the iso-parametric surface defined by the quad's four vertices.
* The ray is defined by a point and a direction vector.
* @param A the ray's origin
* @param U the ray's direction
* @param I the intersection point
* @param l l=1 to intersect exactly the quad, l>1 to intersect also outside
* @return true if the ray intersects the plane, false if parallel
*/
bool Quad3d::intersectQuadIsoMesh(Vector3d const &A, Vector3d const &U, Vector3d &I, double l) const
{
    Vector3d S0 = fromIsoParamCoords(-l,-l);
    Vector3d S1 = fromIsoParamCoords( l,-l);
    Vector3d S2 = fromIsoParamCoords( l, l);
    Vector3d S3 = fromIsoParamCoords(-l, l);
    Triangle3d t0(S0,S1,S3);
    Triangle3d t1(S1,S2, S3);

    if((S[2]-S[0]).norm()<ERRMAX/20.0 || (S[3]-S[1]).norm()<ERRMAX/20.0 )
    {
        // that's bad enough
        return false;
    }

    if(t0.intersectRayInside(A, U, I) || t1.intersectRayInside(A,U,I))
    {
        if((S[2]-S[0]).norm()<ERRMAX || (S[3]-S[1]).norm()<ERRMAX )
        {
            // that's good enough
            return true;
        }
        else
        {
            // try again
            // make a sub mesh of iso parametric quads
            for(int is=0; is<NSUBQUADS-1; is++)
            {
                double s  = -l + 2.0*l*double(is)/(NSUBQUADS-1.0);
                double s1 = -l + 2.0*l*double(is+1)/(NSUBQUADS-1.0);
                for(int it=0; it<NSUBQUADS-1; it++)
                {
                    double t  = -l + 2.0*l*double(it)/(NSUBQUADS-1.0);
                    double t1 = -l + 2.0*l*double(it+1)/(NSUBQUADS-1.0);
                    Vector3d v0,v1,v2,v3;
                    v0.x = S[0].x*(1-s) *(1-t) /4 + S[1].x*(1-s) *(1+t) /4 + S[2].x*(1+s) *(1+t) /4 + S[3].x*(1+s) *(1-t) /4;
                    v0.y = S[0].y*(1-s) *(1-t) /4 + S[1].y*(1-s) *(1+t) /4 + S[2].y*(1+s) *(1+t) /4 + S[3].y*(1+s) *(1-t) /4;
                    v0.z = S[0].z*(1-s) *(1-t) /4 + S[1].z*(1-s) *(1+t) /4 + S[2].z*(1+s) *(1+t) /4 + S[3].z*(1+s) *(1-t) /4;

                    v1.x = S[0].x*(1-s) *(1-t1)/4 + S[1].x*(1-s) *(1+t1)/4 + S[2].x*(1+s) *(1+t1)/4 + S[3].x*(1+s) *(1-t1)/4;
                    v1.y = S[0].y*(1-s) *(1-t1)/4 + S[1].y*(1-s) *(1+t1)/4 + S[2].y*(1+s) *(1+t1)/4 + S[3].y*(1+s) *(1-t1)/4;
                    v1.z = S[0].z*(1-s) *(1-t1)/4 + S[1].z*(1-s) *(1+t1)/4 + S[2].z*(1+s) *(1+t1)/4 + S[3].z*(1+s) *(1-t1)/4;

                    v2.x = S[0].x*(1-s1)*(1-t1)/4 + S[1].x*(1-s1)*(1+t1)/4 + S[2].x*(1+s1)*(1+t1)/4 + S[3].x*(1+s1)*(1-t1)/4;
                    v2.y = S[0].y*(1-s1)*(1-t1)/4 + S[1].y*(1-s1)*(1+t1)/4 + S[2].y*(1+s1)*(1+t1)/4 + S[3].y*(1+s1)*(1-t1)/4;
                    v2.z = S[0].z*(1-s1)*(1-t1)/4 + S[1].z*(1-s1)*(1+t1)/4 + S[2].z*(1+s1)*(1+t1)/4 + S[3].z*(1+s1)*(1-t1)/4;

                    v3.x = S[0].x*(1-s1)*(1-t) /4 + S[1].x*(1-s1)*(1+t) /4 + S[2].x*(1+s1)*(1+t) /4 + S[3].x*(1+s1)*(1-t) /4;
                    v3.y = S[0].y*(1-s1)*(1-t) /4 + S[1].y*(1-s1)*(1+t) /4 + S[2].y*(1+s1)*(1+t) /4 + S[3].y*(1+s1)*(1-t) /4;
                    v3.z = S[0].z*(1-s1)*(1-t) /4 + S[1].z*(1-s1)*(1+t) /4 + S[2].z*(1+s1)*(1+t) /4 + S[3].z*(1+s1)*(1-t) /4;

                    Quad3d sub(v0,v1,v2,v3);
                    if(sub.intersectQuadIsoMesh(A, U, I)) return true;
                }
            }
        }
    }
    return false;
}


/**
* Finds the intersection point of a ray with the approximate plane in which the panel lies.
* The plane is defined by the Quad3's centroid and the Normal
* The ray is defined by a point and a direction vector.
* @param A the ray's origin
* @param U the ray's direction
* @param I the intersection point
* @return true if the ray intersects the plane, false if parallel
*/
bool Quad3d::intersectQuadPlane(Vector3d const &A, Vector3d const &U, Vector3d &I) const
{
    double r = (CoG_G.x-A.x)*Normal.x + (CoG_G.y-A.y)*Normal.y + (CoG_G.z-A.z)*Normal.z ;
    double s = U.x*Normal.x + U.y*Normal.y + U.z*Normal.z;

    if(fabs(s)>0.0)
    {
        double dist = r/s;

        I.x = A.x + U.x * dist;
        I.y = A.y + U.y * dist;
        I.z = A.z + U.z * dist;
        return true;
    }

    return false;
}


/**
* Finds the intersection point of a ray with the Quad3d's two half triangles .
* The ray is defined by a point and a direction vector.
* @param A the ray's origin
* @param U the ray's direction
* @param I the intersection point
* @return true if the ray intersects either of the two trianglesl
*/
bool Quad3d::intersectQuadTriangles(Vector3d const &A, Vector3d const &B, Vector3d &I) const
{
    if     (triangle(0).intersectSegmentInside(A, B, I, true)) return true;
    else if(triangle(1).intersectSegmentInside(A, B, I, true)) return true;
    else I=A;
    return false;
}



Vector3d Quad3d::from2dTo3d(Vector2d pt2d) const
{
    Vector3d pt3d;
    double g[]{0,0,0};
    Triangle3d t3d0(triangle(0));
    if(m_quad2d.triangle(0).contains(pt2d, true))
    {
        m_quad2d.triangle(0).barycentricCoords(pt2d, g);
        pt3d.x = t3d0.vertexAt(0).x*g[0] + t3d0.vertexAt(1).x*g[1] + t3d0.vertexAt(2).x*g[2];
        pt3d.y = t3d0.vertexAt(0).y*g[0] + t3d0.vertexAt(1).y*g[1] + t3d0.vertexAt(2).y*g[2];
        pt3d.z = t3d0.vertexAt(0).z*g[0] + t3d0.vertexAt(1).z*g[1] + t3d0.vertexAt(2).z*g[2];
    }
    else
    {
        Triangle3d t3d1(triangle(1));
        m_quad2d.triangle(1).barycentricCoords(pt2d, g);
        pt3d.x = t3d1.vertexAt(0).x*g[0] + t3d1.vertexAt(1).x*g[1] + t3d1.vertexAt(2).x*g[2];
        pt3d.y = t3d1.vertexAt(0).y*g[0] + t3d1.vertexAt(1).y*g[1] + t3d1.vertexAt(2).y*g[2];
        pt3d.z = t3d1.vertexAt(0).z*g[0] + t3d1.vertexAt(1).z*g[1] + t3d1.vertexAt(2).z*g[2];
    }
    return pt3d;
}


Vector3d Quad3d::fromIsoParamCoords(double s, double t) const
{
    Vector3d pt;
    pt.x = S[0].x*(1-s)*(1-t)/4 + S[1].x*(1-s)*(1+t)/4 + S[2].x*(1+s)*(1+t)/4 + S[3].x*(1+s)*(1-t)/4;
    pt.y = S[0].y*(1-s)*(1-t)/4 + S[1].y*(1-s)*(1+t)/4 + S[2].y*(1+s)*(1+t)/4 + S[3].y*(1+s)*(1-t)/4;
    pt.z = S[0].z*(1-s)*(1-t)/4 + S[1].z*(1-s)*(1+t)/4 + S[2].z*(1+s)*(1+t)/4 + S[3].z*(1+s)*(1-t)/4;
    return pt;
}

