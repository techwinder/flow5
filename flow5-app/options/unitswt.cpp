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


#include <QGridLayout>
#include <QVBoxLayout>
#include <QFontDatabase>
#include <QGroupBox>

#include "unitswt.h"

#include <api/units.h>

#include <core/xflcore.h>


UnitsWt::UnitsWt(QWidget *parent): QWidget(parent)
{
    setWindowTitle(tr("Units"));
    setupLayout();
}


void UnitsWt::setupLayout()
{
    QFont fixedfnt(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGroupBox *pgbConversion = new QGroupBox(tr("Conversion factors"));
        {
            QGridLayout *pConversionLayout = new QGridLayout;
            {
                QLabel *plab1 = new QLabel(tr("Length:"));
                QLabel *plab2 = new QLabel(tr("Area:"));
                QLabel *plab3 = new QLabel(tr("Velocity:"));
                QLabel *plab4 = new QLabel(tr("Mass:"));
                QLabel *plab5 = new QLabel(tr("Force:"));
                QLabel *plab6 = new QLabel(tr("Moment:"));
                QLabel *plab7 = new QLabel(tr("Pressure:"));
                QLabel *plab8 = new QLabel(tr("Inertia:"));
                plab1->setFont(fixedfnt);
                plab2->setFont(fixedfnt);
                plab3->setFont(fixedfnt);
                plab4->setFont(fixedfnt);
                plab5->setFont(fixedfnt);
                plab6->setFont(fixedfnt);
                plab7->setFont(fixedfnt);
                plab8->setFont(fixedfnt);
                pConversionLayout->addWidget(plab1, 1,1, Qt::AlignRight);
                pConversionLayout->addWidget(plab2, 2,1, Qt::AlignRight);
                pConversionLayout->addWidget(plab3, 3,1, Qt::AlignRight);
                pConversionLayout->addWidget(plab4, 4,1, Qt::AlignRight);
                pConversionLayout->addWidget(plab5, 5,1, Qt::AlignRight);
                pConversionLayout->addWidget(plab6, 6,1, Qt::AlignRight);
                pConversionLayout->addWidget(plab7, 7,1, Qt::AlignRight);
                pConversionLayout->addWidget(plab8, 8,1, Qt::AlignRight);


                m_plabLengthFactor   = new QLabel;
                m_plabSurfaceFactor  = new QLabel;
                m_plabWeightFactor   = new QLabel;
                m_plabSpeedFactor    = new QLabel;
                m_plabForceFactor    = new QLabel;
                m_plabMomentFactor   = new QLabel;
                m_plabPressureFactor = new QLabel;
                m_plabInertiaFactor  = new QLabel;

                m_plabLengthFactor->setFont(fixedfnt);
                m_plabSurfaceFactor->setFont(fixedfnt);
                m_plabWeightFactor->setFont(fixedfnt);
                m_plabSpeedFactor->setFont(fixedfnt);
                m_plabForceFactor->setFont(fixedfnt);
                m_plabMomentFactor->setFont(fixedfnt);
                m_plabPressureFactor->setFont(fixedfnt);
                m_plabInertiaFactor->setFont(fixedfnt);

                pConversionLayout->addWidget(m_plabLengthFactor,   1,2, Qt::AlignRight);
                pConversionLayout->addWidget(m_plabSurfaceFactor,  2,2, Qt::AlignRight);
                pConversionLayout->addWidget(m_plabSpeedFactor,    3,2, Qt::AlignRight);
                pConversionLayout->addWidget(m_plabWeightFactor,   4,2, Qt::AlignRight);
                pConversionLayout->addWidget(m_plabForceFactor,    5,2, Qt::AlignRight);
                pConversionLayout->addWidget(m_plabMomentFactor,   6,2, Qt::AlignRight);
                pConversionLayout->addWidget(m_plabPressureFactor, 7,2, Qt::AlignRight);
                pConversionLayout->addWidget(m_plabInertiaFactor,  8,2, Qt::AlignRight);

                m_pcbLength    = new QComboBox;
                m_pcbSurface   = new QComboBox;
                m_pcbSpeed     = new QComboBox;
                m_pcbWeight    = new QComboBox;
                m_pcbForce     = new QComboBox;
                m_pcbMoment    = new QComboBox;
                m_pcbPressure  = new QComboBox;
                m_pcbInertia   = new QComboBox;
                pConversionLayout->addWidget(m_pcbLength,   1,3);
                pConversionLayout->addWidget(m_pcbSurface,  2,3);
                pConversionLayout->addWidget(m_pcbSpeed,    3,3);
                pConversionLayout->addWidget(m_pcbWeight,   4,3);
                pConversionLayout->addWidget(m_pcbForce,    5,3);
                pConversionLayout->addWidget(m_pcbMoment,   6,3);
                pConversionLayout->addWidget(m_pcbPressure, 7,3);
                pConversionLayout->addWidget(m_pcbInertia,  8,3);


                m_plabLengthInvFactor   = new QLabel;
                m_plabSurfaceInvFactor  = new QLabel;
                m_plabWeightInvFactor   = new QLabel;
                m_plabSpeedInvFactor    = new QLabel;
                m_plabForceInvFactor    = new QLabel;
                m_plabMomentInvFactor   = new QLabel;
                m_plabPressureInvFactor = new QLabel;
                m_plabInertiaInvFactor  = new QLabel;
                m_plabLengthInvFactor->setFont(fixedfnt);
                m_plabSurfaceInvFactor->setFont(fixedfnt);
                m_plabWeightInvFactor->setFont(fixedfnt);
                m_plabSpeedInvFactor->setFont(fixedfnt);
                m_plabForceInvFactor->setFont(fixedfnt);
                m_plabMomentInvFactor->setFont(fixedfnt);
                m_plabPressureInvFactor->setFont(fixedfnt);
                m_plabInertiaInvFactor->setFont(fixedfnt);

                pConversionLayout->addWidget(m_plabLengthInvFactor,   1,4, Qt::AlignRight);
                pConversionLayout->addWidget(m_plabSurfaceInvFactor,  2,4, Qt::AlignRight);
                pConversionLayout->addWidget(m_plabSpeedInvFactor,    3,4, Qt::AlignRight);
                pConversionLayout->addWidget(m_plabWeightInvFactor,   4,4, Qt::AlignRight);
                pConversionLayout->addWidget(m_plabForceInvFactor,    5,4, Qt::AlignRight);
                pConversionLayout->addWidget(m_plabMomentInvFactor,   6,4, Qt::AlignRight);
                pConversionLayout->addWidget(m_plabPressureInvFactor, 7,4, Qt::AlignRight);
                pConversionLayout->addWidget(m_plabInertiaInvFactor,  8,4, Qt::AlignRight);
                pConversionLayout->setColumnStretch(4,2);
            }
            pgbConversion->setLayout(pConversionLayout);
        }

        QGroupBox *pgbFluid = new QGroupBox("Fluid density and kinematic viscosity");
        {
            QVBoxLayout *pFluidUnitLayout = new QVBoxLayout;
            {
                QHBoxLayout *pselLayout = new QHBoxLayout;
                {
                    QLabel *plab9 = new QLabel("Unit:");
                    plab9->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    m_prbUnit1 = new QRadioButton("International");
                    m_prbUnit2 = new QRadioButton("Imperial");
                    pselLayout->addWidget(plab9);
                    pselLayout->addWidget(m_prbUnit1);
                    pselLayout->addWidget(m_prbUnit2);
                    pselLayout->addStretch();
                }

                m_plabFluidUnit = new QLabel;
                m_plabFluidUnit->setFont(fixedfnt);
                pFluidUnitLayout->addLayout(pselLayout);
                pFluidUnitLayout->addWidget(m_plabFluidUnit);
            }
            pgbFluid->setLayout(pFluidUnitLayout);
        }
        pMainLayout->addWidget(pgbConversion);
        pMainLayout->addWidget(pgbFluid);
        pMainLayout->addStretch();
    }
    setLayout(pMainLayout);

    connect(m_pcbLength,   SIGNAL(activated(int)), SLOT(onSelChanged()));
    connect(m_pcbSurface,  SIGNAL(activated(int)), SLOT(onSelChanged()));
    connect(m_pcbSpeed,    SIGNAL(activated(int)), SLOT(onSelChanged()));
    connect(m_pcbWeight,   SIGNAL(activated(int)), SLOT(onSelChanged()));
    connect(m_pcbForce,    SIGNAL(activated(int)), SLOT(onSelChanged()));
    connect(m_pcbMoment,   SIGNAL(activated(int)), SLOT(onSelChanged()));
    connect(m_pcbPressure, SIGNAL(activated(int)), SLOT(onSelChanged()));
    connect(m_pcbInertia,  SIGNAL(activated(int)), SLOT(onSelChanged()));

    connect(m_prbUnit1,    SIGNAL(clicked(bool)),      SLOT(onFluidUnit()));
    connect(m_prbUnit2,    SIGNAL(clicked(bool)),      SLOT(onFluidUnit()));
}


