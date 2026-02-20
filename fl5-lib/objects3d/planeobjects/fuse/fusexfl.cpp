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


#define _MATH_DEFINES_DEFINED

#include <QString>
#include <QDebug>


#include <BRepBuilderAPI_Copy.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeShell.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepGProp.hxx>
#include <BRepTools.hxx>
#include <GC_MakeSegment.hxx>
#include <GProp_GProps.hxx>
#include <GeomAPI_PointsToBSplineSurface.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomFill_ConstrainedFilling.hxx>
#include <GeomFill_SimpleBound.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Standard_Version.hxx>
#include <StdFail_NotDone.hxx>
#include <TopExp_Explorer.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Ax2.hxx>

#include <fusexfl.h>

#include <constants.h>
#include <frame.h>
#include <geom_global.h>
#include <occ_globals.h>
#include <panel3.h>
#include <panel4.h>
#include <pointmass.h>
#include <quad2d.h>
#include <quad3d.h>
#include <segment2d.h>
#include <surface.h>
#include <triangulation.h>
#include <units.h>



#include <occ_globals.h>


FuseXfl::FuseXfl(Fuse::enumType fusetype) : Fuse()
{
    if(fusetype!=Fuse::FlatFace && fusetype!=Fuse::NURBS && fusetype!=Fuse::Sections)
        fusetype= Fuse::NURBS;
    m_FuseType = fusetype;
    m_Name = "xfl type fuse";

    m_iHighlightFrame  = -1;

    m_nxNurbsPanels = 11;
    m_nhNurbsPanels = 5;
}


void FuseXfl::makeDefaultFuse()
{
    m_theStyle.m_Color.setRgb(179,183,203);
    m_nxNurbsPanels = 19;
    m_nhNurbsPanels = 7;

    m_xPanels.clear();
    m_hPanels.clear();

    if(isSplineType())
    {
        m_nurbs.setuDegree(3);
        m_nurbs.setvDegree(3);

        for(int ifr=0; ifr<7; ifr++)
        {
            m_nurbs.appendNewFrame();
            m_nurbs.frame(ifr).clearCtrlPoints();
            m_xPanels.push_back(1);
            m_hPanels.push_back(1);
            for(int is=0; is<5; is++)
            {
                m_nurbs.frame(ifr).appendCtrlPoint(Vector3d(0.0,0.0,0.0));
            }
        }

        frame(0).setCtrlPoint(0, 0.0, 0.0, -0.0585);
        frame(0).setCtrlPoint(1, 0.0, 0.0, -0.0585);
        frame(0).setCtrlPoint(2, 0.0, 0.0, -0.0585);
        frame(0).setCtrlPoint(3, 0.0, 0.0, -0.0585);
        frame(0).setCtrlPoint(4, 0.0, 0.0, -0.0585);

        frame(1).setCtrlPoint(0, 0.005, 0.000,0.017162);
        frame(1).setCtrlPoint(1, 0.005, 0.029,0.013762);
        frame(1).setCtrlPoint(2, 0.005, 0.034,-0.06103);
        frame(1).setCtrlPoint(3, 0.005, 0.029,-0.10183);
        frame(1).setCtrlPoint(4, 0.005, 0.000,-0.10523);

        frame(2).setCtrlPoint(0, 0.355, 0.0000, 0.12005);
        frame(2).setCtrlPoint(1, 0.355, 0.0728, 0.130259);
        frame(2).setCtrlPoint(2, 0.355, 0.0962, -0.00234);
        frame(2).setCtrlPoint(3, 0.355, 0.0884, -0.14514);
        frame(2).setCtrlPoint(4, 0.355, 0.0000, -0.15874);

        frame(3).setCtrlPoint(0, 0.605, 0.0000,  0.0783);
        frame(3).setCtrlPoint(1, 0.605, 0.0312,  0.0579);
        frame(3).setCtrlPoint(2, 0.605, 0.0468, -0.0032);
        frame(3).setCtrlPoint(3, 0.605, 0.0312, -0.0644);
        frame(3).setCtrlPoint(4, 0.605, 0.0000, -0.0848);

        frame(4).setCtrlPoint(0, 0.839, 0.000, 0.05348);
        frame(4).setCtrlPoint(1, 0.839, 0.02386, 0.063298);
        frame(4).setCtrlPoint(2, 0.839, 0.03367, 0.012383);
        frame(4).setCtrlPoint(3, 0.839, 0.01703, -0.0163);
        frame(4).setCtrlPoint(4, 0.839, 0.000, -0.0083597);

        frame(5).setCtrlPoint(0, 1.623, 0.000,  0.065);
        frame(5).setCtrlPoint(1, 1.623, 0.026,  0.063);
        frame(5).setCtrlPoint(2, 1.623, 0.031,  0.018);
        frame(5).setCtrlPoint(3, 1.623, 0.026, -0.016);
        frame(5).setCtrlPoint(4, 1.623, 0.000, -0.018);

        frame(6).setCtrlPoint(0, 1.638, 0.00, 0.035);
        frame(6).setCtrlPoint(1, 1.638, 0.00, 0.029);
        frame(6).setCtrlPoint(2, 1.638, 0.00, 0.024);
        frame(6).setCtrlPoint(3, 1.638, 0.00, 0.020);
        frame(6).setCtrlPoint(4, 1.638, 0.00, 0.015);

        frame(0).setuPosition(m_nurbs.uAxis(), 0.000);
        frame(1).setuPosition(m_nurbs.uAxis(), 0.031);
        frame(2).setuPosition(m_nurbs.uAxis(), 0.355);
        frame(3).setuPosition(m_nurbs.uAxis(), 0.605);
        frame(4).setuPosition(m_nurbs.uAxis(), 0.839);
        frame(5).setuPosition(m_nurbs.uAxis(), 1.592);
        frame(6).setuPosition(m_nurbs.uAxis(), 1.638);
    }

    setPanelPos();
    setNURBSKnots();

    std::string logmsg;
    makeDefaultTriMesh(logmsg, "");
}


void FuseXfl::makeDefaultHull()
{
    m_theStyle.m_Color.setRgb(89,136,143);
    m_nxNurbsPanels = 11;
    m_nhNurbsPanels = 7;

    m_xPanels.clear();
    m_hPanels.clear();

    m_nurbs.clearFrames();
    for(int ifr=0; ifr<4; ifr++)
    {
        m_nurbs.appendNewFrame();
        m_nurbs.frame(ifr).clearCtrlPoints();
        m_xPanels.push_back(1);
        m_hPanels.push_back(1);
        for(int is=0; is<4; is++)
        {
            m_nurbs.frame(ifr).appendCtrlPoint(Vector3d(0.0,0.0,0.0));
        }
    }

    m_nurbs.frame(0).ctrlPoint(0).set(-1.0, 0.0, 0.2);
    m_nurbs.frame(0).ctrlPoint(1).set(-1.0, 0.0, 0.2);
    m_nurbs.frame(0).ctrlPoint(2).set(-1.0, 0.0, 0.2);
    m_nurbs.frame(0).ctrlPoint(3).set(-1.0, 0.0, 0.2);

    m_nurbs.frame(1).ctrlPoint(0).set(0.0, 0.0, -0.0);
    m_nurbs.frame(1).ctrlPoint(1).set(0.0, 2.0, -0.0);
    m_nurbs.frame(1).ctrlPoint(2).set(0.0, 0.268, -0.739);
    m_nurbs.frame(1).ctrlPoint(3).set(0.0, 0.0, -1.5);

    m_nurbs.frame(2).ctrlPoint(0).set(3.0, 0.0, 0.0);
    m_nurbs.frame(2).ctrlPoint(1).set(3.0, 1.5, 0.0);
    m_nurbs.frame(2).ctrlPoint(2).set(3.0, 0.5,-0.5);
    m_nurbs.frame(2).ctrlPoint(3).set(3.0, 0.0,-1.0);

    m_nurbs.frame(3).ctrlPoint(0).set(6.0,  0.0, .0);
    m_nurbs.frame(3).ctrlPoint(1).set(6.0,  0.0, .0);
    m_nurbs.frame(3).ctrlPoint(2).set(6.0,  0.0, .0);
    m_nurbs.frame(3).ctrlPoint(3).set(6.0,  0.0, .0);

    m_nurbs.frame(0).setuPosition(m_nurbs.uAxis(), -1.0);
    m_nurbs.frame(1).setuPosition(m_nurbs.uAxis(), 0.0);
    m_nurbs.frame(2).setuPosition(m_nurbs.uAxis(), 3.0);
    m_nurbs.frame(3).setuPosition(m_nurbs.uAxis(), 6.0);

    m_xPanels.resize(m_nurbs.frameCount());
    m_hPanels.resize(m_nurbs.framePointCount());
    for(int i=0; i<nxPanels();i++) m_xPanels[i] = 1;
    for(int i=0; i<nhPanels();i++) m_hPanels[i] = 1;

    m_nurbs.setuDegree(3);
    m_nurbs.setvDegree(3);

    setPanelPos();
    m_nurbs.setKnots();

    // make the default, ruled, triangular mesh to be at the same progress
    // as STL and OCC fuses
    std::string logmsg;
    makeDefaultTriMesh(logmsg, "");
}


