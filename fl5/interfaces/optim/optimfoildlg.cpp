/****************************************************************************

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


#


#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QHeaderView>
#include <QButtonGroup>
#include <QFontMetrics>
#include <QDebug>

#include "optimfoildlg.h"

#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/editors/foiledit/foilwt.h>
#include <interfaces/graphs/containers/graphwt.h>
#include <interfaces/graphs/controls/graphoptions.h>
#include <interfaces/graphs/graph/curve.h>
#include <interfaces/widgets/customdlg/renamedlg.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/ctrltabledelegate.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/customwts/xfldelegate.h>
#include <api/constants.h>
#include <api/xfoiltask.h>
#include <api/foil.h>
#include <api/polar.h>
#include <api/objects2d.h>
#include <api/optstructures.h>
#include <interfaces/optim/psotask.h>
#include <interfaces/optim/psotaskfoil.h>
#include <api/utils.h>

QByteArray OptimFoilDlg::s_LeftSplitterSizes;
QByteArray OptimFoilDlg::s_HSplitterSizes;
QByteArray OptimFoilDlg::s_VSplitterSizes;
QByteArray OptimFoilDlg::s_Geometry;


PSOTaskFoil::enumMod OptimFoilDlg::s_ModType = PSOTaskFoil::SCALE;

OptimizationPoint OptimFoilDlg::s_Opt[NOPT];

int OptimFoilDlg::s_NOpt = 1;


double OptimFoilDlg::s_FlapAngleMin=0;
double OptimFoilDlg::s_FlapAngleMax=10;
double OptimFoilDlg::s_XHinge=0.7;
double OptimFoilDlg::s_YHinge=0.5;


OptObjective OptimFoilDlg::s_Objective[NOPT][NOBJECTIVES];


int    OptimFoilDlg::s_HHn       = 6;
double OptimFoilDlg::s_HHt1      = 1.7;
double OptimFoilDlg::s_HHt2      = 1.0;
double OptimFoilDlg::s_HHmax     = 0.01;

double OptimFoilDlg::s_Thick[2]  = { 0.1,  0.15};
double OptimFoilDlg::s_Camb[2]   = { 0.00, 0.05};
double OptimFoilDlg::s_XThick[2] = { 0.25, 0.5};
double OptimFoilDlg::s_XCamb[2]  = { 0.25, 0.5};


OptimFoilDlg::OptimFoilDlg(QWidget *pParent) : XflDialog(pParent)
{
    setWindowTitle("foil 2d optimization");

    m_pFoil=nullptr;

    m_pBestFoil = new Foil;

    for(int iopt=0; iopt<NOPT; iopt++) m_pPolar[iopt] = new Polar;

    m_bIsSwarmValid  = false;

    m_iLE = -1;

    m_bModified = false;
    m_bSaved = true;

    m_pPSOTaskFoil = nullptr;

    setupLayout();

    switch(s_ModType)
    {
        case PSOTaskFoil::HH:
            m_prbHH->setChecked(true);
            m_prbScale->setChecked(false);
            m_pswVarWidget->setCurrentIndex(0);
            break;
        case PSOTaskFoil::SCALE:
            m_prbHH->setChecked(false);
            m_prbScale->setChecked(true);
            m_pswVarWidget->setCurrentIndex(1);
            break;
    }

    onNOpt(s_NOpt-1);

    fillOptPoints();
    fillObjectives();

    connectSignals();
    onPlotHH();

}


OptimFoilDlg::~OptimFoilDlg()
{
    for(int i=0; i<m_TempFoils.size(); i++) delete m_TempFoils[i];

    delete m_pBestFoil;
    for(int iopt=0; iopt<NOPT; iopt++) delete m_pPolar[iopt];
    if(m_pPSOTaskFoil) delete m_pPSOTaskFoil;

    for(int iopt=0; iopt<NOPT; iopt++)
        m_CpGraph[iopt].deleteCurveModel();
}


void OptimFoilDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pHSplitter = new QSplitter(::Qt::Horizontal);
        {
            m_pHSplitter->setChildrenCollapsible(false);
            QFrame *pLeftFrame = new QFrame();
            {
                QVBoxLayout *pLeftLayout = new QVBoxLayout;
                {
                    m_pLeftSplitter = new QSplitter(Qt::Vertical);
                    {
                        QTabWidget *ptwMain = new QTabWidget;
                        {
                            QFrame *pVarPage = new QFrame;
                            {
                                QVBoxLayout *pVarLayout = new QVBoxLayout;
                                {
                                    QGroupBox *pgbGeomBox = new QGroupBox("Foil geometry");
                                    {
                                        QVBoxLayout *pGeomLayout = new QVBoxLayout;
                                        {
                                            QHBoxLayout *pSelLayout = new QHBoxLayout;
                                            {
                                                QButtonGroup *pGroup = new QButtonGroup;
                                                {
                                                    m_prbHH    = new QRadioButton("HH bump functions");
                                                    m_prbScale = new QRadioButton("Camber and thickness");
                                                    pGroup->addButton(m_prbHH);
                                                    pGroup->addButton(m_prbScale);
                                                }
                                                pSelLayout->addStretch();
                                                pSelLayout->addWidget(m_prbHH);
                                                pSelLayout->addWidget(m_prbScale);
                                                pSelLayout->addStretch();
                                            }

                                            m_pswVarWidget = new QStackedWidget;
                                            {
                                                QFrame *pHHFrame = new QFrame;
                                                {
                                                    QVBoxLayout *pHHPageLayout = new QVBoxLayout;
                                                    {
                                                        QGridLayout *pHHLayout = new QGridLayout;
                                                        {
                                                            QLabel *plabEq = new QLabel;

                                                            QColor back = this->palette().window().color();
                                                            if(back.valueF()<0.5f)
                                                                plabEq->setPixmap(QPixmap(QString::fromUtf8(":/images/HH_inv.png")));
                                                            else
                                                                plabEq->setPixmap(QPixmap(QString::fromUtf8(":/images/HH.png")));

                                                            QLabel *plabNHH = new QLabel("Nb. of functions/side:");
                                                            m_pieNHH = new IntEdit(s_HHn/2); // per side
                                                            m_pieNHH->setToolTip("<p>The number of design variables is twice the number of bump functions.\nRecommendation: n=3</p>");
                                                            QLabel *plabt1 = new QLabel("t1:");
                                                            m_pdeHHt1 = new FloatEdit(s_HHt1);
                                                            m_pdeHHt1->setToolTip("<p>This parameter controls the location of the maximum point of the bump functions.</br>"
                                                                                  "Recommendation: 1 <= t1<= 2</p>");
                                                            QLabel *plabt2 = new QLabel("t2:");
                                                            m_pdeHHt2 = new FloatEdit(s_HHt2);
                                                            m_pdeHHt2->setToolTip("<p>This parameter controls the width of the bump functions.<br>"
                                                                                  "Recommendation: 0.5 <= t2 <= 2</p>");
                                                            QLabel *plabLax = new QLabel("Max. HH amplitude:");
                                                            m_pdeHHmax = new FloatEdit(s_HHmax*100);
                                                            m_pdeHHmax->setToolTip("<p>This parameter controls the amplitude of the bump functions.<br>"
                                                                                   "The greater the amplitude the larger is  the design space, "
                                                                                   "however large amplitudes will lead to wobbly surfaces and may hinder XFoil's convergence.<br>"
                                                                                   "Recommendation; 1.5% or less.</p>");
                                                            QLabel *plabPercent = new QLabel("% Ch.");

                                                            pHHLayout->addWidget(plabNHH,     1, 1);
                                                            pHHLayout->addWidget(m_pieNHH,    1, 2);
                                                            pHHLayout->addWidget(plabLax,     2, 1);
                                                            pHHLayout->addWidget(m_pdeHHmax,  2, 2);
                                                            pHHLayout->addWidget(plabPercent, 2, 3);
                                                            pHHLayout->addWidget(plabt1,      3, 1);
                                                            pHHLayout->addWidget(m_pdeHHt1,   3, 2);
                                                            pHHLayout->addWidget(plabt2,      4, 1);
                                                            pHHLayout->addWidget(m_pdeHHt2,   4, 2);

                                                            pHHLayout->addWidget(plabEq,5,1,1,3,  Qt::AlignCenter);

                                                            //                                        pHHLayout->setRowStretch(6,1);
                                                            pHHLayout->setColumnStretch(1, 1);
                                                            pHHLayout->setColumnStretch(2, 1);
                                                        }

                                                        m_pHHGraphWt = new GraphWt;
                                                        {
                                                            //                                        m_pHHGraphWt->setSizePolicy(QSizePolicy::MinimumExpanding,  QSizePolicy::MinimumExpanding);
                                                            m_pHHGraphWt->setGraph(&m_HHGraph);

                                                            m_HHGraph.setCurveModel(new CurveModel);
                                                            m_HHGraph.setXVariableList({"x"});
                                                            m_HHGraph.setYVariableList({"Amplitude %Chord"});
                                                            m_HHGraph.setAuto(true);
                                                            GraphOptions::resetGraphSettings(m_HHGraph);
                                                        }

                                                        pHHPageLayout->addLayout(pHHLayout);
                                                        pHHPageLayout->addWidget(m_pHHGraphWt);
                                                        pHHPageLayout->setStretchFactor(pHHLayout,1);
                                                        pHHPageLayout->setStretchFactor(m_pHHGraphWt,5);
                                                    }

                                                    pHHFrame->setLayout(pHHPageLayout);
                                                }

                                                QFrame *pScaleFrame = new QFrame;
                                                {
                                                    QVBoxLayout *pGeomLayout = new QVBoxLayout;
                                                    {
                                                        m_plabFoilInfo = new QLabel;
                                                        m_plabFoilInfo->setFont(DisplayOptions::tableFont());
                                                        QGridLayout *pLimitLayout = new QGridLayout;
                                                        {
                                                            QLabel *plabMin    = new QLabel("Min.");
                                                            QLabel *plabMax    = new QLabel("Max.");
                                                            QLabel *plabThick  = new QLabel("Thickness:");
                                                            QLabel *plabCamb   = new QLabel("Camber:");
                                                            QLabel *plabXThick = new QLabel("Max. thickness x-pos.:");
                                                            QLabel *plabXCamb  = new QLabel("Max. camber x-pos.:");
                                                            QFont const &fnt = plabMin->font();
                                                            QFontMetrics fm(fnt);
                                                            plabXThick->setMinimumWidth(fm.averageCharWidth()*25);

                                                            for(int i=0; i<2; i++)
                                                            {
                                                                m_pfeThick[i]    = new FloatEdit(s_Thick[i]*100.0);
                                                                m_pfeXThick[i]   = new FloatEdit(s_XThick[i]*100.0);
                                                                m_pfeCamb[i]     = new FloatEdit(s_Camb[i]*100.0);
                                                                m_pfeXCamb[i]    = new FloatEdit(s_XCamb[i]*100.0);
                                                            }
                                                            pLimitLayout->addWidget(plabMin,         1,2, Qt::AlignCenter);
                                                            pLimitLayout->addWidget(plabMax,         1,3, Qt::AlignCenter);
                                                            pLimitLayout->addWidget(plabThick,       2,1);
                                                            pLimitLayout->addWidget(m_pfeThick[0],   2,2);
                                                            pLimitLayout->addWidget(m_pfeThick[1],   2,3);
                                                            pLimitLayout->addWidget(new QLabel("%"), 2,4);
                                                            pLimitLayout->addWidget(plabCamb,        3,1);
                                                            pLimitLayout->addWidget(m_pfeCamb[0],    3,2);
                                                            pLimitLayout->addWidget(m_pfeCamb[1],    3,3);
                                                            pLimitLayout->addWidget(new QLabel("%"), 3,4);
                                                            pLimitLayout->addWidget(plabXThick,      4,1);
                                                            pLimitLayout->addWidget(m_pfeXThick[0],  4,2);
                                                            pLimitLayout->addWidget(m_pfeXThick[1],  4,3);
                                                            pLimitLayout->addWidget(new QLabel("%"), 4,4);
                                                            pLimitLayout->addWidget(plabXCamb,       5,1);
                                                            pLimitLayout->addWidget(m_pfeXCamb[0],   5,2);
                                                            pLimitLayout->addWidget(m_pfeXCamb[1],   5,3);
                                                            pLimitLayout->addWidget(new QLabel("%"), 5,4);

                                                            pLimitLayout->setRowStretch(6,1);
                                                            pLimitLayout->setColumnStretch(5,1);
                                                        }
                                                        pGeomLayout->addWidget(m_plabFoilInfo);
                                                        pGeomLayout->addLayout(pLimitLayout);
                                                    }
                                                    pScaleFrame->setLayout(pGeomLayout);
                                                }

                                                m_pswVarWidget->addWidget(pHHFrame);
                                                m_pswVarWidget->addWidget(pScaleFrame);
                                            }

                                            pGeomLayout->addLayout(pSelLayout);
                                            pGeomLayout->addWidget(m_pswVarWidget);
                                        }
                                        pgbGeomBox->setLayout(pGeomLayout);
                                    }


                                    QGroupBox *pgbFlap = new QGroupBox("T.E. flap");
                                    {
                                        QGridLayout *pFlapDataLayout = new QGridLayout;
                                        {
                                            m_pdeTEXHinge      = new FloatEdit(s_XHinge*100.0);
                                            m_pdeTEYHinge      = new FloatEdit(s_YHinge*100.0);

                                            QLabel *pLabXHinge = new QLabel("Hinge X Position");
                                            QLabel *pLab4 = new QLabel("% Chord");
                                            QLabel *pLabYHinge = new QLabel("Hinge Y Position");
                                            QLabel *pLab6 = new QLabel("% Thickness");


                                            QLabel *plabMin    = new QLabel("Min.");
                                            QLabel *plabMax    = new QLabel("Max.");
                                            m_pdeFlapAngleMin  = new FloatEdit(s_FlapAngleMin);
                                            m_pdeFlapAngleMax  = new FloatEdit(s_FlapAngleMax);


                                            QLabel *plabAngle = new QLabel("Flap angle range:");
                                            QLabel *pLab2 = new QLabel(DEGch);

                                            pFlapDataLayout->addWidget(plabMin,           6, 2, Qt::AlignCenter);
                                            pFlapDataLayout->addWidget(plabMax,           6, 3, Qt::AlignCenter);
                                            pFlapDataLayout->addWidget(plabAngle,         7, 1);
                                            pFlapDataLayout->addWidget(m_pdeFlapAngleMin, 7, 2);
                                            pFlapDataLayout->addWidget(m_pdeFlapAngleMax, 7, 3);
                                            pFlapDataLayout->addWidget(pLab2,             7, 4);

                                            pFlapDataLayout->addWidget(pLabXHinge,        3, 1);
                                            pFlapDataLayout->addWidget(m_pdeTEXHinge,     3, 2);
                                            pFlapDataLayout->addWidget(pLab4,             3, 3);

                                            pFlapDataLayout->addWidget(pLabYHinge,        4, 1);
                                            pFlapDataLayout->addWidget(m_pdeTEYHinge,     4, 2);
                                            pFlapDataLayout->addWidget(pLab6,             4, 3);

                                            pFlapDataLayout->setRowStretch(2,1);
                                            pFlapDataLayout->setRowStretch(5,1);
                                            pFlapDataLayout->setRowStretch(8,7);
                                        }
                                        pgbFlap->setLayout(pFlapDataLayout);
                                    }

                                    pVarLayout->addWidget(pgbGeomBox);
                                    pVarLayout->addWidget(pgbFlap);
                                    pVarLayout->addStretch();
                                }
                                pVarPage->setLayout(pVarLayout);
                            }

                            QFrame *pAlgoPage= new QFrame;
                            {
                                QGridLayout *pMOSPSOLayout = new QGridLayout;
                                {
                                    QLabel *plabPopSize = new QLabel("Swarm size:");
                                    m_piePopSize = new IntEdit(PSOTask::s_PopSize);
                                    m_piePopSize->setToolTip("Recommendation: 7 to 15");

                                    QLabel *plabMaxIter = new QLabel("Max. iterations:");
                                    m_pieMaxIter = new IntEdit(PSOTask::s_MaxIter);

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

                                    QLabel *plabProbRegen = new QLabel("Regeneration probability:");
                                    m_pdeProbaRegen = new FloatEdit(PSOTask::s_ProbRegenerate*100.0);
                                    m_pdeProbaRegen->setToolTip("<p>The probability that a particle will be re-created at a random position at each iteration.<br>"
                                                                "Increases the likelyhood that the swarm will not get stuck on a local minimum.<br>"
                                                                "Recommendation: 5% to 25%</p>");
                                    QLabel *pLabPercent = new QLabel("%");

                                    m_pchMultithread = new QCheckBox("Multithreaded");
                                    m_pchMultithread->setChecked(PSOTask::s_bMultiThreaded);

                                    pMOSPSOLayout->addWidget(plabPopSize,          1, 1);
                                    pMOSPSOLayout->addWidget(m_piePopSize,         1, 2);

                                    pMOSPSOLayout->addWidget(plabMaxIter,          2, 1);
                                    pMOSPSOLayout->addWidget(m_pieMaxIter,         2, 2);

                                    pMOSPSOLayout->addWidget(plabInertia,          5, 1);
                                    pMOSPSOLayout->addWidget(m_pdeInertiaWeight,   5, 2);

                                    pMOSPSOLayout->addWidget(plabCognitive,        6, 1);
                                    pMOSPSOLayout->addWidget(m_pdeCognitiveWeight, 6, 2);

                                    pMOSPSOLayout->addWidget(plabSocial,           7, 1);
                                    pMOSPSOLayout->addWidget(m_pdeSocialWeight,    7, 2);

                                    pMOSPSOLayout->addWidget(plabProbRegen,        8, 1);
                                    pMOSPSOLayout->addWidget(m_pdeProbaRegen,      8, 2);
                                    pMOSPSOLayout->addWidget(pLabPercent,          8, 3);

                                    pMOSPSOLayout->addWidget(m_pchMultithread,     9, 1);

                                    pMOSPSOLayout->setRowStretch(10,1);
                                }

                                pAlgoPage->setLayout(pMOSPSOLayout);
                            }

                            QFrame *pOptPointsPage = new QFrame;
                            {
                                QVBoxLayout *pOptPointsLayout = new QVBoxLayout;
                                {
                                    QLabel *plabOptPoints = new QLabel("Operating points:");
                                    QHBoxLayout *pNOptLayout = new QHBoxLayout;
                                    {
                                        QLabel *plabNOpt = new QLabel("Nbr. of optimization points=");
                                        m_pcbNOpt = new QComboBox;
                                        QStringList values = {"1", "2", "3", "4"};
                                        m_pcbNOpt->addItems(values);
                                        m_pcbNOpt->setCurrentIndex(s_NOpt-1);

                                        pNOptLayout->addWidget(plabNOpt);
                                        pNOptLayout->addWidget(m_pcbNOpt);
                                        pNOptLayout->addStretch();
                                    }

                                    m_pcptOptPoints = new CPTableView(this);
                                    m_pcptOptPoints->setEditable(true);
                                    m_pcptOptPoints->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

                                    m_pcptOptPoints->setWindowTitle("Optimization points");

                                    m_pOptPointsModel = new QStandardItemModel(this);
                                    m_pOptPointsModel->setColumnCount(6);
                                    m_pOptPointsModel->setHeaderData(0, Qt::Horizontal, "Name");
                                    m_pOptPointsModel->setHeaderData(1, Qt::Horizontal, "Opt. 1");
                                    m_pOptPointsModel->setHeaderData(2, Qt::Horizontal, "Opt. 2");
                                    m_pOptPointsModel->setHeaderData(3, Qt::Horizontal, "Opt. 3");
                                    m_pOptPointsModel->setHeaderData(4, Qt::Horizontal, "Opt. 4");
                                    m_pOptPointsModel->setHeaderData(5, Qt::Horizontal, "Unit");
                                    m_pcptOptPoints->setModel(m_pOptPointsModel);
                                    m_pcptOptPoints->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
                                    m_pcptOptPoints->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
                                    m_pcptOptPoints->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
                                    m_pcptOptPoints->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
                                    m_pcptOptPoints->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
                                    m_pcptOptPoints->horizontalHeader()->setStretchLastSection(false);

                                    m_pOptPointsDelegate = new XflDelegate(this);
                                    m_pOptPointsDelegate->setCheckColumn(-1);
                                    m_pOptPointsDelegate->setActionColumn(-1);
                                    m_pOptPointsDelegate->setDigits({-1,0,0,0,0,-1});
                                    m_pOptPointsDelegate->setItemTypes({XflDelegate::STRING, XflDelegate::INTEGER, XflDelegate::INTEGER, XflDelegate::INTEGER, XflDelegate::INTEGER, XflDelegate::STRING});

                                    m_pcptOptPoints->setItemDelegate(m_pOptPointsDelegate);

                                    QLabel *plabObjectives = new QLabel("Targets:");
                                    m_pcptObjective = new CPTableView;
                                    {
                                        m_pcptObjective->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                                        m_pcptObjective->setEditable(true);
                                        m_pObjModel = new QStandardItemModel(this);
                                        m_pObjModel->setRowCount(1);//temporary
                                        m_pObjModel->setColumnCount(8);
                                        m_pObjModel->setHeaderData(0, Qt::Horizontal, "Active");
                                        m_pObjModel->setHeaderData(1, Qt::Horizontal, "Objective");
                                        m_pObjModel->setHeaderData(2, Qt::Horizontal, "Type");
                                        m_pObjModel->setHeaderData(3, Qt::Horizontal, "Opt. 1");
                                        m_pObjModel->setHeaderData(4, Qt::Horizontal, "Opt. 2");
                                        m_pObjModel->setHeaderData(5, Qt::Horizontal, "Opt. 3");
                                        m_pObjModel->setHeaderData(6, Qt::Horizontal, "Opt. 4");
                                        m_pObjModel->setHeaderData(7, Qt::Horizontal, "Max. error");

                                        m_pcptObjective->setModel(m_pObjModel);
                                        m_pcptObjective->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
                                        m_pcptObjective->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
                                        m_pcptObjective->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
                                        m_pcptObjective->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);


                                        for(int icol=0; icol<m_pObjModel->columnCount(); icol++)
                                            m_pObjModel->setHeaderData(icol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);


                                        m_pObjDelegate = new XflDelegate(this);
                                        m_pObjDelegate->setCheckColumn(0);
                                        m_pObjDelegate->setActionColumn(-1);
                                        m_pObjDelegate->setDigits({-1,-1,0,3,3,3,3,3});
                                        m_pObjDelegate->setItemTypes({XflDelegate::CHECKBOX, XflDelegate::STRING, XflDelegate::INTEGER,
                                                                     XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::DOUBLE});
                                        m_pObjDelegate->setName("Objectives");
                                        m_pcptObjective->setItemDelegate(m_pObjDelegate);
                                    }

                                    m_ppbRunXFoil = new QPushButton("Calculate optimization points");
                                    m_ppbRunXFoil->setToolTip("<p>Run XFoil on the active optimization points.<br>"
                                                              "TE flaps are set to neutral position.<br>"
                                                              "This is for information only.</p>");

                                    pOptPointsLayout->addLayout(pNOptLayout);

                                    pOptPointsLayout->addWidget(plabOptPoints);
                                    pOptPointsLayout->addWidget(m_pcptOptPoints);

                                    pOptPointsLayout->addSpacing(31);

                                    pOptPointsLayout->addWidget(plabObjectives);
                                    pOptPointsLayout->addWidget(m_pcptObjective);
                                    pOptPointsLayout->addWidget(m_ppbRunXFoil);
                                }

                                pOptPointsPage->setLayout(pOptPointsLayout);
                            }

                            QFrame *pTargetPage = new QFrame;
                            {
                                QVBoxLayout *pObjectiveLayout  = new QVBoxLayout;
                                {
                                    m_ppbMakeSwarm = new QPushButton("Make random population");
                                    m_ppbSwarm = new QPushButton("Swarm");
                                    m_ppbStoreBestFoil = new QPushButton("Store best foil");
                                    m_ppbStoreBestFoil->setToolTip("Adds the current best foil to the database");
                                    m_ppbContinueBest = new QPushButton("Continue from current best");
                                    m_ppbContinueBest->setToolTip("Uses the current best foil as the basis for further optimization");

                                    pObjectiveLayout->addStretch();
                                    pObjectiveLayout->addWidget(m_ppbMakeSwarm);
                                    pObjectiveLayout->addWidget(m_ppbSwarm);
                                    pObjectiveLayout->addWidget(m_ppbStoreBestFoil);
                                    pObjectiveLayout->addWidget(m_ppbContinueBest);
                                    pObjectiveLayout->addStretch();
                                }
                                pTargetPage->setLayout(pObjectiveLayout);
                            }

                            ptwMain->addTab(pVarPage,       "Variables");
                            ptwMain->addTab(pAlgoPage,      "MOPSO");
                            ptwMain->addTab(pOptPointsPage, "Optimization points");
                            ptwMain->addTab(pTargetPage,    "Optimizer");

                            connect(ptwMain,  SIGNAL(currentChanged(int)), SLOT(onResizeColumns()));
                        }

                        m_ppto = new PlainTextOutput;
                        QFont fixedfnt(QFontDatabase::systemFont(QFontDatabase::FixedFont));
                        m_ppto->setFont(fixedfnt);

                        m_pLeftSplitter->addWidget(ptwMain);
                        m_pLeftSplitter->addWidget(m_ppto);

                        m_pLeftSplitter->setStretchFactor(1,1);
                        m_pLeftSplitter->setStretchFactor(2,5);

                        m_pLeftSplitter->setChildrenCollapsible(false);
                    }

                    QHBoxLayout  *pBottomLayout = new QHBoxLayout;
                    {
                        QLabel *pFlow5Link = new QLabel;
                        pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Tutorials/Optim2d.html>https://flow5.tech/docs/.../Optim2d.html</a>");
                        pFlow5Link->setOpenExternalLinks(true);
                        pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
                        pFlow5Link->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

                        setButtons(QDialogButtonBox::Close);
                        QPushButton *ppbClear = new QPushButton("Clear");
                        connect(ppbClear, SIGNAL(clicked()), m_ppto, SLOT(clear()));
                        m_pButtonBox->addButton(ppbClear, QDialogButtonBox::ActionRole);
                        pBottomLayout->addWidget(pFlow5Link);
                        pBottomLayout->addStretch();
                        pBottomLayout->addWidget(m_pButtonBox);
                    }

                    pLeftLayout->addWidget(m_pLeftSplitter);
                    pLeftLayout->addLayout(pBottomLayout);
                }
                pLeftFrame->setLayout(pLeftLayout);
            }

            m_pVSplitter = new QSplitter(Qt::Vertical);
            {
                m_pVSplitter->setChildrenCollapsible(false);

                m_pfrCpGraph = new QFrame;
                {
                    QGridLayout *pfrLayout = new QGridLayout;
                    {
                        for(int iopt=0; iopt<NOPT; iopt++)
                        {
                            m_pCpGraphWt[iopt] = new GraphWt;
                            m_CpGraph[iopt].setCurveModel(new CurveModel);
                            m_CpGraph[iopt].setXVariableList({"x"});
                            m_CpGraph[iopt].setYVariableList({"Cp"});
                            m_CpGraph[iopt].setScaleType(GRAPH::EXPANDING);
                            m_CpGraph[iopt].setYInverted(0, true);
                            GraphOptions::resetGraphSettings(m_CpGraph[iopt]);
                            m_pCpGraphWt[iopt]->setGraph(&m_CpGraph[iopt]);
                            m_pCpGraphWt[iopt]->showLegend(true);

                        }
                        // GraphWts are added to layout later during initialization
                    }
                    m_pfrCpGraph->setLayout(pfrLayout);
                }

                m_pFoilWt = new FoilWt;

                m_pVSplitter->addWidget(m_pfrCpGraph);
                m_pVSplitter->addWidget(m_pFoilWt);
                m_pVSplitter->setStretchFactor(0,1);
            }

            m_pHSplitter->addWidget(pLeftFrame);
            m_pHSplitter->addWidget(m_pVSplitter);
            m_pHSplitter->setStretchFactor(1,1);
        }

        pMainLayout->addWidget(m_pHSplitter);

    }
    setLayout(pMainLayout);
}


void OptimFoilDlg::connectSignals()
{
    connect(m_pcbNOpt,          SIGNAL(activated(int)),         SLOT(onNOpt(int)));

    connect(m_pObjModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onCellChanged(QModelIndex,QModelIndex)));
    connect(m_pcptObjective,    SIGNAL(pressed(QModelIndex)),    SLOT(onObjTableClicked(QModelIndex)));

    connect(m_prbHH,            SIGNAL(clicked()),               SLOT(onVarType()));
    connect(m_prbScale,         SIGNAL(clicked()),               SLOT(onVarType()));

    connect(m_pHSplitter,       SIGNAL(splitterMoved(int,int)),  SLOT(onResizeColumns()));

    connect(m_ppbRunXFoil,      SIGNAL(clicked()),               SLOT(onRunXFoil()));
    connect(m_ppbMakeSwarm,     SIGNAL(clicked()),               SLOT(onMakeSwarm()));
    connect(m_ppbSwarm,         SIGNAL(clicked()),               SLOT(onRunOptimizer()));
    connect(m_ppbStoreBestFoil, SIGNAL(clicked()),               SLOT(onStoreBestFoil()));
    connect(m_ppbContinueBest,  SIGNAL(clicked()),               SLOT(onContinueBest()));

    connect(m_pieNHH,           SIGNAL(intChanged(int)),         SLOT(onPlotHH()));
    connect(m_pdeHHt1,          SIGNAL(floatChanged(float)),     SLOT(onPlotHH()));
    connect(m_pdeHHt2,          SIGNAL(floatChanged(float)),     SLOT(onPlotHH()));
    connect(m_pdeHHmax,         SIGNAL(floatChanged(float)),     SLOT(onPlotHH()));
}


void OptimFoilDlg::onNOpt(int NOpp)
{
    s_NOpt = NOpp+1;

//    for(int iopt=0; iopt<NOPT; iopt++)
//        m_pOptPointsDelegate->setEditable(iopt+1, iopt<s_NOpt);

    updateCpGraphLayout();

    for(int iopt=0; iopt<NOPT; iopt++)
        m_pcptOptPoints->setColumnHidden(iopt+1, iopt>=s_NOpt);


    for(int iopt=0; iopt<NOPT; iopt++)
        m_pcptObjective->setColumnHidden(iopt+3, iopt>=s_NOpt);

    m_pcptOptPoints->viewport()->update();
    m_pcptObjective->viewport()->update();
}


void OptimFoilDlg::updateCpGraphLayout()
{
    QGridLayout *pLayout = dynamic_cast<QGridLayout*>(m_pfrCpGraph->layout());
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        pLayout->removeWidget(m_pCpGraphWt[iopt]);
    }

    switch(s_NOpt)
    {
        default:
        case 1:
        {
            pLayout->addWidget(m_pCpGraphWt[0], 1, 1);
            m_pCpGraphWt[0]->setVisible(true);
            m_pCpGraphWt[1]->setVisible(false);
            m_pCpGraphWt[2]->setVisible(false);
            m_pCpGraphWt[3]->setVisible(false);
            break;
        }
        case 2:
        {
            pLayout->addWidget(m_pCpGraphWt[0], 1, 1);
            pLayout->addWidget(m_pCpGraphWt[1], 1, 2);
            m_pCpGraphWt[0]->setVisible(true);
            m_pCpGraphWt[1]->setVisible(true);
            m_pCpGraphWt[2]->setVisible(false);
            m_pCpGraphWt[3]->setVisible(false);
            break;
        }
        case 3:
        {
            pLayout->addWidget(m_pCpGraphWt[0], 1, 1);
            pLayout->addWidget(m_pCpGraphWt[1], 1, 2);
            pLayout->addWidget(m_pCpGraphWt[2], 1, 3);
            m_pCpGraphWt[0]->setVisible(true);
            m_pCpGraphWt[1]->setVisible(true);
            m_pCpGraphWt[2]->setVisible(true);
            m_pCpGraphWt[3]->setVisible(false);
            break;
        }
        case 4:
        {
            pLayout->addWidget(m_pCpGraphWt[0], 1, 1);
            pLayout->addWidget(m_pCpGraphWt[1], 1, 2);
            pLayout->addWidget(m_pCpGraphWt[2], 2, 1);
            pLayout->addWidget(m_pCpGraphWt[3], 2, 2);
            m_pCpGraphWt[0]->setVisible(true);
            m_pCpGraphWt[1]->setVisible(true);
            m_pCpGraphWt[2]->setVisible(true);
            m_pCpGraphWt[3]->setVisible(true);
            break;
        }
    }
}


void OptimFoilDlg::onResizeColumns()
{
/*    int nCols=m_pObjModel->columnCount()-1;
    QHeaderView *pHHeader = m_pcptObjective->horizontalHeader();
    pHHeader->setSectionResizeMode(nCols, QHeaderView::Stretch);
    pHHeader->resizeSection(nCols, 1); // 1 pixel to be resized automatically
    double wt = double(m_pcptObjective->width());
    double wcheck = int(wt/10.0);
    int wType = int(wt/9.0);
    int wopp = int((wt-wcheck)/double(3)*0.95);
    m_pcptObjective->setColumnWidth(0, wcheck);
    m_pcptObjective->setColumnWidth(1, wopp);
    m_pcptObjective->setColumnWidth(2, wType);
    m_pcptObjective->setColumnWidth(3, wopp);*/
}


