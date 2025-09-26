/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QSettings>
#include <QStack>
#include <QRadioButton>
#include <xflgeom/geom3d/vector3d.h>

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
        FloatEdit *m_pdeDt;

        QRadioButton *m_prbRK1, *m_prbRK2, *m_prbRK4;

        LineBtn *m_plbFlowLines;
        QStack<int> m_stackInterval;


        static XPlane *s_pXPlane;
        static XSail *s_pXSail;

    public:
        static float s_Flowdt;
        static int s_FlowNGroups;
        static int s_iRK;
        static Vector3d s_FlowTopLeft, s_FlowBotRight;

};

