/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QLabel>

#include <QCheckBox>
#include <QPushButton>

#include <xfl/opengl/gl3dxflview.h>
#include <xflobjects/objects3d/analysis/aeroforces.h>

class IntEdit;
class FloatEdit;
class PlainTextOutput;
class GraphWt;
class Graph;

class Vector3d;
class LineBtn;

class gl3dAxesView : public gl3dXflView
{
        Q_OBJECT
    public:
        gl3dAxesView(QWidget *pParent = nullptr);
        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:

        void glRenderView() override;
        void glMake3dObjects() override;

        QSize sizeHint() const override {return QSize(1100,1000);}

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        bool intersectTheObject(Vector3d const &,  Vector3d const &, Vector3d &) override {return false;}

        void readData();
        void setupLayout();
        void connectSignals();

    private slots:
        void onConvert();
        void onUpdateAxes();
        void onWindVecsStyle();
        void onBodyLineStyle();
        void onWindLineStyle();
        void onStabLineStyle();

    private:

        AeroForces m_AF;
        CartesianFrame m_CFBody;
        Vector3d m_WindDir, m_WindSide, m_WindNormal;
        FloatEdit *m_pdeAlpha, *m_pdeBeta;

        FloatEdit *m_pdeX, *m_pdeY, *m_pdeZ;

        PlainTextOutput *m_ppto;

        QCheckBox *m_pchWindVec;

        QCheckBox *m_pchVector;

        QCheckBox *m_pchGeomAxes;
        QCheckBox *m_pchBodyAxes;
        QCheckBox *m_pchWindAxes;
        QCheckBox *m_pchStabilityAxes;

        LineBtn *m_plbWindVecs, *m_plbBody, *m_plbWind, *m_plbStab;

        QPushButton *m_ppbConvert;


        static double s_Alpha;
        static double s_Beta;

        static Vector3d s_Vec;

        static LineStyle s_WindVecsStyle;
        static LineStyle s_BodyStyle, s_WindStyle, s_StabStyle;

        static QByteArray s_Geometry;
        static Quaternion s_ab_quat;
};

