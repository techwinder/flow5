/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <QIntValidator>
#include <QLineEdit>
#include <QKeyEvent>

#include <interfaces/widgets/customwts/numedit.h>


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








