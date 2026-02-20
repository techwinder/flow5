/****************************************************************************

    flow5 application
    Copyright (C) 2025 André Deperrois 
    
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

#define _MATH_DEFINES_DEFINED



#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QColorDialog>
#include <QPushButton>

#include <api/fl5color.h>
#include <api/sail.h>
#include <api/units.h>
#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/controls/arcball.h>
#include <interfaces/opengl/controls/colourlegend.h>
#include <interfaces/opengl/views/gl3dview.h>
#include <interfaces/widgets/color/colorbtn.h>
#include <interfaces/widgets/color/colorgraddlg.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>

bool W3dPrefs::s_bSaveViewPoints(true);
bool W3dPrefs::s_bShowRefLength(false);
bool W3dPrefs::s_bEnableClipPlane(true);
bool W3dPrefs::s_bAutoAdjustScale(false);
bool W3dPrefs::s_bWakePanels(false);

double W3dPrefs::s_MassRadius(.017);
QColor W3dPrefs::s_MassColor(95,128,99);

bool W3dPrefs::s_bSpinAnimation(true);
double W3dPrefs::s_SpinDamping(0.01);
int W3dPrefs::s_AnimationTime(500); //ms


LineStyle W3dPrefs::s_SelectStyle(  true, Line::SOLID,   5, fl5Color(255,35, 15),         Line::NOSYMBOL);
LineStyle W3dPrefs::s_HighStyle(    true, Line::SOLID,   5, fl5Color(65,105,225),         Line::NOSYMBOL);
LineStyle W3dPrefs::s_AxisStyle(    true, Line::DASHDOT, 1, fl5Color(150,150,150),        Line::NOSYMBOL);
LineStyle W3dPrefs::s_WindStyle(    true, Line::SOLID,   3, fl5Color(75, 75, 75),         Line::NOSYMBOL);
LineStyle W3dPrefs::s_PanelStyle(   true, Line::SOLID,   1, fl5Color(117, 117, 117),      Line::NOSYMBOL);
LineStyle W3dPrefs::s_OutlineStyle( true, Line::SOLID,   1, fl5Color(41,41,41),           Line::NOSYMBOL);
LineStyle W3dPrefs::s_LiftStyle(    true, Line::SOLID,   3, fl5Color(105, 105, 105),      Line::NOSYMBOL);
LineStyle W3dPrefs::s_VelocityStyle(true, Line::SOLID,   1, fl5Color(255, 100, 100),      Line::NOSYMBOL);
LineStyle W3dPrefs::s_StreamStyle(  true, Line::DASH,    1, fl5Color(105, 105, 105),      Line::NOSYMBOL);
LineStyle W3dPrefs::s_MomentStyle(  true, Line::DASH,    1, fl5Color(200, 177, 100),      Line::NOSYMBOL);
LineStyle W3dPrefs::s_IDragStyle(   true, Line::DASH,    1, fl5Color(215,100,125),        Line::NOSYMBOL);
LineStyle W3dPrefs::s_VDragStyle(   true, Line::DASH,    1, fl5Color(215,125,100),        Line::NOSYMBOL);
LineStyle W3dPrefs::s_CpStyle(      true, Line::SOLID,   1, fl5Color(255,0,0),            Line::NOSYMBOL);
LineStyle W3dPrefs::s_TransStyle(   true, Line::SOLID,   1, fl5Color(171, 103, 220),      Line::NOSYMBOL);
LineStyle W3dPrefs::s_FlowStyle(    true, Line::SOLID,   2, fl5Color(101, 101, 231, 153), Line::NOSYMBOL);

bool W3dPrefs::s_bUseWingColour(false);

bool W3dPrefs::s_bUseBackClr(false);
QColor W3dPrefs::s_WingPanelColor(231,231,231);
QColor W3dPrefs::s_FusePanelColor(241,241,241);
QColor W3dPrefs::s_FlapPanelColor(227,227,227);
QColor W3dPrefs::s_WakePanelColor(215, 215, 215, 105);

bool W3dPrefs::s_bShowGround(true);
QColor W3dPrefs::s_WaterColor(51,77,89,100);
double W3dPrefs::s_BoxX=10, W3dPrefs::s_BoxY=10;

int W3dPrefs::s_iChordwiseRes=29;

int W3dPrefs::s_iBodyAxialRes=57;
int W3dPrefs::s_iBodyHoopRes=29;

int W3dPrefs::s_NContourLines = 11;
LineStyle W3dPrefs::s_ContourLineStyle(true, Line::SOLID, 1, fl5Color(125,125,125), Line::NOSYMBOL);

QColor W3dPrefs::s_VortonColor(191,191,191);
double W3dPrefs::s_VortonRadius = 0.007;

bool W3dPrefs::s_bMultiSample = true;


W3dPrefs::W3dPrefs(QWidget *pParent) : QWidget(pParent)
{
    setWindowTitle(tr("3d Styles"));
    setupLayout();
    connectSignals();
}


void W3dPrefs::connectSignals()
{


    connect(m_pchAnimateTransitions, SIGNAL(clicked(bool)),         SLOT(onOther3dChanged()));
    connect(m_pchBackPanelClr,       SIGNAL(clicked(bool)),         SLOT(onBackPanelClr()));
    connect(m_pchSpinAnimation,      SIGNAL(clicked(bool)),         SLOT(onOther3dChanged()));

    connect(m_pcbFlapPanelClr,       SIGNAL(clicked()),             SLOT(onFlapPanelClr()));
    connect(m_pcbFusePanelClr,       SIGNAL(clicked()),             SLOT(onFusePanelClr()));
    connect(m_pcbMassColor,          SIGNAL(clicked()),             SLOT(onMasses()));
    connect(m_pcbVortonColor,        SIGNAL(clicked()),             SLOT(onVortonClr()));
    connect(m_pcbWakePanelClr,       SIGNAL(clicked()),             SLOT(onWakePanelClr()));
    connect(m_pcbWaterColor,         SIGNAL(clicked()),             SLOT(onWaterColor()));
    connect(m_pcbWingPanelClr,       SIGNAL(clicked()),             SLOT(onWingPanelClr()));

    connect(m_plbAxis,               SIGNAL(clickedLB(LineStyle)),  SLOT(on3dAxis()));
    connect(m_plbContourLines,       SIGNAL(clickedLB(LineStyle)),  SLOT(onContourLines()));
    connect(m_plbFlowLines,          SIGNAL(clickedLB(LineStyle)),  SLOT(onFlowLines()));
    connect(m_plbHighlight,          SIGNAL(clickedLB(LineStyle)),  SLOT(onHighlight()));
    connect(m_plbInducedDrag,        SIGNAL(clickedLB(LineStyle)),  SLOT(onIDrag()));
    connect(m_plbLift,               SIGNAL(clickedLB(LineStyle)),  SLOT(onXCP()));
    connect(m_plbMeshOutline,        SIGNAL(clickedLB(LineStyle)),  SLOT(onVLMMesh()));
    connect(m_plbMoments,            SIGNAL(clickedLB(LineStyle)),  SLOT(onMoments()));
    connect(m_plbOutline,            SIGNAL(clickedLB(LineStyle)),  SLOT(onOutline()));
    connect(m_plbSelect,             SIGNAL(clickedLB(LineStyle)),  SLOT(onSelect()));
    connect(m_plbStreamLines,        SIGNAL(clickedLB(LineStyle)),  SLOT(onStreamLines()));
    connect(m_plbTrans,              SIGNAL(clickedLB(LineStyle)),  SLOT(onTransition()));
    connect(m_plbVelocity,           SIGNAL(clickedLB(LineStyle)),  SLOT(onVelocity()));
    connect(m_plbViscousDrag,        SIGNAL(clickedLB(LineStyle)),  SLOT(onVDrag()));
    connect(m_plbWind,               SIGNAL(clickedLB(LineStyle)),  SLOT(onWind()));
}


void W3dPrefs::readData()
{
    s_bUseWingColour = m_pchUseWingColour->isChecked();

    s_bAutoAdjustScale = m_pchAutoAdjustScale->isChecked();
    s_bEnableClipPlane = m_pcbEnableClipPlane->isChecked();
    s_bShowRefLength   = m_pchShowRefLength->isChecked();
    s_bSaveViewPoints  = m_pchSaveViewPoints->isChecked();

//    Fuse::setOccTessellator(m_prbOcc->isChecked());
    s_iChordwiseRes = m_pieChordwiseRes->value();
    s_iBodyAxialRes = m_pieBodyAxialRes->value();
    s_iBodyHoopRes  = m_pieBodyHoopRes->value();

    Sail::setTessellation(m_pieSailXRes->value(), m_pieSailZRes->value());

    s_bShowGround = m_pchGround->isChecked();
    s_BoxX        = m_pfeBoxX->value()/Units::mtoUnit();
    s_BoxY        = m_pfeBoxY->value()/Units::mtoUnit();

    ArcBall::setSphereRadius(std::min(m_pfeArcballRadius->value()/100.0, 1.0));

    W3dPrefs::setSpinAnimation(m_pchSpinAnimation->isChecked());
    W3dPrefs::setSpinDamping(m_pfeSpinDamping->value()/100.0);
    gl3dView::setAnimationTransitions(m_pchAnimateTransitions->isChecked());
    gl3dView::setTransitionTime(m_pieAnimationTime->value());

    gl3dView::setZAnimAngle(m_pfeZAnimAngle->value());

    s_HighStyle = m_plbHighlight->theLineStyle();
    s_SelectStyle = m_plbSelect->theLineStyle();

    s_NContourLines = m_pieNContourLines->value();

    s_VortonColor = m_pcbVortonColor->color();
    s_VortonRadius = m_pfeVortonRadius->value()/100.0;
}


void W3dPrefs::initWidgets()
{
    m_plbHighlight->setTheStyle(s_HighStyle);
    m_plbSelect->setTheStyle(s_SelectStyle);

    m_plbAxis->setTheStyle(s_AxisStyle);
    m_plbWind->setTheStyle(s_WindStyle);
    m_plbOutline->setTheStyle(s_OutlineStyle);
    m_plbMeshOutline->setTheStyle(s_PanelStyle);
    m_plbLift->setTheStyle(s_LiftStyle);
    m_plbVelocity->setTheStyle(s_VelocityStyle);
    m_plbMoments->setTheStyle(s_MomentStyle);
    m_plbInducedDrag->setTheStyle(s_IDragStyle);
    m_plbViscousDrag->setTheStyle(s_VDragStyle);
    m_plbFlowLines->setTheStyle(s_FlowStyle);
    m_plbStreamLines->setTheStyle(s_StreamStyle);
    m_pchUseWingColour->setChecked(s_bUseWingColour);
    m_plbTrans->setTheStyle(s_TransStyle);

    m_pcbWaterColor->setColor(s_WaterColor);
    m_pchGround->setChecked(s_bShowGround);
    m_pfeBoxX->setValue(s_BoxX*Units::mtoUnit());
    m_pfeBoxY->setValue(s_BoxY*Units::mtoUnit());

    m_pcbMassColor->setColor(s_MassColor);

    m_pchBackPanelClr->setChecked(s_bUseBackClr);
    m_pcbFusePanelClr->setEnabled(!s_bUseBackClr);
    m_pcbWingPanelClr->setEnabled(!s_bUseBackClr);
    m_pcbFlapPanelClr->setEnabled(!s_bUseBackClr);
    m_pcbWakePanelClr->setEnabled(!s_bUseBackClr);
    m_pcbFusePanelClr->setColor(s_FusePanelColor);
    m_pcbWingPanelClr->setColor(s_WingPanelColor);
    m_pcbFlapPanelClr->setColor(s_FlapPanelColor);
    m_pcbWakePanelClr->setColor(s_WakePanelColor);

//    m_prbOcc->setChecked(Fuse::isOccTessellator());
//    m_prbFlow5->setChecked(!Fuse::isOccTessellator());
    m_pieChordwiseRes->setValue(s_iChordwiseRes);
    m_pieBodyAxialRes->setValue(s_iBodyAxialRes);
    m_pieBodyHoopRes->setValue(s_iBodyHoopRes);
    m_pieSailXRes->setValue(Sail::iXRes());
    m_pieSailZRes->setValue(Sail::iZRes());

    m_pfeArcballRadius->setValue(ArcBall::sphereRadius()*100.0);

    m_pchSpinAnimation->setChecked(s_bSpinAnimation);
    m_pfeSpinDamping->setValue(s_SpinDamping*100.0);

    m_pchAnimateTransitions->setChecked(gl3dView::bAnimateTransitions());
    m_pieAnimationTime->setValue(gl3dView::transitionTime());

    m_pfeZAnimAngle->setValue(gl3dView::zAnimAngle());

    onOther3dChanged();

    m_pchAutoAdjustScale->setChecked(s_bAutoAdjustScale);
    m_pcbEnableClipPlane->setChecked(s_bEnableClipPlane);
    m_pchShowRefLength->setChecked(s_bShowRefLength);
    m_pchSaveViewPoints->setChecked(s_bSaveViewPoints);

    updateGradientBtn();

    m_pieNContourLines->setValue(s_NContourLines);
    m_plbContourLines->setTheStyle(s_ContourLineStyle);

    m_pfeVortonRadius->setValue(s_VortonRadius*100.0);
    m_pcbVortonColor->setColor(s_VortonColor);
}


void W3dPrefs::setupLayout()
{
    m_pGroupBox.push_back(new QGroupBox(tr("Colour settings")));
    {
        QVBoxLayout *pColorPrefs = new QVBoxLayout;
        {
            QGridLayout *pColorGridLayout = new QGridLayout;
            {
                QLabel *pLabGeom        = new QLabel(tr("Geometry"));
                QLabel *pLabMesh        = new QLabel(tr("Mesh"));
                QLabel *pLabVortons     = new QLabel(tr("Vortons"));
                pLabGeom->setStyleSheet("font: bold");
                pLabMesh->setStyleSheet("font: bold");
                pLabVortons->setStyleSheet("font: bold");

                QLabel *pLabResults     = new QLabel(tr("Results"));
                QLabel *pLabSel         = new QLabel(tr("Selected"));
                QLabel *pLabHigh        = new QLabel(tr("Highlighted"));
                QLabel *pLabAxis        = new QLabel(tr("Axes"));
                QLabel *pLabWind        = new QLabel(tr("Wind"));
                QLabel *pLabOutline     = new QLabel(tr("Geometry outline"));
                QLabel *pLabTopTr       = new QLabel(tr("Transitions"));
                QLabel *pLabLift        = new QLabel(tr("Lift and forces"));
                QLabel *pLabMoments     = new QLabel(tr("Moments"));
                QLabel *pLabInducedDrag = new QLabel(tr("Induced drag"));
                QLabel *pLabViscousDrag = new QLabel(tr("Viscous drag"));
                QLabel *pLabVelocity    = new QLabel(tr("Velocity vectors"));
                QLabel *pLabStream      = new QLabel(tr("Streamlines"));
                QLabel *pLabFlow        = new QLabel(tr("Flow lines"));
                QLabel *pLabMasses      = new QLabel(tr("Masses"));
                QLabel *pLabVLM         = new QLabel(tr("Panel outline"));
                QLabel *pLabFuse        = new QLabel(tr("Fuse panels:"));
                QLabel *pLabWing        = new QLabel(tr("Wing panels:"));
                QLabel *pLabFlap        = new QLabel(tr("Flap panels:"));
                QLabel *pLabWake        = new QLabel(tr("Wake panels:"));

                m_plbHighlight    = new LineBtn(this);
                m_plbSelect       = new LineBtn(this);

                m_plbAxis         = new LineBtn(this);
                m_plbWind         = new LineBtn(this);
                m_plbOutline      = new LineBtn(this);
                m_plbTrans        = new LineBtn(this);
                m_plbLift         = new LineBtn(this);
                m_plbMoments      = new LineBtn(this);
                m_plbInducedDrag  = new LineBtn(this);
                m_plbViscousDrag  = new LineBtn(this);
                m_plbVelocity     = new LineBtn(this);
                m_plbFlowLines    = new LineBtn(this);
                m_plbStreamLines  = new LineBtn(this);

                m_pchUseWingColour = new QCheckBox(tr("Use wing colour"));

                m_pcbMassColor    = new ColorBtn;

                m_pchBackPanelClr  = new QCheckBox(tr("Use background color for mesh panels"));
                m_plbMeshOutline   = new LineBtn(this);
                m_pcbFusePanelClr = new ColorBtn;
                m_pcbWingPanelClr = new ColorBtn;
                m_pcbFlapPanelClr = new ColorBtn;
                m_pcbWakePanelClr = new ColorBtn;

                m_pfeVortonRadius  = new FloatEdit;
                m_pcbVortonColor = new ColorBtn;

                m_plbHighlight->setBackground(true);
                m_plbSelect->setBackground(true);

                m_plbAxis->setBackground(true);
                m_plbWind->setBackground(true);
                m_plbOutline->setBackground(true);
                m_plbLift->setBackground(true);
                m_plbTrans->setBackground(true);
                m_plbMoments->setBackground(true);
                m_plbInducedDrag->setBackground(true);
                m_plbViscousDrag->setBackground(true);
                m_plbVelocity->setBackground(true);
                m_plbStreamLines->setBackground(true);
                m_plbFlowLines->setBackground(true);
                m_plbMeshOutline->setBackground(true);
                m_pcbVortonColor->setBackground(true);

                pColorGridLayout->setColumnStretch(1,1);
                pColorGridLayout->setColumnStretch(2,2);
                pColorGridLayout->setColumnStretch(3,1);
                pColorGridLayout->setColumnStretch(4,2);

                pColorGridLayout->addWidget(pLabSel,             1,1, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbSelect,         1,2);
                pColorGridLayout->addWidget(pLabHigh,            1,3, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbHighlight,      1,4);

                pColorGridLayout->addWidget(pLabAxis,            2,1, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbAxis,           2,2);

                pColorGridLayout->addWidget(pLabWind,            2,3, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbWind,           2,4);

                pColorGridLayout->addWidget(pLabGeom,            4,1,1,4, Qt::AlignCenter);
                pColorGridLayout->addWidget(pLabOutline,         5,1, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(pLabMasses,          5,3, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbOutline,        5,2);
                pColorGridLayout->addWidget(m_pcbMassColor,     5,4);

                pColorGridLayout->addWidget(pLabMesh,            6,1,1,4, Qt::AlignCenter);
                pColorGridLayout->addWidget(pLabVLM,             7,1, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbMeshOutline,    7,2);

                pColorGridLayout->addWidget(m_pchBackPanelClr,    8,1,1,4, Qt::AlignCenter);
                pColorGridLayout->addWidget(pLabFuse,             9,1, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_pcbFusePanelClr,   9,2);
                pColorGridLayout->addWidget(pLabWing,             9,3, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_pcbWingPanelClr,   9,4);
                pColorGridLayout->addWidget(pLabFlap,             10,1, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_pcbFlapPanelClr,   10,2);
                pColorGridLayout->addWidget(pLabWake,             10,3, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_pcbWakePanelClr,   10,4);

                pColorGridLayout->addWidget(pLabResults,            11,1,1,4, Qt::AlignCenter);
                pColorGridLayout->addWidget(pLabLift,               12,1, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbLift,              12,2);
                pColorGridLayout->addWidget(pLabMoments,            12,3, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbMoments,           12,4);
                pColorGridLayout->addWidget(pLabInducedDrag,        13,1, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbInducedDrag,       13,2);
                pColorGridLayout->addWidget(pLabViscousDrag,        13,3, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbViscousDrag,       13,4);
                pColorGridLayout->addWidget(pLabTopTr,              14,1, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbTrans,             14,2);
                pColorGridLayout->addWidget(pLabVelocity,           14,3, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbVelocity,          14,4);

                pColorGridLayout->addWidget(pLabStream,             15,1, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbStreamLines,       15,2);
                pColorGridLayout->addWidget(m_pchUseWingColour,     15, 3, 1, 2);

                pColorGridLayout->addWidget(pLabFlow,               16,1, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbFlowLines,         16,2);

                pColorGridLayout->addWidget(pLabVortons,            17,1,1,4, Qt::AlignCenter);
                pColorGridLayout->addWidget(new QLabel("Colour:"),  18,1, Qt::AlignVCenter | Qt::AlignRight);
                pColorGridLayout->addWidget(m_pcbVortonColor,      18,2);
                pColorGridLayout->addWidget(new QLabel("Radius:"),  18,3, Qt::AlignVCenter | Qt::AlignRight);
                pColorGridLayout->addWidget(m_pfeVortonRadius,      18,4);
                pColorGridLayout->addWidget(new QLabel("% viewport width"),      18,5, Qt::AlignVCenter | Qt::AlignLeft);

            }

            QGroupBox *pGroundBox = new QGroupBox("Ground/Water");
            {
                QVBoxLayout *pBoxLayout = new QVBoxLayout;
                {
                    QHBoxLayout *pGroundBoxLayout = new QHBoxLayout;
                    {
                        QFormLayout *pGroundLayout = new QFormLayout;
                        {
                            m_pchGround = new QCheckBox("Show ground surface");
                            m_pcbWaterColor   = new ColorBtn;

                            pGroundLayout->addRow(m_pchGround);
                            pGroundLayout->addRow("Ground/Water", m_pcbWaterColor);
                        }
                        QGridLayout *pBoxSizeLayout = new QGridLayout;
                        {
                            QLabel *pLabX = new QLabel("x-length");
                            QLabel *pLabY = new QLabel("y-width");

                            m_pLabXUnit = new QLabel(Units::lengthUnitQLabel());
                            m_pLabYUnit = new QLabel(Units::lengthUnitQLabel());
                            m_pfeBoxX = new FloatEdit;
                            m_pfeBoxY = new FloatEdit;

                            pBoxSizeLayout->addWidget(pLabX,       1, 1, Qt::AlignVCenter|Qt::AlignRight);
                            pBoxSizeLayout->addWidget(m_pfeBoxX,   1, 2);
                            pBoxSizeLayout->addWidget(m_pLabXUnit, 1, 3, Qt::AlignVCenter|Qt::AlignLeft);
                            pBoxSizeLayout->addWidget(pLabY,       2, 1, Qt::AlignVCenter|Qt::AlignRight);
                            pBoxSizeLayout->addWidget(m_pfeBoxY,   2, 2);
                            pBoxSizeLayout->addWidget(m_pLabYUnit, 2, 3, Qt::AlignVCenter|Qt::AlignLeft);
                            pBoxSizeLayout->setColumnStretch(3,1);
                        }

                        pGroundBoxLayout->addLayout(pGroundLayout);
                        pGroundBoxLayout->addStretch(1);
                        pGroundBoxLayout->addLayout(pBoxSizeLayout);
                        pGroundBoxLayout->addStretch(2);
                    }

                    pBoxLayout->addLayout(pGroundBoxLayout);
                    QLabel *pInfoLab = new QLabel("Note: This is only a visual help feature. "
                                                  "The actual ground surface used in the analysis extends to infinity.");
                    pBoxLayout->addWidget(pInfoLab);
                }
                pGroundBox->setLayout(pBoxLayout);
            }

            QGroupBox *pColourMapBox = new QGroupBox("Colour map");
            {
                QVBoxLayout *pContoursLayout = new QVBoxLayout;
                {
                    QHBoxLayout *pColourGradLayout = new QHBoxLayout;
                    {
                        QLabel *pLabGrad = new QLabel("Gradient colours:");
                        m_ppbGradientBtn = new QPushButton;
                        m_ppbGradientBtn->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
                        connect(m_ppbGradientBtn, SIGNAL(clicked()), SLOT(onColorGradient()));
                        pColourGradLayout->addWidget(pLabGrad);
                        pColourGradLayout->addWidget(m_ppbGradientBtn);
                        pContoursLayout->setStretchFactor(pLabGrad, 1);
                        pContoursLayout->setStretchFactor(m_ppbGradientBtn, 7);
                    }
                    QHBoxLayout *pClrMapContoursLayout = new QHBoxLayout;
                    {
                        m_pieNContourLines = new IntEdit(s_NContourLines);
                        m_plbContourLines = new LineBtn;

                        pClrMapContoursLayout->addWidget(new QLabel("Nbr. of isobars (trilinear only):"));
                        pClrMapContoursLayout->addWidget(m_pieNContourLines);
                        pClrMapContoursLayout->addWidget(m_plbContourLines);
                        pClrMapContoursLayout->addStretch();
                    }
                    pContoursLayout->addLayout(pColourGradLayout);
                    pContoursLayout->addLayout(pClrMapContoursLayout);
                }
                pColourMapBox->setLayout(pContoursLayout);
            }

            pColorPrefs->addLayout(pColorGridLayout);
            pColorPrefs->addWidget(pColourMapBox);
            pColorPrefs->addWidget(pGroundBox);
        }
        m_pGroupBox.back()->setLayout(pColorPrefs);
    }

    m_pGroupBox.push_back(new QGroupBox("Tessellation"));
    {
        QVBoxLayout *pTessLayout = new QVBoxLayout;
        {
            QLabel *pTessLabel = new QLabel("<p>Increase the number of points to improve the tessellation of the surfaces.<br>"
                                            "This may increase the loading times and also slow down the display on low-end graphic cards.<br>"
                                            "Reload required to take effect.</p>");

            m_pieChordwiseRes = new IntEdit(37, this);
            m_pieBodyAxialRes = new IntEdit(29, this);
            m_pieBodyHoopRes  = new IntEdit(17, this);
            m_pieSailXRes     = new IntEdit(37, this);
            m_pieSailZRes     = new IntEdit(31, this);

            QGroupBox *pRuledTessellationBox = new QGroupBox("Ruled surfaces");
            {
                QFormLayout *pFormLayout = new QFormLayout;
                {
                    pFormLayout->addRow("Wing chordwise direction:", m_pieChordwiseRes);
                    pFormLayout->addRow("Body axial direction:",     m_pieBodyAxialRes);
                    pFormLayout->addRow("Body hoop direction:",      m_pieBodyHoopRes);
                    pFormLayout->addRow("Sail x-direction:",         m_pieSailXRes);
                    pFormLayout->addRow("Sail z-direction:",         m_pieSailZRes);
                }
                pRuledTessellationBox->setLayout(pFormLayout);
            }
            pTessLayout->addWidget(pTessLabel);
            pTessLayout->addWidget(pRuledTessellationBox);
        }
        m_pGroupBox.back()->setLayout(pTessLayout);
    }

    m_pGroupBox.push_back(new QGroupBox("Other"));
    {
        QVBoxLayout *pOtherLayout = new QVBoxLayout;
        {
            QGridLayout *pGridLayout = new QGridLayout;
            {   
                m_pchSpinAnimation = new QCheckBox("Enable mouse animations");
                m_pfeSpinDamping = new FloatEdit;
                m_pfeSpinDamping->setToolTip("Defines the damping of the animation at each frame update.<br>"
                                             "Set to 0 for perpetual movement.");
                QLabel *plabpcDamping = new QLabel("% damping");
                pGridLayout->addWidget(m_pchSpinAnimation, 1, 1);
                pGridLayout->addWidget(m_pfeSpinDamping,   1, 2);
                pGridLayout->addWidget(plabpcDamping,      1, 3);

                m_pchAnimateTransitions = new QCheckBox("Animate view transitions");
                m_pieAnimationTime = new IntEdit;
                QString tip = "<p>Defines the duration of animations in ms</p>";
                m_pieAnimationTime->setToolTip(tip);
                QLabel *pLabms =new QLabel("ms");
                pGridLayout->addWidget(m_pchAnimateTransitions, 2, 1);
                pGridLayout->addWidget(m_pieAnimationTime,      2, 2);
                pGridLayout->addWidget(pLabms,                  2, 3);

                QLabel *labArcBall = new QLabel("Arcball radius:");
                QLabel *labPercent = new QLabel("% view width");
                m_pfeArcballRadius = new FloatEdit(100.0,0);
                m_pfeArcballRadius->setToolTip("<p>The radius of the arcball as a percentage of the view's width</p>");

                pGridLayout->addWidget(labArcBall,         3, 1, Qt::AlignVCenter |Qt::AlignRight);
                pGridLayout->addWidget(m_pfeArcballRadius, 3, 2);
                pGridLayout->addWidget(labPercent,         3, 3);

                QLabel *pLabZAngle    = new QLabel("Auto z-rotation incremental angle:");
                m_pfeZAnimAngle = new FloatEdit(1);
                QLabel *plabDeg = new QLabel("<p>&deg;</p>");
                pGridLayout->addWidget(pLabZAngle,      4, 1, Qt::AlignVCenter |Qt::AlignRight);
                pGridLayout->addWidget(m_pfeZAnimAngle, 4, 2);
                pGridLayout->addWidget(plabDeg,         4, 3);
                pGridLayout->setColumnStretch(          5, 1);
            }
            m_pchAutoAdjustScale = new QCheckBox("Auto adjust 3d scale");
            m_pchAutoAdjustScale->setToolTip("<p>Automatically adjust the 3D scale to fit the plane in the display when switching between planes</p>");
            m_pcbEnableClipPlane = new QCheckBox("Enable clip plane");
            m_pchShowRefLength   = new QCheckBox("Display reference length");
            m_pchSaveViewPoints  = new QCheckBox("Save viewpoints when exiting 3d views");
            pOtherLayout->addLayout(pGridLayout);

            pOtherLayout->addWidget(m_pchAutoAdjustScale);
            pOtherLayout->addWidget(m_pcbEnableClipPlane);
            pOtherLayout->addWidget(m_pchShowRefLength);
            pOtherLayout->addWidget(m_pchSaveViewPoints);
        }
        m_pGroupBox.back()->setLayout(pOtherLayout);
    }

    //__________________Control buttons___________________

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        for(int ig=0; ig<m_pGroupBox.size(); ig++)
            pMainLayout->addWidget(m_pGroupBox[ig]);
        pMainLayout->addStretch(1);
    }
    setLayout(pMainLayout);
}


void W3dPrefs::onOutline()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_OutlineStyle);
    lm.exec(QCursor::pos());

    s_OutlineStyle = lm.theStyle();
    m_plbOutline->setTheStyle(s_OutlineStyle);
}


void W3dPrefs::on3dAxis()
{
    LineMenu lm(nullptr, false);
    //    lm.enableSubMenus(true, true, true, false);
    lm.initMenu(s_AxisStyle);
    lm.exec(QCursor::pos());

    s_AxisStyle = lm.theStyle();
    m_plbAxis->setTheStyle(s_AxisStyle);
}


void W3dPrefs::onHighlight()
{
    LineMenu lm(nullptr, false);
    //    lm.enableSubMenus(true, true, true, false);
    lm.initMenu(s_HighStyle);
    lm.exec(QCursor::pos());

    s_HighStyle = lm.theStyle();
    m_plbHighlight->setTheStyle(s_HighStyle);
}


void W3dPrefs::onSelect()
{
    LineMenu lm(nullptr, false);
    //    lm.enableSubMenus(true, true, true, false);
    lm.initMenu(s_SelectStyle);
    lm.exec(QCursor::pos());

    s_SelectStyle = lm.theStyle();
    m_plbSelect->setTheStyle(s_SelectStyle);
}


void W3dPrefs::onWind()
{
    LineMenu lm(nullptr, false);
    //    lm.enableSubMenus(true, true, true, false);
    lm.initMenu(s_WindStyle);
    lm.exec(QCursor::pos());

    s_WindStyle = lm.theStyle();
    m_plbWind->setTheStyle(s_WindStyle);
}


void W3dPrefs::onTransition()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_TransStyle);
    lm.exec(QCursor::pos());
    s_TransStyle = lm.theStyle();
    m_plbTrans->setTheStyle(s_TransStyle);
}


void W3dPrefs::onIDrag()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_IDragStyle);
    lm.exec(QCursor::pos());

    s_IDragStyle = lm.theStyle();
    m_plbInducedDrag->setTheStyle(s_IDragStyle);
}


void W3dPrefs::onVDrag()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_VDragStyle);
    lm.exec(QCursor::pos());

    s_VDragStyle = lm.theStyle();
    m_plbViscousDrag->setTheStyle(s_VDragStyle);
}


void W3dPrefs::onXCP()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_LiftStyle);
    lm.exec(QCursor::pos());
    s_LiftStyle = lm.theStyle();
    m_plbLift->setTheStyle(s_LiftStyle);
}


void W3dPrefs::onMoments()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_MomentStyle);
    lm.exec(QCursor::pos());
    s_MomentStyle = lm.theStyle();
    m_plbMoments->setTheStyle(s_MomentStyle);
}


void W3dPrefs::onVelocity()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_VelocityStyle);
    lm.exec(QCursor::pos());

    s_VelocityStyle = lm.theStyle();
    m_plbVelocity->setTheStyle(s_VelocityStyle);
}


void W3dPrefs::onStreamLines()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_StreamStyle);
    lm.exec(QCursor::pos());

    s_StreamStyle = lm.theStyle();
    m_plbStreamLines->setTheStyle(s_StreamStyle);
}


void W3dPrefs::onFlowLines()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_FlowStyle);
    lm.exec(QCursor::pos());

    s_FlowStyle = lm.theStyle();
    m_plbFlowLines->setTheStyle(s_FlowStyle);
}


void W3dPrefs::onContourLines()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_ContourLineStyle);
    lm.exec(QCursor::pos());

    s_ContourLineStyle = lm.theStyle();
    m_plbContourLines->setTheStyle(s_ContourLineStyle);
}


void W3dPrefs::onVLMMesh()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_PanelStyle);
    lm.exec(QCursor::pos());

    s_PanelStyle = lm.theStyle();
    m_plbMeshOutline->setTheStyle(s_PanelStyle);
    update();
}


void W3dPrefs::onMasses()
{
    QColor clr = QColorDialog::getColor(s_MassColor, this, "Mass colour", QColorDialog::ShowAlphaChannel);
    if(clr.isValid())
    {
        s_MassColor = clr;
        m_pcbMassColor->setColor(clr);
    }
    update();
}


void W3dPrefs::onWaterColor()
{
    QColor clr = QColorDialog::getColor(s_WaterColor, this, "Water colour", QColorDialog::ShowAlphaChannel);
    if(clr.isValid())
    {
        s_WaterColor = clr;
        m_pcbWaterColor->setColor(clr);
    }
    update();
}


void W3dPrefs::onBackPanelClr()
{
    s_bUseBackClr = m_pchBackPanelClr->isChecked();
    m_pcbFusePanelClr->setEnabled(!s_bUseBackClr);
    m_pcbWingPanelClr->setEnabled(!s_bUseBackClr);
    m_pcbFlapPanelClr->setEnabled(!s_bUseBackClr);
    m_pcbWakePanelClr->setEnabled(!s_bUseBackClr);
    if(s_bUseBackClr)
    {
        m_pcbFusePanelClr->setColor(DisplayOptions::backgroundColor());
        m_pcbWingPanelClr->setColor(DisplayOptions::backgroundColor());
        m_pcbFlapPanelClr->setColor(DisplayOptions::backgroundColor());
        m_pcbWakePanelClr->setColor(DisplayOptions::backgroundColor());
        s_WingPanelColor = DisplayOptions::backgroundColor();
        s_FusePanelColor = DisplayOptions::backgroundColor();
        s_FlapPanelColor = DisplayOptions::backgroundColor();
        s_WakePanelColor = DisplayOptions::backgroundColor();
    }
}


void W3dPrefs::onFusePanelClr()
{
    QColor clr = QColorDialog::getColor(s_FusePanelColor, this, "Fuse colour", QColorDialog::ShowAlphaChannel);
    if(clr.isValid())
    {
        s_FusePanelColor = clr;
        m_pcbFusePanelClr->setColor(clr);
    }
    update();
}


void W3dPrefs::onWingPanelClr()
{
    QColor clr = QColorDialog::getColor(s_WingPanelColor, this, "Wing colour", QColorDialog::ShowAlphaChannel);
    if(clr.isValid())
    {
        s_WingPanelColor = clr;
        m_pcbWingPanelClr->setColor(clr);
    }
    update();
}


void W3dPrefs::onFlapPanelClr()
{
    QColor clr = QColorDialog::getColor(s_FlapPanelColor, this, "Flap colour", QColorDialog::ShowAlphaChannel);
    if(clr.isValid())
    {
        s_FlapPanelColor = clr;
        m_pcbFlapPanelClr->setColor(clr);
    }
    update();
}


void W3dPrefs::onWakePanelClr()
{   
    QColor clr = QColorDialog::getColor(s_WakePanelColor, this, "Wake colour", QColorDialog::ShowAlphaChannel);
    if(clr.isValid())
    {
        s_WakePanelColor = clr;
        m_pcbWakePanelClr->setColor(clr);
    }
    update();
}


void W3dPrefs::onVortonClr()
{
    QColor clr = QColorDialog::getColor(s_VortonColor, this, "Vorton colour", QColorDialog::ShowAlphaChannel);
    if(clr.isValid())
    {
        s_VortonColor = clr;
        m_pcbVortonColor->setColor(clr);
    }
    update();
}


void W3dPrefs::onShowWake()
{
}


void W3dPrefs::onOther3dChanged()
{
    m_pfeSpinDamping->setEnabled(m_pchSpinAnimation->isChecked());
    m_pieAnimationTime->setEnabled(m_pchAnimateTransitions->isChecked());
}


void W3dPrefs::saveSettings(QSettings &settings)
{
    settings.beginGroup("3DPrefs");
    {
        xfl::saveLineSettings(settings, s_AxisStyle, "AxisStyle");
        xfl::saveLineSettings(settings, s_HighStyle,      "HighlightStyle");
        xfl::saveLineSettings(settings, s_SelectStyle,    "SelectionStyle");
        xfl::saveLineSettings(settings, s_WindStyle,      "WindStyle");
        xfl::saveLineSettings(settings, s_PanelStyle,     "PanelStyle");
        xfl::saveLineSettings(settings, s_OutlineStyle,   "OutlineStyle");
        xfl::saveLineSettings(settings, s_LiftStyle ,     "LiftForceStyle");
        xfl::saveLineSettings(settings, s_MomentStyle,    "MomentStyle");
        xfl::saveLineSettings(settings, s_VDragStyle,     "VDragStyle");
        xfl::saveLineSettings(settings, s_IDragStyle,     "IDragStyle");
        xfl::saveLineSettings(settings, s_VelocityStyle,  "VelocityStyle");
        xfl::saveLineSettings(settings, s_CpStyle,        "CpStyle");
        xfl::saveLineSettings(settings, s_TransStyle,     "TopStyle");
        xfl::saveLineSettings(settings, s_StreamStyle,    "StreamStyle");
        xfl::saveLineSettings(settings, s_FlowStyle,      "FlowLineStyle");

        settings.setValue("OccTessellator", Part::isOccTessellator());

        settings.setValue("NContourLines", s_NContourLines);
        xfl::saveLineSettings(settings, s_ContourLineStyle, "ContourLineStyle");

        settings.setValue("VortonColour", s_VortonColor);
        settings.setValue("VortonRadius", s_VortonRadius);

        settings.setValue("UseWingColour", s_bUseWingColour);

        settings.setValue("showWakePanels", s_bWakePanels);

        settings.setValue("WaterColor",  s_WaterColor);
        settings.setValue("bShowGround", s_bShowGround);
        settings.setValue("GroundBoxX",  s_BoxX);
        settings.setValue("GroundBoxY",  s_BoxY);


        settings.setValue("MassColor", s_MassColor);

        settings.setValue("UseBackColor", s_bUseBackClr);
        settings.setValue("FusePanelClr", s_FusePanelColor);
        settings.setValue("WingPanelClr", s_WingPanelColor);
        settings.setValue("FlapPanelClr", s_FlapPanelColor);
        settings.setValue("WakePanelClr", s_WakePanelColor);

        settings.setValue("EnableClipPlane", s_bEnableClipPlane);
        settings.setValue("ShowRefLength", s_bShowRefLength);
        settings.setValue("AutoAdjust3dScale", s_bAutoAdjustScale);
        settings.setValue("Save3dViewPoints", s_bSaveViewPoints);

        settings.setValue("ChordwiseRes", s_iChordwiseRes);
        settings.setValue("BodyAxialRes", s_iBodyAxialRes);
        settings.setValue("BodyHoopRes",  s_iBodyHoopRes);
        settings.setValue("SailXRes",     Sail::iXRes());
        settings.setValue("SailZRes",     Sail::iZRes());

        settings.setValue("ArcBallRadius", ArcBall::sphereRadius());

        settings.setValue("SpinAnimation",      s_bSpinAnimation);
        settings.setValue("SpinDamping",        s_SpinDamping);

        settings.setValue("AnimateTransitions", gl3dView::bAnimateTransitions());
        settings.setValue("TransitionTime",     gl3dView::transitionTime());
        settings.setValue("ZAnimAngle",         gl3dView::zAnimAngle());

        settings.setValue("EnableMultiSamp", s_bMultiSample);
    }
    settings.endGroup();
}


void W3dPrefs::loadSettings(QSettings &settings)
{
    resetDefaults();
    settings.beginGroup("3DPrefs");
    {
        xfl::loadLineSettings(settings, s_AxisStyle,     "AxisStyle");
        xfl::loadLineSettings(settings, s_HighStyle,     "HighlightStyle");
        xfl::loadLineSettings(settings, s_SelectStyle,   "SelectionStyle");
        xfl::loadLineSettings(settings, s_WindStyle,     "WindStyle");
        xfl::loadLineSettings(settings, s_PanelStyle,    "PanelStyle");
        xfl::loadLineSettings(settings, s_OutlineStyle,  "OutlineStyle");
        xfl::loadLineSettings(settings, s_LiftStyle,     "LiftForceStyle");
        xfl::loadLineSettings(settings, s_MomentStyle,   "MomentStyle");
        xfl::loadLineSettings(settings, s_VDragStyle,    "VDragStyle");
        xfl::loadLineSettings(settings, s_IDragStyle,    "IDragStyle");
        xfl::loadLineSettings(settings, s_VelocityStyle, "VelocityStyle");
        xfl::loadLineSettings(settings, s_CpStyle,       "CpStyle");
        xfl::loadLineSettings(settings, s_TransStyle,    "TopStyle");
        xfl::loadLineSettings(settings, s_StreamStyle,   "StreamStyle");
        xfl::loadLineSettings(settings, s_FlowStyle,     "FlowLineStyle");

        Part::setOccTessellator(settings.value("OccTessellator", Part::isOccTessellator()).toBool());

        s_NContourLines = settings.value("NContourLines", s_NContourLines).toInt();
        xfl::loadLineSettings(settings, s_ContourLineStyle, "ContourLineStyle");

        s_VortonColor = settings.value("VortonColour", s_VortonColor).value<QColor>();
        s_VortonRadius = settings.value("VortonRadius", s_VortonRadius).toDouble();

        s_bUseWingColour = settings.value("UseWingColour", false).toBool();

        s_WaterColor  = settings.value("WaterColor", s_WaterColor).value<QColor>();
        s_bShowGround = settings.value("bShowGround", s_bShowGround).toBool();
        s_BoxX        = settings.value("GroundBoxX",  s_BoxX).toDouble();
        s_BoxY        = settings.value("GroundBoxY",  s_BoxY).toDouble();

        s_MassColor      = settings.value("MassColor",  s_MassColor).value<QColor>();

        s_bWakePanels    = settings.value("showWakePanels", true).toBool();

        s_bUseBackClr    = settings.value("UseBackColor", true).toBool();
        s_FusePanelColor = settings.value("FusePanelClr", s_FusePanelColor).value<QColor>();
        s_WingPanelColor = settings.value("WingPanelClr", s_WingPanelColor).value<QColor>();
        s_FlapPanelColor = settings.value("FlapPanelClr", s_FlapPanelColor).value<QColor>();
        s_WakePanelColor = settings.value("WakePanelClr", s_WakePanelColor).value<QColor>();

        s_bEnableClipPlane = settings.value("EnableClipPlane",   s_bEnableClipPlane).toBool();
        s_bShowRefLength   = settings.value("ShowRefLength",     s_bShowRefLength).toBool();
        s_bAutoAdjustScale = settings.value("AutoAdjust3dScale", s_bAutoAdjustScale).toBool();
        s_bSaveViewPoints  = settings.value("Save3dViewPoints",  s_bSaveViewPoints).toBool();


        s_iChordwiseRes = settings.value("ChordwiseRes", s_iChordwiseRes).toInt();
        s_iBodyAxialRes = settings.value("BodyAxialRes", s_iBodyAxialRes).toInt();
        s_iBodyHoopRes  = settings.value("BodyHoopRes",  s_iBodyHoopRes).toInt();
        int iSailXRes     = settings.value("SailXRes",   Sail::iXRes()).toInt();
        int iSailZRes     = settings.value("SailZRes",   Sail::iZRes()).toInt();
        Sail::setTessellation(iSailXRes, iSailZRes);

        ArcBall::setSphereRadius(settings.value("ArcBallRadius", 0.8).toDouble());

        W3dPrefs::setSpinAnimation(settings.value("SpinAnimation", s_bSpinAnimation).toBool());
        W3dPrefs::setSpinDamping(settings.value("SpinDamping", s_SpinDamping).toDouble());

        gl3dView::setAnimationTransitions(settings.value("AnimateTransitions", gl3dView::bAnimateTransitions()).toBool());
        gl3dView::setTransitionTime(      settings.value("TransitionTime",     gl3dView::transitionTime()).toInt());
        gl3dView::setZAnimAngle(          settings.value("ZAnimAngle",         gl3dView::zAnimAngle()).toDouble());

        s_bMultiSample = settings.value("EnableMultiSamp", s_bMultiSample).toBool();
    }
    settings.endGroup();
}


void W3dPrefs::onRestoreDefaults()
{
    resetDefaults();
    initWidgets();
}


void W3dPrefs::resetDefaults()
{
    s_bWakePanels = false;

    s_SelectStyle   = LineStyle(true, Line::SOLID, 5, fl5Color(255,75,75),  Line::NOSYMBOL);
    s_HighStyle     = LineStyle(true, Line::SOLID, 5, fl5Color(75,75,255),  Line::NOSYMBOL);

    s_WaterColor = QColor(51,77,89,100);
    s_MassColor  = QColor(95,128,99);

    s_PanelStyle.m_Stipple    = Line::SOLID;
    s_PanelStyle.m_Width      = 1;
    s_PanelStyle.m_Color      = fl5Color(87,87,87);
    s_AxisStyle.m_Stipple     = Line::DASHDOT;
    s_AxisStyle.m_Width       = 1;
    s_AxisStyle.m_Color       = fl5Color(150,150,150);
    s_WindStyle.m_Stipple     = Line::DASHDOT;
    s_WindStyle.m_Width       = 3;
    s_WindStyle.m_Color       = fl5Color(75,75,75);
    s_OutlineStyle.m_Stipple  = Line::SOLID;
    s_OutlineStyle.m_Width    = 1;
    s_OutlineStyle.m_Color    = fl5Color(41, 41, 41);
    s_LiftStyle.m_Stipple     = Line::SOLID;
    s_LiftStyle.m_Width       = 3;
    s_LiftStyle.m_Color       = fl5Color(105, 105, 105);
    s_MomentStyle.m_Stipple   = Line::SOLID;
    s_MomentStyle.m_Width     = 2;
    s_MomentStyle.m_Color     = fl5Color(200, 100, 100);

    s_IDragStyle.m_Stipple    = Line::DASH;
    s_IDragStyle.m_Width      = 2;
    s_IDragStyle.m_Color      = fl5Color(215,100,125);
    s_VDragStyle.m_Stipple    = Line::DASH;
    s_VDragStyle.m_Width      = 2;
    s_VDragStyle.m_Color      = fl5Color(215,125,100);

    s_VelocityStyle.m_Stipple = Line::SOLID;
    s_VelocityStyle.m_Width   = 2;
    s_VelocityStyle.m_Color   = fl5Color(255, 100, 100);

    s_CpStyle.m_Stipple = Line::SOLID;
    s_CpStyle.m_Width   = 1;
    s_CpStyle.m_Color   = fl5Color(255,0,0);

    s_FlowStyle.m_Stipple = Line::SOLID;
    s_FlowStyle.m_Width   = 2;
    s_FlowStyle.m_Color   = fl5Color(101, 101, 231, 153);

    s_StreamStyle.m_Stipple  = Line::DASH;
    s_StreamStyle.m_Width    = 1;
    s_StreamStyle.m_Color    = fl5Color(105, 105, 105);

    s_TransStyle.m_Stipple = Line::SOLID;
    s_TransStyle.m_Width   = 2;
    s_TransStyle.m_Color   = fl5Color(171, 103, 220);

    s_bUseBackClr = false;
    s_WingPanelColor = QColor(231,231,231);
    s_FusePanelColor = QColor(241,241,241);
    s_FlapPanelColor = QColor(227,227,227);
    s_WakePanelColor = QColor(215,215,215, 105);

    s_BoxX = s_BoxY = 10.0;
    s_NContourLines = 11;

    s_VortonColor = QColor(191,191,191);
    s_VortonRadius = 0.007;

    s_iChordwiseRes= 27;
    s_iBodyAxialRes=57;
    s_iBodyHoopRes=29;


    ArcBall::setSphereRadius(0.8);

    gl3dView::setAnimationTransitions(true);
    gl3dView::setTransitionTime(500);

    gl3dView::setZAnimAngle(0.25);
}


void W3dPrefs::showBox(int iBox)
{
    if(iBox<0)
    {
        for(int i=0; i<m_pGroupBox.size(); i++)
            m_pGroupBox[i]->setVisible(true);
    }
    else
    {
        for(int i=0; i<m_pGroupBox.size(); i++)
            m_pGroupBox[i]->setVisible(i==iBox);
    }
}


void W3dPrefs::onUpdateUnits()
{
    m_pfeBoxX->setValue(s_BoxX*Units::mtoUnit());
    m_pfeBoxY->setValue(s_BoxY*Units::mtoUnit());
    m_pLabXUnit->setText(Units::lengthUnitQLabel());
    m_pLabYUnit->setText(Units::lengthUnitQLabel());
}


void W3dPrefs::updateGradientBtn()
{
    QString style("background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,");
    QString strange;
    for(int i=0; i<11; i++)
    {
        double fi = double(i)/double(10);
        strange = QString::asprintf(" stop:%11.3f", fi) + ColourLegend::colour(fi).name();
        if(fi<10) strange += ",";
        style += strange;
    }
    style += ")";

    m_ppbGradientBtn->setStyleSheet(style);
    m_ppbGradientBtn->update();
}


void W3dPrefs::onColorGradient()
{
    ColorGradDlg dlg(ColourLegend::colours());
    if(dlg.exec()==QDialog::Accepted)
    {
        ColourLegend::s_Clr = dlg.colours();
        updateGradientBtn();
    }
}

QColor W3dPrefs::velocityColor()  {return xfl::fromfl5Clr(s_VelocityStyle.m_Color);}
QColor W3dPrefs::highlightColor() {return xfl::fromfl5Clr(s_HighStyle.m_Color);}
QColor W3dPrefs::selectColor()    {return xfl::fromfl5Clr(s_SelectStyle.m_Color);}










