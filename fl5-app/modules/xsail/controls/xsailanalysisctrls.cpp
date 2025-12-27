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


#include <QKeyEvent>
#include <QVBoxLayout>

#include "xsailanalysisctrls.h"

#include <interfaces/controls/analysisrangetable.h>
#include <modules/xsail/xsail.h>
#include <api/boatopp.h>
#include <api/boatpolar.h>


XSail *XSailAnalysisCtrls::s_pXSail = nullptr;

XSailAnalysisCtrls::XSailAnalysisCtrls(XSail *pXSail) : QWidget()
{
    s_pXSail = pXSail;

    setupLayout();
    connectSignals();
}


void XSailAnalysisCtrls::connectSignals()
{
    connect(m_pAnalysisRangeTable, SIGNAL(pressed(QModelIndex)), SLOT(onSetControls()));
    connect(m_pchStoreBtOpps,      SIGNAL(clicked()),            SLOT(onStoreBtOpp()));
}


void XSailAnalysisCtrls::setupLayout()
{
    setSizeIncrement(QSizePolicy::Maximum, QSizePolicy::Maximum);
    QVBoxLayout *pAnalysisLayout = new QVBoxLayout;
    {
        m_pAnalysisRangeTable = new AnalysisRangeTable(this);
        m_pAnalysisRangeTable->setName("XSail_Analysis3d_ctrls"); // debug use only

        m_pchStoreBtOpps = new QCheckBox("Store operating points");

        m_ppbAnalyze     = new QPushButton("Calculate");
        m_ppbAnalyze->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
        m_ppbAnalyze->setToolTip("Start the analysis\t(Ctrl+A)");

        pAnalysisLayout->addWidget(m_pAnalysisRangeTable);

        pAnalysisLayout->addWidget(m_pchStoreBtOpps);
        pAnalysisLayout->addWidget(m_ppbAnalyze);
    }

    setLayout(pAnalysisLayout);
}


void XSailAnalysisCtrls::showEvent(QShowEvent *)
{

    setAnalysisRange();
    onSetControls();
}


void XSailAnalysisCtrls::keyPressEvent(QKeyEvent *pEvent)
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
                s_pXSail->onAnalyze();
            }
            pEvent->accept();
            break;
        }
    }
}


/**
 * The user has toggled the request to store or not the operating points of an analysis
 */
void XSailAnalysisCtrls::onStoreBtOpp()
{
    XSail::setStoreOpps3d(m_pchStoreBtOpps->isChecked());
}


/**
 * Initializes the input parameters depending on the type of the active polar
 */
void XSailAnalysisCtrls::onSetControls()
{
    BoatPolar const *pCurBtPolar = s_pXSail->curBtPolar();
    m_ppbAnalyze->setEnabled(pCurBtPolar);
    m_pchStoreBtOpps->setEnabled(pCurBtPolar);
    m_pchStoreBtOpps->setChecked(XSail::bStoreOpps3d());

    if(!pCurBtPolar)
    {
        m_pAnalysisRangeTable->setControls(xfl::EXTERNALPOLAR); // disable
        return;
    }

    m_pAnalysisRangeTable->setControls(pCurBtPolar->type());

    if(pCurBtPolar && pCurBtPolar->isExternalPolar())
    {
        m_ppbAnalyze->setEnabled(false);

        m_pchStoreBtOpps->setEnabled(pCurBtPolar);
    }
    else
    {
        m_ppbAnalyze->setEnabled(m_pAnalysisRangeTable->hasActiveAnalysis());
        m_pchStoreBtOpps->setEnabled(pCurBtPolar);
    }
}


std::vector<double> XSailAnalysisCtrls::oppList() const
{
    std::vector<double> opps;

    QVector<AnalysisRange> ranges;
    m_pAnalysisRangeTable->readTable(ranges);

    for(int ia=0; ia<ranges.size(); ia++)
    {
        if(ranges.at(ia).isActive())
        {
            std::vector<double> const &vals = ranges.at(ia).values();
            opps.insert(opps.end(), vals.begin(), vals.end());
        }
    }

    //sort and remove duplicates
    std::vector<double> sorteduniquevalues;
    std::sort(opps.begin(), opps.end());
    double dlast = -1.e10;
    for(uint i=0; i<opps.size(); i++)
    {
        if(fabs(opps.at(i)-dlast)>1.e-6) sorteduniquevalues.push_back(opps.at(i));
        dlast = opps.at(i);
    }

    return sorteduniquevalues;
}


void XSailAnalysisCtrls::setPolar3d(const BoatPolar *pBtPolar)
{
    if(!pBtPolar)
        m_pAnalysisRangeTable->setPolarType(xfl::EXTERNALPOLAR);
    else
        m_pAnalysisRangeTable->setPolarType(pBtPolar->type());

    m_pAnalysisRangeTable->fillTable();
}


/**
 * Initializes the input parameters depending on the type of the active polar
 */
void XSailAnalysisCtrls::setAnalysisRange()
{
    BoatPolar const *pCurBtPolar = s_pXSail->curBtPolar();

    if (!pCurBtPolar)
    {
        m_pchStoreBtOpps->setEnabled(false);

        m_pAnalysisRangeTable->setPolarType(xfl::EXTERNALPOLAR);
        m_pAnalysisRangeTable->fillTable();
        return;
    }
    else
    {
        m_pchStoreBtOpps->setEnabled(true);

        m_pAnalysisRangeTable->setPolarType(pCurBtPolar->type());
        m_pAnalysisRangeTable->fillTable();
    }
}