/** the copy constructor*/
FuseXfl::FuseXfl(FuseXfl const &aFuseXfl) : Fuse(aFuseXfl)
{
    duplicateFuseXfl(aFuseXfl);
}


void FuseXfl::duplicateFuse(const Fuse &aFuse)
{
    FuseXfl const&xflfuse = dynamic_cast<const FuseXfl&>(aFuse);
    duplicateFuseXfl(xflfuse);
}


/**
 * Copies the data of an existing Body object to this Body
 */
void FuseXfl::duplicateFuseXfl(const FuseXfl &aFuseXfl)
{
    Fuse::duplicateFuse(aFuseXfl);

    //copy the splined surface data
    m_nxNurbsPanels = aFuseXfl.m_nxNurbsPanels;
    m_nhNurbsPanels = aFuseXfl.m_nhNurbsPanels;

    m_nurbs.copy(aFuseXfl.m_nurbs);
    if(isSplineType()) setNURBSKnots();

    m_xPanels = aFuseXfl.m_xPanels;
    m_hPanels = aFuseXfl.m_hPanels;
    m_XPanelPos = aFuseXfl.m_XPanelPos;


    TopoDS_ListIteratorOfListOfShape iterator;
    m_RightSideShell.Clear();
    for (iterator.Initialize(aFuseXfl.m_RightSideShell); iterator.More(); iterator.Next())
    {
        m_RightSideShell.Append(iterator.Value());
    }

    m_iHighlightFrame = -1;
}


void FuseXfl::makeFuseGeometry()
{
    setNURBSKnots();
    setPanelPos();

    // make the faces even for a NURBS type body
    // may be of use for fast intersection of rays in the 3d view
    makeBodyFaces();

    std::string logmsg;
    makeShape(logmsg);
    makeShellsFromShapes();

    computeSurfaceProperties(logmsg, "");
}


int FuseXfl::makeShape(std::string &log)
{
    m_Shape.Clear();
    m_Shell.Clear();
    m_RightSideShell.Clear();

    makeBodySplineShape(log);

    return m_Shape.Extent();
}


void FuseXfl::computeSurfaceProperties(std::string &msg, const std::string &prefx)
{
    QString prefix = QString::fromStdString(prefx);
    QString logmsg;
    QString strong;

    // computeWettedArea();
    // makeQuadMesh(0);
    m_WettedArea = 0.0;
//    for(int i4=0; i4<m_Panel4.size(); i4++)        m_WettedArea += m_Panel4.at(i4).area();
    for(int i3=0; i3<nTriangles(); i3++)  m_WettedArea += triangleAt(i3).area();

    // compute max section width, height and estimated area
    // in each frame section
    Vector3d Point;
    std::vector<Vector3d> rightpoints;
    logmsg.clear();
    m_MaxWidth = 0.0;
    m_MaxHeight = 0.0;
    m_MaxFrameArea = 0.0;

    for(int i=0; i<nurbs().frameCount(); i++)
    {
        rightpoints.clear();
        double halfwidth = 0;
        if(isSplineType() || isSectionType())
        {
            int nh = 20;
            double hinc = 1.0/double(nh-1);
            double u = getu(frame(i).position().x);
            double v = 0.0;
            for (int k=0; k<nh; k++)
            {
                getPoint(u,v,true, Point);
                rightpoints.push_back(Point);
                halfwidth = std::max(halfwidth, fabs(Point.y));
                v += hinc;
            }
        }
        else if(isFlatFaceType())
        {
            for(int ic=0; ic<sideLineCount(); ic++)
            {
                Point = m_nurbs.frame(i).ctrlPointAt(ic);
                rightpoints.push_back(Point);
                halfwidth = std::max(halfwidth, fabs(Point.y));
            }
        }

        m_MaxWidth = std::max(m_MaxWidth, halfwidth*2.0);

        double frameheight = (rightpoints.back()-rightpoints.front()).norm();
        m_MaxHeight = std::max(m_MaxHeight, frameheight);

        BRepBuilderAPI_MakePolygon HalfPolyMaker;
        for(int k=0; k<int(rightpoints.size()); k++)
        {
            // if P or V is coincident to the previous vertex, no edge is built
            HalfPolyMaker.Add(gp_Pnt(rightpoints.at(k).x, rightpoints.at(k).y, rightpoints.at(k).z));
        }

        //close the polygon
        try
        {
            // raises an execption if less than two points, i.e. for the leading and trailing frames
            HalfPolyMaker.Close();
        }
        catch(StdFail_NotDone &)
        {
//            qDebug()<<"FuseXfl::computeSurfaceProperties"<<e.GetMessageString();
        }
        catch(...)
        {
//            qDebug()<<"FuseXfl::computeSurfaceProperties"<<"Unknown error";
        }

        double area=0.0;
        if(HalfPolyMaker.IsDone())
        {
            TopoDS_Face halfface = BRepBuilderAPI_MakeFace(HalfPolyMaker);
            GProp_GProps shellprops;
            BRepGProp::SurfaceProperties(halfface, shellprops);
            area = shellprops.Mass();
        }

        m_MaxFrameArea = std::max(m_MaxFrameArea, area);
    }

    m_Length = length();

    strong = QString::asprintf("Length            = %11g ", m_Length*Units::mtoUnit());
    strong += Units::lengthUnitQLabel();
    logmsg += prefix + strong + "\n";

    strong = QString::asprintf("Total wetted area = %11g ", m_WettedArea*Units::m2toUnit());
    strong += Units::areaUnitQLabel();
    logmsg += prefix + strong + "\n";

    strong = QString::asprintf("Max. frame area   = %11g ", m_MaxFrameArea*Units::m2toUnit());
    strong += Units::areaUnitQLabel();
    logmsg += prefix + strong + "\n";

    strong = QString::asprintf("Max. frame width  = %11g ", m_MaxWidth*Units::mtoUnit());
    strong += Units::lengthUnitQLabel();
    logmsg += prefix + strong + "\n";

    strong = QString::asprintf("Max. frame height = %11g ", m_MaxHeight*Units::mtoUnit());
    strong += Units::lengthUnitQLabel();
    logmsg += prefix + strong + "\n";

    msg = logmsg.toStdString();
}


/**
 * Calculates and returns the length of the Fuse measured from nose to tail.
 * @return the Body length
 */
double FuseXfl::length() const
{
    double Length;
    if(m_nurbs.frameCount())
        Length = fabs(m_nurbs.lastFrame().position().x - m_nurbs.firstFrame().position().x);
    else
        Length = 1.0;
    return Length;
}


/** returns the approximate height of the body measured as the distance of opposite control points in the Frames */
double FuseXfl::height() const
{
    double h=0;
    for(int i=0; i<frameCount(); i++)
    {
        double zmin = 1000.0;
        double zmax = -1000.0;
        for(int ic=0; ic<nurbs().frameAt(i).nCtrlPoints(); ic++)
        {
            zmin = std::min(nurbs().frameAt(i).pointAt(ic).z, zmin);
            zmax = std::max(nurbs().frameAt(i).pointAt(ic).z, zmax);
            h = std::max(zmax-zmin, h);
        }
    }
    return h;
}

/**
 * Returns the posistion of the Body nose
 * @return the Vector3d which defines the position of the Body nose.
 */
Vector3d FuseXfl::leadingPoint()
{
    if(m_nurbs.frameCount())
    {
        return Vector3d(m_nurbs.firstFrame().position().x,
                        0.0,
                        (m_nurbs.firstFrame().firstControlPoint().z + m_nurbs.firstFrame().lastControlPoint().z)/2.0 );
    }
    else return Vector3d(0.0, 0.0, 0.0);
}


/**
 * Returns the length of a 360 degree hoop arc at a given axial position of the Body
 * Used to evaluate local volume and inertias.
 * @param x the longitudinal position at which the arc length is to be calculated.
 * @return  the arc length, in meters.
 */
double FuseXfl::getSectionArcLength(double x) const
{
    // aproximate arc length, used for inertia estimations
    double length = 0.0;
    double ux = getu(x);
    Vector3d Pt, Pt1;
    getPoint(ux, 0.0, true, Pt1);

    int NPoints = 10;//why not?
    for(int i=1; i<=NPoints; i++)
    {
        getPoint(ux, double(i)/double(NPoints), true, Pt);
        length += sqrt((Pt.y-Pt1.y)*(Pt.y-Pt1.y) + (Pt.z-Pt1.z)*(Pt.z-Pt1.z));
        Pt1.y = Pt.y;
        Pt1.z = Pt.z;
    }
    return length*2.0; //to account for left side.
}


Vector3d FuseXfl::centerPoint(double u)
{
    Vector3d Top, Bot;
    getPoint(u, 0.0, true, Top);
    getPoint(u, 1.0, true, Bot);
    return (Top+Bot)/2.0;
}


