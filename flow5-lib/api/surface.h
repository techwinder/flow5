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


#include <Geom_BSplineCurve.hxx>


#include <vector3d.h>
#include <enums_objects.h>
#include <mathelem.h>
#include <panel3.h>
#include <panel4.h>



/**
*@class Surface
*@brief
 * The class which defines the surface object on which the panels are constructed for the VLM and 3d-panel calculations.
 * The mesh panels are not constructed using the Wing object, but on this proxy Surface object.
 *
 A surface extends from one left Foil to the next right Foil in the spanwise directions.

 * The Surface's geometry is defined by
 *    - its Leading edge  : m_LA, m_LB
 *    - its Trailing edge : m_TA, m_TB
 *    - its left and right twist
 *    - its left and right Foil objects
 *    - its Normal vector
 *    - its left and right normal vectors NormalA and NormalB are
 *      the average of the normals of the two continuous surfaces ; used to define junction between panels
 *
 * The points and other geometric data may be requested by calling methods on any of the top, middle, or bottom surfaces.

 Surfaces are constructed and indexed from left tip to right tip.

 For a half-wing, there will be one surface less than the number of foils defined by the user.


 The letter A refers to the Surface's left side, and B refers to the right side.
 The letter L refers to the Leading upstream side, T refers to the Trailing downstream side

 A surface is described by its geometric properties inherited from the wing's definition.
 Surfaces with a span length less than a minimum value are ignored.

 A surface may only supports trailing edge flaps.
 A surface may not be flat if different washout angles have been defined at the tips. However the deviation from
 the flat panel is assumed to be small, and corresponding approximations are made.

 The process of Surface construction starts by the creation of the array of master points at each end Foil.
 These points are defined i.a.w. the number of panels and the type of distribution specified by the user.
 The panel points are interpolated between the left and right Foil points.
 This implies that a wing is built with a fixed number of chordwise panels all along the span.
 The panels on the surface are added incrementally to the global array of panels used for the panel analysis.

 The panels are numbered from left tip to right tip in the span wise direction. Then in the chordwise direction:
        from T.E. to L.E in the case of VLM
        from lower surface TE, to leading edge, and to upper surface TE


 The data is stored in International Standard Units, i.e. meters, seconds, kg, and Newtons.
 Angular data is stored in degrees.


*/

class WingSection;
class Foil;
class PlaneXfl;
class PlanePolar;
class Fuse;
class FuseXfl;
class Vector3d;
class Panel4;
class Panel3;
class Node;
class BSpline3d;


class FL5LIB_EXPORT Surface
{
    friend class  WingXfl;

    public:

        Surface();

        void addFlapPanel3Index(int idx);
        void addFlapPanel4(Panel4 const &p4);
        void addPanel3Index(int idx);
        void addPanel4Index(int idx);
        void copy(Surface const &pSurface);
        void createXPoints(int nRefXFlaps=-1);
        inline void getC4(int kStrip, Vector3d &ptC4, double &tau) const;
        inline void getLeadingPt(int kStrip, Vector3d &C) const;
        inline void getTrailingPt(int kStrip, Vector3d &C) const;
        inline void getNormal(double yrel, Vector3d &N) const;
        void getPanel(int k, int l, xfl::enumSurfacePosition pos, Vector3d &LA, Vector3d &LB, Vector3d &TA, Vector3d &TB) const;
        void getSideNode(double xRel, bool bRight, xfl::enumSurfacePosition pos, Node &node) const;
        void getSurfacePoint(double xArel, double xBrel, double yrel, xfl::enumSurfacePosition pos, Vector3d &Point, Vector3d &PtNormal) const;
        void getSection(double const &tau, double &Chord, double &Area, Vector3d &PtC4) const;
        inline void getYDist(int k, double &y1, double &y2) const;

        void getSidePoints_1(xfl::enumSurfacePosition pos, const Fuse *pFuse, std::vector<Node> &PtA, std::vector<Node> &PtB, int nPoints, xfl::enumDistribution disttype) const;
        void getSidePoints1_task(int nPoints, int tmp_nPoints, Node *nodeA, Node *nodeB) const;

        void getSidePoints_2(xfl::enumSurfacePosition pos, const Fuse *pFuse, std::vector<Vector3d> &PtA, std::vector<Vector3d> &PtB, std::vector<Vector3d> &NA, std::vector<Vector3d> &NB, const std::vector<double> &xPointsA, const std::vector<double> &xPointsB) const;
        void getSidePoints2_task(double xRelA, double xRelB, Node &nodeA, Node &nodeB) const;


