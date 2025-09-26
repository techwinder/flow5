/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once


#include <QVector>
#include <QTextStream>



#include <xflcore/enums_objects.h>
#include <xflfoil/objects2d/foil.h>
#include <xflgeom/geom3d/triangulation.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflobjects/objects3d/analysis/spandistribs.h>
#include <xflobjects/objects3d/part.h>
#include <xflobjects/objects3d/wing/surface.h>
#include <xflobjects/objects3d/wing/wingsection.h>
#include <xflpanels/panels/panel3.h>
#include <xflpanels/panels/panel4.h>

/**
 * @class Wing
 * @brief  This class defines the wing object, provides the methods for the calculation of the wing geometric properties, and
           provides methods for LLT, VLM and Panel methods.
 *
 * A wing is identified and reference by its name, which is used  by the parent Plane objects and by child Polar and WingOPpp objects.
 *
 * The wing is defined by geometric data at a finite number of span-stations : chord, chord position, foil, dihedral and washout.
 *
 * The panel between two span-stations is a WingSection object. The term "panel" is reserved for the mesh panels used to perform a VLM or Panel analysis.
 *
 * The term stations is reserved for the spanwise points used by the LLT, or by the spanwise position of the panels in a VLM or panel analysis.
 * During an LLT or a Panel analysis, the value of the aerodynamic parameters of interest are stored in the member variables of this Wing object.
 * The data is retrieved and stored in an operating point at the end of the analysis.
 * The choice to include the wing-specific methods used in the analysis in this wing class is arbitrary.
 * It simplifies somewhat the implementation of the PanelAnalysis classes, but makes this Wing class more complex.
 *
 * The data is stored in International Standard Units, i.e. meters, kg, and seconds. Angular data is stored in degrees.
*/



class WPolar;
class PlaneOpp;
class Panel3;
class Panel4;
class PointMass;
class Triangle3d;
class Segment3d;

class WingXfl : public Part
{

    friend class LLTTask;
    friend class PlaneXfl;
    friend class PlaneXflDlg;
    friend struct SpanDistribs;
    friend class Surface;
    friend class WingDlg;
    friend class XMLPlaneReader;

    public:
        enum enumType {Main, Elevator, Fin, OtherWing};

    public:

        WingXfl(WingXfl::enumType type=WingXfl::Main);
//        WingXfl(WingXfl const &aWing);

        ~WingXfl();

        PART::enumPartType partType() const override {return PART::Wing;}
        bool isWing() const override {return true;}
        bool isFuse() const override {return false;}
        bool isSail() const override {return false;}


        bool isMainWing()    const  {return m_WingType==WingXfl::Main;}
        bool isElevator()    const  {return m_WingType==WingXfl::Elevator;}
        bool isFin()         const  {return m_WingType==WingXfl::Fin;}
        bool isOtherWing()   const  {return m_WingType==WingXfl::OtherWing;}

        WingXfl::enumType type() const {return m_WingType;}
        void setWingType(WingXfl::enumType type) {m_WingType = type;}

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

        Triangulation const &triangulation() const {return m_Triangulation;}
        QVector<Segment3d> const &outline() const {return m_Outline;}


        void clearTriangles() {m_Triangulation.clear();}
        void appendTriangle(Triangle3d const & t3) {m_Triangulation.appendTriangle(t3);}
        Triangle3d const & triangleAt(int index) const {return m_Triangulation.triangleAt(index);}
        Triangle3d & triangle(int index) {return m_Triangulation.triangle(index);}
        QVector<Triangle3d> const & triangles() const {return m_Triangulation.triangles();}

        void translateTriangles(const Vector3d &T) {m_Triangulation.translate(T);}
        int makeTriangleNodes() {return m_Triangulation.makeNodes();}
        void makeNodeNormals(bool bReversed=false) {m_Triangulation.makeNodeNormals(bReversed);}

        int triangleNodeCount() const {return m_Triangulation.nodeCount();}
        void clearTriangleNodes() {m_Triangulation.clearNodes();}
        QVector<Node> const &triangleNodes() {return m_Triangulation.nodes();}
        void appendTriangleNodes(QVector<Node>const & nodelist) {m_Triangulation.appendNodes(nodelist);}
        void setTriangleNodes(QVector<Node> const &nodes) {m_Triangulation.setNodes(nodes);}

        Node const & triangleNode(int index) {return m_Triangulation.node(index);}


        static QString type(WingXfl::enumType wingType);
        static WingXfl::enumType type(QString const &strWingType);


        bool isWingXfl() const {return true;}

