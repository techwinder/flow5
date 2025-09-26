/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDialogButtonBox>

#include <xflfoil/editors/foildlg.h>


class Foil;
class IntEdit;
class FoilWt;

class NacaFoilDlg : public FoilDlg
{
    Q_OBJECT

    public:
        NacaFoilDlg(QWidget *pParent);

        void setupLayout();

        void showEvent(QShowEvent *pEvent) override;
        void resizeEvent(QResizeEvent *) override;

        Foil * pNACAFoil() const {return m_pBufferFoil;}

        int digits() const {return m_Digits;}

    private:
        bool makeNaca4(int digits);
        bool makeNaca5(int digits);
        void makeThickness(double t);

    private slots:
        void onEditingFinished();
        void onApply() override;

    private:

        QFrame *m_pOverlayFrame;

        QLineEdit *m_pleNumber;
        IntEdit *m_piePanels;
        QLabel * m_plabMessage;


        int m_Digits;

        static int s_nPanels;
};



