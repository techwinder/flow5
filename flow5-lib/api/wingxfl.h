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



#include <aeroforces.h>
#include <enums_objects.h>
#include <foil.h>
#include <panel3.h>
#include <panel4.h>
#include <part.h>
#include <spandistribs.h>
#include <surface.h>
#include <triangulation.h>
#include <vector3d.h>
#include <wingsection.h>


class PlanePolar;
class PlaneOpp;
class Panel3;
class Panel4;
class PointMass;
class Triangle3d;
class Segment3d;

class FL5LIB_EXPORT WingXfl : public Part
{

    friend class  LLTTask;
    friend class  PlaneXfl;
    friend class  PlaneXflDlg;
    friend struct SpanDistribs;
    friend class  Surface;
    friend class  WingDlg;
    friend class  XMLPlaneReader;

    public:

        WingXfl(xfl::enumType type=xfl::Main);
//        WingXfl(WingXfl const &aWing);

        ~WingXfl();

        PART::enumPartType partType() const override {return PART::Wing;}
        bool isWing() const override {return true;}
        bool isFuse() const override {return false;}
        bool isSail() const override {return false;}


        bool isMainWing()    const  {return m_WingType==xfl::Main;}
        bool isElevator()    const  {return m_WingType==xfl::Elevator;}
        bool isFin()         const  {return m_WingType==xfl::Fin;}
        bool isOtherWing()   const  {return m_WingType==xfl::OtherWing;}

        xfl::enumType wingType() const {return m_WingType;}
        void setWingType(xfl::enumType type) {m_WingType = type;}

        double planformSpan()  const {return m_PlanformSpan;}
        double projectedSpan() const {return m_ProjectedSpan;}
        double planformArea()  const {return m_PlanformArea;}
        double projectedArea() const {return m_ProjectedArea;}

        double MAC() const {return m_MAChord;}
        double aspectRatio() const {return m_PlanformSpan*m_PlanformSpan/m_PlanformArea;}

        double GChord() const {return m_PlanformArea/m_PlanformSpan;}

        int nPanel3() const override {return m_nPanel3;}
        int nPanel4() const override {return m_nPanel4;}
        void setNPanel3(int n) {m_nPanel3 = n;}
        void setNPanel4(int n) {m_nPanel4 = n;}

        int nNodes() const {return m_nNodes;}

        std::vector<Segment3d> const &outline() const {return m_Outline;}

        bool isWingXfl() const {return true;}

        void makeDefaultWing();
        void makeDefaultStab();
        void makeDefaultFin();

        void createSurfaces(Vector3d const &T, double XTilt, double YTilt);//generic surface, LLT, VLM or Panel
        void createXPoints();

        int  quadTotal(bool bThinSurface) const;

        void panel4ComputeStrips(const std::vector<Panel4> &panel4list,
                                 const PlanePolar *pWPolar, const Vector3d &CoG, double alpha, double beta, double QInf, const double *Cp, const double *Gamma, SpanDistribs &SpanResSum);
        void panel3ComputeStrips(const std::vector<Panel3> &panel3list,  PlanePolar const *pWPolar, const Vector3d &CoG, double alpha, double beta, double QInf, const double *Cp3Vtx, SpanDistribs &SpanResSum);

        void panel4ComputeInviscidForces(std::vector<Panel4> const &panel4list, const PlanePolar *pWPolar, const Vector3d &cog, double alpha, double beta, double QInf, double *Cp4, const double *Gamma, AeroForces &AF) const;
        void panel3ComputeInviscidForces(std::vector<Panel3> const &panel3list, const PlanePolar *pWPolar, const Vector3d &cog, double alpha, double beta, const double *Cp3Vtx, AeroForces &AF) const;
        void computeViscousForces(const PlanePolar *pWPolar, double alpha, double beta, SpanDistribs &SpanResFF, AeroForces &AF) const;

        void panelComputeBending(const std::vector<Panel4> &panel4list, bool bThinSurface, SpanDistribs &SpanResFF);
        void panelComputeBending(const std::vector<Panel3> &panel3list, bool bThinSurface, SpanDistribs &SpanResFF);

        bool isWingPanel4(int nPanel) const;

        void getFoils(Foil **pFoil0, Foil **pFoil1, double y, double &t);
        void duplicate(const WingXfl *pWing);
        void duplicate(WingXfl const &aWing);

