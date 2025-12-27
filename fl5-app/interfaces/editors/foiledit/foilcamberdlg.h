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
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QSlider>
#include <QStack>
#include <QSplitter>

#include <interfaces/editors/foiledit/foildlg.h>

#include <api/bspline.h>

class Foil;
class IntEdit;
class FloatEdit;
class FoilWt;
class SplineCtrl;


class FoilCamberDlg : public FoilDlg
{
    Q_OBJECT

    public:
        FoilCamberDlg(QWidget *pParent);
        void initDialog(Foil* pFoil) override;

        static Spline &CSpline() {return s_CSpline;}
        static Spline &TSpline() {return s_TSpline;}

    private:
        void connectSignals();
        void setupLayout();
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void resizeEvent(QResizeEvent *) override;
        void clearStack(int pos=0);
        void setPicture();
        void makeDefaultSplines();
        void makeFoilFromSplines();

    private slots:
        void onApply() override;
        void onReset() override;
        void onSplitterMoved();
        void onUndo();
        void onRedo();
        void onNewSpline();
        void onTakePicture();
        void onApproxFoil();

    private:
        int m_StackPos;
        QStack<BSpline> m_CStack;
        QStack<BSpline> m_TStack;

        SplineCtrl *m_pCCtrls, *m_pTCtrls;

        QPushButton *m_ppbActionsMenuBtn;
        QPushButton *m_ppbUndo, *m_ppbRedo;

        QSplitter *m_pHSplitter;

        static QByteArray s_HSplitterSizes;

    public:
        static BSpline s_CSpline;
        static BSpline s_TSpline;
};




