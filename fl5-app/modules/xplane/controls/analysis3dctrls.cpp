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


#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QScrollBar>


#include "analysis3dctrls.h"

#include <interfaces/controls/analysisrangetable.h>
#include <interfaces/controls/t8rangetable.h>
#include <globals/mainframe.h>
#include <modules/xplane/analysis/analysis3dsettings.h>
#include <modules/xplane/xplane.h>

#include <api/planetask.h>
#include <core/xflcore.h>
#include <api/utils.h>
#include <api/units.h>
#include <api/planeopp.h>
#include <api/planepolar.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>

XPlane *Analysis3dCtrls::s_pXPlane = nullptr;


Analysis3dCtrls::Analysis3dCtrls(QWidget *pParent) : QWidget(pParent)
{
    setupLayout();
    connectSignals();
}


void Analysis3dCtrls::connectSignals()
{
    connect(m_pAnalysisRangeTable, SIGNAL(pressed(QModelIndex)), SLOT(onSetControls()));
    connect(m_pXRangeTable,        SIGNAL(pressed(QModelIndex)), SLOT(onSetControls()));
    connect(m_pchStorePOpps,       SIGNAL(clicked(bool)),        SLOT(onOption()));
    connect(m_pchStabDerivatives,  SIGNAL(clicked(bool)),        SLOT(onOption()));
}


void Analysis3dCtrls::setupLayout()
{
//    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);

    QVBoxLayout *pAnalysisLayout = new QVBoxLayout;
    {
        m_plabParamName = new QLabel(ALPHAch);
        m_plabParamName->setAlignment(Qt::AlignHCenter);

        m_pswTables = new QStackedWidget;
        {
            m_pAnalysisRangeTable = new AnalysisRangeTable(this);
            m_pAnalysisRangeTable->setName("Analysis3d_ctrls"); // debug use only

            m_pXRangeTable = new T8RangeTable(this);

            m_pswTables->addWidget(m_pAnalysisRangeTable);
            m_pswTables->addWidget(m_pXRangeTable);
        }

        QHBoxLayout *pOptionLayout = new QHBoxLayout;
        {
            m_pchStorePOpps      = new QCheckBox(tr("Store operating points"));
            m_pchStorePOpps->setToolTip(tr("<p>"
                                        "If activated, the operating points will be stored at the end of the calculation. "
                                        "The results are stored in the polar in all cases."
                                        "</p>"));
            m_pchStabDerivatives = new QCheckBox(tr("Compute derivatives"));
            m_pchStabDerivatives->setToolTip(tr("<p>"
                                             "If activated, stability derivatives and eigenthings will be "
                                             "computed during a T12358 run.<br>"
                                             "Deactivate to save a little computation time."
                                             "</p>"));
            pOptionLayout->addWidget(m_pchStorePOpps);
            pOptionLayout->addStretch();
            pOptionLayout->addWidget(m_pchStabDerivatives);
        }

        m_ppbAnalyze = new QPushButton(tr("Calculate"));
        m_ppbAnalyze->setAutoDefault(true);
        m_ppbAnalyze->setDefault(true);
        m_ppbAnalyze->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
        m_ppbAnalyze->setToolTip(tr("Start the analysis\t(Ctrl+A)"));

        pAnalysisLayout->addWidget(m_plabParamName);
        pAnalysisLayout->addWidget(m_pswTables);
        pAnalysisLayout->addLayout(pOptionLayout);
        pAnalysisLayout->addWidget(m_ppbAnalyze);
    }

    setLayout(pAnalysisLayout);
}


void Analysis3dCtrls::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);

    setAnalysisRange();
    m_pchStorePOpps->setChecked(XPlane::bStoreOpps3d());
    m_pchStabDerivatives->setChecked(Analysis3dSettings::bStabDerivatives());
    PlanePolar const *pCurPlPolar = s_pXPlane->curPlPolar();
    m_pchStabDerivatives->setEnabled(pCurPlPolar && pCurPlPolar->isLinearPolar() && XPlane::bStoreOpps3d());
}


