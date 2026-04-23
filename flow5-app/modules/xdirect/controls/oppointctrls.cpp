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





#include <QGridLayout>
#include <QTimer>

#include "oppointctrls.h"

#include <modules/xdirect/view2d/oppointwt.h>
#include <modules/xdirect/xdirect.h>

#include <interfaces/graphs/graph/graph.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <api/oppoint.h>
#include <api/polar.h>
#include <interfaces/widgets/line/linemenu.h>


XDirect *OpPointCtrls::s_pXDirect = nullptr;

OpPointCtrls::OpPointCtrls(QWidget *pParent) : QWidget(pParent)
{
    m_pOpPointWt = nullptr;
    m_posAnimate = 0; // no animation to start with
    m_bAnimate        = false;
    m_bAnimatePlus    = false;

    m_pAnimateTimer = new QTimer(this);

    setupLayout();
    connectSignals();
}


void OpPointCtrls::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QHBoxLayout *pGraphBoxLayout = new QHBoxLayout;
        {
            m_prbCpCurve = new QRadioButton(tr("Cp"));
            m_prbQCurve  = new QRadioButton(tr("Edge velocity"));
            pGraphBoxLayout->addStretch();
            pGraphBoxLayout->addWidget(m_prbCpCurve);
            pGraphBoxLayout->addStretch();
            pGraphBoxLayout->addWidget(m_prbQCurve);
            pGraphBoxLayout->addStretch();
        }

        QGridLayout *pOppDisplayLayout = new QGridLayout;
        {
            m_pchFillFoil      = new QCheckBox(tr("Fill foil"));
            m_pchNeutralLine   = new QCheckBox(tr("Neutral line"));
            m_pchShowBL        = new QCheckBox(tr("BL (d*)"));
            m_pchShowBL->setToolTip(tr("<p>Displacement thickness</p>"));
            m_pchShowPressure  = new QCheckBox(tr("Pressure"));
            m_plbNeutralStyle  = new LineBtn;
            m_plbBLStyle       = new LineBtn;
            m_plbPressureStyle = new LineBtn;
            m_pchShowInviscid  = new QCheckBox(tr("Show inviscid"));
            m_pchAnimate       = new QCheckBox(tr("Animate"));
            m_pslAnimateSpeed  = new QSlider(Qt::Horizontal);
            m_pslAnimateSpeed->setMinimum(0);
            m_pslAnimateSpeed->setMaximum(1000);
            m_pslAnimateSpeed->setSliderPosition(500);
            m_pslAnimateSpeed->setTickInterval(50);
            m_pslAnimateSpeed->setTickPosition(QSlider::TicksBelow);

            m_pchShowActiveOppOnly = new QCheckBox(tr("Active operating point only"));

            pOppDisplayLayout->addWidget(m_pchFillFoil,          1,1);
            pOppDisplayLayout->addWidget(m_pchNeutralLine,       2,1);
            pOppDisplayLayout->addWidget(m_plbNeutralStyle,      2,2);
            pOppDisplayLayout->addWidget(m_pchShowBL,            3,1);
            pOppDisplayLayout->addWidget(m_plbBLStyle,           3,2);
            pOppDisplayLayout->addWidget(m_pchShowPressure,      4,1);
            pOppDisplayLayout->addWidget(m_plbPressureStyle,     4,2);
            pOppDisplayLayout->addWidget(m_pchShowInviscid,      5,1,1,2);
            pOppDisplayLayout->addWidget(m_pchAnimate,           7,1,1,2);
            pOppDisplayLayout->addWidget(m_pslAnimateSpeed,      8,1,1,2);
            pOppDisplayLayout->addWidget(m_pchShowActiveOppOnly, 9,1,1,2);
            pOppDisplayLayout->setRowStretch(7,5);
        }
        pMainLayout->addLayout(pGraphBoxLayout);
        pMainLayout->addLayout(pOppDisplayLayout);
        pMainLayout->addStretch();
    }
    setLayout(pMainLayout);
}


void OpPointCtrls::setOpPointWidget(OpPointWt *pOpPointWt)
{
    m_pOpPointWt = pOpPointWt;
    setControls();
}


