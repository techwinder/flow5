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

#include <QCheckBox>
#include <QOpenGLTexture>

#include <interfaces/opengl/testgl/gl3dtestglview.h>
#include <api/vector3d.h>

class GLLightDlg;
class ExponentialSlider;
class gl3dShadow : public gl3dTestGLView
{
    Q_OBJECT
    public:
        gl3dShadow(QWidget *pParent=nullptr);
        ~gl3dShadow();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void glRenderView() override;
        void glMake3dObjects() override;

        void renderToShadowMap();
        void renderToScreen();


    private slots:
        void onObjectPos();
        void onTexture(bool bTex);
        void onControls();
        void onLightSettings(bool bShow);

    private:
        ExponentialSlider *m_peslXObj, *m_peslYObj, *m_peslZObj;
        QCheckBox *m_pchLightDlg;

        QOpenGLBuffer m_vboSphere, m_vboSphereEdges;
        QOpenGLBuffer m_vboBackgroundQuad;

        QOpenGLTexture *m_ptexQuad;

        bool m_bResetObjects;
        Vector3d m_Object;

        QTimer m_CtrlTimer;
};

