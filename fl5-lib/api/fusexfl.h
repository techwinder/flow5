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


#include <nurbssurface.h>
#include <enums_objects.h>
#include <fuse.h>
#include <quadmesh.h>
#include <trimesh.h>
#include <quad3d.h>

#include <TopoDS_Face.hxx>

#define NHOOPPOINTS 67  //used for display and to export the geometry
#define NXPOINTS 97     //used for display and to export the geometry

class Panel3;
class Panel4;
class PointMass;
class Quad3d;
class Surface;

/**
 * This class FL5LIB_EXPORT :
 *	 - defines the body object,
 * 	 - provides the methods for the calculation of the plane's geometric properties,
 *   - porvides methods for the panel calculations.
 * The data is stored in International Standard Units, i.e. meters, kg, and seconds.
 * Angular data is stored in degrees.
 */
class FL5LIB_EXPORT FuseXfl : public Fuse
{
    friend class  PlaneXflDlg;
    friend class  FuseXflDlg;
    friend class  FuseXflDefDlg;
    friend class  EditFuseXflDlg;
    friend class  FuseMesherWt;
    friend class  FuseMesherDlg;

    public:
        FuseXfl(Fuse::enumType fusetype = Fuse::NURBS);
        FuseXfl(FuseXfl const &aFuseXfl);

        PART::enumPartType partType() const override {return PART::Fuse;}
        bool isWing() const override {return false;}
        bool isFuse() const override {return true;}
        bool isSail() const override {return false;}
        bool isXflType()       const override {return true;}
        bool isFlatFaceType()  const override {return false;}  // unless overriden
        bool isSplineType()    const override {return false;}  // unless overriden
        bool isSectionType()   const override {return false;} // unless overriden
        bool isOccType()       const override {return false;}
        bool isStlType()       const override {return false;}

        virtual void scale(double XFactor, double YFactor, double ZFactor) override;
        virtual void scaleFrame(double YFactor, double ZFactor, int FrameID=0);
        virtual void translateFrame(Vector3d T, int FrameID);
        virtual void translate(Vector3d const &T) override;
        virtual void makeFuseGeometry() override;
        virtual bool serializePartFl5(QDataStream &ar, bool bIsStoring) override;
        virtual void makeNURBS() {return;}

        bool isClosedVolume(std::string &report) const;

        bool isInNURBSBody(double x, double z) const;
        bool intersectNURBS(Vector3d A, Vector3d B, Vector3d &I, bool bRight);

        virtual double length() const override;
        virtual double height() const;

        double getu(double x) const;
        double getv(double u, Vector3d r, bool bRight) const;
        double getSectionArcLength(double x) const;

        Vector3d centerPoint(double u);
        Vector3d leadingPoint();

        void getProperties(std::string &props, const std::string &prefix, bool bFull=false) override;

        void duplicateFuse(const Fuse &aFuse) override;
        void duplicateFuseXfl(const FuseXfl &aFuseXfl);

        virtual void makeDefaultFuse();
        void makeDefaultHull();
        virtual int makeShape(std::string &log);
        void makeBodySplineShape(std::string &logmsg);
        void makeBodySplineShape_old(std::string &logmsg);
        void makeBodyFlatPanelShape_with2Triangles(std::string &logmsg);
        void makeBodyFlatPanelShape_withSpline(std::string &tracelog);


        int makeDefaultTriMesh(std::string &logmsg, const std::string &prefix) override;

        void clearRightShells() {m_RightSideShell.Clear();}
        void appendRightSideShell(TopoDS_Shell const &rightsideshell);

        void getPoint(double u, double v, bool bRight, Vector3d &Pt) const;
        Vector3d Point(double u, double v, bool bRight) const;
        virtual void removeSideLine(int SideLine);


        void setNURBSKnots() {m_nurbs.setKnots();}
        void setPanelPos();
        void setEdgeWeight(double uw, double vw);

        virtual int insertFrame(const Vector3d &Real);
        virtual int insertFrameBefore(int iFrame);
        virtual int insertFrameAfter(int iFrame);
        virtual int removeFrame(int n) {return m_nurbs.removeFrame(n);}

        int isFramePos(Vector3d const &Real, double deltaX, double deltaZ) const;


        int activeFrameIndex() const  {return m_nurbs.activeFrameIndex();}
        Frame &activeFrame() {return m_nurbs.activeFrame();}
        Frame const &activeFrame() const {return m_nurbs.activeFrame();}
        Frame &frame(int iFrame) {return m_nurbs.frame(iFrame);}
        Frame const &frameAt(int iFrame) const {return m_nurbs.frameAt(iFrame);}

