/*****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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


#include <QRandomGenerator>
#include <QThread>
#include <QApplication>
#include <QDockWidget>
#include <QMessageBox>
#include <QAction>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QToolBar>

#include "xplane.h"


#include <core/displayoptions.h>
#include <core/saveoptions.h>
#include <core/stlreaderdlg.h>
#include <core/xflcore.h>
#include <globals/mainframe.h>
#include <interfaces/controls/analysisrangetable.h>
#include <interfaces/controls/poppctrls/crossflowctrls.h>
#include <interfaces/controls/poppctrls/crossflowctrls.h>
#include <interfaces/controls/poppctrls/flowctrls.h>
#include <interfaces/controls/poppctrls/opp3dscalesctrls.h>
#include <interfaces/controls/poppctrls/streamlinesctrls.h>
#include <interfaces/controls/t8rangetable.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/editors/analysis3ddef/aerodatadlg.h>
#include <interfaces/editors/analysis3ddef/extradragdlg.h>
#include <interfaces/editors/analysis3ddef/t1234578polardlg.h>
#include <interfaces/editors/analysis3ddef/t6polardlg.h>
#include <interfaces/editors/analysis3ddef/wpolarautonamedlg.h>
#include <interfaces/editors/analysisseldlg.h>
#include <interfaces/editors/editplrdlg.h>
#include <interfaces/editors/fuseedit/flatfaceconverterdlg.h>
#include <interfaces/editors/fuseedit/fusemesherdlg.h>
#include <interfaces/editors/fuseedit/fuseoccdlg.h>
#include <interfaces/editors/fuseedit/fusestldlg.h>
#include <interfaces/editors/fuseedit/shapefixerdlg.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fusexfldefdlg.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fusexflobjectdlg.h>
#include <interfaces/editors/inertia/partinertiadlg.h>
#include <interfaces/editors/inertia/planestlinertiadlg.h>
#include <interfaces/editors/inertia/planexflinertiadlg.h>
#include <interfaces/editors/planeedit/planestldlg.h>
#include <interfaces/editors/planeedit/planexfldlg.h>
#include <interfaces/editors/translatedlg.h>
#include <interfaces/editors/wingedit/wingdefdlg.h>
#include <interfaces/editors/wingedit/wingobjectdlg.h>
#include <interfaces/editors/wingedit/wingscaledlg.h>
#include <interfaces/exchange/cadexportdlg.h>
#include <interfaces/exchange/stlwriterdlg.h>
#include <interfaces/exchange/wingexportdlg.h>
#include <interfaces/graphs/containers/graphwt.h>
#include <interfaces/graphs/controls/graphdlg.h>
#include <interfaces/graphs/controls/graphtilevariableset.h>
#include <interfaces/graphs/graph/curve.h>
#include <interfaces/graphs/graph/graph.h>
#include <interfaces/mesh/gmesh_globals.h>
#include <interfaces/mesh/panelcheckdlg.h>
#include <interfaces/opengl/controls/fine3dcontrols.h>
#include <interfaces/opengl/controls/gllightdlg.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/optim/optimplanedlg.h>
#include <interfaces/widgets/customdlg/doublevaluedlg.h>
#include <interfaces/widgets/customdlg/intvaluedlg.h>
#include <interfaces/widgets/customdlg/moddlg.h>
#include <interfaces/widgets/customdlg/objectpropsdlg.h>
#include <interfaces/widgets/customdlg/textdlg.h>
#include <interfaces/widgets/line/linemenu.h>
#include <modules/xobjects.h>
#include <modules/xplane/analysis/analysis3dsettings.h>
#include <modules/xplane/analysis/batchplanedlg.h>
#include <modules/xplane/analysis/batchxmldlg.h>
#include <modules/xplane/analysis/lltanalysisdlg.h>
#include <modules/xplane/analysis/planeanalysisdlg.h>
#include <modules/xplane/controls/analysis3dctrls.h>
#include <modules/xplane/controls/planeexplorer.h>
#include <modules/xplane/controls/popp3dctrls.h>
#include <modules/xplane/controls/stab3dctrls.h>
#include <modules/xplane/controls/stabtimectrls.h>
#include <modules/xplane/controls/targetcurvedlg.h>
#include <modules/xplane/controls/wingseldlg.h>
#include <modules/xplane/controls/xplanewt.h>
#include <modules/xplane/glview/gl3dxplaneview.h>
#include <modules/xplane/glview/glxplanebuffers.h>
#include <modules/xplane/graphs/cpgraphctrls.h>
#include <modules/xplane/graphs/cpviewwt.h>
#include <modules/xplane/graphs/poppgraphctrls.h>
#include <modules/xplane/graphs/xplanelegendwt.h>
#include <modules/xplane/menus/xplaneactions.h>
#include <modules/xplane/menus/xplanemenus.h>
#include <test/tests/panelanalysistest.h>
#include <test/tests/vortontestdlg.h>

#include <api/llttask.h>
#include <api/p3analysis.h>
#include <api/panelanalysis.h>
#include <api/planetask.h>
#include <api/polar.h>
#include <api/objects2d.h>
#include <api/planeopp.h>
#include <api/wingopp.h>
#include <api/planepolar.h>
#include <api/planepolarext.h>
#include <modules/xplane/analysis/plpolarnamemaker.h>
#include <api/objects3d.h>
#include <api/fuseocc.h>
#include <api/fusesections.h>
#include <api/fusestl.h>
#include <api/fusexfl.h>
#include <api/pointmass.h>
#include <api/planestl.h>
#include <api/planexfl.h>
#include <api/surface.h>
#include <api/wingxfl.h>
#include <api/xmlfusewriter.h>
#include <api/xmlplanereader.h>
#include <api/xmlplanewriter.h>
#include <api/xmlwingwriter.h>
#include <api/xmlplanepolarreader.h>
#include <api/xmlplanepolarwriter.h>
#include <api/occ_globals.h>
#include <api/mesh_globals.h>
#include <api/mctriangle.h>
#include <api/panel3.h>
#include <api/panel4.h>
#include <api/units.h>
#include <api/utils.h>

QVector<OptObjective> XPlane::s_Objectives;
bool XPlane::s_bStoreOpps3d(false);

XPlane::XPlane(MainFrame *pMainFrame) : QObject()
{
    s_pMainFrame = pMainFrame;
    // exchange pointers
    Analysis3dCtrls::setXPlane(this);
    CrossFlowCtrls::setXPlane(this);
    BatchXmlDlg::setXPlane(this);
    POpp3dCtrls::setXPlane(this);
    Stab3dCtrls::setXPlane(this);
    StabTimeCtrls::setXPlane(this);
    PlaneExplorer::setXPlane(this);
    gl3dXPlaneView::setXPlane(this);
    glXPlaneBuffers::setXPlane(this);

    XPlaneLegendWt::setXPlane(this);

    Objects3d::setMainFrame(s_pMainFrame);
    GraphTiles::setMainFrame(s_pMainFrame);
    GraphTiles::setXPlane(this);

    GraphTileCtrls::setMainFrame(s_pMainFrame);
    GraphTileCtrls::setXPlane(this);


    m_pPanelResultTest = nullptr;

    //the instance of the analysis dlg
    m_pPanelAnalysisDlg = new PlaneAnalysisDlg(s_pMainFrame);
    PlaneAnalysisDlg::setXPlane(this);
    connect(m_pPanelAnalysisDlg, SIGNAL(analysisFinished(PlanePolar*)), SLOT(onFinishAnalysis(PlanePolar*)));

    m_pLLTAnalysisDlg = new LLTAnalysisDlg(s_pMainFrame);
    LLTAnalysisDlg::setXPlane(this);
    connect(m_pLLTAnalysisDlg, SIGNAL(analysisFinished(PlanePolar*)), SLOT(onFinishAnalysis(PlanePolar*)));

    m_pCurPlane   = nullptr;
    m_pCurPOpp    = nullptr;
    m_pCurPlPolar  = nullptr;

    m_bCurPOppOnly       = false;

    m_bShowEllipticCurve = false;
    m_bShowBellCurve     = false;

    m_pTimerMode = new QTimer(s_pMainFrame);

//    m_iActiveT7Mode = -1;

    // build the Wing Opp graphs
    QStringList XPOppList = {"y ("+Units::lengthUnitQLabel() +")"};

    QFont graphfont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    graphfont.setStyleHint(QFont::SansSerif);

    PlaneOpp::setVariableNames();
    m_WingGraph.clear();
    for(int ig=0; ig<MAXGRAPHS; ig++)
    {
        m_WingGraph.append(new Graph);
        m_WingCurveModel.append(new CurveModel);
        m_WingGraph.at(ig)->setCurveModel(m_WingCurveModel.back());
        m_WingGraph.at(ig)->setXVariableList(XPOppList);
        m_WingGraph.at(ig)->setYVarStdList(PlaneOpp::s_POppVariables);
        m_WingGraph.at(ig)->setName(QString::asprintf("Wing Graph %d", ig+1));
        m_WingGraph.at(ig)->setGraphType(GRAPH::POPPGRAPH);
        m_WingGraph.at(ig)->setAutoX(true);

        m_WingGraph.at(ig)->setXMin(-0.001);//EXPANDING
        m_WingGraph.at(ig)->setXMax( 0.001);//EXPANDING
        m_WingGraph.at(ig)->setYMin(0, 0.000);//EXPANDING
        m_WingGraph.at(ig)->setYMax(0, 0.001);//EXPANDING
        m_WingGraph.at(ig)->setScaleType(GRAPH::EXPANDING);
        m_WingGraph.at(ig)->setMargins(50);
        m_WingGraph.at(ig)->enableRightAxis(true);
        m_WingGraph.at(ig)->setLegendPosition(Qt::AlignTop | Qt::AlignLeft);
        m_WingGraph.at(ig)->setTitleFont(graphfont);
        m_WingGraph.at(ig)->setLabelFont(graphfont);
    }
    m_WingGraph.at(0)->setYVariable(0, 2);
    m_WingGraph.at(1)->setYVariable(0, 3);
    m_WingGraph.at(2)->setYVariable(0, 5);
    m_WingGraph.at(3)->setYVariable(0, 4);
    m_WingGraph.at(4)->setYVariable(0, 8);

    // build the Polar graphs
    PlanePolar::setVariableNames();
    m_WPlrGraph.clear();
    for(int ig=0; ig<MAXGRAPHS; ig++)
    {
        m_WPlrGraph.append(new Graph);
        m_WPlrCurveModel.append(new CurveModel);
        m_WPlrGraph.at(ig)->setXVarStdList(PlanePolar::variableNames());
        m_WPlrGraph.at(ig)->setYVarStdList(PlanePolar::variableNames());
        m_WPlrGraph.at(ig)->setCurveModel(m_WPlrCurveModel.back());
        m_WPlrGraph.at(ig)->setGraphType(GRAPH::WPOLARGRAPH);
        m_WPlrGraph.at(ig)->setName(QString::asprintf("Plane polar graph %d", ig+1));
        m_WPlrGraph.at(ig)->setXMin(-0.0);
        m_WPlrGraph.at(ig)->setXMax( 0.1);
        m_WPlrGraph.at(ig)->setYMin(0, -0.01);
        m_WPlrGraph.at(ig)->setYMax(0,  0.01);
        m_WPlrGraph.at(ig)->setScaleType(GRAPH::RESETTING);
        m_WPlrGraph.at(ig)->setMargins(50);
        m_WPlrGraph.at(ig)->setLegendPosition(Qt::AlignTop | Qt::AlignVCenter);
        m_WPlrGraph.at(ig)->enableRightAxis(true);
        m_WPlrGraph.at(ig)->setTitleFont(graphfont);
        m_WPlrGraph.at(ig)->setLabelFont(graphfont);
    }

    m_WPlrGraph[0]->setVariables(5,4);
    m_WPlrGraph[1]->setVariables(1,4);
    m_WPlrGraph[2]->setVariables(1,9);
    m_WPlrGraph[3]->setVariables(1,5);
    m_WPlrGraph[4]->setVariables(1,16);


    //set the default settings for the time response graphs
    m_TimeGraph.clear();
    int MAXSTABTIMEGRAPHS = 4;
    for(int ig=0; ig<MAXSTABTIMEGRAPHS; ig++)
    {
        m_TimeGraph.append(new Graph);
        m_TimeCurveModel.append(new CurveModel);
        m_TimeGraph[ig]->setCurveModel(m_TimeCurveModel.back());
        m_TimeGraph[ig]->setGraphType(GRAPH::STABTIMEGRAPH);
        m_TimeGraph[ig]->setXMin( 0.0);
        m_TimeGraph[ig]->setXMax( 0.1);
        m_TimeGraph[ig]->setYMin(0, -0.01);
        m_TimeGraph[ig]->setYMax(0,  0.01);
        m_TimeGraph[ig]->setScaleType(GRAPH::RESETTING);
        m_TimeGraph[ig]->setMargins(50);
        m_TimeGraph[ig]->setYInverted(0, false);
        m_TimeGraph[ig]->setName("Time Response");
        m_TimeGraph[ig]->setXVariableList({"time (s)"});
        m_TimeGraph[ig]->setVariables(0,0);
        m_TimeGraph.at(ig)->setTitleFont(graphfont);
        m_TimeGraph.at(ig)->setLabelFont(graphfont);
    }
//    setStabTimeYVariables(); //temporary until units and stability direction are known


    //set the default settings for the root locus graphs
    m_StabPlrGraph.append(new Graph);   // the longitudinal graph
    m_StabPlrCurveModel.append(new CurveModel);
    m_StabPlrGraph.at(0)->setCurveModel(m_StabPlrCurveModel.back());
    m_StabPlrGraph.at(0)->setGraphType(GRAPH::STABPOLARGRAPH);
    m_StabPlrGraph.at(0)->setXVariableList({"Real"});
    m_StabPlrGraph.at(0)->setYVariableList({"Imag/2"+PIch});
    m_StabPlrGraph.at(0)->setXMin( 0.0);
    m_StabPlrGraph.at(0)->setXMax( 0.1);
    m_StabPlrGraph.at(0)->setYMin(0, -0.01);
    m_StabPlrGraph.at(0)->setYMax(0,  0.01);
    m_StabPlrGraph.at(0)->setScaleType(GRAPH::RESETTING);
    m_StabPlrGraph.at(0)->setMargins(50);
    m_StabPlrGraph.at(0)->setYInverted(0, false);
    m_StabPlrGraph.at(0)->setName("Longitudinal Modes");
    m_StabPlrGraph.at(0)->setTitleFont(graphfont);
    m_StabPlrGraph.at(0)->setLabelFont(graphfont);

    m_StabPlrGraph.append(new Graph);   // the lateral graph
    m_StabPlrCurveModel.append(new CurveModel);
    m_StabPlrGraph.at(1)->setCurveModel(m_StabPlrCurveModel.back());
    m_StabPlrGraph.at(1)->setGraphType(GRAPH::STABPOLARGRAPH);
    m_StabPlrGraph.at(1)->setXVariableList({"Real"});
    m_StabPlrGraph.at(1)->setYVariableList({"Imag/2"+PIch});
    m_StabPlrGraph.at(1)->setXMin( 0.0);
    m_StabPlrGraph.at(1)->setXMax( 0.1);
    m_StabPlrGraph.at(1)->setYMin(0, -0.01);
    m_StabPlrGraph.at(1)->setYMax(0,  0.01);
    m_StabPlrGraph.at(1)->setScaleType(GRAPH::RESETTING);
    m_StabPlrGraph.at(1)->setMargins(50);
    m_StabPlrGraph.at(1)->setYInverted(0, false);
    m_StabPlrGraph.at(1)->setName("Lateral Modes");
    m_StabPlrGraph.at(1)->setTitleFont(graphfont);
    m_StabPlrGraph.at(1)->setLabelFont(graphfont);

    m_eView = XPlane::POLARVIEW;


    m_BellCurveExp = 1;
    m_bMaxCL = true;
    m_TargetCurveStyle = {true, Line::DASH, 2, fl5Color(100, 100, 100), Line::NOSYMBOL};


    //make XPlane related widgets
    m_pgl3dXPlaneView = new gl3dXPlaneView(s_pMainFrame);
    m_pgl3dXPlaneView->setXPlaneBuffers(new glXPlaneBuffers);// create once in case the view is duplicated in a separate window
    m_pgl3dXPlaneView->m_bAutoDeleteBuffers = true;

/*    m_pGraphicsView = new QGraphicsView;
    m_pGraphicsView->setViewport(m_pgl3dXPlaneView);
    m_pGraphicsView->show();
    m_pGraphicsView->raise();*/

    if(!m_pXPlaneWt)
    {
        m_pXPlaneWt = new XPlaneWt(this, m_pgl3dXPlaneView);
//        m_pXPlaneWt->gl3dFloatView()->setScaleCtrls(m_pPOpp3dControls->m_pgl3dScales);
        updateVisiblePanels();
        m_pXPlaneWt->updateObjectData();
    }

    m_pAnalysisControls = new Analysis3dCtrls;
    connect(m_pAnalysisControls->m_ppbAnalyze, SIGNAL(clicked()), SLOT(onAnalyze()));

    m_pStabTimeControls = new StabTimeCtrls;

    m_pPlaneExplorer = new PlaneExplorer;

//    m_pgl3dXPlaneView->setScaleCtrls(s_pMainFrame->m_pgl3dScales); // exchange pointers

    m_pPOppGraphCtrls = nullptr;

    m_pPOpp3dCtrls = new POpp3dCtrls(m_pgl3dXPlaneView);
    m_pPOpp3dCtrls->setXPlane(this);
    m_pPOpp3dCtrls->set3dXPlaneView(m_pgl3dXPlaneView);

    m_pActions = new XPlaneActions(this);
    m_pMenus = new XPlaneMenus(s_pMainFrame, this);
}


XPlane::~XPlane()
{
    if(m_pXPlaneWt)
    {
        m_pXPlaneWt->close();
        m_pXPlaneWt = nullptr;
    }

    if(m_pLLTAnalysisDlg)   delete m_pLLTAnalysisDlg;
    if(m_pPanelAnalysisDlg) delete m_pPanelAnalysisDlg;

    if(m_pStabTimeControls) delete m_pStabTimeControls;


    Objects3d::deleteObjects();


    for(int ig=m_WingGraph.count()-1; ig>=0; ig--)
    {
        if(m_WingGraph.at(ig))
        {
            delete m_WingGraph.at(ig);
        }
    }
    m_WingGraph.clear();

    for(int ig=m_WPlrGraph.count()-1; ig>=0; ig--)
    {
        if(m_WPlrGraph.at(ig))
        {
            delete m_WPlrGraph.at(ig);
        }
    }
    m_WPlrGraph.clear();

    for(int ig=m_StabPlrGraph.count()-1; ig>=0; ig--)
    {
        delete m_StabPlrGraph.at(ig);
        m_StabPlrGraph.removeAt(ig);
    }

    for(int ig=m_TimeGraph.count()-1; ig>=0; ig--)
    {
        delete m_TimeGraph.at(ig);
        m_TimeGraph.removeAt(ig);
    }

    for(int ig=m_WingCurveModel.count()-1; ig>=0; ig--)
    {
        delete m_WingCurveModel.at(ig);
        m_WingCurveModel.removeAt(ig);
    }

    for(int ig=m_WPlrCurveModel.count()-1; ig>=0; ig--)
    {
        delete m_WPlrCurveModel.at(ig);
        m_WPlrCurveModel.removeAt(ig);
    }

    for(int ig=m_StabPlrCurveModel.count()-1; ig>=0; ig--)
    {
        delete m_StabPlrCurveModel.at(ig);
        m_StabPlrCurveModel.removeAt(ig);
    }

    for(int ig=m_TimeCurveModel.count()-1; ig>=0; ig--)
    {
        delete m_TimeCurveModel.at(ig);
        m_TimeCurveModel.removeAt(ig);
    }


    delete m_pMenus;
}


void XPlane::connectSignals()
{   
    connect(s_pMainFrame->m_pWPolarTiles,   SIGNAL(varSetChanged(int)),    SLOT(onVarSetChanged(int)));
    connect(s_pMainFrame->m_pPOppTiles,     SIGNAL(varSetChanged(int)),    SLOT(onVarSetChanged(int)));

    connect(s_pMainFrame->m_pWPolarTiles,   SIGNAL(graphChanged(int)),     SLOT(onGraphChanged(int)));
    connect(s_pMainFrame->m_pPOppTiles,     SIGNAL(graphChanged(int)),     SLOT(onGraphChanged(int)));

    connect(m_pTimerMode,                   SIGNAL(timeout()),             SLOT(onAnimateModeSingle()));

    connect(m_pPOpp3dCtrls->m_pOpp3dScalesCtrls,  SIGNAL(update3dScales()),      m_pgl3dXPlaneView, SLOT(onUpdate3dScales()));
    connect(m_pPOpp3dCtrls->m_pStreamLinesCtrls,  SIGNAL(update3dStreamlines()), m_pgl3dXPlaneView, SLOT(onUpdate3dStreamlines()));

    connect(m_pPOpp3dCtrls->m_pOpp3dScalesCtrls,   SIGNAL(update3dScales()),      m_pXPlaneWt->gl3dFloatView(), SLOT(onUpdate3dScales()));
    connect(m_pPOpp3dCtrls->m_pStreamLinesCtrls,   SIGNAL(update3dStreamlines()), m_pXPlaneWt->gl3dFloatView(), SLOT(onUpdate3dStreamlines()));

//    connect(m_pglControls->m_ptbDistance, SIGNAL(clicked()), SLOT(onNodeDistance()));
    connect(m_pgl3dXPlaneView,   SIGNAL(pickedNodePair(QPair<int,int>)), SLOT(onPickedNodePair(QPair<int,int>)));
    connect(m_pgl3dXPlaneView,   SIGNAL(pickedNodeIndex(int)),           SLOT(onPickedNode(int)));
}


void XPlane::checkActions()
{
    m_pActions->checkActions();

    m_pMenus->m_pSubWingMenu->setEnabled( m_pCurPlane && m_pCurPlane->hasMainWing() && !m_pCurPlane->isLocked());
    m_pMenus->m_pSubStabMenu->setEnabled( m_pCurPlane && m_pCurPlane->hasStab()     && !m_pCurPlane->isLocked());
    m_pMenus->m_pSubFinMenu->setEnabled(  m_pCurPlane && m_pCurPlane->hasFin()      && !m_pCurPlane->isLocked());
    m_pMenus->m_pSubFuseMenu->setEnabled( m_pCurPlane && m_pCurPlane->hasFuse()     && !m_pCurPlane->isLocked());
    m_pMenus->m_pCurrentPlaneMenu->setEnabled(m_pCurPlane && !m_pCurPlane->isLocked());
    m_pMenus->m_pCurrentPlaneCtxMenu->setEnabled(m_pCurPlane && !m_pCurPlane->isLocked());

    m_pMenus->m_pCurWPlrMenu->setEnabled(m_pCurPlane && m_pCurPlPolar && !m_pCurPlane->isLocked());
    m_pMenus->m_pCurWPlrCtxMenu->setEnabled(m_pCurPlane && m_pCurPlPolar && !m_pCurPlane->isLocked());

    m_pMenus->m_pCurPOppMenu->setEnabled(m_pCurPlane && m_pCurPOpp && !m_pCurPlane->isLocked());
    m_pMenus->m_pCurPOppCtxMenu->setEnabled(m_pCurPlane && m_pCurPOpp && !m_pCurPlane->isLocked());
}


bool XPlane::isOneGraphView() const
{
    switch(m_eView)
    {
        case XPlane::POPPVIEW:      return s_pMainFrame->m_pPOppTiles->isOneGraph();
        case XPlane::POLARVIEW:    return s_pMainFrame->m_pWPolarTiles->isOneGraph();
        case XPlane::CPVIEW:       return true;
        case XPlane::STABTIMEVIEW:  return s_pMainFrame->m_pStabTimeTiles->isOneGraph();
        case XPlane::STABPOLARVIEW: return s_pMainFrame->m_pStabPolarTiles->isOneGraph();
        default: return false;
    }
}


bool XPlane::isTwoGraphsView() const
{
    switch(m_eView)
    {
        case XPlane::POPPVIEW:      return s_pMainFrame->m_pPOppTiles->isTwoGraphs();
        case XPlane::POLARVIEW:    return s_pMainFrame->m_pWPolarTiles->isTwoGraphs();
        case XPlane::CPVIEW:       return false;
        case XPlane::STABTIMEVIEW:  return s_pMainFrame->m_pStabTimeTiles->isTwoGraphs();
        case XPlane::STABPOLARVIEW: return s_pMainFrame->m_pStabPolarTiles->isTwoGraphs();
        default: return false;
    }
}


bool XPlane::isFourGraphsView() const
{
    switch(m_eView)
    {
        case XPlane::POPPVIEW:      return s_pMainFrame->m_pPOppTiles->isFourGraphs();
        case XPlane::POLARVIEW:    return s_pMainFrame->m_pWPolarTiles->isFourGraphs();
        case XPlane::CPVIEW:       return false;
        case XPlane::STABTIMEVIEW:  return s_pMainFrame->m_pStabTimeTiles->isFourGraphs();
        case XPlane::STABPOLARVIEW: return false;
        default: return false;
    }
}


bool XPlane::isAllGraphsView() const
{
    switch(m_eView)
    {
        case XPlane::POPPVIEW:      return s_pMainFrame->m_pPOppTiles->isAllGraphs();
        case XPlane::POLARVIEW:    return s_pMainFrame->m_pWPolarTiles->isAllGraphs();
        case XPlane::CPVIEW:       return false;
        case XPlane::STABTIMEVIEW:  return false;
        case XPlane::STABPOLARVIEW: return false;
        default: return false;
    }
}


void XPlane::setControls(bool bVisibilityOnly)
{
    // firstly hide
    if(!is3dView())   s_pMainFrame->m_pdwXPlaneResults3d->hide();

    if(!isPOppView() && !isPolarView() && !isStabilityView())
        s_pMainFrame->m_pdwGraphControls->hide();
    if(!isStabTimeView()) s_pMainFrame->showStabTimeCtrls(false);

    // secondly show
    if(MainFrame::xflApp()==xfl::XPLANE)
    {
        s_pMainFrame->m_pdwXPlaneResults3d->setVisible(is3dView());
        s_pMainFrame->m_pdwCp3d->setVisible(isCpView());
    }
    s_pMainFrame->showStabTimeCtrls(isStabTimeView());

    bool bGraphControlsVisible = isPOppView() || isPolarView() || isStabilityView();
    if(MainFrame::xflApp()==xfl::XPLANE)
        s_pMainFrame->m_pdwGraphControls->setVisible(bGraphControlsVisible);

    if(isPOppView())
        s_pMainFrame->m_pdwGraphControls->setWidget(s_pMainFrame->m_pPOppTiles->graphControls());
    else if(isStabPolarView())
        s_pMainFrame->m_pdwGraphControls->setWidget(s_pMainFrame->m_pStabPolarTiles->graphControls());
    else if(isStabTimeView())
        s_pMainFrame->m_pdwGraphControls->setWidget(s_pMainFrame->m_pStabTimeTiles->graphControls());
    else if(isPolarView())
        s_pMainFrame->m_pdwGraphControls->setWidget(s_pMainFrame->m_pWPolarTiles->graphControls());


    if(bVisibilityOnly) return;

    if(isStabTimeView()) m_pStabTimeControls->setControls(); // lengthy
    m_pAnalysisControls->onSetControls();
    m_pPOpp3dCtrls->setControls();

    m_pPlaneExplorer->setCurveParams();
    m_pPlaneExplorer->setOverallCheckStatus();

    if(isPOppView())
        s_pMainFrame->m_pPOppTiles->updateControls();
    else if(isStabPolarView())
        s_pMainFrame->m_pStabTimeTiles->updateControls();
    else if(isStabTimeView())
        s_pMainFrame->m_pStabTimeTiles->updateControls();
    else if(isCpView())
    {
    }
    else
        s_pMainFrame->m_pWPolarTiles->updateControls();

    if(m_pXPlaneWt) m_pXPlaneWt->setControls();

    if(isCpView()) s_pMainFrame->m_pCpGraphCtrl->setControls();

    m_pPOpp3dCtrls->m_pFlowCtrls->enableFlowControls();

    checkActions();
}


void XPlane::onCpView()
{
    stopAnimate();

    if(isCpView())
    {
        setControls();
        updateView();
        return;
    }
    m_eView=XPlane::CPVIEW;

    setGraphTiles();
    s_pMainFrame->setActiveCentralWidget();

    setControls();
    if(m_pCurPOpp && m_pCurPOpp->isTriLinearMethod())
    {
        TriMesh::makeNodeValues(m_pCurPlane->triMesh().nodes(), m_pCurPlane->triMesh().panels(),
                                m_pCurPOpp->m_Cp, m_pCurPOpp->m_NodeValue,
                                m_pCurPOpp->m_NodeValMin, m_pCurPOpp->m_NodeValMax, 1.0);
    }

    createCpCurves();
    updateView();
}


void XPlane::onAddCpSectionCurve()
{
    if(!m_pCurPOpp) return;

    Curve *pCurrentCurve, *pNewCurve;

    pCurrentCurve = s_pMainFrame->m_pCpViewWt->CpGraph()->curve(0);
    pNewCurve = s_pMainFrame->m_pCpViewWt->CpGraph()->addCurve();
    pNewCurve->duplicate(pCurrentCurve);
    pNewCurve->setTheStyle(m_pCurPOpp->theStyle());
    pNewCurve->setWidth(Curve::defaultLineWidth());

    createCpCurves();

    updateView();
}


void XPlane::createCpCurves()
{
    if(!m_pCurPOpp || !m_pCurPlPolar || !m_pCurPlane || !m_pCurPlane->isXflType())
    {
        s_pMainFrame->m_pCpViewWt->CpGraph()->deleteCurves();
        s_pMainFrame->m_pCpViewWt->makeLegend(false);
        return;
    }

    s_pMainFrame->m_pCpViewWt->updateUnits();


    // the first curve is for the current selection
    // the next are those that the user has chosen to keep for display --> don't reset them

    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);

    int sel = s_pMainFrame->m_pCpGraphCtrl->currentWingIndex();
    WingXfl const *pWing = pPlaneXfl->wingAt(sel);
    if(!pWing) return;

    int iStrip = s_pMainFrame->m_pCpGraphCtrl->iStrip();

    QString str3;
    str3 = QString::asprintf("-y/b=%5.2f", s_pMainFrame->m_pCpGraphCtrl->spanRelPos());

    if(!s_pMainFrame->m_pCpViewWt->CpGraph()->curveCount())
        s_pMainFrame->m_pCpViewWt->CpGraph()->addCurve();

    Curve *pCurve = s_pMainFrame->m_pCpViewWt->CpGraph()->curve(0);
    if(pCurve)
    {
        pCurve->clear();
        pCurve->setTheStyle(Line::DASH, 1, QColor(95,95,95), Line::NOSYMBOL, true);
        pCurve->setName(QString::fromStdString(m_pCurPOpp->title(true))+str3);

        std::vector<double> Cp;
        std::vector<Node> pts;
        if     (m_pCurPOpp->isQuadMethod())       Objects3d::fillSectionCp4(       pPlaneXfl, m_pCurPOpp, sel, iStrip, Cp, pts);
        else if(m_pCurPOpp->isTriLinearMethod())  Objects3d::fillSectionCp3Linear( pPlaneXfl, m_pCurPOpp, sel, iStrip, Cp, pts);
        else if(m_pCurPOpp->isTriUniformMethod()) Objects3d::fillSectionCp3Uniform(pPlaneXfl, m_pCurPOpp, sel, iStrip, Cp, pts);

        for(uint i=0; i<pts.size(); i++)
        {
            pCurve->appendPoint((pts.at(i).x-pPlaneXfl->wingLE(0).x) * Units::mtoUnit(), Cp.at(i));
        }
        if(m_pXPlaneWt)
            gl::makeCpSection(pts, Cp, s_pMainFrame->m_pCpGraphCtrl->CpSectionScale(), m_pXPlaneWt->gl3dFloatView()->m_CpSections);
    }

    m_bResetCurves = false;
    s_pMainFrame->m_pCpViewWt->makeLegend(true);
    s_pMainFrame->m_pCpViewWt->CpGraph()->invalidate();

    if(m_pXPlaneWt && m_pXPlaneWt->isVisible())
        m_pXPlaneWt->updateView();
}


