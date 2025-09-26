/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QOpenGLShaderProgram>
#include <QRadioButton>
#include <QCheckBox>
#include <QSlider>
#include <QSettings>

#include <xfl3d/views/gl2dview.h>

#include <QLabel>


class IntEdit;
class FloatEdit;

class gl2dFractal : public gl2dView
{
    Q_OBJECT
    public:
        gl2dFractal(QWidget *pParent = nullptr);

        QPointF defaultOffset() override {return QPointF(+0.5*float(width()),0.0f);}

        void initializeGL() override;
        void glRenderView() override;
        void glMake2dObjects() override {}

        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onMode();
        void onSaveImage() override;

    private:
        QRadioButton *m_prbMandelbrot, *m_prbJulia;
        IntEdit *m_pieMaxIter;
        FloatEdit *m_pdeMaxLength;
        QLabel *m_plabScale;
        QCheckBox *m_pchShowSeed;
        QSlider *m_pslTau;


        QOpenGLBuffer m_vboRoots;
        QOpenGLBuffer m_vboSegs;

        QOpenGLShaderProgram m_shadFrac;
        // shader uniforms
        int m_locJulia;
        int m_locParam;
        int m_locIters;
        int m_locHue;
        int m_locLength;

        bool m_bResetRoots;
        int m_iHoveredRoot;
        int m_iSelectedRoot;
        float m_amp0, m_phi0; /** The seed's initial position */

        static int s_Hue;
        static int s_MaxIter;
        static float s_MaxLength;
        static QVector2D s_Seed;
};
