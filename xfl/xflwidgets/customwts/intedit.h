/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QIntValidator>
#include <QLineEdit>
#include <QKeyEvent>

#include <xflwidgets/customwts/numedit.h>


class IntEdit : public NumEdit
{
    Q_OBJECT
    public:
        IntEdit(QWidget *pParent=nullptr);
        IntEdit(int val, QWidget *pParent=nullptr);

        ~IntEdit() {delete m_pIV;}

        void keyPressEvent(QKeyEvent *pEvent) override;

        int value() const {return m_Value;}
        void setValue(int val);

        void initialize(int value);
        void setValueNoFormat(int val);

        void formatValue() override;
        void readValue() override;
        void setMin(int min) {m_pIV->setBottom(min);}
        void setMax(int max) {m_pIV->setTop(max);}

    signals:
        void intChanged(int);

    public:
        QIntValidator *m_pIV;
        int m_Value;

};