void OptimFoilDlg::onObjTableClicked(QModelIndex index)
{
    if(index.row()>=m_pObjModel->rowCount()) return;
    if(index.column()!=0) return;
    bool bActive = m_pObjModel->data(index, Qt::UserRole).toBool(); // use a QVariant with the EditRole rather thant the QtCheckStateRole - not interested in Qt::PartiallyChecked
    bActive = !bActive; // toggle
    // update the range
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        s_Objective[iopt][index.row()].m_bActive = bActive;
    }
    QModelIndex chindex = m_pObjModel->index(index.row(), 0);
    m_pObjModel->setData(chindex, bActive, Qt::UserRole);

/*    for(int  col=2; col<8; col++)
    {
        if(bActive)
            m_pObjModel->item(index.row(), col)->setFlags(Qt::ItemIsEnabled);
        else
            m_pObjModel->item(index.row(), col)->setFlags(Qt::NoItemFlags);
    }*/

    m_pcptObjective->selectRow(chindex.row()); // only way found to force the repaint
    m_pcptObjective->setCurrentIndex(chindex);

    update();
}


void OptimFoilDlg::onCellChanged(QModelIndex,QModelIndex)
{
    readObjectives();
    m_pcptObjective->viewport()->update();
/*    m_pcptObjective->update();
    update();*/
}