/**
 * Calculates the absolute position of a point on the NURBS from its parametric coordinates.
 * @param u the value of the parameter in the longitudinal direction
 * @param v the value of the parameter in the hoop direction
 * @param bRight if true, the position of the point will be returned for the right side,
 * and for the left side if false
 * @param Pt the calculated point position
 */
void FuseXfl::getPoint(double u, double v, bool bRight, Vector3d &Pt) const
{
    m_nurbs.getPoint(u, v, Pt);
    if(!bRight)  Pt.y = -Pt.y;
}


/**
 * Returns the absolute position of a point on the NURBS from its parametric coordinates.
 * @param u the value of the parameter in the longitudinal direction
 * @param v the value of the parameter in the hoop direction
 * @param bRight if true, the position of the point will be returned for the right side,
 * and for the left side if false
 */
Vector3d FuseXfl::Point(double u, double v, bool bRight) const
{
    Vector3d Pt;
    m_nurbs.getPoint(u, v, Pt);
    if(!bRight)  Pt.y = -Pt.y;
    return Pt;
}


/**
 * Returns the value of the longitudinal parameter given the absolute X-position ON the NURBS surface.
 * @param x in input, the longitudinal position
 * @return the longitudinal paramater on the NURBS surface
 */
double FuseXfl::getu(double x) const
{
    return m_nurbs.getu(x,0.0);
}

/**
 * For a NURBS surface: Given a value of the longitudinal parameter and a vector in the yz plane, returns the
 * value of the hoop paramater for the intersection of a ray originating on the x-axis
 * and directed along the input vector
 * @param u in input, the value of the longitudinal parameter
 * @param r the vector which defines the ray's direction
 * @param bRight true if the intersection should be calculated on the Body's right side, and flase if on the left
 * @return the value of the hoop parameter
 */
double FuseXfl::getv(double u, Vector3d r, bool bRight) const
{
    double sine = 10000.0;

    Vector3d t_R;
    if(u<=0.0)          return 0.0;
    if(u>=1.0)          return 0.0;
    if(r.norm()<1.0e-5) return 0.0;

    int iter=0;
    double v, v1, v2;

    sine = 10000.0;
    iter = 0;
    r.normalize();
    v1 = 0.0; v2 = 1.0;

    while(fabs(sine)>1.0e-4 && iter<200)
    {
        v=(v1+v2)/2.0;
        t_R = Point(u, v, bRight);
        t_R.x = 0.0;
        t_R.normalize();//t_R is the unit radial vector for u,v

        sine = (r.y*t_R.z - r.z*t_R.y);

        if(bRight)
        {
            if(sine>0.0) v1 = v;
            else         v2 = v;
        }
        else
        {
            if(sine>0.0) v2 = v;
            else         v1 = v;
        }
        iter++;
    }
    return (v1+v2)/2.0;
}


void FuseXfl::insertPoint(int iPt)
{
    m_nurbs.insertPoint(iPt);

    m_hPanels.insert(m_hPanels.begin()+iPt, 1);

    setNURBSKnots();
}


/**
 * Inserts a control point in the selected Frame.
 * @param Real the Vector3d which defines the point to insert
 * @return the index of the control point in the array; control points are indexed from bottom to top on the left side.
 */
void FuseXfl::insertPoint(const Vector3d &Real)
{
    int n = activeFrame().insertPoint(Real, 3);

    for (int i=0; i<frameCount(); i++)
    {
        Frame &pFrame = m_nurbs.frame(i);
        if(i != activeFrameIndex())
        {
            pFrame.insertPoint(n);
        }
    }

    m_hPanels.insert(m_hPanels.begin()+n, 1);

    setNURBSKnots();
}


/**
 * Inserts a new Frame object in the Body definition
 * @param iFrame the index of the frame before which a new Frame will be inserted
 * @return the index of the Frame which has been inserted; Frame objects are indexed from nose to tail
 */
int FuseXfl::insertFrameBefore(int iFrame)
{
    Frame pFrame = Frame(sideLineCount());
    if(iFrame==0)
    {
        pFrame.setuPosition(m_nurbs.uAxis(), frame(0).position().x-0.1);
        m_nurbs.prependFrame(pFrame);
    }
    else
    {
        pFrame.setuPosition(m_nurbs.uAxis(), (frame(iFrame).position().x+frame(iFrame-1).position().x)/2.0);

        int n = iFrame;
        m_nurbs.insertFrame(n, pFrame);

        for (int ic=0; ic<sideLineCount(); ic++)
        {
            m_nurbs.frame(n).ctrlPoint(ic).x = (m_nurbs.frame(n+1).ctrlPointAt(ic).x + m_nurbs.frame(n+1).ctrlPointAt(ic).x)/2.0;
            m_nurbs.frame(n).ctrlPoint(ic).y = (m_nurbs.frame(n+1).ctrlPointAt(ic).y + m_nurbs.frame(n+1).ctrlPointAt(ic).y)/2.0;
            m_nurbs.frame(n).ctrlPoint(ic).z = (m_nurbs.frame(n+1).ctrlPointAt(ic).z + m_nurbs.frame(n+1).ctrlPointAt(ic).z)/2.0;
        }
    }
    m_xPanels.insert(m_xPanels.begin()+iFrame, 1);

    setNURBSKnots();
    return iFrame;
}


/**
 * Inserts a new Frame object in the Body definition
 * @param iFrame the index of the frame after which a new Frame will be inserted
 * @return the index of the Frame which has been inserted; Frame objects are indexed from nose to tail
 */
int FuseXfl::insertFrameAfter(int iFrame)
{
    Frame pFrame = Frame(sideLineCount());
    if(iFrame==frameCount()-1)
    {
        pFrame.setuPosition(m_nurbs.uAxis(), frame(iFrame).position().x+0.1);
        m_nurbs.appendFrame(pFrame);
    }
    else
    {
        pFrame.setuPosition(m_nurbs.uAxis(), (frame(iFrame).position().x+frame(iFrame+1).position().x)/2.0);

        int n = iFrame+1;
        m_nurbs.insertFrame(n, pFrame);

        for (int ic=0; ic<sideLineCount(); ic++)
        {
            m_nurbs.frame(n).ctrlPoint(ic).x = (m_nurbs.frame(n+1).ctrlPointAt(ic).x + m_nurbs.frame(n+1).ctrlPointAt(ic).x)/2.0;
            m_nurbs.frame(n).ctrlPoint(ic).y = (m_nurbs.frame(n+1).ctrlPointAt(ic).y + m_nurbs.frame(n+1).ctrlPointAt(ic).y)/2.0;
            m_nurbs.frame(n).ctrlPoint(ic).z = (m_nurbs.frame(n+1).ctrlPointAt(ic).z + m_nurbs.frame(n+1).ctrlPointAt(ic).z)/2.0;
        }
    }

    m_xPanels.insert(m_xPanels.begin()+iFrame+1, 1);

    setNURBSKnots();

    return iFrame+1;
}


/**
 * Inserts a new Frame object in the Body definition
 * @param Real the Vector3d which defines the x and z coordinates of the Frame to insert
 * @return the index of the Frame which has been inserted; Frame objects are indexed from nose to tail
 */
int FuseXfl::insertFrame(Vector3d const &Real)
{
    int n=0;

    if(Real.x<m_nurbs.firstFrame().position().x)
    {
        m_nurbs.prependFrame({sideLineCount()});
        for (int k=0; k<sideLineCount(); k++)
        {
            m_nurbs.firstFrame().setCtrlPoint(k,Real.x,0.0,Real.z);
        }
        m_nurbs.firstFrame().setuPosition(m_nurbs.uAxis(), Real.x);
    }
    else if(Real.x>m_nurbs.lastFrame().position().x)
    {
        m_nurbs.appendFrame({sideLineCount()});

        for (int k=0; k<sideLineCount(); k++)
        {
            m_nurbs.lastFrame().setCtrlPoint(k,0.0,0.0,Real.z);
        }
        m_nurbs.lastFrame().setuPosition(m_nurbs.uAxis(), Real.x);
    }
    else
    {
        for (n=0; n<frameCount()-1; n++)
        {
            if(m_nurbs.frame(n).position().x<=Real.x  &&  Real.x<m_nurbs.frame(n+1).position().x)
            {
                m_nurbs.insertFrame(n+1,  Frame(sideLineCount()));
                m_xPanels.insert(m_xPanels.begin()+1,1);

                for (int k=0; k<sideLineCount(); k++)
                {
                    m_nurbs.frame(n+1).ctrlPoint(k).x = (m_nurbs.frame(n).ctrlPointAt(k).x + m_nurbs.frame(n+2).ctrlPointAt(k).x)/2.0;
                    m_nurbs.frame(n+1).ctrlPoint(k).y = (m_nurbs.frame(n).ctrlPointAt(k).y + m_nurbs.frame(n+2).ctrlPointAt(k).y)/2.0;
                    m_nurbs.frame(n+1).ctrlPoint(k).z = (m_nurbs.frame(n).ctrlPointAt(k).z + m_nurbs.frame(n+2).ctrlPointAt(k).z)/2.0;
                }
                break;
            }
        }
        if(n+1<frameCount())
        {
            m_nurbs.frame(n+1).setuPosition(m_nurbs.uAxis(), Real.x);
            double trans = Real.z - (m_nurbs.frame(n+1).ctrlPointAt(0).z + m_nurbs.frame(n+1).lastControlPoint().z)/2.0;
            for (int k=0; k<sideLineCount(); k++)
            {
                m_nurbs.frame(n+1).ctrlPoint(k).z += trans;
            }
        }
    }

    if(n+1<m_nurbs.frameCount())
        m_nurbs.setActiveFrameIndex(n+1);

    if(n>=frameCount()) m_nurbs.setActiveFrameIndex(frameCount()-1);
    if(n<=0)   m_nurbs.setActiveFrameIndex(0);
    m_iHighlightFrame = -1;

    m_xPanels.insert(m_xPanels.begin()+n, 1);

    setNURBSKnots();

    return n+1;
}


