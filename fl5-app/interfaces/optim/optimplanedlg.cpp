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


#include <QHeaderView>
#include <QtConcurrent/QtConcurrent>
#include <QGridLayout>
#include <QMenu>
#include <QAction>
#include <QMessageBox>

#include "optimplanedlg.h"



#include <api/constants.h>
#include <api/objects3d.h>
#include <api/panelanalysis.h>
#include <api/planeopp.h>
#include <api/planetask.h>
#include <api/planexfl.h>
#include <api/task3d.h>
#include <api/utils.h>
#include <api/planepolar.h>

#include <api/units.h>
#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/editors/analysis3ddef/t1234578polardlg.h>
#include <interfaces/editors/editobjectdelegate.h>
#include <interfaces/graphs/containers/graphwt.h>
#include <interfaces/graphs/controls/graphoptions.h>
#include <interfaces/opengl/fl5views/gl3dparetoview.h>
#include <interfaces/optim/psotask.h>
#include <interfaces/optim/psotaskplane.h>
#include <interfaces/widgets/customdlg/newnamedlg.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/customwts/xfldelegate.h>
#include <interfaces/widgets/customwts/xfltreeview.h>
#include <modules/xobjects.h>
#include <modules/xplane/xplane.h>
#include <test/test3d/gl3doptim2d.h>


QByteArray OptimPlaneDlg::s_Geometry;
int        OptimPlaneDlg::s_iActiveTab = 0;
double     OptimPlaneDlg::s_AlphaMin = 0.0;
double     OptimPlaneDlg::s_AlphaMax = 0.0;

QByteArray OptimPlaneDlg::s_HSplitterSizes;
QByteArray OptimPlaneDlg::s_LeftVSplitterSizes;
QByteArray OptimPlaneDlg::s_RightVSplitterSizes;

OptimPlaneDlg::OptimPlaneDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Optimization 3d");

    m_bChanged = false;
    m_bSaved   = true;

    m_bResetSwarm  = true;
    m_bResetPareto = true;

    m_pPSOTask   = nullptr;
    m_pPlaneXfl  = nullptr;
    m_pBestPlane = nullptr;

    m_IterTotal = 0;
    m_ParticleCounter = 0;

    m_pgl3dPareto = nullptr;

    if(m_Objective.isEmpty())
    {
        m_Objective = { OptObjective("Cl",          0, true,     0.70,  0.005, xfl::EQUALIZE),
                        OptObjective("Cd",          1, true,     0.025, 0.001, xfl::MINIMIZE),
                        OptObjective("Cl/Cd",       2, false,    21.0,  0.100, xfl::MAXIMIZE),
                        OptObjective("Cl^(3/2)/Cd", 3, false,    20.0,  0.100, xfl::MAXIMIZE),
                        OptObjective("Cm",          4, false,     0.0,  0.010, xfl::EQUALIZE),
                        OptObjective("m.g.Vz",      5, false,   500.0,  0.000, xfl::MINIMIZE),
                        OptObjective("BM",          6, false,     1.0,  0.100, xfl::MINIMIZE)
                      };
    }

    makeCommonWt();
    setupLayout();
    connectSignals();
}


OptimPlaneDlg::~OptimPlaneDlg()
{
    if(m_pPSOTask) delete m_pPSOTask;
    m_pPSOTask = nullptr;

    m_ParetoGraph.deleteCurveModel();
    m_VariableGraph.deleteCurveModel();

    for(int io=0; io<NOBJECTIVES; io++)
        m_ObjGraph[io].deleteCurveModel();
}



