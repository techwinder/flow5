/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>

#include <xflfoil/editors/foildlg.h>


class Foil;
class IntEdit;
class FloatEdit;
class FoilWt;
class CubicSpline;
class LineBtn;

class FoilRepanelDlg : public FoilDlg
{
    Q_OBJECT

    public:
        FoilRepanelDlg(QWidget *pParent);
        ~FoilRepanelDlg();
        void initDialog(Foil* pFoil) override;


    private slots:
        void onApply() override;
        void onReset() override;
        void onBufferStyle(LineStyle ls);
        void onNPanels(int);

    private:

        void showEvent(QShowEvent *pEvent) override;
        void resizeEvent(QResizeEvent *) override;

        void setupLayout();

        QFrame *m_pBunchBox;
        QLabel *m_plabWarning;
        IntEdit  *m_pieNPanels;

        QSlider *m_pslBunchAmp;

        CubicSpline *m_pCS;
};