/**
 * Intersects a line segment defined by two points with a NURBS Body's surface
 * @param A the first point which defines the line
 * @param B the second point which defines the line
 * @param I the intersection point
 * @param bRight true if the intersection was found on the right side, false if found on the left side.
 * @return true if an intersection point has been found, false otherwise.
 */
bool FuseXfl::intersectNURBS(Vector3d A, Vector3d B, Vector3d &I, bool bRight)
{
    //intersect line AB with right or left body surface
    //intersection point is I
    Vector3d tmp, M0, M1, t_r, t_N;
    double u=0, v=0, dist=0, t=0, tp=0;
    int iter = 0;
    int itermax = 20;
    double dmax = 1.0e-5;
    dist = 1000.0;//m

    M0.set(0.0, A.y, A.z);
    M1.set(0.0, B.y, B.z);

    if(M0.norm()<M1.norm())
    {
        tmp = A;  A = B;  B = tmp;
    }
    //M0 is the outside Point, M1 is the inside point
    M0 = A; M1 = B;

    //define which side to intersect with
    if(M0.y>=0.0) bRight = true; else bRight = false;

    if(!isInNURBSBody(M1.x, M1.z))
    {
        //consider no intersection (not quite true in special high dihedral cases)
        I = M1;
        return false;
    }

    I = (M0+M1)/2.0; t=0.5;

    while(dist>dmax && iter<itermax)
    {
        //store the previous parameter
        tp = t;
        //first we get the u parameter corresponding to point I
        u = getu(I.x);
        //  t_Q.Set(I.x, 0.0, 0.0);
        //  t_r = (I-t_Q);
        t_r.x = 0.0;
        t_r.y = I.y;
        t_r.z = I.z;
        v = getv(u, t_r, bRight);
        t_N = Point(u, v, bRight);

        //project t_N on M0M1 line
        t = - ( (M0.x - t_N.x) * (M1.x-M0.x) + (M0.y - t_N.y) * (M1.y-M0.y) + (M0.z - t_N.z)*(M1.z-M0.z))
                /( (M1.x -  M0.x) * (M1.x-M0.x) + (M1.y -  M0.y) * (M1.y-M0.y) + (M1.z -  M0.z)*(M1.z-M0.z));

        I.x = M0.x + t * (M1.x-M0.x);
        I.y = M0.y + t * (M1.y-M0.y);
        I.z = M0.z + t * (M1.z-M0.z);

        //  dist = sqrt((t_N.x-I.x)*(t_N.x-I.x) + (t_N.y-I.y)*(t_N.y-I.y) + (t_N.z-I.z)*(t_N.z-I.z));
        dist = fabs(t-tp);
        iter++;
    }

    return dist<dmax;
}


/**
 * returns the index of the Frame  pointed by the input vector, or -10 if none
 * @param Real the input vector
 * @param ZoomFactor the view's scae factor
 * @return the Frame's index, or -10 if none found
 */
int FuseXfl::isFramePos(Vector3d const &Real, double deltaX, double deltaZ) const
{
    for (int k=0; k<frameCount(); k++)
    {
        if (fabs(Real.x-m_nurbs.frameAt(k).position().x) < deltaX &&
            fabs(Real.z-m_nurbs.frameAt(k).zPos())       < deltaZ)
            return k;
    }
    return -10;
}


/**
 * Returns the true if the input point is inside the NURBS Body, false otherwise
 * @param Pt the input point, in the x-z plane, i.e. y=0
 * @return true if the point is inside the Body, false otherwise
 */
bool FuseXfl::isInNURBSBody(double x, double z) const
{
    double u = getu(x);
    if (u <= 0.0 || u >= 1.0) return false;

    return (Point(u,1,true).z<z && z<Point(u,0,true).z);
}


/**
 * Removes a sideline from the Body
 * @param SideLine the index of the sideline to remove
 */
void FuseXfl::removeSideLine(int SideLine)
{
    if(SideLine<0||SideLine>sideLineCount()) return;
    if(sideLineCount()<=2) return;

    for (int i=0; i<m_nurbs.frameCount(); i++)
    {
        m_nurbs.frame(i).removePoint(SideLine);
    }
    setNURBSKnots();
}


/**
 * Scales either the Frame or the entire Body object
 * @param XFactor the scaling factor in the x direction
 * @param YFactor the scaling factor in the y direction
 * @param ZFactor the scaling factor in the z direction
 * @param FrameID the index of the Frame to scale
 */
void FuseXfl::scaleFrame(double YFactor, double ZFactor, int iFrame)
{
    if(iFrame<0 || iFrame>=frameCount()) return;

    for(int j=0; j<m_nurbs.frame(iFrame).nCtrlPoints(); j++)
    {
        m_nurbs.frame(iFrame).ctrlPoint(j).x  = m_nurbs.frame(iFrame).position().x;
        m_nurbs.frame(iFrame).ctrlPoint(j).y *= YFactor;
        m_nurbs.frame(iFrame).ctrlPoint(j).z *= ZFactor;
    }
}


/**
 * For a NURBS Body, sets the default position of the longitudinal parameters
 */
void FuseXfl::setPanelPos()
{
    m_XPanelPos.clear();

    for(int i=0; i<=m_nxNurbsPanels; i++)
    {
        double x = double(i)/double(m_nxNurbsPanels);
//        m_XPanelPos.append(bunchedParameter(m_nurbs.bunchDist(), m_nurbs.bunchAmplitude(), x));
        m_XPanelPos.push_back(sigmoid(-(m_nurbs.bunchAmplitude())*0.85, x));
    }
}


/**
 * Moves a Frame
 * @param Vector3d the translation vector
 * @param FrameID the index of the Frame object to be translated
 */
void FuseXfl::translateFrame(Vector3d T, int FrameID)
{
    if(FrameID>=0 && FrameID<m_nurbs.frameCount())
    {
        frame(FrameID).translate(T);
    }
}


/**
 * Overloaded function
 * Translates either a Frame or the whole Body in the xz plane
 * @param T the Vector3d which defines the translation
 * @param bFrameOnly true if only a Frame is to be translated
 * @param FrameID the index of the Frame object to be translated
 */
void FuseXfl::translate(const Vector3d &T)
{
    Fuse::translate(T);
    for (int i=0; i<frameCount(); i++)
    {
        m_nurbs.frame(i).translate(T);
    }

    makeBodyFaces();
    occ::translateShapes(m_Shape, T);
    occ::translateShapes(m_Shell, T);
    occ::translateShapes(m_RightSideShell, T);
    translateTriPanels(T.x, T.y, T.z);
}


/**
 * Scales either the Frame or the entire Body object
 * @param XFactor the scaling factor in the x direction
 * @param YFactor the scaling factor in the y direction
 * @param ZFactor the scaling factor in the z direction
 */
void FuseXfl::scale(double XFactor, double YFactor, double ZFactor)
{
    for (int i=0; i<frameCount(); i++)
    {
        Frame &fr = m_nurbs.frame(i);
        Vector3d pos = fr.position();
        pos.x *= XFactor;
        fr.setuPosition(m_nurbs.uAxis(), pos.x);

        for(int j=0; j<fr.nCtrlPoints(); j++)
        {
            Vector3d &ctrlpt = fr.ctrlPoint(j);
            ctrlpt.y *= YFactor;
            ctrlpt.z *= ZFactor;
        }
    }

    makeFuseGeometry();

    makeQuadMesh(0, Vector3d());
    std::string strange;
    makeDefaultTriMesh(strange, "");
}


double FuseXfl::framePosition(int iFrame) const
{
    if(frameCount()==0) return 0.0;
    if(iFrame<0 || iFrame>=frameCount()) return 0.0;
    return m_nurbs.frameAt(iFrame).position().x;
}


void FuseXfl::setEdgeWeight(double uw, double vw)
{
    m_nurbs.setuEdgeWeight(uw);
    m_nurbs.setvEdgeWeight(vw);
}