void OptimPlaneDlg::setupLayout()
{
    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        m_pHSplitter = new QSplitter;
        {
            m_pHSplitter->setChildrenCollapsible(false);

            m_pLeftVSplitter = new QSplitter(Qt::Vertical);
            {
                m_pLeftVSplitter->setChildrenCollapsible(false);

                m_ptwControls = new QTabWidget;
                {
                    // add the plane specific tabs to the layout
                    m_plwPlanes = new QListWidget;

                    m_pVarFrame = new QFrame;
                    {
                        QVBoxLayout *pVarLayout = new QVBoxLayout;
                        {
                            m_ptvPlane = new XflTreeView;
                            {
                                m_ptvPlane->setEditTriggers(QAbstractItemView::AllEditTriggers);
                                m_pVariableModel = new QStandardItemModel(this);
                                m_pVariableModel->setColumnCount(4);
                                m_pVariableModel->setHeaderData(0, Qt::Horizontal, "Variable");
                                m_pVariableModel->setHeaderData(1, Qt::Horizontal, "Min.");
                                m_pVariableModel->setHeaderData(2, Qt::Horizontal, "Max.");
                                m_pVariableModel->setHeaderData(3, Qt::Horizontal, "Unit");
                                for(int icol=0; icol<m_pVariableModel->columnCount(); icol++)
                                    m_pVariableModel->setHeaderData(icol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
                                m_ptvPlane->setModel(m_pVariableModel);

                                m_pVariableDelegate = new EditObjectDelegate(this);
                                m_ptvPlane->setItemDelegate(m_pVariableDelegate);
                            }
                            m_ppbResetVariables = new QPushButton("Reset variables");

                            pVarLayout->addWidget(m_ptvPlane);
                            pVarLayout->addWidget(m_ppbResetVariables);
                        }
                        m_pVarFrame->setLayout(pVarLayout);
                    }

                    m_pAnalysisFrame = new QFrame;
                    {
                        QVBoxLayout *pAnalysisLayout = new QVBoxLayout;
                        {
                            m_pptePolar = new QPlainTextEdit;
                            m_pptePolar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                            m_pptePolar->setFont(DisplayOptions::tableFont());
                            m_ppbAnalysisDef = new QPushButton("Define analysis");
                            m_ppbRunAnalysis = new QPushButton("Run analysis");
                            pAnalysisLayout->addWidget(m_pptePolar);
                            pAnalysisLayout->addWidget(m_ppbAnalysisDef);
                            pAnalysisLayout->addWidget(m_ppbRunAnalysis);
                        }
                        m_pAnalysisFrame->setLayout(pAnalysisLayout);
                    }

                    QFrame *pOptimizerFrame = new QFrame;
                    {
                        QVBoxLayout *pOptimizerLayout = new QVBoxLayout;
                        {
                            m_pcptObjective = new CPTableView;
                            {
                                m_pcptObjective->setEditable(true);
                                m_pObjModel = new QStandardItemModel(this);
                                m_pObjModel->setRowCount(1);//temporary
                                m_pObjModel->setColumnCount(5);
                                m_pObjModel->setHeaderData(0, Qt::Horizontal, "Active");
                                m_pObjModel->setHeaderData(1, Qt::Horizontal, "Objective");
                                m_pObjModel->setHeaderData(2, Qt::Horizontal, "Type");
                                m_pObjModel->setHeaderData(3, Qt::Horizontal, "Target");
                                m_pObjModel->setHeaderData(4, Qt::Horizontal, "Max. error");
                                for(int icol=0; icol<m_pObjModel->columnCount(); icol++)
                                    m_pObjModel->setHeaderData(icol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
                                m_pcptObjective->setModel(m_pObjModel);

                                m_pObjDelegate = new XflDelegate(this);
                                m_pObjDelegate->setCheckColumn(0);
                                m_pObjDelegate->setActionColumn(-1);
                                m_pObjDelegate->setDigits({0,-1,0,3,3});
                                m_pObjDelegate->setItemTypes({XflDelegate::CHECKBOX, XflDelegate::STRING,
                                                              XflDelegate::INTEGER, XflDelegate::DOUBLE, XflDelegate::DOUBLE});
                                m_pObjDelegate->setName("Objectives");
                                m_pcptObjective->setItemDelegate(m_pObjDelegate);
                            }

                            pOptimizerLayout->addWidget(m_pcptObjective);
                            pOptimizerLayout->addWidget(m_ppbMakeSwarm);
                            pOptimizerLayout->addWidget(m_ppbSwarm);
                            pOptimizerLayout->addWidget(m_ppbStoreBest);
                            pOptimizerLayout->addWidget(m_ppbContinueBest);
                        }
                        pOptimizerFrame->setLayout(pOptimizerLayout);
                    }
                    m_ptwControls->addTab(m_plwPlanes,       tr("Plane"));
                    m_ptwControls->addTab(m_pAnalysisFrame,  tr("Analysis"));
                    m_ptwControls->addTab(m_pVarFrame,       tr("Variables"));
                    m_ptwControls->addTab(m_pPSOFrame,       tr("PSO"));
                    m_ptwControls->addTab(pOptimizerFrame,   tr("Optimizer"));
                }

                QFrame *pCtrlFrame = new QFrame;
                {
                    QVBoxLayout *pCtrlLayout = new QVBoxLayout;
                    {
                        pCtrlLayout->addWidget(m_ppto);
                        pCtrlLayout->addWidget(m_pButtonBox);
                    }
                    pCtrlFrame->setLayout(pCtrlLayout);
                }

                m_pLeftVSplitter->addWidget(m_ptwControls);
                m_pLeftVSplitter->addWidget(pCtrlFrame);
            }

            m_pRightVSplitter = new QSplitter(Qt::Vertical);
            {
                m_pRightVSplitter->setChildrenCollapsible(false);

                QFrame *pObjGraphFrame = new QFrame;
                {
                    QVBoxLayout *pObjgraphLayout = new QVBoxLayout;
                    {
                        for(int io=0; io<NOBJECTIVES; io++)
                        {
                            m_pObjGraphWt[io] = new GraphWt;
                            m_pObjGraphWt[io]->setGraph(&m_ObjGraph[io]);

                            m_ObjGraph[io].setCurveModel(new CurveModel);
                            m_ObjGraph[io].setXVarStdList({"Iter."});
                            m_ObjGraph[io].setYVarStdList({m_Objective.at(io).m_Name});
                            m_ObjGraph[io].setYRange(0, 0.0, 0.100);
                            m_ObjGraph[io].setScaleType(GRAPH::EXPANDING);
                            GraphOptions::resetGraphSettings(m_ObjGraph[io]);
                        }

                        for(int io=0; io<NOBJECTIVES; io++)
                            pObjgraphLayout->addWidget(m_pObjGraphWt[io]);

                        pObjGraphFrame->setLayout(pObjgraphLayout);
                    }
                }

                m_ptwPareto = new QTabWidget;
                {
                    m_pVariableGraphWt = new GraphWt;
                    m_pVariableGraphWt->setGraph(&m_VariableGraph);
                    m_pVariableGraphWt->showLegend(true);
                    m_pVariableGraphWt->setLegendPosition(Qt::AlignTop | Qt::AlignHCenter);

                    m_VariableGraph.setCurveModel(new CurveModel);
                    m_VariableGraph.setXVariableList({"Iter."});
                    m_VariableGraph.setYVariableList({"Variable range [0-100%]"});
                    m_VariableGraph.setYRange(0, 0.0, 100.0);
                    m_VariableGraph.setScaleType(GRAPH::EXPANDING);
                    GraphOptions::resetGraphSettings(m_VariableGraph);

                    m_ptwPareto->addTab(m_pVariableGraphWt, tr("Variables"));
                    m_ptwPareto->addTab(m_pParetoGraphWt, tr("Pareto graph"));
                    m_ptwPareto->addTab(m_pcptResults, tr("Frontier particles"));
                }
                m_pRightVSplitter->addWidget(pObjGraphFrame);
                m_pRightVSplitter->addWidget(m_ptwPareto);
            }

            m_pHSplitter->addWidget(m_pLeftVSplitter);
            m_pHSplitter->addWidget(m_pRightVSplitter);
        }
        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);

    m_pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/MOPSO/MOPSO.html>https://flow5.tech/docs/flow5_doc/MOPSO/MOSPSO.html</a>");

}



void OptimPlaneDlg::makeCommonWt()
{
    m_pPSOFrame = new QFrame;
    {
        QGridLayout *pSwarmLayout = new QGridLayout;
        {
            QLabel *plabPopSize = new QLabel("Swarm size:");
            m_pieSwarmSize = new IntEdit(PSOTask::s_PopSize);

            QLabel *plabArchiveSize = new QLabel("Max. Pareto size:");
            m_pieArchiveSize = new IntEdit(PSOTask::s_ArchiveSize);
            m_pieArchiveSize->setToolTip("The maximum size of the Pareto frontier");

            QLabel *plabInertia = new QLabel("Inertia weight:");
            m_pdeInertiaWeight = new FloatEdit(PSOTask::s_InertiaWeight);
            m_pdeInertiaWeight->setToolTip("<p>The inertia weight determines the influence of the particle's "
                                           "current velocity on its updated velocity.<br>"
                                           "Recommendation: 0.3</p>");

            QLabel *plabCognitive = new QLabel("Cognitive weight:");
            m_pdeCognitiveWeight = new FloatEdit(PSOTask::s_CognitiveWeight);
            m_pdeCognitiveWeight->setToolTip("<p>The cognitive weight determines the influence of the particle's best position.<br>"
                                             "Recommendation: 0.7</p>");

            QLabel *plabSocial = new QLabel("Social weight:");
            m_pdeSocialWeight = new FloatEdit(PSOTask::s_SocialWeight);
            m_pdeSocialWeight->setToolTip("<p>The social weight determines the influence of the global best-known position.<br>"
                                          "Recommendation: 0.7</p<");

            QLabel *pLabRegen = new QLabel("Regeneration probability:");
            m_pdePropRegenerate = new FloatEdit(PSOTask::s_ProbRegenerate*100.0);
            m_pdePropRegenerate->setRange(0.0, 100.0);
            m_pdePropRegenerate->setToolTip("<p>The probability that a particle will be re-created at a random position at each iteration.<br>"
                                        "Increases the likelyhood that the swarm will not get stuck on a local minimum.<br>"
                                        "Recommendation: 5% to 25%</p>");
            QLabel *pLabPercent = new QLabel("%");

            QLabel *plabMaxIter = new QLabel("Max. iterations:");
            m_pieMaxIter = new IntEdit(PSOTask::s_MaxIter);

            m_ppbRestoreDefault = new QPushButton("Restore defaults");


            m_pchMultiThread = new QCheckBox("Multi-threaded");
            m_pchMultiThread->setChecked(PSOTask::s_bMultiThreaded);

            QLabel *pFlow5Link = new QLabel;
            pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/MOPSO/MOPSO.html>https://flow5.tech/docs/flow5_doc/MOPSO/MOPSO.html</a>");
            pFlow5Link->setOpenExternalLinks(true);
            pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);


            pSwarmLayout->addWidget(plabPopSize,          1, 1);
            pSwarmLayout->addWidget(m_pieSwarmSize,       1, 2);

            pSwarmLayout->addWidget(plabArchiveSize,      2, 1);
            pSwarmLayout->addWidget(m_pieArchiveSize,     2, 2);

            pSwarmLayout->addWidget(plabInertia,          3, 1);
            pSwarmLayout->addWidget(m_pdeInertiaWeight,   3, 2);

            pSwarmLayout->addWidget(plabCognitive,        4, 1);
            pSwarmLayout->addWidget(m_pdeCognitiveWeight, 4, 2);

            pSwarmLayout->addWidget(plabSocial,           5, 1);
            pSwarmLayout->addWidget(m_pdeSocialWeight,    5, 2);

            pSwarmLayout->addWidget(pLabRegen,            6, 1);
            pSwarmLayout->addWidget(m_pdePropRegenerate,  6, 2);
            pSwarmLayout->addWidget(pLabPercent,          6, 3);

            pSwarmLayout->addWidget(plabMaxIter,          7, 1);
            pSwarmLayout->addWidget(m_pieMaxIter,         7, 2);

            pSwarmLayout->addWidget(m_ppbRestoreDefault,  8, 2);

            pSwarmLayout->addWidget(m_pchMultiThread,     9,1,1,3);

            pSwarmLayout->addWidget(pFlow5Link,           11,1,1,2);

            pSwarmLayout->setRowStretch(10,1);
        }
        m_pPSOFrame->setLayout(pSwarmLayout);
    }

    //PARETO
    m_pcptResults = new CPTableView;
    {
//        m_pcptResults->horizontalHeader()->setStretchLastSection(true);
        m_pcptResults->horizontalHeader()->setSectionsClickable(true);
        m_pcptResults->horizontalHeader()->setSortIndicatorShown(true);

        m_pcptResults->setEditable(true);
        m_pResultsModel = new ActionItemModel(this);
        m_pResultsModel->setRowCount(1);//temporary
        m_pResultsModel->setColumnCount(2); // temporary
        m_pResultsModel->setHeaderData(0, Qt::Horizontal, "Particle");
        m_pResultsModel->setHeaderData(1, Qt::Horizontal, "Action");
        m_pResultsModel->setActionColumn(1); //temporary

        m_pcptResults->setModel(m_pResultsModel);
        connect(m_pcptResults->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), SLOT(onSortColumn(int,Qt::SortOrder)));

        m_pResultsDelegate = new XflDelegate(this);
        m_pResultsDelegate->setCheckColumn(-1); //temporary
        m_pResultsDelegate->setActionColumn(1); //temporary
        m_pResultsDelegate->setDigits({-1,-1,3,3}); //temporary
        m_pResultsDelegate->setItemTypes({XflDelegate::STRING, XflDelegate::CHECKBOX, XflDelegate::DOUBLE, XflDelegate::DOUBLE});
        m_pResultsDelegate->setName("Pareto results");
        m_pcptResults->setItemDelegate(m_pResultsDelegate);
    }

    m_pParetoGraphWt = new GraphWt;
    m_pParetoGraphWt->setGraph(&m_ParetoGraph);
    m_pParetoGraphWt->enableContextMenu(true);
    m_pParetoGraphWt->showLegend(true);
    m_pParetoGraphWt->setLegendPosition(Qt::AlignTop | Qt::AlignHCenter);
    m_pParetoGraphWt->setWindowTitle("Pareto graph");

    m_ParetoGraph.setName("Pareto graph");
    m_ParetoGraph.setCurveModel(new CurveModel);
    m_ParetoGraph.setXVariableList({"Obj0"});
    m_ParetoGraph.setYVariableList({"Obj1"});
    m_ParetoGraph.setScaleType(GRAPH::EXPANDING);
    GraphOptions::resetGraphSettings(m_ParetoGraph);

    m_ppbMakeSwarm = new QPushButton("Make random swarm");
    m_ppbSwarm = new QPushButton("Swarm");
    m_ppbStoreBest= new QPushButton("Store best");
    m_ppbStoreBest->setToolTip("Adds the current best to the database");
    m_ppbContinueBest = new QPushButton("Continue from current best");
    m_ppbContinueBest->setToolTip("Uses the current best plane as the basis for further optimization");

    m_pFlow5Link = new QLabel;
    m_pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/MOPSO/3d_inverse.html#StepByStep>https://flow5.tech/docs/flow5_doc/MOPSO/3d_inverse.html</a>");
    m_pFlow5Link->setOpenExternalLinks(true);
    m_pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);


    m_ppto = new PlainTextOutput;

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    {
        m_ppbMenuBtn = new QPushButton("Actions");
        {
            QMenu *pMenu = new QMenu("Actions", this);
            {
                 QAction *p2dDemo = new QAction("2d single-objective demo", this);
                 p2dDemo->setShortcut(Qt::Key_F11);
                 connect(p2dDemo, SIGNAL(triggered()), SLOT(on2dDemo()));

                 QAction *pResetParetoFrontier = new QAction("Reset Pareto frontier", this);
                 connect(pResetParetoFrontier,  SIGNAL(triggered()), SLOT(onResetParetoFrontier()));

                 pMenu->addAction(p2dDemo);
                 pMenu->addSeparator();
                 pMenu->addAction(pResetParetoFrontier);
            }
            m_ppbMenuBtn->setMenu(pMenu);
        }
        m_pButtonBox->addButton(m_ppbMenuBtn, QDialogButtonBox::ActionRole);

        QPushButton *ppbClear = new QPushButton("Clear output");
        connect(ppbClear, SIGNAL(clicked()), m_ppto, SLOT(clear()));
        m_pButtonBox->addButton(ppbClear, QDialogButtonBox::ActionRole);

        QAction *pTestAction = new QAction("Test", this);
        m_pButtonBox->addAction(pTestAction);

        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }
}


