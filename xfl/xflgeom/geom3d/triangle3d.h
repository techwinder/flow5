/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QString>
#include <xflgeom/geom3d/cartesianframe.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflgeom/geom3d/segment3d.h>
#include <xflgeom/geom2d/triangle2d.h>

class Triangle3d
{
    public:
        Triangle3d();
//        Triangle3d(Triangle3d const &triangle);
        Triangle3d(Vector3d const *vertex);
        Triangle3d(Node const &vtx0, Node const &vtx1, Node const &vtx2);

        void setTriangle();
        void setTriangle(Node const &vtx0, Node const &vtx1, Node const &vtx2);
        inline void setNormal(const Vector3d &N);
        inline void setNormal(double nx, double ny, double nz);
        void flipNormal() {m_Normal.reverse();}

        void reverseOrientation();

        Segment3d const &edge(int iVtx) const {return m_Edge[iVtx%3];}
        inline Segment3d const &edge(int iVtx0, int iVtx1) const;

        void clearConnections() {m_Neighbour[0] = m_Neighbour[1] = m_Neighbour[2] = -1;}
        void setNeighbour(int it, int iEdge) {if( (iEdge<0) || (iEdge>2)) return; m_Neighbour[iEdge] = it;}
        int edgeIndex(Segment3d const &seg, double precision) const;

        double minEdgeLength() const;
        double maxEdgeLength() const;

        bool isTooLong(double length) const {return maxEdgeLength()>length;}
        inline bool isSkinny() const;
        double qualityFactor(double &r, double &shortestEdge) const;

        int neighbour(int idx) const {return m_Neighbour[idx];}
        inline int neighbourCount() const;

        bool hasVertex(Vector3d vtx) const{
            if     (m_S[0].isSame(vtx)) return true;
            else if(m_S[1].isSame(vtx)) return true;
            else if(m_S[2].isSame(vtx)) return true;
            return false;
        }
        bool hasVertex(int nodeindex) const { return (nodeindex==m_S[0].index()) || (nodeindex==m_S[1].index()) || (nodeindex==m_S[2].index());}

        CartesianFrame const & frame() const {return m_CF;}

        void setVertex(int ivtx, Node const &V) {m_S[ivtx%3].x=V.x; m_S[ivtx%3].y=V.y; m_S[ivtx%3].z=V.z;}
        void setVertex(int ivtx, double x, double y, double z) {m_S[ivtx%3].x=x; m_S[ivtx%3].y=y; m_S[ivtx%3].z=z;}
        Node &vertex(int i) {return m_S[i%3];}
        Node const &vertexAt(int i) const {return m_S[i%3];}

        inline void setVertices(const Node *vtx);

        void setVertexIndex(int iv, int index) {if(iv>=0&& iv<3) m_S[iv].setIndex(index);}
        void setVertexIndexes(int n0, int n1, int n2)	{m_S[0].setIndex(n0); m_S[1].setIndex(n1);	m_S[2].setIndex(n2);}
        int nodeIndex(int iv) const {return m_S[iv].index();}

        inline void makeXZsymmetric();

        inline bool containsPointProjection(Vector3d const &seg, Vector3d &proj, double fuzzy) const;
        bool intersectSegmentProjection(Segment3d const &seg) const;
        bool intersectSegmentInside(Segment3d const &seg, Vector3d &I, bool bEdgesInside) const {return intersectSegmentInside(seg.vertexAt(0), seg.vertexAt(1), I, bEdgesInside);}
        bool intersectSegmentInside(Vector3d const &A, Vector3d const &B, Vector3d &I, bool bEdgesInside) const;
        bool intersectRayInside(Vector3d const &A, Vector3d const &U, Vector3d &I) const;
        inline bool intersectRayPlane(Vector3d const &A, Vector3d const &U, Vector3d &I) const;


        bool isNull() const {return m_bNullTriangle;}
        bool isValid() const {return !m_bNullTriangle;}

//        void cartesianCoords(double *g, Vector3d &ptGlobal) const;

