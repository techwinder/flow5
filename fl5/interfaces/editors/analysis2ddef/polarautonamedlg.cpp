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



#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>

#include "polarautonamedlg.h"

#include <api/polar.h>
#include <api/units.h>
#include <modules/xdirect/analysis/polarnamemaker.h>


PolarAutoNameDlg::PolarAutoNameDlg() : QDialog()
{
    m_pPolar = new Polar;
    setupLayout();
    connectSignals();
}


PolarAutoNameDlg::~PolarAutoNameDlg()
{
    delete m_pPolar;
}


void PolarAutoNameDlg::setupLayout()
{
    QFont fnt;
    QFontMetrics fm(fnt);
    setMinimumWidth(55*fm.averageCharWidth());

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QVBoxLayout *pDemoNameLayout = new QVBoxLayout;
        {
            QLabel *plabDemoNameExplain = new QLabel("Demo name:");
            m_plePolarName = new QLineEdit;
            m_plePolarName->setReadOnly(true);
            pDemoNameLayout->addWidget(plabDemoNameExplain);
            pDemoNameLayout->addWidget(m_plePolarName);
        }

        QGroupBox *pTypeBox = new QGroupBox("Polar type");
        {
            QHBoxLayout *pTypeLayout = new QHBoxLayout;
            {
                m_prbType1 = new QRadioButton("Type 1 (fixed speed)");
                m_prbType2 = new QRadioButton("Type 2 (fixed lift)");
                m_prbType3 = new QRadioButton("Type 3 (rubber chord)");
                m_prbType4 = new QRadioButton("Type 4 (fixed aoa)");
                pTypeLayout->addWidget(m_prbType1);
                pTypeLayout->addWidget(m_prbType2);
                pTypeLayout->addWidget(m_prbType3);
                pTypeLayout->addWidget(m_prbType4);
                pTypeLayout->addStretch();
            }
            pTypeBox->setLayout(pTypeLayout);
        }

        QGridLayout *pOptionsLayout = new QGridLayout;
        {
            QLabel *plabExplanation = new QLabel("Include in the automatic polar name:");
            m_pchType        = new QCheckBox("Type");
            m_pchBLMethod    = new QCheckBox("BL Method");
            m_pchReynolds    = new QCheckBox("Reynolds/Aoa");
            m_pchMach        = new QCheckBox("Mach");
            m_pchNCrit       = new QCheckBox("NCrit");
            m_pchXTrTop      = new QCheckBox("Top transition");
            m_pchXTrBot      = new QCheckBox("Bot transition");
            pOptionsLayout->addWidget(plabExplanation,    1,1);
            pOptionsLayout->addWidget(m_pchType,     2,1);
            pOptionsLayout->addWidget(m_pchBLMethod, 2,2);
            pOptionsLayout->addWidget(m_pchReynolds, 3,1);
            pOptionsLayout->addWidget(m_pchMach,     3,2);
            pOptionsLayout->addWidget(m_pchNCrit,    4,1);
            pOptionsLayout->addWidget(m_pchXTrTop,   4,2);
            pOptionsLayout->addWidget(m_pchXTrBot,   5,2);
        }

        QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        {
            // Close button has reject role for whatever reason
            connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }

        pMainLayout->addLayout(pOptionsLayout);
        pMainLayout->addStretch();
        pMainLayout->addSpacing(30);
        pMainLayout->addLayout(pDemoNameLayout);
        pMainLayout->addStretch();
        pMainLayout->addWidget(pButtonBox);
    }
    setLayout(pMainLayout);
}


