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

#include <QString>

#include <iostream>
#include <filesystem>

#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeShell.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepTools.hxx>
#include <BRepTools_ReShape.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <GProp_GProps.hxx>
#include <GeomAPI_IntCS.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <GeomLProp_SLProps.hxx>
#include <Geom_Line.hxx>
#include <Geom_UndefinedValue.hxx>
#include <IntCurvesFace_ShapeIntersector.hxx>
#include <Interface_EntityIterator.hxx>
#include <Interface_Static.hxx>
#include <STEPControl_Reader.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <StdFail_NotDone.hxx>
#include <StepBasic_LengthMeasureWithUnit.hxx>
#include <StepData_StepModel.hxx>
#include <TCollection_AsciiString.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Builder.hxx>
#include <TopoDS_FrozenShape.hxx>
#include <TopoDS_UnCompatibleShapes.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Ax2.hxx>
#include <gp_GTrsf.hxx>
#include <gp_Lin.hxx>
#include <gp_Trsf.hxx>

#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <BRepCheck_ListOfStatus.hxx>
#include <TColStd_SequenceOfAsciiString.hxx>

#include <occ_globals.h>
#include <constants.h>
#include <wingxfl.h>
#include <fuse.h>
#include <occmeshparams.h>
#include <geom_global.h>
#include <nurbssurface.h>
#include <bspline3d.h>

std::string occ::shapeType(TopoDS_Shape const &aShape)
{
    std::string ShapeType;
    if(aShape.IsNull())
    {
        ShapeType = "NULL";
        return ShapeType;
    }

    switch (aShape.ShapeType())
    {
        case TopAbs_EDGE:      ShapeType = "EDGE";           break;
        case TopAbs_FACE:      ShapeType = "FACE";           break;
        case TopAbs_WIRE:      ShapeType = "WIRE";           break;
        case TopAbs_SHAPE:     ShapeType = "SHAPE";          break;
        case TopAbs_SHELL:     ShapeType = "SHELL";          break;
        case TopAbs_COMPOUND:  ShapeType = "COMPOUND";       break;
        case TopAbs_COMPSOLID: ShapeType = "COMPSOLID";      break;
        case TopAbs_SOLID:     ShapeType = "SOLID";          break;
        default: ShapeType = "something else"; break;
    }
    return ShapeType;
}


std::string occ::shapeOrientation(const TopoDS_Shape &aShape)
{
    std::string ShapeOrientation;
    if(aShape.IsNull())
    {
        ShapeOrientation = "NULL";
        return ShapeOrientation;
    }
    switch (aShape.Orientation())
    {
        case TopAbs_FORWARD:      ShapeOrientation = "FORWARD";           break;
        case TopAbs_REVERSED:     ShapeOrientation = "REVERSED";          break;
        case TopAbs_INTERNAL:     ShapeOrientation = "INTERNAL";          break;
        case TopAbs_EXTERNAL:     ShapeOrientation = "EXTERNAL";          break;
    }
    return ShapeOrientation;
}


int occ::listSubShapes(TopoDS_Shape const &aShape, TopAbs_ShapeEnum SubShapeType, std::vector<std::string> &strList, std::string prefx)
{
    QString prefix = QString::fromStdString(prefx);
    QString strange;
    strList.clear();
    TopExp_Explorer shapeExplorer;
    int nSub = 0;
    for(shapeExplorer.Init(aShape, SubShapeType); shapeExplorer.More(); shapeExplorer.Next())
    {
        TopoDS_Shape aSub = shapeExplorer.Current();
        strange = QString::asprintf("SubShape %2d:  ", nSub);
        strange = prefix + strange + QString::fromStdString(shapeType(aSub));
        if     (aSub.Orientation()==TopAbs_FORWARD)  strange += "_FORWARD";
        else if(aSub.Orientation()==TopAbs_REVERSED) strange += "_REVERSED";

        strList.push_back(strange.toStdString());

        nSub++;
    }
    return nSub;
}


