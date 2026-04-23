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


#include <QKeyEvent>
#include <QSplitter>
#include <QHBoxLayout>
#include <QAction>
#include <QMenu>

#include "graphtiles.h"

#include <globals/mainframe.h>
#include <interfaces/graphs/controls/graphtilectrls.h>
#include <core/displayoptions.h>
#include <core/saveoptions.h>
#include <interfaces/graphs/containers/graphwt.h>
#include <interfaces/graphs/containers/legendwt.h>
#include <interfaces/graphs/controls/graphtilevariableset.h>
#include <interfaces/graphs/controls/graphdlg.h>
#include <interfaces/graphs/globals/graph_globals.h>
#include <interfaces/graphs/globals/graphsvgwriter.h>
#include <interfaces/graphs/graph/graph.h>

MainFrame* GraphTiles::s_pMainFrame(nullptr);
XPlane* GraphTiles::s_pXPlane(nullptr);
XSail* GraphTiles::s_pXSail(nullptr);
XDirect* GraphTiles::s_pXDirect(nullptr);


GraphTiles::GraphTiles(int nGraphs, QWidget *parent) : QWidget(parent)
{
    setCursor(Qt::CrossCursor);
    m_pLegendWt = nullptr;
    m_pGraphControls = nullptr;
    m_pCtxMenu = nullptr;

    m_eGraphView = xfl::ALLGRAPHS;

    m_nMaxGraphs = nGraphs;

    for(int iGraph=0; iGraph<nGraphs; iGraph++) m_GraphWidget.append(new GraphWt(this));
    m_nGraphWidgets = nGraphs;


    m_VariableSet.resize(1);
    m_VariableSet.front().setName("Main set");
    m_iActiveVariableSet = 0;


    m_nGraphWidgets = 0;
    m_iActiveGraphWidget = -1;

    m_SingleGraphOrientation = Qt::Horizontal;

    m_pScrollArea = new QScrollArea;
    m_pScrollArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setCursor(Qt::ArrowCursor);

    makeSplitters();

}


GraphTiles::~GraphTiles()
{
    if(m_pScrollArea) delete m_pScrollArea;
}


void GraphTiles::contextMenuEvent(QContextMenuEvent *pEvent)
{
    if(m_pCtxMenu)
        m_pCtxMenu->exec(pEvent->globalPos());
}


Graph *GraphTiles::graph(int iGraph)
{
    if(iGraph<0 || iGraph>=m_GraphWidget.count()) return nullptr;

    return m_GraphWidget.at(iGraph)->graph();
}


GraphWt *GraphTiles::graphWt(int iGraph)
{
    if(iGraph<0 || iGraph>=m_GraphWidget.count()) return nullptr;

    return m_GraphWidget.at(iGraph);
}


GraphWt *GraphTiles::graphWt(Graph *pGraph)
{
    for(int igw=0; igw<m_GraphWidget.count(); igw++)
    {
        if(m_GraphWidget.at(igw)->graph()==pGraph) return m_GraphWidget[igw];
    }
    return nullptr;
}


void GraphTiles::setLegendWt(LegendWt *pLegendWt)
{
    m_pLegendWt=pLegendWt;
    m_pLegendWt->setParent(this);
}


void GraphTiles::setMaxGraphs(int nMaxGraphs)
{
    m_nMaxGraphs = nMaxGraphs;
    m_pGraphControls->setMaxGraphs(nMaxGraphs);
}


void GraphTiles::setGraphControls(GraphTileCtrls *pCtrls)
{
    m_pGraphControls=pCtrls;
    m_pGraphControls->setGraphTileWt(this);
    m_pGraphControls->setupLayout();
    m_pGraphControls->connectSignals();
}


void GraphTiles::keyPressEvent(QKeyEvent *pEvent)
{
//    bool bShift = false;
    bool bCtrl  = false;
    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_1:
        {
            if(bCtrl) pEvent->ignore();
            else onGraph0();
            return;
        }
        case Qt::Key_2:
        {
            if(bCtrl) pEvent->ignore();
            else onGraph1();
            return;
        }
        case Qt::Key_3:
        {
            if(bCtrl) pEvent->ignore();
            else onGraph2();
            return;
        }
        case Qt::Key_4:
        {
            if(bCtrl) pEvent->ignore();
            else onGraph3();
            return;
        }
        case Qt::Key_5:
        {
            if(bCtrl) pEvent->ignore();
            else onGraph4();
            return;
        }
/*        case Qt::Key_6:
        {
            if(bCtrl) pEvent->ignore();
            else setSingleGraph(5);
            return;
        }*/
        case Qt::Key_T:
        {
            onTwoGraphs();
            return;
        }
        case Qt::Key_F:
        {
            onFourGraphs();
            return;
        }
        case Qt::Key_A:
        {
            onAllGraphs();
            return;
        }
        case Qt::Key_R:
        {
            if(bCtrl)
                onResetSplitters();
            return;
        }
        default:
            break;
    }
    pEvent->ignore();

}