void OptimFoilDlg::reject()
{
    onClose();
}


void OptimFoilDlg::onClose()
{
    cancelTask();

    if(!m_bSaved)
    {
        int resp = QMessageBox::question(this, "Question", "Save current best?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if(resp == QMessageBox::Yes)
            onStoreBestFoil();
        else if(resp == QMessageBox::Cancel)
            return;
    }
    accept();
}


void OptimFoilDlg::showEvent(QShowEvent *pEvent)
{
    XflDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
    if(s_LeftSplitterSizes.length()>0) m_pLeftSplitter->restoreState(s_LeftSplitterSizes);
    if(s_HSplitterSizes.length()>0)    m_pHSplitter->restoreState(s_HSplitterSizes);
    if(s_VSplitterSizes.length()>0)    m_pVSplitter->restoreState(s_VSplitterSizes);
    onResizeColumns();

}


void OptimFoilDlg::hideEvent(QHideEvent *pEvent)
{
    XflDialog::hideEvent(pEvent);
    readData();

    s_Geometry = saveGeometry();
    s_LeftSplitterSizes = m_pLeftSplitter->saveState();
    s_HSplitterSizes    = m_pHSplitter->saveState();
    s_VSplitterSizes    = m_pVSplitter->saveState();
}


void OptimFoilDlg::resizeEvent(QResizeEvent *pEvent)
{
    onResizeColumns();
    XflDialog::resizeEvent(pEvent);
}


void OptimFoilDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("OptimFoil");
    {
        for(int iopt=0; iopt<NOPT; iopt++)
        {
            s_Opt[iopt].m_Name         = settings.value(QString::asprintf("OptName_%d", iopt),    QString::asprintf("Optimization point %d", iopt+1)).toString();
            s_Opt[iopt].m_Alpha        = settings.value(QString::asprintf("Alpha_%d", iopt),      s_Opt[iopt].m_Alpha).toDouble();
            s_Opt[iopt].m_Re           = settings.value(QString::asprintf("Reynolds_%d", iopt),   s_Opt[iopt].m_Re).toDouble();
            s_Opt[iopt].m_Mach         = settings.value(QString::asprintf("Mach_%d", iopt),       s_Opt[iopt].m_Mach).toDouble();
            s_Opt[iopt].m_NCrit        = settings.value(QString::asprintf("NCrit_%d", iopt),      s_Opt[iopt].m_NCrit).toDouble();
            s_Opt[iopt].m_XtrTop       = settings.value(QString::asprintf("XtrTop_%d", iopt),     s_Opt[iopt].m_XtrTop).toDouble();
            s_Opt[iopt].m_XtrBot       = settings.value(QString::asprintf("XtrBot_%d", iopt),     s_Opt[iopt].m_XtrBot).toDouble();
        }

        s_FlapAngleMin    = settings.value("FlapAngleMin",    s_FlapAngleMin).toDouble();
        s_FlapAngleMax    = settings.value("FlapAngleMax",    s_FlapAngleMax).toDouble();
        s_XHinge          = settings.value("XHinge",          s_XHinge).toDouble();
        s_YHinge          = settings.value("YHinge",          s_YHinge).toDouble();

        s_NOpt            = settings.value("NOpt",            s_NOpt).toInt();

        for(int iopt=0; iopt<NOPT; iopt++)
        {
            s_Objective[iopt][0] = {"Cl",     0, true,   0.4,  1.e-3, xfl::MAXIMIZE};
            s_Objective[iopt][1] = {"Cd",     1, true,  0.01,  1.e-4, xfl::MINIMIZE};
            s_Objective[iopt][2] = {"Cl/Cd",  2, false, 31.0,  1.0,   xfl::MAXIMIZE};
            s_Objective[iopt][3] = {"Cp_min", 3, false, -0.5,  1.e-3, xfl::MAXIMIZE};
            s_Objective[iopt][4] = {"Cm",     4, false,  0.0,  1.e-3, xfl::EQUALIZE};

            for(int k=0; k<NOBJECTIVES; k++)
            {
                int ObjType = settings.value(QString::asprintf("ObjType_%d_%d", iopt, k), s_Objective[iopt][k].m_Type-1).toInt();
                switch(ObjType)
                {
                    default:
                    case  0:    s_Objective[iopt][k].m_Type = xfl::EQUALIZE;  break;
                    case -1:    s_Objective[iopt][k].m_Type = xfl::MINIMIZE;  break;
                    case  1:    s_Objective[iopt][k].m_Type = xfl::MAXIMIZE;  break;
                }

                s_Objective[iopt][k].m_bActive  = settings.value(QString::asprintf("Active_%d_%d", iopt, k),  s_Objective[iopt][k].m_bActive).toBool();
                s_Objective[iopt][k].m_Target   = settings.value(QString::asprintf("Target_%d_%d", iopt, k),  s_Objective[iopt][k].m_Target).toDouble();
                s_Objective[iopt][k].m_MaxError = settings.value(QString::asprintf("Error_%d_%d",  iopt, k),  s_Objective[iopt][k].m_MaxError).toDouble();
            }
        }

        int ModType = settings.value("ModType", s_ModType).toInt();
        if(ModType==0) s_ModType = PSOTaskFoil::HH; else s_ModType = PSOTaskFoil::SCALE;

        s_HHn             = settings.value("HHn",             s_HHn).toInt();
        s_HHt1            = settings.value("HHt1",            s_HHt1).toDouble();
        s_HHt2            = settings.value("HHt2",            s_HHt2).toDouble();
        s_HHmax           = settings.value("HHmax",           s_HHmax).toDouble();

        s_Thick[0]        = settings.value("ThickMin",        s_Thick[0]).toDouble();
        s_Thick[1]        = settings.value("ThickMax",        s_Thick[1]).toDouble();
        s_XThick[0]       = settings.value("XThickMin",       s_XThick[0]).toDouble();
        s_XThick[1]       = settings.value("XThickMax",       s_XThick[1]).toDouble();

        s_Camb[0]         = settings.value("CambMin",         s_Camb[0]).toDouble();
        s_Camb[1]         = settings.value("CambMax",         s_Camb[1]).toDouble();
        s_XCamb[0]        = settings.value("XCambMin",        s_XCamb[0]).toDouble();
        s_XCamb[1]        = settings.value("XCambMax",        s_XCamb[1]).toDouble();


        PSOTask::s_bMultiThreaded  = settings.value("bMultithread",    PSOTask::s_bMultiThreaded).toBool();
        PSOTask::s_PopSize         = settings.value("PopSize",         PSOTask::s_PopSize).toInt();
        PSOTask::s_MaxIter         = settings.value("MaxIter",         PSOTask::s_MaxIter).toInt();

        PSOTask::s_InertiaWeight   = settings.value("InertiaWeight",   PSOTask::s_InertiaWeight).toDouble();
        PSOTask::s_CognitiveWeight = settings.value("CognitiveWeight", PSOTask::s_CognitiveWeight).toDouble();
        PSOTask::s_SocialWeight    = settings.value("SocialWeight",    PSOTask::s_SocialWeight).toDouble();
        PSOTask::s_ProbRegenerate  = settings.value("ProbRegen",       PSOTask::s_ProbRegenerate).toDouble();

        s_Geometry           = settings.value("WindowGeom",        QByteArray()).toByteArray();
        s_LeftSplitterSizes  = settings.value("LeftSplitterSizes", QByteArray()).toByteArray();
        s_HSplitterSizes     = settings.value("HSplitterSizes",    QByteArray()).toByteArray();
        s_VSplitterSizes     = settings.value("VSplitterSizes",    QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void OptimFoilDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("OptimFoil");
    {
        for(int iopt=0; iopt<NOPT; iopt++)
        {
            settings.setValue(QString::asprintf("OptName_%d", iopt),   s_Opt[iopt].m_Name);
            settings.setValue(QString::asprintf("Alpha_%d", iopt),     s_Opt[iopt].m_Alpha);
            settings.setValue(QString::asprintf("Reynolds_%d", iopt),  s_Opt[iopt].m_Re);            
            settings.setValue(QString::asprintf("Mach_%d", iopt),      s_Opt[iopt].m_Mach);
            settings.setValue(QString::asprintf("NCrit_%d", iopt),     s_Opt[iopt].m_NCrit);
            settings.setValue(QString::asprintf("XtrTop_%d", iopt),    s_Opt[iopt].m_XtrTop);
            settings.setValue(QString::asprintf("XtrBot_%d", iopt),    s_Opt[iopt].m_XtrBot);
        }

        settings.setValue("NOpt",              s_NOpt);

        for(int iopt=0; iopt<NOPT; iopt++)
        {
            for(int k=0; k<NOBJECTIVES; k++)
            {
                settings.setValue(QString::asprintf("ObjType_%d_%d", iopt, k), s_Objective[iopt][k].m_Type-1);
                settings.setValue(QString::asprintf("Active_%d_%d",  iopt, k), s_Objective[iopt][k].m_bActive);
                settings.setValue(QString::asprintf("Target_%d_%d",  iopt, k), s_Objective[iopt][k].m_Target);
                settings.setValue(QString::asprintf("Error_%d_%d",   iopt, k), s_Objective[iopt][k].m_MaxError);
            }
        }

        settings.setValue("FlapAngleMin",      s_FlapAngleMin);
        settings.setValue("FlapAngleMax",      s_FlapAngleMax);
        settings.setValue("XHinge",            s_XHinge);
        settings.setValue("YHinge",            s_YHinge);

        settings.setValue("ModType", s_ModType);

        settings.setValue("HHn",               s_HHn);
        settings.setValue("HHt1",              s_HHt1);
        settings.setValue("HHt2",              s_HHt2);
        settings.setValue("HHmax",             s_HHmax);

        settings.setValue("ThickMin",          s_Thick[0]);
        settings.setValue("ThickMax",          s_Thick[1]);
        settings.setValue("XThickMin",         s_XThick[0]);
        settings.setValue("XThickMax",         s_XThick[1]);

        settings.setValue("CambMin",           s_Camb[0]);
        settings.setValue("CambMax",           s_Camb[1]);
        settings.setValue("XCambMin",          s_XCamb[0]);
        settings.setValue("XCambMax",          s_XCamb[1]);

        settings.setValue("PopSize",           PSOTask::s_PopSize);
        settings.setValue("bMultithread",      PSOTask::s_bMultiThreaded);
        settings.setValue("MaxIter",           PSOTask::s_MaxIter);

        settings.setValue("InertiaWeight",     PSOTask::s_InertiaWeight);
        settings.setValue("CognitiveWeight",   PSOTask::s_CognitiveWeight);
        settings.setValue("SocialWeight",      PSOTask::s_SocialWeight);
        settings.setValue("ProbRegen",         PSOTask::s_ProbRegenerate);

        settings.setValue("WindowGeom",        s_Geometry);
        settings.setValue("LeftSplitterSizes", s_LeftSplitterSizes);
        settings.setValue("HSplitterSizes",    s_HSplitterSizes);
        settings.setValue("VSplitterSizes",    s_VSplitterSizes);
    }
    settings.endGroup();
}


void OptimFoilDlg::enableControls(bool bEnable)
{
    m_ppbMakeSwarm->setEnabled(bEnable);
    m_ppbStoreBestFoil->setEnabled(bEnable);
    m_ppbContinueBest->setEnabled(bEnable);
}


Foil *OptimFoilDlg::onStoreBestFoil()
{
    Foil *pNewFoil = new Foil(*m_pBestFoil);

//    pNewFoil->initGeometry();
    pNewFoil->setFlaps();

    QString basename = "Optimized";

    int iter = 1;
    QString name = basename + QString::asprintf("_%d", iter);
    while(Objects2d::foilExists(name.toStdString()))
    {
        name = basename + QString::asprintf("_%d", iter);
        iter++;
    }
    pNewFoil->setName(name.toStdString());

    QStringList NameList;
    for(int k=0; k<Objects2d::nFoils(); k++)
    {
        Foil const*pOldFoil = Objects2d::foilAt(k);
        NameList.append(QString::fromStdString(pOldFoil->name()));
    }

    RenameDlg renDlg(this);
    renDlg.initDialog(pNewFoil->name(), Objects2d::foilNames(), "Enter the foil's new name");
    if(renDlg.exec() == QDialog::Rejected)
    {
        delete pNewFoil;
        return nullptr;
    }

    pNewFoil->setName(renDlg.newName().toStdString());
    Objects2d::insertThisFoil(pNewFoil);


    QColor back = this->palette().window().color();
    pNewFoil->setLineColor(xfl::randomfl5Color(back.valueF()<0.5f));
    outputText("Saved the current best foil with name: " + QString::fromStdString(pNewFoil->name())+EOLch);
    m_bSaved = true;

    m_bModified = true;

    return pNewFoil;
}


void OptimFoilDlg::onContinueBest()
{
    Foil *pNewBest = onStoreBestFoil();

    if(!pNewBest) return;

    setFoil(pNewBest); // continue from here
    m_bIsSwarmValid = false;
    outputText("Continuing with foil: "+QString::fromStdString(pNewBest->name())+"\n");
    outputText("Invalidating current swarm\n");
    update();
}


void OptimFoilDlg::readData()
{
    readOptPoints();

    s_HHn          = m_pieNHH->value()*2;
    s_HHt1         = m_pdeHHt1->value();
    s_HHt2         = m_pdeHHt2->value();
    s_HHmax        = m_pdeHHmax->value()/100.0;

    s_Thick[0]     = m_pfeThick[0]->value()/100.0;
    s_Thick[1]     = m_pfeThick[1]->value()/100.0;
    s_XThick[0]    = m_pfeXThick[0]->value()/100.0;
    s_XThick[1]    = m_pfeXThick[1]->value()/100.0;
    s_Camb[0]      = m_pfeCamb[0]->value()/100.0;
    s_Camb[1]      = m_pfeCamb[1]->value()/100.0;
    s_XCamb[0]     = m_pfeXCamb[0]->value()/100.0;
    s_XCamb[1]     = m_pfeXCamb[1]->value()/100.0;

    s_FlapAngleMin = m_pdeFlapAngleMin->value();
    s_FlapAngleMax = m_pdeFlapAngleMax->value();
    s_XHinge       = m_pdeTEXHinge->value()/100.0;
    s_YHinge       = m_pdeTEYHinge->value()/100.0;


    // temporary duplication
    PSOTask::s_MaxIter         = m_pieMaxIter->value();
    PSOTask::s_PopSize         = m_piePopSize->value();
    PSOTask::s_bMultiThreaded  = m_pchMultithread->isChecked();

    PSOTask::s_InertiaWeight   = m_pdeInertiaWeight->value();
    PSOTask::s_CognitiveWeight = m_pdeCognitiveWeight->value();
    PSOTask::s_SocialWeight    = m_pdeSocialWeight->value();
    PSOTask::s_ProbRegenerate  = m_pdeProbaRegen->value()/100.0;
}


void OptimFoilDlg::onMakeSwarm(bool bShow)
{
    if(m_pPSOTaskFoil && !m_pPSOTaskFoil->isFinished())
    {
        outputText("\n***Cannot make a swarm when a task is running***\n\n");
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    outputText("\nMaking swarm...\n");
    m_ppto->update();

    m_pFoilWt->clearFoils();
    m_pFoilWt->addFoil(m_pFoil);
    m_pFoilWt->addFoil(m_pBestFoil);
    for(int i=0; i<m_TempFoils.size(); i++) delete m_TempFoils[i];
    m_TempFoils.clear();
    m_pFoilWt->update();

    readData();


    /*    bool bConverged(false);
    double Cl(0), Cd(0), ClCd(0),  Cpmin(0), Cm(0);
    QVector<double>Cpv;
    runXFoil(m_pFoil, s_Alpha, Cl, Cd,  ClCd, Cpmin, Cm, Cpv, bConverged); // required to determine the leading edge index for the specified AOA

    QString strange;
    strange = QString::asprintf("   Cl     = %7.3g\n"
                                "   Cd     = %7.3g\n"
                                "   Cl/Cd  = %7.3g\n"
                                "   Cp_min = %7.3g\n"
                                "   Cm     = %7.3g\n\n",
                                Cl, Cd, ClCd, Cpmin, Cm);
    outputText(strange);
    m_ppto->update();*/

    makePSOSwarm();


    if(bShow)
    {
        QColor clr = Qt::cyan;

        for (int isw=0; isw<m_pPSOTaskFoil->swarmSize(); isw++)
        {
            Foil *pFoil = new Foil;
            pFoil->setName(QString::asprintf("Particle_%d", isw).toStdString());
            pFoil->setLineColor(xfl::tofl5Clr(clr));
            clr = clr.darker(113);

            Particle const &particle = m_pPSOTaskFoil->particle(isw);
            m_pPSOTaskFoil->makeFoil(particle, pFoil);

            m_pFoilWt->addFoil(pFoil);
            m_TempFoils.append(pFoil);
        }

    }

    m_bIsSwarmValid = true;

    update();

    QApplication::restoreOverrideCursor();
}


void OptimFoilDlg::makePSOSwarm()
{
    //initialize task
    if(m_pPSOTaskFoil) delete m_pPSOTaskFoil;
    m_pPSOTaskFoil =  new PSOTaskFoil;

    m_pPSOTaskFoil->setParent(this);

    updateTaskParameters();

    // adjust only the hinge location, angle is a variable
    m_pFoil->setTEFlapData(true, s_XHinge, s_YHinge, 0.0);

    m_pPSOTaskFoil->setFoil(m_pFoil, m_iLE);
    m_pPSOTaskFoil->setPolars(m_pPolar);
    m_pPSOTaskFoil->setModType(s_ModType);
    m_pPSOTaskFoil->setHHParams(s_HHt1, s_HHt2, s_HHmax);

    //make the swarm
    m_pPSOTaskFoil->onMakeParticleSwarm();
    m_pPSOTaskFoil->clearPareto();   // current Pareto may be obsolete if target values have changed since swarm creation
    m_pPSOTaskFoil->makeParetoFrontier();

    m_pPSOTaskFoil->setAnalysisStatus(xfl::FINISHED);
    //using SIGNAL/SLOT communication instead of Events to ensure that messages arrive in order
    connect(m_pPSOTaskFoil, SIGNAL(iterEvent(OptimEvent*)), this, SLOT(onIterEvent(OptimEvent*)), Qt::BlockingQueuedConnection);
}


void OptimFoilDlg::setPSOObjectives(PSOTaskFoil *pPSOTask)
{
    int nObj=0;
    for(int i=0; i<NOBJECTIVES;  i++)
    {
        nObj += s_Objective[0][i].m_bActive;
    }
    nObj *= s_NOpt;

    pPSOTask->setNOpt(s_NOpt);
    pPSOTask->setNObjectives(nObj);

    nObj=0;
    for(int iopt=0; iopt<s_NOpt; iopt++)
    {
        for(int i=0; i<NOBJECTIVES;  i++)
        {
            if(s_Objective[iopt][i].m_bActive)
            {
                pPSOTask->setObjective(nObj, s_Objective[iopt][i]);
                nObj++;
            }
        }
    }
    Q_ASSERT(nObj==pPSOTask->nObjectives());
}


void OptimFoilDlg::updateTaskParameters()
{
    updatePolars();

    if(!m_pPSOTaskFoil) return;

    m_pPSOTaskFoil->setModType(s_ModType);
    m_pPSOTaskFoil->setHHParams(s_HHt1, s_HHt2, s_HHmax);

    updateVariables(m_pPSOTaskFoil);

    setPSOObjectives(m_pPSOTaskFoil);

    m_pPSOTaskFoil->updateFitnesses();
    m_pPSOTaskFoil->updateErrors();
    m_pPSOTaskFoil->clearPareto();   // current Pareto may be obsolete if target values have changed since swarm creation
    m_pPSOTaskFoil->makeParetoFrontier();
}


void OptimFoilDlg::outputText(QString const &msg)
{
    m_ppto->onAppendQText(msg);
}


void OptimFoilDlg::cancelTask()
{
    if(m_pPSOTaskFoil && m_pPSOTaskFoil->isRunning())
    {
        outputText("\nSending interruption request...\n\n");
        m_ppto->update();
        m_pPSOTaskFoil->cancelAnalyis();
        m_ppbSwarm->setText("Swarm");
        QApplication::restoreOverrideCursor(); // you never know
    }
}


int OptimFoilDlg::nActiveObjectives() const
{
    int nObj=0;
    for(int i=0; i<NOBJECTIVES;  i++)
        nObj+=s_Objective[0][i].m_bActive;
    return nObj;
}


void OptimFoilDlg::onRunOptimizer()
{
    if(m_pPSOTaskFoil && m_pPSOTaskFoil->isRunning())
    {
        cancelTask();
        return;
    }

    readData();

    int nObj=nActiveObjectives()*s_NOpt;

    if(nObj==0)
    {
        outputText("\nAt least one objective needs be active - aborting\n\n");
        enableControls(true);
        return;
    }

    onRunXFoil();

    enableControls(false);

    m_bSaved = false;
    QApplication::setOverrideCursor(Qt::BusyCursor);

    m_pFoilWt->clearFoils();
    m_pFoilWt->addFoil(m_pFoil);
    m_pFoilWt->addFoil(m_pBestFoil);

    for(int ig=0; ig<NOBJECTIVES; ig++)
    {
        OptObjective const &obj = s_Objective[0][ig];
        if(obj.m_Type==xfl::EQUALIZE && obj.m_MaxError<PRECISION)
        {
            std::string strange = "Warning: " + obj.m_Name+" error should be >0 for EQUALIZE objective\n";
            outputStdText(strange);
        }
    }


    bool bIsSwarmValid = m_bIsSwarmValid;
    if(!m_pPSOTaskFoil) bIsSwarmValid = false;
    else
    {
        if(PSOTask::s_PopSize != m_pPSOTaskFoil->swarmSize())
        {
            outputText("Swarm size has changed: invalidating current swarm\n");
            bIsSwarmValid = false;
        }

        if(nObj!=m_pPSOTaskFoil->nObjectives())
        {
            outputText("Objectives have changed: invalidating current swarm\n");
            bIsSwarmValid = false;
        }

        int dimension = 0;
        switch (s_ModType)
        {
            case PSOTaskFoil::HH:      dimension = s_HHn+s_NOpt;   break;
            case PSOTaskFoil::SCALE:   dimension = 4+s_NOpt;       break;
        }

        if(dimension!=m_pPSOTaskFoil->nVariables())
        {
            outputText("Variables have changed: invalidating current swarm\n");
            bIsSwarmValid = false;
        }
    }

    if(!bIsSwarmValid)
    {
        onMakeSwarm(false);
    }
    else updateTaskParameters();

    m_pPSOTaskFoil->setAnalysisStatus(xfl::RUNNING);
    m_pPSOTaskFoil->restartIterations();
    m_pPSOTaskFoil->clearPareto();  // current Pareto may be obsolete if target values have changed since swarm creation
    m_pPSOTaskFoil->makeParetoFrontier();

    QThread *pThread = new QThread;
    m_pPSOTaskFoil->moveToThread(pThread); // don't touch it until the PSO end task event is received

    outputText("Launching optimization task asynchronously\n");
    connect(pThread, SIGNAL(started()),  m_pPSOTaskFoil, SLOT(onStartIterations()));
    connect(pThread, SIGNAL(finished()), pThread,    SLOT(deleteLater())); // deletes the thread but not the object
    //    connect(pThread, SIGNAL(finished()), this,       SLOT(onThreadFinished()), Qt::DirectConnection); // deletes the thread but not the object


    pThread->start();
    //    pThread->setPriority(QThread::NormalPriority);

    m_ppbSwarm->setText("Interrupt task");
    update();
}


void OptimFoilDlg::updateVariables(PSOTaskFoil *pPSOTask2d)
{
    int dimension = 0;

    switch (s_ModType)
    {
        case PSOTaskFoil::HH:
        {
            dimension = s_HHn+s_NOpt;
            pPSOTask2d->setDimension(dimension);  // two sides
            for(int i=0; i<s_HHn; i++)
                pPSOTask2d->setVariable(i, {QString::asprintf("HH%d", i).toStdString(), -s_HHmax, s_HHmax});

            break;
        }
        case PSOTaskFoil::SCALE:
        {
            dimension = 4+s_NOpt;
            pPSOTask2d->setDimension(dimension);
            pPSOTask2d->setVariable(0, {"Thickness",  s_Thick[0],  s_Thick[1]});
            pPSOTask2d->setVariable(1, {"Camber",     s_Camb[0],   s_Camb[1]});
            pPSOTask2d->setVariable(2, {"XThickness", s_XThick[0], s_XThick[1]});
            pPSOTask2d->setVariable(3, {"XCamber",    s_XCamb[0],  s_XCamb[1]});

            break;
        }
    }

    for(int iopt=0; iopt<s_NOpt; iopt++)
        pPSOTask2d->setVariable(dimension-s_NOpt+iopt, {QString::asprintf("FlapAngle_%d", iopt).toStdString(), s_FlapAngleMin, s_FlapAngleMax});
}

void OptimFoilDlg::setFoil(Foil *pFoil)
{
    m_pFoil = pFoil;
    m_pFoil->setTEFlapAngle(0.0);
    m_pFoil->setVisible(true);

    if(s_ModType==PSOTaskFoil::SCALE)
    {
        QString strange(QString::fromStdString(m_pFoil->name())+EOLch);
        strange += QString::asprintf("   Thickness =%5.2f%% at x=%5.2f%%\n",   m_pFoil->maxThickness()*100.0, m_pFoil->xThickness()*100.0);
        strange += QString::asprintf("   Camber    =%5.2f%% at x=%5.2f%%\n\n", m_pFoil->maxCamber()*100.0,    m_pFoil->xCamber()*100.0);
        m_plabFoilInfo->setText(strange);
    }

    m_pBestFoil->copy(m_pFoil, false);
    m_pBestFoil->setLineColor(xfl::LightCoral);
    m_pBestFoil->setLineStipple(Line::DASH);
    m_pBestFoil->setPointStyle(Line::NOSYMBOL);
    m_pBestFoil->setName(std::string("Best foil"));

    m_pFoilWt->clearFoils();
    m_pFoilWt->addFoil(m_pFoil);
    m_pFoilWt->addFoil(m_pBestFoil);
}


void OptimFoilDlg::onRunXFoil()
{
    readData();
    updatePolars();
    outputText("\nRunning XFoil on active optimization points.\n"
               "T.E. flaps are set to neutral position.\n"
               "For information only.\n");

    QVector<double>Cpv;

    for(int iopt=0; iopt<s_NOpt; iopt++)
    {
        bool bConverged(false);
        double Cl(0),  Cd(0),  ClCd(0),  Cpmin(0),  Cm(0);
        runXFoil(m_pFoil, m_pPolar[iopt], s_Opt[iopt].m_Alpha, Cl, Cd, ClCd, Cpmin, Cm, Cpv, bConverged); // required to determine the leading edge index for the specified AOA
        if(!bConverged) outputText("Seed foil is unconverged\n");

        Curve *pCurve = m_CpGraph[iopt].curve(0);
        if(!pCurve) pCurve = m_CpGraph[iopt].addCurve("Cp_seed");
        pCurve->setTheStyle(m_pFoil->theStyle());
        pCurve->clear();
        for(int i=0; i<Cpv.size(); i++)
            pCurve->appendPoint(m_pFoil->x(i), Cpv.at(i));


/*
        double Clb(0), Cdb(0), ClCdb(0), Cpminb(0), Cmb(0);
        runXFoil(m_pBestFoil, m_pPolar[iopt], s_Opt[iopt].m_Alpha, Clb, Cdb, ClCdb, Cpminb, Cmb, Cpv, bConverged); // required to determine the leading edge index for the specified AOA
        if(!bConverged) outputText("Best foil is unconverged\n");

        pCurve = m_CpGraph[iopt].curve(1);
        if(!pCurve) pCurve = m_CpGraph[iopt].addCurve("Cp_best");
        pCurve->setTheStyle(m_pBestFoil->theStyle());
        pCurve->clear();
        for(int i=0; i<Cpv.size(); i++)
            pCurve->appendPoint(m_pBestFoil->x(i), Cpv.at(i));*/


        QString strange = QString::asprintf("Optimization point %d:\n", iopt+1);
 /*       strange = QString::asprintf("              Seed foil  Current best\n"
                                    "      Cl     =  %7.3g       %7.3g\n"
                                    "      Cd     =  %7.3g       %7.3g\n"
                                    "      Cl/Cd  =  %7.3g       %7.3g\n"
                                    "      Cp_min =  %7.3g       %7.3g\n"
                                    "      Cm     =  %7.3g       %7.3g",
                                    Cl, Clb, Cd, Cdb, ClCd, ClCdb, Cpmin, Cpminb, Cm, Cmb);*/

        strange += QString::asprintf("              Seed foil\n"
                                     "      Cl     =  %7.3g\n"
                                     "      Cd     =  %7.3g\n"
                                     "      Cl/Cd  =  %7.3g\n"
                                     "      Cp_min =  %7.3g\n"
                                     "      Cm     =  %7.3g",
                                     Cl, Cd, ClCd, Cpmin, Cm);

//        outputText(strange+"\n\n");
        m_pCpGraphWt[iopt]->setOutputInfo(strange);

        m_CpGraph[iopt].invalidate();
        m_pCpGraphWt[iopt]->update();
    }
}


void OptimFoilDlg::runXFoil(Foil const*pFoil, Polar *pPolar, double alpha,
                         double &Cl, double &Cd, double &ClCd, double &Cpmin, double &Cm, QVector<double>&Cpv,
                         bool &bConverged)
{
    Foil tempfoil(*pFoil);

    XFoilTask *pTask = new XFoilTask(); // watch the stack
//   pTask->setEventDestination(this);
    XFoil const &xfoil = pTask->XFoilInstance();

    pTask->initialize(&tempfoil, pPolar, false);
    pTask->setAoAAnalysis(true);
    pTask->setAlphaRange(alpha, alpha, 1.0);
    pTask->run();

    m_iLE = -1;
    for(int i=0; i<xfoil.n-1; i++)
    {
        if(xfoil.x[i+1]<xfoil.x[i+2])
        {
            m_iLE = i;
            break;
        }
    }

    Cl    = xfoil.cl;
    Cd    = xfoil.cd;
    ClCd  = xfoil.cl/xfoil.cd;
    Cpmin = xfoil.cpmn;
    Cm    = xfoil.cm;
    bConverged = xfoil.lvconv;

    pTask->initialize(&tempfoil, pPolar, false);
    pTask->run();
    bConverged = bConverged && xfoil.lvconv;

    Cpv.resize(xfoil.n);
    if(xfoil.lvisc && xfoil.lvconv)
    {
        for (int k=0; k<xfoil.n; k++)
        {
            Cpv[k] = xfoil.cpv[k+1];
        }
    }

    update();
//    delete pTask;  // setAutoDelete(true)
}


void OptimFoilDlg::updateCpGraphs(Particle const &particle)
{
    if(!m_pPSOTaskFoil) return;

    QString strange;

    int dim = particle.dimension();
    int geomdim = dim-s_NOpt; //-flap angle x NOpt

    for(int iopt=0; iopt<s_NOpt; iopt++)
    {
        OptimizationPoint const &optpoint = s_Opt[iopt];
        double flapangle = particle.pos(geomdim+iopt);

        if(fabs(flapangle)>FLAPANGLEPRECISION)
        {
            m_pBestFoil->setTEFlapAngle(flapangle);
            m_pBestFoil->setFlaps();
        }
        else
        {
            m_pBestFoil->setTEFlapAngle(0.0);
            m_pBestFoil->applyBase();
        }

        bool bConverged(false);
        double Cl(0), Cd(0), ClCd(0), Cpmin(0), Cm(0);
        QVector<double>Cpv;
        runXFoil(m_pBestFoil, m_pPolar[iopt], m_pPolar[iopt]->aoaSpec(), Cl, Cd, ClCd, Cpmin, Cm, Cpv, bConverged);
        Curve *pCurve = m_CpGraph[iopt].curve(1);
        if(!pCurve) pCurve = m_CpGraph[iopt].addCurve("Cp_best");
        pCurve->setTheStyle(m_pBestFoil->theStyle());
        pCurve->clear();
        for(int i=0; i<Cpv.size(); i++)
            pCurve->appendPoint(m_pBestFoil->x(i), Cpv.at(i));


        strange = optpoint.m_Name + "\n";
        strange += ALPHAch + QString::asprintf("  = %7.2g",   m_pPolar[iopt]->aoaSpec()) + DEGch+"\n";
        strange += QString::asprintf("Re = %7g\n\n", m_pPolar[iopt]->Reynolds());


        strange += "              target      particle\n";
        strange += QString::asprintf("Flap angle =               %7.3g", flapangle)  + DEGch+"\n";

        for(int iobj=0; iobj<NOBJECTIVES; iobj++)
        {
//                OptObjective const &obj = m_pPSOTaskFoil->objective(iobj);
            OptObjective const &obj = s_Objective[iopt][iobj];
            if(!obj.m_bActive) continue;

            std::string str = obj.m_Name;
            str.resize(11, ' ');
            strange += QString::fromStdString(str);
            if      (obj.m_Name=="Cl")      strange += QString::asprintf("= %7.3g       %7.3g", obj.m_Target, Cl);
            else if (obj.m_Name=="Cd")      strange += QString::asprintf("= %7.3g       %7.3g", obj.m_Target, Cd);
            else if (obj.m_Name=="Cl/Cd")   strange += QString::asprintf("= %7.3g       %7.3g", obj.m_Target, ClCd);
            else if (obj.m_Name=="Cp_min")  strange += QString::asprintf("= %7.3g       %7.3g", obj.m_Target, Cpmin);
            else if (obj.m_Name=="Cm")      strange += QString::asprintf("= %7.3g       %7.3g", obj.m_Target, Cm);

            strange += " \n";
        }

        m_pCpGraphWt[iopt]->setOutputInfo(strange);
    }

    // leave things as they were + display without flap angle
    m_pBestFoil->setTEFlapAngle(0.0);
    m_pBestFoil->applyBase();
}


void OptimFoilDlg::onIterEvent(OptimEvent *result)
{
    Particle const &particle = result->particle();

/*    QString strange;
    qDebug()<<"IterEvent";
    listParticle(particle, strange);
    qDebug("%s", strange.toStdString().c_str());
    qDebug()<<"____________________";*/

    if(m_pPSOTaskFoil)
    {
        m_pPSOTaskFoil->makeFoil(particle, m_pBestFoil);

        QString strange = QString::asprintf("Iteration %2d", result->iter());
        outputText(strange+"\n");

        updateCpGraphs(particle);
    }

    update();
}


void OptimFoilDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type()==OPTIM_END_EVENT)
    {
        OptimEvent *pOptEvent = dynamic_cast<OptimEvent*>(pEvent);
        Particle const &particle = pOptEvent->particle();

        QString strange;

        if (m_pPSOTaskFoil)
        {
            m_pPSOTaskFoil->makeFoil(particle, m_pBestFoil);

//            int nVar = m_pPSOTaskFoil->nActiveVariables();
            int nActiveObj = nActiveObjectives();

            for(int iopt=0; iopt<s_NOpt; iopt++)
            {
                OptimizationPoint const &optpoint = s_Opt[iopt];

                double flapangle = particle.pos(particle.dimension()-s_NOpt+iopt);

                strange += optpoint.m_Name + "\n";
                strange += "   " + ALPHAch + QString::asprintf("          = %7.2g", m_pPolar[iopt]->aoaSpec()) + DEGch+"\n";
                strange += QString::asprintf("   Re         = %7g\n", m_pPolar[iopt]->Reynolds());
                strange += QString::asprintf("   Flap angle = %7.3g", flapangle)  + DEGch+"\n";

                strange += "              target   particle\n";

                int iobj=0;
                for(int i=0; i<NOBJECTIVES; i++)
                {
                    OptObjective const &obj = s_Objective[iopt][i];
                    if(!obj.m_bActive) continue;

                    QString str;
                    str += "   "+ QString::fromStdString(obj.m_Name);
                    str.resize(11, ' ');
                    str += "= ";

                    str += QString::asprintf("%7.3g    %7.3g\n", s_Objective[iopt][i].m_Target, particle.fitness(iopt*nActiveObj + iobj));
                    strange += str;

                    iobj++;
                }
                strange += "\n";
            }

            m_pBestFoil->setDescription(strange.toStdString());


            if(!m_pPSOTaskFoil->isConverged()) outputText("---Unconverged---\n");
            else
            {
                QString strong = QString::asprintf("The winner is particle %d\n", pOptEvent->iBest());
                listParticle(particle, strong, "  ");
                m_ppto->onAppendQText(strong+"\n");
            }

            m_ppto->onAppendQText(strange);
        }

        enableControls(true);
        m_ppbSwarm->setText("Swarm");
        QApplication::restoreOverrideCursor();
    }
    else if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        outputText(pMsgEvent->msg());
    }
    else
        XflDialog::customEvent(pEvent);
}


