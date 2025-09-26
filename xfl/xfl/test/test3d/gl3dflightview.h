/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QListWidget>
#include <QCheckBox>

#include <xfl3d/testgl/gl3dtestglview.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflgeom/geom3d/triangle3d.h>

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

        QVector<Triangle3d> m_Triangles;

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