void GraphTiles::onGraph0()
{
    setSingleGraph(0);
}


void GraphTiles::onGraph1()
{
    if(m_nMaxGraphs<2) return;
    setSingleGraph(1);
}


void GraphTiles::onGraph2()
{
    if(m_nMaxGraphs<3) return;
    setSingleGraph(2);
}


void GraphTiles::onGraph3()
{
    if(m_nMaxGraphs<4) return;
    setSingleGraph(3);
}


void GraphTiles::onGraph4()
{
    if(m_nMaxGraphs<5) return;
    setSingleGraph(4);
}


void GraphTiles::onCurGraphSettings()
{
    if(!isVisible()) return;
    if(activeGraphWt())
    {
        activeGraphWt()->onGraphSettings();
    }
    setFocus();
}


void GraphTiles::onExportGraphDataToClipboard()
{
    if(!isVisible()) return;

    Graph *pGraph = activeGraph();
    if(!pGraph) return;
    pGraph->toClipboard();
    setFocus();
}


void GraphTiles::onExportGraphDataToFile()
{
    if(!isVisible()) return;

    Graph const *pGraph = activeGraph();
    if(!pGraph) return;
    exportGraphDataToFile(pGraph);
    setFocus();
}


void GraphTiles::exportGraphToSVG(QString &tempfilepath)
{
    if(!isVisible()) return;

    Graph const *pGraph = activeGraph();
    GraphWt const *pGraphWt = activeGraphWt();
    if(!pGraphWt ||!pGraph) return;
    exportGraphToSvg(pGraphWt, pGraph, tempfilepath);

    setFocus();
}


Graph *GraphTiles::activeGraph()
{
    for(int igw=0; igw<m_GraphWidget.count(); igw++)
    {
        if(m_GraphWidget.at(igw)->isVisible())
        {
            if(m_GraphWidget.at(igw)->hasFocus())
                return m_GraphWidget.at(igw)->graph();
        }
    }
//    if(m_iActiveGraphWidget>=0 && m_iActiveGraphWidget<m_GraphWidget.size()) return m_GraphWidget.at(m_iActiveGraphWidget)->graph();
    return nullptr;
}


GraphWt *GraphTiles::activeGraphWt()
{
    for(int igw=0; igw<m_GraphWidget.count(); igw++)
    {
        if(m_GraphWidget.at(igw)->isVisible()&& m_GraphWidget.at(igw)->hasFocus())
            return m_GraphWidget.at(igw);
    }
    return nullptr;
}



void GraphTiles::clearSplitter(QSplitter *pSplitter)
{
    int iter=0;
    while(pSplitter->count() && iter<10)
    {
        QWidget *pWidget = pSplitter->widget(pSplitter->count()-1);
        if(pWidget) pWidget->setParent(nullptr);
        iter++;
    }
}


void GraphTiles::on4GraphsSubSubHSplitter1Moved()
{
    m_p4GraphsSubSubHSplitter2->setSizes(m_p4GraphsSubSubHSplitter1->sizes());
    onSave4GraphSplitterSizes();
}


void GraphTiles::on4GraphsSubSubHSplitter2Moved()
{
    m_p4GraphsSubSubHSplitter1->setSizes(m_p4GraphsSubSubHSplitter2->sizes());
    onSave4GraphSplitterSizes();
}


void GraphTiles::onAllGraphsSubSubHSplitter1Moved()
{
    m_pAllGraphsSubHSplitter2->setSizes(m_pAllGraphsSubHSplitter1->sizes());
    onSaveAllGraphSplitterSizes();
}


void GraphTiles::onAllGraphsSubSubHSplitter2Moved()
{
    m_pAllGraphsSubHSplitter1->setSizes(m_pAllGraphsSubHSplitter2->sizes());
    onSaveAllGraphSplitterSizes();
}


void GraphTiles::onSave1GraphSplitterSizes()
{
    m_1GraphHSizes = m_p1GraphHSplitter->saveState();
}


void GraphTiles::onSave2GraphSplitterSizes()
{
    m_2GraphsVSizes = m_p2GraphsVSplitter->saveState();
    m_2GraphsSubHSizes = m_p2GraphsSubHSplitter->saveState();
}


void GraphTiles::onSave4GraphSplitterSizes()
{
    m_4GraphsSubSubHSizes = m_p4GraphsSubSubHSplitter1->saveState();
    m_4GraphsSubVSizes = m_p4GraphsSubVSplitter->saveState();
    m_4GraphsHSizes = m_p4GraphsHSplitter->saveState();
}


void GraphTiles::onSaveAllGraphSplitterSizes()
{
    m_AllGraphsVSizes = m_pAllGraphsVSplitter->saveState();
    m_AllGraphsSubHSizes = m_pAllGraphsSubHSplitter1->saveState();
}