void OptimFoilDlg::listParticle(Particle const &particle, QString &log, QString prefix)
{
    QString strange;
    int dim = 0;
    switch (s_ModType)
    {
        case PSOTaskFoil::HH:
        {
            for(int i=0; i<s_HHn; i++)
            {
                strange += prefix + QString::asprintf("Bump %d: amp.=%7.3f%%\n", i, particle.pos(i)*100.0);
            }

            dim = s_HHn;

            break;
        }
        case PSOTaskFoil::SCALE:
        {
            strange += prefix + QString::asprintf("Max. thickness = %7.3f%% at x=%7.3f%%\n", particle.pos(0)*100.0, particle.pos(2)*100.0);
            strange += prefix + QString::asprintf("Max. camber    = %7.3f%% at x=%7.3f%%\n", particle.pos(1)*100.0, particle.pos(3)*100.0);

            dim = 4;
            break;
        }
    }

    for(int iopt=0; iopt<s_NOpt; iopt++)
        strange += prefix + QString::asprintf("Flap angle     = %7.3f", particle.pos(dim+iopt))+ DEGch + "\n";

    log += strange;
}


void OptimFoilDlg::readObjectives()
{
    bool bOk(false);
    QString strange;
    QModelIndex ind;

    for(int row=0; row<m_pObjModel->rowCount(); row++)
    {
        for(int iopt=0; iopt<NOPT; iopt++)
        {
            OptObjective &opt = s_Objective[iopt][row];
            ind = m_pObjModel->index(row, 0);
            opt.m_bActive = m_pObjModel->data(ind, Qt::UserRole).toBool();

            ind = m_pObjModel->index(row, 1);
            opt.m_Name = m_pObjModel->data(ind).toString().toStdString();

            ind = m_pObjModel->index(row, 2);
            int val = m_pObjModel->data(ind).toInt();
            if     (val<0) opt.m_Type = xfl::MINIMIZE;
            else if(val>0) opt.m_Type = xfl::MAXIMIZE;
            else           opt.m_Type = xfl::EQUALIZE;

            int col = iopt+3;
            ind = m_pObjModel->index(row, col);
            strange = m_pObjModel->data(ind).toString();
            strange.replace(" ","");
            opt.m_Target = strange.toDouble(&bOk);

            ind = m_pObjModel->index(row, 7);
            strange = m_pObjModel->data(ind).toString();
            strange.replace(" ","");
            opt.m_MaxError = strange.toDouble(&bOk);
        }
    }
}


