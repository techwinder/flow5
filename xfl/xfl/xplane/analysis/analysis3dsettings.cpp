/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED


#include <QFontMetrics>
#include <QFont>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>


#include "analysis3dsettings.h"



#include <xfl/xplane/analysis/planeanalysisdlg.h>
#include <xflcore/displayoptions.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflgraph/containers/graphwt.h>
#include <xflgraph/controls/graphoptions.h>
#include <xflgraph/graph/graph.h>
#include <xflobjects/analysis3d/boattask.h>
#include <xflobjects/analysis3d/llttask.h>
#include <xflobjects/analysis3d/panelanalysis.h>
#include <xflobjects/analysis3d/planetask.h>
#include <xflobjects/analysis3d/task3d.h>
#include <xflobjects/objects3d/analysis/planeopp.h>
#include <xflobjects/objects3d/wing/wingxfl.h>
#include <xflpanels/panels/panel3.h>
#include <xflpanels/panels/panel4.h>
#include <xflpanels/panels/vortex.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>


bool Analysis3dSettings::s_bKeepOpenOnErrors = true;
int Analysis3dSettings::s_iPage = 0;
QByteArray Analysis3dSettings::s_WindowGeometry;


Analysis3dSettings::Analysis3dSettings(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("3d analysis advanced settings");
    setupLayout();
}


void Analysis3dSettings::onResetDefaults()
{
    Task3d::setMaxNRHS(100);

    PlaneTask::setViscInitVTwist(false);
    PlaneTask::setViscRelaxFactor(0.5);
    PlaneTask::setMaxViscError(.01);
    PlaneTask::setMaxViscIter(35);

    PanelAnalysis::setDoublePrecision(true);

    Vortex::setCoreRadius(0.000001);
    Vortex::setVortexModel(Vortex::POTENTIAL);

    WingXfl::setMinSurfaceLength(.001);
    Panel4::setVortexFracPos(0.25);
    Panel4::setCtrlPtFracPos(0.75);

    Panel3::setQuadratureOrder(5);
    Panel::setRFF(10);

    setData();
}


