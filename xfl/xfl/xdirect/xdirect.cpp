/****************************************************************************

    flow5
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


#include <QApplication>
#include <QClipboard>
#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>

#include <xfl/globals/mainframe.h>
#include <xfl/globals/fileio.h>
#include <xflcore/displayoptions.h>
#include <xfl/xdirect/view2d/dfoillegendwt.h>
#include <xfl/xdirect/view2d/dfoilwt.h>
#include <xfl/xdirect/view2d/oppointwt.h>
#include <xfl/xdirect/analysis/analysis2dsettings.h>
#include <xfl/xdirect/analysis/xfoilbatchdlg.h>
#include <xfl/xdirect/analysis/xfoilanalysisdlg.h>
#include <xfl/xdirect/controls/analysis2dctrls.h>
#include <xfl/xdirect/controls/foiltableview.h>
#include <xfl/xdirect/controls/foiltreeview.h>
#include <xfl/xdirect/controls/lecircledlg.h>
#include <xfl/xdirect/controls/oppointctrls.h>
#include <xfl/xdirect/graphs/blgraphctrls.h>
#include <xfl/xdirect/graphs/xdirectlegendwt.h>
#include <xfl/xdirect/menus/xdirectactions.h>
#include <xfl/xdirect/menus/xdirectmenus.h>
#include <xfl/xdirect/mgt/foilplrlistdlg.h>
#include <xfl/xdirect/xdirect.h>

#include <xfl/editors/analysis2ddef/foilpolardlg.h>
#include <xfl/editors/analysis2ddef/polarautonamedlg.h>
#include <xfl/editors/editplrdlg.h>
#include <xflfoil/analysis2d/xfoiltask.h>
#include <xflcore/flow5events.h>
#include <xflcore/saveoptions.h>
#include <xflcore/xflcore.h>
#include <xflfoil/editors/foilflapdlg.h>
#include <xflfoil/editors/foil1splinedlg.h>
#include <xflfoil/editors/foil2splinedlg.h>
#include <xflfoil/editors/foilcamberdlg.h>
#include <xflfoil/editors/foilcoorddlg.h>
#include <xflfoil/editors/foilscaledlg.h>
#include <xflfoil/editors/foilnormalizedlg.h>
#include <xflfoil/editors/interpolatefoilsdlg.h>
#include <xflfoil/editors/foilledlg.h>
#include <xflfoil/editors/foilnacadlg.h>
#include <xflfoil/editors/foilrepaneldlg.h>
#include <xflfoil/editors/foiltegapdlg.h>
#include <xflfoil/globals/objects2d_globals.h>
#include <xflgraph/containers/graphwt.h>
#include <xflgraph/controls/graphdlg.h>
#include <xflgraph/controls/graphoptions.h>
#include <xflgraph/controls/graphtilevariableset.h>
#include <xflgraph/globals/graph_globals.h>
#include <xflgraph/graph/curve.h>
#include <xflgraph/graph/graph.h>
#include <xflmath/constants.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/objects2d.h>
#include <xflfoil/objects2d/oppoint.h>
#include <xflfoil/objects2d/polar.h>
#include <xflobjects/objects3d/objects3d.h>
#include <xflobjects/objects3d/analysis/analysisseldlg.h>
#include <xfloptim/interface/optimfoil.h>
#include <xflscript/xml/foil/foilsvgwriter.h>
#include <xflscript/xml/foil/xmlpolarreader.h>
#include <xflscript/xml/foil/xmlpolarwriter.h>
#include <xflwidgets/customdlg/objectpropsdlg.h>
#include <xflwidgets/customdlg/renamedlg.h>
#include <xflwidgets/customdlg/textdlg.h>
#include <xflwidgets/customwts/cptableview.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/line/linemenu.h>
#include <xflwidgets/customdlg/doublevaluedlg.h>


#include <xflfoil/objects2d/oppoint.h>

bool XDirect::s_bKeepOpenErrors(true);
bool XDirect::s_bBLTopSide(true);
bool XDirect::s_bBLBotSide(true);
bool XDirect::s_bCurOppOnly(true);


LineStyle XDirect::s_lsTopBL = {true, Line::SOLID, 2, Qt::darkCyan,    Line::NOSYMBOL, "Top BL"};
LineStyle XDirect::s_lsBotBL = {true, Line::SOLID, 2, Qt::darkYellow,  Line::NOSYMBOL, "Bottom BL"};


XDirect::XDirect(MainFrame *pMainFrame)
{
    s_pMainFrame = pMainFrame;

    GraphTiles::setMainFrame(s_pMainFrame);
    GraphTiles::setXDirect(this);

    BLGraphCtrls::setMainFrame(s_pMainFrame);
    OpPointWt::setXDirect(this);
    OpPointWt::setMainFrame(s_pMainFrame);
    BLGraphCtrls::setXDirect(this);
    XDirectLegendWt::setXDirect(this);
    FoilTreeView::setXDirect(this);
    FoilTableView::setXDirect(this);

    m_pFoilTreeView = new FoilTreeView;
    m_pFoilTable = new FoilTableView(s_pMainFrame);

    m_pActions = new XDirectActions(s_pMainFrame, this);
    m_pMenus = new XDirectMenus(s_pMainFrame, this);

    XFoilBatchDlg::setXDirect(this);

    Analysis2dCtrls::setXDirect(this);
    m_pAnalysisControls = new Analysis2dCtrls;

    OpPointCtrls::setXDirect(this);
    m_pOpPointControls = new OpPointCtrls;

    m_bResetCurves    = true;

    m_bTrans          = false;

    m_bShowInviscid   = false;

    m_eView  = OPPVIEW;
    m_iPlrGraph = 0;
    m_iBLView = xfl::ALLGRAPHS;

    XDirect::setCurPolar(nullptr);
    XDirect::setCurOpp(nullptr);

    m_pXFADlg = nullptr;

    m_pXFADlg = new XFoilAnalysisDlg(s_pMainFrame);
    connect(m_pXFADlg, SIGNAL(analysisFinished()), this, SLOT(onFinishAnalysis()));

    makeBLGraphs();
    makeOppGraph();
    makePolarGraphs();

}


void XDirect::makeBLGraphs()
{
    QStringList XVarList = {"X"};
    QStringList OppVarList = {"Cp", "Ue", "D*", THETACHAR, "H"};

    for(int ig=0; ig<MAXGRAPHS; ig++)
    {
        m_BLGraph.append(new Graph);
        m_BLCurveModel.append(new CurveModel);
        m_BLGraph.last()->setCurveModel(m_BLCurveModel.back());
        m_BLGraph.last()->setName(QString::asprintf("BLGraph_%d", ig));
        m_BLGraph.last()->setXVariableList(XVarList);
        m_BLGraph.last()->setYVariableList(OppVarList);
        m_BLGraph.last()->setGraphType(GRAPH::OPPGRAPH);
        m_BLGraph.last()->setXMin(0.0);
        m_BLGraph.last()->setXMax(1.0);
        m_BLGraph.last()->setYMin(0, -0.1);
        m_BLGraph.last()->setYMax(0, 0.1);
        m_BLGraph.last()->setScaleType(GRAPH::RESETTING);
        m_BLGraph.last()->setBorderColor(QColor(200,200,200));
        m_BLGraph.last()->setBorder(true);
        m_BLGraph.last()->setBorderStyle(Line::SOLID);
        m_BLGraph.last()->setBorderWidth(3);
        m_BLGraph.last()->setMargins(50);
        m_BLGraph.last()->setLegendVisible(false);
        m_BLGraph.last()->enableRightAxis(true);
    }
    m_BLGraph[0]->setVariables(0,0);
    m_BLGraph[1]->setVariables(0,1);
    m_BLGraph[2]->setVariables(0,2);
    m_BLGraph[3]->setVariables(0,3);
    m_BLGraph[4]->setVariables(0,4);
}


void XDirect::makeOppGraph()
{
    QStringList XVarList = {"X"};
    QStringList OppVarList = {"Cp", "Ue"};

    m_pOpPointWt = new OpPointWt(s_pMainFrame);
    m_pOppGraph = new Graph;
    m_pOppCurveModel = new CurveModel;
    m_pOppGraph->setXVariableList(XVarList);
    m_pOppGraph->setYVariableList(OppVarList);
    m_pOppGraph->setCurveModel(m_pOppCurveModel);
    m_pOppGraph->setName("OpPoint graph");
    m_pOppGraph->setGraphType(GRAPH::OPPGRAPH);
    m_pOppGraph->setXMin(0.0);
    m_pOppGraph->setXMax(1.0);
    m_pOppGraph->setYMin(0, -0.1);
    m_pOppGraph->setYMax(0, 0.1);
    m_pOppGraph->setScaleType(GRAPH::EXPANDING);
    m_pOppGraph->setBorderColor(QColor(200,200,200));
    m_pOppGraph->setBorder(true);
    m_pOppGraph->setBorderStyle(Line::SOLID);
    m_pOppGraph->setBorderWidth(3);
    m_pOppGraph->setMargins(50);
    m_pOppGraph->setLegendVisible(false);
    m_pOppGraph->setVariables(0,0);
    m_pOppGraph->enableRightAxis(true);
    m_pOpPointWt->setCpGraph(m_pOppGraph);
    m_pOpPointControls->setOpPointWidget(m_pOpPointWt);
}


void XDirect::makePolarGraphs()
{
    for(int ig=0; ig<MAXGRAPHS; ig++)
    {
        m_PlrGraph.append(new Graph);
        m_PlrCurveModel.append(new CurveModel);
        m_PlrGraph.last()->setXVariableList(Polar::s_VariableNames);
        m_PlrGraph.last()->setYVariableList(Polar::s_VariableNames);
        m_PlrGraph.last()->setCurveModel(m_PlrCurveModel.last());
        m_PlrGraph.last()->setName(QString("Polar_Graph_%1").arg(ig));
        m_PlrGraph.last()->setGraphType(GRAPH::POLARGRAPH);
        m_PlrGraph.last()->setXMin(0.0);
        m_PlrGraph.last()->setXMax(0.1);
        m_PlrGraph.last()->setYMin(0, -0.1);
        m_PlrGraph.last()->setYMax(0, 0.1);
        m_PlrGraph.last()->setScaleType(GRAPH::RESETTING);
        m_PlrGraph.last()->setBorderColor(QColor(200,200,200));
        m_PlrGraph.last()->setBorder(true);
        m_PlrGraph.last()->setBorderStyle(Line::SOLID);
        m_PlrGraph.last()->setBorderWidth(3);
        m_PlrGraph.last()->setMargins(50);
        m_PlrGraph.last()->enableRightAxis(true);
    }
    m_PlrGraph[0]->setVariables(3,2);
    m_PlrGraph[1]->setVariables(0,2);
    m_PlrGraph[2]->setVariables(0,5);
    m_PlrGraph[3]->setVariables(0,3);
    m_PlrGraph[4]->setVariables(0,8);
//    m_PlrGraph[5]->setVariables(0,10);
}


XDirect::~XDirect()
{
    delete m_pOppGraph;      m_pOppGraph = nullptr;
    delete m_pOppCurveModel; m_pOppCurveModel = nullptr;

    for(int ig=m_BLGraph.count()-1; ig>=0; ig--)
    {
        delete m_BLGraph.at(ig);
        m_BLGraph.removeAt(ig);
    }
    for(int ig=m_BLCurveModel.count()-1; ig>=0; ig--)
    {
        delete m_BLCurveModel.at(ig);
        m_BLCurveModel.removeAt(ig);
    }

    for(int ig=m_PlrGraph.count()-1; ig>=0; ig--)
    {
        delete m_PlrGraph.at(ig);
        m_PlrGraph.removeAt(ig);
    }
    for(int ig=m_PlrCurveModel.count()-1; ig>=0; ig--)
    {
        delete m_PlrCurveModel.at(ig);
        m_PlrCurveModel.removeAt(ig);
    }


    if(m_pXFADlg) delete m_pXFADlg;

    if(m_pFoilTreeView) delete m_pFoilTreeView;
    if(m_pFoilTable) delete m_pFoilTable;

    if(m_pMenus) delete m_pMenus;
}

Foil *   XDirect::curFoil()  {return Objects2d::curFoil();}
Polar*   XDirect::curPolar() {return Objects2d::curPolar();}
OpPoint* XDirect::curOpp()   {return Objects2d::curOpp();}

void XDirect::checkNeutralLineAction(bool bShow) {m_pActions->m_pShowNeutralLine->setChecked(bShow);}


void XDirect::showDockWidgets()
{
    if(s_pMainFrame->s_iApp!=xfl::XDIRECT)
    {
        s_pMainFrame->m_pdwGraphControls->hide();
        s_pMainFrame->m_pdwOpPoint->hide();
        s_pMainFrame->m_pdwXDirect->hide();
        s_pMainFrame->m_pdwFoilTree->hide();
        s_pMainFrame->m_pdwFoilTable->hide();
        return;
    }

    switch(m_eView)
    {
        case DESIGNVIEW:
        {
            s_pMainFrame->m_pdwGraphControls->hide();
            s_pMainFrame->m_pdwOpPoint->hide();
            s_pMainFrame->m_pdwXDirect->hide();
            s_pMainFrame->m_pdwFoilTree->hide();
            s_pMainFrame->m_pdwFoilTable->show();
            break;
        }
        case BLVIEW:
        {
            s_pMainFrame->m_pdwGraphControls->setWidget(s_pMainFrame->m_pBLTiles->graphControls());
            s_pMainFrame->m_pdwOpPoint->hide();
            s_pMainFrame->m_pdwXDirect->show();
            s_pMainFrame->m_pdwFoilTable->hide();
            s_pMainFrame->m_pdwGraphControls->show();
            s_pMainFrame->m_pdwFoilTree->show();
            break;
        }
        case OPPVIEW:
        {
            s_pMainFrame->m_pdwGraphControls->hide();
            s_pMainFrame->m_pdwFoilTable->hide();
            s_pMainFrame->m_pdwOpPoint->show();
            s_pMainFrame->m_pdwXDirect->show();
            s_pMainFrame->m_pdwFoilTree->show();
            break;
        }
        case POLARVIEW:
        {
            s_pMainFrame->m_pdwGraphControls->setWidget(s_pMainFrame->m_pPolarTiles->graphControls());
            s_pMainFrame->m_pdwOpPoint->hide();
            s_pMainFrame->m_pdwFoilTable->hide();
            s_pMainFrame->m_pdwGraphControls->show();
            s_pMainFrame->m_pdwXDirect->show();
            s_pMainFrame->m_pdwFoilTree->show();
            break;
        }
    }
    s_pMainFrame->m_ptbDFoil->setVisible(isDesignView());
}


void XDirect::setControls()
{
    m_pFoilTreeView->setCurveParams();
    m_pFoilTreeView->setOverallCheckStatus();

    m_pMenus->m_pActivePolarMenu->setEnabled(curPolar());
    m_pMenus->m_pActiveOppMenu->setEnabled(curOpp());

    checkNeutralLineAction(m_pOpPointWt->bNeutralLine());

    m_pActions->m_pSetCpVarGraph->setChecked(CpGraph()->yVariable(0)==0);
    m_pActions->m_pSetQVarGraph->setChecked(CpGraph()->yVariable(0)==1);

    m_pActions->m_pDesignAct->setChecked(isDesignView());
    m_pActions->m_pBLAct->setChecked(isBLView());
    m_pActions->m_pOpPointsAct->setChecked(isOppView());
    m_pActions->m_pPolarsAct->setChecked(isPolarView());

    m_pActions->m_pShowInviscidCurve->setChecked(m_bShowInviscid);
    if(m_pOpPointWt && m_pOpPointWt->CpGraph())
        m_pActions->m_pShowCpLegend->setChecked(m_pOpPointWt->CpGraph()->isLegendVisible());

    m_pActions->m_pRenameCurFoil->setEnabled(curFoil());
    m_pActions->m_pFoilDescription->setEnabled(curFoil());
    m_pActions->m_pDuplicateCurFoil->setEnabled(curFoil());
    m_pActions->m_pDeleteCurFoil->setEnabled(curFoil());
    m_pActions->m_pExportCurFoilDat->setEnabled(curFoil());
    m_pActions->m_pExportCurFoilSVG->setEnabled(curFoil());

    m_pActions->m_pDefinePolarAct->setEnabled(curFoil());
    m_pActions->m_pDeleteFoilOpps->setEnabled(curFoil());
    m_pActions->m_pDeleteFoilPolars->setEnabled(curFoil());

    m_pActions->m_pEditCurPolar->setEnabled(Objects2d::curPolar());
    m_pActions->m_pDeletePolar->setEnabled(Objects2d::curPolar());
    m_pActions->m_pExportCurPolar->setEnabled(Objects2d::curPolar());
    m_pActions->m_pHidePolarOpps->setEnabled(Objects2d::curPolar());
    m_pActions->m_pShowPolarOpps->setEnabled(Objects2d::curPolar());
    m_pActions->m_pDeletePolarOpps->setEnabled(Objects2d::curPolar());

    m_pActions->m_pDerotateFoil->setEnabled(curFoil());
//    m_pActions->m_pNormalizeFoil->setEnabled(curFoil());
    m_pActions->m_pRefineGlobalFoil->setEnabled(curFoil());
    m_pActions->m_pEditCoordsFoil->setEnabled(curFoil());
    m_pActions->m_pScaleFoil->setEnabled(curFoil());
    m_pActions->m_pSetLERadius->setEnabled(curFoil());
    m_pActions->m_pSetTEGap->setEnabled(curFoil());
    m_pActions->m_pSetFlap->setEnabled(curFoil());

    m_pActions->m_pDeleteCurOpp->setEnabled(curOpp());
    m_pActions->m_pExportCurOpp->setEnabled(curOpp());
    m_pActions->m_pCopyCurOppData->setEnabled(curOpp());
    m_pActions->m_pGetOppProps->setEnabled(curOpp());

    m_pOpPointControls->setControls();
    s_pMainFrame->m_pBLTiles->graphControls()->setControls();
    s_pMainFrame->m_pPolarTiles->graphControls()->setControls();

    // DFoil Actions
    m_pActions->m_pFillFoil->setChecked(DFoilWt::isFilling());
    m_pActions->m_pShowLEPosition->setChecked(DFoilWt::isLEPositionVisible());
    m_pActions->m_pShowTEHinge->setChecked(DFoilWt::isTEHingeVisible());
}


void XDirect::connectSignals()
{
    connect(m_pOpPointWt,                   SIGNAL(curveClicked(Curve*)),       SLOT(onCurveClicked(Curve*)));
    connect(m_pOpPointWt,                   SIGNAL(curveDoubleClicked(Curve*)), SLOT(onCurveDoubleClicked(Curve*)));
    connect(m_pOpPointWt,                   SIGNAL(graphChanged(Graph*)),       SLOT(onOpPointGraphChanged()));

    connect(s_pMainFrame->m_pPolarTiles,    SIGNAL(graphChanged(int)),          SLOT(onGraphChanged(int)));
    connect(s_pMainFrame->m_pPolarTiles,    SIGNAL(varSetChanged(int)),         SLOT(onVarSetChanged(int)));
    connect(m_pActions->m_pResetFoilScale,  SIGNAL(triggered()), m_pOpPointWt,  SLOT(onResetFoilScale()));
    connect(m_pActions->m_pShowNeutralLine, SIGNAL(triggered()), m_pOpPointWt,  SLOT(onShowNeutralLine()));

    connect(s_pMainFrame->m_pDFoilWt,       SIGNAL(foilSelected(Foil*)), m_pFoilTreeView, SLOT(selectFoil(Foil*)));
    connect(s_pMainFrame->m_pDFoilWt,       SIGNAL(foilSelected(Foil*)), m_pFoilTable,    SLOT(selectFoil(Foil*)));

}


void XDirect::onGraphChanged(int)
{
    resetCurves();
    updateView();
}


void XDirect::onVarSetChanged(int)
{
    for(int ig=0; ig<m_PlrGraph.size(); ig++)
    {
        m_PlrGraph.at(ig)->setAuto(true);
        m_PlrGraph.at(ig)->resetLimits();
        m_PlrGraph.at(ig)->invalidate();
    }

    resetCurves();
    updateView();
}


void XDirect::createCurves()
{
    switch(m_eView)
    {
        case BLVIEW:
        {
            createBLCurves();
            break;
        }
        case OPPVIEW:
        {
            createOppCurves();
            break;
        }
        case POLARVIEW:
        {
            createPolarCurves();
            break;
        }
        default:
            break;
    }
}


void XDirect::createBLCurves()
{
    for(int ig=0; ig<m_BLGraph.size(); ig++)
    {
        m_BLGraph[ig]->deleteCurves();
        m_BLGraph[ig]->clearSelection();
    }

    // curOpp only in this case to avoid overloading the graphs;
    OpPoint *pOpPoint = curOpp();
    if(!pOpPoint) return;

    pOpPoint->clearCurves();
    if (pOpPoint && pOpPoint->isVisible() && (pOpPoint==curOpp() || !s_bCurOppOnly))
    {
        for(int ig=0; ig<m_BLGraph.size(); ig++)
        {
            // left axis curves only
            if(pOpPoint->isXFoil()) fillBLXFoilCurves(pOpPoint, m_BLGraph.at(ig), m_bShowInviscid);
        }
    }


    for(int ig=0; ig<m_BLGraph.size(); ig++)
    {
        m_BLGraph[ig]->invalidate();
    }

    emit curvesUpdated();
}


void XDirect::createOppCurves()
{
    m_pOppGraph->deleteCurves();
    m_pOppGraph->clearSelection();

    for (int k=0; k<Objects2d::nOpPoints(); k++)
    {
        OpPoint *pOpPoint = Objects2d::opPointAt(k);

        pOpPoint->clearCurves();
        if (pOpPoint && pOpPoint->isVisible() && (pOpPoint==curOpp() || !s_bCurOppOnly))
        {
            Curve *pViscCurve = m_pOppGraph->addCurve();
            Curve *pInviscidCurve = nullptr;
            if(m_bShowInviscid) pInviscidCurve = m_pOppGraph->addCurve();
            fillOppCurves(pOpPoint, pViscCurve, pInviscidCurve, m_pOppGraph->yVariable(0));
            if(pOpPoint==curOpp() && !s_bCurOppOnly)
            {
                m_pOppGraph->selectCurve(pViscCurve);
                m_pOppGraph->selectCurve(pInviscidCurve);
            }

            if(m_pOppGraph->hasRightAxis())
            {
                Curve *pViscCurve = m_pOppGraph->addCurve(AXIS::RIGHTYAXIS);
                Curve *pInviscidCurve = nullptr;
                if(m_bShowInviscid) pInviscidCurve = m_pOppGraph->addCurve(AXIS::RIGHTYAXIS);
                fillOppCurves(pOpPoint, pViscCurve, pInviscidCurve, m_pOppGraph->yVariable(1));
                if(pOpPoint==curOpp())
                {
                    m_pOppGraph->selectCurve(pViscCurve);
                    m_pOppGraph->selectCurve(pInviscidCurve);
                }
            }
        }
    }

    m_pOppGraph->invalidate();

    emit curvesUpdated();
}


void XDirect::createPolarCurves()
{
    for(int ig=0; ig<m_PlrGraph.size(); ig++)
    {
        m_PlrGraph[ig]->deleteCurves();
        m_PlrGraph[ig]->clearSelection();
    }

    for (int k=0; k<Objects2d::nPolars(); k++)
    {
        Polar *pPolar = Objects2d::polarAt(k);
        pPolar->clearCurves();
        if (pPolar->isVisible() && pPolar->m_Alpha.size()>0)
        {
            OpPoint *pOppHigh=nullptr;
            if(Graph::isHighLighting() && curOpp() && pPolar->hasOpp(curOpp())) pOppHigh = curOpp();

            Curve* pCurve = nullptr;
            Curve* pTr2Curve = nullptr;
            for(int ig=0; ig<m_PlrGraph.size(); ig++)
            {
                Graph*pGraph = m_PlrGraph[ig];
                pCurve = pGraph->addCurve(pPolar->foilName() + " / " + pPolar->name());
                pCurve->setTheStyle(pPolar->theStyle());

                fillCurve(pCurve, pPolar, pGraph->xVariable(), pGraph->yVariable(0), pOppHigh);

                if(pGraph->yVariable(0) == 6)    pTr2Curve = pGraph->addCurve();
                else                            pTr2Curve = nullptr;
                pPolar->appendCurve(pCurve);
                if(pPolar==Objects2d::curPolar()) pGraph->selectCurve(pCurve);

                if(pTr2Curve)
                {
                    pTr2Curve->setTheStyle(pPolar->theStyle());
                    fillCurve(pTr2Curve, pPolar, pGraph->xVariable(), 7, pOppHigh);
                    pCurve->setName(pPolar->foilName() + " / " + pPolar->name() + " / Xtr1");
                    pTr2Curve->setName(pPolar->foilName() + " / " + pPolar->name() + " / Xtr2");
                    pPolar->appendCurve(pTr2Curve);
                    if(pPolar==Objects2d::curPolar()) pGraph->selectCurve(pTr2Curve);
                }
                if(pGraph->hasRightAxis())
                {
                    pCurve = pGraph->addCurve(pPolar->name(), AXIS::RIGHTYAXIS);
                    pCurve->setTheStyle(pPolar->theStyle());

                    fillCurve(pCurve, pPolar, pGraph->xVariable(), pGraph->yVariable(1), pOppHigh);

                    if(pGraph->yVariable(0) == 6)    pTr2Curve = pGraph->addCurve();
                    else                            pTr2Curve = nullptr;
                    pPolar->appendCurve(pCurve);
                    if(pPolar==Objects2d::curPolar()) pGraph->selectCurve(pCurve);

                    if(pTr2Curve)
                    {
                        pTr2Curve->setTheStyle(pPolar->theStyle());
                        fillCurve(pTr2Curve, pPolar, pGraph->xVariable(), 7, pOppHigh);
                        pCurve->setName(pPolar->foilName() + " / " + pPolar->name() + " / Xtr1");
                        pTr2Curve->setName(pPolar->foilName() + " / " + pPolar->name() + " / Xtr2");
                        pPolar->appendCurve(pTr2Curve);
                        if(pPolar==Objects2d::curPolar()) pGraph->selectCurve(pTr2Curve);
                    }
                }
            }
        }
    }
    for(int ig=0; ig<m_PlrGraph.size(); ig++)
    {
        m_PlrGraph[ig]->invalidate();
    }
    emit curvesUpdated();
}


void XDirect::fillBLXFoilCurves(OpPoint *pOpp, Graph *pGraph, bool bInviscid)
{
    Foil const*pOpFoil = Objects2d::foil(pOpp->foilName());

    pGraph->setYInverted(0,false);

    switch(pGraph->yVariable(0))
    {
        case 0:  // Cp
        {
            pGraph->setYInverted(0, true);
            Curve *pCpv    = pGraph->addCurve();
            pCpv->setTheStyle(pOpp->theStyle());
            pCpv->setName(pOpp->name());
            pOpp->appendCurve(pCpv);

            Curve *pCpi = nullptr;

            pCpi = pGraph->addCurve();
            pOpp->appendCurve(pCpi);
            pCpi->setVisible(bInviscid);
            pCpi->setSymbol(pOpp->pointStyle());
            pCpi->setStipple(Line::DASH);
            pCpi->setColor(pOpp->lineColor().darker(150));
            pCpi->setWidth(pOpp->lineWidth());
            pCpi->setName(pOpp->name() + " - Inviscid");

            for (int j=0; j<pOpFoil->nNodes(); j++)
            {
                if(pOpp->bViscResults())    pCpv->appendPoint(pOpFoil->x(j), pOpp->Cpv.at(j));
                if(pOpp==curOpp() && pCpi)  pCpi->appendPoint(pOpFoil->x(j), pOpp->Cpi.at(j));
            }

            if(pOpp==curOpp())
            {
                pGraph->selectCurve(pCpv);
            }

            break;
        }
        case 1:  // Edge velocity
        {
            double y[IVX][3];
            memset(y, 0, IVX*3*sizeof(double));
            double uei(0);

            //---- fill compressible ue arrays

            for (int ibl=2; ibl<=pOpp->m_BLXFoil.nside1; ibl++)
            {
                uei = pOpp->m_BLXFoil.uedg[ibl][1];
                y[ibl][1] = uei * (1.0-pOpp->m_BLXFoil.tklam)
                        / (1.0-pOpp->m_BLXFoil.tklam*(uei/pOpp->m_BLXFoil.qinf)*(uei/pOpp->m_BLXFoil.qinf));
            }

            for (int ibl=2; ibl<=pOpp->m_BLXFoil.nside2; ibl++)
            {
                uei = pOpp->m_BLXFoil.uedg[ibl][2];
                y[ibl][2] = uei * (1.0-pOpp->m_BLXFoil.tklam)
                        / (1.0-pOpp->m_BLXFoil.tklam*(uei/pOpp->m_BLXFoil.qinf)*(uei/pOpp->m_BLXFoil.qinf));
            }

            if(s_bBLTopSide)
            {
                Curve * pTopCurve(nullptr);
                pTopCurve = pGraph->addCurve();
                pTopCurve->setName("Top");
                pTopCurve->setTheStyle(s_lsTopBL);
                pOpp->appendCurve(pTopCurve);
                for (int i=2; i<=pOpp->m_BLXFoil.nside1-1; i++)
                    pTopCurve->appendPoint(pOpp->m_BLXFoil.xbl[i][1], y[i][1]);
            }
            if(s_bBLBotSide)
            {
                Curve * pBotCurve(nullptr);
                pBotCurve = pGraph->addCurve();
                pBotCurve->setName("Bottom");
                pBotCurve->setTheStyle(s_lsBotBL);
                pOpp->appendCurve(pBotCurve);
                for (int i=2; i<=pOpp->m_BLXFoil.nside2-1; i++)
                    pBotCurve->appendPoint(pOpp->m_BLXFoil.xbl[i][2], y[i][2]);
            }

            break;
        }
        case 2:  // Dstar
        {
            if(s_bBLTopSide)
            {
                Curve *pTopCurve(nullptr);
                pTopCurve = pGraph->addCurve();
                pTopCurve->setName("Top");
                pTopCurve->setTheStyle(s_lsTopBL);
                pOpp->appendCurve(pTopCurve);
                for (int i=2; i<pOpp->m_BLXFoil.nside1; i++)
                {
                    pTopCurve->appendPoint(pOpp->m_BLXFoil.xbl[i][1], pOpp->m_BLXFoil.dstr[i][1]);
                }
            }
            if(s_bBLBotSide)
            {
                Curve *pBotCurve(nullptr);
                pBotCurve = pGraph->addCurve();
                pBotCurve->setName("Bottom");
                pBotCurve->setTheStyle(s_lsBotBL);
                pOpp->appendCurve(pBotCurve);
                for (int i=2; i<pOpp->m_BLXFoil.nside2; i++)
                {
                    pBotCurve->appendPoint(pOpp->m_BLXFoil.xbl[i][2], pOpp->m_BLXFoil.dstr[i][2]);
                }
            }

            break;
        }
        case 3:  // theta
        {
            if(s_bBLTopSide)
            {
                Curve * pTopCurve(nullptr);
                pTopCurve = pGraph->addCurve();
                pTopCurve->setTheStyle(s_lsTopBL);
                pTopCurve->setName("Top");
                pOpp->appendCurve(pTopCurve);

                for (int i=2; i<pOpp->m_BLXFoil.nside1; i++)
                {
                    pTopCurve->appendPoint(pOpp->m_BLXFoil.xbl[i][1], pOpp->m_BLXFoil.thet[i][1]);
                }
            }
            if(s_bBLBotSide)
            {
                Curve * pBotCurve(nullptr);
                pBotCurve = pGraph->addCurve();
                pBotCurve->setName("Bottom");
                pBotCurve->setTheStyle(s_lsBotBL);
                pOpp->appendCurve(pBotCurve);

                for (int i=2; i<pOpp->m_BLXFoil.nside2; i++)
                {
                    pBotCurve->appendPoint(pOpp->m_BLXFoil.xbl[i][2], pOpp->m_BLXFoil.thet[i][2]);
                }
            }

            break;
        }
        case 4: //Hk
        {
            if(s_bBLTopSide)
            {
                Curve * pTopCurve(nullptr);
                pTopCurve = pGraph->addCurve();
                pTopCurve->setName("Top");
                pTopCurve->setTheStyle(s_lsTopBL);
                pOpp->appendCurve(pTopCurve);
                for (int i=2; i<=pOpp->m_BLXFoil.nside2-1; i++)
                {
                    pTopCurve->appendPoint(pOpp->m_BLXFoil.xbl[i][2], pOpp->m_BLXFoil.Hk[i][2]);
                }
            }
            if(s_bBLBotSide)
            {
                Curve * pBotCurve(nullptr);
                pBotCurve = pGraph->addCurve();
                pBotCurve->setName("Bottom");
                pBotCurve->setTheStyle(s_lsBotBL);
                pOpp->appendCurve(pBotCurve);

                for (int i=2; i<=pOpp->m_BLXFoil.nside1-1; i++)
                {
                    pBotCurve->appendPoint(pOpp->m_BLXFoil.xbl[i][1], pOpp->m_BLXFoil.Hk[i][1]);
                }
            }

            break;
        }
        default:
            break;
    }
}


void XDirect::fillOppCurves(OpPoint *pOpp, Curve*pViscCurve, Curve*pInviscidCurve, int YVar)
{
    if(pViscCurve)
    {
        pViscCurve->setTheStyle(pOpp->theStyle());
        pViscCurve->setName(pOpp->name());
        pOpp->appendCurve(pViscCurve);
    }

    Foil *pCurFoil = Objects2d::foil(pOpp->foilName());
    if(!pCurFoil) return;

    Foil tmpfoil(pCurFoil);
    if(fabs(pOpp->theta())>FLAPANGLEPRECISION)
    {
        tmpfoil.setTEFlapAngle(pOpp->theta());
        tmpfoil.setFlaps();
    }

    if(tmpfoil.nNodes() != pOpp->nPoints())
        return; // something went wrong when the flap was set.

    if(pInviscidCurve)
    {
        pOpp->appendCurve(pInviscidCurve);
        pInviscidCurve->setSymbol(pOpp->pointStyle());
        pInviscidCurve->setStipple(Line::DASH);
        pInviscidCurve->setColor(pOpp->lineColor().darker(150));
        pInviscidCurve->setWidth(std::max(pOpp->lineWidth()-1,1));
        QString str= QString::asprintf("Re=%7.0f-Alpha=%5.2f_Inviscid", pOpp->Reynolds(), pOpp->aoa());
        pInviscidCurve->setName(str);
    }

    switch(YVar)
    {
        case 0:  // Cp
        {
            if(pViscCurve)     pViscCurve->setName(pViscCurve->name()+"_Cp");
            if(pInviscidCurve) pInviscidCurve->setName(pInviscidCurve->name()+"_Cp");
            for (int j=0; j<int(tmpfoil.nNodes()); j++)
            {
                if(pViscCurve && pOpp->bViscResults()) pViscCurve->appendPoint(tmpfoil.node(j).x, pOpp->Cpv.at(j));
                if(pInviscidCurve) pInviscidCurve->appendPoint(tmpfoil.node(j).x, pOpp->Cpi.at(j));
            }
            break;
        }
        case 1:  // Q
        {
            if(pViscCurve)     pViscCurve->setName(pViscCurve->name()+"_Q");
            if(pInviscidCurve) pInviscidCurve->setName(pInviscidCurve->name()+"_Q");
            for (int j=0; j<int(tmpfoil.nNodes()); j++)
            {
                if(pViscCurve && pOpp->bViscResults()) pViscCurve->appendPoint(tmpfoil.node(j).x, pOpp->Qv.at(j));
                if(pInviscidCurve)                     pInviscidCurve->appendPoint(tmpfoil.node(j).x, pOpp->Qi.at(j));
            }

            break;
        }

        default:
            break;
    }
}

void XDirect::onOpPointGraphChanged()
{
    BLGraph(0)->resetLimits();
    BLGraph(0)->setAuto(true);
    BLGraph(0)->setYInverted(0, false);

    setControls();

    m_bResetCurves = true;
    updateView();
}


void XDirect::keyPressEvent(QKeyEvent *pEvent)
{
//    bool bShift = false;
//    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;
    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;
//    bool bAlt = false;
//    if(pEvent->modifiers() & Qt::AltModifier)   bAlt =true;

    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (pEvent->modifiers().testFlag(Qt::AltModifier) && pEvent->modifiers().testFlag(Qt::ShiftModifier))
            {
                pEvent->accept();
                onOpPointProps();
                break;
            }
            else if (pEvent->modifiers().testFlag(Qt::AltModifier))
            {
                if(m_eView==DESIGNVIEW)
                    onFoilProps();
                else
                    onPolarProps();
                pEvent->accept();
                break;
            }

            break;
        case Qt::Key_Tab:
            m_pAnalysisControls->onReadAnalysisData();
            break;
        case Qt::Key_Escape:
            stopAnimate();
            updateView();
            pEvent->accept();
            break;
        case Qt::Key_A:
            if(bCtrl) onAnalyze();
            pEvent->accept();
            break;
        case Qt::Key_D:
        {
            if(bCtrl) onDuplicateFoil();
            pEvent->accept();
            break;
        }
        case Qt::Key_L:
            if(bCtrl)
                onSetLERadius();
            else
                s_pMainFrame->onLogFile();
            pEvent->accept();
            break;
        case Qt::Key_6:
        {
            if(bCtrl)
            {
                s_pMainFrame->onXPlane();
                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_F2:
        {
            if(curPolar())     onRenameCurPolar();
            else if(curFoil()) onRenameCurFoil();
            pEvent->accept();
            break;
        }
        default:
            break;
    }
}


void XDirect::loadSettings(QSettings &settings)
{
    settings.beginGroup("XDirect");
    {
        int k = settings.value("XDirectView", 0).toInt();
        if      (k==0) m_eView=DESIGNVIEW;
        else if (k==1) m_eView=BLVIEW;
        else if (k==2) m_eView=OPPVIEW;
        else if (k==3) m_eView=POLARVIEW;


        s_lsTopBL.loadSettings(settings, "TopBLStyle");
        s_lsBotBL.loadSettings(settings, "BotBLStyle");

        s_bCurOppOnly     = settings.value("CurOppOnly").toBool();
        m_bShowInviscid   = settings.value("ShowInviscid", false).toBool();

        m_iPlrGraph       = settings.value("PlrGraph").toInt();

        m_iActiveGraph    = settings.value("iActivePolarGraph", 0).toInt();

        s_bKeepOpenErrors = settings.value("KeepOpenErrors").toBool();

        m_pFoilTreeView->setSplitterSize(settings.value("FoilTreeSplitterSizes").toByteArray());

        LineStyle ls;
        ls.loadSettings(settings, "SplineFoil");
        Foil2SplineDlg::s_SF.setTheStyle(ls);

        ls.loadSettings(settings, "BSpline");
        Foil1SplineDlg::Bspline().setTheStyle(ls);
        ls.loadSettings(settings, "C3Spline");
        Foil1SplineDlg::C3Spline().setTheStyle(ls);

        ls.loadSettings(settings, "CamberSpline");
        FoilCamberDlg::CSpline().setTheStyle(ls);
        ls.loadSettings(settings, "ThickSpline");
        FoilCamberDlg::TSpline().setTheStyle(ls);
    }
    settings.endGroup();

    m_pFoilTreeView->setPropertiesFont(DisplayOptions::tableFont());

    EditPlrDlg::loadSettings(settings);
    OpPointWt::loadSettings(settings);
    FoilPolarDlg::loadSettings(settings);
    PolarAutoNameDlg::loadSettings(settings);
    XFoilBatchDlg::loadSettings(settings);
    XFoilAnalysisDlg::loadSettings(settings);
    XFoilTask::loadSettings(settings);
    FoilDlg::loadSettings(settings);
    OptimFoil::loadSettings(settings);

    m_pDFoilWt->loadSettings(settings);

    for(int ig=0; ig<m_BLGraph.count(); ig++) m_BLGraph[ig]->loadSettings(settings);

    s_pMainFrame->m_pBLTiles->loadSettings(settings, "XDirectBLTiles");
    if(s_pMainFrame->m_pBLTiles->variableSetCount()>0)
    {
        GraphTileVariableSet const & variableset = s_pMainFrame->m_pBLTiles->variableSet(0);
        for(int ig=0; ig<m_BLGraph.size(); ig++)
        {
            m_BLGraph.at(ig)->setVariables(variableset.XVar(ig), variableset.YVar(ig));
        }
    }

    for(int ig=0; ig<m_PlrGraph.count(); ig++) m_PlrGraph[ig]->loadSettings(settings);
    s_pMainFrame->m_pPolarTiles->loadSettings(  settings, "XDirectPlrTiles");
    if(s_pMainFrame->m_pPolarTiles->variableSetCount()>0)
    {
        GraphTileVariableSet const & variableset = s_pMainFrame->m_pPolarTiles->variableSet(0);
        for(int ig=0; ig<m_PlrGraph.size(); ig++)
        {
            m_PlrGraph.at(ig)->setVariables(variableset.XVar(ig), variableset.YVar(ig));
        }
    }

    m_pOppGraph->loadSettings(settings);

    m_pFoilTable->setTableFontStruct(DisplayOptions::tableFontStruct());
}


void XDirect::onAnalyze()
{
    if(!curFoil() || !curPolar()) return;

    if(curFoil()->nNodes()>=255)
    {
        s_pMainFrame->displayMessage("XFoil requires that NPanels<255\n\n", true);
        return;
    }

    m_pAnalysisControls->onReadAnalysisData();
    m_pAnalysisControls->enableAnalyze(false);


    if(!m_pXFADlg) return;
    Foil *pFoil  = curFoil();
    Polar*pPolar = curPolar();
    if(!pFoil || !pPolar) return;

    Objects2d::setCurOpp(nullptr);

    if(pFoil->hasTEFlap())
    {
        double theta = pPolar->TEFlapAngle();
        if(fabs(theta)>FLAPANGLEPRECISION)
        {
           pFoil->setTEFlapAngle(theta);
           pFoil->setFlaps();
        }
        else
        {
            pFoil->applyBase();
        }
    }

    m_pXFADlg->initializeAnalysis(pFoil, pPolar, m_pAnalysisControls->ranges());
    m_pXFADlg->show();
    m_pXFADlg->analyze();
}


void XDirect::onFinishAnalysis()
{
    Objects2d::setPolarStyle(curPolar(), curPolar()->theStyle(), true, true, true, true);

    m_pFoilTreeView->addOpps(curPolar());
    if(curPolar()->isXFoil())
    {
        if(m_pXFADlg)
        {
            setOpp(m_pXFADlg->lastOpp());
            if(!m_pXFADlg->hasErrors() || !s_bKeepOpenErrors)
                m_pXFADlg->hide();
        }
    }

    if(curOpp()) m_pFoilTreeView->selectOpPoint(curOpp());
    else         m_pFoilTreeView->selectPolar(curPolar());

    //refresh the view
    m_bResetCurves = true;
    updateView();
    setControls();

    m_pAnalysisControls->enableAnalyze(true);

    m_pFoilTreeView->setFocus();

    emit projectModified();
}


void XDirect::onBatchAnalysis()
{
    XFoilBatchDlg *pBatchDlg = new XFoilBatchDlg(s_pMainFrame);
    pBatchDlg->setFoil(Objects2d::curFoil());
    pBatchDlg->initDialog();

    pBatchDlg->exec();

    delete pBatchDlg;

    setPolar();
    m_pFoilTreeView->fillModelView();

    setOpp();
    setControls();
    updateView();

    emit projectModified();
}


void XDirect::onAutoPolarNameOptions()
{
    PolarAutoNameDlg dlg;
    dlg.initDialog(&FoilPolarDlg::thePolar());
    dlg.exec();
}


void XDirect::onCpGraph()
{
    onOpPointView();

    if(m_pOppGraph->yVariable(0)==0) return;

    if(m_pOppGraph->yVariable(0)!=0)
    {
        m_pOppGraph->setAuto(true);
        m_pOppGraph->setYVariable(0, 0);
    }

    m_pOppGraph->setYInverted(0, true);
    m_bResetCurves = true;

    setControls();

    m_pOppGraph->invalidate();
    m_pOpPointWt->setFoilScale(false);
    updateView();
}


void XDirect::onCpi(bool bInviscid)
{
    m_bShowInviscid = bInviscid;

    m_bResetCurves = true;
//    setControls();
    updateView();
}


void XDirect::onCurOppOnly(bool bCurOppOnly)
{
    s_bCurOppOnly = bCurOppOnly;

    if(curOpp()) curOpp()->setVisible(true);
    m_bResetCurves = true;
    updateView();
}


void XDirect::onDefineAnalysis()
{
    if(!curFoil()) return;

    FoilPolarDlg fpDlg(s_pMainFrame);

    fpDlg.initDialog(curFoil(), &FoilPolarDlg::thePolar());

    int res = fpDlg.exec();
    if (res == QDialog::Accepted)
    {
        Polar *pNewPolar = new Polar();
        pNewPolar->copySpecification(&FoilPolarDlg::thePolar());
        if(Curve::alignChildren())
        {
            pNewPolar->setTheStyle(curFoil()->theStyle());
        }
        else
        {
            pNewPolar->setLineColor(xfl::randomColor());
            pNewPolar->setLineWidth(Curve::defaultLineWidth());
        }
        pNewPolar->setFoilName(curFoil()->name());
        pNewPolar->setName(FoilPolarDlg::thePolar().name());
        pNewPolar->setVisible(true);
        insertNewPolar(pNewPolar);
        setPolar(pNewPolar);
        m_pFoilTreeView->insertPolar(pNewPolar);
        m_pFoilTreeView->selectPolar(pNewPolar);

        updateView();
        emit projectModified();
    }
    setControls();
}


void XDirect::onEditCurPolar()
{
    if(!curFoil()) return;

    Polar *pCurPolar = Objects2d::curPolar();
    if (!pCurPolar) return;

    Polar *pModPolar = new Polar();
    pModPolar->copySpecification(Objects2d::curPolar());

    FoilPolarDlg Dlg(s_pMainFrame);
    Dlg.initDialog(curFoil(), pModPolar);

    m_bResetCurves = true;
    updateView();

    if(Dlg.exec() == QDialog::Accepted)
    {
        pModPolar->copySpecification(&FoilPolarDlg::thePolar());

        pModPolar = insertNewPolar(pModPolar);
        setPolar(pModPolar);
        if(pModPolar) m_pFoilTreeView->insertPolar(pModPolar);

        updateView();
        emit projectModified();
    }
    else
    {
        delete pModPolar;
    }

    m_bResetCurves = true;
    updateView();
}


void XDirect::onDeleteCurFoil()
{
    QString strong;
    strong = "Are you sure you want to delete\n"+ curFoil()->name() +"\n";
    strong+= "and all associated operating points and polars?";

    int resp = QMessageBox::question(s_pMainFrame, "Question", strong,  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if(resp != QMessageBox::Yes) return;

    Foil *pDoomedFoil = curFoil();

    // set pointers to zero now before redraw operations on a non-existing foil
    Objects2d::setCurFoil(nullptr);
    Objects2d::setCurPolar(nullptr);
    Objects2d::setCurOpp(nullptr);

    m_pFoilTreeView->removeFoil(pDoomedFoil);

    Foil*pNextFoil = Objects2d::deleteFoil(pDoomedFoil);

    setFoil(pNextFoil);

    m_pFoilTreeView->selectFoil(curFoil());
    m_pFoilTable->selectFoil(curFoil());
    m_pFoilTable->updateTable();
    m_pDFoilWt->resetLegend();

    m_bResetCurves = true;

    emit projectModified();

    setControls();

    m_pDFoilWt->updateView();

    updateView();
}


/**
 * The user has requested the deletion of the current OpPoint.
 */