void OptimPlaneDlg::connectSignals()
{
    connect(m_ppbMakeSwarm,      SIGNAL(clicked()),               SLOT(onMakeSwarm()));
    connect(m_ppbSwarm,          SIGNAL(clicked()),               SLOT(onSwarm()));
    connect(m_ppbStoreBest    ,  SIGNAL(clicked()),               SLOT(onStoreBest()));
    connect(m_ppbContinueBest,   SIGNAL(clicked()),               SLOT(onContinueBest()));
    connect(m_ppbRestoreDefault, SIGNAL(clicked()),               SLOT(onRestorePSODefaults()));
    connect(m_ptwControls,       SIGNAL(currentChanged(int)),     SLOT(onResizeColumns()));
    connect(m_pcptResults,       SIGNAL(clicked(QModelIndex)),    SLOT(onActionResultClicked(QModelIndex)));
    connect(m_pieSwarmSize,      SIGNAL(intChanged(int)),         SLOT(invalidateSwarm()));

    connect(m_pVariableModel,    SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(onVariableChanged(QModelIndex,QModelIndex)));
    connect(m_ppbResetVariables,   SIGNAL(clicked()),                      SLOT(onResetOptVariables()));
    connect(m_pcptObjective,     SIGNAL(pressed(QModelIndex)),           SLOT(onObjTableClicked(QModelIndex)));
    connect(m_pObjModel,         SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(onObjectiveChanged()));
    connect(m_plwPlanes,         SIGNAL(itemClicked(QListWidgetItem*)),  SLOT(onPlaneSelected(QListWidgetItem*)));
    connect(m_pHSplitter,        SIGNAL(splitterMoved(int,int)),         SLOT(onResizeColumns()));
    connect(m_ppbAnalysisDef,    SIGNAL(clicked()),                      SLOT(onAnalysisDef()));
    connect(m_ppbRunAnalysis,    SIGNAL(clicked()),                      SLOT(onRunAnalysis()));
    connect(m_ptwPareto,         SIGNAL(currentChanged(int)),            SLOT(onResizeColumns()));
}


void OptimPlaneDlg::on2dDemo()
{
    gl3dOptim2d *p2d = new gl3dOptim2d;
    p2d->setWindowModality(Qt::WindowModal);
    p2d->show();
}


void OptimPlaneDlg::onRestorePSODefaults()
{
    PSOTask::restoreDefaults();

    m_pieSwarmSize->setValue(        PSOTask::s_PopSize);
    m_pieArchiveSize->setValue(    PSOTask::s_ArchiveSize);
    m_pieMaxIter->setValue(        PSOTask::s_MaxIter);
    m_pdeInertiaWeight->setValue(  PSOTask::s_InertiaWeight);
    m_pdeCognitiveWeight->setValue(PSOTask::s_CognitiveWeight);
    m_pdeSocialWeight->setValue(   PSOTask::s_SocialWeight);
    m_pdePropRegenerate->setValue( PSOTask::s_ProbRegenerate*100.0);
}


void OptimPlaneDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
                return;
            }
            else
            {
                accept();
                return;
            }
            break;
        }
        case Qt::Key_Escape:
        {
            if(m_pPSOTask)
            {
                m_pPSOTask->cancelAnalyis();
                return;
                break;
            }
            break;
        }
        default:
            pEvent->ignore();
    }
    QDialog::keyPressEvent(pEvent);
}


void OptimPlaneDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    if(s_LeftVSplitterSizes.length()>0) m_pLeftVSplitter->restoreState(s_LeftVSplitterSizes);
    if(s_RightVSplitterSizes.length()>0) m_pRightVSplitter->restoreState(s_RightVSplitterSizes);

    onResizeColumns();
}


void OptimPlaneDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
    s_HSplitterSizes      = m_pHSplitter->saveState();
    s_LeftVSplitterSizes  = m_pLeftVSplitter->saveState();
    s_RightVSplitterSizes = m_pRightVSplitter->saveState();
    m_pParetoGraphWt->hide();
    readData();
    readObjectives();

    int nActive=0;
    QString strange, prefix;
    readVariables(nActive, strange, prefix); // fills the OptVariable vector
}


void OptimPlaneDlg::resizeEvent(QResizeEvent *pEvent)
{
    onResizeColumns();
    QDialog::resizeEvent(pEvent);
}


void OptimPlaneDlg::onResizeColumns()
{
//    int nCols = m_pResultsModel->columnCount()-1;
    QHeaderView *pHHeader = m_pcptResults->horizontalHeader();
    pHHeader->setSectionResizeMode(QHeaderView::Interactive);
    pHHeader->setStretchLastSection(true);
//    pHHeader->resizeSection(nCols, 1); // 1 pixel to be resized automatically
    double wt = double(m_pcptResults->width())*0.85;
    int wfrac = int(wt/m_pResultsModel->columnCount()-1);
    for(int icol=0; icol<m_pResultsModel->columnCount()-1; icol++)
        m_pcptResults->setColumnWidth(icol, wfrac);

    int nCols=m_pObjModel->columnCount()-1;
    pHHeader = m_pcptObjective->horizontalHeader();
    pHHeader->setSectionResizeMode(nCols, QHeaderView::Stretch);
    pHHeader->resizeSection(nCols, 1); // 1 pixel to be resized automatically
    wt = double(m_pcptObjective->width());
    int wch = int(wt/10.0);
    int w = int((wt-wch)/double(3)*0.85);
    m_pcptObjective->setColumnWidth(0, wch);
    m_pcptObjective->setColumnWidth(1, w);
    m_pcptObjective->setColumnWidth(2, wch);
    m_pcptObjective->setColumnWidth(3, w);
    m_pcptObjective->setColumnWidth(4, w);

    pHHeader = m_ptvPlane->header();
    pHHeader->setSectionResizeMode(3, QHeaderView::Stretch);
    double dw = double(m_ptvPlane->width())/100.0;
    m_ptvPlane->setColumnWidth(0, int(dw*40.0));
    m_ptvPlane->setColumnWidth(1, int(dw*20.0));
    m_ptvPlane->setColumnWidth(2, int(dw*20.0));
}


void OptimPlaneDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("Optim3d");
    {
        PSOTask::s_PopSize         = settings.value("PopSize",         PSOTask::s_PopSize).toInt();
        PSOTask::s_MaxIter         = settings.value("MaxIter",         PSOTask::s_MaxIter).toInt();
        PSOTask::s_bMultiThreaded  = settings.value("Multithreaded",   PSOTask::s_bMultiThreaded).toBool();

        PSOTask::s_CognitiveWeight = settings.value("CognitiveWeight", PSOTask::s_CognitiveWeight).toDouble();
        PSOTask::s_SocialWeight    = settings.value("SocialWeight",    PSOTask::s_SocialWeight).toDouble();
        PSOTask::s_ArchiveSize     = settings.value("ArchiveSize",     PSOTask::s_ArchiveSize).toInt();

        s_HSplitterSizes      = settings.value("HSplitterSizes",  QByteArray()).toByteArray();
        s_LeftVSplitterSizes  = settings.value("LeftVSplitterSizes",  QByteArray()).toByteArray();
        s_RightVSplitterSizes = settings.value("RightVSplitterSizes",  QByteArray()).toByteArray();
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void OptimPlaneDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("Optim3d");
    {
        settings.setValue("PopSize",         PSOTask::s_PopSize);
        settings.setValue("MaxIter",         PSOTask::s_MaxIter);
        settings.setValue("Multithreaded",   PSOTask::s_bMultiThreaded);

        settings.setValue("InertiaWeight",   PSOTask::s_InertiaWeight);
        settings.setValue("CognitiveWeight", PSOTask::s_CognitiveWeight);
        settings.setValue("SocialWeight",    PSOTask::s_SocialWeight);
        settings.setValue("ArchiveSize",     PSOTask::s_ArchiveSize);

        settings.setValue("HSplitterSizes",      s_HSplitterSizes);
        settings.setValue("LeftVSplitterSizes",  s_LeftVSplitterSizes);
        settings.setValue("RightVSplitterSizes", s_RightVSplitterSizes);
        settings.setValue("WindowGeom",          s_Geometry);
    }
    settings.endGroup();
}


void OptimPlaneDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)
        onClose();
}


void OptimPlaneDlg::reject()
{
    onClose();
}