void OptimFoilDlg::updatePolars()
{
    // update the polar with the latest user data
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        m_pPolar[iopt]->setType(    xfl::T1POLAR);
        m_pPolar[iopt]->setAoaSpec(     s_Opt[iopt].m_Alpha); // repurposing T4 aoa
        m_pPolar[iopt]->setReynolds(s_Opt[iopt].m_Re);
        m_pPolar[iopt]->setMach(    s_Opt[iopt].m_Mach);
        m_pPolar[iopt]->setNCrit(   s_Opt[iopt].m_NCrit);
        m_pPolar[iopt]->setXTripTop(s_Opt[iopt].m_XtrTop);
        m_pPolar[iopt]->setXTripBot(s_Opt[iopt].m_XtrBot);
    }
}


void OptimFoilDlg::onPlotHH()
{
    readData();

    m_HHGraph.deleteCurves();

    int npts = 300;
    double hh = 0.0, x=0.0;
    for(int j=0; j<s_HHn/2; j++)
    {
        Curve *pCurve = m_HHGraph.addCurve(QString::asprintf("f%d", j+1));
        pCurve->setWidth(2);
        pCurve->resize(npts);
        double t1 = s_HHt1*double(j+1)/double(s_HHn+1); // HH undefined for t1=0
        for(int k=0; k<npts; k++)
        {
            x = double(k)/double(npts-1);
            hh = HicksHenne(x, t1, s_HHt2, 0.0, 1.0)*s_HHmax*100.0;
            pCurve->setPoint(k, x, hh);
        }
    }

    m_HHGraph.resetLimits();
    m_pHHGraphWt->update();
}