void Analysis3dSettings::setupLayout()
{
    m_pTabWt = new QTabWidget;
    {
        QFrame *pCommonFrame = new QFrame;
        {
            QGridLayout *pGeomLayout = new QGridLayout;
            {
                QLabel *plabnRHS  = new QLabel("Max. number of operating points/analysis=");
                m_pieMaxRHS = new IntEdit;
                m_pieMaxRHS->setToolTip("<p>Defines the maximum number of operating points to be calculated in a single run."
                                          "Intended as a safety limit to prevent excessively lengthy analyses."
                                          "</p>");

                m_pchKeepOpenOnErrors  = new QCheckBox("Keep analysis window opened on errors");

                QLabel *plabWingPanels  = new QLabel("Ignore wing panels with span width <");
                QLabel *plabLength1     = new QLabel(Units::lengthUnitLabel());
                m_pdeMinPanelSize = new FloatEdit;
                m_pdeMinPanelSize->setToolTip("<p>Wing surfaces with span width less than this value will be ignored "
                                              "in the mesh operation.<br>"
                                              "This is to allow the definition of duplicate sections at a given span positions.</p>");

                QLabel *plabRFF = new QLabel("Far field factor=");
                m_pdeRFF = new FloatEdit;
                m_pdeRFF->setToolTip("<p>RFF is the far-field radius factor used in the evaluation of influence coefficients. "
                                          "If the evaluation point is at a distance greater than  RFF x panel_size, then the "
                                          "panel is considered to be a point source or point doublet with strength = area x density.<br>"
                                          "This reduces significantly the computation times at the expense of precision. Practically, "
                                          "the difference is in the order of 0.5% at RFF=7.<br>"
                                          "RFF=10 is the recommended value.<br>"
                                          "Cf. Report NASA 4023 for a more detailed explanation.</p>");

                pGeomLayout->addWidget(m_pchKeepOpenOnErrors,2,1,1,3);
                pGeomLayout->addWidget(plabnRHS,             4,1, Qt::AlignRight);
                pGeomLayout->addWidget(m_pieMaxRHS,          4,2);
                pGeomLayout->addWidget(plabWingPanels,       5,1, Qt::AlignRight);
                pGeomLayout->addWidget(m_pdeMinPanelSize,    5,2);
                pGeomLayout->addWidget(plabLength1,          5,3);
                pGeomLayout->addWidget(plabRFF,              6,1, Qt::AlignRight);
                pGeomLayout->addWidget(m_pdeRFF,             6,2);
                pGeomLayout->setRowStretch(                  7,1);
                pGeomLayout->setColumnStretch(               4,1);
            }
            pCommonFrame->setLayout(pGeomLayout);
        }

        QFrame *pLLTFrame = new QFrame;
        {
            QGridLayout *pLLTLayout = new QGridLayout;
            {
                QLabel *pLabNStations = new QLabel("Number of spanwise stations=");
                QLabel *pLabRelax     = new QLabel("Relaxation factor=");
                QLabel *pLabAlphaCv   = new QLabel("<p>&alpha; precision=</p>");
                QLabel *pLabIter      = new QLabel("Maximum iterations=");
                QLabel *pLabDegree    = new QLabel("<p>&deg;</p>");

                m_pieLLTNStation     = new IntEdit;
                m_pfeLLTRelax        = new FloatEdit;
                m_pfeLLTAlphaPrec    = new FloatEdit;
                m_pieLLTIterMax      = new IntEdit;
                pLLTLayout->addWidget(pLabNStations,     1,1, Qt::AlignRight | Qt::AlignVCenter);
                pLLTLayout->addWidget(pLabRelax,         2,1, Qt::AlignRight | Qt::AlignVCenter);
                pLLTLayout->addWidget(pLabAlphaCv,       3,1, Qt::AlignRight | Qt::AlignVCenter);
                pLLTLayout->addWidget(pLabIter,          4,1, Qt::AlignRight | Qt::AlignVCenter);
                pLLTLayout->addWidget( m_pieLLTNStation, 1,2);
                pLLTLayout->addWidget(m_pfeLLTRelax,     2,2);
                pLLTLayout->addWidget(m_pfeLLTAlphaPrec, 3,2);
                pLLTLayout->addWidget(pLabDegree,        3,3, Qt::AlignLeft | Qt::AlignVCenter);
                pLLTLayout->addWidget(m_pieLLTIterMax,   4,2);
                pLLTLayout->setRowStretch(   5,1);
                pLLTLayout->setColumnStretch(5,4);
            }
            pLLTFrame->setLayout(pLLTLayout);
        }

        QFrame *pVLMFrame = new QFrame;
        {
            QGridLayout *pVLMLayout = new QGridLayout;
            {
                m_pdeVortexPos    = new FloatEdit(25.0, 2);
                m_pdeControlPos   = new FloatEdit(75.0, 2);

                QLabel *pLab6  = new QLabel("Vortex position=");
                QLabel *pLab7  = new QLabel("Control point position=");
                QLabel *pLab8  = new QLabel("%");
                QLabel *pLab9  = new QLabel("%");
                pLab6->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                pLab7->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                pLab8->setAlignment(Qt::AlignLeft  | Qt::AlignVCenter);
                pLab9->setAlignment(Qt::AlignLeft  | Qt::AlignVCenter);
                pVLMLayout->addWidget(pLab6,1,1);
                pVLMLayout->addWidget(pLab7,2,1);
                pVLMLayout->addWidget(m_pdeVortexPos,1,2);
                pVLMLayout->addWidget(m_pdeControlPos,2,2);
                pVLMLayout->addWidget(pLab8,1,3);
                pVLMLayout->addWidget(pLab9,2,3);
                pVLMLayout->setColumnStretch(3,5);
                pVLMLayout->setRowStretch(4,1);
            }
            pVLMFrame->setLayout(pVLMLayout);
        }

        QFrame *p3dPanelFrame = new QFrame;
        {
            QVBoxLayout *p3dPanelLayout = new QVBoxLayout;
            {
                QHBoxLayout *pQuadratureLayout = new QHBoxLayout;
                {
                    QLabel *pQuadLab = new QLabel("Order of 2d Gaussian quadrature for triangle scalar products=");
                    m_pieQuadPoints = new IntEdit(0);
                    pQuadratureLayout->addWidget(pQuadLab);
                    pQuadratureLayout->addWidget(m_pieQuadPoints);
                    pQuadratureLayout->addStretch();
                }
                QString tip = "Order 1:  1 Gaussian point<br>"
                              "Order 2:  3 Gaussian points<br>"
                              "Order 3:  4 Gaussian points<br>"
                              "Order 4:  6 Gaussian points<br>"
                              "Order 5:  7 Gaussian points<br>"
                              "Order 6: 12 Gaussian points<br>"
                              "Order 7: 13 Gaussian points<br>"
                              "Order 8: 16 Gaussian points<br>"
                              "<b>Recommendation</b>: 3 &le; order &le; 5";
                QLabel *pTipLab = new QLabel(tip);

                pTipLab->setFont(DisplayOptions::tableFont());

                p3dPanelLayout->addLayout(pQuadratureLayout);
                p3dPanelLayout->addWidget(pTipLab);
                p3dPanelLayout->addStretch();
            }
            p3dPanelFrame->setLayout(p3dPanelLayout);
        }

        QFrame *pSolverFrame = new QFrame;
        {
            QGridLayout *pPrecisionLayout = new QGridLayout;
            {
                QLabel *pLabPrecision = new QLabel("Matrix float precision:");
                m_prbSinglePrecision = new QRadioButton("Single precision, 4 bytes/value");
                m_prbDoublePrecision = new QRadioButton("Double precision, 8 bytes/value (recommended)");
                QString precisiontip = "This defines with what precision the influence matrix will be stored. "
                                       "Single precision will use half the memory required for double precision. "
                                       "Accurracy of the results may also be reduced by a few percent.<br>"
                                       "Use single precision only for large calculations where memory allocation "
                                       "may be an issue.";
                pLabPrecision->setToolTip(precisiontip);
                m_prbSinglePrecision->setToolTip(precisiontip);
                m_prbDoublePrecision->setToolTip(precisiontip);
                pPrecisionLayout->addWidget(pLabPrecision,1,1,1,2);
                pPrecisionLayout->addWidget(m_prbSinglePrecision,2,2);
                pPrecisionLayout->addWidget(m_prbDoublePrecision,3,2);
                pPrecisionLayout->setColumnStretch(3,2);
                pPrecisionLayout->setRowStretch(4,1);
            }

            pSolverFrame->setLayout(pPrecisionLayout);
        }

        QFrame *pViscLoopFrame = new QFrame;
        {
            QGridLayout *pViscIterLayout = new QGridLayout;
            {
                QLabel *pLabRelax   = new QLabel("Relax. factor=");
                QLabel *pLabAlphaCv = new QLabel("Virtual twist precision=");
                QLabel *pLabIter    = new QLabel("Max. iterations=");
                QLabel *pLabDegree    = new QLabel("<p>&deg;</p>");

                m_pfeViscPanelRelax = new FloatEdit(1.0);
                m_pfeViscPanelRelax->setRange(0.0,1.0);
                QString tip = "<p>The relaxation factor is a multiplier applied to the increase of virtual twist at each viscous iteration. "
                              "At low Reynolds numbers, convergence may be improved by reducing this coefficient to a value less than 1."
                              "The coefficient should always be greater than zero and less than 1.<br>"
                              "Recommendation: 0.3 < relax < 0.7"
                              "</p>";
                m_pfeViscPanelRelax->setToolTip(tip);

                m_pfeViscPanelTwistPrec    = new FloatEdit;
                tip = "<p>This parameter controls the maximum acceptable error for virtual twist. "
                      "Convergence is achieved if the max. increase of virtual twist from one "
                      "viscous iteration to the next is less than this value.<br>"
                      "Recommendation: 0.01&deg;</p>";
                m_pfeViscPanelTwistPrec->setToolTip(tip);

                m_pieViscPanelIterMax      = new IntEdit;
                tip = "<p>This parameter sets the maximum number of viscous iterations.<br>"
                      "Recommendation: max. iter = 30</p>";
                m_pieViscPanelIterMax->setToolTip(tip);

                m_pchViscInitVTwist = new QCheckBox("Initialize the virtual twist at each new operating point calculation");
                QString TipInit("<p>"
                                "If activated the virtual twist will be set to 0 when starting the calculation "
                                "of a new operating point. "
                                "Otherwise the iterations will start with the virtual twist from the previous "
                                "converged operating point.<br>"
                                "Recommendation: do not activate</p>");
                m_pchViscInitVTwist->setToolTip(TipInit);

                pViscIterLayout->addWidget(pLabRelax,                 1,1, Qt::AlignRight | Qt::AlignVCenter);
                pViscIterLayout->addWidget(m_pfeViscPanelRelax,       1,2);
                pViscIterLayout->addWidget(pLabAlphaCv,               2,1, Qt::AlignRight | Qt::AlignVCenter);
                pViscIterLayout->addWidget(m_pfeViscPanelTwistPrec,   2,2);
                pViscIterLayout->addWidget(pLabDegree,                2,3, Qt::AlignLeft | Qt::AlignVCenter);
                pViscIterLayout->addWidget(pLabIter,                  3,1, Qt::AlignRight | Qt::AlignVCenter);
                pViscIterLayout->addWidget(m_pieViscPanelIterMax,     3,2);
                pViscIterLayout->addWidget(m_pchViscInitVTwist,       4,1,1,3);
                pViscIterLayout->setRowStretch(5,1);
                pViscIterLayout->setColumnStretch(4,1);
            }
            pViscLoopFrame->setLayout(pViscIterLayout);
        }

        QFrame *pVortexCoreFrame = new QFrame;
        {
            QHBoxLayout *pVortexLayout = new QHBoxLayout;
            {
                QVBoxLayout *pLeftLayout = new QVBoxLayout;
                {
                    QGridLayout *pVortexCoreLayout = new QGridLayout;
                    {
                       QLabel *pLabRadius = new QLabel("Core radius");
                        pLabRadius->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        m_pdeCoreRadius    = new FloatEdit(Vortex::coreRadius());
                        QLabel *plabLength2 = new QLabel(Units::lengthUnitLabel());

                        QLabel *pLabVortexModel = new QLabel("Vortex model");
                        m_pcbVortexModel = new QComboBox;
                        QStringList items;
                        items << "POTENTIAL" << "CUT_OFF" << "LAMB_OSEEN" << "RANKINE" << "SCULLY" << "VATISTAS";
                        m_pcbVortexModel->addItems(items);

                        pVortexCoreLayout->addWidget(pLabRadius,       1, 1);
                        pVortexCoreLayout->addWidget(m_pdeCoreRadius,  1, 2);
                        pVortexCoreLayout->addWidget(plabLength2,      1, 3);
                        pVortexCoreLayout->addWidget(pLabVortexModel,  2, 1);
                        pVortexCoreLayout->addWidget(m_pcbVortexModel, 2, 2);
                        pVortexCoreLayout->setColumnStretch(3,1);
                        pVortexCoreLayout->setRowStretch(3,1);
                    }

                    QString tip("<p>The core radius is a critical distance from the center of the cylinder "
                                "around the trailing vortices and the wake panel edges.<br>"
                                "It is used to calculate a damping factor to apply to the tangential potential velocity. "
                                "This is to prevent the velocity from going to infinity when calculating "
                                "the induced drag in the far-field plane, or when computing the streamlines.<br>"
                                "This value should be as high as possible, but at least one order of magnitude "
                                "less than the smallest panel's width.<br>"
                                "<b>Recommendations:<b>"
                                "<ul>"
                                "<li>core radius = ~0.001 mm</li>"
                                "<li>Lamb-Oseen model.</li"
                                "</ul>"
                                "</p>");
                    QLabel *pLabComment = new QLabel(tip);
                    pLabComment->setWordWrap(true);

                    pLeftLayout->addLayout(pVortexCoreLayout);
                    pLeftLayout->addStretch();
                    pLeftLayout->addWidget(pLabComment);
                }

                m_pVortexGraphWt = new GraphWt(this);
                {
                    m_pVortexGraphWt->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                    m_pVortexGraphWt->enableContextMenu(true);
                    m_pVortexGraphWt->enableCurveStylePage(true);
                    Graph *m_pGraph = new Graph;
                    m_pVortexGraphWt->setGraph(m_pGraph);
                    GraphOptions::resetGraphSettings(*m_pGraph);
                    m_pGraph->setName("Vortex model");
                    m_pGraph->setCurveModel(new CurveModel);
                    m_pGraph->setXVariableList({"r ("+Units::lengthUnitLabel()+")"});
                    m_pGraph->setYVariableList({"V_"+THETACHAR});
                    m_pGraph->setLegendPosition(Qt::AlignTop|Qt::AlignLeft);
                    m_pGraph->setLegendVisible(true);
                    m_pGraph->setScaleType(GRAPH::RESETTING);
                    m_pGraph->setAuto(true);
                }

                pVortexLayout->addLayout(pLeftLayout);
                pVortexLayout->addWidget(m_pVortexGraphWt);
                pVortexLayout->setStretchFactor(pLeftLayout, 1);
                pVortexLayout->setStretchFactor(m_pVortexGraphWt, 3);
            }
            pVortexCoreFrame->setLayout(pVortexLayout);
        }

        QFrame *pVPWFrame = new QFrame;
        {
            QGridLayout *pVPWBoxLayout = new QGridLayout;
            {
                QString tip = "Activates the vorton strength exchange due to viscous diffusion and vortex stretching.\n"
                      "See \'A boundary element-vortex particle hybrid method with inviscid shedding scheme\', Youjiang Wang et al.\n"
                      "for detailed explanations.\n"
                      "Recommendation: activate";
                m_pchVortonStrengthEx = new QCheckBox("Vorton strength exchange");
                m_pchVortonStrengthEx->setToolTip(tip);

                tip = "Activates the vorton space redistribution do imporve numerical stability.\n"
                      "See \'A boundary element-vortex particle hybrid method with inviscid shedding scheme\', Youjiang Wang et al.\n"
                      "for detailed explanations.\n"
                      "Recommendation: activate";
                m_pchVortonRedist = new QCheckBox("Vorton redistribution");
                m_pchVortonRedist->setToolTip(tip);

                QLabel *pLabCoreSize = new QLabel("The vorton core size and VPW length have been moved to the analysis definition.");

                pVPWBoxLayout->addWidget(m_pchVortonStrengthEx, 1, 1, 1, 2);
                pVPWBoxLayout->addWidget(m_pchVortonRedist,     2, 1, 1, 2);
                pVPWBoxLayout->addWidget(pLabCoreSize,          3, 1, 1, 3);
                pVPWBoxLayout->setRowStretch(5,1);
                pVPWBoxLayout->setColumnStretch(4,1);
            }

            pVPWFrame->setLayout(pVPWBoxLayout);
        }

        m_pTabWt->addTab(pCommonFrame,     "Common");
        m_pTabWt->addTab(pLLTFrame,        "LLT");
        m_pTabWt->addTab(pVLMFrame,        "VLM");
        m_pTabWt->addTab(p3dPanelFrame,    "3d linear panels");
        m_pTabWt->addTab(pSolverFrame,     "Solver");
        m_pTabWt->addTab(pViscLoopFrame,   "Viscous loop");
        m_pTabWt->addTab(pVortexCoreFrame, "Vortex");
        m_pTabWt->addTab(pVPWFrame,        "Vorton wake");
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::Reset);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pTabWt);
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);

    connect(m_pTabWt,            SIGNAL(currentChanged(int)),           SLOT(onMakeVortexGraph()));
    connect(m_pdeCoreRadius,     SIGNAL(floatChanged(float)),           SLOT(onMakeVortexGraph()));
    connect(m_pcbVortexModel,    SIGNAL(currentIndexChanged(int)),      SLOT(onMakeVortexGraph()));
}