void OptimPlaneDlg::onClose()
{
    if(m_pPSOTask && m_pPSOTask->isRunning())
    {
        onOutputMessage("\nCannot exit when a task is running\n\n");
        return;
    }

    if(!m_bSaved)
    {
        int resp = QMessageBox::question(this, tr("Question"), tr("Discard results?"),  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if(resp != QMessageBox::Yes) return;
    }

    accept();
}


void OptimPlaneDlg::initDialog(PlaneXfl const*pPlaneXfl)
{
    if(PlanePolarDlg::staticWPolar().isType123())
        PSOTaskPlane::setStaticPolar(PlanePolarDlg::staticWPolar());
    else
        PSOTaskPlane::staticPolar().setType(xfl::T1POLAR);

    std::string props;
    PSOTaskPlane::staticPolar().getProperties(props, nullptr);
    m_pptePolar->setPlainText(QString::fromStdString(props));

    m_pPlaneXfl = pPlaneXfl;


    fillPlaneList();

    if(m_pPlaneXfl)
    {
        QList<QListWidgetItem *> items = m_plwPlanes->findItems(QString::fromStdString(m_pPlaneXfl->name()), Qt::MatchExactly);
        if(items.size())
            m_plwPlanes->setCurrentItem(items.front());
        int nopt = int(m_pPlaneXfl->m_OptVariables.size());

        int nvars = NPLANEFIELDS + m_pPlaneXfl->nWings()*(NWINGFIELDS-1);
        for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
            nvars += m_pPlaneXfl->wingAt(iw)->nSections()*NSECTIONFIELDS;

        if(nopt==0 || nopt!=nvars)
            makeOptVariables();
    }

    fillVariables();
    fillObjectives();

    m_ptwControls->setCurrentIndex(s_iActiveTab);

    showObjectiveGraphs();
}


void OptimPlaneDlg::resetOutput()
{
    for(int io=0; io<NOBJECTIVES; io++)
    {
        Curve *pCurve = m_ObjGraph[io].curve(io);
        if(!pCurve) pCurve = m_ObjGraph[io].addCurve();
        pCurve->setStdName(m_Objective.at(io).m_Name);
        pCurve->reset();
    }

    // initialize the variable graph
    m_VariableGraph.deleteCurves();

    for(uint ivar=0; ivar<m_pPlaneXfl->m_OptVariables.size(); ivar++)
    {
        OptVariable const &var = m_pPlaneXfl->m_OptVariables.at(ivar);
        if(var.m_Max-var.m_Min>DELTAVAR)
        {
            Curve *pCurve = m_VariableGraph.addCurve();
            pCurve->setName(variableName(ivar));
            pCurve->clear();
        }
    }
}


void OptimPlaneDlg::updateParetoViews(int iSelect)
{
    if(!m_pPSOTask) return;

//        if(m_pPSOTask->nObjectives()!=2) return;
    double err0=0, err1=0, err2=0;
    m_ParetoGraph.setHighLighting(true);
    m_ParetoGraph.setXVarStdList({m_pPSOTask->objective(0).m_Name+"_err"});
   if(m_pPSOTask->nObjectives()>=2)
   {
       m_ParetoGraph.setYVarStdList({m_pPSOTask->objective(1).m_Name+"_err"});
       m_pParetoGraphWt->setOverlayedRect(true, m_pPSOTask->objective(0).m_MaxError, 0, 0, m_pPSOTask->objective(1).m_MaxError);
   }
   else
       m_pParetoGraphWt->setOverlayedRect(false, m_pPSOTask->objective(0).m_MaxError, 0, 0, 0);

    Curve *pCurveSw  = m_ParetoGraph.curve(0);
    if(!pCurveSw)
    {
        pCurveSw = m_ParetoGraph.addCurve("Swarm");
        pCurveSw->setStipple(Line::NOLINE);
        pCurveSw->setWidth(2);
        pCurveSw->setColor(Qt::darkCyan);
        pCurveSw->setSymbol(Line::BIGCIRCLE);
    }
    pCurveSw->clear();
    for(int i=0; i<m_pPSOTask->swarmSize(); i++)
    {
        Particle const &particle = m_pPSOTask->swarmAt(i);
        err0 = fabs(particle.fitness(0)-m_pPSOTask->objective(0).m_Target);
        if(particle.nObjectives()>1)
            err1 = fabs(particle.fitness(1)-m_pPSOTask->objective(1).m_Target);
        else
            err1 = 0.0;
        pCurveSw->appendPoint(err0, err1);
    }

    Curve *pCurveDom = m_ParetoGraph.curve(1);
    if(!pCurveDom)
    {
        pCurveDom = m_ParetoGraph.addCurve("Frontier");
        pCurveDom->setStipple(Line::NOLINE);
        pCurveDom->setSymbol(Line::BIGCIRCLE);
        pCurveDom->setColor(Qt::darkRed);
    }
    pCurveDom->clear();
    for(int i=0; i<m_pPSOTask->paretoSize(); i++)
    {
        Particle const &particle = m_pPSOTask->pareto(i);
        err0 = fabs(particle.fitness(0)-m_pPSOTask->objective(0).m_Target);
        if(particle.nObjectives()>1) err1 = fabs(particle.fitness(1)-m_pPSOTask->objective(1).m_Target);
        else                         err1 = 0.0;
        pCurveDom->appendPoint(err0, err1, QString::asprintf("p%d", i));
    }
    m_ParetoGraph.selectCurve(pCurveDom);
    pCurveDom->setSelectedPoint(iSelect);

//        m_ParetoGraph.invalidate();
    m_pParetoGraphWt->update();

    err0 = m_pPSOTask->objective(0).m_MaxError;
    if(m_pPSOTask->nObjectives()>1) err1 = m_pPSOTask->objective(1).m_MaxError;
    else                            err1 = 0.0;
    if(m_pPSOTask->nObjectives()>2) err2 = m_pPSOTask->objective(2).m_MaxError;
    else                            err2 = 0.0;

    double length = std::max(err0, err1);
    length = std::max(length, err2);
    if(m_pgl3dPareto)
    {
        m_pgl3dPareto->setBox(err0, err1, err2);
        m_pgl3dPareto->setReferenceLength(length*20.0);
        m_pgl3dPareto->setSwarm(m_pPSOTask->theSwarm());
        m_pgl3dPareto->setPareto(m_pPSOTask->thePareto());
        m_pgl3dPareto->setBest(iSelect);
        m_pgl3dPareto->update();
    }
}


void OptimPlaneDlg::readData()
{
    PSOTask::s_ArchiveSize     = m_pieArchiveSize->value();
    PSOTask::s_InertiaWeight   = m_pdeInertiaWeight->value();
    PSOTask::s_CognitiveWeight = m_pdeCognitiveWeight->value();
    PSOTask::s_SocialWeight    = m_pdeSocialWeight->value();
    PSOTask::s_ProbRegenerate  = m_pdePropRegenerate->value()/100.0;

    PSOTask::s_MaxIter         = m_pieMaxIter->value();
    PSOTask::s_PopSize         = m_pieSwarmSize->value();
    PSOTask::setMultithreaded(m_pchMultiThread->isChecked());
}



void OptimPlaneDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == OPTIM_PARTICLE_EVENT)
    {
        m_ParticleCounter++;
        m_ppto->onAppendQText(QString::asprintf("   made %2d/%2d particles\n", m_ParticleCounter, PSOTask::s_PopSize));
    }
    if(pEvent->type() == OPTIM_MAKESWARM_EVENT)
    {
        m_pPSOTask->clearPareto();  // current Pareto may be obsolete
        m_pPSOTask->makeParetoFrontier();
        m_pPSOTask->setAnalysisStatus(xfl::PENDING);// not started yet

        fillResults();

        updateParetoViews(-1);
        m_ParetoGraph.resetLimits();
        m_ParetoGraph.invalidate();
        update();


        m_bResetSwarm  = false;
        m_bResetPareto = false;

        enableControls(true);
        m_ppbSwarm->setEnabled(true);
    }
    else if(pEvent->type() == MESSAGE_EVENT)
    {
/*        MessageEvent *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        m_ppto->onAppendQText(pMsgEvent->msg()); */
    }
    else if(pEvent->type() == OPTIM_ITER_EVENT)
    {
    }
    else if(pEvent->type()==OPTIM_END_EVENT)
    {
        OptimEvent *pOptEvent = dynamic_cast<OptimEvent*>(pEvent);
        m_BestParticle = pOptEvent->particle();
        m_IterTotal += pOptEvent->iter();

        QString strange = QString::asprintf("The winner is particle %d\n",pOptEvent->iBest());
        listParticle(m_BestParticle, strange, "  ");
        m_ppto->onAppendQText(strange+"\n");

        fillResults();
        updateParetoViews(pOptEvent->iBest());

        strange = QString::asprintf("Optimization time: %.2f s\n", double(m_Clock.elapsed())/1000.0);
        m_ppto->onAppendQText(strange+"\n");

        m_ppbSwarm->setText(tr("Swarm"));
        m_ppbSwarm->setEnabled(true);
        m_bSaved = false;
        enableControls(true);
    }
    else QDialog::customEvent(pEvent);
}


bool OptimPlaneDlg::onMakeSwarm()
{
    if(m_pPSOTask && m_pPSOTask->isRunning())
    {
        onOutputMessage("Analysis is running\n");
        return false;
    }

    int nActive=0;
    QString log;
    onOutputMessage("Reading variables\n");
    readVariables(nActive, log, "   "); // fills the OptVariable vector
//    onOutputMessage(log+ EOLCHAR);

    if(nActive==0)
    {
        onOutputMessage("No active variable - aborting\n");
        return false;
    }

    readData();

    if(!readObjectives())
    {
        onOutputMessage("Error reading the objectives - aborting\n");
        return false;
    }

    // reset the iteration total
    m_IterTotal = 0;
    clearResults();
    // initialize the objective graph
    resetOutput();

    if(!m_pPSOTask)
    {
        m_pPSOTask = new PSOTaskPlane;
        connect(m_pPSOTask, &PSOTask::iterEvent,       this,  &OptimPlaneDlg::onIterEvent, Qt::BlockingQueuedConnection);
        connect(m_pPSOTask, &PSOTask::outputMessage,   this, &OptimPlaneDlg::onOutputMessage);

    }
    m_pPSOTask->setPlane(m_pPlaneXfl);
    m_pPSOTask->setVariables(m_pPlaneXfl->m_OptVariables);

    m_pPSOTask->setParent(this);

    setTaskObjectives(m_pPSOTask);

    m_ParticleCounter = 0;

    //run the instance asynchronously
    QThread *pThread = new QThread;
    m_pPSOTask->setAnalysisStatus(xfl::RUNNING);
    m_pPSOTask->moveToThread(pThread); // don't touch it until the PSO end task event is received

    onOutputMessage("Launching swarm build task asynchronously\n");
    connect(pThread, SIGNAL(started()),  m_pPSOTask, SLOT(onMakeParticleSwarm()));
    connect(pThread, SIGNAL(finished()), pThread,    SLOT(deleteLater())); // deletes the thread but not the object

    pThread->start();
    pThread->setPriority(xfl::threadPriority());

    enableControls(false);
    m_ppbSwarm->setEnabled(false);
    return true;
}