        void init();
        void clearQuadFlapMesh(){m_FlapNode4.clear();  m_FlapPanel4.clear();}
        void clearTriFlapMesh() {m_FlapPanel3.clear();}
        void rotateX(Vector3d const &O, double XTilt);
        void rotateY(Vector3d const &O, double YTilt);
        void rotateZ(Vector3d const &O, double ZTilt);
        void setCornerPoints(Vector3d const &PLA, Vector3d const &PTA, Vector3d const &PLB, Vector3d const &PTB) {m_LA = PLA; m_LB = PLB; m_TA = PTA; m_TB = PTB;}
        inline void setNormal();
        void setFlap();

        void makeSideNodes(Fuse const *pTranslatedFuse, bool bDebug=false);
        void makeSideNodeTask(int l, Fuse const *pTranslatedFuse, double alpha_dA, double alpha_dB);

        void setTwist(bool bQuarterChord);
        void setTwist2();
        void translate(Vector3d const &T);
        void translate(double tx, double ty, double tz);

        bool isCenterSurf()      const {return m_bIsCenterSurf;}
        bool isLeftSurf()        const {return m_bIsLeftSurf;}
        bool isRightSurf()       const {return m_bIsRightSurf;}
        bool isTipLeft()         const {return m_bIsTipLeft;}
        bool isTipRight()        const {return m_bIsTipRight;}
        bool isInSymPlane()      const {return m_bIsInSymPlane;}
        bool isClosedLeftSide()  const {return m_bClosedLeftSide;}
        bool isClosedRightSide() const {return m_bClosedRightSide;}

        void setCenterSurf(bool bCenterSurf)   {m_bIsCenterSurf    = bCenterSurf;}
        void setLeftSurf(bool bLeftSurf)       {m_bIsLeftSurf      = bLeftSurf;}
        void setRightSurf(bool bRightSurf)     {m_bIsRightSurf     = bRightSurf;}
        void setTipLeft(bool bTipLeft)         {m_bIsTipLeft       = bTipLeft;}
        void setTipRight(bool bTipRight)       {m_bIsTipRight      = bTipRight;}
        void setClosedLeftSide(bool bClosed)   {m_bClosedLeftSide  = bClosed;}
        void setClosedRightSide(bool bClosed)  {m_bClosedRightSide = bClosed;}
        void setIsInSymPlane(bool bIsSymPlane) {m_bIsInSymPlane    = bIsSymPlane;}

        bool hasTEFlap()    const {return m_bTEFlap;}


        bool bJoinRight()  const {return m_bJoinRight;}
        void setJoinRight(bool b) {m_bJoinRight=b;}

        bool hasPanel3(int idx3) const;
        bool hasPanel4(int idx4) const;

        bool hasFlapPanel3(const Panel3 &p3) const;
        bool hasFlapPanel3(int idx3) const;
        std::vector<int> const & flapPanel3() const {return m_FlapPanel3;}


        bool hasFlapPanel4(Panel4 const &p4) const;
        bool hasFlapPanel4(int idx4)   const;
        bool isFlapNode(int nNode)     const;


        double stripTwist(int k) const;
        double twistAt(double tau) const {return  m_TwistA + (m_TwistB-m_TwistA) *tau;}
        inline double chord(int k) const;
        inline double chord(double tau) const;
        double offset(double tau) const {return m_LA.x + (m_LB.x-m_LA.x) * fabs(tau);}
        double foilArea(double tau) const;
        inline double stripWidth(int k) const;
        double spanLength() const {return sqrt((m_LB.y - m_LA.y)*(m_LB.y - m_LA.y) + (m_LB.z - m_LA.z)*(m_LB.z - m_LA.z));}
        double planformLength() const {return m_Length;}
        void setPlanformLength(double l) {m_Length=l;}

        int innerSection() const {return m_InnerSection;}
        int outerSection() const {return m_OuterSection;}

        xfl::enumDistribution xDistType() const {return m_XDistType;}
        xfl::enumDistribution yDistType() const {return m_YDistType;}

        void setXDistType(xfl::enumDistribution type) {m_XDistType=type;}
        void setYDistType(xfl::enumDistribution type) {m_YDistType=type;}

        int NXPanels()  const  {return m_NXPanels;}
        int NYPanels()  const  {return m_NYPanels;}
        int NXFlap()    const  {return m_NXFlap;}
        int nQuads()    const  {return m_nQuads;}

        void setNXPanels(int nx) {m_NXPanels=nx;}
        void setNYPanels(int ny) {m_NYPanels=ny;}
        void setNXFlap(int nxFlap) {m_NXFlap = nxFlap;}
        void setNQuads(int nQuads) {m_nQuads=nQuads;}