void Analysis3dSettings::saveSettings(QSettings &settings)
{
    settings.beginGroup("Analysis3dSettings");
    {
        settings.setValue("WindowGeom",         s_WindowGeometry);

        settings.setValue("KeepOpenOnErrors",   s_bKeepOpenOnErrors);

        settings.setValue("DoublePrecision",    PanelAnalysis::bDoublePrecision());

        settings.setValue("ViscInitVTwist",     PlaneTask::bViscInitVTwist());
        settings.setValue("ViscRelaxFactor",    PlaneTask::viscRelaxFactor());
        settings.setValue("MaxViscIter",        PlaneTask::maxViscIter());
        settings.setValue("MaxViscError",       PlaneTask::maxViscError());

        settings.setValue("VortonSE",           Task3d::bVortonStretch());
        settings.setValue("VortonRedist",       Task3d::bVortonRedist());

        settings.setValue("MaxNRHS",            Task3d::maxNRHS());


        settings.setValue("VortexModel",        Vortex::vortexModel());
        settings.setValue("CoreRadius",        Vortex::coreRadius());
        settings.setValue("RFF",                Panel::RFF());
        settings.setValue("MinPanelSize",       WingXfl::minSurfaceLength());
    }
    settings.endGroup();
}


void Analysis3dSettings::loadSettings(QSettings &settings)
{
    settings.beginGroup("Analysis3dSettings");
    {
        s_WindowGeometry    = settings.value("WindowGeom", s_WindowGeometry).toByteArray();

        s_bKeepOpenOnErrors = settings.value("KeepOpenOnErrors",  s_bKeepOpenOnErrors).toBool();

        PanelAnalysis::setDoublePrecision(settings.value("DoublePrecision", true).toBool());

        Task3d::setMaxNRHS(           settings.value("MaxNRHS",            Task3d::maxNRHS()).toInt());

        PlaneTask::setViscInitVTwist(    settings.value("ViscInitVTwist",     PlaneTask::bViscInitVTwist()).toBool());
        PlaneTask::setMaxViscIter(       settings.value("MaxViscIter",        PlaneTask::maxViscIter()).toInt());
        PlaneTask::setMaxViscError(      settings.value("MaxViscError",       PlaneTask::maxViscError()).toDouble());

        Task3d::setVortonStretch(    settings.value("VortonSE",       Task3d::bVortonStretch()).toBool());
        Task3d::setVortonRedist(settings.value("VortonRedist",   Task3d::bVortonRedist()).toBool());

        switch (settings.value("VortexModel", Vortex::vortexModel()).toInt())
        {
            case 0: Vortex::setVortexModel(Vortex::POTENTIAL);   break;
            case 1: Vortex::setVortexModel(Vortex::CUT_OFF);     break;
            case 2: Vortex::setVortexModel(Vortex::LAMB_OSEEN);  break;
            case 3: Vortex::setVortexModel(Vortex::RANKINE);     break;
            case 4: Vortex::setVortexModel(Vortex::SCULLY);      break;
            case 5: Vortex::setVortexModel(Vortex::VATISTAS);    break;
        }
        Vortex::setCoreRadius(settings.value("CoreRadius", Vortex::coreRadius()).toDouble());
        Panel::setRFF(settings.value("RFF", Panel::RFF()).toDouble());
        WingXfl::setMinSurfaceLength(settings.value("MinPanelSize", WingXfl::minSurfaceLength()).toDouble());
    }
    settings.endGroup();
}


