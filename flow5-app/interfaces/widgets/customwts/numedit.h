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


#include <QLineEdit>
#include <QKeyEvent>



class NumEdit : public QLineEdit
{
    Q_OBJECT
    public:
        NumEdit(QWidget *pWidget);

    protected:
        void showEvent(QShowEvent*pEvent) override;
        void focusInEvent (QFocusEvent * pEvent) override;
        void focusOutEvent (QFocusEvent * pEvent) override;
        QSize sizeHint() const override;

        virtual void readValue() = 0;
        virtual void formatValue() = 0;

    public slots:
        void paste();

    protected:
};

