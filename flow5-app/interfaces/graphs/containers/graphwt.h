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
#include <QTimer>
#include <QElapsedTimer>
#include <QLabel>

#include <core/fontstruct.h>
#include <interfaces/graphs/graph/graph.h>
class Graph;
class Curve;


class GraphWt : public QWidget
{
    Q_OBJECT

    public:
        GraphWt(QWidget *pParent=nullptr);
        virtual ~GraphWt();

        virtual void setGraph(Graph *pGraph);

        Graph *graph() {return m_pGraph;}
        void setNullGraph() {m_pGraph=nullptr;}

        void connectSignals();

        void showLegend(bool bShow);
        void setLegendPosition(Qt::Alignment pos);
        void setLegendOrigin(QPoint const &pt) {m_LegendOrigin = pt;}
        QPointF const &legendOrigin() const {return m_LegendOrigin;}

        void enableContextMenu(bool bEnable) {m_bEnableContextMenu=bEnable;}
        void enableCurveStylePage(bool bEnable) {m_bCurveStylePage=bEnable;} // in graphdlg

        void setDefaultSize(QSize sz) {m_DefaultSize = sz;}

        void scaleAxes(int iy, double zoomfactor);

        void setOverlayedRect(bool bShow, double tlx, double tly, double brx, double bry);

        void setOutputInfo(QString const &info);
        void clearOutputInfo() {m_plabInfoOutput->clear();}


    public:
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void paintEvent(QPaintEvent *pEvent) override;
        void resizeEvent (QResizeEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void keyReleaseEvent(QKeyEvent *pEvent) override;
        void mouseDoubleClickEvent (QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void wheelEvent(QWheelEvent *pEvent) override;
        void closeEvent(QCloseEvent *pEvent) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;
        QSize sizeHint() const override {return m_DefaultSize;}
        QSize minimumSizeHint() const override {return QSize(50,50);}

    signals:
        void graphChanged(Graph*); // settings changed
        void graphResized(Graph*);
        void curveClicked(Curve*,int);
        void curveDoubleClicked(Curve*);
        void widgetClosed(GraphWt*);
        void graphWindow(Graph*);
        void graphExport(Graph*);


    public slots:
        virtual void onResetGraphScales();
        void onCloseWindow();
        void onCopyData();
        void onDynamicIncrement();
        void onExportGraphDataToFile();
        void onExportGraphDataToSvg();
        void onGraphSettings();
        void onHovered();
        void onShowGraphLegend();

    protected:
        void repositionLegend();
        void startDynamicTimer();
        void stopDynamicTimer();

    protected:

        bool m_bCurveStylePage; // in graphdlg
        bool m_bEnableContextMenu;
        QAction *m_pResetScales, *m_pGraphSettings;
        QAction *m_pToClipboard, *m_pToFile, *m_pToSVG;
        QAction *m_pShowGraphLegend;
        QAction *m_pCloseGraph;

        Graph *m_pGraph;

        QPointF m_LegendOrigin;

        QPointF m_LastPoint;           /**< The client position of the previous mouse position*/
        QPoint  m_LastPressedPt;    /**< The client position of the previous mouse press position*/

        bool m_bTransGraph;
        bool m_bXPressed;                  /**< true if the X key is pressed */
        bool m_bYPressed;                  /**< true if the Y key is pressed */

        QTimer m_HoverTimer;

        QSize m_DefaultSize;

        QTimer  m_DynTimer;
        QElapsedTimer m_MoveTime;
        bool m_bDynTranslation;
        bool m_bDynScaling;
        bool m_bDynResetting;
        int m_iResetDyn;
        QPointF m_Trans;
        double m_ZoomFactor;
        Axis m_xAxisStart, m_yAxisStart[2];
        Axis m_xAxisEnd, m_yAxisEnd[2];

        bool m_bOverlayRectangle;
        QPointF m_TopLeft, m_BotRight; // in graph coordinates; should really be a Vector2d


        QLabel *m_plabInfoOutput;
};

