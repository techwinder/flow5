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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <QLabel>

#include "xsaildisplayctrls.h"

#include <api/boat.h>
#include <api/boatopp.h>
#include <api/boatpolar.h>
#include <api/sailobjects.h>
#include <api/units.h>
#include <api/utils.h>
#include <core/xflcore.h>
#include <interfaces/controls/poppctrls/opp3dscalesctrls.h>
#include <interfaces/opengl/controls/fine3dcontrols.h>
#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <modules/xsail/controls/xsaildisplayctrls.h>
#include <modules/xsail/view/gl3dxsailview.h>
#include <modules/xsail/xsail.h>


XSail *XSailDisplayCtrls::s_pXSail = nullptr;
bool XSailDisplayCtrls::s_bPartForces  = false;

bool XSailDisplayCtrls::s_bMoments     = false;

bool XSailDisplayCtrls::s_bStreamLines = false;
bool XSailDisplayCtrls::s_bFlow        = false;
bool XSailDisplayCtrls::s_bWind        = false;
bool XSailDisplayCtrls::s_bWater       = false;
bool XSailDisplayCtrls::s_bVortons     = false;
bool XSailDisplayCtrls::s_bWakePanels  = false;
bool XSailDisplayCtrls::s_bGamma       = false;
bool XSailDisplayCtrls::s_bPanelForce  = false;
bool XSailDisplayCtrls::s_b3dCp        = false;


XSailDisplayCtrls::XSailDisplayCtrls(gl3dXSailView *pgl3dBoatView, Qt::Orientation orientation, bool bRowLayout)
    : gl3dControls(pgl3dBoatView)
{
    m_pgl3dXSailView = pgl3dBoatView;
    m_pgl3dXSailView->m_pDisplayCtrls = this;


    m_bAnimateWOpp     = false;
    m_bAnimateWOppPlus = true;
    m_posAnimateWOpp   = 0;
    m_pTimerWOpp= new QTimer(this);


    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    if(orientation==Qt::Horizontal) bRowLayout=true;
    setupLayout(orientation, bRowLayout);
    connectSignals();
}


void XSailDisplayCtrls::connectSignals()
{
    connectBaseSignals();

    connect(m_pchAxes,         SIGNAL(clicked(bool)), m_pgl3dView, SLOT(onAxes(bool)));
    connect(m_pchSurfaces,     SIGNAL(clicked(bool)), m_pgl3dXSailView, SLOT(onSurfaces(bool)));
    connect(m_pchOutline,      SIGNAL(clicked(bool)), m_pgl3dXSailView, SLOT(onOutline(bool)));
    connect(m_pchPanels,       SIGNAL(clicked(bool)), m_pgl3dXSailView, SLOT(onPanels(bool)));

    connect(m_ptbWindBack,     SIGNAL(clicked(bool)), m_pgl3dXSailView, SLOT(onWindBack()));
    connect(m_ptbWindFront,    SIGNAL(clicked(bool)), m_pgl3dXSailView, SLOT(onWindFront()));

    connect(m_pchCp,           SIGNAL(clicked()), SLOT(on3dCp()));
    connect(m_pchPanelForce,   SIGNAL(clicked()), SLOT(onPanelForce()));
    connect(m_pchGamma,        SIGNAL(clicked()), SLOT(onGamma()));

    connect(m_pchPartForces,   SIGNAL(clicked()), SLOT(onShowLift()));
    connect(m_pchPickPanel,    SIGNAL(clicked()), SLOT(on3dPickCp()));
    connect(m_pchMoment,       SIGNAL(clicked()), SLOT(onMoment()));
    connect(m_pchStream,       SIGNAL(clicked()), SLOT(onStreamlines()));
    connect(m_pchFlow,         SIGNAL(clicked()), SLOT(onFlow()));
    connect(m_pchWind,         SIGNAL(clicked()), SLOT(onWind()));
    connect(m_pchWater,        SIGNAL(clicked()), SLOT(onWater()));
    connect(m_pchWakePanels,   SIGNAL(clicked()), SLOT(onWakePanels()));
    connect(m_pchVortons,      SIGNAL(clicked()), SLOT(onVortons()));

    connect(m_pchWOppAnimate,        SIGNAL(clicked()),        SLOT(onAnimateWOpp()));
    connect(m_pslAnimateWOppSpeed, SIGNAL(sliderMoved(int)), SLOT(onAnimateWOppSpeed(int)));

    connect(m_pTimerWOpp,    SIGNAL(timeout()), SLOT(onAnimateWOppSingle()));

    connect(m_pgl3dView,     SIGNAL(viewModified()), m_pFineCtrls, SLOT(onResetCtrls()));
}