        void computeStations();
        void computeGeometry();
        void computeStructuralInertia(Vector3d const &PartPosition) override;
        bool intersectWing(const Vector3d &O, const Vector3d &U, Vector3d &I, int &idxSurf, double &dist, bool bDirOnly) const;

        int makeTriPanels(int ip3start, int indStart, bool bThickSurfaces);

        void scaleSweep(double NewSweep);
        void scaleTwist(double NewTwist);
        void scaleSpan(double NewSpan);
        void scaleChord(double NewChord);
        void scaleArea(double newArea);
        void scaleAR(double newAR);
        void scaleTR(double newTR);

        void surfacePoint(double xRel, double ypos, xfl::enumSurfacePosition pos, Vector3d &Point, Vector3d &PtNormal) const;

        bool isTwoSided()  const  {return m_bTwoSided;}
        void setTwoSided(bool bTwoSided) {m_bTwoSided = bTwoSided;}

        bool isClosedInnerSide() const {return m_bCloseInnerSide;}
        void setClosedInnerSide(bool bClosed) {m_bCloseInnerSide = bClosed;}


        int nSections() const {return int(m_Section.size());}
        std::vector<WingSection> const &sections() const {return m_Section;}
        void clearWingSections() {m_Section.clear();}
        void insertSection(int iSection);
        bool appendWingSection();
        bool appendWingSection(WingSection const &ws);
        bool appendWingSection(double Chord, double Twist, double Pos, double Dihedral, double Offset, int nXPanels, int nYPanels,
                               xfl::enumDistribution XPanelDist, xfl::enumDistribution YPanelDist, const std::string &RightFoilName, const std::string &LeftFoilName);
        void removeWingSection(int const iSection);
        WingSection const &section(int iSec) const {return m_Section.at(iSec);}
        WingSection &section(int iSec) {return m_Section[iSec];}
        WingSection &rootSection() {return m_Section.front();}
        WingSection &tipSection()  {return m_Section.back();}
        WingSection const &rootSection() const {return m_Section.front();}
        WingSection const &tipSection()  const {return m_Section.back();}
        WingSection *pSection(int iSec);

        void clearSurfaces() {m_Surface.clear();}


        std::vector<Panel3> const &panels() const {return m_TriMesh.panels();}
        std::vector<Node> const &nodes() const {return m_TriMesh.nodes();}

        int nXPanels() const {return m_Section.front().m_NXPanels;} /** Returns the number of chordwise quad panels at the first span section */
        int nXPanel3() const {return m_Section.front().m_NXPanels *2;}/** Returns the number of chordwise triangular panels at the first span section */

        void setNXPanels(int iSection, int nx) {if(iSection>=0 && iSection<nSections()) m_Section[iSection].m_NXPanels = std::max(1,nx);}
        void setNYPanels(int iSection, int ny) {if(iSection>=0 && iSection<nSections()) m_Section[iSection].m_NYPanels = std::max(1,ny);}
        int nXPanels(int iSection) const {if(iSection>=0 && iSection<nSections()) return m_Section.at(iSection).m_NXPanels; else return 0;}
        int nYPanels(int iSection) const {if(iSection>=0 && iSection<nSections()) return m_Section.at(iSection).m_NYPanels; else return 0;}

        int nYPanels() const;

        bool hasPanel3(int idx3) const;
        bool hasPanel4(int idx4) const;

        void setXPanelDist(int iSection, xfl::enumDistribution xdistrib){if(iSection>=0 && iSection<nSections()) m_Section[iSection].m_XPanelDist = xdistrib;}
        void setYPanelDist(int iSection, xfl::enumDistribution ydistrib){if(iSection>=0 && iSection<nSections()) m_Section[iSection].m_YPanelDist = ydistrib;}
        xfl::enumDistribution xPanelDist(int iSection) const {if(iSection>=0 && iSection<nSections()) return m_Section.at(iSection).m_XPanelDist; else return xfl::UNIFORM;}
        xfl::enumDistribution yPanelDist(int iSection) const {if(iSection>=0 && iSection<nSections()) return m_Section.at(iSection).m_YPanelDist; else return xfl::UNIFORM;}

        bool isWingFoil(const Foil *pFoil) const;


