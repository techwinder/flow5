/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QPushButton>
#include <QCheckBox>
#include<QLabel>
#include <QSlider>

#include <xfl3d/testgl/gl3dtestglview.h>
#include <xflgeom/geom3d/vector3d.h>

class IntEdit;
class FloatEdit;
class PlainTextOutput;

class gl3dQuat : public gl3dTestGLView
{
    Q_OBJECT

    public:
        gl3dQuat(QWidget *pParent = nullptr);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    private:
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        void initializeGL() override;
        void glMake3dObjects() override;
        void glRenderView() override;


    private slots:
        void onUpdateInput();

    private:

        QOpenGLBuffer m_vboVertices;

        bool m_bResetSegs;
        QVector<Vector3d> m_LineStrip;

        QSlider *m_pslQuat[4];
        QCheckBox *m_pchTriangles;
        QCheckBox *m_pchOutline;
        PlainTextOutput *m_ppto;

        Quaternion m_Qt[3];

        Vector3d m_Vector[3];

        static QByteArray s_Geometry;
        static Quaternion s_Quat;

};