bool FuseXfl::serializePartXFL(QDataStream &ar, bool bIsStoring, int format)
{
    int i=0,k=0,n=0,p=0;

    double dble=0,m=0,px=0,py=0,pz=0;
    QString str;

    if(bIsStoring)
    {
    }
    else
    {
        if(format<100000 || format>200000) return false;

        ar >> str;  m_Name = str.toStdString();
        ar >> str;  m_Description = str.toStdString();
        m_theStyle.m_Color.serialize(ar, false);

        ar >> k;
        if(k==1) m_FuseType = Fuse::FlatFace;
        else     m_FuseType = Fuse::NURBS;

        ar >> k; //m_iRes
        ar >> m_nxNurbsPanels >> m_nhNurbsPanels;
        ar >> dble;   m_nurbs.setBunchAmplitude(dble);

        m_hPanels.clear();
        ar >> n;
        for(k=0; k<n; k++)
        {
            ar >> p;
            m_hPanels.push_back(p);
        }

        m_nurbs.clearFrames();
        m_xPanels.clear();
        ar >> n;
        for(k=0; k<n; k++)
        {
            m_nurbs.appendNewFrame();

            ar >> p;
            m_xPanels.push_back(p);

            ar >> dble;
            m_nurbs.frame(k).setuPosition(m_nurbs.uAxis(), dble);
            for(int ic=0; ic<m_nurbs.frame(k).nCtrlPoints(); ic++)
            {
                m_nurbs.frame(k).ctrlPoint(ic).x = dble;
            }

            m_nurbs.frame(k).serializeFrameXfl(ar, bIsStoring);
        }

        ar >> dble;
        m_Inertia.setStructuralMass(dble);

        clearPointMasses();
        ar >> k;
        for(i=0; i<k; i++)
        {
            ar >> m >> px >> py >> pz;
            ar >> str;
            m_Inertia.appendPointMass(m, Vector3d(px, py, pz), str.toStdString());
        }

        // space allocation
        ar >> k; // m_bTextures = k ? true : false;
        ar >> k; // m_bReversed = k ? true : false;

        for (int i=2; i<18; i++) ar >> k;
        ar >> k;
        k = std::min(k, frameCount()-1);
        k = std::max(k,2); k = std::min(k,5);
        m_nurbs.setuDegree(k);

        ar >> k;
        k = std::min(k, framePointCount()-1);
        k = std::max(k,2); k = std::min(k,5);
        m_nurbs.setvDegree(k);

        for (int i=0; i<50; i++) ar >> dble;

        // make the shapes, shells, and triangulation

        makeFuseGeometry();
        computeStructuralInertia(Vector3d());

        // make the triangular mesh
        std::string strange;
        makeDefaultTriMesh(strange, "");
    }
    return true;
}


/**
 * Serialize the Body data to or from a QDataStream associated to a .xfl file
 * @param ar the binary QDataStream from/to which the data shall be directed
 * @param bIsStoring true if storing, false if loading the data
 * @return true if the operation was successful, false otherwise
 */
bool FuseXfl::serializePartFl5(QDataStream &ar, bool bIsStoring)
{
    Fuse::serializePartFl5(ar, bIsStoring);

    int k(0);
    int n(0);
    int nIntSpares(0);
    int nDbleSpares(0);
    double dble(0);
    // 500001 : new fl5 format
    int ArchiveFormat = 500001;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        if     (m_FuseType==Fuse::FlatFace) ar << 1;
        else if(m_FuseType==Fuse::NURBS)    ar << 2;
        else if(m_FuseType==Fuse::Sections) ar << 3;

        // point and spline data
        m_nurbs.serializeFl5(ar, bIsStoring);

        //mesh data
        ar << m_nxNurbsPanels << m_nhNurbsPanels;
        for(k=0; k<sideLineCount(); k++) ar << m_hPanels[k];
        for(k=0; k<frameCount();    k++) ar << m_xPanels[k];

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat!=500001) return false;

        ar >> k;
        switch(k)
        {
            case 1: m_FuseType = Fuse::FlatFace;     break;
            default:
            case 2: m_FuseType = Fuse::NURBS;        break;
            case 3: m_FuseType = Fuse::Sections;     break;
        }

        // point and spline data
        m_nurbs.serializeFl5(ar, bIsStoring);

        //mesh data
        ar >> m_nxNurbsPanels >> m_nhNurbsPanels;

        m_hPanels.resize(sideLineCount());
        for(k=0; k<sideLineCount(); k++)
        {
            ar >> m_hPanels[k];
        }
        m_xPanels.resize(frameCount());
        for(k=0; k<frameCount(); k++)
        {
            ar >> m_xPanels[k];
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        // make the shapes, shells, and triangulation
        makeFuseGeometry();
        computeStructuralInertia(Vector3d());

        // make the default triangular mesh
        std::string strange;
        makeDefaultTriMesh(strange, "");
    }

    return true;
}


/**
 * Check if the volume enclosed by the two half surfaces is fully closed.
 * The conditions to be verified are
 *   - leading and trailing frames have their control points in the y=0 plane
 *   - the first and last point of each frame is in the y=0 plane.
 * @return true if it is, false if it isn't
 */
bool FuseXfl::isClosedVolume(std::string &report) const
{
    if(m_nurbs.frameCount()<=2) return false;

    bool bClosed = true;
    report.clear();

    // check leading frame
    bool bisFrameClosed = true;

    Frame const &pFirstFrame = m_nurbs.firstFrame();
    for(int ic=0; ic<pFirstFrame.nCtrlPoints(); ic++)
    {
        if(fabs(pFirstFrame.ctrlPointAt(ic).y)>PRECISION)
        {
            bisFrameClosed = false;
            break;
        }
    }
    if(!bisFrameClosed)
    {
        std::string strong = "Some control points of the leading frame are not in the plane of symetry (y=0)";
        report += strong + "\n";
        bClosed = false;
    }


    // check trailing frame
    Frame const &pLastFrame = m_nurbs.lastFrame();
    bisFrameClosed = true;
    for(int ic=0; ic<pLastFrame.nCtrlPoints(); ic++)
    {
        if(fabs(pLastFrame.ctrlPointAt(ic).y)>PRECISION)
        {
            bisFrameClosed = false;
            break;
        }
    }
    if(!bisFrameClosed)
    {
        std::string strong = "Some control points of the trailing frame are not in the plane of symetry (y=0)";
        report += strong + "\n";
        bClosed = false;
    }


    // check intermediate frames
    bisFrameClosed = true;
    int iFrame=0;
    for(iFrame=1; iFrame<m_nurbs.frameCount()-1; iFrame++)
    {
        Frame const &pFrame = m_nurbs.frameAt(iFrame);
        if(fabs(pFrame.firstControlPoint().y)>PRECISION || fabs(pFrame.lastControlPoint().y)>PRECISION)
        {
            bisFrameClosed=false;
            break;
        }
    }
    if(!bisFrameClosed)
    {
        QString strong = QString::asprintf("The end points of frame %d are not in the plane of symetry (y=0)", iFrame);
        report += strong.toStdString() + "\n";
        bClosed = false;
    }

    return bClosed;
}


int FuseXfl::quadCount() const
{
    int total =  m_nxNurbsPanels * m_nhNurbsPanels;
    return total*2;
}


/**
 * Make the triangle panels for an xfl type fuse.
 * Done by splitting each quad in two, and checking that the triangles
 * are non-denegerated before adding them to the array.
 * @todo no need to pass bodypanel3 parameter, make fuse panel array instead
 */
int FuseXfl::makeDefaultTriMesh(std::string &logmsg, const std::string &prefix)
{
    std::vector<Panel3> panel3list;

    // make left side quad panels only
    makeQuadMesh(0, Vector3d());

    m_TriMesh.clearPanels();

    // split each quad in two triangles
    int i3=0;
    for(uint i4=0; i4<m_Panel4.size(); i4++)
    {
        Panel4 const &p4 = m_Panel4.at(i4);
        if(!p4.isLeftWingPanel()) break;

        Panel3 p30(p4.m_Node[0], p4.m_Node[2], p4.m_Node[3]);
        Panel3 p31(p4.m_Node[0], p4.m_Node[1], p4.m_Node[2]);

        if(!p30.isNullTriangle())
        {
            panel3list.push_back(p30);
            panel3list.back().setIndex(i3);
            panel3list.back().setSurfacePosition(xfl::FUSESURFACE);
            i3++;
        }
        if(!p31.isNullTriangle())
        {
            panel3list.push_back(p31);
            panel3list.back().setIndex(i3);
            panel3list.back().setSurfacePosition(xfl::FUSESURFACE);
            i3++;
        }
    }

    m_TriMesh.addPanels(panel3list);

    // duplicate symmetric triangular panels
    Vector3d S[3];
    for(uint i3=0; i3<panel3list.size(); i3++)
    {
        Panel3 const &p3 = panel3list.at(i3);
        for(int in=0; in<3; in++)
        {
            S[in].x =  p3.vertexAt(in).x;
            S[in].y = -p3.vertexAt(in).y;// + 2*bodypos.y;
            S[in].z =  p3.vertexAt(in).z;
        }
        m_TriMesh.addPanel(Panel3(S[0], S[2], S[1]));
        m_TriMesh.lastPanel().setSurfacePosition(xfl::FUSESURFACE);      
    }

    m_TriMesh.makeNodeArrayFromPanels(0, logmsg, prefix);
    for(int in=0; in<m_TriMesh.nNodes(); in++) m_TriMesh.node(in).setSurfacePosition(xfl::FUSESURFACE);

    return i3;
}