void OptimFoilDlg::onVarType()
{
    if(m_prbHH->isChecked())
    {
        s_ModType = PSOTaskFoil::HH;
        m_pswVarWidget->setCurrentIndex(0);
    }
    else
    {
        s_ModType = PSOTaskFoil::SCALE;
        m_pswVarWidget->setCurrentIndex(1);
    }

    outputText("\nInvalidating current swarm\n");
    m_bIsSwarmValid = false;
}


void OptimFoilDlg::onFillOptPoint()
{
//    int iopt = activeOptPoint();

    fillObjectives();

    m_pcptObjective->update();
}


void OptimFoilDlg::fillObjectives()
{
    m_pObjModel->setRowCount(NOBJECTIVES);

    QModelIndex ind;
    m_pObjModel->blockSignals(true); // do not emit the dataChanged signal

    for(int row=0; row<NOBJECTIVES; row++)
    {
        ind = m_pObjModel->index(row, 0, QModelIndex());
        m_pObjModel->itemFromIndex(ind)->setToolTip("Activate or deactivate the objective");
        m_pObjModel->setData(ind, s_Objective[0][row].m_bActive, Qt::UserRole);

        ind = m_pObjModel->index(row, 1, QModelIndex());
        m_pObjModel->itemFromIndex(ind)->setToolTip("The name of the objective");
        m_pObjModel->setData(ind, QString::fromStdString(s_Objective[0][row].m_Name));

        ind = m_pObjModel->index(row, 2, QModelIndex());
        m_pObjModel->itemFromIndex(ind)->setToolTip("-1 to MINIMIZE the objective\n"
                                                    " 0 to EQUALIZE\n"
                                                    " 1 to MAXIMIZE");
        switch(s_Objective[0][row].m_Type)
        {
            case xfl::MINIMIZE:  m_pObjModel->setData(ind, -1);  break;
            case xfl::MAXIMIZE:  m_pObjModel->setData(ind, 1);   break;
            case xfl::EQUALIZE:  m_pObjModel->setData(ind, 0);   break;
        }
        for(int iopt=0; iopt<NOPT; iopt++)
        {
            OptObjective const &obj = s_Objective[iopt][row];
            int col = iopt+3;
            ind = m_pObjModel->index(row, col, QModelIndex());
            m_pObjModel->itemFromIndex(ind)->setToolTip("The maximum acceptable value if the objective is to MINIMIZE\n"
                                                        "The target value if the objective is to EQUALIZE\n"
                                                        "The minimum acceptable value if the objective is to MAXIMIZE");
            m_pObjModel->setData(ind, obj.m_Target);
        }


        ind = m_pObjModel->index(row, 7, QModelIndex());
        m_pObjModel->itemFromIndex(ind)->setToolTip("The tolerance on the target value.\n"
                                                    "Only used if the objective is to EQUALIZE\n"
                                                    "Ignored otherwise.");
        m_pObjModel->setData(ind, s_Objective[0][row].m_MaxError);

/*        for(int col=1; col<8; col++)
        {
            if(s_Objective[0][row].m_bActive)
                m_pObjModel->item(row, col)->setFlags(Qt::ItemIsEnabled);
            else
                m_pObjModel->item(row, col)->setFlags(Qt::NoItemFlags);
        }*/
    }
    m_pObjModel->blockSignals(false);


}


