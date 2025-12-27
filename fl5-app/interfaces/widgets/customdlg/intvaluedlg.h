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
#include <interfaces/widgets/customdlg/xfldialog.h>


class IntEdit;

class IntValueDlg : public XflDialog
{
    public:
        IntValueDlg(QWidget *pParent=nullptr);

        void setValue(int intvalue);
        int value() const;
        void setLeftLabel(QString const &label) {m_pLeftLabel->setText(label);}
        void setRightLabel(QString const &label) {m_pRightLabel->setText(label);}

    private:
        void setupLayout();

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *) override;


    private:
        QLabel *m_pLeftLabel, *m_pRightLabel;
        IntEdit *m_pieIntEdit;
        static QByteArray s_WindowGeometry;
};