/**
 * Creates the body faces
 * Used only if is of the flat face type (?)
 * @return the number of Faces which have been created and appended
 */
int FuseXfl::makeBodyFaces()
{
    double dj=0, dj1=0;
    Vector3d PLA, PTA, PLB, PTB;

    m_LeftFace.clear();
    m_RightFace.clear();

    for (int i=0; i<frameCount()-1; i++)
    {
        dj  = 0.0;
        dj1 = 1.0;

        //body left side
        for (int k=0; k<sideLineCount()-1; k++)
        {
            //build the four corner points of the strips
            PLB.x =  (1.0- dj) * framePosition(i)             +  dj * framePosition(i+1);
            PLB.y =  (1.0- dj) * frame(i).ctrlPointAt(k).y   +  dj * frame(i+1).ctrlPointAt(k).y;
            PLB.z =  (1.0- dj) * frame(i).ctrlPointAt(k).z   +  dj * frame(i+1).ctrlPointAt(k).z;

            PTB.x =  (1.0-dj1) * framePosition(i)             + dj1 * framePosition(i+1);
            PTB.y =  (1.0-dj1) * frame(i).ctrlPointAt(k).y   + dj1 * frame(i+1).ctrlPointAt(k).y;
            PTB.z =  (1.0-dj1) * frame(i).ctrlPointAt(k).z   + dj1 * frame(i+1).ctrlPointAt(k).z;

            PLA.x =  (1.0- dj) * framePosition(i)             +  dj * framePosition(i+1);
            PLA.y =  (1.0- dj) * frame(i).ctrlPointAt(k+1).y +  dj * frame(i+1).ctrlPointAt(k+1).y;
            PLA.z =  (1.0- dj) * frame(i).ctrlPointAt(k+1).z +  dj * frame(i+1).ctrlPointAt(k+1).z;

            PTA.x =  (1.0-dj1) * framePosition(i)             + dj1 * framePosition(i+1);
            PTA.y =  (1.0-dj1) * frame(i).ctrlPointAt(k+1).y + dj1 * frame(i+1).ctrlPointAt(k+1).y;
            PTA.z =  (1.0-dj1) * frame(i).ctrlPointAt(k+1).z + dj1 * frame(i+1).ctrlPointAt(k+1).z;

            m_RightFace.push_back({PLA, PTA, PTB, PLB});
            m_RightFace.back().setMeshSides(m_xPanels[i], m_hPanels[k]);

            PLA.y = -PLA.y;
            PLB.y = -PLB.y;
            PTA.y = -PTA.y;
            PTB.y = -PTB.y;
            m_LeftFace.push_back({PLA, PTA, PTB, PLB});
            m_LeftFace.back().setMeshSides(m_xPanels[i], m_hPanels[k]);
        }
    }
    return int(m_LeftFace.size() + m_RightFace.size());
}


int FuseXfl::makeSurfaceTriangulation(int axialres, int hoopres)
{
    clearTriangles();
    clearTriangleNodes();
    makeSplineTriangulation(axialres, hoopres);

    makeTriangleNodes();
    makeNodeNormals();

    return nTriangles();
}


void FuseXfl::makeSplineTriangulation(int nx, int nh)
{
    Vector3d S00, S01, S10, S11;

    // make the left side
    for (int k=0; k<nx; k++)
    {
        double uk  = double(k)   /double(nx);
        double uk1 = double(k+1) /double(nx);

        double vl=0.0;
        getPoint(uk,  vl, false, S00);
        getPoint(uk1, vl, false, S10);

        for (int l=0; l<nh; l++)
        {
            double vl1 = double(l+1) / double(nh);
            getPoint(uk,  vl1, false, S01);
            getPoint(uk1, vl1, false, S11);

            if(!S00.isSame(S01) && !S01.isSame(S11) && !S11.isSame(S00))
                m_Triangulation.appendTriangle(Triangle3d(S00, S01, S11));
            if(!S00.isSame(S11) && !S11.isSame(S10) && !S10.isSame(S00))
                m_Triangulation.appendTriangle(Triangle3d(S00, S11, S10));
            S00 = S01;
            S10 = S11;
//            vl = vl1;
        }
    }

    //make the right side
    Vector3d S0, S1, S2;
    int nLeftTriangles=nTriangles();
    for(int it=0; it<nLeftTriangles; it++)
    {
        S0 = triangle(it).vertex(0);       S0.y=-S0.y;
        S1 = triangle(it).vertex(1);       S1.y=-S1.y;
        S2 = triangle(it).vertex(2);       S2.y=-S2.y;
        m_Triangulation.appendTriangle(Triangle3d(S0, S2, S1));
    }
}


void FuseXfl::makeFlatFaceTriangulation()
{
    Vector3d S00, S01, S10, S11;

    for (int i=0; i<frameCount()-1; i++)
    {
        for (int k=0; k<sideLineCount()-1; k++)
        {
            //build the quad's four corner points
            S00.x =  framePosition(i);
            S00.y =  frame(i).ctrlPointAt(k).y;
            S00.z =  frame(i).ctrlPointAt(k).z;

            S01.x =  framePosition(i);
            S01.y =  frame(i).ctrlPointAt(k+1).y;
            S01.z =  frame(i).ctrlPointAt(k+1).z;

            S10.x =  framePosition(i+1);
            S10.y =  frame(i+1).ctrlPointAt(k).y;
            S10.z =  frame(i+1).ctrlPointAt(k).z;

            S11.x =  framePosition(i+1);
            S11.y =  frame(i+1).ctrlPointAt(k+1).y;
            S11.z =  frame(i+1).ctrlPointAt(k+1).z;

            Triangle3d t30(S00, S10, S11);
            if(!t30.isNull()) m_Triangulation.appendTriangle(t30);
            Triangle3d t31(S00, S11, S01);
            if(!t31.isNull()) m_Triangulation.appendTriangle(t31);
        }
    }
    for (int i=0; i<frameCount()-1; i++)
    {
        for (int k=0; k<sideLineCount()-1; k++)
        {
            //build the quad's four corner points
            S00.x =  framePosition(i);
            S00.y = -frame(i).ctrlPointAt(k).y;
            S00.z =  frame(i).ctrlPointAt(k).z;

            S01.x =  framePosition(i);
            S01.y = -frame(i).ctrlPointAt(k+1).y;
            S01.z =  frame(i).ctrlPointAt(k+1).z;

            S10.x =  framePosition(i+1);
            S10.y = -frame(i+1).ctrlPointAt(k).y;
            S10.z =  frame(i+1).ctrlPointAt(k).z;

            S11.x =  framePosition(i+1);
            S11.y = -frame(i+1).ctrlPointAt(k+1).y;
            S11.z =  frame(i+1).ctrlPointAt(k+1).z;

            Triangle3d t30(S00, S11, S10);
            if(!t30.isNull()) m_Triangulation.appendTriangle(t30);
            Triangle3d t31(S00, S01, S11);
            if(!t31.isNull()) m_Triangulation.appendTriangle(t31);
        }
    }
}


void FuseXfl::makeBodySplineShape(std::string &logmsg)
{
    std::string strong;

    if(frameCount()<=0) return;

    //------Build the OCC NURBS surface-----
    Handle(Geom_BSplineSurface) HRightSurface, HLeftSurface;
    occ::makeOCCNURBSFromNurbs(m_nurbs, false, HRightSurface, logmsg);
    occ::makeOCCNURBSFromNurbs(m_nurbs, true,  HLeftSurface,  logmsg);
//qDebug()<<HRightSurface.IsNull()<<HLeftSurface.IsNull();
    if(HRightSurface.IsNull()) return;
    if(HLeftSurface.IsNull()) return;

    //-----make a shell from the surface-----
    try
    {
        BRepBuilderAPI_MakeShell RightShellMaker(HRightSurface);
        if(!RightShellMaker.IsDone())
        {
            logmsg += "   Error making right side SHELL from NURBS... \n";
            return;
        }
        TopoDS_Shell RightBodyShell = RightShellMaker.Shell();
        if(RightBodyShell.IsNull()) return;

        m_RightSideShell.Append(RightBodyShell);
        m_Shape.Append(RightBodyShell);

        BRepBuilderAPI_MakeShell LeftShellMaker(HLeftSurface);

        if(!LeftShellMaker.IsDone())
        {
            logmsg += "   Error making left side SHELL from NURBS... \n";
            return;
        }
        TopoDS_Shell LeftBodyShell = LeftShellMaker.Shell();
        if(LeftBodyShell.IsNull()) return;

//        LeftBodyShell.Reverse();
        m_Shape.Append(LeftBodyShell);
        strong = "   The left NURBS has type "+ occ::shapeType(LeftBodyShell) + "\n";
        logmsg += strong;
        strong = "   The right NURBS has type "+ occ::shapeType(RightBodyShell) + "\n";
        logmsg += strong;
    }
    catch (StdFail_NotDone &e)
    {
        logmsg += std::string("   StdFail_NotDone: ") + e.GetMessageString() + EOLstr;
        return;
    }
    catch (...)
    {
        logmsg += "   Unknown failure\n";
        return;
    }
}


