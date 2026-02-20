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
#include <QPushButton>

#include "foilpolardlg.h"

#include <api/foil.h>
#include <api/geom_params.h>
#include <api/polar.h>
#include <api/utils.h>

#include <core/xflcore.h>
#include <interfaces/editors/analysis2ddef/polarautonamedlg.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <modules/xdirect/analysis/polarnamemaker.h>


QByteArray FoilPolarDlg::s_WindowGeometry;


Polar FoilPolarDlg::s_Polar;


FoilPolarDlg::FoilPolarDlg(QWidget *pParent) : XflDialog(pParent)
{
    setWindowTitle(tr("Foil analysis definition"));
    m_bAutoName = true;
    m_pFoil = nullptr;
    setupLayout();
    connectSignals();
}


void FoilPolarDlg::setupLayout()
{
    QGroupBox *pgbName = new QGroupBox(tr("Analysis name"));
    {
        QVBoxLayout *pNameLayout = new QVBoxLayout;
        {
            m_plabFoilName = new QLabel;
            QHBoxLayout *pAnalysisLayout = new QHBoxLayout;
            {
                m_pleAnalysisName = new QLineEdit;
                m_pchAutoName = new QCheckBox(tr("Default name"));
                QPushButton *pctrlOptions = new QPushButton(tr("Options"));
                pAnalysisLayout->addWidget(m_pchAutoName);
                pAnalysisLayout->addWidget(m_pleAnalysisName);
                pAnalysisLayout->addWidget(pctrlOptions);
                connect(pctrlOptions, SIGNAL(clicked(bool)), this, SLOT(onNameOptions()));
            }
            pNameLayout->addWidget(m_plabFoilName);
            pNameLayout->addLayout(pAnalysisLayout);
        }
        pgbName->setLayout(pNameLayout);
    }

    QGroupBox *pgbTransition = new QGroupBox(tr("Transitions"));
    {
        QGridLayout *pTransitionsLayout = new QGridLayout;
        {
            QLabel *plabFreeTrans    = new QLabel(tr("<p>Free transitions (e<sup>n</sup>) method</p>"));
            QLabel *plabForcedTrans  = new QLabel(tr("Forced transitions:"));
            QLabel *plabNCrit        = new QLabel(tr("NCrit="));
            QLabel *plabTopTrip      = new QLabel(tr("Trip location (top)"));
            QLabel *plabBotTrip      = new QLabel(tr("Trip location (bottom)"));
            m_pfeNCrit    = new FloatEdit();
            m_pfeTopTrans = new FloatEdit();
            m_pfeBotTrans = new FloatEdit();

            pTransitionsLayout->addWidget(plabFreeTrans,   1,1, 1,1, Qt::AlignLeft);
            pTransitionsLayout->addWidget(plabForcedTrans, 2,1, 1,1, Qt::AlignLeft);
            pTransitionsLayout->addWidget(plabNCrit,       1,2, 1,1, Qt::AlignRight);
            pTransitionsLayout->addWidget(plabTopTrip,     2,2, 1,1, Qt::AlignRight);
            pTransitionsLayout->addWidget(plabBotTrip,     3,2, 1,1, Qt::AlignRight);
            pTransitionsLayout->addWidget(m_pfeNCrit,      1,3, 1,1, Qt::AlignRight);
            pTransitionsLayout->addWidget(m_pfeTopTrans,   2,3, 1,1, Qt::AlignRight);
            pTransitionsLayout->addWidget(m_pfeBotTrans,   3,3, 1,1, Qt::AlignRight);
            pTransitionsLayout->setColumnStretch(4,1);
            pgbTransition->setLayout(pTransitionsLayout);
        }
    }

    QGroupBox *pgbType = new QGroupBox(tr("Analysis type"));
    {
        QHBoxLayout *pAnalysisTypeLayout = new QHBoxLayout;
        {
            m_prbType1 = new QRadioButton(tr("Type 1"));
            m_prbType2 = new QRadioButton(tr("Type 2"));
            m_prbType3 = new QRadioButton(tr("Type 3"));
            m_prbType4 = new QRadioButton(tr("Type 4"));
            m_prbType6 = new QRadioButton(tr("Type 6"));
            m_prbType1->setToolTip(tr("<p>Fixed Reynolds number polar</p>"));
            m_prbType2->setToolTip(tr("<p>Fixed Lift polar</p>"));
            m_prbType3->setToolTip(tr("<p>Rubber-chord polar</p>"));
            m_prbType4->setToolTip(tr("<p>Fixed angle of attack polar</p>"));
            m_prbType6->setToolTip(tr("<p>Control polar.<br> The control parameter is the T.E. flap angle &theta; (&deg;).</p>"));

            pAnalysisTypeLayout->addStretch();
            pAnalysisTypeLayout->addWidget(m_prbType1);
            pAnalysisTypeLayout->addWidget(m_prbType2);
            pAnalysisTypeLayout->addWidget(m_prbType3);
            pAnalysisTypeLayout->addWidget(m_prbType4);
            pAnalysisTypeLayout->addWidget(m_prbType6);
            pAnalysisTypeLayout->addStretch();
            pgbType->setLayout(pAnalysisTypeLayout);
        }
    }

    QGroupBox *pgbFlightSpec = new QGroupBox(tr("Operating point"));
    {
        QVBoxLayout *pFlightSpecLayout = new QVBoxLayout;
        {
            QGridLayout *pFlightDataLayout = new QGridLayout;
            {
                QLabel *plabAlpha = new QLabel(tr("<p>&alpha;=</p>"));
                QLabel *plabDeg   = new QLabel(tr("<p>&deg;</p>"));
                m_plabRe          = new QLabel(tr("Re ="));
                m_plabMach        = new QLabel(tr("Mach ="));

                m_pfeAlpha        = new FloatEdit;
                m_pfeReynolds     = new FloatEdit;
                m_pfeMach         = new FloatEdit;
                pFlightDataLayout->addWidget(plabAlpha,          1,1, Qt::AlignRight);
                pFlightDataLayout->addWidget(m_pfeAlpha,         1,2);
                pFlightDataLayout->addWidget(plabDeg,            1,3);
                pFlightDataLayout->addWidget(m_plabRe,           3,1, Qt::AlignRight);
                pFlightDataLayout->addWidget(m_pfeReynolds,      3,2);
                pFlightDataLayout->addWidget(m_plabMach,         4,1, Qt::AlignRight);
                pFlightDataLayout->addWidget(m_pfeMach,          4,2);
            }
            pFlightSpecLayout->addLayout(pFlightDataLayout);
        }
        pgbFlightSpec->setLayout(pFlightSpecLayout);
    }

    QGroupBox *pgbTEFLap = new QGroupBox(tr("T.E. flap"));
    {
        QVBoxLayout *pTELayout = new QVBoxLayout;
        {
            m_plabHinge = new QLabel;
            QHBoxLayout *pTEAngleLayout = new QHBoxLayout;
            {
                QLabel *plabTEFlapAngle = new QLabel(THETAch + tr("="));
                m_pfeTheta = new FloatEdit;
                m_pfeTheta->setToolTip(tr("<p>The trailing edge flap angle.<br>"
                                       "The flap angle is fixed in T1234 polars and is the analysis variable in T6 polars.</p>"));
                QLabel *plabDegree = new QLabel(tr("<p>&deg;</p>"));
                pTEAngleLayout->addStretch();
                pTEAngleLayout->addWidget(plabTEFlapAngle);
                pTEAngleLayout->addWidget(m_pfeTheta);
                pTEAngleLayout->addWidget(plabDegree);
                pTEAngleLayout->addStretch();
            }
            pTELayout->addWidget(m_plabHinge);
            pTELayout->addLayout(pTEAngleLayout);
        }
        pgbTEFLap->setLayout(pTELayout);
    }

    setButtons(QDialogButtonBox::Ok | QDialogButtonBox::Discard);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addStretch();
        pMainLayout->addWidget(pgbName);
        pMainLayout->addStretch();
        pMainLayout->addStretch();
        pMainLayout->addWidget(pgbType);
        pMainLayout->addStretch();
        pMainLayout->addWidget(pgbFlightSpec);
        pMainLayout->addStretch();
        pMainLayout->addWidget(pgbTransition);
        pMainLayout->addStretch();
        pMainLayout->addWidget(pgbTEFLap);
        pMainLayout->addStretch();
        pMainLayout->addWidget(m_pButtonBox);
        pMainLayout->addStretch();
    }

    setLayout(pMainLayout);
}


