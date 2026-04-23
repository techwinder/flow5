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
#include <string>

#include <TopoDS_Shape.hxx>
#include <TopoDS_ListOfShape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shell.hxx>
#include <Geom_Surface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>

#include <triangle3d.h>
#include <bspline3d.h>

class Fuse;
class WingXfl;
class Surface;
class OccMeshParams;
class NURBSSurface;


/** @todo not all methods need to be exported */
namespace occ
{
    FL5LIB_EXPORT std::string shapeType(const TopoDS_Shape &aShape);
    FL5LIB_EXPORT std::string shapeOrientation(TopoDS_Shape const &aShape);

    FL5LIB_EXPORT int listSubShapes(TopoDS_Shape const &aShape, TopAbs_ShapeEnum SubShapeType, std::vector<std::string> &strList, std::string prefx="");
    FL5LIB_EXPORT void listAllSubShapes(TopoDS_Shape const &aShape, std::vector<std::string> &strlist);
    FL5LIB_EXPORT void listShapeContent(TopoDS_Shape const &shape, std::string &logmsg, const std::string &prefx="", bool bFull=false);
    FL5LIB_EXPORT void  checkShape(TopoDS_Shape const &shape, std::string &logmsg, const std::string &prefix);

    FL5LIB_EXPORT bool makeOCCSplineFromPoints(std::vector<Vector3d> const &pointList, Handle(Geom_BSplineCurve)& theSpline, std::string &logmsg);
    FL5LIB_EXPORT bool makeOCCSplineFromBSpline3d(BSpline3d const &b3d, Handle(Geom_BSplineCurve)& hspline, std::string &logmsg);
    FL5LIB_EXPORT bool makeOCCSplineFromBSpline3d_0(BSpline3d const &b3d, Handle(Geom_BSplineCurve)& hspline, std::string &logmsg);
    FL5LIB_EXPORT void makeOCCNURBSFromNurbs(NURBSSurface const &nurbs, bool bXZSymmetric, Handle(Geom_BSplineSurface)& hnurbs, std::string &logmsg);

    FL5LIB_EXPORT void makeFaceFromNodeStrip(const std::vector<Node> &pts, TopoDS_Face &face, std::string &log);
    FL5LIB_EXPORT void makeFaceFromTriangle(Vector3d const &Pt0, Vector3d const &Pt1, Vector3d const &Pt2, TopoDS_Face &face, std::string &log);
    FL5LIB_EXPORT void makeFaceTriMesh(TopoDS_Face const &face, std::vector<Triangle3d> &trianglelist, double maxelementsize);

    FL5LIB_EXPORT void findEdges(TopoDS_Shape const &shape, TopoDS_ListOfShape &edges, std::string &logmsg);
    FL5LIB_EXPORT void findWires(const TopoDS_Shape &theshape, TopoDS_Wire &theOuterWire, TopoDS_ListOfShape &wires, std::string &logmsg, std::string prefix="");

    FL5LIB_EXPORT void shapeBoundingBox(const TopoDS_Shape &shape, double &Xmin, double &Ymin, double &Zmin, double &Xmax, double &Ymax, double &Zmax);

    FL5LIB_EXPORT double edgeLength(TopoDS_Edge const &edge);
    FL5LIB_EXPORT double edgeLength(TopoDS_Edge const &edge, double u0, double u1);

    FL5LIB_EXPORT void makeEdgeUniformSplitList(TopoDS_Face const &Face, TopoDS_Edge const &Edge, double maxlength, std::vector<double> &uval);
    FL5LIB_EXPORT void makeEdgeUniformSplitList(TopoDS_Edge const &Edge, double maxlength, std::vector<double> &uval);
    FL5LIB_EXPORT void makeEdgeSplitList(TopoDS_Edge const &Edge, double maxlength, double maxdeflection, std::vector<double> &uval);

    FL5LIB_EXPORT double facePerimeter(TopoDS_Face const &face);
    FL5LIB_EXPORT double faceArea(TopoDS_Face const &face);
    FL5LIB_EXPORT double wireLength(TopoDS_Wire const &wire);
    FL5LIB_EXPORT void faceAverageSize(TopoDS_Face const &face, double &uSize, double &vSize, double &uRange, double &vRange);

    FL5LIB_EXPORT bool discretizeEdge(const TopoDS_Edge& edge, int npts, std::vector<Vector3d>&points) ;

    FL5LIB_EXPORT void stitchFaces(double stitchprecision, TopoDS_Shape &theshape, TopoDS_Shell &theshell, std::string &logmsg);

    FL5LIB_EXPORT void makeWingShape(WingXfl const *pWing, double stitchprecision, TopoDS_Shape &wingshape, std::string &logmsg);
    FL5LIB_EXPORT bool makeFoilMidWires(Surface const &aSurf, TopoDS_Wire & LeftWire, TopoDS_Wire &RightWire, std::string &logmsg);
    FL5LIB_EXPORT bool makeFoilWires(Surface const &aSurf, TopoDS_Wire &TLWire, TopoDS_Wire & BLWire, TopoDS_Wire &TRWire, TopoDS_Wire &BRWire, std::string &logmsg);
    FL5LIB_EXPORT bool makePolyLineWire(std::vector<Vector3d> const &Pt, TopoDS_Wire & LeftWire, std::string &logmsg);
    FL5LIB_EXPORT bool makePolyLineWire(std::vector<Node> const &nd, TopoDS_Wire & LeftWire, std::string &logmsg);

    FL5LIB_EXPORT bool makeSplineWire(const BSpline3d &spline, TopoDS_Wire &wire, std::string &logmsg);

