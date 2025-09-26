/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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

#include <xflfoil/editors/foildlg.h>

#include <xflgeom/geom2d/splines/bspline.h>

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




