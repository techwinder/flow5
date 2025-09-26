/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QOpenGLShaderProgram>
#include <QRadioButton>
#include <QCheckBox>
#include <QSlider>
#include <QSettings>

#include <xfl3d/views/gl2dview.h>

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