void XSailDisplayCtrls::setupLayout(Qt::Orientation , bool )
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    QGridLayout *pCheckDispLayout = new QGridLayout;
    {
        m_pchPanelForce = new QCheckBox("F/s=q.Cp");
        m_pchPanelForce->setToolTip("Display the force 1/2.rho.V2.S.Cp acting on the panel");
        m_pchPartForces     = new QCheckBox("Part forces");
        m_pchMoment         = new QCheckBox("Moments");
        m_pchCp             = new QCheckBox("Cp");
        m_pchGamma          = new QCheckBox(GAMMAch);
        QString gammatip("<p>Displays the distribution of doublet densities or vortex strengths.<br>"
                         "This is the main result of the potential flow calculation, and <u>all other results</u> "
                         "are derived from this distribution. It is therefore important that it "
                         "does not exhibit numerical issues which would show up as singularities i.e. areas with peak values."
                         "</p>");
        m_pchGamma->setToolTip(gammatip);
        m_pchStream         = new QCheckBox("Streamlines");
        m_pchFlow           = new QCheckBox("Flow");
        m_pchFlow->setToolTip("<p>Launches an animation of the flow around the model.<br>"
                              "The parameters are set in the last tab of the scales widget.</p>");
        m_pchWakePanels     = new QCheckBox("Wake panels");
        m_pchVortons        = new QCheckBox("Vortons");
        m_pchWind           = new QCheckBox("Wind");
        m_pchWater          = new QCheckBox("Water");
        m_pchPickPanel      = new QCheckBox("Pick value");
        m_pchPickPanel->setShortcut(QKeySequence(Qt::SHIFT|Qt::Key_H));
        m_pchPickPanel->setToolTip("<p>Activate this checkbox and move the mouse\nover the Cp or Forces color plot. (Shift+H)</p>");

        m_pchWOppAnimate    = new QCheckBox("Animate");

        m_pslAnimateWOppSpeed  = new QSlider(Qt::Horizontal);
        m_pslAnimateWOppSpeed->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        m_pslAnimateWOppSpeed->setMinimum(0);
        m_pslAnimateWOppSpeed->setMaximum(500);
        m_pslAnimateWOppSpeed->setSliderPosition(250);
        m_pslAnimateWOppSpeed->setTickInterval(50);
        m_pslAnimateWOppSpeed->setTickPosition(QSlider::TicksBelow);
        pCheckDispLayout->addWidget(m_pchCp,         1, 1);
        pCheckDispLayout->addWidget(m_pchPanelForce, 1, 2);
        pCheckDispLayout->addWidget(m_pchPartForces, 2, 1);
        pCheckDispLayout->addWidget(m_pchGamma,      2, 2);
        pCheckDispLayout->addWidget(m_pchMoment,     3, 1);
        pCheckDispLayout->addWidget(m_pchFlow,       4, 1);
        pCheckDispLayout->addWidget(m_pchStream,     4, 2);
        pCheckDispLayout->addWidget(m_pchWakePanels, 5, 1);
        pCheckDispLayout->addWidget(m_pchVortons,    5, 2);
        pCheckDispLayout->addWidget(m_pchWind,       6, 1);
        pCheckDispLayout->addWidget(m_pchWater,      6, 2);
        pCheckDispLayout->addWidget(m_pchWOppAnimate, 7, 1);
        pCheckDispLayout->addWidget(m_pslAnimateWOppSpeed, 7,2);
        pCheckDispLayout->addWidget(m_pchPickPanel,  8,1);

        pCheckDispLayout->setRowStretch(7,5);

//        pCheckDispLayout->setContentsMargins(0,0,0,0);
        pCheckDispLayout->setSpacing(0);
    }

    QGridLayout *pThreeDParamsLayout = new QGridLayout;
    {
        m_pchAxes     = new QCheckBox("Axes",     this);
        m_pchSurfaces = new QCheckBox("Surfaces", this);
        m_pchOutline  = new QCheckBox("Outline",  this);
        m_pchPanels   = new QCheckBox("Panels",   this);
        m_pchSurfaces->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        m_pchOutline->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum);
        m_pchPanels->setSizePolicy(  QSizePolicy::Maximum, QSizePolicy::Maximum);
        m_pchAxes->setSizePolicy(    QSizePolicy::Maximum, QSizePolicy::Maximum);

        pThreeDParamsLayout->addWidget(m_pchAxes,     1,1);
        pThreeDParamsLayout->addWidget(m_pchPanels,   1,2);
        pThreeDParamsLayout->addWidget(m_pchSurfaces, 2,1);
        pThreeDParamsLayout->addWidget(m_pchOutline,  2,2);

        pThreeDParamsLayout->setContentsMargins(0,0,0,0);
        pThreeDParamsLayout->setSpacing(0);
    }

    QVBoxLayout *pAxisViewLayout = new QVBoxLayout;
    {
        QAction *pBack = new QAction("Back", this);
        m_ptbWindBack = new QToolButton();
        m_ptbWindBack->setDefaultAction(pBack);
        m_ptbWindBack->setToolTip("B");

        QAction *pFront = new QAction("Front", this);
        m_ptbWindFront = new QToolButton();
        m_ptbWindFront->setDefaultAction(pFront);
        m_ptbWindFront->setToolTip("F");

        QHBoxLayout *pTopRowLayout = new QHBoxLayout;
        {
            pTopRowLayout->addStretch();
            pTopRowLayout->addWidget(m_ptbWindBack);
            pTopRowLayout->addWidget(m_ptbX);
            pTopRowLayout->addWidget(m_ptbY);
            pTopRowLayout->addWidget(m_ptbZ);
            pTopRowLayout->addWidget(m_ptbIso);
            pTopRowLayout->addWidget(m_ptbWindFront);
            pTopRowLayout->addStretch();
        }
        QHBoxLayout *pBotRowLayout = new QHBoxLayout;
        {
            pBotRowLayout->addStretch();
            pBotRowLayout->addWidget(m_ptbHFlip);
            pBotRowLayout->addWidget(m_ptbVFlip);
            pBotRowLayout->addWidget(m_ptbReset);
            pBotRowLayout->addWidget(m_ptbDistance);
            m_ptbDistance->setEnabled(false);
            pBotRowLayout->addWidget(m_ptbFineCtrls);
            pBotRowLayout->addStretch();
        }

        pAxisViewLayout->addLayout(pTopRowLayout);
        pAxisViewLayout->addLayout(pBotRowLayout);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pCheckDispLayout);
        pMainLayout->addLayout(pThreeDParamsLayout);
        pMainLayout->addStretch();
        pMainLayout->addLayout(pAxisViewLayout);
        pMainLayout->addWidget(m_pFineCtrls);
    }
    setLayout(pMainLayout);
}


