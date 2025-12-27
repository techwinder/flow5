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

#include <string>

#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepGProp.hxx>
#include <BRepTools.hxx>
#include <GProp_GProps.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shell.hxx>
#include <gp_Ax1.hxx>




#include <fuse.h>

#include <geom_global.h>
#include <constants.h>
#include <planepolar.h>
#include <objects_global.h>
#include <occ_globals.h>
#include <panel3.h>
#include <panel4.h>
#include <units.h>



Fuse::Fuse() : Part()
{
    m_Name = "Fuse name";

    m_FuseType = Fuse::Other;

    m_bLocked = false;

    m_WettedArea = 0.0;
    m_MaxWidth = 0.0;
    m_MaxHeight = 0.0;
    m_MaxFrameArea = 0.0;

    m_rx=m_ry=m_rz=0.0;

    // temporary variable for multithreaded fuse intersections
    m_nBlocks = 0;

    m_MaxElementSize  = 0.05;
}


/** The copy constructor. Reimplemented to perform deep copy on the arrays of shapes */
Fuse::Fuse(const Fuse &aFuse)
{
    // don't call virtual method in constructor
    duplicatePart(aFuse);
    m_bLocked = false;

    m_FuseType     = aFuse.m_FuseType;
    m_WettedArea   = aFuse.m_WettedArea;
    m_MaxWidth     = aFuse.m_MaxWidth;
    m_MaxHeight    = aFuse.m_MaxHeight;
    m_MaxFrameArea = aFuse.m_MaxFrameArea;

    m_BaseTriangulation = aFuse.m_BaseTriangulation;
    m_Triangulation = aFuse.m_Triangulation;

    m_Shape = aFuse.m_Shape;
    m_Shell = aFuse.m_Shell;

    m_TriMesh     = aFuse.m_TriMesh;

    m_MaxElementSize = aFuse.m_MaxElementSize;
    m_OccTessParams = aFuse.m_OccTessParams;
}


void Fuse::duplicateFuse(Fuse const &aFuse)
{
    duplicatePart(aFuse);
    m_bLocked = false;

    m_FuseType     = aFuse.m_FuseType;
    m_WettedArea   = aFuse.m_WettedArea;
    m_MaxWidth     = aFuse.m_MaxWidth;
    m_MaxHeight    = aFuse.m_MaxHeight;
    m_MaxFrameArea = aFuse.m_MaxFrameArea;

    m_BaseTriangulation = aFuse.m_BaseTriangulation;
    m_Triangulation = aFuse.m_Triangulation;

    m_Shape = aFuse.m_Shape;
    m_Shell = aFuse.m_Shell;

    m_TriMesh     = aFuse.m_TriMesh;

    m_MaxElementSize = aFuse.m_MaxElementSize;
    m_OccTessParams = aFuse.m_OccTessParams;
}


void Fuse::makeShellsFromShapes()
{
    m_Shell.Clear();
    for(TopTools_ListIteratorOfListOfShape shapeit(m_Shape); shapeit.More(); shapeit.Next())
    {
        TopExp_Explorer shapeExplorer;
        for (shapeExplorer.Init(shapeit.Value(),TopAbs_SHELL); shapeExplorer.More(); shapeExplorer.Next())
        {
            TopoDS_Shell aShell = TopoDS::Shell(shapeExplorer.Current());
            m_Shell.Append(aShell);
        }
    }
}


/**
 * Makes a default triangular mesh.
 * Overriden for xfl and stl type fuses.
 */
int Fuse::makeDefaultTriMesh(std::string &, const std::string &)
{
    m_TriMesh.clearMesh();
    return 0;
}


void Fuse::translateTriPanels(double tx, double ty, double tz)
{
    m_TriMesh.translatePanels(tx, ty, tz);
}


void Fuse::computeWettedArea()
{
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

    m_WettedArea = aProps.Mass(); // in m²
}


double Fuse::formFactor() const
{
    double dl = sqrt(m_MaxWidth*m_MaxWidth+m_MaxHeight*m_MaxHeight)/length();
    return 1.0 + 1.5*pow(dl, 1.5) + 50.0*pow(dl,3);
}


void Fuse::clearOccTriangulation()
{
    for(TopTools_ListIteratorOfListOfShape ShapeIt(m_Shell); ShapeIt.More(); ShapeIt.Next())
    {
        BRepTools::Clean(ShapeIt.Value());
    }
}


void Fuse::makeFuseGeometry()
{
    std::string logmsg;
    computeSurfaceProperties(logmsg, "");
}