void OptimPlaneDlg::onSwarm()
{
    clearResults();

    if(!m_pPSOTask || m_pPSOTask->theSwarm().isEmpty())
    {
        m_ppto->onAppendQText("Swarm is not valid - make a random swarm first\n");
        return;
    }

    if(!m_pPSOTask) return; //error when creating the swarm

    if(m_pPSOTask->isRunning())
    {
        onOutputMessage("\n***** cancelling PSO task *****\n");
        m_pPSOTask->setAnalysisStatus(xfl::CANCELLED);
        // m_ppbSwarm->setText("Swarm"); restored when the finish event is received
        m_ppbSwarm->setEnabled(false); // prevent double commands
        return;
    }
    readData();

    // ensure that the variables are up to date
    int nActive=0;
    QString strange;
    onOutputMessage("\nUpdating variable ranges\n");
    readVariables(nActive, strange, "   "); // fills the OptVariable vector
    onOutputMessage(strange+"\n");

    listObjectives(strange);
    onOutputMessage(strange+EOLch);

    if(nActive==0)
    {
        onOutputMessage("No active variable - aborting\n");
        return;
    }
    m_pPSOTask->setVariables(m_OptVariable);

    if(m_bResetSwarm)
    {
        m_ppto->onAppendQText("Swarm is not valid - make a random swarm first\n");
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if(m_bResetPareto)
    {
        // case of changed objective values
        onOutputMessage("Initializing Pareto frontier... ");
        readObjectives();
        setTaskObjectives(m_pPSOTask);
        m_pPSOTask->clearPareto();  // current Pareto may be obsolete
        m_pPSOTask->makeParetoFrontier();
        if(m_pPSOTask->thePareto().isEmpty())
        {
            onOutputMessage("Empty Pareto error: check input data\n");
            QApplication::restoreOverrideCursor();
            return;
        }
        else
            onOutputMessage("done.\n");
    }
    QApplication::restoreOverrideCursor();

    //run the instance asynchronously
    QThread *pThread = new QThread;
    m_pPSOTask->setAnalysisStatus(xfl::RUNNING);
    m_pPSOTask->moveToThread(pThread); // don't touch it until the PSO end task event is received

    enableControls(false); // prevent the user from doing stupid things
    m_Clock.restart(); // put pressure on something (Jerry)

    onOutputMessage("Launching optimization task asynchronously\n");
    connect(pThread,   SIGNAL(started()),      m_pPSOTask, SLOT(onStartIterations()));
    connect(pThread,   SIGNAL(finished()),     pThread,    SLOT(deleteLater())); // deletes the thread but not the object
    pThread->start();
    pThread->setPriority(xfl::threadPriority());
    m_ppbSwarm->setText(tr("Interrupt task"));
}


void OptimPlaneDlg::enableControls(bool bEnable)
{
    m_ppbMakeSwarm->setEnabled(bEnable);
    m_ppbStoreBest->setEnabled(bEnable);
    m_ppbContinueBest->setEnabled(bEnable);
    m_pPSOFrame->setEnabled(bEnable);
    m_pButtonBox->setEnabled(bEnable);

    m_plwPlanes->setEnabled(bEnable);
    m_pcptObjective->setEnabled(bEnable);
    m_pVarFrame->setEnabled(bEnable);
    m_pAnalysisFrame->setEnabled(bEnable);
}


void OptimPlaneDlg::onOutputMessage(QString const &msg)
{
    m_ppto->onAppendQText(msg);
}


void OptimPlaneDlg::onSortColumn(int col, Qt::SortOrder order)
{
    m_pResultsModel->sort(col, order);
}


void OptimPlaneDlg::onActionResultClicked(QModelIndex index)
{
    if(!m_pPSOTask) return;

    if(!index.isValid())
    {
    }
    else
    {
        if(index.column()==m_pResultsModel->actionColumn())
        {
            int row = index.row();
            m_BestParticle = m_pPSOTask->pareto(row);
            m_pcptResults->selectRow(index.row());
            QRect itemrect = m_pcptResults->visualRect(index);
            QPoint menupos = m_pcptResults->mapToGlobal(itemrect.topLeft());
            QMenu *pActionMenu = new QMenu(this);
            {
                QAction *pStoreBestPlane = new QAction("Store plane", this);
                pStoreBestPlane->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
                connect(pStoreBestPlane,   SIGNAL(triggered()), SLOT(onStoreBest()));

                pActionMenu->addAction(pStoreBestPlane);
                pActionMenu->exec(menupos);
            }
        }
    }
}


void OptimPlaneDlg::onResetParetoFrontier()
{
    if(!m_pPSOTask) return;
    m_pPSOTask->clearPareto();
    m_pPSOTask->makeParetoFrontier();
    updateParetoViews(-1);
}


void OptimPlaneDlg::fillResults()
{
    if(!m_pPSOTask || !m_pPSOTask->paretoSize())
    {
        clearResults();
        return;
    }

    m_pResultsModel->setRowCount(m_pPSOTask->paretoSize());
    int nCols = 2;// particle id + action column
    nCols += m_pPSOTask->nActiveVariables();
    nCols += m_pPSOTask->nObjectives();
    m_pResultsModel->setColumnCount(nCols);
    QVector<int> digits(nCols);
    digits.fill(5);
    m_pResultsDelegate->setDigits(digits);
    int iCol=1;
    for(int ivar=0; ivar<m_pPSOTask->nVariables(); ivar++)
    {
        OptVariable const &var = m_pPSOTask->variable(ivar);
        if(var.m_Max-var.m_Min>DELTAVAR)
            m_pResultsModel->setHeaderData(iCol++, Qt::Horizontal, QString::fromStdString(m_pPSOTask->variable(ivar).m_Name));
    }
    for(int iobj=0; iobj<m_pPSOTask->nObjectives(); iobj++)
        m_pResultsModel->setHeaderData(iCol++, Qt::Horizontal, QString::fromStdString(m_pPSOTask->objective(iobj).m_Name));

    m_pResultsModel->setHeaderData(0, Qt::Horizontal, "Particle");
    m_pResultsModel->setHeaderData(nCols-1, Qt::Horizontal, "Action");
    m_pResultsModel->setActionColumn(nCols-1); //temporary
    m_pResultsDelegate->setActionColumn(nCols-1);

    QModelIndex ind;
    double factor=1.0;
    QString labunit;
    for(int row=0; row<m_pPSOTask->paretoSize(); row++)
    {
        Particle const &particle = m_pPSOTask->pareto(row);
        ind = m_pResultsModel->index(row, 0, QModelIndex());
        m_pResultsModel->setData(ind, QString::asprintf("p%d", row));
        iCol=1;
        for(int ivar=0; ivar<particle.dimension(); ivar++)
        {
            OptVariable const &var = m_pPSOTask->variable(ivar);
            getUnit(ivar, factor, labunit);
            if(var.m_Max-var.m_Min>DELTAVAR)
            {
                ind = m_pResultsModel->index(row, iCol++, QModelIndex());
                m_pResultsModel->setData(ind, particle.pos(ivar)*factor);
            }
        }
        for(int io=0; io<particle.nObjectives(); io++)
        {
            ind = m_pResultsModel->index(row, iCol++, QModelIndex());
            m_pResultsModel->setData(ind, particle.fitness(io));
        }
    }

    onResizeColumns();
}


void OptimPlaneDlg::listObjectives(QString &list) const
{
    list.clear();
    list = "Objectives:" + EOLch;

    for(int iobj=0; iobj<m_Objective.size(); iobj++)
    {
        OptObjective const &obj = m_Objective.at(iobj);
        if(obj.m_bActive)
        {
            list += "   " + QString::fromStdString(obj.m_Name) + QString::asprintf(": %7g", obj.m_Target) + EOLch;
        }
    }

}


void OptimPlaneDlg::listParticle(Particle const &particle, QString &log, QString const &prefix) const
{
    log += prefix + "Position:\n";
    for(int ivar=0; ivar<particle.dimension(); ivar++)
    {
        OptVariable const &var = m_OptVariable.at(ivar);
        double pos = particle.pos(ivar);
        QString labunit;
        double factor = 1.0;
        if(var.m_Max-var.m_Min>DELTAVAR)
        {
            getUnit(ivar, factor, labunit);
            pos *= factor;
            log += prefix + "   " + variableName(ivar).leftJustified(19, ' ') + QString::asprintf("= %9.3f", pos) + labunit + "\n";
        }
    }

    log += prefix + "Fitness:\n";
    int nobj=0;
    for(int iobj=0; iobj<m_Objective.size(); iobj++)
    {
        OptObjective const &obj = m_Objective.at(iobj);
        if(obj.m_bActive)
        {
            log += prefix + "   " + QString::fromStdString(obj.m_Name) + QString::asprintf("=%7g\n", particle.fitness(nobj));
            nobj++;
        }
    }
}


int OptimPlaneDlg::nActiveObjectives() const
{
    int nActiveObj = 0;
    for(int iobj=0; iobj<m_Objective.size(); iobj++)
        if(m_Objective.at(iobj).m_bActive) nActiveObj++;
    return nActiveObj;
}


bool OptimPlaneDlg::readObjectives()
{
    bool bOk0=false, bOk1=false;

    QString strange;

    QModelIndex ind;

    for(int row=0; row<m_pObjModel->rowCount(); row++)
    {
        ind = m_pObjModel->index(row, 0);
        m_Objective[row].m_bActive = m_pObjModel->data(ind, Qt::UserRole).toBool();

        ind = m_pObjModel->index(row, 1);
        m_Objective[row].m_Name = m_pObjModel->data(ind).toString().toStdString();

        ind = m_pObjModel->index(row, 2);
        int val = m_pObjModel->data(ind).toInt();
        if     (val<0) m_Objective[row].m_Type = xfl::MINIMIZE;
        else if(val>0) m_Objective[row].m_Type = xfl::MAXIMIZE;
        else           m_Objective[row].m_Type = xfl::EQUALIZE;

        ind = m_pObjModel->index(row, 3);
        strange = m_pObjModel->data(ind).toString();
        strange.replace(" ","");
        m_Objective[row].m_Target = strange.toDouble(&bOk0);

        ind = m_pObjModel->index(row, 4);
        strange = m_pObjModel->data(ind).toString();
        strange.replace(" ","");
        m_Objective[row].m_MaxError = strange.toDouble(&bOk1);
    }
    return true;
}


void OptimPlaneDlg::setTaskObjectives(PSOTaskPlane *pPSOTask)
{
    if(!pPSOTask) return;
    pPSOTask->setNObjectives(nActiveObjectives());

    int nActiveObj = 0;
    for(int iobj=0; iobj<m_Objective.size(); iobj++)
    {
        if(m_Objective.at(iobj).m_bActive)
        {
            pPSOTask->setObjective(nActiveObj, m_Objective.at(iobj));
            nActiveObj++;
        }
    }
    pPSOTask->updateErrors(); // current particle errors are obsolete
}


void OptimPlaneDlg::onVariableChanged(QModelIndex,QModelIndex)
{
    int nActive=0;
    QString strange, prefix("   ");
    readVariables(nActive, strange, prefix); // to save the changes on the fly in case the user switches planes

    strange = EOLch + "Active variables:" + EOLch + strange +EOLch;
    onOutputMessage(strange);
}


void OptimPlaneDlg::onResetOptVariables()
{
    makeOptVariables();
    fillVariables();
}


void OptimPlaneDlg::fillPlaneList()
{
    m_plwPlanes->clear();
    for(int ip=0; ip<Objects3d::nPlanes(); ip++)
    {
        PlaneXfl const*pPlaneXfl = dynamic_cast<PlaneXfl const*>(Objects3d::planeAt(ip));
        if(pPlaneXfl) m_plwPlanes->addItem(QString::fromStdString(pPlaneXfl->name()));
    }
}


void OptimPlaneDlg::makeOptVariables()
{
    std::vector<OptVariable> &OptVars = m_pPlaneXfl->m_OptVariables;

    int nvars = NPLANEFIELDS + m_pPlaneXfl->nWings()*(NWINGFIELDS-1);
    for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
        nvars += m_pPlaneXfl->wingAt(iw)->nSections()*NSECTIONFIELDS;

    OptVars.resize(nvars); // first line is aoa

    int ivar=0;
    OptVars[ivar++] = {ALPHAstr, s_AlphaMin, s_AlphaMax};
    OptVars[ivar++] = {"Mass",  m_pPlaneXfl->totalMass()};
    OptVars[ivar++] = {"CoG.x", m_pPlaneXfl->CoG_t().x};
    OptVars[ivar++] = {"CoG.z", m_pPlaneXfl->CoG_t().z};

    for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = m_pPlaneXfl->wingAt(iw);

        OptVars[ivar++] = {"LE.x",          pWing->position().x  };
        OptVars[ivar++] = {"LE.y",          pWing->position().y  };
        OptVars[ivar++] = {"LE.z",          pWing->position().z  };
        OptVars[ivar++] = {"Rx",            pWing->rx()          };
        OptVars[ivar++] = {"Ry",            pWing->ry()          };
        OptVars[ivar++] = {"Rz",            pWing->rz()          };
        OptVars[ivar++] = {"span",          pWing->planformSpan()};
        OptVars[ivar++] = {"root chord",    pWing->rootChord()   };
        OptVars[ivar++] = {"average sweep", pWing->averageSweep()};
        OptVars[ivar++] = {"twist",         pWing->twist()       };
        OptVars[ivar++] = {"area",          pWing->planformArea()};
        OptVars[ivar++] = {"AR",            pWing->aspectRatio() };
        OptVars[ivar++] = {"TR",            pWing->taperRatio()  };

        for(int isec=0; isec<pWing->nSections(); isec++)
        {
            WingSection const &sec = pWing->section(isec);
            OptVars[ivar++] = {"y",        sec.yPosition()};
            OptVars[ivar++] = {"chord",    sec.chord()};
            OptVars[ivar++] = {"offset",   sec.offset()};
            OptVars[ivar++] = {"dihedral", sec.dihedral()};
            OptVars[ivar++] = {"twist",    sec.twist()};
        }
    }
}


