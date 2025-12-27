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

#include <QTabWidget>

#include <interfaces/editors/analysis3ddef/polar3ddlg.h>


class Boat;
class BoatPolar;
class IntEdit;
class FloatEdit;
class CPTableView;
class XflDelegate;

class SplinedGraphWt;

class BtPolarDlg : public Polar3dDlg
{
    Q_OBJECT

    public:
        BtPolarDlg(QWidget *pParent);
        ~BtPolarDlg();

        void initDialog(const Boat *pBoat, BoatPolar *pBtPolar=nullptr);
        void resizeEvent(QResizeEvent *pEvent) override;

        QString const &polarName() const {return m_BtPolarName;}

        static BoatPolar const &staticBtPolar() {return s_BtPolar;}

    private:
        void connectSignals();
        void readData() override;

        void setupLayout();
        void fillVariableList();
        void makeTables();
        void resizeColumns() override;
        void enableControls() override;

        void readBtVariables();
        void readWindGradient();
        void readFluidProperties() override;
        void readReferenceDimensions() override;
        void readMethodData() override;
        void checkMethods() override;
        void setPolar3dName() override;


    private slots:
        void onOK() override;
        void onReset() override;
        void onMethod() override;
        void onEditingFinished() override;
        void onNameOptions() override;
        void onAeroData() override;

        void onBtVariableChanged();
        void onButton(QAbstractButton*pButton);
        void onVortonWake() override;
        void onRefDims();

    private:
        //Variables
        static BoatPolar s_BtPolar;

        QString m_BtPolarName;

        Boat const *m_pBoat;

        QCheckBox *m_pchThickSails;

        CPTableView *m_pcptBtVariable;
        QStandardItemModel *m_pBtVariableModel;
        XflDelegate *m_pBtVariableDelegate;

        QTabWidget *m_pTabWidget;

        SplinedGraphWt *m_pWindGraphWt;

        FloatEdit *m_pdeXCoG, *m_pdeYCoG, *m_pdeZCoG;

        QCheckBox *m_pchUseSailDim;
        FloatEdit *m_pfeRefArea, *m_pfeRefChord;
};