void UnitsWt::initWidget()
{
    QString strange;
    QStringList list;
    list <<"mm" << "cm"<<"dm"<<"m"<<"in"<<"ft";
    m_pcbLength->clear();
    m_pcbLength->addItems(list);

    m_pcbSurface->clear();
    m_pcbSurface->addItem(QString::fromUtf8("mm²"));
    m_pcbSurface->addItem(QString::fromUtf8("cm²"));
    m_pcbSurface->addItem(QString::fromUtf8("dm²"));
    m_pcbSurface->addItem(QString::fromUtf8("m²"));
    m_pcbSurface->addItem(QString::fromUtf8("in²"));
    m_pcbSurface->addItem(QString::fromUtf8("ft²"));

    m_pcbSpeed->clear();
    m_pcbSpeed->addItem("m/s");
    m_pcbSpeed->addItem("km/h");
    m_pcbSpeed->addItem("ft/s");
    m_pcbSpeed->addItem("kt (int.)");
    m_pcbSpeed->addItem("mph");

    m_pcbWeight->clear();
    m_pcbWeight->addItem("g");
    m_pcbWeight->addItem("kg");
    m_pcbWeight->addItem("oz");
    m_pcbWeight->addItem("lb");


    m_pcbForce->clear();
    for(int i=0; i<4; i++)
    {
        strange = Units::forceUnitQLabel(i);
        m_pcbForce->addItem(strange);
    }

    m_pcbMoment->clear();
    m_pcbMoment->addItem("N.m");
    m_pcbMoment->addItem("lbf.in");
    m_pcbMoment->addItem("lbf.ft");

    m_pcbPressure->clear();
    m_pcbPressure->addItem("Pa");
    m_pcbPressure->addItem("hPa");
    m_pcbPressure->addItem("kPa");
    m_pcbPressure->addItem("MPa");
    m_pcbPressure->addItem("bar");
    m_pcbPressure->addItem("psi");
    m_pcbPressure->addItem("ksi");    //6

    m_pcbInertia->clear();
    m_pcbInertia->addItem(QString::fromUtf8("kg.m²"));
    m_pcbInertia->addItem(QString::fromUtf8("lbm.ft²"));

    m_pcbLength->setCurrentIndex(  Units::lengthUnit());
    m_pcbWeight->setCurrentIndex(  Units::massUnit());
    m_pcbSurface->setCurrentIndex( Units::areaUnit());
    m_pcbSpeed->setCurrentIndex(   Units::speedUnit());
    m_pcbForce->setCurrentIndex(   Units::forceUnit());
    m_pcbMoment->setCurrentIndex(  Units::momentUnit());
    m_pcbPressure->setCurrentIndex(Units::pressureUnit());
    m_pcbInertia->setCurrentIndex( Units::inertiaUnit());

    m_prbUnit1->setChecked(Units::fluidUnitType()==Units::IS);
    m_prbUnit2->setChecked(Units::fluidUnitType()==Units::IMPERIAL);

    updateFluid();

    m_pcbLength->setFocus();
    setLabels();
}


