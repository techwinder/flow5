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
#include <QScrollArea>
#include <QSplitter>

#include <api/vector2d.h>
#include <api/linestyle.h>

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




