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


#include <QGroupBox>
#include <QVBoxLayout>

#include "analysis2dctrls.h"
#include <api/polar.h>
#include <api/utils.h>
#include <api/xfoiltask.h>
#include <core/xflcore.h>
#include <interfaces/controls/analysisrangetable.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <modules/xdirect/xdirect.h>

XDirect *Analysis2dCtrls::s_pXDirect = nullptr;


Analysis2dCtrls::Analysis2dCtrls(QWidget *pParent) : QWidget(pParent)
{
    setupLayout();
    connectSignals();
}


void Analysis2dCtrls::connectSignals()
{
    connect(m_pchStoreOpp,   SIGNAL(clicked()),             SLOT(onStoreOpp()));
    connect(m_pRangeTable,   SIGNAL(pressed(QModelIndex)),  SLOT(onSetControls()));


    connect(m_prbAlpha,      SIGNAL(clicked()),             SLOT(onSpec()));
    connect(m_prbCl,         SIGNAL(clicked()),             SLOT(onSpec()));

    connect(m_pchStoreOpp,   SIGNAL(clicked()),             SLOT(onStoreOpp()));
    connect(m_ppbAnalyze,    SIGNAL(clicked()), s_pXDirect, SLOT(onAnalyze()));
}


void Analysis2dCtrls::setupLayout()
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

    QVBoxLayout *pAnalysisGroupLayout = new QVBoxLayout;
    {
        m_pchStoreOpp = new QCheckBox(tr("Store operating points"));
        m_ppbAnalyze  = new QPushButton(tr("Calculate"));

        m_ppbAnalyze->setToolTip(tr("Start the analysis\t(Ctrl+A)"));

        QHBoxLayout *pSpecVarsLayout = new QHBoxLayout;
        {
            m_prbAlpha    = new QRadioButton(ALPHAch);
            m_prbCl       = new QRadioButton(tr("Cl"));
            m_prbReynolds = new QRadioButton(tr("Reynolds"));
            m_prbTheta    = new QRadioButton(THETAch);
            pSpecVarsLayout->addStretch();
            pSpecVarsLayout->addWidget(m_prbAlpha);
            pSpecVarsLayout->addWidget(m_prbCl);
            pSpecVarsLayout->addWidget(m_prbReynolds);
            pSpecVarsLayout->addWidget(m_prbTheta);
            pSpecVarsLayout->addStretch();
        }

        m_pRangeTable = new AnalysisRangeTable(this);
        m_pRangeTable->setFoilPolar(true);
        m_pRangeTable->setName("Foil_ranges"); // debug use only


        pAnalysisGroupLayout->addLayout(pSpecVarsLayout);
        pAnalysisGroupLayout->addWidget(m_pRangeTable);
        pAnalysisGroupLayout->addWidget(m_pchStoreOpp);
        pAnalysisGroupLayout->addWidget(m_ppbAnalyze);
    }
    setLayout(pAnalysisGroupLayout);
}


QVector<AnalysisRange> Analysis2dCtrls::ranges() const
{
    return m_pRangeTable->ranges();
}


void Analysis2dCtrls::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);
    onSetControls();
}


void Analysis2dCtrls::keyPressEvent(QKeyEvent *pEvent)
{
/*    bool bShift = false;
    if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
    bool bCtrl = false;
    if(event->modifiers() & Qt::ControlModifier)   bCtrl =true;
*/
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            onReadAnalysisData();
            if(m_ppbAnalyze->hasFocus())  s_pXDirect->onAnalyze();
            else
            {
                activateWindow();
                m_ppbAnalyze->setFocus();
            }
            break;
        }
    }
}


void Analysis2dCtrls::onInputChanged()
{
    onReadAnalysisData();
}


void Analysis2dCtrls::onSpec()
{
    if      (m_prbAlpha->isChecked()) XDirect::setAoAAnalysis(true);
    else if (m_prbCl->isChecked())    XDirect::setAoAAnalysis(false);

    fillAnalysisTable();
}


void Analysis2dCtrls::onStoreOpp()
{
    XDirect::setStoreOpps(m_pchStoreOpp->isChecked());
}


void Analysis2dCtrls::onReadAnalysisData()
{
    Polar *pCurPolar = s_pXDirect->curPolar();
    if(!pCurPolar) return;

    if      (m_prbAlpha->isChecked()) XDirect::setAoAAnalysis(true);
    else if (m_prbCl->isChecked())    XDirect::setAoAAnalysis(false);


    XDirect::setStoreOpps(m_pchStoreOpp->isChecked());
}


void Analysis2dCtrls::onSetControls()
{
    m_pchStoreOpp->setChecked(XDirect::bStoreOpps());

    Polar const*pCurPolar = s_pXDirect->curPolar();

    if(pCurPolar)
    {
        if(pCurPolar->isControlPolar())
        {
            if(XDirect::bAlpha()) m_prbAlpha->setChecked(true);
            else                    m_prbCl->setChecked(true);
            m_prbAlpha->setEnabled(false);
            m_prbCl->setEnabled(false);
            m_prbReynolds->setEnabled(false);
            m_prbTheta->setEnabled(true);
            m_prbTheta->setChecked(true);
        }
        else if(pCurPolar->isFixedaoaPolar())
        {
            m_prbAlpha->setEnabled(false);
            m_prbCl->setEnabled(false);
            m_prbReynolds->setEnabled(true);
            m_prbTheta->setEnabled(false);
            m_prbReynolds->setChecked(true);
        }
        else
        {
            if(XDirect::bAlpha()) m_prbAlpha->setChecked(true);
            else                    m_prbCl->setChecked(true);
            m_prbAlpha->setEnabled(true);
            m_prbCl->setEnabled(true);
            m_prbReynolds->setEnabled(false);
            m_prbTheta->setEnabled(false);
        }
    }
    else
    {
        if(XDirect::bAlpha())
        {
            m_prbAlpha->setChecked(true);
            m_pRangeTable->setRangeType(AnalysisRange::ALPHA);
        }
        else
        {
            m_prbCl->setChecked(true);
            m_pRangeTable->setRangeType(AnalysisRange::CL);
        }
        m_prbAlpha->setEnabled(false);
        m_prbCl->setEnabled(false);
        m_prbReynolds->setEnabled(false);
        m_prbTheta->setEnabled(false);
    }

    m_ppbAnalyze->setEnabled(pCurPolar);
    m_pchStoreOpp->setEnabled(pCurPolar);

    m_pRangeTable->setEnabled(pCurPolar);

    fillAnalysisTable();
}


void Analysis2dCtrls::fillAnalysisTable()
{
    Polar const*pCurPolar = s_pXDirect->curPolar();

    if (!pCurPolar)
    {
        m_pRangeTable->setPolarType(xfl::EXTERNALPOLAR);
        m_pRangeTable->fillTable();
        return;
    }

    if(pCurPolar->isType123())
    {
        if     (m_prbAlpha->isChecked()) m_pRangeTable->setRangeType(AnalysisRange::ALPHA);
        else if(m_prbCl->isChecked()) m_pRangeTable->setRangeType(AnalysisRange::CL);
    }
    else if(pCurPolar->isType4()) m_pRangeTable->setRangeType(AnalysisRange::REYNOLDS);
    else if(pCurPolar->isType6()) m_pRangeTable->setRangeType(AnalysisRange::THETA);

    m_pRangeTable->fillTable();

}


void Analysis2dCtrls::enableAnalyze(bool b)
{
    m_ppbAnalyze->setEnabled(b);
}


