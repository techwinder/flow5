/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QCheckBox>
#include <QOpenGLTexture>

#include <xfl3d/testgl/gl3dtestglview.h>
#include <xflgeom/geom3d/vector3d.h>

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