        double rootChord()     const  {if(nSections()>0) return m_Section.front().m_Chord;    else return 0.0;}
        double rootOffset()    const  {if(nSections()>0) return m_Section.front().m_Offset;   else return 0.0;}
        double tipChord()      const  {if(nSections()>0) return m_Section.back().m_Chord;     else return 0.0;}
        double tipTwist()      const  {if(nSections()>0) return m_Section.back().m_Twist;     else return 0.0;}
        double tipOffset()     const  {if(nSections()>0) return m_Section.back().m_Offset;    else return 0.0;}
        double tipPos()        const  {if(nSections()>0) return m_Section.back().m_YPosition; else return 0.0;}

        int nStations() const {return m_NStation;}

        void setChord(int iSection, double chord) {if(iSection>=0 && iSection<nSections()) m_Section[iSection].m_Chord=chord;}
        double chord(int iSection) const {if(iSection>=0 && iSection<nSections()) return m_Section.at(iSection).m_Chord; else return 0.0;}

        void setOffset(int iSection, double offset) {if(iSection>=0 && iSection<nSections()) m_Section[iSection].m_Offset =offset;}
        double offset(int iSection) const {if(iSection>=0 && iSection<nSections()) return m_Section.at(iSection).m_Offset; else return 0.0;}

        void setSectionLength(int iSection, double length) {if(iSection>=0 && iSection<nSections()) m_Section[iSection].m_Length=length;}
        double sectionLength(int iSection) const {if(iSection>=0 && iSection<nSections()) return m_Section.at(iSection).m_Length; else return 0.0;}

        void setTwist(int iSection, double twist) {if(iSection>=0 && iSection<nSections()) m_Section[iSection].m_Twist = twist;}
        double twist(int iSection) const {if(iSection>=0 && iSection<nSections()) return m_Section.at(iSection).m_Twist; else return 0.0;}
        double twist() const {return m_Section.back().twist();}

        void setYPosition(int iSection, double yPos) {if(iSection>=0 && iSection<nSections()) m_Section[iSection].m_YPosition = yPos;}
        double yPosition(int iSection) const {if(iSection>=0 && iSection<nSections()) return m_Section.at(iSection).m_YPosition; else return 0.0;}

        void setDihedral(int iSection, double dih) {if(iSection>=0 && iSection<nSections()) m_Section[iSection].m_Dihedral = dih;}
        double dihedral(int iSection) const {if(iSection>=0 && iSection<nSections()) return m_Section.at(iSection).m_Dihedral; else return 0.0;}

        void setYProj(int iSection, double yproj) {if(iSection>=0 && iSection<nSections()) m_Section[iSection].m_YProj = yproj;}
        double yProj(int iSection) const  {if(iSection>=0 && iSection<nSections()) return m_Section.at(iSection).m_YProj; else return 0.0;}

        void setZPosition(int iSection, double z) {if(iSection>=0 && iSection<nSections()) m_Section[iSection].m_ZPos = z;}
        double zPosition(int iSection) const {if(iSection>=0 && iSection<nSections()) return m_Section.at(iSection).m_ZPos; else return 0.0;}

        //LLT specific methods

        double ySectionRel(double SpanPos) const;
        double getChord(double yob) const;
        double getOffset(double yob) const;
        double getDihedral(double yob) const;
        double getTwist(double y) const;
        double averageSweep() const;

        double xLE(double SpanPos) const;
        double xTE(double SpanPos) const;

        double C4(double yob) const;
        double ZPosition(double y) const;

        double length() const override {return m_ProjectedSpan;}

        bool isSymmetric() const {return m_bSymmetric;}
        void setsymmetric(bool bsymmetric){m_bSymmetric = bsymmetric;}

        int nFlaps() const {return m_nFlaps;}

        void setRightFoilName(int iSection, std::string const &foilname){if(iSection>=0 && iSection<nSections()) m_Section[iSection].m_RightFoilName = foilname;}
        void setLeftFoilName(int iSection, std::string const &foilname) {if(iSection>=0 && iSection<nSections()) m_Section[iSection].m_LeftFoilName  = foilname;}
        std::string rightFoilName(int iSection) const {if(iSection>=0 && iSection<nSections()) return m_Section.at(iSection).m_RightFoilName; else return "";}
        std::string leftFoilName(int iSection)  const {if(iSection>=0 && iSection<nSections()) return m_Section.at(iSection).m_LeftFoilName;  else return "";}

        bool serializePartXFL(QDataStream &ar, bool bIsStoring);
        bool serializePartFl5(QDataStream &ar, bool bIsStoring) override;

        double integralC2(double y1, double y2, double c1, double c2) const;