void XDirect::onDeleteCurOpp()
{
    OpPoint* pOpPoint = curOpp();
    stopAnimate();

    if (!pOpPoint) return;

    double alfa = curOpp()->m_Alpha;
    m_pFoilTreeView->removeOpPoint(pOpPoint);
    Objects2d::deleteOpp(curOpp());

    double dist = 1000.0;
    double alfa_m1 = -100.0;
    for(int iOpp=0; iOpp<Objects2d::nOpPoints(); iOpp++)
    {
        OpPoint *pOpp = Objects2d::opPointAt(iOpp);
        if(pOpp->polarName().compare(curPolar()->name())==0 && pOpp->foilName().compare(curFoil()->name())==0)
        {
            if(fabs(pOpp->m_Alpha - alfa)<dist)
            {
                dist = fabs(pOpp->m_Alpha - alfa);
                alfa_m1 = pOpp->m_Alpha;
            }
        }
    }
    setOpp(alfa_m1);
    m_pFoilTreeView->selectOpPoint();
    updateView();

    setControls();
    emit projectModified();
}


void XDirect::onDeleteCurPolar()
{
    if(!Objects2d::curPolar()) return;

    QString str;

    str = "Are you sure you want to delete the polar :\n  " + Objects2d::curPolar()->name();
    str += "\n and all the associated operating points?";

    if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, "Question", str,
                                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel)) return;

    QString nextPolarName = m_pFoilTreeView->removePolar(Objects2d::curPolar());

    Objects2d::deletePolar(curPolar());
    Objects2d::setCurPolar(nullptr);
    Objects2d::setCurOpp(nullptr);

    Polar *pNextPolar = Objects2d::polar(curFoil(), nextPolarName);
    setPolar(pNextPolar);

    if(pNextPolar)
        m_pFoilTreeView->selectPolar(Objects2d::curPolar());
    else
        m_pFoilTreeView->selectFoil(Objects2d::curFoil());

    emit projectModified();
    updateView();
}


