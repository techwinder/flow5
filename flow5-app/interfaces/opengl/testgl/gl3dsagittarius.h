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

#include <QComboBox>
#include <QDate>
#include <QCheckBox>
#include <QLabel>

#include <interfaces/opengl/views/light.h>
#include <interfaces/opengl/testgl/gl3dtestglview.h>
#include <interfaces/opengl/testgl/spaceobject.h>
#include <api/vector3d.h>
#include <interfaces/graphs/graph/graph.h>

class IntEdit;
class FloatEdit;
class GraphWt;

class gl3dSagittarius : public gl3dTestGLView
{
    Q_OBJECT

    public:
        gl3dSagittarius(QWidget *pParent = nullptr);
        ~gl3dSagittarius() override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onMoveStars();
        void onRestart();
        void onStarSelection();

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;

        void glRenderView() override;
        void glMake3dObjects() override;

        void makeStars();
        Planet const &selectedStar() const;

    private:

        QDate m_Started, m_Current;
        bool m_bResetStars;
        bool m_bResetTrail;

        int m_iLead;

        QVector<Planet> m_Star;

        QTimer m_Timer;

        IntEdit *m_pieSteps;

        FloatEdit *m_pdeDt;

        QLabel *m_plabInfo;
        QCheckBox *m_pchMultiThread;
        QCheckBox *m_pchEllipse;
        QComboBox *m_pcbStar;

        GraphWt *m_pGraphDistWt;
        Graph m_GraphDist;

        GraphWt *m_pGraphVelWt;
        Graph m_GraphVel;

        QVector<QOpenGLBuffer> m_vboStar;
        QVector<QOpenGLBuffer> m_vboEllipse;
        QOpenGLBuffer m_vboEllipseFan;

        QVector<QOpenGLBuffer> m_vboTrace;
        QVector<QVector<Vector3d>> m_Trace;

        static bool s_bMultithread;
        static int s_nStepsPerDay;
        static double s_dt;
        static int s_TailSize;
};