void FoilPolarDlg::showEvent(QShowEvent *pEvent)
{
    XflDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);
}


void FoilPolarDlg::hideEvent(QHideEvent *pEvent)
{
    XflDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();
}


void FoilPolarDlg::connectSignals()
{
    connect(m_pchAutoName,   SIGNAL(clicked(bool)),       SLOT(onAutoName(bool)));

    connect(m_pfeNCrit,      SIGNAL(floatChanged(float)), SLOT(onEditingFinished()));
    connect(m_pfeTopTrans,   SIGNAL(floatChanged(float)), SLOT(onEditingFinished()));
    connect(m_pfeBotTrans,   SIGNAL(floatChanged(float)), SLOT(onEditingFinished()));

    connect(m_pfeAlpha,      SIGNAL(floatChanged(float)), SLOT(onEditingFinished()));
    connect(m_pfeMach,       SIGNAL(floatChanged(float)), SLOT(onEditingFinished()));
    connect(m_pfeReynolds,   SIGNAL(floatChanged(float)), SLOT(onEditingFinished()));
    connect(m_pfeTheta,      SIGNAL(floatChanged(float)), SLOT(onEditingFinished()));

    connect(m_prbType1,      SIGNAL(clicked()),           SLOT(onPolarType()));
    connect(m_prbType2,      SIGNAL(clicked()),           SLOT(onPolarType()));
    connect(m_prbType3,      SIGNAL(clicked()),           SLOT(onPolarType()));
    connect(m_prbType4,      SIGNAL(clicked()),           SLOT(onPolarType()));
    connect(m_prbType6,      SIGNAL(clicked()),           SLOT(onPolarType()));
}


