/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


/** @file
 *
 * This file implements the Curve object for the graphs.
 *
 */



#pragma once


#include <QVector>
#include <QColor>
#include <QPolygonF>


#include <xflcore/linestyle.h>


/**
* @class Curve
* This class defines the curve object used by the Graph class.
*/
class Curve
{
    public:
        Curve();
        Curve(Curve const &curve);

        int size() const {return m_pts.count();}
        int count() const {return m_pts.size();}


        int  appendPoint(double xn, double yn);
        int  appendPoint(double xn, double yn, QString const &tag);
        void popFront();

        void clear() {m_pts.clear(); m_Tag.clear();}
        void reset() {clear();}
        void resize(int n) {m_pts.resize(n);}

        double x(int ic) const {if(ic>=0 && ic<m_pts.size()) return m_pts.at(ic).x(); else return 0;}
        double y(int ic) const {if(ic>=0 && ic<m_pts.size()) return m_pts.at(ic).y(); else return 0;}

        QPointF const &firstPt() const {return points().front();}
        QPointF const &lastPt( ) const {return points().back();}

        QPolygonF const & points() const {return m_pts;}

        bool isEmpty() const {return m_pts.size()==0;}

        int closestPoint(double xs, double ys, double xScale, double yScale) const;
        int closestPoint(double const &xs, double const &ys, double &xSel, double &ySel, double &dist) const;
        void copyData(const Curve *pCurve);
        void duplicate(const Curve *pCurve);

        void setPoint(int ic, double xc, double yc) {if(ic>=0 && ic<m_pts.size()) {m_pts[ic]={xc,yc};}}
        void setPoints(QVector<double> const &xc, QVector<double> const&yc);
        void setPoints(QPolygonF const &pts) {m_pts=pts;}

        int selectedPoint() const {return m_iSelectedPt;}
        void setSelectedPoint(int n) {m_iSelectedPt = n;}

        bool isVisible() const {return m_theStyle.m_bIsVisible;}
        void setVisible(bool bVisible){m_theStyle.m_bIsVisible = bVisible;}

        QColor const &color() const {return m_theStyle.m_Color;}
        void setColor(QColor const &clr){m_theStyle.m_Color = clr;}

        Line::enumLineStipple stipple() const {return m_theStyle.m_Stipple;}
        void setStipple(Line::enumLineStipple stipple) {m_theStyle.m_Stipple = stipple;}

        LineStyle const &theStyle() const {return m_theStyle;}
        void setTheStyle(LineStyle lineStyle) {m_theStyle = lineStyle;}
        void setTheStyle(Line::enumLineStipple Style, int Width, QColor color, Line::enumPointStyle PointStyle, bool bVisible);

        bool pointsVisible() const {return m_theStyle.m_Symbol != Line::NOSYMBOL;}
        Line::enumPointStyle symbol() const {return m_theStyle.m_Symbol;}
        void setSymbol(Line::enumPointStyle symbols) {m_theStyle.m_Symbol=symbols;}


        int width() const {return m_theStyle.m_Width;}
        void setWidth(int nWidth){m_theStyle.m_Width = nWidth;}

        void setName(QString const &Title){ m_Name = Title;}
        QString const &name() const { return m_Name;}


        double  xMin() const;
        double  xMax() const;
        double  yMin() const;
        double  yMax() const;

        bool isYAxis(int iAxis) const {if(iAxis==0) return isLeftAxis(); else return isRightAxis();}
        bool isLeftAxis() const {return m_bLeftAxis;}
        bool isRightAxis() const {return !m_bLeftAxis;}
        void setLeftAxis(bool bLeft) {m_bLeftAxis=bLeft;}

        void clearTags() {m_Tag.clear();}
        int tagSize() const {return m_Tag.size();}
        void setTags(QStringList const &tags) {m_Tag=tags;}
        QString const&tag(int ipt) {return m_Tag.at(ipt);}
        void setTag(int ipt, QString const&tag) {m_Tag[ipt]=tag;}

        static void setDefaultLineWidth(int iWidth) {s_DefaultLineWidth = iWidth;}
        static int defaultLineWidth() {return s_DefaultLineWidth;}

        static void setAlignChildren(bool bAlign) {s_bAlignChildren=bAlign;}
        static bool alignChildren() {return s_bAlignChildren;}

    public:
        //	Curve Data

        QPolygonF m_pts;          /**< the array of points */

        QStringList m_Tag;

    private:
        QString m_Name;                       /**< the curves's name */
        int m_iSelectedPt;                         /**< the index of the curve's currently selected point, or -1 if none is selected */
        LineStyle m_theStyle;

        bool m_bLeftAxis;

        static int s_DefaultLineWidth;
        static bool s_bAlignChildren;
};


