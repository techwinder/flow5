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
#include <QGridLayout>
#include <QVBoxLayout>
#include <QAction>

#include "popp3dctrls.h"
#include <core/xflcore.h>
#include <interfaces/controls/poppctrls/crossflowctrls.h>
#include <interfaces/controls/poppctrls/flowctrls.h>
#include <interfaces/controls/poppctrls/opp3dscalesctrls.h>
#include <interfaces/controls/poppctrls/streamlinesctrls.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <modules/xplane/controls/stab3dctrls.h>
#include <modules/xplane/glview/gl3dxplaneview.h>
#include <modules/xplane/xplane.h>
#include <interfaces/opengl/controls/fine3dcontrols.h>
#include <api/units.h>
#include <api/utils.h>
#include <api/planeopp.h>
#include <api/planepolar.h>
#include <api/objects3d.h>
#include <api/planexfl.h>



XPlane *POpp3dCtrls::s_pXPlane = nullptr;


POpp3dCtrls::POpp3dCtrls(gl3dXflView*p3dView, QWidget *pParent) : QTabWidget(pParent)
{
    setMovable(true);

    m_bLiftStrip      = false;
    m_bXTop           = false;
    m_bXBot           = false;
    m_bICd            = false;
    m_bVCd            = false;
    m_bGamma          = false;
    m_bPanelForce     = false;
    m_b3dCp           = false;
    m_bDownwash       = false;
    m_bPartForces     = false;
    m_bFlaps          = false;
    m_bMoments        = false;
    m_bStreamLines    = false;
    m_bWakePanels     = false;
    m_bVortons        = false;
    m_bHPlane         = false;
    m_bAnimateWOpp    = false;

    m_bAnimateWOppPlus   = true;
    m_pTimerWOpp= new QTimer(this);
    m_posAnimateWOpp = 0;

    m_pgl3dXPlaneView = dynamic_cast<gl3dXPlaneView*>(p3dView);
    m_pgl3dXPlaneView->setResultControls(this);

    setupLayout();
    connectSignals();
}


POpp3dCtrls::~POpp3dCtrls()
{
}


void POpp3dCtrls::setXPlane(XPlane *pXPlane)
{
    s_pXPlane = pXPlane;
    Opp3dScalesCtrls::setXPlane(pXPlane);
    FlowCtrls::setXPlane(pXPlane);
    CrossFlowCtrls::setXPlane(pXPlane);
}


void POpp3dCtrls::set3dXPlaneView(gl3dXPlaneView *pView)
{
    m_pgl3dXPlaneView = pView;

    m_pOpp3dScalesCtrls->set3dXPlaneView(pView);
    m_pStreamLinesCtrls->set3dXPlaneView(pView);
    m_pFlowCtrls->set3dXPlaneView(pView);
    m_pCrossFlowCtrls->set3dXPlaneView(pView);
}