void FoilPolarDlg::initDialog(Foil const *pFoil, Polar *pPolar)
{
    m_pFoil = pFoil;
    m_plabFoilName->setText(QString::fromStdString(pFoil->name()));

    if(pPolar) s_Polar.copySpecification(pPolar);

    m_pfeNCrit->setValue(s_Polar.NCrit());
    m_pfeTopTrans->setValue(s_Polar.XTripTop());
    m_pfeBotTrans->setValue(s_Polar.XTripBot());


    m_pchAutoName->setChecked(m_bAutoName);
    m_pleAnalysisName->setEnabled(!m_bAutoName);
    if(pPolar) m_pleAnalysisName->setText(QString::fromStdString(pPolar->name()));

    m_pfeAlpha->setValue(s_Polar.aoaSpec());
    m_pfeMach->setValue(s_Polar.Mach());

    s_Polar.setBLMethod(BL::XFOIL);

    m_prbType6->setEnabled(m_pFoil->hasTEFlap());
    if(s_Polar.isType6() && !m_pFoil->hasTEFlap())
        s_Polar.setType(xfl::T1POLAR);

    switch(s_Polar.type())
    {
        case xfl::T1POLAR:  m_prbType1->setChecked(true); break;
        case xfl::T2POLAR:  m_prbType2->setChecked(true); break;
        case xfl::T4POLAR:  m_prbType4->setChecked(true); break;
        case xfl::T6POLAR:  m_prbType6->setChecked(true); break;
        default:            m_prbType1->setChecked(true); break;
    }

    onPolarType();

    m_pfeReynolds->setValue(s_Polar.Reynolds());

    QString strange;
    if(m_pFoil->hasTEFlap())
        strange = tr("T.E. flap hinge position = (%1%, %2%)\n")
                      .arg(pFoil->TEXHinge()*100.0)
                      .arg(pFoil->TEYHinge()*100.0);
    else
        strange = tr("No T.E. flap");
    m_plabHinge->setText(strange);

    if(pFoil->hasTEFlap())
    {
        m_pfeTheta->setEnabled(true);
        m_pfeTheta->setValue(pFoil->TEFlapAngle());
    }
    else
    {
        m_pfeTheta->setEnabled(false);
        m_pfeTheta->setValue(0.0);
    }

    enableControls();
}