        inline void normalProjection(Vector3d const &pt, Vector3d &projected) const;

        void globalToLocal(Vector3d const &V, Vector3d &VLocal) const {m_CF.globalToLocal(V, VLocal);}
        Vector3d globalToLocal(Vector3d const &V) const {return m_CF.globalToLocal(V);}
        Vector3d globalToLocal(double const &Vx, double const &Vy, double const &Vz) const {return m_CF.globalToLocal(Vector3d(Vx,Vy, Vz));}
        Vector3d localToGlobal(Vector3d const &V) const {return m_CF.localToGlobal(V);}

        void localToGlobalPosition(double const&xl, double const &yl, double const &zl, double &XG, double &YG, double &ZG) const {m_CF.localToGlobalPosition(xl, yl, zl, XG, YG, ZG);}
        Vector3d localToGlobalPosition(Vector3d const &Pl) const {return m_CF.localToGlobalPosition(Pl);}
        void globalToLocalPosition(double const&XG, double const&YG, double const&ZG, double &xl, double &yl, double &zl) const {m_CF.globalToLocalPosition(XG, YG, ZG, xl, yl, zl);}
        Vector3d globalToLocalPosition(Vector3d const &P) const {return m_CF.globalToLocalPosition(P);}

        Triangle2d const & triangle2d()  const {return m_triangle2d;}

        Vector3d const & normal() const {return m_Normal;}
        Vector3d & normal() {return m_Normal;}

        void setNodeNormal(int iv, Vector3d const &N) {if(iv<0||iv>2) return; else m_S[iv].setNormal(N);}
        void setNodeNormals(Vector3d const &N0, Vector3d const &N1, Vector3d const &N2) {m_S[0].setNormal(N0); m_S[1].setNormal(N1); m_S[2].setNormal(N2);}
        Vector3d const &nodeNormal(int iNode) const {Q_ASSERT(iNode>=0 && iNode<3); return m_S[iNode].normal();}
        Vector3d &nodeNormal(int iNode) {Q_ASSERT(iNode>=0 && iNode<3); return m_S[iNode].normal();}

        void flipXZ();
        void scale(double xscalefactor, double yscalefactor, double zscalefactor);
        void translate(double tx, double ty, double tz);
        void translate(const Vector3d &T) {translate(T.x,T.y,T.z);}
        void rotate(Vector3d const &center, const Vector3d &axis, double angle);

        bool isPositiveOrientation() const {return m_SignedArea>0.0;}
        double signedArea() const {return m_SignedArea;}
        double area() const {return fabs(m_SignedArea);}
        Vector3d const & CoG_g() const {return m_CoG_g;}
        Vector3d const & CoG_l() const {return m_CoG_l;}

        inline double angle(int iVtx) const;

        inline void splitAtEdgeMidPoints(QVector<Triangle3d> &splittriangles) const;
        inline void splitAtCoG(QVector<Triangle3d> &splittriangles) const;
        void splitTriangle(QVector<Segment3d> const & raylist,
                           QVector<Segment3d> &polygon3d, QVector<Segment3d>&PSLG3d,
                           QVector<Triangle3d> &trianglelist, bool bLeftSide) const;

        void splitIn4Triangles(QVector<Triangle3d> &trianglelist) const;
        void splitIn3Triangles(Vector3d const &ptinside, QVector<Triangle3d> &trianglelist) const;

        //inline method test for tentative speed increase

