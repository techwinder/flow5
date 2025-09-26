/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QRadioButton>
#include <QSlider>

#include <xflcore/linestyle.h>

class XDirect;
class LineBtn;
class OpPointWt;

class OpPointCtrls : public QWidget
{
	Q_OBJECT

    public:
        OpPointCtrls(QWidget *pParent=nullptr);
        void setControls();
        void setOpPointWidget(OpPointWt *pOpPointWt);
        void stopAnimate();

        static void setXDirect(XDirect *pXDirect) {s_pXDirect = pXDirect;}

    private:
        void setupLayout();
        void connectSignals();

    public slots:
        void onBLCurveStyle(LineStyle);
        void onPressureCurveStyle(LineStyle);
        void onAnimateSpeed(int val);
        void onAnimate(bool bChecked);
        void onAnimateSingle();
        void onShowActiveOppOnly();

        void onShowPressure(bool bPressure);
        void onShowBL(bool bBL);
        void onFillFoil(bool bFill);

    private:
        QCheckBox *m_pchFillFoil;
        QCheckBox *m_pchShowBL, *m_pchShowPressure;

        LineBtn *m_plbBLStyle, *m_plbPressureStyle;

        QRadioButton *m_prbCpCurve, *m_prbQCurve;

        QCheckBox* m_pchAnimate, *m_pchShowActiveOppOnly;
        QSlider* m_pslAnimateSpeed;

        OpPointWt *m_pOpPointWt;

        bool m_bAnimate;           /**< true if a result animation is underway */
        bool m_bAnimatePlus;       /**< true if the animation is going from lower to higher alpha, false if decreasing */
        int m_posAnimate;          /**< the current aoa in the animation */
        QTimer *m_pAnimateTimer;

        static XDirect *s_pXDirect;
};

