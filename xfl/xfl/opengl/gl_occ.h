/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QOpenGLBuffer>

#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>

#include <xflgeom/geom3d/vector3d.h>
#include <xflocc/occmeshparams.h>


void glMakeWire(const TopoDS_Wire &wire, QOpenGLBuffer &vbo);
void glMakeShapeTriangles(const TopoDS_Shape &shape, const OccMeshParams &params, QOpenGLBuffer &vbo);
void glMakeShellOutline(const TopoDS_ListOfShape &shapes, const Vector3d &position, QOpenGLBuffer &vbo, int nWireRes=30);
void glMakeEdges(TopoDS_Shape const &Shape, QOpenGLBuffer &vboEdges, QVector<Vector3d> &labelpoints);
void glMakeEdges(TopoDS_ListOfShape const &theEdges, QOpenGLBuffer &vboEdges, QVector<Vector3d> &labelpoints);
void glMakeEdge(const TopoDS_Edge &Edge, QOpenGLBuffer &vboEdge);
