/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
    This file is part of flow5.

    flow5 is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    flow5 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with flow5.
    If not, see <https://www.gnu.org/licenses/>.


*****************************************************************************/

#pragma once

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QSettings>

class XPlane;
class XSail;
class ExponentialSlider;
class FloatEdit;
class gl3dXPlaneView;
class gl3dXSailView;

class Opp3dScalesCtrls : public QWidget
{
    friend class gl3dXPlaneView;
    friend class gl3dXSailView;

    Q_OBJECT
    public:
        Opp3dScalesCtrls(QWidget *parent = nullptr);


        void set3dXPlaneView(gl3dXPlaneView *pView) { m_pgl3dXPlaneView = pView;}
        void set3dXSailView(gl3dXSailView *pView);
        void updateUnits();
        void updateGammaRange(float GMin, float GMax);
        void updateCpRange(float CpMin, float CpMax);
        void updatePressureRange(float PressureMin, float PressureMax);

        void initWidget();

        static void setXPlane(XPlane *pXPlane) {s_pXPlane=pXPlane;}
        static void setXSail(XSail *pXSail) {s_pXSail=pXSail;}


        static float gammaMin() {return float(s_GammaMin);}
        static float gammaMax() {return float(s_GammaMax);}
        static bool isAutoCpScale() {return s_bAutoCpScale;}
        static bool isAutoGammaScale() {return s_bAutoGammaScale;}
        static bool isAutoPressureScale() {return s_bAutoPressureScale;}
        static void setCpRange(double min, double max) {s_CpMin=min; s_CpMax=max;}
        static float CpMin() {return float(s_CpMin);}
        static float CpMax() {return float(s_CpMax);}
        static float pressureMin() {return float(s_PressureMin);}
        static float pressureMax() {return float(s_PressureMax);}

        static void setPanelForceScale(double scale) {s_PanelForceScale=scale;}
        static void setLiftScale(double scale)       {s_LiftScale=scale;}
        static void setPartForceScale(double scale)  {s_PartForceScale=scale;}
        static void setMomentScale(double scale)     {s_MomentScale=scale;}
        static void setDragScale(double scale)       {s_DragScale=scale;}
        static float panelForceScale() {return float(s_PanelForceScale);}
        static float liftScale()       {return float(s_LiftScale);}
        static float partForceScale()  {return float(s_PartForceScale);}
        static float momentScale()     {return float(s_MomentScale);}
        static float dragScale()       {return float(s_DragScale);}


        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void connectSignals();

    private slots:
        void onCpScale();
        void onGammaScale();
        void onPressureScale();
        void onPanelForceScale();
        void onPartForceScale();
        void onVelocityScale();
        void onMomentScale();
        void onDragScale();
        void onLiftScale();

    signals:
        void update3dScales();

    private:
        gl3dXPlaneView *m_pgl3dXPlaneView;
        gl3dXSailView *m_pgl3dXSailView;

        static XPlane *s_pXPlane;
        static XSail *s_pXSail;

        static bool s_bAutoGammaScale;		      /**< true if the Gamma (doublet density) scale should be set automatically */
        static double s_GammaMin;                 /**< minimum value of the gamma scale in 3D view */
        static double s_GammaMax;                 /**< maximum value of the gamma scale in 3D view */

        static bool s_bAutoCpScale;		          /**< true if the Cp scale should be set automatically */
        static double s_CpMin;                    /**< minimum value of the Cp scale in 3D view */
        static double s_CpMax;                    /**< maximum value of the Cp scale in 3D view */

        static bool s_bAutoPressureScale;         /**< true if the Pressure scale should be set automatically */
        static double s_PressureMin;              /**< minimum value of the Cp scale in 3D view */
        static double s_PressureMax;              /**< maximum value of the Cp scale in 3D view */

        static double s_PanelForceScale;          /**< scaling factor for the panel forces in 3D view >*/
        static double s_LiftScale;                /**< scaling factor for the lift display in 3D view >*/
        static double s_PartForceScale;           /**< scaling factor for the part force display in 3D view >*/
        static double s_MomentScale;              /**< scaling factor for the moment display in 3D view >*/
        static double s_DragScale;                /**< scaling factor for the drag display in 3D view >*/

        ExponentialSlider *m_pesLiftScale, *m_pesMoment;
        ExponentialSlider *m_pesPartForce;
        ExponentialSlider *m_pesDrag, *m_pesVelocity;
        ExponentialSlider *m_pesPanelForce;
        QCheckBox *m_pchAutoGammaScale, *m_pchAutoCpScale, *m_pchAutoPressureScale;
        FloatEdit *m_pfeGammaMin, *m_pfeGammaMax;
        FloatEdit *m_pfeCpMin, *m_pfeCpMax;
        FloatEdit *m_pfePressureMin, *m_pfePressureMax;


        QLabel *m_plabPressureUnit1, *m_plabPressureUnit2;

};

