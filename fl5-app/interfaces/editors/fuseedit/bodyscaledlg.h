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
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>

class IntEdit;
class FloatEdit;

class BodyScaleDlg : public QDialog
{
    Q_OBJECT

    public:
        BodyScaleDlg(QWidget *pParent=nullptr);
        void initDialog(bool bFrameOnly=false);

        double XFactor() const {return m_XFactor;}
        double YFactor() const {return m_YFactor;}
        double ZFactor() const {return m_ZFactor;}

        bool bFrameOnly() const {return m_bFrameOnly;}
        void setFrameOnly(bool bFrameOnly) {m_bFrameOnly=bFrameOnly;}
        void setFrameIndex(int iFrame) {m_FrameID = iFrame;}
        void enableFrameID(bool bEnable);

    private slots:
        void onOK();
        void onRadioBtn();
        void onEditingFinished();
        void onButton(QAbstractButton*pButton);

    private:
        void setupLayout();
        void enableControls();
        void keyPressEvent(QKeyEvent *event) override;

    private:

        QDialogButtonBox *m_pButtonBox;


        QRadioButton *m_prbBody, *m_prbFrame;
        FloatEdit *m_pdeXScaleFactor;
        FloatEdit *m_pdeYScaleFactor;
        FloatEdit *m_pdeZScaleFactor;
        IntEdit *m_pieFrameID;


    private:
        double m_XFactor, m_YFactor, m_ZFactor;
        bool m_bFrameOnly;
        int m_FrameID;
};

