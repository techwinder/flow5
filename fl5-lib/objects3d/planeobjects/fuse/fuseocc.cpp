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



#include <string>
#include <sstream>
#include <QString>


#include <QDataStream>

#include <BRepAdaptor_Curve.hxx>
#include <BRepGProp.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <GProp_GProps.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopoDS.hxx>

#include <fuseocc.h>
#include <panel4.h>
#include <constants.h>
#include <units.h>
#include <panel3.h>
#include <occ_globals.h>


FuseOcc::FuseOcc() : Fuse()
{
    m_FuseType = Fuse::Occ;
    m_Name = "CAD type fuse";
    m_theStyle.m_Color.setRgb(142,161,183);
}


bool FuseOcc::serializePartFl5(QDataStream &ar, bool bIsStoring)
{
    Fuse::serializePartFl5(ar, bIsStoring);

    int nIntSpares=0;
    int nDbleSpares=0;
    int n=0;
    double dble=0;

    //500001 : new fl5 format;

    int ArchiveFormat = 500001;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar<<int(m_Shape.Size());

        /** @todo use global occ::shapeToBrep */
        std::stringstream sstream;
        for(TopTools_ListIteratorOfListOfShape shapeit(m_Shape); shapeit.More(); shapeit.Next())
        {
            sstream.str(std::string()); // clear the stream
            BRepTools::Write(shapeit.Value(), sstream); // stream the brep to the stringstream
            std::string string = sstream.str();
            QString brepstr = QString::fromStdString(string);
            ar << brepstr; // write the QString to the archive file
        }

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        dble=0.0;
        for (int i=0; i<nDbleSpares; i++) ar << dble;

    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat!=500001) return false;

        m_Shape.Clear();
        int nShapes;
        ar >> nShapes;
        for(int iShape=0; iShape<nShapes; iShape++)
        {
            QString brepstr;
            ar >> brepstr;
            try
            {
                std::stringstream sstream;
                sstream << brepstr.toStdString().c_str();

                TopoDS_Shape shape;
                BRep_Builder aBuilder;
                BRepTools::Read(shape, sstream, aBuilder);
                if(shape.IsNull())
                {
//                    qDebug()<<"Error serializing CAD fuse " + m_Name;
                    return false;
                }
                m_Shape.Append(shape);
            }
            catch(...)
            {
//                qDebug()<<"Error converting Rrep for CAD fuse " + m_Name;
                return false;
            }
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        makeShellsFromShapes();
        makeFuseGeometry();
    }
    return true;
}


void FuseOcc::getProperties(std::string &properties, std::string const &prefx, bool bFull)
{
    Fuse::getProperties(properties, prefx);

    if(bFull)
    {
        QString strange;
        strange = QString::asprintf("Fuse is made of %d shapes\n", int(m_Shape.Size()));
        properties += "\n"+prefx+strange.toStdString();

        std::string occstr;
        for(TopTools_ListIteratorOfListOfShape shapeit(m_Shape); shapeit.More(); shapeit.Next())
        {
            occ::listShapeContent(shapeit.Value(), occstr, prefx);
            properties += prefx+occstr;
        }
    }
}


void FuseOcc::scale(double XFactor, double , double )
{
    Fuse::scale(XFactor, XFactor, XFactor);
    occ::scaleShapes(m_Shape, XFactor);
    occ::scaleShapes(m_Shell, XFactor);

}


void FuseOcc::translate(const Vector3d &T)
{
    Fuse::translate(T);
    occ::translateShapes(m_Shape, T);
    occ::translateShapes(m_Shell, T);
    translateTriPanels(T);
}


void FuseOcc::rotate(const Vector3d &origin, const Vector3d &axis, double theta)
{
    Fuse::rotate(origin, axis, theta);
    occ::rotateShapes(m_Shape, origin, axis, theta);
    makeShellsFromShapes();
    makeFuseGeometry();

    m_TriMesh.rotatePanels(origin, axis, theta);
}


void FuseOcc::reverseFuse()
{
    TopoDS_ListIteratorOfListOfShape iterator;
//    m_Shape.Clear();
    for (iterator.Initialize(m_Shape); iterator.More(); iterator.Next())
    {
        iterator.Value().Reverse();
    }
}


