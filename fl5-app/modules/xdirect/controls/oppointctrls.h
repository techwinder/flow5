/****************************************************************************

    flow5 application
    Copyright (C) 2025 André Deperrois 
    
    This file is part of flow5.

    flow5 is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    flow5 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with flow5.
    If not, see <https://www.gnu.org/licenses/>.


*****************************************************************************/


#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QRadioButton>
#include <QSlider>

#include <api/linestyle.h>

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
        void onNeutralLineStyle(LineStyle);
        void onBLCurveStyle(LineStyle);
        void onPressureCurveStyle(LineStyle);
        void onAnimateSpeed(int val);
        void onAnimate(bool bChecked);
        void onAnimateSingle();
        void onShowActiveOppOnly();

        void onShowPressure(bool bPressure);
        void onShowBL(bool bBL);
        void onFillFoil(bool bFill);
        void onNeutralLine(bool bShow);

    private:
        QCheckBox *m_pchFillFoil, *m_pchNeutralLine;
        QCheckBox *m_pchShowBL, *m_pchShowPressure, *m_pchShowInviscid;

        LineBtn *m_plbNeutralStyle, *m_plbBLStyle, *m_plbPressureStyle;

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

