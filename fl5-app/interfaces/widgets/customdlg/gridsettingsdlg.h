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
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QDialogButtonBox>

#include <interfaces/widgets/view/grid.h>
#include <interfaces/widgets/customwts/gridcontrol.h>

class LineBtn;
class FloatEdit;

class GridSettingsDlg : public QDialog
{
    Q_OBJECT

    public:
        GridSettingsDlg(QWidget *pParent=nullptr);
        void initDialog(const Grid &grid, bool bShowUnit);

        Grid const &grid() {return m_Grid;}

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *event) override;

    private slots:
        void onButton(QAbstractButton *pButton);


    private:
        void setupLayout();

        GridControl *m_pGridControl;

        Grid m_Grid;

        QDialogButtonBox *m_pButtonBox;

        bool m_bWithUnit;

        static QByteArray s_WindowGeometry;

};