void FoilPolarDlg::onPolarType()
{
    if(m_prbType1->isChecked())
    {
        m_plabRe->setText(tr("Reynolds ="));
        m_plabMach->setText(tr("Mach ="));
        s_Polar.setType(xfl::T1POLAR);
    }
    else if(m_prbType2->isChecked())
    {
        m_plabRe->setText(tr("Re.sqrt(Cl) ="));
        m_plabMach->setText(tr("Ma.sqrt(Cl) ="));
        s_Polar.setType(xfl::T2POLAR);
    }
    else if(m_prbType3->isChecked())
    {
        m_plabRe->setText(tr("Re.Cl ="));
        m_plabMach->setText(tr("Mach ="));
        s_Polar.setType(xfl::T3POLAR);
    }
    else if(m_prbType4->isChecked())
    {
        m_plabMach->setText(tr("Mach ="));
        s_Polar.setType(xfl::T4POLAR);
    }
    else if(m_prbType6->isChecked())
    {
        m_plabMach->setText(tr("Mach ="));
        s_Polar.setType(xfl::T6POLAR);
    }

    enableControls();
    setPlrName();
}


void FoilPolarDlg::enableControls()
{
    switch(s_Polar.type())
    {
        default:
        case xfl::T1POLAR:
        case xfl::T2POLAR:
        case xfl::T3POLAR:
            m_pfeAlpha->setEnabled(false);
            m_pfeReynolds->setEnabled(true);
            break;
        case xfl::T4POLAR:
            m_pfeAlpha->setEnabled(true);
            m_pfeReynolds->setEnabled(false);
            break;
        case xfl::T6POLAR:
            m_pfeAlpha->setEnabled(true);
            m_pfeReynolds->setEnabled(true);
            break;
    }

    if(!m_pFoil->hasTEFlap())
    {
        m_prbType6->setEnabled(false);
        m_pfeTheta->setValue(0.0);
    }
    m_pfeTheta->setEnabled(m_pFoil->hasTEFlap() && !s_Polar.isType6());
}


void FoilPolarDlg::readData()
{
    s_Polar.setName(m_pleAnalysisName->text().toStdString());

    s_Polar.setBLMethod(BL::XFOIL);


    s_Polar.m_ACrit    = m_pfeNCrit->value();
    s_Polar.m_XTripTop = m_pfeTopTrans->value();
    s_Polar.m_XTripBot = m_pfeBotTrans->value();


    if     (m_prbType1->isChecked()) s_Polar.setType(xfl::T1POLAR);
    else if(m_prbType2->isChecked()) s_Polar.setType(xfl::T2POLAR);
    else if(m_prbType3->isChecked()) s_Polar.setType(xfl::T3POLAR);
    else if(m_prbType4->isChecked()) s_Polar.setType(xfl::T4POLAR);
    else if(m_prbType6->isChecked()) s_Polar.setType(xfl::T6POLAR);

    s_Polar.m_aoaSpec  = m_pfeAlpha->value();
    s_Polar.m_Reynolds = m_pfeReynolds->value();
    s_Polar.m_Mach     = m_pfeMach->value();

    if(s_Polar.isType4())
    {
        s_Polar.setAoaSpec(m_pfeAlpha->value());
    }

    double theta = m_pfeTheta->value();
    if(fabs(theta)>FLAPANGLEPRECISION) s_Polar.setTEFlapAngle(theta);
    else                               s_Polar.setTEFlapAngle(0.0);
}


