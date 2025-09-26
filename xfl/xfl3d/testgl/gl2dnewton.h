/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QCheckBox>
#include <QOpenGLShaderProgram>
#include <QRadioButton>
#include <QSettings>

#include <xfl3d/views/gl2dview.h>
#include <xfl3d/views/shadloc.h>

#include <QLabel>



class IntEdit;
class FloatEdit;

class gl2dNewton : public gl2dView
{
    Q_OBJECT
    public:
        gl2dNewton(QWidget *pParent = nullptr);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        QPointF defaultOffset() override {return QPointF(0.0f,0.0f);}
        void initializeGL() override;
        void glRenderView() override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;

    private slots:
        void onAnimate(bool bAnimate);
        void onMoveRoots();
        void onNRoots();
        void onSaveImage() override;

    private:
        QOpenGLShaderProgram m_shadNewton;
        // shader uniforms
        int m_locIters;
        int m_locTolerance;
        int m_locColor[MAXROOTS];
        int m_locRoot[MAXROOTS];

        int m_locNRoots;


        bool m_bResetRoots;
        QOpenGLBuffer m_vboRoots;

        float m_amp0[MAXROOTS], m_phi0[MAXROOTS]; /** The roots initial position */
        QVector2D m_Root[MAXROOTS];  /** The roots current position */
        int m_Time;
        double m_omega[2*MAXROOTS];

        int m_iHoveredRoot;
        int m_iSelectedRoot;

        IntEdit *m_pieMaxIter;
        FloatEdit *m_pfeTolerance;
        QLabel *m_plabScale;
        QRadioButton *m_prb3roots, *m_prb5roots;
        QCheckBox *m_pchShowRoots;
        QCheckBox *m_pchAnimateRoots;

        QTimer m_Timer;

        static int s_MaxIter;
        static float s_Tolerance;
        static QColor s_Colors[MAXROOTS];

};

