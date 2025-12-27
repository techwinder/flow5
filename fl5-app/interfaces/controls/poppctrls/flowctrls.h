/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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
#include <QLabel>
#include <QCheckBox>
#include <QSettings>
#include <QStack>
#include <QRadioButton>
#include <api/vector3d.h>

#define FLOWPERIOD 1

class XPlane;
class XSail;
class ExponentialSlider;
class FloatEdit;
class gl3dXPlaneView;
class gl3dXSailView;
class IntEdit;
class LineBtn;

class FlowCtrls : public QWidget
{
    Q_OBJECT

    public:
        FlowCtrls(QWidget *pParent=nullptr);

        enum flowODE {EULER, RK2, RK4};

        void initWidget();

        void updateUnits();
        void setFPS();
        void enableFlowControls();
        void updateFlowInfo();


        void set3dXPlaneView(gl3dXPlaneView *pView) {m_pgl3dXPlaneView=pView;}
        void set3dXSailView(gl3dXSailView *pView) {m_pgl3dXSailView=pView;}

        static void setXPlane(XPlane *pXPlane) {s_pXPlane=pXPlane;}
        static void setXSail(XSail *pXSail) {s_pXSail=pXSail;}


        static Vector3d flowTopLeft() {return s_FlowTopLeft;}
        static Vector3d flowBotRight() {return s_FlowBotRight;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    private:
        void setupLayout();
        void connectSignals();

    private slots:
        void onFlowLineStyle();
        void onFlowRestart();
        void onFlowUpdate();

    private:
        gl3dXPlaneView *m_pgl3dXPlaneView;
        gl3dXSailView *m_pgl3dXSailView;


        // FLow
        QLabel *m_plabNParticles;
        QLabel *m_plabFPS;

        QLabel *m_plabFlowLength[3];
        FloatEdit *m_pfeStart, *m_pfeEnd;
        FloatEdit *m_pfeTop,  *m_pfeBot;
        FloatEdit *m_pfeLeft, *m_pfeRight;

        IntEdit *m_pieNGroups;
        FloatEdit *m_pfeDt;

        QRadioButton *m_prbRK1, *m_prbRK2, *m_prbRK4;

        LineBtn *m_plbFlowLines;
        QStack<int> m_stackInterval;


        static XPlane *s_pXPlane;
        static XSail *s_pXSail;

    public:
        static float s_Flowdt;
        static int s_FlowNGroups;
        static flowODE s_ODE;
        static Vector3d s_FlowTopLeft, s_FlowBotRight;

};

