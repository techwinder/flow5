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

#include <QRadioButton>
#include <QCheckBox>
#include <QSlider>

#include <interfaces/opengl/testgl/gl3dtestglview.h>
#include <api/vector3d.h>
#include <api/linestyle.h>

class IntEdit;
class FloatEdit;
class LineBtn;

class gl3dAttractors : public gl3dTestGLView
{
    Q_OBJECT

    enum enumAttractor {LORENZ, NEWTON, THOMAS, DADRAS, CHENLEE, AIZAWA, ROSSLER,
                        SPROTT, FOURWINGS, HALVORSEN, RABINOVICH, NOSE, TCUCS1, ARNEODO};

    public:
        gl3dAttractors(QWidget *pParent = nullptr);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void glRenderView() override;
        void glMake3dObjects() override;

        void keyPressEvent(QKeyEvent *pEvent) override;
//        void showEvent(QShowEvent *pEvent) override;

        double f(double x, double y, double z) const;
        double g(double x, double y, double z) const;
        double h(double x, double y, double z) const;

    private slots:
        void moveThem();
        void onRandomSeed();
        void onLineStyle(LineStyle);
        void onAttractor();

    private:
        QTimer m_Timer;
        IntEdit *m_pieNTrace, *m_pieTailSize;
        QCheckBox *m_pchLeadingSphere, *m_pchDynColor;
        LineBtn *m_plbStyle;
        QSlider *m_pslSpeed;

        QVector<QRadioButton*> m_prbAttractors;

        QVector<QVector<Vector3d>> m_Trace;
        QVector<QVector<double>> m_Velocity;
        double m_MaxVelocity;

        QOpenGLBuffer m_vboPoints;
        QOpenGLBuffer m_vboTrace;

        bool m_bResetAttractor;

        int m_iLead;



        static int s_NTrace;
        static int s_TailSize;
        static enumAttractor s_iAttractor;
        static LineStyle s_ls;
        static bool s_bDynColor;
};