void POpp3dCtrls::setupLayout()
{
    QFrame *pfr3dPage = new QFrame;
    {
        QVBoxLayout *p3dLayout = new QVBoxLayout;
        {
            QGridLayout *pCheckDispLayout = new QGridLayout;
            {
                m_pchGamma          = new QCheckBox(GAMMAch);
                QString gammatip("<p>Displays the distribution of doublet densities or vortex strengths.<br>"
                                 "This is the main result of the potential flow calculation, and <u>all other results</u> "
                                 "are derived from this distribution. It is therefore important that it "
                                 "does not exhibit numerical issues which would show up as singularities i.e. areas with peak values."
                                 "</p>");
                m_pchGamma->setToolTip(gammatip);
                m_pchCp             = new QCheckBox("Cp");
                m_pchCp->setToolTip("<p>Display the panel pressure coefficients</p>");
                m_pchPanelForce     = new QCheckBox("F/s=q.Cp");
                m_pchPanelForce->setToolTip("<p>Display the pressures &frac12; &rho; V<sup>2</sup> Cp acting on the panels</p>");
                m_pchStripLift      = new QCheckBox("Strip lift");
                m_pchPartForces     = new QCheckBox("Part forces");
                m_pchIDrag          = new QCheckBox("Induced drag");
                m_pchVDrag          = new QCheckBox("Viscous drag");
                m_pchTrans          = new QCheckBox("Transitions");
                m_pchMoment         = new QCheckBox("Moments");
                m_pchDownwash       = new QCheckBox("Downwash");
                m_pchStream         = new QCheckBox("Streamlines");
                m_pchFlaps          = new QCheckBox("Flaps");
                m_pchFlow           = new QCheckBox("Flow");
                m_pchFlow->setToolTip("<p>Launches an animation of the flow around the model.<br>"
                                      "The parameters are set in the last tab of the scales widget.</p>");
                m_pchWakePanels     = new QCheckBox("Wake panels");
                m_pchWakePanels->setToolTip("<p>T6 polars only</p>");
                m_pchVortons        = new QCheckBox("Vortons");
                m_pchVortons->setToolTip("<p>T6 polars + VPW only</p>");
                m_pchHPlane         = new QCheckBox("Ground/Free surface");
                m_pchPickPanel      = new QCheckBox("Pick value");
                m_pchPickPanel->setShortcut(QKeySequence(Qt::SHIFT|Qt::Key_H));
                QString tip("<p>Activate this checkbox and move the mouse\nover the Cp or Forces color plot. (Shift+H)</p>");
                m_pchPickPanel->setToolTip(tip);
                m_pchPOppAnimate    = new QCheckBox("Animate");

                m_pslAnimPOppSpeed  = new QSlider(Qt::Horizontal);
                m_pslAnimPOppSpeed->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
                m_pslAnimPOppSpeed->setMinimum(0);
                m_pslAnimPOppSpeed->setMaximum(500);
                m_pslAnimPOppSpeed->setSliderPosition(250);
                m_pslAnimPOppSpeed->setTickInterval(50);
                m_pslAnimPOppSpeed->setTickPosition(QSlider::TicksBelow);
                pCheckDispLayout->addWidget(m_pchCp,               2, 1);
                pCheckDispLayout->addWidget(m_pchGamma,            2, 2);
                pCheckDispLayout->addWidget(m_pchStripLift,        3, 1);
                pCheckDispLayout->addWidget(m_pchPanelForce,       3, 2);
                pCheckDispLayout->addWidget(m_pchPartForces,       4, 1);
                pCheckDispLayout->addWidget(m_pchMoment,           4, 2);
                pCheckDispLayout->addWidget(m_pchIDrag,            5, 1);
                pCheckDispLayout->addWidget(m_pchVDrag,            5, 2);
                pCheckDispLayout->addWidget(m_pchTrans,            6, 1);
                pCheckDispLayout->addWidget(m_pchDownwash,         6, 2);
                pCheckDispLayout->addWidget(m_pchFlaps,            7, 1);
                pCheckDispLayout->addWidget(m_pchStream,           7, 2);
                pCheckDispLayout->addWidget(m_pchWakePanels,       8, 1);
                pCheckDispLayout->addWidget(m_pchFlow,             8, 2);
                pCheckDispLayout->addWidget(m_pchVortons,          9, 1);
                pCheckDispLayout->addWidget(m_pchHPlane,           9, 2);
                pCheckDispLayout->addWidget(m_pchPickPanel,       10, 1);
                pCheckDispLayout->addWidget(m_pchPOppAnimate,     11, 1);
                pCheckDispLayout->addWidget(m_pslAnimPOppSpeed,   11, 2);

                pCheckDispLayout->setRowStretch(13,1);
                pCheckDispLayout->setColumnStretch(1,1);
                pCheckDispLayout->setColumnStretch(2,1);

                pCheckDispLayout->setSpacing(0);
            }

            m_pgl3dCtrls = new gl3dGeomControls(m_pgl3dXPlaneView, WingLayout, false);
            m_pgl3dCtrls->showTessCtrl(false);
            m_pgl3dCtrls->showNormalCtrl(false);
            m_pgl3dCtrls->showHighlightCtrl(false);

            p3dLayout->addLayout(pCheckDispLayout);
            p3dLayout->addWidget(m_pgl3dCtrls);
        }
        pfr3dPage->setLayout(p3dLayout);
    }

    m_pStab3dCtrls      = new Stab3dCtrls;
    m_pOpp3dScalesCtrls = new Opp3dScalesCtrls;
    m_pStreamLinesCtrls = new StreamLineCtrls;
    m_pFlowCtrls        = new FlowCtrls;
    m_pCrossFlowCtrls   = new CrossFlowCtrls;

    addTab(pfr3dPage,           "Display");
    addTab(m_pOpp3dScalesCtrls, "Scales");
    addTab(m_pStab3dCtrls,      "Stability");
    addTab(m_pStreamLinesCtrls,  "Streamlines");
    addTab(m_pFlowCtrls,        "Flow");
    addTab(m_pCrossFlowCtrls,   "Wake");
}