void XSailDisplayCtrls::showEvent(QShowEvent *pEvent)
{
    gl3dControls::showEvent(pEvent);
    setControls();
}


void XSailDisplayCtrls::setControls()
{
    gl3dControls::setControls();
    m_pchAxes->setChecked(    m_pgl3dXSailView->bAxes());
    m_pchSurfaces->setChecked(m_pgl3dXSailView->bSurfaces());
    m_pchOutline->setChecked( m_pgl3dXSailView->bOutline());
    m_pchPanels->setChecked(  m_pgl3dXSailView->bVLMPanels());

    m_pchPartForces->setEnabled(s_pXSail->curBtOpp());
    m_pchMoment->setEnabled(    s_pXSail->curBtOpp());
    m_pchPanelForce->setEnabled(s_pXSail->curBtOpp());
    m_pchCp->setEnabled(        s_pXSail->curBtOpp());
    m_pchGamma->setEnabled(     s_pXSail->curBtOpp());
    m_pchStream->setEnabled(    s_pXSail->curBtOpp());
#ifdef Q_OS_MAC
    m_pchFlow->setEnabled(false);
#else
    m_pchFlow->setEnabled(      s_pXSail->curBtOpp() && s_pXSail->curBtOpp()->isTriUniformMethod());
#endif
    m_pchWind->setEnabled(      s_pXSail->curBtOpp());
    m_pchWakePanels->setEnabled(s_pXSail->curBtOpp());
    m_pchVortons->setEnabled(   s_pXSail->curBtOpp());

    m_pchCp->setChecked(s_b3dCp);
    m_pchGamma->setChecked(s_bGamma);
    m_pchPanelForce->setChecked(s_bPanelForce);
    m_pchMoment->setChecked(s_bMoments);
    m_pchPartForces->setChecked(s_bPartForces);
    m_pchStream->setChecked(s_bStreamLines);
    m_pchFlow->setChecked(s_bFlow);
    m_pchWind->setChecked(s_bWind);
    m_pchWakePanels->setChecked(s_bWakePanels);
    m_pchVortons->setChecked(s_bVortons);
    m_pchWater->setChecked(s_bWater);

    m_pchPickPanel->setEnabled(s_pXSail->curBtPolar());

    m_pchWOppAnimate->setEnabled(s_pXSail->curBtPolar());
    m_pslAnimateWOppSpeed->setEnabled(s_pXSail->curBtPolar() && m_pchWOppAnimate->isChecked());

    BoatOpp const *pBtOpp = s_pXSail->curBtOpp();
    if(!pBtOpp)
        m_pgl3dXSailView->m_LegendOverlay.setVisible(false);
    else
    {
        m_pgl3dXSailView->m_LegendOverlay.setVisible(s_b3dCp || s_bGamma || s_bPanelForce);
    }
}