        void makeDefaultWing();
        void makeDefaultStab();
        void makeDefaultFin();

        void createSurfaces(Vector3d const &T, double XTilt, double YTilt);//generic surface, LLT, VLM or Panel
        void createXPoints();

        int  quadTotal(bool bThinSurface) const;

        void panel4ComputeStrips(const QVector<Panel4> &panel4list,
                                 const WPolar *pWPolar, const Vector3d &CoG, double alpha, double beta, double QInf, const double *Cp, const double *Gamma, SpanDistribs &SpanResSum);
        void panel3ComputeStrips(const QVector<Panel3> &panel3list,  WPolar const *pWPolar, const Vector3d &CoG, double alpha, double beta, double QInf, const double *Cp3Vtx, SpanDistribs &SpanResSum);

        void panel4ComputeInviscidForces(QVector<Panel4> const &panel4list, const WPolar *pWPolar, const Vector3d &cog, double alpha, double beta, double QInf, double *Cp4, const double *Gamma, AeroForces &AF) const;
        void panel3ComputeInviscidForces(QVector<Panel3> const &panel3list, const WPolar *pWPolar, const Vector3d &cog, double alpha, double beta, const double *Cp3Vtx, AeroForces &AF) const;
        void computeViscousForces(const WPolar *pWPolar, double alpha, double beta, SpanDistribs &SpanResFF, AeroForces &AF) const;

        void panelComputeBending(const QVector<Panel4> &panel4list, bool bThinSurface, SpanDistribs &SpanResFF);
        void panelComputeBending(const QVector<Panel3> &panel3list, bool bThinSurface, SpanDistribs &SpanResFF);

        bool isWingPanel4(int nPanel) const;
        bool isWingNode(QVector<Panel4> const &panel4list, int nNode) const;

        void getFoils(Foil **pFoil0, Foil **pFoil1, double y, double &t);
        void duplicate(const WingXfl *pWing);
        void duplicate(WingXfl const &aWing);

        void computeStations();
        void computeGeometry();
        void computeStructuralInertia(Vector3d const &PartPosition) override;
        bool intersectWing(const Vector3d &O, const Vector3d &U, Vector3d &I, int &idxSurf, double &dist, bool bDirOnly) const;

        int makeTriPanels(QVector<Panel3> &panel3list, QVector<Node> &nodelist, bool bThickSurfaces);

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


        void clearWingSections() {m_Section.clear();}
        void insertSection(int iSection);
        bool appendWingSection();
        bool appendWingSection(WingSection const &ws);        bool appendWingSection(double Chord, double Twist, double Pos, double Dihedral, double Offset, int nXPanels, int nYPanels,
                               xfl::enumDistribution XPanelDist, xfl::enumDistribution YPanelDist, QString RightFoilName, QString LeftFoilName);
        void removeWingSection(int const iSection);

        void clearSurfaces() {m_Surface.clear();}

        //access methods
        int nSections() const {return m_Section.count();}
        int sectionCount() const {return m_Section.count();}
        int nXPanels() const {return m_Section.first().m_NXPanels;} /** Returns the number of chordwise quad panels at the first span section */
        int nXPanel3() const {return m_Section.first().m_NXPanels *2;}/** Returns the number of chordwise triangular panels at the first span section */

        void setNXPanels(int iSection, int nx) {if(iSection>=0 && iSection<sectionCount()) m_Section[iSection].m_NXPanels = std::max(1,nx);}
        void setNYPanels(int iSection, int ny) {if(iSection>=0 && iSection<sectionCount()) m_Section[iSection].m_NYPanels = std::max(1,ny);}
        int nXPanels(int iSection) const {if(iSection>=0 && iSection<sectionCount()) return m_Section.at(iSection).m_NXPanels; else return 0;}
        int nYPanels(int iSection) const {if(iSection>=0 && iSection<sectionCount()) return m_Section.at(iSection).m_NYPanels; else return 0;}

        int nYPanels() const;

        bool hasPanel3(int idx3) const;
        bool hasPanel4(int idx4) const;

        void setXPanelDist(int iSection, xfl::enumDistribution xdistrib){if(iSection>=0 && iSection<sectionCount()) m_Section[iSection].m_XPanelDist = xdistrib;}
        void setYPanelDist(int iSection, xfl::enumDistribution ydistrib){if(iSection>=0 && iSection<sectionCount()) m_Section[iSection].m_YPanelDist = ydistrib;}
        xfl::enumDistribution xPanelDist(int iSection) const {if(iSection>=0 && iSection<sectionCount()) return m_Section.at(iSection).m_XPanelDist; else return xfl::UNIFORM;}
        xfl::enumDistribution yPanelDist(int iSection) const {if(iSection>=0 && iSection<sectionCount()) return m_Section.at(iSection).m_YPanelDist; else return xfl::UNIFORM;}

