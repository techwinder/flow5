/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QFrame>

#include <xflfoil/editors/foildlg.h>

class PlainTextOutput;

class FoilNormalizeDlg : public FoilDlg
{
    Q_OBJECT

    public:
        FoilNormalizeDlg(QWidget *pParent);

        void initDialog(Foil *pFoil) override;

    private slots:
        void onReset() override;
        void onNormalize();
        void onDerotate();

    private:

        void setupLayout();
        void connectSignals();
        void updateProperties();
        void showEvent(QShowEvent *pEvent) override;
        void resizeEvent(QResizeEvent *) override;

    private:
        PlainTextOutput *m_ppto;
        QFrame *m_pOverlayFrame;
        QPushButton *m_ppbDerotate;
        QPushButton *m_ppbNormalize;
};