void OptimPlaneDlg::fillVariables()
{
    m_pVariableModel->removeRows(0, m_pVariableModel->rowCount());

    if(!m_pPlaneXfl) return;

    std::vector<OptVariable> &OptVars = m_pPlaneXfl->m_OptVariables;

    QStandardItem *pRootItem = m_pVariableModel->invisibleRootItem();
    QList<QStandardItem*> planeitems, wingitems, sectionitems;

    double factor=1.0;
    QString labunit;
    int ivar=0;

    for(ivar=0; ivar<NPLANEFIELDS; ivar++)
    {
        getUnit(ivar, factor, labunit);
        OptVariable const &var = OptVars.at(ivar);
        planeitems = xfl::prepareDoubleRow(QString::fromStdString(var.m_Name), var.m_Min*factor, var.m_Max*factor, labunit);
        pRootItem->appendRow(planeitems);
    }

    for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = m_pPlaneXfl->wingAt(iw);

        QList<QStandardItem*> wingfolder = xfl::prepareRow(QString::fromStdString(pWing->name()), QString(), QString(), QString());
        pRootItem->appendRow(wingfolder);
        for(int iwingvar=0; iwingvar<NWINGFIELDS-1; iwingvar++)
        {
            getUnit(ivar, factor, labunit);
            OptVariable const &var = OptVars.at(ivar);
            wingitems = xfl::prepareDoubleRow(QString::fromStdString(var.m_Name), var.m_Min*factor, var.m_Max*factor, labunit);
            wingfolder.front()->appendRow(wingitems);
            ivar++;
        }

        for(int isec=0; isec<pWing->nSections(); isec++)
        {
            QList<QStandardItem*> sectionfolder = xfl::prepareRow(QString::asprintf("section %d", isec), QString(), QString(), QString());
            wingfolder.front()->appendRow(sectionfolder);
            for(int isecvar=0; isecvar<NSECTIONFIELDS; isecvar++)
            {
                getUnit(ivar, factor, labunit);
                OptVariable const &var = OptVars.at(ivar);
                sectionitems = xfl::prepareDoubleRow(QString::fromStdString(var.m_Name), var.m_Min*factor, var.m_Max*factor, labunit);
                sectionfolder.front()->appendRow(sectionitems);
                ivar++;
            }
        }
    }
}


void OptimPlaneDlg::readVariables(int &nActive, QString &log, QString const &prefix)
{
    if(!m_pPlaneXfl) return;
    QString field;
    QString strange, error, labunit;

    nActive = 0;
    double vmin(0), vmax(0);

    std::vector<OptVariable> &OptVars = m_pPlaneXfl->m_OptVariables;

    int nvar = 0;
    double factor = 1.0;
    QModelIndex planelevel = m_pVariableModel->index(0,0);

    // aoa, Mass, CoG.x and CoG.z
    for(int ipl=0; ipl<NPLANEFIELDS; ipl++)
    {
        OptVariable &planevar = OptVars[ipl];
        field = planelevel.sibling(planelevel.row(),0).data().toString();
        vmin  = planelevel.sibling(planelevel.row(),1).data().toDouble();
        vmax  = planelevel.sibling(planelevel.row(),2).data().toDouble();

        getUnit(nvar, factor, labunit);

        if(vmin>vmax)
        {
            double tmp = vmax;
            vmax = vmin;
            vmin = tmp;
        }
        else if(vmax-vmin>DELTAVAR)
        {
            log += prefix + field.leftJustified(16, ' ') + QString::asprintf("[%9.3f - %9.3f] ", vmin, vmax) + labunit + "\n";
            nActive++;
        }

        planevar = {field.toStdString(), vmin/factor, vmax/factor};

        nvar++;
        planelevel = planelevel.sibling(planelevel.row()+1,0);
    }
    s_AlphaMin = OptVars.front().m_Min;
    s_AlphaMax = OptVars.front().m_Max;

    field  = planelevel.sibling(planelevel.row(),0).data().toString();

    for(int irow=0; irow<m_pVariableModel->rowCount(); irow++)
    {
        WingXfl const*pWing = m_pPlaneXfl->wingAt(irow);
        QStandardItem *pWingItem = m_pVariableModel->itemFromIndex(planelevel);
        error.clear();
        strange.clear();
        if(pWingItem && pWingItem->child(0,0))
        {
            QModelIndex winglevel = pWingItem->child(0,0)->index();

            for(int i=0; i<NWINGFIELDS-1; i++)
            {
                field  = winglevel.sibling(winglevel.row(),0).data().toString();
                vmin = winglevel.sibling(winglevel.row(),1).data().toDouble();
                vmax = winglevel.sibling(winglevel.row(),2).data().toDouble();

                getUnit(nvar, factor, labunit);

                if(vmin>vmax+DELTAVAR)
                {
                    double tmp = vmax;
                    vmax = vmin;
                    vmin = tmp;
                }

                if(vmax-vmin>DELTAVAR)
                {
                    strange += prefix + "   " + field.leftJustified(13, ' ') + QString::asprintf("[%9.3f - %9.3f] ", vmin, vmax) + labunit + "\n";
                    nActive++;
                }

                OptVars[nvar++] = {field.toStdString(), vmin/factor, vmax/factor};
                winglevel = winglevel.sibling(winglevel.row()+1,0);
            }


            for(int isec=0; isec<pWing->nSections(); isec++)
            {
                QStandardItem *pSectionItem = m_pVariableModel->itemFromIndex(winglevel);
                QModelIndex sectionlevel = pSectionItem->child(0,0)->index();
                for(int isecvar=0; isecvar<NSECTIONFIELDS; isecvar++)
                {
                    field  = sectionlevel.sibling(sectionlevel.row(),0).data().toString();
                    vmin = sectionlevel.sibling(sectionlevel.row(),1).data().toDouble();
                    vmax = sectionlevel.sibling(sectionlevel.row(),2).data().toDouble();

                    getUnit(nvar, factor, labunit);

                    if(vmin>vmax+DELTAVAR)
                    {
                        double tmp = vmax;
                        vmax = vmin;
                        vmin = tmp;
                    }

                    if(vmax-vmin>DELTAVAR)
                    {
                        QString strong = field + QString::asprintf("(%d)", isec);
                        strange += prefix + "   " + strong.leftJustified(13, ' ') + QString::asprintf("[%9.3f - %9.3f] ", vmin, vmax) + labunit + "\n";
                        nActive++;
                    }

                    OptVars[nvar++] = {field.toStdString(), vmin/factor, vmax/factor};
                    sectionlevel = sectionlevel.sibling(sectionlevel.row()+1,0);
                }
                winglevel = winglevel.sibling(winglevel.row()+1,0);
            }

            if(error.length() || strange.length())
            {
                log += prefix + QString::fromStdString(pWing->name()) + ":\n";
                if(error.length())   log += error;
                if(strange.length()) log += strange;
            }

            planelevel = planelevel.sibling(planelevel.row()+1,0);
        }
    }
    m_OptVariable = m_pPlaneXfl->m_OptVariables;
}


