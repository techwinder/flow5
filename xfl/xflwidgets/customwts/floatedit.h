/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDoubleValidator>
#include <QLineEdit>
#include <QKeyEvent>

#include <xflwidgets/customwts/numedit.h>


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






