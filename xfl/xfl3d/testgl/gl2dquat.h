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

class gl2dQuat : public gl2dView
{
    Q_OBJECT
    public:
        gl2dQuat(QWidget *pParent = nullptr);

        QPointF defaultOffset() override {return QPointF(+0.5*float(width()),0.0f);}

        void initializeGL() override;
        void glRenderView() override;
        void glMake2dObjects() override {}


        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onSlice();
        void onSaveImage() override;

    private:
        IntEdit *m_pieMaxIter;
        FloatEdit *m_pfeMaxLength;
        QLabel *m_plabScale;
        QSlider *m_pslTau;

        QLabel *m_plabSlice[2];
        QSlider *m_pslSlice[2];
        QSlider *m_pslSeed[4];

        QRadioButton *m_prbSlice[6];

        QOpenGLShaderProgram m_shadQuat;
        // shader uniforms
        int m_locJulia;
        int m_locSeed;
        int m_locSlice;
        int m_locSlicer;
        int m_locIters;
        int m_locColor;
        int m_locLength;



        static int s_Hue;
        static int s_MaxIter;
        static int s_iSlice;
        static float s_MaxLength;
        static QVector4D s_Seed;
        static QVector2D s_Slicer;
};