    FL5LIB_EXPORT void makeSurfaceWires(WingXfl const *pWing, double scalefactor, TopoDS_ListOfShape &wires, std::string &logmsg);

    FL5LIB_EXPORT bool makeFuseSolid(Fuse *pFuse, TopoDS_Solid &solidshape, std::string &logmsg);


    FL5LIB_EXPORT void makeFaceRuledTriangulation(TopoDS_Face const &face, std::vector<Vector3d> &pointlist, std::vector<Triangle3d> &trianglelist);

    FL5LIB_EXPORT int shellTriangulationWithOcc(TopoDS_Shell const &shell, OccMeshParams const &params, std::vector<Triangle3d> &triangles);
    FL5LIB_EXPORT int shapeTriangulationWithOcc(TopoDS_Shape const &shape, OccMeshParams const &params, std::vector<Triangle3d> &triangles);

    FL5LIB_EXPORT bool importCADShapes(const std::string &filename, TopoDS_ListOfShape &shapes, double &dimension, std::string &logmsg);
    FL5LIB_EXPORT bool importBRep(std::string const &filename, TopoDS_ListOfShape &shapes, double &dimension, std::string &logmsg);
    FL5LIB_EXPORT bool importSTEP(std::string const &filename, TopoDS_ListOfShape &shapes, double &dimension, std::string &logmsg);

    FL5LIB_EXPORT bool intersectFace(TopoDS_Face const &aFace, Segment3d const &seg, Vector3d &I);
    FL5LIB_EXPORT bool intersectShape(TopoDS_Shape const &aShape, Segment3d const &seg, Vector3d &I, bool bRightSide);
    FL5LIB_EXPORT void intersectShape(TopoDS_Shape const &aShape, std::vector<Segment3d> const &seg, std::vector<Vector3d> &I, std::vector<bool> &bIntersect);

    FL5LIB_EXPORT bool makeEquiTriangle(const TopoDS_Face &aFace, const Segment3d &baseseg, double maxedgelength, double growthfactor, Triangle3d &triangle) ;

    FL5LIB_EXPORT bool makeXflNurbsfromOccNurbs(Handle(Geom_BSplineSurface) occnurbs, NURBSSurface &xflnurbs);

    FL5LIB_EXPORT void flipShapeXZ(TopoDS_Shape &shape);
    FL5LIB_EXPORT void translateShape(TopoDS_Shape &shape, Vector3d const &T);
    FL5LIB_EXPORT void scaleShape(TopoDS_Shape &shape, double scalefactor);
    FL5LIB_EXPORT void rotateShape(TopoDS_Shape &shape, Vector3d const &O, Vector3d const &axis, double theta);

    FL5LIB_EXPORT void flipShapesXZ(TopoDS_ListOfShape &shapes);
    FL5LIB_EXPORT void translateShapes(TopoDS_ListOfShape &shapes, Vector3d const &T);
    FL5LIB_EXPORT void scaleShapes(TopoDS_ListOfShape &shapes, double scalefactor);
    FL5LIB_EXPORT void scaleShapes(TopoDS_ListOfShape &shapes, double xfactor, double yfactor, double zfactor);
    FL5LIB_EXPORT void rotateShapes(TopoDS_ListOfShape &shapes, Vector3d const &O, Vector3d const &axis, double theta);

    FL5LIB_EXPORT bool makeWing2NurbsShape(WingXfl const *pWing, double stitchprecision, int degree, int nCtrlPoints, int nOutPoints, TopoDS_Shape &wingshape, std::string &logmsg);
    FL5LIB_EXPORT bool makeWingSplineSweep(WingXfl const *pWing, double stitchprecision, int degree, int nCtrlPoints, int nOutPoints, TopoDS_Shape &wingshape, std::string &logmsg);

    FL5LIB_EXPORT bool makeWingSweepMidSection(WingXfl const *pWing, TopoDS_Shape &midshape, std::string &logmsg);
    FL5LIB_EXPORT bool makeWingSplineSweepMidSection(WingXfl const *pWing, int degree, int nCtrlPoints, int nOutPoints, TopoDS_Shape &wingshape, std::string &logmsg);

    FL5LIB_EXPORT bool makeWingSplineSweepMultiSections(WingXfl const *pWing, double stitchprecision, int degree, int nCtrlPoints, int nOutPoints, TopoDS_Shape &wingshape, std::string &logmsg);
    FL5LIB_EXPORT bool makeWingSplineSweepSolid(WingXfl const *pWing, double stitchprecision, int degree, int nCtrlPoints, int nOutPoints, TopoDS_Shape &wingshape, std::string &logmsg);

    FL5LIB_EXPORT void makeShapeEdges(TopoDS_Shape const &shape, std::vector<std::vector<Segment3d> > &edges);

    FL5LIB_EXPORT void getShapeFace(TopoDS_Shape const &shape, int iFace, TopoDS_Face &face);
    FL5LIB_EXPORT int nFaces(TopoDS_Shape const &shape);
    FL5LIB_EXPORT int nEdges(TopoDS_Shape const &shape);
    FL5LIB_EXPORT bool isSameEdge(TopoDS_Edge const &edge0, TopoDS_Edge const &edge1);
    FL5LIB_EXPORT void getEdge(TopoDS_Shape const &shape, int iFace, int iEdge, TopoDS_Edge &edge);

    FL5LIB_EXPORT void removeFace(const TopoDS_Face &face, TopoDS_Shape &shape);

    FL5LIB_EXPORT bool shapeToBrep(const TopoDS_Shape &shape, std::string &brep);
    FL5LIB_EXPORT bool shapesToBreps(const TopoDS_ListOfShape &shapes, std::vector<std::string> &breps);
}