void XPlane::createWOppCurves()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;

    for(int ig=0; ig<m_WingGraph.size(); ig++)
    {
        m_WingGraph[ig]->deleteCurves();
        m_WingGraph[ig]->clearSelection();
    }

    // Browse through the array of plane operating points
    // add a curve for those selected, and fill them with data
    for (int k=0; k<Objects3d::nPOpps(); k++)
    {
        PlaneOpp *pPOpp = Objects3d::POppAt(k);
        pPOpp->m_Curve.clear();
        if (pPOpp && pPOpp->isVisible() && (!m_bCurPOppOnly || (m_pCurPOpp==pPOpp)))
        {
            Plane const *pPlane = Objects3d::plane(pPOpp->planeName());
            PlanePolar const *pWPolar = Objects3d::wPolar(pPlane, pPOpp->polarName());
            if(!pPlane || !pPlane->isXflType()) continue;
            if(!pWPolar) continue;

            PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

            // there is only one wingopp in the case of the LLT
            for(int iw=0; iw<pPOpp->nWOpps(); iw++)
            {
                WingXfl const *pWing = pPlaneXfl->wingAt(iw);
                bool bShowWOpp = false;
                if(WingSelDlg::mainWingCurve()  && pWing->isMainWing() )  bShowWOpp = true;
                if(WingSelDlg::elevatorCurve()  && pWing->isElevator() )  bShowWOpp = true;
                if(WingSelDlg::finCurve()       && pWing->isFin()      )  bShowWOpp = true;
                if(WingSelDlg::otherWingCurve() && pWing->isOtherWing())  bShowWOpp = true;
                if(bShowWOpp)
                {
                    for(int ig=0; ig<m_WingGraph.count(); ig++)
                    {
                        Graph *pGraph = m_WingGraph[ig];

                        for(int iy=0; iy<2; iy++)
                        {
                            if(pGraph->hasYAxis(iy))
                            {
                                AXIS::enumAxis yaxis = iy==0 ? AXIS::LEFTYAXIS : AXIS::RIGHTYAXIS;
                                Curve *pWingCurve = pGraph->addCurve(yaxis);
                                pWingCurve->setTheStyle(pPOpp->theStyle());
                                //only show the legend for the main wing
                                if(iw==0) pWingCurve->setName(QString::fromStdString(pPOpp->title(true)));
                                else      pWingCurve->setName("");
//                                pGraph->setYTitle(iy, pGraph->yVariableName(iy));
                                fillWOppCurve(pWPolar, pPOpp, iw, pGraph->yVariable(iy), pWingCurve);
                                pPOpp->appendCurve(pWingCurve);
                            }
                        }
                    }
                }
            }
        }
    }

    selectWOppCurves();  /** @todo what's the use? */

    //if the elliptic curve is requested, and if the graph variable is local lift, then add the curve
    PlaneXfl const* pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);
    if(pPlaneXfl->nWings()>0)
    {
        if(m_bShowEllipticCurve && m_pCurPOpp)
        {
            double lift=0, maxlift = 0.0;

            if(m_bMaxCL) maxlift = m_pCurPOpp->WOpp(0).maxLift();
            else
            {
                lift=0.0;
                for (int i=0; i<pPlaneXfl->wingAt(0)->nStations(); i++)
                {
                    double x = m_pCurPOpp->WOpp(0).spanResults().m_StripPos[i]/m_pCurPlane->span()*2.0;
                    double y = sqrt(1.0 - x*x);
                    lift += y*m_pCurPOpp->WOpp(0).spanResults().m_StripArea[i] ;
                }
                maxlift = m_pCurPOpp->m_AF.CL() / lift * m_pCurPlane->planformArea(m_pCurPlPolar->bIncludeOtherWingAreas());
            }

            for(int ig=0; ig<m_WingGraph.size(); ig++)
            {
                if(m_WingGraph[ig]->yVariable(0)==1)
                {
                    Curve *pCurve = m_WingGraph[ig]->addCurve();
                    pCurve->setTheStyle(m_TargetCurveStyle);
                    for (int idi=-50; idi<=55; idi++)
                    {
                        double idd = double(idi);
                        double x = m_pCurPlane->span()/2.0 * cos(idd*PI/50.0) * ( 1.0-PRECISION);
                        double y = maxlift*sqrt(1.0 - x*x/m_pCurPlane->span()/m_pCurPlane->span()*4.0);
                        pCurve->appendPoint(x*Units::mtoUnit(),y);
                    }
                }
            }
        }
        //if the target bell curve is requested, and if the graph variable is local lift, then add the curve
        if(m_bShowBellCurve && m_pCurPOpp)
        {
            double maxlift=0;
            if(m_bMaxCL) maxlift = m_pCurPOpp->WOpp(0).maxLift();
            else
            {
                double lift=0.0;
                for (int i=0; i<pPlaneXfl->wingAt(0)->nStations(); i++)
                {
                    double x = m_pCurPOpp->WOpp(0).spanResults().m_StripPos[i]/m_pCurPlane->span()*2.0;
                    double y = pow(cos(x*PI/2.0), m_BellCurveExp);
                    lift += y*m_pCurPOpp->WOpp(0).spanResults().m_StripArea[i] ;
                }
                maxlift = m_pCurPOpp->m_AF.CL() / lift * m_pCurPlane->planformArea(m_pCurPlPolar->bIncludeOtherWingAreas());
            }

            for(int ig=0; ig<m_WingGraph.size(); ig++)
            {
                if(m_WingGraph.at(ig)->yVariable(0)==1)
                {
                    Curve *pCurve = m_WingGraph[ig]->addCurve();
                    pCurve->setTheStyle(m_TargetCurveStyle);

                    for (int idi=-50; idi<=50; idi++)
                    {
                        double idd = double(idi);
                        double x = m_pCurPlane->span()/2.0 * cos(idd*PI/50.0);
                        double y = maxlift * pow(cos(x/m_pCurPlane->span()*2.0*PI/2.0), m_BellCurveExp);
                        pCurve->appendPoint(x*Units::mtoUnit(),y);
                    }
                }
            }
        }
    }

    m_bResetCurves = false;
    for(int ig=0; ig<m_WingGraph.size(); ig++)
    {
        m_WingGraph[ig]->invalidate();
    }
}


void XPlane::createWPolarCurves()
{
    for(int ig=0; ig<m_WPlrGraph.count(); ig++)
    {
        m_WPlrGraph[ig]->deleteCurves();
        m_WPlrGraph[ig]->clearSelection();
    }

    for (int k=0; k<Objects3d::nPolars(); k++)
    {
        PlanePolar *pWPolar = Objects3d::plPolarAt(k);
        pWPolar->clearCurves();

        if (pWPolar->isVisible() && pWPolar->dataSize()>0)
        {
            for(int ig=0; ig<m_WPlrGraph.count(); ig++)
            {
                Graph *pGraph = m_WPlrGraph[ig];

                // add left axis curves
                Curve *pCurveL = pGraph->addCurve(QString::fromStdString(pWPolar->planeName() + " / " + pWPolar->name()), AXIS::LEFTYAXIS, DisplayOptions::isDarkTheme());
                fillWPlrCurve(pCurveL, pWPolar, pGraph->xVariable(), pGraph->yVariable(0));
                pCurveL->setTheStyle(pWPolar->theStyle());
                pWPolar->appendCurve(pCurveL);
                if(pWPolar==m_pCurPlPolar) pGraph->selectCurve(pCurveL);

                // add right axis curves
                if(pGraph->hasRightAxis())
                {
                    Curve *pCurveR = pGraph->addCurve(QString::fromStdString(pWPolar->planeName() + " / " + pWPolar->name()), AXIS::RIGHTYAXIS, DisplayOptions::isDarkTheme());
                    fillWPlrCurve(pCurveR, pWPolar, pGraph->xVariable(), pGraph->yVariable(1));
                    pCurveR->setTheStyle(pWPolar->theStyle());
                    pWPolar->appendCurve(pCurveR);
                    if(pWPolar==m_pCurPlPolar) pGraph->selectCurve(pCurveR);
                }
            }
        }
    }

    m_bResetCurves = false;

    for(int ig=0; ig<m_WPlrGraph.count(); ig++)
    {
        m_WPlrGraph[ig]->invalidate();
    }
}


void XPlane::selectWOppCurves()
{
    if(!m_pCurPOpp) return;

    for(int ig=0; ig<m_WingGraph.count(); ig++)
    {
        m_WingGraph.at(ig)->clearSelection();
        //cross compare curve pointers to those of the PlaneOpp
        for(int ic=0; ic<m_WingGraph.at(ig)->curveCount(); ic++)
        {
            if(m_pCurPOpp->hasCurve(m_WingGraph.at(ig)->curve(ic)))
            {
                m_WingGraph.at(ig)->selectCurve(ic);
            }
        }
    }
}


void XPlane::createStabilityCurves()
{
    if(isStabTimeView())
    {
        if(!m_pCurPOpp || (!m_pCurPOpp->isType7() && !m_pCurPOpp->isType8()))
        {
            for(int it=0; it<m_TimeGraph.size(); it++) m_TimeGraph[it]->deleteCurves();
            return;//nothing to plot
        }

        Curve *pCurve[]{nullptr, nullptr, nullptr, nullptr};
        QString curvetitle = m_pStabTimeControls->selectedCurveName();
        pCurve[0] = m_TimeGraph[0]->curve(curvetitle);
        if(pCurve[0]) pCurve[0]->clear();        else return;
        pCurve[1] = m_TimeGraph[1]->curve(curvetitle);
        if(pCurve[1]) pCurve[1]->clear();        else return;
        pCurve[2] = m_TimeGraph[2]->curve(curvetitle);
        if(pCurve[2]) pCurve[2]->clear();        else return;
        pCurve[3] = m_TimeGraph[3]->curve(curvetitle);
        if(pCurve[3]) pCurve[3]->clear();        else return;

        m_pStabTimeControls->fillTimeCurve(m_pCurPOpp, pCurve);

        pCurve[0]->setVisible(true);
        pCurve[1]->setVisible(true);
        pCurve[2]->setVisible(true);
        pCurve[3]->setVisible(true);

        m_bResetCurves = false;
        for(int ig=0; ig<m_TimeGraph.size(); ig++) m_TimeGraph[ig]->invalidate();
    }
    else if(isStabPolarView())
    {
        m_StabPlrGraph.at(0)->deleteCurves();
        m_StabPlrGraph.at(1)->deleteCurves();
        m_StabPlrGraph.at(0)->clearSelection();
        m_StabPlrGraph.at(1)->clearSelection();

        for (int k=0; k<Objects3d::nPolars(); k++)
        {
            PlanePolar *pWPolar = Objects3d::plPolarAt(k);
            pWPolar->clearCurves();

            if (!pWPolar->isVisible() || pWPolar->dataSize()<=0) continue;

            // four curves, one for each mode so that they can be
            // plotted as functions of angle of attacks
            for(int iCurve=0; iCurve<4; iCurve++)
            {
                Curve *pCurve = m_StabPlrGraph.at(0)->addCurve();
                pCurve->setTheStyle(pWPolar->theStyle());
                pCurve->setName(QString::fromStdString(pWPolar->planeName() + " / " + pWPolar->name())+QString::asprintf("_Mode_%d", iCurve+1));
                fillStabCurve(pCurve, pWPolar, iCurve);
                pWPolar->appendCurve(pCurve);
                if(pWPolar==m_pCurPlPolar)
                {
                    m_StabPlrGraph.at(0)->selectCurve(pCurve);
                }
            }

            //Lateral modes
            for(int iCurve=0; iCurve<4; iCurve++)
            {
                Curve *pCurve = m_StabPlrGraph.at(1)->addCurve();
                pCurve->setTheStyle(pWPolar->theStyle());
                pCurve->setName(QString::fromStdString(pWPolar->planeName() + " / " + pWPolar->name())+QString::asprintf("_Mode_%d", iCurve+1));
                fillStabCurve(pCurve, pWPolar, iCurve+4);
                pWPolar->appendCurve(pCurve);
                if(pWPolar==m_pCurPlPolar)
                {
                    m_StabPlrGraph.at(1)->selectCurve(pCurve);
                }
            }
        }

        m_bResetCurves = false;
        m_StabPlrGraph.at(0)->invalidate();
        m_StabPlrGraph.at(1)->invalidate();
    }
}


void XPlane::fillWOppCurve(PlanePolar const*pWPolar, PlaneOpp const *pPOpp, int iw, int iVar, Curve *pCurve)
{
    if(iVar<0 || !pCurve || !pPOpp) return;
    if(iw<0 ||iw>pPOpp->nWOpps()) return;
    WingOpp const &wopp = pPOpp->WOpp(iw);
    SpanDistribs const &SD = wopp.spanResults();

    int nstation = int(SD.m_StripPos.size()); // should be the same as wopp.m_NStation
    switch(iVar)
    {
        default:
        case 0:  // Local lift coef.
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_Cl.at(i));
            }
            break;
        }
        case 1:  // Local lift C.Cl/M.A.C.
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_Cl.at(i) * SD.m_Chord.at(i)/wopp.m_MAChord);
            }
            break;
        }
        case 2:  // Airfoil viscous drag coef.
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_PCd.at(i));
            }
            break;
        }
        case 3:  // Induced drag coef.
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_ICd.at(i));
            }
            break;
        }
        case 4:  // Total drag coef.
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_PCd.at(i)+ SD.m_ICd.at(i));
            }
            break;
        }
        case 5:  // Local drag C.Cd/M.A.C.
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), (SD.m_PCd.at(i)+ SD.m_ICd.at(i))* SD.m_Chord.at(i)/wopp.m_MAChord);
            }
            break;
        }
        case 6:  // 1/4 chord pressure pitching moment coef.
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_CmC4.at(i));
            }
            break;
        }
        case 7:  // CoG visc. pitching moment coef.
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_CmViscous.at(i));
            }
            break;
        }
        case 8: // CoG pressure pitching moment coef.
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_CmPressure.at(i));
            }
            break;
        }
        case 9: // Reynolds
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_Re.at(i));
            }
            break;
        }
        case 10: // Top transition x-pos%
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_XTrTop.at(i));
            }
            break;
        }
        case 11: // Bottom transition x-pos%"
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_XTrBot.at(i));
            }
            break;
        }
        case 12: // Centre of pressure x-pos%
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_XCPSpanRel.at(i)*100.0);
            }
            break;
        }
        case 13: //Strip lift
        {
            double qDyn = 0.5 * pPOpp->QInf()* pPOpp->QInf() * pWPolar->density();
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.stripLift(i, qDyn) * Units::NtoUnit());
            }
            break;
        }
        case 14: //Bending moment
        {
//            double qDyn = 0.5 * pPOpp->QInf()* pPOpp->QInf() * pWPolar->density();
//            double bm=0;
            Vector3d mom;

            if(nstation%2==0)
            {
                // add left side points
                for (int i=0; i<nstation/2; i++)
                {
                    pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_BendingMoment.at(i) * Units::NmtoUnit());
                    // calculate the central bending moment
                    mom += SD.m_PtC4.at(i) * SD.m_F.at(i);
//                    bm += SD.stripLift(i, qDyn) * (-SD.m_SpanPos.at(i));
                }
                // this is a two-sided wing, insert additional central point
                pCurve->appendPoint(0, -mom.x * Units::NmtoUnit());

                //add right side points
                for (int i=nstation/2; i<nstation; i++)
                    pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_BendingMoment.at(i) * Units::NmtoUnit());
            }
            else
            {
                // one-sided wing, plot as is
                for (int i=0; i<nstation; i++)
                    pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_BendingMoment.at(i) * Units::NmtoUnit());
            }

            break;
        }
        case 15:  // Total angle
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), pPOpp->alpha() + SD.m_Ai.at(i) + SD.m_Twist.at(i));
            }
            break;
        }
        case 16: //Effective aoa
        {
            for (int i=0; i<nstation; i++)
            {
                double aoa_effective = SD.m_Alpha_0.at(i) + (SD.m_Cl.at(i)/2.0/PI) *180.0/PI - SD.m_VTwist.at(i);

                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), aoa_effective);
            }
            break;
        }
        case 17:  // Induced angle
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_Ai.at(i));
            }
            break;
        }
        case 18: //Virtual twist
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_VTwist.at(i));
            }
            break;
        }
        case 19: // Strip circulation
        {
            for (int i=0; i<nstation; i++)
            {
                pCurve->appendPoint(SD.m_StripPos.at(i)*Units::mtoUnit(), SD.m_Gamma.at(i));
            }
            break;
        }
    }
}


void XPlane::fillStabCurve(Curve *pCurve, const PlanePolar *pWPolar, int iMode)
{
    pCurve->setSelectedPoint(-1);

    for (int i=0; i<pWPolar->dataSize(); i++)
    {
        double x = pWPolar->eigenValue(iMode, i).real();
        double y = pWPolar->eigenValue(iMode, i).imag()/2./PI;

        pCurve->appendPoint(x, y);
        if(m_pCurPlane && m_pCurPOpp && Graph::isHighLighting())
        {
            if(fabs(pWPolar->m_Alpha.at(i)-m_pCurPOpp->alpha())<ANGLEPRECISION)
            {
                if(m_pCurPlPolar->hasPOpp(m_pCurPOpp))
                {
                    pCurve->setSelectedPoint(i);
                }
            }
        }
    }
}


void XPlane::fillWPlrCurve(Curve *pCurve, PlanePolar const *pWPolar, int XVar, int YVar)
{
    pCurve->setSelectedPoint(-1);
    for (int i=0; i<pWPolar->dataSize(); i++)
    {
        double x = pWPolar->variable(XVar, i); // returns the value in user-defined units
        double y = pWPolar->variable(YVar, i); // returns the value in user-defined units


        // do not add negative speeds to the speed polar graphs- negative speeds occur in T1 analyses
        if(XVar==29 || XVar==30)
        {
            if(x<=0.0) continue; // don't append
        }
        if(YVar==29 || YVar==30)
        {
            if(y<=0.0) continue; // don't append
        }

        pCurve->appendPoint(x,y);

        if(m_pCurPOpp && pWPolar==m_pCurPlPolar && Graph::isHighLighting())
        {
            double d(0.001);

            if (m_pCurPOpp && m_pCurPlane && pWPolar->planeName()==m_pCurPlane->name() && m_pCurPOpp->polarName() ==pWPolar->name())
            {
                if(pWPolar->type()<xfl::T4POLAR)
                {
                    if(fabs(pWPolar->m_Alpha.at(i)-m_pCurPOpp->m_Alpha)<d)
                    {
                        pCurve->setSelectedPoint(i);
                    }
                }
                else if(pWPolar->isFixedaoaPolar())
                {
                    if(fabs(pWPolar->m_QInfinite.at(i)-m_pCurPOpp->m_QInf)<d)
                    {
                        pCurve->setSelectedPoint(i);
                    }
                }
                else if(pWPolar->isBetaPolar())
                {
                    if(fabs(pWPolar->m_Beta.at(i)-m_pCurPOpp->m_Beta)<d)
                    {
                        pCurve->setSelectedPoint(i);
                    }
                }
                else if(pWPolar->isStabilityPolar() || pWPolar->isControlPolar())
                {
                    if(fabs(pWPolar->m_Ctrl.at(i)-m_pCurPOpp->m_Ctrl)<d)
                    {
                        pCurve->setSelectedPoint(i);
                    }
                }
                else if(pWPolar->isType8Polar())
                {
                    if(fabs(pWPolar->m_Alpha.at(i)    -m_pCurPOpp->m_Alpha)<d &&
                       fabs(pWPolar->m_Beta.at(i)     -m_pCurPOpp->m_Beta )<d &&
                       fabs(pWPolar->m_QInfinite.at(i)-m_pCurPOpp->m_QInf )<d)
                    {
                        pCurve->setSelectedPoint(i);
                    }
                }
            }
        }
    }
}


void XPlane::onGraphChanged(int)
{
    resetCurves();
    updateView();
}


void XPlane::onVarSetChanged(int)
{
    if (m_eView==XPlane::POPPVIEW)
    {
        for(int ig=0; ig<m_WingGraph.size(); ig++)
        {
            m_WingGraph.at(ig)->setAuto(true);
            m_WingGraph.at(ig)->resetLimits();
            m_WingGraph.at(ig)->invalidate();
        }
    }
    else if (m_eView==XPlane::POLARVIEW)
    {
        for(int ig=0; ig<m_WPlrGraph.size(); ig++)
        {
            m_WPlrGraph.at(ig)->setAuto(true);
            m_WPlrGraph.at(ig)->resetLimits();
            m_WPlrGraph.at(ig)->invalidate();
        }
    }
    resetCurves();
    updateView();
}


void XPlane::resetGraphScales()
{
    if (m_eView==XPlane::POPPVIEW)
    {
        for(int ig=0; ig<m_WingGraph.size(); ig++)
        {
            m_WingGraph.at(ig)->setAuto(true);
            m_WingGraph.at(ig)->resetLimits();
            m_WingGraph.at(ig)->invalidate();
        }
    }
    else if (m_eView==XPlane::POLARVIEW)
    {
        for(int ig=0; ig<m_WPlrGraph.size(); ig++)
        {
            m_WPlrGraph.at(ig)->setAuto(true);
            m_WPlrGraph.at(ig)->resetLimits();
            m_WPlrGraph.at(ig)->invalidate();
        }
    }
    updateView();
}


void XPlane::onOpen3dViewInNewWindow()
{
    if(!m_pXPlaneWt)
    {
        m_pXPlaneWt = new XPlaneWt(this, m_pgl3dXPlaneView);
//        m_pXPlaneWt->gl3dFloatView()->setScaleCtrls(s_pMainFrame->m_pgl3dScales);
        updateVisiblePanels();
        m_pXPlaneWt->updateObjectData();
    }

    m_pXPlaneWt->show();
    m_pXPlaneWt->raise();
}


void XPlane::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl  = (pEvent->modifiers() & Qt::ControlModifier) ? true : false;
    bool bShift = (pEvent->modifiers() & Qt::ShiftModifier)   ? true : false;
//    bool bAlt   = (pEvent->modifiers() & Qt::AltModifier)     ? true : false;

    m_pgl3dXPlaneView->m_bArcball      = false;
    m_pgl3dXPlaneView->m_bPanelNormals = false;

    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if (pEvent->modifiers().testFlag(Qt::AltModifier))
            {
                if(bShift) onPlaneOppProperties();
                else       onPolarProperties();
                break;
            }
            pEvent->accept();
            break;
        }
        case Qt::Key_Escape:
        {
            if(m_pPOpp3dCtrls->getDistance())
            {
                m_pPOpp3dCtrls->stopDistance();
                m_pgl3dXPlaneView->setPicking(xfl::NOPICK);
                m_pgl3dXPlaneView->clearMeasure();
            }

            stopAnimate();
            updateView();
            break;
        }
        case Qt::Key_1:
        {
            if(bCtrl)
            {
                s_pMainFrame->onDFoil();
                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_D:
        {
            if(bCtrl) onDuplicateCurPlane();
            break;
        }
        case Qt::Key_L:
        {
            if(bShift || bCtrl)
            {
            }
            else
            {
                s_pMainFrame->onLogFile();
            }
            break;
        }
        case Qt::Key_W:
        {
            if(bCtrl)
            {
                if(bShift && is3dView())
                {
                    onOpen3dViewInNewWindow();
                }
            }
            break;
        }
        case Qt::Key_F2:
        {
            if(bShift)
            {
                onRenameCurWPolar();
            }
            else if(bCtrl)
            {
            }
            else
            {
                if     (m_pPlaneExplorer->isPlaneSelected())  onRenameCurPlane();
                else if(m_pPlaneExplorer->isWPolarSelected()) onRenameCurWPolar();
            }
            break;
        }
        case Qt::Key_F12:
        {
            if(bCtrl || bShift)
            {
            }
            else
            {
                onPlaneInertia();
            }
            break;
        }
        default:
            pEvent->ignore();
            return;
    }
    pEvent->accept();
}


void XPlane::keyReleaseEvent(QKeyEvent*)
{
    m_pgl3dXPlaneView->m_bArcball=false;

    updateView();
}


bool XPlane::loadSettings(QSettings &settings)
{
    settings.beginGroup("XPlane");
    {
        QPoint pt;
        QSize size;
        pt.rx() = settings.value("Stab3dControls_x").toInt();
        pt.ry() = settings.value("Stab3dControls_y").toInt();
        size    = settings.value("Stab3dControlsSize").toSize();

        m_pgl3dXPlaneView->m_bSurfaces     = settings.value("bSurfaces").toBool();
        m_pgl3dXPlaneView->m_bOutline      = settings.value("bOutline").toBool();
        m_pgl3dXPlaneView->m_bMeshPanels   = settings.value("bVLMPanels").toBool();
        m_pgl3dXPlaneView->m_bAxes         = settings.value("bAxes").toBool();
        m_pgl3dXPlaneView->m_bShowCpScale  = settings.value("bShowCpScale").toBool();

        m_bCurPOppOnly       = settings.value("CurWOppOnly", false).toBool();
        m_bShowEllipticCurve = settings.value("bShowElliptic").toBool();
        m_bShowBellCurve     = settings.value("bShowTargetCurve").toBool();
        m_BellCurveExp       = settings.value("BellCurveExp", 1).toDouble();
        m_bMaxCL             = settings.value("CurveMaxCL", true ).toBool();
        xfl::loadLineSettings(settings, m_TargetCurveStyle, "TargetCurveStyle");

        Panel3::setQuadratureOrder(settings.value("QuadratureOrder", 3).toInt());

        s_bStoreOpps3d = settings.value("StorePlaneOpp").toBool();

        int k = settings.value("iView").toInt();
        switch(k)
        {
            case 0: m_eView = XPlane::POPPVIEW;      break;
            default:
            case 1: m_eView = XPlane::POLARVIEW;    break;
            case 2: m_eView = XPlane::W3DVIEW;       break;
            case 3: m_eView = XPlane::CPVIEW;        break;
            case 4: m_eView = XPlane::STABTIMEVIEW;  break;
            case 5: m_eView = XPlane::STABPOLARVIEW; break;
        }

        AeroDataDlg::setTemperature(settings.value("Temperature", AeroDataDlg::temperature()).toDouble());
        AeroDataDlg::setAltitude(   settings.value("Altitude", AeroDataDlg::altitude()).toDouble());

    }
    settings.endGroup();

    settings.beginGroup("LLTAnalysis");
    {
        LLTTask::setNSpanStations(settings.value("NLLTStations",  LLTTask::nSpanStations()).toInt());
        LLTTask::setRelaxationFactor(settings.value("RelaxFactor",   LLTTask::relaxationFactor()).toDouble());
        LLTTask::setConvergencePrecision(settings.value("CvPrecision",   LLTTask::convergencePrecision()).toDouble());
        LLTTask::setMaxIter(settings.value("MaxIterations", LLTTask::maxIter()).toInt());
    }
    settings.endGroup();


    AnalysisRangeTable::loadSettings(settings);
    T8RangeTable::loadSettings(settings);
    Analysis3dSettings::loadSettings(settings);
    BatchPlaneDlg::loadSettings(settings);
    BatchXmlDlg::loadSettings(settings);
    CADExportDlg::loadSettings(settings);
    ExtraDragDlg::loadSettings(settings);
    FlatFaceConverterDlg::loadSettings(settings);
    FuseOccDlg::loadSettings(settings);
    FuseStlDlg::loadSettings(settings);
    FuseXflDefDlg::loadSettings(settings);
    GLLightDlg::loadSettings(settings);
    LLTAnalysisDlg::loadSettings(settings);
    OptimPlaneDlg::loadSettings(settings);
    PanelCheckDlg::loadSettings(settings);
    PartInertiaDlg::loadSettings(settings);
    PlaneAnalysisDlg::loadSettings(settings);
    PlaneDlg::loadSettings(settings);
    PlaneInertiaDlg::loadSettings(settings);
    PlanePolarDlg::loadSettings(settings);
    PlaneSTLDlg::loadSettings(settings);
    PlaneExplorer::loadSettings(settings);
    PlaneXflDlg::loadSettings(settings);
    STLWriterDlg::loadSettings(settings);
    ShapeFixerDlg::loadSettings(settings);
    StlReaderDlg::loadSettings(settings);
    WPolarAutoNameDlg::loadSettings(settings);
    WingDefDlg::loadSettings(settings);
    WingObjectDlg::loadSettings(settings);
    WingSelDlg::loadSettings(settings);
    WingExportDlg::loadSettings(settings);
    XPlaneWt::loadSettings(settings);

    for(int ig=0; ig<m_WingGraph.size(); ig++) m_WingGraph[ig]->loadSettings(settings);
    // set the variable names with loaded units.
    QStringList XPOppList = {"y ("+Units::lengthUnitQLabel() +")"};
    for(int ig=0; ig<m_WingGraph.size(); ig++)
    {
        m_WingGraph.at(ig)->setXVariableList(XPOppList);
        m_WingGraph.at(ig)->setYVarStdList(PlaneOpp::variableNames());
    }
    s_pMainFrame->m_pPOppTiles->loadSettings(settings, "POppGraphTiles");
    if(s_pMainFrame->m_pPOppTiles->variableSetCount()>0)
    {
        GraphTileVariableSet const & variableset = s_pMainFrame->m_pPOppTiles->variableSet(0);
        for(int ig=0; ig<m_WingGraph.size(); ig++)
        {
            m_WingGraph.at(ig)->setVariables(variableset.XVar(ig), variableset.YVar(ig));
        }
    }


    for(int ig=0; ig<m_WPlrGraph.size(); ig++) m_WPlrGraph[ig]->loadSettings(settings);
    for(int ig=0; ig<m_WPlrGraph.size(); ig++)
    {
        m_WPlrGraph.at(ig)->setXVarStdList(PlanePolar::variableNames());
        m_WPlrGraph.at(ig)->setYVarStdList(PlanePolar::variableNames());
    }
    s_pMainFrame->m_pWPolarTiles->loadSettings(settings, "WPolarTiles");
    if(s_pMainFrame->m_pWPolarTiles->variableSetCount()>0)
    {
        GraphTileVariableSet const & variableset = s_pMainFrame->m_pWPolarTiles->variableSet(0);
        for(int ig=0; ig<m_WPlrGraph.size(); ig++)
        {
            m_WPlrGraph.at(ig)->setVariables(variableset.XVar(ig), variableset.YVar(ig));
        }
    }



    for(int ig=0; ig<m_TimeGraph.size(); ig++) m_TimeGraph[ig]->loadSettings(settings);
    s_pMainFrame->m_pStabTimeTiles->loadSettings( settings, "StabTimeTiles");
    if(s_pMainFrame->m_pStabTimeTiles->variableSetCount()>0)
    {
        GraphTileVariableSet const & variableset = s_pMainFrame->m_pStabTimeTiles->variableSet(0);
        for(int ig=0; ig<m_TimeGraph.size(); ig++)
        {
            m_TimeGraph.at(ig)->setVariables(variableset.XVar(ig), variableset.YVar(ig));
        }
    }

    m_StabPlrGraph.at(0)->loadSettings(settings);
    m_StabPlrGraph.at(1)->loadSettings(settings);
    s_pMainFrame->m_pStabPolarTiles->loadSettings(settings, "StabPolarTiles");
    if(s_pMainFrame->m_pStabPolarTiles->variableSetCount()>0)
    {
        GraphTileVariableSet const & variableset = s_pMainFrame->m_pStabPolarTiles->variableSet(0);
        for(int ig=0; ig<m_StabPlrGraph.size(); ig++)
        {
            m_StabPlrGraph.at(ig)->setVariables(variableset.XVar(ig), variableset.YVar(ig));
        }
    }

    m_pStabTimeControls->loadSettings(settings);
    m_pPOpp3dCtrls->loadSettings(settings);

    m_pPlaneExplorer->setTreeFont(DisplayOptions::treeFontStruct().font());
    m_pPlaneExplorer->setPropertiesFont(DisplayOptions::tableFont());


    m_pPOpp3dCtrls->initWidget();

    return true;
}


