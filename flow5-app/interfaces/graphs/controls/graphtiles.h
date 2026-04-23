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
#include <QSplitter>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QSettings>
#include <QListWidgetItem>

#include <core/enums_core.h>
#include <interfaces/graphs/controls/graphtilevariableset.h>

class MainFrame;
class XPlane;
class XDirect;
class XSail;
class Graph;
class LegendWt;
class GraphWt;
class GraphTileCtrls;


class GraphTiles : public QWidget
{
    Q_OBJECT

    public:
        GraphTiles(int nGraphs,QWidget *parent = nullptr);
        ~GraphTiles();

        void connectSignals();

        int nGraphWts() const {return m_GraphWidget.count();}
        int graphWtCount() const {return m_GraphWidget.count();}
        int activeGraphIndex() const {return m_iActiveGraphWidget;}
        Graph *activeGraph();
        Graph *graph(int iGraph);
        GraphWt *graphWt(int iGraph);
        GraphWt *graphWt(Graph *pGraph);
        GraphWt *activeGraphWt();
        int activeGraphWtIndex() const {return m_iActiveGraphWidget;}

        void setSingleGraphOrientation(Qt::Orientation orientation) {m_SingleGraphOrientation=orientation;}
        void setContextMenu(QMenu *pMenu) {m_pCtxMenu=pMenu;}

        void setLegendWt(LegendWt *pLegendWt);
        LegendWt *legendWt() const {return m_pLegendWt;}
        void setMaxGraphs(int nMaxGraphs);

        void setGraphControls(GraphTileCtrls *pCtrls);
        GraphTileCtrls *graphControls() {return m_pGraphControls;}
        void updateControls();

        xfl::enumGraphView iView() const {return m_eGraphView;}
        void setView(xfl::enumGraphView eview) {m_eGraphView=eview;}

        bool isOneGraph()   const {return m_eGraphView==xfl::ONEGRAPH;}
        bool isTwoGraphs()  const {return m_eGraphView==xfl::TWOGRAPHS;}
        bool isFourGraphs() const {return m_eGraphView==xfl::FOURGRAPHS;}
        bool isAllGraphs()  const {return m_eGraphView==xfl::ALLGRAPHS;}

        void setOneGraph()   {m_eGraphView=xfl::ONEGRAPH;}
        void setTwoGraphs()  {m_eGraphView=xfl::TWOGRAPHS;}
        void setFourGraphs() {m_eGraphView=xfl::FOURGRAPHS;}
        void setAllGraphs()  {m_eGraphView=xfl::ALLGRAPHS;}

        void showInGraphLegend(bool bVisible);

        void exportGraphToSVG(QString &tempfilepath);

        int activeLayout() const {return m_iActiveVariableSet;}
        void setVariableSet(int index);
        void saveActiveSet();
        GraphTileVariableSet &currentVariableSet() {return m_VariableSet[m_iActiveVariableSet];}
        QVector<GraphTileVariableSet> const & variableSets() const {return m_VariableSet;}
        GraphTileVariableSet const & variableSet(int il) const {return m_VariableSet.at(il);}
        int variableSetCount() const {return m_VariableSet.size();}
        void removeVariableSet(int index);
        void appendVariableSet(const QString &setname, int *xvar, int *yvar);
        void duplicateVariableSet(int index, bool bBefore);
        void setVariableSetName(int index, QString const &newname);
        void moveSetDown(int index);
        void moveSetUp(int index);

        void setupMainLayout();
        void set1SplitterOrientation(Qt::Orientation orientation) {m_p1GraphHSplitter->setOrientation(orientation);}


        void setGraphList(QVector<Graph*>pGraphList);
        void setGraphLayout(int nGraphs, int iGraphWidget=-1, Qt::Orientation orientation =Qt::Horizontal);
        void makeLegend(bool bHighlight);

        void loadSettings(QSettings &settings, QString const &groupname);
        void saveSettings(QSettings &settings, QString const &groupname);

