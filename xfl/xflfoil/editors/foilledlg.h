/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QFrame>

#include <xflfoil/editors/foildlg.h>


class FloatEdit;

class FoilLEDlg : public FoilDlg
{
    Q_OBJECT

    public:
        FoilLEDlg(QWidget *pParent);

        void initDialog(Foil *pFoil) override;

    private slots:
        void onApply() override;
        void onLERadiusDisplay();

    private:

        void setupLayout();
        void showEvent(QShowEvent *pEvent) override;
        void resizeEvent(QResizeEvent *) override;

    private:
        QFrame *m_pOverlayFrame;

        FloatEdit *m_pfeDisplayRadius;
        FloatEdit *m_pfeBlend, *m_pfeLEfactor;

        static double s_LErfac;
        static double s_BlendLength;
        static double s_LERadius;
};

