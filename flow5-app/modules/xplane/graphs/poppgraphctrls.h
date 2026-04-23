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

#include <QSlider>
#include <QTimer>
#include <interfaces/graphs/controls/graphtilectrls.h>


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


