/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QSlider>

#include <xflfoil/editors/foildlg.h>


#include <xflcore/linestyle.h>

class Foil;
class FloatEdit;
class LineBtn;
class FoilWt;

class InterpolateFoilsDlg : public FoilDlg
{
    Q_OBJECT

    public:
        InterpolateFoilsDlg(QWidget *pParent);
        ~InterpolateFoilsDlg();

        void initDialogFoils(); // new name to avoid compiler warning for hidden method

        Foil const *interpolatedFoil() const {return m_pBufferFoil;}

    private:
        void showEvent(QShowEvent *pEvent) override;
        void showSelectedFoils();
        void setupLayout();
        void setFoil1();
        void setFoil2();
        void readParams() override;


    private slots:
        void onSelChangeFoil1(int);
        void onSelChangeFoil2(int);
        void onFrac();

        void onSlider(int val);
        void onApply() override;


    private:
        QComboBox *m_pcbFoil1, *m_pcbFoil2;
        QLabel *m_plabProps1, *m_plabProps2, *m_plabProps3;
        QSlider *m_pslMix;
        FloatEdit *m_pdeFrac;

    private:
        Foil *m_pFoil1, *m_pFoil2;
        double m_Frac;
};










