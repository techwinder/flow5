/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

#include <xfl3d/testgl/gl3dtestglview.h>
#include <xflgeom/geom3d/boid.h>

#include <xflpanels/panels/vortex.h>

class IntEdit;
class FloatEdit;



class gl3dFlowVtx : public gl3dTestGLView
{
    Q_OBJECT
    public:
        gl3dFlowVtx(QWidget *pParent = nullptr);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void initializeGL() override;
        void glRenderView() override;
        void glMake3dObjects() override;

        void makeVortices();
        void makeBoids();

        void readParams();

    public slots:
        void onPause();
        void onRestart();
        void moveThem();

    private:
        QLabel *m_plabNMaxGroups;
        QLabel *m_plabNParticles;
        IntEdit *m_pieNGroups;
        FloatEdit *m_pfeGamma;
        FloatEdit *m_pfeVInf;
        FloatEdit *m_pfeDt;

        QTimer m_Timer;
        int m_Period;

        QOpenGLShaderProgram m_shadCompute;
        QOpenGLBuffer m_vboBoids, m_vboTraces;
        QOpenGLBuffer m_vboVortices;
        QOpenGLBuffer m_ssboVortices;

        int m_locGamma;
        int m_locVInf;
        int m_locNVortices;
        int m_locDt;
        int m_locRandSeed;

        Vortex m_Vortex;
        QVector<Boid> m_Boid;

        bool m_bResetBoids;
        bool m_bResetVortices;

        static float s_dt, s_VInf, s_Gamma;
        static int s_NGroups;
};