void OpPointCtrls::connectSignals()
{
    connect(m_prbCpCurve,           SIGNAL(clicked(bool)), s_pXDirect, SLOT(onCpGraph()));
    connect(m_prbQCurve,            SIGNAL(clicked(bool)), s_pXDirect, SLOT(onOppGraph()));
    connect(m_pchShowActiveOppOnly, SIGNAL(clicked(bool)), s_pXDirect, SLOT(onCurOppOnly(bool)));
    connect(m_pchShowInviscid,      SIGNAL(clicked(bool)), s_pXDirect, SLOT(onCpi(bool)));

    connect(m_pAnimateTimer,        SIGNAL(timeout()),            SLOT(onAnimateSingle()));
    connect(m_pchAnimate,           SIGNAL(clicked(bool)),        SLOT(onAnimate(bool)));
    connect(m_pslAnimateSpeed,      SIGNAL(sliderMoved(int)),     SLOT(onAnimateSpeed(int)));

    connect(m_pchFillFoil,          SIGNAL(clicked(bool)),        SLOT(onFillFoil(bool)));
    connect(m_pchNeutralLine,       SIGNAL(clicked(bool)),        SLOT(onNeutralLine(bool)));
    connect(m_pchShowBL,            SIGNAL(clicked(bool)),        SLOT(onShowBL(bool)));
    connect(m_pchShowPressure,      SIGNAL(clicked(bool)),        SLOT(onShowPressure(bool)));
    connect(m_plbNeutralStyle,      SIGNAL(clickedLB(LineStyle)), SLOT(onNeutralLineStyle(LineStyle)));
    connect(m_plbBLStyle,           SIGNAL(clickedLB(LineStyle)), SLOT(onBLCurveStyle(LineStyle)));
    connect(m_plbPressureStyle,     SIGNAL(clickedLB(LineStyle)), SLOT(onPressureCurveStyle(LineStyle)));

}


void OpPointCtrls::onShowPressure(bool bPressure)
{
    m_pOpPointWt->showPressure(bPressure);
    m_pOpPointWt->update();
    m_plbPressureStyle->setEnabled(bPressure);
}


void OpPointCtrls::onShowBL(bool bBL)
{
    m_pOpPointWt->showBL(bBL);
    m_pOpPointWt->update();
    m_plbBLStyle->setEnabled(bBL);
}


void OpPointCtrls::onFillFoil(bool bFill)
{
    m_pOpPointWt->setFillFoil(bFill);
    m_pOpPointWt->update();
}


void OpPointCtrls::onNeutralLine(bool bShow)
{
    m_pOpPointWt->showNeutralLine(bShow);
    m_pOpPointWt->update();
}


void OpPointCtrls::setControls()
{
    if(!s_pXDirect) return;
    m_prbQCurve->setChecked(s_pXDirect->CpGraph()->yVariable(0)==1);
    m_prbCpCurve->setChecked(s_pXDirect->CpGraph()->yVariable(0)==0);
    m_prbCpCurve->setEnabled(s_pXDirect->curOpp());
    m_prbQCurve->setEnabled(s_pXDirect->curOpp());

    m_pchShowInviscid->setEnabled(s_pXDirect->curOpp());
    m_pchShowInviscid->setChecked(s_pXDirect->bShowInviscid());

    m_pchFillFoil->setChecked(OpPointWt::bFillFoil());
    m_pchNeutralLine->setChecked(OpPointWt::neutralStyle().m_bIsVisible);
    m_pchShowPressure->setEnabled(!s_pXDirect->isPolarView() && XDirect::curOpp());
    m_pchShowBL->setEnabled(!s_pXDirect->isPolarView() && XDirect::curOpp());
    m_pchAnimate->setEnabled(!s_pXDirect->isPolarView() && XDirect::curOpp());
    m_pslAnimateSpeed->setEnabled(!s_pXDirect->isPolarView() && XDirect::curOpp() && m_pchAnimate->isChecked());

    m_plbNeutralStyle->setTheStyle(OpPointWt::neutralStyle());
    m_plbBLStyle->setTheStyle(OpPointWt::BLStyle());
    m_plbPressureStyle->setTheStyle(OpPointWt::pressureStyle());

    m_pchShowActiveOppOnly->setChecked(s_pXDirect->curOppOnly());
    m_pchShowActiveOppOnly->setEnabled(!s_pXDirect->isPolarView());
}


void OpPointCtrls::onNeutralLineStyle(LineStyle)
{
    LineMenu *pLineMenu = new LineMenu(nullptr, false);
    pLineMenu->initMenu(OpPointWt::neutralStyle());
    pLineMenu->exec(QCursor::pos());
    LineStyle ls = pLineMenu->theStyle();
    m_plbNeutralStyle->setTheStyle(ls);
    m_pOpPointWt->setNeutralLineStyle(ls);
    s_pXDirect->updateView();
}


