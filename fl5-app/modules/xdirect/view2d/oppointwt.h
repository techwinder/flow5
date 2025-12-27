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


#include <QWidget>
#include <QSettings>
#include <QLabel>
#include <QElapsedTimer>
#include <QTimer>

#include <api/linestyle.h>
#include <api/vector2d.h>
#include <interfaces/graphs/graph/axis.h>

class MainFrame;
class XDirect;
class OpPoint;
class Graph;
class Curve;
class Foil;
class Node2d;

class OpPointWt : public QWidget
{
    Q_OBJECT

    public:
        OpPointWt(QWidget *pParent = nullptr);
        ~OpPointWt();

        void setFoilScale(bool bYOffset);

        void showPressure(bool bPressure) {m_bPressure = bPressure;}
        void showBL(bool bBL) {m_bBL = bBL;}

    public:

        void setCpGraph(Graph* pGraph) {m_pCpGraph = pGraph;}
        Graph *CpGraph() const {return m_pCpGraph;}

        void setFoilData(const QString &data);
        void setOppData(const QString &data);

        static void setFillFoil(bool bFill) {s_bFillFoil = bFill;}
        static bool bFillFoil() {return s_bFillFoil;}

        static void showNeutralLine(bool b) {s_NeutralStyle.m_bIsVisible=b;}
        static bool bNeutralLine() {return s_NeutralStyle.m_bIsVisible;}

        static void setNeutralLineColor(QColor const &clr) {s_NeutralStyle.m_Color= {short(clr.red()), short(clr.green()), short(clr.blue())};}

        static LineStyle neutralStyle() {return s_NeutralStyle;}
        static LineStyle BLStyle() {return s_BLStyle;}
        static LineStyle pressureStyle() {return s_PressureStyle;}

        static void setNeutralLineStyle(LineStyle const&ls) {s_NeutralStyle = ls;}
        static void setBLStyle(LineStyle const &ls) {s_BLStyle = ls;}
        static void setPressureStyle(LineStyle const &ls) {s_PressureStyle = ls;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        static void setMainFrame(MainFrame*pMainFrame) {s_pMainFrame = pMainFrame;}
        static void setXDirect(XDirect *pXDirect) {s_pXDirect=pXDirect;}

    public slots:
        void onShowNeutralLine();
        void onResetFoilScale();
        void onGraphSettings();
        void onHovered();
        void onDynamicIncrement();
        void onResetIncrement();

    signals:
        void graphChanged(Graph *);
        void curveClicked(Curve *);
        void curveDoubleClicked(Curve *);

    protected:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void keyReleaseEvent(QKeyEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void paintEvent(QPaintEvent *pEvent) override;
        void resizeEvent(QResizeEvent *event) override;
        void wheelEvent(QWheelEvent *pEvent) override;
        void mouseDoubleClickEvent(QMouseEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

    private:
        void resetGraphScale();
        void paintOpPoint(QPainter &painter);
        void paintGraph(QPainter &painter);

        void paintPressure(QPainter &painter, double scalex, double scaley);
        void paintBLXFoil(QPainter &painter, const OpPoint *pOpPoint, double scalex, double scaley);
        void paintNode2d(QPainter &painter, QVector<Node2d> const &node, double alpha);

        Vector2d mousetoReal(const QPointF &point) const;

        void stopDynamicTimer();
        void startDynamicTimer();
        void scaleAxes(int iy, double zoomfactor);

        void zoomView(const QPointF &pt, double zoomFactor);

        QPointF defaultOffset() const;
        double defaultScale() const;

        double m_fScale, m_fScaleY;
        QPointF m_ptOffset;

        QTimer m_ResetTimer;
        QTimer m_DynTimer;
        int m_iTimerInc;
        QElapsedTimer m_MoveTime;
        bool m_bDynTranslation;
        bool m_bDynScaling;
        bool m_bDynResetting;
        int m_iResetDyn;
        QPointF m_Trans;
        double m_ScaleInc;
        double m_ScaleYInc;
        double m_ZoomFactor;
        Axis m_xAxisStart, m_yAxisStart[2];
        Axis m_xAxisEnd, m_yAxisEnd[2];
        QPoint  m_LastPressedPt;    /**< The client position of the previous mouse press position*/

        bool m_bTransFoil;
        bool m_bTransGraph;
        bool m_bAnimate;
        bool m_bBL;                /**< true if the Boundary layer shoud be displayed */
        bool m_bPressure;          /**< true if the pressure distirbution should be displayed */

        bool m_bXDown;                  /**< true if the X key is pressed */
        bool m_bYDown;                  /**< true if the Y key is pressed */

        Graph *m_pCpGraph;
        QRectF m_rGraphRect;
        QPointF m_LastPoint;

        QTimer *m_pHoverTimer;

        QLabel *m_plabFoilDataOutput, *m_plabOppDataOutput;

        static LineStyle s_BLStyle;         /**< the style used to draw the boundary layer */
        static LineStyle s_PressureStyle;   /**< the style used to draw the pressure arrows */
        static LineStyle s_NeutralStyle;    /**< the style used to draw the neutral line */

        static bool s_bFillFoil;

        static MainFrame *s_pMainFrame;   /**< A void pointer to the instance of the MainFrame object. */
        static XDirect *s_pXDirect;
};