void FuseXfl::makeBodySplineShape_old(std::string &logmsg)
{
    std::string strong;

    if(frameCount()<=0) return;

    TColgp_Array2OfPnt RightPoles(0, frameCount()-1, 0, sideLineCount()-1);
    TColgp_Array2OfPnt LeftPoles( 0, frameCount()-1, 0, sideLineCount()-1);

    //------Store the control points in OCC format-----
    for(int iFrame=0; iFrame<frameCount(); iFrame++)
    {
        Frame const &fr = frame(iFrame);
        for(int j=0; j<fr.nCtrlPoints(); j++)
        {
            Vector3d const &ptR = fr.pointAt(j);
            RightPoles.SetValue(iFrame, j, gp_Pnt(ptR.x, ptR.y, ptR.z));

            Vector3d ptL = fr.pointAt(fr.nCtrlPoints()-j-1);
            ptL.y = -ptL.y;
            LeftPoles.SetValue( iFrame, j, gp_Pnt(ptL.x, ptL.y, ptL.z));
        }
    }

    //------OCC requires that the knots are strictly crescending, so define a minimal increment.
    double eps = 1.e-5;

    //------Make the knots-----
    int p = nurbs().uDegree();
    int uSize = int(nurbs().uKnot().size())-2*p+2;
    TColStd_Array1OfReal    uKnots(0, uSize-1);
    TColStd_Array1OfInteger uMults(0, uSize-1);
    uKnots.SetValue(0, 0.0);
    uMults.SetValue(0, p);
    uKnots.SetValue(uSize-1, 1.0);
    uMults.SetValue(uSize-1, p);
    for(uint iu=1; iu<nurbs().uKnot().size()-2*p+1; iu++)
    {
        double knot = nurbs().uKnot().at(p+iu-1);
        // occ requires that the knot values are strictly increasing
        if(fabs(knot)<PRECISION)     knot=eps;
        if(fabs(1.0-knot)<PRECISION) knot=1.0-eps;
        uKnots.SetValue(iu, knot);
        uMults.SetValue(iu, 1);
    }

    p = nurbs().vDegree();
    int vSize = int(nurbs().vKnot().size())-2*p+2;
    TColStd_Array1OfReal    vKnots(0, vSize-1);
    TColStd_Array1OfInteger vMults(0, vSize-1);
    vKnots.SetValue(0, 0.0);
    vMults.SetValue(0, p);
    vKnots.SetValue(vSize-1, 1.0);
    vMults.SetValue(vSize-1, p);
    for(uint iv=1; iv<nurbs().vKnot().size()-2*p+1; iv++)
    {
        double knot = nurbs().vKnot().at(p+iv-1);
        // occ requires that the knot values are strictly increasing
        if(fabs(knot)<PRECISION)     knot=eps;
        if(fabs(1.0-knot)<PRECISION) knot=1.0-eps;
        vKnots.SetValue(iv, knot);
        vMults.SetValue(iv, 1);
    }

    //------Build the right NURBS surface-----
    Handle(Geom_BSplineSurface) HRightSurface, HLeftSurface;
    try{
        HRightSurface = new Geom_BSplineSurface(RightPoles, uKnots, vKnots, uMults, vMults,
                                                nurbs().uDegree(), nurbs().vDegree());
        HLeftSurface = new Geom_BSplineSurface(LeftPoles, uKnots, vKnots, uMults, vMults,
                                                nurbs().uDegree(), nurbs().vDegree());
/*        HRightSurface = new Geom_BSplineSurface(RightPoles, uKnots, vKnots, uMults, vMults,
                                                1, 1);
        HLeftSurface = new Geom_BSplineSurface(LeftPoles, uKnots, vKnots, uMults, vMults,
                                                1, 1);*/
     }
    catch (Standard_ConstructionError &e)
    {
        strong = "   Spline construction error... "+std::string(e.GetMessageString()) + "\n";
        logmsg += strong;
        return;
    }
    catch(Standard_Failure &s)
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

    //-----make a shell from the surface-----

    try
    {
        BRepBuilderAPI_MakeShell RightShellMaker(HRightSurface);
        if(!RightShellMaker.IsDone())
        {
            logmsg += "   Error making right side SHELL from NURBS... \n";
            return;
        }
        TopoDS_Shell RightBodyShell = RightShellMaker.Shell();
        if(RightBodyShell.IsNull()) return;

        m_RightSideShell.Append(RightBodyShell);
        m_Shape.Append(RightBodyShell);

        BRepBuilderAPI_MakeShell LeftShellMaker(HLeftSurface);

        if(!LeftShellMaker.IsDone())
        {
            logmsg += "   Error making left side SHELL from NURBS... \n";
            return;
        }
        TopoDS_Shell LeftBodyShell = LeftShellMaker.Shell();
        if(LeftBodyShell.IsNull()) return;

//        m_LeftSideShell.Append(LeftBodyShell);
        m_Shape.Append(LeftBodyShell);
        strong = "   The left NURBS has type "+ occ::shapeType(LeftBodyShell) + "\n";
        logmsg += strong;
        strong = "   The right NURBS has type "+ occ::shapeType(RightBodyShell) + "\n";
        logmsg += strong;
    }
    catch (StdFail_NotDone &e)
    {
        logmsg += std::string("   StdFail_NotDone: ") + e.GetMessageString() + EOLstr;
        return;
    }
    catch (...)
    {
        logmsg += "   Unknown failure\n";
        return;
    }
}


void FuseXfl::makeBodyFlatPanelShape_with2Triangles(std::string &logmsg)
{
    Vector3d P1, P2, P3;
    TopTools_ListOfShape RightFaceList;
    QString tracelog;

    std::string occstr;
    for (int k=0; k<sideLineCount()-1;k++)
    {
        for (int j=0; j<frameCount()-1;j++)
        {
            for(int iTriangle=0; iTriangle<2; iTriangle++)
            {
                if(iTriangle==0)
                {
                    P1 = frame(j).ctrlPointAt(k);       P1.x = frame(j).position().x;
                    P3 = frame(j+1).ctrlPointAt(k);     P3.x = frame(j+1).position().x;
                    P2 = frame(j+1).ctrlPointAt(k+1);   P2.x = frame(j+1).position().x;
                }
                else
                {
                    P1 = frame(j).ctrlPointAt(k);       P1.x = frame(j).position().x;
                    P3 = frame(j+1).ctrlPointAt(k+1);   P3.x = frame(j+1).position().x;
                    P2 = frame(j).ctrlPointAt(k+1);     P2.x = frame(j).position().x;
                }

                TopoDS_Face face;

                occ::makeFaceFromTriangle(P1, P2, P3, face, occstr);

                if(!face.IsNull()) RightFaceList.Append(face);
            }
        }
    }

    tracelog = QString::fromStdString(occstr);

    // sew the Panels together
    BRepBuilderAPI_Sewing stitcher(0.001);
    int nFaces=0;
    for(TopTools_ListIteratorOfListOfShape FaceIt(RightFaceList); FaceIt.More(); FaceIt.Next())
    {
        stitcher.Add(FaceIt.Value());
        nFaces++;
    }

    TopoDS_Shell RightBodyShell;
    try
    {
        QString strong = QString::asprintf("   Sewing %d left faces", nFaces) + "\n";
        tracelog+= strong;
        stitcher.Perform();
        TopoDS_Shape sewedShape = stitcher.SewedShape();
        if(!sewedShape.IsNull())
        {
            strong = "   Sewed shape is a "+ QString::fromStdString(occ::shapeType(sewedShape))+"\n";
            tracelog+= strong;

            if(sewedShape.ShapeType()==TopAbs_SHELL)
            {
                RightBodyShell= TopoDS::Shell(stitcher.SewedShape()); // is a TopoDS_SHELL
                m_Shape.Append(RightBodyShell);
            }
            else
            {
                strong = QString::asprintf("   Nb of free edges=%d\n", stitcher.NbFreeEdges());
                tracelog += strong;
                strong = QString::asprintf("   Nb of contiguous edges=%d\n", stitcher.NbContigousEdges());
                tracelog += strong;
            }
        }
    }
    catch(Standard_TypeMismatch const &ex)
    {
        QString strong;
        strong = "   Left body shells not made: " + QString::fromStdString(ex.GetMessageString())+"\n";
        tracelog+= strong;
    }

    // make the symmetric shell
    if(RightBodyShell.IsNull()) return;

    RightBodyShell.Reverse();
    m_RightSideShell.Append(RightBodyShell);

    gp_Trsf trsfMirror;
    trsfMirror.SetMirror(gp_Ax2(gp_Pnt(0.0,0.0,0.0), gp_Dir(0.0,1.0,0.0), gp_Dir(0.0,0.0,1.0)));
    BRepBuilderAPI_Transform trfSym(trsfMirror);
    trfSym.Perform(RightBodyShell);

    // make the symmetric shell
    BRepBuilderAPI_Copy ShapeCopier(RightBodyShell);
    TopoDS_Shell LeftBodyShell = TopoDS::Shell(ShapeCopier.Shape());

    LeftBodyShell = TopoDS::Shell(trfSym.Shape());
    m_Shape.Append(LeftBodyShell);

    logmsg = tracelog.toStdString();
}


