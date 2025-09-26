/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

/**
 *@file
 *
 * This class defines the foil object used in 2d and 3d calculations
 *
 */



#pragma once

#include <QTextStream>
#include <QPoint>
#include <QVector>


#include <xflgeom/geom2d/splines/cubicspline.h>
#include <xflgeom/geom2d/node2d.h>
#include <xflcore/linestyle.h>
#include <xflcore/xflobject.h>

#include <xflgeom/geom2d/splines/bspline.h>


/**
*@class Foil
*@brief
*	The class which defines the Foil object used in 2D and 3D calculations.

The class stores two geometries:
    - the base foil, which unless advised otherwise is unchanged from the moment it has been first loaded or created
    - the current foil, on which the geometrical modifications are applied
       such as flap deflection, camber and thickness scaling
@todo One of the very early classes in this project. Would need a general revision.
Also it mixes the construction methods and the GUI; would be better to move the GUI to a derived child class for polymorphism.
*/
class Foil : public XflObject
{
    public:
        Foil();
        Foil(const Foil *pFoil);
        Foil(const Spline *pSpline);

        Vector2d basePoint(int i) {return m_BaseNode.at(i);}
        void setBasePoint(int ipt, Vector2d const& pt) {m_BaseNode[ipt]=pt;}

        QVector<Node2d> nodes() const;
        Node2d const &node(int index) const;
        void setNode(int idx, double xnew, double ynew) {if(idx>=0 && idx<nBaseNodes()) m_BaseNode[idx]={xnew, ynew};}
        Vector2d const &normal(int i) const {return m_Node.at(i).normal();}

        void getY(QVector<Vector2d> const &vec, double m_x, double &m_y) const;
        double baseUpperY(double m_x) const;
        double baseLowerY(double m_x) const;
        Vector2d midYRel(double sRel, Vector2d &N) const;
        Vector2d lowerYRel(double xRel, Vector2d &N) const;
        Vector2d upperYRel(double xRel, Vector2d &N) const;

        void setDefaultMidLinePointDistribution(int nMidPoints);
        void makeBaseMidLine();

        double TEGap()               const;
        double area()                const;
        double bottomSlope(double x) const;
        double camber(double x)      const;
        double camberSlope(double x) const;
        double length()              const;
        double midLineAngle()        const;
        double maxCamber()           const;
        double maxThickness()        const;
        double thickness(double x)   const;
        double topSlope(double x)    const;
        double xCamber()             const;
        double xThickness()          const;

        void setCamber(double xcamb, double camb);
        void setThickness(double xthick, double thick);

        void makeBaseFromCamberAndThickness();
        void rebuildPointSequenceFromBase();


        double normalizeGeometry();

        double deRotate();

        bool makeTopBotSurfaces();

        void makeCubicSpline(int nCtrlPts=-1);
        void makeCubicSpline(CubicSpline &cs, int nCtrlPts=-1) const;
        void makeNormalsFromCubic(bool bBase);

        bool makeApproxBSpline(BSpline &bs, int deg, int nCtrlPts, int nOutputPts) const;

        bool sharpTE() const;
        Vector2d TEbisector() const;
        Vector2d const &TE() const {return m_TE;}
        Vector2d const &LE() const {return m_LE;}

        bool exportFoilToDat(QTextStream &out) const;
        bool initGeometry(bool bFast=false);

        void appendBasePoint(double m_x, double m_y);

        void copy(Foil const *pSrcFoil, bool bMeta=true);
        void copy(Foil const &SrcFoil, bool bMeta=true, bool bForceDeepCopy=false);

        void setFlaps();
        void setLEFlapData(bool bFlap, double xhinge, double yhinge, double angle);
        void setTEFlapData(bool bFlap, double xhinge, double yhinge, double angle);
        void setTEFlapAngle(double angle){m_TEFlapAngle=angle;}

        bool serializeXfl(QDataStream &ar, bool bIsStoring);
        bool serializeFl5(QDataStream &ar, bool bIsStoring);

        void interpolate(Foil const *pFoil1, Foil const *pFoil2, double frac);

        bool isCamberLineVisible() const {return m_bCamberLine;}
        void showCamberLine(bool bVisible) {m_bCamberLine = bVisible;}

        const QString & description() const {return m_Description;}
        void setDescription(QString const &desc) {m_Description=desc;}

        QString properties(bool bLong) const;

        int nNodes()  const {return m_Node.size();}
        int nBaseNodes() const {return m_BaseNode.size();}

