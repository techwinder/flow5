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

#include <QOpenGLBuffer>
#include <QSurfaceFormat>


#include <api/linestyle.h>
#include <api/quad3d.h>
#include <api/segment3d.h>
#include <api/triangulation.h>
#include <api/vector3d.h>


class NURBSSurface;
namespace gl
{
    void getMemoryStatus(int &total_mem_kb, int &cur_avail_mem_kb);
    void printFormat(QSurfaceFormat const & ctxtFormat, QString &log, QString const prefix=QString());
    void printBuffer(QOpenGLBuffer &vbo, int stride);

    GLushort stipple(Line::enumLineStipple stipple);



    void makeLines(const std::vector<Vector3d> &points, QOpenGLBuffer &vbo);
    void makeLineStrip(const QVector<Vector3d> &strip, QOpenGLBuffer &vbo);
    void make2dLineStrip(QVector<Vector2d> const &strip, float zOffset, QOpenGLBuffer &vbo);

    void makeTetra(Vector3d const &pt, double side, QOpenGLBuffer &vboFaces, QOpenGLBuffer &vboEdges);
    void makeCube(Vector3d const &pt, double dx, double dy, double dz, QOpenGLBuffer &vboFaces, QOpenGLBuffer &vboEdges);

    void makeCircle(double rad, Vector3d const &O, QOpenGLBuffer &vbo);
    void makeUnitDisk(int nTriangles, QOpenGLBuffer &vbo);
    void makeDisk(double rad, Vector3d const &O, QOpenGLBuffer &vbo);
    void makeEllipseLineStrip(double a, double e, Vector3d const &O, QOpenGLBuffer &vbo);
    void makeEllipseFan(double a, double e, Vector3d const &O, QOpenGLBuffer &vbo);

    void makeQuad(const Quad3d &quad, QOpenGLBuffer &vbo);
    void makeQuad(const Node &V0, const Node &V1, const Node &V2, const Node &V3, QOpenGLBuffer &vbo);
    void makeQuadTex(double xside, double yside, QOpenGLBuffer &vbo);
    void makeQuad2d(const QRectF &rect, QOpenGLBuffer &vbo);

    void makeSegments(std::vector<Segment3d> const &segments, const Vector3d &pos, QOpenGLBuffer &vbo);
    void makeSegments(QVector<Segment3d> const &segments, const Vector3d &pos, QOpenGLBuffer &vbo);
    void makeTriangles3Vtx(const std::vector<Triangle3d> &triangles, bool bFlatNormals, QOpenGLBuffer &vbo);
    void makeTrianglesOutline(const std::vector<Triangle3d> &triangles, const Vector3d &position, QOpenGLBuffer &vbo);
    void makeTriangulation3Vtx(const Triangulation &triangulation, const Vector3d &pos, QOpenGLBuffer &vbo, bool bFlatNormals);
    void makeTriangleNormals(const std::vector<Triangle3d> &trianglelist, float coef, QOpenGLBuffer &vbo);
    void makeTriangleNodeNormals(const std::vector<Triangle3d> &trianglelist, float coef, QOpenGLBuffer &vbo);
    void makeNodeNormals(const std::vector<Node> &nodelist, const Vector3d &pos, float coef, QOpenGLBuffer &vbo);
    void makeArrow(Vector3d const &start, Vector3d const &end, QOpenGLBuffer &vbo);
    void makeArrows(QVector<Vector3d> const &point, QVector<Vector3d> const &arrow, double coef, QOpenGLBuffer &vbo);
    void makeArrows(QVector<Vector2d> const &point, QVector<Vector2d> const &arrow, float zoffset, double coef, QOpenGLBuffer &vbo);

    void makeTriangles3Vtx(const QVector<Triangle2d> &triangles, float z, QOpenGLBuffer &vbo);
    void makeTrianglesOutline(QVector<Triangle2d> const &triangles, const Vector2d &position, QOpenGLBuffer &vbo);

    void makeNurbsOutline(const NURBSSurface &nurbs, Vector3d const &pos, int NX, int NZ, QOpenGLBuffer &vbo);
    void makeFrameHighlight(const NURBSSurface &nurbs, const Vector3d &nurbspos, QOpenGLBuffer &vbo);


    void makeNodeForces(const std::vector<Node> &nodes, const std::vector<double> &CpNodes, float qDyn, double &rmin, double &rmax, bool bAuto, double scale, QOpenGLBuffer &vbo);


    void makeTriangle(const Vector3d &V0, const Vector3d &V1, const Vector3d &V2, QOpenGLBuffer &vbo);
    void makeTriangle(Vector3d *V, QOpenGLBuffer &vbo);
    void makeTriangle(Triangle3d const &t3d, QOpenGLBuffer &vbo);
    void makeTriangle(Vector2d const &V0, Vector2d const &V1, Vector2d const &V2, QOpenGLBuffer &vbo);

    void makeQuadColorMap(QOpenGLBuffer &vbo, int nrows, int ncols, QVector<Vector3d> const&nodes, QVector<double> const &values, float lmin, float lmax, bool bAuto, bool bMultiThreaded);
    void makeQuadColorMapRow(int r, int ncols, const QVector<Vector3d> &nodes, const QVector<double> &values, float *nodeVertexArray);
    void makeQuadContoursOnGrid(QOpenGLBuffer &vbo, int nrows, int ncols, QVector<Vector3d> const&nodes, QVector<double> const &values, bool bMultithreaded);


    void makeQuadContour(double threshold, int nrows,
                         QVector<Vector3d> const&node, QVector<double> const &value,
                         int ic);


    void makeCpSection(const std::vector<Node> &pts, std::vector<double> const&Cp, double coef, std::vector<Node> &cpsections);

    void lookUpQuadKey(int key, int *i);
    void lookUpTriangularKey(int key, int &i0, int &i1, int &i2, int &i3);

    void makeSegments2d(QVector<Segment2d> const &segments, Vector2d const &pos, QOpenGLBuffer &vbo);
    void makeColourMap2d(QVector<Triangle2d> const &panel3, float height, QVector<double> const &values, double lmin, double lmax, QOpenGLBuffer &vbo);

}




/* external temp variables for multithreading  */
extern double t_lmin, t_range;
extern QVector<QVector<Segment3d>> t_futuresegs;