        static void setMainFrame(MainFrame *pMainFrame) {s_pMainFrame=pMainFrame;}
        static void setXPlane(XPlane*pXPlane)           {s_pXPlane=pXPlane;}
        static void setXSail(XSail *pXSail)             {s_pXSail=pXSail;}
        static void setXDirect(XDirect *pXDirect)       {s_pXDirect=pXDirect;}

    protected:
        void adjustLayout();

        void makeSplitters();

        void keyPressEvent(QKeyEvent *event) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;

        void setSingleGraph(int iGraph);
        void clearSplitter(QSplitter *pSplitter);


        void setOneGraphLayout();
        void setTwoGraphsLayout();
        void setFourGraphsLayout();
        void setAllGraphsLayout();

        void setSplitterDefaultSizes();

        void setOneGraphDefaultSizes();
        void setTwoGraphsDefaultSizes();
        void setFourGraphsDefaultSizes();
        void setAllGraphsDefaultSizes();


    protected slots:
        void onGraphChanged(Graph *pGraph = nullptr);

        void onAllGraphSettings();

        void onGraph0();
        void onGraph1();
        void onGraph2();
        void onGraph3();
        void onGraph4();
        void onSingleGraph();
        void onTwoGraphs();
        void onFourGraphs();
        void onAllGraphs();

        void onSave1GraphSplitterSizes();
        void onSave2GraphSplitterSizes();
        void onSave4GraphSplitterSizes();
        void onSaveAllGraphSplitterSizes();

        void on4GraphsSubSubHSplitter1Moved();
        void on4GraphsSubSubHSplitter2Moved();
        void onAllGraphsSubSubHSplitter1Moved();
        void onAllGraphsSubSubHSplitter2Moved();


    public slots:
        void onExportGraphDataToClipboard();
        void onExportGraphDataToFile();
        void onCurGraphSettings();
        void onResetCurGraphScales();
        void onResetSplitters();

    signals:
        void varSetChanged(int);
        void graphChanged(int);

    protected:

        xfl::enumGraphView m_eGraphView;    /**< defines how many graphs will be displayed in the tile view */

        QList<GraphWt*>m_GraphWidget;
        LegendWt *m_pLegendWt;
        QScrollArea *m_pScrollArea;

        GraphTileCtrls *m_pGraphControls;

        int m_nMaxGraphs;
        int m_nGraphWidgets;
        int m_iActiveGraphWidget;

        Qt::Orientation m_SingleGraphOrientation;

        QMenu *m_pCtxMenu;

        QVector<GraphTileVariableSet> m_VariableSet;
        int m_iActiveVariableSet;

        // layout
        QHBoxLayout *m_pMainHBoxLayout;


        QSplitter *m_p1GraphHSplitter = nullptr;
        QSplitter *m_p2GraphsSubHSplitter = nullptr,  *m_p2GraphsVSplitter = nullptr;
        QSplitter *m_p4GraphsHSplitter = nullptr,  *m_p4GraphsSubVSplitter = nullptr,  *m_p4GraphsSubSubHSplitter1 = nullptr,  *m_p4GraphsSubSubHSplitter2 = nullptr;
        QSplitter *m_pAllGraphsVSplitter = nullptr, *m_pAllGraphsSubHSplitter1 = nullptr, *m_pAllGraphsSubHSplitter2 = nullptr;

        QByteArray m_1GraphHSizes;
        QByteArray m_2GraphsSubHSizes,  m_2GraphsVSizes;
        QByteArray m_4GraphsHSizes,     m_4GraphsSubVSizes,    m_4GraphsSubSubHSizes;
        QByteArray m_AllGraphsVSizes,   m_AllGraphsSubHSizes;

        static MainFrame *s_pMainFrame;
        static XDirect *s_pXDirect;
        static XPlane *s_pXPlane;
        static XSail *s_pXSail;

};

