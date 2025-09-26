/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#pragma once

#include <QString>
#include <xflgeom/geom3d/vector3d.h>
#include <xflgeom/geom3d/segment3d.h>
#include <xflgeom/geom3d/triangle3d.h>
#include <xflgeom/geom2d/quad2d.h>
/**
 * @note the quad does not necessarily lie in a plane
 */
class Quad3d
{
    public:
        Quad3d();
        Quad3d(const Vector3d &vtx0, const Vector3d &vtx1, const Vector3d &vtx2, const Vector3d &vtx3);

        void setQuad();
        void setQuad(const Vector3d &S0, const Vector3d &S1, const Vector3d &S2, const Vector3d &S3);
        void initialize();

        Vector3d const&vertex(int i) const {return S[i];}

        void globalToLocal(Vector3d const &V, Vector3d &VLocal) const;
        Vector3d globalToLocal(Vector3d const &VTest) const;
        Vector3d globalToLocal(double const &Vx, double const &Vy, double const &Vz) const;
        Vector3d localToGlobal(Vector3d const &VTest) const;
        void localToGlobalPosition(double const&xl, double const &yl, double const &zl, double &XG, double &YG, double &ZG) const;
        Vector3d localToGlobalPosition(Vector3d const &Pl) const;
        void globalToLocalPosition(double const&XG, double const&YG, double const&ZG, double &xl, double &yl, double &zl) const;
        Vector3d globalToLocalPosition(Vector3d const &P) const;

        bool hasVertex(Vector3d vtx) const;

        void setMeshSides(int nu, int nv){m_nu=nu; m_nv=nv;}

        void splitQuad(QVector<Segment3d> const & raylist,
                       QVector<Segment3d> &polygon3d, QVector<Segment3d>&PSLG3d,
                       QVector<Triangle3d> &trianglelist) const;

        Segment3d edge(int i) const;

        Quad2d quad2d() const {return m_quad2d;}
        void displayNodes(const QString &msg) const;

        bool intersectQuadIsoMesh(Vector3d const &A, Vector3d const &U, Vector3d &I, double l=1.0) const;
        bool intersectQuadTriangles(Vector3d const &A, Vector3d const &U, Vector3d &I) const;
        bool intersectQuadPlane(Vector3d const &A, Vector3d const &U, Vector3d &I) const;

        bool isoParamCoords(double x2d, double y2d, double &s, double &t) const;
        Vector3d fromIsoParamCoords(double s, double t) const;

        Vector3d from2dTo3d(Vector2d pt2d) const;

        Triangle3d triangle(int iTriangle) const;

    private:
        Vector3d S[4];           /**< the nodes in circular order, in global coordinates */
        Vector3d Sl[4];          /**< the nodes in circular order, in local coordinates */
        double Area;             /**< The panel's area; */
        double m_length;
        Vector3d Normal;         /**< the unit vector normal to the panel in global coordinates */
        Vector3d m, l;           /**< the unit vectors which lie in the panel's plane. Cf. document NACA 4023 */

        Vector3d CoG_G;          /**< the panel's centroid, in global coordinates */
        Vector3d CoG_L;          /**< the panel's centroid, in local coordinates */
        Vector3d O;              /**< the origin of th local reference frame, in global coordinates */
        double lij[9];           /**< The 3x3 matrix used to transform local coordinates in absolute coordinates */

        Quad2d m_quad2d;

        int m_nu;                /**< the number of panels in the quad's main direction */
        int m_nv;                /**< the number of panels in the quad's tranvserse direction */
};