void POpp3dCtrls::connectSignals()
{
    connect(m_pchFlaps,         SIGNAL(clicked()),     SLOT(onFlaps()));
    connect(m_pchGamma,         SIGNAL(clicked()),     SLOT(onGamma()));
    connect(m_pchCp,            SIGNAL(clicked()),     SLOT(on3dCp()));
    connect(m_pchPanelForce,    SIGNAL(clicked()),     SLOT(onPanelForce()));
    connect(m_pchStripLift,     SIGNAL(clicked()),     SLOT(onStripLift()));
    connect(m_pchPartForces,    SIGNAL(clicked()),     SLOT(onPartForces()));
    connect(m_pchIDrag,         SIGNAL(clicked()),     SLOT(onShowIDrag()));
    connect(m_pchVDrag,         SIGNAL(clicked()),     SLOT(onShowVDrag()));
    connect(m_pchTrans,         SIGNAL(clicked()),     SLOT(onShowTransitions()));
    connect(m_pchPickPanel,     SIGNAL(clicked()),     SLOT(on3dPickCp()));
    connect(m_pchMoment,        SIGNAL(clicked()),     SLOT(onMoment()));
    connect(m_pchDownwash,      SIGNAL(clicked()),     SLOT(onDownwash()));
    connect(m_pchWakePanels,    SIGNAL(clicked()),     SLOT(onWakePanels()));
    connect(m_pchVortons,       SIGNAL(clicked()),     SLOT(onVortons()));
    connect(m_pchHPlane,        SIGNAL(clicked()),     SLOT(onGround()));
    connect(m_pchStream,        SIGNAL(clicked(bool)), SLOT(onStreamlines(bool)));
    connect(m_pchFlow,          SIGNAL(clicked(bool)), SLOT(onFlow(bool)));

    connect(m_pchPOppAnimate,   SIGNAL(clicked()),        SLOT(onAnimatePOpp()));
    connect(m_pslAnimPOppSpeed, SIGNAL(sliderMoved(int)), SLOT(onAnimatePOppSpeed(int)));

    connect(m_pTimerWOpp,       SIGNAL(timeout()), SLOT(onAnimatePOppSingle()));

}


void POpp3dCtrls::initWidget()
{
    m_pOpp3dScalesCtrls->initWidget();
    m_pStab3dCtrls->setControls();
    m_pStreamLinesCtrls->initWidget();
    m_pFlowCtrls->initWidget();
    m_pCrossFlowCtrls->initWidget();
}


void POpp3dCtrls::POpp3dCtrls::updateUnits()
{
    m_pOpp3dScalesCtrls->updateUnits();
    m_pStreamLinesCtrls->updateUnits();
    m_pFlowCtrls->updateUnits();
    m_pCrossFlowCtrls->updateUnits();
}