        bool isWingFoil(const Foil *pFoil) const;

        WingSection const &section(int iSec) const {return m_Section.at(iSec);}
        WingSection &section(int iSec) {return m_Section[iSec];}
        WingSection &rootSection() {return m_Section.first();}
        WingSection &tipSection()  {return m_Section.last();}
        WingSection const &rootSection() const {return m_Section.first();}
        WingSection const &tipSection()  const {return m_Section.last();}
        WingSection *pSection(int iSec);

        double rootChord()     const  {if(sectionCount()>0) return m_Section.first().m_Chord;    else return 0.0;}
        double rootOffset()    const  {if(sectionCount()>0) return m_Section.first().m_Offset;   else return 0.0;}
        double tipChord()      const  {if(sectionCount()>0) return m_Section.back().m_Chord;     else return 0.0;}
        double tipTwist()      const  {if(sectionCount()>0) return m_Section.back().m_Twist;     else return 0.0;}
        double tipOffset()     const  {if(sectionCount()>0) return m_Section.back().m_Offset;    else return 0.0;}
        double tipPos()        const  {if(sectionCount()>0) return m_Section.back().m_YPosition; else return 0.0;}

        int nStations() const {return m_NStation;}

        void setChord(int iSection, double chord) {if(iSection>=0 && iSection<sectionCount()) m_Section[iSection].m_Chord=chord;}
        double chord(int iSection) const {if(iSection>=0 && iSection<sectionCount()) return m_Section.at(iSection).m_Chord; else return 0.0;}

        void setOffset(int iSection, double offset) {if(iSection>=0 && iSection<sectionCount()) m_Section[iSection].m_Offset =offset;}
        double offset(int iSection) const {if(iSection>=0 && iSection<sectionCount()) return m_Section.at(iSection).m_Offset; else return 0.0;}

        void setSectionLength(int iSection, double length) {if(iSection>=0 && iSection<sectionCount()) m_Section[iSection].m_Length=length;}
        double sectionLength(int iSection) const {if(iSection>=0 && iSection<sectionCount()) return m_Section.at(iSection).m_Length; else return 0.0;}

        void setTwist(int iSection, double twist) {if(iSection>=0 && iSection<sectionCount()) m_Section[iSection].m_Twist = twist;}
        double twist(int iSection) const {if(iSection>=0 && iSection<sectionCount()) return m_Section.at(iSection).m_Twist; else return 0.0;}
        double twist() const {return m_Section.last().twist();}

        void setYPosition(int iSection, double yPos) {if(iSection>=0 && iSection<sectionCount()) m_Section[iSection].m_YPosition = yPos;}
        double yPosition(int iSection) const {if(iSection>=0 && iSection<sectionCount()) return m_Section.at(iSection).m_YPosition; else return 0.0;}

        void setDihedral(int iSection, double dih) {if(iSection>=0 && iSection<sectionCount()) m_Section[iSection].m_Dihedral = dih;}
        double dihedral(int iSection) const {if(iSection>=0 && iSection<sectionCount()) return m_Section.at(iSection).m_Dihedral; else return 0.0;}

        void setYProj(int iSection, double yproj) {if(iSection>=0 && iSection<sectionCount()) m_Section[iSection].m_YProj = yproj;}
        double yProj(int iSection) const  {if(iSection>=0 && iSection<sectionCount()) return m_Section.at(iSection).m_YProj; else return 0.0;}

        void setZPosition(int iSection, double z) {if(iSection>=0 && iSection<sectionCount()) m_Section[iSection].m_ZPos = z;}
        double zPosition(int iSection) const {if(iSection>=0 && iSection<sectionCount()) return m_Section.at(iSection).m_ZPos; else return 0.0;}

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

        void setRightFoilName(int iSection, QString const &foilname){if(iSection>=0 && iSection<sectionCount()) m_Section[iSection].m_RightFoilName = foilname;}
        void setLeftFoilName(int iSection, QString const &foilname) {if(iSection>=0 && iSection<sectionCount()) m_Section[iSection].m_LeftFoilName  = foilname;}
        QString rightFoilName(int iSection) const {if(iSection>=0 && iSection<sectionCount()) return m_Section.at(iSection).m_RightFoilName; else return QString();}
        QString leftFoilName(int iSection)  const {if(iSection>=0 && iSection<sectionCount()) return m_Section.at(iSection).m_LeftFoilName;  else return QString();}

