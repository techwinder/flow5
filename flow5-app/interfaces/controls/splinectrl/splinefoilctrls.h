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


#include <QWidget>
#include <QComboBox>
#include <QSlider>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QCheckBox>

#include <api/linestyle.h>

class FloatEdit;
class IntEdit;
class XflDelegate;
class SplineFoil;
class CPTableView;
class LineBtn;

class SplineFoilCtrls: public QWidget
{
        Q_OBJECT
        friend class AFoil;

    public:
        SplineFoilCtrls(QWidget *pParent);

        void initDialog(SplineFoil *pSF);

        void fillPointLists();

    private slots:
        void onUpdate();
        void onSplineStyle(LineStyle);

    signals:
        void splineFoilChanged();

    private:
        void showEvent(QShowEvent *pEvent) override;

        void readData();
        void setControls();
        void setupLayout();
        void connectSignals();
        void updateSplines();

    private:
        IntEdit   *m_pieOutExtrados;
        IntEdit   *m_pieOutIntrados;
        QComboBox *m_pcbDegExtrados;
        QComboBox *m_pcbDegIntrados;
        QCheckBox *m_pchSymmetric, *m_pchCloseLE, *m_pchCloseTE;

        CPTableView *m_pcptUpperList, *m_pcptLowerList;
        QStandardItemModel *m_pUpperListModel,*m_pLowerListModel;
        XflDelegate *m_pUpperFloatDelegate, *m_pLowerFloatDelegate;

        LineBtn *m_plbSplineStyle;


    protected:
        SplineFoil *m_pSF;
};

