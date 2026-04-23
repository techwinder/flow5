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

#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>

#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customdlg/xfldialog.h>

class DoubleValueDlg : public XflDialog
{
    public:
        DoubleValueDlg(QWidget *pParent, QVector<double> values, QStringList const &leftlabels, QStringList const &rightlabels);

        void setValue(int iVal, double value) {if(iVal>=0 && iVal<m_pDoubleEdit.size()) m_pDoubleEdit[iVal]->setValue(value);}
        double value(int iVal) const{if(iVal>=0 && iVal<m_pDoubleEdit.size()) return m_pDoubleEdit[iVal]->value(); else return 0;}

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        static QByteArray s_WindowGeometry;

    private:
        QVector<FloatEdit *> m_pDoubleEdit;
};