bool XPlane::saveSettings(QSettings &settings)
{
    settings.beginGroup("XPlane");
    {
        settings.setValue("bSurfaces",        m_pgl3dXPlaneView->m_bSurfaces);
        settings.setValue("bOutline",         m_pgl3dXPlaneView->m_bOutline);
        settings.setValue("bVLMPanels",       m_pgl3dXPlaneView->m_bMeshPanels);
        settings.setValue("bAxes",            m_pgl3dXPlaneView->m_bAxes);

        settings.setValue("bShowCpScale",     m_pgl3dXPlaneView->m_bShowCpScale);

        settings.setValue("CurWOppOnly",      m_bCurPOppOnly);
        settings.setValue("bShowElliptic",    m_bShowEllipticCurve);
        settings.setValue("bShowTargetCurve", m_bShowBellCurve);
        settings.setValue("BellCurveExp",     m_BellCurveExp);
        settings.setValue("CurveMaxCL",       m_bMaxCL);
        xfl::saveLineSettings(settings,       m_TargetCurveStyle, "TargetCurveStyle");

        settings.setValue("QuadratureOrder", Panel3::quadratureOrder());
        settings.setValue("StorePlaneOpp",   s_bStoreOpps3d);

        switch(m_eView)
        {
            case XPlane::POPPVIEW:               settings.setValue("iView", 0);    break;
            case XPlane::POLARVIEW:             settings.setValue("iView", 1);    break;
            case XPlane::W3DVIEW:                settings.setValue("iView", 2);    break;
            case XPlane::CPVIEW:                 settings.setValue("iView", 3);    break;
            case XPlane::STABTIMEVIEW:           settings.setValue("iView", 4);    break;
            case XPlane::STABPOLARVIEW:          settings.setValue("iView", 5);    break;
            case XPlane::OTHERVIEW:                                                break;
        }

        settings.setValue("Temperature", AeroDataDlg::temperature());
        settings.setValue("Altitude", AeroDataDlg::altitude());
    }
    settings.endGroup();

    settings.beginGroup("LLTAnalysis");
    {
        settings.setValue("NLLTStations",  LLTTask::nSpanStations());
        settings.setValue("RelaxFactor",   LLTTask::relaxationFactor());
        settings.setValue("CvPrecision",   LLTTask::convergencePrecision());
        settings.setValue("MaxIterations", LLTTask::maxIter());
    }
    settings.endGroup();

    m_pPOpp3dCtrls->saveSettings(settings);
    m_pStabTimeControls->saveSettings(settings);
    m_StabPlrGraph.at(0)->saveSettings(settings);
    m_StabPlrGraph.at(1)->saveSettings(settings);

    for(int ig=0; ig<m_WPlrGraph.count(); ig++)
        m_WPlrGraph[ig]->saveSettings(settings);
    for(int ig=0; ig<m_WingGraph.count(); ig++) m_WingGraph[ig]->saveSettings(settings);
    for(int ig=0; ig<m_TimeGraph.count(); ig++) m_TimeGraph[ig]->saveSettings(settings);

    s_pMainFrame->m_pWPolarTiles->saveSettings(   settings, "WPolarTiles");
    s_pMainFrame->m_pPOppTiles->saveSettings(     settings, "POppGraphTiles");
    s_pMainFrame->m_pStabPolarTiles->saveSettings(settings, "StabPolarTiles");
    s_pMainFrame->m_pStabTimeTiles->saveSettings( settings, "StabTimeTiles");
    s_pMainFrame->m_pCpViewWt->saveSettings(settings);


    AnalysisRangeTable::saveSettings(settings);
    T8RangeTable::saveSettings(settings);
    Analysis3dSettings::saveSettings(settings);
    BatchPlaneDlg::saveSettings(settings);
    BatchXmlDlg::saveSettings(settings);
    CADExportDlg::saveSettings(settings);
    T6PolarDlg::saveSettings(settings);
    ExtraDragDlg::saveSettings(settings);
    FlatFaceConverterDlg::saveSettings(settings);
    FuseOccDlg::saveSettings(settings);
    FuseStlDlg::saveSettings(settings);
    FuseXflDefDlg::saveSettings(settings);
    GLLightDlg::saveSettings(settings);
    LLTAnalysisDlg::saveSettings(settings);
    OptimPlaneDlg::saveSettings(settings);
    PanelCheckDlg::saveSettings(settings);
    PartInertiaDlg::saveSettings(settings);
    PlaneAnalysisDlg::saveSettings(settings);
    PlaneDlg::saveSettings(settings);
    PlaneInertiaDlg::saveSettings(settings);
    PlanePolarDlg::saveSettings(settings);
    PlaneSTLDlg::saveSettings(settings);
    PlaneExplorer::saveSettings(settings);
    PlaneXflDlg::saveSettings(settings);
    STLWriterDlg::saveSettings(settings);
    ShapeFixerDlg::saveSettings(settings);
    StlReaderDlg::saveSettings(settings);
    WPolarAutoNameDlg::saveSettings(settings);
    WingDefDlg::saveSettings(settings);
    WingObjectDlg::saveSettings(settings);
    WingSelDlg::saveSettings(settings);
    WingExportDlg::saveSettings(settings);
    XPlaneWt::saveSettings(settings);

    return true;
}


void XPlane::on3dView()
{
    if(is3dView())
    {
        setControls();
        updateView();
        return;
    }

    m_eView = XPlane::W3DVIEW;
    setControls();

    s_pMainFrame->setActiveCentralWidget();
    updateView();
}


void XPlane::resetPrefs()
{
    m_pgl3dXPlaneView->setLabelFonts();

    m_pgl3dXPlaneView->resetglGeom();
    m_pgl3dXPlaneView->resetglMesh();
    m_pgl3dXPlaneView->resetglPOpp();
    m_pgl3dXPlaneView->resetglStreamlines();

    if(m_pCurPlane)
    {
        QString props = QString::fromStdString(m_pCurPlane->planeData(true));
        if(m_pCurPlPolar)
        {
            props = QString::fromStdString(m_pCurPlane->planeData(m_pCurPlPolar->bIncludeOtherWingAreas()));
            props +="\n";
            QString strange;
            if(m_pCurPlPolar->isQuadMethod() && m_pCurPlane->isXflType())
            {
                PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);
                strange = QString::asprintf("Quad panels     = %d", pPlaneXfl->quadMesh().nPanels());
            }
            else if(m_pCurPlPolar->isTriangleMethod())
                strange = QString::asprintf("Triangles       = %d", m_pCurPlane->triMesh().nPanels());
            props += strange;
        }

        m_pgl3dXPlaneView->setBotLeftOutput(props);
        if(m_pXPlaneWt) m_pXPlaneWt->updateObjectData();
    }
    else
    {
        m_pgl3dXPlaneView->setBotLeftOutput(QString());
        if(m_pXPlaneWt) m_pXPlaneWt->updateObjectData();
    }
    if(m_pCurPOpp)  m_pgl3dXPlaneView->setBotRightOutput(planeOppData());
    else            m_pgl3dXPlaneView->clearBotRightOutput();

    m_pgl3dXPlaneView->updatePartFrame(m_pCurPlane);

    m_pgl3dXPlaneView->glMakeArcBall(m_pgl3dXPlaneView->m_ArcBall);
    m_pgl3dXPlaneView->glMakeArcPoint(m_pgl3dXPlaneView->m_ArcBall);

    m_pgl3dXPlaneView->onZAnimate(m_pgl3dXPlaneView->bZAnimation());

    s_pMainFrame->m_pCpViewWt->CpGraph()->setYInverted(0, true);
    m_pPlaneExplorer->setPropertiesFont(DisplayOptions::tableFont());
    m_pPlaneExplorer->setFont(DisplayOptions::treeFont());
    m_pPlaneExplorer->setTreeFont(DisplayOptions::treeFontStruct().font());

    if(!W3dPrefs::isClipPlaneEnabled())
    {
        m_pgl3dXPlaneView->m_ClipPlanePos = 500.0f;
        m_pPOpp3dCtrls->resetClipPlane();
    }

    if(s_pMainFrame->xflApp()==xfl::XPLANE)
    {
        m_bResetCurves = true;
        setControls();
        updateView();
    }

    m_pgl3dXPlaneView->m_ColourLegend.setColorGradient();

}


void XPlane::onAnimateModeSingle(bool bStep)
{
    if(!is3dView())
    {
        m_pTimerMode->stop();
        return; //nothing to animate
    }
    if(!m_pCurPlane || !m_pCurPlPolar || !m_pCurPlPolar->isStabilityPolar() || !m_pCurPOpp)
    {
        m_pTimerMode->stop();
        return; //nothing to animate
    }

    m_pPOpp3dCtrls->m_pStab3dCtrls->incrementTime(m_pCurPOpp, bStep);

    updateView();
}


void XPlane::onAdjustWingGraphToWingSpan()
{
    if(!m_pCurPlane) return;

    double halfspan = m_pCurPlane->planformSpan()/2.0;
    double xmin = -halfspan*Units::mtoUnit();
    for(int ig=0; ig<m_WingGraph.size(); ig++)
    {
        m_WingGraph[ig]->setAutoX(false);
        m_WingGraph[ig]->setXMax( halfspan*Units::mtoUnit());
        m_WingGraph[ig]->setXMin(xmin);
    }
}


void XPlane::onCurPOppOnly()
{
    m_bCurPOppOnly = !m_bCurPOppOnly;
    m_bResetCurves = true;
    updateView();
    setControls();
}


void XPlane::onDefineT6Polar()
{
    if(!m_pCurPlane) return;

    m_pgl3dXPlaneView->m_bArcball = false;
    stopAnimate();

    T6PolarDlg CtrlDlg(s_pMainFrame);
    CtrlDlg.initPolar3dDlg(m_pCurPlane);
    int res = CtrlDlg.exec();

    if(res == QDialog::Accepted)
    {
        emit projectModified();

        PlanePolar* pNewControlPolar      = new PlanePolar;
        pNewControlPolar->setPlaneName(m_pCurPlane->name());
        pNewControlPolar->setName(T6PolarDlg::staticWPolar().name());

        pNewControlPolar->duplicateSpec(&T6PolarDlg::staticWPolar());

        pNewControlPolar->setLineWidth(m_pCurPlane->theStyle().m_Width);
        pNewControlPolar->setLineStipple(m_pCurPlane->theStyle().m_Stipple);
        pNewControlPolar->setPointStyle(Line::NOSYMBOL);
        Objects3d::setPlPolarColor(m_pCurPlane, pNewControlPolar, xfl::darkFactor());
        pNewControlPolar->setVisible(true);

        pNewControlPolar->setGroundEffect(false);
        pNewControlPolar->setAlphaSpec(0.0);
        pNewControlPolar->setGroundHeight(0.0);

        pNewControlPolar = Objects3d::insertNewPolar(pNewControlPolar, m_pCurPlane);
        if(pNewControlPolar)
        {
            m_pCurPOpp = nullptr;
            setPolar(pNewControlPolar);
            m_pPlaneExplorer->insertWPolar(pNewControlPolar);
            m_pPlaneExplorer->selectPlPolar(pNewControlPolar, false);
        }
        m_pgl3dXPlaneView->resetglGeom();
        m_pgl3dXPlaneView->resetglPOpp();
        m_pgl3dXPlaneView->resetglMesh();
        m_pgl3dXPlaneView->s_bResetglWake = true;

        updateView();
    }

    setControls();
}


void XPlane::onDefineT123578Polar()
{
    if(!m_pCurPlane) return;

    stopAnimate();

    T1234578PolarDlg wpDlg(s_pMainFrame);
    wpDlg.initPolar3dDlg(m_pCurPlane);

    int res = wpDlg.exec();

    if (res == QDialog::Accepted)
    {
        //Then add the WPolar to the array
        emit projectModified();
        PlanePolar* pNewWPolar  = new PlanePolar;
        pNewWPolar->duplicateSpec(&T1234578PolarDlg::staticWPolar());
        pNewWPolar->setLineWidth(m_pCurPlane->theStyle().m_Width);
        pNewWPolar->setLineStipple(m_pCurPlane->theStyle().m_Stipple);
        pNewWPolar->setPointStyle(m_pCurPlane->theStyle().m_Symbol);
        Objects3d::setPlPolarColor(m_pCurPlane, pNewWPolar, xfl::darkFactor());
        pNewWPolar->setPlaneName(m_pCurPlane->name());
        pNewWPolar->setName(wpDlg.staticWPolar().name());

        double refarea=0.0;
        if(pNewWPolar->referenceDim()==xfl::PLANFORM)
        {
            refarea = m_pCurPlane->planformArea(pNewWPolar->bIncludeOtherWingAreas());
            pNewWPolar->setReferenceSpanLength(m_pCurPlane->planformSpan());
        }
        else if(pNewWPolar->referenceDim()==xfl::PROJECTED)
        {
            pNewWPolar->setReferenceSpanLength(m_pCurPlane->projectedSpan());
            refarea = m_pCurPlane->projectedArea(pNewWPolar->bIncludeOtherWingAreas());
        }
        pNewWPolar->setReferenceArea(refarea);

        pNewWPolar->setVisible(true);

        pNewWPolar = Objects3d::insertNewPolar(pNewWPolar, m_pCurPlane);

        if(pNewWPolar)
        {
            setPolar(pNewWPolar);
            m_pPlaneExplorer->insertWPolar(pNewWPolar);
            m_pPlaneExplorer->selectPlPolar(pNewWPolar, false);
            m_pCurPOpp = nullptr;
        }

        m_pgl3dXPlaneView->resetglGeom();
        m_pgl3dXPlaneView->resetglMesh();
        m_pgl3dXPlaneView->s_bResetglWake = true;
        m_pgl3dXPlaneView->resetglPOpp();

        updateView();
    }
    setControls();
}


void XPlane::onDefineT7Polar()
{
    if(!m_pCurPlane) return;

    stopAnimate();

    T1234578PolarDlg wpDlg(s_pMainFrame);
    wpDlg.initPolar3dDlg(m_pCurPlane);

    wpDlg.setType7Polar();

    int res = wpDlg.exec();

    if (res == QDialog::Accepted)
    {
        //Then add the WPolar to the array
        emit projectModified();
        PlanePolar* pNewWPolar  = new PlanePolar;
        pNewWPolar->duplicateSpec(&T1234578PolarDlg::staticWPolar());
        pNewWPolar->setLineWidth(m_pCurPlane->theStyle().m_Width);
        pNewWPolar->setLineStipple(m_pCurPlane->theStyle().m_Stipple);
        pNewWPolar->setPointStyle(m_pCurPlane->theStyle().m_Symbol);
        Objects3d::setPlPolarColor(m_pCurPlane, pNewWPolar, xfl::darkFactor());
        pNewWPolar->setPlaneName(m_pCurPlane->name());
        pNewWPolar->setName(wpDlg.staticWPolar().name());

        double refarea=0.0;
        if(pNewWPolar->referenceDim()==xfl::PLANFORM)
        {
            refarea = m_pCurPlane->planformArea(pNewWPolar->bIncludeOtherWingAreas());
            pNewWPolar->setReferenceSpanLength(m_pCurPlane->planformSpan());
        }
        else if(pNewWPolar->referenceDim()==xfl::PROJECTED)
        {
            pNewWPolar->setReferenceSpanLength(m_pCurPlane->projectedSpan());
            refarea = m_pCurPlane->projectedArea(pNewWPolar->bIncludeOtherWingAreas());
        }
        pNewWPolar->setReferenceArea(refarea);

        pNewWPolar->setVisible(true);

        pNewWPolar = Objects3d::insertNewPolar(pNewWPolar, m_pCurPlane);

        if(pNewWPolar)
        {
            setPolar(pNewWPolar);
            m_pPlaneExplorer->insertWPolar(pNewWPolar);
            m_pPlaneExplorer->selectPlPolar(pNewWPolar, false);
            m_pCurPOpp = nullptr;
        }

        m_pgl3dXPlaneView->resetglGeom();
        m_pgl3dXPlaneView->resetglMesh();
        m_pgl3dXPlaneView->s_bResetglWake = true;
        m_pgl3dXPlaneView->resetglPOpp();

        updateView();
    }
    setControls();
}


void XPlane::onBatchAnalysis()
{
    BatchPlaneDlg BatchDlg(s_pMainFrame);
    BatchDlg.exec();
    if(BatchDlg.bChanged())
    {
        m_pPlaneExplorer->updatePOpps();
//        m_pPlaneTreeView->selectWPolar(m_pCurWPolar, false);
        emit projectModified();
    }
    m_bResetCurves = true;
    updateView();
    setControls();
}


void XPlane::onBatchAnalysis2()
{
    m_pCurPlane = nullptr;
    m_pCurPlPolar = nullptr;
    m_pCurPOpp = nullptr;

    BatchXmlDlg BatchDlg(s_pMainFrame);
    BatchDlg.exec();
    if(BatchDlg.bChanged())
    {
        m_pPlaneExplorer->updatePOpps();
//        m_pPlaneTreeView->selectWPolar(m_pCurWPolar, false);
        emit projectModified();
    }
    m_bResetCurves = true;
    updateView();
    setControls();
}


void XPlane::onDuplicateCurAnalysis()
{
    if(!m_pCurPlane || !m_pCurPlPolar) return;

    PlanePolar* pNewWPolar(nullptr);
    if(m_pCurPlPolar->isExternalPolar())
    {
        pNewWPolar = new PlanePolarExt;
        pNewWPolar->copy(m_pCurPlPolar);
    }
    else pNewWPolar = new PlanePolar(*m_pCurPlPolar);

    pNewWPolar->setVisible(true);

    pNewWPolar = Objects3d::insertNewPolar(pNewWPolar, m_pCurPlane);

    if(pNewWPolar)
    {
        setPolar(pNewWPolar);
        m_pPlaneExplorer->insertWPolar(pNewWPolar);
        m_pPlaneExplorer->selectPlPolar(pNewWPolar, false);
        m_pCurPOpp = nullptr;
    }

    m_pgl3dXPlaneView->resetglGeom();
    m_pgl3dXPlaneView->resetglMesh();
    m_pgl3dXPlaneView->s_bResetglWake = true;
    m_pgl3dXPlaneView->resetglPOpp();
    updateView();
    setControls();

    emit projectModified();
}


void XPlane::onDuplicateAnalyses()
{
    if(!m_pCurPlane) return;

    AnalysisSelDlg dlg(s_pMainFrame);
    dlg.initDialog(nullptr, m_pCurPlane, nullptr);
    if(dlg.exec()!=QDialog::Accepted) return;
    if(!dlg.selected3dPolars().size()) return;

    PlanePolar* pNewWPolar(nullptr);
    for(int ip=0; ip<dlg.selected3dPolars().size(); ip++)
    {
        Polar3d const *pOldPolar = dlg.selectedPolar3d(ip);
        if(pOldPolar->isExternalPolar())
        {
            PlanePolarExt const *pPolarExt = dynamic_cast<PlanePolarExt const*>(pOldPolar);
            PlanePolarExt *pNewWPolarExt = new PlanePolarExt;
            pNewWPolarExt->copy(pPolarExt);
            pNewWPolar = pNewWPolarExt;
        }
        else
        {
//            WPolar const *pPolar = dynamic_cast<WPolar const*>(pOldPolar);
            pNewWPolar = new PlanePolar;
            pNewWPolar->duplicateSpec(pOldPolar);
        }
        pNewWPolar->setPlaneName(m_pCurPlane->name());
        pNewWPolar->setName(pOldPolar->name());
        pNewWPolar->setLineColor(m_pCurPlane->lineColor());
        pNewWPolar->setVisible(true);

        PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pCurPlane);
        if(pPlaneXfl)
        {
            pNewWPolar->resizeFlapCtrls(pPlaneXfl); // source plane may not have the same number of flaps as dest plane
        }
        pNewWPolar = Objects3d::insertNewPolar(pNewWPolar, m_pCurPlane);
        if(pNewWPolar) m_pPlaneExplorer->insertWPolar(pNewWPolar);
    }

    if(pNewWPolar)
    {
        setPolar(pNewWPolar);
        m_pPlaneExplorer->selectPlPolar(pNewWPolar, false);
        m_pCurPOpp = nullptr;
    }

    m_pgl3dXPlaneView->resetglGeom();
    m_pgl3dXPlaneView->resetglMesh();
    m_pgl3dXPlaneView->s_bResetglWake = true;
    m_pgl3dXPlaneView->resetglPOpp();
    updateView();
    setControls();

    emit projectModified();
}


void XPlane::onImportExternalPolar()
{
    if(!m_pCurPlane) return;

    stopAnimate();

    PlanePolarExt* pNewPlPolar  = new PlanePolarExt;
    pNewPlPolar->setType(xfl::EXTERNALPOLAR);
    pNewPlPolar->setAnalysisMethod(xfl::NOMETHOD);
    pNewPlPolar->setReferenceArea(0.0);
    pNewPlPolar->setVisible(true);
    pNewPlPolar->setLineWidth(Curve::defaultLineWidth());
    pNewPlPolar->setPointStyle(m_pCurPlane->theStyle().m_Symbol);
    Objects3d::setPlPolarColor(m_pCurPlane, pNewPlPolar, xfl::darkFactor());
    pNewPlPolar->setPlaneName(m_pCurPlane->name());
    pNewPlPolar->setName(std::string("Imported polar"));

    EditPlrDlg wpDlg(s_pMainFrame);
    wpDlg.initDialog(nullptr, pNewPlPolar, nullptr);
    connect(&wpDlg, SIGNAL(dataChanged()), this, SLOT(onResetWPolarCurves()));

    if (wpDlg.exec() != QDialog::Accepted)
    {
        delete pNewPlPolar;
        return;
    }
    // Add the WPolar to the array
    emit projectModified();

    Objects3d::insertNewPolar(pNewPlPolar, m_pCurPlane);

    if(pNewPlPolar)
    {
        setPolar(pNewPlPolar);
        m_pPlaneExplorer->insertWPolar(pNewPlPolar);
        m_pPlaneExplorer->selectPlPolar(pNewPlPolar, false);
        m_pCurPOpp = nullptr;
    }

    m_pgl3dXPlaneView->resetglGeom();
    m_pgl3dXPlaneView->resetglMesh();
    m_pgl3dXPlaneView->s_bResetglWake = true;
    m_pgl3dXPlaneView->resetglPOpp();
    updateView();

    setControls();
}


void XPlane::onEditCurPlPolarPts()
{
    stopAnimate();

    if(!m_pCurPlane || !m_pCurPlPolar) return;

    PlanePolar* pMemWPolar = nullptr;
    if(m_pCurPlPolar->isExternalPolar())
        pMemWPolar = new PlanePolarExt;
    else
        pMemWPolar = new PlanePolar;
    pMemWPolar->copy(m_pCurPlPolar);

    EditPlrDlg epDlg(s_pMainFrame);
    epDlg.initDialog(nullptr, m_pCurPlPolar);

    connect(&epDlg, SIGNAL(dataChanged()), this, SLOT(onResetWPolarCurves()));

    Line::enumPointStyle iPoints = m_pCurPlPolar->pointStyle();
    m_pCurPlPolar->setPointStyle(Line::NOSYMBOL);

    m_bResetCurves = true;
    updateView();

    if(epDlg.exec() == QDialog::Accepted)
    {
        emit projectModified();
    }
    else
    {
        m_pCurPlPolar->copy(pMemWPolar);
    }
    m_pCurPlPolar->setPointStyle(iPoints);

    delete pMemWPolar;

    m_bResetCurves = true;
    updateView();
    setControls();
}


/**
 * The user has requested a deletion of all the WOpps or POpps associated to the active WPolar.
 */
void XPlane::onDeleteWPlrPOpps()
{
    if(!m_pCurPlane || !m_pCurPlPolar) return;

    emit projectModified();

    m_pPlaneExplorer->removeWPolarPOpps(m_pCurPlPolar);

    for (int i=Objects3d::nPOpps()-1; i>=0; i--)
    {
        PlaneOpp*pPOpp =  Objects3d::POppAt(i);

        if(pPOpp->polarName() == m_pCurPlPolar->name() &&
                pPOpp->planeName() == m_pCurPlane->name())
        {
            Objects3d::removePOppAt(i);
            delete pPOpp;
        }
    }

    m_pCurPOpp = nullptr;
    m_pgl3dXPlaneView->resetglMesh();
    m_pgl3dXPlaneView->s_bResetglWake = true;


    setPlaneOpp(nullptr);
    setControls();
    m_bResetCurves = true;
    updateView();
}


/**
 * The user has requested a deletion of all the POpps
 */
void XPlane::onDeleteAllPOpps()
{
    emit projectModified();
    PlaneOpp* pPOpp;

    for (int i = Objects3d::nPOpps()-1; i>=0; i--)
    {
        pPOpp =  Objects3d::POppAt(i);
        Objects3d::removePOppAt(i);
        delete pPOpp;
    }

    m_pCurPOpp = nullptr;
    updateTreeView();

    setPlaneOpp(nullptr);

    setControls();

    m_bResetCurves = true;
    updateView();
}


void XPlane::onDeleteCurPlane()
{
    if(!m_pCurPlane) return;
    if(m_pCurPlane->isLocked())
    {
        displayMessage("The plane is locked by an analysis and cannot be deleted.\n\n", true, false);
        return;
    }
    stopAnimate();

    QString strong;
    if(m_pCurPlane) strong = tr("Are you sure you want to delete the plane :\n") + QString::fromStdString(m_pCurPlane->name()) +"?\n";
    if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, tr("Question"), strong, QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel)) return;
    QString nextPlaneName = m_pPlaneExplorer->removePlane(m_pCurPlane);
    Objects3d::deletePlaneResults(m_pCurPlane, true);
    Objects3d::deleteExternalPolars(m_pCurPlane);
    Objects3d::deletePlane(m_pCurPlane);

    m_pCurPlane = nullptr;
    m_pCurPlPolar = nullptr;
    m_pCurPOpp = nullptr;

    setPlane(nextPlaneName);
    setPolar();
    setPlaneOpp(nullptr);

    m_pPlaneExplorer->selectObjects();
    setControls();
    m_bResetCurves = true;

    emit projectModified();
    updateView();
}


void XPlane::onDeleteCurPOpp()
{
    if(!m_pCurPOpp) return;

    PlaneOpp *pCurPOpp = m_pCurPOpp;

    int io(0);
    for (io=0; io<Objects3d::nPOpps(); io++)
    {
        PlaneOpp* pOldPOpp = Objects3d::POppAt(io);
        if(pOldPOpp == m_pCurPOpp)
        {
            m_pPlaneExplorer->removePlaneOpp(m_pCurPOpp); // triggers a change of CurPOpp
            Objects3d::removePOppAt(io);
            delete pCurPOpp;
            m_pCurPOpp = nullptr;
            break;
        }
    }

    if(m_pCurPOpp) return; // failsafe, should not occur

    bool bFound(false);

    // select the operating point closest to the one which has been deleted
    for(int iPOpp=io; iPOpp<Objects3d::nPOpps(); iPOpp++)
    {
        PlaneOpp *pPOpp = Objects3d::POppAt(iPOpp);
        if(pPOpp)
        {
            if(pPOpp->polarName().compare(m_pCurPlPolar->name())==0 && pPOpp->planeName().compare(m_pCurPlane->name())==0)
            {
                setPlaneOpp(pPOpp);
                bFound = true;
            }
        }
        if(bFound) break;
    }

    if(!bFound)
    {
        m_pCurPOpp = nullptr;
        setPlaneOpp(nullptr);
    }

    if(m_pCurPOpp) m_pPlaneExplorer->selectPlaneOpp(m_pCurPOpp);
    else           m_pPlaneExplorer->selectPlPolar(m_pCurPlPolar, true);
    emit projectModified();

    m_bResetCurves = true;
    updateView();
}


/**
* The user has requested a deletion of all operating points associated to the wing or plane
*/
void XPlane::onDeletePlanePOpps()
{
    if(!m_pCurPlane) return;

    for(int iw=0; iw<Objects3d::nPolars(); iw++)
    {
        PlanePolar const *pWPolar = Objects3d::plPolarAt(iw);
        if(pWPolar->planeName()==m_pCurPlane->name())
            m_pPlaneExplorer->removeWPolarPOpps(pWPolar);
    }

    for (int i=Objects3d::nPOpps()-1; i>=0; i--)
    {
        PlaneOpp *pPOpp = Objects3d::POppAt(i);
        if (pPOpp->planeName()==m_pCurPlane->name())
        {
            Objects3d::removePOppAt(i);
            delete pPOpp;
        }
    }
    m_pCurPOpp = nullptr;

    emit projectModified();

    setControls();
    m_bResetCurves = true;
    updateView();
}


void XPlane::onDeletePlaneWPolars()
{
    if(!m_pCurPlane) return;

    QString PlaneName, strong;

    PlaneName = QString::fromStdString(m_pCurPlane->name());

    strong = tr("Are you sure you want to delete the polars associated to :\n") +  PlaneName +"?\n";
    if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, tr("Question"), strong, QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
        return;

    if(m_pCurPlane)
    {
        m_pPlaneExplorer->removeWPolars(m_pCurPlane);
    }

    Objects3d::deletePlaneResults(m_pCurPlane, true);

    setPolar(nullptr);
    m_pPlaneExplorer->selectPlane(m_pCurPlane);

    emit projectModified();
    setControls();
    updateView();
}


void XPlane::onDeleteCurPlPolar()
{
    if(!m_pCurPlPolar) return;
    if(m_pCurPlPolar->isLocked())
    {
        displayMessage("The polar is locked by an analysis and cannot be deleted.\n\n", true, false);
        return;
    }

    QString strong = tr("Are you sure you want to delete the polar:\n") +  QString::fromStdString(m_pCurPlPolar->name()) +"?\n";
    if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, tr("Question"), strong,
                                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel)) return;

    stopAnimate();


    PlanePolar *pWPolarDel = m_pCurPlPolar; // in case of unfortunate signal/slot to setWPolar;
    QString nextWPolarName = m_pPlaneExplorer->removeWPolar(m_pCurPlPolar);
    Objects3d::deletePlPolar(pWPolarDel);

    m_pCurPOpp = nullptr;
    m_pCurPlPolar = nullptr;
    emit projectModified();

    setPolar(nextWPolarName);

    if(m_pCurPlPolar) m_pPlaneExplorer->selectPlPolar(m_pCurPlPolar, false);
    else             m_pPlaneExplorer->selectPlane(m_pCurPlane);

    m_pPlaneExplorer->setObjectProperties();

    setControls();
    updateView();
}


/**
 * The user has requested a duplication of the currently selected wing or plane
 */
void XPlane::onDuplicateCurPlane()
{
    if(!m_pCurPlane) return;
    Plane *pPlane = duplicatePlane(m_pCurPlane);
     if(!pPlane) return;

    m_pPlaneExplorer->insertPlane(pPlane);
    pPlane = setPlane(pPlane);
    m_pPlaneExplorer->selectPlane(pPlane);
    updateView();
    emit projectModified();
}