void GraphTiles::setOneGraphLayout()
{
    clearSplitter(m_p1GraphHSplitter);
    m_p1GraphHSplitter->addWidget(m_GraphWidget.at(m_iActiveGraphWidget));
    m_p1GraphHSplitter->addWidget(m_pScrollArea);

    if(m_1GraphHSizes.length()==0)
    {
        int w = width();
        QList<int> sizes = {int(double(w)*3./4.),  w - int(double(w)*3./4.)};
        m_p1GraphHSplitter->setSizes(sizes);
    }
    else
    {
        m_p1GraphHSplitter->restoreState(m_1GraphHSizes);
    }

    m_p1GraphHSplitter->show();
}


void GraphTiles::setTwoGraphsLayout()
{
    clearSplitter(m_p2GraphsSubHSplitter);
    clearSplitter(m_p2GraphsSubHSplitter);
    clearSplitter(m_p2GraphsVSplitter);

    m_p2GraphsSubHSplitter->addWidget(m_GraphWidget.at(0));
    m_p2GraphsSubHSplitter->addWidget(m_GraphWidget.at(1));
    m_p2GraphsVSplitter->addWidget(m_p2GraphsSubHSplitter);
    m_p2GraphsVSplitter->addWidget(m_pScrollArea);

    if(m_2GraphsVSizes.length()==0)
    {
        int h = height();
        QList<int> sizes ={  int(double(h)*3./4.), h-int(double(h)*3./4.)};
        m_p2GraphsVSplitter->setSizes(sizes);
    }
    else {
        m_p2GraphsVSplitter->restoreState(m_2GraphsVSizes);
    }

    m_p2GraphsSubHSplitter->restoreState(m_2GraphsSubHSizes);
    m_p2GraphsVSplitter->show();
}


void GraphTiles::setFourGraphsLayout()
{
/*    for(int igw=0; igw<m_graphWidget.count(); igw++)
        m_graphWidget.at(igw)->setVisible(igw==0 || igw==1 || igw==2 || igw==3);*/

    clearSplitter(m_p4GraphsSubSubHSplitter1);
    clearSplitter(m_p4GraphsSubSubHSplitter2);
    clearSplitter(m_p4GraphsSubVSplitter);
    clearSplitter(m_p4GraphsHSplitter);

    m_p4GraphsSubSubHSplitter1->addWidget(m_GraphWidget.at(0));
    m_p4GraphsSubSubHSplitter1->addWidget(m_GraphWidget.at(1));
    m_p4GraphsSubSubHSplitter2->addWidget(m_GraphWidget.at(2));
    m_p4GraphsSubSubHSplitter2->addWidget(m_GraphWidget.at(3));

    m_p4GraphsSubVSplitter->addWidget(m_p4GraphsSubSubHSplitter1);
    m_p4GraphsSubVSplitter->addWidget(m_p4GraphsSubSubHSplitter2);
    m_p4GraphsHSplitter->addWidget(m_p4GraphsSubVSplitter);
    m_p4GraphsHSplitter->addWidget(m_pScrollArea);

    if(m_4GraphsSubSubHSizes.length()==0)
    {
        int w = width();

        QList<int> sizes = {int(double(w)*1.5/4.), int(double(w)*1.5/4.)};
        m_p4GraphsSubSubHSplitter1->setSizes(sizes);
        m_p4GraphsSubSubHSplitter2->setSizes(sizes);
    }
    else
    {
        m_p4GraphsSubSubHSplitter1->restoreState(m_4GraphsSubSubHSizes);
        m_p4GraphsSubSubHSplitter2->restoreState(m_4GraphsSubSubHSizes);
    }

    if(m_4GraphsHSizes.length()==0)
    {
        int w = width();
        QList<int> sizes = {int(double(w)*3./4.), w-int(double(w)*3./4.)};
        m_p4GraphsHSplitter->setSizes(sizes);
    }
    else m_p4GraphsHSplitter->restoreState(m_4GraphsHSizes);

    m_p4GraphsSubVSplitter->restoreState(m_4GraphsSubVSizes);

    m_p4GraphsHSplitter->show();
}


