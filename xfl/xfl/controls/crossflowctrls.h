/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QSettings>

#include <xflgeom/geom3d/vector3d.h>
#include <xflcore/linestyle.h>


#define FLOWPERIOD 1

class XPlane;
class XSail;
class ExponentialSlider;
class IntEdit;
class FloatEdit;
class POpp3dCtrls;
class gl3dXPlaneView;
class gl3dXSailView;
class WPolar;
class Opp3d;
class PlaneOpp;
class LineBtn;

class CrossFlowCtrls : public QWidget
{
    Q_OBJECT

    friend class XPlane;
    friend class XSail;
    friend class gl3dXPlaneView;
    friend class gl3dXSailView;

    public:
        CrossFlowCtrls(QWidget *pMainFrame=nullptr);

        void setqDynFactor(double qDynFactor);
        void updateUnits();
        void set3dXPlaneView(gl3dXPlaneView *pView);
        void set3dXSailView(gl3dXSailView *pView);
        void makeXPlaneVelocityVectors();
        void makeXSailVelocityVectors();

        void initWidget();

        bool bGridVelocity() const {return m_bGridVelocity;}
        bool bVorticityMap() const {return m_bVorticityMap;}

        static float omegaMin() {return float(s_OmegaMin);}
        static float omegaMax() {return float(s_OmegaMax);}


        static void setXPlane(XPlane *pXPlane) {s_pXPlane=pXPlane;}
        static void setXSail(XSail *pXSail) {s_pXSail=pXSail;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:

        void onMakeCrossFlowPlane();
        void onOmegaScale();


    signals:
        void update3dScales();
        void updateWake();

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;

        void setupLayout();
        void connectSignals();


        void setWakeData();
        void enableWakeControls();
        void readWakeData();
        void makeOmegaMap();
        void makePressureRow(int irow, double x, double z, const WPolar *pWPolar, const Opp3d *pOpp3d);
        void makeVorticityRow(int irow, double x, double z, const Opp3d *pOpp3d);
        void makeWakeVelocity();



    private:

        gl3dXPlaneView *m_pgl3dXPlaneView;
        gl3dXSailView *m_pgl3dXSailView;


        //Wake
        QCheckBox *m_pchCrossFlowVel;
        QCheckBox *m_pchVorticityMap;
        QSlider *m_pslPlanePos;
        FloatEdit *m_pdeWidth;
        FloatEdit *m_pdeHeight;
        IntEdit *m_pieVelocitySamples;
        IntEdit *m_pieVorticitySamples;
        QLabel *m_pLabLen1, *m_pLabLen2;
        QCheckBox *m_pchAutoOmegaScale;
        FloatEdit *m_pdeOmegaMin, *m_pdeOmegaMax;
        QComboBox *m_pcbOmegaDir;



        QVector<Vector3d> m_GridNodesVel, m_GridVectors;

        QVector<Vector3d> m_GridNodesPressure;
        QVector<double> m_PressureField;

        QVector<Vector3d> m_GridNodesOmega;
        QVector<double> m_OmegaField;



    public:
        static XPlane *s_pXPlane;
        static XSail *s_pXSail;


        // wake
        bool m_bGridVelocity;
        bool m_bVorticityMap;

        static double s_Width;
        static double s_Height;
        static double s_XPos;
        static int s_nVelocitySamples;
        static int s_nPressureSamples;
        static int s_nVorticitySamples;

        static bool s_bAutoOmegaScale;	         /**< true if the Vorticity scale should be set automatically */
        static double s_OmegaMax, s_OmegaMin;    /** vorticity range of the vorton wake */

        static double s_OmegaCoef;   /** the scale factor for the legend vorticity */
        static int s_OmegaDir;    /** the direction of the vorticity to display */
};