void XDirect::onDeletePolarOpps()
{
    if(!curFoil() || !Objects2d::curPolar()) return;

    OpPoint *pOpp;

    for(int i=Objects2d::nOpPoints()-1; i>=0; i--)
    {
        pOpp = Objects2d::opPointAt(i);
        if(pOpp->foilName()==curFoil()->name() && pOpp->polarName()==Objects2d::curPolar()->name())
        {
            Objects2d::removeOpPointAt(i);
            delete pOpp;
        }
    }

    setCurOpp(nullptr);
    emit projectModified();

    m_pFoilTreeView->updateObjectView();
    m_pFoilTable->updateTable();

    m_bResetCurves = true;
    //    setCurveParams();
    setControls();
    updateView();
}


void XDirect::onDeleteFoilOpps()
{
    if(!curFoil()) return;

    for(int i=Objects2d::nOpPoints()-1; i>=0; i--)
    {
        OpPoint *pOpp = Objects2d::opPointAt(i);
        if(pOpp->foilName()==curFoil()->name())
        {
            Objects2d::removeOpPointAt(i);
            delete pOpp;
        }
    }
    setCurOpp(nullptr);

    m_pFoilTreeView->updateObjectView();
    m_pFoilTable->updateTable();

    m_bResetCurves = true;
    //    setCurveParams();
    setControls();
    updateView();

    emit projectModified();
}