void GraphTiles::setAllGraphsLayout()
{
    clearSplitter(m_pAllGraphsSubHSplitter1);
    clearSplitter(m_pAllGraphsSubHSplitter2);

    m_pAllGraphsSubHSplitter1->addWidget(m_GraphWidget.at(0));
    m_pAllGraphsSubHSplitter1->addWidget(m_GraphWidget.at(1));
    m_pAllGraphsSubHSplitter1->addWidget(m_GraphWidget.at(2));
    m_pAllGraphsSubHSplitter2->addWidget(m_pScrollArea);
    m_pAllGraphsSubHSplitter2->addWidget(m_GraphWidget.at(3));
    m_pAllGraphsSubHSplitter2->addWidget(m_GraphWidget.at(4));

    m_pAllGraphsSubHSplitter1->setChildrenCollapsible(false);
    m_pAllGraphsSubHSplitter2->setChildrenCollapsible(false);

    if(m_AllGraphsSubHSizes.length()==0)
    {
        int w = width();
        QList<int> sizes = {int(double(w)*5./10.0), int(double(w)*2.5/10.0), int(double(w)*2.5/10.0)};
        m_pAllGraphsSubHSplitter1->setSizes(sizes);
        m_pAllGraphsSubHSplitter2->setSizes(sizes);
    }
    else
    {
        m_pAllGraphsSubHSplitter1->restoreState(m_AllGraphsSubHSizes);
        m_pAllGraphsSubHSplitter2->restoreState(m_AllGraphsSubHSizes);
    }

    m_pAllGraphsVSplitter->restoreState(m_AllGraphsVSizes);

    m_pAllGraphsVSplitter->show();
}


void GraphTiles::makeSplitters()
{
    m_pMainHBoxLayout = new QHBoxLayout;

    m_p1GraphHSplitter = new QSplitter(Qt::Horizontal);
    m_p1GraphHSplitter->setHandleWidth(1);
    m_p1GraphHSplitter->setChildrenCollapsible(false);

    m_p2GraphsVSplitter = new QSplitter(Qt::Vertical);
    {
        m_p2GraphsVSplitter->setHandleWidth(1);
        m_p2GraphsVSplitter->setChildrenCollapsible(false);
        m_p2GraphsSubHSplitter = new QSplitter(Qt::Horizontal, this);
        m_p2GraphsSubHSplitter->setHandleWidth(1);
        m_p2GraphsSubHSplitter->setChildrenCollapsible(false);

        m_p2GraphsVSplitter->addWidget(m_p2GraphsSubHSplitter);
    }

    m_p4GraphsHSplitter= new QSplitter(Qt::Horizontal);
    {
        m_p4GraphsHSplitter->setHandleWidth(1);
        m_p4GraphsHSplitter->setChildrenCollapsible(false);
        m_p4GraphsSubVSplitter = new QSplitter(Qt::Vertical);
        {
            m_p4GraphsSubVSplitter->setHandleWidth(1);
            m_p4GraphsSubVSplitter->setChildrenCollapsible(false);

            m_p4GraphsSubSubHSplitter1 = new QSplitter(Qt::Horizontal);
            m_p4GraphsSubSubHSplitter1->setHandleWidth(1);
            m_p4GraphsSubSubHSplitter1->setChildrenCollapsible(false);

            m_p4GraphsSubSubHSplitter2 = new QSplitter(Qt::Horizontal);
            m_p4GraphsSubSubHSplitter2->setHandleWidth(1);
            m_p4GraphsSubSubHSplitter2->setChildrenCollapsible(false);

            m_p4GraphsSubVSplitter->addWidget(m_p4GraphsSubSubHSplitter1);
            m_p4GraphsSubVSplitter->addWidget(m_p4GraphsSubSubHSplitter2);
        }
        m_p4GraphsHSplitter->addWidget(m_p4GraphsSubVSplitter);
    }

    m_pAllGraphsVSplitter = new QSplitter(Qt::Vertical, this);
    {
        m_pAllGraphsVSplitter->setHandleWidth(1);
        m_pAllGraphsVSplitter->setChildrenCollapsible(false);

        m_pAllGraphsSubHSplitter1 = new QSplitter(Qt::Horizontal, this);
        m_pAllGraphsSubHSplitter1->setHandleWidth(1);
        m_pAllGraphsSubHSplitter1->setChildrenCollapsible(false);

        m_pAllGraphsSubHSplitter2 = new QSplitter(Qt::Horizontal, this);
        m_pAllGraphsSubHSplitter2->setHandleWidth(1);
        m_pAllGraphsSubHSplitter2->setChildrenCollapsible(false);

        m_pAllGraphsVSplitter->addWidget(m_pAllGraphsSubHSplitter1);
        m_pAllGraphsVSplitter->addWidget(m_pAllGraphsSubHSplitter2);
    }

    m_pScrollArea->setParent(nullptr);
    for(int i=0; i<m_GraphWidget.size(); i++)
        m_GraphWidget.at(i)->setParent(nullptr);
}