        bool isSame(Triangle3d const &t3d, double precision) const
        {
            if(m_S[0].isSame(t3d.m_S[0], precision))
            {
                if(m_S[1].isSame(t3d.m_S[1], precision))
                {
                    if(m_S[2].isSame(t3d.m_S[2], precision)) return true;
                }
                else if(m_S[1].isSame(t3d.m_S[2], precision))
                {
                    if(m_S[2].isSame(t3d.m_S[1], precision)) return true;
                }
            }
            else if(m_S[0].isSame(t3d.m_S[1], precision))
            {
                if(m_S[1].isSame(t3d.m_S[0], precision))
                {
                    if(m_S[2].isSame(t3d.m_S[2], precision)) return true;
                }
                else if(m_S[1].isSame(t3d.m_S[2], precision))
                {
                    if(m_S[2].isSame(t3d.m_S[0], precision)) return true;
                }
            }
            else if(m_S[0].isSame(t3d.m_S[2], precision))
            {
                if(m_S[1].isSame(t3d.m_S[0], precision))
                {
                    if(m_S[2].isSame(t3d.m_S[1], precision)) return true;
                }
                else if(m_S[1].isSame(t3d.m_S[1], precision))
                {
                    if(m_S[2].isSame(t3d.m_S[0], precision)) return true;
                }
            }
            return false;
        }

        void setNeighbours(int const *neigh) {m_Neighbour[0]=neigh[0]; m_Neighbour[1]=neigh[1]; m_Neighbour[2]=neigh[2];}

        void barycentricCoords(Vector3d const &ptLocal, double *g) const  { barycentricCoords(ptLocal.x, ptLocal.y, g); }
        void barycentricCoords(double xl, double yl, double *g) const
        {
            g[0] = gmat[0] + gmat[1]*xl+ gmat[2]*yl;
            g[1] = gmat[3] + gmat[4]*xl+ gmat[5]*yl;
            g[2] = gmat[6] + gmat[7]*xl+ gmat[8]*yl;
        }

        QString properties(bool bLong=false) const;
        void displayNodes(const QString &msg=QString()) const;

    protected:
        inline void initialize();


    protected:
        Node m_S[3];                      /** the three triangle vertices, in global coordinates**/

        Vector3d m_Sl[3];                 /**< The three triangle vertices, in local coordinates */

        int m_Neighbour[3];               /**< the indexes of the three neighbour triangle sharing one of the edge, or -1 if none */
        double m_SignedArea;              /**< The panel's signed area; */
        bool m_bNullTriangle;
                                          /** @todo remove & replace with m_S[].index() */
        Vector3d O;                       /**< the origin of the local reference frame, in global coordinates */
        Vector3d Ol;                      /**< the origin in local coordinates, i.e. (0,0,0) */

        Vector3d m_Normal;
        Segment3d m_Edge[3];              /**< the three sides, in global coordinates */
        Vector3d m_S01l, m_S02l, m_S12l;  /**< the three sides, in local coordinates */
        Vector3d m, l;                    /**< the unit vectors which lie in the panel's plane. Cf. document NACA 4023 */

        Vector3d m_CoG_g;                 /**< the position of the center of gravity in global coordinates */
        Vector3d m_CoG_l;                 /**< the position of the center of gravity in local coordinates */
        Vector3d S01, S02, S12;           /**< the three sides, in global coordinates */

        Triangle2d m_triangle2d;

        CartesianFrame m_CF;

        double gmat[9];             /**< the transformation matrix from local coordinates to barycentric coordinates;*/
};



inline void Triangle3d::setVertices(Node const *vtx)
{
    m_S[0] = vtx[0];
    m_S[1] = vtx[1];
    m_S[2] = vtx[2];
}


inline void Triangle3d::makeXZsymmetric()
{
    m_S[0].y = -m_S[0].y;

    double S1x = m_S[1].x;
    double S1y = m_S[1].y;
    double S1z = m_S[1].z;

    m_S[1].x =  m_S[2].x;
    m_S[1].y = -m_S[2].y;
    m_S[1].z =  m_S[2].z;

    m_S[2].x =  S1x;
    m_S[2].y = -S1y;
    m_S[2].z =  S1z;

    setTriangle();
}


/** Calculates the projection of the input point on the triangle */
inline void Triangle3d::normalProjection(Vector3d const &pt, Vector3d &projected) const
{
    Vector3d GP = pt - m_CoG_g;
    double height = GP.dot(m_Normal);
    projected = pt - m_Normal* height;
}


