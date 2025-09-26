/****************************************************************************

    Xfl3d application
    Copyright (C) Andre Deperrois
    License: GPL v3

*****************************************************************************/

#pragma once

#include <QLabel>
#include <QRadioButton>
#include <QSlider>
#include <QCheckBox>
#include <QStack>

#include <xflgeom/geom3d/boid.h>
#include <xfl3d/testgl/gl3dtestglview.h>
#include <xflgeom/geom3d/vector3d.h>





class IntEdit;
class FloatEdit;

class gl3dBoids2 : public gl3dTestGLView
{
    Q_OBJECT
    public:
        gl3dBoids2(QWidget *pParent = nullptr);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;

        void initializeGL() override;
        void glRenderView() override;
        void glMake3dObjects() override;


    private slots:
        void onSlider();
        void onSwarmReset();

    private:
        QOpenGLShaderProgram m_shadBoids;

        int m_locCube;
        int m_locWidth;
        int m_locHeight;
        int m_locCohesion;
        int m_locSeparation;
        int m_locAlignment;
        int m_locMaxSpeed;
        int m_locHasPredator;
        int m_locPredator;

        bool m_bResetBox;
        bool m_bResetInstances = true;

        float m_BoxWidth;

        IntEdit *m_pieNGroups;
        QSlider *m_pslCohesion;
        QSlider *m_pslSeparation;
        QSlider *m_pslAlignment;
        QSlider *m_pslPredator;
        QSlider *m_pslRatio;
        QSlider *m_pslMaxSpeed;

        QLabel *m_plabNMaxGroups;
        QLabel *m_plabNParticles;
        QCheckBox *m_pchPredator;
        QCheckBox *m_pchTrace;

        QLabel *m_plabCohesion, *m_plabAlignment, *m_plabSeparation, *m_plabPredator, *m_plabMaxSpeed;

//        QRadioButton *m_prbBox, *m_prbSphere;
        QSlider *m_pslBoxOpacity;

        QOpenGLBuffer m_vboBoids, m_vboTraces;

        QTimer m_Timer;

        QLabel *m_plabFrameRate;
        QStack<int> m_stackInterval;

        static int s_NGroups;
        static float s_Cohesion;
        static float s_Separation;
        static float s_Alignment;
        static float s_Predator;
        static float s_Ratio;
        static float s_MaxSpeed;

};


