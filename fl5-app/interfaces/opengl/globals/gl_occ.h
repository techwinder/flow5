/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>

#include <api/vector3d.h>
#include <api/occmeshparams.h>

namespace gl
{
    void glMakeWire(const TopoDS_Wire &wire, QOpenGLBuffer &vbo);
    void glMakeShapeTriangles(const TopoDS_Shape &shape, const OccMeshParams &params, QOpenGLBuffer &vbo);
    void glMakeShellOutline(const TopoDS_ListOfShape &shapes, const Vector3d &position, QOpenGLBuffer &vbo, int nWireRes=30);
    void glMakeEdges(TopoDS_Shape const &Shape, QOpenGLBuffer &vboEdges, QVector<Vector3d> &labelpoints);
    void glMakeEdges(TopoDS_ListOfShape const &theEdges, const Vector3d &T, QOpenGLBuffer &vboEdges, QVector<Vector3d> &labelpoints);
    void glMakeEdge(const TopoDS_Edge &Edge, QOpenGLBuffer &vboEdge);
}
