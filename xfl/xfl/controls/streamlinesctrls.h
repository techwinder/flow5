/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QSlider>
#include <QRadioButton>
#include <QPushButton>
#include <QSettings>

class XPlane;
class XSail;
class ExponentialSlider;
class FloatEdit;
class gl3dXPlaneView;
class gl3dXSailView;
class IntEdit;
class LineBtn;
class FloatEdit;

class StreamLineCtrls : public QWidget
{
    Q_OBJECT
    public:

        enum eStreamStart {TRAILINGEDGE, Y_LINE, Z_LINE};

    public:
        StreamLineCtrls(QWidget *pParent=nullptr);

        void initWidget();
        void updateUnits();

        void set3dXPlaneView(gl3dXPlaneView *pView) {m_pgl3dXPlaneView=pView;}
        void set3dXSailView(gl3dXSailView *pView) {m_pgl3dXSailView=pView;}

        static double initialLength();
        static bool startAtTE()    {return s_pos==TRAILINGEDGE;}
        static bool startAtYLine() {return s_pos==Y_LINE;}
        static bool startAtZLine() {return s_pos==Z_LINE;}

        static int nStreamLines() {return s_NStreamLines;}
        static double deltaL() {return s_DeltaL;}

        static double XOffset() {return s_XOffset/100.0;}
        static double YOffset() {return s_YOffset/100.0;}
        static double ZOffset() {return s_ZOffset/100.0;}

        static void setNX(int nx) {s_NX=nx;}
        static int nX() {return s_NX;}

        static void setXFactor(double xf) {s_XFactor=xf;}
        static double XFactor() {return s_XFactor;}

        static void setL0(double ll0) {s_L0=ll0;}
        static double l0() {return s_L0;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    private:
        void setupLayout();
        void connectSignals();
        void readStreamParams();
        void enableStreamControls();

    private slots:
        void onCalcStreamlines();
        void onStreamStyle();
        void onFloatOffset();
        void onSliderOffset();

    signals:
        void update3dStreamlines();

    private:
        gl3dXPlaneView *m_pgl3dXPlaneView;
        gl3dXSailView *m_pgl3dXSailView;

        IntEdit *m_pieNXPoint;
        FloatEdit *m_pdeMaxLength, *m_pdeL0, *m_pdeXFactor;

        LineBtn *m_plbStreamLines;
        QCheckBox *m_pchUseWingColour;

        QSlider *m_pslXOffset, *m_pslZOffset;
        ExponentialSlider *m_pslYOffset;
        FloatEdit *m_pfeXOffset, *m_pfeYOffset, *m_pfeZOffset;

        IntEdit *m_pieNStreamLines;
        FloatEdit *m_pdeDeltaPos;
        QPushButton *m_ppbUpdateStreamLines;

        QLabel *m_plabLengthUnit0, *m_plabLengthUnit1, *m_plabLengthUnit2, *m_plabLengthUnit3, *m_plabLengthUnit4, *m_plabLengthUnit5;


        QRadioButton *m_prbTE, *m_prbYLine, *m_prbZLine;

        static double s_XOffset, s_YOffset, s_ZOffset;
        static eStreamStart s_pos;
        static int s_NX;
        static double s_L0;
        static double s_XFactor;
        static int s_NStreamLines;
        static double s_DeltaL;

};

