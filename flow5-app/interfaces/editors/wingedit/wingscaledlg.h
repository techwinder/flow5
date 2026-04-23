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
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QLabel>

class WingXfl;
class FloatEdit;

class WingScaleDlg : public QDialog
{
    Q_OBJECT

    public:
        WingScaleDlg(QWidget *pParent);
        void initDialog(double const &RefSpan, double const &RefChord, double const &RefSweep, double const &RefTwist, const double &RefArea, const double &RefAR, const double &RefTR);
        void initDialog(WingXfl *pWing);
        void keyPressEvent(QKeyEvent *event) override;

    private:
        void setupLayout();
        void readData();
        void setResults();
        void enableControls();

    private slots:
        void onOK();
        void onClickedCheckBox();
        void onEditingFinished();
        void onButton(QAbstractButton*pButton);


    public:
        QDialogButtonBox *m_pButtonBox;
        QCheckBox *m_pchSpan, *m_pchChord, *m_pchSweep, *m_pchTwist;
        QCheckBox *m_pchScaleArea, *m_pchScaleAR, *m_pchScaleTR;
        FloatEdit *m_pdeNewSpan, *m_pdeNewChord, *m_pdeNewSweep, *m_pdeNewTwist;
        FloatEdit *m_pdeNewArea, *m_pdeNewAR, *m_pdeNewTR;
        QLabel *m_plabRefSpan, *m_plabRefChord, *m_plabRefSweep, *m_plabRefTwist;
        QLabel *m_plabRefArea,*m_plabRefAR, *m_plabRefTR;
        QLabel *m_plabSpanRatio, *m_plabChordRatio, *m_plabSweepRatio, *m_plabTwistRatio;
        QLabel *m_plabAreaRatio,*m_plabARRatio, *m_plabTRRatio;

        bool m_bSweep, m_bSpan, m_bChord, m_bTwist;
        bool m_bArea, m_bAR, m_bTR;

        double m_NewSweep, m_NewChord, m_NewTwist, m_NewSpan, m_NewArea, m_NewAR, m_NewTR;
        double m_RefSweep, m_RefChord, m_RefTwist, m_RefSpan, m_RefArea, m_RefAR, m_RefTR;
};