void Analysis3dSettings::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);
}


void Analysis3dSettings::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_iPage = m_pTabWt->currentIndex();
    s_WindowGeometry = saveGeometry();
}


void Analysis3dSettings::initDialog(int iPage)
{
    setData();

    m_pdeVortexPos->setEnabled(false);
    m_pdeControlPos->setEnabled(false);

    if(iPage>=0) s_iPage=iPage;
    m_pTabWt->setCurrentIndex(s_iPage);
}


void Analysis3dSettings::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Reset) == pButton)  onResetDefaults();
    else if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)  reject();
}


void Analysis3dSettings::reject()
{
    readData();
    QDialog::reject();
}


void Analysis3dSettings::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
                m_pButtonBox->button(QDialogButtonBox::Close)->setFocus();
            else {
                close();
            }
            break;
        }
        case Qt::Key_Escape:
        {
            QDialog::reject();
            break;
        }
        default:
            break;
    }
    pEvent->accept();
}


void Analysis3dSettings::setData()
{
    m_pfeLLTRelax->setValue(LLTTask::relaxationFactor());
    m_pfeLLTAlphaPrec->setValue(LLTTask::convergencePrecision());
    m_pieLLTNStation->setValue(LLTTask::nSpanStations());
    m_pieLLTIterMax->setValue(LLTTask::maxIter());

    m_pchKeepOpenOnErrors->setChecked(s_bKeepOpenOnErrors);

    m_pieMaxRHS->setValue(Task3d::maxNRHS());

    m_pcbVortexModel->setCurrentIndex(Vortex::vortexModel());
    m_pdeCoreRadius->setValue(Vortex::coreRadius()* Units::mtoUnit());


    m_pdeRFF->setValue(Panel::RFF());
    m_pdeMinPanelSize->setValue(WingXfl::minSurfaceLength()* Units::mtoUnit());

    m_pdeControlPos->setValue(Panel4::ctrlPtFracPos()*100.0);
    m_pdeVortexPos->setValue(Panel4::vortexFracPos()*100.0);

    m_pieQuadPoints->setValue(Panel3::quadratureOrder());

    m_prbSinglePrecision->setChecked(!PanelAnalysis::bDoublePrecision());
    m_prbDoublePrecision->setChecked(PanelAnalysis::bDoublePrecision());

    //Viscous loop
    m_pchViscInitVTwist->setChecked(PlaneTask::bViscInitVTwist());
    m_pfeViscPanelRelax->setValue(PlaneTask::viscRelaxFactor());
    m_pfeViscPanelTwistPrec->setValue(PlaneTask::maxViscError());
    m_pieViscPanelIterMax->setValue(PlaneTask::maxViscIter());

    // Vortex particle wake
//    m_pchVortonStrengthEx->setChecked(Task3d::bVortonStretch());
//    m_pchVortonRedist->setChecked(Task3d::bVortonRedist());
    m_pchVortonStrengthEx->setChecked(false);
    m_pchVortonRedist->setChecked(false);
    m_pchVortonStrengthEx->setEnabled(false);
    m_pchVortonRedist->setEnabled(false);
}


