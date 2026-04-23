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

#include <QOpenGLShaderProgram>
#include <QRadioButton>
#include <QCheckBox>
#include <QSlider>
#include <QSettings>

#include <interfaces/opengl/views/gl2dview.h>

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