        Foil const *foilA() const;
        Foil const *foilB() const;

        Foil *foilA();
        Foil *foilB();

        void setFoilNames(std::string const &nameA, std::string const &nameB) {m_FoilNameA=nameA; m_FoilNameB=nameB;}

        std::string const & leftFoilName() const {return m_FoilNameA;}
        std::string const & rightFoilName() const {return m_FoilNameB;}

        Vector3d &LA() {return m_LA;}
        Vector3d &LB() {return m_LB;}

        Vector3d &TA() {return m_TA;}
        Vector3d &TB() {return m_TB;}

        Vector3d const &LA() const {return m_LA;}
        Vector3d const &LB() const {return m_LB;}

        Vector3d const &TA() const {return m_TA;}
        Vector3d const &TB() const {return m_TB;}


        Vector3d const &normal() const {return m_Normal;}


        Vector3d const &hingePoint()  const {return m_HingePoint;}
        Vector3d const &hingeVector() const {return m_HingeVector;}

        int makeTipNodes(std::vector<Node> &nodes, bool bLeft, int nStrips) const;
        void makePanelNodes(std::vector<Node> &nodes, bool bMakeLeftNodes, bool bMidSurface) const;

        void makeTriPanels(std::vector<Panel3> &panel3list, std::vector<Node> &nodes, int ip3start, bool bThickSurfaces, int nTipStrips, int &leftnodeidx, int &rightnodeindex);
        int makeQuadPanels(std::vector<Panel4> &panel4list, int &nWakeColumn, bool bThickSurface, int nTipStrips);

        int isNode(Vector3d const &Pt, std::vector<Panel4> const & panel4list) const;
        std::vector<int> const &panel3list() const {return m_Panel3List;}
        std::vector<int> const &panel4List() const {return m_Panel4List;}

        inline Vector3d TEbisector(Vector3d const &Pt) const;

        std::vector<double> xDistribA() const {return m_xPointA;}
        std::vector<double> xDistribB() const {return m_xPointB;}
        void setXDistribA(std::vector<double> const &leftdistrib)  {m_xPointA=leftdistrib;}
        void setXDistribB(std::vector<double> const &rightdistrib) {m_xPointB=rightdistrib;}

        int nPanel3() const {return int(m_Panel3List.size());}
        int nPanel4() const {return int(m_Panel4List.size());}

        bool makeSectionSplines(BSpline3d & leftspline, BSpline3d &rightspline) const;
        bool makeSectionHalfSpline(xfl::enumSurfacePosition pos, bool bLeft, int degree, int nCtrlPoints, int nOutPoints, BSpline3d &spline) const;

        bool makeSectionSplinesOcc(bool bTop, bool bLeft, Handle(Geom_BSplineCurve)& theSpline) const;

        void setNormals(Vector3d const& NA, Vector3d const &NB) {m_NormalA=NA; m_NormalB=NB;}
        void setTwist(double thetaA, double thetaB) {m_TwistA=thetaA; m_TwistB=thetaB;}

        void setInnerSection(int is) {m_InnerSection=is;}
        void setOuterSection(int is) {m_OuterSection=is;}


    public:
        std::vector<Node> m_SideA;        /**< the array of panel points on the left foil's mid-line*/
        std::vector<Node> m_SideB;        /**< the array of panel points on the right foil's mid-line*/
        std::vector<Node> m_SideA_Top;    /**< the array of panel points on the left foil's top-line*/
        std::vector<Node> m_SideB_Top;    /**< the array of panel points on the right foil's top-line*/
        std::vector<Node> m_SideA_Bot;    /**< the array of panel points on the left foil's bottom-line*/
        std::vector<Node> m_SideB_Bot;    /**< the array of panel points on the right foil's bottom-line*/

        Vector3d m_Normal;            /**< the Surface's normal vector */
        Vector3d m_NormalA;           /**< the normal at the left tip, defined as the average of this Surface's normal and of the one adjacent on the left side, if any */
        Vector3d m_NormalB;           /**< the normal at the right tip, defined as the average of this Surface's normal and of the one adjacent on the right side, if any */
        double m_TwistA;           /**< the twist at side A in degrees */
        double m_TwistB;           /**< the twist at side B in degrees */

        Vector3d m_TEBisectorA;
        Vector3d m_TEBisectorB;

        void setIndex(int idx) {m_Index=idx;}
        int index() const {return m_Index;}

    private :

        int m_Index;               /**< the index of the surface in the wing's array of surfaces; debug use only */

