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

#include "poppgraphctrls.h"

#include <interfaces/graphs/controls/graphtiles.h>
#include <globals/mainframe.h>
#include <modules/xplane/xplane.h>
#include <api/objects3d.h>
#include <api/planexfl.h>
#include <api/planepolar.h>
#include <api/planeopp.h>
#include <interfaces/widgets/customwts/cptableview.h>

POppGraphCtrls::POppGraphCtrls(GraphTiles *pParent) : GraphTileCtrls(pParent)
{
    m_bAnimateWOpp = false;
    m_bAnimateWOppPlus = true;
    m_posAnimateWOpp = 0;
}


POppGraphCtrls::~POppGraphCtrls()
{
}


void POppGraphCtrls::checkGraphActions()
{
    for(int ig=0; ig<5; ig++)    m_prbGraph[ig]->setChecked(false);
    m_prbTwoGraphs->setChecked(false);
    m_prbFourGraphs->setChecked(false);
    m_prbAllGraphs->setChecked(false);

    if     (m_pGraphTileWt->isOneGraph())   m_prbGraph[m_pGraphTileWt->activeGraphIndex()]->setChecked(true);
    else if(m_pGraphTileWt->isTwoGraphs())  m_prbTwoGraphs->setChecked(true);
    else if(m_pGraphTileWt->isFourGraphs()) m_prbFourGraphs->setChecked(true);
    else if(m_pGraphTileWt->isAllGraphs())  m_prbAllGraphs->setChecked(true);
}


void POppGraphCtrls::stopAnimate()
{
    if(!m_bAnimateWOpp) return;
    m_bAnimateWOpp = false;
    m_pchWOppAnimate->setChecked(false);
    m_TimerWOpp.stop();
}


void POppGraphCtrls::onAnimateWOpp(bool bAnimate)
{
    if(!bAnimate)
    {
        s_pXPlane->stopAnimate();
        return;
    }

    if(!s_pXPlane->m_pCurPlane || !s_pXPlane->m_pCurPlPolar || !s_pXPlane->isPOppView())
    {
        m_bAnimateWOpp = false;
        return;
    }

    m_posAnimateWOpp = 0;
    if(s_pXPlane->m_pCurPOpp)
    {
        for (int l=0; l<Objects3d::nPOpps(); l++)
        {
            PlaneOpp const*pPOpp = Objects3d::POppAt(l);

            if (pPOpp &&
                pPOpp->polarName() == s_pXPlane->m_pCurPlPolar->name() &&
                pPOpp->planeName() == s_pXPlane->m_pCurPlane->name())
            {
                if(fabs(s_pXPlane->m_pCurPOpp->alpha() - pPOpp->alpha())<AOAPRECISION)
                {
                    m_posAnimateWOpp = l;
                    break;
                }
            }
        }
    }

    m_bAnimateWOpp  = true;
    int speed = m_pslAnimateWOppSpeed->value();
    m_TimerWOpp.setInterval(800-speed);
    m_TimerWOpp.start();
}


/**
* A signal has been received from the timer to update the WOPP display
* So displays the next WOpp in the sequence.
*/
void POppGraphCtrls::onAnimateWOppSingle()
{
    bool bIsValid=false, bSkipOne=false;
    int size=0;

    if(!s_pXPlane->isPOppView()) return; //nothing to animate
    if(!s_pXPlane->m_pCurPlane || !s_pXPlane->m_pCurPlPolar) return;

    size = Objects3d::nPOpps();
    if(size<=1) return;

    bIsValid = false;
    bSkipOne = false;

    while(!bIsValid)
    {
        PlaneOpp *pPOpp = nullptr;
        //Find the current position to display


        pPOpp = Objects3d::POppAt(m_posAnimateWOpp);
        if(!pPOpp) return;

        bIsValid =(pPOpp->polarName()==s_pXPlane->m_pCurPlPolar->name()  &&  pPOpp->planeName()==s_pXPlane->m_pCurPlane->name());

        if (bIsValid && !bSkipOne)
        {
            s_pXPlane->m_pCurPOpp = pPOpp;
            s_pXPlane->m_bCurPOppOnly = true;

            s_pXPlane->m_bResetCurves = true;
            s_pXPlane->updateView();
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


void POppGraphCtrls::onAnimateWOppSpeed(int val)
{
    m_TimerWOpp.setInterval(800-val);
}


void POppGraphCtrls::setupLayout()
{
    QGridLayout *pMainLayout = new QGridLayout;
    {
        m_pchWOppAnimate       = new QCheckBox(tr("Animate"));

        m_pslAnimateWOppSpeed  = new QSlider(Qt::Horizontal);
        m_pslAnimateWOppSpeed->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        m_pslAnimateWOppSpeed->setMinimum(0);
        m_pslAnimateWOppSpeed->setMaximum(500);
        m_pslAnimateWOppSpeed->setSliderPosition(250);
        m_pslAnimateWOppSpeed->setTickInterval(50);
        m_pslAnimateWOppSpeed->setTickPosition(QSlider::TicksBelow);

        m_pchShowActiveOppOnly = new QCheckBox(tr("Active operating point only"));

        pMainLayout->addWidget(m_pcptVariableSet,       1,1,1,2);
        pMainLayout->addWidget(m_prbGraph[0],           2,1);
        pMainLayout->addWidget(m_prbGraph[1],           2,2);
        pMainLayout->addWidget(m_prbGraph[2],           3,1);
        pMainLayout->addWidget(m_prbGraph[3],           3,2);
        pMainLayout->addWidget(m_prbGraph[4],           4,1);
        pMainLayout->addWidget(m_prbTwoGraphs,          4,2);
        pMainLayout->addWidget(m_prbFourGraphs,         5,1);
        pMainLayout->addWidget(m_prbAllGraphs,          5,2);
        pMainLayout->addWidget(m_pchShowActiveOppOnly,  7,1,1,2);
        pMainLayout->addWidget(m_pchWOppAnimate,        8,1);
        pMainLayout->addWidget(m_pslAnimateWOppSpeed,   8,2);
        pMainLayout->setRowStretch(8,3);
    }
    setLayout(pMainLayout);
}


void POppGraphCtrls::connectSignals()
{
    connect(m_pchWOppAnimate,       SIGNAL(clicked(bool)),    SLOT(onAnimateWOpp(bool)));
    connect(m_pslAnimateWOppSpeed,  SIGNAL(sliderMoved(int)), SLOT(onAnimateWOppSpeed(int)));
    connect(&m_TimerWOpp,           SIGNAL(timeout()),        SLOT(onAnimateWOppSingle()));

    connect(m_pchShowActiveOppOnly,  SIGNAL(clicked()), s_pXPlane, SLOT(onCurPOppOnly()));
    GraphTileCtrls::connectSignals();
}


void POppGraphCtrls::setControls()
{
    GraphTileCtrls::setControls();

    m_pchShowActiveOppOnly->setChecked(s_pXPlane->m_bCurPOppOnly);
}



