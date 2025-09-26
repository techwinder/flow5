/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once


#include <QString>
#include <QVector>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shell.hxx>
#include <Geom_Surface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>

#include <xflgeom/geom3d/triangle3d.h>
#include <xflgeom/geom3d/bspline3d.h>

class Fuse;
class WingXfl;
class Surface;
class OccMeshParams;
class NURBSSurface;

namespace occ
{
    QString shapeType(const TopoDS_Shape &aShape);
    QString shapeOrientation(TopoDS_Shape const &aShape);


    int listSubShapes(TopoDS_Shape const &aShape, TopAbs_ShapeEnum SubShapeType, QStringList &strList, QString prefix=QString());
    void listAllSubShapes(TopoDS_Shape const &aShape, QStringList &strList);
    void listShapeContent(TopoDS_Shape const &shape, QString &logmsg, const QString &prefix=QString(), bool bFull=false);
    void checkShape(TopoDS_Shape const &shape, QString & logmsg, const QString &prefix);

    bool makeOCCSplineFromPoints(QVector<Vector3d> const &pointList, Handle(Geom_BSplineCurve)& theSpline, QString &logmsg);
    bool makeOCCSplineFromBSpline3d(BSpline3d const &b3d, Handle(Geom_BSplineCurve)& hspline, QString &logmsg);
    bool makeOCCSplineFromBSpline3d_0(BSpline3d const &b3d, Handle(Geom_BSplineCurve)& hspline, QString &logmsg);
    void makeOCCNURBSFromNurbs(NURBSSurface const &nurbs, bool bXZSymmetric, Handle(Geom_BSplineSurface)& hnurbs, QString &logmsg);

    void makeFaceFromNodeStrip(const QVector<Node> &pts, TopoDS_Face &face, QString &log);
    void makeFaceFromTriangle(Vector3d const &Pt0, Vector3d const &Pt1, Vector3d const &Pt2, TopoDS_Face &face, QString &log);
    void makeFaceTriMesh(TopoDS_Face const &face, QVector<Triangle3d> &trianglelist, double maxelementsize);

    void findEdges(TopoDS_Shape const &shape, TopoDS_ListOfShape &edges, QString &logmsg);
    void findWires(const TopoDS_Shape &theshape, TopoDS_Wire &theOuterWire, TopoDS_ListOfShape &wires, QString &logmsg, QString prefix=QString());

    void shapeBoundingBox(const TopoDS_Shape &shape, double &Xmin, double &Ymin, double &Zmin, double &Xmax, double &Ymax, double &Zmax);

    double edgeLength(TopoDS_Edge const &edge);
    double edgeLength(TopoDS_Edge const &edge, double u0, double u1);

    void makeEdgeUniformSplitList(TopoDS_Face const &Face, TopoDS_Edge const &Edge, double maxlength, QVector<double> &uval);
    void makeEdgeUniformSplitList(TopoDS_Edge const &Edge, double maxlength, QVector<double> &uval);
    void makeEdgeSplitList(TopoDS_Edge const &Edge, double maxlength, double maxdeflection, QVector<double> &uval);

    double facePerimeter(TopoDS_Face const &face);
    double faceArea(TopoDS_Face const &face);
    double wireLength(TopoDS_Wire const &wire);
    void faceAverageSize(TopoDS_Face const &face, double &uSize, double &vSize, double &uRange, double &vRange);

    bool discretizeEdge(const TopoDS_Edge& edge, int npts, QVector<Vector3d>&points) ;

    void stitchFaces(double stitchprecision, TopoDS_Shape &theshape, TopoDS_Shell &theshell, QString &logmsg);

    void makeWingShape(WingXfl const *pWing, double stitchprecision, TopoDS_Shape &wingshape, QString &logmsg);
    void makeFoilWires(Surface const &aSurf, TopoDS_Wire &TLWire, TopoDS_Wire & BLWire, TopoDS_Wire &TRWire, TopoDS_Wire &BRWire, QString &logmsg);


    bool makeSplineWire(const BSpline3d &spline, TopoDS_Wire &wire, QString &logmsg);

    void makeSurfaceWires(WingXfl const *pWing, double scalefactor, TopoDS_ListOfShape &wires, QString &logmsg);

    bool makeFuseSolid(Fuse *pFuse, TopoDS_Solid &solidshape, QString &logmsg);