void PolarAutoNameDlg::connectSignals()
{
    connect(m_prbType1,       SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
    connect(m_prbType2,       SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
    connect(m_prbType3,       SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
    connect(m_prbType4,       SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));

    connect(m_pchType,        SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
    connect(m_pchBLMethod,    SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
    connect(m_pchReynolds,    SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
    connect(m_pchMach,        SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
    connect(m_pchNCrit,       SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
    connect(m_pchXTrTop,      SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
    connect(m_pchXTrBot,      SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
}


void PolarAutoNameDlg::initDialog(Polar *pPolar)
{
    m_pPolar->setType(pPolar->type());

    if     (m_pPolar->isFixedSpeedPolar()) m_prbType1->setChecked(true);
    else if(m_pPolar->isFixedLiftPolar())  m_prbType2->setChecked(true);
    else if(m_pPolar->isFixedaoaPolar())   m_prbType3->setChecked(true);
    else if(m_pPolar->isFixedaoaPolar())   m_prbType4->setChecked(true);


    m_pchType->setChecked(PolarNameMaker::s_bType);
    m_pchBLMethod->setChecked(PolarNameMaker::s_bBLMethod);
    m_pchReynolds->setChecked(PolarNameMaker::s_bReynolds);
    m_pchMach->setChecked(PolarNameMaker::s_bMach);
    m_pchNCrit->setChecked(PolarNameMaker::s_bNCrit);
    m_pchXTrTop->setChecked(PolarNameMaker::s_bXTrTop);
    m_pchXTrBot->setChecked(PolarNameMaker::s_bXTrBot);

    m_plePolarName->setText(PolarNameMaker::makeName(m_pPolar));
}


void PolarAutoNameDlg::readData()
{
    if     (m_prbType1->isChecked()) m_pPolar->setType(xfl::T1POLAR);
    else if(m_prbType2->isChecked()) m_pPolar->setType(xfl::T2POLAR);
    else if(m_prbType3->isChecked()) m_pPolar->setType(xfl::T3POLAR);
    else if(m_prbType4->isChecked()) m_pPolar->setType(xfl::T4POLAR);

    PolarNameMaker::s_bType        = m_pchType->isChecked();
    PolarNameMaker::s_bBLMethod    = m_pchBLMethod->isChecked();
    PolarNameMaker::s_bReynolds    = m_pchReynolds->isChecked();
    PolarNameMaker::s_bMach        = m_pchMach->isChecked();
    PolarNameMaker::s_bNCrit       = m_pchNCrit->isChecked();
    PolarNameMaker::s_bXTrTop      = m_pchXTrTop->isChecked();
    PolarNameMaker::s_bXTrBot      = m_pchXTrBot->isChecked();
}


void PolarAutoNameDlg::onOptionChanged()
{
    readData();
    QString name = PolarNameMaker::makeName(m_pPolar);
    m_plePolarName->setText(name);
}


void PolarAutoNameDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("PolarAutoName");
    {
        PolarNameMaker::s_bType         = settings.value("Type", true).toBool();
        PolarNameMaker::s_bBLMethod     = settings.value("BLMethod", true).toBool();
        PolarNameMaker::s_bReynolds     = settings.value("Reynolds", true).toBool();
        PolarNameMaker::s_bMach         = settings.value("Mach", true).toBool();
        PolarNameMaker::s_bNCrit        = settings.value("NCrit", true).toBool();
        PolarNameMaker::s_bXTrTop       = settings.value("XTopTr", true).toBool();
        PolarNameMaker::s_bXTrBot       = settings.value("XBotTr", true).toBool();
    }
    settings.endGroup();
}



void PolarAutoNameDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("PolarAutoName");
    {
        settings.setValue("Type",     PolarNameMaker::s_bType);
        settings.setValue("BLMethod", PolarNameMaker::s_bBLMethod);
        settings.setValue("Reynolds", PolarNameMaker::s_bReynolds);
        settings.setValue("Mach",     PolarNameMaker::s_bMach);
        settings.setValue("NCrit",    PolarNameMaker::s_bNCrit);
        settings.setValue("XTopTr",   PolarNameMaker::s_bXTrTop);
        settings.setValue("XBotTr",   PolarNameMaker::s_bXTrBot);
    }
    settings.endGroup();
}