void XDirect::onDeleteFoilPolars()
{
    if(!curFoil()) return;

    stopAnimate();

    QString strong;

    strong = "Are you sure you want to delete polars and oprating points\n";
    strong +="associated to "+curFoil()->name()  + "?";
    if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, "Question", strong,
                                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
        return;


    // start by removing all OpPoints
    for (int l=Objects2d::nOpPoints()-1; l>=0; l--)
    {
        OpPoint *pOpPoint = Objects2d::opPointAt(l);
        if (pOpPoint->foilName() == curFoil()->name())
        {
            Objects2d::removeOpPointAt(l);
            delete pOpPoint;
        }
    }

    // then remove the Polar and update views
    for (int l=Objects2d::nPolars()-1; l>=0; l--)
    {
        Polar* pPolar = Objects2d::polarAt(l);
        if (pPolar->foilName() == curFoil()->name())
        {
            Objects2d::removePolarAt(l);
            delete pPolar;
        }
    }
    setCurPolar(nullptr);
    setCurOpp(nullptr);

    setPolar();

    m_bResetCurves = true;

    m_pFoilTreeView->updateObjectView();
    m_pFoilTable->updateTable();

    emit projectModified();

    setControls();
    updateView();
}


void XDirect::onEditCurPolarPts()
{
    if (!Objects2d::curPolar()) return;

    Polar *pMemPolar = new Polar;
    pMemPolar->copy(Objects2d::curPolar());

    EditPlrDlg epDlg(s_pMainFrame);
    epDlg.initDialog(Objects2d::curPolar(), nullptr);

    connect(&epDlg, SIGNAL(dataChanged()), this, SLOT(onResetPolarCurve()));

    LineStyle style= Objects2d::curPolar()->theStyle();


    Objects2d::curPolar()->setPointStyle(Line::LITTLECIRCLE);

    m_bResetCurves = true;
    updateView();

    if(epDlg.exec() == QDialog::Accepted)
    {
        emit projectModified();
    }
    else
    {
        Objects2d::curPolar()->copy(pMemPolar);
    }
    Objects2d::curPolar()->setTheStyle(style);

    m_bResetCurves = true;
    updateView();

    delete pMemPolar;
}


void XDirect::onExportAllPolars()
{
    //select the directory for output
    QString dirPathName = QFileDialog::getExistingDirectory(s_pMainFrame, "Export Directory",
                                                            SaveOptions::lastDirName());
    s_pMainFrame->exportAllPolars(dirPathName, SaveOptions::exportFileType());
}


void XDirect::onExportCpGraph()
{
    exportGraphDataToFile(BLGraph(0));
}


/**
 * The user has requested the export of the current foil to a text file
 */
void XDirect::onExportCurFoilToDat()
{
    if(!curFoil())    return;

    QString FileName;
    FileName = curFoil()->name();
    FileName.replace("/", " ");
    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export Foil",
                                            SaveOptions::datFoilDirName()+"/"+FileName+".dat",
                                            "Foil File (*.dat)");
    if(!FileName.length()) return;

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);

    curFoil()->exportFoilToDat(out);
    XFile.close();
}


/**
 * The user has requested the export of the current foil to a text file
 */
void XDirect::onExportCurFoilToSVG()
{
    if(!curFoil()) return;

    QString FileName = curFoil()->name();
    FileName.replace("/", " ");
    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export Foil",
                                            SaveOptions::lastDirName()+"/"+FileName+".svg",
                                            "SVG file (*.svg)");
    if(!FileName.length()) return;

    QFileInfo fi(FileName);
    SaveOptions::setLastDirName(fi.path());

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    FoilSVGWriter svgwriter(XFile);
    svgwriter.writeFoil(curFoil());

    XFile.close();
}


/**
 * The user has requested the export of the current OpPoint to a text file
 */
void XDirect::onExportCurOpp()
{
    if(!curFoil() || !Objects2d::curPolar() || !curOpp())    return;

    QString FileName = curOpp()->name();
    FileName = FileName.replace(" ", "");
    QString filter;

    QFileDialog fd(s_pMainFrame);

    if(SaveOptions::exportFileType()==xfl::TXT)
    {
        filter = "Text File (*.txt)";

    }
    else
    {
        filter = "Comma Separated Values (*.csv)";
    }
    fd.setDefaultSuffix(SaveOptions::textSeparator());

    FileName = fd.getSaveFileName(s_pMainFrame, "Export the operating point",
                                  SaveOptions::lastDirName() + "/"+FileName+".stl",
                                  "Text File (*.txt);;Comma Separated Values (*.csv)",
                                  &filter);

    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastDirName(FileName.left(pos));
    pos = FileName.lastIndexOf(".csv");
    if (pos>0) SaveOptions::setExportFileType(xfl::CSV);
    else       SaveOptions::setExportFileType(xfl::TXT);

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);
    out << onCopyCurOppData();
    XFile.close();
}


void XDirect::onExportPolarOpps()
{
    if(!Objects2d::nPolars())
    {
        QString strange  = "No Operating Point to export to file.";
        s_pMainFrame->displayMessage(strange+"\n", true);
        s_pMainFrame->onShowLogWindow(true);

        return;
    }

    QString FileName;

    QString filter;
    if(SaveOptions::exportFileType()==xfl::TXT) filter = "Text File (*.txt)";
    else                                       filter = "Comma Separated Values (*.csv)";

    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export operating point",
                                            SaveOptions::lastDirName() ,
                                            "Text File (*.txt);;Comma Separated Values (*.csv)",
                                            &filter);

    if(!FileName.length()) return;
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastDirName(FileName.left(pos));
    pos = FileName.lastIndexOf(".csv");
    if (pos>0) SaveOptions::setExportFileType(xfl::CSV);
    else       SaveOptions::setExportFileType(xfl::TXT);

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);

    QString Header, strong;
    out<<xfl::versionName(true);
    out<<"\n\n";
    strong = curFoil()->name() + "\n";
    out << strong;

    OpPoint *pOpPoint;

    for (int i=0; i<Objects2d::nOpPoints(); i++)
    {
        pOpPoint = Objects2d::opPointAt(i);
        if(pOpPoint->foilName() == Objects2d::curPolar()->foilName() && pOpPoint->polarName() == Objects2d::curPolar()->name() )
        {
            if(SaveOptions::exportFileType()==xfl::TXT)
                strong = QString("Reynolds = %1   Mach = %2  NCrit = %3\n")
                        .arg(pOpPoint->Reynolds(), 7, 'f', 0)
                        .arg(pOpPoint->m_Mach, 4,'f',0)
                        .arg(pOpPoint->NCrit, 3, 'f',1);
            else
                strong = QString("Reynolds =, %1,Mach =, %2,NCrit =, %3\n")
                        .arg(pOpPoint->Reynolds(), 7, 'f', 0)
                        .arg(pOpPoint->m_Mach, 4,'f',0)
                        .arg(pOpPoint->NCrit, 3, 'f',1);

            out<<strong;
            if(SaveOptions::exportFileType()==1) Header = QString("  Alpha        Cd        Cl        Cm        XTr1      XTr2   TEHMom    Cpmn\n");
            else        Header = QString("Alpha,Cd,Cl,Cm,XTr1,XTr2,TEHMom,Cpmn\n");
            out<<Header;

            if(SaveOptions::exportFileType()==xfl::TXT)
                strong = QString("%1   %2   %3   %4   %5   %6   %7  %8\n")
                        .arg(pOpPoint->aoa(),7,'f',3)
                        .arg(pOpPoint->Cd,9,'f',3)
                        .arg(pOpPoint->Cl,7,'f',3)
                        .arg(pOpPoint->Cm,7,'f',3)
                        .arg(pOpPoint->XTrTop,7,'f',3)
                        .arg(pOpPoint->XTrBot,7,'f',3)
                        .arg(pOpPoint->m_TEHMom,7,'f',4)
                        .arg(pOpPoint->Cpmn,7,'f',4);
            else
                strong = QString("%1,%2,%3,%4,%5,%6,%7,%8\n")
                        .arg(pOpPoint->aoa(),7,'f',3)
                        .arg(pOpPoint->Cd,9,'f',3)
                        .arg(pOpPoint->Cl,7,'f',3)
                        .arg(pOpPoint->Cm,7,'f',3)
                        .arg(pOpPoint->XTrTop,7,'f',3)
                        .arg(pOpPoint->XTrBot,7,'f',3)
                        .arg(pOpPoint->m_TEHMom,7,'f',4)
                        .arg(pOpPoint->Cpmn,7,'f',4);

            out<<strong;
            if(SaveOptions::exportFileType()==xfl::TXT) out<< " Cpi          Cpv\n-----------------\n";
            else                                       out << "Cpi,Cpv\n";

            for (int j=0; j<curFoil()->nNodes(); j++)
            {
                if(pOpPoint->bViscResults())
                {
                    if(SaveOptions::exportFileType()==xfl::TXT) strong = QString("%1   %2\n").arg(pOpPoint->Cpi.at(j), 7,'f',4)
                            .arg(pOpPoint->Cpv.at(j), 7, 'f',4);
                    else                                        strong = QString("%1,%2\n").arg(pOpPoint->Cpi.at(j), 7,'f',4)
                            .arg(pOpPoint->Cpv.at(j), 7, 'f',4);
                }
                else
                {
                    strong=QString("%1\n").arg(pOpPoint->Cpi.at(j),7,'f',4);
                }

                out << strong;
            }
            out << "\n\n";
        }
    }
    XFile.close();
}


void XDirect::onExportCurPolar()
{
    if(!curFoil() || !curPolar())    return;

    QString FileName, filter;

    if(SaveOptions::exportFileType()==xfl::TXT) filter = "Text File (*.txt)";
    else                                       filter = "Comma Separated Values (*.csv)";

    FileName = Objects2d::curPolar()->name();
    FileName.replace("/", " ");
    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export Polar",
                                            SaveOptions::lastDirName() + "/"+FileName,
                                            "Text File (*.txt);;Comma Separated Values (*.csv)",
                                            &filter);
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastDirName(FileName.left(pos));
    pos = FileName.lastIndexOf(".csv");
    if (pos>0) SaveOptions::setExportFileType(xfl::CSV);
    else       SaveOptions::setExportFileType(xfl::TXT);

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);
    //    curPolar()->exportPolar(out, VERSIONNAME, SaveOptions::exportFileType());
    out << onCopyCurPolarData();
    XFile.close();
}


QString XDirect::onCopyCurOppData()
{
    if(!curFoil() || !curPolar() || !curOpp())    return QString();

    OpPoint *pOpp = curOpp();
    QString sep = "  ";
    if(SaveOptions::exportFileType()==xfl::CSV) sep = SaveOptions::textSeparator()+ " ";

    QString oppdata;
    QTextStream out;
    out.setString(&oppdata);
    curOpp()->exportOpp(out, xfl::versionName(true), SaveOptions::exportFileType()==xfl::CSV, SaveOptions::textSeparator());

    QString strange;
    oppdata  += "x            " + sep;
    oppdata  += "Cp_inviscid  " + sep;
    oppdata  += "Cp_viscous   " + sep;
    oppdata  += "Q_inviscid   " + sep;
    oppdata  += "Q_viscous\n";
    for(int i=0; i<int(curFoil()->nNodes()); i++)
    {
        strange = QString::asprintf("%13.5g", curFoil()->x(i));      oppdata += strange + sep;
        strange = QString::asprintf("%13.5g", pOpp->Cpi.at(i));      oppdata += strange + sep;
        strange = QString::asprintf("%13.5g", pOpp->Cpv.at(i));      oppdata += strange + sep;
        strange = QString::asprintf("%13.5g", pOpp->Qi.at(i));       oppdata += strange + sep;
        strange = QString::asprintf("%13.5g", pOpp->Qv.at(i));       oppdata += strange;
        oppdata+="\n";
    }

    QApplication::clipboard()->setText(oppdata);
    s_pMainFrame->displayMessage("The operating point data has been copied to the clipboard\n", false);

    return oppdata;
}


QString XDirect::onCopyCurPolarData()
{
    if(!curFoil() || !curPolar())    return QString();
    QString sep = "  ";
    if(SaveOptions::exportFileType()==xfl::CSV) sep = SaveOptions::textSeparator();

    Polar *pPolar = curPolar();
    QString polardata;
    QString strange;

    QTextStream out;

    pPolar->exportPolar(out, xfl::versionName(true), true, SaveOptions::exportFileType()==xfl::CSV);
    polardata += "\n";

    polardata += " alpha       " + sep;
    polardata += "Cl           " + sep;
    polardata += "XCp          " + sep;
    polardata += "Cd           " + sep;
    polardata += "Cdp          " + sep;
    polardata += "Cm           " + sep;
    polardata += "XtrTop       " + sep;
    polardata += "XtrBot       " + sep;
    polardata += "HMom         " + sep;
    polardata += "Cpmn         " + sep;
    polardata += "Cl/cd        " + sep;
    polardata += "Cl32/Cd      " + sep;
    polardata += "sqrt(Cl)     " + sep;
    polardata += "Re           " ;
    polardata += "\n";

    for(int i=0; i<pPolar->m_Alpha.count(); i++)
    {
        strange = QString::asprintf("%13.5g", pPolar->m_Alpha.at(i));    polardata += strange + sep;
        strange = QString::asprintf("%13.5g", pPolar->m_Cl.at(i));       polardata += strange + sep;
        strange = QString::asprintf("%13.5g", pPolar->m_XCp.at(i));      polardata += strange + sep;
        strange = QString::asprintf("%13.5g", pPolar->m_Cd.at(i));       polardata += strange + sep;
        strange = QString::asprintf("%13.5g", pPolar->m_Cdp.at(i));      polardata += strange + sep;
        strange = QString::asprintf("%13.5g", pPolar->m_Cm.at(i));       polardata += strange + sep;
        strange = QString::asprintf("%13.5g", pPolar->m_XTrTop.at(i));     polardata += strange + sep;
        strange = QString::asprintf("%13.5g", pPolar->m_XTrBot.at(i));     polardata += strange + sep;
        strange = QString::asprintf("%13.5g", pPolar->m_HMom.at(i));     polardata += strange + sep;
        strange = QString::asprintf("%13.5g", pPolar->m_Cpmn.at(i));     polardata += strange + sep;
        strange = QString::asprintf("%13.5g", pPolar->m_ClCd.at(i));     polardata += strange + sep;
        strange = QString::asprintf("%13.5g", pPolar->m_Cl32Cd.at(i));   polardata += strange + sep;
        strange = QString::asprintf("%13.5g", pPolar->m_RtCl.at(i));     polardata += strange + sep;
        strange = QString::asprintf("%13.5g", pPolar->m_Re.at(i));       polardata += strange;
        polardata += '\n';
    }

    QApplication::clipboard()->setText(polardata);
    s_pMainFrame->displayMessage("The polar data has been copied to the clipboard\n", false);

    return polardata;
}