void OptimFoilDlg::fillOptPoints()
{
    m_pOptPointsModel->setRowCount(6);

    QModelIndex ind;

    // alpha
    int row = 0;
    ind = m_pOptPointsModel->index(row, 0, QModelIndex());
    m_pOptPointsModel->setData(ind, ALPHAch);
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        int col = iopt+1;
        ind = m_pOptPointsModel->index(row, col, QModelIndex());
        m_pOptPointsModel->setData(ind, s_Opt[iopt].m_Alpha);
    }
    ind = m_pOptPointsModel->index(row, NOPT+1, QModelIndex());
    m_pOptPointsModel->setData(ind, DEGch);


    // Re
    row = 1;
    ind = m_pOptPointsModel->index(row, 0, QModelIndex());
    m_pOptPointsModel->setData(ind, "Reynolds");
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        int col = iopt+1;
        ind = m_pOptPointsModel->index(row, col, QModelIndex());
        m_pOptPointsModel->setData(ind, s_Opt[iopt].m_Re);
    }


    // Mach
    row = 2;
    ind = m_pOptPointsModel->index(row, 0, QModelIndex());
    m_pOptPointsModel->setData(ind, "Mach");
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        int col = iopt+1;
        ind = m_pOptPointsModel->index(row, col, QModelIndex());
        m_pOptPointsModel->setData(ind, s_Opt[iopt].m_Mach);
    }

    // NCrit
    row = 3;
    ind = m_pOptPointsModel->index(row, 0, QModelIndex());
    m_pOptPointsModel->setData(ind, "NCrit");
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        int col = iopt+1;
        ind = m_pOptPointsModel->index(row, col, QModelIndex());
        m_pOptPointsModel->setData(ind, s_Opt[iopt].m_NCrit);
    }

    // XtrTop
    row = 4;
    ind = m_pOptPointsModel->index(row, 0, QModelIndex());
    m_pOptPointsModel->setData(ind, "Top transition:");
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        int col = iopt+1;
        ind = m_pOptPointsModel->index(row, col, QModelIndex());
        m_pOptPointsModel->setData(ind, s_Opt[iopt].m_XtrTop);
    }

    // XtrBot
    row = 5;
    ind = m_pOptPointsModel->index(row, 0, QModelIndex());
    m_pOptPointsModel->setData(ind, "Bot. transition:");
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        int col = iopt+1;
        ind = m_pOptPointsModel->index(row, col, QModelIndex());
        m_pOptPointsModel->setData(ind, s_Opt[iopt].m_XtrBot);
    }
}


