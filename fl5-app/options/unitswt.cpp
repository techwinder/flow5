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

#include <options/unitswt.h>
#include <api/units.h>


UnitsWt::UnitsWt(QWidget *parent): QWidget(parent)
{
    setWindowTitle("Units");
    setupLayout();
}


void UnitsWt::setupLayout()
{
    QFont fixedfnt(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGroupBox *pgbConversion = new QGroupBox("Conversion factors");
        {
            QGridLayout *pConversionLayout = new QGridLayout;
            {
                QLabel *pLab1 = new QLabel("Length:");
                QLabel *pLab2 = new QLabel("Area:");
                QLabel *pLab3 = new QLabel("Velocity:");
                QLabel *pLab4 = new QLabel("Mass:");
                QLabel *pLab5 = new QLabel("Force:");
                QLabel *pLab6 = new QLabel("Moment:");
                QLabel *pLab7 = new QLabel("Pressure:");
                QLabel *pLab8 = new QLabel("Inertia:");
                pLab1->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                pLab2->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                pLab3->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                pLab4->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                pLab5->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                pLab6->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                pLab7->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                pLab8->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                pLab1->setFont(fixedfnt);
                pLab2->setFont(fixedfnt);
                pLab3->setFont(fixedfnt);
                pLab4->setFont(fixedfnt);
                pLab5->setFont(fixedfnt);
                pLab6->setFont(fixedfnt);
                pLab7->setFont(fixedfnt);
                pLab8->setFont(fixedfnt);
                pConversionLayout->addWidget(pLab1, 1,1);
                pConversionLayout->addWidget(pLab2, 2,1);
                pConversionLayout->addWidget(pLab3, 3,1);
                pConversionLayout->addWidget(pLab4, 4,1);
                pConversionLayout->addWidget(pLab5, 5,1);
                pConversionLayout->addWidget(pLab6, 6,1);
                pConversionLayout->addWidget(pLab7, 7,1);
                pConversionLayout->addWidget(pLab8, 8,1);


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
                m_plabLengthFactor->setAlignment(  Qt::AlignRight | Qt::AlignCenter);
                m_plabSurfaceFactor->setAlignment( Qt::AlignRight | Qt::AlignCenter);
                m_plabWeightFactor->setAlignment(  Qt::AlignRight | Qt::AlignCenter);
                m_plabSpeedFactor->setAlignment(   Qt::AlignRight | Qt::AlignCenter);
                m_plabForceFactor->setAlignment(   Qt::AlignRight | Qt::AlignCenter);
                m_plabMomentFactor->setAlignment(  Qt::AlignRight | Qt::AlignCenter);
                m_plabPressureFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                m_plabInertiaFactor->setAlignment( Qt::AlignRight | Qt::AlignCenter);

                pConversionLayout->addWidget(m_plabLengthFactor,   1,2);
                pConversionLayout->addWidget(m_plabSurfaceFactor,  2,2);
                pConversionLayout->addWidget(m_plabSpeedFactor,    3,2);
                pConversionLayout->addWidget(m_plabWeightFactor,   4,2);
                pConversionLayout->addWidget(m_plabForceFactor,    5,2);
                pConversionLayout->addWidget(m_plabMomentFactor,   6,2);
                pConversionLayout->addWidget(m_plabPressureFactor, 7,2);
                pConversionLayout->addWidget(m_plabInertiaFactor,  8,2);

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
                pConversionLayout->addWidget(m_pcbInertia, 8,3);


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

                m_plabLengthInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                m_plabSurfaceInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                m_plabWeightInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                m_plabSpeedInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                m_plabForceInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                m_plabMomentInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                m_plabPressureInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                m_plabInertiaInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                pConversionLayout->addWidget(m_plabLengthInvFactor,   1,4);
                pConversionLayout->addWidget(m_plabSurfaceInvFactor,  2,4);
                pConversionLayout->addWidget(m_plabSpeedInvFactor,    3,4);
                pConversionLayout->addWidget(m_plabWeightInvFactor,   4,4);
                pConversionLayout->addWidget(m_plabForceInvFactor,    5,4);
                pConversionLayout->addWidget(m_plabMomentInvFactor,   6,4);
                pConversionLayout->addWidget(m_plabPressureInvFactor, 7,4);
                pConversionLayout->addWidget(m_plabInertiaInvFactor,  8,4);
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
    m_pcbLength->addItems(list);        //5

    m_pcbSurface->clear();
    m_pcbSurface->addItem(QString::fromUtf8("mm²"));        //0
    m_pcbSurface->addItem(QString::fromUtf8("cm²"));        //1
    m_pcbSurface->addItem(QString::fromUtf8("dm²"));        //2
    m_pcbSurface->addItem(QString::fromUtf8("m²"));        //3
    m_pcbSurface->addItem(QString::fromUtf8("in²"));        //4
    m_pcbSurface->addItem(QString::fromUtf8("ft²"));        //5

    m_pcbSpeed->clear();
    m_pcbSpeed->addItem("m/s");       //0
    m_pcbSpeed->addItem("km/h");      //1
    m_pcbSpeed->addItem("ft/s");      //2
    m_pcbSpeed->addItem("kt (int.)"); //3
    m_pcbSpeed->addItem("mph");       //4

    m_pcbWeight->clear();
    m_pcbWeight->addItem("g");        //0
    m_pcbWeight->addItem("kg");       //1
    m_pcbWeight->addItem("oz");       //2
    m_pcbWeight->addItem("lb");       //3


    m_pcbForce->clear();
    for(int i=0; i<4; i++)
    {
        strange = Units::forceUnitQLabel(i);
        m_pcbForce->addItem(strange);
    }

    m_pcbMoment->clear();
    m_pcbMoment->addItem("N.m");        //0
    m_pcbMoment->addItem("lbf.in");    //1
    m_pcbMoment->addItem("lbf.ft");    //2

    m_pcbPressure->clear();
    m_pcbPressure->addItem("Pa");     //0
    m_pcbPressure->addItem("hPa");    //1
    m_pcbPressure->addItem("kPa");    //2
    m_pcbPressure->addItem("MPa");    //3
    m_pcbPressure->addItem("bar");    //4
    m_pcbPressure->addItem("psi");    //5
    m_pcbPressure->addItem("ksi");    //6

    m_pcbInertia->clear();
    m_pcbInertia->addItem(QString::fromUtf8("kg.m²"));    //0
    m_pcbInertia->addItem(QString::fromUtf8("lbm.ft²"));    //1

    m_pcbLength->setCurrentIndex(  Units::lengthUnitIndex());
    m_pcbWeight->setCurrentIndex(  Units::weightUnitIndex());
    m_pcbSurface->setCurrentIndex( Units::areaUnitIndex());
    m_pcbSpeed->setCurrentIndex(   Units::speedUnitIndex());
    m_pcbForce->setCurrentIndex(   Units::forceUnitIndex());
    m_pcbMoment->setCurrentIndex(  Units::momentUnitIndex());
    m_pcbPressure->setCurrentIndex(Units::pressureUnitIndex());
    m_pcbInertia->setCurrentIndex( Units::inertiaUnitIndex());

    m_prbUnit1->setChecked(Units::fluidUnitType()==0);
    m_prbUnit2->setChecked(Units::fluidUnitType()==1);
    onFluidUnit();

    m_pcbLength->setFocus();
    setLabels();
}


void UnitsWt::onSelChanged()
{
    Units::setLengthUnitIndex(    m_pcbLength->currentIndex());
    Units::setAreaUnitIndex(      m_pcbSurface->currentIndex());
    Units::setWeightUnitIndex(      m_pcbWeight->currentIndex());
    Units::setSpeedUnitIndex(     m_pcbSpeed->currentIndex());
    Units::setForceUnitIndex(     m_pcbForce->currentIndex());
    Units::setMomentUnitIndex(    m_pcbMoment->currentIndex());
    Units::setPressureUnitIndex(  m_pcbPressure->currentIndex());
    Units::setInertiaUnitIndex(   m_pcbInertia->currentIndex());
    Units::setUnitConversionFactors();

    setLabels();

    emit unitsChanged();
}


void UnitsWt::onFluidUnit()
{
    if(m_prbUnit1->isChecked()) Units::setFluidUnitType(0);
    else                        Units::setFluidUnitType(1);
    Units::setUnitConversionFactors();
    m_plabFluidUnit->setText("Density:             " + Units::densityUnitQLabel()   +"\n" +
                             "Kinematic viscosity: " + Units::viscosityUnitQLabel() +"\n");

    emit unitsChanged();
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