void Analysis3dSettings::readData()
{
    s_iPage = m_pTabWt->currentIndex();
    LLTTask::setMaxIter(m_pieLLTIterMax->value());
    LLTTask::setConvergencePrecision(m_pfeLLTAlphaPrec->value());
    LLTTask::setNSpanStations(m_pieLLTNStation->value());
    LLTTask::setRelaxationFactor(m_pfeLLTRelax->value());


    s_bKeepOpenOnErrors = m_pchKeepOpenOnErrors->isChecked();

    PanelAnalysis::setDoublePrecision(m_prbDoublePrecision->isChecked());

    Panel3::setQuadratureOrder(m_pieQuadPoints->value());

    WingXfl::setMinSurfaceLength(m_pdeMinPanelSize->value() / Units::mtoUnit());

    Task3d::setMaxNRHS(m_pieMaxRHS->value());

    Panel::setRFF(m_pdeRFF->value());

    double CoreRadius = m_pdeCoreRadius->value() / Units::mtoUnit();
    Vortex::setCoreRadius(CoreRadius);
    switch(m_pcbVortexModel->currentIndex())
    {
        default:
        case 0: Vortex::setVortexModel(Vortex::POTENTIAL);  break;
        case 1: Vortex::setVortexModel(Vortex::CUT_OFF);    break;
        case 2: Vortex::setVortexModel(Vortex::LAMB_OSEEN); break;
        case 3: Vortex::setVortexModel(Vortex::RANKINE);    break;
        case 4: Vortex::setVortexModel(Vortex::SCULLY);     break;
        case 5: Vortex::setVortexModel(Vortex::VATISTAS);   break;
    }

    Panel4::setCtrlPtFracPos(m_pdeControlPos->value()/100.0);
    Panel4::setVortexFracPos(m_pdeVortexPos->value()/100.0);

    // Viscous loop
    PlaneTask::setViscInitVTwist(m_pchViscInitVTwist->isChecked());
    PlaneTask::setViscRelaxFactor(m_pfeViscPanelRelax->value());
    PlaneTask::setMaxViscError(m_pfeViscPanelTwistPrec->value());
    PlaneTask::setMaxViscIter(m_pieViscPanelIterMax->value());

    // VPW
    Task3d::setVortonStretch(m_pchVortonStrengthEx->isChecked());
    Task3d::setVortonRedist(m_pchVortonRedist->isChecked());
}


