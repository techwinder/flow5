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

#include <interfaces/graphs/containers/graphwt.h>
#include <api/bspline.h>

class SplinedGraphWt : public GraphWt
{
    Q_OBJECT

    public:
        SplinedGraphWt();

        BSpline const & spline() const {return m_Spline;}
        BSpline & spline() {return m_Spline;}

        void setGraph(Graph *pGraph) override;
        void initialize();
        void setAutoConvert(bool bauto) {m_bAutoConvert=bauto;}
        void setEndPointConstrain(bool b) {m_bConstrainEndPoints=b;}
        void setEndPtsOnCurve(bool b) {m_bOnCurve=b;}
        void convertSpline();

        void setXLimits(double xmin, double xmax) {m_XMin=xmin; m_XMax=xmax;}
        void setYLimits(double ymin, double ymax) {m_YMin=ymin; m_YMax=ymax;}

        void setSplineStyle(LineStyle const &ls) {m_Spline.setTheStyle(ls);}

        //debug only
        void setIndexes(int imin, int imax) {m_CurveIdxMin=imin; m_CurveIdxMax=imax;}
        int curveIdxMin() const {return m_CurveIdxMin;}
        int curveIdxMax() const {return m_CurveIdxMax;}


    protected:
        void paintEvent(       QPaintEvent* pEvent) override;
        void mouseMoveEvent(   QMouseEvent *pEvent) override;
        void mousePressEvent(  QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(300,200);}


        void resetGraphScales();

    signals:
        void splineModified(int,int);


    public slots:
        void onResetGraphScales() override;

    private:
        QPoint m_PointDown;        /**< the screen point where the left mouse button was last pressed */
        QPoint m_ptPopUp;          /**< the screen point where the right mouse button was last pressed */

        BSpline m_Spline;

        bool m_bAutoConvert;        /**< if true, converts the spline to the first curve in the graph each time it is modified */
        bool m_bConstrainEndPoints; /**< if true, the first control point is forced at (0,0) and the last is forced at (x,0) */
        bool m_bOnCurve;            /**< if true, the end points are constrained on the curve */
        double m_XMin, m_XMax; /** the limits along the x-axis for the control points */
        double m_YMin, m_YMax; /** the limits along the y-axis for the control points */

        int m_CurveIdxMin;     /** the index of the curve's pointwhich coincides with the spline's first control point */
        int m_CurveIdxMax;     /** the index of the curve's pointwhich coincides with the spline's last control point */
};

