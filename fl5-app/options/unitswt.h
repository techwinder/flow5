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


