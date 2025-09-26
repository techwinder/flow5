/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



/**
 *@file This file implements the spline class used in foil direct and inverse design.
 */


#pragma once

#include <QVector>
#include <QColor>
#include <QTextStream>


#include <xflmath/mathelem.h>
#include <xflgeom/geom2d/vector2d.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflcore/linestyle.h>





/**
*@class Spline
*@brief  The class which defines the 2d spline object.

The spline is used in direct foil design to represent upper and lower surfaces, and in XInverse foil design to define the specification curve.

*/
class Spline
{
    public:
        enum enumType{ BSPLINE, CUBIC, BEZIER, POINT, ARC, NACA4, OTHER };
        enum enumBunch {NOBUNCH, UNIFORM, SIGMOID, DOUBLESIG};

    public:
        Spline();
        virtual ~Spline();

        virtual Spline* clone() const = 0;
        virtual void resetSpline();
        virtual void exportToFile(QTextStream &out) const;
        virtual void setDegree(int ) {}
        virtual int degree() const {return 3;}
        virtual void duplicate(const Spline &spline);
        virtual bool serializeFl5(QDataStream &ar, bool bIsStoring);

        virtual bool updateSpline() = 0;
        virtual void makeCurve() = 0;
        virtual void splinePoint(double u, double &x, double &y) const = 0;
        virtual void splineDerivative(double u, double &dx, double &dy) const = 0;
        Vector2d splinePt(double u) const; // name changed to avoid hiding splinePoint in sub classes
        Vector2d splineNormal(double u) const;

        virtual double length(double t0, double t1) const;

        virtual double xMin() const;
        virtual double xMax() const;
        virtual double yMin() const;
        virtual double yMax() const;

        virtual double closest(double x, double y, float precision) const;

        virtual double getY(double xinterp, bool bRelative) const;

        virtual void scale(double ratio);

        enumType splineType() const {return m_SplineType;}
        //    void setSplineType(Spline::enumSplineType type) {m_SplineType=type;} //don't

        bool isBSpline()      const {return m_SplineType==BSPLINE;}
        bool isCubicSpline()  const {return m_SplineType==CUBIC;}
        bool isBezierSpline() const {return m_SplineType==BEZIER;}
        bool isArcSpline()    const {return m_SplineType==ARC;}
        bool isPointSpline()  const {return m_SplineType==POINT;}
        bool isNaca4Spline()  const {return m_SplineType==Spline::NACA4;}

        void appendControlPoint(double x, double y, double w=1.0);
        void appendControlPoint(const Vector2d &Pt, double w=1.0);
        void resizeControlPoints(int nPts);

        void clearControlPoints() {m_CtrlPt.clear();}
        int ctrlPointCount() const {return m_CtrlPt.size();}
        int nCtrlPoints() const {return m_CtrlPt.size();}
        void setControlSize(int n) {m_CtrlPt.resize(n); m_Weight.resize(n);}
        void setCtrlPoints(const QVector<Vector2d> &ptList, double w=1.0);

        QVector<Vector2d> const &ctrlPts() {return m_CtrlPt;}
        Vector2d &controlPoint(int i) {return m_CtrlPt[i];}
        Vector2d const &controlPoint(int i) const {return m_CtrlPt.at(i);}
        Vector2d const &lastCtrlPoint()     const {return m_CtrlPt.last();}
        Vector2d const &firstCtrlPoint()    const {return m_CtrlPt.first();}
        void setCtrlPoint(int n, double x, double y) {m_CtrlPt[n]=Vector2d(x,y);}
        void setCtrlPoint(int n, Vector2d const &pt) {m_CtrlPt[n]=pt;}
        void setFirstCtrlPoint(Vector2d const&pt) {if(m_CtrlPt.size()>0) m_CtrlPt.first()=pt;}
        void setLastCtrlPoint(double x, double y) {if(m_CtrlPt.size()>0) m_CtrlPt.back()=Vector2d(x,y);}
        void setLastCtrlPoint(Vector2d const&pt)  {if(m_CtrlPt.size()>0) m_CtrlPt.last()=pt;}

        void appendCtrlPoints(QVector<Vector2d> const & ptList, double w=1.0);
        void appendCtrlPoints(QVector<Vector2d> const & ptList, QVector<double> weightList);

        bool insertCtrlPoint(double x, double y, double w=1.0);
        void insertCtrlPointAt(int iSel, double x, double y, double w=1.0);
        void insertCtrlPointAt(int iSel, Vector2d const &pt, double w=1.0);
        void insertCtrlPointAt(int iSel);
        bool removeCtrlPoint(int k);
        int isCtrlPoint(double x, double y, double tolx, double toly) const;


