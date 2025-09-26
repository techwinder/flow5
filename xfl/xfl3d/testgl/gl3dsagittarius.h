/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QComboBox>
#include <QDate>
#include <QCheckBox>
#include <QLabel>

#include <xfl3d/views/light.h>
#include <xfl3d/testgl/gl3dtestglview.h>
#include <xfl3d/testgl/spaceobject.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflgraph/graph/graph.h>

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






