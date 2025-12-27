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
#include <QFrame>
#include <QRadioButton>
#include <QDialogButtonBox>

class ColorBtn;

class ColorGradDlg : public QDialog
{
    Q_OBJECT
    public:
        ColorGradDlg(QVector<QColor>const &clrs, QWidget *parent = nullptr);

    public slots:
        void onColorBtn();
        void onNColors();
        void onButton(QAbstractButton *pButton);
        QVector<QColor> &colours() {return  m_Clr;}

    protected:
        QSize sizeHint() const override {return QSize(350,550);}

    private:
        QColor colour(float tau) const;
        void makeCtrlFrameLayout();
        void setupLayout();
        void updateColouredFrame();

    private:

        QFrame *m_pDiscreteClrFrame;
        QWidget *m_pTestClrFrame;

        QVector<ColorBtn*> m_pColorBtn;
        QVector<QColor> m_Clr;

        QRadioButton *m_prb2Colors;
        QRadioButton *m_prb3Colors;
        QDialogButtonBox *m_pButtonBox;
};

