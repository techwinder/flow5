/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>

#include <xfl/opengl/gl3dxflview.h>
#include <xflgeom/geom3d/triangle3d.h>
#include <xflpanels/panels/panel3.h>
#include <xflpanels/panels/vorton.h>

class IntEdit;
class FloatEdit;
class Vortex;

class gl3dSingularity : public gl3dXflView
{
    Q_OBJECT

    public:
        gl3dSingularity(QWidget *pParent=nullptr);
        ~gl3dSingularity();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void glRenderView() override;
        void glMake3dObjects() override;
        bool intersectTheObject(Vector3d const&, Vector3d const &, Vector3d &) override {return false;}
        QSize sizeHint() const override {return QSize(1500, 900);}
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

    private slots:
        void onUpdateData();

    private:
        QRadioButton *m_prbSource, *m_prbVortex, *m_prbDoublet;

        IntEdit *m_pieNCircles;
        IntEdit *m_pieNArrows;
        FloatEdit *m_pfeArrowScale;
        FloatEdit *m_pfeInnerRadius;
        FloatEdit *m_pfeProgression;

        QOpenGLBuffer m_vboArrows;

        bool m_bResetArrows;

        Vortex *m_pVortex;

        static int s_NCircles;
        static int s_NArrows;
        static float s_ArrowLength;
        static float s_Radius;
        static float s_Progression;
};

