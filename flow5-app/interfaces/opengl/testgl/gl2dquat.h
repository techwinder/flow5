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
#include <QVector2D>
#include <QVector4D>

#include <interfaces/opengl/views/gl2dview.h>

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