        void setPointWeight(int p, double w);
        void setPointWeights(QVector<double> const &weights) {m_Weight=weights;}
        void setUniformWeights();
        double const &weight(int i) const {return m_Weight.at(i);}
        void setWeight(int i, double w) {if(i>=0 && i<m_Weight.size()) m_Weight[i]=w;}

        void copysymmetric(const Spline &spline);

        void exportSpline(QTextStream &out) const;

        void makeDefaultControlPoints(bool bClosed, bool bTopHalfOnly);

        void setTheStyle(LineStyle ls) {m_theStyle=ls;}
        LineStyle theStyle() const {return m_theStyle;}

        bool isVisible() const {return m_theStyle.m_bIsVisible;}
        void setVisible(bool bVisible) {m_theStyle.m_bIsVisible = bVisible;}

        Line::enumLineStipple stipple() const {return m_theStyle.m_Stipple;}
        void setStipple(Line::enumLineStipple style){m_theStyle.m_Stipple=style;}

        int width() const {return m_theStyle.m_Width;}
        void setWidth(int width){m_theStyle.m_Width = width;}

        void setColor(QColor clr) {m_theStyle.m_Color=clr;}
        QColor color() const {return m_theStyle.m_Color;}

        Line::enumPointStyle pointStyle() const {return m_theStyle.m_Symbol;}
        void setPointStyle(Line::enumPointStyle PointStyle) {m_theStyle.m_Symbol=PointStyle;}

        bool bShowCtrlPts() const {return m_bShowCtrlPts;}
        void showCtrlPts(bool bShow) {m_bShowCtrlPts=bShow;}

        bool bShowNormals() const {return m_bShowNormals;}
        void showNormals(bool bShow) {m_bShowNormals=bShow;}

        int outputSize() const {return m_Output.size();}
        void setOutputSize(int n) {m_Output.resize(n);}
        QVector<Vector2d> const &outputPts() const {return m_Output;}
        Vector2d outputPt(int i) const {if(i<m_Output.size()) return m_Output.at(i); else return Vector2d();}

        int selectedPoint() const {return m_iSelect;}
        void setSelectedPoint(int iPoint) {m_iSelect = iPoint;}
        void setSelectedPoint(Vector2d const&pt) {if(m_iSelect>=0 && m_iSelect<m_CtrlPt.size()) m_CtrlPt[m_iSelect]=pt;}

        int highlightedPoint() const {return m_iHighlight;}
        void setHighlighted(int n) {m_iHighlight=n;}


        bool isModified() const {return m_bIsModified;}
        void setModified(bool bModified) {m_bIsModified=bModified;}

        bool isClosed() const {return m_bClosed;}
        void setClosed(bool bClosed);

        bool issymmetric() const {return m_bForcesymmetric;}
        void setForcedsymmetric(bool bSym);

        bool isSingular() const {return m_bSingular;}

        void setBunchParameters(enumBunch bunchtype, double bunchamp) {m_BunchType=bunchtype, m_BunchAmp=bunchamp;}

        void setBunchAmplitude(double amp) {m_BunchAmp=amp;}
        double bunchAmplitude() const {return m_BunchAmp;}

        void setBunchType(enumBunch type) {m_BunchType=type;}
        enumBunch bunchType() const {return m_BunchType;}

        void translate(double tx, double ty);

        void makeSplit(int nsegs, xfl::enumDistribution distrib, QVector<double> &split) const;

    protected:
        int m_iHighlight;                /**< the index of the currently highlighted control point, i.e. the point over which the mouse hovers, or -1 of none. */
        int m_iSelect;                   /**< the index of the currently selected control point, i.e. the point on which the user has last click, or -1 if none. */

        QVector<Vector2d> m_CtrlPt;      /**< the array of the positions of the spline's control points */
        QVector<double> m_Weight;   /**< the array of weight of control points. Used for B-Splines only. Default is 1. The higher the value, the more the curve will be pulled towards the control points. */
        QVector<Vector2d> m_Output;      /**< the array of output points, size of which is m_iRes @todo use a QVarLengthArray or a QList*/

        LineStyle m_theStyle;

        enumType m_SplineType;

        bool m_bShowCtrlPts;
        bool m_bShowNormals;
        bool m_bIsModified;

        bool m_bClosed;
        bool m_bForcesymmetric; /** if true, the first and last control points are forced on the x-axis, i.e. y=0
                                   @todo has nothing to do here*/
        bool m_bSingular;

        double m_BunchAmp;  /** k=0.0 --> uniform bunching, k=1-->full varying bunch */
        enumBunch m_BunchType;
};


