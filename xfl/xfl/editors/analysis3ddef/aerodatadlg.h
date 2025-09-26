/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once



/**
 *@file
 *
 * This file defines the AeroDataDlg class, which is used to calculate the air density and kinematic viscosity
 * as a function of temperature and altitude, using the ISA atmospheric model
 *
 */



#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QComboBox>

class FloatEdit;
/**
 *@class AeroDataDlg
 *@brief The class is used to calculate the air density and kinematic viscosity
 * as a function of temperature and altitude, using the ISA atmospheric model
 */

class AeroDataDlg : public QDialog
{
    Q_OBJECT

    public:
        AeroDataDlg(QWidget *parent = nullptr);
        double airDensity();   //[kg/m³]
        double kinematicViscosity();     //[kg/m³]

        static void setTemperature(double T) {s_Temperature=T;}
        static double temperature() {return s_Temperature;}

        static void setAltitude(double T) {s_Altitude=T;}
        static double altitude() {return s_Altitude;}

    private:
        void setupLayout();

        double temperatureCorrection(double temp);
        double airTemperature(double Altitude);   //[K]
        double airPressure(double Altitude);    //[Pa]
        double airDensity(double Altitude, double temp);   //[kg/m³]
        double dynamicViscosity(double Altitude, double temp);     //[kg/m³]
        double kinematicViscosity(double Altitude , double Temp);   //[kg/m³]
        double speedOfSound(double temp);       //[m/s]

    private:
        void keyPressEvent(QKeyEvent *event) override;
        void showEvent(QShowEvent *) override;
        void hideEvent(QHideEvent *) override;

    private slots:
        void onTempUnit();
        void updateResults();
        void onButton(QAbstractButton*pButton);

    private:
        static double s_Altitude;    // meters
        static double s_Temperature; // degree Kelvin
        static bool s_bCelsius;
        static QByteArray s_Geometry;

        double UniversalGasConstant;        // [J/(mol.K)]
        double DryAirMolarMass;             // [kg/mol]
        double AdiabaticIndex;
        double SutherlandsConstant;         // [K]
        double ReferenceViscosity;          // [Pa.s] !!!!

    private:
        QComboBox *m_pcbTempUnit;
        QDialogButtonBox *m_pButtonBox;

        FloatEdit *m_pfeTemperature, *m_pfeAltitude;

        QLabel *m_plabAirDensity, *m_plabAirPressure;
        QLabel *m_plabKinematicViscosity;
        QLabel *m_plabDynamicViscosity;
        QLabel *m_plabSpeedOfSound;
};










