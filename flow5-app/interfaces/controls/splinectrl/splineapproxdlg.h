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
#include <QDialog>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QListWidget>
#include <QComboBox>

class IntEdit;

class SplineApproxDlg : public QDialog
{
    Q_OBJECT

    public:
        SplineApproxDlg(QWidget *pParent);

        void initDialog(bool bCubic, int degree, int nCtrlPts);

        QString selectedFoilName() const {if(m_plwNames->currentItem()) return m_plwNames->currentItem()->text(); else return QString();}
        int degree() const;
        int nCtrlPoints() const;

    private:
        void setupLayout();
        QSize sizeHint() const override {return QSize(700,900);}

    private slots:
        void onButton(QAbstractButton *pButton);

    private:

        QListWidget *m_plwNames;

        QComboBox *m_pcbSplineDegree;
        IntEdit	*m_pieCtrlPoints;

        QDialogButtonBox *m_pButtonBox;

};