void XDirect::onFoilCoordinates()
{
    if(!curFoil())    return;

    stopAnimate();

    FoilCoordDlg fcoDlg(s_pMainFrame);
    fcoDlg.initDialog(curFoil());

    if(fcoDlg.exec() == QDialog::Accepted)
    {
        Foil *pNewFoil = new Foil();
        pNewFoil->copy(fcoDlg.bufferFoil());

        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            m_pFoilTable->updateTable();
            m_pFoilTable->selectFoil(pNewFoil);
            m_pDFoilWt->resetLegend();

            emit projectModified();
            updateView();
            return;
        }
        else delete pNewFoil;
    }
}


void XDirect::onFoilFromCoords()
{
    FoilCoordDlg dlg(s_pMainFrame);
    dlg.initDialog(nullptr);
    if(dlg.exec()==QDialog::Accepted)
    {
        Foil *pNewFoil = new Foil;
        pNewFoil->copy(dlg.bufferFoil());
        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            m_pFoilTable->updateTable();
            m_pFoilTable->selectFoil(pNewFoil);
            m_pDFoilWt->resetLegend();

            emit projectModified();
            updateView();
            return;
        }
        else delete pNewFoil;
    }
}


void XDirect::onHideAllOpps()
{
    for (int i=0; i<Objects2d::nOpPoints(); i++)
    {
        OpPoint *pOpp = Objects2d::opPointAt(i);
        pOpp->hide();
    }
    emit projectModified();
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();
    m_pFoilTreeView->update();
    updateView();
}


void XDirect::onHideAllPolars()
{
    for (int i=0; i<Objects2d::nPolars(); i++)
    {
        Polar *pPolar = Objects2d::polarAt(i);
        pPolar->setVisible(false);
    }
    emit projectModified();
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();
    m_pFoilTreeView->update();
    updateView();
}


void XDirect::onHideFoilPolars()
{
    if(!curFoil()) return;

    for (int i=0; i<Objects2d::nPolars(); i++)
    {
        Polar *pPolar = Objects2d::polarAt(i);
        if(pPolar->foilName() == curFoil()->name())
        {
            pPolar->setVisible(false);
        }
    }
    emit projectModified();
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();
    updateView();
}


void XDirect::onHideFoilOpps()
{
    if(!curFoil() || !Objects2d::curPolar()) return;

    for(int i=0; i<Objects2d::nOpPoints(); i++)
    {
        OpPoint *pOpp = Objects2d::opPointAt(i);
        if(pOpp->foilName()==curFoil()->name())
            pOpp->hide();
    }
    emit projectModified();
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();
    updateView();
}



void XDirect::onHidePolarOpps()
{
    if(!curFoil() || !Objects2d::curPolar()) return;

    for(int i=0; i<Objects2d::nOpPoints(); i++)
    {
        OpPoint *pOpp = Objects2d::opPointAt(i);
        if(pOpp->foilName()==curFoil()->name() && pOpp->polarName()==Objects2d::curPolar()->name())
            pOpp->hide();
    }
    emit projectModified();
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();
    updateView();
}


void XDirect::onImportXFoilPolars()
{
    QStringList pathNames;
    pathNames = QFileDialog::getOpenFileNames(s_pMainFrame, "Open File",
                                              SaveOptions::lastDirName(),
                                              "XFoil polar format (*.*)");

    if(!pathNames.size()) return ;
    int pos = pathNames.at(0).lastIndexOf("/");
    if(pos>0) SaveOptions::setLastDirName(pathNames.at(0).left(pos));

    QString logmsg;
    Polar *pPolar(nullptr);

    for(int iFile=0; iFile<pathNames.size(); iFile++)
    {
        logmsg.clear();
        QFile XFile(pathNames.at(iFile));
        pPolar = importXFoilPolar(XFile, logmsg);
        if(!pPolar)
        {
            s_pMainFrame->onShowLogWindow(true);
            s_pMainFrame->displayMessage(logmsg+"\n", false);
            continue;
        }
        else
        {
            s_pMainFrame->displayMessage("The file "+pathNames.at(iFile) +" has been read successfully\n", false);
        }

        Foil *pFoil = Objects2d::foil(pPolar->foilName());
        if(!pFoil)
        {
            QString strange = "No Foil with the name "+pPolar->foilName()+ " could be found. "
                              "The polar " +XFile.fileName() + " will not be stored.\n";
            delete pPolar;
            pPolar = nullptr;
            s_pMainFrame->displayMessage(strange, false);
            s_pMainFrame->onShowLogWindow(true);
            continue;
        }
        pPolar->setTheStyle(pFoil->theStyle());
        Objects2d::insertPolar(pPolar);
    }


    setCurOpp(nullptr);
    setPolar(pPolar);
    m_pFoilTreeView->updateObjectView();
    m_pFoilTable->updateTable();

    updateView();
    emit projectModified();
}


/**
 * The user has requested to import a polar from a text file in JavaFoil format
 * The Polar will be added to the array only if a Foil with the parent name exists.
 *  @todo Note: this option has not been tested in years... the JavaFoil format may have changed since
 */
void XDirect::onImportJavaFoilPolar()
{
    QString FoilName;
    QString strong, str;

    QString PathName;
    bool bOK(false);
    QByteArray textline;

    PathName = QFileDialog::getOpenFileName(s_pMainFrame, "Open File",
                                            SaveOptions::lastDirName(),
                                            "JavaFoil Polar Format (*.*)");
    if(!PathName.length())        return ;
    int pos = PathName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastDirName(PathName.left(pos));

    QFile XFile(PathName);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange = "Could not open the file "+PathName;
        s_pMainFrame->displayMessage(strange+"\n", false);
        s_pMainFrame->onShowLogWindow(true);
        return;
    }

    QTextStream in(&XFile);

    bool bIsReading = true;
    int res(0), Line(0);
    double Re(0);

    double alpha(0), CL(0), CD(0), CM(0), Xt(0), Xb(0);

    Line = 0;
    if(!xfl::readAVLString(in, Line, FoilName)) return;


    FoilName = FoilName.trimmed();

    if(!Objects2d::foil(FoilName))
    {
        str = "No Foil with the name "+FoilName;
        str+= "\ncould be found. The polar(s) will not be stored.";
        s_pMainFrame->displayMessage(str+"\n", false);
        s_pMainFrame->onShowLogWindow(true);

        return;
    }
    if(!xfl::readAVLString(in, Line, strong)) return; //blank line

    while(bIsReading)
    {
        if(!xfl::readAVLString(in, Line, strong)) break; //Re number

        strong = strong.right(strong.length()-4);
        Re = strong.toDouble(&bOK);
        if(!bOK)
        {
            bIsReading = false;
        }
        else
        {
            Polar *pPolar = new Polar();
            pPolar->setFoilName(FoilName);
            pPolar->setReynolds(Re);
            pPolar->setName(QString("T%1_Re2_M3_JavaFoil")
                                 .arg(pPolar->type())
                                 .arg(pPolar->Reynolds()/1000000.0,0,'f',2)
                                 .arg(pPolar->Mach(),0,'f',2));

            QColor clr =xfl::randomColor(!DisplayOptions::isLightTheme());
            pPolar->setLineColor(clr);
            insertNewPolar(pPolar);
            setCurPolar(pPolar);

            if(!xfl::readAVLString(in, Line, strong)) break;//?    Cl    Cd    Cm 0.25    TU    TL    SU    SL    L/D
            if(!xfl::readAVLString(in, Line, strong)) break;//[?]    [-]    [-]    [-]    [-]    [-]    [-]    [-]    [-]

            res = 6;
            while(res==6)
            {
                bIsReading  = xfl::readAVLString(in, Line, strong);//values
                if(!bIsReading) break;
                strong = strong.trimmed();
                if(strong.length())
                {
                    strong.replace(',', '.');

                    textline = strong.toLatin1();
//                    text = textline.constData();
//                    res = sscanf(text, "%lf%lf%lf%lf%lf%lf",&alpha, &CL, &CD, &CM, &Xt, &Xb);

                    QTextStream(&textline) >> alpha >> CL >> CD >> CM >> Xt >> Xb;
                    pPolar->addPoint(alpha, CD, 0.0, CL, CM, 0.0, 0.0, Re, 0.0, 0.0, Xt, Xb, 0, 0, 0, 0);
                }
                else
                {
                    res = 0;
                }
            }
        }
        setCurOpp(nullptr);
        setPolar();
        m_pFoilTreeView->updateObjectView();
        m_pFoilTable->updateTable();

        updateView();
        emit projectModified();
    }
}


/**
 * The user has requested the launch of the interface to create a foil from the interpolation of two existing Foil objects.
 */
void XDirect::onInterpolateFoils()
{
    if(Objects2d::nFoils()<2)
    {
        s_pMainFrame->displayMessage("At least two foils are required for interpolation.\n\n", false);
        s_pMainFrame->onShowLogWindow(true);

        return;
    }

    stopAnimate();

    InterpolateFoilsDlg ifDlg(s_pMainFrame);

    ifDlg.initDialogFoils();

    if(ifDlg.exec() == QDialog::Accepted)
    {
        Foil *pNewFoil = new Foil;
        pNewFoil->copy(ifDlg.interpolatedFoil());
        pNewFoil->setName(ifDlg.bufferFoil()->name());

        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            m_pFoilTable->updateTable();
            m_pFoilTable->selectFoil(pNewFoil);
            m_pDFoilWt->resetLegend();

            emit projectModified();
            updateView();
            return;
        }
        else delete pNewFoil;
    }
}


void XDirect::onCircleFoil()
{
    stopAnimate();

    Foil* pCurFoil = curFoil();
    OpPoint* pCurOpp  = curOpp();
    setCurFoil(nullptr);
    setCurOpp(nullptr);

    m_bResetCurves = true;

    updateView();

    QStringList left({"Radius","Number of points:"});
    QStringList right({""});
    QVector<double> vals({0.5f, 100});

    DoubleValueDlg  dDlg(s_pMainFrame, vals, left, right);

    if (dDlg.exec() == QDialog::Accepted)
    {
        Foil *pCircleFoil = new Foil();
        pCircleFoil->clearPointArrays();
        double radius = dDlg.value(0);
        int N = dDlg.value(1);
        double xc=0.5f;
        double yc=0.0;
        for(int i=0; i<N; i++)
        {
            double theta = double(i)/double(N-1)*2.0*PI;
            pCircleFoil->appendBasePoint(xc+radius*cos(theta), yc+radius*sin(theta));
        }

        pCircleFoil->setLineStipple(Line::SOLID);
        pCircleFoil->setPointStyle(Line::NOSYMBOL);

        pCircleFoil->setLineColor(xfl::randomColor(DisplayOptions::isDarkTheme()));

        pCircleFoil->setName("Circular foil");

        setCurOpp(nullptr);

        if(addNewFoil(pCircleFoil))
        {
        }
        else
        {
            delete pCircleFoil;
            return;
        }

        pCircleFoil->initGeometry();
        setFoil(pCircleFoil);
        m_pFoilTreeView->insertFoil(pCircleFoil);
        m_pFoilTreeView->selectFoil(pCircleFoil);

        m_pFoilTable->updateTable();
        m_pFoilTable->selectFoil(pCircleFoil);
        m_pDFoilWt->resetLegend();

        emit projectModified();
    }
    else
    {
        setCurFoil(pCurFoil);
        setCurOpp(pCurOpp);
    }
    setControls();
    updateView();
}



void XDirect::onSquareFoil()
{
    stopAnimate();

    Foil* pCurFoil = curFoil();
    OpPoint* pCurOpp  = curOpp();
    setCurFoil(nullptr);
    setCurOpp(nullptr);

    m_bResetCurves = true;

    updateView();

    QStringList left({"Side","Rotation angle"});
    QStringList right({"","degrees"});
    QVector<double> vals({0.5f,0.0});

    DoubleValueDlg  dDlg(s_pMainFrame, vals, left, right);

    if (dDlg.exec() == QDialog::Accepted)
    {
        Foil *pSquareFoil = new Foil();
        pSquareFoil->clearPointArrays();
        double side = dDlg.value(0);
        double theta = (45.0+dDlg.value(1))*PI/180.0;
        double xc=side;
        double yc=0.0;
        for(int i=0; i<5; i++)
        {

            pSquareFoil->appendBasePoint(xc+side*sqrt(2.0)*cos(float(i)*PI/2.0+theta), yc+side*sqrt(2.0)*sin(float(i)*PI/2.0+theta));
        }

        pSquareFoil->setLineStipple(Line::SOLID);
        pSquareFoil->setPointStyle(Line::NOSYMBOL);

        pSquareFoil->setLineColor(xfl::randomColor(DisplayOptions::isDarkTheme()));

        pSquareFoil->setName("Square foil");

        setCurOpp(nullptr);

        if(addNewFoil(pSquareFoil))
        {
        }
        else
        {
            delete pSquareFoil;
            return;
        }

        pSquareFoil->initGeometry();
        setFoil(pSquareFoil);
        m_pFoilTreeView->insertFoil(pSquareFoil);
        m_pFoilTreeView->selectFoil(pSquareFoil);

        m_pFoilTable->updateTable();
        m_pFoilTable->selectFoil(pSquareFoil);
        m_pDFoilWt->resetLegend();

        emit projectModified();
    }
    else
    {
        setCurFoil(pCurFoil);
        setCurOpp(pCurOpp);
    }
    setControls();
    updateView();
}


void XDirect::onNacaFoils()
{
    stopAnimate();

    Foil* pCurFoil = curFoil();
    setCurFoil(nullptr);
    setCurOpp(nullptr);

    m_bResetCurves = true;

    updateView();

    NacaFoilDlg nacaDlg(s_pMainFrame);
    nacaDlg.initDialog();
    if (nacaDlg.exec() == QDialog::Accepted)
    {
        Foil *pNewFoil = new Foil();
        pNewFoil->copy(nacaDlg.pNACAFoil());
        pNewFoil->setLineStipple(Line::SOLID);
        pNewFoil->setPointStyle(Line::NOSYMBOL);

        pNewFoil->setLineColor(xfl::randomColor(DisplayOptions::isDarkTheme()));

        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            m_pFoilTable->updateTable();
            m_pFoilTable->selectFoil(pNewFoil);
            m_pDFoilWt->resetLegend();

            emit projectModified();
            m_bResetCurves = true;
            updateView();
        }
        else delete pNewFoil;
    }
    else
    {
        setCurFoil(pCurFoil);
    }
    setControls();
    updateView();
}


void XDirect::onDerotateFoil()
{
    stopAnimate();

    Foil* pCurFoil = curFoil();
    if(!pCurFoil) return;

    m_bResetCurves = true;

    updateView();

    FoilNormalizeDlg normdlg(s_pMainFrame);
    normdlg.initDialog(curFoil());
    if (normdlg.exec() == QDialog::Accepted)
    {
        Foil *pNormFoil = new Foil();
        pNormFoil->copy(normdlg.bufferFoil());
        pNormFoil->setLineStipple(Line::SOLID);
        pNormFoil->setPointStyle(Line::NOSYMBOL);
        pNormFoil->setTheStyle(curFoil()->theStyle());
        pNormFoil->setName(curFoil()->name());

        if(addNewFoil(pNormFoil))
        {
        }
        else
        {
            delete pNormFoil;
            return;
        }

        pNormFoil->initGeometry();
        setFoil(pNormFoil);
        m_pFoilTreeView->insertFoil(pNormFoil);
        m_pFoilTreeView->selectFoil(pNormFoil);

        m_pFoilTable->updateTable();
        m_pFoilTable->selectFoil(pNormFoil);
        m_pDFoilWt->resetLegend();

        emit projectModified();
    }
    else
    {
        setCurFoil(pCurFoil);
    }
    setControls();
    updateView();
}


void XDirect::onDesignView()
{
    if(isDesignView()) return;

    for(int i=0; i<Objects2d::nFoils();i++)
    {
        Foil *pFoil = Objects2d::foil(i);
        pFoil->applyBase(); // cancel TE flaps in this view
    }

    m_eView = DESIGNVIEW;
    s_pMainFrame->setActiveCentralWidget();

    m_bResetCurves = true;

    setGraphTiles();
    showDockWidgets();
    setControls();
    updateView();
}


void XDirect::onBLView()
{
    if(isBLView()) return;

    m_eView = BLVIEW;
    s_pMainFrame->setActiveCentralWidget();

    m_bResetCurves = true;
    m_pAnalysisControls->onSetControls();

    setGraphTiles();
    showDockWidgets();
    setControls();
    updateView();
}


void XDirect::onOpPointView()
{
    if(isOppView()) return;

    m_eView = OPPVIEW;
    s_pMainFrame->setActiveCentralWidget();

    m_bResetCurves = true;

    showDockWidgets();
    setControls();
    updateView();
}


