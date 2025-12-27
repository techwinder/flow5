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

#include <QDoubleValidator>
#include <QLineEdit>
#include <QKeyEvent>

#include <interfaces/widgets/customwts/numedit.h>


class FloatEdit : public NumEdit
{
    Q_OBJECT
    public:
        FloatEdit(QWidget *pParent=nullptr);
        FloatEdit(float val, int decimals=-1, QWidget *pParent=nullptr);

        void keyPressEvent(QKeyEvent *pEvent) override;


        double value() const {return double(m_Value);}
        float valuef() const {return m_Value;}
        void setValue(float val);
        void setValuef(float val);

        void initialize(float value, int decimals);

        void setValueNoFormat(float val);

        void formatValue() override;
        void readValue() override;
        void setMin(float min) {m_MinValue=min;}
        void setMax(float max) {m_MaxValue=max;}
        void setRange(float min, float max) {m_MinValue=min; m_MaxValue=max;}


        void setDigits(int decimals) {m_Digits = decimals;}
        int digits() const {return m_Digits;}

        void setNotation(QDoubleValidator::Notation notation) {m_Notation = notation;}


    signals:
        void floatChanged(float);


    public:
        float m_Value;//we need to store a full precision value, irrespective of the display
        float m_MinValue;
        float m_MaxValue;
        int m_Digits;
        QDoubleValidator::Notation m_Notation;

};