void XSailDisplayCtrls::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dXSailCtrls");
    {
        s_bPartForces   = settings.value("bLiftStrip",  false).toBool();
        s_bPanelForce   = settings.value("bPanelForce", false).toBool();
        s_b3dCp         = settings.value("b3DCp",       false).toBool();
        s_bMoments      = settings.value("bMoments",    false).toBool();
        s_bWater        = settings.value("bWater",      false).toBool();
        s_bWind         = settings.value("bWind",       false).toBool();
        s_bWakePanels   = settings.value("bWakePanels", false).toBool();
        s_bVortons      = settings.value("bVortons",    false).toBool();
    }
    settings.endGroup();
}


void XSailDisplayCtrls::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dXSailCtrls");
    {
        settings.setValue("bLiftStrip",  s_bPartForces);
        settings.setValue("bPanelForce", s_bPanelForce);
        settings.setValue("b3DCp",       s_b3dCp);
        settings.setValue("bMoments",    s_bMoments);
        settings.setValue("bWater",      s_bWater);
        settings.setValue("bWind",       s_bWind);
        settings.setValue("bWakePanels", s_bWakePanels);
        settings.setValue("bVortons",    s_bVortons);
    }
    settings.endGroup();
}


/**
 * Updates the display after the user has toggled the switch for the display of Cp coefficients
 */
