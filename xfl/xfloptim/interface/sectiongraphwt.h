/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QFrame>
#include <QCheckBox>


#include <xflgraph/containers/graphwt.h>
#include <xflgeom/geom2d/splines/bspline.h>

class SectionGraphWt : public GraphWt
{
    Q_OBJECT

    friend class OptimCp3d;

    public:
        SectionGraphWt();

        BSpline const & spline(int is) const {return m_Spline.at(is);}
        BSpline & spline(int is) {return m_Spline[is];}
        int splineCount() const {return m_Spline.size();}
        void clearSplines() {m_Spline.clear();}
        void hideSplines() {for(int is=0; is<m_Spline.size(); is++) m_Spline[is].setVisible(false);}
        void setActiveSpline(int is) {m_iActiveSpline=is;}

        int curveIdxMin(int is) const {return m_CurveIdxMin.at(is);}
        int curveIdxMax(int is) const {return m_CurveIdxMax.at(is);}

        void setGraph(Graph *pGraph) override;
        void initialize();
        void setEndPtsOnCurve(bool b) {m_bOnCurve=b;}

        QSize sizeHint() const override {return QSize(300,200);}

        void makeControls();
        void makeSplines();

        void enableSpline(bool b) {m_bEnableSpline=b;}

    protected:
        void contextMenuEvent( QContextMenuEvent *pEvent) override;
        void mouseMoveEvent(   QMouseEvent       *pEvent) override;
        void mousePressEvent(  QMouseEvent       *pEvent) override;
        void mouseReleaseEvent(QMouseEvent       *pEvent) override;
        void paintEvent(       QPaintEvent       *pEvent) override;
        void resizeEvent(      QResizeEvent      *pEvent) override;

        void resetGraphScales();

    private:
        void resizeControls();
        void makeDefaultSpline(int ispline);

    signals:
        void curvesChanged();
        void splineChanged();
        void splineConnected(); // This signal is emmitted when a new spline has been connected and the swarm has become invalid

    private slots:
        void onShowAllCurves();
        void onHideAllCurves();
        void onResetSpline();

    private:
        bool m_bEnableSpline;

        QFrame *m_pFrame;
        QVector<QCheckBox*> m_pchCurve;

        QPoint m_PointDown;        /**< the screen point where the left mouse button was last pressed */
        QPoint m_ptPopUp;          /**< the screen point where the right mouse button was last pressed */

        QVector<BSpline> m_Spline;

         bool m_bOnCurve;            /**< if true, the end points are constrained on the curve */

        QVector<int> m_CurveIdxMin;     /** the index of the curve's pointwhich coincides with the spline's first control point */
        QVector<int> m_CurveIdxMax;     /** the index of the curve's pointwhich coincides with the spline's last control point */

        int m_iActiveSpline;
};


