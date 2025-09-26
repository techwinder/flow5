/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QComboBox>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>


class UnitsWt : public QWidget
{
    Q_OBJECT

    public:
        UnitsWt(QWidget *parent);
        void initWidget();

        static double toCustomUnit(int index);

    private:
        void setupLayout();
        void setLabels();

    private slots:
        void onSelChanged();
        void onFluidUnit();

    signals:
        void unitsChanged();

    private:
        QComboBox	*m_pcbMoment;
        QComboBox	*m_pcbSurface;
        QComboBox	*m_pcbWeight;
        QComboBox	*m_pcbSpeed;
        QComboBox	*m_pcbLength;
        QComboBox	*m_pcbForce;
        QComboBox   *m_pcbPressure;
        QComboBox   *m_pcbInertia;
        QLabel *m_plabForceFactor,    *m_plabForceInvFactor;
        QLabel *m_plabLengthFactor,   *m_plabLengthInvFactor;
        QLabel *m_plabSpeedFactor,    *m_plabSpeedInvFactor;
        QLabel *m_plabSurfaceFactor,  *m_plabSurfaceInvFactor;
        QLabel *m_plabWeightFactor,   *m_plabWeightInvFactor;
        QLabel *m_plabMomentFactor,   *m_plabMomentInvFactor;
        QLabel *m_plabPressureFactor, *m_plabPressureInvFactor;
        QLabel *m_plabInertiaFactor,  *m_plabInertiaInvFactor;

        QRadioButton *m_prbUnit1, *m_prbUnit2;
        QLabel *m_plabFluidUnit;
};


