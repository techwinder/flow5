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



#pragma once

#include <vector>

#include <vector2d.h>
#include <vector3d.h>


class Segment2d;
class Segment3d;
class Triangle3d;
class Spline;
class BSpline;
class CubicSpline;
class NURBSSurface;
class Node;

namespace geom
{

    FL5LIB_EXPORT bool intersectXYPlane(Vector3d const &A,  Vector3d const &U,  Vector3d &I);

    FL5LIB_EXPORT bool intersectQuad3d(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB,
                          Vector3d const &A,  Vector3d const &U,  Vector3d &I, bool bDirOnly);

    FL5LIB_EXPORT bool intersect(Vector2d const &A, Vector2d const &B, Vector2d const &C, Vector2d const &D, Vector2d &M);
    FL5LIB_EXPORT bool intersectSegment(Vector2d const &A0, Vector2d const &A1, Vector2d const &B0, Vector2d const &B1, Vector2d &IPt, bool bPointsInside, double vtx_precision=1.0e-5);
    FL5LIB_EXPORT bool intersectRay(Vector2d const &A0, Vector2d const &A1, Vector2d const &O, Vector2d const &U, Vector2d &IPt);

    FL5LIB_EXPORT bool intersectTriangles(const std::vector<Triangle3d> &triangles, Vector3d const&A, Vector3d const&B, Node &I, bool bMultiThreaded);
    bool blockIntersect(int iBlock, int m_nBlocks, const std::vector<Triangle3d> &triangulation, Vector3d A, Vector3d B, Node &I);


    FL5LIB_EXPORT bool intersectTriangles3d(Triangle3d const &t0, Triangle3d const &t1, Segment3d &seg, double precision);
    FL5LIB_EXPORT bool overlapSegments3d(Segment3d const &s0, Segment3d const &s1, Segment3d &seg, double precision, bool bCheckColinearity=false);

    FL5LIB_EXPORT double distanceToLine2d(const Vector2d &A, const Vector2d &B, const Vector2d &P);

    FL5LIB_EXPORT double distanceToLine3d(Vector3d const &A, Vector3d const &B, const Vector3d &P);


    FL5LIB_EXPORT void splitTriangle(std::vector<Segment3d> const & raylist,
                                     std::vector<Segment3d> &polygon3d, std::vector<Segment3d>&PSLG3d,
                                     std::vector<Triangle3d> &trianglelist, bool bLeftSide) ;

    FL5LIB_EXPORT void makeSphere(double radius, int nSplit, std::vector<Triangle3d> &triangles);

    FL5LIB_EXPORT void rotateOnPlane(Vector3d const &PlaneNormal, Vector3d const &Vin, Vector3d &VOut);
    FL5LIB_EXPORT void rotateOnPlane(Vector3d const &Origin, Vector3d const &PlaneNormal, Vector3d const &Ptin, Vector3d &PtOut);

    FL5LIB_EXPORT double basis(int i, int deg, double t, const double *knots);
    FL5LIB_EXPORT double basisDerivative(int i, int deg, double t, const double *knots);

    FL5LIB_EXPORT void makeNurbsTriangulation(NURBSSurface const &nurbs, int nx, int nh, std::vector<Triangle3d> &triangles);

    FL5LIB_EXPORT int isVector3d(const std::vector<Vector3d> &Nodes, Vector3d &Pt);
}