        double x(int k) const {return m_Node.at(k).x;}
        double y(int k) const {return m_Node.at(k).y;}
        double xb(int k) const {return m_BaseNode.at(k).x;}
        double yb(int k) const {return m_BaseNode.at(k).y;}

        void clearPointArrays();
        void resizePointArrays(int size);

        void applyBase();

        bool hasTEFlap() const {return m_bTEFlap;}
        bool hasLEFlap() const {return m_bLEFlap;}
        double TEFlapAngle() const {return m_TEFlapAngle;}
        double LEFlapAngle() const {return m_LEFlapAngle;}
        double TEXHinge() const {return m_TEXHinge;}
        double TEYHinge() const {return m_TEYHinge;}
        Vector2d TEHinge() const;

        double LEXHinge() const {return m_LEXHinge;}
        double LEYHinge() const {return m_LEYHinge;}
        Vector2d LEHinge() const;

        int iSelectedPt() const {return m_iSelectedPt;}
        void setSelectedPt(int iH) {m_iSelectedPt = iH;}

        bool isFilled() const {return m_bFill;}
        void setFilled(bool bFill) {m_bFill=bFill;}

        void listCoords(bool bBaseCoords=false);
        void listSurface(bool bBase);

        void setBaseCamberLine(QVector<Vector2d> const &cb) {m_BaseCbLine=cb;}
        void setThicknessLine(QVector<double> const &th) {m_Thickness=th;}

        void resizeBaseArrays(int size) {m_BaseTop.resize(size); m_BaseBot.resize(size);}

        void setBaseTop(QVector<Vector2d> const &points) {m_BaseTop=points;}
        void setBaseBot(QVector<Vector2d> const &points) {m_BaseBot=points;}

        int LEindex() const {return m_iLE;}

        bool bIsInside(float x, float y) const;
        bool isInside(float x, float y) const;
        bool intersectFoil(const Vector2d &A, const Vector2d &B, Vector2d &I) const;

        void makeModPermanent();

    private:
        void resetFoil();

        void setLEFromCubicSpline();
        void setTE();

        void setTEFlap();
        void setLEFlap();

        bool project(Vector2d const &pt, Node2d &node) const;
        void getNodeOnCubic(double tau, Node2d &n2d) const;

    private:

        QString m_Description;	         /**< a free description */

        int m_iSelectedPt;                    /**< the index of the point to highlight in the display */
        bool m_bFill;                        /**< true if the inside of the foil should be displayed with a solid color */

    public:

        QVector<Node2d> m_BaseNode;          /**< the array of base foil points, i.e. non-flapped */
        QVector<Node2d> m_Node;              /**< the array of coordinates of the flapped foil*/

        QVector<Vector2d> m_BaseCbLine;      /**< the mid camber line points for the base unflapped foil */
        QVector<Vector2d> m_CbLine;          /**< the mid camber line points for the flapped foil */
        QVector<double> m_Thickness;         /**< the local thicknesses at each mid point*/

        QVector<Vector2d> m_BaseTop;	     /**< the upper surface points of the base geometry, i.e. non-flapped */
        QVector<Vector2d> m_BaseBot;         /**< the lower surface points of the base geometry, i.e. non-flapped */

        QVector<Vector2d> m_Top;	         /**< the upper surface points = base + flapped */
        QVector<Vector2d> m_Bot;	         /**< the lower surface points = base + flapped  */

        bool m_bCamberLine;                  /**< true if the foil mid camber line is to be displayed */


        Vector2d m_TE;                       /**< the trailing edge point */
        Vector2d m_LE;                       /**< the leading edge point. Note: NOT a foil point*/
        int m_iLE;                           /**< the index of the leading edge point - defined as the point of min x. */

        bool m_bTEFlap;          /**< true if the foil has a trailing edge flap */
        double m_TEFlapAngle;    /**< the trailing edge flap angle, in degrees*/
        double m_TEXHinge;       /**< the x-position of the trailing edge flap, in chord % */
        double m_TEYHinge;       /**< the y-position of the trailng edge flap, in chord %*/

        bool m_bLEFlap;          /**< true if the foil has a leading edge flap */
        double m_LEFlapAngle;    /**< the leading edge flap angle, in degrees */
        double m_LEXHinge;       /**< the x-position of the leading edge flap, in chord % */
        double m_LEYHinge;       /**< the y-position of the leading edge flap, in chord %*/

        CubicSpline m_CubicSpline;
        BSpline m_BSpline;

        double m_BSfracLE;         /**< the B-spline parameter such that m_BSpline(frac)=LE */
        double m_CSfracLE;         /**< the cubic spline parameter such that m_CSpline(frac)=LE */

};



