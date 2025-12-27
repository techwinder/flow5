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

#include <QObject>
#include <QWidget>
#include <QCheckBox>
#include <QToolButton>
#include <QPushButton>
#include <QSlider>
#include <QFrame>
#include <QSettings>


#include <interfaces/opengl/controls/gl3dcontrols.h>

class XSail;
class gl3dXSailView;

class XSailDisplayCtrls : public gl3dControls
{
    friend class XSail;
    friend class LegendColor;
    friend class gl3dXSailView;

    Q_OBJECT

    public:
        XSailDisplayCtrls(gl3dXSailView *pgl3dBoatView, Qt::Orientation orientation, bool bRowLayout);

    public:
        void setControls() override;
        void stopAnimate();
        void cancelStream();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        static void setXSail(XSail *pXSail) {s_pXSail=pXSail;}

    private:
        void setupLayout(Qt::Orientation orientation, bool bInColums);
        void connectSignals();

    protected:
        void showEvent(QShowEvent *pEvent) override;

    public slots:
        void on3dCp();
        void onGamma();
        void onPanelForce();
        void on3dPickCp();
        void onShowLift();
        void onMoment();
        void onStreamlines();
        void onFlow();
        void onWind();
        void onWater();
        void onWakePanels();
        void onVortons();
        void onAnimateWOpp();
        void onAnimateWOppSingle();
        void onAnimateWOppSpeed(int val);

    private:
        QSlider *m_pslAnimateWOppSpeed;
        QCheckBox *m_pchPartForces, *m_pchWOppAnimate;
        QCheckBox *m_pchMoment, *m_pchStream, *m_pchFlow;
        QCheckBox *m_pchCp, *m_pchPanelForce, *m_pchGamma;
        QCheckBox *m_pchWakePanels, *m_pchVortons;
        QCheckBox *m_pchWind, *m_pchWater;
        QCheckBox *m_pchPickPanel;

        static bool s_bPartForces;                       /**< true if the lift curve should be displayed in the operating point or in the 3D view >*/
        static bool s_bMoments;                    /**< true if the arrows representing moments are to be displayed on the 3D openGl view */
        static bool s_bStreamLines;                     /**< true if the streamlines should be displayed in the operating point or 3D view*/
        static bool s_bFlow;
        static bool s_bWind;
        static bool s_bWater;
        static bool s_bVortons;
        static bool s_bWakePanels;
        static bool s_bGamma;
        static bool s_bPanelForce;
        static bool s_b3dCp;

        bool m_bAnimateWOpp;                /**< true if there is an animation going on for an operating point */
        bool m_bAnimateWOppPlus;            /**< true if the animation is going in aoa crescending order */
        int m_posAnimateWOpp;       /**< the current animation aoa ind ex for WOpp animation */

        QTimer *m_pTimerWOpp;         /**< A pointer to the timer which signals the animation in the operating point and 3d view */

        QCheckBox *m_pchAxes;

        QCheckBox *m_pchSurfaces, *m_pchOutline;
        QCheckBox *m_pchPanels;
        QToolButton *m_ptbWindBack, *m_ptbWindFront;


        static XSail *s_pXSail;
        gl3dXSailView *m_pgl3dXSailView;
};