void UnitsWt::onSelChanged()
{
    switch(m_pcbLength->currentIndex())
    {
        case 0: Units::setLengthUnit(Units::MM);   break;
        case 1: Units::setLengthUnit(Units::CM);   break;
        case 2: Units::setLengthUnit(Units::DM);   break;
        default:
        case 3: Units::setLengthUnit(Units::M);    break;
        case 4: Units::setLengthUnit(Units::INCH); break;
        case 5: Units::setLengthUnit(Units::FT); break;
    }

    switch( m_pcbSpeed->currentIndex())
    {
        default:
        case 0: Units::setSpeedUnit(Units::MS);   break;
        case 1: Units::setSpeedUnit(Units::KMH);  break;
        case 2: Units::setSpeedUnit(Units::FTS);  break;
        case 3: Units::setSpeedUnit(Units::KT);   break;
        case 4: Units::setSpeedUnit(Units::MPH);  break;
    }

    switch(m_pcbSurface->currentIndex())
    {
        case 0: Units::setAreaUnit(Units::MM2);  break;
        case 1: Units::setAreaUnit(Units::CM2);  break;
        case 2: Units::setAreaUnit(Units::DM2);  break;
        default:
        case 3: Units::setAreaUnit(Units::M2);   break;
        case 4: Units::setAreaUnit(Units::IN2);  break;
        case 5: Units::setAreaUnit(Units::FT2);  break;
    }

    switch(m_pcbWeight->currentIndex())
    {
        case 0: Units::setMassUnit(Units::G);      break;
        default:
        case 1: Units::setMassUnit(Units::KG);     break;
        case 2: Units::setMassUnit(Units::OZ);     break;
        case 3: Units::setMassUnit(Units::LB);     break;
    }

    switch(m_pcbForce->currentIndex())
    {
        case 0: Units::setForceUnit(Units::N);     break;
        default:
        case 1: Units::setForceUnit(Units::KN);    break;
        case 2: Units::setForceUnit(Units::TON);   break;
        case 3: Units::setForceUnit(Units::LBF);   break;
    }

    switch(m_pcbMoment->currentIndex())
    {
        default:
        case 0: Units::setMomentUnit(Units::NM);     break;
        case 1: Units::setMomentUnit(Units::LBFIN);  break;
        case 2: Units::setMomentUnit(Units::LBFFT);  break;
    }

    switch(m_pcbPressure->currentIndex())
    {
        default:
        case 0: Units::setPressureUnit(Units::PA);      break;
        case 1: Units::setPressureUnit(Units::HPA);     break;
        case 2: Units::setPressureUnit(Units::KPA);     break;
        case 3: Units::setPressureUnit(Units::MPA);     break;
        case 4: Units::setPressureUnit(Units::BAR);     break;
        case 5: Units::setPressureUnit(Units::PSI);     break;
        case 6: Units::setPressureUnit(Units::KSI);     break;
    }

    switch(m_pcbInertia->currentIndex())
    {
        default:
        case 0: Units::setInertiaUnit(Units::KGM2);   break;
        case 1: Units::setInertiaUnit(Units::LBMFT2); break;
    }

    Units::setUnitConversionFactors();

    setLabels();

    emit unitsChanged();
}