void XSailDisplayCtrls::on3dCp()
{
    s_b3dCp = m_pchCp->isChecked();

    if(s_b3dCp)
    {
        m_pgl3dXSailView->s_bResetglPanelCp = true;
        m_pgl3dXSailView->m_bSurfaces = false;

        m_pchSurfaces->setChecked(false);

        s_bGamma = false;
        m_pchGamma->setChecked(false);

        s_bPanelForce = false;
        m_pchPanelForce->setChecked(false);
    }

    m_pgl3dXSailView->m_LegendOverlay.setTitle("Cp");
    m_pgl3dXSailView->m_LegendOverlay.setVisible(s_b3dCp);
    m_pgl3dXSailView->m_LegendOverlay.makeLegend();
    m_pgl3dXSailView->update();
}


/**
 * Updates the display after the user has toggled the switch for the display of Cp coefficients
 */
void XSailDisplayCtrls::onGamma()
{
    s_bGamma = m_pchGamma->isChecked();

    if(s_bGamma)
    {
        m_pgl3dXSailView->s_bResetglPanelGamma = true;
        m_pgl3dXSailView->m_bSurfaces = false;

        m_pchSurfaces->setChecked(false);

        s_bPanelForce = false;
        m_pchPanelForce->setChecked(false);

        s_b3dCp = false;
        m_pchCp->setChecked(false);
    }

    m_pgl3dXSailView->m_LegendOverlay.setTitle("Gamma");
    m_pgl3dXSailView->m_LegendOverlay.setVisible(s_bGamma);
    m_pgl3dXSailView->m_LegendOverlay.makeLegend();
    m_pgl3dXSailView->update();
}


/**
 * The user has toggled the display of the forces acting on the panels in the 3D view
 */
void XSailDisplayCtrls::onPanelForce()
{
    s_bPanelForce = m_pchPanelForce->isChecked();
    if(s_bPanelForce)
    {
        s_b3dCp =false;
        m_pchCp->setChecked(false);

        s_bGamma = false;
        m_pchGamma->setChecked(false);
    }
    if(!m_bAnimateWOpp) m_pgl3dXSailView->update();

    m_pgl3dXSailView->m_LegendOverlay.setTitle("("+Units::pressureUnitQLabel()+")");
    m_pgl3dXSailView->m_LegendOverlay.setVisible(s_bPanelForce);
    m_pgl3dXSailView->m_LegendOverlay.makeLegend();
    m_pgl3dXSailView->update();
}


void XSailDisplayCtrls::on3dPickCp()
{
    if(m_pchPickPanel->isChecked())
        m_pgl3dXSailView->setPicking(xfl::PANEL3); /** @todo node in case of trilinear analysis*/
    else
        m_pgl3dXSailView->stopPicking();

    m_pgl3dXSailView->update();
}


void XSailDisplayCtrls::onShowLift()
{
    s_bPartForces     = m_pchPartForces->isChecked();
    if(!m_bAnimateWOpp) m_pgl3dXSailView->update();
}


void XSailDisplayCtrls::onMoment()
{
    s_bMoments = m_pchMoment->isChecked();
    m_pgl3dXSailView->update();
}


void XSailDisplayCtrls::onStreamlines()
{
    s_bStreamLines = m_pchStream->isChecked();
    if(s_bStreamLines) gl3dXSailView::resetglStreamLines();
    m_pgl3dXSailView->update();
}


void XSailDisplayCtrls::onFlow()
{
    s_bFlow = m_pchFlow->isChecked();

    //update the SSBO and VBO
    m_pgl3dXSailView->resetFlow();

    m_pgl3dXSailView->startFlow(s_bFlow);
}


void XSailDisplayCtrls::onWakePanels()
{
    s_bWakePanels = m_pchWakePanels->isChecked();
    m_pgl3dXSailView->update();
    m_pgl3dXSailView->setFocus();
}


void XSailDisplayCtrls::onVortons()
{
    s_bVortons = m_pchVortons->isChecked();
    m_pgl3dXSailView->update();
    m_pgl3dXSailView->setFocus();
}


