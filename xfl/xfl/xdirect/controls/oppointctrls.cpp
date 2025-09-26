/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED



#include <QGridLayout>
#include <QTimer>

#include "oppointctrls.h"

#include <xfl/xdirect/view2d/oppointwt.h>
#include <xfl/xdirect/xdirect.h>

#include <xflgraph/graph/graph.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/objects2d.h>
#include <xflfoil/objects2d/oppoint.h>
#include <xflfoil/objects2d/polar.h>
#include <xflwidgets/line/linemenu.h>


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
            m_prbCpCurve = new QRadioButton("Cp");
            m_prbQCurve  = new QRadioButton("Edge velocity");
            pGraphBoxLayout->addStretch();
            pGraphBoxLayout->addWidget(m_prbCpCurve);
            pGraphBoxLayout->addStretch();
            pGraphBoxLayout->addWidget(m_prbQCurve);
            pGraphBoxLayout->addStretch();
        }

        QGridLayout *pOppDisplayLayout = new QGridLayout;
        {
            m_pchFillFoil      = new QCheckBox("Fill foil");
            m_pchShowBL        = new QCheckBox("BL (d*)");
            m_pchShowBL->setToolTip("Displacement thickness");
            m_pchShowPressure  = new QCheckBox("Pressure");
            m_plbBLStyle       = new LineBtn;
            m_plbPressureStyle = new LineBtn;
            m_pchAnimate       = new QCheckBox("Animate");
            m_pslAnimateSpeed  = new QSlider(Qt::Horizontal);
            m_pslAnimateSpeed->setMinimum(0);
            m_pslAnimateSpeed->setMaximum(1000);
            m_pslAnimateSpeed->setSliderPosition(500);
            m_pslAnimateSpeed->setTickInterval(50);
            m_pslAnimateSpeed->setTickPosition(QSlider::TicksBelow);

            m_pchShowActiveOppOnly = new QCheckBox("Active operating point only");

            pOppDisplayLayout->addWidget(m_pchFillFoil,          1,1);
            pOppDisplayLayout->addWidget(m_pchShowBL,            2,1);
            pOppDisplayLayout->addWidget(m_plbBLStyle,           2,2);
            pOppDisplayLayout->addWidget(m_pchShowPressure,      3,1);
            pOppDisplayLayout->addWidget(m_plbPressureStyle,     3,2);
            pOppDisplayLayout->addWidget(m_pchAnimate,           4,1,1,2);
            pOppDisplayLayout->addWidget(m_pslAnimateSpeed,      5,1,1,2);
            pOppDisplayLayout->addWidget(m_pchShowActiveOppOnly, 6,1,1,2);
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

    connect(m_pAnimateTimer,        SIGNAL(timeout()),            SLOT(onAnimateSingle()));
    connect(m_pchAnimate,           SIGNAL(clicked(bool)),        SLOT(onAnimate(bool)));
    connect(m_pslAnimateSpeed,      SIGNAL(sliderMoved(int)),     SLOT(onAnimateSpeed(int)));

    connect(m_pchFillFoil,          SIGNAL(clicked(bool)),        SLOT(onFillFoil(bool)));
    connect(m_pchShowBL,            SIGNAL(clicked(bool)),        SLOT(onShowBL(bool)));
    connect(m_pchShowPressure,      SIGNAL(clicked(bool)),        SLOT(onShowPressure(bool)));
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


void OpPointCtrls::setControls()
{
    if(!s_pXDirect) return;
    m_prbQCurve->setChecked(s_pXDirect->CpGraph()->yVariable(0)==1);
    m_prbCpCurve->setChecked(s_pXDirect->CpGraph()->yVariable(0)==0);
    m_prbCpCurve->setEnabled(s_pXDirect->curOpp());
    m_prbQCurve->setEnabled(s_pXDirect->curOpp());

    m_pchFillFoil->setChecked(OpPointWt::bFillFoil());

    m_pchShowPressure->setEnabled(!s_pXDirect->isPolarView() && XDirect::curOpp());
    m_pchShowBL->setEnabled(!s_pXDirect->isPolarView() && XDirect::curOpp());
    m_pchAnimate->setEnabled(!s_pXDirect->isPolarView() && XDirect::curOpp());
    m_pslAnimateSpeed->setEnabled(!s_pXDirect->isPolarView() && XDirect::curOpp() && m_pchAnimate->isChecked());
    m_plbBLStyle->setEnabled(!s_pXDirect->isPolarView() && XDirect::curOpp() && m_pchShowBL->isChecked());
    m_plbPressureStyle->setEnabled(!s_pXDirect->isPolarView() && XDirect::curOpp() && m_pchShowPressure->isChecked());

    m_plbBLStyle->setTheStyle(OpPointWt::BLStyle());
    m_plbPressureStyle->setTheStyle(OpPointWt::pressureStyle());

    m_pchShowActiveOppOnly->setChecked(s_pXDirect->curOppOnly());
    m_pchShowActiveOppOnly->setEnabled(!s_pXDirect->isPolarView());
}


void OpPointCtrls::onBLCurveStyle(LineStyle)
{
    LineMenu *m_pLineMenu = new LineMenu(nullptr, false);
    m_pLineMenu->initMenu(OpPointWt::BLStyle());
    m_pLineMenu->exec(QCursor::pos());
    LineStyle ls = m_pLineMenu->theStyle();
    m_plbBLStyle->setTheStyle(ls);
    m_pOpPointWt->setBLStyle(ls);
}



void OpPointCtrls::onPressureCurveStyle(LineStyle)
{
    LineMenu *m_pLineMenu = new LineMenu(nullptr);
    m_pLineMenu->initMenu(OpPointWt::pressureStyle());
    m_pLineMenu->exec(QCursor::pos());
    LineStyle ls = m_pLineMenu->theStyle();
    m_plbPressureStyle->setTheStyle(ls);
    m_pOpPointWt->setPressureStyle(ls);
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

    if(!Objects2d::curFoil() || !Objects2d::curPolar())
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