void FuseXfl::makeBodyFlatPanelShape_withSpline(std::string &logmsg)
{
    Vector3d P1, P2, P3, P4;
    std::vector<Vector3d> Pt;
    QString strong, tracelog;

    // define parameters
    Standard_Real Tol = 0.00001;
    Standard_Real dummy = 0.;
    Standard_Integer MaxDeg = 3;
    Standard_Integer MaxSeg = 8;
    Handle(Geom_TrimmedCurve) TC[5];
    Handle(GeomAdaptor_Curve) adapCurve[5];
    Handle(GeomFill_SimpleBound) bnd[5];
    TopoDS_Shell SideShell[2];// cannot use symmetry and reverse, need to actually build the edges in the right order


    std::string occstr;
    // side 0 is right, side 1 is left
    for(int iSide=0; iSide<2; iSide++)
    {
        TopTools_ListOfShape FaceList;
        for (int k=0; k<sideLineCount()-1; k++)
        {
            for (int j=0; j<frameCount()-1; j++)
            {
                // get the panel's four corner points
                if(iSide==0)
                {
                    // right side order
                    P1 = frame(j).ctrlPointAt(k);       P1.x = frame(j).position().x;
                    P2 = frame(j+1).ctrlPointAt(k);     P2.x = frame(j+1).position().x;
                    P3 = frame(j+1).ctrlPointAt(k+1);   P3.x = frame(j+1).position().x;
                    P4 = frame(j).ctrlPointAt(k+1);     P4.x = frame(j).position().x;
                }
                else
                {
                    // left side order
                    P1 = frame(j).ctrlPointAt(k);       P1.x = frame(j).position().x;     P1.y = -P1.y;
                    P2 = frame(j).ctrlPointAt(k+1);     P2.x = frame(j).position().x;     P2.y = -P2.y;
                    P3 = frame(j+1).ctrlPointAt(k+1);   P3.x = frame(j+1).position().x;   P3.y = -P3.y;
                    P4 = frame(j+1).ctrlPointAt(k);     P4.x = frame(j+1).position().x;   P4.y = -P4.y;
                }

                Pt.clear();
                Pt.push_back(P1);
                if(!P2.isSame(P1)) Pt.push_back(P2);
                if(!P3.isSame(P2)) Pt.push_back(P3);
                if(!P4.isSame(P3)) Pt.push_back(P4);
                if(!P1.isSame(P4)) Pt.push_back(P1);

                if(Pt.size()<=3)
                    continue;

                for(int i=0; i<4; i++) bnd[i].Nullify();
                for(uint i=0; i<Pt.size()-1; i++)
                {
                    TC[i] = GC_MakeSegment(gp_Pnt(Pt[i].x, Pt[i].y, Pt[i].z), gp_Pnt(Pt[i+1].x, Pt[i+1].y, Pt[i+1].z));
                    adapCurve[i] = new GeomAdaptor_Curve(TC[i]);
                }
                Handle(GeomFill_SimpleBound) bnd1 =  new GeomFill_SimpleBound(adapCurve[0], Tol, dummy);
                Handle(GeomFill_SimpleBound) bnd2 =  new GeomFill_SimpleBound(adapCurve[1], Tol, dummy);
                Handle(GeomFill_SimpleBound) bnd3 =  new GeomFill_SimpleBound(adapCurve[2], Tol, dummy);

                if(Pt.size()==5)
                {
                    Handle(GeomFill_SimpleBound) bnd4 =  new GeomFill_SimpleBound(adapCurve[3], Tol, dummy);
                    // define and initialize ConstrainedFilling
                    GeomFill_ConstrainedFilling aFCF(MaxDeg,MaxSeg);
                    aFCF.Init(bnd1,bnd2,bnd3,bnd4);
                    // make BSpline Surface
                    Handle(Geom_BSplineSurface) aBSplineSurf = aFCF.Surface();

                    BRepBuilderAPI_MakeShell ShellMaker(aBSplineSurf);
                    if(!ShellMaker.IsDone())
                    {
                        tracelog += "   Error making SHELL from NURBS... \n";
                        return;
                    }
                    FaceList.Append(ShellMaker.Shell());
                }
                else if(Pt.size()==4)
                {
                    //make a triangle
                    TopoDS_Face face;
                    occ::makeFaceFromTriangle(Pt.at(0), Pt.at(1), Pt.at(2), face, occstr);
                    if(face.IsNull())
                    {
                        tracelog += "   Error making face from 3 points... \n";
                        return;
                    }
                    else FaceList.Append(face);
                }
            }
        }

        tracelog += QString::fromStdString(occstr);
        occstr.clear();

        //sew the Panels together
        BRepBuilderAPI_Sewing stitcher(0.003);
        int nFaces=0;
        for(TopTools_ListIteratorOfListOfShape FaceIt(FaceList); FaceIt.More(); FaceIt.Next())
        {
            stitcher.Add(FaceIt.Value());
            nFaces++;
        }

        try
        {
            strong = QString::asprintf("   Sewing %d faces", nFaces) + "\n";
            tracelog+= strong;
            stitcher.Perform();
            TopoDS_Shape sewedShape = stitcher.SewedShape();
            if(!sewedShape.IsNull())
            {
                strong = "   Sewed shape is a "+ QString::fromStdString(occ::shapeType(sewedShape))+"\n";
                tracelog+= strong;

                if(sewedShape.ShapeType()==TopAbs_SHELL)
                {
                    SideShell[iSide] = TopoDS::Shell(stitcher.SewedShape()); // is a TopoDS_SHELL
                    m_Shape.Append(SideShell[iSide]);
                    if(iSide==0)
                        m_RightSideShell.Append(SideShell[iSide]);
                }
                else
                {
                    strong = QString::asprintf("   Nb. of free edges=%d\n", stitcher.NbFreeEdges());
                    tracelog += strong;
                    strong = QString::asprintf("   Nb. of contiguous edges=%d\n", stitcher.NbContigousEdges());
                    tracelog += strong;
                }
            }
        }
        catch(Standard_TypeMismatch const &ex)
        {
            std::string strong = "   Right side body shells not made: " + std::string(ex.GetMessageString())+"\n";
            tracelog += QString::fromStdString(strong);
        }
    }
    logmsg = tracelog.toStdString();
}


void FuseXfl::appendRightSideShell(const TopoDS_Shell &rightsideshell)
{
    // make the symetri shell
    if(rightsideshell.IsNull()) return;

    m_RightSideShell.Append(rightsideshell);
    m_Shell.Append(rightsideshell);

    // note this left shell has wrong vertice orientation
    gp_Trsf trsfMirror;
    trsfMirror.SetMirror(gp_Ax2(gp_Pnt(0.0,0.0,0.0), gp_Dir(0.0,1.0,0.0), gp_Dir(0.0,0.0,1.0)));
    BRepBuilderAPI_Transform trfSym(trsfMirror);
    trfSym.Perform(rightsideshell);

    // make the symmetric shell
    BRepBuilderAPI_Copy ShapeCopier(rightsideshell);
    TopoDS_Shell LeftBodyShell = TopoDS::Shell(ShapeCopier.Shape());

    LeftBodyShell = TopoDS::Shell(trfSym.Shape());
    m_Shell.Append(LeftBodyShell);
}



void FuseXfl::getProperties(std::string &props, const std::string &prefix, bool )
{
    Fuse::getProperties(props, prefix);

    QString strong;
    strong = QString::asprintf("Quads           = %6d", quadCount());
    props += "\n" + prefix + strong.toStdString();
}


/** make NX x NH flat panels on each side */
void FuseXfl::toFlatType(std::vector<double> const &fracpos, int nh)
{
    int nx = int(fracpos.size());
    if(nx<2) return;

    nh = std::max(nh, 2);

    NURBSSurface newnurbs;
    newnurbs.setFrameCount(nx);
    newnurbs.setFramePointCount(nh+1);

    Vector3d Pt;

    for (int i=0; i<nx; i++)
    {
        Frame &pFrame = newnurbs.frame(i);

//        double u  = double(i)/double(nx);
        double u = fracpos.at(i);
        m_nurbs.getPoint(u,  0,  Pt);

        pFrame.setXPosition(Pt.x);
        pFrame.setZPosition(0.0);

        for (int k=0; k<=nh; k++)
        {
            double v = double(k)/double(nh);
            m_nurbs.getPoint(u, v, Pt);

            pFrame.setCtrlPoint(k, Pt);
        }
    }
    m_nurbs.copy(newnurbs);
    m_FuseType = Fuse::FlatFace;

    m_xPanels.resize(nx+1);
    m_hPanels.resize(nx+1);

    std::fill(m_xPanels.begin(), m_xPanels.end(), 2);
    std::fill(m_hPanels.begin(), m_hPanels.end(), 2);

    makeFuseGeometry();

    m_TriMesh.clearMesh();
}




