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

/**
 *@file This file contains the definition of the class WPolarDlg which is used to define the data for a WPolar object.
 */

#pragma once


#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <QStackedWidget>
#include <QRadioButton>
#include <QStandardItemModel>
#include <QLineEdit>


#include <interfaces/editors/analysis3ddef/planepolardlg.h>


class CPTableView;
class CtrlTableModel;


class T1234578PolarDlg : public PlanePolarDlg
{
    Q_OBJECT

    public:
        T1234578PolarDlg(QWidget *pParent);
        void initPolar3dDlg(Plane const *pPlane, PlanePolar const *pWPolar=nullptr) override;
        void setType7Polar();

    private:

        void connectSignals();

        void readData() override;
        void enableControls() override;
        void resizeColumns() override;

        void setReynolds();
        void setupLayout();
        void setWingLoad();

        void fillAVLCtrlList();
        void fillAVLGains();
        void readAVLCtrls();

    private slots:

        void onAVLContextMenu(QPoint pt);
        void onAVLRowChanged(QModelIndex index);
        void onAppendAVLCtrl();
        void onDuplicateAVLCtrl();
        void onDeleteAVLCtrl();
        void onAVLCtrlChanged();
        void onAVLGainChanged();
        void onMoveAVLCtrl();

        void onOK() override;
        void onPolarType();
        void onEditingFinished() override;


    private:

        FloatEdit *m_pfeQInf;
        FloatEdit *m_pfeAlphaSpec, *m_pfePhiSpec;

        QRadioButton *m_prbType1, *m_prbType2, *m_prbType3, *m_prbType4, *m_prbType5, *m_prbType7, *m_prbType8;

        QFrame *m_pfrAlpha, *m_pfrPhi, *m_pfrQInf, *m_frFlightInfo;
        QLabel *m_plabGlide, *m_plabVh;

        QLabel *m_plabReInfo;
        QLabel *m_plabWingLoad;

        CPTableView *m_pcptAVLCtrls;
        QStandardItemModel *m_pAVLCtrlModel;
        CPTableView *m_pcptAVLGains;
        CtrlTableModel *m_pAVLGainModel;
        CtrlTableDelegate *m_pAVLGainDelegate;

};








