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

#include <cmath>


#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "aerodatadlg.h"
#include <api/units.h>
#include <interfaces/widgets/customwts/floatedit.h>

// International Standard Atmosphere

#define STANDARDTEMPERATURE  288.15  // [°K]
#define STANDARDGRAVITY      9.80665   // [m/s²]
#define STANDARDPRESSURE     101325  // [Pa]
#define STANDARDLAPSERATE    0.0065  // [K/m]


QByteArray AeroDataDlg::s_Geometry;


bool AeroDataDlg::s_bCelsius = true;
double AeroDataDlg::s_Altitude = 0.0;
double AeroDataDlg::s_Temperature = STANDARDTEMPERATURE;

AeroDataDlg::AeroDataDlg(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Air data"));

    UniversalGasConstant = 8.3144621;     // [J/(mol.K)]
    DryAirMolarMass      = 0.02896442;    // [kg/mol]
    AdiabaticIndex       = 1.4;
    SutherlandsConstant  = 120;           // [K]
    ReferenceViscosity   = 17.894e-6;     // [Pa.s] !!!!

    setupLayout();

    if(s_bCelsius)    m_pfeTemperature->setValue(s_Temperature-STANDARDTEMPERATURE+15);
    else
    {
        double temp = s_Temperature-STANDARDTEMPERATURE+15;
        m_pfeTemperature->setValue(temp*9.0/5.0 + 32.0);
    }


    m_pfeAltitude->setValue(s_Altitude);

    updateResults();
}



double AeroDataDlg::airTemperature(double Altitude)   //[K]
{
// Troposphere only <= 11000 m
    return STANDARDTEMPERATURE -  Altitude * STANDARDLAPSERATE;
}


double AeroDataDlg::airPressure(double Altitude)    //[Pa]
{
// Troposphere only <= 11000 m
    return STANDARDPRESSURE * pow((airTemperature(Altitude) / STANDARDTEMPERATURE),
                                  (STANDARDGRAVITY * DryAirMolarMass / UniversalGasConstant / STANDARDLAPSERATE));
}


double AeroDataDlg::airDensity(double Altitude, double temp)   //[kg/m³]
{
// Troposphere only <= 11000 m
// TemperatureCorrection is 0 for standard atmosphere

    return  airPressure(Altitude) * DryAirMolarMass / UniversalGasConstant / (airTemperature(Altitude) + temperatureCorrection(temp));
}


double AeroDataDlg::dynamicViscosity(double Altitude, double temp)     //[kg/m³]
{
// Troposphere only <= 11000 m
// TemperatureCorrection is 0 for standard atmosphere

    double T = airTemperature(Altitude) + temperatureCorrection(temp);
    return    ReferenceViscosity * (STANDARDTEMPERATURE + SutherlandsConstant)
                                 / (T                   + SutherlandsConstant) * pow((T / STANDARDTEMPERATURE), 1.5);
}


double AeroDataDlg::temperatureCorrection(double temp)
{
    return temp-STANDARDTEMPERATURE;
}


double AeroDataDlg::kinematicViscosity(double Altitude , double temp)   //[kg/m³]
{
// Troposphere only <= 11000 m
// TemperatureCorrection is 0 for standard atmosphere
    return dynamicViscosity(Altitude, temp) / airDensity(Altitude, temp);
}

double AeroDataDlg::kinematicViscosity()
{
    return kinematicViscosity(s_Altitude, s_Temperature);
}


double AeroDataDlg::speedOfSound(double temp)       //[m/s]
{
    return sqrt(AdiabaticIndex * UniversalGasConstant * temp / DryAirMolarMass);
}


double AeroDataDlg::airDensity()
{
    return airDensity(s_Altitude, s_Temperature);
}