void OptimPlaneDlg::onAnalysisDef()
{
    T1234578PolarDlg wpDlg(this);
    wpDlg.initPolar3dDlg(m_pPlaneXfl, nullptr);

    int res = wpDlg.exec();

    if (res == QDialog::Accepted)
    {
        PSOTaskPlane::setStaticPolar(T1234578PolarDlg::staticWPolar());
    }

    std::string props;
    PSOTaskPlane::staticPolar().getProperties(props, nullptr);
    m_pptePolar->setPlainText(QString::fromStdString(props));
}


void OptimPlaneDlg::onRunAnalysis()
{
    PlaneXfl *pPlaneXfl = new PlaneXfl;
    pPlaneXfl->duplicate(m_pPlaneXfl);

    PlanePolar wpolar;
    wpolar.duplicateSpec(&PSOTaskPlane::staticPolar());
    wpolar.setAutoInertia(true); //since the CoG is set as a plane variable
    wpolar.setReferenceChordLength(m_pPlaneXfl->mac());
    wpolar.setReferenceArea(m_pPlaneXfl->projectedArea(false));
    wpolar.setReferenceSpanLength(m_pPlaneXfl->projectedSpan());

    std::vector<double> opplist{s_AlphaMin, s_AlphaMax}; // first coordinate is the aoa

    if(runPlaneAnalysis(pPlaneXfl, &wpolar, opplist))
    {

        QString strange;
        if(wpolar.dataSize()==1)
        {
            strange +=                   "                  Alpha_min\n";
            strange += QString::asprintf(" Alpha        = %11.3f\n", s_AlphaMin);

            strange += QString::asprintf("   Cl         = %11.3f\n",     wpolar.aeroForce(0).CL());
            strange += QString::asprintf("   Cd         = %11.3f\n",     wpolar.aeroForce(0).CD());
            strange += QString::asprintf("   Cl/CD      = %11.3f\n",     wpolar.aeroForce(0).CL()/wpolar.aeroForce(0).CD());
            strange += QString::asprintf("   Cm         = %11.3f\n",     wpolar.aeroForce(0).Cm());
            strange += QString::asprintf("   m.g.Vz     = %11.3f W\n",   wpolar.m_Mass_var.at(0)*9.81*wpolar.Vz(0));
            strange += QString::asprintf("   BM         = %11.3f N.m\n", wpolar.m_MaxBending.at(0));
        }
        else if(wpolar.dataSize()==2)
        {
            strange +=                   "                  Alpha_min        Alpha_max\n";
            strange += QString::asprintf(" Alpha        = %11.3f      %11.3f\n", s_AlphaMin, s_AlphaMax);
            strange += QString::asprintf("   Cl         = %11.3f      %11.3f\n",     wpolar.aeroForce(0).CL(), wpolar.aeroForce(1).CL());
            strange += QString::asprintf("   Cd         = %11.3f      %11.3f\n",     wpolar.aeroForce(0).CD(), wpolar.aeroForce(1).CD());
            strange += QString::asprintf("   Cl/CD      = %11.3f      %11.3f\n",     wpolar.aeroForce(0).CL()/wpolar.aeroForce(0).CD(),
                                                                                     wpolar.aeroForce(1).CL()/wpolar.aeroForce(1).CD());
            strange += QString::asprintf("   Cm         = %11.3f      %11.3f\n",     wpolar.aeroForce(0).Cm(), wpolar.aeroForce(1).Cm());
            strange += QString::asprintf("   m.g.Vz     = %11.3f      %11.3f W\n",   wpolar.m_Mass_var.at(0)*9.81*wpolar.Vz(0),
                                                                                     wpolar.m_Mass_var.at(1)*9.81*wpolar.Vz(1));
            strange += QString::asprintf("   BM         = %11.3f      %11.3f N.m\n", wpolar.m_MaxBending.at(0), wpolar.m_MaxBending.at(1));
        }
        else
        {
            strange = " **** Analysis error **** \n";
        }

        onOutputMessage(strange);

    }
    delete pPlaneXfl;
}


bool OptimPlaneDlg::runPlaneAnalysis(PlaneXfl *pPlaneXfl, PlanePolar *pWPolar, std::vector<double> const &opplist)
{
    PlaneTask *pTask = new PlaneTask;
    Task3d::setCancelled(false);
    TriMesh::setCancelled(false);
    pTask->setAnalysisStatus(xfl::RUNNING);

    pWPolar->setAutoInertia(true); //since the CoG is set as a plane variable
    pWPolar->setReferenceChordLength(m_pPlaneXfl->mac());
    pWPolar->setReferenceArea(m_pPlaneXfl->projectedArea(false));
    pWPolar->setReferenceSpanLength(m_pPlaneXfl->projectedSpan());

    pPlaneXfl->makePlane(pWPolar->bThickSurfaces(), true, pWPolar->isTriangleMethod());
    pTask->setObjects(pPlaneXfl, pWPolar);
    pTask->setOppList(opplist);
    XPlane::setStoreOpps3d(false);

    pTask->run();
    bool bSuccess = !pTask->hasErrors();
    delete pTask;
    return bSuccess;
}


PlaneXfl* OptimPlaneDlg::onStoreBest()
{
    if(!m_pPSOTask)
    {
        m_ppto->onAppendQText("\nNo active result to save\n");
        return nullptr;
    }

    PlaneXfl *pBestPlane = new PlaneXfl;
    makePSOPlane(&m_BestParticle, m_pPlaneXfl, pBestPlane);
    pBestPlane->setName(std::string("Optimized plane"));
    pBestPlane->setInitialized(false);
    pBestPlane->setLineColor(xfl::randomfl5Color());
    QString strange;
    listParticle(m_BestParticle, strange, QString());
    strange.truncate(strange.length()-2);
    pBestPlane->setDescription(strange.toStdString());


    Plane *pPlane(nullptr);
    if(Objects3d::plane(pBestPlane->name()))
        pPlane = Objects3d::setModifiedPlane(pBestPlane);
    else
    {
        pPlane = Objects3d::addPlane(pBestPlane);
    }

    m_pBestPlane = dynamic_cast<PlaneXfl*>(pPlane); // so that it can be selected when exiting the module

    fillPlaneList();
    if(m_pPlaneXfl)
    {
        QList<QListWidgetItem *> items = m_plwPlanes->findItems(QString::fromStdString(m_pPlaneXfl->name()), Qt::MatchExactly);
        if(items.size()) m_plwPlanes->setCurrentItem(items.front());
    }

    m_ppto->onAppendQText("Saved the plane with the name: "+QString::fromStdString(pBestPlane->name())+"\n");
    m_bSaved = true;

    m_bChanged = true;
    return m_pBestPlane;
}


void OptimPlaneDlg::onContinueBest()
{
    PlaneXfl *pNewBest = onStoreBest();
    if(!pNewBest) return;

    m_pPlaneXfl = pNewBest;

    int nopt = int(m_pPlaneXfl->m_OptVariables.size());
    if(nopt==0 || nopt!=NPLANEFIELDS+m_pPlaneXfl->nWings()*NWINGFIELDS)
        makeOptVariables();
    m_ppto->onAppendQText("Continuing with plane: "+QString::fromStdString(m_pPlaneXfl->name()) + "\n");
    m_ppto->onAppendStdText(m_pPlaneXfl->planeData(false) + "\n\n");

    onOutputMessage("Invalidating current swarm\n");
    m_bResetSwarm = true;
    update();
}


void OptimPlaneDlg::onPlaneSelected(QListWidgetItem *pItem)
{
    if(!pItem)
    {
        m_pPlaneXfl = nullptr;
        m_ppto->onAppendQText("No selected plane\n");
    }
    else
    {
        m_pPlaneXfl = dynamic_cast<PlaneXfl const*>(Objects3d::planeAt(pItem->text().toStdString()));
        int nopt = int(m_pPlaneXfl->m_OptVariables.size());
        if(nopt==0 || nopt!=NPLANEFIELDS+m_pPlaneXfl->nWings()*NWINGFIELDS)
            makeOptVariables();
        m_ppto->onAppendQText("Selected plane: "+QString::fromStdString(m_pPlaneXfl->name()) + "\n");
        m_ppto->onAppendStdText(m_pPlaneXfl->planeData(false) + "\n\n");
    }

    fillVariables();
}


void OptimPlaneDlg::getUnit(int ivar, double &factor, QString &labunit) const
{
    if(ivar==0)
    {
        factor  = 1.0;
        labunit = DEGch;
    }
    else if(ivar==1)
    {
        factor  = Units::kgtoUnit();
        labunit = Units::massUnitQLabel();
    }
    else if(ivar==2 || ivar==3)
    {
        factor  = Units::mtoUnit();
        labunit = Units::lengthUnitQLabel();
    }
    else
    {
//        int iwvar = (ivar-NPLANEFIELDS)%NWINGFIELDS;
        int iwvar = ivar-NPLANEFIELDS;
        for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
        {
            WingXfl const*pWing = m_pPlaneXfl->wingAt(iw);
            if(iwvar<13)
            {
                switch(iwvar)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 6:
                    case 7:
                        factor  = Units::mtoUnit();
                        factor  = Units::mtoUnit();
                        labunit = Units::lengthUnitQLabel();
                        return;
                    case 3:
                    case 4:
                    case 5:
                    case 8:
                    case 9:
                        factor  = 1.0;
                        labunit = DEGch;
                        return;
                    case 10:
                        factor  = Units::m2toUnit();
                        labunit = Units::areaUnitQLabel();
                        return;
                    case 11:
                    case 12:
                        factor  = 1.0;
                        labunit = QString();
                        return;
                }
            }
            else if(iwvar<13+pWing->nSections()*NSECTIONFIELDS)
            {
                int isecvar = (iwvar-13)%NSECTIONFIELDS;
                switch(isecvar)
                {
                    case 0:
                    case 1:
                    case 2:
                        factor  = Units::mtoUnit();
                        factor  = Units::mtoUnit();
                        labunit = Units::lengthUnitQLabel();
                        return;
                    case 3:
                    case 4:
                        factor  = 1.0;
                        labunit = DEGch;
                        return;
                }
            }
            else
                iwvar -= (13+pWing->nSections()*NSECTIONFIELDS);
        }
    }
}


