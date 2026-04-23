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

#include <QLabel>
#include <QStack>

#include <interfaces/opengl/testgl/gl3dtestglview.h>


class IntEdit;
class FloatEdit;


struct LorenzBO
{
    QVector4D position;
    QVector4D color;
};


class gl3dLorenz2 : public gl3dTestGLView
{
    Q_OBJECT
    public:
        gl3dLorenz2(QWidget *pParent = nullptr);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    private:
        void initializeGL() override;
        void glRenderView() override;
        void glMake3dObjects() override;

    private slots:
        void onRestart();
        void onParticleSize();

    private:
        QOpenGLVertexArrayObject m_vao; /** generic vao required for the core profile >3.x*/
        QOpenGLShaderProgram m_shadLorenz2;

        // CS uniforms
        int m_locRadius, m_locDt;

        // VS/FS uniforms
        int m_locPosition, m_locFillColor;

        QOpenGLBuffer m_ssbParticle;

        bool m_bResetParticles;

        QTimer m_Timer;

        FloatEdit *m_pdeScatter;
        FloatEdit *m_pdeX, *m_pdeY, *m_pdeZ;
        FloatEdit *m_pdeDt;

        IntEdit *m_pieNGroups;
        QLabel *m_plabNMaxGroups;
        QLabel *m_plabNParticles;

        QLabel *m_plabFrameRate;

        FloatEdit *m_pdeParticleSize;

        QStack<int> m_stackInterval;

        static double s_X0, s_Y0, s_Z0;
        static double s_Scatter;
        static double s_dt;
        static float s_Size;


        static int s_NGroups;
};