bool Fuse::stitchShells(TopoDS_Shell &fusedshell, std::string &msg, std::string prefx)
{
    QString prefix = QString::fromStdString(prefx);
    QString logmsg;
    // sew the Panels together
    BRepBuilderAPI_Sewing stitcher(0.001);
    int nFaces=0;

    for(TopTools_ListIteratorOfListOfShape FaceIt(m_Shell); FaceIt.More(); FaceIt.Next())
    {
        stitcher.Add(FaceIt.Value());
        nFaces++;
    }

    (void)nFaces;

    try
    {
        QString strong;
        logmsg += prefix + "Sewing shells\n";
        stitcher.Perform();
        TopoDS_Shape sewedShape = stitcher.SewedShape();
        if(!sewedShape.IsNull())
        {
            strong = prefix + "Sewed shape is a "+ QString::fromStdString(occ::shapeType(sewedShape))+"\n";
            logmsg+= strong;

            if(sewedShape.ShapeType()==TopAbs_SHELL)
            {
                fusedshell = TopoDS::Shell(stitcher.SewedShape()); // is a TopoDS_SHELL
            }
            else
            {
                strong = prefix + QString::asprintf("Nb of free edges=%d\n",stitcher.NbFreeEdges());
                logmsg += strong;
                strong = prefix + QString::asprintf("Nb of contiguous edges=%d\n", stitcher.NbContigousEdges());
                logmsg += strong;
            }
        }
        else
        {
            throw(11);
        }
    }
    catch(Standard_TypeMismatch &ex)
    {
        QString strong;
        strong = prefix + "Fused shells not made: " + QString(ex.GetMessageString())+"\n";
        logmsg += strong;
        msg = logmsg.toStdString();
        return false;
    }
    catch(...)
    {
        QString strong;
        strong = prefix + "Unrecoverable error sewing shells\n";
        logmsg += strong;
        msg = logmsg.toStdString();
        return false;
    }

    return true;
}


void Fuse::scale(double XFactor, double YFactor, double ZFactor)
{
    for(int it=0; it<m_BaseTriangulation.nTriangles(); it++)
        m_BaseTriangulation.triangle(it).scale(XFactor, YFactor, ZFactor);

    for(int it=0; it<m_Triangulation.nTriangles(); it++) m_Triangulation.triangle(it).scale(XFactor, YFactor, ZFactor);
    m_Length *= XFactor;

    // scale the tri mesh, if any
    /** @todo standardize with translate and rotate methods */
    m_TriMesh.scale(XFactor, YFactor, ZFactor);
}


void Fuse::translate(Vector3d const &T)
{
    for(int it=0; it<m_BaseTriangulation.nTriangles(); it++)
    {
        m_BaseTriangulation.triangle(it).translate(T);
    }

    for(int it=0; it<m_Triangulation.nTriangles(); it++)
    {
        m_Triangulation.triangle(it).translate(T);
    }
    for(int i=0; i<m_Triangulation.nodeCount(); i++)
    {
        m_Triangulation.node(i).translate(T);
    }
}


void Fuse::rotate(Vector3d const &origin, Vector3d const &axis, double theta)
{
    m_BaseTriangulation.rotate(origin, axis, theta);
    m_Triangulation.rotate(origin, axis, theta);
}


void Fuse::computeAero(std::vector<Panel4> const &panel4, const double *Cp,
                       PlanePolar const *pWPolar, double Alpha, Vector3d &CP, Vector3d &Force, Vector3d &Moment) const
{
    double panellift=0.0, totallift=0.0;
    Vector3d PanelForce, LeverArm;


    Vector3d WindNormal = objects::windNormal(Alpha, 0.0);

    // Inviscid pressure forces
    Vector3d FiBodyAxis(0,0,0);
    Vector3d MiBodyAxis(0,0,0);
    CP.set(0.0,0.0,0.0);
    for (int p=0; p<nPanel4(); p++)
    {
        Panel4 const &p4 = panel4.at(m_FirstPanel4Index + p);
        PanelForce = p4.normal() * (-Cp[p4.index()]) * p4.area();     // N/q
        FiBodyAxis += PanelForce;                            // N/q

        panellift = PanelForce.dot(WindNormal);              // N/q
        totallift += panellift;                              // N/q
        CP        += p4.CoG() * panellift;                   // N.m/q

        LeverArm = p4.CoG() - pWPolar->CoG();                // m
        MiBodyAxis += LeverArm * PanelForce;               // N.m/q
    }

    /** @todo need to choose a definition for the Centre of Pressure */
    CP  *= 1.0/totallift;                           // N.m/q

    Force = FiBodyAxis;
    Moment = MiBodyAxis;
}