QString OptimPlaneDlg::variableName(int ivar) const
{
    if(ivar<NPLANEFIELDS) return QString::fromStdString(m_pPlaneXfl->m_OptVariables.at(ivar).m_Name);

    int ipvar = 4;

    for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = m_pPlaneXfl->wingAt(iw);
        for(int k=0; k<NWINGFIELDS-1; k++)
        {
            if(ipvar==ivar)
                return QString::fromStdString(m_pPlaneXfl->wingAt(iw)->name()+"_"+m_pPlaneXfl->m_OptVariables.at(ivar).m_Name);
            ipvar++;
        }
        for(int isec=0; isec<pWing->nSections(); isec++)
        {
            for(int k=0; k<NSECTIONFIELDS; k++)
            {
                if(ipvar==ivar)
                    return QString::fromStdString(m_pPlaneXfl->wingAt(iw)->name())
                            +QString::asprintf("_sec%d_", isec)
                            +QString::fromStdString(m_pPlaneXfl->m_OptVariables.at(ivar).m_Name);
                ipvar++;
            }
        }
    }
    return QString();
}


void OptimPlaneDlg::fillObjectives()
{
    m_pObjModel->setRowCount(m_Objective.size());

    QModelIndex ind;
    m_pObjModel->blockSignals(true); // do notemit the dataChanged signal

    for(int row=0; row<m_Objective.size(); row++)
    {
        OptObjective const &obj = m_Objective.at(row);
        ind = m_pObjModel->index(row, 0, QModelIndex());
        m_pObjModel->itemFromIndex(ind)->setToolTip("Activate or deactivate the objective");
        m_pObjModel->setData(ind, obj.m_bActive, Qt::UserRole);

        ind = m_pObjModel->index(row, 1, QModelIndex());
        m_pObjModel->itemFromIndex(ind)->setToolTip("The name of the objective");
        if(row==6)
        {
            m_pObjModel->itemFromIndex(ind)->setToolTip("Main wing root bending moment");
        }

        m_pObjModel->setData(ind, QString::fromStdString(obj.m_Name));

        ind = m_pObjModel->index(row, 2, QModelIndex());
        m_pObjModel->itemFromIndex(ind)->setToolTip("-1 to MINIMIZE the objective\n"
                                                    " 0 to EQUALIZE\n"
                                                    " 1 to MAXIMIZE");
        switch(obj.m_Type)
        {
            case xfl::MINIMIZE:  m_pObjModel->setData(ind, -1); break;
            case xfl::MAXIMIZE:  m_pObjModel->setData(ind, 1); break;
            case xfl::EQUALIZE:  m_pObjModel->setData(ind, 0); break;
        }
        ind = m_pObjModel->index(row, 3, QModelIndex());
        m_pObjModel->itemFromIndex(ind)->setToolTip("The target value");
        m_pObjModel->setData(ind, obj.m_Target);

        ind = m_pObjModel->index(row, 4, QModelIndex());
        m_pObjModel->setData(ind, obj.m_MaxError);
        m_pObjModel->itemFromIndex(ind)->setToolTip("The tolerance on the target value.\n"
                                                    "Only of use if the objective is to EQUALIZE");
    }
    m_pObjModel->blockSignals(false);
}


void OptimPlaneDlg::showObjectiveGraphs()
{
    for(int i=0; i<NOBJECTIVES; i++)
    {
        m_pObjGraphWt[i]->setVisible(m_Objective.at(i).m_bActive);
    }
}


void OptimPlaneDlg::onObjTableClicked(QModelIndex index)
{
    if(index.row()>=m_pObjModel->rowCount()) return;

    int const ACTIVECOL = 0;
    if(index.column()!=ACTIVECOL) return;

    bool bActive = m_pObjModel->data(index, Qt::UserRole).toBool(); // use a QVariant with the EditRole rather thant the QtCheckStateRole - not interested in Qt::PartiallyChecked
    bActive = !bActive; // toggle
    // update the range
    m_Objective[index.row()].m_bActive = bActive;
    QModelIndex chindex = m_pObjModel->index(index.row(), ACTIVECOL);
    m_pObjModel->setData(chindex, bActive, Qt::UserRole);
    m_pcptObjective->selectRow(chindex.row()); // only way found to force the repaint
    m_pcptObjective->setCurrentIndex(chindex);

    m_ppto->onAppendQText("Invalidating swarm\n");
    invalidateSwarm();

    showObjectiveGraphs();
    update();
}


void OptimPlaneDlg::clearResults()
{
    m_pResultsModel->removeRows(0, m_pResultsModel->rowCount());


    for(int io=0; io<NOBJECTIVES; io++)
    {
        Curve *pCurve = m_ObjGraph[io].curve(0);
        if(pCurve) pCurve->reset();
        m_ObjGraph[io].resetLimits();
        m_ObjGraph[io].invalidate();
    }

    for(int ic=0; ic<m_VariableGraph.curveCount(); ic++)
    {
        m_VariableGraph.curve(ic)->reset();
        m_VariableGraph.resetXLimits();
        m_VariableGraph.invalidate();
    }
    update();
}


void OptimPlaneDlg::onObjectiveChanged()
{
    readObjectives();
    invalidatePareto();
}


void OptimPlaneDlg::onIterEvent(OptimEvent*pEvent)
{
    OptimEvent *pOptEvent = dynamic_cast<OptimEvent*>(pEvent);
    m_BestParticle = pOptEvent->particle();

    QString strange =QString::asprintf("Iter. %3d / Particle %d: ", m_IterTotal+pOptEvent->iter(), pOptEvent->iBest());

    int iobj = 0;
    for(int io=0; io<NOBJECTIVES; io++)
    {
        if(m_Objective[io].m_bActive)
        {
            strange += "  " + QString::fromStdString(m_Objective[io].m_Name) +
                              QString::asprintf("=%7g ", m_BestParticle.fitness(iobj));
            iobj++;
        }
    }
    m_ppto->onAppendQText(strange + "\n");

    PlaneXfl *pPlaneXfl = new PlaneXfl;
    makePSOPlane(&m_BestParticle, m_pPlaneXfl, pPlaneXfl);

    PlanePolar wpolar;
    wpolar.duplicateSpec(&PSOTaskPlane::staticPolar());
    std::vector<double> opplist;
    if(fabs(s_AlphaMin-s_AlphaMax)<1.0-5) opplist.push_back(s_AlphaMin);
    else opplist.push_back(m_BestParticle.pos(0));

    if(runPlaneAnalysis(pPlaneXfl, &wpolar, opplist))
    {
        Curve *pCurve(nullptr);
        pCurve = m_ObjGraph[0].curve(0);
        if(pCurve)     pCurve->appendPoint(double(m_IterTotal+pOptEvent->iter()), wpolar.aeroForce(0).CL());
        pCurve = m_ObjGraph[1].curve(0);
        if(pCurve)     pCurve->appendPoint(double(m_IterTotal+pOptEvent->iter()), wpolar.aeroForce(0).CD());
        pCurve = m_ObjGraph[2].curve(0);
        if(pCurve)     pCurve->appendPoint(double(m_IterTotal+pOptEvent->iter()), wpolar.aeroForce(0).CL()/wpolar.aeroForce(0).CD());
        pCurve = m_ObjGraph[3].curve(0);
        if(pCurve)     pCurve->appendPoint(double(m_IterTotal+pOptEvent->iter()), sqrt(wpolar.aeroForce(0).CL()*wpolar.aeroForce(0).CL()*wpolar.aeroForce(0).CL())/wpolar.aeroForce(0).CD());
        pCurve = m_ObjGraph[4].curve(0);
        if(pCurve)     pCurve->appendPoint(double(m_IterTotal+pOptEvent->iter()), wpolar.aeroForce(0).Cm());
        pCurve = m_ObjGraph[5].curve(0);
        if(pCurve)     pCurve->appendPoint(double(m_IterTotal+pOptEvent->iter()), wpolar.m_Mass_var.at(0)*9.81*wpolar.Vz(0));
        pCurve = m_ObjGraph[6].curve(0);
        if(pCurve)     pCurve->appendPoint(double(m_IterTotal+pOptEvent->iter()), wpolar.m_MaxBending.at(0));

        for(int io=0; io<NOBJECTIVES; io++)
        {
            m_ObjGraph[io].invalidate();
            m_ObjGraph[io].resetLimits();
            m_pObjGraphWt[io]->update();
        }
        int iActiveVar=0;
        double factor=1.0;
        QString labunit, name;
        int w=0;
        for(int ivar=0; ivar<m_BestParticle.dimension(); ivar++)
        {
            OptVariable const &var = m_pPlaneXfl->m_OptVariables.at(ivar);
            if(var.m_Max-var.m_Min>DELTAVAR)
            {
                getUnit(ivar, factor, labunit);
                Curve *pCurve = m_VariableGraph.curve(iActiveVar);
                if(!pCurve)
                {
                    pCurve = m_VariableGraph.addCurve();
                    pCurve->setName(variableName(ivar));
                }
                double pos = m_BestParticle.pos(ivar);
                double rel = (pos-var.m_Min) / (var.m_Max-var.m_Min);
                pCurve->appendPoint(double(m_IterTotal+pOptEvent->iter()), rel*100.0);
                name = variableName(ivar) + QString::asprintf(" = %7.3f ", m_BestParticle.pos(ivar)*factor) + labunit;
                pCurve->setName(name);
                w = std::max(w, DisplayOptions::textFontStruct().width(name));
                iActiveVar++;
            }
        }
        m_pParetoGraphWt->setLegendOrigin({m_pParetoGraphWt->width()-w-5, m_pParetoGraphWt->height()+13});

        m_VariableGraph.invalidate();
        m_VariableGraph.resetXLimits();

        fillResults();
        updateParetoViews(pOptEvent->iBest());

    }
    delete pPlaneXfl;

    update();

}


void OptimPlaneDlg::setObjectives(QVector<OptObjective> const &objectives)
{
    m_Objective=objectives;
    if(m_Objective.size()<=6)
        m_Objective.append({"BM",          6, false,     1.0,  0.100, xfl::MINIMIZE});
}
