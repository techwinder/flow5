/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QGroupBox>
#include <QVBoxLayout>

#include "analysis2dctrls.h"
#include <xfl/controls/analysisrangetable.h>
#include <xfl/xdirect/xdirect.h>
#include <xflfoil/objects2d/polar.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflfoil/analysis2d/xfoiltask.h>
#include <xflcore/xflcore.h>

XDirect *Analysis2dCtrls::s_pXDirect = nullptr;


Analysis2dCtrls::Analysis2dCtrls(QWidget *pParent) : QWidget(pParent)
{
    setupLayout();
    connectSignals();
}


void Analysis2dCtrls::connectSignals()
{
    connect(m_pchStoreOpp,   SIGNAL(clicked()),      SLOT(onStoreOpp()));
    connect(m_pRangeTable, SIGNAL(pressed(QModelIndex)), SLOT(onSetControls()));


    connect(m_pchInitBL,     SIGNAL(clicked()),      SLOT(onReadAnalysisData()));

    connect(m_prbSpec1,      SIGNAL(clicked()),      SLOT(onSpec()));
    connect(m_prbSpec2,      SIGNAL(clicked()),      SLOT(onSpec()));

    connect(m_pchViscous,    SIGNAL(clicked()),      SLOT(onViscous()));
    connect(m_pchStoreOpp,   SIGNAL(clicked()),      SLOT(onStoreOpp()));
    connect(m_ppbAnalyze,    SIGNAL(clicked()),      s_pXDirect, SLOT(onAnalyze()));
}


void Analysis2dCtrls::setupLayout()
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

    QVBoxLayout *pAnalysisGroupLayout = new QVBoxLayout;
    {
        m_pchStoreOpp = new QCheckBox("Store operating point");
        m_ppbAnalyze  = new QPushButton("Analyze");

        m_ppbAnalyze->setToolTip("Start the analysis\t(Ctrl+A)");

        QHBoxLayout *pSpecVarsLayout = new QHBoxLayout;
        {
            m_prbSpec1 = new QRadioButton(ALPHACHAR);
            m_prbSpec2 = new QRadioButton("Cl");
            m_prbSpec3 = new QRadioButton("Re");
            m_prbSpec4 = new QRadioButton(THETACHAR);
            pSpecVarsLayout->addWidget(m_prbSpec1);
            pSpecVarsLayout->addWidget(m_prbSpec2);
            pSpecVarsLayout->addWidget(m_prbSpec3);
            pSpecVarsLayout->addWidget(m_prbSpec4);
        }

        m_pRangeTable = new AnalysisRangeTable(this);
        m_pRangeTable->setFoilPolar(true);
        m_pRangeTable->setName("Foil_ranges"); // debug use only


        QHBoxLayout *pAnalysisSettings = new QHBoxLayout;
        {
            m_pchViscous  = new QCheckBox("Viscous");
            m_pchInitBL   = new QCheckBox("Initialize B.L.");
            m_pchInitBL->setToolTip("<p>Initializes the boundary layer before starting a new range of XFoil calculations.<br>"
                                    "Recommendation: activate."
                                    "</p>");
            pAnalysisSettings->addWidget(m_pchViscous);
            pAnalysisSettings->addWidget(m_pchInitBL);
        }

        pAnalysisGroupLayout->addLayout(pSpecVarsLayout);
        pAnalysisGroupLayout->addWidget(m_pRangeTable);
        pAnalysisGroupLayout->addLayout(pAnalysisSettings);
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
    if      (m_prbSpec1->isChecked()) XFoilTask::setAoAAnalysis(true);
    else if (m_prbSpec2->isChecked()) XFoilTask::setAoAAnalysis(false);

    fillAnalysisTable();
}


void Analysis2dCtrls::onStoreOpp()
{
    XFoilTask::setStoreOpps(m_pchStoreOpp->isChecked());
}


void Analysis2dCtrls::onViscous()
{
    XFoilTask::setViscous(m_pchViscous->isChecked());
}


void Analysis2dCtrls::onReadAnalysisData()
{
    Polar *pCurPolar = s_pXDirect->curPolar();
    if(!pCurPolar) return;

    if      (m_prbSpec1->isChecked()) XFoilTask::setAoAAnalysis(true);
    else if (m_prbSpec2->isChecked()) XFoilTask::setAoAAnalysis(false);


    XFoilTask::setInitBL(m_pchInitBL->isChecked());
    XFoilTask::setViscous(m_pchViscous->isChecked());
    XFoilTask::setStoreOpps(m_pchStoreOpp->isChecked());
}


void Analysis2dCtrls::onSetControls()
{
    m_pchViscous->setChecked(XFoilTask::bViscous());
    m_pchInitBL->setChecked(XFoilTask::bInitBL());
    m_pchStoreOpp->setChecked(XFoilTask::bStoreOpps());

    Polar const*pCurPolar = s_pXDirect->curPolar();

    if(pCurPolar)
    {
        if(pCurPolar->isControlPolar())
        {
            if(XFoilTask::bAlpha()) m_prbSpec1->setChecked(true);
            else                    m_prbSpec2->setChecked(true);
            m_prbSpec1->setEnabled(false);
            m_prbSpec2->setEnabled(false);
            m_prbSpec3->setEnabled(false);
            m_prbSpec4->setEnabled(true);
            m_prbSpec4->setChecked(true);
        }
        else if(pCurPolar->isFixedaoaPolar())
        {
            m_prbSpec1->setEnabled(false);
            m_prbSpec2->setEnabled(false);
            m_prbSpec3->setEnabled(true);
            m_prbSpec4->setEnabled(false);
            m_prbSpec3->setChecked(true);
        }
        else
        {
            if(XFoilTask::bAlpha()) m_prbSpec1->setChecked(true);
            else                    m_prbSpec2->setChecked(true);
            m_prbSpec1->setEnabled(true);
            m_prbSpec2->setEnabled(true);
            m_prbSpec3->setEnabled(false);
            m_prbSpec4->setEnabled(false);
        }
    }
    else
    {
        if(XFoilTask::bAlpha())
        {
            m_prbSpec1->setChecked(true);
            m_pRangeTable->setRangeType(AnalysisRange::ALPHA);
        }
        else
        {
            m_prbSpec2->setChecked(true);
            m_pRangeTable->setRangeType(AnalysisRange::CL);
        }
        m_prbSpec1->setEnabled(false);
        m_prbSpec2->setEnabled(false);
        m_prbSpec3->setEnabled(false);
        m_prbSpec4->setEnabled(false);
    }

    m_ppbAnalyze->setEnabled(pCurPolar);
    m_pchViscous->setEnabled(pCurPolar);
    m_pchInitBL->setEnabled(pCurPolar);
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
        if     (m_prbSpec1->isChecked()) m_pRangeTable->setRangeType(AnalysisRange::ALPHA);
        else if(m_prbSpec2->isChecked()) m_pRangeTable->setRangeType(AnalysisRange::CL);
    }
    else if(pCurPolar->isType4()) m_pRangeTable->setRangeType(AnalysisRange::REYNOLDS);
    else if(pCurPolar->isType6()) m_pRangeTable->setRangeType(AnalysisRange::THETA);

    m_pRangeTable->fillTable();

}


void Analysis2dCtrls::enableAnalyze(bool b)
{
    m_ppbAnalyze->setEnabled(b);
}