void GraphTiles::connectSignals()
{
    connect(m_p1GraphHSplitter,         SIGNAL(splitterMoved(int,int)), SLOT(onSave1GraphSplitterSizes()));

    connect(m_p2GraphsVSplitter,        SIGNAL(splitterMoved(int,int)), SLOT(onSave2GraphSplitterSizes()));
    connect(m_p2GraphsSubHSplitter,     SIGNAL(splitterMoved(int,int)), SLOT(onSave2GraphSplitterSizes()));

    connect(m_p4GraphsHSplitter,        SIGNAL(splitterMoved(int,int)), SLOT(onSave4GraphSplitterSizes()));
    connect(m_p4GraphsSubVSplitter,     SIGNAL(splitterMoved(int,int)), SLOT(onSave4GraphSplitterSizes()));
    connect(m_p4GraphsSubSubHSplitter1, SIGNAL(splitterMoved(int,int)), SLOT(on4GraphsSubSubHSplitter1Moved()));
    connect(m_p4GraphsSubSubHSplitter2, SIGNAL(splitterMoved(int,int)), SLOT(on4GraphsSubSubHSplitter2Moved()));

    connect(m_pAllGraphsVSplitter,      SIGNAL(splitterMoved(int,int)), SLOT(onSaveAllGraphSplitterSizes()));
    connect(m_pAllGraphsSubHSplitter1,  SIGNAL(splitterMoved(int,int)), SLOT(onAllGraphsSubSubHSplitter1Moved()));
    connect(m_pAllGraphsSubHSplitter2,  SIGNAL(splitterMoved(int,int)), SLOT(onAllGraphsSubSubHSplitter2Moved()));

    for(int igw=0; igw<m_GraphWidget.count(); igw++)
    {
        connect(m_GraphWidget.at(igw), SIGNAL(graphChanged(Graph*)),       this,       SLOT(onGraphChanged(Graph*)));
    }

}


/** only identified solution to have correct splitter sizes the first time the app is launched */
void GraphTiles::resizeEvent(QResizeEvent *pEvent)
{
//    setSplitterDefaultSizes();
    QWidget::resizeEvent(pEvent);
}


void GraphTiles::setSplitterDefaultSizes()
{
    setOneGraphDefaultSizes();
    setTwoGraphsDefaultSizes();
    setFourGraphsDefaultSizes();
    setAllGraphsDefaultSizes();
}


void GraphTiles::setOneGraphDefaultSizes()
{
    int w = width();
    QList<int> GraphHSizes;
    double frac = 3./4.;
    GraphHSizes.push_back(int(double(w)*frac));
    GraphHSizes.push_back(w - int(double(w)*frac));
    m_p1GraphHSplitter->setSizes(GraphHSizes);
    m_1GraphHSizes = m_p1GraphHSplitter->saveState();
}


void GraphTiles::setTwoGraphsDefaultSizes()
{
    int w = width();
    int h = height();
    QList<int> GraphsVSizes2;
    GraphsVSizes2.push_back(  int(double(h)*3./4.));
    GraphsVSizes2.push_back(h-int(double(h)*3./4.));
    m_p2GraphsVSplitter->setSizes(GraphsVSizes2);
    m_2GraphsVSizes = m_p2GraphsVSplitter->saveState();

    QList<int> GraphsSubHSizes2;
    GraphsSubHSizes2.push_back(int(double(w)/2.));
    GraphsSubHSizes2.push_back(int(double(w)/2.));
    m_p2GraphsSubHSplitter->setSizes(GraphsSubHSizes2);
    m_2GraphsSubHSizes = m_p2GraphsSubHSplitter->saveState();
}


void GraphTiles::setFourGraphsDefaultSizes()
{
    int w = width();
    int h = height();
    QList<int> FourGraphsHSizes;
    FourGraphsHSizes.push_back(  int(double(w)*3./4.));
    FourGraphsHSizes.push_back(w-int(double(w)*3./4.));
    m_p4GraphsHSplitter->setSizes(FourGraphsHSizes);
    m_4GraphsHSizes = m_p4GraphsHSplitter->saveState();

    QList<int> FourGraphsSubVSizes;
    FourGraphsSubVSizes.push_back(int(double(h)/2.0));
    FourGraphsSubVSizes.push_back(int(double(h)/2.0));
    m_p4GraphsSubVSplitter->setSizes(FourGraphsSubVSizes);
    m_p4GraphsSubVSplitter->setSizes(FourGraphsSubVSizes);
    m_4GraphsSubVSizes = m_p4GraphsSubVSplitter->saveState();


    QList<int> FourGraphsSubSubHSizes;
    FourGraphsSubSubHSizes.push_back(int(double(w)*1.5/4.));
    FourGraphsSubSubHSizes.push_back(int(double(w)*1.5/4.));
    m_p4GraphsSubSubHSplitter1->setSizes(FourGraphsSubSubHSizes);
    m_p4GraphsSubSubHSplitter2->setSizes(FourGraphsSubSubHSizes);
    m_4GraphsSubSubHSizes = m_p4GraphsSubSubHSplitter1->saveState();
}


