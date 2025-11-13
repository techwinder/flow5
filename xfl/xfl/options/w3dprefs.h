/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    
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
#include <QCheckBox>
#include <QSettings>
#include <QRadioButton>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

#include <xflcore/linestyle.h>


class IntEdit;
class DoubleEdit;
class ColorMenuBtn;
class LineBtn;
class OccMeshParams;

class W3dPrefs : public QWidget
{
    Q_OBJECT

    public:
        W3dPrefs(QWidget *pParent=nullptr);

        void initWidgets();

        void readData();

        void showBox(int iBox);
        void updateGradientBtn();

        static int chordwiseRes() {return s_iChordwiseRes;}
        static int sailXRes() {return s_iSailXRes;}
        static int sailZRes() {return s_iSailZRes;}
        static int bodyAxialRes() {return s_iBodyAxialRes;}
        static int bodyHoopRes() {return s_iBodyHoopRes;}
        static void setAxialTess(int iAxial) {s_iBodyAxialRes=iAxial;}
        static void setHoopTess(int iHoop) {s_iBodyHoopRes=iHoop;}

        static QColor velocityColor(){return s_VelocityStyle.m_Color;}
        static int velocityWidth() {return s_VelocityStyle.m_Width;}

        static bool autoAdjust3dScale() {return s_bAutoAdjustScale;}

        static bool isClipPlaneEnabled() {return s_bEnableClipPlane;}
        static void setClipPlaneEnabled(bool bEnabled) {s_bEnableClipPlane=bEnabled;}

        static Line::enumLineStipple highlightPattern() {return s_HighStyle.m_Stipple;}
        static int highlightWidth() {return s_HighStyle.m_Width;}
        static QColor highlightColor() {return s_HighStyle.m_Color;}

        static int selectWidth() {return s_SelectStyle.m_Width;}
        static QColor selectColor() {return s_SelectStyle.m_Color;}

        static void resetDefaults();

        static void setBackColor(QColor const &clr) {s_BackColor=clr;}

        static void setSpinAnimation(bool b) {s_bSpinAnimation=b;}
        static void setSpinDamping(double damp) {s_SpinDamping=damp;}

        static bool bSpinAnimation() {return s_bSpinAnimation;}
        static double spinDamping() {return s_SpinDamping;}

        static void saveSettings(QSettings &settings);
        static void loadSettings(QSettings &settings);

    private slots:
        void on3dAxis();
        void onBackPanelClr();
        void onContourLines();
        void onVelocity();
        void onFlapPanelClr(QColor clr);
        void onFusePanelClr(QColor clr);
        void onHighlight();
        void onIDrag();
        void onMasses(QColor clr);
        void onMoments();
        void onOther3dChanged();
        void onOutline();
        void onRestoreDefaults();
        void onSelect();
        void onShowWake();
        void onStreamLines();
        void onFlowLines();
        void onTransition();
        void onVDrag();
        void onVLMMesh();
        void onWakePanelClr(QColor clr);
        void onVortonClr(QColor clr);
        void onWaterColor(QColor clr);
        void onWind();
        void onWingPanelClr(QColor clr);
        void onXCP();
        void onUpdateUnits();

        void onColorGradient();

    private:
        void setupLayout();
        void connectSignals();


        QVector<QGroupBox *>m_pGroupBox;

        QCheckBox *m_pchBackPanelClr;

        LineBtn *m_plbHighlight, *m_plbSelect;
        LineBtn *m_plbAxis, *m_plbOutline, *m_plbMeshOutline, *m_plbTrans;
        LineBtn *m_plbLift, *m_plbMoments, *m_plbInducedDrag, *m_plbViscousDrag, *m_plbVelocity;
        LineBtn *m_plbWind;
        LineBtn *m_plbStreamLines;
        LineBtn *m_plbFlowLines;
        QCheckBox *m_pchUseWingColour;

        ColorMenuBtn *m_pcmbVortonColour;
        DoubleEdit *m_pdeVortonRadius;

        ColorMenuBtn *m_pcmbMassColor;
        ColorMenuBtn *m_pcmbFusePanelClr, *m_pcmbWingPanelClr, *m_pcmbFlapPanelClr, *m_pcmbWakePanelClr;

        QCheckBox *m_pchSpinAnimation;
        DoubleEdit *m_pdeSpinDamping;

        QCheckBox*m_pchAnimateTransitions, *m_pchAutoAdjustScale, *m_pcbEnableClipPlane, *m_pchShowRefLength;
        QCheckBox*m_pchSaveViewPoints;

        IntEdit *m_pieAnimationTime;

        DoubleEdit *m_pdeZAnimAngle;

        QRadioButton *m_prbOcc, *m_prbFlow5;
        IntEdit *m_pieChordwiseRes, *m_pieBodyAxialRes, *m_pieBodyHoopRes;
        IntEdit *m_pieSailXRes, *m_pieSailZRes;

        DoubleEdit *m_pdeArcballRadius;

        QCheckBox *m_pchGround;
        ColorMenuBtn *m_pcmbWaterColor;
        DoubleEdit *m_pdeBoxX, *m_pdeBoxY, *m_pdeBoxZ;
        QLabel *m_pLabXUnit, *m_pLabYUnit, *m_pLabZUnit;

        QPushButton *m_ppbGradientBtn;

        IntEdit *m_pieNContourLines;
        LineBtn *m_plbContourLines;

    public:

        static double s_MassRadius;
        static QColor s_MassColor;

        static LineStyle s_SelectStyle;
        static LineStyle s_HighStyle;
        static LineStyle s_AxisStyle;
        static LineStyle s_WindStyle;
        static LineStyle s_PanelStyle;
        static LineStyle s_OutlineStyle;

        // results
        static LineStyle s_LiftStyle;
        static LineStyle s_MomentStyle;
        static LineStyle s_VelocityStyle;
        static LineStyle s_StreamStyle;
        static LineStyle s_IDragStyle;
        static LineStyle s_VDragStyle;
        static LineStyle s_TransStyle;
        static LineStyle s_CpStyle;
        static LineStyle s_FlowStyle;

        static bool s_bUseWingColour;

        // panels
        static bool s_bWakePanels;
        static bool s_bUseBackClr;
        static QColor s_FusePanelColor;
        static QColor s_WingPanelColor;
        static QColor s_FlapPanelColor;
        static QColor s_WakePanelColor;

        static bool s_bAutoAdjustScale;
        static bool s_bEnableClipPlane;
        static bool s_bShowRefLength;
        static bool s_bSaveViewPoints;

        static int s_iChordwiseRes;
        static int s_iSailXRes;
        static int s_iSailZRes;
        static int s_iBodyAxialRes;
        static int s_iBodyHoopRes;

        static OccMeshParams s_OccTessParams;

        static bool s_bShowGround;
        static QColor s_WaterColor;
        static double s_BoxX, s_BoxY, s_BoxZ;


        static int s_NContourLines;
        static LineStyle s_ContourLineStyle;

        static double const &vortonRadius() {return s_VortonRadius;}
        static QColor const &vortonColour() {return s_VortonColor;}

        static QColor s_VortonColor;
        static double s_VortonRadius;

        static bool s_bSpinAnimation;
        static double s_SpinDamping;

        static QColor s_BackColor; // not a color parameter; used to set the defaukt color for the panel buttons
};