void occ::listAllSubShapes(TopoDS_Shape const &aShape, std::vector<std::string> &strList)
{

    TopExp_Explorer shapeExplorer;
    int nSub = 0;

    nSub = 0;
    for (shapeExplorer.Init(aShape, TopAbs_COMPOUND); shapeExplorer.More(); shapeExplorer.Next()) nSub++;
    strList.push_back(QString::asprintf("Nb. COMPOUND  = %d", nSub).toStdString());

    nSub = 0;
    for (shapeExplorer.Init(aShape, TopAbs_COMPSOLID); shapeExplorer.More(); shapeExplorer.Next()) nSub++;
    strList.push_back(QString::asprintf("Nb. COMPSOLID = %d", nSub).toStdString());

    nSub = 0;
    for (shapeExplorer.Init(aShape, TopAbs_SOLID); shapeExplorer.More(); shapeExplorer.Next()) nSub++;
    strList.push_back(QString::asprintf("Nb. SOLID     = %d", nSub).toStdString());

    nSub = 0;
    for (shapeExplorer.Init(aShape, TopAbs_SHELL); shapeExplorer.More(); shapeExplorer.Next()) nSub++;
    strList.push_back(QString::asprintf("Nb. SHELL     = %d", nSub).toStdString());

    nSub = 0;
    for (shapeExplorer.Init(aShape, TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next()) nSub++;
    strList.push_back(QString::asprintf("Nb. FACE      = %d", nSub).toStdString());

    nSub = 0;
    for (shapeExplorer.Init(aShape, TopAbs_WIRE); shapeExplorer.More(); shapeExplorer.Next()) nSub++;
    strList.push_back(QString::asprintf("Nb. WIRE      = %d", nSub).toStdString());

    nSub = 0;
    for (shapeExplorer.Init(aShape, TopAbs_EDGE); shapeExplorer.More(); shapeExplorer.Next()) nSub++;
    strList.push_back(QString::asprintf("Nb. EDGE      = %d", nSub).toStdString());

    nSub = 0;
    for (shapeExplorer.Init(aShape, TopAbs_VERTEX); shapeExplorer.More(); shapeExplorer.Next()) nSub++;
    strList.push_back(QString::asprintf("Nb. VERTEX    = %d", nSub).toStdString());
}


void occ::listShapeContent(TopoDS_Shape const &shape, std::string &logmsg, std::string const &prefx, bool bFull)
{
    QString prefix = QString::fromStdString(prefx);
    QString logg;
    logg = prefix +"Shape is a " + QString::fromStdString(occ::shapeType(shape)) + " made of\n";
    std::vector<std::string> strList;

    int nSolids = occ::listSubShapes(shape, TopAbs_SOLID, strList);
    logg += prefix + QString::asprintf("   %d SOLID(s)\n", nSolids);
    if(bFull)
    {
        for(std::string const& str : strList)
        {
            logg += prefix + "      "+ QString::fromStdString(str) + "\n";
        }
    }

    int nShells = occ::listSubShapes(shape, TopAbs_SHELL, strList);
    logg += prefix + QString::asprintf("   %d SHELL(s)\n", nShells);
    if(bFull)
    {
        for(std::string const& str : strList)
        {
            logg += prefix + "      "+ QString::fromStdString(str) + "\n";
        }
    }

    int nFaces  = listSubShapes(shape, TopAbs_FACE, strList);
    logg += prefix + QString::asprintf("   %d FACE(s)\n", nFaces);
    if(bFull)
    {
        for(std::string const& str : strList)
        {
            logg += prefix + "      "+ QString::fromStdString(str) + "\n";
        }
    }

    int nWires  = occ::listSubShapes(shape, TopAbs_WIRE, strList);
    logg += prefix + QString::asprintf("   %d WIRE(s)\n", nWires);
    int nEdges  = occ::listSubShapes(shape, TopAbs_EDGE, strList);
    logg += prefix + QString::asprintf("   %d EDGE(s)", nEdges);
    logg += "\n";

    logmsg = logg.toStdString();
}


void occ::checkShape(TopoDS_Shape const &shape, std::string & logmsg, std::string const & prefix)
{
    logmsg.clear();
/*    logmsg += prefix + std::string("Shape status:\n");
    logmsg += prefix + std::string("   checked:  ") + (shape.Checked()  ? "true" : "false") + std::string("\n");
    logmsg += prefix + std::string("   closed:   ") + (shape.Closed()   ? "true" : "false") + std::string("\n");
    logmsg += prefix + std::string("   convex:   ") + (shape.Convex()   ? "true" : "false") + std::string("\n");
    logmsg += prefix + std::string("   free:     ") + (shape.Free()     ? "true" : "false") + std::string("\n");
    logmsg += prefix + std::string("   infinite: ") + (shape.Infinite() ? "true" : "false") + std::string("\n");*/

    BRepCheck_Analyzer ShapeAnalyzer(shape);
    if(ShapeAnalyzer.IsValid()) logmsg += "   Shape topology is VALID\n\n";
    else
    {
        logmsg += prefix + "   Shape topology is NOT VALID\n";
        logmsg += prefix + "   BRepCheck_Analyzer status:\n";

        const Handle(BRepCheck_Result)& result = ShapeAnalyzer.Result(shape);
        const BRepCheck_ListOfStatus& status = result->StatusOnShape(shape);

        BRepCheck_ListIteratorOfListOfStatus it(status);
        while (it.More())
        {
            logmsg += prefix+"      ";
            BRepCheck_Status& val = it.Value();
            switch (val)
            {
                default:
                case BRepCheck_NoError:
                    logmsg += "No error";
                    break;
                case BRepCheck_InvalidImbricationOfShells:
                    logmsg += "Invalid imbrication of shells";
                    break;
                case BRepCheck_InvalidPolygonOnTriangulation:
                    logmsg += "Invalid polygon on triangulation";
                    break;
                case BRepCheck_EnclosedRegion:
                    logmsg += "Enclosed region";
                    break;
                case BRepCheck_InvalidPointOnCurve:
                    logmsg += "Invalid point on curve";
                    break;
                case BRepCheck_InvalidPointOnCurveOnSurface:
                    logmsg += "Invalid point on curve on surface";
                    break;
                case BRepCheck_InvalidPointOnSurface:
                    logmsg += "Invalid point on surface";
                    break;
                case BRepCheck_No3DCurve:
                    logmsg += "No 3d curve";
                    break;
                case BRepCheck_Multiple3DCurve:
                    logmsg += "Multiple 3d curve";
                    break;
                case BRepCheck_Invalid3DCurve:
                    logmsg += "Invalid 3d curve";
                    break;
                case BRepCheck_NoCurveOnSurface:
                    logmsg += "No curve on surface";
                    break;
                case BRepCheck_InvalidCurveOnSurface:
                    logmsg += "Invalid curve on surface";
                    break;
                case BRepCheck_InvalidCurveOnClosedSurface:
                    logmsg += "Invalid curve on closed surface";
                    break;
                case BRepCheck_InvalidSameRangeFlag:
                    logmsg += "Invalid same-range flag";
                    break;
                case BRepCheck_InvalidSameParameterFlag:
                    logmsg += "Invalid same-parameter flag";
                    break;
                case BRepCheck_InvalidDegeneratedFlag:
                    logmsg += "Invalid degenerated flag";
                    break;
                case BRepCheck_FreeEdge:
                    logmsg += "Free edge";
                    break;
                case BRepCheck_InvalidMultiConnexity:
                    logmsg += "Invalid multi-connexity";
                    break;
                case BRepCheck_InvalidRange:
                    logmsg += "Invalid range";
                    break;
                case BRepCheck_EmptyWire:
                    logmsg += "Empty wire";
                    break;
                case BRepCheck_RedundantEdge:
                    logmsg += "Redundant edge";
                    break;
                case BRepCheck_SelfIntersectingWire:
                    logmsg += "Self-intersecting wire";
                    break;
                case BRepCheck_NoSurface:
                    logmsg += "No surface";
                    break;
                case BRepCheck_InvalidWire:
                    logmsg += "Invalid wires";
                    break;
                case BRepCheck_RedundantWire:
                    logmsg += "Redundant wires";
                    break;
                case BRepCheck_IntersectingWires:
                    logmsg += "Intersecting wires";
                    break;
                case BRepCheck_InvalidImbricationOfWires:
                    logmsg += "Invalid imbrication of wires";
                    break;
                case BRepCheck_EmptyShell:
                    logmsg += "Empty shell";
                    break;
                case BRepCheck_RedundantFace:
                    logmsg += "Redundant face";
                    break;
                case BRepCheck_UnorientableShape:
                    logmsg += "Unorientable shape";
                    break;
                case BRepCheck_NotClosed:
                    logmsg += "Not closed";
                    break;
                case BRepCheck_NotConnected:
                    logmsg += "Not connected";
                    break;
                case BRepCheck_SubshapeNotInShape:
                    logmsg += "Sub-shape not in shape";
                    break;
                case BRepCheck_BadOrientation:
                    logmsg += "Bad orientation";
                    break;
                case BRepCheck_BadOrientationOfSubshape:
                    logmsg += "Bad orientation of sub-shape";
                    break;
                case BRepCheck_InvalidToleranceValue:
                    logmsg += "Invalid tolerance value";
                    break;
                case BRepCheck_CheckFail:
                    logmsg += "Check failed";
                    break;
            }
            logmsg += "\n";
            it.Next();
        }
    }
}


void occ::makeFaceFromTriangle(Vector3d const &P1, Vector3d const &P2, Vector3d const &P3, TopoDS_Face &face, std::string &log)
{
    std::string strong;
    try
    {
        BRepBuilderAPI_MakeWire FaceWireMaker1;
        if(!P1.isSame(P2) && !P2.isSame(P3) && !P3.isSame(P1))
        {
            TopoDS_Edge Edge12 = BRepBuilderAPI_MakeEdge(gp_Pnt(P1.x, P1.y, P1.z), gp_Pnt(P2.x, P2.y, P2.z));
            FaceWireMaker1.Add(Edge12);

            TopoDS_Edge Edge23 = BRepBuilderAPI_MakeEdge(gp_Pnt(P2.x, P2.y, P2.z), gp_Pnt(P3.x, P3.y, P3.z));
            FaceWireMaker1.Add(Edge23);

            TopoDS_Edge Edge31 = BRepBuilderAPI_MakeEdge(gp_Pnt(P3.x, P3.y, P3.z), gp_Pnt(P1.x, P1.y, P1.z));
            FaceWireMaker1.Add(Edge31);

            if(!FaceWireMaker1.IsDone())
            {
                switch(FaceWireMaker1.Error())
                {
                    case BRepBuilderAPI_WireDone:
                        log += "   Trace::BRepBuilderAPI_WireDone\n"; break;
                    case BRepBuilderAPI_EmptyWire:
                        log += "   Trace::BRepBuilderAPI_EmptyWire\n"; break;
                    case BRepBuilderAPI_DisconnectedWire:
                        log += "   Trace::BRepBuilderAPI_DisconnectedWire\n"; break;
                    case BRepBuilderAPI_NonManifoldWire:
                        log += "   Trace::BRepBuilderAPI_NonManifoldWire\n"; break;
                }
                log += strong;
            }
            else
            {
                TopoDS_Wire FaceWire = FaceWireMaker1.Wire();
                BRepBuilderAPI_MakeFace FaceMaker(FaceWire);
                if(!FaceMaker.IsDone())
                {
                    switch(FaceMaker.Error())
                    {
                        case BRepBuilderAPI_FaceDone:
                            log += "   Trace::BRepBuilderAPI_FaceDone\n"; break;
                        case BRepBuilderAPI_NoFace:
                            log += "   Trace::BRepBuilderAPI_NoFace\n"; break;
                        case BRepBuilderAPI_NotPlanar:
                            log += "   Trace::BRepBuilderAPI_NotPlanar\n"; break;
                        case BRepBuilderAPI_CurveProjectionFailed:
                            log += "   Trace::BRepBuilderAPI_CurveProjectionFailed\n"; break;
                        case BRepBuilderAPI_ParametersOutOfRange:
                            log += "   Trace::BRepBuilderAPI_ParametersOutOfRange\n"; break;
                    }
                    log+= strong;
                }
                else
                {
                    face = FaceMaker.Face();
                }
            }
        }
    }
    catch(StdFail_NotDone const &ex)
    {
        std::string strong =  "   Error making body face\n" + std::string(ex.GetMessageString()) +"\n";
        log += strong;
        return;
    }
}


/** incomplete */
void occ::makeFaceTriMesh(TopoDS_Face const &face, std::vector<Triangle3d> &trianglelist, double maxelementsize)
{
    if(face.IsNull()) return;

    std::vector<Segment3d> PSLG3d;

    // make a Quad3d from the TopoDS_Face
//    BRepAdaptor_Surface surfaceadaptor(face);
//    GeomAdaptor_Surface aGAS = surfaceadaptor.Surface();

    // make a PSLG from the edges
    TopExp_Explorer facexplorer;

    for (facexplorer.Init(face, TopAbs_WIRE); facexplorer.More(); facexplorer.Next())
    {
        TopoDS_Wire wire = TopoDS::Wire(facexplorer.Current());
        TopExp_Explorer wireexplorer;

        int iEdge=0; // current face index
        for (wireexplorer.Init(wire, TopAbs_EDGE); wireexplorer.More(); wireexplorer.Next())
        {
            TopoDS_Edge edge = TopoDS::Edge(wireexplorer.Current());
            BRepAdaptor_Curve curveadaptor(edge);

            double Umin = curveadaptor.FirstParameter();
            double Umax = curveadaptor.LastParameter();

            //make the node array
            gp_Pnt pt0;
            pt0 = curveadaptor.Value(Umin);

            gp_Pnt pt1;
            pt1 = curveadaptor.Value(Umax);

            Segment3d seg(Vector3d(pt0.X(), pt0.Y(), pt0.Z()), Vector3d(pt1.X(), pt1.Y(), pt1.Z()));
            if(seg.length()>=1.5*maxelementsize)
            {
                std::vector<Segment3d> seglist = seg.split(maxelementsize);
                assert(seglist.size()>=1);
                for(Segment3d const &seg : seglist)
                    PSLG3d.push_back(seg);
            }
            else     PSLG3d.push_back({Vector3d(pt0.X(), pt0.Y(), pt0.Z()), Vector3d(pt1.X(), pt1.Y(), pt1.Z())});
            iEdge++; // next edge
            //            PSLG3d.back().displayNodes("PSLG3d:");
        }
        (void)iEdge;
    }

    // restore somehow
    //    facequad.splitFace(PSLG3d, trianglelist);
    trianglelist.clear();
}


void occ::findWires(const TopoDS_Shape &theshape, TopoDS_Wire &theOuterWire, TopoDS_ListOfShape &wires,
                    std::string &, std::string )
{
    if(theshape.IsNull()) return;

    ShapeAnalysis_FreeBounds shapeAnalyzer(theshape); // using the constructor for a "compound of shells" even though the shape is a Face?
    const TopoDS_Compound& closedwires = shapeAnalyzer.GetClosedWires();
    double squarediag = -1.0;
    int nClosed=0;
    for(TopExp_Explorer iter(closedwires, TopAbs_WIRE); iter.More(); iter.Next())
    {
        wires.Append(iter.Current());
        Bnd_Box box;
        BRepBndLib::Add(iter.Current(), box);

        if (box.SquareExtent()>squarediag)
        {
            squarediag  = box.SquareExtent();
            theOuterWire = TopoDS::Wire(iter.Current());
        }
        nClosed++;
    }
    (void)nClosed;
//    strange = QString::asprintf("   Includes %d closed wires\n", nClosed);
//    logmsg += prefix + strange;

    const TopoDS_Compound& openwires = shapeAnalyzer.GetOpenWires();
    int nOpen=0;
    for(TopExp_Explorer iter(openwires, TopAbs_WIRE); iter.More() ; iter.Next())
    {
        wires.Append(iter.Current());
        nOpen++;
    }
    (void)nOpen;
//    strange = QString::asprintf("   Includes %d open wires\n", nOpen);
//    logmsg += prefix + strange;
}


void occ::findEdges(TopoDS_Shape const &theshape, TopoDS_ListOfShape &edges, std::string &logmsg)
{
    std::string strange;

    if(theshape.IsNull())
    {
        logmsg += "   invalid shape\n";
        return;
    }
    int nValid= 0;
    int nInvalid = 0;
    TopExp_Explorer shapeExplorer;
    for (shapeExplorer.Init(theshape, TopAbs_EDGE); shapeExplorer.More(); shapeExplorer.Next())
    {
        TopoDS_Edge anEdge = TopoDS::Edge(shapeExplorer.Current());
        if(anEdge.IsNull())
        {
            nInvalid++;
        }
        else
        {
            edges.Append(anEdge);
            nValid++;
        }
    }

    strange = QString::asprintf("   Found %d valid EDGE\n   Found %d invalid EDGE\n", nValid, nInvalid).toStdString();
    logmsg.append(strange);
}


/** returns the edge lengths of the box which bounds the shape */
void occ::shapeBoundingBox(TopoDS_Shape const &shape, double &Xmin, double &Ymin, double &Zmin, double &Xmax, double &Ymax, double &Zmax)
{
    TopExp_Explorer shapeExplorer;
    double xmin=0, ymin=0, zmin=0;
    double xmax=0, ymax=0, zmax=0;
    for (shapeExplorer.Init(shape, TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
    {
        TopoDS_Shape aSub = shapeExplorer.Current();
        Bnd_Box box;
        BRepBndLib::Add(aSub, box); // Use triangulation in this case.
        box.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        Xmin = std::min(xmin, Xmin);
        Ymin = std::min(ymin, Ymin);
        Zmin = std::min(zmin, Zmin);
        Xmax = std::max(xmax, Xmax);
        Ymax = std::max(ymax, Ymax);
        Zmax = std::max(zmax, Zmax);
    }
}


void occ::makeEdgeUniformSplitList(TopoDS_Face const &Face, TopoDS_Edge const &Edge, double maxlength, std::vector<double> &uval)
{
    uval.clear();
    double length = occ::edgeLength(Edge);
    int nSegs = std::max(1, int(std::round(length/maxlength)));

/*    if(nSegs==1)
    {
        uval.append({1.0e-5, 0.99999});
        return;
    }*/

    try
    {
        BRepAdaptor_Curve curveAdaptor;
        curveAdaptor.Initialize(Edge, Face);

        GCPnts_UniformAbscissa uniformAbscissa;
        uniformAbscissa.Initialize(curveAdaptor, nSegs);

        if(uniformAbscissa.IsDone())
        {
            int NPoints = uniformAbscissa.NbPoints();
            for(int i=1; i<=NPoints; i++)
            {
                double param = uniformAbscissa.Parameter(i);
                uval.push_back(param);
            }
        }

        if(uval.size()<2)
        {
            // NPoints may be 1 if nSegs=1
            uval.resize(2);
            uval.front() = curveAdaptor.FirstParameter();
            uval.back()  = curveAdaptor.LastParameter();
        }
    }
    catch(Standard_Failure &)
    {
        qDebug("Standard failure making EdgeUniformSplitList\n");
    }
    catch(...)
    {
        qDebug("unknown error making EdgeUniformSplitList\n");
    }

//    if(nSegs==1) qDebug("makeEdgeUniformSplitList  %11g  %11g  %11g", uval.at(0), uval.at(1), Precision::Confusion());

}

void occ::makeEdgeUniformSplitList(TopoDS_Edge const &Edge, double maxlength, std::vector<double> &uval)
{
    uval.clear();
    double length = occ::edgeLength(Edge);
    int nSegs = std::max(1, int(std::round(length/maxlength)));
    int nPts = nSegs+1;

    try
    {
        BRepAdaptor_Curve curveAdaptor;
        curveAdaptor.Initialize(Edge);

        GCPnts_UniformAbscissa uniformAbscissa;
        uniformAbscissa.Initialize(curveAdaptor, nPts);

        if(uniformAbscissa.IsDone())
        {
            int NPoints = uniformAbscissa.NbPoints();
            for(int i=1; i<=NPoints; i++)
            {
                double param = uniformAbscissa.Parameter(i);
                uval.push_back(param);
            }
        }

        if(uval.size()<2)
        {
            // NPoints may be 1 if nSegs=1
            uval.resize(2);
            uval.front() = curveAdaptor.FirstParameter();
            uval.back()  = curveAdaptor.LastParameter();
        }
    }
    catch(Standard_Failure &)
    {
        qDebug("Standard failure making EdgeUniformSplitList\n");
    }
    catch(...)
    {
        qDebug("unknown error making EdgeUniformSplitList\n");
    }

//    if(nSegs==1) qDebug("makeEdgeUniformSplitList  %11g  %11g  %11g", uval.at(0), uval.at(1), Precision::Confusion());

}


void occ::makeEdgeSplitList(TopoDS_Edge const &Edge, double maxlength, double maxdeflection, std::vector<double> &uval)
{
    double length{0};
    //get the curve in the face's parametric space
    double curvemin=0, curvemax=0;
    Handle(Geom_Curve) hCurve = BRep_Tool::Curve(Edge, curvemin, curvemax);

    uval = {curvemin, curvemax};

    gp_Pnt P0, P1, P12;
    hCurve->D0(uval.front(), P0);
    hCurve->D0(uval.back(), P1);
    hCurve->D0((uval.front()+uval.back())/2.0, P12);
    //get the distance from P12 to segment [P0,P1]
    Vector3d V0 = {P0.X(),P0.Y(),P0.Z()};
    Vector3d V1 = {P1.X(),P1.Y(),P1.Z()};
    Vector3d V12 = {P12.X(),P12.Y(),P12.Z()};
    double deflection = 0.0;

    int iter=0;
    do
    {
        for(int i=int(uval.size())-1; i>=1; i--)
        {
            hCurve->D0(uval.at(i), P0);
            hCurve->D0(uval.at(i-1), P1);
            hCurve->D0((uval.at(i-1)+uval.at(i))/2.0, P12);
            //get the distance from P12 to segment [P0,P1]
            Vector3d V0  = {P0.X(),P0.Y(),P0.Z()};
            Vector3d V1  = {P1.X(),P1.Y(),P1.Z()};
            Vector3d V12 = {P12.X(),P12.Y(),P12.Z()};

            double dl = P0.Distance(P1);

            bool bInsert = false;
            if     (dl>maxlength)    bInsert = true;

            if(bInsert)
            {
                uval.insert(uval.begin()+i, (uval.at(i)+uval.at(i-1))/2.0);
            }
        }
        //update segment length
        length = 0.0;
        deflection = 0.0;

        for(uint i=0; i<uval.size()-1; i++)
        {
            hCurve->D0(uval.at(i), P0);
            hCurve->D0(uval.at(i+1), P1);
            hCurve->D0((uval.at(i)+uval.at(i+1))/2.0, P12);
            //get the distance from P12 to segment [P0,P1]
            Vector3d V0 = {P0.X(),P0.Y(),P0.Z()};
            Vector3d V1 = {P1.X(),P1.Y(),P1.Z()};
            Vector3d V12 = {P12.X(),P12.Y(),P12.Z()};

            double dl = P0.Distance(P1);
            length = std::max(length, dl);
            if(length>maxlength) break; // another loop iteration is required

/*            double h = distanceToLine3d(V0, V1, V12);
            deflection = std::max(deflection, h);
            if(deflection>maxdeflection) break; // another loop iteration is required*/
        }

        iter++;
    }
    while ((length>maxlength || deflection>maxdeflection) && iter<100);

    // normalize parameters to [0,1] interval
    double u0 = uval.front();
    double range = uval.back()-uval.front();
    for(uint i=0; i<uval.size(); i++)
    {
        uval[i] = (uval.at(i)-u0)/range;
//        hCurve->D0(uval.at(i), P0);
    }
}

/** Returns the edge's length from p-space parameters u0 to u1 */
double occ::edgeLength(TopoDS_Edge const &edge, double u0, double u1)
{
    int nsegs = 11;
    BRepAdaptor_Curve CA;
    CA.Initialize(edge);
    gp_Pnt P0, P1;
    double l = 0;
    CA.D0(u0, P0);
    for(int i=1; i<nsegs; i++)
    {
        double u = u0 + double(i)/double(nsegs-1) * (u1-u0);
        CA.D0(u, P1);
        l += P0.Distance(P1);
        P0 = P1;
    }
    return l;
}


double occ::edgeLength(TopoDS_Edge const &edge)
{
    GProp_GProps edgeprops;
    BRepGProp::LinearProperties(edge, edgeprops);
    return edgeprops.Mass();// The resulting mass is equal to the length.
}


double occ::faceArea(TopoDS_Face const &face)
{
    GProp_GProps edgeprops;
    BRepGProp::SurfaceProperties(face, edgeprops);
    return edgeprops.Mass();// The resu
}


double occ::facePerimeter(TopoDS_Face const &face)
{
    std::string strange;

    TopoDS_ListOfShape innerwires;
    TopoDS_Wire theouterwire;
    // may have free edges, so need to find the outer wire
    occ::findWires(face, theouterwire, innerwires, strange);
    if(theouterwire.IsNull()) return 0.0;

    return occ::wireLength(theouterwire);
}


double occ::wireLength(TopoDS_Wire const &wire)
{
    TopExp_Explorer shapeExplorer;
    double l=0;
    for (shapeExplorer.Init(wire, TopAbs_EDGE); shapeExplorer.More(); shapeExplorer.Next())
    {
        TopoDS_Edge const &edge = TopoDS::Edge(shapeExplorer.Current());
        if(!edge.IsNull())
            l += edgeLength(edge);
    }
    return l;
}


/** calculates measures of the Face's geometric length
 * in the parametric u and v directions */
void occ::faceAverageSize(TopoDS_Face const &face,
                     double &ulength, double &vlength, double &uRange, double &vRange)
{
    int NMEASURES = 5;

    TopoDS_ListOfShape thewires;
    TopoDS_Wire theouterwire;

    //Find the Face's average length in u and v directions
    BRepAdaptor_Surface SA(face);

    double u=0,v=0,du=0,dv=0;
    double umin=0, umax=0, vmin=0, vmax=0;
    umin = SA.FirstUParameter();
    umax = SA.LastUParameter();
    vmin = SA.FirstVParameter();
    vmax = SA.LastVParameter();

    uRange = umax-umin;
    vRange = vmax-vmin;

    std::vector<gp_Pnt> pnts(NMEASURES*NMEASURES);
    du = (umax-umin)/double(NMEASURES-1);
    dv = (vmax-vmin)/double(NMEASURES-1);
    for(int i=0; i<NMEASURES; i++)
    {
        u = umin + double(i)*du;
        for(int j=0; j<NMEASURES; j++)
        {
            v = vmin + double(j)*dv;
            pnts[i*NMEASURES+j] = SA.Value(u, v);
        }
    }

    ulength = 0.0;
    for(int jv=0; jv<NMEASURES; jv++)
    {
        double urowlength=0;
        for(int iu=0; iu<NMEASURES-1; iu++)
        {
            urowlength += pnts[iu*NMEASURES+jv].Distance(pnts[(iu+1)*NMEASURES+jv]);
        }
        ulength += urowlength;
//        ulength = std::max(ulength, urowlength);
    }
    ulength *=1.0/double(NMEASURES);

    vlength = 0.0;
    for(int iu=0; iu<NMEASURES; iu++)
    {
        double vcollength=0;
        for(int jv=0; jv<NMEASURES-1; jv++)
        {
            vcollength += pnts[iu*NMEASURES+jv].Distance(pnts[iu*NMEASURES+jv+1]);
        }
        vlength += vcollength;
//        vlength = std::max(vlength, vcollength);
    }
    vlength *=1.0/double(NMEASURES);
}


bool occ::discretizeEdge(const TopoDS_Edge& edge, int npts, std::vector<Vector3d>&points)
{
    BRepAdaptor_Curve curve_adaptator (edge);
    GCPnts_UniformAbscissa discretizer(curve_adaptator, npts);

    if (!discretizer.IsDone () || discretizer.NbPoints ()==0) return false;

    points.resize(discretizer.NbPoints());
    for (int i=1; i<discretizer.NbPoints(); i++)
    {
        gp_Pnt p = curve_adaptator.Value (discretizer.Parameter (i));
        points.push_back({p.X(), p.Y(), p.Z()});
    }
    return true;
}


void occ::stitchFaces(double stitchprecision, TopoDS_Shape &theshape, TopoDS_Shell &theshell, std::string &logmsg)
{
    BRepBuilderAPI_Sewing stitcher(stitchprecision);

    // stitch
    TopExp_Explorer shapeExplorer;
    for (shapeExplorer.Init(theshape, TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
    {
        TopoDS_Shape aSub = shapeExplorer.Current();
        stitcher.Add(aSub);
    }
    stitcher.Perform();
    logmsg += QString::asprintf("   Nb of free edges=%d\n", stitcher.NbFreeEdges()).toStdString();
    logmsg += QString::asprintf("   Nb of contiguous edges=%d\n", stitcher.NbContigousEdges()).toStdString();

/*    If all faces have been sewn correctly, the result is a shell. Otherwise, it is a compound.
    After a successful sewing operation all faces have a coherent orientation.*/

    try
    {
        TopoDS_Shape sewedshape = stitcher.SewedShape();
        if(sewedshape.IsNull()) return;

        theshell = TopoDS::Shell(sewedshape);
        //make the solid
        if(!theshell.IsNull())
        {
            BRepBuilderAPI_MakeSolid solidMaker(theshell);
            if(!solidMaker.IsDone())
            {
                logmsg += "   Solid not made... \n";
                theshape.Nullify();
                return;
            }
            theshape = solidMaker.Shape();

            logmsg += "   Wing stitching result is " + occ::shapeType(theshape) + "\n";

            occ::checkShape(theshape, logmsg, "");
        }
    }
    catch(Standard_TypeMismatch const &)
    {
        logmsg += "     ****** StitchFaces:: Type mismatch error *********\n"
                  "                          Increase the sewing precision\n"
                  "                          Recommendation: 0.1 mm\n";
    }
}


bool occ::makeFuseSolid(Fuse *pFuse, TopoDS_Solid &solidshape, std::string &logmsg)
{
    pFuse->makeShellsFromShapes();
    logmsg += "Processing fuse "+pFuse->name()+"\n";
    try
    {
        //make the solid
        BRepBuilderAPI_Sewing stitcher(1.e-4);
        for(TopTools_ListIteratorOfListOfShape shellIt(pFuse->shells()); shellIt.More(); shellIt.Next())
        {
            TopoDS_Shell shell = TopoDS::Shell(shellIt.Value());
            TopExp_Explorer shapeExplorer;
            for (shapeExplorer.Init(shell, TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
            {
                TopoDS_Shape aSub = shapeExplorer.Current();
                stitcher.Add(aSub);
            }
        }
        // stitch
        stitcher.Perform();
        logmsg += QString::asprintf("   Nb of free edges=%d\n",       stitcher.NbFreeEdges()).toStdString();
        logmsg += QString::asprintf("   Nb of contiguous edges=%d\n", stitcher.NbContigousEdges()).toStdString();


//    If all faces have been sewn correctly, the result is a shell. Otherwise, it is a compound.
//    After a successful sewing operation all faces have a coherent orientation.

        TopoDS_Shape sewedshape = stitcher.SewedShape();
        if(sewedshape.IsNull())
        {
            logmsg += "   Fuse stitched shape is NULL\n";
            return false;
        }

        TopoDS_Shell FuseShellShape = TopoDS::Shell(sewedshape);
        //make the solid
        if(!FuseShellShape.IsNull())
        {
            BRepBuilderAPI_MakeSolid solidMaker(FuseShellShape);
            if(!solidMaker.IsDone())
            {
                logmsg += "   Solid not made... \n";
                solidshape.Nullify();
                return false;
            }
            solidshape = TopoDS::Solid(solidMaker.Shape());

            logmsg += "   Fuse stitching result is " +shapeType(solidshape) + "\n";
            std::string strange;
            occ::listShapeContent(solidshape, strange, "   ");
            logmsg += strange;
            occ::checkShape(solidshape, logmsg, "   ");
        }
    }
    catch(Standard_TypeMismatch const &)
    {
        logmsg += "     Fuse::makeFuseSolid: Type mismatch error\n";
    }
    catch(...)
    {
        logmsg += "     Fuse::makeFuseSolid: Unknown error\n";
    }
    return true;
}


void occ::makeWingShape(WingXfl const *pWing, double stitchprecision, TopoDS_Shape &wingshape, std::string &logmsg)
{
    if(!pWing)
    {
        logmsg += "No wing to process\n";
        return;
    }
//    logmsg.clear();
    std::string strong = "Processing wing "+ pWing->name() + "\n";
    logmsg += strong;

    BRepBuilderAPI_Sewing stitcher(stitchprecision);

    for(int iSurf=0; iSurf<pWing->nSurfaces(); iSurf++)
    {
        Surface const &surf = pWing->surfaceAt(iSurf);

        TopoDS_Wire TopLeftWire, TopRightWire, BotLeftWire, BotRightWire;

        makeFoilWires(surf, TopLeftWire, BotLeftWire, TopRightWire, BotRightWire, logmsg);

        if(TopLeftWire.IsNull() || BotLeftWire.IsNull())
            continue;

        //LEFT TIP SURFACE
        if(surf.isTipLeft())
        {
            //LEFT TIP PATCH
            //assemble the LEFT top and bot wires to make a profile
            BRepBuilderAPI_MakeWire leftProfileMaker;
            leftProfileMaker.Add(TopLeftWire);
            leftProfileMaker.Add(BotLeftWire);
            if(!leftProfileMaker.IsDone())
            {
                logmsg += "   Error making leftProfile\n";
                return;
            }
            TopoDS_Wire LeftWire = leftProfileMaker.Wire();
            if(LeftWire.IsNull())
            {
                logmsg += "   Left Wire make failed\n";
                return;
            }
            BRepBuilderAPI_MakeFace FaceMaker(LeftWire);
            if(!FaceMaker.IsDone())
            {
                logmsg += "   Error making left tip patch\n";
            }
            else
            {
                TopoDS_Face theLeftTipPatch = FaceMaker.Face();
//                logmsg += "     TIP LEFT SHAPE IS type "+shapeType(theLeftTipPatch) +"\n");
//                FaceList.Append(theLeftTipPatch);
                try
                {
                    stitcher.Add(theLeftTipPatch);
                }
                catch(TopoDS_UnCompatibleShapes const &ex)
                {
                    logmsg += "   incompatible shapes" + std::string(ex.GetMessageString());
                }
                catch(TopoDS_FrozenShape const &ex)
                {
                    logmsg += "   frozen shapes" + std::string(ex.GetMessageString());
                }
                catch(...)
                {
                    logmsg += "   Some other error\n";
                }

//                logmsg += "     Added LEFT TIP PATCH\n");
            }
        }
        if(surf.isTipRight())
        {
            //RIGHT TIP PATCH
            //assemble the RIGHT top and bot wires to make a profile
            BRepBuilderAPI_MakeWire rightProfileMaker;
            rightProfileMaker.Add(TopRightWire);
            rightProfileMaker.Add(BotRightWire);
            if(!rightProfileMaker.IsDone())
            {
                logmsg += "   Error making Right Profile\n";
                return;
            }
            TopoDS_Wire RightWire = rightProfileMaker.Wire();
            if(RightWire.IsNull())
            {
                logmsg += "   Right Wire make failed\n";
                return;
            }
            BRepBuilderAPI_MakeFace FaceMaker(RightWire);
            if(!FaceMaker.IsDone())
            {
                logmsg += "   Error making right tip patch\n";
            }
            else
            {
                TopoDS_Face theRightTipPatch = FaceMaker.Face();
//                logmsg += "   TIP RIGHT SHAPE IS "+shapeType(theRightTipPatch) +"\n");
//                FaceList.Append(theRightTipPatch);
                stitcher.Add(theRightTipPatch);
//                logmsg += "   Added RIGHT TIP PATCH\n");
            }
        }

        // Sweep top surface
        // Initializes an algorithm for building a shell or a solid passing through a set of sections, where:
        // isSolid is set to true if the construction algorithm is required to build a solid or to false
        // if it is required to build a shell (the default value),
        // ruled is set to true if the faces generated between the edges of two consecutive wires are
        // ruled surfaces or to false (the default value) if they are smoothed out by approximation,
        // pres3d defines the precision criterion used by the approximation algorithm;
        // the default value is 1.0e-6.
        BRepOffsetAPI_ThruSections TopSweeper(false, true);
        TopSweeper.SetContinuity(GeomAbs_C0);
        TopSweeper.AddWire(TopLeftWire);
        TopSweeper.AddWire(TopRightWire);
        try
        {
            TopSweeper.Build();
        }
        catch(Standard_DomainError const &ex)
        {
            logmsg += "     catching "+ std::string(ex.GetMessageString())+"\n";
        }

        if(!TopSweeper.IsDone())
        {
            logmsg += "     error sweeping\n";
            return;
        }

        TopoDS_Shape TopShape = TopSweeper.Shape();
//        logmsg += "     Top surface has type " + shapeType(TopShape) + "\n");

        TopExp_Explorer shapeExplorer;
        for (shapeExplorer.Init(TopShape, TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
        {
            TopoDS_Shape aSub = shapeExplorer.Current();
            stitcher.Add(aSub);
        }

        BRepOffsetAPI_ThruSections BotSweeper(false, true);
        BotSweeper.SetContinuity(GeomAbs_C0);
        BotSweeper.AddWire(BotLeftWire);
        BotSweeper.AddWire(BotRightWire);
        try
        {
            BotSweeper.Build();
        }
        catch(Standard_DomainError const &ex)
        {
            logmsg += "     catching "+ std::string(ex.GetMessageString())+"\n";
        }

        if(!BotSweeper.IsDone())
        {
            logmsg += "     error sweeping\n";
            return;
        }

        TopoDS_Shape BotShape = BotSweeper.Shape();
//        logmsg += "     BOT surface has type " + shapeType(BotShape) + "\n");

        for (shapeExplorer.Init(BotShape, TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
        {
            TopoDS_Shape aSub = shapeExplorer.Current();
            stitcher.Add(aSub);
        }
    }

    // stitch
    stitcher.Perform();
    logmsg += QString::asprintf("   Nb of free edges=%d\n", stitcher.NbFreeEdges()).toStdString();
    logmsg += QString::asprintf("   Nb of contiguous edges=%d\n", stitcher.NbContigousEdges()).toStdString();

//    If all faces have been sewn correctly, the result is a shell. Otherwise, it is a compound.
//    After a successful sewing operation all faces have a coherent orientation.

    try
    {
        TopoDS_Shape sewedshape = stitcher.SewedShape();
        if(sewedshape.IsNull()) return;

        TopoDS_Shell WingShellShape = TopoDS::Shell(sewedshape);
        //make the solid
        if(!WingShellShape.IsNull())
        {
            BRepBuilderAPI_MakeSolid solidMaker(WingShellShape);
            if(!solidMaker.IsDone())
            {
                logmsg += "   Solid not made... \n";
                wingshape.Nullify();
                return;
            }
            wingshape = solidMaker.Shape();
//            wingshape = TopoDS::Shell(sewedshape);

            logmsg += "   Wing stitching result is " + occ::shapeType(wingshape) + "\n";

            std::string str;
            occ::checkShape(wingshape, str, "   ");
            logmsg += "   " + pWing->name() +": "+  str;
        }
    }
    catch(Standard_TypeMismatch const &)
    {
        logmsg += "     WingShapes:: Type mismatch error\n";
    }

}


bool occ::makeFoilWires(Surface const &aSurf,
                        TopoDS_Wire &TLWire, TopoDS_Wire & BLWire, TopoDS_Wire &TRWire, TopoDS_Wire &BRWire,
                        std::string &logmsg)
{
    int nPoints = int(aSurf.xDistribA().size());
    std::vector<Vector3d> PtA_T(nPoints),  PtA_B(nPoints), PtB_T(nPoints), PtB_B(nPoints);
    std::vector<Vector3d> NA(nPoints), NB(nPoints);

    aSurf.getSidePoints_2(xfl::TOPSURFACE, nullptr, PtA_T, PtB_T, NA, NB, aSurf.xDistribA(), aSurf.xDistribB());
    aSurf.getSidePoints_2(xfl::BOTSURFACE, nullptr, PtA_B, PtB_B, NA, NB, aSurf.xDistribA(), aSurf.xDistribB());

    // LEFT FOIL
    //TOP Wire
    BRepBuilderAPI_MakePolygon TLPolyMaker;
    for(uint i=0; i<PtA_T.size(); i++)
    {
        TLPolyMaker.Add(gp_Pnt(PtA_T[i].x, PtA_T[i].y, PtA_T[i].z));
    }
    TLWire = TLPolyMaker.Wire();
    if(!TLPolyMaker.IsDone() || TLWire.IsNull())
    {
        logmsg += "   error making topLeftWire\n";
        return false;
    }

    //BOT Wire
    BRepBuilderAPI_MakePolygon BLPolyMaker;
    for(uint i=0; i<PtA_B.size(); i++)
    {
        BLPolyMaker.Add(gp_Pnt(PtA_B[i].x, PtA_B[i].y, PtA_B[i].z));
    }
    BLWire = BLPolyMaker.Wire();
    if(!BLPolyMaker.IsDone() || BLWire.IsNull())
    {
        logmsg += "   error making botLeftwire\n";
        return false;
    }


    // RIGHT FOIL
    //TOP Wire
    BRepBuilderAPI_MakePolygon TRPolyMaker;
    for(uint i=0; i<PtB_T.size(); i++)
    {
        TRPolyMaker.Add(gp_Pnt(PtB_T[i].x, PtB_T[i].y, PtB_T[i].z));
    }

    TRWire = TRPolyMaker.Wire();
    if(!TRPolyMaker.IsDone() || TRWire.IsNull())
    {
        logmsg += "error making topRightWire\n";
        return false;
    }

    // BOT wire
    BRepBuilderAPI_MakePolygon BRPolyMaker;
    for(uint i=0; i<PtB_B.size(); i++)
    {
        BRPolyMaker.Add(gp_Pnt(PtB_B[i].x, PtB_B[i].y, PtB_B[i].z));
    }

    BRWire = BRPolyMaker.Wire();
    if(!BRPolyMaker.IsDone() || BRWire.IsNull())
    {
        logmsg += "error making botRightWire\n";
        return false;
    }

    return true;
}



bool occ::makeFoilMidWires(Surface const &aSurf,TopoDS_Wire & LeftWire, TopoDS_Wire &RightWire, std::string &logmsg)
{
    int nPoints = int(aSurf.xDistribA().size());
    std::vector<Vector3d>  PtA(nPoints), PtB(nPoints);
    std::vector<Vector3d> NA(nPoints), NB(nPoints);

    aSurf.getSidePoints_2(xfl::MIDSURFACE, nullptr, PtA, PtB, NA, NB, aSurf.xDistribA(), aSurf.xDistribB());

    // LEFT FOIL
    if(!makePolyLineWire(PtA, LeftWire, logmsg))
    {
        logmsg += "   error making left wire\n";
        return false;
    }

    // RIGHT FOIL
    if(!makePolyLineWire(PtB, RightWire, logmsg))
    {
        logmsg += "   error making right wire\n";
        return false;
    }
    return true;
}


bool occ::makePolyLineWire(std::vector<Vector3d> const &Pt, TopoDS_Wire &theWire, std::string &logmsg)
{
    BRepBuilderAPI_MakeWire PolyLineMaker;
    for(uint i=0; i<Pt.size()-1; i++)
    {
        Vector3d const P0 = Pt.at(i);
        Vector3d const P1 = Pt.at(i+1);
        if(!P0.isSame(P1))
        {
            TopoDS_Edge anEdge = BRepBuilderAPI_MakeEdge(gp_Pnt(P0.x, P0.y, P0.z),
                                                         gp_Pnt(P1.x, P1.y, P1.z));
            PolyLineMaker.Add(anEdge);
        }

    }
    theWire = PolyLineMaker.Wire();


    if(!PolyLineMaker.IsDone() || theWire.IsNull())
    {
        logmsg += "   error making polyline wire\n";
        return false;
    }
    return true;
}


bool occ::makePolyLineWire(std::vector<Node> const &nd, TopoDS_Wire &theWire, std::string &logmsg)
{
    BRepBuilderAPI_MakeWire PolyLineMaker;
    for(uint i=0; i<nd.size()-1; i++)
    {
        Vector3d const P0 = nd.at(i);
        Vector3d const P1 = nd.at(i+1);
        if(!P0.isSame(P1))
        {
            TopoDS_Edge anEdge = BRepBuilderAPI_MakeEdge(gp_Pnt(P0.x, P0.y, P0.z),
                                                         gp_Pnt(P1.x, P1.y, P1.z));
            PolyLineMaker.Add(anEdge);
        }

    }
    theWire = PolyLineMaker.Wire();


    if(!PolyLineMaker.IsDone() || theWire.IsNull())
    {
        logmsg += "   error making polyline wire\n";
        return false;
    }
    return true;
}


bool occ::makeSplineWire(BSpline3d const &spline, TopoDS_Wire &wire, std::string &logmsg)
{
    Handle(Geom_BSplineCurve) thespline;
//    if(!makeOCCSplineFromPoints(spline.controlPoints(), thespline, logmsg))
    if(!occ::makeOCCSplineFromBSpline3d(spline, thespline, logmsg))
    {
        return false;
    }
    try
    {
        BRepBuilderAPI_MakeEdge edgemaker(thespline);
        if(!edgemaker.IsDone())
            logmsg += "Error making left wire";
        else
        {
            BRepBuilderAPI_MakeWire wiremaker(edgemaker.Edge());
            if(!wiremaker.IsDone())
            {
                logmsg += "Error making wire";
                return false;
            }
            else
                wire = wiremaker.Wire();
        }
    }
    catch(StdFail_NotDone const &e)
    {
        logmsg += "BRepBuilderAPI_MakeEdge::StdFail_NotDone - "+std::string(e.GetMessageString());
        return false;
    }
    catch(...)
    {
        logmsg += "BRepBuilderAPI_MakeEdge - unknown error";
        return false;
    }
    return true;
}


void occ::makeSurfaceWires(WingXfl const *pWing, double scalefactor, TopoDS_ListOfShape &wires, std::string &logmsg)
{
    TopoDS_Wire TLWire, BLWire;
    std::vector<Vector3d> PtA_T, PtA_B, PtB_T, PtB_B;
    std::vector<Vector3d> NA, NB;

    // make leading and trailing edges
    BRepBuilderAPI_MakePolygon LETEPolyMaker;
    //LE
    for(int is=0; is<pWing->nSurfaces(); is++)
    {
        Surface const &aSurf = pWing->surfaceAt(is);
        Vector3d const &LA = aSurf.LA();
        Vector3d const &LB = aSurf.LB();
        Vector3d const &TA = aSurf.TA();
        Vector3d const &TB = aSurf.TB();

        if(is==0) LETEPolyMaker.Add(gp_Pnt(TA.x*scalefactor, TA.y*scalefactor, TA.z*scalefactor)); // close the wire

        LETEPolyMaker.Add(gp_Pnt(LA.x*scalefactor, LA.y*scalefactor, LA.z*scalefactor));

        if(is==pWing->nSections())
        {
            // close the wire
            LETEPolyMaker.Add(gp_Pnt(LB.x*scalefactor, LB.y*scalefactor, LB.z*scalefactor));
            LETEPolyMaker.Add(gp_Pnt(TB.x*scalefactor, TB.y*scalefactor, TB.z*scalefactor));
        }
    }

    //TE
    for(int is=pWing->nSurfaces()-1;is>=0; is--)
    {
        Surface const &aSurf = pWing->surfaceAt(is);
        Vector3d const &TA = aSurf.TA();
        LETEPolyMaker.Add(gp_Pnt(TA.x*scalefactor, TA.y*scalefactor, TA.z*scalefactor));
    }
    if(LETEPolyMaker.IsDone())  wires.Append(LETEPolyMaker.Wire());
    else
        std::cerr << "Error making contour wire"<<std::endl;

    for(int is=0; is<pWing->nSurfaces(); is++)
    {
        Surface const &aSurf = pWing->surfaceAt(is);

        int nPoints = int(aSurf.xDistribA().size());

        PtA_T.resize(nPoints);
        PtB_T.resize(nPoints);
        PtA_B.resize(nPoints);
        PtB_B.resize(nPoints);
        NA.resize(nPoints);
        NB.resize(nPoints);

        aSurf.getSidePoints_2(xfl::TOPSURFACE, nullptr, PtA_T, PtB_T, NA, NB, aSurf.xDistribA(), aSurf.xDistribB());
        aSurf.getSidePoints_2(xfl::BOTSURFACE, nullptr, PtA_B, PtB_B, NA, NB, aSurf.xDistribA(), aSurf.xDistribB());

        // LEFT FOIL
        //TOP Wire
        BRepBuilderAPI_MakePolygon TLPolyMaker;
        for(uint i=0; i<PtA_T.size(); i++)
        {
            TLPolyMaker.Add(gp_Pnt(PtA_T[i].x*scalefactor, PtA_T[i].y*scalefactor, PtA_T[i].z*scalefactor));
        }
        TLWire = TLPolyMaker.Wire();
        if(!TLPolyMaker.IsDone() || TLWire.IsNull())
        {
            logmsg += "   error making topLeftWire\n";
            return;
        }

        //BOT Wire
        BRepBuilderAPI_MakePolygon BLPolyMaker;
        for(uint i=0; i<PtA_B.size(); i++)
        {
            BLPolyMaker.Add(gp_Pnt(PtA_B[i].x*scalefactor, PtA_B[i].y*scalefactor, PtA_B[i].z*scalefactor));
        }
        BLWire = BLPolyMaker.Wire();
        if(!BLPolyMaker.IsDone() || BLWire.IsNull())
        {
            logmsg += "   error making botLeftwire\n";
            return;
        }
        wires.Append(TLWire);
        wires.Append(BLWire);

        if(is==pWing->nSections()-1)
        {
            //append the last right wire
            TopoDS_Wire TRWire, BRWire;
            //TOP Wire
            BRepBuilderAPI_MakePolygon TRPolyMaker;
            for(uint i=0; i<PtB_T.size(); i++)
            {
                TRPolyMaker.Add(gp_Pnt(PtB_T[i].x*scalefactor, PtB_T[i].y*scalefactor, PtB_T[i].z*scalefactor));
            }

            TRWire = TRPolyMaker.Wire();
            if(!TRPolyMaker.IsDone() || TRWire.IsNull())
            {
                logmsg += "error making topRightWire\n";
                return;
            }

            // BOT wire
            BRepBuilderAPI_MakePolygon BRPolyMaker;
            for(uint i=0; i<PtB_B.size(); i++)
            {
                BRPolyMaker.Add(gp_Pnt(PtB_B[i].x*scalefactor, PtB_B[i].y*scalefactor, PtB_B[i].z*scalefactor));
            }

            BRWire = BRPolyMaker.Wire();
            if(!BRPolyMaker.IsDone() || BRWire.IsNull())
            {
                logmsg += "error making botRightWire\n";
                return;
            }
            wires.Append(TRWire);
            wires.Append(BRWire);
        }
    }
}


void occ::makeFaceRuledTriangulation(TopoDS_Face const &face, std::vector<Vector3d> &pointlist, std::vector<Triangle3d> &trianglelist)
{
    if(face.IsNull()) return;

    pointlist.clear();
    trianglelist.clear();

    int nx = 11;
    int ny = 11;

    // make a Quad3d from the TopoDS_Face
    BRepAdaptor_Surface surfaceadaptor(face);
    GeomAdaptor_Surface aGAS = surfaceadaptor.Surface(); /** @todo no need */

    double umin = aGAS.FirstUParameter();
    double umax = aGAS.LastUParameter();
    double vmin = aGAS.FirstVParameter();
    double vmax = aGAS.LastVParameter();

    gp_Pnt pt00, pt10, pt11, pt01;

    Vector3d S00, S01, S10, S11;

    double du = (umax-umin)/double(nx-1);
    double dv = (vmax-vmin)/double(ny-1);

    Vector3d V0, V1, V2;

    // make the contour
    double u=0,v=0;
    for(int i=0; i<nx; i++)
    {
        u = umin+double(i-1)*du;
        pt00 = aGAS.Value(u,vmin);
        pointlist.push_back({pt00.X(), pt00.Y(), pt00.Z()});
    }
    for(int j=1; j<ny; j++)
    {
        v = vmin+double(j-1)*dv;
        pt00 = aGAS.Value(umax,v);
        pointlist.push_back({pt00.X(), pt00.Y(), pt00.Z()});
    }
    for(int i=nx-1; i>=0; i--)
    {
        u = umin+double(i-1)*du;
        pt00 = aGAS.Value(u,vmax);
        pointlist.push_back({pt00.X(), pt00.Y(), pt00.Z()});
    }
    for(int j=ny-1; j>=0; j--)
    {
        v = vmin+double(v-1)*dv;
        pt00 = aGAS.Value(umin,v);
        pointlist.push_back({pt00.X(), pt00.Y(), pt00.Z()});
    }

    // make the triangulation
    double u0=0;
    double v0=vmin;
    double u1=0, v1=0;
    for(int i=1; i<nx; i++)
    {
        u0 = umin+double(i-1)*du;
        u1 = umin+double(i)  *du;
        if(u0<=umin) u0=umin+PRECISION;
//        if(u1>=umax) v1=umax-PRECISION;
        pt00 = aGAS.Value(u0,v0);
        pt10 = aGAS.Value(u1,v0);

        for(int j=1; j<ny; j++)
        {
            v1 = vmin+double(j)  *dv;
            if(v1>=vmax) v1=vmax-PRECISION;
            pt01 = aGAS.Value(u0,v1);
            pt11 = aGAS.Value(u1,v1);

            V0.set(pt00.X(), pt00.Y(), pt00.Z());
            V1.set(pt10.X(), pt10.Y(), pt10.Z());
            V2.set(pt01.X(), pt01.Y(), pt01.Z());
            trianglelist.push_back({V0, V1, V2});

            V0.set(pt10.X(), pt10.Y(), pt10.Z());
            V1.set(pt11.X(), pt11.Y(), pt11.Z());
            V2.set(pt01.X(), pt01.Y(), pt01.Z());
            trianglelist.push_back({V0, V1, V2});

            pt00 = pt01;
            pt10 = pt11;
        }
    }
}


int occ::shapeTriangulationWithOcc(const TopoDS_Shape &shape, OccMeshParams const &params, std::vector<Triangle3d> &triangles)
{
    BRepTools::Clean(shape);
    if(params.isRelativeDeflection())
        BRepMesh_IncrementalMesh(shape, params.deflectionRelative(), params.isRelativeDeflection(),
                                 params.angularDeviation()*PI/180.0, true);
    else
        BRepMesh_IncrementalMesh(shape, params.deflectionAbsolute(), params.isRelativeDeflection(),
                                 params.angularDeviation()*PI/180.0, true);
    TopExp_Explorer shapeExplorer;

    int nFace = 0;
    for (shapeExplorer.Init(shape,TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
    {
        TopoDS_Face aFace = TopoDS::Face(shapeExplorer.Current());
        TopLoc_Location location;
        Handle(Poly_Triangulation) hTriangulation = BRep_Tool::Triangulation(aFace, location);

        if(hTriangulation.IsNull())
        {
        }
        else
        {
            for (int i=1; i<=hTriangulation->NbTriangles(); i++)
            {
                const Poly_Triangle& tri = hTriangulation->Triangle(i);
                const Vector3d p1(hTriangulation->Node(tri(1)).X(), hTriangulation->Node(tri(1)).Y(), hTriangulation->Node(tri(1)).Z());
                const Vector3d p2(hTriangulation->Node(tri(2)).X(), hTriangulation->Node(tri(2)).Y(), hTriangulation->Node(tri(2)).Z());
                const Vector3d p3(hTriangulation->Node(tri(3)).X(), hTriangulation->Node(tri(3)).Y(), hTriangulation->Node(tri(3)).Z());

                if(aFace.Orientation()==TopAbs_FORWARD) triangles.push_back({p1,p2,p3});
                else                                    triangles.push_back({p1,p3,p2});
            }
        }
        nFace++;
    }
    (void)nFace;
    return int(triangles.size());
}


int occ::shellTriangulationWithOcc(const TopoDS_Shell &shell, OccMeshParams const &params, std::vector<Triangle3d> &triangles)
{
    BRepTools::Clean(shell);
    if(params.isRelativeDeflection())
        BRepMesh_IncrementalMesh(shell, params.deflectionRelative(), params.isRelativeDeflection(),
                                 params.angularDeviation()*PI/180.0, true);
    else
        BRepMesh_IncrementalMesh(shell, params.deflectionAbsolute(), params.isRelativeDeflection(),
                                 params.angularDeviation()*PI/180.0, true);
    TopExp_Explorer shapeExplorer;

    int nFace = 0;
    for (shapeExplorer.Init(shell,TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
    {
        TopoDS_Face aFace = TopoDS::Face(shapeExplorer.Current());
        TopLoc_Location location;
        Handle(Poly_Triangulation) hTriangulation = BRep_Tool::Triangulation(aFace, location);

        if(hTriangulation.IsNull())
        {
        }
        else
        {
            for (int i=1; i<=hTriangulation->NbTriangles(); i++)
            {
                const Poly_Triangle& tri = hTriangulation->Triangle(i);
                const Vector3d p1 = Vector3d(hTriangulation->Node(tri(1)).X(), hTriangulation->Node(tri(1)).Y(), hTriangulation->Node(tri(1)).Z());
                const Vector3d p2 = Vector3d(hTriangulation->Node(tri(2)).X(), hTriangulation->Node(tri(2)).Y(), hTriangulation->Node(tri(2)).Z());
                const Vector3d p3 = Vector3d(hTriangulation->Node(tri(3)).X(), hTriangulation->Node(tri(3)).Y(), hTriangulation->Node(tri(3)).Z());

                if(aFace.Orientation()==TopAbs_FORWARD) triangles.push_back({p1,p2,p3});
                else                                    triangles.push_back({p1,p3,p2});
            }
        }
        nFace++;
    }
    (void)nFace;
    return int(triangles.size());
}


bool occ::importCADShapes(std::string const &filename, TopoDS_ListOfShape &shapes,
                          double &dimension, std::string &logmsg)
{
    if(!filename.length()) return false;

    dimension = 1.0;

    std::filesystem::path filePath(filename);
    if (filePath.extension() == ".brep") // Heed the dot.
    {
        return occ::importBRep(filename, shapes, dimension, logmsg);
    }
    else if (filePath.extension() == ".step" || filePath.extension() == ".stp")
    {
        return occ::importSTEP(filename, shapes, dimension, logmsg);
    }

    return false;
}


bool occ::importBRep(std::string const &filename, TopoDS_ListOfShape &shapes, double &dimension, std::string &logmsg)
{
    dimension = 1.0;
    BRep_Builder aBuilder;
    TopoDS_Shape brep;
    Standard_Boolean aResult = BRepTools::Read(brep, filename.c_str(), aBuilder);
    std::string msg;
    if (aResult)
    {
        shapes.Append(brep);
        occ::listShapeContent(brep, msg);
        logmsg+=msg;
        return true;
    }
    return false;
}


bool occ::importSTEP(const std::string &filename, TopoDS_ListOfShape &shapes, double &dimension, std::string &logmsg)
{
    std::string msg;
    QString logg;
    QString strange;

    logg.clear();
//    logmsg += "   Assuming file length unit is mm - scale the model and re-tessellate if necessary\n";

//    TCollection_AsciiString  aFilePath = std::string::fromStdString(filename).toUtf8().data();
    TCollection_AsciiString  aFilePath = filename.c_str();
    STEPControl_Reader aReader;
    Interface_Static::SetCVal("xstep.cascade.unit", "M"); // because flow5 works in metres and OpenCascade in millimetres
//qDebug()    <<"importSTEP xstep.cascade.unit"<<Interface_Static::IVal("xstep.cascade.unit");

    IFSelect_ReturnStatus status = aReader.ReadFile(aFilePath.ToCString());
    TColStd_SequenceOfAsciiString theUnitLengthNames;
    TColStd_SequenceOfAsciiString theUnitAngleNames;
    TColStd_SequenceOfAsciiString theUnitSolidAngleNames;

    if (status == IFSelect_RetDone)
    {
        aReader.FileUnits(theUnitLengthNames, theUnitAngleNames, theUnitSolidAngleNames);

        NCollection_Sequence<TCollection_AsciiString>::Iterator iter(theUnitLengthNames);
/*        for (; iter.More(); iter.Next())
        {
            qDebug() << "TheStepUnits: " << iter.Value().ToCString();
        }*/

        Handle(StepData_StepModel)  stepmodel = aReader.StepModel();
        if(stepmodel.IsNull())
        {
            logg += "   Unknown error importing STEP model\n";
            return false;
        }

/*        Interface_EntityIterator entIt = stepmodel->Entities();
        for ( ; entIt.More(); entIt.Next() )
        {
            const Handle(Standard_Transient)& ent     = entIt.Value();
//            const Handle(Standard_Type)&      entType = ent->DynamicType();

            if ( ent->IsKind( STANDARD_TYPE(StepBasic_LengthMeasureWithUnit) ) )
            {
                Handle(StepBasic_LengthMeasureWithUnit)  cpEnt = Handle(StepBasic_LengthMeasureWithUnit)::DownCast(ent);
                double unit = cpEnt->ValueComponent();
                strange = std::string::asprintf("   Length unit conversion factor to meter = %f\n", unit);
                logmsg += strange;
            }
        }*/
        //Interface_TraceFile::SetDefault();
        //        bool failsonly = false;
        //        aReader.PrintCheckLoad(failsonly, IFSelect_ItemsByEntity);

        int nbr = aReader.NbRootsForTransfer();
        QString strong;

        for (Standard_Integer n=1; n<=nbr; n++)
        {
            try
            {
                strong = QString::asprintf("   Transferring  root %d/%d\n", n, nbr);
                logg += strong;

                aReader.ClearShapes();
                bool bOk = aReader.TransferRoot(n);
                int nbs = aReader.NbShapes();
                strong = QString::asprintf("      Loading %d shape(s)\n", nbs);
                logg += strong;


                IFSelect_PrintCount mode = IFSelect_ItemsByEntity;

                aReader.PrintCheckLoad(false, mode);
                if (bOk && nbs>0)
                {
                    for (int i=1; i<=nbs; i++)
                    {
                        TopoDS_Shape aShape = aReader.Shape(i);
                        shapes.Append(aShape);
                        listShapeContent(aShape, msg, "      ");
                        logg += QString::fromStdString(msg);
                    }
                }
                logg += "\n";
            }
            catch(...)
            {
                logg += "   Unknown error importing STEP model\n";
                return false;
            }
        }

        strange = QString::asprintf("   Imported %d shape(s)\n", shapes.Extent());
        logg += strange;

        // get some kind of reference dimension
        dimension=0.0;
        BRepAdaptor_Surface adaptor;
        for(TopoDS_ListIteratorOfListOfShape bodyIt(shapes); bodyIt.More(); bodyIt.Next())
        {
            TopExp_Explorer shapeExplorer;
            int iFace=0;
            for (shapeExplorer.Init(bodyIt.Value(), TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
            {
                double dx=0, dy=0, dz=0, d0=0, d1=0;
                gp_Pnt P00,P01, P10, P11;
                TopoDS_Face face = TopoDS::Face(shapeExplorer.Current());
                adaptor.Initialize(face);
                try
                {
                    double umin = adaptor.FirstUParameter();
                    double umax = adaptor.LastUParameter();
                    double vmin = adaptor.FirstVParameter();
                    double vmax = adaptor.LastVParameter();
                    adaptor.D0(umin, vmin, P00);
                    adaptor.D0(umin, vmax, P01);
                    adaptor.D0(umax, vmax, P11);
                    adaptor.D0(umax, vmin, P10);
                }
                catch(Geom_UndefinedValue const &ex)
                {

                    strange  = "Exception raised when calculating object length: " + QString::fromStdString(ex.GetMessageString());
                    strange += "\n";
                    strange += "Aborting\n";
                    logg += strange;

                    return false;
                }

                dx = P00.X()-P11.X();   dy = P00.Y()-P11.Y();   dz = P00.Z()-P11.Z();
                d0 = sqrt(dx*dx+dy*dy+dz*dz);
                dimension = std::max(d0, dimension);

                dx = P01.X()-P10.X();   dy = P01.Y()-P10.Y();   dz = P01.Z()-P10.Z();
                d1 = sqrt(dx*dx+dy*dy+dz*dz);
                dimension = std::max(d1, dimension);

                iFace++;
            }
            (void)iFace;
        }

        strong = QString::asprintf("   Reference length to display the model= %7.2f meters\n\n", dimension);
        logg += strong;
/*        Handle(ShapeFix_Shell) SFS = new ShapeFix_Shell();
        for(TopoDS_ListIteratorOfListOfShape bodyIt(occbody.m_Shell); bodyIt.More(); bodyIt.Next())
        {
            TopoDS_Shell aShell = TopoDS::Shell(bodyIt.Value());
            SFS->FixFaceOrientation(aShell);
        }*/
    }
    else
    {
        strange = "Error importing STEP file\n";
        logg += strange;

        return false;
    }

    strange = "Importing STEP file...  DONE\n\n";
    logg += strange;

    logmsg = logg.toStdString();
    return true;
}


bool occ::intersectFace(TopoDS_Face const &aFace, Segment3d const &seg, Vector3d &I)
{
    IntCurvesFace_ShapeIntersector intersector;
    intersector.Load(aFace, 1.e-6);
    gp_Pnt A(seg.vertexAt(0).x, seg.vertexAt(0).y, seg.vertexAt(0).z);
    gp_Dir U(seg.unitDir().x, seg.unitDir().y, seg.unitDir().z);
    gp_Lin line(A, U);
    intersector.Perform(line, -RealLast(),RealLast());
    if(intersector.IsDone() && intersector.NbPnt()>0)
    {
        gp_Pnt Icc = intersector.Pnt(1); // OCC uses 1-based arrays
        I.set(Icc.X(), Icc.Y(), Icc.Z());
        return true;
    }
    else
        return false;
}


/*
bool occ::intersectShape(TopoDS_Shape const &aShape, Segment3d const &seg, Vector3d &I, bool bRightSide)
{
    GeomAPI_IntCS intersector;
    gp_Pnt A(seg.vertexAt(0).x, seg.vertexAt(0).y, seg.vertexAt(0).z);
    gp_Dir U(seg.unitDir().x, seg.unitDir().y, seg.unitDir().z);
    gp_Lin gpline(A, U);
    Handle(Geom_Line) line = new Geom_Line(gpline);
    TopExp_Explorer shapeexplorer;
    for(shapeexplorer.Init(aShape,TopAbs_FACE); shapeexplorer.More(); shapeexplorer.Next())
    {
        TopoDS_Face const &aFace = TopoDS::Face(shapeexplorer.Current());
        BRepAdaptor_Surface surfaceadaptor(aFace);
        Handle(Geom_Surface) aGAS = surfaceadaptor.Surface().Surface();
        intersector.Perform(line, aGAS);
        if(intersector.IsDone())
        {
            if(intersector.NbPoints()>0)
            {
                gp_Pnt Icc = intersector.Point(1); // OCC uses 1-based arrays
                I.set(Icc.X(), Icc.Y(), Icc.Z());
                return true;
            }
        }
    }
    return false;
}*/


bool occ::intersectShape(TopoDS_Shape const &aShape, Segment3d const &seg, Vector3d &I, bool bRightSide)
{
    //shift seg.y to avoid occ::falling on the edge of the fuse side face in the case of the fin
    //occ intersector fails in this case
    Segment3d shiftedy(seg);
    bool bShifted = false;
    double shift = 5.e-5; // since default internal precision of OCC is 1µm (?)
    if(!bRightSide) shift = -shift;
    if(fabs(shiftedy.vertexAt(0).y)<1.e-6)
    {
        shiftedy.vertex(0).y = shift;
        bShifted = true;
    }
    if(fabs(shiftedy.vertexAt(1).y)<1.e-6)
    {
        shiftedy.vertex(1).y = shift;
        bShifted = true;
    }

    gp_Pnt A(shiftedy.vertexAt(0).x, shiftedy.vertexAt(0).y, shiftedy.vertexAt(0).z);
    gp_Dir U(shiftedy.unitDir().x,   shiftedy.unitDir().y,   shiftedy.unitDir().z);
    gp_Lin gpline(A, U);

    TopExp_Explorer shapeexplorer;

    bool bIntersect = false;

    double dmax = LARGEVALUE;
    for(shapeexplorer.Init(aShape, TopAbs_FACE); shapeexplorer.More(); shapeexplorer.Next())
    {
        TopoDS_Face const &aFace = TopoDS::Face(shapeexplorer.Current());
//        IntCurvesFace_Intersector intersector(aFace, Precision::Confusion());
        IntCurvesFace_Intersector intersector(aFace, Precision::Confusion(), false);

        intersector.Perform(gpline, -100, 100);
        if(intersector.IsDone())
        {
            if(intersector.NbPnt()>0)
            {
                gp_Pnt Icc = intersector.Pnt(1); // OCC uses 1-based arrays
                // shift back
                double y = Icc.Y();

                if(bShifted) y -= shift;

                double d0 = seg.vertexAt(0).distanceTo(Icc.X(), Icc.Y(), Icc.Z());
                double d1 = seg.length();
                if(d0<d1 && d0<dmax)
                {
                    dmax=d0;
                    I.set(Icc.X(), y, Icc.Z());
                    bIntersect = true;
                }
            }
        }
    }
    return bIntersect;
}


void occ::intersectShape(TopoDS_Shape const &aShape, std::vector<Segment3d> const &segs, std::vector<Vector3d> &I, std::vector<bool> &bIntersect)
{
    IntCurvesFace_ShapeIntersector intersector;
    I.resize(segs.size());
    bIntersect.resize(segs.size());
//    Geom_Line line(gpline);
    TopExp_Explorer shapeexplorer;
    for(shapeexplorer.Init(aShape,TopAbs_FACE); shapeexplorer.More(); shapeexplorer.Next())
    {
        TopoDS_Face const &aFace = TopoDS::Face(shapeexplorer.Current());
        intersector.Load(aFace, 1.e-6);

        for(uint isg=0; isg<segs.size(); isg++)
        {
            Segment3d const & seg = segs.at(isg);
            gp_Pnt A(seg.vertexAt(0).x, seg.vertexAt(0).y, seg.vertexAt(0).z);
            gp_Dir U(seg.unitDir().x, seg.unitDir().y, seg.unitDir().z);
            gp_Lin gpline(A, U);
            intersector.Perform(gpline, -RealLast(), RealLast());
            if(intersector.IsDone() && intersector.NbPnt()>0)
            {
                gp_Pnt Icc = intersector.Pnt(1); // OCC uses 1-based arrays
                I[isg].set(Icc.X(), Icc.Y(), Icc.Z());
                bIntersect[isg] = true;
            }
            else bIntersect[isg] = false;
        }
    }
}


/**
 * Builds an ideal triangle on the surface, with first edge seg
 */
bool occ::makeEquiTriangle(TopoDS_Face const &aFace, Segment3d const &baseseg, double maxedgelength, double growthfactor, Triangle3d &triangle)
{
    // make the segment's average normal
    // could also request the face normal at the mid-point, but may take longer
    Vector3d N = baseseg.vertexAt(0).normal() + baseseg.vertexAt(1).normal();
    N.normalize();
    Vector3d Nt = N*baseseg.unitDir();

    //project V on the surface
    double umin=0, umax=0, vmin=0, vmax=0;
    BRepTools::UVBounds(aFace, umin, umax, vmin, vmax);

    // remove the boundaries;  in case the projection falls outside,
    // this prevents OCC from returning an invalid solution with u=umin or umax
/*    umin += 1.e-6;    umax -=1.e-6; */

    Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace);
    GeomAPI_ProjectPointOnSurf projector;

    double height = std::min(baseseg.length()*sin(PI/3), maxedgelength) * growthfactor;
    int iter = 0;
    int npts = 0;
    gp_Pnt nearest;
    do
    {
        iter++;
        Vector3d V3 = baseseg.midPoint() + Nt*height;
        gp_Pnt P3(V3.x, V3.y, V3.z);

        try {
//            projector.Init(P3, aSurface, umin, umax, vmin, vmax, Extrema_ExtAlgo::Extrema_ExtAlgo_Grad);
            projector.Init(P3, aSurface);

            if(!projector.IsDone())
            {
                // Something went wrong
                // Reduce the triangle's height and try again
                height *= 0.9;
                continue;
            }
            if(projector.NbPoints()<1)
            {
                // Something went wrong
                // Reduce the triangle's height and try again
                height *= 0.9;
                continue;
            }
            npts = projector.NbPoints();
            nearest = projector.NearestPoint();
/*            double drel = P3.Distance(nearest)/maxedgelength;
            if(drel>0.1)
            {
                // large curvature, reduce the height and try again
                height *= 0.9;
                continue;
            }*/
        }
        catch (Standard_OutOfRange const &)
        {
            qDebug("Standard_OutOfRange");
            break;
        }
        catch (StdFail_NotDone const &)
        {
            qDebug("StdFail_NotDone");
            break;
        }

        // try all the projector's results one by one until we find the nearest
        for(int ip=1; ip<=npts; ip++)
        {
            double u=0,v=0;
            projector.Parameters(ip,u,v);
            gp_Pnt P = projector.Point(ip);
            // if the projected point is outside the surface, it seems that
            // the projector returns u=umin, so discard these cases
            if(u>umin+1.e-6 && u<umax-1.e-6)
            {
                // make sure this is the nearest projection point in case of
                //  a convex surface
                if(nearest.Distance(P)<1.e-6)
                {
                    GeomLProp_SLProps props(aSurface, u, v, 1, 1.e-5); // max 1 derivation; resolution ?
                    if(props.IsNormalDefined())
                    {
                        gp_Pnt Pt = props.Value(); // is same as P

                        gp_Dir Dir = props.Normal();

                        Node nd(Pt.X(), Pt.Y(), Pt.Z());
                        nd.setNormal(Dir.X(), Dir.Y(), Dir.Z());
                        triangle = {baseseg.vertexAt(0), baseseg.vertexAt(1), nd}; // make it the same orientation as the PSLG

                        return true; // Success
                    }
                }
            }
        }

        // The projection falls outside the surface.
        // This occurs for instance at the nose and the
        // spline edges when a large max length is specified.
        // Reduce the triangle's height and try again
        height *= 0.9;

    }
    while(iter<30);

    //    for(int ip=1; ip<=projector.NbPoints(); ip++)
    //    {
    //        projector.Parameters(ip,u,v);

    //        GeomLProp_SLProps props(aSurface, u, v, 1, 1.e-5); // max 1 derivation; resolution ?
    //        gp_Pnt Pt = props.Value(); // is same as P
    //        // GeomLProp_SLProps returns a first NULL point at the spline's nose
    //        if(props.IsNormalDefined() && sqrt(Pt.X()*Pt.X()+Pt.Y()*Pt.Y()+Pt.Z()*Pt.Z())>1.e-4)
    //        {
    //            gp_Dir Dir = props.Normal();
    //            Node nd(Pt.X(), Pt.Y(), Pt.Z());
    //            nd.setNormal(Dir.X(), Dir.Y(), Dir.Z());
    //            triangle = {seg.vertexAt(0), seg.vertexAt(1), nd}; // make it the same orientation as the PSLG
    //            break;
    //        }
    //    }
    return false;
}


void occ::makeFaceFromNodeStrip(std::vector<Node> const &pts, TopoDS_Face &face, std::string &log)
{
    std::string strong;
    try
    {
        BRepBuilderAPI_MakeWire FaceWireMaker1;
        for(uint i=0; i<pts.size(); i++)
        {
            int i1 = (i+1)%pts.size();
            if(!pts.at(i).isSame(pts.at(i1)))
            {
                int i0 = i;
                int i1 = (i+1)%pts.size();
                TopoDS_Edge Edge12 = BRepBuilderAPI_MakeEdge(gp_Pnt(pts.at(i0).x, pts.at(i0).y, pts.at(i0).z), gp_Pnt(pts.at(i1).x, pts.at(i1).y, pts.at(i1).z));
                FaceWireMaker1.Add(Edge12);
            }
        }
        if(!FaceWireMaker1.IsDone())
        {
            switch(FaceWireMaker1.Error())
            {
                case BRepBuilderAPI_WireDone:
                    log += "   Trace::BRepBuilderAPI_WireDone\n"; break;
                case BRepBuilderAPI_EmptyWire:
                    log += "   Trace::BRepBuilderAPI_EmptyWire\n"; break;
                case BRepBuilderAPI_DisconnectedWire:
                    log += "   Trace::BRepBuilderAPI_DisconnectedWire\n"; break;
                case BRepBuilderAPI_NonManifoldWire:
                    log += "   Trace::BRepBuilderAPI_NonManifoldWire\n"; break;
            }
            log+= strong;
        }
        else
        {
            TopoDS_Wire FaceWire = FaceWireMaker1.Wire();
            BRepBuilderAPI_MakeFace FaceMaker(FaceWire);
            if(!FaceMaker.IsDone())
            {
                switch(FaceMaker.Error())
                {
                    case BRepBuilderAPI_FaceDone:
                        log += "   Trace::BRepBuilderAPI_FaceDone\n"; break;
                    case BRepBuilderAPI_NoFace:
                        log += "   Trace::BRepBuilderAPI_NoFace\n"; break;
                    case BRepBuilderAPI_NotPlanar:
                        log += "   Trace::BRepBuilderAPI_NotPlanar\n"; break;
                    case BRepBuilderAPI_CurveProjectionFailed:
                        log += "   Trace::BRepBuilderAPI_CurveProjectionFailed\n"; break;
                    case BRepBuilderAPI_ParametersOutOfRange:
                        log += "   Trace::BRepBuilderAPI_ParametersOutOfRange\n"; break;
                }
                log+= strong;
            }
            else
            {
                face = FaceMaker.Face();
            }
        }
    }
    catch(StdFail_NotDone const &ex)
    {
        std::string strong =  "   Error making body face\n" + std::string(ex.GetMessageString()) +"\n";
        log+= strong;
        return;
    }
}


bool occ::makeXflNurbsfromOccNurbs(Handle(Geom_BSplineSurface) occnurbs, NURBSSurface &xflnurbs)
{
    if(occnurbs.IsNull()) return false;

    xflnurbs.setuDegree(occnurbs->UDegree());
    xflnurbs.setvDegree(occnurbs->VDegree());
    xflnurbs.setFrameCount(occnurbs->NbUPoles());
    xflnurbs.setFramePointCount(occnurbs->NbVPoles());
    for(int ifr=0; ifr<xflnurbs.frameCount(); ifr++)
    {
        Frame &fr = xflnurbs.frame(ifr);
        for(int ic=0; ic<xflnurbs.framePointCount(); ic++)
        {
            gp_Pnt pt(occnurbs->Pole(ifr+1, ic+1));
            fr.setCtrlPoint(ic, pt.X(), pt.Y(), pt.Z());
            fr.setXPosition(pt.X());
        }
    }

/*    for(int ifr=0; ifr<xflnurbs.frameCount(); ifr++)
    {
        Frame &fr = xflnurbs.frame(ifr);
        for(int ic=0; ic<xflnurbs.framePointCount(); ic++)
        {
            fr.ctrlPointAt(ic).listCoords(std::string::asprintf(" frame %2d   pt %2d", ifr, ic));
        }
    }*/

    xflnurbs.setKnots();
    return true;
}


bool occ::makeOCCSplineFromPoints(std::vector<Vector3d> const &pointlist, Handle(Geom_BSplineCurve)& theSpline, std::string &logmsg)
{
    TColgp_Array1OfPnt pts(0, int(pointlist.size())-1);
    for(uint i=0; i<pointlist.size(); i++)
    {
        pts.SetValue(i, gp_Pnt(pointlist.at(i).x, pointlist.at(i).y, pointlist.at(i).z));
    }

    GeomAPI_PointsToBSpline splineMaker(pts, 3, 8, GeomAbs_C2, 1.0e-3);
    try
    {
        theSpline = splineMaker.Curve();
/*        qDebug()<<"theSpline"<<theSpline->Degree()<<theSpline->Continuity()<<theSpline->IsRational();
        qDebug()<<"   knots"<<theSpline->Knots().Lower()<<theSpline->Knots().Upper();
        for(int i=theSpline->Knots().Lower(); i<=theSpline->Knots().Upper(); i++)
            qDebug()<<"       "<<theSpline->Knot(i)<<theSpline->Multiplicity(i);
        qDebug()<<"_____";*/
    }
    catch(StdFail_NotDone &)
    {
        logmsg += "   failed to make OCC spline make points\n";
        return false;
    }
    return true;
}


bool occ::makeOCCSplineFromBSpline3d_0(BSpline3d const &b3d, Handle(Geom_BSplineCurve)& hspline, std::string &logmsg)
{
    TColgp_Array1OfPnt poles(0, b3d.nCtrlPoints()-1);
    TColStd_Array1OfReal weights(0, b3d.nCtrlPoints()-1);
    for(int i=0; i< b3d.nCtrlPoints(); i++)
    {
        Vector3d  const &pt = b3d.controlPoint(i);
        poles.SetValue(i, gp_Pnt(pt.x, pt.y, pt.z));
        weights.SetValue(i,1.0);
    }

    //------Make the knots-----
    //------OCC requires that the knots are strictly crescending, so define a minimal increment.
    double eps = 1.e-3;

    int p = b3d.degree();
    int uSize = int(b3d.knots().size())-2*p+2;
    TColStd_Array1OfReal    knots(0, uSize-1);
    TColStd_Array1OfInteger mults(0, uSize-1);
    knots.SetValue(0, 0.0);
    mults.SetValue(0, p);
    knots.SetValue(uSize-1, 1.0);
    mults.SetValue(uSize-1, p);
    for(uint iu=1; iu<b3d.knots().size()-2*p+1; iu++)
    {
        double knot = b3d.knots().at(p+iu-1);
        // occ requires that the knot values are strictly increasing
        if(fabs(knot)<PRECISION)     knot=eps;
        if(fabs(1.0-knot)<PRECISION) knot=1.0-eps;
        knots.SetValue(iu, knot);
        mults.SetValue(iu, 1);
    }
    try
    {
        hspline = new Geom_BSplineCurve(poles, weights, knots, mults, p);
//        qDebug()<<"theSpline"<<hspline->Degree()<<hspline->Continuity()<<hspline->IsRational();
//        qDebug()<<"   knots"<<hspline->Knots().Lower()<<hspline->Knots().Upper();
//        for(int i=hspline->Knots().Lower(); i<=hspline->Knots().Upper(); i++)
//            qDebug()<<"       "<<hspline->Knot(i)<<hspline->Multiplicity(i);
//        qDebug()<<"_____";
    }
    catch (Standard_ConstructionError const &e)
    {
        logmsg += "   Spline construction error... " + std::string(e.GetMessageString()) + "\n";
        return false;
    }
    catch(Standard_Failure const &s)
    {
        logmsg += "   Standard failure... " + std::string(s.GetMessageString())+"\n";
        return false;
    }
    catch (...)
    {
        logmsg += "   Unknown failure\n";
        return false;
    }
    return true;
}


bool occ::makeOCCSplineFromBSpline3d(BSpline3d const &b3d, Handle(Geom_BSplineCurve)& hspline, std::string &logmsg)
{
    TColgp_Array1OfPnt poles(0, b3d.nCtrlPoints()-1);
    TColStd_Array1OfReal    weights(0, b3d.nCtrlPoints()-1);
    for(int i=0; i<b3d.nCtrlPoints(); i++)
    {
        Vector3d  const &pt = b3d.controlPoint(i);
        poles.SetValue(i, gp_Pnt(pt.x, pt.y, pt.z));
        weights.SetValue(i,1.0);
    }

    //------Make the knots-----
    int p = b3d.degree()+1;
    int uSize = int(b3d.knots().size())-2*p+1;
    TColStd_Array1OfReal    knots(0, uSize);
    TColStd_Array1OfInteger mults(0, uSize);
    knots.SetValue(0, 0.0);
    mults.SetValue(0, p);
    knots.SetValue(uSize, 1.0);
    mults.SetValue(uSize, p);
    for(int iu=1; iu<uSize; iu++)
    {
        double knot = b3d.knots().at(p+iu-1);
        knots.SetValue(iu, knot);
        mults.SetValue(iu, 1);
    }

    try
    {
        hspline = new Geom_BSplineCurve(poles, weights, knots, mults, b3d.degree());
/*        qDebug()<<"theSpline"<<hspline->Degree()<<hspline->Continuity()<<hspline->IsRational();
        qDebug()<<"   knots"<<hspline->Knots().Lower()<<hspline->Knots().Upper();
        for(int i=hspline->Knots().Lower(); i<=hspline->Knots().Upper(); i++)
            qDebug()<<"       "<<hspline->Knot(i)<<hspline->Multiplicity(i);
        qDebug()<<"_____";*/
    }
    catch (Standard_ConstructionError const &e)
    {
        logmsg += "   Spline construction error... " + std::string(e.GetMessageString()) + "\n";
        return false;
    }
    catch(Standard_Failure const &s)
    {
        logmsg += "   Standard failure... " + std::string(s.GetMessageString())+"\n";
        return false;
    }
    catch (...)
    {
        logmsg += "   Unknown failure\n";
        return false;
    }
    return true;
}


void occ::makeOCCNURBSFromNurbs(NURBSSurface const &nurbs, bool bXZSymmetric, Handle(Geom_BSplineSurface)& hnurbs, std::string &logmsg)
{
    logmsg.clear();
    std::string strong;

    int nu = nurbs.frameCount();
    int nv = nurbs.framePointCount();
    if(nu<=0) return;

    TColgp_Array2OfPnt Poles(0, nu-1, 0, nv-1);
    TColgp_Array2OfPnt LeftPoles( 0, nu-1, 0, nv-1);

    //------Store the control points in OCC format-----
    for(int iFrame=0; iFrame<nu; iFrame++)
    {
        Frame const &fr = nurbs.frameAt(iFrame);
        for(int j=0; j<fr.nCtrlPoints(); j++)
        {

            if(!bXZSymmetric)
            {
                Vector3d const &ptR = fr.pointAt(j);
                Poles.SetValue(iFrame, j, gp_Pnt(ptR.x, ptR.y, ptR.z));
            }
            else
            {
                Vector3d ptL = fr.pointAt(fr.nCtrlPoints()-j-1);
                Poles.SetValue(iFrame, j, gp_Pnt(ptL.x, -ptL.y, ptL.z));
            }
        }
    }

    //------OCC requires that the knots are strictly crescending, so define a minimal increment.
    double eps = 1.e-5;

    //------Make the knots-----
    int p = nurbs.uDegree();
    int uSize = int(nurbs.uKnot().size())-2*p+2;
    TColStd_Array1OfReal    uKnots(0, uSize-1);
    TColStd_Array1OfInteger uMults(0, uSize-1);
    uKnots.SetValue(0, 0.0);
    uMults.SetValue(0, p);
    uKnots.SetValue(uSize-1, 1.0);
    uMults.SetValue(uSize-1, p);
    for(uint iu=1; iu<nurbs.uKnot().size()-2*p+1; iu++)
    {
        double knot = nurbs.uKnot().at(p+iu-1);
        // occ requires that the knot values are strictly increasing
        if(fabs(knot)<PRECISION)     knot=eps;
        if(fabs(1.0-knot)<PRECISION) knot=1.0-eps;
        uKnots.SetValue(iu, knot);
        uMults.SetValue(iu, 1);
    }

    p = nurbs.vDegree();
    int vSize = int(nurbs.vKnot().size())-2*p+2;
    TColStd_Array1OfReal    vKnots(0, vSize-1);
    TColStd_Array1OfInteger vMults(0, vSize-1);
    vKnots.SetValue(0, 0.0);
    vMults.SetValue(0, p);
    vKnots.SetValue(vSize-1, 1.0);
    vMults.SetValue(vSize-1, p);
    for(uint iv=1; iv<nurbs.vKnot().size()-2*p+1; iv++)
    {
        double knot = nurbs.vKnot().at(p+iv-1);
        // occ requires that the knot values are strictly increasing
        if(fabs(knot)<PRECISION)     knot=eps;
        if(fabs(1.0-knot)<PRECISION) knot=1.0-eps;
        vKnots.SetValue(iv, knot);
        vMults.SetValue(iv, 1);
    }

    //------Build the right NURBS surface-----
    Handle(Geom_BSplineSurface) HRightSurface, HLeftSurface;
    try{
        hnurbs = new Geom_BSplineSurface(Poles, uKnots, vKnots, uMults, vMults,
                                         nurbs.uDegree(), nurbs.vDegree());
    }
    catch (Standard_ConstructionError const &e)
    {
        strong = "   Spline construction error... "+std::string(e.GetMessageString()) + "\n";
        logmsg += strong;
        return;
    }
    catch(Standard_Failure const &s)
    {
        strong = "   Standard failure... "+std::string(s.GetMessageString())+"\n";
        logmsg += strong;
        return;
    }
    catch (...)
    {
        logmsg += "   Unknown failure\n";
        return;
    }
}


void occ::flipShapesXZ(TopoDS_ListOfShape &shapes)
{
    gp_Trsf mirror;
    mirror.SetMirror(gp_Ax2(gp_Pnt(0,0,0), gp_Dir(0,1,0)));
    BRepBuilderAPI_Transform themirror(mirror);
    for(TopTools_ListIteratorOfListOfShape shapeit(shapes); shapeit.More(); shapeit.Next())
    {
        themirror.Perform(shapeit.Value(), Standard_True);
        shapeit.Value() = themirror.Shape();
    }
}


void occ::scaleShapes(TopoDS_ListOfShape &shapes, double scalefactor)
{
    if(fabs(scalefactor)<PRECISION) return;
    if(fabs(scalefactor-1.0)<PRECISION) return;

    gp_Trsf Scale;
    Scale.SetScale(gp_Pnt(0.0,0.0,0.0), scalefactor);
    BRepBuilderAPI_Transform thescaler(Scale);

    for(TopTools_ListIteratorOfListOfShape shapeit(shapes); shapeit.More(); shapeit.Next())
    {
        thescaler.Perform(shapeit.Value(), Standard_True);
        shapeit.Value() = thescaler.Shape();
    }
}


void occ::scaleShapes(TopoDS_ListOfShape &shapes, double xfactor, double yfactor, double zfactor)
{
    gp_GTrsf aTrsf;
    gp_Mat rot(xfactor, 0, 0, 0, yfactor, 0, 0, 0, zfactor);
    aTrsf.SetVectorialPart(rot);
    for(TopTools_ListIteratorOfListOfShape shapeit(shapes); shapeit.More(); shapeit.Next())
    {
        BRepBuilderAPI_GTransform thescaler(shapeit.Value(), aTrsf);
        thescaler.Perform(shapeit.Value(), Standard_True);
        shapeit.Value() = thescaler.Shape();
    }
}


void occ::translateShapes(TopoDS_ListOfShape &shapes, Vector3d const &T)
{
    if(T.norm()<LENGTHPRECISION) return;

    gp_Trsf Translation;
    Translation.SetTranslation(gp_Vec(T.x, T.y, T.z));
    BRepBuilderAPI_Transform thetranslator(Translation);

    for(TopTools_ListIteratorOfListOfShape shapeit(shapes); shapeit.More(); shapeit.Next())
    {
        thetranslator.Perform(shapeit.Value(), Standard_True);
        shapeit.Value() = thetranslator.Shape();
    }
}


void occ::rotateShapes(TopoDS_ListOfShape &shapes, Vector3d const &O, Vector3d const &axis, double theta)
{
    if(fabs(theta)<ANGLEPRECISION) return;

    gp_Trsf Rot;
    Rot.SetRotation(gp_Ax1(gp_Pnt(O.x, O.y, O.z), gp_Vec(axis.x, axis.y, axis.z)), theta*PI/180.0);
    BRepBuilderAPI_Transform theRotation(Rot);

    for(TopTools_ListIteratorOfListOfShape shapeit(shapes); shapeit.More(); shapeit.Next())
    {
        theRotation.Perform(shapeit.Value(), Standard_True);
        shapeit.Value() = theRotation.Shape();
    }
}


void occ::flipShapeXZ(TopoDS_Shape &shape)
{
    gp_Trsf mirror;
    mirror.SetMirror(gp_Ax2(gp_Pnt(0,0,0), gp_Dir(0,1,0)));
    BRepBuilderAPI_Transform themirror(mirror);

    themirror.Perform(shape, Standard_True);
    shape = themirror.Shape();
}


void occ::scaleShape(TopoDS_Shape &shape, double scalefactor)
{
    if(fabs(scalefactor)<1.0e-6) return;
    if(fabs(scalefactor-1.0)<1.0e-6) return;

    gp_Trsf Scale;
    Scale.SetScale(gp_Pnt(0.0,0.0,0.0), scalefactor);
    BRepBuilderAPI_Transform thescaler(Scale);

    thescaler.Perform(shape, Standard_True);
//    shape = thescaler.Shape();
}


void occ::translateShape(TopoDS_Shape &shape, Vector3d const &T)
{
    if(T.norm()<LENGTHPRECISION) return;

    gp_Trsf Translation;
    Translation.SetTranslation(gp_Vec(T.x, T.y, T.z));
    BRepBuilderAPI_Transform thetranslator(Translation);

    thetranslator.Perform(shape, Standard_True);
    shape = thetranslator.Shape();
}


void occ::rotateShape(TopoDS_Shape &shape, Vector3d const &O, Vector3d const &axis, double theta)
{
    if(fabs(theta)<ANGLEPRECISION) return;

    gp_Trsf Rot;
    Rot.SetRotation(gp_Ax1(gp_Pnt(O.x, O.y, O.z), gp_Vec(axis.x, axis.y, axis.z)), theta*PI/180.0);
    BRepBuilderAPI_Transform theRotation(Rot);

    theRotation.Perform(shape, Standard_True);
//    shape = theRotation.Shape();
}


/**
 * Make a wing shape from two nurbs surface, one for the TOP surface and one for the BOTTOM surface.
 * This (a) ensures that there is an EDGE at the LE and (b) facilitates subsequent meshing operations.
 * The NURBS is constructed on the Surface's two end splines.
 * The end splines are approximation of the surface end sections.
 */
bool occ::makeWing2NurbsShape(WingXfl const *pWing, double stitchprecision, int degree, int nCtrlPoints, int nOutPoints,
                              TopoDS_Shape &wingshape, std::string &logmsg)
{
    QString logg;

    std::string str;

    if(!pWing)
    {
        logg += "No wing to process\n";
        return false;
    }
    std::string strange;

    std::string strong = "Processing wing "+ pWing->name() + "\n";
    logg += QString::fromStdString(strong);

    BRepBuilderAPI_Sewing stitcher(stitchprecision);

    for(int iSurf=0; iSurf<pWing->nSurfaces(); iSurf++)
    {
        Surface const &surf = pWing->surfaceAt(iSurf);

        BSpline3d b3dtop, b3dbot;
        TopoDS_Wire TopWire, BotWire;

        //LEFT TIP SURFACE
        if(surf.isTipLeft())
        {
            BRepBuilderAPI_MakeWire WireMaker;
            surf.makeSectionHalfSpline(xfl::TOPSURFACE, true, degree, nCtrlPoints, nOutPoints, b3dtop);
            surf.makeSectionHalfSpline(xfl::BOTSURFACE, true, degree, nCtrlPoints, nOutPoints, b3dbot);
            makeSplineWire(b3dtop, TopWire, str);
            logg += QString::fromStdString(str);
            makeSplineWire(b3dbot, BotWire, str);
            logg += QString::fromStdString(str);
            WireMaker.Add(TopWire);
            WireMaker.Add(BotWire);

/*            Handle(Geom_BSplineCurve) HTopCurve, HBotCurve;
            surf.makeSectionSplines(true,  true, HTopCurve);
            surf.makeSectionSplines(false, true, HBotCurve);
qDebug()<<"occspliness"<<HBotCurve->Degree()<<HBotCurve->NbPoles()<<HTopCurve->Degree()<<HTopCurve->NbPoles();
            BRepBuilderAPI_MakeEdge botedgemaker(HBotCurve);
            BRepBuilderAPI_MakeEdge topedgemaker(HTopCurve);
            if(!botedgemaker.IsDone() || !topedgemaker.IsDone())
            {
                logmsg += "Error making left EDGES";
                return false;
            }
            //assemble the top and bot wires to make a profile
            WireMaker.Add(botedgemaker.Edge());
            WireMaker.Add(topedgemaker.Edge());*/

            if(!WireMaker.IsDone())
            {
                logg += "   Error making left tip WIRE\n";
                return false;
            }
            TopoDS_Wire RightWire = WireMaker.Wire();
            if(RightWire.IsNull())
            {
                logg += "   Error making left tip WIRE\n";
                return false;
            }
            BRepBuilderAPI_MakeFace FaceMaker(RightWire);
            if(!FaceMaker.IsDone())
            {
                logg += "   Error making left tip FACE\n";
            }
            else
            {
                TopoDS_Face theLeftTipPatch = FaceMaker.Face();
                stitcher.Add(theLeftTipPatch);
            }
        }

        //RIGHT TIP PATCH
        if(surf.isTipRight())
        {
            surf.makeSectionHalfSpline(xfl::TOPSURFACE, false, degree, nCtrlPoints, nOutPoints, b3dtop);
            surf.makeSectionHalfSpline(xfl::BOTSURFACE, false, degree, nCtrlPoints, nOutPoints, b3dbot);
            makeSplineWire(b3dtop, TopWire, str);
            logg += QString::fromStdString(str);

            makeSplineWire(b3dbot, BotWire, str);
            logg += QString::fromStdString(str);

            //assemble the top and bot wires to make a profile
            BRepBuilderAPI_MakeWire WireMaker;
            WireMaker.Add(TopWire);
            WireMaker.Add(BotWire);
            if(!WireMaker.IsDone())
            {
                logg += "   Error making right tip WIRE\n";
                return false;
            }
            TopoDS_Wire RightWire = WireMaker.Wire();
            if(RightWire.IsNull())
            {
                logg += "   Error making right tip WIRE\n";
                return false;
            }
            BRepBuilderAPI_MakeFace FaceMaker(RightWire);
            if(!FaceMaker.IsDone())
            {
                logg += "   Error making right tip FACE\n";
            }
            else
            {
                TopoDS_Face theRightTipPatch = FaceMaker.Face();
                stitcher.Add(theRightTipPatch);
            }
        }

        // make a degree 1 nurbs between the  left and top right splines.
        BSpline3d b3dleft, b3dright;
        xfl::enumSurfacePosition bSide[] = {xfl::TOPSURFACE, xfl::BOTSURFACE}; // top, bottom
        for(int iside=0; iside<2; iside++)
        {
            surf.makeSectionHalfSpline(bSide[iside], true,  degree, nCtrlPoints, nOutPoints, b3dleft);
            surf.makeSectionHalfSpline(bSide[iside], false, degree, nCtrlPoints, nOutPoints, b3dright);
            NURBSSurface nurbs;
            nurbs.setUAxis(1);
            nurbs.setVAxis(0);
            nurbs.setuDegree(1);
            nurbs.setvDegree(b3dleft.degree());
            nurbs.setFrameCount(2);
            nurbs.setFramePointCount(b3dleft.nCtrlPoints());
            nurbs.frame(0).setCtrlPoints(b3dleft.controlPoints());
            nurbs.frame(1).setCtrlPoints(b3dright.controlPoints());
            nurbs.setKnots();

            //------Build the right NURBS surface-----
            Handle(Geom_BSplineSurface) HNURBSSurface;
            makeOCCNURBSFromNurbs(nurbs, false, HNURBSSurface, str);
            logg += QString::fromStdString(str);
            TopoDS_Face NURBSface;
            //-----make a shell from the surface-----
            try
            {
                BRepBuilderAPI_MakeFace shellmaker(HNURBSSurface,1.e-5);
                if(!shellmaker.IsDone())
                {
                    logg += "   Error making SHELL from NURBS... \n";
                    return false;
                }

                NURBSface = shellmaker.Face();

                if(NURBSface.IsNull())
                {
                    logg += "   NURBS shape is NULL\n";
                    return false;
                }
            }
            catch (StdFail_NotDone const &ex)
            {
                logg += "   BRepBuilderAPI_MakeShell::StdFail_NotDone" + QString(ex.GetMessageString()) +"\n";
                return false;
            }
            catch (...)
            {
                logg += "   BRepBuilderAPI_MakeShell::Unknown failure\n";
                return false;
            }

            if(iside==0) NURBSface.Reverse();
            stitcher.Add(NURBSface);
/*

Surface::s_DebugPts.clear();
Surface::s_DebugVecs.clear();
Handle(Geom_Surface) aSurface = BRep_Tool::Surface(NURBSface);
for(int iu=1; iu<3; iu++)
{
    double u = double(iu)/3.0;
    for(int iv=1; iv<3; iv++)
    {
        double v = double(iv)/3.0;
        GeomLProp_SLProps props(aSurface, u, v, 1, 1.e-5);
        if(props.IsNormalDefined())
        {
            gp_Pnt Pt = props.Value(); // is same as P

            gp_Dir Dir = props.Normal();

            Surface::s_DebugPts.append({Pt.X(), Pt.Y(), Pt.Z()});
            Surface::s_DebugVecs.append({Dir.X()/20.0, Dir.Y()/20.0, Dir.Z()/20.0});
        }
    }
}*/
        }
    }

    // stitch
    stitcher.Perform();
    logg += QString::asprintf("   Nb of free edges       = %d\n", stitcher.NbFreeEdges());
    logg += QString::asprintf("   Nb of contiguous edges = %d\n", stitcher.NbContigousEdges());
    logg += QString::asprintf("   Nb of multiple edges   = %d\n", stitcher.NbMultipleEdges());

//    If all faces have been sewn correctly, the result is a shell. Otherwise, it is a compound.
//    After a successful sewing operation all faces have a coherent orientation.

    try
    {
        TopoDS_Shape sewedshape = stitcher.SewedShape();
        if(sewedshape.IsNull()) return false;

        TopoDS_Shell WingShellShape = TopoDS::Shell(sewedshape);
        //make the solid
        if(!WingShellShape.IsNull())
        {
            BRepBuilderAPI_MakeSolid solidMaker(WingShellShape);
            if(!solidMaker.IsDone())
            {
                logg += "   Solid not made... \n";
                wingshape.Nullify();
                return false;
            }
            wingshape = solidMaker.Shape();

            logg += "   Wing stitching result is " + QString::fromStdString(shapeType(wingshape)) + "\n";

            listShapeContent(wingshape, strange, "   ", true);
            logg += QString::fromStdString(strange) +"\n";

            checkShape(wingshape, str, "   ");
/*            BRepCheck_Analyzer ShapeAnalyzer(wingshape);
            if(ShapeAnalyzer.IsValid()) logmsg += "   Shape topology is VALID\n\n";
            else                        logmsg += "   Shape topology is NOT VALID\n";*/
        }
    }
    catch(Standard_TypeMismatch const &ex)
    {
        logg += "     BRepBuilderAPI_MakeSolid::Standard_TypeMismatch " + QString(ex.GetMessageString()) +"\n";
    }

    logg += "\n";

    logmsg = logg.toStdString();

    return true;
}


bool occ::makeWingSplineSweep(WingXfl const *pWing, double stitchprecision, int degree, int nCtrlPoints, int nOutPoints,
                              TopoDS_Shape &wingshape, std::string &logmsg)
{

    if(!pWing)
    {
        logmsg += "No wing to process\n";
        return false;
    }

    std::string str;
    QString logg;
    QString strong = "Processing wing "+ QString::fromStdString(pWing->name()) + "\n";
    logg += strong;

    BRepBuilderAPI_Sewing stitcher(stitchprecision);

    BSpline3d b3dtopleft, b3dbotleft, b3dtopright, b3dbotright;
    TopoDS_Wire TopLeftWire, BotLeftWire, TopRightWire, BotRightWire;

    for(int iSurf=0; iSurf<pWing->nSurfaces(); iSurf++)
    {
        Surface const &surf = pWing->surfaceAt(iSurf);

        if(!surf.makeSectionHalfSpline(xfl::TOPSURFACE,  true, degree, nCtrlPoints, nOutPoints, b3dtopleft) ||
           !makeSplineWire(b3dtopleft, TopLeftWire, str))
        {
            logg += QString::fromStdString(str);
            logg += QString::asprintf("   Error making top left spline of surface %d", iSurf);
            return false;
        }

        if(!surf.makeSectionHalfSpline(xfl::BOTSURFACE, true, degree, nCtrlPoints, nOutPoints, b3dbotleft) ||
           !makeSplineWire(b3dbotleft, BotLeftWire, str))
        {
            logg += QString::fromStdString(str);
            logg += QString::asprintf("   Error making bottom left spline of surface %d", iSurf);
            return false;
        }

        if(!surf.makeSectionHalfSpline(xfl::TOPSURFACE,  false, degree, nCtrlPoints, nOutPoints, b3dtopright) ||
           !makeSplineWire(b3dtopright, TopRightWire, str))
        {
            logg += QString::fromStdString(str);
            logg += QString::asprintf("   Error making top right spline of surface %d", iSurf);
            return false;
        }

        if(!surf.makeSectionHalfSpline(xfl::BOTSURFACE, false, degree, nCtrlPoints, nOutPoints, b3dbotright) ||
           !makeSplineWire(b3dbotright, BotRightWire, str))
        {
            logg += QString::fromStdString(str);
            logg += QString::asprintf("   Error making bottom right spline of surface %d", iSurf);
            return false;
        }

        //LEFT TIP SURFACE
        if(surf.isTipLeft())
        {
            BRepBuilderAPI_MakeWire LeftWireMaker;
            LeftWireMaker.Add(TopLeftWire);
            LeftWireMaker.Add(BotLeftWire);

            if(!LeftWireMaker.IsDone())
            {
                logg += "   Error making left tip WIRE\n";
                return false;
            }
            TopoDS_Wire LeftWire = LeftWireMaker.Wire();
            if(LeftWire.IsNull())
            {
                logg += "   Error making left tip WIRE\n";
                return false;
            }
            BRepBuilderAPI_MakeFace FaceMaker(LeftWire);
            if(!FaceMaker.IsDone())
            {
                logg += "   Error making left tip FACE\n";
            }
            else
            {
                TopoDS_Face theLeftTipPatch = FaceMaker.Face();
                stitcher.Add(theLeftTipPatch);
            }
        }

        //RIGHT TIP PATCH
        if(surf.isTipRight())
        {
            //assemble the top and bot wires to make a profile
            BRepBuilderAPI_MakeWire RightWireMaker;
            RightWireMaker.Add(TopRightWire);
            RightWireMaker.Add(BotRightWire);
            if(!RightWireMaker.IsDone())
            {
                logg += "   Error making right tip WIRE\n";
                return false;
            }
            TopoDS_Wire RightWire = RightWireMaker.Wire();
            if(RightWire.IsNull())
            {
                logg += "   Error making right tip WIRE\n";
                return false;
            }
            BRepBuilderAPI_MakeFace FaceMaker(RightWire);
            if(!FaceMaker.IsDone())
            {
                logg += "   Error making right tip FACE\n";
            }
            else
            {
                TopoDS_Face theRightTipPatch = FaceMaker.Face();
                stitcher.Add(theRightTipPatch);
            }
        }

        // Top sweep
        BRepOffsetAPI_ThruSections TopSweeper(false, true);
//        qDebug()<<"topsweeeper"<<TopSweeper.ParType()<<TopSweeper.MaxDegree()<<TopSweeper.UseSmoothing()<<TopSweeper.Continuity();
/*        TopSweeper.CheckCompatibility(true);
        TopSweeper.SetSmoothing(false);
        TopSweeper.SetParType(Approx_IsoParametric);
        TopSweeper.SetContinuity(GeomAbs_C0); */

        TopSweeper.AddWire(TopRightWire);
        TopSweeper.AddWire(TopLeftWire);

        BRepOffsetAPI_ThruSections BotSweeper(false, true, 1.0e-4);
        BotSweeper.AddWire(BotLeftWire);
        BotSweeper.AddWire(BotRightWire);

        try
        {
//            TopSweeper.Build(); // does nothing according to OCC doc
//            BotSweeper.Build(); // does nothing according to OCC doc
            TopoDS_Shape TopSweptShape = TopSweeper.Shape(); // calls Build according to OCC doc
            TopoDS_Shape BotSweptShape = BotSweeper.Shape();
            stitcher.Add(TopSweptShape);
            stitcher.Add(BotSweptShape);
        }
        catch(Standard_DomainError const &)
        {
            logg += "     Standard_DomainError sweeping wires\n";
        }
        catch (StdFail_NotDone &)
        {
            logg += "   StdFail_NotDone sweeping wires\n";
            return false;
        }
        catch (...)
        {
            logg += "   Unknown error sweeping section wires\n";
            return false;
        }
    }

    // stitch
    stitcher.Perform();

//    If all faces have been sewn correctly, the result is a shell. Otherwise, it is a compound.
//    After a successful sewing operation all faces have a coherent orientation.

    try
    {
        TopoDS_Shape sewedshape = stitcher.SewedShape();
        if(sewedshape.IsNull()) return false;

        TopoDS_Shell WingShellShape = TopoDS::Shell(sewedshape);
        //make the solid
        if(!WingShellShape.IsNull())
        {
            BRepBuilderAPI_MakeSolid solidMaker(WingShellShape);
            if(!solidMaker.IsDone())
            {
                logg += "   Solid not made... \n";
                wingshape.Nullify();
                return false;
            }
            wingshape = solidMaker.Shape().Reversed();

            logg += "   Wing stitching result is " + QString::fromStdString(shapeType(wingshape)) + "\n";

            std::string log;

            checkShape(wingshape, log, "   ");
            logg += QString::fromStdString(log);
            log.clear();
            listShapeContent(wingshape, str, "   ", true);

            logg += QString::fromStdString(str);

            logg += QString::asprintf("   Nb. of free edges=%d\n",       stitcher.NbFreeEdges());
            logg += QString::asprintf("   Nb. of contiguous edges=%d\n", stitcher.NbContigousEdges());
        }
    }
    catch(Standard_TypeMismatch const &)
    {
        logg += "     WingShapes:: Type mismatch error\n";
        return false;
    }

    logg += "\n";
    return true;
}


bool occ::makeWingSweepMidSection(WingXfl const *pWing, TopoDS_Shape &midshape, std::string &logmsg)
{
    // Sweep the mid section
    BRepOffsetAPI_ThruSections Sweeper(false, false, 1.0e-4);
/*        Sweeper.CheckCompatibility(true);
        Sweeper.SetSmoothing(false);
        Sweeper.SetParType(Approx_IsoParametric);
        Sweeper.SetContinuity(GeomAbs_C0); */
    for(int jsurf=0; jsurf<pWing->nSurfaces(); jsurf++)
    {
        Surface const &surf = pWing->surfaceAt(jsurf);
        TopoDS_Wire midwire;
        if(!occ::makePolyLineWire(surf.m_SideA, midwire, logmsg))
            return false;
        Sweeper.AddWire(midwire);
    }

    midshape = Sweeper.Shape();
    if(!Sweeper.IsDone() || midshape.IsNull())
    {
        logmsg += "   Swept shape is invalid.\n";
        return false;
    }
    return true;
}


bool occ::makeWingSplineSweepMidSection(WingXfl const *pWing, int degree, int nCtrlPoints, int nOutPoints,
                                        TopoDS_Shape &wingshape, std::string &logmsg)
{
    if(!pWing)
    {
        logmsg += "No wing to process\n";
        return false;
    }

    std::string strong = "Processing wing "+ pWing->name() + "\n";
    logmsg += strong;

    BRepOffsetAPI_ThruSections MidSweeper(false, false);


    BSpline3d b3dmid;
    TopoDS_Wire MidWire;

    for(int iSurf=0; iSurf<pWing->nSurfaces(); iSurf++)
    {
        Surface const &surf = pWing->surfaceAt(iSurf);

        surf.makeSectionHalfSpline(xfl::MIDSURFACE, true, degree, nCtrlPoints, nOutPoints, b3dmid);
        makeSplineWire(b3dmid, MidWire, logmsg);

        Handle(Geom_BSplineCurve) botspline;
        makeOCCSplineFromBSpline3d(b3dmid, botspline, logmsg);

        MidSweeper.AddWire(MidWire);
    }

    try
    {
        MidSweeper.Build();
        if(!MidSweeper.IsDone())
        {
            logmsg += "     Error sweeping bottom section wires\n";
            return false;
        }

        wingshape = MidSweeper.Shape();
    }
    catch(Standard_DomainError const &)
    {
        logmsg += "     Standard_DomainError sweeping wires\n";
    }
    catch (StdFail_NotDone const &)
    {
        logmsg += "   StdFail_NotDone sweeping wires\n";
        return false;
    }
    catch (...)
    {
        logmsg += "   Unknown error sweeping section wires\n";
        return false;
    }

    logmsg += "\n";
    return !wingshape.IsNull();
}


bool occ::makeWingSplineSweepMultiSections(WingXfl const *pWing, double stitchprecision, int degree, int nCtrlPoints, int nOutPoints,
                                           TopoDS_Shape &wingshape, std::string &logmsg)
{
    if(!pWing)
    {
        logmsg += "No wing to process\n";
        return false;
    }

    std::string strong = "Processing wing "+ pWing->name() + "\n";
    logmsg += strong;

    BRepBuilderAPI_Sewing stitcher(stitchprecision);
    BRepOffsetAPI_ThruSections TopSweeper(false, false);
    BRepOffsetAPI_ThruSections BotSweeper(false, false);

/*   // No effect
    TopSweeper.SetMaxDegree(1);
    TopSweeper.SetContinuity(GeomAbs_C0);
    TopSweeper.SetParType(Approx_IsoParametric);*/

    BSpline3d b3dtop, b3dbot;
    TopoDS_Wire TopWire, BotWire;

    BRepBuilderAPI_MakeWire LeftWireMaker;
    BRepBuilderAPI_MakeWire RightWireMaker;

    for(int iSurf=0; iSurf<pWing->nSurfaces(); iSurf++)
    {
        Surface const &surf = pWing->surfaceAt(iSurf);

        surf.makeSectionHalfSpline(xfl::TOPSURFACE, true, degree, nCtrlPoints, nOutPoints, b3dtop);
        surf.makeSectionHalfSpline(xfl::BOTSURFACE, true, degree, nCtrlPoints, nOutPoints, b3dbot);
        makeSplineWire(b3dtop, TopWire, logmsg);
        makeSplineWire(b3dbot, BotWire, logmsg);

        Handle(Geom_BSplineCurve) topspline, botspline;
        makeOCCSplineFromBSpline3d(b3dtop, topspline, logmsg);
        makeOCCSplineFromBSpline3d(b3dbot, botspline, logmsg);

        TopSweeper.AddWire(TopWire);
        BotSweeper.AddWire(BotWire);

        //LEFT TIP SURFACE
        if(surf.isTipLeft())
        {
            LeftWireMaker.Add(TopWire);
            LeftWireMaker.Add(BotWire);

            if(!LeftWireMaker.IsDone())
            {
                logmsg += "   Error making left tip WIRE\n";
                return false;
            }
            TopoDS_Wire LeftWire = LeftWireMaker.Wire();
            if(LeftWire.IsNull())
            {
                logmsg += "   Error making left tip WIRE\n";
                return false;
            }
            BRepBuilderAPI_MakeFace FaceMaker(LeftWire);
            if(!FaceMaker.IsDone())
            {
                logmsg += "   Error making left tip FACE\n";
            }
            else
            {
                TopoDS_Face theLeftTipPatch = FaceMaker.Face();
                stitcher.Add(theLeftTipPatch);
            }
        }

        //RIGHT TIP PATCH
        if(surf.isTipRight())
        {
            surf.makeSectionHalfSpline(xfl::TOPSURFACE,  false, degree, nCtrlPoints, nOutPoints, b3dtop);
            surf.makeSectionHalfSpline(xfl::BOTSURFACE, false, degree, nCtrlPoints, nOutPoints, b3dbot);
            makeSplineWire(b3dtop, TopWire, logmsg);
            makeSplineWire(b3dbot, BotWire, logmsg);

            TopSweeper.AddWire(TopWire);
            BotSweeper.AddWire(BotWire);

            //assemble the top and bot wires to make a profile
            RightWireMaker.Add(TopWire);
            RightWireMaker.Add(BotWire);
            if(!RightWireMaker.IsDone())
            {
                logmsg += "   Error making right tip WIRE\n";
                return false;
            }
            TopoDS_Wire RightWire = RightWireMaker.Wire();
            if(RightWire.IsNull())
            {
                logmsg += "   Error making right tip WIRE\n";
                return false;
            }
            BRepBuilderAPI_MakeFace FaceMaker(RightWire);
            if(!FaceMaker.IsDone())
            {
                logmsg += "   Error making right tip FACE\n";
            }
            else
            {
                TopoDS_Face theRightTipPatch = FaceMaker.Face();
                stitcher.Add(theRightTipPatch);
            }
        }
    }
    try
    {
        TopSweeper.Build();
        if(!TopSweeper.IsDone())
        {
            logmsg += "     Error sweeping top section wires\n";
            return false;
        }
        BotSweeper.Build();
        if(!BotSweeper.IsDone())
        {
            logmsg += "     Error sweeping bottom section wires\n";
            return false;
        }

        TopoDS_Shape TopSweptShape = TopSweeper.Shape();
        TopoDS_Shape BotSweptShape = BotSweeper.Shape();
        stitcher.Add(TopSweptShape);
        stitcher.Add(BotSweptShape);
    }
    catch(Standard_DomainError const &)
    {
        logmsg += "     Standard_DomainError sweeping wires\n";
    }
    catch (StdFail_NotDone const &)
    {
        logmsg += "   StdFail_NotDone sweeping wires\n";
        return false;
    }
    catch (...)
    {
        logmsg += "   Unknown error sweeping section wires\n";
        return false;
    }


    // stitch
    stitcher.Perform();
    logmsg += QString::asprintf("   Nb of free edges=%d\n", stitcher.NbFreeEdges()).toStdString();
    logmsg += QString::asprintf("   Nb of contiguous edges=%d\n", stitcher.NbContigousEdges()).toStdString();

//    If all faces have been sewn correctly, the result is a shell. Otherwise, it is a compound.
//    After a successful sewing operation all faces have a coherent orientation.

    try
    {
        TopoDS_Shape sewedshape = stitcher.SewedShape();
        if(sewedshape.IsNull()) return false;

        TopoDS_Shell WingShellShape = TopoDS::Shell(sewedshape);
        //make the solid
        if(!WingShellShape.IsNull())
        {
            BRepBuilderAPI_MakeSolid solidMaker(WingShellShape);
            if(!solidMaker.IsDone())
            {
                logmsg += "   Solid not made... \n";
                wingshape.Nullify();
                return false;
            }
            wingshape = solidMaker.Shape();
//            wingshape = TopoDS::Shell(sewedshape);

            logmsg += "   Wing stitching result is " +shapeType(wingshape) + "\n";

            checkShape(wingshape, logmsg, "   ");
        }
    }
    catch(Standard_TypeMismatch const &)
    {
        logmsg += "     WingShapes:: Type mismatch error\n";
        return false;
    }

    logmsg += "\n";
    return true;
}


bool occ::makeWingSplineSweepSolid(WingXfl const *pWing, double , int degree, int nCtrlPoints, int nOutPoints,
                                   TopoDS_Shape &wingshape, std::string &logmsg)
{
    if(!pWing)
    {
        logmsg += "No wing to process\n";
        return false;
    }
    std::string strange = "Processing wing "+ pWing->name() + "\n";
    logmsg += strange;

    BRepOffsetAPI_ThruSections Sweeper(true, false);
//    Sweeper.SetMaxDegree(1);                  // No effect
//    Sweeper.SetContinuity(GeomAbs_C0);        // No effect
//    Sweeper.SetParType(Approx_IsoParametric); // No effect

    BSpline3d b3dtop, b3dbot;

    for(int iSurf=0; iSurf<pWing->nSurfaces(); iSurf++)
    {
        Surface const &surf = pWing->surfaceAt(iSurf);

        surf.makeSectionHalfSpline(xfl::TOPSURFACE, true, degree, nCtrlPoints, nOutPoints, b3dtop);
        surf.makeSectionHalfSpline(xfl::BOTSURFACE, true, degree, nCtrlPoints, nOutPoints, b3dbot);

        Handle(Geom_BSplineCurve) topspline, botspline;
        makeOCCSplineFromBSpline3d(b3dtop, topspline, logmsg);
        makeOCCSplineFromBSpline3d(b3dbot, botspline, logmsg);

        try
        {
            TopoDS_Edge topedge = BRepBuilderAPI_MakeEdge(topspline);
            TopoDS_Edge botedge = BRepBuilderAPI_MakeEdge(botspline);
            BRepBuilderAPI_MakeWire wiremaker;
            wiremaker.Add(topedge);
            wiremaker.Add(botedge);
            Sweeper.AddWire(wiremaker.Wire());
        }
        catch (StdFail_NotDone const &)
        {
            logmsg += "   Error making section wire\n";
            return false;
        }

        //RIGHT TIP PATCH
        if(surf.isTipRight())
        {
            surf.makeSectionHalfSpline(xfl::TOPSURFACE, false, degree, nCtrlPoints, nOutPoints, b3dtop);
            surf.makeSectionHalfSpline(xfl::BOTSURFACE, false, degree, nCtrlPoints, nOutPoints, b3dbot);

            Handle(Geom_BSplineCurve) topspline, botspline;
            makeOCCSplineFromBSpline3d(b3dtop, topspline, logmsg);
            makeOCCSplineFromBSpline3d(b3dbot, botspline, logmsg);

            try
            {
                TopoDS_Edge topedge = BRepBuilderAPI_MakeEdge(topspline);
                TopoDS_Edge botedge = BRepBuilderAPI_MakeEdge(botspline);
                BRepBuilderAPI_MakeWire wiremaker;
                wiremaker.Add(topedge);
                wiremaker.Add(botedge);
                Sweeper.AddWire(wiremaker.Wire());
            }
            catch (StdFail_NotDone const &)
            {
                logmsg += "   Error making section wire\n";
                return false;
            }
        }
    }

    try
    {
        wingshape = Sweeper.Shape().Reversed();
    }
    catch(Standard_DomainError const &)
    {
        logmsg += "     Standard_DomainError sweeping wires\n";
    }
    catch (StdFail_NotDone const &)
    {
        logmsg += "   StdFail_NotDone sweeping wires\n";
        return false;
    }
    catch (...)
    {
        logmsg += "   Unknown error sweeping section wires\n";
        return false;
    }


    logmsg += "\n";
    return true;
}


#define EDGERES 11

void occ::makeShapeEdges(TopoDS_Shape const &shape, std::vector<std::vector<Segment3d>> &edges)
{
    if(shape.IsNull())
    {
        for(uint i=0; i<edges.size(); i++)
            edges[i].clear();
        edges.clear();
    }

    TopExp_Explorer shapeExplorer;
    edges.resize(nEdges(shape));
    for(uint i=0; i<edges.size(); i++)
        edges[i].resize(EDGERES);

    Standard_Real First=0, Last=0;
    gp_Pnt pt0, pt1;
    double df0=0, df1=0, u0=0, u1=0;
    int iEdge=0;
    for (shapeExplorer.Init(shape, TopAbs_EDGE); shapeExplorer.More(); shapeExplorer.Next())
    {
        TopoDS_Edge const &edge = TopoDS::Edge(shapeExplorer.Current());

        Handle(Geom_Curve) const edgecurve = BRep_Tool::Curve(edge, First, Last);
        std::vector<Segment3d> &slg = edges[iEdge];
        for(int i=0; i<EDGERES; i++)
        {
            df0 = double(i)  /(EDGERES);
            df1 = double(i+1)/(EDGERES);
            u0 = First + df0 * (Last-First);
            u1 = First + df1 * (Last-First);
            edgecurve->D0(u0, pt0);
            edgecurve->D0(u1, pt1);
            slg[i].setNodes({pt0.X(), pt0.Y(), pt0.Z()}, {pt1.X(), pt1.Y(), pt1.Z()}); // Node normals are not needed
        }
        iEdge++;
    }
}


/**
 * OCC duplicates the EDGEs shared by two FACEs, but does not apparently
 * provide any tool to check that they are the same.
 * This method creates vertices on each EDGE and checks that they are the same.
 * @todo there must be a simpler way to do this
 */
bool occ::isSameEdge(TopoDS_Edge const &edge0, TopoDS_Edge const &edge1)
{
    if(fabs(edgeLength(edge0)-edgeLength(edge1))>0.0001) return false;

    BRepAdaptor_Curve CA0(edge0);
    BRepAdaptor_Curve CA1(edge1);

    double u00 = CA0.FirstParameter();
    double u01 = CA0.LastParameter();

    double u10 = CA1.FirstParameter();
    double u11 = CA1.LastParameter();

    double u0=0, u1=0;
    gp_Pnt P0, P1;
    for(int i=0; i<5; i++)
    {
        u0 = u00 + (u01-u00)*double(i)/4.0;
        P0 = CA0.Value(u0);
        u1 = u10 + (u11-u10)*double(i)/4.0;
        P1 = CA1.Value(u1);
        if(P0.Distance(P1)>1.0e-4) return false;
    }
    return true;
}


void occ::getShapeFace(TopoDS_Shape const &shape, int iFace, TopoDS_Face &face)
{
    TopExp_Explorer shapeExplorer;
    int jFace=0;
    for (shapeExplorer.Init(shape, TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
    {
        if(jFace==iFace)
        {
            face = TopoDS::Face(shapeExplorer.Current());
            return;
        }
        jFace++;
    }
}


int occ::nFaces(TopoDS_Shape const &shape)
{
    TopExp_Explorer shapeExplorer;
    int iFace=0;
    for (shapeExplorer.Init(shape, TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
    {
        iFace++;
    }
    return iFace;
}


int occ::nEdges(TopoDS_Shape const &shape)
{
    TopExp_Explorer shapeExplorer;
    int iEdge=0;
    for (shapeExplorer.Init(shape, TopAbs_EDGE); shapeExplorer.More(); shapeExplorer.Next())
    {
        iEdge++;
    }
    return iEdge;
}


void occ::getEdge(TopoDS_Shape const &shape, int iFace, int iEdge, TopoDS_Edge &edge)
{
    TopExp_Explorer FaceExplorer;
    TopExp_Explorer EdgeExplorer;
    int iface=0, iedge=0;

    for(FaceExplorer.Init(shape, TopAbs_FACE); FaceExplorer.More(); FaceExplorer.Next())
    {
        if(iface==iFace)
        {
            TopoDS_Face const &face = TopoDS::Face(FaceExplorer.Current());

            iedge = 0;
            for(EdgeExplorer.Init(face, TopAbs_EDGE); EdgeExplorer.More(); EdgeExplorer.Next())
            {
                if(iedge==iEdge)
                {
                    edge = TopoDS::Edge(EdgeExplorer.Current());
                    return;
                }
                iedge++;
            }
        }
        iface++;
    }
}


void occ::removeFace(TopoDS_Face const &face, TopoDS_Shape &shape)
{
    BRepTools_ReShape reshape;
    reshape.Remove(face);
    reshape.Apply(shape);
}


bool occ::shapeToBrep(const TopoDS_Shape &shape, std::string &brep)
{
    try
    {
        std::stringstream sstream;
        sstream.str(std::string()); // clear the stream
        BRepTools::Write(shape, sstream); // stream the brep to the stringstream
        brep = sstream.str();
        return true;
    }
    catch(...)
    {

    }

    return false;
}


bool occ::shapesToBreps(TopoDS_ListOfShape const &shapes, std::vector<std::string> &breps)
{
    std::string brepstr;
    breps.clear();
    for(TopTools_ListIteratorOfListOfShape shapeit(shapes); shapeit.More(); shapeit.Next())
    {
        if(!occ::shapeToBrep(shapeit.Value(), brepstr)) return false;
        breps.push_back(brepstr);
    }

    return true;
}



