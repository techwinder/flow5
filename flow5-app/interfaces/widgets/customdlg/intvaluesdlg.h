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

#include <interfaces/widgets/customwts/intedit.h>


class IntValuesDlg : public QDialog
{
    Q_OBJECT

    public:
        IntValuesDlg(QWidget*pParent, QVector<int> const &values, QStringList const &labels);

        void setValue(int iVal, int value) {if(iVal>=0 && iVal<m_pIntEdit.size()) m_pIntEdit[iVal]->setValue(value);}
        int value(int iVal) const {if(iVal>=0 && iVal<m_pIntEdit.size()) return m_pIntEdit[iVal]->value(); else return 0;}

        void setLabel(int iLabel, QString label);

    private:
        void showEvent(QShowEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void setupLayout(int nValues);

    private:
        QVector<IntEdit *> m_pIntEdit;
        QVector<QLabel*> m_pLabel;

        QDialogButtonBox *m_pButtonBox;
};