        int uniformizeXPanelNumber();

        Surface &surface(int iSurface) {return m_Surface[iSurface];}
        Surface const &surfaceAt(int iSurface) const {return m_Surface.at(iSurface);}
        int nSurfaces() const {return int(m_Surface.size());}
        Surface const &firstSurface() const {return m_Surface.front();}
        Surface const &lastSurface() const  {return m_Surface.back();}
        Surface const &rootLeftSurface() const;
        Surface const &rootRightSurface() const;

        double stripArea(int p) const {return m_StripArea.at(p);}

        double taperRatio()  const;


        bool checkFoils(std::string &log) const;

        int nTriangles() const;
        void exportToAVL(std::string &avlstring, int index, const Vector3d &T, double ry, double lengthunit) const;

        void resizeSpanDistribs(int nStations=-1);

        void getProperties(std::string &properties, const std::string &prefx) const;

        double flapMomentAt(int iFlap) const {return m_FlapMoment.at(iFlap);}

        void makeTriangulation(const Fuse *pFuse, int CHORDPANELS);

        int nTipStrips() const {return m_nTipStrips;}
        void setNTipStrips(int nstrips) {m_nTipStrips=nstrips;}

        bool connectInnerSurfaces(std::vector<Panel3> &panels, bool bThickSurfaces);
        bool connectSurfaceToNext(int iSurf, std::vector<Panel3> &panels, bool bConnectFlaps, bool bThickSurfaces);

        bool hasCenterGap() const;

        void makeMidWires(std::vector<std::vector<Node> > &midwires) const;
        void makeTopBotWires(std::vector<std::vector<Node> > &midwires) const;

        static double minSurfaceLength() {return s_MinSurfaceLength;}
        static void setMinSurfaceLength(double size) {s_MinSurfaceLength=size;}


    private:
        xfl::enumType m_WingType;  /** Defines the type of wing on the plane : main, second, elevator, fin, other */

        double m_MAChord;                      /**< the wing's mean aerodynamic chord */
        double m_PlanformSpan;                 /**< the planform span, i.e. if the dihedral was 0 at each junction */
        double m_ProjectedSpan;                /**< the span projected on the xy plane defined by z=0 */
        double m_PlanformArea;                 /**< the planform wing area, i.e. if the dihedral was 0 at each junction */
        double m_ProjectedArea;                /**< the wing area projected on the xy plane defined by z=0; */

        int m_nPanel3;                         /**< the number of triangular panels on this Wing; depends on whether the polar is a thin or thicksurface type */
        int m_nPanel4;                         /**< the number of quad panels on this Wing; depends on whether the polar is a thin or thicksurface type */
        int m_nNodes;                          /**< the number of nodes on this Wing; depends on whether the polar is a thin or thicksurface type */

        std::vector<Segment3d> m_Outline;   /** The outline segments  used for displays - stored to save redraw times */


        bool m_bSymmetric;	             /**< true if the wing's geometry is symmetric */
        bool m_bTwoSided;               /**< true if the wing describes a double fin symmetric about the y=0 plane */
        bool m_bCloseInnerSide;
        int m_NStation;                  /**< the number of stations for wing calculation; either the number of points of LLT, or the number of spanwise panels  */

        int m_nTipStrips;                /**< the number of horizontal panel strips in the left and right tip patches; introduced in v7.01 beta 09 */

        int m_nXFlapPanels;              /**< teh number of panels on the flaps in the chordwise direction; unused as of v7.55; provision for future mod. */

        int m_nFlaps;                    /**< the number of T.E. flaps, numbered from left wing to right wing; for a main wing this number is even*/
        std::vector<double> m_FlapMoment;      /**< the flap moments resulting from the panel of VLM analysis */

        std::vector<double> m_StripArea;
        std::vector<double> m_StripPos;
        std::vector<double> m_Chord;
        std::vector<double> m_Offset;
        std::vector<double> m_Twist;
        std::vector<Vector3d> m_PtC4;

    private:
        std::vector<WingSection> m_Section;         /**< the array of wing sections. A WingSection extends between a foil and the next. */
        std::vector<Surface> m_Surface;             /**< the array of Surface objects associated to the wing */
        std::vector<int> m_StripStartNodes;         /**< the indexes of the nodes at the start of each wingsection */

        static double s_MinSurfaceLength;       /**< wing minimum panel size ; panels of less length are ignored */
};