void XSailDisplayCtrls::onWind()
{
    s_bWind= m_pchWind->isChecked();
    m_pgl3dXSailView->update();
    m_pgl3dXSailView->setFocus();
}


void XSailDisplayCtrls::onWater()
{
    s_bWater= m_pchWater->isChecked();
    m_pgl3dXSailView->update();
    m_pgl3dXSailView->setFocus();
}


void XSailDisplayCtrls::stopAnimate()
{
    if(!m_bAnimateWOpp) return;
    m_bAnimateWOpp = false;
    m_pchWOppAnimate->setChecked(false);
    m_pTimerWOpp->stop();
}


void XSailDisplayCtrls::cancelStream()
{
    s_bStreamLines = false;
    m_pchStream->setChecked(false);
}


/**
 * Launches the animation of the WOpp display
 * Will display all the available WOpps for this WPolar in sequence
*/
void XSailDisplayCtrls::onAnimateWOpp()
{
    if(!s_pXSail->curBoat() || !s_pXSail->curBtPolar() || !s_pXSail->is3dView())
    {
        m_bAnimateWOpp = false;
        return;
    }

    BoatOpp*pBtOpp;

    if(!m_pchWOppAnimate->isChecked())
    {
        stopAnimate();
        m_pslAnimateWOppSpeed->setEnabled(false);
        return;
    }
    m_pslAnimateWOppSpeed->setEnabled(true);

    for (int l=0; l<SailObjects::nBtOpps(); l++)
    {
        pBtOpp = SailObjects::btOpp(l);

        if (pBtOpp &&
            pBtOpp->polarName() == s_pXSail->curBtPolar()->name() &&
            pBtOpp->boatName() == s_pXSail->curBoat()->name())
        {
                if(s_pXSail->curBtOpp()->ctrl() - pBtOpp->ctrl()<0.0001)
                    m_posAnimateWOpp = l;
        }
    }


    m_bAnimateWOpp  = true;
    int speed = m_pslAnimateWOppSpeed->value();
    m_pTimerWOpp->setInterval(800-speed);
    m_pTimerWOpp->start();
}


/**
* A signal has been received from the timer to update the WOPP display
* So displays the next WOpp in the sequence.
*/
void XSailDisplayCtrls::onAnimateWOppSingle()
{
    bool bIsValid, bSkipOne;

    BoatOpp *pBtOpp;

    if(!s_pXSail->is3dView()) return; //nothing to animate
    if(!s_pXSail->curBoat()|| !s_pXSail->curBtPolar()) return;

    int size = SailObjects::nBtOpps();
    if(size<=1) return;

    bIsValid = false;
    bSkipOne = false;

    while(!bIsValid)
    {
        pBtOpp = nullptr;
        //Find the current position to display

        pBtOpp = SailObjects::btOpp(m_posAnimateWOpp);
        if(!pBtOpp) return;

        bIsValid =(pBtOpp->polarName()==s_pXSail->curBtPolar()->name()  &&  pBtOpp->boatName()==s_pXSail->curBoat()->name());

        if (bIsValid && !bSkipOne)
        {
            s_pXSail->m_pCurBtOpp = pBtOpp;

            m_pgl3dXSailView->resetglBtOpp();
            m_pgl3dXSailView->s_bResetglLift     = true;
            m_pgl3dXSailView->s_bResetglMoments  = true;
            m_pgl3dXSailView->s_bResetglDrag     = true;
            m_pgl3dXSailView->s_bResetglWake     = true;
            m_pgl3dXSailView->s_bResetglStream   = true;

            m_pgl3dXSailView->update();


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


/**
* Modfies the animation after the user has changed the animation speed for the WOpp display
*/
void XSailDisplayCtrls::onAnimateWOppSpeed(int val)
{
    if(m_pTimerWOpp->isActive())
    {
        m_pTimerWOpp->setInterval(800-val);
    }
}
