/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once


#include <QCheckBox>
#include <QFrame>

#include <xflfoil/editors/foildlg.h>


class Foil;
class FoilWt;
class FloatEdit;


class FoilFlapDlg : public FoilDlg
{
    Q_OBJECT

    public:
        FoilFlapDlg(QWidget *pParent);

    public:
        void initDialog(Foil *pFoil) override;


    private:
        void enableTEFlap(bool bEnable);
        void enableLEFlap(bool bEnable);
        void readParams() override;
        void setupLayout();

    protected:
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

    private slots:
        void onLEFlapCheck();
        void onTEFlapCheck();

        void onOK() override;
        void onApply() override;

    private:
        bool m_bTEFlap;
        bool m_bLEFlap;


        double m_LEXHinge, m_LEYHinge, m_LEFlapAngle;
        double m_TEXHinge, m_TEYHinge, m_TEFlapAngle;


        QFrame *m_pOverlayFrame;
        FloatEdit	*m_pfeLEYHinge;
        FloatEdit	*m_pfeLEXHinge;
        FloatEdit	*m_pfeLEFlapAngle;
        FloatEdit	*m_pfeTEYHinge;
        FloatEdit	*m_pfeTEXHinge;
        FloatEdit	*m_pfeTEFlapAngle;

        QCheckBox *m_pchLEFlapCheck;
        QCheckBox *m_pchTEFlapCheck;

        QCheckBox *m_pchMakePermanent;
};