void XPlane::onEditCurFuse()
{
    m_pgl3dXPlaneView->m_bArcball = false;
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pCurPlane);
    if(!pPlaneXfl->hasFuse()) return;

    if(m_pCurPlane->isLocked())
    {
        displayMessage("The plane is locked by an analysis and cannot be modified.\n\n", true, false);
        return;
    }

    PlaneXfl *pModPlane = new PlaneXfl();

    pModPlane->duplicate(m_pCurPlane);
    bool bGeomChanged = false;
    bool bDescriptionChanged = false;

    int iExitCode = QDialog::Rejected;

    Fuse *pModFuse = pModPlane->fuse(0);

    if(pModFuse->isFlatFaceType() || pModFuse->isSplineType())
    {
        FuseXfl *pModFuseXfl = dynamic_cast<FuseXfl*>(pModFuse);

        QAction *pSenderAction = qobject_cast<QAction *>(sender());
        if (!pSenderAction) return;

        FuseXflDlg *pXflFDFlg = nullptr;
        if     (pSenderAction==m_pActions->m_pEditFuse)       pXflFDFlg = new FuseXflDefDlg(s_pMainFrame);
        else if(pSenderAction==m_pActions->m_pEditFuseObject) pXflFDFlg = new FuseXflObjectDlg(s_pMainFrame);
        if(!pXflFDFlg) return;
        pXflFDFlg->initDialog(pModFuseXfl);
        iExitCode = pXflFDFlg->exec();
        if(iExitCode==QDialog::Rejected) return;

        bGeomChanged = pXflFDFlg->geometryChanged();
        bDescriptionChanged = pXflFDFlg->bDescriptionChanged();

        delete pXflFDFlg;
    }
    else if(pModFuse->isSectionType())
    {
        FuseSections *pModFuseXfl = dynamic_cast<FuseSections*>(pModFuse);

        FuseXflDlg *pXflFDFlg = nullptr;
        pXflFDFlg = new FuseXflDefDlg(s_pMainFrame);
        pXflFDFlg->initDialog(pModFuseXfl);
        iExitCode = pXflFDFlg->exec();
        if(iExitCode==QDialog::Rejected) return;

        bGeomChanged = pXflFDFlg->geometryChanged();
        bDescriptionChanged = pXflFDFlg->bDescriptionChanged();

        delete pXflFDFlg;
    }
    else if(pModFuse->isOccType())
    {
        FuseOcc *pModBodyOcc = dynamic_cast<FuseOcc*>(pModFuse);
        FuseOccDlg BOccDlg(s_pMainFrame);
        BOccDlg.initDialog(pModBodyOcc);

        iExitCode = BOccDlg.exec();
        if(iExitCode==QDialog::Rejected) return;
        bGeomChanged = BOccDlg.geometryChanged();
        bDescriptionChanged = BOccDlg.bDescriptionChanged();
    }
    else if(pModFuse->isStlType())
    {
        FuseStl *pModBodyStl = dynamic_cast<FuseStl*>(pModFuse);
        FuseStlDlg sbDlg(s_pMainFrame);
        sbDlg.initDialog(pModBodyStl);

        iExitCode = sbDlg.exec();
        if(iExitCode==QDialog::Rejected) return;
        bGeomChanged = sbDlg.geometryChanged();
        bDescriptionChanged = sbDlg.bDescriptionChanged();
    }

    if(bDescriptionChanged)
    {
        emit projectModified();
        Fuse *pFuse = pPlaneXfl->fuse(0);
        pFuse->setName(pModFuse->name());
        pFuse->setDescription(pModFuse->description());
        pFuse->setColor(pModFuse->color());
        pFuse->setOccTessParams(pModFuse->occTessParams());
        pFuse->setGmshTessParams(pModFuse->gmshTessParams());
        QString strange;
//        pFuse->makeShellTriangulation(strange, QString());
        gmesh::makeFuseTriangulation(pFuse, strange);
        m_pgl3dXPlaneView->resetglGeom();
        updateView();
    }

    if(!bGeomChanged && iExitCode==QDialog::Accepted)
    {
        updateView();
        delete pModPlane;
        return;
    }

    setModPlane(pModPlane, Objects3d::hasResults(m_pCurPlane), iExitCode==10);
    updateView();
}


void XPlane::onFuseTriMesh()
{
    if(!m_pCurPlane || !m_pCurPlane->hasFuse()) return;
    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);

    m_pgl3dXPlaneView->m_bArcball = false;

    if(m_pCurPlane->isLocked())
    {
        displayMessage("The plane is locked by an analysis and cannot be modified.\n\n", true, false);
        return;
    }

    PlaneXfl *pModPlane = new PlaneXfl();
    pModPlane->duplicate(pPlaneXfl);

    Fuse *pFuse = pModPlane->fuse(0);
    if(!pFuse) return;

    FuseMesherDlg *pFMDlg = new FuseMesherDlg(s_pMainFrame);
    pFMDlg->initDialog(pFuse);
    if(pFMDlg->exec()!=QDialog::Accepted)
    {
        delete pModPlane;
        return;
    }

    pModPlane->makeTriMesh(true);

    setModPlane(pModPlane, Objects3d::hasResults(pPlaneXfl), false);

    updateView();
}


void XPlane::onResetFuseMesh()
{
    m_pgl3dXPlaneView->m_bArcball = false;
    if(!m_pCurPlane || !m_pCurPlane->hasFuse() || !m_pCurPlane->isXflType()) return;
    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);

    if(m_pCurPlane->isLocked())
    {
        displayMessage("The plane is locked by an analysis and cannot be modified.\n\n", true, false);
        return;
    }

    PlaneXfl *pModPlane = new PlaneXfl();
    pModPlane->duplicate(pPlaneXfl);

    Fuse *pFuse = pModPlane->fuse(0);
    if(!pFuse) return;
    std::string logmsg;
    pFuse->makeDefaultTriMesh(logmsg, "");
    pModPlane->makeTriMesh(true);

    setModPlane(pModPlane, Objects3d::hasResults(pPlaneXfl), false);
    updateView();
}


void XPlane::onScaleFuse()
{
    m_pgl3dXPlaneView->m_bArcball = false;
    if(!m_pCurPlane || !m_pCurPlane->isXflType() || !m_pCurPlane->hasFuse()) return;
    if(m_pCurPlane->isLocked())
    {
        displayMessage("The plane is locked by an analysis and cannot be modified.\n\n", true, false);
        return;
    }

    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);

    PlaneXfl *pModPlane = new PlaneXfl();
    pModPlane->duplicate(pPlaneXfl);

    Fuse *pFuse = pModPlane->fuse(0);

    DoubleValueDlg dlg(s_pMainFrame, {1.0}, {"Scale factor"}, {QString()});
    if(dlg.exec()==QDialog::Accepted)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        pFuse->scale(dlg.value(0), dlg.value(0), dlg.value(0));

//        pModPlane->makePlane();
        QApplication::restoreOverrideCursor();
    }
    else
    {
        m_pgl3dXPlaneView->resetglGeom();
        updateView();
        return;
    }

    setModPlane(pModPlane, Objects3d::hasResults(m_pCurPlane), false);

    updateView();
}


/** ask and process the user's intention for this new modified plane */
Plane* XPlane::setModPlane(Plane *pModPlane, bool bUsed, bool bAsNew)
{
    emit projectModified();

    ModDlg mdDlg(s_pMainFrame);

    if(bAsNew)
    {
        //The user wants to save mods to a new plane object
        Plane *pPlane = Objects3d::setModifiedPlane(pModPlane);
        if(pPlane)
        {
            pPlane->setInitialized(false);
            setPlane(pPlane);
            if(pPlane->isXflType() && pPlane->hasFuse())
            {
                PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(pPlane);
                pPlaneXfl->fuse(0)->makeFuseGeometry();
            }
            m_pPlaneExplorer->insertPlane(pPlane);
            m_pPlaneExplorer->selectPlane(pPlane);
        }
        setControls();

        m_pgl3dXPlaneView->resetglGeom();
        m_pgl3dXPlaneView->resetglMesh();
        m_pgl3dXPlaneView->s_bResetglWake = true;
        m_bResetCurves = true;

        return pPlane;
    }
    else if(bUsed)
    {
        mdDlg.setQuestion("The modification will erase all results associated to this plane\n"
                              "Continue?");
        mdDlg.initDialog();

        int Ans = mdDlg.exec();

        if (Ans == QDialog::Rejected)
        {
            //restore geometry
            delete pModPlane; // clean up
            return nullptr;
        }
        else if(Ans==20)
        {
            //save mods to a new plane object
            Plane *pPlane = Objects3d::setModifiedPlane(pModPlane);
            if(pPlane)
            {
                pPlane->setInitialized(false);
                setPlane(pPlane);
                if(pPlane->isXflType() && pPlane->hasFuse())
                {
                    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(pPlane);
                    pPlaneXfl->fuse(0)->makeFuseGeometry();
                }

                m_pPlaneExplorer->insertPlane(pPlane);
                m_pPlaneExplorer->updatePlane(pPlane);
                m_pPlaneExplorer->selectPlane(pPlane);
            }
            setControls();

            m_pgl3dXPlaneView->resetglGeom();
            m_pgl3dXPlaneView->resetglMesh();
            m_bResetCurves = true;

            return pPlane;
        }
    }

    // either not used, or the user has decided to go ahead and overwrite
    Objects3d::deletePlaneResults(m_pCurPlane, false); // delete polar data and operating points
    Objects3d::deletePlane(m_pCurPlane, false); // delete the plane, but keep the polars;
    m_pCurPlane = nullptr;
    Objects3d::addPlane(pModPlane); // add the plane to the array - polars are attached implicitely through plane name

    if(pModPlane->isXflType())
        pModPlane->setInitialized(false); // mark the plane for rebuild

    m_pCurPOpp = nullptr; // all the plane's oppoints have been deleted

    setPlane(pModPlane);
    setPolar(m_pCurPlPolar);

    m_pPlaneExplorer->updatePlane(m_pCurPlane);
    m_pPlaneExplorer->selectObjects();
    setControls();

    m_bResetCurves = true;

    return m_pCurPlane;
}


void XPlane::onEditCurPlane()
{
    m_pgl3dXPlaneView->m_bArcball = false;
    if(!m_pCurPlane) return;
    if(m_pCurPlane->isLocked())
    {
        displayMessage("The plane is locked by an analysis and cannot be modified.\n\n", false, false);
        return;
    }

    int iExitCode = 0;
    bool bChanged = false;
    bool bDescriptionChanged = false;
    Plane* pModPlane = nullptr;
    if(m_pCurPlane->isXflType())
    {
        pModPlane = new PlaneXfl;
        PlaneXfl *pModPlaneXfl = dynamic_cast<PlaneXfl*>(pModPlane);
        pModPlaneXfl->duplicate(m_pCurPlane);
        pModPlaneXfl->setInitialized(false);
        pModPlaneXfl->restoreMesh();

        PlaneXflDlg pDlg(s_pMainFrame);
        pDlg.initDialog(pModPlaneXfl, false);
        iExitCode = pDlg.exec();
        bChanged = pDlg.bChanged();
        bDescriptionChanged = pDlg.bDescriptionChanged();
    }
    else if(m_pCurPlane->isSTLType())
    {
        pModPlane = new PlaneSTL;
        PlaneSTL *pModPlaneSTL = dynamic_cast<PlaneSTL*>(pModPlane);
        pModPlaneSTL->duplicate(m_pCurPlane);
        pModPlaneSTL->restoreMesh();

        PlaneSTLDlg pDlg(s_pMainFrame);
        pDlg.initDialog(pModPlaneSTL, false);
        iExitCode = pDlg.exec();
        bChanged = pDlg.bChanged();
        bDescriptionChanged = pDlg.bDescriptionChanged();
    }
    else return;

    if(iExitCode==QDialog::Rejected)
    {
        delete pModPlane;
        return;
    }

    if(bDescriptionChanged)
    {
        m_pCurPlane->copyMetaData(pModPlane);
    }
    emit projectModified();
    m_pPlaneExplorer->setObjectProperties();

    if(iExitCode==QDialog::Accepted && !bChanged)
    {
        m_pgl3dXPlaneView->updatePartFrame(m_pCurPlane);
        updateView();
        return;
    }

    setModPlane(pModPlane, Objects3d::hasResults(m_pCurPlane), iExitCode==10);

    m_pgl3dXPlaneView->setPlaneReferenceLength(m_pCurPlane);
    if(m_pXPlaneWt) m_pXPlaneWt->gl3dFloatView()->setPlaneReferenceLength(m_pCurPlane);
    updateView();
}


void XPlane::onEditCurPlaneDescription()
{
    if(!m_pCurPlane) return;

    TextDlg dlg(QString::fromStdString(m_pCurPlane->description()), s_pMainFrame);

    if(dlg.exec() != QDialog::Accepted) return;
    m_pCurPlane->setDescription(dlg.newText().toStdString());
    m_pPlaneExplorer->setObjectProperties();
}


void XPlane::onTranslatePlane()
{
    m_pgl3dXPlaneView->m_bArcball = false;
    if(!m_pCurPlane) return;
    if(m_pCurPlane->isLocked())
    {
        displayMessage("The plane is locked by an analysis and cannot be modified.\n\n", true, false);
        return;
    }


    TranslateDlg dlg(s_pMainFrame);
    if(dlg.exec()!=QDialog::Accepted)
    {
        return;
    }
    m_pCurPlane->translate(dlg.translationVector());

    m_pCurPlane->setInitialized(false);
    setPlane();
    if(m_pCurPlPolar) setPolar();

    emit projectModified();
    m_pPlaneExplorer->setObjectProperties();

//    setModPlane(pModPlane, Objects3d::hasResults(m_pCurPlane), true);

    m_pgl3dXPlaneView->setPlaneReferenceLength(m_pCurPlane);
    if(m_pXPlaneWt) m_pXPlaneWt->gl3dFloatView()->setPlaneReferenceLength(m_pCurPlane);
    updateView();
}


void XPlane::onScalePlane()
{
    m_pgl3dXPlaneView->m_bArcball = false;
    if(!m_pCurPlane) return;
    if(m_pCurPlane->isLocked())
    {
        displayMessage("The plane is locked by an analysis and cannot be modified.\n\n", true, false);
        return;
    }

    Plane* pModPlane = nullptr;
    if(m_pCurPlane->isXflType())
    {
        pModPlane = new PlaneXfl;
        PlaneXfl *pModPlaneXfl = dynamic_cast<PlaneXfl*>(pModPlane);
        if(pModPlaneXfl)
        {
            pModPlaneXfl->duplicate(m_pCurPlane);
            pModPlaneXfl->setInitialized(false);
        }
    }
    else if(m_pCurPlane->isSTLType())
    {
        pModPlane = new PlaneSTL;
        PlaneSTL *pModPlaneSTL = dynamic_cast<PlaneSTL*>(pModPlane);
        if(pModPlaneSTL)
        {
            pModPlaneSTL->duplicate(m_pCurPlane);
            pModPlaneSTL->restoreMesh();
        }
    }
    else return;

    QStringList labels("Scale factor:");
    QStringList rightlabels("");
    QVector<double> vals({1});
    DoubleValueDlg dlg(s_pMainFrame, vals, labels, rightlabels);
    if(dlg.exec()!=QDialog::Accepted)
    {
        delete pModPlane;
        return;
    }
    pModPlane->scale(dlg.value(0));

    emit projectModified();
    m_pPlaneExplorer->setObjectProperties();

    setModPlane(pModPlane, Objects3d::hasResults(m_pCurPlane), true);

    m_pgl3dXPlaneView->setPlaneReferenceLength(m_pCurPlane);
    if(m_pXPlaneWt) m_pXPlaneWt->gl3dFloatView()->setPlaneReferenceLength(m_pCurPlane);
    updateView();
}


void XPlane::onEditCurWing()
{
    m_pgl3dXPlaneView->m_bArcball = false;
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    if(m_pCurPlane->isLocked())
    {
        displayMessage("The plane is locked by an analysis and cannot be modified.\n\n", true, false);
        return;
    }

    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pCurPlane);

    PlaneXfl *pModPlane = new PlaneXfl;
    pModPlane->duplicate(m_pCurPlane);

    WingXfl *pWing = nullptr;
    WingXfl *pModWing = nullptr;

    QString WingType;
    switch(pSenderAction->data().toInt())
    {
        case xfl::Main:
            pWing = pPlaneXfl->mainWing();
            pModWing = pModPlane->mainWing();
            WingType = "MAINWING";
            break;
        case xfl::Elevator:
            pWing = pPlaneXfl->stab();
            pModWing = pModPlane->stab();
            WingType = "ELEVATOR";
            break;
        case xfl::Fin:
            pWing = pPlaneXfl->fin();
            pModWing = pModPlane->fin();
            WingType = "FIN";
            break;
    }

    if(!pWing)
    {
        QString strange = "No wing of type "+WingType+" found.\n\n";
        displayMessage(strange, true, false);
        return;
    }

    WingDlg *pWngDlg = nullptr;
/*    if (pSenderAction==m_pActions->m_pEditWingObject || pSenderAction==m_pActions->m_pEditStabObject || pSenderAction==m_pActions->m_pEditFinObject)
    {
        pWngDlg = new WingObjectDlg(s_pMainFrame);
    }
    else*/
    if(pSenderAction==m_pActions->m_pEditWingDef || pSenderAction==m_pActions->m_pEditStabDef || pSenderAction==m_pActions->m_pEditFinDef)
    {
        pWngDlg = new WingDefDlg(s_pMainFrame);
    }
    if(!pWngDlg)
    {
        delete pModPlane;
        return;
    }

    pWngDlg->setPlaneName(QString::fromStdString(pPlaneXfl->name()));
    pWngDlg->initDialog(pModWing);

    int iExitCode = pWngDlg->exec();
    if(iExitCode==QDialog::Rejected)
    {
        delete pWngDlg;
        delete pModPlane;
        return;
    }

    if(pWngDlg->bDescriptionChanged())
    {
        emit projectModified();
        pWing->setColor(pModWing->color());
        pWing->setName(pModWing->name());
        pWing->setDescription(pModWing->description());
    }
    emit projectModified();

    if(!pWngDlg->bChanged() && iExitCode==QDialog::Accepted)
    {
        updateView();
        delete pWngDlg;
        delete pModPlane;
        return;
    }
    delete pWngDlg;

    setModPlane(pModPlane, Objects3d::hasResults(m_pCurPlane), iExitCode==10);
    updateView();
}


void XPlane::onWingProps()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    PlaneXfl  *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pCurPlane);

    WingXfl *pWing;
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;
    switch(pSenderAction->data().toInt())
    {
        case xfl::Main:
            pWing = pPlaneXfl->mainWing();
            break;
        case xfl::Elevator:
            pWing = pPlaneXfl->stab();
            break;
        case xfl::Fin:
            pWing = pPlaneXfl->fin();
            break;
        default:
            return;
    }
    if(!pWing) return;

    std::string props;
    pWing->getProperties(props, "   ");

    std::string strange;
    strange = "Properties of wing " + pWing->name() + EOLstr;
    displayStdMessage(strange+props+"\n\n", true, false);
}


void XPlane::onFuseProps()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType() || !m_pCurPlane->hasFuse()) return;
    std::string props;
    PlaneXfl * pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pCurPlane);

    pPlaneXfl->fuse(0)->getProperties(props, "   ");

    std::string strange = "Properties of fuse " + pPlaneXfl->fuse(0)->name() + EOLstr;
    displayStdMessage(strange+props+"\n\n", true, false);
}


void XPlane::onTranslateWing()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pCurPlane);

    WingXfl *pWing=nullptr;
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;
    switch(pSenderAction->data().toInt())
    {
        case xfl::Main:
            pWing = pPlaneXfl->mainWing();
            break;
        case xfl::Elevator:
            pWing = pPlaneXfl->stab();
            break;
        case xfl::Fin:
            pWing = pPlaneXfl->fin();
            break;
        default:
            return;
    }
    if(!pWing) return;
    int iWing = -1;
    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        if(pPlaneXfl->wing(iw)==pWing)
        {
            iWing = iw;
            break;
        }
    }
    if(iWing<0) return;

    TranslateDlg dlg(s_pMainFrame);
    if(dlg.exec()!=QDialog::Accepted) return;

    PlaneXfl *pModPlane  = new PlaneXfl;
    pModPlane->duplicate(m_pCurPlane);
    pModPlane->setWingLE(iWing, pModPlane->wingLE(iWing)+dlg.translationVector());

    setModPlane(pModPlane, Objects3d::hasResults(m_pCurPlane), false);
    updateView();
}


void XPlane::onTranslateFuse()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pCurPlane);

    Fuse *pFuse = pPlaneXfl->fuse(0);
    if(!pFuse) return;

    TranslateDlg dlg(s_pMainFrame);
    if(dlg.exec()!=QDialog::Accepted) return;

    PlaneXfl *pModPlane  = new PlaneXfl;
    pModPlane->duplicate(m_pCurPlane);
    pModPlane->setFusePos(0, pModPlane->fusePos(0)+dlg.translationVector());

    setModPlane(pModPlane, Objects3d::hasResults(m_pCurPlane), false);
    updateView();
}


void XPlane::onScaleWing()
{
    if(!m_pCurPlane) return;

    PlaneXfl* pModPlane= new PlaneXfl;
    pModPlane->duplicate(m_pCurPlane);

    WingXfl *pModWing=nullptr;
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;
    QString WingType;
    switch(pSenderAction->data().toInt())
    {
        case xfl::Main:
            pModWing = pModPlane->mainWing();
            WingType = "MAINWING";
            break;
        case xfl::Elevator:
            pModWing = pModPlane->stab();
            WingType = "ELEVATOR";
            break;
        case xfl::Fin:
            pModWing = pModPlane->fin();
            WingType = "FIN";
            break;
    }

    if(!pModWing) return; // something went wrong

    WingScaleDlg wsDlg(s_pMainFrame);
    wsDlg.initDialog(pModWing->planformSpan(),
                     pModWing->rootChord(),
                     pModWing->averageSweep(),
                     pModWing->tipTwist(),
                     pModWing->planformArea(),
                     pModWing->aspectRatio(),
                     pModWing->taperRatio());

    if(QDialog::Accepted != wsDlg.exec())
        return;

    if (wsDlg.m_bSpan || wsDlg.m_bChord || wsDlg.m_bSweep || wsDlg.m_bTwist || wsDlg.m_bArea || wsDlg.m_bAR || wsDlg.m_bTR)
    {
        if(wsDlg.m_bSpan)  pModWing->scaleSpan(wsDlg.m_NewSpan);
        if(wsDlg.m_bChord) pModWing->scaleChord(wsDlg.m_NewChord);
        if(wsDlg.m_bSweep) pModWing->scaleSweep(wsDlg.m_NewSweep);
        if(wsDlg.m_bTwist) pModWing->scaleTwist(wsDlg.m_NewTwist);
        if(wsDlg.m_bArea)  pModWing->scaleArea(wsDlg.m_NewArea);
        if(wsDlg.m_bAR)    pModWing->scaleAR(wsDlg.m_NewAR);
        if(wsDlg.m_bTR)    pModWing->scaleTR(wsDlg.m_NewTR);
    }
    else return;

    setModPlane(pModPlane, Objects3d::hasResults(m_pCurPlane), false);
    updateView();
}


/**
 * Copies the data from the active operating point to the clipboard
 */
void XPlane::onCopyCurPOppData()
{
    if (!m_pCurPOpp) return;

    QClipboard *pClipBoard = QApplication::clipboard();
    QString strange, strong;
    exportMainDataToString(m_pCurPOpp, m_pCurPlane, strange, SaveOptions::exportFileType(), SaveOptions::textSeparator());
    if(m_pCurPOpp->isQuadMethod())
        exportPanel4DataToString(m_pCurPOpp,  m_pCurPlane, m_pCurPlPolar, SaveOptions::exportFileType(), strong);
    else if(m_pCurPOpp->isTriangleMethod())
        exportPanel3DataToString(m_pCurPOpp, m_pCurPlane, m_pCurPlPolar, SaveOptions::exportFileType(), SaveOptions::textSeparator(), strong);

    pClipBoard->setText(strange +  strong);
    displayMessage("The data of the operating point has been copied to the clipboard", false, true);
}


/**
 * Exports the data from the active WOpp to the text file
 */
void XPlane::onExportCurPOpp()
{
    if(!m_pCurPOpp)return ;// is there anything to export?

    QString filter;
    if(SaveOptions::exportFileType()==xfl::TXT) filter = "Text File (*.txt)";
    else                                        filter = "Comma Separated Values (*.csv)";

    QString FileName, sep, str, strong, strange;

    strong = QString("a=%1_v=%2").arg(m_pCurPOpp->alpha(), 5,'f',2).arg(m_pCurPOpp->QInf()*Units::mstoUnit(),6,'f',2);
    str = Units::speedUnitQLabel();
    strong = QString::fromStdString(m_pCurPOpp->planeName()) + "_" + strong + str;

    strong.replace(" ","");
    strong.replace("/", "");
    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export operating point",
                                            SaveOptions::lastDirName() +'/'+strong,
                                            "Text File (*.txt);;Comma Separated Values (*.csv)",
                                            &filter);

    if(!FileName.length()) return;
    int pos = FileName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastDirName(FileName.left(pos));
    pos = FileName.lastIndexOf(".csv");
    if (pos>0) SaveOptions::setExportFileType(xfl::CSV);
    else       SaveOptions::setExportFileType(xfl::TXT);

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&XFile);

    sep = SaveOptions::textSeparator();

    exportMainDataToString(m_pCurPOpp, m_pCurPlane, strange, SaveOptions::exportFileType(), SaveOptions::textSeparator());
    out <<strange;
    strong.clear();
    if(m_pCurPOpp->isQuadMethod())
        exportPanel4DataToString(m_pCurPOpp, m_pCurPlane, m_pCurPlPolar, SaveOptions::exportFileType(), strong);
    else if(m_pCurPOpp->isTriangleMethod())
        exportPanel3DataToString(m_pCurPOpp, m_pCurPlane, m_pCurPlPolar, SaveOptions::exportFileType(), SaveOptions::textSeparator(), strong);
    out << strong;

    out << ("\n\n");

    XFile.close();
}


void XPlane::onExportWPolarToFile()
{
    if (!m_pCurPlPolar) return;

    QString FileName, filter;

    if(SaveOptions::exportFileType()==xfl::TXT) filter = "Text File (*.txt)";
    else                                        filter = "Comma Separated Values (*.csv)";

    FileName = QString::fromStdString(m_pCurPlPolar->name());
    FileName.replace("/", "_");
    FileName.replace(".", "_");
    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export Polar",
                                            SaveOptions::lastDirName() + "/"+FileName,
                                            "Text File (*.txt);;Comma Separated Values (*.csv)",
                                            &filter);

    if(!FileName.length()) return;

    if(filter.indexOf("*.txt")>0)
    {
        SaveOptions::setExportFileType(xfl::TXT);
        if(FileName.indexOf(".txt")<0) FileName +=".txt";
    }
    else if(filter.indexOf("*.csv")>0)
    {
        SaveOptions::setExportFileType(xfl::CSV);
        if(FileName.indexOf(".csv")<0) FileName +=".csv";
    }

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    std::string polardata;
    std::string sep = "  ";
    if(SaveOptions::exportFileType()==xfl::CSV) sep = SaveOptions::textSeparator().toStdString()+ " ";
    polardata = m_pCurPlPolar->exportToString(sep);

    QTextStream out(&XFile);
    out << QString::fromStdString(polardata);

    XFile.close();

    updateView();
}


QString XPlane::onExportPlPolarToClipboard()
{
    if(!m_pCurPlPolar) return QString();
    std::string polardata;
    std::string sep = "  ";
    if(SaveOptions::exportFileType()==xfl::CSV) sep = SaveOptions::textSeparator().toStdString()+ " ";
    polardata = m_pCurPlPolar->exportToString(sep);
    QClipboard *pClipBoard = QApplication::clipboard();
    pClipBoard->setText(QString::fromStdString(polardata));

    return QString::fromStdString(polardata);
}


void XPlane::onAutoWPolarNameOptions()
{
    WPolarAutoNameDlg dlg;
    dlg.initDialog(PlanePolarDlg::staticWPolar());
    dlg.exec();
}


void XPlane::onExportAllPlPolars()
{
    QString filename, DirName, polarname;
    QFile XFile;
    QTextStream out;

    //select the directory for output
    DirName = QFileDialog::getExistingDirectory(s_pMainFrame,  "Export directory", SaveOptions::lastDirName());

    for(int l=0; l<Objects3d::nPolars(); l++)
    {
        PlanePolar *pWPolar = Objects3d::plPolarAt(l);
        polarname = QString::fromStdString(pWPolar->planeName() + "_" + pWPolar->name());
        polarname.replace("/", "_");
        polarname.replace(".", "_");
        filename = polarname;
        filename = DirName + "/" +filename;
        if(SaveOptions::exportFileType()==xfl::TXT) filename += ".txt";
        else                                          filename += ".csv";

        XFile.setFileName(filename);
        if (XFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            out.setDevice(&XFile);

            std::string sep = "  ";
            if(SaveOptions::exportFileType()==xfl::CSV) sep = SaveOptions::textSeparator().toStdString() + " ";
            std::string exportstr = pWPolar->exportToString(sep);
            out << QString::fromStdString(exportstr);
            XFile.close();
            displayMessage("Exported the polar:" + polarname + "\n", false, true);
        }
        else
        {
            displayMessage("Error exporting the polar:" + polarname + "\n", true, false);
        }
    }
}


void XPlane::onExporttoAVL()
{
    if (!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);

    QString filter =".avl";

    QString FileName, strong;


    FileName = QString::fromStdString(m_pCurPlane->name()+".avl");
    FileName.replace("/", " ");
    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export plane",
                                            SaveOptions::lastDirName() + "/"+FileName,
                                            "AVL Text File (*.avl)", &filter);
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastDirName(FileName.left(pos));

    pos = FileName.indexOf(".avl", Qt::CaseInsensitive);
    if(pos<0) FileName += ".avl";


    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly)) return;

    QTextStream out(&XFile);
    out << "# \n";
    out << "# TODO: check consistency of area unit and length units in this file\n";
    out << "# TODO: check consistency with inertia units of the .mass file\n";
    out << "# TODO: remove duplicate sections in wing definitions\n";
    out << "# \n";
    out << "# \n";

    strong = s_pMainFrame->projectName();
    int len = strong.length();
    if (strong.right(1) == "*") strong = strong.left(len-1);
    if(!strong.length()) out << "Project";
    else out << strong;
    out << "\n";
    out << "0.0                                 | Mach\n";

    if(pPlaneXfl->mainWing()->isSymmetric()) out << ("0     0     0.0                     | iYsym  iZsym  Zsym\n");
    else                                     out << ("0     0     0.0                     | iYsym  iZsym  Zsym\n");

    double area = 0;
    if(m_pCurPlPolar) area = pPlaneXfl->planformArea(m_pCurPlPolar->bIncludeOtherWingAreas());
    else             area = pPlaneXfl->planformArea(true);

    strong = QString::asprintf("%9g   %9g   %9g   | Sref   Cref   Bref\n",
                               area*Units::mtoUnit()*Units::mtoUnit(),
                               pPlaneXfl->mainWing()->MAC()*Units::mtoUnit(),
                               pPlaneXfl->planformSpan()*Units::mtoUnit());
    out << strong;

    if(m_pCurPlane)
        strong = QString::asprintf("%g   %g   %g   | Xref   Yref   Zref\n",
                                   m_pCurPlane->CoG_t().x*Units::mtoUnit(),
                                   m_pCurPlane->CoG_t().y*Units::mtoUnit(),
                                   m_pCurPlane->CoG_t().z*Units::mtoUnit());

    out << strong;

    out << " 0.00                               | CDp  (optional)\n";

    out << EOLch << EOLch<< EOLch;


    int index = QRandomGenerator::global()->bounded(100);

    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = pPlaneXfl->wingAt(iw);

        WingXfl wing;
        wing.duplicate(pWing);
        wing.createSurfaces(Vector3d(), wing.rx(), 0.0);

        std::string avlstring;
        wing.exportToAVL(avlstring, index+iw, wing.position(), wing.ry(), Units::mtoUnit());

        out << QString::fromStdString(avlstring);
    }
    XFile.close();
}