void AeroDataDlg::setupLayout()
{
    QLabel *pValid = new QLabel(tr("<p>Applicable in the troposphere<br> i.e. Altitude &lt; 11000m</p>"));
    pValid->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    QGridLayout *pDataLayout = new QGridLayout;
    {
        QLabel *plabTemp           = new QLabel(tr("Temperature="));
        QLabel *plabAltitude       = new QLabel(tr("Altitude="));
        QLabel *plabAirPressure    = new QLabel(tr("Air pressure="));
        QLabel *plabAirDens        = new QLabel("<p>&rho;=</p>");
        QLabel *plabDynamicVisc    = new QLabel("<p>&mu;=</p>");
        QLabel *plabKinematicVisc  = new QLabel("<p>&nu;=</p>");
        QLabel *plabSpeedOfSound   = new QLabel(tr("Speed of Sound="));


        m_pfeTemperature = new FloatEdit(s_Temperature);
        m_pfeAltitude    = new FloatEdit(s_Altitude);
        connect(m_pfeTemperature, SIGNAL(floatChanged(float)), SLOT(updateResults()));
        connect(m_pfeAltitude,    SIGNAL(floatChanged(float)), SLOT(updateResults()));

        m_plabAirPressure        = new QLabel;
        m_plabAirDensity         = new QLabel;
        m_plabAirDensity->setToolTip(tr("Density"));
        m_plabDynamicViscosity   = new QLabel;
        m_plabDynamicViscosity->setToolTip(tr("Dynamic viscosity"));
        m_plabKinematicViscosity = new QLabel;
        m_plabKinematicViscosity->setToolTip(tr("Kinematic viscosity"));
        m_plabSpeedOfSound       = new QLabel;

        m_plabAirPressure->setAlignment(Qt::AlignRight);
        m_plabAirDensity->setAlignment(Qt::AlignRight);
        m_plabDynamicViscosity->setAlignment(Qt::AlignRight);
        m_plabKinematicViscosity->setAlignment(Qt::AlignRight);
        m_plabSpeedOfSound->setAlignment(Qt::AlignRight);

        m_pcbTempUnit = new QComboBox;
        m_pcbTempUnit->addItem(QString::fromUtf8("  °C"));
        m_pcbTempUnit->addItem(QString::fromUtf8("  °F"));
        if(s_bCelsius) m_pcbTempUnit->setCurrentIndex(0);
        else           m_pcbTempUnit->setCurrentIndex(1);
        connect(m_pcbTempUnit, SIGNAL(currentIndexChanged(int)), SLOT(onTempUnit()));

        QLabel * plabAltitudeUnit = new QLabel("m");
        QLabel * plabPressureUnit = new QLabel("Pa");
        QLabel * plabDensUnit     = new QLabel("kg/m³");
        QLabel * plabDynViscUnit  = new QLabel(QString::fromUtf8("m/s²"));
        QLabel * plabKinViscUnit  = new QLabel(QString::fromUtf8("m²/s"));
        QLabel * plabSpeedUnit    = new QLabel("m/s");

        pDataLayout->addWidget(plabTemp,          1, 1, Qt::AlignRight);
        pDataLayout->addWidget(plabAltitude,      2, 1, Qt::AlignRight);
        pDataLayout->addWidget(plabAirPressure,   4, 1, Qt::AlignRight);
        pDataLayout->addWidget(plabAirDens,       5, 1, Qt::AlignRight);
        pDataLayout->addWidget(plabDynamicVisc,   6, 1, Qt::AlignRight);
        pDataLayout->addWidget(plabKinematicVisc, 7, 1, Qt::AlignRight);
        pDataLayout->addWidget(plabSpeedOfSound,  8, 1, Qt::AlignRight);

        pDataLayout->addWidget(m_pfeTemperature,         1, 2);
        pDataLayout->addWidget(m_pfeAltitude,            2, 2);
        pDataLayout->addWidget(m_plabAirPressure,        4, 2);
        pDataLayout->addWidget(m_plabAirDensity,         5, 2);
        pDataLayout->addWidget(m_plabDynamicViscosity,   6, 2);
        pDataLayout->addWidget(m_plabKinematicViscosity, 7, 2);
        pDataLayout->addWidget(m_plabSpeedOfSound,       8, 2);

        pDataLayout->addWidget(m_pcbTempUnit,    1, 3);
        pDataLayout->addWidget(plabAltitudeUnit, 2, 3);
        pDataLayout->addWidget(plabPressureUnit, 4, 3);
        pDataLayout->addWidget(plabDensUnit,     5, 3);
        pDataLayout->addWidget(plabDynViscUnit,  6, 3);
        pDataLayout->addWidget(plabKinViscUnit,  7, 3);
        pDataLayout->addWidget(plabSpeedUnit,    8, 3);

        pDataLayout->setColumnStretch(3,1);
    }


    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout * pMainLayout = new QVBoxLayout();
    {
        pMainLayout->addWidget(pValid);
//        pMainLayout->addStretch();
        pMainLayout->addLayout(pDataLayout);
        pMainLayout->addStretch();
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);
}



void AeroDataDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)      accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}


void AeroDataDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void AeroDataDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void AeroDataDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
//                updateResults();
                m_pButtonBox->setFocus();
                return;
            }
            else
            {
                accept();
                return;
            }
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
        default:
            event->ignore();
    }
}


void AeroDataDlg::updateResults()
{
    if(m_pcbTempUnit->currentIndex()==0)
    {
        s_Temperature = m_pfeTemperature->value()+STANDARDTEMPERATURE-15;
    }
    else
    {
        // convert first to Celsius : Deduct 32, then multiply by 5, then divide by 9
        s_Temperature = (m_pfeTemperature->value()-32.0)*5.0/9.0;
        s_Temperature += STANDARDTEMPERATURE-15;
    }

    s_Altitude = m_pfeAltitude->value();
    m_plabAirPressure->setText(QString::asprintf("%g", airPressure(s_Altitude)));

    double density = airDensity(s_Altitude, s_Temperature);
    double dynViscosity = dynamicViscosity(s_Altitude, s_Temperature);

    m_plabAirDensity->setText(QString::asprintf("%g", density));
    m_plabDynamicViscosity->setText(QString::asprintf("%g", dynViscosity));
    m_plabKinematicViscosity->setText(QString::asprintf("%g", dynViscosity/density));

    m_plabSpeedOfSound->setText(QString::asprintf("%g",speedOfSound(s_Temperature)));
}



void AeroDataDlg::onTempUnit()
{
    s_bCelsius = m_pcbTempUnit->currentIndex()==0;

    if(s_bCelsius)
    {
        m_pfeTemperature->setValue(s_Temperature-STANDARDTEMPERATURE+15);
    }
    else
    {
        m_pfeTemperature->setValue( (s_Temperature-STANDARDTEMPERATURE+15)*9.0/5.0+32);
    }
}






