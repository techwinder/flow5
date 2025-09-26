/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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

#include <xflfoil/editors/foildlg.h>
#include <xflgeom/geom2d/splines/cubicspline.h>
#include <xflgeom/geom2d/splines/bspline.h>

#include <xflcore/linestyle.h>

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

        QRadioButton *m_prbCS, *m_prbBS;

        QComboBox *m_pcbSplineDegree;

        IntEdit *m_pieNPanels;
        QSlider *m_pslBunchAmp;

        QCheckBox *m_pchShowCtrlPts, *m_pchShowNormals;

        LineBtn *m_plbSplineStyle;

        QCheckBox *m_pchClosedTE;


        static QByteArray s_HSplitterSizes;

        static CubicSpline s_CS;         /**< the instance of the CubicSpline object >*/
        static BSpline s_BS;             /**< the instance of the BSpline object >*/

        static bool s_bCubicSpline;
};