void XDirect::onPolarView()
{
    if(isPolarView()) return;
    m_eView = POLARVIEW;
    s_pMainFrame->setActiveCentralWidget();

    m_bResetCurves = true;

    setGraphTiles();
    showDockWidgets();
    setControls();
    updateView();
}


void XDirect::onFoilScale()
{
    if(!curFoil()) return;

    stopAnimate();

    FoilScaleDlg fgeDlg(s_pMainFrame);
    fgeDlg.initDialog(curFoil());

    if(fgeDlg.exec() == QDialog::Accepted)
    {
        Foil *pNewFoil = new Foil();
        pNewFoil->copy(fgeDlg.bufferFoil());

        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            m_pFoilTable->updateTable();
            m_pFoilTable->selectFoil(pNewFoil);
            m_pDFoilWt->resetLegend();

            emit projectModified();
            updateView();
            return;
        }
        else delete pNewFoil;
    }
}


void XDirect::onRefineGlobally()
{
    if(!curFoil()) return;
    stopAnimate();

    FoilRepanelDlg tdpDlg(s_pMainFrame);
    tdpDlg.initDialog(curFoil());

    if(QDialog::Accepted == tdpDlg.exec())
    {
        Foil *pNewFoil = new Foil;
        pNewFoil->copy(tdpDlg.bufferFoil());
        pNewFoil->setName(curFoil()->name());
        pNewFoil->setTheStyle(curFoil()->theStyle());
        pNewFoil->show();
        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            m_pFoilTable->updateTable();
            m_pFoilTable->selectFoil(pNewFoil);
            m_pDFoilWt->resetLegend();

            emit projectModified();
            m_bResetCurves = true;
            updateView();
        }
        else delete pNewFoil;
    }

    updateView();
}


void XDirect::onFoilFrom1Spline()
{
    stopAnimate();

    Foil1SplineDlg fsDlg(s_pMainFrame);
    fsDlg.initDialog(nullptr);

    if(QDialog::Accepted == fsDlg.exec())
    {
        Foil *pNewFoil = new Foil;
        pNewFoil->copy(fsDlg.bufferFoil());
        pNewFoil->show();
        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            m_pFoilTable->updateTable();
            m_pFoilTable->selectFoil(pNewFoil);
            m_pDFoilWt->resetLegend();

            emit projectModified();
            m_bResetCurves = true;
            updateView();
        }
        else delete pNewFoil;
    }
}


void XDirect::onFoilFrom2Splines()
{
    stopAnimate();

    Foil2SplineDlg fsDlg(s_pMainFrame);
    fsDlg.initDialog(nullptr);

    if(QDialog::Accepted == fsDlg.exec())
    {
        Foil *pNewFoil = new Foil;
        pNewFoil->copy(fsDlg.bufferFoil());

        pNewFoil->setTheStyle(Foil2SplineDlg::s_SF.theStyle());

        pNewFoil->show();
        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            m_pFoilTable->updateTable();
            m_pFoilTable->selectFoil(pNewFoil);
            m_pDFoilWt->resetLegend();

            emit projectModified();
            m_bResetCurves = true;
            updateView();
        }
        else delete pNewFoil;
    }
}


void XDirect::onFoilFromCamber()
{
    stopAnimate();

    FoilCamberDlg fsDlg(s_pMainFrame);
    fsDlg.initDialog(nullptr);

    if(QDialog::Accepted == fsDlg.exec())
    {
        Foil *pNewFoil = new Foil;
        pNewFoil->copy(fsDlg.bufferFoil());

//        pNewFoil->setTheStyle(SplineFoilDlg::s_SF.theStyle());

        pNewFoil->show();
        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            m_pFoilTable->updateTable();
            m_pFoilTable->selectFoil(pNewFoil);
            m_pDFoilWt->resetLegend();

            emit projectModified();
            m_bResetCurves = true;
            updateView();
        }
        else delete pNewFoil;
    }
}


/**
 * The user has requested the display of the velocity in the Cp graph.
 */
void XDirect::onOppGraph()
{
    onOpPointView();
    if(m_pOppGraph->yVariable(0)==1) return;

    if(m_pOppGraph->yVariable(0)!=1)
    {
        m_pOppGraph->resetLimits();
        m_pOppGraph->setAuto(true);
        m_pOppGraph->setYVariable(0,1);
    }
    m_pOppGraph->setYInverted(0,false);
    m_bResetCurves = true;

//    setControls();

    //    CpGraph()->setXScale();
    m_pOppGraph->invalidate();
    m_pOpPointWt->setFoilScale(false);
    updateView();
}


void XDirect::onRenameCurPolar()
{
    if(!curPolar()) return;
    if(!curFoil()) return;

    int resp, k,l;
    Polar* pPolar = nullptr;
    OpPoint * pOpp = nullptr;
    QString OldName = Objects2d::curPolar()->name();

    RenameDlg renameDlg(s_pMainFrame);
    renameDlg.initDialog(curPolar()->name(), Objects2d::polarList(curFoil()), "Enter the new name for the foil polar:");

    bool bExists = true;

    while (bExists)
    {
        resp = renameDlg.exec();
        if(resp==QDialog::Accepted)
        {
            if (OldName == renameDlg.newName()) return;
            //Is the new name already used?
            bExists = false;
            for (k=0; k<Objects2d::nPolars(); k++)
            {
                pPolar = Objects2d::polarAt(k);
                if ((pPolar->foilName()==curFoil()->name()) && (pPolar->name() == renameDlg.newName()))
                {
                    bExists = true;
                    break;
                }
            }
            if(!bExists)
            {
                for (l=Objects2d::nOpPoints()-1;l>=0; l--)
                {
                    pOpp = Objects2d::opPointAt(l);
                    if (pOpp->polarName() == OldName &&  pOpp->foilName() == curFoil()->name())
                    {
                        pOpp->setPolarName(renameDlg.newName());
                    }
                }
                Objects2d::renamePolar(curPolar(), renameDlg.newName());
            }
            emit projectModified();
        }
        else if(resp ==10)
        {
            //user wants to overwrite
            if (OldName == renameDlg.newName()) return;
            for (k=0; k<Objects2d::nPolars(); k++)
            {
                pPolar = Objects2d::polarAt(k);
                if (pPolar->name() == renameDlg.newName())
                {
//                    bExists = true;
                    break;
                }
            }
            for (l=Objects2d::nOpPoints()-1;l>=0; l--)
            {
                pOpp = Objects2d::opPointAt(l);
                if (pOpp->polarName() == Objects2d::curPolar()->name())
                {
                    Objects2d::removeOpPointAt(l);
                    if(pOpp==curOpp()) setCurOpp(nullptr);
                    delete pOpp;
                }
            }
            Objects2d::removePolarAt(k);
            if(pPolar==Objects2d::curPolar()) setCurPolar(nullptr);
            delete pPolar;

            //and rename everything
            Objects2d::curPolar()->setName(renameDlg.newName());

            for (l=Objects2d::nOpPoints()-1;l>=0; l--)
            {
                pOpp = Objects2d::opPointAt(l);
                if (pOpp->polarName() == OldName &&    pOpp->foilName() == curFoil()->name())
                {
                    pOpp->setPolarName(renameDlg.newName());
                }
            }

            bExists = false;
            emit projectModified();
        }
        else
        {
            return ;//cancelled
        }
    }

    m_pFoilTreeView->updateObjectView();
    m_pFoilTable->updateTable();

    updateView();
}


void XDirect::onFoilProps()
{
    if(!Objects2d::curFoil()) return;
    ObjectPropsDlg *pOPDlg = new ObjectPropsDlg(s_pMainFrame);
    QString strangeProps;
    strangeProps = Objects2d::curFoil()->properties(true);
    pOPDlg->initDialog(Objects2d::curFoil()->name(), strangeProps);
    pOPDlg->show();
}


void XDirect::onPolarProps()
{
    if(!Objects2d::curPolar()) return;
    ObjectPropsDlg *pOPDlg = new ObjectPropsDlg(s_pMainFrame);
    QString strangeProps;
    Objects2d::curPolar()->getProperties(curFoil(), strangeProps);
    pOPDlg->initDialog(Objects2d::curPolar()->name(), strangeProps);
    pOPDlg->show();
}


void XDirect::onOpPointProps()
{
    if(!curOpp()) return;
    ObjectPropsDlg *pOPDlg = new ObjectPropsDlg(s_pMainFrame);
    QString strangeProps;
    curOpp()->getOppProperties(strangeProps,SaveOptions::textSeparator());
    pOPDlg->initDialog("Operating point properties", strangeProps);
    pOPDlg->show();
}


/**
 * The user has requested to reset all polar graph scales to their automatic default value
 */
void XDirect::onResetAllPolarGraphsScales()
{
    for(int ig=0; ig<m_PlrGraph.count(); ig++)
    {
        m_PlrGraph[ig]->setAuto(true);
        m_PlrGraph[ig]->resetXLimits();
        m_PlrGraph[ig]->resetYLimits();
    }
    updateView();
}


/**
 * The user has requested the deletion of the dataof the current Polar.
 * The associated OpPoint objects will be deleted too.
 */
void XDirect::onResetCurPolar()
{
    if(!Objects2d::curPolar()) return;
    Objects2d::curPolar()->reset();

    OpPoint*pOpp;
    for(int i=Objects2d::nOpPoints()-1;i>=0;i--)
    {
        pOpp = Objects2d::opPointAt(i);
        if(pOpp->foilName()==curFoil()->name() && pOpp->polarName()==Objects2d::curPolar()->name())
        {
            Objects2d::removeOpPointAt(i);
            delete pOpp;
        }
    }
    setCurOpp(nullptr);

    m_pFoilTreeView->updateObjectView();
    m_pFoilTable->updateTable();

    m_bResetCurves = true;
    updateView();

    emit projectModified();
}


void XDirect::onSavePolars()
{
    if(!curFoil() || !Objects2d::nPolars()) return;

    QString FileName;
    FileName = curFoil()->name() + ".plr";
    FileName.replace("/", " ");

    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Polar File",
                                            SaveOptions::plrPolarDirName()+"/"+FileName,
                                            "Polar File (*.plr)");
    if(!FileName.length()) return;

    QString strong = FileName.right(4);
    if(strong !=".plr" && strong !=".PLR") FileName += ".plr";

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly)) return;

//    QFileInfo fi(FileName);

    QDataStream ar(&XFile);

#if QT_VERSION >= 0x040500
    ar.setVersion(QDataStream::Qt_4_5);
#endif
    ar.setByteOrder(QDataStream::LittleEndian);

    writeFoilPolars(ar, curFoil());

    XFile.close();
}


Polar* XDirect::insertNewPolar(Polar *pNewPolar)
{
    if(!pNewPolar) return nullptr;

    bool bExists = true;

    //check if this Polar is already inserted
    for(int ip=0; ip<Objects2d::nPolars(); ip++)
    {
        Polar const*pOldPolar = Objects2d::polarAt(ip);
        if(pOldPolar==pNewPolar)
        {
            return pNewPolar;
        }
    }

    //make a list of existing names
    QStringList NameList;
    for(int k=0; k<Objects2d::nPolars(); k++)
    {
        Polar const*pPolar = Objects2d::polarAt(k);
        if(pPolar->foilName()==pNewPolar->foilName())
            NameList.append(pPolar->name());
    }

    //Is the new Polar's name already used?
    bExists = false;
    for (int k=0; k<NameList.count(); k++)
    {
        if(pNewPolar->name()==NameList.at(k))
        {
            bExists = true;
            break;
        }
    }

    if(!bExists)
    {
        Objects2d::insertPolar(pNewPolar);
        return pNewPolar;
    }

    // an old object with the Polar's name exists for this foil, ask for a new one
    RenameDlg dlg;
    Foil *pFoil = Objects2d::foil(pNewPolar->foilName());
    if(!pFoil) return nullptr;
    dlg.initDialog(pNewPolar->name(), Objects2d::polarList(curFoil()), "Enter the Polar's new name:");
    int resp = dlg.exec();

    if(resp==10)
    {
        //user wants to overwrite an existing name
        //so find the existing Polar with that name

        Objects2d::insertPolar(pNewPolar);

        return pNewPolar;

    }
    else if(resp==QDialog::Rejected)
    {
        return nullptr;
    }
    else if(resp==QDialog::Accepted)
    {
        //not rejected, no overwrite, else the user has selected a non-existing name, rename and insert
        pNewPolar->setName(dlg.newName());
        Objects2d::insertPolar(pNewPolar);

        return pNewPolar;

    }
    return nullptr;//should never get here
}


void XDirect::onSetFlap()
{
    if(!curFoil()) return;
    stopAnimate();

    setCurOpp(nullptr);
    m_bResetCurves = true;

    FoilFlapDlg flpDlg(s_pMainFrame);
    flpDlg.initDialog(curFoil());

    if(QDialog::Accepted == flpDlg.exec())
    {
        Foil *pNewFoil = new Foil();
        pNewFoil->copy(flpDlg.bufferFoil());
        pNewFoil->setTheStyle(curFoil()->theStyle());
        QString newfoilname = curFoil()->name();
        pNewFoil->setName(newfoilname);
        pNewFoil->setTheStyle(curFoil()->theStyle());
        pNewFoil->show();
        pNewFoil->applyBase();

        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            m_pFoilTable->updateTable();
            m_pFoilTable->selectFoil(pNewFoil);
            m_pDFoilWt->resetLegend();

            emit projectModified();
        }
        else delete pNewFoil;
    }

    updateView();
}


void XDirect::onSetLERadius()
{
    Foil *pFoil= curFoil();
    if(!pFoil) return;
    stopAnimate();

    setCurOpp(nullptr);
    m_bResetCurves = true;

    FoilLEDlg lDlg(s_pMainFrame);
    lDlg.initDialog(pFoil);

    if(QDialog::Accepted == lDlg.exec())
    {
        Foil *pNewFoil = new Foil();
        pNewFoil->copy(lDlg.bufferFoil());
        pNewFoil->setTheStyle(pFoil->theStyle());

        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            m_pFoilTable->updateTable();
            m_pFoilTable->selectFoil(pNewFoil);
            m_pDFoilWt->resetLegend();

            emit projectModified();
            updateView();
            return;
        }
        else
            delete pNewFoil;
    }

    updateView();
}


void XDirect::onSetTEGap()
{
    if(!curFoil())    return;
    stopAnimate();

    setCurOpp(nullptr);
    m_bResetCurves = true;

    FoilTEGapDlg tegDlg(s_pMainFrame);
    tegDlg.initDialog(curFoil());

    if(QDialog::Accepted == tegDlg.exec())
    {
        Foil *pNewFoil = new Foil();
        pNewFoil->copy(tegDlg.bufferFoil());
        pNewFoil->setTheStyle(curFoil()->theStyle());

        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            m_pFoilTable->updateTable();
            m_pFoilTable->selectFoil(pNewFoil);
            m_pDFoilWt->resetLegend();

            emit projectModified();
            updateView();
            return;
        }
        else delete pNewFoil;
    }
    updateView();
}


void XDirect::onShowAllOpps()
{
    s_bCurOppOnly = false;

    for (int i=0; i<Objects2d::nOpPoints(); i++)
    {
        OpPoint *pOpp = Objects2d::opPointAt(i);
        pOpp->show();
    }
    emit projectModified();
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();
    updateView();
}


void XDirect::onShowAllPolars()
{
    Polar *pPolar;
    for (int i=0; i<Objects2d::nPolars(); i++)
    {
        pPolar = Objects2d::polarAt(i);
        pPolar->show();
    }
    emit projectModified();
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();
    updateView();
}


void XDirect::onShowFoilPolarsOnly()
{
    if(!curFoil()) return;
    if(!isPolarView()) return;

    for (int i=0; i<Objects2d::nPolars(); i++)
    {
        Polar *pPolar = Objects2d::polarAt(i);
        pPolar->setVisible((pPolar->foilName() == curFoil()->name()));
    }
    emit projectModified();
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();
    updateView();
}


void XDirect::onShowFoilPolars()
{
    if(!curFoil()) return;
    Polar *pPolar;
    for (int i=0; i<Objects2d::nPolars(); i++)
    {
        pPolar = Objects2d::polarAt(i);
        if(pPolar->foilName() == curFoil()->name())
        {
            pPolar->setVisible(true);
        }
    }
    emit projectModified();
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();

    updateView();
}


void XDirect::onShowFoilOpps()
{
    if(!curFoil() || !Objects2d::curPolar()) return;

    OpPoint *pOpp;

    s_bCurOppOnly = false;

    for(int i=0; i<Objects2d::nOpPoints(); i++)
    {
        pOpp = Objects2d::opPointAt(i);
        if(pOpp->foilName()==curFoil()->name())
            pOpp->show();
    }
    emit projectModified();
    if(isOppView()) m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();

    updateView();
}