void XPlane::onShowAllPlPolars()
{
    if(isPolarView() || isStabPolarView())
    {
        for (int i=0; i<Objects3d::nPolars(); i++)
        {
            PlanePolar *pWPolar = Objects3d::plPolarAt(i);
            pWPolar->setVisible(true);
        }
        emit projectModified();
    }

    m_pPlaneExplorer->updateVisibilityBoxes();
    m_pPlaneExplorer->update();
    m_bResetCurves = true;
    updateView();
}


void XPlane::onHideAllPlPolars()
{
    if(isPolarView() || isStabPolarView())
    {
        for (int i=0; i<Objects3d::nPolars(); i++)
        {
            PlanePolar *pWPolar = Objects3d::plPolarAt(i);
            pWPolar->setVisible(false);
        }
        emit projectModified();
    }

    m_pPlaneExplorer->updateVisibilityBoxes();
    m_pPlaneExplorer->update();
    m_bResetCurves = true;
    updateView();
}


void XPlane::onHideAllWPlrOpps()
{
    m_bCurPOppOnly = false;

    if(m_pCurPlane)
    {
        for (int i=0; i< Objects3d::nPOpps(); i++)
        {
            PlaneOpp *pPOpp = Objects3d::POppAt(i);
            if (pPOpp->planeName() == m_pCurPlPolar->planeName() &&
                    pPOpp->polarName() == m_pCurPlPolar->name())
            {
                pPOpp->setVisible(false);
            }
        }
    }
    emit projectModified();
    m_pPlaneExplorer->updateVisibilityBoxes();

    m_bResetCurves = true;
    updateView();
}


void XPlane::onHideAllPOpps()
{
    m_bCurPOppOnly = false;

    for (int i=0; i< Objects3d::nPOpps(); i++)
    {
        PlaneOpp *pPOpp = Objects3d::POppAt(i);
        pPOpp->setVisible(false);
    }
    emit projectModified();
    m_pPlaneExplorer->updateVisibilityBoxes();

    m_bResetCurves = true;
    updateView();
}


void XPlane::onHidePlaneOpps()
{
    for (int i=0; i< Objects3d::nPOpps(); i++)
    {
        PlaneOpp *pPOpp = Objects3d::POppAt(i);
        if (pPOpp->planeName() == m_pCurPlPolar->planeName())
        {
            pPOpp->setVisible(false);
        }
    }

    emit projectModified();
    m_pPlaneExplorer->updateVisibilityBoxes();
    m_bResetCurves = true;
    updateView();
}


void XPlane::onHidePlaneWPolars()
{
    if(!m_pCurPlane) return;

    QString PlaneName;
    if(m_pCurPlane)     PlaneName = QString::fromStdString(m_pCurPlane->name());
    else return;

    PlanePolar *pWPolar;
    for (int i=0; i<Objects3d::nPolars(); i++)
    {
        pWPolar = Objects3d::plPolarAt(i);
        if (pWPolar->planeName() == PlaneName.toStdString())
        {
            pWPolar->setVisible(false);
            if(pWPolar->isStabilityPolar()) pWPolar->setPointStyle(Line::NOSYMBOL);
        }
    }

    m_pPlaneExplorer->updateVisibilityBoxes();
    emit projectModified();
    m_bResetCurves = true;
    updateView();
}


void XPlane::onNewPlane()
{
    PlaneXfl* pPlane = new PlaneXfl(true);
    pPlane->setLineWidth(Curve::defaultLineWidth());

    pPlane->makePlane(true, false, true);

    PlaneXflDlg pDDlg(s_pMainFrame);

    pDDlg.initDialog(pPlane, true);

    int iExitCode = pDDlg.exec();

    if(iExitCode==QDialog::Accepted)
    {
        if(Objects3d::plane(pPlane->name()))
            m_pCurPlane = Objects3d::setModifiedPlane(pPlane);
        else
        {
            Objects3d::addPlane(pPlane);
            m_pCurPlane = pPlane;
        }
    }
    else if(iExitCode==10)
    {
        m_pCurPlane = Objects3d::setModifiedPlane(pPlane);
    }
    else
    {
        delete pPlane;
        return;
    }

    emit projectModified();

    if(m_pCurPlane)
        m_pCurPlane->setInitialized(false);

    m_pPlaneExplorer->insertPlane(pPlane);
    m_pPlaneExplorer->update();
    m_pPlaneExplorer->selectPlane(pPlane);

    m_pCurPlPolar = nullptr;
    m_pCurPOpp = nullptr;
    setPlane();

    m_pgl3dXPlaneView->resetglMesh();
    setControls();
    updateView();
}


void XPlane::onImportSTLPlane()
{
    StlReaderDlg dlg(s_pMainFrame);
    if(dlg.exec()==QDialog::Rejected) return;

    if(dlg.triangleList().size()==0)
    {
        return; // nothing imported
    }

    PlaneSTL *pPlaneSTL = new PlaneSTL;
    pPlaneSTL->setBaseTriangles(dlg.triangleList());
    pPlaneSTL->setInitialized(false);

    PlaneSTLDlg pDDlg(s_pMainFrame);
    pDDlg.initDialog(pPlaneSTL, true);

    int iExitCode = pDDlg.exec();

    if(iExitCode==QDialog::Accepted)
    {
        if(Objects3d::plane(pPlaneSTL->name()))
            m_pCurPlane =Objects3d::setModifiedPlane(pPlaneSTL);
        else
        {
            Objects3d::addPlane(pPlaneSTL);
            m_pCurPlane = pPlaneSTL;
        }
    }
    else if(iExitCode==10)
    {
        m_pCurPlane =Objects3d::setModifiedPlane(pPlaneSTL);
    }
    else
    {
        delete pPlaneSTL;
        return;
    }

    emit projectModified();

    m_pgl3dXPlaneView->resetglMesh();

    setPlane();
    setControls();

    m_pPlaneExplorer->insertPlane(pPlaneSTL);
    m_pPlaneExplorer->selectPlane(pPlaneSTL);

    setControls();
    updateView();
}


void XPlane::onEditExtraDrag()
{
    if(!m_pCurPlane || !m_pCurPlPolar || m_pCurPlPolar->isExternalPolar()) return;
    if(m_pCurPlPolar->isLocked())
    {
        displayMessage("The polar object is locked by an analysis and cannot be modified.\n\n", true, false);
        return;
    }

    ExtraDragDlg dlg(s_pMainFrame);
    dlg.initDialog(m_pCurPlPolar);
    if(dlg.exec()==QDialog::Accepted)
    {
        dlg.setExtraDragData(m_pCurPlPolar);
        m_pCurPlPolar->recalcExtraDrag();
        m_bResetCurves = true;
        updateView();
        emit projectModified();
    }
}


void XPlane::onEditCurPlPolar()
{
//    stopAnimate();

    if(!m_pCurPlane || !m_pCurPlPolar || m_pCurPlPolar->isExternalPolar()) return;
    if(m_pCurPlPolar->isLocked())
    {
        displayMessage("The polar object is locked by an analysis and cannot be modified.\n\n", true, false);
        return;
    }

    std::string WPolarName;
    int res=-1;

    PlanePolar *pNewWPolar = new PlanePolar;

    if(m_pCurPlPolar->isStabilityPolar())
    {
        T1234578PolarDlg dlg(s_pMainFrame);
        dlg.initPolar3dDlg(m_pCurPlane, m_pCurPlPolar);
        res = dlg.exec();
        pNewWPolar->duplicateSpec(&PlanePolarDlg::staticWPolar());
        WPolarName = PlanePolarDlg::staticWPolar().name();
    }
    else if(m_pCurPlPolar->isControlPolar())
    {
        T6PolarDlg dlg(s_pMainFrame);
        dlg.initPolar3dDlg(m_pCurPlane, m_pCurPlPolar);
        res = dlg.exec();
        pNewWPolar->duplicateSpec(&PlanePolarDlg::staticWPolar());
        WPolarName = PlanePolarDlg::staticWPolar().name();
    }
    else
    {
        T1234578PolarDlg dlg(s_pMainFrame);
        dlg.initPolar3dDlg(m_pCurPlane, m_pCurPlPolar);

        res = dlg.exec();
        pNewWPolar->duplicateSpec(&PlanePolarDlg::staticWPolar());
        WPolarName = PlanePolarDlg::staticWPolar().name();
    }

    if (res == QDialog::Accepted)
    {
        emit projectModified();

        if(m_pCurPlPolar->isLocked())
        {
            displayMessage("The polar object is locked by an analysis and cannot be modified.\n\n", true, false);
            delete pNewWPolar;
            return;
        }

        pNewWPolar->setPlaneName(m_pCurPlane->name());
        pNewWPolar->setName(WPolarName);
        //        pNewWPolar->setLineColor(xfl::randomColor(!DisplayOptions::isLightTheme()));
        pNewWPolar->setVisible(true);

        pNewWPolar = Objects3d::insertNewPolar(pNewWPolar, m_pCurPlane);

        m_pCurPOpp = nullptr;

        m_pgl3dXPlaneView->resetglGeom();
        m_pgl3dXPlaneView->resetglMesh();
        m_pgl3dXPlaneView->resetglPOpp();
        m_pgl3dXPlaneView->s_bResetglWake = true;

        setPolar(pNewWPolar);

        m_pPlaneExplorer->insertWPolar(m_pCurPlPolar);
        m_pPlaneExplorer->selectPlPolar(m_pCurPlPolar, false);
        updateView();
    }
    else
    {
        delete pNewWPolar;
    }
    setControls();
}



void XPlane::onRenameCurWPolar()
{
    if(!m_pCurPlPolar) return;
    if(!m_pCurPlane) return;

    Objects3d::renamePlPolar(m_pCurPlPolar, m_pCurPlane);

    updateTreeView();
    m_pPlaneExplorer->selectPlPolar(m_pCurPlPolar, false);

    emit projectModified();

    m_bResetCurves = true;
    updateView();
}


void XPlane::onRenameCurPlane()
{
    if(!m_pCurPlane) return;
    QString oldName = QString::fromStdString(m_pCurPlane->name());
    Objects3d::renamePlane(QString::fromStdString(m_pCurPlane->name()));

    QString newName = QString::fromStdString(m_pCurPlane->name());
    if(newName.compare(oldName)!=0)
    {
        m_pPlaneExplorer->removePlane(oldName);
        m_pPlaneExplorer->insertPlane(m_pCurPlane);
        m_pPlaneExplorer->selectPlane(m_pCurPlane);
        emit projectModified();
    }
   updateView();
}


void XPlane::onResetCurPlPolar()
{
    if (!m_pCurPlane || !m_pCurPlPolar) return;
    QString strong = tr("Are you sure you want to reset the content of the polar :\n")+ QString::fromStdString(m_pCurPlPolar->name()) +"?\n";
    if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, tr("Question"), strong,
                                                  QMessageBox::Yes|QMessageBox::No,
                                                  QMessageBox::Yes)) return;
    Objects3d::deleteWPolarResults(m_pCurPlPolar);
    m_pCurPOpp = nullptr;

    m_pPlaneExplorer->removeWPolarPOpps(m_pCurPlPolar);
    m_pPlaneExplorer->setObjectProperties();
    emit projectModified();
    m_bResetCurves = true;
    updateView();
}



void XPlane::onShowAllPOpps()
{
    //Switch all WOpps view to on for all Plane and WPolar
    m_bCurPOppOnly = false;
    m_pActions->m_pShowCurWOppOnly->setChecked(false);

    for (int i=0; i< Objects3d::nPOpps(); i++)
    {
        PlaneOpp *pPOpp = Objects3d::POppAt(i);
        pPOpp->setVisible(true);
    }

    emit projectModified();
    m_pPlaneExplorer->updateVisibilityBoxes();

    m_bResetCurves = true;
    updateView();
}


void XPlane::onShowPlaneWPolarsOnly()
{
    if(!m_pCurPlane) return;

    for (int i=0; i<Objects3d::nPolars(); i++)
    {
        PlanePolar *pWPolar = Objects3d::plPolarAt(i);
        pWPolar->setVisible(pWPolar->planeName() == m_pCurPlane->name());
    }

    m_pPlaneExplorer->updateVisibilityBoxes();
    emit projectModified();
    m_bResetCurves = true;
    updateView();
}


void XPlane::onShowOnlyCurPlPolar()
{
    if(!m_pCurPlane || !m_pCurPlPolar) return;

    for (int i=0; i<Objects3d::nPolars(); i++)
    {
        PlanePolar *pWPolar = Objects3d::plPolarAt(i);
        pWPolar->setVisible(pWPolar==m_pCurPlPolar);
    }

    m_pCurPlPolar->setVisible(true);
    m_pPlaneExplorer->updateVisibilityBoxes();
    emit projectModified();
    m_bResetCurves = true;
    updateView();
}


void XPlane::onShowPlaneWPolars()
{
    if(!m_pCurPlane) return;
    int i;
    QString PlaneName;
    if(m_pCurPlane)     PlaneName = QString::fromStdString(m_pCurPlane->name());
    else return;

    PlanePolar *pWPolar;
    for (i=0; i<Objects3d::nPolars(); i++)
    {
        pWPolar = Objects3d::plPolarAt(i);
        if (pWPolar->planeName()==PlaneName.toStdString()) pWPolar->setVisible(true);
    }


    m_pPlaneExplorer->updateVisibilityBoxes();
    emit projectModified();
    m_bResetCurves = true;
    updateView();
}


void XPlane::onShowPlaneOpps()
{
    PlaneOpp *pPOpp;
    int i;
    for (i=0; i< Objects3d::nPOpps(); i++)
    {
        pPOpp = Objects3d::POppAt(i);
        if (pPOpp->planeName() == m_pCurPlPolar->planeName())
        {
            pPOpp->setVisible(true);
        }
    }

    emit projectModified();
    m_pPlaneExplorer->updateVisibilityBoxes();
    m_bResetCurves = true;
    updateView();
}


void XPlane::onShowWPlrPOpps()
{
    int i;
    //Switch all WOpps view to on for the current Plane and WPolar
    m_bCurPOppOnly = false;

    PlaneOpp *pPOpp;
    if(m_pCurPlane)
    {
        for (i=0; i< Objects3d::nPOpps(); i++)
        {
            pPOpp = Objects3d::POppAt(i);
            if (m_pCurPlPolar->hasPOpp(pPOpp))
            {
                pPOpp->setVisible(true);
            }
        }
    }
    emit projectModified();
    m_pPlaneExplorer->updateVisibilityBoxes();

    m_bResetCurves = true;
    updateView();
}


void XPlane::onShowWPolarOppsOnly()
{
    if(!m_pCurPlane || !m_pCurPlPolar) return;
    for(int i=0; i<Objects3d::nPOpps(); i++)
    {
        PlaneOpp *pPOpp = Objects3d::POppAt(i);
        if(m_pCurPlPolar->hasPOpp(pPOpp))
        {
            pPOpp->setVisible(true);
        }
        else pPOpp->setVisible(false);
    }
    m_bResetCurves = true;
    updateView();
}


void XPlane::onShowTargetCurve()
{
    TargetCurveDlg dlg;
    dlg.initDialog(m_bShowEllipticCurve, m_bShowBellCurve, m_bMaxCL, m_BellCurveExp, m_TargetCurveStyle);
    dlg.exec();

    m_BellCurveExp       = dlg.bellCurveExp();
    m_bMaxCL             = dlg.bMaxCl();
    m_bShowEllipticCurve = dlg.bShowEllipticCurve();
    m_bShowBellCurve     = dlg.bShowBellCurve();
    m_TargetCurveStyle   = dlg.lineStyle();

    m_bResetCurves = true;
    updateView();
}


void XPlane::onShowPanelNormals()
{
    m_pgl3dXPlaneView->m_bPanelNormals = m_pActions->m_pShowPanelNormals->isChecked();
    if(m_pgl3dXPlaneView->m_bPanelNormals)
    {
        m_pgl3dXPlaneView->m_bNodeNormals = false; // show either but not both
        m_pActions->m_pShowNodeNormals->setChecked(false);
    }
    m_pgl3dXPlaneView->resetglMesh();
    if(is3dView()) updateView();
}


void XPlane::onShowNodeNormals()
{
    m_pgl3dXPlaneView->m_bNodeNormals = m_pActions->m_pShowNodeNormals->isChecked();
    if(m_pgl3dXPlaneView->m_bNodeNormals)
    {
        m_pgl3dXPlaneView->m_bPanelNormals = false; // show either but not both
        m_pActions->m_pShowPanelNormals->setChecked(false);
    }
    m_pgl3dXPlaneView->resetglMesh();
    updateView();
}


void XPlane::onShowVortices()
{
    m_pgl3dXPlaneView->m_bVortices = m_pActions->m_pShowVortices->isChecked();
    updateView();
}


void XPlane::updateStabilityDirection(bool bLongitudinal)
{
    s_pMainFrame->m_pStabPolarTiles->setOneGraph();
    for(int ig=0; ig<m_TimeGraph.size(); ig++)
    {
        m_TimeGraph[ig]->deleteCurves();
        m_TimeGraph[ig]->invalidate();
    }

    m_pPlaneExplorer->setCurveParams();
    setControls();
    setGraphTiles(); //needed to switch between longitudinal and lateral graphs
    s_pMainFrame->m_pStabTimeTiles->makeLegend(true);
    setStabTimeYVariables(bLongitudinal);

    m_bResetCurves = true;
    updateView();
}


void XPlane::onStabTimeView()
{
    stopAnimate();
    m_eView =  XPlane::STABTIMEVIEW;

    setGraphTiles();
    s_pMainFrame->setActiveCentralWidget();

    m_pPlaneExplorer->setCurveParams();
    setControls();
    setStabTimeYVariables(m_pStabTimeControls->isStabLongitudinal());

    m_bResetCurves = true;
    updateView();
}


void XPlane::setStabTimeYVariables(bool bLong)
{
    if(bLong)
    {
        m_TimeGraph[0]->setYVariableList({"u ("+Units::speedUnitQLabel()+")"});
        m_TimeGraph[1]->setYVariableList({"w ("+Units::speedUnitQLabel()+")"});
        m_TimeGraph[2]->setYVariableList({"q ("+DEGch+"/s)"});
        m_TimeGraph[3]->setYVariableList({THETAch + " ("+DEGch+")"});
    }
    else
    {
        m_TimeGraph[0]->setYVariableList({"v ("+Units::speedUnitQLabel()+")"});
        m_TimeGraph[1]->setYVariableList({"p ("+DEGch+"/s)"});
        m_TimeGraph[2]->setYVariableList({"r ("+DEGch+"/s)"});
        m_TimeGraph[3]->setYVariableList({PHIch + " ("+DEGch+")"});
    }
}


void XPlane::onRootLocusView()
{
    stopAnimate();
    m_eView = XPlane::STABPOLARVIEW;

    setGraphTiles();
    s_pMainFrame->setActiveCentralWidget();
    m_pPlaneExplorer->setCurveParams();
    setControls();
    m_bResetCurves = true;
    updateView();
}


/**
 * The user has requested to change the display of stability results to the modal view
 */
void XPlane::onModalView()
{
    m_eView = XPlane::W3DVIEW;

    setGraphTiles();
    s_pMainFrame->setActiveCentralWidget();
    m_pPlaneExplorer->setCurveParams();
    setControls();
    updateView();
}

/*
void XPlane::onWakePanels(bool bChecked)
{
    m_pgl3dXPlaneView->m_bWakePanels = bChecked;
    if(is3dView()) updateView();
}*/


/**
 * The user has requested the edition of the inertia data for the current wing or plane
 * Updates the inertia, resets the depending polars, and deletes the obsolete operating points
 * Updates the display
 */
void XPlane::onPlaneInertia()
{
    if(!m_pCurPlane) return;

    int iExitCode(0);
    Plane *pModPlane(nullptr);
    if(m_pCurPlane->isXflType())
    {
        PlaneXflInertiaDlg iDlg(s_pMainFrame);

        pModPlane = new PlaneXfl;
        pModPlane->duplicate(m_pCurPlane);
        iDlg.initDialog(pModPlane);

        iExitCode = iDlg.exec();
        if(iExitCode==QDialog::Rejected)
            return;
    }
    else if(m_pCurPlane->isSTLType())
    {
        PlaneStlInertiaDlg iDlg(s_pMainFrame);

        pModPlane = new PlaneSTL;
        pModPlane->duplicate(m_pCurPlane);
        iDlg.initDialog(pModPlane);

        iExitCode = iDlg.exec();
        if(iExitCode==QDialog::Rejected)
            return;
    }

    if(pModPlane)
    {
        setModPlane(pModPlane, Objects3d::hasResults(m_pCurPlane), iExitCode==10);
    }

    m_pPlaneExplorer->setObjectProperties();
    updateView();
}


void XPlane::onWingInertia()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pCurPlane);

    WingXfl workwing;
    int iWing=-1;
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;
    switch(pSenderAction->data().toInt())
    {
        case xfl::Main:
            iWing = pPlaneXfl->wingIndex(pPlaneXfl->mainWing());
            break;
        case xfl::Elevator:
            iWing = pPlaneXfl->wingIndex(pPlaneXfl->stab());
            break;
        case xfl::Fin:
            iWing = pPlaneXfl->wingIndex(pPlaneXfl->fin());
            break;
        default:
            return;
    }
    if(iWing<0) return;

    workwing.duplicate(pPlaneXfl->wingAt(iWing));
    workwing.createSurfaces(Vector3d(), pPlaneXfl->rxAngle(iWing), pPlaneXfl->ryAngle(iWing));

    //update the inertia
    workwing.computeStructuralInertia(pPlaneXfl->wingLE(iWing)); //wing surfaces are defined in the plane's body axis

    PartInertiaDlg dlg(&workwing, nullptr, s_pMainFrame);
    dlg.setPlaneName(QString::fromStdString(pPlaneXfl->name()));

    if(dlg.exec()==QDialog::Rejected)
    {
        return;
    }

    PlaneXfl *pModPlane  = new PlaneXfl;
    pModPlane->duplicate(m_pCurPlane);
    pModPlane->wing(iWing)->duplicate(workwing);
    setModPlane(pModPlane, Objects3d::hasResults(m_pCurPlane), false);
    m_pPlaneExplorer->setObjectProperties();
    updateView();
}


void XPlane::onFuseInertia()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType() || !m_pCurPlane->hasFuse()) return;
    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);

    Fuse *pFuse = pPlaneXfl->fuseAt(0)->clone();
    if (!pFuse) return;

    //update the inertia
    pFuse->computeStructuralInertia(Vector3d()); //fuse geometry is defined in its own body axis

    // save the inertia properties
    bool bAutoInertia = pFuse->bAutoInertia();
    Inertia meminertia = pFuse->inertia();

    PartInertiaDlg dlg(nullptr, pFuse, s_pMainFrame);

    if(dlg.exec()!=QDialog::Accepted)
    {
        // restore
        pFuse->setAutoInertia(bAutoInertia);
        pFuse->setInertia(meminertia);
        return;
    }

    PlaneXfl *pModPlane  = new PlaneXfl;
    pModPlane->duplicate(m_pCurPlane);
    pModPlane->fuse(0)->setInertia(pFuse->inertia());
    setModPlane(pModPlane, Objects3d::hasResults(m_pCurPlane), false);
    m_pPlaneExplorer->setObjectProperties();

    updateView();
}


void XPlane::onPlaneOppView()
{
    stopAnimate();
    if(isPOppView())
    {
        setControls();
        updateView();
        return;
    }

    m_eView=XPlane::POPPVIEW;
    setGraphTiles();

    s_pMainFrame->setActiveCentralWidget();

    m_pPlaneExplorer->updateVisibilityBoxes();
    setControls();

    m_bResetCurves = true;
    updateView();
}


void XPlane::onPolarView()
{
    stopAnimate();

    if(isPolarView())
    {
        setControls();
        updateView();
        return;
    }
    m_eView=XPlane::POLARVIEW;
    setGraphTiles();

    s_pMainFrame->setActiveCentralWidget();

    m_pPlaneExplorer->updateVisibilityBoxes();
    setControls();

    m_bResetCurves = true;
    updateView();
}


QString XPlane::planeOppData()
{
    if(!m_pCurPOpp)  return QString();

    QString Result, str, strong;
    QString format = xfl::isLocalized() ? "%L1" : "%1";

    int linelength = 25;
    str = Units::speedUnitQLabel();
    int l = str.length();
    if     (l==2) strong = "V" + INFch + QString(" = "+format).arg(m_pCurPOpp->m_QInf*Units::mstoUnit(), 8, 'f', 3);
    else if(l==3) strong = "V" + INFch + QString(" = "+format).arg(m_pCurPOpp->m_QInf*Units::mstoUnit(), 8, 'f', 3);
    else if(l==4) strong = "V" + INFch + QString(" = "+format).arg(m_pCurPOpp->m_QInf*Units::mstoUnit(), 7, 'f', 2);
    else          strong = QString();


    for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
    Result += strong + " " + str + "\n";

    strong = ALPHAch + QString(" = " + format).arg(m_pCurPOpp->alpha(), 8, 'f', 3);
    for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
    Result += strong + DEGch + "\n";

    strong = BETAch + QString(" = " + format).arg(m_pCurPOpp->beta(),   8, 'f', 3);
    for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
    Result += strong + DEGch + "\n";

    strong = PHIch + QString(" = " + format).arg(m_pCurPOpp->phi(),     8, 'f', 3);
    for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
    Result += strong + DEGch + "\n";

    strong = QString("CL = " + format).arg(m_pCurPOpp->m_AF.CL(), 8, 'f', 3);
    for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
    Result += strong +"\n";

    strong = QString("CD = " + format).arg(m_pCurPOpp->m_AF.CDv()+m_pCurPOpp->m_AF.CDi(), 8, 'f', 3);
    for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
    Result += strong +"\n";

    strong = QString("CY = " + format).arg(m_pCurPOpp->m_AF.Cy(), 8, 'f', 3);
    for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
    Result += strong +"\n";

    /*        oswald=CZ^2/CXi/PI/AR;*/
    if(m_pCurPlane && m_pCurPlane->isXflType())
    {
        PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);
        if(pPlaneXfl->hasMainWing())
        {
            double cxielli=m_pCurPOpp->aeroForces().CL()*m_pCurPOpp->aeroForces().CL()/PI/pPlaneXfl->mainWing()->aspectRatio();
            strong = QString("Eff. = "+format).arg(cxielli/m_pCurPOpp->aeroForces().CDi(), 8, 'f', 3);
            for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
            Result += strong +"\n";
        }
    }

    strong = QString("CL/CD = " + format).arg(m_pCurPOpp->m_AF.CL()/(m_pCurPOpp->m_AF.CDi()+m_pCurPOpp->m_AF.CDv()), 8, 'f', 3);
    for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
    Result += strong +"\n";

    double Cli = m_pCurPOpp->m_AF.Cli();
    strong = QString("Cl = " + format).arg(Cli, 8, 'f', 3);
    for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
    Result += strong +"\n";

    double Cm = m_pCurPOpp->m_AF.Cm();
    strong = QString("Cm = " + format).arg(Cm, 8, 'f', 3);
    for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
    Result += strong +"\n";

    double Cn = m_pCurPOpp->m_AF.Cn();
    strong = QString("Cn = " + format).arg(Cn, 8, 'f', 3);
    for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
    Result += strong +"\n";

    str = Units::lengthUnitQLabel();
    l = str.length();

    if(l==1)  str+=" ";
    if(m_pCurPOpp->isType7())
    {
        strong = QString("X_NP = "+ format).arg(m_pCurPOpp->m_SD.XNP*Units::mtoUnit(), 8, 'f', 3);
        for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
        Result += strong + str + "\n";
    }

    strong = QString("X_CP = "+ format).arg(m_pCurPOpp->m_AF.centreOfPressure().x*Units::mtoUnit(), 8, 'f', 3);
    for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
    Result += strong + " " +str + "\n";
    strong = QString("X_CG = " + format).arg(m_pCurPlPolar->CoG().x*Units::mtoUnit(), 8, 'f', 3);
    for(int i=strong.length(); i<linelength; i++) strong = " "+strong;
    Result += strong + " " +str;

    return Result;
}


void XPlane::onAnalyze()
{
    if(!m_pCurPlane || !m_pCurPlPolar)   return;
    if(m_pCurPlPolar->isExternalPolar()) return;

    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);
    if(pPlaneXfl)
    {
        // check that all the foils are in the database...
        // ...could have been deleted or renamed or not imported with AVL wing or whatever
        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            WingXfl const*pWing = pPlaneXfl->wingAt(iw);
            if(pWing)
            {
                for (int l=0; l<pWing->nSections(); l++)
                {
                    if (!Objects2d::foil(pWing->rightFoilName(l)))
                    {
                        std::string strong = pWing->name() + ": Could not find the foil "+ pWing->rightFoilName(l) +"...\nAborting Calculation";
                        displayMessage(QString::fromStdString(strong) +"\n", true, false);
                        return;
                    }
                    if (!Objects2d::foil(pWing->leftFoilName(l)))
                    {
                        std::string strong = pWing->name() + ": Could not find the foil "+ pWing->leftFoilName(l) +"...\nAborting Calculation";
                        displayMessage(QString::fromStdString(strong) +"\n", true, false);
                        return;
                    }
                }
            }
        }
    }

    m_pAnalysisControls->enableAnalyze(false);
    if(m_pCurPlPolar->isPanelMethod())
    {
        std::vector<T8Opp> xranges;
        if(m_pCurPlPolar->isType8())
        {
            m_pAnalysisControls->getXRanges(xranges);
            if(!xranges.size())
                return;
        }
        else if(!m_pAnalysisControls->oppList().size())
            return;

        m_pPanelAnalysisDlg->show();
        m_pPanelAnalysisDlg->analyze(m_pCurPlane, m_pCurPlPolar, m_pAnalysisControls->oppList(), xranges);
    }
    else if(m_pCurPlane->isXflType() && m_pCurPlPolar->isLLTMethod())
    {
        PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pCurPlane);
        m_pLLTAnalysisDlg->initDialog(pPlaneXfl, m_pCurPlPolar, m_pAnalysisControls->oppList());
        m_pLLTAnalysisDlg->show();
        m_pLLTAnalysisDlg->update();

        m_pLLTAnalysisDlg->analyze();
    }
}


