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

class Sail;
class FloatEdit;

class SailScaleDlg : public QDialog
{
    Q_OBJECT
    public:
        SailScaleDlg(QWidget *pParent);

        void initDialog(Sail *pSail);
        void keyPressEvent(QKeyEvent *pEvent) override;

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

        QCheckBox *m_pchScaleArea, *m_pchScaleAR, *m_pchTwist;
        FloatEdit *m_pdeNewTwist, *m_pdeNewArea, *m_pdeNewAR;

        QLabel *m_plabRefArea, *m_plabRefAR, *m_plabRefTwist;
        QLabel *m_plabAreaRatio,*m_plabARRatio, *m_plabTwistRatio;

        bool m_bArea, m_bAR, m_bTwist;

        double m_NewArea, m_NewAR, m_NewTwist;
        double m_RefArea, m_RefAR, m_RefTwist;
};