void POpp3dCtrls::setControls()
{
    m_pgl3dCtrls->setControls();

    PlaneXfl const* pPlaneXfl = dynamic_cast<PlaneXfl const*>(s_pXPlane->curPlane());
    PlanePolar const *pWPolar = s_pXPlane->curWPolar();
    PlaneOpp const *pPOpp = s_pXPlane->curPOpp();

    m_pchFlaps->setEnabled((pPlaneXfl && pPlaneXfl->nFlaps()>0));
    m_pchGamma->setEnabled(           pPOpp && !pPOpp->isLLTMethod());
    m_pchCp->setEnabled(              pPOpp && !pPOpp->isLLTMethod());
    m_pchPanelForce->setEnabled(      pPOpp && !pPOpp->isLLTMethod());
    m_pchStripLift->setEnabled(        pPOpp);
    m_pchPartForces->setEnabled(      pPOpp);
    m_pchTrans->setEnabled(           pPOpp);
    m_pchIDrag->setEnabled(           pPOpp);
    m_pchVDrag->setEnabled(           pPOpp);
    m_pchDownwash->setEnabled(        pPOpp);
    m_pchMoment->setEnabled(          pPOpp && !pPOpp->isLLTMethod());
    m_pchStream->setEnabled(          pPOpp && !pPOpp->isLLTMethod());
#ifdef Q_OS_MAC
    m_pchFlow->setEnabled(false);
#else
    int oglmajor = gl3dView::oglMajor();
    int oglminor = gl3dView::oglMinor();
    if(oglmajor*10+oglminor<43)
    {
        m_pchFlow->setEnabled(false);
    }
    else m_pchFlow->setEnabled(pPOpp && pPOpp->isTriUniformMethod());
#endif

    m_pchCp->setChecked(m_b3dCp);
    m_pchDownwash->setChecked(m_bDownwash);
    m_pchFlaps->setChecked(m_bFlaps);
    m_pchGamma->setChecked(m_bGamma);
    m_pchHPlane->setChecked(pWPolar && pWPolar->bHPlane() && m_bHPlane);
    m_pchHPlane->setEnabled(pWPolar && pWPolar->bHPlane());
    m_pchIDrag->setChecked(m_bICd);
    m_pchMoment->setChecked(m_bMoments);
    m_pchPOppAnimate->setEnabled(pPOpp);
    m_pchPanelForce->setChecked(m_bPanelForce);
    m_pchPartForces->setChecked(m_bPartForces);
    m_pchStream->setChecked(m_bStreamLines);
    m_pchStripLift->setChecked(m_bLiftStrip);
    m_pchTrans->setChecked(m_bXTop);
    m_pchVDrag->setChecked(m_bVCd);
    m_pslAnimPOppSpeed->setEnabled(pPOpp && m_pchPOppAnimate->isChecked());


    m_pchPickPanel->setChecked(m_pgl3dXPlaneView->isPicking());

    if(pPOpp && pPOpp->isLLTMethod())
    {
        m_bStreamLines = m_b3dCp = m_bPanelForce = m_bMoments = false;

        m_pchGamma->setChecked(       false);
        m_pchCp->setChecked(          false);
        m_pchPanelForce->setChecked(  false);
        m_pchStream->setChecked(      false);
        m_pchFlow->setChecked(        false);
        m_pchMoment->setChecked(      false);

        m_pchPickPanel->setChecked(false);
    }


    if(!pPOpp)
        m_pgl3dXPlaneView->showColourLegend(false);
    else
        m_pgl3dXPlaneView->showColourLegend(needsColorLegend());

    m_pchVortons->setEnabled(pPOpp && pPOpp->isType6() && pPOpp->hasVortons());
    m_pchVortons->setChecked(m_bVortons);

    m_pchHPlane->setChecked(m_bHPlane);

    m_pchWakePanels->setChecked(m_bWakePanels);
    m_pchWakePanels->setEnabled(pWPolar && pWPolar->isType6());
    m_pchWakePanels->setEnabled(pWPolar);

}


bool POpp3dCtrls::needsColorLegend() const
{
    PlaneOpp const *pPOpp = s_pXPlane->m_pCurPOpp;

    bool bVorticity = m_pCrossFlowCtrls->bVorticityMap() && pPOpp && pPOpp->hasVortons();
    return m_b3dCp || m_bGamma  || m_bPanelForce  || bVorticity;
}


void POpp3dCtrls::loadSettings(QSettings &settings)
{
    settings.beginGroup("POpp3dCtrls");
    {
        m_bGamma        = settings.value("bGamma",      false).toBool();
        m_b3dCp         = settings.value("b3DCp",       false).toBool();
        m_bPanelForce   = settings.value("bPanelForce", false).toBool();
        m_bXTop         = settings.value("bXTop",       false).toBool();
        m_bXBot         = settings.value("bXBot",       false).toBool();
        m_bLiftStrip    = settings.value("bLiftStrip",  false).toBool();
        m_bICd          = settings.value("bICd",        false).toBool();
        m_bVCd          = settings.value("bVCd",        false).toBool();
        m_bDownwash     = settings.value("bDownwash",   false).toBool();
        m_bPartForces   = settings.value("bPartForces", false).toBool();
        m_bFlaps        = settings.value("bFlaps",      false).toBool();
        m_bMoments      = settings.value("bMoments",    false).toBool();
        m_bWakePanels   = settings.value("bWakePanels", false).toBool();
        m_bVortons      = settings.value("bVortons",    false).toBool();
        m_bHPlane       = settings.value("bGround",     false).toBool();
    }
    settings.endGroup();
}


