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


#include "blgraphctrls.h"

#include <globals/mainframe.h>
#include <interfaces/graphs/controls/graphtiles.h>
#include <modules/xdirect/xdirect.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/line/linemenu.h>

BLGraphCtrls::BLGraphCtrls(GraphTiles *pParent) : GraphTileCtrls(pParent)
{
}


void BLGraphCtrls::connectSignals()
{
    GraphTileCtrls::connectSignals();

    connect(m_pchTop,          SIGNAL(clicked(bool)),        SLOT(onBLSide()));
    connect(m_pchBot,          SIGNAL(clicked(bool)),        SLOT(onBLSide()));
    connect(m_pchShowInviscid, SIGNAL(clicked(bool)),        SLOT(onInviscid()));
    connect(m_plbTopStyle,     SIGNAL(clickedLB(LineStyle)), SLOT(onTopCurveStyle(LineStyle)));
    connect(m_plbBotStyle,     SIGNAL(clickedLB(LineStyle)), SLOT(onBotCurveStyle(LineStyle)));
}


void BLGraphCtrls::setupLayout()
{
    m_pcptVariableSet->hide();
    QGridLayout *pGraphLayout = new QGridLayout;
    {
        pGraphLayout->addWidget(m_prbGraph[0],   1,1);
        pGraphLayout->addWidget(m_prbGraph[1],   1,2);
        pGraphLayout->addWidget(m_prbGraph[2],   2,1);
        pGraphLayout->addWidget(m_prbGraph[3],   2,2);
        pGraphLayout->addWidget(m_prbGraph[4],   3,1);
        pGraphLayout->addWidget(m_prbTwoGraphs,  3,2);
        pGraphLayout->addWidget(m_prbFourGraphs, 4,1);
        pGraphLayout->addWidget(m_prbAllGraphs,  4,2);
        pGraphLayout->setRowStretch(5,1);
    }

    QFrame *pBLOptionsFrame = new QFrame;
    {
        QVBoxLayout *pBLOptionsLayout = new QVBoxLayout;
        {
            QGridLayout *plsLayout = new QGridLayout;
            {
                m_pchTop = new QCheckBox("Top");
                m_pchBot = new QCheckBox("Bottom");
                m_plbTopStyle = new LineBtn;
                m_plbBotStyle = new LineBtn;
                plsLayout->addWidget(m_pchTop,       1,1);
                plsLayout->addWidget(m_plbTopStyle,  1,2);
                plsLayout->addWidget(m_pchBot,       2,1);
                plsLayout->addWidget(m_plbBotStyle,  2,2);
                plsLayout->setColumnStretch(3,1);
            }

            m_pchShowInviscid = new QCheckBox("Inviscid curve");
            pBLOptionsLayout->addWidget(m_pchShowInviscid);
            pBLOptionsLayout->addLayout(plsLayout);
        }
        pBLOptionsFrame->setLayout(pBLOptionsLayout);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(pBLOptionsFrame);
        pMainLayout->addLayout(pGraphLayout);
    }
    setLayout(pMainLayout);
}


void BLGraphCtrls::setControls()
{
    GraphTileCtrls::setControls();
    m_pchShowInviscid->setChecked(s_pXDirect->m_bShowInviscid);
    m_pchTop->setChecked(XDirect::s_bBLTopSide);
    m_pchBot->setChecked(XDirect::s_bBLTopSide);


    m_plbTopStyle->setTheStyle(XDirect::s_lsTopBL);
    m_plbBotStyle->setTheStyle(XDirect::s_lsBotBL);
}


void BLGraphCtrls::onBLSide()
{
    XDirect::s_bBLTopSide = m_pchTop->isChecked();
    XDirect::s_bBLBotSide = m_pchBot->isChecked();
    s_pXDirect->resetCurves();
    s_pXDirect->updateView();
}


void BLGraphCtrls::onInviscid()
{
    s_pXDirect->m_bShowInviscid = m_pchShowInviscid->isChecked();
    s_pXDirect->resetCurves();
    s_pXDirect->updateView();
}


void BLGraphCtrls::onTopCurveStyle(LineStyle)
{
    LineMenu *m_pLineMenu = new LineMenu(nullptr, false);
    m_pLineMenu->initMenu(XDirect::s_lsTopBL);
    m_pLineMenu->exec(QCursor::pos());
    LineStyle ls = m_pLineMenu->theStyle();
    m_plbTopStyle->setTheStyle(ls);
    XDirect::s_lsTopBL = ls;
    s_pXDirect->resetCurves();
    s_pXDirect->updateView();
}


void BLGraphCtrls::onBotCurveStyle(LineStyle)
{
    LineMenu *m_pLineMenu = new LineMenu(nullptr);
    m_pLineMenu->initMenu(XDirect::s_lsBotBL);
    m_pLineMenu->exec(QCursor::pos());
    LineStyle ls = m_pLineMenu->theStyle();
    m_plbBotStyle->setTheStyle(ls);
    XDirect::s_lsBotBL = ls;
    s_pXDirect->resetCurves();
    s_pXDirect->updateView();
}