void XPlane::onFinishAnalysis(PlanePolar *pWPolar)
{
    if(!pWPolar) return;

    m_pgl3dXPlaneView->viewBuffers()->clearLiveVortons();

    PlaneOpp *pLastPOpp = nullptr;
    if(pWPolar->isLLTMethod())
    {
        pLastPOpp = m_pLLTAnalysisDlg->lastPOpp();
        if(m_pLLTAnalysisDlg->hasErrors() && Analysis3dSettings::keepOpenOnErrors())
        {
        }
        else
            m_pLLTAnalysisDlg->close();
    }
    else
    {
        pLastPOpp = m_pPanelAnalysisDlg->lastPOpp();
        if(m_pPanelAnalysisDlg->hasErrors() && Analysis3dSettings::keepOpenOnErrors())
        {
        }
        else m_pPanelAnalysisDlg->close();
    }

    m_pPlaneExplorer->addPOpps(pWPolar);
    if(m_pCurPlPolar==pWPolar)
    {
        setPlaneOpp(pLastPOpp);
        if(pLastPOpp) m_pPlaneExplorer->selectPlaneOpp(pLastPOpp);
        else          m_pPlaneExplorer->selectPlPolar(m_pCurPlPolar, true);
    }
    else
        setPlaneOpp(nullptr);

    m_pPlaneExplorer->setCurveParams();

    //refresh the view
    m_bResetCurves = true;

    updateView();
    setControls();

    m_pAnalysisControls->enableAnalyze(true);

    MCTriangle::resetNullCount();

    emit projectModified();
}


Plane *XPlane::setPlane(QString const &PlaneName)
{
    return setPlane(Objects3d::plane(PlaneName.toStdString()));
}


Plane *XPlane::setPlane(Plane* pPlane)
{
    if(isAnalysisRunning())
    {
        return m_pCurPlane;
    }

    Surface::s_DebugPts.clear();
    Surface::s_DebugVecs.clear();
//    PanelAnalysis::s_DebugPts.clear();
//    PanelAnalysis::s_DebugVecs.clear();

    onClearHighlightSelection();

    m_pgl3dXPlaneView->onCancelThreads();

    if(m_pCurPlane)
    {
        if(!m_pCurPlane->isInitialized())
        {
            // initialize on the fly when needed
            m_pCurPlane->makePlane(true, false, true);
        }
        if(pPlane==m_pCurPlane)
        {
            std::string log;
            if(!m_pCurPlane->checkFoils(log))
            {
                displayStdMessage(log, false, false);
            }

            m_pgl3dXPlaneView->setPlaneReferenceLength(m_pCurPlane);
            if(m_pXPlaneWt) m_pXPlaneWt->gl3dFloatView()->setPlaneReferenceLength(m_pCurPlane);
            m_pgl3dXPlaneView->setPlane(m_pCurPlane);
            m_pgl3dXPlaneView->setVisiblePanels(std::vector<Panel3>());
            m_pgl3dXPlaneView->setVisiblePanels(std::vector<Panel4>());


            if(m_pXPlaneWt) m_pXPlaneWt->updateObjectData();
            return m_pCurPlane;
        }
        else if(!pPlane)
        {
            pPlane = m_pCurPlane;
        }
        else
        {
            //it's another plane
            m_pCurPOpp = nullptr;
            m_pCurPlPolar = nullptr;
        }
    }

    m_pgl3dXPlaneView->s_bResetglGeom = true;
    m_pgl3dXPlaneView->resetglMesh();
    m_pgl3dXPlaneView->s_bResetglWake = true;

    if(!pPlane)
    {
        //get the first one in the list
        if(Objects3d::nPlanes())
        {
            m_pCurPlane = Objects3d::planeAt(0);
        }
    }
    else
        m_pCurPlane = pPlane;


    m_pgl3dXPlaneView->setVisiblePanels(std::vector<Panel3>());
    m_pgl3dXPlaneView->setVisiblePanels(std::vector<Panel4>());

    if(!m_pCurPlane)
    {
        // no plane,
        //clear the pointers
        //clear the GUI
        m_pCurPlPolar = nullptr;
        m_pCurPOpp  = nullptr;

        m_bResetCurves = true;

        m_pPlaneExplorer->fillModelView();
        m_pPlaneExplorer->setObjectProperties();

        m_pgl3dXPlaneView->s_bResetglGeom = true;
        m_pgl3dXPlaneView->resetglMesh();
        m_pgl3dXPlaneView->s_bResetglWake = true;
        updateView();

        m_pgl3dXPlaneView->setPlane(nullptr);
        m_pgl3dXPlaneView->clearBotLeftOutput();
        m_pgl3dXPlaneView->clearBotRightOutput();
        if(m_pXPlaneWt) m_pXPlaneWt->updateObjectData();
        return nullptr;
    }
    if(!m_pCurPlane->isInitialized())
    {
        // initialize on the fly when needed
        m_pCurPlane->makePlane(true, false, true);
        m_pgl3dXPlaneView->s_bResetglGeom = true;
        m_pgl3dXPlaneView->resetglMesh();
        m_pgl3dXPlaneView->s_bResetglWake = true;
    }

    std::string log;
    if(m_pCurPlane && !m_pCurPlane->checkFoils(log))
    {
        displayStdMessage(log, false, false);
    }

    m_pgl3dXPlaneView->setPlaneReferenceLength(m_pCurPlane);
    if(m_pXPlaneWt) m_pXPlaneWt->gl3dFloatView()->setPlaneReferenceLength(m_pCurPlane);

    m_pgl3dXPlaneView->setPlane(m_pCurPlane);
    if(m_pXPlaneWt) m_pXPlaneWt->updateObjectData();


    bool bSetScale = (m_pgl3dXPlaneView->objectReferenceLength()<0.0);
    if(bSetScale)
    {
        m_pgl3dXPlaneView->reset3dScale();
    }
    else if(W3dPrefs::autoAdjust3dScale())
    {
        m_pgl3dXPlaneView->on3dReset();
        if(m_pXPlaneWt) m_pXPlaneWt->gl3dFloatView()->on3dReset();
    }

    return m_pCurPlane;
}


void XPlane::setPolar(QString const &PlrName)
{
    PlanePolar *pWPolar = Objects3d::wPolar(m_pCurPlane, PlrName.toStdString());
    setPolar(pWPolar); // may be null
}


void XPlane::setPolar(PlanePolar *pPlPolar)
{
    if(isAnalysisRunning())
    {
        return;
    }

    onClearHighlightSelection();

    m_pgl3dXPlaneView->onCancelThreads();

    m_pgl3dXPlaneView->clearTopRightOutput(); // in case there is no PlaneOpp, otherwise will be filled when setting the POpp
    m_pgl3dXPlaneView->clearBotRightOutput(); // in case there is no PlaneOpp, otherwise will be filled when setting the POpp

    if(!pPlPolar)
    {
        //if we didn't find anything, find the first for this plane
        for (int iwp=0; iwp<Objects3d::nPolars(); iwp++)
        {
            PlanePolar *pOldWPolar = Objects3d::plPolarAt(iwp);
            if (pOldWPolar->planeName().compare(m_pCurPlane->name())==0)
            {
                pPlPolar = pOldWPolar;
                break;
            }
        }
    }

    if(!m_pCurPlane || !pPlPolar)
    {
        m_pCurPlPolar = nullptr;
        m_pCurPOpp = nullptr;
        m_pgl3dXPlaneView->clearTopRightOutput();
        m_pgl3dXPlaneView->clearBotRightOutput();
        if(m_pXPlaneWt)
        {
            m_pXPlaneWt->updateObjectData();
        }

        updateVisiblePanels();
        m_pgl3dXPlaneView->resetglMesh();
        m_bResetCurves = true;
        return;
    }

    m_pCurPlPolar = pPlPolar;

    if(m_pCurPlane && m_pCurPlPolar)
    {
        //make sure the polar is up to date with the latest plane data
        if(m_pCurPlPolar->bAutoInertia())
        {
            m_pCurPlPolar->setMass(m_pCurPlane->totalMass());
            m_pCurPlPolar->setCoG(m_pCurPlane->CoG_t());
            m_pCurPlPolar->setInertiaTensor(m_pCurPlane->Ixx_t(), m_pCurPlane->Iyy_t(), m_pCurPlane->Izz_t(), m_pCurPlane->Ixz_t());
        }

        if(m_pCurPlPolar->referenceDim()!=xfl::CUSTOM)
        {
            // get the latest dimensions from the plane definition
            //
            m_pCurPlPolar->setReferenceChordLength(m_pCurPlane->mac());
            if(m_pCurPlPolar->referenceDim()==xfl::PLANFORM)
            {
                m_pCurPlPolar->setReferenceArea(m_pCurPlane->planformArea(m_pCurPlPolar->bIncludeOtherWingAreas()));
                m_pCurPlPolar->setReferenceSpanLength(m_pCurPlane->planformSpan());
            }
            else if(m_pCurPlPolar->referenceDim()==xfl::PROJECTED)
            {
                m_pCurPlPolar->setReferenceArea(m_pCurPlane->projectedArea(m_pCurPlPolar->bIncludeOtherWingAreas()));
                m_pCurPlPolar->setReferenceSpanLength(m_pCurPlane->projectedSpan());
            }
        }
    }


    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl *>(m_pCurPlane);

    if(m_pCurPlPolar)
    {
        if(pPlaneXfl && m_pCurPlPolar->isQuadMethod())
        {
            pPlaneXfl->makeQuadMesh(m_pCurPlPolar->bThickSurfaces(), m_pCurPlPolar->bIgnoreBodyPanels());
        }

        if(m_pCurPlPolar->isTriangleMethod())
        {
            if(m_pCurPlane->isXflType())
                m_pCurPlane->makeTriMesh(m_pCurPlPolar->bThickSurfaces());
        }

        m_pCurPlane->restoreMesh();

        switch(m_pCurPlPolar->type())
        {
            case xfl::T1POLAR:
            case xfl::T2POLAR:
            case xfl::T3POLAR:
            case xfl::T5POLAR:
            case xfl::T7POLAR:
            case xfl::T8POLAR:
            {
                if(pPlaneXfl && m_pCurPlPolar->hasActiveFlap())
                {
                    std::string strange;
                    pPlaneXfl->setFlaps(m_pCurPlPolar, strange);
                }
                if(fabs(m_pCurPlPolar->phi())>AOAPRECISION)
                {
                    if(pPlaneXfl && m_pCurPlPolar->isQuadMethod())                pPlaneXfl->quadMesh().rotate(0,0,m_pCurPlPolar->phi());
                    else if(m_pCurPlPolar->isTriangleMethod())       m_pCurPlane->triMesh().rotate(0,0,m_pCurPlPolar->phi());
                }
                break;
            }
            case xfl::T6POLAR:
            {
                if(pPlaneXfl)
                {
                    //check that the size of the Angle Range controls still matches the plane's control size
                    int nctrls = 0;
                    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
                    {
                        nctrls++;
                        nctrls += pPlaneXfl->wingAt(iw)->nFlaps();
                    }
                    // if not, resize
                    if(nctrls!=m_pCurPlPolar->nAngleRangeCtrls())
                    {
                        m_pCurPlPolar->resetAngleRanges(m_pCurPlane);
                    }
                }
                break;
            }
            default: break;
        }


        if(pPlaneXfl && m_pCurPlPolar->isQuadMethod())
        {
            if(!m_pCurPlPolar->bVortonWake())
            {
                pPlaneXfl->refQuadMesh().makeWakePanels(m_pCurPlPolar->NXWakePanel4(), m_pCurPlPolar->wakePanelFactor(), m_pCurPlPolar->wakeLength(),
                                                        Vector3d(1.0,0,0), true);
            }
            else
            {
                pPlaneXfl->refQuadMesh().makeWakePanels(3, 1.0, m_pCurPlPolar->bufferWakeLength(), Vector3d(1.0,0,0), false);
            }
        }

        if(m_pCurPlPolar->isTriangleMethod())
        {
            if(!m_pCurPlPolar->bVortonWake())
            {
                m_pCurPlane->triMesh().makeWakePanels(m_pCurPlPolar->NXWakePanel4(), m_pCurPlPolar->wakePanelFactor(), m_pCurPlPolar->wakeLength(),
                                                        Vector3d(1.0,0,0), true);
            }
            else
            {
                m_pCurPlane->triMesh().makeWakePanels(3, 1.0, m_pCurPlPolar->bufferWakeLength(), Vector3d(1.0,0,0), false);
            }
        }

    }

    QString props = QString::fromStdString(m_pCurPlane->planeData(pPlPolar->bIncludeOtherWingAreas()));
    QString strange;
    if(m_pCurPlane->isXflType())
    {
        props +="\n";
        if(pPlPolar->isQuadMethod())
        {
            PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);
            strange = QString::asprintf("Quad panels     = %d", pPlaneXfl->quadMesh().nPanels());
        }
        else if(pPlPolar->isTriangleMethod())
            strange = QString::asprintf("Triangles       = %d", m_pCurPlane->triMesh().nPanels());
    }
    props += strange;

    m_pgl3dXPlaneView->setBotLeftOutput(props);
    updateVisiblePanels();
    if(m_pXPlaneWt) m_pXPlaneWt->updateObjectData();

    if(m_pgl3dXPlaneView->isPicking())
    {
        if      (m_pCurPlPolar->isQuadMethod())       m_pgl3dXPlaneView->setPicking(xfl::PANEL4);
        else if (m_pCurPlPolar->isTriUniformMethod()) m_pgl3dXPlaneView->setPicking(xfl::PANEL3);
        else if (m_pCurPlPolar->isTriLinearMethod())  m_pgl3dXPlaneView->setPicking(xfl::MESHNODE);
    }

    m_pAnalysisControls->setAnalysisRange();
    m_pStabTimeControls->fillAVLcontrols(m_pCurPlPolar);

    m_pgl3dXPlaneView->resetglMesh();
    m_bResetCurves = true;

}


void XPlane::stopAnimate()
{
    m_pPOppGraphCtrls->stopAnimate();
    m_pPOpp3dCtrls->stopAnimate();

    m_pPOpp3dCtrls->m_pStab3dCtrls->stopAnimate();
    m_pPOpp3dCtrls->stopFlow();

    if(m_pCurPlane)
    {
        setPlaneOpp(m_pCurPOpp);
        m_pPlaneExplorer->selectPlaneOpp(m_pCurPOpp);
    }
}


void XPlane::stopAnimateMode()
{
    m_pTimerMode->stop();
}


void XPlane::updateUnits()
{
    QString strLength  = Units::lengthUnitQLabel();

    PlanePolar::setVariableNames();
    for(int ig=0; ig<m_WPlrGraph.size(); ig++)
    {
        m_WPlrGraph.at(ig)->setXVarStdList(PlanePolar::variableNames());
        m_WPlrGraph.at(ig)->setYVarStdList(PlanePolar::variableNames());
    }

    PlaneOpp::setVariableNames();
    QStringList XPOppList = {"y (" + strLength +")"};
    for(int ig=0; ig<m_WingGraph.size(); ig++)
    {
        m_WingGraph.at(ig)->setXVariableList(XPOppList);
        m_WingGraph.at(ig)->setYVarStdList(PlaneOpp::variableNames());
    }

    setStabTimeYVariables(m_pStabTimeControls->isStabLongitudinal());

    if (isPOppView())
    {
        onAdjustWingGraphToWingSpan();
    }
    else if(isCpView()) createCpCurves();

    m_pPlaneExplorer->setCurveParams();
    m_pPlaneExplorer->setObjectProperties();
    m_bResetCurves = true;

    m_pgl3dXPlaneView->setBotRightOutput(planeOppData());
    if(m_pXPlaneWt) m_pXPlaneWt->updateObjectData();

    m_pPOpp3dCtrls->updateUnits();

    updateView();
}


void XPlane::updateView()
{
    if(m_bResetCurves)
    {
        if(is3dView())
        {
            createWPolarCurves();
            createWOppCurves(); // to update the highlighted curve in case a POpp view is opened in a floating window
//            s_pMainFrame->m_pWPolarTiles->makeLegend(true);
        }
        else if(isPolarView())
        {
            createWPolarCurves();
            createWOppCurves(); // to update the highlighted curve in case a POpp view is opened in a floating window
            s_pMainFrame->m_pWPolarTiles->makeLegend(true);
        }
        else if(isPOppView())
        {
            createWOppCurves();
            createWPolarCurves(); // to update the highlighted POpp in case a WPolar view is opened in a floating window
            s_pMainFrame->m_pPOppTiles->makeLegend(true);
        }
        else if(isStabPolarView())
        {
            createStabilityCurves();
            createWOppCurves(); // to update the highlighted curve in case a POpp view is opened in a floating window
            s_pMainFrame->m_pStabPolarTiles->makeLegend(true);
        }
        else if(isStabTimeView())
        {
            createStabilityCurves();
            createWOppCurves(); // to update the highlighted curve in case a POpp view is opened in a floating window
            s_pMainFrame->m_pStabTimeTiles->makeLegend(true);
        }
        else if (isCpView())
        {
            createCpCurves();
        }

        emit curvesUpdated();

        m_bResetCurves = false;
    }

    if     (is3dView())        m_pgl3dXPlaneView->update();
    else if(isPolarView())     s_pMainFrame->m_pWPolarTiles->update();
    else if(isPOppView())      s_pMainFrame->m_pPOppTiles->update();
    else if(isStabPolarView()) s_pMainFrame->m_pStabPolarTiles->update();
    else if(isStabTimeView())  s_pMainFrame->m_pStabTimeTiles->update();
    else if(isCpView())        s_pMainFrame->m_pCpViewWt->update();

    if(m_pXPlaneWt && m_pXPlaneWt->isVisible())
        m_pXPlaneWt->updateView();

//    s_pMainFrame->update();
}


void XPlane::setView(xfl::enumGraphView eView)
{
    switch (m_eView)
    {
        case XPlane::POPPVIEW:
        {
            s_pMainFrame->m_pPOppTiles->setView(eView);
            break;
        }
        case XPlane::POLARVIEW:
        {
            s_pMainFrame->m_pWPolarTiles->setView(eView);
            break;
        }
        case XPlane::STABPOLARVIEW:
        {
            s_pMainFrame->m_pStabPolarTiles->setView(eView);
            break;
        }
        case XPlane::STABTIMEVIEW:
        {
            s_pMainFrame->m_pStabTimeTiles->setView(eView);
            break;
        }
        default:
        {
            break;
        }
    }
}


void XPlane::onPolarProperties()
{
    if(!m_pCurPlPolar) return;
    ObjectPropsDlg *pOPDlg = new ObjectPropsDlg(s_pMainFrame);
    std::string strangeProps;

    m_pCurPlPolar->getProperties(strangeProps, m_pCurPlane);

    pOPDlg->initDialog(QString::fromStdString(m_pCurPlane->name()+ "/" +m_pCurPlPolar->name()), QString::fromStdString(strangeProps));
    pOPDlg->show();
}


void XPlane::onPlaneOppProperties()
{
    if(!m_pCurPOpp) return;
    ObjectPropsDlg *pOPDlg = new ObjectPropsDlg(s_pMainFrame);
    std::string strangeprops;
    m_pCurPOpp->getProperties(m_pCurPlane, m_pCurPlPolar, strangeprops);
    pOPDlg->initDialog(QString::fromStdString(m_pCurPOpp->title(false)), QString::fromStdString(strangeprops));
    pOPDlg->show();
}


PlaneOpp* XPlane::setPlaneOpp(PlaneOpp *pPOpp)
{
    if(!m_pCurPlane || !m_pCurPlPolar)
    {
        m_pgl3dXPlaneView->resetglMesh();
        m_pCurPOpp = nullptr;
        m_pgl3dXPlaneView->clearTopRightOutput();
        m_pgl3dXPlaneView->clearBotRightOutput();
        if(m_pXPlaneWt) m_pXPlaneWt->updateObjectData();
        m_pgl3dXPlaneView->resetglPOpp();
        m_pPOpp3dCtrls->m_pFlowCtrls->enableFlowControls();
        return nullptr;
    }

    if(!pPOpp)
    {
        for(int ipopp=0; ipopp<Objects3d::nPOpps(); ipopp++)
        {
            PlaneOpp *pOldPOpp = Objects3d::POppAt(ipopp);
            if(pOldPOpp->planeName().compare(m_pCurPlane->name())==0 && pOldPOpp->polarName().compare(m_pCurPlPolar->name())==0)
            {
                pPOpp = pOldPOpp;
                break;
            }
        }
    }

    onClearHighlightSelection();

    m_pgl3dXPlaneView->onCancelThreads();

    if(!pPOpp)
    {
        //nothing left to try
        if(m_pPanelResultTest) m_pPanelResultTest->setAnalysis(m_pCurPlane, m_pCurPlPolar, m_pCurPOpp);

        setPolar(m_pCurPlPolar);// to restore the flapped mesh
        return nullptr;
    }

    // we have a valid PlaneOpp
    m_pCurPOpp = pPOpp;

    if(m_pCurPOpp->isType7())
    {
        if(isStabTimeView())
        {
            m_pStabTimeControls->setControls();
            m_pStabTimeControls->setMode();
        }
        else if(is3dView())
        {
            m_pPOpp3dCtrls->m_pStab3dCtrls->setMode(m_pCurPOpp);
        }
    }


    if(m_pCurPlane)
        m_pCurPlane->restoreMesh();

    PlaneXfl * pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pCurPlane);
    if(pPlaneXfl)
    {
        switch(m_pCurPlPolar->type())
        {
            case xfl::T1POLAR:
            case xfl::T2POLAR:
            case xfl::T3POLAR:
            case xfl::T5POLAR:
            case xfl::T7POLAR:
            case xfl::T8POLAR:
            {
                if(m_pCurPlPolar->hasActiveFlap())
                {
                    std::string strange;
                    pPlaneXfl->setFlaps(m_pCurPlPolar, strange);
                }
                break;
            }
            case xfl::T6POLAR:
            {
                std::string log;
                if(m_pCurPlPolar->isQuadMethod())
                {
                    pPlaneXfl->setRangePositions4(m_pCurPlPolar, m_pCurPOpp->ctrl(), log);
                }
                else if(m_pCurPlPolar->isTriangleMethod())
                {
                    pPlaneXfl->setRangePositions3(m_pCurPlPolar, m_pCurPOpp->ctrl(), log);
                }
                break;
            }
            default: break;
        }
    }

    if(pPlaneXfl && m_pCurPOpp->isQuadMethod())
        pPlaneXfl->quadMesh().rotate(m_pCurPOpp->alpha(), m_pCurPOpp->beta(), m_pCurPOpp->phi());
    else if(m_pCurPOpp->isTriangleMethod())
        m_pCurPlane->triMesh().rotate(m_pCurPOpp->alpha(), m_pCurPOpp->beta(), m_pCurPOpp->phi());

    Vector3d WindDir(1,0,0);
    // extend the wake behind the plane's last trailing point;

    if(m_pCurPOpp->isType6())
    {
        if(pPlaneXfl && m_pCurPOpp->isQuadMethod())
        {
            if(!m_pCurPlPolar->bVortonWake())
            {
                Vector3d pt;
                pPlaneXfl->quadMesh().getLastTrailingPoint(pt);
                pPlaneXfl->quadMesh().makeWakePanels(m_pCurPlPolar->NXWakePanel4(), m_pCurPlPolar->wakePanelFactor(),
                                                     pt.x + m_pCurPlPolar->wakeLength(), WindDir, true);
            }
            else
            {
                pPlaneXfl->quadMesh().makeWakePanels(3, 1.0, m_pCurPlPolar->bufferWakeLength(), WindDir, false);
            }
        }
        else if(m_pCurPOpp->isTriangleMethod())
        {
            if(!m_pCurPlPolar->bVortonWake())
            {
                Vector3d pt;
                m_pCurPlane->triMesh().getLastTrailingPoint(pt);
                m_pCurPlane->triMesh().makeWakePanels(m_pCurPlPolar->NXWakePanel4(), m_pCurPlPolar->wakePanelFactor(),
                                                      pt.x + m_pCurPlPolar->wakeLength(), WindDir, true);
            }
            else
            {
                m_pCurPlane->triMesh().makeWakePanels(m_pCurPlPolar->NXBufferWakePanels(), 1, m_pCurPlPolar->bufferWakeLength(), WindDir, false);
            }
        }
    }

    updateVisiblePanels(m_pCurPlane, m_pCurPlPolar);

//    if(is3dView())
    {
        m_pgl3dXPlaneView->resetWakeControls();
        m_pgl3dXPlaneView->clearTopRightOutput();
        m_pgl3dXPlaneView->setBotRightOutput(planeOppData());
        m_pgl3dXPlaneView->m_ColourLegend.makeLegend();
    }
    if(m_pXPlaneWt) m_pXPlaneWt->updateObjectData();

    if(m_pPanelResultTest) m_pPanelResultTest->setAnalysis(m_pCurPlane, m_pCurPlPolar, m_pCurPOpp);

    if(m_eView==XPlane::CPVIEW)
    {
        if(pPOpp->isTriLinearMethod())
        {
            TriMesh::makeNodeValues(m_pCurPlane->triMesh().nodes(), m_pCurPlane->triMesh().panels(),
                                    pPOpp->m_Cp, pPOpp->m_NodeValue,
                                    pPOpp->m_NodeValMin, pPOpp->m_NodeValMax, 1.0);
        }
    }

    if(m_pPOpp3dCtrls->isFlowActive() && pPOpp->isTriUniformMethod())
    {
        m_pgl3dXPlaneView->restartFlow();
    }
    else m_pgl3dXPlaneView->cancelFlow();

    m_pgl3dXPlaneView->resetglPOpp();
    m_pgl3dXPlaneView->resetglMesh();
    m_pgl3dXPlaneView->resetglStreamlines();

    setControls();

    m_bResetCurves = true;

    return m_pCurPOpp;
}


void XPlane::setGraphTiles()
{
    switch(m_eView)
    {
        case XPlane::POPPVIEW:
        {
            switch(s_pMainFrame->m_pPOppTiles->iView())
            {
                case xfl::ONEGRAPH:
                    s_pMainFrame->m_pPOppTiles->setGraphLayout(1);
                    break;
                case xfl::TWOGRAPHS:
                    s_pMainFrame->m_pPOppTiles->setGraphLayout(2);
                    break;
                case xfl::FOURGRAPHS:
                    s_pMainFrame->m_pPOppTiles->setGraphLayout(4);
                    break;
                default:
                case xfl::ALLGRAPHS:
                    s_pMainFrame->m_pPOppTiles->setGraphLayout(m_WingGraph.size());
                    break;
            }
            break;
        }

        case XPlane::POLARVIEW:
        {
            switch(s_pMainFrame->m_pWPolarTiles->iView())
            {
                case xfl::ONEGRAPH:
                    s_pMainFrame->m_pWPolarTiles->setGraphLayout(1);
                    break;
                case xfl::TWOGRAPHS:
                    s_pMainFrame->m_pWPolarTiles->setGraphLayout(2);
                    break;
                case xfl::FOURGRAPHS:
                    s_pMainFrame->m_pWPolarTiles->setGraphLayout(4);
                    break;
                default:
                case xfl::ALLGRAPHS:
                    s_pMainFrame->m_pWPolarTiles->setGraphLayout(m_WPlrGraph.size());
                    break;
            }
            break;
        }

        case XPlane::CPVIEW:
        {
            break;
        }

        case XPlane::STABPOLARVIEW:
        {
            s_pMainFrame->m_pStabPolarTiles->setGraphLayout(2);
            break;
        }

        case XPlane::STABTIMEVIEW:
        {
            switch(s_pMainFrame->m_pStabTimeTiles->iView())
            {
                case xfl::ONEGRAPH:
                    s_pMainFrame->m_pStabTimeTiles->setGraphLayout(1);
                    break;
                case xfl::TWOGRAPHS:
                    s_pMainFrame->m_pStabTimeTiles->setGraphLayout(2);
                    break;
                default:
                case xfl::FOURGRAPHS:
                case xfl::ALLGRAPHS:
                    s_pMainFrame->m_pStabTimeTiles->setGraphLayout(4);
                    break;
            }
            break;
        }

        default:
        {
            int maxWidgets = s_pMainFrame->m_pWPolarTiles->graphWtCount();
            for(int ig=0; ig<maxWidgets; ig++)
            {
                if(s_pMainFrame->m_pWPolarTiles->graphWt(ig))
                    s_pMainFrame->m_pWPolarTiles->graphWt(ig)->setGraph(nullptr);
            }
        }
    }
}


void XPlane::onExporttoSTL()
{
    if (!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    STLWriterDlg dlg(s_pMainFrame);
    PlaneXfl * pPlaneXfl = dynamic_cast<PlaneXfl *>(m_pCurPlane);

    dlg.initDialog(pPlaneXfl, nullptr, nullptr, nullptr);
    dlg.exec();
}


void XPlane::onExportFuseMeshToSTL()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    if(!m_pCurPlane->hasFuse()) return;
    PlaneXfl const* pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);
    Fuse const*pFuse = pPlaneXfl->fuseAt(0);

    QString filter ="STL File (*.stl)";


    QString filename(QString::fromStdString(pFuse->name()).trimmed()+".stl");
    filename.replace('/', '_');
    filename.replace(' ', '_');
    filename += "_mesh";

    filename = QFileDialog::getSaveFileName(nullptr, "Export to STL File",
                                            SaveOptions::STLDirName() + "/"+filename,
                                            "STL File (*.stl)",
                                            &filter);
    if(!filename.length()) return;

    Objects3d::exportMeshToSTLFile(filename, pFuse->triMesh(), Units::mtoUnit());
}


void XPlane::onExportMeshToSTLFile()
{
    if(!m_pCurPlane) return;

    QString filter ="STL File (*.stl)";

    QString filename(QString::fromStdString(m_pCurPlane->name()).trimmed()+".stl");
    filename.replace('/', '_');
    filename.replace(' ', '_');
    filename += "_mesh";

    filename = QFileDialog::getSaveFileName(nullptr, "Export to STL File",
                                            SaveOptions::STLDirName() + "/"+filename,
                                            "STL File (*.stl)",
                                            &filter);
    if(!filename.length()) return;

    Objects3d::exportMeshToSTLFile(filename, m_pCurPlane->triMesh(), Units::mtoUnit());
}


void XPlane::onImportPlanesfromXML()
{
    QStringList pathNames;
    pathNames = QFileDialog::getOpenFileNames(s_pMainFrame, "Open XML file",
                                              SaveOptions::xmlPlaneDirName(),
                                              "Plane XML file (*.xml)");
    if(!pathNames.size()) return;
//    QFileInfo fileInfo(pathNames.at(0));

    PlaneXfl *pPlane = nullptr;
    for(int iFile=0; iFile<pathNames.size(); iFile++)
    {
        QFile XFile(pathNames.at(iFile));
        pPlane = importPlaneFromXML(XFile);
    }

    updateTreeView();

    if(pPlane)
    {
        setPlane(pPlane);
        m_pPlaneExplorer->selectPlane(pPlane);
    }

    emit projectModified();

    setControls();
    updateView();
}


/**
 * Imports the plane geometry from an XML file
 */
PlaneXfl *XPlane::importPlaneFromXML(QFile &xmlFile)
{
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        QString strange = "Could not open the file "+xmlFile.fileName()+"\n";
        displayMessage(strange, true, false);
        return nullptr;
    }

    XmlPlaneReader planereader(xmlFile);
    if(!planereader.readFile())
    {
        QString strong;
        QString errormsg;
        errormsg = "Failed to read the file "+xmlFile.fileName()+"\n";
        strong = QString::asprintf("   error at line %d column %d\n",int(planereader.lineNumber()),int(planereader.columnNumber()));
        errormsg += strong;
        displayMessage(errormsg, true, false);
        return nullptr;
    }

    PlaneXfl *pPlane  = planereader.plane();
    if(!pPlane)
    {
        QString strong = "No plane definition found in the file " + xmlFile.fileName() +"\n";
        displayMessage(strong, true, false);
        return nullptr;
    }

    displayMessage("Plane "+QString::fromStdString(pPlane->name())+" imported successfully\n", false, true);

    if(Objects3d::planeExists(pPlane->name())) Objects3d::setModifiedPlane(pPlane);
    else                                       Objects3d::insertPlane(pPlane);

    return pPlane;
}


