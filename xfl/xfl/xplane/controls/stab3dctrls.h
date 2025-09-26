/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QSettings>
#include <QSlider>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>

class XPlane;
class PlaneOpp;
class FloatEdit;

#define NMODES 8


class Stab3dCtrls : public QWidget
{
    Q_OBJECT

    public:
        Stab3dCtrls(QWidget *pParent=nullptr);

        void setControls();
        void setMode(PlaneOpp const *pPOpp=nullptr);

        int activeT7Mode() const {return m_iActiveMode;}

        void stopAnimate();
        void incrementTime(const PlaneOpp *pPOpp, bool bStep);
        void getPosition(PlaneOpp const*pPOpp, int iMode, double time);


        double modeTime() const {return m_ModeElapsedTime;}
        double const *modeState() const {return m_ModeState;}

        static void setXPlane(XPlane *pXPlane) {s_pXPlane = pXPlane;}
        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;

        void connectSignals();
        void setupLayout();

        void updateDof();

    private slots:
        void onAnimateMode();
        void onAnimateRestart();
        void onAnimationAmplitude(int val);
        void onModeSelection();

    public:

        double m_ModeState[6];      /**< defines the value of the 6 dof (x, y, z, rx, ry, rz) to display the position and orientation of the geometry */
        double m_ModeNormFactor;          /**< defines the amplitude of the modal animation */
        double m_ModeElapsedTime;          /**< defines the elapsed time for the modal animation */

    private:

        int m_iActiveMode;


        double m_vabs[4], m_phi[4];

        QRadioButton *m_prbNoMode;
        QRadioButton *m_prbMode[8];
        QSlider *m_pslAnimAmplitude;
        QPushButton *m_ppbAnimate, *m_ppbAnimateRestart;
        FloatEdit *m_pfeModeStep;
        QLabel *m_plabdof;

        static double s_dt;
        static double s_Amplitude;
        static XPlane *s_pXPlane;

};

