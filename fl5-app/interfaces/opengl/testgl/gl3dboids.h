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
#include <QSlider>

#include <interfaces/opengl/testgl/gl3dtestglview.h>
#include <api/vector3d.h>
#include <api/boid.h>



class IntEdit;
class FloatEdit;

class gl3dBoids : public gl3dTestGLView
{
    Q_OBJECT
    public:
        gl3dBoids(QWidget *pParent = nullptr);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;

        void glRenderView() override;
        void glMake3dObjects() override;

        void makeBoids(int size);

        void moveBoidBlock(int iBlock, Vector3d *accel);

        Vector3d cohesionForce(  const Boid &boid);
        Vector3d separationForce(const Boid &boid);
        Vector3d alignmentForce( const Boid &boid);
        Vector3d borderForce(    const Boid &boid);

    private slots:
        void onMoveBoids();
        void onSlider();
        void onSwarmReset();

    private:
        bool m_bResetBox;
        bool m_bResetInstances = true;

        double m_X, m_Y, m_Z;

        double m_Radius;

        QVector<Boid> m_Boids;

        IntEdit *m_pieFlockSize;
        QSlider *m_pslCohesion;
        QSlider *m_pslSeparation;
        QSlider *m_pslAlignment;

        QLabel *m_plabCohesion, *m_plabAlignment, *m_plabSeparation;

        QSlider *m_pslBoxOpacity;

        QTimer m_Timer;

        QOpenGLBuffer m_vboBox, m_vboBoxEdges;
        QOpenGLBuffer m_vboInstPositions;

        int m_nBlocks;

        static int s_FlockSize;
        static double s_Cohesion;
        static double s_Separation;
        static double s_Alignment;
};