void XPlane::onExportWingToXML()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl *>(m_pCurPlane);

    WingXfl const *pWing = nullptr;
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;
    QString WingType;
    switch(pSenderAction->data().toInt())
    {
        case xfl::Main:
            pWing = pPlaneXfl->mainWing();
            WingType = "MAINWING";
            break;
        case xfl::Elevator:
            pWing = pPlaneXfl->stab();
            WingType = "ELEVATOR";
            break;
        case xfl::Fin:
            pWing = pPlaneXfl->fin();
            WingType = "FIN";
            break;
        default:
            return;
    }

    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = QString::fromStdString(pWing->name()).trimmed()+".xml";
    strong.replace(' ', '_');
    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export to xml file",
                                            SaveOptions::xmlPlaneDirName() +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;
//    QFileInfo fileInfo(FileName);

    if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    XmlWingWriter wingwriter(XFile);
    wingwriter.writeXMLWing(*pWing, SaveOptions::bXmlWingFoils());

    XFile.close();
}


/**
 * Exports the wing's triangular mesh to an STL file
 */
void XPlane::onExportWingMeshToSTL()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;

    PlaneXfl * pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pCurPlane);
    WingXfl *pWing = nullptr;
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    switch(pSenderAction->data().toInt())
    {
        case xfl::Main:
            pWing = pPlaneXfl->mainWing();
            break;
        case xfl::Elevator:
            pWing = pPlaneXfl->stab();
            break;
        case xfl::Fin:
            pWing = pPlaneXfl->fin();
            break;
        default:
            return;
    }
    if(!pWing) return;


    QString filter ="STL File (*.stl)";
    QString filename(QString::fromStdString(pWing->name()).trimmed()+".stl");
    filename.replace('/', '_');
    filename.replace(' ', '_');
    filename += "_mesh";
    filename = QFileDialog::getSaveFileName(nullptr, "Export to STL File",
                                            SaveOptions::STLDirName() + "/"+filename,
                                            "STL File (*.stl)",
                                            &filter);
    if(!filename.length()) return;


    pWing->makeTriPanels(0, 0, true);

    Objects3d::exportMeshToSTLFile(filename, pWing->triMesh(), Units::mtoUnit());
}


void XPlane::onExportWingToSTL()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl *>(m_pCurPlane);

    WingXfl *pWing = nullptr;
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;
    QString WingType;
    switch(pSenderAction->data().toInt())
    {
        case xfl::Main:
            pWing = pPlaneXfl->mainWing();
            WingType = "MAINWING";
            break;
        case xfl::Elevator:
            pWing = pPlaneXfl->stab();
            WingType = "ELEVATOR";
            break;
        case xfl::Fin:
            pWing = pPlaneXfl->fin();
            WingType = "FIN";
            break;
        default:
            return;
    }

    STLWriterDlg dlg(s_pMainFrame);
    dlg.initDialog(nullptr, pWing, nullptr, nullptr);
    dlg.exec();
}


void XPlane::onExportWingToCAD()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl *>(m_pCurPlane);

    WingXfl const *pWing = nullptr;
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;
    QString WingType;
    switch(pSenderAction->data().toInt())
    {
        case xfl::Main:
            pWing = pPlaneXfl->mainWing();
            WingType = "MAINWING";
            break;
        case xfl::Elevator:
            pWing = pPlaneXfl->stab();
            WingType = "ELEVATOR";
            break;
        case xfl::Fin:
            pWing = pPlaneXfl->fin();
            WingType = "FIN";
            break;
        default:
            return;
    }

    std::string logmsg;
    TopoDS_Shape wingshape;
    occ::makeWingShape(pWing, 0.0001, wingshape, logmsg);
    WingExportDlg dlg(s_pMainFrame);
    dlg.init(pWing);
    dlg.exec();
}


void XPlane::onExportFuseToXML()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);

    Fuse const *pFuse = pPlaneXfl->fuseAt(0);
    if(!pFuse || !pFuse->isXflType()) return ;// is there anything to export?

    FuseXfl const *pFuseXfl = dynamic_cast<FuseXfl const*>(pFuse);

    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = QString::fromStdString(pFuse->name()).trimmed()+".xml";
    strong.replace(' ', '_');
    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export plane definition to xml file",
                                            SaveOptions::xmlPlaneDirName() +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;
    int pos = FileName.indexOf(".xml", Qt::CaseInsensitive);
    if(pos<0) FileName += ".xml";

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    XmlFuseWriter fusewriter(XFile);

    fusewriter.writeXMLBody(*pFuseXfl);

    XFile.close();
}


void XPlane::onExportFuseToSTL()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);

    Fuse const *pFuse = pPlaneXfl->fuseAt(0);
    if(!pFuse) return ;// is there anything to export?

    QString filter ="STL File (*.stl)";
    QString FileName;
    FileName = QString::fromStdString(pFuse->name()).trimmed()+".stl";
    FileName.replace('/', '_');
    FileName.replace(' ', '_');

    QFileDialog Fdlg;
    FileName = Fdlg.getSaveFileName(s_pMainFrame, "Export to STL File",
                                    SaveOptions::STLDirName() + "/"+FileName,
                                    "STL File (*.stl)",
                                    &filter);

    if(!FileName.length()) return;


    bool bBinary = true;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastDirName(FileName.left(pos));

    pos = FileName.indexOf(".stl", Qt::CaseInsensitive);
    if(pos<0) FileName += ".stl";

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly))
    {
        displayMessage("Could not open the file for writing\n", true, false);
        return;
    }

    if(bBinary)
    {
        QDataStream out(&XFile);
        out.setByteOrder(QDataStream::LittleEndian);
//        m_pFuse->exportStlTriangulation(out,1);
        Objects3d::exportTriangulation(out,1.0, pFuse->triangles());
    }
    else
    {
//        QTextStream out(&XFile);
    }

    XFile.close();
}


void XPlane::onExportFuseToCAD()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType()) return;
    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pCurPlane);

    Fuse *pFuse = pPlaneXfl->fuse(0);
    if(!pFuse) return ;// is there anything to export?

    CADExportDlg dlg(s_pMainFrame);
    pFuse->makeFuseGeometry();
    dlg.init(pFuse->shapes(), QString::fromStdString(pFuse->name()));
    dlg.exec();
}


/**
 * Exports the plane geometry to an XML file
 */
void XPlane::onExportPlanetoXML()
{
    if(!m_pCurPlane || !m_pCurPlane->isXflType())return ;// is there anything to export?

    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);

    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = QString::fromStdString(m_pCurPlane->name()).trimmed()+".xml";
    //    strong.replace(' ', '_');
    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export to xml file",
                                            SaveOptions::xmlPlaneDirName() +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;
//    QFileInfo fileInfo(FileName);

    if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    XmlPlaneWriter planeWriter(XFile);
    planeWriter.writeXMLPlane(*pPlaneXfl, SaveOptions::bXmlWingFoils());

    XFile.close();
}


void XPlane::onExportAnalysisToXML()
{
    if(!m_pCurPlane || !m_pCurPlPolar) return ;// is there anything to export?

    QString filter = "XML file (*.xml)";
    QString FileName, strong;

//    strong = m_pCurPlane->planeName()+"_"+m_pCurWPolar->name();
    strong = QString::fromStdString(m_pCurPlPolar->name());
    strong.replace("/", "_");
    strong.replace(".", "_");
//    strong += ".xml";
    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export analysis definition to xml file",
                                            SaveOptions::xmlWPolarDirName() +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;
//    QFileInfo fileInfo(FileName);

    if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0)
        FileName += ".xml";


    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    XmlPlanePolarWriter wpolarWriter(XFile);
    wpolarWriter.writeXMLWPolar(m_pCurPlPolar);

    XFile.close();
}


void XPlane::updateTreeView()
{
    if(!m_pCurPlane) m_pPlaneExplorer->setObjectProperties();
    m_pPlaneExplorer->fillModelView();
}


void XPlane::onStyleChanged()
{
    resetCurves();
    updateView();
    emit projectModified();
}


void XPlane::makeLegend()
{
    if     (isPOppView())      s_pMainFrame->m_pPOppTiles->makeLegend(true);
    else if(isStabPolarView()) s_pMainFrame->m_pStabPolarTiles->makeLegend(true);
    else if(isStabTimeView())  s_pMainFrame->m_pStabTimeTiles->makeLegend(true);
    else if(isCpView())        s_pMainFrame->m_pCpViewWt->makeLegend(true);
    else                       s_pMainFrame->m_pWPolarTiles->makeLegend(true);
}


void XPlane::onCurveClicked(Curve*pCurve, int ipt)
{
    switch (m_eView)
    {
        case XPlane::POPPVIEW:
        {
            if(!pCurve)
            {
                for(int ig=0; ig<m_WingGraph.size(); ig++) m_WingGraph[ig]->clearSelection();
                s_pMainFrame->m_pPOppTiles->legendWt()->makeLegend(false);
                updateView();
                emit curvesUpdated(); // to deselect the operating point
                setControls();
                return;
            }

            for(int ipo=0; ipo<Objects3d::nPOpps(); ipo++)
            {
                PlaneOpp *pPOpp = Objects3d::POppAt(ipo);
                for(int ic=0; ic<pPOpp->nCurves(); ic++)
                {
                    if(pPOpp->m_Curve.at(ic)==pCurve)
                    {
                        //this is the one which has been clicked
                        setPlane(QString::fromStdString(pPOpp->planeName()));
                        setPolar(QString::fromStdString(pPOpp->polarName()));
                        setPlaneOpp(pPOpp);
                        m_pPlaneExplorer->selectPlaneOpp(pPOpp);
                        m_pPlaneExplorer->setCurveParams();
                        QString strange = "Selected the operating point: "+QString::fromStdString(pPOpp->planeName()+"/"+pPOpp->name()) + EOLch;
                        displayMessage(strange, false, true);
                        updateView();
                        s_pMainFrame->m_pPOppTiles->update();
                        setControls();
                        return;
                    }
                }
            }
            break;
        }
        case XPlane::STABPOLARVIEW:
        case XPlane::POLARVIEW:
        {
            if(!pCurve)
            {
                for(int ig=0; ig<m_WPlrGraph.size();    ig++) m_WPlrGraph[ig]->clearSelection();
                for(int ig=0; ig<m_StabPlrGraph.size(); ig++) m_StabPlrGraph[ig]->clearSelection();
                if     (isPolarView())     s_pMainFrame->m_pWPolarTiles->legendWt()->makeLegend(false);
                else if(isStabPolarView()) s_pMainFrame->m_pStabPolarTiles->legendWt()->makeLegend(false);

                updateView();
                emit curvesUpdated(); // to deselect the operating point
                setControls();
                return;
            }

            for(int iwp=0; iwp<Objects3d::nPolars(); iwp++)
            {
                PlanePolar *pWPolar=Objects3d::plPolarAt(iwp);
                for(int ic=0; ic<pWPolar->curveCount(); ic++)
                {
                    if(pWPolar->curve(ic)==pCurve)
                    {
                        //this is the one which has been clicked
                        setPlane(QString::fromStdString(pWPolar->planeName()));
                        setPolar(pWPolar);

                        if(ipt>=0)
                        {
                            setPlaneOpp(nullptr);
                        }

                        m_pPlaneExplorer->selectPlPolar(pWPolar, false);
                        m_pPlaneExplorer->setCurveParams();
//                        displayMessage("Selected the polar: "+pWPolar->planeName()+"/"+pWPolar->name() + "\n");
                        updateView();
                        if     (isPolarView())     s_pMainFrame->m_pWPolarTiles->update();
                        else if(isStabPolarView()) s_pMainFrame->m_pStabPolarTiles->update();
                        setControls();
                        return;
                    }
                }
            }
            break;
        }
        case XPlane::STABTIMEVIEW:
        {
            if(!pCurve)
            {
                for(int ig=0; ig<m_TimeGraph.size();    ig++) m_TimeGraph[ig]->clearSelection();
                s_pMainFrame->m_pStabTimeTiles->legendWt()->makeLegend(false);
                updateView();
                emit curvesUpdated(); // to deselect the operating point
                return;
            }

            QString curvename = pCurve->name();
            Curve *pSelCurve = m_TimeGraph.at(0)->curve(curvename, true);
            if(!pSelCurve) return;

            m_pStabTimeControls->selectCurve(pSelCurve);
            for(int i=0; i<m_TimeGraph.size(); i++)
            {
                m_TimeGraph.at(i)->clearSelection();
                Curve *pCurve = m_TimeGraph.at(i)->curve(pSelCurve->name());
                m_TimeGraph[i]->selectCurve(pCurve);
            }
            s_pMainFrame->m_pStabTimeTiles->makeLegend(true);

            updateView();
            break;
        }
        case XPlane::CPVIEW:
        {
            if(!pCurve)
            {
                s_pMainFrame->m_pCpViewWt->CpGraph()->clearSelection();
                s_pMainFrame->m_pCpViewWt->makeLegend(false);
                updateView();
                return;
            }

            s_pMainFrame->m_pCpViewWt->CpGraph()->clearSelection();
            s_pMainFrame->m_pCpViewWt->CpGraph()->selectCurve(pCurve);
            s_pMainFrame->m_pCpViewWt->makeLegend(true);

            updateView();
            break;
        }
        default: break;
    }

    setControls();
}


void XPlane::onCurveDoubleClicked(Curve*pCurve)
{
    if(!pCurve) return;

    if(isPOppView())
    {
        for(int ipo=0; ipo<Objects3d::nPOpps(); ipo++)
        {
            PlaneOpp *pPOpp = Objects3d::POppAt(ipo);
            for(int ic=0; ic<pPOpp->nCurves(); ic++)
            {
                if(pPOpp->m_Curve.at(ic)==pCurve)
                {
                    //this is the one which has been double clicked
                    LineStyle ls(pPOpp->theStyle());
                    LineMenu *pLineMenu = new LineMenu(nullptr);
                    pLineMenu->initMenu(ls);
                    pLineMenu->exec(QCursor::pos());
                    ls = pLineMenu->theStyle();
                    pPOpp->setLineStipple(ls.m_Stipple);
                    pPOpp->setLineWidth(ls.m_Width);
                    pPOpp->setLineColor(ls.m_Color);
                    pPOpp->setPointStyle(ls.m_Symbol);

                    m_bResetCurves = true;
                    m_pPlaneExplorer->setCurveParams();
                    updateView();
                    s_pMainFrame->m_pPOppTiles->update();
                    setControls();
                    return;
                }
            }
        }
    }
    else if(isPolarView() || isStabPolarView())
    {
        for(int iwp=0; iwp<Objects3d::nPolars(); iwp++)
        {
            PlanePolar *pWPolar=Objects3d::plPolarAt(iwp);
            for(int ic=0; ic<pWPolar->curveCount(); ic++)
            {
                if(pWPolar->curve(ic)==pCurve)
                {
                    //this is the one which has been clicked
                    LineStyle ls(pWPolar->theStyle());
                    LineMenu *pLineMenu = new LineMenu(nullptr);
                    pLineMenu->initMenu(ls);
                    pLineMenu->exec(QCursor::pos());
                    ls = pLineMenu->theStyle();
                    pWPolar->setLineStipple(ls.m_Stipple);
                    pWPolar->setLineWidth(ls.m_Width);
                    pWPolar->setLineColor(ls.m_Color);
                    pWPolar->setPointStyle(ls.m_Symbol);

                    m_pPlaneExplorer->setCurveParams();
                    m_bResetCurves = true;
                    updateView();
                    if(isPolarView())           s_pMainFrame->m_pWPolarTiles->update();
                    else if (isStabPolarView()) s_pMainFrame->m_pStabPolarTiles->update();
                    setControls();
                    return;
                }
            }
        }
    }
    else if(isStabTimeView())
    {
        if(Graph::isHighLighting())
        {
            QString curvename = pCurve->name();

            Curve *pSelCurve = m_TimeGraph.at(0)->curve(curvename, true);
            if(!pSelCurve) return;

            m_pStabTimeControls->selectCurve(pSelCurve);
            for(int i=0; i<m_TimeGraph.size(); i++)
            {
                m_TimeGraph.at(i)->clearSelection();
                Curve *pCurve = m_TimeGraph.at(i)->curve(pSelCurve->name());
                m_TimeGraph[i]->selectCurve(pCurve);
            }
            s_pMainFrame->m_pStabTimeTiles->makeLegend(true);
        }
        updateView();
    }
    else if(isCpView())
    {
        if(Graph::isHighLighting())
        {
            QString curvename = pCurve->name();

            Curve *pSelCurve = s_pMainFrame->m_pCpViewWt->CpGraph()->curve(curvename, true);
            if(!pSelCurve)
            {
                return;
            }
            s_pMainFrame->m_pCpViewWt->CpGraph()->clearSelection();
            s_pMainFrame->m_pCpViewWt->CpGraph()->selectCurve(pSelCurve);
            s_pMainFrame->m_pCpViewWt->makeLegend(true);
        }
        updateView();
    }
    setControls();
}


void XPlane::onUpdateMeshDisplay()
{
    m_pgl3dXPlaneView->resetglMesh();
    m_pgl3dXPlaneView->s_bResetglWake = true;
    updateView();
}


bool XPlane::isAnalysisRunning() const
{
    if(m_pPanelAnalysisDlg) return m_pPanelAnalysisDlg->isAnalysisRunning();
    return false;
}


void XPlane::onWingCurveSelection()
{
    WingSelDlg dlg(s_pMainFrame);

    dlg.exec();
    m_bResetCurves = true;
    updateView();
}


void XPlane::outputNodeProperties(int nodeindex, double pickedval)
{
    if(!m_pCurPlane) return;
    if(!m_pCurPlPolar) return;
    if(!m_pCurPlPolar->isTriLinearMethod()) return;

    QString strange, strong;

    // check index validity
    if(m_pCurPlPolar->isTriangleMethod())
    {
        if(nodeindex<0 || nodeindex>=m_pCurPlane->nNodes()) return;
    }

    Node const &node = m_pCurPlane->triMesh().node(nodeindex);

    if(m_pCurPlPolar)
    {
        strange = QString::fromStdString(node.properties()) + EOLch;
    }
    if(m_pCurPOpp && (m_pPOpp3dCtrls->m_b3dCp || m_pPOpp3dCtrls->m_bGamma || m_pPOpp3dCtrls->m_bPanelForce))
    {
        if(m_pPOpp3dCtrls->m_b3dCp)
        {
            strong = QString::asprintf("   Cp = %g\n", pickedval);
            strange += strong;
        }
        else if(m_pPOpp3dCtrls->m_bGamma)
        {
            strong = QString::asprintf("   Mu = %g\n", pickedval);
            strange += strong;
        }
    }
    strange += "\n";

    displayMessage(strange, true, false);
}


void XPlane::outputPanelProperties(int panelindex)
{
    if(!m_pCurPlane) return;
    std::string str;
    QString strange, strong;

    // check index validity
    if(m_pCurPlPolar->isTriangleMethod())
    {
        if(panelindex<0 || panelindex>=m_pCurPlane->nPanel3()) return;
    }
    else if(m_pCurPlPolar->isQuadMethod())
    {
        if(panelindex<0 || panelindex>=m_pCurPlane->nPanel4()) return;
    }

    bool bLong = false;
#ifdef QT_DEBUG
    bLong = true;
#endif
    if(m_pCurPlPolar)
    {
        if(m_pCurPlPolar->isTriangleMethod())
        {
            str = m_pCurPlane->panel3(panelindex).properties(bLong);
        }
        else if(m_pCurPlPolar->isQuadMethod() && m_pCurPlane->isXflType())
        {
            PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);
            str = pPlaneXfl->panel4(panelindex).properties(bLong);
        }
        strange = QString::fromStdString(str);
    }

    if(m_pCurPOpp && (m_pPOpp3dCtrls->m_b3dCp || m_pPOpp3dCtrls->m_bGamma || m_pPOpp3dCtrls->m_bPanelForce))
    {
        double q = 0.5*m_pCurPlPolar->density() *m_pCurPOpp->m_QInf*m_pCurPOpp->m_QInf;
        double cp(0), gamma(0);
        if(m_pCurPOpp->isTriangleMethod())
        {
            Panel3 const &p3 = m_pCurPlane->panel3At(panelindex);
            for(int in=0; in<3; in++)
            {
                cp    += m_pCurPOpp->Cp(p3.index()*3+in);
                gamma += m_pCurPOpp->gamma(p3.index()*3+in);
            }
            cp    /= 3.0;
            gamma /= 3.0;
        }
        else
        {
            cp    = m_pCurPOpp->Cp(panelindex);
            gamma = m_pCurPOpp->gamma(panelindex);
        }

        strong = QString::asprintf("  Gamma = %13g", gamma);
        strange += "\n" + strong + "\n";
        strong = QString::asprintf("  Cp    = %13g", cp);
        strange += strong + "\n";

        strong = QString::asprintf("  F/s   = %13g ", cp*q*Units::PatoUnit());
        strong += Units::pressureUnitQLabel() +"\n";
        strange += strong + "\n";
    }
    strange +="\n\n";

    displayMessage(strange, false, false);
}


void XPlane::onOpenAnalysisWindow()
{
    if(m_pCurPlPolar && m_pCurPlPolar->isLLTMethod())
        m_pLLTAnalysisDlg->show();
    else
        m_pPanelAnalysisDlg->show();
}


void XPlane::onMeshInfo()
{
    if(!m_pCurPlane || !m_pCurPlPolar) return;
    QString log;
    QString strange, strong;
    std::string str;

    log = QString::fromStdString(m_pCurPlane->name()) + EOLch;

    if(m_pCurPlPolar->isQuadMethod() && m_pCurPlane->isXflType())
    {
        PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);

        strong = "  Quad panels:";
        log += strong +"\n";
        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            WingXfl const*pWing = pPlaneXfl->wingAt(iw);
            strange = QString::fromStdString(pWing->name());
            strong = QString::asprintf("= %4d quads\n", pWing->nPanel4());
            log += "    " + strange + strong;
        }

        for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++)
        {
            Fuse const*pFuse = pPlaneXfl->fuseAt(ifuse);
            strange = QString::fromStdString(pFuse->name());
            strong = QString::asprintf("= %4d quads\n", pFuse->nPanel4());
            log += "    " + strange + strong;
        }
        pPlaneXfl->quadMesh().getMeshInfo(str);
        log += QString::fromStdString(str);
    }
    else if(m_pCurPlPolar->isTriangleMethod())
    {
        strong = "  Triangular panels:";
        log += strong +"\n";

        if(m_pCurPlane->isXflType())
        {
            PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);

            for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
            {
                WingXfl const*pWing = pPlaneXfl->wingAt(iw);
                strange = QString::fromStdString(pWing->name());
                strong = QString::asprintf("= %4d triangles\n", pWing->nPanel3());
                log += "    " + strange + strong;
            }

            for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++)
            {
                Fuse const*pFuse = pPlaneXfl->fuseAt(ifuse);
                strange = QString::fromStdString(pFuse->name());
                strong = QString::asprintf("= %4d triangles\n", pFuse->nPanel3());
                log += "    " + strange + strong;
            }
        }
        else
        {
            strange = QString::fromStdString(m_pCurPlane->name());
            strong = QString::asprintf("= %4d triangles\n", m_pCurPlane->nPanel3());
            log += "    " + strange + strong;
        }
        m_pCurPlane->triMesh().getMeshInfo(str);
        log += QString::fromStdString(str);
    }
    log += "\n";

    displayMessage(log, true, false);
}


void XPlane::onCheckPanels()
{
    m_pgl3dXPlaneView->clearHighlightList();

    if(!m_pCurPlPolar || !m_pCurPlane) return;

    m_pgl3dXPlaneView->m_bMeshPanels = true;
    m_pPOpp3dCtrls->checkPanels(m_pgl3dXPlaneView->m_bMeshPanels);
    m_pPOpp3dCtrls->m_b3dCp  = false;
    m_pPOpp3dCtrls->m_bGamma = false;
    m_pPOpp3dCtrls->m_pchCp->setChecked(false);
    m_pPOpp3dCtrls->m_pchGamma->setChecked(false);

    std::string log = "Checking panels\n";
    displayStdMessage(log, false, false);
    log.clear();
    PanelCheckDlg dlg(m_pCurPlPolar->isQuadMethod());
    int res = dlg.exec();
    bool bCheck = dlg.checkSkinny() || dlg.checkMinArea() || dlg.checkMinSize() || dlg.checkMinAngles() || dlg.checkMinQuadWarp();
    if(res!=QDialog::Accepted)
    {
        m_pgl3dXPlaneView->resetglMesh();
        return;
    }

    QVector<int> highpanellist = dlg.panelIndexes();
    m_pgl3dXPlaneView->appendHighlightList(highpanellist);
    for(int i3=0; i3<highpanellist.size(); i3++)
    {
        outputPanelProperties(highpanellist.at(i3));
    }

    QVector<int> highnodelist = dlg.nodeIndexes();
//    m_pgl3dXPlaneView->appendHighlightList(highpanellist);
    for(int in=0; in<highnodelist.size(); in++)
    {
        outputNodeProperties(highnodelist.at(in),0.0);
    }
    s_pMainFrame->onShowLogWindow(true);

    if(!bCheck)
    {
        m_pgl3dXPlaneView->resetglMesh();
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);


    // check Triangles
    if(m_pCurPlPolar->isTriangleMethod())
    {
        std::vector<int> skinnylist, anglelist, arealist, sizelist;

        m_pCurPlane->triMesh().checkPanels(log, dlg.checkSkinny(), dlg.checkMinAngles(), dlg.checkMinArea(), dlg.checkMinSize(),
                                           skinnylist, anglelist, arealist, sizelist,
                                           PanelCheckDlg::qualityFactor(), PanelCheckDlg::minAngle(), PanelCheckDlg::minArea(), PanelCheckDlg::minSize());

        QVector<int> qVec;
        if(dlg.checkSkinny())    qVec = QVector<int>(skinnylist.begin(), skinnylist.end());
        if(dlg.checkMinAngles()) qVec = QVector<int>(anglelist.begin(), anglelist.end());
        if(dlg.checkMinArea())   qVec = QVector<int>(arealist.begin(), arealist.end());
        if(dlg.checkMinSize())   qVec = QVector<int>(sizelist.begin(), sizelist.end());
        m_pgl3dXPlaneView->appendHighlightList(qVec);
    }

    // check Quads
    if(m_pCurPlPolar->isQuadMethod() && m_pCurPlane->isXflType())
    {
        PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl *>(m_pCurPlane);

        std::vector<int> anglelist, arealist, warplist;

        std::string str;
        pPlaneXfl->quadMesh().checkPanels(str, dlg.checkMinAngles(), dlg.checkMinArea(), dlg.checkMinQuadWarp(),
                                          anglelist, arealist, warplist,
                                          PanelCheckDlg::minAngle(), PanelCheckDlg::minArea(), PanelCheckDlg::maxQuadWarp());

        log += str;

        QVector<int> qVec;
        if      (dlg.checkMinAngles())   qVec = QVector<int>(anglelist.begin(), anglelist.end());
        else if (dlg.checkMinArea())     qVec = QVector<int>(arealist.begin(), arealist.end());
        else if (dlg.checkMinQuadWarp()) qVec = QVector<int>(warplist.begin(), warplist.end());

        m_pgl3dXPlaneView->appendHighlightList(qVec);
    }

    displayStdMessage(log + "\n\n", true, false);

    m_pgl3dXPlaneView->resetglMesh();
    updateView();
    QApplication::restoreOverrideCursor();
}


void XPlane::onConnectTriangles()
{
    if(!m_pCurPlane) return;
    if(!m_pCurPlPolar)
    {
        displayMessage("Connect panels: no Polar selected\n", true, false);
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    onClearHighlightSelection();

    m_pCurPlane->connectTriMesh(false, false, xfl::isMultiThreaded());

    QString log("\nTriangle connections done\n\n");
    displayMessage(log, false, true);

    QApplication::restoreOverrideCursor();

    m_pgl3dXPlaneView->resetglMesh();
    m_pgl3dXPlaneView->update();
}


void XPlane::onCheckFreeEdges()
{
    if(!m_pCurPlane || !m_pCurPlPolar) return;
    std::vector<Segment3d> freeedges;
    if(m_pCurPlPolar->isTriangleMethod())
        m_pCurPlane->triMesh().getFreeEdges(freeedges);
    else
    {
        if(m_pCurPlane->isXflType())
        {
            PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);
            std::vector<QPair<int, int>> pairerrors;
            pPlaneXfl->quadMesh().getFreeEdges(freeedges, pairerrors);
            if(pairerrors.size())
            {
                QString strange;
                for(uint i=0; i<pairerrors.size(); i++)
                {
                    strange += QString::asprintf("Connection error with quad panels %4d and %4d\n", pairerrors.at(i).first, pairerrors.at(i).second);
                }
                displayMessage(strange, true, false);
            }
        }
    }
    QVector<Segment3d> qVec = QVector<Segment3d>(freeedges.begin(), freeedges.end());
    m_pgl3dXPlaneView->setSegments(qVec);
    m_pgl3dXPlaneView->resetglMesh();
    m_pgl3dXPlaneView->update();
    QString strange;
    strange = QString::asprintf("Found %d free edges\n\n", int(freeedges.size()));
    displayMessage(strange, false, true);
}


void XPlane::onCenterViewOnPanel()
{
    if(!m_pCurPlane || !m_pCurPlPolar) return;

    IntValueDlg dlg(s_pMainFrame);
    dlg.setValue(-1);
    dlg.setLeftLabel("Panel index:");
    if(dlg.exec()==QDialog::Accepted)
    {
        int ip = dlg.value();
        if(m_pCurPlane->isXflType() && m_pCurPlPolar->isQuadMethod())
        {
            PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const *>(m_pCurPlane);
            if(ip>=0 && ip<pPlaneXfl->quadMesh().nPanels())
            {
                m_pgl3dXPlaneView->centerViewOn(pPlaneXfl->quadMesh().panelAt(ip).CoG());
                displayStdMessage(pPlaneXfl->quadMesh().panelAt(ip).properties(true)+"\n\n", false, false);
            }
        }
        else
        {
            if(ip>=0 && ip<m_pCurPlane->triMesh().nPanels())
            {
                m_pgl3dXPlaneView->centerViewOn(m_pCurPlane->triMesh().panelAt(ip).CoG());
                displayStdMessage(m_pCurPlane->triMesh().panelAt(ip).properties(true)+"\n\n", false, false);
            }
        }

        QVector<int> highlist = {ip};
        m_pgl3dXPlaneView->setHighlightList(highlist);
        m_pgl3dXPlaneView->resetglMesh();
    }
}


void XPlane::onClearHighlightSelection()
{
    m_pgl3dXPlaneView->clearHighlightList();
    m_pgl3dXPlaneView->clearSegments();
}


Plane *XPlane::duplicatePlane(Plane *pPlane) const
{
    if(!pPlane) return nullptr;
    Plane *pNewPlane =nullptr;
    if(pPlane->isXflType())
    {
        PlaneXfl* pNewPlaneXfl = new PlaneXfl;
        pNewPlaneXfl->duplicate(pPlane);
        pNewPlaneXfl->setInitialized(false); // may not be necessary
        pNewPlane = pNewPlaneXfl;
    }
    else if(pPlane->isSTLType())
    {
        PlaneSTL* pNewPlaneSTL = new PlaneSTL;
        pNewPlaneSTL->duplicate(pPlane);
        pNewPlane = pNewPlaneSTL;
    }
    else return nullptr;

    return Objects3d::setModifiedPlane(pNewPlane);
}


void XPlane::onResetWPolarCurves()
{
    m_bResetCurves=true;
    updateView();
}


void XPlane::cancelStreamLines()
{
    m_pPOpp3dCtrls->m_bStreamLines = false;
}