void POpp3dCtrls::saveSettings(QSettings &settings)
{
    settings.beginGroup("POpp3dCtrls");
    {
        settings.setValue("bGamma",      m_bGamma);
        settings.setValue("b3DCp",       m_b3dCp);
        settings.setValue("bPanelForce", m_bPanelForce);
        settings.setValue("bICd",        m_bICd);
        settings.setValue("bVCd",        m_bVCd);
        settings.setValue("bXTop",       m_bXTop);
        settings.setValue("bXBot",       m_bXBot);
        settings.setValue("bLiftStrip",  m_bLiftStrip);
        settings.setValue("bDownwash",   m_bDownwash);
        settings.setValue("bPartForces", m_bPartForces);
        settings.setValue("bFlaps",      m_bFlaps);
        settings.setValue("bMoments",    m_bMoments);
        settings.setValue("bWakePanels", m_bWakePanels);
        settings.setValue("bVortons",    m_bVortons);
        settings.setValue("bground",     m_bHPlane);
    }
    settings.endGroup();
}


void POpp3dCtrls::on3dCp()
{
    m_b3dCp = m_pchCp->isChecked();

    if(m_b3dCp)
    {
        m_pgl3dXPlaneView->s_bResetglPanelCp = true;
        m_pgl3dXPlaneView->m_bSurfaces = false;
        m_pgl3dCtrls->checkSurfaces(false);

        m_bGamma = false;
        m_pchGamma->setChecked(false);

        m_bPanelForce = false;
        m_pchPanelForce->setChecked(false);
    }

    double qDyn=1.0;
    PlaneOpp *pPOpp = s_pXPlane->m_pCurPOpp;
    if(pPOpp && m_b3dCp)
    {
        if(s_pXPlane->m_pCurPOpp->isTriLinearMethod())
        {
            TriMesh::makeNodeValues(s_pXPlane->m_pCurPlane->triMesh().nodes(), s_pXPlane->m_pCurPlane->triMesh().panels(),
                                    pPOpp->Cp(), pPOpp->m_NodeValue,
                                    pPOpp->m_NodeValMin, pPOpp->m_NodeValMax,
                                    qDyn);
        }
    }

    m_pgl3dXPlaneView->showColourLegend(needsColorLegend());
    m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::onGamma()
{
    m_bGamma = m_pchGamma->isChecked();

    if(m_bGamma)
    {
        m_pgl3dXPlaneView->s_bResetglPanelGamma = true;

        m_pgl3dXPlaneView->m_bSurfaces = false;
        m_pgl3dCtrls->checkSurfaces(false);

        m_bPanelForce = false;
        m_pchPanelForce->setChecked(false);

        m_b3dCp = false;
        m_pchCp->setChecked(false);
    }

    double qDyn=1.0;
    PlaneOpp *pPOpp = s_pXPlane->m_pCurPOpp;
    if(pPOpp && m_bGamma)
    {
        if(pPOpp->isTriLinearMethod())
        {
            TriMesh::makeNodeValues(s_pXPlane->m_pCurPlane->triMesh().nodes(), s_pXPlane->m_pCurPlane->triMesh().panels(),
                                    pPOpp->gamma(), pPOpp->m_NodeValue,
                                    pPOpp->m_NodeValMin, pPOpp->m_NodeValMax,
                                    qDyn);
        }
    }

    m_pgl3dXPlaneView->showColourLegend(needsColorLegend());

    m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::onPanelForce()
{
    m_bPanelForce = m_pchPanelForce->isChecked();

    if(m_bPanelForce)
    {
        m_b3dCp =false;
        m_pchCp->setChecked(false);

        m_bGamma = false;
        m_pchGamma->setChecked(false);
    }
    if(!m_bAnimateWOpp) m_pgl3dXPlaneView->update();


    double qDyn=1.0;
    PlaneOpp *pPOpp = s_pXPlane->m_pCurPOpp;
    if(pPOpp && m_bPanelForce)
    {
        qDyn=0.5*s_pXPlane->m_pCurWPolar->density()*s_pXPlane->m_pCurPOpp->QInf()*s_pXPlane->m_pCurPOpp->QInf();
        qDyn *= Units::PatoUnit();

        if(s_pXPlane->m_pCurPOpp->isTriLinearMethod())
        {
            TriMesh::makeNodeValues(s_pXPlane->m_pCurPlane->triMesh().nodes(), s_pXPlane->m_pCurPlane->triMesh().panels(),
                                    pPOpp->Cp(), pPOpp->m_NodeValue,
                                    pPOpp->m_NodeValMin, pPOpp->m_NodeValMax,
                                    qDyn);
        }
    }

    m_pgl3dXPlaneView->showColourLegend(needsColorLegend());
    m_pgl3dXPlaneView->s_bResetglPanelForce = true;

    m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::on3dPickCp()
{
    if(m_pchPickPanel->isChecked())
    {
        if(!s_pXPlane->m_pCurWPolar)
            m_pgl3dXPlaneView->setPicking(xfl::PANEL3);
        else
        {
            if(s_pXPlane->curWPolar()->isQuadMethod())
                m_pgl3dXPlaneView->setPicking(xfl::PANEL4);
            else if(s_pXPlane->curWPolar()->isTriUniformMethod())
                m_pgl3dXPlaneView->setPicking(xfl::PANEL3);
            else if(s_pXPlane->curWPolar()->isTriLinearMethod())
                m_pgl3dXPlaneView->setPicking(xfl::MESHNODE);
        }
    }
    else
        m_pgl3dXPlaneView->stopPicking();
    m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::onDownwash()
{
    m_bDownwash = m_pchDownwash->isChecked();
    m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::onStripLift()
{
    m_bLiftStrip     = m_pchStripLift->isChecked();
    if(!m_bAnimateWOpp) m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::onPartForces()
{
    m_bPartForces = m_pchPartForces->isChecked();
    m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::onFlaps()
{
    m_bFlaps = m_pchFlaps->isChecked();
    m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::onShowIDrag()
{
    m_bICd = m_pchIDrag->isChecked();
    m_pgl3dXPlaneView->s_bResetglDrag = true;
    if(!m_bAnimateWOpp) m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::onShowVDrag()
{
    m_bVCd = m_pchVDrag->isChecked();
    m_pgl3dXPlaneView->s_bResetglDrag = true;

    if(!m_bAnimateWOpp) m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::onShowTransitions()
{
    m_bXTop = m_pchTrans->isChecked();
    m_bXBot = m_pchTrans->isChecked();

    if(!m_bAnimateWOpp) m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::onMoment()
{
    m_bMoments = m_pchMoment->isChecked();
    m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::stopAnimate()
{
    if(!m_bAnimateWOpp) return;
    m_bAnimateWOpp = false;
    m_pchPOppAnimate->setChecked(false);
    m_pTimerWOpp->stop();
}


void POpp3dCtrls::onWakePanels()
{
    m_bWakePanels = m_pchWakePanels->isChecked();
    m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::onVortons()
{
    m_bVortons = m_pchVortons->isChecked();
    m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::onGround()
{
    m_bHPlane = m_pchHPlane->isChecked();
    m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::onStreamlines(bool bStream)
{
    m_bStreamLines = bStream;
    m_pgl3dXPlaneView->update();
}


void POpp3dCtrls::stopFlow()
{
    m_pchFlow->setChecked(false);
    m_pgl3dXPlaneView->startFlow(false);
}


void POpp3dCtrls::onFlow(bool bFlow)
{
    if(bFlow)
    {
        //update the SSBO and VBO
        m_pgl3dXPlaneView->resetFlow();
    }
    m_pgl3dXPlaneView->startFlow(bFlow);
}


void POpp3dCtrls::onAnimatePOpp()
{
    if(!s_pXPlane->m_pCurPlane || !s_pXPlane->m_pCurWPolar || !s_pXPlane->is3dView())
    {
        m_bAnimateWOpp = false;
        return;
    }

    PlaneOpp*pPOpp;

    if(!m_pchPOppAnimate->isChecked())
    {
        s_pXPlane->stopAnimate();
        m_pslAnimPOppSpeed->setEnabled(false);
        return;
    }
    m_pslAnimPOppSpeed->setEnabled(true);

    for (int l=0; l<Objects3d::nPOpps(); l++)
    {
        pPOpp = Objects3d::POppAt(l);

        if (pPOpp &&
                pPOpp->polarName() == s_pXPlane->m_pCurWPolar->name() &&
                pPOpp->planeName() == s_pXPlane->m_pCurPlane->name())
        {
            if(s_pXPlane->m_pCurPOpp->alpha() - pPOpp->alpha()<AOAPRECISION)
                m_posAnimateWOpp = l;
        }
    }

    m_bAnimateWOpp  = true;
    int speed = m_pslAnimPOppSpeed->value();
    m_pTimerWOpp->setInterval(800-speed);
    m_pTimerWOpp->start();
}


void POpp3dCtrls::onAnimatePOppSingle()
{
    if(!s_pXPlane->is3dView()) return; //nothing to animate
    if(!s_pXPlane->m_pCurPlane || !s_pXPlane->m_pCurWPolar) return;

    int size = Objects3d::nPOpps();
    if(size<=1) return;

    bool bIsValid = false;
    bool bSkipOne = false;

    while(!bIsValid)
    {
        //Find the current position to display

        PlaneOpp *pPOpp = Objects3d::POppAt(m_posAnimateWOpp);
        if(!pPOpp) return;

        bIsValid =(pPOpp->polarName()==s_pXPlane->m_pCurWPolar->name()  &&
                   pPOpp->planeName()==s_pXPlane->m_pCurPlane->name());

        if (bIsValid && !bSkipOne)
        {
            s_pXPlane->setPlaneOpp(pPOpp);

            m_pgl3dXPlaneView->s_bResetglOpp      = true;
            m_pgl3dXPlaneView->s_bResetglDownwash = true;
            m_pgl3dXPlaneView->s_bResetglLift     = true;
            m_pgl3dXPlaneView->s_bResetglMoments  = true;
            m_pgl3dXPlaneView->s_bResetglDrag     = true;
            m_pgl3dXPlaneView->s_bResetglWake     = true;
            m_pgl3dXPlaneView->s_bResetglStream   = true;

            m_pgl3dXPlaneView->update();


            //select the PlanePOpp in the top listbox
            //            s_pMainFrame->SelectWOpp(m_pCurPOpp->m_pPlaneWOpp[0]);
        }
        else if(bIsValid) bSkipOne = false;

        if(m_bAnimateWOppPlus)
        {
            m_posAnimateWOpp++;
            if (m_posAnimateWOpp >= size)
            {
                m_posAnimateWOpp = size-1;
                m_bAnimateWOppPlus = false;
                bSkipOne = true;
            }
        }
        else
        {
            m_posAnimateWOpp--;
            if (m_posAnimateWOpp <0)
            {
                m_posAnimateWOpp = 0;
                m_bAnimateWOppPlus = true;
                bSkipOne = true;
            }
        }

        if(m_posAnimateWOpp<0 || m_posAnimateWOpp>=size) return;
    }
}


void POpp3dCtrls::onAnimatePOppSpeed(int val)
{
    if(m_pTimerWOpp->isActive())
    {
        m_pTimerWOpp->setInterval(800-val);
    }
}


void POpp3dCtrls::stopDistance()
{
    m_pgl3dCtrls->m_ptbDistance->setChecked(false);
    m_pgl3dCtrls->m_ptbDistance->defaultAction()->setChecked(false);
}


bool POpp3dCtrls::getDistance() const
{
    return m_pgl3dCtrls->m_ptbDistance->isChecked();
}


void POpp3dCtrls::resetClipPlane()
{
    m_pgl3dCtrls->resetClipPlane();
}


void POpp3dCtrls::checkPanels(bool b)
{
    m_pgl3dCtrls->checkPanels(b);
}


