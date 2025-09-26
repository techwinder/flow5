/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QScrollBar>


#include "analysis3dctrls.h"

#include <xfl/controls/analysisrangetable.h>
#include <xfl/controls/t8rangetable.h>
#include <xfl/globals/mainframe.h>
#include <xfl/xplane/analysis/analysis3dsettings.h>
#include <xfl/xplane/xplane.h>

#include <xflobjects/analysis3d/planetask.h>
#include <xflcore/xflcore.h>
#include <xflcore/units.h>
#include <xflobjects/objects3d/analysis/planeopp.h>
#include <xflobjects/objects3d/analysis/wpolar.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>

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
    connect(m_pchStorePOpps,       SIGNAL(clicked(bool)),        SLOT(onStorePOpps()));
}


void Analysis3dCtrls::setupLayout()
{
//    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);

    QVBoxLayout *pAnalysisLayout = new QVBoxLayout;
    {
        m_plabParamName = new QLabel(ALPHACHAR);
        m_plabParamName->setAlignment(Qt::AlignHCenter);

        m_pswTables = new QStackedWidget;
        {
            m_pAnalysisRangeTable = new AnalysisRangeTable(this);
            m_pAnalysisRangeTable->setName("Analysis3d_ctrls"); // debug use only

            m_pXRangeTable = new T8RangeTable(this);

            m_pswTables->addWidget(m_pAnalysisRangeTable);
            m_pswTables->addWidget(m_pXRangeTable);
        }


        m_pchStorePOpps  = new QCheckBox("Store operating points");

        m_ppbAnalyze = new QPushButton("Analyze");
        m_ppbAnalyze->setAutoDefault(true);
        m_ppbAnalyze->setDefault(true);
        m_ppbAnalyze->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
        m_ppbAnalyze->setToolTip("Start the analysis\t(Ctrl+A)");

        pAnalysisLayout->addWidget(m_plabParamName);
        pAnalysisLayout->addWidget(m_pswTables);
        pAnalysisLayout->addWidget(m_pchStorePOpps);
        pAnalysisLayout->addWidget(m_ppbAnalyze);
    }

    setLayout(pAnalysisLayout);
}


void Analysis3dCtrls::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);

    setAnalysisRange();
    m_pchStorePOpps->setChecked(PlaneOpp::bStoreOpps3d());
}


void Analysis3dCtrls::setParameterLabels()
{
    WPolar const*pWPolar = s_pXPlane->curWPolar();
    if(pWPolar)
    {
        switch(pWPolar->type())
        {
            case xfl::T1POLAR:
            case xfl::T2POLAR:
            case xfl::T3POLAR:
            {
                m_plabParamName->setText(ALPHACHAR);
                break;
            }
            case xfl::T4POLAR:
            {
                QString str;
                Units::getSpeedUnitLabel(str);
                m_plabParamName->setText("V"+INFCHAR);
                break;
            }
            case xfl::T6POLAR:
            case xfl::T7POLAR:
            {
                m_plabParamName->setText("Control parameter");
                break;
            }
            case xfl::T5POLAR:
            {
                m_plabParamName->setText(BETACHAR);
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


void Analysis3dCtrls::getXRanges(QVector<T8Opp> &ranges) const
{
    m_pXRangeTable->readRangeTable(ranges, true);
}


QVector<double> Analysis3dCtrls::oppList() const
{
    return m_pAnalysisRangeTable->oppList();
}


void Analysis3dCtrls::setAnalysisRange()
{
    WPolar const *pCurWPolar = s_pXPlane->curWPolar();

    if (!pCurWPolar)
    {
        m_pchStorePOpps->setEnabled(false);

        m_pAnalysisRangeTable->setPolarType(xfl::EXTERNALPOLAR);
        m_pAnalysisRangeTable->fillTable();
        return;
    }

    m_pchStorePOpps->setEnabled(true);

    if(pCurWPolar->isType8())
    {
        m_pswTables->setCurrentWidget(m_pXRangeTable);
        m_pXRangeTable->fillRangeTable();
        m_plabParamName->clear();
    }
    else
    {
        m_pswTables->setCurrentWidget(m_pAnalysisRangeTable);
        m_pAnalysisRangeTable->setPolarType(pCurWPolar->type());
        m_pAnalysisRangeTable->fillTable();
        setParameterLabels();
    }
}


void Analysis3dCtrls::onStorePOpps()
{
    PlaneOpp::setStoreOpps3d(m_pchStorePOpps->isChecked());
}


void Analysis3dCtrls::onSetControls()
{
    WPolar const *pCurWPolar = s_pXPlane->curWPolar();

    m_ppbAnalyze->setEnabled(pCurWPolar);
    m_pchStorePOpps->setEnabled(pCurWPolar);

    if(!pCurWPolar)
    {
        m_pAnalysisRangeTable->setControls(xfl::EXTERNALPOLAR); // disable
        return;
    }

    if(pCurWPolar->isType7())
    {
        // no range needed
        m_ppbAnalyze->setEnabled(true);
    }
    else if(pCurWPolar->isType8())
    {
        m_ppbAnalyze->setEnabled(m_pXRangeTable->hasActiveAnalysis());
    }
    else
    {
        m_pAnalysisRangeTable->setControls(pCurWPolar->type());
        m_ppbAnalyze->setEnabled(m_pAnalysisRangeTable->hasActiveAnalysis());
    }

    if(pCurWPolar && pCurWPolar->isExternalPolar())
    {
        m_ppbAnalyze->setEnabled(false);

        m_pchStorePOpps->setEnabled(pCurWPolar);
    }
    else
    {
        m_pchStorePOpps->setEnabled(pCurWPolar);

        if (s_pXPlane->isAnalysisRunning())
            m_ppbAnalyze->setEnabled(false);
    }
}