        bool m_bIsInSymPlane;      /**< true if the Surface is positioned in the symetry xz plane defined by y=0. Case of a single fin. */
        bool m_bTEFlap;            /**< true if the Surface has a flap on the trailing edge */
        bool m_bIsLeftSurf;        /**< true if the Surface is built on the left wing */
        bool m_bIsRightSurf;       /**< true if the Surface is built on the right wing */
        bool m_bIsTipLeft;         /**< true if the Surface is built on the tip left wing */
        bool m_bIsTipRight;        /**< true if the Surface is built on the tip right wing */
        bool m_bIsCenterSurf;      /**< true if the Surface is either a left or right center surface... need to connect to body */
        bool m_bJoinRight;             /**< true if the surface's right side should be connected to the next right surface's right left side - for panel analysis only */
        bool m_bClosedLeftSide;
        bool m_bClosedRightSide;

        double m_Length;           /**< the Surface's planform length from A to B*/

        double m_posATE, m_posBTE;      /**< the relative flap hinge positions at sides A and B */

        std::vector<double> m_xPointA;        /**< the chordwise relative position of the VLM panel left corner points at side A */
        std::vector<double> m_xPointB;        /**< the chordwise relative position of the VLM panel right corner points at side B */

        xfl::enumDistribution m_XDistType;            /**< the type of distribution along the Surface's x axis */
        xfl::enumDistribution m_YDistType;            /**< the type of distribution along the Surface's y axis */
        int m_NXLead;                 /**< the number of panels upstream of the flap, i.e. between the leading edge and the hinge */
        int m_NXFlap;                 /**< the number of panels on the flap, i.e. between the hinge and the trailing edge */
        int m_nQuads;                 /**< the number of panel elements constructer on this Surface. */


        std::vector<int> m_FlapNode4;     /**< @todo remove the array of flap node indexes, used to avoid defining two nodes at the same location */
        std::vector<int> m_FlapPanel4;   /**< the array of flap panel indexes */
        std::vector<int> m_FlapPanel3;   /**< the array of flap panel indexes */

        Vector3d m_HingePoint;       /**< a point on the trailing flap hinge */
        Vector3d m_HingeVector;      /**< a vector which defines the axis of the hinge */

        Surface *m_pLeftSurface;     /**< a pointer to this Surface's left neighbour, or NULL if none */
        Surface *m_pRightSurface;    /**< a pointer to this Surface's right neighbour, or NULL if none */

        int m_InnerSection;            /**< the index of the inner wing's section corresponding to this surface */
        int m_OuterSection;            /**< the index of the outer wing's section corresponding to this surface */
        int m_NYPanels;                /**< the number of quad spanwise panels on this surface */
        int m_NXPanels;                /**< the number of quad chordwise panels on this surface */

        std::string m_FoilNameA;           /**< the name of the Surface's left foil name */
        std::string m_FoilNameB;           /**< the name of the Surface's right foil name */

        std::vector<int> m_Panel3List;    /**< the list of panel3 indexes of this surface */
        std::vector<int> m_Panel4List;    /**< the list of panel4 indexes of this surface */
        int m_FirstStripPanelIndex, m_LastStripPanelIndex; /**< temporary fields used to connect center-left and center-right surfaces */


        Vector3d m_LA;              /**< the Surface's leading left point */
        Vector3d m_LB;              /**< the Surface's leading right point */
        Vector3d m_TA;              /**< the Surface's trailing left point */
        Vector3d m_TB;              /**< the Surface's trailing right point */

        mutable double tmp_alpha_dA, tmp_alpha_dB, tmp_delta;
        mutable xfl::enumSurfacePosition tmp_pos;
        mutable Foil const *tmp_pFoilA, *tmp_pFoilB;
        mutable Fuse const*tmp_pFuse;


    public:
        static std::vector<Vector3d> s_DebugPts;
        static std::vector<Vector3d> s_DebugVecs;
};



/**
 * Returns the relative left and right span positions of a given strip
 * @param k the 0-based index of the strip.
 * @param y1 a reference to the relative left span position.
 * @param y2 a reference to the relative left span position.
 */
inline void Surface::getYDist(int k, double &y1, double &y2) const
{
    double YPanels = double(m_NYPanels);
    double dk      = double(k);

    double tau1 = dk/YPanels;
    y1 = xfl::getDistribFraction(tau1, m_YDistType);
    double tau2 = (dk+1)/YPanels;
    y2 = xfl::getDistribFraction(tau2, m_YDistType);
}


/** Sets the surface average normal vector */
inline void Surface::setNormal()
{
    Vector3d LATB, TALB;
    LATB = m_TB - m_LA;
    TALB = m_LB - m_TA;
    m_Normal = LATB * TALB;
    m_Normal.normalize();
}