        void setActiveFrameIndex(int iFrame) {m_nurbs.setActiveFrameIndex(iFrame);}

        double framePosition(int iFrame) const;

        int frameCount()      const {return m_nurbs.frameCount();}
        int framePointCount() const {return m_nurbs.framePointCount();}
        int sideLineCount()   const {return m_nurbs.framePointCount();}// same as FramePointCount();

        virtual void insertPoint(int iPt);
        virtual void insertPoint(Vector3d const &Real);

        int nPanel4() const override {return int(m_Panel4.size());}
        std::vector<Panel4> const &quadPanels() const {return m_Panel4;}
        Panel4 const &panel4(int idx) const {return m_Panel4.at(idx);}

        void computeSurfaceProperties(std::string &msg, std::string const &prefx) override;


        bool serializePartXFL(QDataStream &ar, bool bIsStoring, int format);

        const NURBSSurface &nurbs() const {return m_nurbs;}
        NURBSSurface& nurbs() {return m_nurbs;}
        //	NURBSSurface *splineSurface() {return &m_SplineSurface;}

        virtual int quadCount() const;

        virtual int makeQuadMesh(int idx0, Vector3d const &pos) = 0;

        int makeBodyFaces();

        virtual int makeSurfaceTriangulation(int axialres, int hoopres);
        void makeSplineTriangulation(int nx, int nh);
        void makeFlatFaceTriangulation();

        void setHighlighted(int iHigh) {m_iHighlightFrame=iHigh;}
        int highlightedFrame() const {return m_iHighlightFrame;}

        void toFlatType(const std::vector<double> &fracpos, int nh);
        TopoDS_ListOfShape const& rightSideShells() const {return m_RightSideShell;}
        TopoDS_ListOfShape &rightSideShells() {return m_RightSideShell;}

        // for NURBS type fuses
        int nxNurbsPanels() const {return m_nxNurbsPanels;}
        int nhNurbsPanels() const {return m_nhNurbsPanels;}
        void setNxNurbsPanels(int nx) {m_nxNurbsPanels=nx;}
        void setNhNurbsPanels(int nh) {m_nhNurbsPanels=nh;}

        // for QUAD face fuses
        std::vector<int> const &xPanels() const {return m_xPanels;}
        std::vector<int> const &hPanels() const {return m_hPanels;}
        int nxPanels() const {return int(m_xPanels.size());}
        int nhPanels() const {return int(m_hPanels.size());}
        void resizeHPanels(int n) {m_hPanels.resize(n);}
        void resizeXPanels(int n) {m_xPanels.resize(n);}
        void clearXPanels() {m_xPanels.clear();}
        void clearHPanels() {m_hPanels.clear();}
        void setXPanels(std::vector<int> const &xpans) {m_xPanels=xpans;}
        void setHPanels(std::vector<int> const &hpans) {m_hPanels=hpans;}
        void setXPanels(int idx, int n) {if (idx>=0 && idx<nxPanels()) m_xPanels[idx]=n; }
        void setHPanels(int idx, int n) {if (idx>=0 && idx<nhPanels()) m_hPanels[idx]=n; }
        int xPanels(int idx) const {if (idx>=0 && idx<nxPanels()) return m_xPanels.at(idx); else return 1;}
        int hPanels(int idx) const {if (idx>=0 && idx<nhPanels()) return m_hPanels.at(idx); else return 1;}
        void appendXPanel(int n) {m_xPanels.push_back(n);}
        void appendHPanel(int n) {m_hPanels.push_back(n);}

        //____________________VARIABLES_____________________________________________


    protected:

        TopoDS_ListOfShape m_RightSideShell;  /**< Only the right side shells. Used to reduce meshing times, by making left side triangles by symmetry */
        std::vector<Quad3d> m_LeftFace, m_RightFace;  /**< the body's flat faces if of the flat face type */

        std::vector<Panel4> m_Panel4;
        std::vector<int> m_xPanels;              /**< the number of mesh panels between two frames */
        std::vector<int> m_hPanels;              /**< the number of mesh panels in the hoop direction between two sidelines */
        std::vector<double> m_XPanelPos;
        NURBSSurface m_nurbs;             /**< the spline surface which defines the left (port) side of the body */

        int m_iHighlightFrame;                    /**< the currently selected Frame to highlight */

        int m_nxNurbsPanels;                           /**< For a NURBS body, the number of mesh elements in the direction of the x-axis */
        int m_nhNurbsPanels;                           /**< For a NURBS body, the number of mesh elements in the hoop direction */
};