/** returns an array of segements from the body's TopoDS_Edge;*/
void FuseOcc::makeEdges(std::vector<Segment3d>&lines)
{
    lines.clear();
    TopoDS_ListIteratorOfListOfShape iterator;
    int nShapes = 0;
    for (iterator.Initialize(m_Shape); iterator.More(); iterator.Next())
    {
        TopExp_Explorer shapeExplorer;
        int nEdges = 0;
        for (shapeExplorer.Init(iterator.Value(),TopAbs_EDGE); shapeExplorer.More(); shapeExplorer.Next())
        {
            TopoDS_Edge anEdge = TopoDS::Edge(shapeExplorer.Current());
            BRepAdaptor_Curve curveadaptor(anEdge);
            double Umin = curveadaptor.FirstParameter();
            double Umax = curveadaptor.LastParameter();

            gp_Pnt pt0;
            pt0 = curveadaptor.Value(Umin);

            gp_Pnt pt1;
            pt1 = curveadaptor.Value(Umax);

            lines.push_back({Vector3d(pt0.X(), pt0.Y(), pt0.Z()), Vector3d(pt1.X(), pt1.Y(), pt1.Z())});
            nEdges++;
        }
        (void)nEdges;
        nShapes++;
    }
    (void)nShapes;
}


void FuseOcc::computeSurfaceProperties(std::string &logmsg, const std::string &prefix)
{
    computeWettedArea();

    double xmax=0.0, xmin=0.0;
    double ymax=0.0, ymin=0.0;
    double zmax=0.0, zmin=0.0;
    m_Length = 0.0;
    m_WettedArea = 0.0;
    for(int i=0; i<nTriangles(); i++)
    {
        Triangle3d const &T = triangle(i);
        m_WettedArea += T.area();
        xmin = std::min(xmin, T.vertexAt(0).x);
        xmin = std::min(xmin, T.vertexAt(1).x);
        xmin = std::min(xmin, T.vertexAt(2).x);
        xmax = std::max(xmax, T.vertexAt(0).x);
        xmax = std::max(xmax, T.vertexAt(1).x);
        xmax = std::max(xmax, T.vertexAt(2).x);

        ymin = std::min(ymin, T.vertexAt(0).y);
        ymin = std::min(ymin, T.vertexAt(1).y);
        ymin = std::min(ymin, T.vertexAt(2).y);
        ymax = std::max(ymax, T.vertexAt(0).y);
        ymax = std::max(ymax, T.vertexAt(1).y);
        ymax = std::max(ymax, T.vertexAt(2).y);

        zmin = std::min(zmin, T.vertexAt(0).z);
        zmin = std::min(zmin, T.vertexAt(1).z);
        zmin = std::min(zmin, T.vertexAt(2).z);
        zmax = std::max(zmax, T.vertexAt(0).z);
        zmax = std::max(zmax, T.vertexAt(1).z);
        zmax = std::max(zmax, T.vertexAt(2).z);
    }
    m_Length = fabs(xmin-xmax);
    m_MaxWidth = fabs(ymax-ymin);
    m_MaxHeight = fabs(zmax-zmin);

    std::string strong;
    strong = QString::asprintf("Length          = %9.5g ", length()*Units::mtoUnit()).toStdString();
    strong += Units::lengthUnitLabel() + "\n";
    logmsg += prefix + strong;

    strong = QString::asprintf("Max. width      = %9.5g ", m_MaxWidth*Units::mtoUnit()).toStdString();
    strong += Units::lengthUnitLabel() + "\n";
    logmsg += prefix + strong;

    strong = QString::asprintf("Max. height     = %9.5g ", m_MaxHeight*Units::mtoUnit()).toStdString();
    strong += Units::lengthUnitLabel() + "\n";
    logmsg += prefix + strong;

    strong = QString::asprintf("Wetted area     = %9.5g ", m_WettedArea*Units::m2toUnit()).toStdString();
    strong += Units::areaUnitLabel();
    logmsg += prefix + strong;
}