    void makeFaceRuledTriangulation(TopoDS_Face const &face, QVector<Vector3d> &pointlist, QVector<Triangle3d> &trianglelist);

    int shellTriangulationWithOcc(TopoDS_Shell const &shell, OccMeshParams const &params, QVector<Triangle3d> &triangles);
    int shapeTriangulationWithOcc(TopoDS_Shape const &shape, OccMeshParams const &params, QVector<Triangle3d> &triangles);

    bool importCADShapes(const QString &filename, TopoDS_ListOfShape &shapes, double &dimension, double stitchprecision, QString &logmsg, QObject *pParent=nullptr);
    bool importBRep(QString const &filename, TopoDS_ListOfShape &shapes, double &dimension, QString &logmsg, QObject *pParent);
    bool importSTEP(QString const &filename, TopoDS_ListOfShape &shapes, double &dimension, QString &logmsg, QObject *pParent);
    bool importIGES(QString const &filename, TopoDS_ListOfShape &shapes, double &dimension, double stitchprecision, QString &logmsg, QObject *pParent);

    bool intersectFace(TopoDS_Face const &aFace, Segment3d const &seg, Vector3d &I);
    bool intersectShape(TopoDS_Shape const &aShape, Segment3d const &seg, Vector3d &I, bool bRightSide);
    void intersectShape(TopoDS_Shape const &aShape, QVector<Segment3d> const &seg, QVector<Vector3d> &I, QVector<bool> &bIntersect);

    bool makeEquiTriangle(const TopoDS_Face &aFace, const Segment3d &baseseg, double maxedgelength, double growthfactor, Triangle3d &triangle) ;

    bool makeXflNurbsfromOccNurbs(Handle(Geom_BSplineSurface) occnurbs, NURBSSurface &xflnurbs);

    void flipShapeXZ(TopoDS_Shape &shape);
    void translateShape(TopoDS_Shape &shape, Vector3d const &T);
    void scaleShape(TopoDS_Shape &shape, double scalefactor);
    void rotateShape(TopoDS_Shape &shape, Vector3d const &O, Vector3d const &axis, double theta);

    void flipShapesXZ(TopoDS_ListOfShape &shapes);
    void translateShapes(TopoDS_ListOfShape &shapes, Vector3d const &T);
    void scaleShapes(TopoDS_ListOfShape &shapes, double scalefactor);
    void scaleShapes(TopoDS_ListOfShape &shapes, double xfactor, double yfactor, double zfactor);
    void rotateShapes(TopoDS_ListOfShape &shapes, Vector3d const &O, Vector3d const &axis, double theta);

    bool makeWing2NurbsShape(WingXfl const *pWing, double stitchprecision, int degree, int nCtrlPoints, int nOutPoints, TopoDS_Shape &wingshape, QString &logmsg);
    bool makeWingSplineSweep(WingXfl const *pWing, double stitchprecision, int degree, int nCtrlPoints, int nOutPoints, TopoDS_Shape &wingshape, QString &logmsg);

    bool makeWingSplineSweepMultiSections(WingXfl const *pWing, double stitchprecision, int degree, int nCtrlPoints, int nOutPoints, TopoDS_Shape &wingshape, QString &logmsg);
//    bool makeWingSplineSweepSolid(WingXfl const *pWing, double stitchprecision, int nCtrlPoints, int nOutPoints, TopoDS_Shape &wingshape, QString &logmsg);
    bool makeWingSplineSweepSolid(WingXfl const *pWing, double stitchprecision, int degree, int nCtrlPoints, int nOutPoints, TopoDS_Shape &wingshape, QString &logmsg);

    void makeShapeEdges(TopoDS_Shape const &shape, QVector<QVector<Segment3d> > &edges);

    void getShapeFace(TopoDS_Shape const &shape, int iFace, TopoDS_Face &face);
    int nFaces(TopoDS_Shape const &shape);
    int nEdges(TopoDS_Shape const &shape);
    bool isSameEdge(TopoDS_Edge const &edge0, TopoDS_Edge const &edge1);
    void getEdge(TopoDS_Shape const &shape, int iFace, int iEdge, TopoDS_Edge &edge);

    void removeFace(const TopoDS_Face &face, TopoDS_Shape &shape);

}