void XPlane::updateVisiblePanels(Plane *pPlane, PlanePolar const*pWPolar)
{
    if(!pPlane)  pPlane  = m_pCurPlane;
    if(!pWPolar) pWPolar = m_pCurPlPolar;

    if(!pWPolar)
    {
        std::vector<Panel3> Panel3Visible;
        std::vector<Panel4> Panel4Visible;
        m_pgl3dXPlaneView->setVisiblePanels(Panel3Visible);
        m_pgl3dXPlaneView->setVisiblePanels(Panel4Visible);
    }
    else
    {
        if( pWPolar->isQuadMethod() && pPlane->isXflType())
        {
            std::vector<Panel4> Panel4Visible;
            PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
            for(int ip=0; ip<pPlaneXfl->nParts(); ip++)
            {
                Part const *pPart = pPlaneXfl->partAt(ip);
                if(pPart && pPart->isVisible())
                {
                    int n0 = pPart->firstPanel4Index();
                    int n4 = pPart->nPanel4();
                    for(int i4=n0; i4<std::min(n0+n4, pPlane->nPanel4()); i4++) //bound in case the polar has the ignore fuse panel options
                    {
                        Panel4 const &p4 = pPlaneXfl->quadMesh().panelAt(i4);
                        Panel4Visible.push_back(p4);
                    }
                }
            }
            m_pgl3dXPlaneView->setVisiblePanels(Panel4Visible);
            if(!m_pXPlaneWt.isNull())
                m_pXPlaneWt->gl3dFloatView()->setVisiblePanels(Panel4Visible);
        }
        else if(pWPolar->isTriangleMethod())
        {
            std::vector<Panel3> Panel3Visible;
            QVector<int> VisibleNodeIndexes;
            PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const *>(pPlane);
            if(pPlaneXfl)
            {
                for(int ip=0; ip<pPlaneXfl->nParts(); ip++)
                {
                    Part const *pPart = pPlaneXfl->partAt(ip);
                    if(pPart && pPart->isVisible())
                    {
                        int n0 = pPart->firstPanel3Index();
                        int n3 = pPart->nPanel3();
                        for(int i3=n0; i3<std::min(n0+n3, pPlane->nPanel3()); i3++) //bound in case the polar has the ignore fuse panel options
                        {
                            Panel3 const &p3 = pPlane->triMesh().panelAt(i3);
                            Panel3Visible.push_back(p3);
                        }
                    }
                }
            }
            else
            {
                Panel3Visible = pPlane->triMesh().panels();
            }

            for(uint i3=0; i3<Panel3Visible.size(); i3++)
            {
                Panel3 const &p3 = Panel3Visible.at(i3);
                for(int in=0; in<3; in++)
                {
                    int idx = p3.nodeIndex(in);
                    if(!VisibleNodeIndexes.contains(idx)) VisibleNodeIndexes.append(idx);
                }
            }
            std::vector<Node> NodeVisible(VisibleNodeIndexes.size());
            for(int in=0; in<VisibleNodeIndexes.size(); in++)
            {
                NodeVisible[in] = pPlane->triMesh().nodeAt(VisibleNodeIndexes.at(in));
            }

            m_pgl3dXPlaneView->setVisiblePanels(Panel3Visible);
            m_pgl3dXPlaneView->setVisibleNodes(NodeVisible);
            if(!m_pXPlaneWt.isNull())
                m_pXPlaneWt->gl3dFloatView()->setVisiblePanels(Panel3Visible);
        }
    }
}


void XPlane::onNodeDistance()
{
    m_pgl3dXPlaneView->setPicking(m_pPOpp3dCtrls->getDistance() ? xfl::MESHNODE : xfl::NOPICK);
    if(!m_pPOpp3dCtrls->getDistance()) m_pgl3dXPlaneView->clearMeasure();
    m_pgl3dXPlaneView->setSurfacePick(xfl::NOSURFACE);
    m_pgl3dXPlaneView->update();
}


void XPlane::onPickedNode(int iNode)
{
    Node nsrc  = m_pCurPlane->node(iNode);
    QString strange;
    strange  = QString::asprintf("Node %5d: (%9g, %9g, %9g) ", iNode, nsrc.x*Units::mtoUnit(), nsrc.y*Units::mtoUnit(), nsrc.z*Units::mtoUnit());
    strange += Units::lengthUnitQLabel() + "\n";
    displayMessage(strange, true, false);
}


void XPlane::onPickedNodePair(QPair<int, int> nodepair)
{
    if(!m_pCurPlane) return;

    if(nodepair.first <0 || nodepair.first >=m_pCurPlane->nNodes()) return;
    if(nodepair.second<0 || nodepair.second>=m_pCurPlane->nNodes()) return;
    Node nsrc  = m_pCurPlane->node(nodepair.first);
    Node ndest = m_pCurPlane->node(nodepair.second);

    Segment3d seg(nsrc, ndest);
    m_pgl3dXPlaneView->setMeasure(seg);
    m_pgl3dXPlaneView->resetPickedNodes();
    m_pgl3dXPlaneView->update();

    QString strange;
    strange += QString::asprintf("   distance = %13g ", seg.length()*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n";
    strange += QString::asprintf("         dx = %13g ", seg.segment().x*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n";
    strange += QString::asprintf("         dy = %13g ", seg.segment().y*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n";
    strange += QString::asprintf("         dz = %13g ", seg.segment().z*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n\n";
    displayMessage(strange, true, false);
}


void XPlane::setLiveVortons(double alpha, const std::vector<std::vector<Vorton> > &vortons)
{
    bool b = m_pXPlaneWt && m_pXPlaneWt->isVisible();

    if(!is3dView() && !b) return;
    if(vortons.size()<=0) return;

    m_pgl3dXPlaneView->viewBuffers()->setLiveVortons(alpha, vortons);
    m_pgl3dXPlaneView->s_bResetglVortons = true;
    m_pgl3dXPlaneView->update();
    if(b)  m_pXPlaneWt->updateView();
}



void XPlane::onImportAnalysesFromXML()
{    
    QStringList PathNameList;
    PathNameList = QFileDialog::getOpenFileNames(s_pMainFrame, "Open XML file",
                                                 SaveOptions::xmlWPolarDirName(),
                                                 "Analysis XML file (*.xml)");
    if(!PathNameList.size()) return;

    PlanePolar *pWPolar(nullptr);
    QString strange;
    for(int iFile=0; iFile<PathNameList.size(); iFile++)
    {
        QFile XFile(PathNameList.at(iFile));
        pWPolar = importAnalysisFromXML(XFile);

        if(pWPolar)
        {
            m_pCurPOpp = nullptr;

            Plane *pPlane = Objects3d::plane(pWPolar->planeName());
            if(!pPlane) pPlane = m_pCurPlane;

            if(!pPlane)
            {
                strange = "The polar " + QString::fromStdString(pWPolar->name()) + " could not be associated to any plane... discarding\n";
                delete pWPolar;
                continue;
            }

            PlaneXfl const*pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

            if(pPlaneXfl)
            {
                std::string str;
                bool bMatch = pWPolar->checkFlaps(pPlaneXfl, str);

                strange += QString::fromStdString(str);

                if(!bMatch)
                {
                    pWPolar->resizeFlapCtrls(pPlaneXfl); // cleaning up import

                    strange += "The polar's flap settings have been resized to match the plane's number of flaps\n";
                    displayMessage(strange, true, false);
                }
            }

            strange = "Associating the imported polar " + QString::fromStdString(pWPolar->name() + "to the plane " + pPlane->name());
            displayMessage(strange, true, false);
            pWPolar = Objects3d::insertNewPolar(pWPolar, pPlane);
            m_pPlaneExplorer->insertWPolar(pWPolar);
        }
    }

    if(pWPolar)
    {
        setPolar(pWPolar);
        m_pPlaneExplorer->selectPlPolar(pWPolar, false);
    }

    emit projectModified();

    m_pgl3dXPlaneView->resetglGeom();
    m_pgl3dXPlaneView->resetglMesh();
    m_pgl3dXPlaneView->resetglPOpp();
    updateView();
}


PlanePolar *XPlane::importAnalysisFromXML(QFile &xmlFile)
{
    displayMessage("\n\n", false, false);
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        QString strange = "Could not open the file " + xmlFile.fileName() +"\n";
        displayMessage(strange, true, false);
        return nullptr;
    }

    QFileInfo fi(xmlFile);
    displayMessage("Importing xml file " + fi.fileName() + "\n", false, false);

    XmlPlanePolarReader wpolarreader(xmlFile);
    wpolarreader.readXMLPolarFile();

    if(wpolarreader.hasError())
    {
        QString errorMsg = wpolarreader.errorString() + QString("\nline %1 column %2").arg(wpolarreader.lineNumber()).arg(wpolarreader.columnNumber());
        errorMsg +="\n";
        displayMessage(errorMsg, true, false);
        return nullptr;
    }
    PlanePolar *pWPolar = wpolarreader.wpolar();

    Plane *pPlane = Objects3d::plane(pWPolar->planeName());
    if(!pPlane && m_pCurPlane)
    {
        displayMessage("Attaching the analysis to the active plane.\n", false, false);
        pWPolar->setPlaneName(m_pCurPlane->name());
        pPlane = m_pCurPlane;
    }
    else if(!pPlane)
    {
        QString strange;
        strange = QString::asprintf("No plane with the name %s\n", pWPolar->planeName().c_str());
        displayMessage(strange, true, false);
        delete pWPolar;
        return nullptr;
    }

    if(pWPolar->name().length()==0)
        pWPolar->setName(PlPolarNameMaker::makeName(pPlane, pWPolar).toStdString());

    PlaneXfl const*pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);
    if(pPlaneXfl)
    {
        for(int ie=0; ie<pWPolar->nAVLCtrls(); ie++)
            pWPolar->AVLCtrl(ie).resizeValues(pPlaneXfl->nAVLGains());
    }

    return pWPolar;
}


void XPlane::onOptim3d()
{
    PlaneXfl const*pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pCurPlane);

    OptimPlaneDlg o3d(s_pMainFrame);
    if(s_Objectives.isEmpty())
    {
        s_Objectives = {OptObjective("Cl",          0, true,   0.55, 0.005, xfl::EQUALIZE),
                        OptObjective("Cd",          1, true,   0.03, 0.000, xfl::MINIMIZE),
                        OptObjective("Cl/Cd",       2, false,  21.0, 0.000, xfl::MAXIMIZE),
                        OptObjective("Cl^(3/2)/Cd", 3, false,  20.0, 0.000, xfl::MAXIMIZE),
                        OptObjective("Cm",          4, false,  0.00, 0.010, xfl::EQUALIZE),
                        OptObjective("m.g.Vz",      5, false, 500.0, 0.000, xfl::MINIMIZE)};
    }

    o3d.setObjectives(s_Objectives);
    o3d.initDialog(pPlaneXfl);
    o3d.exec();
    if(o3d.bChanged())
    {
        emit projectModified();

        setPlane(o3d.bestPlane());

        m_pPlaneExplorer->fillModelView();
        m_pPlaneExplorer->update();
        m_pPlaneExplorer->selectPlane(o3d.bestPlane());

        m_pCurPlPolar = nullptr;
        m_pCurPOpp = nullptr;
        setPlane();

        m_pgl3dXPlaneView->resetglMesh();
        setControls();
    }

    s_Objectives = o3d.objectives();

    updateView();
}



void XPlane::displayStdMessage(std::string const &msg, bool bShowWindow, bool bStatusBar, int duration)
{
    s_pMainFrame->displayMessage(QString::fromStdString(msg), bShowWindow, bStatusBar, duration);
}


void XPlane::displayMessage(QString const &msg, bool bShowWindow, bool bStatusBar, int duration)
{
    s_pMainFrame->displayMessage(msg, bShowWindow, bStatusBar, duration);
}


void XPlane::exportMainDataToString(PlaneOpp const *pPOpp, Plane const*pPlane, QString &poppdata, xfl::enumTextFileType filetype, QString const &textsep) const
{
    QString strange;
    QString title;
    QString len = Units::lengthUnitQLabel();
    QString inertia = Units::inertiaUnitQLabel();

    QString sep = "  ";
    if(filetype==xfl::CSV) sep = textsep+ " ";


    poppdata += QString::fromStdString(pPOpp->planeName())+"\n";
    poppdata += QString::fromStdString(pPOpp->polarName())+"\n\n";
    poppdata +=   ALPHAch.rightJustified(17, ' ') + sep
                + BETAch.rightJustified(17, ' ') + sep
                + PHIch.rightJustified(17, ' ') + sep
                + QString("ctrl").rightJustified(17, ' ') + sep
                + QString("VInf("+Units::speedUnitQLabel()+")").rightJustified(17, ' ') +"\n";

    strange = QString::asprintf("%17g", pPOpp->m_Alpha);                     poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_Beta);                      poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_Phi);                       poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_Ctrl);                      poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_QInf*Units::mstoUnit());    poppdata += strange;

    poppdata += "\n\n";

    strange =   QString("CL").rightJustified(17, ' ')          + sep
              + QString("CX").rightJustified(17, ' ')          + sep
              + QString("CY").rightJustified(17, ' ')          + sep
              + QString("CD_inviscid").rightJustified(17, ' ') + sep
              + QString("CD_viscous").rightJustified(17, ' ')  + sep
              + QString("Cl").rightJustified(17, ' ')          + sep
              + QString("Cm_inviscid").rightJustified(17, ' ') + sep
              + QString("Cm_viscous").rightJustified(17, ' ')  + sep
              + QString("Cn_inviscid").rightJustified(17, ' ') + sep
              + QString("Cn_viscous").rightJustified(17, ' ') + "\n";
    poppdata += strange;

    strange = QString::asprintf("%17g", pPOpp->m_AF.CL());     poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_AF.CD());     poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_AF.Cy());     poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_AF.CDi());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_AF.CDv());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_AF.Cli());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_AF.Cmi());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_AF.Cmv());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_AF.Cni());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_AF.Cnv());    poppdata += strange;

    poppdata += "\n\n";

    strange = QString("CP.x("+len+")").rightJustified(17, ' ');         poppdata += strange+sep;
    strange = QString("CP.y("+len+")").rightJustified(17, ' ');         poppdata += strange+sep;
    strange = QString("CP.z("+len+")").rightJustified(17, ' ');         poppdata += strange+sep;
    strange = QString("NP.x("+len+")").rightJustified(17, ' ');         poppdata += strange+sep;
    poppdata += "\n";

    strange = QString::asprintf("%17g", pPOpp->m_AF.centreOfPressure().x*Units::mtoUnit());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_AF.centreOfPressure().y*Units::mtoUnit());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_AF.centreOfPressure().z*Units::mtoUnit());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_SD.XNP   *Units::mtoUnit());    poppdata += strange+sep;
    poppdata += "\n\n";

    strange = QString("mass("+Units::massUnitQLabel()+")").rightJustified(17, ' ');     poppdata += strange+sep;
    strange = QString("CoG.x("+len+")").rightJustified(17, ' ');                       poppdata += strange+sep;
    strange = QString("CoG.y("+len+")").rightJustified(17, ' ');                       poppdata += strange+sep;
    strange = QString("CoG.z("+len+")").rightJustified(17, ' ');                       poppdata += strange+sep;
    strange = QString("CoG_Ixx("+inertia+")").rightJustified(17, ' ');                 poppdata += strange+sep;
    strange = QString("CoG_Iyy("+inertia+")").rightJustified(17, ' ');                 poppdata += strange+sep;
    strange = QString("CoG_Izz("+inertia+")").rightJustified(17, ' ');                 poppdata += strange+sep;
    strange = QString("CoG_Ixz("+inertia+")").rightJustified(17, ' ');                 poppdata += strange+sep;
    poppdata +="\n";

    strange = QString::asprintf("%17g", pPOpp->m_Mass*Units::kgtoUnit());            poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_CoG.x*Units::mtoUnit());            poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_CoG.y*Units::mtoUnit());            poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_CoG.z*Units::mtoUnit());            poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_Inertia[0]*Units::kgm2toUnit());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_Inertia[1]*Units::kgm2toUnit());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_Inertia[2]*Units::kgm2toUnit());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", pPOpp->m_Inertia[3]*Units::kgm2toUnit());    poppdata += strange + "\n\n";

    if(pPOpp->isType7() || pPOpp->isType8())
    {
        //non dimensional stability derivatives
        strange  = QString("CXu").rightJustified(17, ' ') + sep;
        strange += QString("CZu").rightJustified(17, ' ') + sep;
        strange += QString("Cmu").rightJustified(17, ' ') + sep;
        strange += QString("CXa").rightJustified(17, ' ') + sep;
        strange += QString("CLa").rightJustified(17, ' ') + sep;
        strange += QString("Cma").rightJustified(17, ' ') + sep;
        strange += QString("CXq").rightJustified(17, ' ') + sep;
        strange += QString("CLq").rightJustified(17, ' ') + sep;
        strange += QString("Cmq").rightJustified(17, ' ') + sep;
        strange += QString("CYb").rightJustified(17, ' ') + sep;
        strange += QString("CYp").rightJustified(17, ' ') + sep;
        strange += QString("CYr").rightJustified(17, ' ') + sep;
        strange += QString("Clb").rightJustified(17, ' ') + sep;
        strange += QString("Clp").rightJustified(17, ' ') + sep;
        strange += QString("Clr").rightJustified(17, ' ') + sep;
        strange += QString("Cnb").rightJustified(17, ' ') + sep;
        strange += QString("Cnp").rightJustified(17, ' ') + sep;
        strange += QString("Cnr").rightJustified(17, ' ') + "\n";
        poppdata += strange;

        strange = QString::asprintf("%17g", pPOpp->m_SD.CXu);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.CZu);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.Cmu);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.CXa);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.CZa);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.Cma);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.CXq);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.CZq);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.Cmq);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.CYb);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.CYp);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.CYr);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.Clb);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.Clp);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.Clr);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.Cnb);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.Cnp);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", pPOpp->m_SD.Cnr);            poppdata += strange;
        poppdata += "\n";

        //non-dimensional control derivatives
        if(pPOpp->m_SD.ControlNames.size())
        {
            strange  = QString("CXe").rightJustified(17, ' ') + sep;
            strange += QString("CYe").rightJustified(17, ' ') + sep;
            strange += QString("CZe").rightJustified(17, ' ') + sep;
            strange += QString("CLe").rightJustified(17, ' ') + sep;
            strange += QString("CMe").rightJustified(17, ' ') + sep;
            strange += QString("CNe").rightJustified(17, ' ') +"\n";
            poppdata += strange;

            for(uint i=0; i<pPOpp->m_SD.ControlNames.size(); i++)
            {
                poppdata += QString::fromStdString(pPOpp->m_SD.ControlNames.at(i)) + "\n";
                strange = QString::asprintf("%17g", pPOpp->m_SD.CXe.at(i));            poppdata += strange+sep;
                strange = QString::asprintf("%17g", pPOpp->m_SD.CYe.at(i));            poppdata += strange+sep;
                strange = QString::asprintf("%17g", pPOpp->m_SD.CZe.at(i));            poppdata += strange+sep;
                strange = QString::asprintf("%17g", pPOpp->m_SD.CLe.at(i));            poppdata += strange+sep;
                strange = QString::asprintf("%17g", pPOpp->m_SD.CMe.at(i));            poppdata += strange+sep;
                strange = QString::asprintf("%17g", pPOpp->m_SD.CNe.at(i));            poppdata += strange;
            }
        }
    }
    poppdata += "\n\n";

    if(pPlane->isXflType())
    {
        PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            if(iw<pPOpp->nWOpps())
            {
                //if there are any flaps
                if(pPOpp->WOpp(iw).m_FlapMoment.size())
                {
                    poppdata += QString::fromStdString(pPlaneXfl->wingAt(iw)->name()) +"\n";
                    for (int l=0; l<pPOpp->WOpp(iw).m_nFlaps; l++)
                    {
                        strange = QString::asprintf("Flap_%d_moment=%11.5g ", l+1, pPOpp->WOpp(iw).m_FlapMoment.at(l)*Units::NmtoUnit());
                        strange += Units::momentUnitQLabel();
                        poppdata += strange +"\n";
                    }
                    poppdata += "\n";
                }
            }
        }
    }
    poppdata += "\n\n";

    title  = QString("y("+Units::lengthUnitQLabel()+")").rightJustified(17, ' ') + sep;
    title += QString("Re").rightJustified(17, ' ') + sep;
    title += QString("Ai").rightJustified(17, ' ') + sep;
    title += QString("Cd_i").rightJustified(17, ' ') + sep;
    title += QString("Cd_v").rightJustified(17, ' ') + sep;
    title += QString("Cl").rightJustified(17, ' ') + sep;
    title += QString("CP.x(%)").rightJustified(17, ' ') + sep;
    title += QString("Trans.top").rightJustified(17, ' ') + sep;
    title += QString("Trans.bot").rightJustified(17, ' ') + sep;
    title += QString("Cm_i").rightJustified(17, ' ') + sep;
    title += QString("Cm_v").rightJustified(17, ' ') + sep;
    title += QString("Bending.mom").rightJustified(17, ' ') + sep;
    title += QString("Vd.x").rightJustified(17, ' ') + sep;
    title += QString("Vd.y").rightJustified(17, ' ') + sep;
    title += QString("Vd.z").rightJustified(17, ' ') + sep;
    title += QString("F.x").rightJustified(17, ' ') + sep;
    title += QString("F.y").rightJustified(17, ' ') + sep;
    title += QString("F.z").rightJustified(17, ' ') + sep;
    title += GAMMAch.rightJustified(17, ' ');
    title += '\n';

    if(pPlane->isXflType())
    {
        PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            if(iw>=pPOpp->nWOpps()) break; // error somewhere

            WingOpp const &aWOpp = pPOpp->WOpp(iw);

            poppdata += QString::fromStdString(pPlaneXfl->wingAt(iw)->name())+'\n';
            poppdata += title;
            for(int i=0; i<aWOpp.m_NStation; i++)
            {
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_StripPos.at(i)*Units::mtoUnit());     poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_Re.at(i));             poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_Ai.at(i));             poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_ICd.at(i));            poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_PCd.at(i));            poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_Cl.at(i));             poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_XCPSpanRel.at(i));     poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_XTrTop.at(i));         poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_XTrBot.at(i));         poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_CmC4.at(i));           poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_CmViscous.at(i));      poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_BendingMoment.at(i));  poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_Vd.at(i).x);           poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_Vd.at(i).y);           poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_Vd.at(i).z);           poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_F.at(i).x);            poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_F.at(i).y);            poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_F.at(i).z);            poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_Gamma.at(i));          poppdata += strange+sep;

                poppdata += '\n';
            }
            poppdata += '\n';
        }
    }
}


void XPlane::exportPanel4DataToString(PlaneOpp const *pPOpp, Plane const *pPlane, PlanePolar const *pWPolar,
                                       xfl::enumTextFileType exporttype,
                                       QString &paneldata) const
{
    paneldata.clear();
    if(!pPlane->isXflType())
    {
        return;
    }

    if(!pWPolar->isQuadMethod()) return;

    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

    QString strong, Format;
    QTextStream out;
    out.setString(&paneldata);
    out << "Main Wing Cp Coefficients\n";

    int coef = 1;

    if(!pWPolar->bThinSurfaces())
    {
        coef = 2;
    }
    if(exporttype==xfl::TXT) out << " Panel     CtrlPt.x        CtrlPt.y        CtrlPt.z       Nx      Ny       Nz        Area       Cp\n";
    else                     out << "Panel,CtrlPt.x,CtrlPt.y,CtrlPt.z,Nx,Ny,Nz,Area,Cp\n";

    if(exporttype==xfl::TXT) Format = "%1     %2     %3     %4     %5     %6     %7     %8     %9\n";
    else                     Format = "%1, %2, %3, %4, %5, %6, %7, %8, %9\n";


    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        if(pPlaneXfl->wingAt(iw))
        {
            out << QString::fromStdString(pPlaneXfl->wingAt(iw)->name()) + "Cp Coefficients"+"\n";
            int p=0;
            int iStrip = 0;
            for (int j=0; j<pPlaneXfl->wingAt(iw)->nSurfaces(); j++)
            {
                Surface const & surf = pPlaneXfl->wingAt(iw)->surfaceAt(j);
                if(surf.isTipLeft() && !pWPolar->bThinSurfaces())
                {
                    while (pPlaneXfl->panel4(pPlaneXfl->wingAt(iw)->firstPanel4Index() + p).isSidePanel())
                        p++;
                }

                for(int k=0; k<surf.NYPanels(); k++)
                {
                    iStrip++;
                    strong = QString("Strip %1\n").arg(iStrip);
                    out << strong;

                    for(int l=0; l<surf.NXPanels() * coef; l++)
                    {
                        Panel4 const &p4 = pPlaneXfl->panel4(pPlaneXfl->wingAt(iw)->firstPanel4Index() + p);
                        strong = QString(Format).arg(p,7)
                                     .arg(p4.ctrlPt(pPOpp->isVLMMethod()).x,11,'f')
                                     .arg(p4.ctrlPt(pPOpp->isVLMMethod()).y,11,'f')
                                     .arg(p4.ctrlPt(pPOpp->isVLMMethod()).z,11,'f')
                                     .arg(p4.normal().x,11,'f')
                                     .arg(p4.normal().y,11,'f')
                                     .arg(p4.normal().z,11,'f')
                                     .arg(p4.area(),11,'f')
                                     .arg(pPOpp->WOpp(iw).m_dCp[p],11,'f');

                        out << strong;
                        p++;
                    }
                }
            }
        }
        out << ("\n\n");
    }
}


void XPlane::exportPanel3DataToString(PlaneOpp const *pPOpp, Plane const *pPlane, PlanePolar const *pWPolar,
                                        xfl::enumTextFileType exporttype, QString const &textsep,
                                        QString &paneldata) const
{

    paneldata.clear();
    if(!pPlane || !pWPolar) return;
    if(!pWPolar->isTriangleMethod()) return;

    QString sep = "  ";
    if(exporttype==xfl::CSV) sep = textsep + " ";

    QString strong, strange;
    QTextStream out;
    out.setString(&paneldata);

    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

    if(pPlaneXfl)
    {
        int coef = 1;
        if(!pWPolar->bThinSurfaces())  coef = 2;

        strange =   QString("Panel").rightJustified(17, ' ')        + sep
                  + QString("CtrlPt.x").rightJustified(17, ' ')     + sep
                  + QString("CtrlPt.y").rightJustified(17, ' ')     + sep
                  + QString("CtrlPt.z").rightJustified(17, ' ')     + sep
                  + QString("N.x").rightJustified(17, ' ')          + sep
                  + QString("N.y").rightJustified(17, ' ')          + sep
                  + QString("N.z").rightJustified(17, ' ')          + sep
                  + QString("Area").rightJustified(17, ' ')         + sep
                  + QString("Cp").rightJustified(17, ' ') + "\n";
        out << strange;

        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            out << QString::fromStdString(pPlaneXfl->wingAt(iw)->name()) + " - Cp Coefficients"+"\n";
            int p=0;
            int iStrip = 0;
            for (int j=0; j<pPlaneXfl->wingAt(iw)->nSurfaces(); j++)
            {
                Surface const & surf = pPlaneXfl->wingAt(iw)->surfaceAt(j);
                if(surf.isTipLeft() && !pWPolar->bThinSurfaces())
                {
                    while (pPlaneXfl->panel3At(pPlaneXfl->wingAt(iw)->firstPanel3Index() + p).isSidePanel())
                        p++;
                }

                for(int k=0; k<surf.NYPanels(); k++)
                {
                    iStrip++;
                    strong = QString("Strip %1\n").arg(iStrip);
                    out << strong;

                    for(int l=0; l<surf.NXPanels() * coef *2; l++)
                    {
                        Panel3 const &p3 = pPlaneXfl->panel3At(pPlaneXfl->wingAt(iw)->firstPanel3Index() + p);

                        double cp=0;
                        for(int in=0; in<3; in++) cp += pPOpp->m_Cp.at(p3.index()*3+in);
                        cp /= 3.0;

                        strong = QString::asprintf("%17d", p3.index())        +sep;
                        strong += QString::asprintf("%17g", p3.CoG().x)       +sep;
                        strong += QString::asprintf("%17g", p3.CoG().y)       +sep;
                        strong += QString::asprintf("%17g", p3.CoG().z)       +sep;
                        strong += QString::asprintf("%17g", p3.normal().x)    +sep;
                        strong += QString::asprintf("%17g", p3.normal().y)    +sep;
                        strong += QString::asprintf("%17g", p3.normal().z)    +sep;
                        strong += QString::asprintf("%17g", p3.area())        +sep;
                        strong += QString::asprintf("%17g", cp)               +"\n";

                        out << strong;
                        p++;
                    }
                }
            }
            out << ("\n\n");
        }

        for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++)
        {
            Fuse const *pFuse = pPlaneXfl->fuseAt(ifuse);
            out << QString::fromStdString(pFuse->name()) + " - Cp Coefficients"+"\n";

            for(int p=0; p<pFuse->nPanel3(); p++)
            {
                Panel3 const &p3 = pPlaneXfl->panel3At(pFuse->firstPanel3Index() + p);

                double cp=0;
                for(int in=0; in<3; in++) cp += pPOpp->m_Cp.at(p3.index()*3+in);
                cp /= 3.0;

                strong = QString::asprintf("%17d", p3.index())        +sep;
                strong += QString::asprintf("%17g", p3.CoG().x)       +sep;
                strong += QString::asprintf("%17g", p3.CoG().y)       +sep;
                strong += QString::asprintf("%17g", p3.CoG().z)       +sep;
                strong += QString::asprintf("%17g", p3.normal().x)    +sep;
                strong += QString::asprintf("%17g", p3.normal().y)    +sep;
                strong += QString::asprintf("%17g", p3.normal().z)    +sep;
                strong += QString::asprintf("%17g", p3.area())        +sep;
                strong += QString::asprintf("%17g", cp)               +"\n";


                out << strong;
            }
            out <<"\n\n";
        }
    }
    else
    {
        PlaneSTL const *pPlaneSTL = dynamic_cast<PlaneSTL const*>(pPlane);
        if(!pPlaneSTL) return;


        out << QString::fromStdString(pPlane->name()) + " - Cp Coefficients"+"\n";

        strange =   QString("Panel").rightJustified(17, ' ')        + sep
                  + QString("CtrlPt.x").rightJustified(17, ' ')     + sep
                  + QString("CtrlPt.y").rightJustified(17, ' ')     + sep
                  + QString("CtrlPt.z").rightJustified(17, ' ')     + sep
                  + QString("N.x").rightJustified(17, ' ')          + sep
                  + QString("N.y").rightJustified(17, ' ')          + sep
                  + QString("N.z").rightJustified(17, ' ')          + sep
                  + QString("Area").rightJustified(17, ' ')         + sep
                  + QString("Cp").rightJustified(17, ' ') + "\n";
        out << strange;



        for(int k=0; k<pPOpp->nPanel3(); k++)
        {
            Panel3 const &p3 = pPlaneSTL->panel3At(k);

            double cp=0;
            for(int in=0; in<3; in++) cp += pPOpp->m_Cp.at(p3.index()*3+in);
            cp /= 3.0;

            strong = QString::asprintf("%17d", p3.index())        +sep;
            strong += QString::asprintf("%17g", p3.CoG().x)       +sep;
            strong += QString::asprintf("%17g", p3.CoG().y)       +sep;
            strong += QString::asprintf("%17g", p3.CoG().z)       +sep;
            strong += QString::asprintf("%17g", p3.normal().x)    +sep;
            strong += QString::asprintf("%17g", p3.normal().y)    +sep;
            strong += QString::asprintf("%17g", p3.normal().z)    +sep;
            strong += QString::asprintf("%17g", p3.area())        +sep;
            strong += QString::asprintf("%17g", cp)               +"\n";


            out << strong;

        }

        out << ("\n\n");
    }
}