inline Vector3d Surface::TEbisector(Vector3d const &Pt) const
{
    Vector3d TAPt = Pt-m_TA;
    Vector3d TATB = m_TB-m_TA;
    double l2 = TATB.dot(TATB);
    double yrel = std::max(0.0, TATB.dot(TAPt));
    double tau = sqrt(yrel/l2);
    return (m_TEBisectorA * (1-tau) + m_TEBisectorB *tau).normalized();
}




/**
 * Returns the normal vector at a specified relative span position.
 * @param tau the relative percentage of the Surface's span length
 * @return N the average normal at the specified location
 */
inline void Surface::getNormal(double yrel, Vector3d &N) const
{
    N = m_NormalA * (1.0-yrel) + m_NormalB * yrel;
    N.normalize();
}


/**
 * Returns the leading point of the specified strip
 * @param k the 0-based index of the strip for which the leading point shall be returned.
 * @param C the strip's leading point.
 */
inline void Surface::getLeadingPt(int kStrip, Vector3d &C) const
{
    Vector3d LA, LB, TA, TB;
    getPanel(kStrip,m_NXPanels-1, xfl::MIDSURFACE, LA, LB, TA, TB);

    C.x    = (LA.x+LB.x)/2.0;
    C.y    = (LA.y+LB.y)/2.0;
    C.z    = (LA.z+LB.z)/2.0;
}



/**
 * Returns the trailing point of the specified strip
 * @param k the 0-based index of the strip for which the trailing point shall be returned.
 * @param C the strip's leading point.
 */
inline void Surface::getTrailingPt(int kStrip, Vector3d &C) const
{
    Vector3d LA, LB, TA, TB;
    getPanel(kStrip,0,xfl::MIDSURFACE, LA, LB, TA, TB);

    C.x    = (TA.x+TB.x)/2.0;
    C.y    = (TA.y+TB.y)/2.0;
    C.z    = (TA.z+TB.z)/2.0;
}


/**
 * Returns the quarter-chord point of a specified strip
 * @param k the 0-based index of the strip for which the quarter-chord point shall be returned.
 * @param Pt the quarter-chord point
 * @param tau the relative span position of the Pt
 */
inline void Surface::getC4(int kStrip, Vector3d &ptC4, double &tau) const
{
    Vector3d LA, LB, TA, TB;
    getPanel(kStrip,m_NXPanels-1,xfl::MIDSURFACE, LA, LB, TA, TB);
    double xl = (LA.x+LB.x)/2.0;
    double yl = (LA.y+LB.y)/2.0;
    double zl = (LA.z+LB.z)/2.0;
    getPanel(kStrip,0,xfl::MIDSURFACE, LA, LB, TA, TB);
    double xt = (TA.x+TB.x)/2.0;
    double yt = (TA.y+TB.y)/2.0;
    double zt = (TA.z+TB.z)/2.0;
    ptC4.x = xl*.75 + xt*.25;
    ptC4.y = yl*.75 + yt*.25;
    ptC4.z = zl*.75 + zt*.25;

    tau = sqrt((ptC4.y-m_LA.y)*(ptC4.y-m_LA.y) + (ptC4.z-m_LA.z)*(ptC4.z-m_LA.z)) / m_Length;
}


/**
 * Returns the chord length of the specified strip
 * @param k the 0-based index of the strip for which the chord shall be returned.
 * @return the chord length
 */
inline double Surface::chord(int k) const
{
    double y1=0, y2=0;
    getYDist(k, y1, y2);
    return chord((y1+y2)/2.0);
}


/**
 * Returns the chord length at the specified relative span position.
 * @param tau the relative percentage of the Surface's span length
 * @return the chord length
 */
inline double Surface::chord(double tau) const
{
    //assumes LA-TB have already been loaded
    Vector3d V1, V2;
    double ChordA=0, ChordB=0;

    V1 = m_TA-m_LA;
    V2 = m_TB-m_LB;

    ChordA = V1.norm();
    ChordB = V2.norm();

    return ChordA + (ChordB-ChordA) * fabs(tau);
}


/**
 * Returns the strip width at a specified index
 * @param k the index of the strip 0<=k<m_NYPanels
 * @return the strip width
 */
inline double Surface::stripWidth(int k) const
{
    Vector3d LA, LB, TA, TB;

    getPanel(k, 0, xfl::MIDSURFACE, LA, LB, TA, TB);
    return sqrt((LA.y-LB.y)*(LA.y-LB.y) + (LA.z-LB.z)*(LA.z-LB.z));
}