inline int Triangle3d::neighbourCount() const
{
    int count = 0;
    if(m_Neighbour[0]>=0) count++;
    if(m_Neighbour[1]>=0) count++;
    if(m_Neighbour[2]>=0) count++;
    return count;
}


/**
 * A triangle is said to be skinny if the circumradius-to-shortest edge ratio is greater than B
 */
inline bool Triangle3d::isSkinny() const
{
    double r=0.0, e=0.0;
    return qualityFactor(r,e) > Triangle2d::qualityBound();
}


inline void Triangle3d::setNormal(Vector3d const &N)
{
    m_Normal.set(N);
}


inline void Triangle3d::setNormal(double nx, double ny, double nz)
{
    m_Normal.set(nx,ny,nz);
}


/** Initializes member variables to zero */
inline void Triangle3d::initialize()
{
    m_Neighbour[0] = m_Neighbour[1] = m_Neighbour[2] = -1;

    m_bNullTriangle = true;
    m_SignedArea  = 0.0;
    m_S[0].setIndex(-1);
    m_S[1].setIndex(-1);
    m_S[2].setIndex(-1);
}


/** returns the edge between vertices i0 and i1 */
inline Segment3d const &Triangle3d::edge(int i0, int i1) const
{
    if(i0==0)
    {
        if     (i1==1) return m_Edge[2];
        else if(i1==2) return m_Edge[1];
    }
    else if(i0==1)
    {
        if     (i1==0) return m_Edge[2];
        else if(i1==2) return m_Edge[0];
    }
    else if(i0==2)
    {
        if     (i1==1) return m_Edge[0];
        else if(i1==0) return m_Edge[1];
    }

    Q_ASSERT(false);
    return m_Edge[0]; // should never get here, but need to return something.
}