void XDirect::onShowPolarOpps()
{
    if(!curFoil() || !Objects2d::curPolar()) return;

    s_bCurOppOnly = false;

    for(int i=0; i<Objects2d::nOpPoints(); i++)
    {
        OpPoint *pOpp = Objects2d::opPointAt(i);
        if(pOpp->foilName()==curFoil()->name() && pOpp->polarName()==Objects2d::curPolar()->name())
            pOpp->show();
    }
    emit projectModified();
    if(isOppView()) m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();

    updateView();
}


void XDirect::saveSettings(QSettings &settings)
{
    settings.beginGroup("XDirect");
    {
        if      (m_eView==DESIGNVIEW)  settings.setValue("XDirectView", 0);
        else if (m_eView==BLVIEW)      settings.setValue("XDirectView", 1);
        else if (m_eView==OPPVIEW)     settings.setValue("XDirectView", 2);
        else if (m_eView==POLARVIEW)   settings.setValue("XDirectView", 3);

        s_lsTopBL.saveSettings(settings, "TopBLStyle");
        s_lsBotBL.saveSettings(settings, "BotBLStyle");

        settings.setValue("CurOppOnly", s_bCurOppOnly);
        settings.setValue("ShowInviscid", m_bShowInviscid);
        settings.setValue("PlrGraph", m_iPlrGraph);

        settings.setValue("iActivePolarGraph", s_pMainFrame->m_pPolarTiles->activeGraphWtIndex());
        settings.setValue("iActiveOppGraph", s_pMainFrame->m_pPolarTiles->activeGraphWtIndex());

        settings.setValue("KeepOpenErrors", s_bKeepOpenErrors);

        settings.setValue("FoilTreeSplitterSizes", m_pFoilTreeView->splitterSize());


        Foil2SplineDlg::s_SF.theStyle().saveSettings(settings, "SplineFoil");
        Foil1SplineDlg::Bspline().theStyle().saveSettings(settings, "BSpline");
        Foil1SplineDlg::C3Spline().theStyle().saveSettings(settings, "C3Spline");
        FoilCamberDlg::CSpline().theStyle().saveSettings(settings, "CamberSpline");
        FoilCamberDlg::TSpline().theStyle().saveSettings(settings, "ThickSpline");
    }
    settings.endGroup();

    OpPointWt::saveSettings(settings);
    EditPlrDlg::saveSettings(settings);
    FoilPolarDlg::saveSettings(settings);
    PolarAutoNameDlg::saveSettings(settings);
    XFoilBatchDlg::saveSettings(settings);
    XFoilAnalysisDlg::saveSettings(settings);
    XFoilTask::saveSettings(settings);
    FoilDlg::saveSettings(settings);
    OptimFoil::saveSettings(settings);

    m_pDFoilWt->saveSettings(settings);

    for(int ig=0; ig<m_BLGraph.count(); ig++)   m_BLGraph[ig]->saveSettings(settings);
    s_pMainFrame->m_pBLTiles->saveSettings(     settings, "XDirectBLTiles");

    for(int ig=0; ig<m_PlrGraph.count(); ig++)  m_PlrGraph[ig]->saveSettings(settings);
    s_pMainFrame->m_pPolarTiles->saveSettings(  settings, "XDirectPlrTiles");

    m_pOppGraph->saveSettings(settings);
}


Foil* XDirect::setFoil(Foil* pFoil)
{
    if(pFoil && pFoil==curFoil())
    {
        m_pAnalysisControls->onSetControls();
        return pFoil;
    }
    stopAnimate();

    setCurFoil(pFoil);
    setCurPolar(nullptr);
    setCurOpp(nullptr);

    if(!curFoil())
    {
        //take the first in the array, if any
        if(Objects2d::nFoils()>0)
        {
            setCurFoil(Objects2d::s_oaFoil.first());
        }
    }

//    setPolar();
    if(curFoil())
    {
        m_pOpPointWt->setFoilData(curFoil()->properties(false));
    }
    else m_pOpPointWt->setFoilData(QString());

    m_pDFoilWt->selectFoil(pFoil);

    return curFoil();
}


Polar *XDirect::setPolar(Polar *pPolar)
{
    if(pPolar && pPolar==curPolar()) return pPolar;

    stopAnimate();

    if(!curFoil()|| !curFoil()->name().length())
    {
        setCurPolar(nullptr);
        setCurOpp(nullptr);
        m_pAnalysisControls->onSetControls();
        return nullptr;
    }

    Foil *pCurFoil = curFoil();

    setCurPolar(pPolar);

    if(pPolar)
    {
        double theta = pPolar->TEFlapAngle();
        if(pCurFoil->hasTEFlap() && fabs(theta)>FLAPANGLEPRECISION)
        {
           pCurFoil->setTEFlapAngle(theta);
           pCurFoil->setFlaps();
        }
    }
    else
    {
        pCurFoil->setTEFlapAngle(0.0);
        pCurFoil->applyBase();
    }

    m_bResetCurves = true;
    m_pAnalysisControls->onSetControls();

//    setOpp();
    return pPolar;
}


OpPoint* XDirect::setOpp(double OpParameter)
{
    OpPoint *pOpp(nullptr);

    if(!curFoil() || !curPolar())
    {
        setCurOpp(nullptr);
        m_pOpPointWt->setOppData(QString());
        return nullptr;
    }

    if(OpParameter < -1234567.0) //the default
    {
        if(curOpp() && curOpp()->foilName() == curFoil()->name() && curOpp()->polarName()==Objects2d::curPolar()->name())
            pOpp = curOpp();
        else if(curOpp())
        {
            //try to use the same parameters
            if     (curPolar()->isControlPolar())  OpParameter = curOpp()->theta();
            else if(curPolar()->isFixedaoaPolar()) OpParameter = curOpp()->Reynolds();
            else                                              OpParameter = curOpp()->aoa();
            pOpp = Objects2d::opPointAt(curFoil(), curPolar(), OpParameter);
        }
    }
    else
    {
        pOpp = Objects2d::opPointAt(curFoil(), Objects2d::curPolar(), OpParameter);
    }

    if(!pOpp)
    {
        //if unsuccessful so far,
        //try to get the first one from the array
        for(int iOpp=0; iOpp<Objects2d::nOpPoints(); iOpp++)
        {
            OpPoint *pOldOpp = Objects2d::opPointAt(iOpp);
            if(pOldOpp->foilName()==curFoil()->name() && pOldOpp->polarName()==Objects2d::curPolar()->name())
            {
                pOpp = pOldOpp;
                break;
            }
        }
    }

    setOpp(pOpp);

    return curOpp();
}


void XDirect::setOpp(OpPoint *pOpPoint)
{
    if(pOpPoint && pOpPoint==curOpp()) return;
    setCurOpp(pOpPoint);
    m_bResetCurves = true;

    Foil *pFoil = curFoil();
    if (pFoil && pOpPoint)
    {
        if(fabs(pOpPoint->theta())>FLAPANGLEPRECISION)
        {
            pFoil->setTEFlapAngle(pOpPoint->theta());
            pFoil->setFlaps();
        }
        else pFoil->applyBase();
    }

    if(pOpPoint)
    {
        m_pOpPointWt->setOppData(oppData(curOpp()));
    }
    else m_pOpPointWt->setOppData(QString());
}


QString XDirect::oppData(OpPoint const *pOpp) const
{
    if(!pOpp) return QString();
    QString strong, OpPointProperties;
    int linelength = 25;

    strong = QString::asprintf("Re    = %9.0f ", pOpp->m_Reynolds);
    strong = strong.rightJustified(linelength,' ');
    OpPointProperties += strong +"\n";

    strong = ALPHACHAR + QString::asprintf("     = %9.3f", pOpp->m_Alpha);
    strong += DEGCHAR;
    strong = strong.rightJustified(linelength,' ');
    OpPointProperties += strong +"\n";

    strong = QString::asprintf("Mach  = %9.3f ", pOpp->m_Mach);
    strong = strong.rightJustified(linelength,' ');
    OpPointProperties += strong + "\n";

    strong = QString::asprintf("NCrit = %9.3f ", pOpp->NCrit);
    strong = strong.rightJustified(linelength,' ');
    OpPointProperties += strong + "\n";

    strong = QString::asprintf("Cl    = %9.3f ", pOpp->Cl);
    strong = strong.rightJustified(linelength,' ');
    OpPointProperties += strong + "\n";

    strong = QString::asprintf("Cd    = %9.3f ", pOpp->Cd);
    strong = strong.rightJustified(linelength,' ');
    OpPointProperties += strong + "\n";

    strong = QString::asprintf("Cl/Cd = %9.3f ", pOpp->Cl/pOpp->Cd);
    strong = strong.rightJustified(linelength,' ');
    OpPointProperties += strong + "\n";

    strong = QString::asprintf("Cm    = %9.3f ", pOpp->Cm);
    strong = strong.rightJustified(linelength,' ');
    OpPointProperties += strong + "\n";

    strong = QString::asprintf("Cdp   = %9.3f ", pOpp->Cdp);
    strong = strong.rightJustified(linelength,' ');
    OpPointProperties += strong + "\n";

    strong = QString::asprintf("Cpmin = %9.3f ", pOpp->Cpmn);
    strong = strong.rightJustified(linelength,' ');
    OpPointProperties += strong + "\n";

    strong = QString::asprintf("XCP   = %9.3f ", pOpp->XCP);
    strong = strong.rightJustified(linelength,' ');
    OpPointProperties += strong + "\n";

    strong = QString::asprintf("top trans. = %9.3f ", pOpp->XTrTop);
    strong = strong.rightJustified(linelength,' ');
    OpPointProperties += strong + "\n";

    strong = QString::asprintf("bot trans. = %9.3f ", pOpp->XTrBot);
    strong = strong.rightJustified(linelength,' ');
    OpPointProperties += strong + EOLCHAR;

    strong = THETACHAR + QString::asprintf("     = %9.3f", pOpp->theta()) + DEGCHAR;
    strong = strong.rightJustified(linelength,' ');
    OpPointProperties += strong;

    return OpPointProperties;
}


void XDirect::stopAnimate()
{
    m_pOpPointControls->stopAnimate();
}


void XDirect::updateView()
{
    if(m_bResetCurves)
    {
        if (isDesignView())
        {
            m_pDFoilWt->update();
        }
        else if (isBLView())
        {
            createBLCurves();
            s_pMainFrame->m_pBLTiles->makeLegend(true);
        }
        //remake the opp and polar curves always to highlight selection in separate window
        createOppCurves();
        createPolarCurves();
        s_pMainFrame->m_pPolarTiles->makeLegend(true);

        m_bResetCurves = false;
    }

    if     (isBLView())      s_pMainFrame->m_pBLTiles->update();
    else if(isOppView())     m_pOpPointWt->update();
    else if(isPolarView())   s_pMainFrame->m_pPolarTiles->update();

    s_pMainFrame->update(); // updates the toolbars and other buttons and legends
}


void XDirect::onDuplicateAnalyses()
{
    if(!curFoil()) return;

    AnalysisSelDlg dlg(s_pMainFrame);
    dlg.initDialog(curFoil(), nullptr, nullptr);
    if(dlg.exec()!=QDialog::Accepted) return;
    if(!dlg.selected2dPolars().size()) return;

    Polar* pNewPolar(nullptr);
    for(int ip=0; ip<dlg.selected2dPolars().size(); ip++)
    {
        Polar const *pOldPolar = dlg.selected2dPolar(ip);
        pNewPolar = new Polar;
        pNewPolar->copySpecification(pOldPolar);

        pNewPolar->setFoilName(curFoil()->name());
        pNewPolar->setName(pOldPolar->name());
        pNewPolar->setLineColor(curFoil()->lineColor());
        pNewPolar->setVisible(true);

        Objects2d::insertPolar(pNewPolar);
        if(pNewPolar) m_pFoilTreeView->insertPolar(pNewPolar);
    }

    if(pNewPolar)
    {
        setPolar(pNewPolar);
        m_pFoilTreeView->selectPolar(pNewPolar);
        setCurOpp(nullptr);
    }

    updateView();
    setControls();

    emit projectModified();
}


void XDirect::onDuplicateFoil()
{
    if(!curFoil()) return;

    Foil *pNewFoil = new Foil;
    pNewFoil->copy(curFoil());

    if(addNewFoil(pNewFoil))
    {
        m_pFoilTreeView->insertFoil(pNewFoil);
        m_pFoilTreeView->selectFoil(pNewFoil);
        m_pFoilTable->updateTable();
        m_pFoilTable->selectFoil(pNewFoil);
        m_pDFoilWt->resetLegend();

        setFoil(pNewFoil);
        emit projectModified();
    }
}


void XDirect::onRenameCurFoil()
{
    QString oldName = curFoil()->name();
    if(!renameFoil(curFoil())) return;
    setFoil(curFoil());

    if(oldName.compare(curFoil()->name())!=0)
    {
        m_pFoilTreeView->removeFoil(oldName);
        m_pFoilTreeView->insertFoil(curFoil());
        m_pFoilTreeView->selectFoil(curFoil());

        m_pFoilTable->updateTable();
        m_pFoilTable->selectFoil(curFoil());
        m_pDFoilWt->resetLegend();
    }

    m_pDFoilWt->updateView();

    emit projectModified();
}


void XDirect::onSetFoilDescription()
{
    if(!Objects2d::curFoil()) return;

    TextDlg dlg(Objects2d::curFoil()->description(), s_pMainFrame);

    if(dlg.exec() != QDialog::Accepted) return;
    Objects2d::curFoil()->setDescription(dlg.newText());
    m_pFoilTreeView->setObjectProperties();
}


Foil* XDirect::addNewFoil(Foil *pFoil)
{
    if(!pFoil) return nullptr;

    RenameDlg renDlg(s_pMainFrame);
    renDlg.initDialog(pFoil->name(), Objects2d::foilNames(), "Enter the foil's new name");

    if(renDlg.exec() != QDialog::Rejected)
    {
        pFoil->setName(renDlg.newName());
        Objects2d::insertThisFoil(pFoil);
        Objects2d::setCurPolar(nullptr);
        Objects2d::setCurOpp(nullptr);

        return pFoil;
    }
    return nullptr;
}


bool XDirect::renameFoil(Foil *pFoil)
{
    if(!pFoil) return false;

    RenameDlg renDlg(s_pMainFrame);
    renDlg.initDialog(pFoil->name(), Objects2d::foilNames(), "Enter the foil's new name");

    if(renDlg.exec() != QDialog::Rejected)
    {
        Objects3d::updateFoilName(pFoil->name(),  renDlg.newName());
        Objects2d::renameThisFoil(pFoil, renDlg.newName());
        return true;
    }
    return false;
}


void XDirect::setGraphTiles()
{
    if(isBLView())
    {
        switch(m_iBLView)
        {
            case xfl::ONEGRAPH:
                s_pMainFrame->m_pBLTiles->setGraphLayout(1, 0);
                break;
            case xfl::TWOGRAPHS:
                s_pMainFrame->m_pBLTiles->setGraphLayout(2, 0);
                break;
            case xfl::FOURGRAPHS:
                s_pMainFrame->m_pBLTiles->setGraphLayout(4, 0);
                break;
            default:
            case xfl::ALLGRAPHS:
                s_pMainFrame->m_pBLTiles->setGraphLayout(m_BLGraph.count(), 0);
                break;
        }
    }
    else if(isPolarView())
    {
        switch(s_pMainFrame->m_pPolarTiles->iView())
        {
            case xfl::ONEGRAPH:
                s_pMainFrame->m_pPolarTiles->setGraphLayout(1, m_iActiveGraph);
                break;
            case xfl::TWOGRAPHS:
                s_pMainFrame->m_pPolarTiles->setGraphLayout(2, 0);
                break;
            case xfl::FOURGRAPHS:
                s_pMainFrame->m_pPolarTiles->setGraphLayout(4, 0);
                break;
            default:
            case xfl::ALLGRAPHS:
                s_pMainFrame->m_pPolarTiles->setGraphLayout(m_PlrGraph.count(), 0);
                break;
        }
    }
}


void XDirect::onImportXMLAnalysis()
{
    QString PathName;
    PathName = QFileDialog::getOpenFileName(s_pMainFrame, "Open XML File",
                                            SaveOptions::xmlPolarDirName(),
                                            "Analysis XML file")+"(*.xml)";
    if(!PathName.length())        return ;

    QFile XFile(PathName);
    importAnalysisFromXML(XFile);
}


