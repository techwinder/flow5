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
class PlainTextOutput;

class gl2dComplex : public gl2dView
{
    Q_OBJECT
    public:
        gl2dComplex(QWidget *pParent = nullptr);


        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    private:
        QPointF defaultOffset() override {return QPointF(0.0f,0.0f);}
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        void initializeGL() override;
        void glRenderView() override;
        void glMake2dObjects() override {}


    private slots:
        void onSaveImage() override;


    private:

        PlainTextOutput *m_ppto;
        QLabel *m_plabScale;

        QOpenGLShaderProgram m_shadComplex;
        // shader uniforms
        int m_locZeta;
        static QByteArray s_Geometry;

};