void Analysis3dCtrls::setParameterLabels()
{
    PlanePolar const*pWPolar = s_pXPlane->curPlPolar();
    if(pWPolar)
    {
        switch(pWPolar->type())
        {
            case xfl::T1POLAR:
            case xfl::T2POLAR:
            case xfl::T3POLAR:
            {
                m_plabParamName->setText(ALPHAch);
                break;
            }
            case xfl::T4POLAR:
            {
//                QString str = Units::speedUnitLabel();
                m_plabParamName->setText(tr("V")+INFch);
                break;
            }
            case xfl::T6POLAR:
            case xfl::T7POLAR:
            {
                m_plabParamName->setText(tr("Control parameter"));
                break;
            }
            case xfl::T5POLAR:
            {
                m_plabParamName->setText(BETAch);
                break;
            }
            case xfl::T8POLAR:
            {
                m_plabParamName->clear(); // never reached, using XRangeTable instead
                break;
            }
            case xfl::EXTERNALPOLAR:
            {
                m_plabParamName->clear();
                break;
            }
            case xfl::BOATPOLAR:
            {
                m_plabParamName->clear(); // not applicable
                break;
            }
        }
    }
    else
    {
        m_plabParamName->clear();
    }
}


void Analysis3dCtrls::keyPressEvent(QKeyEvent *pEvent)
{
//    bool bCtrl = (event->modifiers() & Qt::ControlModifier)? true : false;
//    bool bShift = (event->modifiers() & Qt::ShiftModifier)  ? true : false;

    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_ppbAnalyze->hasFocus())
            {
                activateWindow();
                m_ppbAnalyze->setFocus();
            }
            else
            {
                s_pXPlane->onAnalyze();
            }
            pEvent->accept();
            break;
        }
    }
}


void Analysis3dCtrls::getXRanges(std::vector<T8Opp> &ranges) const
{
    std::vector<T8Opp> Ranges;
    m_pXRangeTable->readRangeTable(Ranges, true);
    ranges.clear();
    ranges.insert(ranges.end(), Ranges.begin(), Ranges.end());
}


std::vector<double> Analysis3dCtrls::oppList() const
{
    return m_pAnalysisRangeTable->oppList();
}


void Analysis3dCtrls::setAnalysisRange()
{
    PlanePolar const *pCurPlPolar = s_pXPlane->curPlPolar();

    if (!pCurPlPolar)
    {
        m_pchStorePOpps->setEnabled(false);
        m_pchStabDerivatives->setEnabled(false);

        m_pAnalysisRangeTable->setPolarType(xfl::EXTERNALPOLAR);
        m_pAnalysisRangeTable->fillTable();
        return;
    }

    m_pchStorePOpps->setEnabled(true);
    m_pchStabDerivatives->setEnabled(pCurPlPolar && pCurPlPolar->isLinearPolar() && XPlane::bStoreOpps3d());

    if(pCurPlPolar->isType8())
    {
        m_pswTables->setCurrentWidget(m_pXRangeTable);
        m_pXRangeTable->fillRangeTable();
        m_plabParamName->clear();
    }
    else
    {
        m_pswTables->setCurrentWidget(m_pAnalysisRangeTable);
        m_pAnalysisRangeTable->setPolarType(pCurPlPolar->type());
        m_pAnalysisRangeTable->fillTable();
        setParameterLabels();
    }
}


void Analysis3dCtrls::onOption()
{
    XPlane::setStoreOpps3d(m_pchStorePOpps->isChecked());
    Analysis3dSettings::setStabDerivatives(m_pchStabDerivatives->isChecked());
    PlanePolar const *pCurPlPolar = s_pXPlane->curPlPolar();
    m_pchStabDerivatives->setEnabled(pCurPlPolar && pCurPlPolar->isLinearPolar() && XPlane::bStoreOpps3d());
}


void Analysis3dCtrls::onSetControls()
{
    PlanePolar const *pCurPlPolar = s_pXPlane->curPlPolar();

    m_ppbAnalyze->setEnabled(pCurPlPolar);
    m_pchStorePOpps->setEnabled(pCurPlPolar);
    m_pchStabDerivatives->setEnabled(pCurPlPolar && pCurPlPolar->isLinearPolar() && XPlane::bStoreOpps3d());

    if(!pCurPlPolar)
    {
        m_pAnalysisRangeTable->setControls(xfl::EXTERNALPOLAR); // disable
        return;
    }

    if(pCurPlPolar->isType7())
    {
        // no range needed
        m_ppbAnalyze->setEnabled(true);
    }
    else if(pCurPlPolar->isType8())
    {
        m_ppbAnalyze->setEnabled(m_pXRangeTable->hasActiveAnalysis());
    }
    else
    {
        m_pAnalysisRangeTable->setControls(pCurPlPolar->type());
        m_ppbAnalyze->setEnabled(m_pAnalysisRangeTable->hasActiveAnalysis());
    }

    if(pCurPlPolar && pCurPlPolar->isExternalPolar())
    {
        m_ppbAnalyze->setEnabled(false);

    }
    else
    {
        if (s_pXPlane->isAnalysisRunning())
            m_ppbAnalyze->setEnabled(false);
    }
}




