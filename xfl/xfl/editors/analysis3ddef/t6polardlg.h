/****************************************************************************

    flow5
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
#include <QCheckBox>
#include <QSettings>

#include <xfl/editors/analysis3ddef/planepolardlg.h>
#include <xfl/editors/analysis3ddef/ctrltablemodel.h>


class Plane;
class WingXfl;
class WPolar;
class DoubleEdit;
class IntEdit;
class CPTableView;
class CtrlTableDelegate;



class T6PolarDlg : public PlanePolarDlg
{
    Q_OBJECT

    public:
        T6PolarDlg(QWidget *pParent);
        ~T6PolarDlg();

        void initPolar3dDlg(const Plane *pPlane, const WPolar *pWPolar=nullptr) override;

    private:
        void setupLayout();
        void connectSignals();
        void resizeColumns() override;


    private slots:
        void onOK() override;
        void onPlaneInertia() override;
        void onEditingFinished() override;
        void onAdjustedVelocity();
        void onRangeCellChanged();
        void onInertiaCellChanged();
        void onAngleCellChanged();

    private:
        void enableControls() override;
        //	void setAutoWPolarName(WPolar * pWPolar, Plane *pPlane);

        void fillOppRangePage();
        void fillAngleControlList();
        void fillInertiaPage() override;
        void readAngleRangeData();
        void readOperatingData();
        void readInertiaData();
        void readData() override;
        void setViscous();


    private:
        CPTableView *m_pcptOppRange;
        CtrlTableModel *m_pOppRangeControlModel;
        CtrlTableDelegate *m_pOppRangeCtrlDelegate;

        CPTableView *m_pcptInertia;
        CtrlTableModel *m_pInertiaControlModel;
        CtrlTableDelegate *m_pInertiaCtrlDelegate;

        CPTableView *m_pcptAngles;
        CtrlTableModel *m_pAngleControlModel;
        CtrlTableDelegate *m_pAngleCtrlDelegate;

        QCheckBox *m_pchAdjustedVelocity;

};