void GraphTiles::setAllGraphsDefaultSizes()
{
    int w = width();
    int h = height();
    QList<int> AllGraphsVSizes;
    AllGraphsVSizes.push_back(int(double(h)/2.));
    AllGraphsVSizes.push_back(int(double(h)/2.));
    m_pAllGraphsVSplitter->setSizes(AllGraphsVSizes);
    m_AllGraphsVSizes = m_pAllGraphsVSplitter->saveState();

    QList<int> AllGraphsSubHSizes;
    AllGraphsSubHSizes.push_back(int(double(w)*5./10.0));
    AllGraphsSubHSizes.push_back(int(double(w)*2.5/10.0));
    AllGraphsSubHSizes.push_back(int(double(w)*2.5/10.0));
    m_pAllGraphsSubHSplitter1->setSizes(AllGraphsSubHSizes);
    m_pAllGraphsSubHSplitter2->setSizes(AllGraphsSubHSizes);

    m_pAllGraphsSubHSplitter2->restoreState(m_pAllGraphsSubHSplitter1->saveState());
    m_AllGraphsSubHSizes = m_pAllGraphsSubHSplitter1->saveState();
}


void GraphTiles::makeLegend(bool bHighlight)
{
    // Highlighting may not be necessary if all curves have been deselected
    m_pLegendWt->makeLegend(bHighlight);
    m_pScrollArea->setWidget(m_pLegendWt);
}


void GraphTiles::setupMainLayout()
{
    m_p1GraphHSplitter->hide();
    m_p2GraphsVSplitter->hide();
    m_p4GraphsHSplitter->hide();
    m_pAllGraphsVSplitter->hide();

    m_pMainHBoxLayout->addWidget(m_p1GraphHSplitter);
    m_pMainHBoxLayout->addWidget(m_p2GraphsVSplitter);
    m_pMainHBoxLayout->addWidget(m_p4GraphsHSplitter);
    m_pMainHBoxLayout->addWidget(m_pAllGraphsVSplitter);

    setLayout(m_pMainHBoxLayout);
}


void GraphTiles::adjustLayout()
{
    blockSignals(true);

    m_p1GraphHSplitter->hide();
    m_p2GraphsVSplitter->hide();
    m_p4GraphsHSplitter->hide();
    m_pAllGraphsVSplitter->hide();

    if     (m_nGraphWidgets==1) setOneGraphLayout();
    else if(m_nGraphWidgets==2) setTwoGraphsLayout();
    else if(m_nGraphWidgets==4) setFourGraphsLayout();
    else                        setAllGraphsLayout();

    if(m_pLegendWt)
    {
        m_pLegendWt->setStyleSheet(QString::asprintf("background: %s;", DisplayOptions::backgroundColor().name(QColor::HexRgb).toStdString().c_str()));
        m_pLegendWt->setAutoFillBackground(true);
    }

    blockSignals(false);
}


void GraphTiles::showInGraphLegend(bool bVisible)
{
    if(activeGraphWt())
    {
        activeGraphWt()->showLegend(bVisible);
    }
    setFocus();
}


void GraphTiles::onResetCurGraphScales()
{
    if(!isVisible()) return;

    if(activeGraphWt())
    {
        activeGraphWt()->onResetGraphScales();
    }
    setFocus();
}


void GraphTiles::setGraphList(QVector<Graph *> pGraphList)
{
    for(int ig=0; ig<qMin(MAXGRAPHS, pGraphList.count()); ig++)
        m_GraphWidget.at(ig)->setGraph(pGraphList.at(ig));
}


void GraphTiles::setGraphLayout(int nGraphs, int iGraphWidget, Qt::Orientation orientation)
{
    m_nGraphWidgets = std::min(nGraphs,MAXGRAPHS);

    if(iGraphWidget>=0 && iGraphWidget<nGraphs) m_iActiveGraphWidget = iGraphWidget;

    if(m_pLegendWt)
    {
        m_pLegendWt->setGraph(m_GraphWidget.front()->graph());
        m_pLegendWt->setStyleSheet(QString::asprintf("background: %s;", DisplayOptions::backgroundColor().name(QColor::HexRgb).toStdString().c_str()));
        m_pLegendWt->setAutoFillBackground(true);
    }
    m_SingleGraphOrientation = orientation;

    adjustLayout();

    m_GraphWidget.at(0)->setFocus();
    update();
}


void GraphTiles::onSingleGraph()
{
    if(!isVisible()) return;

    QAction *pAction = qobject_cast<QAction *>(sender());
    if (!pAction) return;
    int iGraph = pAction->data().toInt();
    setSingleGraph(iGraph);
}


void GraphTiles::setSingleGraph(int iGraph)
{
    if(iGraph<0 || iGraph>=m_GraphWidget.size()) return;

    m_eGraphView =  xfl::ONEGRAPH;

    m_nGraphWidgets = 1;
    m_iActiveGraphWidget = iGraph;
    if(m_pLegendWt)
    {
        m_pLegendWt->setGraph(m_GraphWidget.at(iGraph)->graph());
    }

    adjustLayout();

    m_pGraphControls->checkGraphActions();

    update();
    setFocus();
}


