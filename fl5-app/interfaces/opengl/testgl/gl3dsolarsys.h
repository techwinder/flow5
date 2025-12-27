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

#include <QDate>
#include <QLabel>

#include <interfaces/opengl/views/light.h>
#include <interfaces/opengl/testgl/gl3dtestglview.h>
#include <interfaces/opengl/testgl/spaceobject.h>
#include <api/vector3d.h>

class IntEdit;
class FloatEdit;


class gl3dSolarSys : public gl3dTestGLView
{
    Q_OBJECT

    public:
        gl3dSolarSys(QWidget *pParent = nullptr);
        ~gl3dSolarSys() override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onMovePlanets();
        void onRestart();
        void onPlanetSize(int size);

        void onCeres(bool bShow);
        void onHalley(bool bShow);


    private:
        void hideEvent(QHideEvent *pEvent) override;
        void initializeGL() override;
        void glRenderView() override;
        void glMake3dObjects() override;
        void keyPressEvent(QKeyEvent *pEvent) override;


        void makePlanets();


    private:
        QDate m_Elapsed;
        bool m_bResetPlanets;

        bool m_bCeres;
        bool m_bHalley;

        QVector<Planet> m_Planet;

        Planet m_Ceres;
        Planet m_Halley;

        QTimer m_Timer;

        FloatEdit *m_pdeDt, *m_pdePlanetSize;

        QLabel *m_plabDate;
        QLabel *m_plabHalley;

        QVector<QOpenGLBuffer> m_vboCircle;
        QOpenGLBuffer m_vboSaturnDisk;
        QOpenGLBuffer m_vboCeresEllipse;
        QOpenGLBuffer m_vboHalleyEllipse;


        Light m_RefLight;

        static double s_dt;
        static double s_PlanetSize;
};


