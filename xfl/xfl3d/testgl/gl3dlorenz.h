/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QLabel>

#include <xfl3d/testgl/gl3dtestglview.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflcore/linestyle.h>

class IntEdit;
class FloatEdit;
class LineBtn;
class GraphWt;

class gl3dLorenz : public gl3dTestGLView
{
    Q_OBJECT

    public:
        gl3dLorenz(QWidget *pParent = nullptr);
        ~gl3dLorenz() override;

        void hideGraphWt();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void glRenderView() override;
        void glMake3dObjects() override;

        void keyPressEvent(QKeyEvent *pEvent) override;


    private slots:
        void moveIt();
        void onResetDefaults();
        void onRestart();
        void onLineStyle(LineStyle);

    private:
        double f(double x, double y, double z) const;
        double g(double x, double y, double z) const;
        double h(double x, double y, double z) const;


    private:
        bool m_bResetAttractor;
        int m_Counter;
        int m_iLead;

        QElapsedTimer m_LastTime;

        QTimer *m_pTimer;
        QVector<Vector3d> m_Trace;

        QOpenGLBuffer m_vboTrace;

        FloatEdit *m_pdeSigma, *m_pdeRho, *m_pdeBeta;
        FloatEdit *m_pdeX, *m_pdeY, *m_pdeZ;
        IntEdit *m_pieIntervalms, *m_pieMaxPts;
        FloatEdit *m_pdeDt;
        LineBtn *m_plbStyle;

        QLabel *m_plabFrameRate;
        GraphWt *m_pGraphWt;

        static int s_RefreshInterval;
        static int s_MaxPts;
        static double s_dt;
        static double s_Sigma, s_Rho, s_Beta;
        static Vector3d s_P;
        static LineStyle s_ls;

};

