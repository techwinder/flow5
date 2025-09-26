/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#pragma once

#include <QDialog>
#include <QFrame>
#include <QDialogButtonBox>
#include <xflfoil/editors/foildlg.h>


class FloatEdit;

class FoilTEGapDlg : public FoilDlg
{
    Q_OBJECT

    public:
        FoilTEGapDlg(QWidget *pParent);

        void initDialog(Foil *pFoil) override;
        void showEvent(QShowEvent *pEvent) override;
        void resizeEvent(QResizeEvent*) override;

        void setupLayout();

    private slots:
        void onApply() override;


    private:
        QFrame * m_pOverlayFrame;
        FloatEdit *m_pdeBlend, *m_pdeGap;

        static double s_BlendLength;
};

