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

#include "wpolarautonamedlg.h"

#include <api/planepolar.h>
#include <api/planexfl.h>
#include <api/units.h>
#include <api/planepolarnamemaker.h>




WPolarAutoNameDlg::WPolarAutoNameDlg() : QDialog()
{
    setWindowTitle(tr("Polar name options"));

    //make a dummy plane
    m_pPlane = new PlaneXfl;
    m_pPlane->makeDefaultPlane();

    //make a dummy full option polar
    m_pWPolar = new PlanePolar;

    m_pWPolar->setAutoInertia(false);
    {
        m_pWPolar->setMass(1.0);
        m_pWPolar->setCoG({0.1,0.0,0.01});
    }

    m_pWPolar->setReferenceArea(m_pPlane->mainWing()->projectedArea());
    m_pWPolar->setReferenceChordLength(m_pPlane->mainWing()->MAC());
    m_pWPolar->setReferenceSpanLength(m_pPlane->mainWing()->planformSpan());

    // add extra drag
    std::string strange = "ExtraDrag0";
    double area = 1.0;
    double coef = 0.005;
    m_pWPolar->appendExtraDrag({strange, area, coef});

    // add fuse drag
    m_pWPolar->setIncludeFuseDrag(true);
    //    m_pWPolar->setFuseFormFactor(m_pPlane->fuse(0)->formFactor());
    m_pWPolar->setFuseDragMethod(PlanePolar::KARMANSCHOENHERR);

    // add T6 range
    strange = "Velocity";
    m_pWPolar->m_OperatingRange[0].setName("Velocity");
    m_pWPolar->m_OperatingRange[0].setCtrlMin(10.0);
    m_pWPolar->m_OperatingRange[0].setCtrlMax(50.0);
    strange = "aoa";
    m_pWPolar->m_OperatingRange[1].setName("aoa");
    m_pWPolar->m_OperatingRange[1].setCtrlMin(0.0);
    m_pWPolar->m_OperatingRange[1].setCtrlMax(10.0);

    setupLayout();
    connectSignals();
}


WPolarAutoNameDlg::~WPolarAutoNameDlg()
{
    delete m_pWPolar;
    delete m_pPlane;
}


void WPolarAutoNameDlg::setupLayout()
{
    QFont fnt;
    QFontMetrics fm(fnt);
    setMinimumWidth(55*fm.averageCharWidth());

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_plePolarName = new QLineEdit;
        m_plePolarName->setReadOnly(true);

        QGroupBox *pTypeBox = new QGroupBox(tr("Polar type"));
        {
            QHBoxLayout *pTypeLayout = new QHBoxLayout;
            {
                m_prbType1 = new QRadioButton(tr("Type 1 (fixed speed)"));
                m_prbType2 = new QRadioButton(tr("Type 2 (fixed lift)"));
                m_prbType6 = new QRadioButton(tr("Type 6 (control polar)"));
                m_prbType7 = new QRadioButton(tr("Type 7 (stability polar)"));
                pTypeLayout->addWidget(m_prbType1);
                pTypeLayout->addWidget(m_prbType2);
                pTypeLayout->addWidget(m_prbType6);
                pTypeLayout->addWidget(m_prbType7);
                pTypeLayout->addStretch();
            }
            pTypeBox->setLayout(pTypeLayout);
        }

        QGridLayout *pOptionsLayout = new QGridLayout;
        {
            m_pchType        = new QCheckBox(tr("Type"));
            m_pchMethod      = new QCheckBox(tr("Method"));
            m_pchSurfaces    = new QCheckBox(tr("Thin wing surfaces"));
            m_pchControls    = new QCheckBox(tr("Control data"));
            m_pchViscosity   = new QCheckBox(tr("Viscosity"));
            m_pchInertia     = new QCheckBox(tr("Inertia"));
            m_pchExtraDrag   = new QCheckBox(tr("Extra drag"));
            m_pchFuseDrag    = new QCheckBox(tr("Fuse drag"));
            m_pchGround      = new QCheckBox(tr("Ground/free surface"));
            pOptionsLayout->addWidget(m_pchType,      1, 1);
            pOptionsLayout->addWidget(m_pchViscosity, 1, 2);
            pOptionsLayout->addWidget(m_pchMethod,    2, 1);
            pOptionsLayout->addWidget(m_pchSurfaces,  2, 2);
            pOptionsLayout->addWidget(m_pchInertia,   3, 1);
            pOptionsLayout->addWidget(m_pchControls,  4, 1);
            pOptionsLayout->addWidget(m_pchExtraDrag, 3, 2);
            pOptionsLayout->addWidget(m_pchFuseDrag,  4, 2);
            pOptionsLayout->addWidget(m_pchGround,    5, 2);
        }


        QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        {
            // Close button has reject role for whatever reason
            connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }

        pMainLayout->addWidget(pTypeBox);
        pMainLayout->addWidget(m_plePolarName);
        pMainLayout->addLayout(pOptionsLayout);
        pMainLayout->addWidget(pButtonBox);
    }
    setLayout(pMainLayout);
}


