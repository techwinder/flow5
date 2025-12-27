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

#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>


class XflDialog : public QDialog
{
    Q_OBJECT
    public:
        XflDialog(QWidget *pParent);

        bool bChanged() const {return m_bChanged;}
        bool bDescriptionChanged() const {return m_bDescriptionChanged;}

    protected slots:
        void reject() override;
        virtual void keyPressEvent(QKeyEvent *pEvent) override;
        virtual void onButton(QAbstractButton*pButton);
        virtual void onApply() {} // base class method does nothing
        virtual void onReset() {} // base class method does nothing
        virtual void onDefaults() {} // base class method does nothing


    protected:
        void setButtons(QDialogButtonBox::StandardButtons buttons);

    protected:
        QDialogButtonBox *m_pButtonBox;

        bool m_bChanged;
        bool m_bDescriptionChanged;
};


