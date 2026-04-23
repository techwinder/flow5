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

#define _MATH_DEFINES_DEFINED


#include <QApplication>


#include <QVBoxLayout>

#include "cpviewwt.h"


#include <core/displayoptions.h>
#include <api/units.h>
#include <core/xflcore.h>
#include <globals/mainframe.h>
#include <interfaces/graphs/containers/graphwt.h>
#include <interfaces/graphs/containers/legendwt.h>
#include <interfaces/graphs/globals/graph_globals.h>
#include <interfaces/graphs/graph/graph.h>
#include <modules/xplane/xplane.h>

MainFrame *CpViewWt::s_pMainFrame = nullptr;

QByteArray CpViewWt::s_1GraphHSizes;


CpViewWt::CpViewWt(QWidget *pParent) : QWidget(pParent)
{
    setupLayout();
    connectSignals();

    m_pCpGraph = new Graph;
    m_pCpCurveModel = new CurveModel;
    m_pCpGraph->setCurveModel(m_pCpCurveModel);
    m_pCpGraph->setName("Cp graph");
    m_pCpGraph->setGraphType(GRAPH::CPGRAPH);
    m_pCpGraph->setXVariableList({"x ("+Units::lengthUnitQLabel() + ")"});
    m_pCpGraph->setYVariableList({"Cp"});
    m_pCpGraph->setXMin( 0.0);
    m_pCpGraph->setXMax( 0.1);
    m_pCpGraph->setYMin(0, -0.01);
    m_pCpGraph->setYMax(0,  0.01);
    m_pCpGraph->setScaleType(GRAPH::RESETTING);
    m_pCpGraph->setMargins(50);
    m_pCpGraph->setYInverted(0, true);
    QFont graphfont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    graphfont.setStyleHint(QFont::SansSerif);
    m_pCpGraph->setTitleFont(graphfont);
    m_pCpGraph->setLabelFont(graphfont);

    m_pCpGraphWt->setGraph(m_pCpGraph);
    m_pLegendWt->setGraph(m_pCpGraph);

//    connect(m_pHoverTimer, SIGNAL(timeout()), this, SLOT(onHovered()));
}


CpViewWt::~CpViewWt()
{
    delete m_pCpGraph;
    delete m_pCpCurveModel;
    m_pCpGraph = nullptr;
    m_pCpCurveModel = nullptr;
    if(m_pScrollArea) delete m_pScrollArea;
}


void CpViewWt::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_p1GraphHSplitter = new QSplitter(Qt::Vertical);
        {
            m_pCpGraphWt = new GraphWt(s_pMainFrame);
            m_pCpGraphWt->enableContextMenu(true);

            m_pLegendWt = new LegendWt(this);
            m_pScrollArea = new QScrollArea;
            m_pScrollArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            m_pScrollArea->setWidgetResizable(true);
            m_pScrollArea->setWidget(m_pLegendWt);

            m_p1GraphHSplitter->addWidget(m_pCpGraphWt);
            m_p1GraphHSplitter->addWidget(m_pScrollArea);
//            m_p1GraphHSplitter->setChildrenCollapsible(false);
            m_p1GraphHSplitter->setCollapsible(0, false);
            m_p1GraphHSplitter->setCollapsible(1, false);
        }
        pMainLayout->addWidget(m_p1GraphHSplitter);
    }
    setLayout(pMainLayout);
}


void CpViewWt::connectSignals()
{
    connect(m_pLegendWt, SIGNAL(updateGraphs()), SLOT(onUpdateCpGraph()));
}


void CpViewWt::showEvent(QShowEvent *pEvent)
{
    m_p1GraphHSplitter->restoreState(s_1GraphHSizes);
    QWidget::showEvent(pEvent);
}


void CpViewWt::hideEvent(QHideEvent *pEvent)
{
    s_1GraphHSizes = m_p1GraphHSplitter->saveState();
    QWidget::hideEvent(pEvent);
}


void CpViewWt::resizeEvent(QResizeEvent *)
{
    if(m_pCpGraph)
    {
        int h = rect().height();
        int h4 = int(h/3.0);
        m_rGraphRect = QRect(0, 0, + rect().width(), rect().height()-h4);
        //        m_pCpGraph->setMargin(50);

        m_pCpGraph->setGraphScales(m_rGraphRect);
    }
    showEvent(nullptr);
}


void CpViewWt::updateUnits()
{
    m_pCpGraph->setXVariableList({"x ("+Units::lengthUnitQLabel() + ")"});
}


void CpViewWt::makeLegend(bool bHighlight)
{
    m_pLegendWt->makeLegend(bHighlight);
    m_pScrollArea->setWidget(m_pLegendWt);
}


void CpViewWt::onResetCurGraphScales()
{
    if(!isVisible()) return;

    m_pCpGraphWt->onResetGraphScales();
    setFocus();
}


void CpViewWt::onExportGraphDataToClipboard()
{
    if(!isVisible()) return;
    if(!m_pCpGraph) return;
    m_pCpGraph->toClipboard();
    setFocus();
}


void CpViewWt::onCurGraphSettings()
{
    if(!isVisible()) return;

    m_pCpGraphWt->onGraphSettings();
    setFocus();
}


void CpViewWt::onExportGraphDataToFile()
{
    if(!isVisible()) return;

    if(!m_pCpGraph) return;
    exportGraphDataToFile(m_pCpGraph);
    setFocus();
}


void CpViewWt::onUpdateCpGraph()
{
    m_pCpGraph->invalidate();
    m_pCpGraphWt->update();
}


void CpViewWt::showInGraphLegend(bool bShow)
{
    m_pCpGraph->setLegendVisible(bShow);
}


void CpViewWt::saveSettings(QSettings &settings)
{
    settings.beginGroup("CpViewWt");
    {
        settings.setValue("1GraphHSizes", s_1GraphHSizes);
    }
    settings.endGroup();
}


void CpViewWt::loadSettings(QSettings &settings)
{
    settings.beginGroup("CpViewWt");
    {
        s_1GraphHSizes = settings.value("1GraphHSizes").toByteArray();
    }
    settings.endGroup();
}




