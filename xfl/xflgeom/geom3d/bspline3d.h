/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QVector>

#include <xflgeom/geom3d/vector3d.h>
#include <xflcore/linestyle.h>

class BSpline3d
{
    public:
        BSpline3d();

        int degree() const {return m_degree;}
        void setDegree(int deg) {m_degree=deg;}

        void appendControlPoint(double x, double y, double z, double w=1.0);
        void appendControlPoint(const Vector3d &Pt, double w=1.0);
        void resizeControlPoints(int nPts);

        void clearControlPoints() {m_CtrlPt.clear();}
        int ctrlPointCount() const {return m_CtrlPt.size();}
        void setControlSize(int n) {m_CtrlPt.resize(n); m_Weight.resize(n);}

        QVector<Vector3d> const &controlPoints() const {return m_CtrlPt;}
        Vector3d &controlPoint(int i) {return m_CtrlPt[i];}
        Vector3d const &controlPoint(int i) const {return m_CtrlPt[i];}
        Vector3d const &lastCtrlPoint()     const {return m_CtrlPt.last();}
        Vector3d const &firstCtrlPoint()    const {return m_CtrlPt.first();}
        void setCtrlPoint(int n, double x, double y, double z) {m_CtrlPt[n]=Vector3d(x,y,z);}
        void setCtrlPoint(int n, Vector3d const &pt) {m_CtrlPt[n]=pt;}
        void setFirstCtrlPoint(Vector3d const&pt) {if(m_CtrlPt.size()>0) m_CtrlPt.first()=pt;}
        void setLastCtrlPoint(double x, double y, double z) {if(m_CtrlPt.size()>0) m_CtrlPt.back()=Vector3d(x,y,z);}
        void setLastCtrlPoint(Vector3d const&pt)  {if(m_CtrlPt.size()>0) m_CtrlPt.last()=pt;}
        void setCtrlPoints(const QVector<Vector3d> &ptList, double w=1.0);

        void appendCtrlPoints(QVector<Vector3d> const & ptList, double w=1.0);
        void appendCtrlPoints(QVector<Vector3d> const & ptList, QVector<double> weightlist);

        void insertCtrlPointAt(int iSel, double x, double y, double z, double w=1.0);
        void insertCtrlPointAt(int iSel, Vector3d pt, double w=1.0);
        void insertCtrlPointAt(int iSel);
        bool removeCtrlPoint(int k);
        int isCtrlPoint(double x, double y, double tolx, double toly) const;

        bool isModified() const {return m_bIsModified;}
        void setModified(bool bModified) {m_bIsModified=bModified;}

        void resetSpline();
        bool updateSpline();
        void splinePoint(double t, Vector3d &pt) const;
        void splineDerivative(double t, Vector3d &der) const;
        void splineDerivative(BSpline3d &der) const;
        void makeCurve();

        bool approximate(int degree, int nPts, QVector<Vector3d> const& pts);

        int outputSize() const {return m_Output.size();}
        void setOutputSize(int n) {m_Output.resize(n);}
        QVector<Vector3d> const &outputPts() {return m_Output;}
        Vector3d outputPt(int i) const {if(i<m_Output.size()) return m_Output.at(i); else return Vector3d();}

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

        void setKnots(QVector<double> const&knots) {m_knot=knots;}
        QVector<double> const &knots() const {return m_knot;}
        double knot(int ik) const {if(ik>=0 && ik<m_knot.size()) return m_knot.at(ik); else return 0.0;}

        void setUniformWeights();

        bool isSingular() const {return m_bSingular;}

    private:
        bool splineKnots();

    private:
        int m_iHighlight;                /**< the index of the currently highlighted control point, i.e. the point over which the mouse hovers, or -1 of none. */
        int m_iSelect;                   /**< the index of the currently selected control point, i.e. the point on which the user has last click, or -1 if none. */

        QVector<Vector3d> m_CtrlPt;      /**< the array of the positions of the spline's control points */
        QVector<double> m_Weight;   /**< the array of weight of control points. Used for B-Splines only. Default is 1. The higher the value, the more the curve will be pulled towards the control points. */
        QVector<Vector3d> m_Output;      /**< the array of output points, size of which is m_iRes @todo use a QVarLengthArray or a QList*/

        LineStyle m_theStyle;

        bool m_bShowCtrlPts;
        bool m_bShowNormals;
        bool m_bIsModified;

        bool m_bClosed;
        bool m_bForcesymmetric; /** if true, the first and last control points are forced on the x-axis, i.e. y=0
                                   @todo has nothing to do here*/
        bool m_bSingular;

        double m_BunchAmp;  /** k=0.0 --> uniform bunching, k=1-->full varying bunch */
        double m_BunchDist; /** k=0.0 --> uniform bunching, k=1 weigth on endpoints */

        // BSpline specific
        QVector<double> m_knot;            /**< the array of the values of the spline's knot */
        int m_degree;
};