void XDirect::importAnalysisFromXML(QFile &xmlFile)
{
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        QString strange = "Could not open the file\n"+xmlFile.fileName();
        s_pMainFrame->displayMessage(strange+"\n", false);
        s_pMainFrame->onShowLogWindow(true);

        return;
    }

    Polar *pPolar = new Polar;
    XmlPolarReader polarReader(xmlFile, pPolar);
    polarReader.readXMLPolarFile();

    if(polarReader.hasError())
    {
        QString errorMsg = polarReader.errorString() + QString::asprintf("\nline %d column %d", int(polarReader.lineNumber()), int(polarReader.columnNumber()));
        s_pMainFrame->displayMessage(errorMsg+"\n", false);
        s_pMainFrame->onShowLogWindow(true);
    }
    else
    {
        Foil *pFoil = Objects2d::foil(pPolar->foilName());
        if(!pFoil && curFoil())
        {
            s_pMainFrame->displayMessage("Attaching the analysis to the active foil", false);
            pPolar->setFoilName(curFoil()->name());
//            pFoil = curFoil();
        }
        else if(!pFoil)
        {
            s_pMainFrame->displayMessage("No foil to which the polar %can be attached", true);
            delete pPolar;
            return;
        }

        insertNewPolar(pPolar);
        setCurOpp(nullptr);
        setCurPolar(pPolar);

        m_pFoilTreeView->updateObjectView();
        m_pFoilTable->updateTable();

        emit projectModified();
        setControls();
    }
    updateView();
}


void XDirect::onExportXMLAnalysis()
{
    if(!Objects2d::curPolar()) return ;// is there anything to export?

    Polar *pCurPolar = Objects2d::curPolar();
    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = pCurPolar->name();
    strong.replace("/", "_");
    strong.replace(".", "_");

    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export analysis definition to xml file",
                                            SaveOptions::xmlPolarDirName() +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;

    if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";


    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;


    XmlPolarWriter polarWriter(XFile);
    polarWriter.writeXMLPolar(pCurPolar);

    XFile.close();
}


void XDirect::updateFoilExplorers()
{
    m_pFoilTreeView->updateObjectView();
    m_pFoilTable->updateTable();
    m_pDFoilWt->resetLegend();
}


void XDirect::onStyleChanged()
{
    m_bResetCurves = true;
    updateView();
    emit projectModified();
}


void XDirect::onCurveClicked(Curve*pCurve)
{
    if(isOppView() || isBLView())
    {
        if(!pCurve)
        {
            for(int ig=0; ig<m_BLGraph.size(); ig++) m_BLGraph[ig]->clearSelection();
            LegendWt *pLegendWt = s_pMainFrame->m_pBLTiles->legendWt();
            if(pLegendWt)
            {
                s_pMainFrame->m_pBLTiles->legendWt()->makeLegend(false);
            }
            updateView();
            setControls();
            return;
        }

        for(int io=0; io<Objects2d::nOpPoints(); io++)
        {
            OpPoint *pOpp = Objects2d::opPointAt(io);
            for(int ic=0; ic<pOpp->curveCount(); ic++)
            {
                if(pOpp->curve(ic)==pCurve)
                {
                    //this is the one which has been clicked
                    setFoil(Objects2d::foil(pOpp->foilName()));
                    setPolar(Objects2d::polar(pOpp->foilName(), pOpp->polarName()));
                    setOpp(pOpp->aoa());
                    m_pFoilTreeView->selectOpPoint(pOpp);
                    m_pFoilTreeView->setCurveParams();
                    //                    QString strange = "Selected the operating point: "+pOpp->foilName()+"/"+pOpp->polarName()+"/"+QString::fromUtf8("%1°").arg(pOpp->aoa(),5,'f',3);
                    //                    s_pMainFrame->displayMessage(strange,5000);
                    updateView();
                    m_pOpPointWt->update();
                    return;
                }
            }
        }
    }
    else if(isPolarView())
    {
        if(!pCurve)
        {
            for(int ig=0; ig<m_PlrGraph.size();    ig++) m_PlrGraph[ig]->clearSelection();
            s_pMainFrame->m_pPolarTiles->legendWt()->makeLegend(false);

            updateView();
            setControls();
            return;
        }

        for(int iwp=0; iwp<Objects2d::nPolars(); iwp++)
        {
            Polar *pPolar=Objects2d::polarAt(iwp);
            for(int ic=0; ic<pPolar->curveCount(); ic++)
            {
                if(pPolar->curve(ic)==pCurve)
                {
                    //this is the one which has been clicked
                    setFoil(Objects2d::foil(pPolar->foilName()));
                    setPolar(pPolar);
                    m_pFoilTreeView->selectPolar(pPolar);
                    m_pFoilTreeView->setCurveParams();
//                    s_pMainFrame->statusBar()->showMessage("Selected the polar: "+pPolar->foilName()+"/"+pPolar->polarName(), 5000);
                    updateView();
                    return;
                }
            }
        }
    }
}


void XDirect::onCurveDoubleClicked(Curve*pCurve)
{
    if(!pCurve) return;

    if(isOppView())
    {
        for(int io=0; io<Objects2d::nOpPoints(); io++)
        {
            OpPoint *pOpp = Objects2d::opPointAt(io);
            for(int ic=0; ic<pOpp->curveCount(); ic++)
            {
                if(pOpp->curve(ic)==pCurve)
                {
                    //this is the one which has been double clicked
                    LineStyle ls(pOpp->theStyle());
                    LineMenu *pLineMenu = new LineMenu(nullptr);
                    pLineMenu->initMenu(ls);
                    pLineMenu->exec(QCursor::pos());
                    ls = pLineMenu->theStyle();
                    pOpp->setLineStipple(ls.m_Stipple);
                    pOpp->setLineWidth(ls.m_Width);
                    pOpp->setLineColor(ls.m_Color);
                    pOpp->setPointStyle(ls.m_Symbol);
                    emit projectModified();
                    m_pFoilTreeView->setCurveParams();
                    m_bResetCurves = true;
                    updateView();
                    m_pOpPointWt->update();
                    return;
                }
            }
        }
    }
    else
    {
        for(int iwp=0; iwp<Objects2d::nPolars(); iwp++)
        {
            Polar *pPolar=Objects2d::polarAt(iwp);
            for(int ic=0; ic<pPolar->curveCount(); ic++)
            {
                if(pPolar->curve(ic)==pCurve)
                {
                    //this is the one which has been clicked
                    setFoil(Objects2d::foil(pPolar->foilName()));
                    setPolar(pPolar);
                    m_pFoilTreeView->selectPolar(pPolar);
                    m_pFoilTreeView->setCurveParams();
                    updateView();
                    return;
                }
            }
        }
    }
}


void XDirect::setCurFoil(Foil*pFoil)
{
    Objects2d::setCurFoil(pFoil);
}


void XDirect::setCurPolar(Polar*pPolar)
{
    Objects2d::setCurPolar(pPolar);
}


void XDirect::setCurOpp(OpPoint* pOpp)
{
    Objects2d::setCurOpp(pOpp);
}


void XDirect::resetPrefs()
{
    m_bResetCurves = true;
    m_pFoilTreeView->setPropertiesFont(DisplayOptions::tableFont());
    m_pFoilTable->setTableFontStruct(DisplayOptions::tableFontStruct());
    updateView();
}


void XDirect::onShowCpLegend()
{
    m_pOpPointWt->CpGraph()->setLegendVisible(m_pActions->m_pShowCpLegend->isChecked());
}


void XDirect::writeFoilPolars(QDataStream &ar, Foil *pFoil)
{
    // 100003 : added foil comment
    // 100002 : means we are serializings opps in the new numbered format
    // 100001 : transferred NCrit, XTopTr, XBotTr to polar file
    // 500001 : v7 format
    int ArchiveFormat = 500001;
    ar << ArchiveFormat;

    //first write the foil
    ar << 1; //only one foil to write
    pFoil->serializeFl5(ar,true);

    //count polars associated to the foil
    Polar * pPolar =nullptr;
    int n=0;
    for (int i=0; i<Objects2d::nPolars();i++)
    {
        pPolar = Objects2d::polarAt(i);
        if (pPolar->foilName() == pFoil->name()) n++;
    }

    //then write the polars
    ar << n;
    for (int i=0; i<Objects2d::nPolars();i++)
    {
        pPolar = Objects2d::polarAt(i);
        if (pPolar->foilName() == pFoil->name())
            pPolar->serializePolarFl5(ar, true);
    }
}


void XDirect::onResetPolarCurve()
{
    m_bResetCurves = true;
    updateView();
}


void XDirect::onShowActiveFoilOnly()
{
    if(!curFoil()) return;

    for (int k=0; k<Objects2d::nFoils(); k++)
    {
        Foil *pFoil = Objects2d::foil(k);
        pFoil->setVisible(pFoil==curFoil());
    }

    if(isPolarView())
    {
        for (int ip=0; ip<Objects2d::nPolars(); ip++)
        {
            Polar *pPolar = Objects2d::polarAt(ip);
            pPolar->setVisible(pPolar->foilName()==curFoil()->name());
        }
    }

    if(isBLView())
    {
        for (int io=0; io<Objects2d::nOpPoints(); io++)
        {
            OpPoint *pOpp = Objects2d::opPointAt(io);
            pOpp->setVisible(pOpp->foilName()==curFoil()->name());
        }
    }

    m_bResetCurves = true;
    m_pFoilTreeView->updateObjectView();
    m_pFoilTable->updateTable();

    updateView();
    emit projectModified();
}


void XDirect::onShowActivePolarOnly()
{
    if(!curPolar() || !curFoil()) return;

    for (int ip=0; ip<Objects2d::nPolars(); ip++)
    {
        Polar *pPolar = Objects2d::polarAt(ip);
        pPolar->setVisible(pPolar==curPolar() && pPolar->foilName()==curFoil()->name());
    }

    if(isBLView())
    {
        for (int io=0; io<Objects2d::nOpPoints(); io++)
        {
            OpPoint *pOpp = Objects2d::opPointAt(io);
            pOpp->setVisible(pOpp->polarName()==curPolar()->name() && pOpp->foilName()==curFoil()->name());
        }
    }

    m_bResetCurves = true;
    m_pFoilTreeView->updateObjectView();
    m_pFoilTable->updateTable();
    updateView();
    emit projectModified();
}


void XDirect::onShowAllFoils()
{
    for (int k=0; k<Objects2d::nFoils(); k++)
    {
        Foil* pFoil = Objects2d::foil(k);
        pFoil->setVisible(true);
    }
    m_pDFoilWt->updateView();
    m_pFoilTreeView->updateObjectView();
    m_pFoilTable->updateTable();
    m_pFoilTable->viewport()->update();
    emit projectModified();
}


void XDirect::onHideAllFoils()
{
    for (int k=0; k<Objects2d::nFoils(); k++)
    {
        Foil *pFoil = Objects2d::foil(k);
        pFoil->setVisible(false);
    }

    m_pDFoilWt->updateView();
    m_pFoilTreeView->updateObjectView();
    m_pFoilTable->updateTable();
    m_pFoilTable->viewport()->update();
    emit projectModified();
}


void XDirect::onShowLegend(bool bShow)
{
    m_pDFoilWt->showLegend(bShow);
    m_pDFoilWt->updateView();
    setControls();
}


void XDirect::onShowTEHinge(bool bShow)
{
    DFoilWt::showTEHinge(bShow);
    m_pDFoilWt->updateView();
}


void XDirect::onShowLEPosition(bool bShow)
{
    DFoilWt::showLEPosition(bShow);
    m_pDFoilWt->updateView();
}


void XDirect::onFillFoil(bool bFill)
{
    DFoilWt::setFilling(bFill);
    m_pDFoilWt->updateView();
}


void XDirect::onAFoilLECircle()
{
    LECircleDlg LEDlg(s_pMainFrame);
    LEDlg.setLERadius(m_pDFoilWt->LECircleRadius());
    LEDlg.showRadius(m_pDFoilWt->isLECircleVisible());
    LEDlg.initDialog();

    LEDlg.exec();
    m_pDFoilWt->setLECircleRadius(LEDlg.LERadius());
    m_pDFoilWt->setLECircleVisible(LEDlg.bShowRadius());
    m_pDFoilWt->updateView();
}


void XDirect::fillCurve(Curve *pCurve, Polar const *pPolar, int XVar, int YVar, OpPoint const *pHighlightOpp) const
{
    pCurve->clear();

    QVector <double> const &pX = pPolar->getVariable(XVar);
    QVector <double> const &pY = pPolar->getVariable(YVar);
    double fx = 1.0;
    double fy = 1.0;

    pCurve->setSelectedPoint(-1);

    for (int i=0; i<pPolar->m_Alpha.size(); i++)
    {
        if (XVar==12)
        {
            if(pX.at(i)>0.0)
            {
                if (YVar==12)
                {
                    if(pY.at(i)>0.0)
                    {
                        pCurve->appendPoint(1.0/sqrt(pX.at(i)), 1.0/sqrt(pY.at(i)));
                    }
                }
                else
                {
                    pCurve->appendPoint(1.0/sqrt(pX.at(i)), pY.at(i)*fy);
                }
            }
        }
        else{
            if (YVar==12)
            {
                if(pY.at(i)>0.0)
                {
                    pCurve->appendPoint(pX.at(i)*fx, 1.0/sqrt(pY.at(i)));
                }
            }
            else
            {
                pCurve->appendPoint(pX.at(i)*fx, pY.at(i)*fy);
            }
        }

        // highlight the operating point
        if (pHighlightOpp)
        {
            if(pPolar->isControlPolar())
            {
                if(fabs(pPolar->m_Control.at(i) - pHighlightOpp->theta())<0.001)
                {
                    pCurve->setSelectedPoint(i);
                }
            }
            else if(pPolar->isFixedaoaPolar())
            {
                if(fabs(pPolar->m_Re.at(i) - pHighlightOpp->Reynolds())<0.1)
                {
                    pCurve->setSelectedPoint(i);
                }
            }
            else
            {
                if(fabs(pPolar->m_Alpha.at(i) - pHighlightOpp->aoa())<0.001)
                {
                    pCurve->setSelectedPoint(i);
                }
            }
        }
    }
}


void XDirect::onOpenAnalysisWindow()
{
    m_pXFADlg->show();
}


void XDirect::onAnalysisSettings()
{
    Analysis2dSettings analysis2dDlg(s_pMainFrame);
    analysis2dDlg.initWidget();
    analysis2dDlg.exec();
}


void XDirect::onScanPolarFiles()
{
    FoilPlrListDlg dlg(s_pMainFrame);
    dlg.initDlg(SaveOptions::plrPolarDirName());
    dlg.exec();

    m_pFoilTreeView->updateObjectView();
    m_pFoilTable->updateTable();

    m_bResetCurves = true;
    updateView();
    emit projectModified();
}


bool XDirect::isOneGraphView() const
{
    switch(m_eView)
    {
        case DESIGNVIEW:  return false;
        case OPPVIEW:     return true;
        case POLARVIEW:   return s_pMainFrame->m_pPolarTiles->isOneGraph();
        case BLVIEW:      return s_pMainFrame->m_pBLTiles->isOneGraph();
    }
    return false;
}


bool XDirect::isTwoGraphsView() const
{
    switch(m_eView)
    {
        case DESIGNVIEW:  return false;
        case OPPVIEW:     return false;
        case POLARVIEW:   return s_pMainFrame->m_pPolarTiles->isTwoGraphs();
        case BLVIEW:      return s_pMainFrame->m_pBLTiles->isTwoGraphs();
    }
    return false;
}


bool XDirect::isFourGraphsView() const
{
    switch(m_eView)
    {
        case DESIGNVIEW:  return false;
        case OPPVIEW:     return false;
        case POLARVIEW:   return s_pMainFrame->m_pPolarTiles->isFourGraphs();
        case BLVIEW:      return s_pMainFrame->m_pBLTiles->isFourGraphs();
    }
    return false;
}


bool XDirect::isAllGraphsView() const
{
    switch(m_eView)
    {
        case DESIGNVIEW:  return false;
        case OPPVIEW:     return false;
        case POLARVIEW:   return s_pMainFrame->m_pPolarTiles->isAllGraphs();
        case BLVIEW:      return s_pMainFrame->m_pBLTiles->isAllGraphs();
    }
    return false;
}


void XDirect::onOptimFoil()
{
    if(!curFoil()) return;
    OptimFoil o2d(s_pMainFrame);
    o2d.setFoil(curFoil());
    o2d.exec();
    if(o2d.isModified())
    {
        m_pFoilTreeView->fillModelView();
        emit projectModified();
    }
    updateView();
}