void WPolarAutoNameDlg::connectSignals()
{
    connect(m_prbType1,       SIGNAL(clicked(bool)), SLOT(onOptionChanged()));
    connect(m_prbType2,       SIGNAL(clicked(bool)), SLOT(onOptionChanged()));
    connect(m_prbType6,       SIGNAL(clicked(bool)), SLOT(onOptionChanged()));
    connect(m_prbType7,       SIGNAL(clicked(bool)), SLOT(onOptionChanged()));

    connect(m_pchType,        SIGNAL(clicked(bool)), SLOT(onOptionChanged()));
    connect(m_pchMethod,      SIGNAL(clicked(bool)), SLOT(onOptionChanged()));
    connect(m_pchSurfaces,    SIGNAL(clicked(bool)), SLOT(onOptionChanged()));
    connect(m_pchControls,    SIGNAL(clicked(bool)), SLOT(onOptionChanged()));
    connect(m_pchViscosity,   SIGNAL(clicked(bool)), SLOT(onOptionChanged()));
    connect(m_pchInertia,     SIGNAL(clicked(bool)), SLOT(onOptionChanged()));
    connect(m_pchExtraDrag,   SIGNAL(clicked(bool)), SLOT(onOptionChanged()));
    connect(m_pchFuseDrag,    SIGNAL(clicked(bool)), SLOT(onOptionChanged()));
    connect(m_pchGround,      SIGNAL(clicked(bool)), SLOT(onOptionChanged()));
}



void WPolarAutoNameDlg::initDialog(PlanePolar const &wPolar)
{
    m_pWPolar->setType(wPolar.type());
    if     (m_pWPolar->isFixedSpeedPolar()) m_prbType1->setChecked(true);
    else if(m_pWPolar->isFixedLiftPolar())  m_prbType2->setChecked(true);
    else if(m_pWPolar->isControlPolar())    m_prbType6->setChecked(true);
    else if(m_pWPolar->isStabilityPolar())  m_prbType7->setChecked(true);

    m_pchType->setChecked(PlanePolarNameMaker::s_bType);
    m_pchMethod->setChecked(PlanePolarNameMaker::s_bMethod);
    m_pchSurfaces->setChecked(PlanePolarNameMaker::s_bSurfaces);
    m_pchControls->setChecked(PlanePolarNameMaker::s_bControls);
    m_pchViscosity->setChecked(PlanePolarNameMaker::s_bViscosity);
    m_pchInertia->setChecked(PlanePolarNameMaker::s_bInertia);
    m_pchExtraDrag->setChecked(PlanePolarNameMaker::s_bExtraDrag);
    m_pchFuseDrag->setChecked(PlanePolarNameMaker::s_bFuseDrag);
    m_pchGround->setChecked(PlanePolarNameMaker::s_bGround);

    m_plePolarName->setText(PlanePolarNameMaker::makeName(m_pPlane, m_pWPolar));
}


void WPolarAutoNameDlg::readData()
{
    if     (m_prbType1->isChecked()) m_pWPolar->setType(xfl::T1POLAR);
    else if(m_prbType2->isChecked()) m_pWPolar->setType(xfl::T2POLAR);
    else if(m_prbType6->isChecked()) m_pWPolar->setType(xfl::T6POLAR);
    else if(m_prbType7->isChecked()) m_pWPolar->setType(xfl::T7POLAR);

    PlanePolarNameMaker::s_bType      = m_pchType->isChecked();
    PlanePolarNameMaker::s_bMethod    = m_pchMethod->isChecked();
    PlanePolarNameMaker::s_bSurfaces  = m_pchSurfaces->isChecked();
    PlanePolarNameMaker::s_bControls  = m_pchControls->isChecked();
    PlanePolarNameMaker::s_bViscosity = m_pchViscosity->isChecked();
    PlanePolarNameMaker::s_bInertia   = m_pchInertia->isChecked();
    PlanePolarNameMaker::s_bExtraDrag = m_pchExtraDrag->isChecked();
    PlanePolarNameMaker::s_bFuseDrag  = m_pchFuseDrag->isChecked();
    PlanePolarNameMaker::s_bGround    = m_pchGround->isChecked();
}


void WPolarAutoNameDlg::onOptionChanged()
{
    readData();
    QString name = PlanePolarNameMaker::makeName(m_pPlane, m_pWPolar);
    m_plePolarName->setText(name);
}


void WPolarAutoNameDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("WPolarAutoName");
    {
        PlanePolarNameMaker::s_bType       = settings.value("Type", true).toBool();
        PlanePolarNameMaker::s_bMethod     = settings.value("Method", true).toBool();
        PlanePolarNameMaker::s_bSurfaces   = settings.value("Surfaces", true).toBool();
        PlanePolarNameMaker::s_bBC         = settings.value("BC", false).toBool();
        PlanePolarNameMaker::s_bControls   = settings.value("Controls", true).toBool();
        PlanePolarNameMaker::s_bViscosity  = settings.value("Viscosity", true).toBool();
        PlanePolarNameMaker::s_bInertia    = settings.value("Inertia", true).toBool();
        PlanePolarNameMaker::s_bExtraDrag  = settings.value("ExtraDrag", false).toBool();
        PlanePolarNameMaker::s_bFuseDrag   = settings.value("FuseDrag", false).toBool();
        PlanePolarNameMaker::s_bGround     = settings.value("GroundFS", false).toBool();
    }
    settings.endGroup();
}



void WPolarAutoNameDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("WPolarAutoName");
    {
        settings.setValue("Type", PlanePolarNameMaker::s_bType);
        settings.setValue("Method", PlanePolarNameMaker::s_bMethod);
        settings.setValue("Surfaces", PlanePolarNameMaker::s_bSurfaces);
        settings.setValue("BC", PlanePolarNameMaker::s_bBC);
        settings.setValue("Controls", PlanePolarNameMaker::s_bControls);
        settings.setValue("Viscosity", PlanePolarNameMaker::s_bViscosity);
        settings.setValue("Inertia", PlanePolarNameMaker::s_bInertia);
        settings.setValue("ExtraDrag", PlanePolarNameMaker::s_bExtraDrag);
        settings.setValue("FuseDrag", PlanePolarNameMaker::s_bFuseDrag);
        settings.setValue("GroundFS", PlanePolarNameMaker::s_bGround);
    }
    settings.endGroup();
}