/**
* Finds the intersection point of a ray with the plane in which the panel lies.
* The ray is defined by a point and a direction vector.
* @param A the ray's origin
* @param U the ray's direction
* @param I the intersection point
* @return true if the ray intersects the plane, false if parallel
*/
inline bool Triangle3d::intersectRayPlane(Vector3d const &A, Vector3d const &U, Vector3d &I) const
{
    double r = (m_CoG_g.x-A.x)*m_Normal.x + (m_CoG_g.y-A.y)*m_Normal.y + (m_CoG_g.z-A.z)*m_Normal.z ;
    double s = U.x*m_Normal.x + U.y*m_Normal.y + U.z*m_Normal.z;

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

/*
inline double Triangle3d::angle(int iVtx) const
{
    if(isNull()) return 0.0;
    if(iVtx==0)
    {
        Vector3d s1 = m_S[1]-m_S[0];
        Vector3d s2 = m_S[2]-m_S[0];
        double cost = s1.dot(s2)/s1.norm()/s2.norm();
        if(cost>=1.0) return 0.0;
        if(cost<=-1) return 180.0;
        return acos(cost) * 180.0/PI;
    }
    else if(iVtx==1)
    {
        Vector3d s1 = m_S[0]-m_S[1];
        Vector3d s2 = m_S[2]-m_S[1];
        double cost = s1.dot(s2)/s1.norm()/s2.norm();
        if(cost>=1.0) return 0.0;
        if(cost<=-1) return 180.0;
        return acos(cost) * 180.0/PI;
    }
    else if(iVtx==2)
    {
        Vector3d s1 = m_S[0]-m_S[2];
        Vector3d s2 = m_S[1]-m_S[2];
        double cost = s1.dot(s2)/s1.norm()/s2.norm();
        if(cost>=1.0) return 0.0;
        if(cost<=-1) return 180.0;
        return acos(cost) * 180.0/PI;
    }
    return 0.0;
}*/


/** return the angle in degree of the vertex with index iVtx*/
inline double Triangle3d::angle(int iVtx) const
{
    if(isNull()) return 0.0;
    if(iVtx==0)
    {
        double s1x = m_S[1].x-m_S[0].x;
        double s1y = m_S[1].y-m_S[0].y;
        double s1z = m_S[1].z-m_S[0].z;
        double s2x = m_S[2].x-m_S[0].x;
        double s2y = m_S[2].y-m_S[0].y;
        double s2z = m_S[2].z-m_S[0].z;
        double l1 = sqrt(s1x*s1x + s1y*s1y + s1z*s1z);
        double l2 = sqrt(s2x*s2x + s2y*s2y + s2z*s2z);
        double cost = (s1x*s2x + s1y*s2y + s1z*s2z) / l1 /l2;
        if(cost>=1.0) return 0.0;
        if(cost<=-1) return 180.0;
        return acos(cost) * 180.0/PI;
    }
    else if(iVtx==1)
    {
        double s1x = m_S[0].x-m_S[1].x;
        double s1y = m_S[0].y-m_S[1].y;
        double s1z = m_S[0].z-m_S[1].z;
        double s2x = m_S[2].x-m_S[1].x;
        double s2y = m_S[2].y-m_S[1].y;
        double s2z = m_S[2].z-m_S[1].z;
        double l1 = sqrt(s1x*s1x + s1y*s1y + s1z*s1z);
        double l2 = sqrt(s2x*s2x + s2y*s2y + s2z*s2z);
        double cost = (s1x*s2x + s1y*s2y + s1z*s2z) / l1 /l2;
        if(cost>=1.0) return 0.0;
        if(cost<=-1) return 180.0;
        return acos(cost) * 180.0/PI;
    }
    else if(iVtx==2)
    {
        double s1x = m_S[0].x-m_S[2].x;
        double s1y = m_S[0].y-m_S[2].y;
        double s1z = m_S[0].z-m_S[2].z;
        double s2x = m_S[1].x-m_S[2].x;
        double s2y = m_S[1].y-m_S[2].y;
        double s2z = m_S[1].z-m_S[2].z;
        double l1 = sqrt(s1x*s1x + s1y*s1y + s1z*s1z);
        double l2 = sqrt(s2x*s2x + s2y*s2y + s2z*s2z);
        double cost = (s1x*s2x + s1y*s2y + s1z*s2z) / l1 /l2;
        if(cost>=1.0) return 0.0;
        if(cost<=-1) return 180.0;
        return acos(cost) * 180.0/PI;
    }
    return 0.0;
}


inline void Triangle3d::splitAtEdgeMidPoints(QVector<Triangle3d> &splittriangles) const
{
    Vector3d vtx0 = edge(0).midPoint();
    Vector3d vtx1 = edge(1).midPoint();
    Vector3d vtx2 = edge(2).midPoint();
    splittriangles.clear();
    splittriangles.append({vtx0, m_S[2], vtx1});
    splittriangles.append({vtx1, m_S[0], vtx2});
    splittriangles.append({vtx2, m_S[1], vtx0});
    splittriangles.append({vtx0, vtx1, vtx2});
}


inline void Triangle3d::splitAtCoG(QVector<Triangle3d> &splittriangles) const
{
    splittriangles.clear();
    splittriangles.append({m_S[0], m_CoG_g, m_S[1]});
    splittriangles.append({m_S[1], m_CoG_g, m_S[2]});
    splittriangles.append({m_S[2], m_CoG_g, m_S[0]});
}



/**
 * Tests if the projection of the segment on the triangle's plane intersects, or if this segments lies inside.
 * Not interested in the intersection point or segment itself
*/
inline bool Triangle3d::containsPointProjection(Vector3d const &pt, Vector3d &proj, double fuzzy) const
{
    Vector3d proj_l;

    normalProjection(pt, proj);
    m_CF.globalToLocalPosition(proj, proj_l);

    double g[] = {0,0,0};
    barycentricCoords(proj_l, g);
    return (g[0]>=-fuzzy && g[0]<=1.0+fuzzy)  &&  (g[1]>=-fuzzy && g[1]<=1.0+fuzzy)  &&  (g[2]>=-fuzzy && g[2]<=1.0+fuzzy);
}






