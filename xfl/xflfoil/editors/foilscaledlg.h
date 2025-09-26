/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QDialogButtonBox>

#include <xflfoil/editors/foildlg.h>


class FloatEdit;
class Foil;

class FoilWt;

class FoilScaleDlg : public FoilDlg
{
    Q_OBJECT

    public:
        FoilScaleDlg(QWidget *pParent);

        void initDialog(Foil *pFoil) override;


    private slots:
        void onReset() override;
        void onApply() override;
        void onCamberSlide(int pos);
        void onXCamberSlide(int pos);
        void onThickSlide(int pos);
        void onXThickSlide(int pos);
        void onCamber();
        void onXCamber();
        void onThickness();
        void onXThickness();

    private:
        void resizeEvent(QResizeEvent *) override;
        void showEvent(QShowEvent *pEvent) override;
        void setupLayout();


    private:
        QSlider	*m_pslCamberSlide, *m_pslThickSlide, *m_pslXThickSlide, *m_pslXCamberSlide;
        FloatEdit *m_pdeXCamber, *m_pdeXThickness, *m_pdeThickness, *m_pdeCamber;


    private:

        QFrame *m_pOverlayFrame;
//        double m_fCamber;
//        double m_fThickness;
//        double m_fXCamber;
//        double m_fXThickness;
};