void FoilPolarDlg::onEditingFinished()
{
    readData();
    setPlrName();

    m_pfeReynolds->setValue(s_Polar.Reynolds());

    setPlrName();
}


void FoilPolarDlg::onAutoName(bool bChecked)
{
    m_bAutoName = bChecked;
    if(!m_bAutoName)
    {
        m_pleAnalysisName->setFocus();
        m_pleAnalysisName->selectAll();
    }
    else setPlrName();

    m_pleAnalysisName->setEnabled(!m_bAutoName);
}


void FoilPolarDlg::onNameOptions()
{
    PolarAutoNameDlg dlg;
    dlg.initDialog(&s_Polar);
    dlg.exec();
    setPlrName();
}


void FoilPolarDlg::setPlrName()
{
    if(m_bAutoName)
    {
        s_Polar.setName(PolarNameMaker::makeName(&s_Polar).toStdString());
        m_pleAnalysisName->setText(QString::fromStdString(s_Polar.name()));
    }
}


void FoilPolarDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)
    {
        readData();
        s_Polar.setName(m_pleAnalysisName->text().toStdString());
        if(s_Polar.isType6()) s_Polar.setTEFlapAngle(0.0);
    }
    XflDialog::onButton(pButton);
}


void FoilPolarDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("FoilPolarDlg");
    {
        s_WindowGeometry = settings.value("WindowGeom", QByteArray()).toByteArray();

        s_Polar.setNCrit(     settings.value("NCrit",     9.0).toDouble());
        s_Polar.setXTripTop(  settings.value("XTopTr",    1.0).toDouble());
        s_Polar.setXTripBot(  settings.value("XBotTr",    1.0).toDouble());
        s_Polar.setReynolds(  settings.value("Reynolds",  1.e6).toDouble());
        s_Polar.setMach(      settings.value("Mach",      0.0).toDouble());
        s_Polar.setAoaSpec(       settings.value("ASpec",     0.0).toDouble());

        int m = settings.value("BLMethod", 1).toInt();
        switch(m)
        {
            default:
            case 1: s_Polar.setBLMethod(BL::XFOIL);         break;
        }

        int b = settings.value("Type", 1).toInt();
        if     (b==1) s_Polar.setType(xfl::T1POLAR);
        else if(b==2) s_Polar.setType(xfl::T2POLAR);
        else if(b==3) s_Polar.setType(xfl::T3POLAR);
        else if(b==4) s_Polar.setType(xfl::T4POLAR);
        else if(b==6) s_Polar.setType(xfl::T6POLAR);
    }
    settings.endGroup();
}



void FoilPolarDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("FoilPolarDlg");
    {
        settings.setValue("NCrit",    s_Polar.NCrit());
        settings.setValue("XTopTr",   s_Polar.XTripTop());
        settings.setValue("XBotTr",   s_Polar.XTripBot());
        settings.setValue("Reynolds", s_Polar.Reynolds());
        settings.setValue("Mach",     s_Polar.Mach());
        settings.setValue("ASpec",    s_Polar.aoaSpec());

        switch(s_Polar.BLMethod())
        {
            default:
            case BL::XFOIL:         settings.setValue("BLMethod", 1);  break;
        }

        if     (s_Polar.isFixedSpeedPolar())  settings.setValue("Type", 1);
        else if(s_Polar.isFixedLiftPolar())   settings.setValue("Type", 2);
        else if(s_Polar.isRubberChordPolar()) settings.setValue("Type", 3);
        else if(s_Polar.isFixedaoaPolar())    settings.setValue("Type", 4);
        else if(s_Polar.isControlPolar())     settings.setValue("Type", 6);

        settings.setValue("WindowGeom", s_WindowGeometry);

    }
    settings.endGroup();
}