        bool serializePartXFL(QDataStream &ar, bool bIsStoring);
        bool serializePartFl5(QDataStream &ar, bool bIsStoring) override;

        double integralC2(double y1, double y2, double c1, double c2) const;

        int uniformizeXPanelNumber();

        Surface &surface(int iSurface) {return m_Surface[iSurface];}
        Surface const &surfaceAt(int iSurface) const {return m_Surface.at(iSurface);}
        int surfaceCount() const {return m_Surface.size();}
        int nSurfaces() const {return m_Surface.size();}

        double stripArea(int p) const {return m_StripArea.at(p);}

        double taperRatio()  const;


        bool checkFoils(QString &log) const;

        int triangleCount() const;
        void exportAVLWing(QTextStream &out, int index, double y, double Thetay, double lengthunit) const;

        void resizeSpanDistribs(int nStations=-1);

        void getProperties(QString &props, const QString &prefix) const;

        double flapMomentAt(int iFlap) const {return m_FlapMoment.at(iFlap);}

        void makeTriangulation(QVector<Triangle3d> &triangles, int CHORDPANELS, int SPANPANELS, double scalefactor=1.0) const;
        void makeTriangulation(const Fuse *pFuse, int CHORDPANELS) const;

        int nTipStrips() const {return m_nTipStrips;}
        void setNTipStrips(int nstrips) {m_nTipStrips=nstrips;}

        bool connectInnerSurfaces(QVector<Panel3> &panels, bool bThickSurfaces);
        bool connectSurfaceToNext(int iSurf, QVector<Panel3> &panels, bool bConnectFlaps, bool bThickSurfaces);

        bool hasCenterGap() const;


        static double minSurfaceLength() {return s_MinSurfaceLength;}
        static void setMinSurfaceLength(double size) {s_MinSurfaceLength=size;}


    private:
        WingXfl::enumType m_WingType;  /** Defines the type of wing on the plane : main, second, elevator, fin, other */

        double m_MAChord;                      /**< the wing's mean aerodynamic chord */
        double m_PlanformSpan;                 /**< the planform span, i.e. if the dihedral was 0 at each junction */
        double m_ProjectedSpan;                /**< the span projected on the xy plane defined by z=0 */
        double m_PlanformArea;                 /**< the planform wing area, i.e. if the dihedral was 0 at each junction */
        double m_ProjectedArea;                /**< the wing area projected on the xy plane defined by z=0; */

        int m_nPanel3;                         /**< the number of triangular panels on this Wing; depends on whether the polar is a thin or thicksurface type */
        int m_nPanel4;                         /**< the number of quad panels on this Wing; depends on whether the polar is a thin or thicksurface type */
        int m_nNodes;                          /**< the number of nodes on this Wing; depends on whether the polar is a thin or thicksurface type */

        mutable Triangulation m_Triangulation;      /** The surface triangulation used for displays - stored to save redraw times */
        mutable QVector<Segment3d> m_Outline;   /** The outline segments  used for displays - stored to save redraw times */


        bool m_bSymmetric;	             /**< true if the wing's geometry is symmetric */
        bool m_bTwoSided;               /**< true if the wing describes a double fin symmetric about the y=0 plane */
        bool m_bCloseInnerSide;
        int m_NStation;                  /**< the number of stations for wing calculation; either the number of points of LLT, or the number of spanwise panels  */

        int m_nTipStrips;                /**< the number of horizontal panel strips in the left and right tip patches; introduced in v7.01 beta 09 */

        int m_nFlaps;                    /**< the number of T.E. flaps, numbered from left wing to right wing; for a main wing this number is even*/
        QVector<double> m_FlapMoment;      /**< the flap moments resulting from the panel of VLM analysis */

        QVector<double> m_StripArea;
        QVector<double> m_StripPos;
        QVector<double> m_Chord;
        QVector<double> m_Offset;
        QVector<double> m_Twist;
        QVector<Vector3d> m_PtC4;

    public:
        QVector<WingSection> m_Section;         /**< the array of wing sections. A WingSection extends between a foil and the next. */
        QVector<Surface> m_Surface;             /**< the array of Surface objects associated to the wing */
        QVector<int> m_StripStartNodes;         /**< the indexes of the nodes at the start of each wingsection */

        static double s_MinSurfaceLength;       /**< wing minimum panel size ; panels of less length are ignored */
};