void OptimFoilDlg::readOptPoints()
{
    // alpha
    int row = 0;
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        int col = iopt+1;
        s_Opt[iopt].m_Alpha = m_pOptPointsModel->index(row, col, QModelIndex()).data().toDouble();
    }

    // Re
    row = 1;
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        int col = iopt+1;
        s_Opt[iopt].m_Re = m_pOptPointsModel->index(row, col, QModelIndex()).data().toDouble();
    }

    // Mach
    row = 2;
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        int col = iopt+1;
        s_Opt[iopt].m_Mach = m_pOptPointsModel->index(row, col, QModelIndex()).data().toDouble();
    }

    // NCrit
    row = 3;
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        int col = iopt+1;
        s_Opt[iopt].m_NCrit = m_pOptPointsModel->index(row, col, QModelIndex()).data().toDouble();
    }

    // NCrit
    row = 4;
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        int col = iopt+1;
        s_Opt[iopt].m_XtrTop = m_pOptPointsModel->index(row, col, QModelIndex()).data().toDouble();
    }

    // NCrit
    row = 5;
    for(int iopt=0; iopt<NOPT; iopt++)
    {
        int col = iopt+1;
        s_Opt[iopt].m_XtrTop = m_pOptPointsModel->index(row, col, QModelIndex()).data().toDouble();
    }
}







