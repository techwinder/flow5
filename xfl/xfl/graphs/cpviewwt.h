/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <QWidget>
#include <QSettings>
#include <QLabel>
#include <QScrollArea>
#include <QSplitter>

#include <xflgeom/geom2d/vector2d.h>
#include <xflcore/linestyle.h>

class MainFrame;
class XPlane;
class Graph;
class Curve;

class PlaneXfl;
class GraphWt;
class LegendWt;
class CurveModel;

class CpViewWt : public QWidget
{
    Q_OBJECT

    public:
        CpViewWt(QWidget *pParent = nullptr);
        ~CpViewWt() override;

    public:
        Graph *CpGraph() const {return m_pCpGraph;}

        void makeLegend(bool bHighlight);

        void showInGraphLegend(bool bShow);

        void updateUnits();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        static void setMainFrame(MainFrame*pMainFrame) {s_pMainFrame = pMainFrame;}

    protected:
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private:
        void setupLayout();
        void connectSignals();

    public slots:
        void onResetCurGraphScales();
        void onExportGraphDataToFile();
        void onExportGraphDataToClipboard();
        void onCurGraphSettings();
        void onUpdateCpGraph();

    private:
        GraphWt *m_pCpGraphWt;
        LegendWt *m_pLegendWt;
        QScrollArea *m_pScrollArea;

        Graph *m_pCpGraph;
        CurveModel *m_pCpCurveModel;
        QRect m_rGraphRect;

        QSplitter *m_p1GraphHSplitter;

        static QByteArray s_1GraphHSizes;

        static MainFrame *s_pMainFrame;   /**< A void pointer to the instance of the MainFrame object. */
};




