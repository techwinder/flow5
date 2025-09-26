/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QLabel>

#include <xflcore/fontstruct.h>
#include <xflgraph/graph/graph.h>
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
        void clearOutputInfo() {m_plabInfoOutput->setText(QString());}

        static bool bMouseTrack() {return s_bMouseTracking;}
        static void setMouseTrack(bool bTrack) {s_bMouseTracking=bTrack;}

        static void setSpinAnimation(bool b) {s_bSpinAnimation=b;}
        static void setSpinDamping(double damp) {s_SpinDamping=damp;}

        static bool bSpinAnimation() {return s_bSpinAnimation;}
        static double spinDamping() {return s_SpinDamping;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

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
//        QSize minimumSizeHint() const override {return QSize(300,200);}

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


        static bool s_bMouseTracking;

        static bool s_bSpinAnimation;
        static double s_SpinDamping;
};

