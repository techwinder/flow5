/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#define _MATH_DEFINES_DEFINED

#include <string>
#include <sstream>


#include <QDataStream>
#include <QDebug>

#include <BRepAdaptor_Curve.hxx>
#include <BRepGProp.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <GProp_GProps.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopoDS.hxx>

#include "fuseocc.h"
#include <xflpanels/panels/panel4.h>
#include <xflmath/constants.h>

#include <xflpanels/panels/panel3.h>
#include <xflcore/units.h>
#include <xflocc/occ_globals.h>

FuseOcc::FuseOcc() : Fuse()
{
    m_FuseType = Fuse::Occ;
    m_Name = "CAD type fuse";
}


void FuseOcc::computeStructuralInertia(const Vector3d &PartPosition)
{
    if(!m_bAutoInertia) return;

    Vector3d CoG;

    GProp_GProps aProps; // Global properties object
    // initializing Global properties object with surface properties of aShell

    TopoDS_ListIteratorOfListOfShape iterator;
    int nShapes = 0;
    for (iterator.Initialize(m_Shape); iterator.More(); iterator.Next())
    {
        int nShells = 0;
        TopExp_Explorer shapeExplorer;
        for (shapeExplorer.Init(iterator.Value(),TopAbs_SHELL); shapeExplorer.More(); shapeExplorer.Next())
        {
            // Computes the surface global properties of the shape S, i.e. the global properties induced by each face
            // of the shape S, and brings them together with the global properties still retained by the framework SProps.
            // If the current system of SProps was empty, its global properties become equal to the surface global
            // properties of S. For this computation, no surface density is attached to the faces. Consequently, the
            // added mass corresponds to the sum of the areas of the faces of S. The density of the component systems,
            // i.e. that of each component of the current system of SProps, and that of S which is considered to be
            // equal to 1, must be coherent. Note that this coherence cannot be checked. You are advised to use a
            // framework for each different value of density, and then to bring these frameworks together into a
            // global one. The point relative to which the inertia of the system is computed is the reference point
            // of the framework SProps. Note : if your programming ensures that the framework SProps retains only
            // surface global properties, brought together, for example, by the function SurfaceProperties, for
            // objects the density of which is equal to 1 (or is not defined), the function Mass will return the
            // total area of faces of the system analysed by SProps. Warning No check is performed to verify that the
            // shape S retains truly surface properties. If S is simply a vertex, an edge or a wire, it is not considered
            // to present any additional global properties. SkipShared is special flag, which allows to take in calculation
            // shared topological entities or not For ex., if SkipShared = True, faces, shared by two or more shells,
            // are taken into calculation only once.
            GProp_GProps shellprops;
            BRepGProp::SurfaceProperties(shapeExplorer.Current(), shellprops);
            aProps.Add(shellprops, 1.0);
            nShells++;
        }
        (void)nShells;
        nShapes++;
    }
    (void)nShapes;
    gp_Mat IM = aProps.MatrixOfInertia();

    m_WettedArea = aProps.Mass(); // with unit density

    double CoGIxx = IM(1,1)/m_WettedArea*m_Inertia.structuralMass();
    double CoGIyy = IM(2,2)/m_WettedArea*m_Inertia.structuralMass();
    double CoGIzz = IM(3,3)/m_WettedArea*m_Inertia.structuralMass();
    double CoGIxz = IM(1,3)/m_WettedArea*m_Inertia.structuralMass();

    gp_Pnt com = aProps.CentreOfMass();
    CoG.set(com.X()-PartPosition.x, com.Y()-PartPosition.y, com.Z()-PartPosition.z);

    m_Inertia.setCoG_s(CoG);
    m_Inertia.setIxx_s(CoGIxx);
    m_Inertia.setIyy_s(CoGIyy);
    m_Inertia.setIzz_s(CoGIzz);
    m_Inertia.setIxz_s(CoGIxz);

/*
    qDebug("fuse %s inertia:\n", fuseName().toStdString().c_str());
    qDebug("%s\n",m_Inertia.toString().toStdString().c_str());
    qDebug("____\n\n");
*/
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

        std::stringstream sstream;
        for(TopTools_ListIteratorOfListOfShape shapeit(m_Shape); shapeit.More(); shapeit.Next())
        {
            sstream.str(std::string()); // clear the stream
            BRepTools::Write(shapeit.Value(), sstream); // stream the brep to the stringstream
            std::string s = sstream.str();
            QString brepstr = QString::fromStdString(s);
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

            // do a deep copy before using the QString in the istream
            QByteArray ba = brepstr.toLatin1();
            const int q = ba.size();
//            const int m = brepstr.size();
//            const int l = strlen(brepstr.toStdString().c_str());
            char *strong = new char[q+100];
            memset(strong, 0, q+100);

            strcpy(strong, ba.constData());

            try
            {
                std::stringstream sstream;
    
                sstream.exceptions(std::ios::failbit | std::ios::eofbit | std::ios::badbit);

                sstream << strong;

                TopoDS_Shape shape;
                BRep_Builder aBuilder;
                BRepTools::Read(shape, sstream, aBuilder);
                m_Shape.Append(shape);
//                delete [] strong;
            }
            catch(...)
            {
                qDebug()<<"reading exception";
//                delete [] strong;
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


void FuseOcc::getProperties(QString &props, QString const &prefix, bool bFull)
{
    Fuse::getProperties(props, prefix);

    if(bFull)
    {
        QString strange;
        strange = QString::asprintf("Fuse is made of %d shapes\n", m_Shape.Size());
        props += "\n"+prefix+strange;
        for(TopTools_ListIteratorOfListOfShape shapeit(m_Shape); shapeit.More(); shapeit.Next())
        {
            occ::listShapeContent(shapeit.Value(), strange, prefix);
            props += prefix+strange;
        }
    }
}


void FuseOcc::scale(double XFactor, double , double )
{
    Fuse::scale(XFactor, XFactor, XFactor);
    occ::scaleShapes(m_Shape, XFactor);
    occ::scaleShapes(m_Shell, XFactor);
    QString strange;
    makeShellTriangulation(strange, QString());
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
    QString logmsg;

    makeShellTriangulation(logmsg, "   ");
}


/** returns an array of segements from the body's TopoDS_Edge;*/
void FuseOcc::makeEdges(QVector<Segment3d>&lines)
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


void FuseOcc::computeSurfaceProperties(QString &logmsg, QString const &prefix)
{
    computeWettedArea();

    double xmax=0.0, xmin=0.0;
    double ymax=0.0, ymin=0.0;
    double zmax=0.0, zmin=0.0;
    m_Length = 0.0;
    m_WettedArea = 0.0;
    for(int i=0; i<triangleCount(); i++)
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

    QString strong;
    strong = QString::asprintf("Length          = %9.5g ", length()*Units::mtoUnit());
    strong += Units::lengthUnitLabel() + "\n";
    logmsg += prefix + strong;

    strong = QString::asprintf("Max. width      = %9.5g ", m_MaxWidth*Units::mtoUnit());
    strong += Units::lengthUnitLabel() + "\n";
    logmsg += prefix + strong;

    strong = QString::asprintf("Max. height     = %9.5g ", m_MaxHeight*Units::mtoUnit());
    strong += Units::lengthUnitLabel() + "\n";
    logmsg += prefix + strong;

    strong = QString::asprintf("Wetted area     = %9.5g ", m_WettedArea*Units::m2toUnit());
    strong += Units::areaUnitLabel();
    logmsg += prefix + strong;
}