void UnitsWt::onFluidUnit()
{
    updateFluid();
    emit unitsChanged();
}


void UnitsWt::updateFluid()
{
    if(m_prbUnit1->isChecked()) Units::setFluidUnitType(Units::IS);
    else                        Units::setFluidUnitType(Units::IMPERIAL);
    Units::setUnitConversionFactors();
    m_plabFluidUnit->setText(tr("Density:             ") + Units::densityUnitQLabel()   +"\n" +
                             tr("Kinematic viscosity: ") + Units::viscosityUnitQLabel() +"\n");
}


void UnitsWt::setLabels()
{
    QString strUnitLabel, strange, strUnit;
    int len1 = 11;
    int len2 = 17;
    strUnitLabel = Units::lengthUnitQLabel();
    strange = QString::asprintf("1 m = %11.5g",Units::mtoUnit());
    m_plabLengthFactor->setText(strange);
    strUnit = QString::asprintf("%11.5g m",1./Units::mtoUnit());
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabLengthInvFactor->setText(strange);

    strUnitLabel = Units::areaUnitQLabel();
    strange = QString::fromUtf8("1 m² = %1").arg(Units::m2toUnit(),11,'g',5);
    m_plabSurfaceFactor->setText(strange);
    strUnit = QString::fromUtf8("%1 m²").arg(1./Units::m2toUnit(),11,'g',5);
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabSurfaceInvFactor->setText(strange);

    strUnitLabel = Units::speedUnitQLabel();
    strange = QString::asprintf("1 m/s = %11.5g",Units::mstoUnit());
    m_plabSpeedFactor->setText(strange);
    strUnit = QString::asprintf("%11.5g m/s",1./Units::mstoUnit());
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabSpeedInvFactor->setText(strange);

    strUnitLabel = Units::massUnitQLabel();
    strange = QString::asprintf("1 kg = %11.5g",Units::kgtoUnit());
    m_plabWeightFactor->setText(strange);
    strUnit = QString::asprintf("%11.5g kg",1./Units::kgtoUnit());
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabWeightInvFactor->setText(strange);

    strUnitLabel = Units::forceUnitQLabel();
    strange = QString::asprintf("1 N = %11.5g",Units::NtoUnit());
    m_plabForceFactor->setText(strange);
    strUnit = QString::asprintf("%11.5g N",1./Units::NtoUnit());
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabForceInvFactor->setText(strange);

    strUnitLabel = Units::momentUnitQLabel();
    strange = QString::asprintf("1 N.m = %11.5g",Units::NmtoUnit());
    m_plabMomentFactor->setText(strange);
    strUnit = QString::asprintf("%11.5g N.m",1./Units::NmtoUnit());
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabMomentInvFactor->setText(strange);

    strUnitLabel = Units::pressureUnitQLabel();
    strange = QString::asprintf("1 Pa = %11.5g",Units::PatoUnit());
    m_plabPressureFactor->setText(strange);
    strUnit = QString::asprintf("%11.5g Pa",1./Units::PatoUnit());
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabPressureInvFactor->setText(strange);

    strUnitLabel = Units::inertiaUnitQLabel();
    strange= QString::fromUtf8("1 kg.m² = %1").arg(Units::kgm2toUnit(), 11,'g',5);
    m_plabInertiaFactor->setText(strange);
    strUnit = QString::fromUtf8("%1 kg.m²").arg(1./Units::kgm2toUnit(),11,'g',5);
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabInertiaInvFactor->setText(strange);
}


