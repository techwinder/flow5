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

#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QSettings>

#include <interfaces/opengl/controls/gl3dcontrols.h>

class XPlane;
class gl3dXPlaneView;
class gl3dXflView;
class gl3dGeomControls;
class Stab3dCtrls;
class Opp3dScalesCtrls;
class StreamLineCtrls;
class FlowCtrls;
class CrossFlowCtrls;

class POpp3dCtrls : public QTabWidget
{
    Q_OBJECT
    friend class XPlane;
    friend class gl3dXPlaneView;

    public:
        POpp3dCtrls(gl3dXflView*p3dView, QWidget *pParent=nullptr);
        ~POpp3dCtrls();

        void setControls();
        void stopAnimate();

        void initWidget();
        void updateUnits();

        bool getDistance() const;
        void stopDistance();

        void resetClipPlane();

        void loadSettings(QSettings &settings);
        void saveSettings(QSettings &settings);

        bool needsColorLegend() const;
        bool isFlowActive() const {return m_pchFlow->isChecked();}

        void checkPanels(bool b);

        void stopFlow();

        void set3dXPlaneView(gl3dXPlaneView *pView);
        static void setXPlane(XPlane *pXPlane);

    private:
        void setupLayout();
        void connectSignals();

    private slots:
        void on3dCp();
        void onGamma();
        void onPanelForce();
        void on3dPickCp();
        void onStripLift();
        void onShowIDrag();
        void onShowVDrag();
        void onShowTransitions();
        void onDownwash();
        void onPartForces();
        void onMoment();
        void onStreamlines(bool bStream);
        void onFlow(bool bFlow);
        void onFlaps();

        void onWakePanels();
        void onVortons();
        void onGround();
        void onAnimatePOpp();
        void onAnimatePOppSingle();
        void onAnimatePOppSpeed(int val);

    private:


        QCheckBox *m_pchGamma, *m_pchCp, *m_pchPanelForce;
        QCheckBox *m_pchStripLift, *m_pchIDrag, *m_pchVDrag, *m_pchTrans;
        QCheckBox *m_pchPartForces;
        QCheckBox *m_pchMoment, *m_pchDownwash;
        QCheckBox *m_pchStream, *m_pchFlow;
        QCheckBox *m_pchWakePanels, *m_pchVortons;
        QCheckBox *m_pchFlaps;
        QCheckBox *m_pchHPlane;
        QCheckBox *m_pchPickPanel;

        QCheckBox *m_pchPOppAnimate;
        QSlider *m_pslAnimPOppSpeed;

        bool m_bICd;                        /**< true if the induced drag forces should be displayed in the operating point or 3D view >*/
        bool m_bVCd;                        /**< true if the viscous drag forces should be displayed in the operating point or 3D view >*/
        bool m_bXTop;                       /**< true if the transition on the top surface should be displayed in the operating point or in 3D view >*/
        bool m_bXBot;                       /**< true if the transition on the bottom surface should be displayed in the operating point or in 3D view >*/
        bool m_bLiftStrip;                  /**< true if the lift curve should be displayed in the operating point or in the 3D view >*/
        bool m_bDownwash;                   /**< true if the arrows represeting downwash are to be displayed in the 3D openGl view */
        bool m_bPartForces;                 /**< true if the arrows representing Part Forces are to be displayed in the 3D openGl view */
        bool m_bMoments;                    /**< true if the arrows representing moments are to be displayed in the 3D openGl view */
        bool m_bStreamLines;                /**< true if the streamlines should be displayed in the operating point or 3D view*/
        bool m_bFlaps;
        bool m_bWakePanels;
        bool m_bVortons;
        bool m_bHPlane;
        bool m_bGamma;
        bool m_bPanelForce;
        bool m_b3dCp;
        bool m_bAnimateWOpp;                /**< true if there is an animation going on for an operating point */
        bool m_bAnimateWOppPlus;            /**< true if the animation is going in aoa crescending order */
        int m_posAnimateWOpp;       /**< the current animation aoa ind ex for WOpp animation */

        QTimer *m_pTimerWOpp;         /**< A pointer to the timer which signals the animation in the operating point and 3d view */

        Stab3dCtrls *m_pStab3dCtrls;
        Opp3dScalesCtrls *m_pOpp3dScalesCtrls;
        StreamLineCtrls *m_pStreamLinesCtrls;
        FlowCtrls *m_pFlowCtrls;
        CrossFlowCtrls *m_pCrossFlowCtrls;

        gl3dXPlaneView *m_pgl3dXPlaneView;

        gl3dGeomControls *m_pgl3dCtrls;

        static XPlane *s_pXPlane;
};


