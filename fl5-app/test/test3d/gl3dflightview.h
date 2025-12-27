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

#include <QListWidget>
#include <QCheckBox>

#include <interfaces/opengl/testgl/gl3dtestglview.h>
#include <api/vector3d.h>
#include <api/triangle3d.h>

class PlaneSTL;
class Triangle3d;
class GLLightDlg;

class gl3dFlightView : public gl3dTestGLView
{
    Q_OBJECT

    public:
        gl3dFlightView(QWidget *pParent = nullptr);
        ~gl3dFlightView();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        void glRenderView() override;

    private:
        void glMake3dObjects() override;
        void keyPressEvent(QKeyEvent *pEvent) override;

        void restartTimer();
        void renderToShadowMap();
        void renderToScreen();

    private slots:
        void moveIt();
        void onObjectScale(int size);

    private:
        double f(double x, double y, double z) const;
        double g(double x, double y, double z) const;
        double h(double x, double y, double z) const;

    private:
        QCheckBox *m_pchLightDlg;

        bool m_bResetObject;
        bool m_bResetTrace;

        std::vector<Triangle3d> m_Triangles;

        QOpenGLBuffer m_vboBackgroundQuad;


        QOpenGLBuffer m_vboStlTriangulation;
        QOpenGLBuffer m_vboStlOutline;

        QTimer m_Timer;

        int m_iLead;
        QVector<Vector3d> m_Trace;
        QOpenGLBuffer m_vboTrace;

        QMatrix4x4 m_matPlane;

        static LineStyle s_ls;
        static double s_PlaneScale;
};


