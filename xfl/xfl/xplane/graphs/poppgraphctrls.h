/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QSlider>
#include <QTimer>
#include <xfl/graphs/graphtilectrls.h>


class POppGraphCtrls : public GraphTileCtrls
{
        Q_OBJECT
    public:
        POppGraphCtrls(GraphTiles *pParent=nullptr);
        ~POppGraphCtrls();

        void checkGraphActions() override;
        void connectSignals() override;
        void stopAnimate();

    private:
        void setupLayout() override;
        void setControls() override;

    public slots:
        void onAnimateWOpp(bool bAnimate);
        void onAnimateWOppSingle();
        void onAnimateWOppSpeed(int val);

    private:
        bool m_bAnimateWOpp;
        bool m_bAnimateWOppPlus;            /**< true if the animation is going in aoa crescending order */
        int m_posAnimateWOpp;               /**< the current animation aoa ind ex for WOpp animation */
        QTimer m_TimerWOpp;                 /**< A pointer to the timer which signals the animation in the operating point and 3d view */

        QCheckBox *m_pchWOppAnimate;
        QSlider *m_pslAnimateWOppSpeed;

        QCheckBox *m_pchShowActiveOppOnly;

};