void Analysis3dSettings::onMakeVortexGraph()
{
    Graph *pGraph = m_pVortexGraphWt->graph();
    if(!pGraph) return;
    if(m_pTabWt->currentIndex()!=6) return;
    pGraph->deleteCurves();

    double corerad = m_pdeCoreRadius->value()/Units::mtoUnit();

    int iModel = m_pcbVortexModel->currentIndex();

    Vector3d A(0,0,-5);
    Vector3d B(0,0, 5);
    Vector3d V, pt;

    double Vyref = 100.0; // used to limit the vertical scale
    if(corerad>0)
    {
        vortexInducedVelocity(A, B, {corerad,0,0}, V, corerad, Vortex::POTENTIAL);
        Vyref = fabs(V.y);
    }

    Curve *pCurve[7] {nullptr};
    pCurve[0] = pGraph->addCurve("POTENTIAL (left)");
    pCurve[0]->setColor(Qt::darkGray);
    pCurve[0]->setStipple(Line::DASH);
    pCurve[1] = pGraph->addCurve("POTENTIAL (right)");
    pCurve[1]->setColor(Qt::darkGray);
    pCurve[1]->setStipple(Line::DASH);
    pCurve[2] = pGraph->addCurve("CUT_OFF");
    pCurve[2]->setColor(Qt::yellow);
    pCurve[3] = pGraph->addCurve("LAMB_OSEEN");
    pCurve[3]->setColor(Qt::red);
    pCurve[4] = pGraph->addCurve("RANKINE");
    pCurve[4]->setColor(xfl::Orchid);
    pCurve[5] = pGraph->addCurve("SCULLY");
    pCurve[5]->setColor(xfl::CornFlowerBlue);
    pCurve[6] = pGraph->addCurve("VATISTAS");
    pCurve[6]->setColor(xfl::PhugoidGreen);
    if(iModel==0)
    {
        pCurve[0]->setWidth(5);
        pCurve[1]->setWidth(5);
    }
    else if(iModel<6)
    {
        pCurve[iModel+1]->setWidth(5);
    }

    double xmin = -3.0*corerad;
    double xmax = +3.0*corerad;

    for(int i=0; i<=100; i++)
    {
        double x = xmin + double(i) *(xmax-xmin)/100.0;
        pt.set(x, 0, 0);
        if(x<-DISTANCEPRECISION)
        {
            vortexInducedVelocity(A, B, pt, V, corerad, Vortex::POTENTIAL);
            if(fabs(V.y)<2.0*Vyref) pCurve[0]->appendPoint(x*Units::mtoUnit(), V.y);
        }
        else if(x>DISTANCEPRECISION)
        {
            vortexInducedVelocity(A, B, pt, V, corerad, Vortex::POTENTIAL);
            if(fabs(V.y)<2.0*Vyref) pCurve[1]->appendPoint(x*Units::mtoUnit(), V.y);
        }
        vortexInducedVelocity(A, B, pt, V, corerad, Vortex::CUT_OFF);
        pCurve[2]->appendPoint(x*Units::mtoUnit(), V.y);
        vortexInducedVelocity(A, B, pt, V, corerad, Vortex::LAMB_OSEEN);
        pCurve[3]->appendPoint(x*Units::mtoUnit(), V.y);
        vortexInducedVelocity(A, B, pt, V, corerad, Vortex::RANKINE);
        pCurve[4]->appendPoint(x*Units::mtoUnit(), V.y);
        vortexInducedVelocity(A, B, pt, V, corerad, Vortex::SCULLY);
        pCurve[5]->appendPoint(x*Units::mtoUnit(), V.y);
        vortexInducedVelocity(A, B, pt, V, corerad, Vortex::VATISTAS);
        pCurve[6]->appendPoint(x*Units::mtoUnit(), V.y);
    }
    pGraph->invalidate();
    update();
}