void GraphTiles::onTwoGraphs()
{
    if(!isVisible()) return;

    if(m_nMaxGraphs<2) return;

    m_eGraphView =  xfl::TWOGRAPHS;
    m_nGraphWidgets = 2;
    m_iActiveGraphWidget = 0;
    if(m_pLegendWt)
    {
        m_pLegendWt->setGraph(m_GraphWidget.at(0)->graph());
    }

    adjustLayout();
    m_pGraphControls->checkGraphActions();

    update();
    setFocus();
}


void GraphTiles::onFourGraphs()
{
    if(!isVisible()) return;   

    if(m_nMaxGraphs<4) return;

    m_eGraphView = xfl::FOURGRAPHS;
    m_nGraphWidgets = 4;
    m_iActiveGraphWidget = 0;
    if(m_pLegendWt)
    {
        m_pLegendWt->setGraph(m_GraphWidget.at(0)->graph());
    }

    adjustLayout();
    m_pGraphControls->checkGraphActions();

    update();
    setFocus();
}


void GraphTiles::onAllGraphs()
{
    if(!isVisible()) return;

    if(m_nMaxGraphs<5) return;

    m_eGraphView =  xfl::ALLGRAPHS;

    m_nGraphWidgets = 5;
    m_iActiveGraphWidget = 0;
    if(m_pLegendWt)
    {
        m_pLegendWt->setGraph(m_GraphWidget.at(0)->graph());
    }

    adjustLayout();
    m_pGraphControls->checkGraphActions();

    update();
    setFocus();
}


void GraphTiles::onAllGraphSettings()
{
    if(!isVisible()) return;

    GraphDlg grDlg(this);
    grDlg.setActivePage(1);

    grDlg.setGraph(m_GraphWidget.at(0)->graph());

    if(grDlg.exec() == QDialog::Accepted)
    {

        for(int ig=1; ig<m_GraphWidget.size(); ig++)
        {
            Graph *pGraph = m_GraphWidget.at(ig)->graph();
            if(pGraph)
            {
                pGraph->copySettings(m_GraphWidget.at(0)->graph());
            }
        }
    }

    update();
    setFocus();
}


void GraphTiles::onGraphChanged(Graph *pGraph)
{
    if(!pGraph) return;
    saveActiveSet();
    emit graphChanged(m_iActiveGraphWidget);
}


void GraphTiles::loadSettings(QSettings &settings, const QString &groupname)
{
    settings.beginGroup(groupname);
    {
        int n = settings.value("GraphView", 0).toInt();
        switch (n)
        {
            case 1: m_eGraphView = xfl::ONEGRAPH;        break;
            case 2: m_eGraphView = xfl::TWOGRAPHS;       break;
            case 4: m_eGraphView = xfl::FOURGRAPHS;      break;
            default:
            case 0: m_eGraphView = xfl::ALLGRAPHS;       break;
        }
        m_iActiveGraphWidget = settings.value("iActiveGraph", 0).toInt();

        m_1GraphHSizes        = settings.value("1GraphHSizes").toByteArray();

        m_2GraphsVSizes       = settings.value("2GraphsVSizes").toByteArray();
        m_2GraphsSubHSizes    = settings.value("2GraphsSubHSizes").toByteArray();

        m_4GraphsHSizes       = settings.value("4GraphsHSizes").toByteArray();
        m_4GraphsSubVSizes    = settings.value("4GraphsSubVSizes").toByteArray();
        m_4GraphsSubSubHSizes = settings.value("4GraphsSubSubHSizes").toByteArray();

        m_AllGraphsVSizes     = settings.value("AllGraphsVSizes").toByteArray();
        m_AllGraphsSubHSizes  = settings.value("AllGraphsSubHSizes").toByteArray();

        bool bLayoutExists = settings.contains("NVariableSets");
        if(bLayoutExists)
        {
            int nlayouts = settings.value("NVariableSets").toInt();
            m_VariableSet.resize(nlayouts);
            for(int il=0; il<nlayouts; il++)
            {
                GraphTileVariableSet &layout = m_VariableSet[il];
                QString strange = QString::asprintf("Variable_Set%d", il);
                layout.loadSettings(settings, strange);
            }
        }
    }
    settings.endGroup();
}


