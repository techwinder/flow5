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


#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSplitter>
#include <QStack>

#include <interfaces/editors/foiledit/foildlg.h>
#include <api/cubicspline.h>
#include <api/bspline.h>
#include <api/bezierspline.h>
#include <api/linestyle.h>

class IntEdit;
class Spline;
class LineBtn;

class Foil;
class IntEdit;
class FloatEdit;
class FoilWt;

class Foil1SplineDlg : public FoilDlg
{
    Q_OBJECT

    public:
        Foil1SplineDlg(QWidget *pParent);
        void initDialog(Foil* pFoil) override;

        static CubicSpline &C3Spline() {return s_CS;}
        static BSpline &Bspline() {return s_BS;}

    private:
        void connectSignals();
        void setupLayout();
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void resizeEvent(QResizeEvent *) override;
        void clearBStack(int pos=0);
        void clearCStack(int pos=0);
        void setPicture();
        void fillCtrls(Spline const*pSpline);

        bool isBSpline() const {return s_SplineType==Spline::BSPLINE;}
        bool isBezier()  const {return s_SplineType==Spline::BEZIER;}
        bool isCubic()   const {return s_SplineType==Spline::CUBIC;}

        Spline *getActiveSpline();

    private slots:
        void onApply() override;
        void onReset() override;
        void onSplitterMoved();
        void onUndo();
        void onRedo();
        void onMakeDefaultSpline();
        void onTakePicture();
        void onApproxFoil();
        void onClosedTE();
        void onOutputSize();
        void onSplineStyle(LineStyle);
        void onShowCtrlPts(bool bShow);
        void onShowNormals(bool bNormals);
        void onSplineDegree();
        void onSplineType();
        void onBunch(int);

    private:

        int m_BStackPos;                       /**< the current position on the Undo stack >*/
        int m_CStackPos;                       /**< the current position on the Undo stack >*/
        QStack<BSpline> m_BSplineStack;        /**< the stack of incremental modifications to the B-SpLine >*/
        QStack<CubicSpline> m_CSplineStack;    /**< the stack of incremental modifications to the CubicSpLine >*/

        QPushButton *m_ppbUndo, *m_ppbRedo;
        QPushButton *m_ppbActionsMenuBtn;
        QSplitter *m_pHSplitter;

        QRadioButton *m_prbBS, *m_prbBZ, *m_prbCS;

        QComboBox *m_pcbSplineDegree;

        IntEdit *m_pieNPanels;
        QSlider *m_pslBunchAmp;

        QCheckBox *m_pchShowCtrlPts, *m_pchShowNormals;

        LineBtn *m_plbSplineStyle;

        QCheckBox *m_pchClosedTE;

        QAction *m_pActApproxFoil;

        static QByteArray s_HSplitterSizes;

        static CubicSpline s_CS;         /**< the instance of the CubicSpline object >*/
        static BSpline s_BS;             /**< the instance of the BSpline object >*/
        static BezierSpline s_BZ;
        static Spline::enumType s_SplineType;
};