void OpPointCtrls::onBLCurveStyle(LineStyle)
{
    LineMenu *m_pLineMenu = new LineMenu(nullptr, false);
    m_pLineMenu->initMenu(OpPointWt::BLStyle());
    m_pLineMenu->exec(QCursor::pos());
    LineStyle ls = m_pLineMenu->theStyle();
    m_plbBLStyle->setTheStyle(ls);
    m_pOpPointWt->setBLStyle(ls);
    s_pXDirect->updateView();
}


void OpPointCtrls::onPressureCurveStyle(LineStyle)
{
    LineMenu *m_pLineMenu = new LineMenu(nullptr);
    m_pLineMenu->initMenu(OpPointWt::pressureStyle());
    m_pLineMenu->exec(QCursor::pos());
    LineStyle ls = m_pLineMenu->theStyle();
    m_plbPressureStyle->setTheStyle(ls);
    m_pOpPointWt->setPressureStyle(ls);
    s_pXDirect->updateView();
}


void OpPointCtrls::onShowActiveOppOnly()
{
    if(!s_pXDirect) return;
}


void OpPointCtrls::onAnimateSpeed(int val)
{
    if(m_pAnimateTimer->isActive())
    {
        m_pAnimateTimer->setInterval(1000-val);
    }
}


void OpPointCtrls::onAnimate(bool bChecked)
{
    m_pslAnimateSpeed->setEnabled(bChecked);

    if(!XDirect::curFoil() || !XDirect::curPolar())
    {
        m_bAnimate = false;
        return;
    }

    if(bChecked)
    {
        for (int llll=0; llll<Objects2d::nOpPoints(); llll++)
        {
            OpPoint* pOpPoint = Objects2d::opPointAt(llll);

            if (pOpPoint &&
                pOpPoint->polarName()  == XDirect::curPolar()->name() &&
                pOpPoint->foilName() == XDirect::curFoil()->name())
            {
                    if(fabs(XDirect::curOpp()->m_Alpha - pOpPoint->aoa())<0.0001)
                        m_posAnimate = llll-1;
            }
        }
        m_bAnimate  = true;
        int speed = m_pslAnimateSpeed->value();
        m_pAnimateTimer->setInterval(800-speed);
        m_pAnimateTimer->start();
    }
    else
    {
        m_pAnimateTimer->stop();
        m_bAnimate = false;
        if(m_posAnimate<0 || m_posAnimate>=Objects2d::nOpPoints()) return;
        OpPoint* pOpPoint = Objects2d::opPointAt(m_posAnimate);
        if(pOpPoint) s_pXDirect->setOpp(pOpPoint->aoa());
//        UpdateView();
        return;
    }
}


void OpPointCtrls::onAnimateSingle()
{
    bool bIsValid(false);

    if(Objects2d::nOpPoints()<=1) return;

    Foil *pFoil = XDirect::curFoil();
    Polar const *pPolar = XDirect::curPolar();
    OpPoint const *pCurOpp = XDirect::curOpp();

    // find the next oppoint related to this foil and polar pair
    while(!bIsValid)
    {
        if(m_bAnimatePlus)
        {
            m_posAnimate++;
            if (m_posAnimate >= Objects2d::nOpPoints())
            {
                m_posAnimate = Objects2d::nOpPoints()-2;
                m_bAnimatePlus = false;
            }
        }
        else
        {
            m_posAnimate--;
            if (m_posAnimate <0)
            {
                m_posAnimate = 1;
                m_bAnimatePlus = true;
            }
        }
        if(m_posAnimate<0 || m_posAnimate>=Objects2d::nOpPoints()) return;

        OpPoint* pOpPoint = Objects2d::opPointAt(m_posAnimate);

        if (    pOpPoint &&
                pOpPoint->polarName() == pPolar->name() &&
                pOpPoint->foilName() == pFoil->name() &&
                pOpPoint != pCurOpp)
        {
            bIsValid = true;
            s_pXDirect->setOpp(pOpPoint);
            s_pXDirect->resetCurves();
            s_pXDirect->updateView();
        }
    }
}


void OpPointCtrls::stopAnimate()
{
    if(m_bAnimate)
    {
        m_pAnimateTimer->stop();
        m_bAnimate = false;
        m_pchAnimate->setChecked(false);
        s_pXDirect->setOpp();
    }
}

