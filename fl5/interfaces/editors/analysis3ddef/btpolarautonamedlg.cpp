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


#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGroupBox>

#include "btpolarautonamedlg.h"

#include <interfaces/editors/analysis3ddef/btpolarnamemaker.h>
#include <api/boatpolar.h>
#include <api/boat.h>
#include <api/sail.h>
#include <api/units.h>


BtPolarAutoNameDlg::BtPolarAutoNameDlg() : QDialog()
{
    setWindowTitle("Polar name options");

    //make a dummy plane
    m_pBoat = new Boat;
    m_pBoat->makeDefaultBoat();

    //make a dummy full option polar
    m_pBtPolar = new BoatPolar;

    m_pBtPolar->setAutoInertia(false);
    {
        m_pBtPolar->setMass(1.0);
        m_pBtPolar->setCoG({0.1,0.0,0.01});
    }


    // add extra drag
    std::string strange = "ExtraDrag0";
    double area = 1.0;
    double coef = 0.005;
    m_pBtPolar->appendExtraDrag({strange, area, coef});

    // add control data
    int nSails = m_pBoat->nSails();
    m_pBtPolar->m_SailAngleMin.clear();
    m_pBtPolar->m_SailAngleMax.clear();
    for(int is=0; is<nSails; is++)
    {
        m_pBtPolar->m_SailAngleMin.push_back(0.0);
        m_pBtPolar->m_SailAngleMax.push_back(15.0);
    }

    // add T6 range
    strange = "Velocity";
    m_pBtPolar->m_TWSMin = 10.0;
    m_pBtPolar->m_TWSMax = 20.0;

    strange = "beta";
    m_pBtPolar->m_TWAMin = 0.0;
    m_pBtPolar->m_TWAMax = 45.0;

    strange = "phi";
    m_pBtPolar->m_TWAMin = 0.0;
    m_pBtPolar->m_TWAMax = 30.0;

    setupLayout();
    connectSignals();
}


BtPolarAutoNameDlg::~BtPolarAutoNameDlg()
{
    delete m_pBtPolar;
    delete m_pBoat;
}


void BtPolarAutoNameDlg::setupLayout()
{
    QFont fnt;
    QFontMetrics fm(fnt);
    setMinimumWidth(55*fm.averageCharWidth());

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_plePolarName = new QLineEdit;
        m_plePolarName->setReadOnly(true);


        QGridLayout *pOptionsLayout = new QGridLayout;
        {
            m_pchMethod      = new QCheckBox("Method");
            m_pchControls    = new QCheckBox("Control data");
            m_pchViscosity   = new QCheckBox("Viscosity");
            m_pchExtraDrag   = new QCheckBox("Extra drag");
            m_pchGround      = new QCheckBox("Ground effect");
            pOptionsLayout->addWidget(m_pchMethod,    1, 1);
            pOptionsLayout->addWidget(m_pchViscosity, 1, 2);
            pOptionsLayout->addWidget(m_pchControls,  1, 3);
            pOptionsLayout->addWidget(m_pchExtraDrag, 2, 1);
            pOptionsLayout->addWidget(m_pchGround,    2, 2);
        }


        QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        {
            // Close button has reject role for whatever reason
            connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }

        pMainLayout->addWidget(m_plePolarName);
        pMainLayout->addLayout(pOptionsLayout);
        pMainLayout->addWidget(pButtonBox);
    }
    setLayout(pMainLayout);
}


void BtPolarAutoNameDlg::connectSignals()
{
    connect(m_pchMethod,      SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
    connect(m_pchControls,    SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
    connect(m_pchViscosity,   SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
    connect(m_pchExtraDrag,   SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
    connect(m_pchGround,      SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
}


void BtPolarAutoNameDlg::onOptionChanged()
{
    readData();
    QString name = BtPolarNameMaker::makeName(m_pBoat, m_pBtPolar);
    m_plePolarName->setText(name);
}


void BtPolarAutoNameDlg::initDialog(Boat const &boat, BoatPolar &btPolar)
{
    m_pBtPolar->setType(btPolar.type());

    m_pBtPolar->duplicateSpec(&btPolar);
    m_pBoat->duplicate(&boat);

    m_pchMethod->setChecked(BtPolarNameMaker::s_bMethod);
    m_pchControls->setChecked(BtPolarNameMaker::s_bControls);
    m_pchViscosity->setChecked(BtPolarNameMaker::s_bViscosity);
    m_pchExtraDrag->setChecked(BtPolarNameMaker::s_bExtraDrag);
    m_pchGround->setChecked(BtPolarNameMaker::s_bGround);

    m_plePolarName->setText(BtPolarNameMaker::makeName(m_pBoat, m_pBtPolar));
}


void BtPolarAutoNameDlg::readData()
{
    m_pBtPolar->setType(xfl::T6POLAR);
    BtPolarNameMaker::s_bMethod    = m_pchMethod->isChecked();
    BtPolarNameMaker::s_bControls  = m_pchControls->isChecked();
    BtPolarNameMaker::s_bViscosity = m_pchViscosity->isChecked();
    BtPolarNameMaker::s_bExtraDrag = m_pchExtraDrag->isChecked();
    BtPolarNameMaker::s_bGround    = m_pchGround->isChecked();
}



void BtPolarAutoNameDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("BtPolarAutoNameDlg");
    {
        BtPolarNameMaker::s_bMethod     = settings.value("Method", true).toBool();
        BtPolarNameMaker::s_bBC         = settings.value("BC", false).toBool();
        BtPolarNameMaker::s_bControls   = settings.value("Controls", true).toBool();
        BtPolarNameMaker::s_bViscosity  = settings.value("Viscosity", true).toBool();
        BtPolarNameMaker::s_bInertia    = settings.value("Inertia", true).toBool();
        BtPolarNameMaker::s_bExtraDrag  = settings.value("ExtraDrag", false).toBool();
        BtPolarNameMaker::s_bGround     = settings.value("GroundEffect", false).toBool();
    }
    settings.endGroup();
}



void BtPolarAutoNameDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("BtPolarAutoNameDlg");
    {
        settings.setValue("Method",       BtPolarNameMaker::s_bMethod);
        settings.setValue("BC",           BtPolarNameMaker::s_bBC);
        settings.setValue("Controls",     BtPolarNameMaker::s_bControls);
        settings.setValue("Viscosity",    BtPolarNameMaker::s_bViscosity);
        settings.setValue("Inertia",      BtPolarNameMaker::s_bInertia);
        settings.setValue("ExtraDrag",    BtPolarNameMaker::s_bExtraDrag);
        settings.setValue("GroundEffect", BtPolarNameMaker::s_bGround);
    }
    settings.endGroup();
}


