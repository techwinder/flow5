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
#include <xflfoil/objects2d/splinefoil.h>

class Foil;
class IntEdit;
class FloatEdit;
class FoilWt;
class SplineFoilCtrls;

class Foil2SplineDlg : public FoilDlg
{
    Q_OBJECT
    public:
        Foil2SplineDlg(QWidget *pParent);
        void initDialog(Foil* pFoil) override;

    private:
        void connectSignals();
        void setupLayout();
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void resizeEvent(QResizeEvent *) override;
        void clearStack(int pos=0);
        void setPicture();

    private slots:
        void onApply() override;
        void onReset() override;
        void onSplitterMoved();
        void onUndo();
        void onRedo();
        void onNewSpline();
        void onTakePicture();

    private:
        int m_StackPos;                       /**< the current position on the Undo stack >*/
        int m_CStackPos;                       /**< the current position on the Undo stack >*/
        QStack<SplineFoil> m_SFStack;        /**< the stack of incremental modifications to the B-SpLine >*/

        QPushButton *m_ppbUndo, *m_ppbRedo;

        QSplitter *m_pHSplitter;
        SplineFoilCtrls *m_pSFCtrls;

        static QByteArray s_HSplitterSizes;

    public:
        static SplineFoil s_SF;
};