void GraphTiles::saveSettings(QSettings &settings, const QString &groupname)
{
    settings.beginGroup(groupname);
    {
        switch (m_eGraphView)
        {
            case xfl::ONEGRAPH:     settings.setValue("GraphView",1);   break;
            case xfl::TWOGRAPHS:    settings.setValue("GraphView",2);   break;
            case xfl::FOURGRAPHS:   settings.setValue("GraphView",4);   break;
            default:
            case xfl::ALLGRAPHS:    settings.setValue("GraphView",0);   break;
        }

        settings.setValue("iActiveGraph", m_iActiveGraphWidget);

        settings.setValue("1GraphHSizes",        m_1GraphHSizes);

        settings.setValue("2GraphsVSizes",       m_2GraphsVSizes);
        settings.setValue("2GraphsSubHSizes",    m_2GraphsSubHSizes);

        settings.setValue("4GraphsHSizes",       m_4GraphsHSizes);
        settings.setValue("4GraphsSubVSizes",    m_4GraphsSubVSizes);
        settings.setValue("4GraphsSubSubHSizes", m_4GraphsSubSubHSizes);

        settings.setValue("AllGraphsVSizes",     m_AllGraphsVSizes);
        settings.setValue("AllGraphsSubHSizes",  m_AllGraphsSubHSizes);

        settings.setValue("NVariableSets", m_VariableSet.size());
        for(int il=0; il<m_VariableSet.size(); il++)
        {
            GraphTileVariableSet &layout = m_VariableSet[il];
            QString strange = QString::asprintf("Variable_Set%d", il);
            layout.saveSettings(settings, strange);
        }
    }
    settings.endGroup();
}

void GraphTiles::onResetSplitters()
{
    if(!isVisible()) return;

    switch(m_eGraphView)
    {
        case xfl::NOGRAPH:
            break;

        case xfl::ONEGRAPH:
            setOneGraphDefaultSizes();
            break;

        case xfl::TWOGRAPHS:
            setTwoGraphsDefaultSizes();
            break;

        case xfl::FOURGRAPHS:
            setFourGraphsDefaultSizes();
            break;

        case xfl::ALLGRAPHS:
            setAllGraphsDefaultSizes();
            break;
    }

    update();
}


void GraphTiles::saveActiveSet()
{
    // save the current layout's variables
    if(m_iActiveVariableSet>=0 && m_iActiveVariableSet<m_VariableSet.size())
    {
        GraphTileVariableSet &activeset = m_VariableSet[m_iActiveVariableSet];
        for(int ig=0; ig<m_GraphWidget.size(); ig++)
        {
            Graph const*pGraph = m_GraphWidget.at(ig)->graph();

            if(pGraph)
            {
                bool bYinv = pGraph->bYInverted(0);
                activeset.setVariables(ig, pGraph->xVariable(), pGraph->yVariable(0), bYinv);
            }
        }
    }
}


void GraphTiles::setVariableSet(int index)
{
    // set the new layout
    if(index<0 || index>=m_VariableSet.size()) return;

    m_iActiveVariableSet=index;

    GraphTileVariableSet const &newvarset = variableSet(index);

    for(int ig=0; ig<nGraphWts(); ig++)
    {
        Graph *pGraph =m_GraphWidget.at(ig)->graph();
        if(pGraph)
        {
            pGraph->setVariables(newvarset.XVar(ig), newvarset.YVar(ig), -1);
            pGraph->setYInverted(0, newvarset.bYInverted(ig));
        }
    }

    emit varSetChanged(index);
    update();
}


void GraphTiles::moveSetDown(int index)
{
    if(index<0 || index>=m_VariableSet.size()-1) return;

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    m_VariableSet.swapItemsAt(index, index+1);
#else
    GraphTileVariableSet varset = m_VariableSet.at(index+1);
    m_VariableSet.removeAt(index+1);
    m_VariableSet.insert(index, varset);
#endif
    m_iActiveVariableSet = index+1;
    updateControls();
}


void GraphTiles::moveSetUp(int index)
{
    if(index<=0 || index>=m_VariableSet.size()) return;
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    m_VariableSet.swapItemsAt(index, index-1);
#else
    GraphTileVariableSet varset = m_VariableSet.at(index-1);
    m_VariableSet.removeAt(index-1);
    m_VariableSet.insert(index, varset);
#endif
    m_iActiveVariableSet = index-1;
    updateControls();
}


void GraphTiles::removeVariableSet(int index)
{
    if(index<0 || index>=m_VariableSet.size()) return;
    m_VariableSet.removeAt(index);
}


void GraphTiles::appendVariableSet(QString const &setname, int *xvar, int *yvar)
{
    GraphTileVariableSet newset;
    newset.setName(setname);
    newset.setVariables(xvar, yvar);
    m_VariableSet.append(newset);
}


void GraphTiles::duplicateVariableSet(int index, bool bBefore)
{
    if(index<0 || index>=m_VariableSet.size()) return;
    m_VariableSet.insert(index, m_VariableSet.at(index));
    if(bBefore) m_VariableSet[index].setName("New set");
    else        m_VariableSet[index+1].setName("New set");
}


void GraphTiles::setVariableSetName(int index, QString const &newname)
{
    if(index<0 || index>=m_VariableSet.size()) return;
    m_VariableSet[index].setName(newname);
}


void GraphTiles::updateControls()
{
    m_pGraphControls->setControls();
    m_pGraphControls->selectActiveVariableSet(m_iActiveVariableSet);
}