void Fuse::computeAero(std::vector<Panel3> const &panel3, const double *Cp3Vtx,
                       PlanePolar const *pWPolar, double Alpha, Vector3d &CP, Vector3d &Force, Vector3d &Moment) const
{
    double panellift=0.0, totallift=0.0;
    Vector3d PanelForce, LeverArm;


    //   Define wind axis
    Vector3d WindDirection = objects::windDirection(Alpha, 0.0);
    Vector3d WindNormal = objects::windNormal(Alpha, 0.0);

    // Inviscid pressure forces
    Vector3d FiBodyAxis(0,0,0);
    Vector3d MiBodyAxis(0,0,0);
    CP.set(0.0,0.0,0.0);
    for (int p=0; p<nPanel3(); p++)
    {
        Panel3 const &p3 = panel3.at(m_FirstPanel3Index + p);
        int index = p3.index();
        PanelForce = p3.normal() * (-Cp3Vtx[3*index]) * p3.area();     // N/q
        FiBodyAxis += PanelForce;                            // N/q

        panellift = PanelForce.dot(WindNormal);              // N/q
        totallift += panellift;                              // N/q
        CP        += p3.CoG() * panellift;                   // N.m/q

        LeverArm = p3.CoG() - pWPolar->CoG();                // m
        MiBodyAxis += LeverArm * PanelForce;                 // N.m/q
    }

    /** @todo need to choose a definition for the Centre of Pressure */
    CP *= 1.0/totallift;                           // N.m/q

    Force = FiBodyAxis;
    Moment = MiBodyAxis;
}


void Fuse::computeViscousForces(PlanePolar const *pWPolar, double Alpha, double QInf, Vector3d &Force, Vector3d &Moment) const
{
    Vector3d WindDirection = objects::windDirection(Alpha, 0.0);

    // add viscous drag force and moment
    Vector3d FvBodyAxis(0,0,0);
    Vector3d MvBodyAxis(0,0,0);
    if(pWPolar->hasFuseDrag())
    {
        FvBodyAxis = WindDirection * pWPolar->fuseDrag(this, QInf);       // N/q
        MvBodyAxis = FvBodyAxis * (m_Inertia.CoG_t()-pWPolar->CoG());                                       // N.m/q
    }

    // project viscous force on wind axes
//    m_AF.setFuseDrag(FvBodyAxis.dot(WindDirection));        // N/q
//    m_AF.setMv(MvBodyAxis);        // N.m/q   // =0

    Force = FvBodyAxis;
    Moment = MvBodyAxis;
}


bool Fuse::serializePartFl5(QDataStream &ar, bool bIsStoring)
{
    Part::serializePartFl5(ar, bIsStoring);

    int n=0;

    // 500001: new fl5 format;
    // 500003; added max element size in beta 12
    int ArchiveFormat = 500003;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << m_LE.x << m_LE.y << m_LE.z;
        ar << m_rx << m_ry << m_rz;

        m_OccTessParams.serializeParams(ar, bIsStoring);

        ar << m_MaxElementSize;

        n=0;
        ar << n;
        ar << n;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500001 || ArchiveFormat>500003) return false;

        ar >> m_LE.x >> m_LE.y >> m_LE.z;
        ar >> m_rx >> m_ry >> m_rz;

        if(ArchiveFormat>=500002)
            m_OccTessParams.serializeParams(ar, bIsStoring);

        if(ArchiveFormat>=500003)
            ar >> m_MaxElementSize;
        else
            m_MaxElementSize = m_OccTessParams.maxElementSize();

        ar >> n;
        ar >> n;
    }
    return true;
}


void Fuse::computeStructuralInertia(Vector3d const &PartPosition)
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


