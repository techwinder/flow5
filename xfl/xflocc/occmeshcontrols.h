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
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QDialogButtonBox>

#include <xflocc/occmeshparams.h>


class FloatEdit;
class IntEdit;
class OccMeshControls : public QDialog
{
    Q_OBJECT

    public:
        OccMeshControls(QWidget *parent = nullptr);
        void initDialog(const OccMeshParams &params, bool bShowBtnBox);

        void readParams();

        void updateUnits();

        bool isChanged() const {return m_bChanged;}

        bool isRelativeDeflection() {return m_Params.isRelativeDeflection();}
        void setRelativeDeflection(bool bAbs) {m_Params.setDefRelative(bAbs);}

        double linearDefAbsolute() {return m_Params.deflectionAbsolute();}
        void setLinearDefAbsolute(double sz) {m_Params.setDefAbsolute(sz);}

        double linearDeflectionRel() {return m_Params.deflectionRelative();}
        void setLinearDeflectionRel(double sz) {m_Params.setDefRelative(sz);}

        double angularDeviation() {return m_Params.angularDeviation();}
        void setAngularDeviation(double theta) {m_Params.setAngularDeviation(theta);}

        OccMeshParams const &occParameters() const {return m_Params;}

    private:
        void setupLayout();
        void connectSignals();
        void setControls();
        void showEvent(QShowEvent *pEvent) override;

    private slots:
        void onParamChanged();
        void accept() override;

    private:
        bool m_bChanged;

        FloatEdit *m_pdeLinDefAbs, *m_pdeLinDefRel, *m_pdeAngDeviation;
        QLabel *m_plabAbsUnit, *m_plabRelUnit;
        QLabel *m_pLabLinDefAbs, *m_pLabLinDefRel;

        QRadioButton *m_prbLinAbs, *m_prbLinRel;

        QDialogButtonBox *m_pButtonBox;

        OccMeshParams m_Params;
};


