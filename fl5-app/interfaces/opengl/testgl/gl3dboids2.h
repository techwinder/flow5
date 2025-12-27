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

#include <QLabel>
#include <QRadioButton>
#include <QSlider>
#include <QCheckBox>
#include <QStack>

#include <api/boid.h>
#include <interfaces/opengl/testgl/gl3dtestglview.h>
#include <api/vector3d.h>





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