void Fuse::getProperties(std::string &properties, const std::string &prefx, bool )
{
    QString prefix = QString::fromStdString(prefx);
    QString props;
    QString lenlab  = Units::lengthUnitQLabel();
    QString arealab = Units::areaUnitQLabel();

    switch(m_FuseType)
    {
        case FlatFace:  props = prefix + "Type            = FLAT FACES\n";  break;
        case NURBS:     props = prefix + "Type            = NURBS\n";      break;
        case Sections:  props = prefix + "Type            = SECTIONS\n";   break;
        case Occ:       props = prefix + "Type            = OCC\n";        break;
        case Stl:       props = prefix + "Type            = STL\n";        break;
        case Other:     props = prefix + "Type            = OTHER\n";      break;
    }

    std::string str;
    computeSurfaceProperties(str, prefx);

    QString strong = QString::fromStdString(str);
    strong = QString::asprintf("Length          = %9.5g ", length()*Units::mtoUnit());
    strong += lenlab + "\n";
    props +=  prefix + strong;

    strong = QString::asprintf("Max. width      = %9.5g ", m_MaxWidth*Units::mtoUnit());
    strong += lenlab + "\n";
    props +=  prefix + strong;

    strong = QString::asprintf("Max. height     = %9.5g ", m_MaxHeight*Units::mtoUnit());
    strong += lenlab + "\n";
    props +=  prefix + strong;

    strong = QString::asprintf("Max. cross area = %9.5g ", m_MaxFrameArea*Units::m2toUnit());
    strong += arealab + "\n";
    props +=  prefix + strong;

    strong = QString::asprintf("Wetted area     = %9.5g ", wettedArea()*Units::m2toUnit());
    strong += arealab + "\n";
    props +=  prefix + strong;

    strong = QString::asprintf("Triangles       = %6d", nPanel3());
    props +=  prefix + strong;

    properties = props.toStdString();
}


/** used in gl3dFuseView */
bool Fuse::intersectFuseTriangulation(Vector3d const &A, Vector3d const &B, Vector3d &I) const
{
    Vector3d N;
    return m_BaseTriangulation.intersect(A, B, I, N);
}


/** used for wing surface construction */
bool Fuse::intersectFuse(Vector3d const &A, Vector3d const &B, Vector3d &I) const
{
    return intersectFuse(A,B,I,false);
}


bool Fuse::intersectFuse(Vector3d const &A, Vector3d const &B, Vector3d &I, bool bRightSide) const
{
    int nShapes = m_Shape.Extent();
    std::vector<bool> bIntersect(nShapes, false);
    std::vector<Vector3d> ISh(nShapes);

    int iShape = 0;
    for(TopTools_ListIteratorOfListOfShape shapeit(m_Shape); shapeit.More(); shapeit.Next())
    {
        bIntersect[iShape] = occ::intersectShape(shapeit.Value(), {A,B}, ISh[iShape], bRightSide);
        iShape++;
    }


    bool bResult = false;
    double dmax = LARGEVALUE;
    for(int i=0; i<nShapes; i++)
    {
        if(bIntersect[i])
        {
            bResult = true;
            double d = sqrt((ISh.at(i).x-A.x)*(ISh.at(i).x-A.x)+(ISh.at(i).y-A.y)*(ISh.at(i).y-A.y)+(ISh.at(i).z-A.z)*(ISh.at(i).z-A.z));
            if(d<dmax)
            {
                I = ISh[i];
                dmax=d;
            }
        }
    }

    return bResult;
}


Fuse::enumType Fuse::bodyPanelType(std::string strPanelType)
{
    if(strPanelType.compare("FLATPANELS")==0) return Fuse::FlatFace;
    else                                      return Fuse::NURBS;
}


std::string Fuse::bodyPanelType(Fuse::enumType panelType)
{
    switch(panelType)
    {
        case Fuse::FlatFace:  return "FLATPANELS";
        case Fuse::NURBS: return "NURBS";
        default: return std::string();
    }
}


void Fuse::listShells()
{
    QString strange;
    int ishell=0;
    for(TopTools_ListIteratorOfListOfShape iterator(shells()); iterator.More(); iterator.Next())
    {
        strange = QString::asprintf("Shell %d ", ishell);
        if     (iterator.Value().Orientation()==TopAbs_FORWARD)  strange += "is FORWARD";
        else if(iterator.Value().Orientation()==TopAbs_REVERSED) strange += "is REVERSED";
        qDebug("%s", strange.toStdString().c_str());

        std::string str;
        occ::listShapeContent(iterator.Value(), str, "   ");
        qDebug("%s", str.c_str());

        ishell++;
    }
}


void Fuse::listShapes()
{
    QString strange;
    int ishell=0;
    for(TopTools_ListIteratorOfListOfShape iterator(shapes()); iterator.More(); iterator.Next())
    {
        strange = QString::asprintf("Shell %d ", ishell);
        if     (iterator.Value().Orientation()==TopAbs_FORWARD)  strange += "is FORWARD";
        else if(iterator.Value().Orientation()==TopAbs_REVERSED) strange += "is REVERSED";
        qDebug("%s", strange.toStdString().c_str());

        std::string str;
        occ::listShapeContent(iterator.Value(), str, "   ");
        qDebug("%s", str.c_str());

        ishell++;
    }
}



