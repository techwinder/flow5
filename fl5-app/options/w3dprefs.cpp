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



#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QColorDialog>
#include <QPushButton>

#include <xfl3d/globals/w3dprefs.h>
#include <xfl3d/controls/arcball.h>
#include <xfl3d/controls/colourlegend.h>
#include <xfl3d/views/gl3dview.h>
#include <xflcore/units.h>
#include <xflobjects/objects3d/fuse/fuse.h>
#include <xflpanels/mesh/occ/occmeshparams.h>
#include <xflwidgets/color/colorgraddlg.h>
#include <xflwidgets/color/colormenubtn.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/line/linebtn.h>
#include <xflwidgets/line/linemenu.h>

QColor W3dPrefs::s_BackColor;

bool W3dPrefs::s_bSaveViewPoints = true;
bool W3dPrefs::s_bShowRefLength = false;
bool W3dPrefs::s_bEnableClipPlane = true;
bool W3dPrefs::s_bAutoAdjustScale = false;
bool W3dPrefs::s_bWakePanels = false;

double W3dPrefs::s_MassRadius = .017;
QColor W3dPrefs::s_MassColor = QColor(95,128,99);

bool W3dPrefs::s_bSpinAnimation(true);
double W3dPrefs::s_SpinDamping(0.01);


LineStyle W3dPrefs::s_SelectStyle     = LineStyle(true, Line::SOLID,   5, QColor(255,35, 15),         Line::NOSYMBOL);
LineStyle W3dPrefs::s_HighStyle       = LineStyle(true, Line::SOLID,   5, QColor(65,105,225),         Line::NOSYMBOL);
LineStyle W3dPrefs::s_AxisStyle       = LineStyle(true, Line::DASHDOT, 1, QColor(150,150,150),        Line::NOSYMBOL);
LineStyle W3dPrefs::s_WindStyle       = LineStyle(true, Line::SOLID,   3, QColor(75, 75, 75),         Line::NOSYMBOL);
LineStyle W3dPrefs::s_PanelStyle      = LineStyle(true, Line::SOLID,   1, QColor(117, 117, 117),      Line::NOSYMBOL);
LineStyle W3dPrefs::s_OutlineStyle    = LineStyle(true, Line::SOLID,   1, QColor(41,41,41),           Line::NOSYMBOL);
LineStyle W3dPrefs::s_LiftStyle       = LineStyle(true, Line::SOLID,   3, QColor(105, 105, 105),      Line::NOSYMBOL);
LineStyle W3dPrefs::s_VelocityStyle   = LineStyle(true, Line::SOLID,   1, QColor(255, 100, 100),      Line::NOSYMBOL);
LineStyle W3dPrefs::s_StreamStyle     = LineStyle(true, Line::DASH,    1, QColor(105, 105, 105),      Line::NOSYMBOL);
LineStyle W3dPrefs::s_MomentStyle     = LineStyle(true, Line::DASH,    1, QColor(200, 177, 100),      Line::NOSYMBOL);
LineStyle W3dPrefs::s_IDragStyle      = LineStyle(true, Line::DASH,    1, QColor(215,100,125),        Line::NOSYMBOL);
LineStyle W3dPrefs::s_VDragStyle      = LineStyle(true, Line::DASH,    1, QColor(215,125,100),        Line::NOSYMBOL);
LineStyle W3dPrefs::s_CpStyle         = LineStyle(true, Line::SOLID,   1, QColor(255,0,0),            Line::NOSYMBOL);
LineStyle W3dPrefs::s_TransStyle      = LineStyle(true, Line::SOLID,   1, QColor(171, 103, 220),      Line::NOSYMBOL);
LineStyle W3dPrefs::s_FlowStyle       = LineStyle(true, Line::SOLID,   2, QColor(101, 101, 231, 153), Line::NOSYMBOL);

bool W3dPrefs::s_bUseWingColour = false;

bool W3dPrefs::s_bUseBackClr = false;
QColor W3dPrefs::s_WingPanelColor = QColor(231,231,231);
QColor W3dPrefs::s_FusePanelColor = QColor(241,241,241);
QColor W3dPrefs::s_FlapPanelColor = QColor(227,227,227);
QColor W3dPrefs::s_WakePanelColor = QColor(215, 215, 215, 105);

bool W3dPrefs::s_bShowGround = true;
QColor W3dPrefs::s_WaterColor = QColor(51,77,89,100);
double W3dPrefs::s_BoxX=30, W3dPrefs::s_BoxY=30, W3dPrefs::s_BoxZ=0.1;

int W3dPrefs::s_iChordwiseRes=29;
int W3dPrefs::s_iSailXRes = 37;
int W3dPrefs::s_iSailZRes = 31;
int W3dPrefs::s_iBodyAxialRes=57;
int W3dPrefs::s_iBodyHoopRes=29;

int W3dPrefs::s_NContourLines = 11;
LineStyle W3dPrefs::s_ContourLineStyle = LineStyle(true, Line::SOLID, 1, QColor(125,125,125), Line::NOSYMBOL);;

QColor W3dPrefs::s_VortonColor = QColor(191,191,191);
double W3dPrefs::s_VortonRadius = 0.010;


OccMeshParams W3dPrefs::s_OccTessParams;


W3dPrefs::W3dPrefs(QWidget *pParent) : QWidget(pParent)
{
    setWindowTitle(tr("3d Styles"));
    setupLayout();
    connectSignals();
}


void W3dPrefs::connectSignals()
{
    connect(m_pchSpinAnimation,      SIGNAL(clicked(bool)),         SLOT(onOther3dChanged()));
    connect(m_pchAnimateTransitions, SIGNAL(clicked(bool)),         SLOT(onOther3dChanged()));

    connect(m_plbHighlight,          SIGNAL(clickedLB(LineStyle)),  SLOT(onHighlight()));
    connect(m_plbSelect,             SIGNAL(clickedLB(LineStyle)),  SLOT(onSelect()));

    connect(m_plbAxis,               SIGNAL(clickedLB(LineStyle)),  SLOT(on3dAxis()));
    connect(m_plbWind,               SIGNAL(clickedLB(LineStyle)),  SLOT(onWind()));
    connect(m_plbOutline,            SIGNAL(clickedLB(LineStyle)),  SLOT(onOutline()));
    connect(m_plbMeshOutline,        SIGNAL(clickedLB(LineStyle)),  SLOT(onVLMMesh()));
    connect(m_plbTrans,              SIGNAL(clickedLB(LineStyle)),  SLOT(onTransition()));
    connect(m_plbLift,               SIGNAL(clickedLB(LineStyle)),  SLOT(onXCP()));
    connect(m_plbMoments,            SIGNAL(clickedLB(LineStyle)),  SLOT(onMoments()));
    connect(m_plbInducedDrag,        SIGNAL(clickedLB(LineStyle)),  SLOT(onIDrag()));
    connect(m_plbViscousDrag,        SIGNAL(clickedLB(LineStyle)),  SLOT(onVDrag()));
    connect(m_plbVelocity,           SIGNAL(clickedLB(LineStyle)),  SLOT(onVelocity()));
    connect(m_plbStreamLines,        SIGNAL(clickedLB(LineStyle)),  SLOT(onStreamLines()));
    connect(m_plbFlowLines,          SIGNAL(clickedLB(LineStyle)),  SLOT(onFlowLines()));

    connect(m_plbContourLines,       SIGNAL(clickedLB(LineStyle)),  SLOT(onContourLines()));

    connect(m_pcmbWaterColor,        SIGNAL(clickedCB(QColor)),     SLOT(onWaterColor(QColor)));

    connect(m_pcmbMassColor,         SIGNAL(clickedCB(QColor)),     SLOT(onMasses(QColor)));
    connect(m_pchBackPanelClr,       SIGNAL(clicked(bool)),         SLOT(onBackPanelClr()));
    connect(m_pcmbFusePanelClr,      SIGNAL(clickedCB(QColor)),     SLOT(onFusePanelClr(QColor)));
    connect(m_pcmbWingPanelClr,      SIGNAL(clickedCB(QColor)),     SLOT(onWingPanelClr(QColor)));
    connect(m_pcmbFlapPanelClr,      SIGNAL(clickedCB(QColor)),     SLOT(onFlapPanelClr(QColor)));
    connect(m_pcmbWakePanelClr,      SIGNAL(clickedCB(QColor)),     SLOT(onWakePanelClr(QColor)));

    connect(m_pcmbVortonColour,     SIGNAL(clickedCB(QColor)),     SLOT(onVortonClr(QColor)));
}


void W3dPrefs::readData()
{
    s_bUseWingColour = m_pchUseWingColour->isChecked();

    s_bAutoAdjustScale = m_pchAutoAdjustScale->isChecked();
    s_bEnableClipPlane = m_pcbEnableClipPlane->isChecked();
    s_bShowRefLength   = m_pchShowRefLength->isChecked();
    s_bSaveViewPoints  = m_pchSaveViewPoints->isChecked();

    Fuse::setOccTessellator(m_prbOcc->isChecked());
    s_iChordwiseRes = m_pieChordwiseRes->value();
    s_iBodyAxialRes = m_pieBodyAxialRes->value();
    s_iBodyHoopRes  = m_pieBodyHoopRes->value();
    s_iSailXRes     = m_pieSailXRes->value();
    s_iSailZRes     = m_pieSailZRes->value();

    s_bShowGround = m_pchGround->isChecked();
    s_BoxX        = m_pdeBoxX->value()/Units::mtoUnit();
    s_BoxY        = m_pdeBoxY->value()/Units::mtoUnit();
    s_BoxZ        = m_pdeBoxZ->value()/Units::mtoUnit();

    ArcBall::setSphereRadius(std::min(m_pdeArcballRadius->value()/100.0, 1.0));

    W3dPrefs::setSpinAnimation(m_pchSpinAnimation->isChecked());
    W3dPrefs::setSpinDamping(m_pdeSpinDamping->value()/100.0);
    gl3dView::setAnimationTransitions(m_pchAnimateTransitions->isChecked());
    gl3dView::setTransitionTime(m_pieAnimationTime->value());

    gl3dView::setZAnimAngle(m_pdeZAnimAngle->value());

    s_HighStyle = m_plbHighlight->theLineStyle();
    s_SelectStyle = m_plbSelect->theLineStyle();

    s_NContourLines = m_pieNContourLines->value();

    s_VortonColor = m_pcmbVortonColour->color();
    s_VortonRadius = m_pdeVortonRadius->value()/100.0;
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
    m_plbStreamLines->setTheStyle(s_StreamStyle);
    m_plbFlowLines->setTheStyle(s_FlowStyle);
    m_pchUseWingColour->setChecked(s_bUseWingColour);
    m_plbTrans->setTheStyle(s_TransStyle);

    m_pcmbWaterColor->setColor(s_WaterColor);
    m_pchGround->setChecked(s_bShowGround);
    m_pdeBoxX->setValue(s_BoxX*Units::mtoUnit());
    m_pdeBoxY->setValue(s_BoxY*Units::mtoUnit());
    m_pdeBoxZ->setValue(s_BoxZ*Units::mtoUnit());

    m_pcmbMassColor->setColor(s_MassColor);

    m_pchBackPanelClr->setChecked(s_bUseBackClr);
    m_pcmbFusePanelClr->setEnabled(!s_bUseBackClr);
    m_pcmbWingPanelClr->setEnabled(!s_bUseBackClr);
    m_pcmbFlapPanelClr->setEnabled(!s_bUseBackClr);
    m_pcmbWakePanelClr->setEnabled(!s_bUseBackClr);
    m_pcmbFusePanelClr->setColor(s_FusePanelColor);
    m_pcmbWingPanelClr->setColor(s_WingPanelColor);
    m_pcmbFlapPanelClr->setColor(s_FlapPanelColor);
    m_pcmbWakePanelClr->setColor(s_WakePanelColor);

    m_prbOcc->setChecked(Fuse::isOccTessellator());
    m_prbFlow5->setChecked(!Fuse::isOccTessellator());
    m_pieChordwiseRes->setValue(s_iChordwiseRes);
    m_pieBodyAxialRes->setValue(s_iBodyAxialRes);
    m_pieBodyHoopRes->setValue(s_iBodyHoopRes);
    m_pieSailXRes->setValue(s_iSailXRes);
    m_pieSailZRes->setValue(s_iSailZRes);

    m_pdeArcballRadius->setValue(ArcBall::sphereRadius()*100.0);

    m_pchSpinAnimation->setChecked(s_bSpinAnimation);
    m_pdeSpinDamping->setValue(s_SpinDamping*100.0);

    m_pchAnimateTransitions->setChecked(gl3dView::bAnimateTransitions());
    m_pieAnimationTime->setValue(gl3dView::transitionTime());

    m_pdeZAnimAngle->setValue(gl3dView::zAnimAngle());

    onOther3dChanged();

    m_pchAutoAdjustScale->setChecked(s_bAutoAdjustScale);
    m_pcbEnableClipPlane->setChecked(s_bEnableClipPlane);
    m_pchShowRefLength->setChecked(s_bShowRefLength);
    m_pchSaveViewPoints->setChecked(s_bSaveViewPoints);

    updateGradientBtn();

    m_pieNContourLines->setValue(s_NContourLines);
    m_plbContourLines->setTheStyle(s_ContourLineStyle);

    m_pdeVortonRadius->setValue(s_VortonRadius*100.0);
    m_pcmbVortonColour->setColor(s_VortonColor);
}


void W3dPrefs::setupLayout()
{
    m_pGroupBox.push_back(new QGroupBox("Colour settings"));
    {
        QVBoxLayout *pColorPrefs = new QVBoxLayout;
        {
            QGridLayout *pColorGridLayout = new QGridLayout;
            {
                QLabel *pLabGeom        = new QLabel(tr("Geometry"));
                QLabel *pLabMesh        = new QLabel(tr("Mesh"));
                QLabel *pLabVortons     = new QLabel("Vortons");
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
                m_plbStreamLines  = new LineBtn(this);
                m_plbFlowLines    = new LineBtn(this);

                m_pchUseWingColour = new QCheckBox(tr("Use wing colour"));

                m_pcmbMassColor    = new ColorMenuBtn;

                m_pchBackPanelClr  = new QCheckBox(tr("Use background color for mesh panels"));
                m_plbMeshOutline   = new LineBtn(this);
                m_pcmbFusePanelClr  = new ColorMenuBtn;
                m_pcmbWingPanelClr  = new ColorMenuBtn;
                m_pcmbFlapPanelClr  = new ColorMenuBtn;
                m_pcmbWakePanelClr  = new ColorMenuBtn;

                m_pdeVortonRadius   = new DoubleEdit;
                m_pcmbVortonColour = new ColorMenuBtn;

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
                m_pcmbVortonColour->setBackground(true);

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
                pColorGridLayout->addWidget(m_pcmbMassColor,     5,4);

                pColorGridLayout->addWidget(pLabMesh,            6,1,1,4, Qt::AlignCenter);
                pColorGridLayout->addWidget(pLabVLM,             7,1, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_plbMeshOutline,    7,2);

                pColorGridLayout->addWidget(m_pchBackPanelClr,    8,1,1,4, Qt::AlignCenter);
                pColorGridLayout->addWidget(pLabFuse,             9,1, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_pcmbFusePanelClr,   9,2);
                pColorGridLayout->addWidget(pLabWing,             9,3, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_pcmbWingPanelClr,   9,4);
                pColorGridLayout->addWidget(pLabFlap,             10,1, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_pcmbFlapPanelClr,   10,2);
                pColorGridLayout->addWidget(pLabWake,             10,3, Qt::AlignVCenter|Qt::AlignRight);
                pColorGridLayout->addWidget(m_pcmbWakePanelClr,   10,4);

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
                pColorGridLayout->addWidget(m_pcmbVortonColour,     18,2);
                pColorGridLayout->addWidget(new QLabel("Radius:"),  18,3, Qt::AlignVCenter | Qt::AlignRight);
                pColorGridLayout->addWidget(m_pdeVortonRadius,      18,4);
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
                            m_pcmbWaterColor   = new ColorMenuBtn;

                            pGroundLayout->addRow(m_pchGround);
                            pGroundLayout->addRow(tr("Ground/Water"), m_pcmbWaterColor);
                        }
                        QGridLayout *pBoxSizeLayout = new QGridLayout;
                        {
                            QLabel *pLabX = new QLabel("x-length");
                            QLabel *pLabY = new QLabel("y-width");
                            QLabel *pLabZ = new QLabel("z-height");

                            m_pLabXUnit = new QLabel(Units::lengthUnitLabel());
                            m_pLabYUnit = new QLabel(Units::lengthUnitLabel());
                            m_pLabZUnit = new QLabel(Units::lengthUnitLabel());
                            m_pdeBoxX = new DoubleEdit;
                            m_pdeBoxY = new DoubleEdit;
                            m_pdeBoxZ = new DoubleEdit;

                            pBoxSizeLayout->addWidget(pLabX,     1, 1, Qt::AlignVCenter|Qt::AlignRight);
                            pBoxSizeLayout->addWidget(m_pdeBoxX, 1, 2);
                            pBoxSizeLayout->addWidget(m_pLabXUnit, 1, 3, Qt::AlignVCenter|Qt::AlignLeft);
                            pBoxSizeLayout->addWidget(pLabY,     2, 1, Qt::AlignVCenter|Qt::AlignRight);
                            pBoxSizeLayout->addWidget(m_pdeBoxY, 2, 2);
                            pBoxSizeLayout->addWidget(m_pLabYUnit, 2, 3, Qt::AlignVCenter|Qt::AlignLeft);
                            pBoxSizeLayout->addWidget(pLabZ,     3, 1, Qt::AlignVCenter|Qt::AlignRight);
                            pBoxSizeLayout->addWidget(m_pdeBoxZ, 3, 2);
                            pBoxSizeLayout->addWidget(m_pLabZUnit, 3, 3, Qt::AlignVCenter|Qt::AlignLeft);
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
        m_pGroupBox.last()->setLayout(pColorPrefs);
    }

    m_pGroupBox.push_back(new QGroupBox(tr("Tessellation")));
    {
        QVBoxLayout *pTessLayout = new QVBoxLayout;
        {
            QHBoxLayout *pTessellatorLayout = new QHBoxLayout;
            {
                QLabel *pTesslab = new QLabel(tr("Fuselage tessellator:"));
                m_prbOcc   = new QRadioButton("OCC inc. mesh");
                m_prbFlow5 = new QRadioButton("flow5 mesher");
                m_prbOcc->setEnabled(false);
                m_prbFlow5->setEnabled(false);

                pTessellatorLayout->addWidget(pTesslab);
                pTessellatorLayout->addWidget(m_prbOcc);
                pTessellatorLayout->addWidget(m_prbFlow5);
                pTessellatorLayout->addStretch();
            }

            QLabel *pTessLabel = new QLabel(tr("Increase the number of points to improve the tessellation of the surfaces.<br>"
                                               "This may increase the loading times and also slow down the display on low-end graphic cards.<br>"
                                               "Reload required to take effect."));

            m_pieChordwiseRes = new IntEdit(37, this);
            m_pieBodyAxialRes = new IntEdit(29, this);
            m_pieBodyHoopRes  = new IntEdit(17, this);
            m_pieSailXRes     = new IntEdit(37, this);
            m_pieSailZRes     = new IntEdit(31, this);

            QGroupBox *pRuledTessellationBox = new QGroupBox("Ruled surfaces");
            {
                QFormLayout *pFormLayout = new QFormLayout;
                {
                    pFormLayout->addRow(tr("Wing chordwise direction:"), m_pieChordwiseRes);
                    pFormLayout->addRow(tr("Body axial direction:"),     m_pieBodyAxialRes);
                    pFormLayout->addRow(tr("Body hoop direction:"),      m_pieBodyHoopRes);
                    pFormLayout->addRow(tr("Sail x-direction:"),         m_pieSailXRes);
                    pFormLayout->addRow(tr("Sail z-direction:"),         m_pieSailZRes);
                }
                pRuledTessellationBox->setLayout(pFormLayout);
            }
            pTessLayout->addLayout(pTessellatorLayout);
            pTessLayout->addWidget(pTessLabel);
            pTessLayout->addWidget(pRuledTessellationBox);
        }
        m_pGroupBox.last()->setLayout(pTessLayout);
    }

    m_pGroupBox.push_back(new QGroupBox(tr("Other")));
    {
        QVBoxLayout *pOtherLayout = new QVBoxLayout;
        {
            QGridLayout *pGridLayout = new QGridLayout;
            {   
                m_pchSpinAnimation = new QCheckBox("Enable mouse animations");
                m_pdeSpinDamping = new DoubleEdit;
                m_pdeSpinDamping->setToolTip("Defines the damping of the animation at each frame update.<br>"
                                             "Set to 0 for perpetual movement.");
                QLabel *plabpcDamping = new QLabel("% damping");
                pGridLayout->addWidget(m_pchSpinAnimation, 1, 1);
                pGridLayout->addWidget(m_pdeSpinDamping,   1, 2);
                pGridLayout->addWidget(plabpcDamping,      1, 3);

                m_pchAnimateTransitions = new QCheckBox(tr("Animate view transitions"));
                m_pieAnimationTime = new IntEdit;
                QString tip = tr("Defines the duration of animations in ms");
                m_pieAnimationTime->setToolTip(tip);
                QLabel *pLabms =new QLabel("ms");
                pGridLayout->addWidget(m_pchAnimateTransitions, 2, 1);
                pGridLayout->addWidget(m_pieAnimationTime,      2, 2);
                pGridLayout->addWidget(pLabms,                  2, 3);

                QLabel *labArcBall = new QLabel(tr("Arcball radius:"));
                QLabel *labPercent = new QLabel("% view width");
                m_pdeArcballRadius = new DoubleEdit(100.0,0);
                m_pdeArcballRadius->setToolTip(tr("The radius of the arcball as a percentage of the view's width"));

                pGridLayout->addWidget(labArcBall,         3, 1, Qt::AlignVCenter |Qt::AlignRight);
                pGridLayout->addWidget(m_pdeArcballRadius, 3, 2);
                pGridLayout->addWidget(labPercent,         3, 3);

                QLabel *pLabZAngle    = new QLabel("Auto z-rotation incremental angle:");
                m_pdeZAnimAngle = new DoubleEdit(1);
                QLabel *plabDeg = new QLabel("<p>&deg;</p>");
                pGridLayout->addWidget(pLabZAngle,      4, 1, Qt::AlignVCenter |Qt::AlignRight);
                pGridLayout->addWidget(m_pdeZAnimAngle, 4, 2);
                pGridLayout->addWidget(plabDeg,         4, 3);
                pGridLayout->setColumnStretch(          5, 1);
            }
            m_pchAutoAdjustScale = new QCheckBox("Auto adjust 3d scale");
            m_pchAutoAdjustScale->setToolTip(tr("Automatically adjust the 3D scale to fit the plane in the display when switching between planes"));
            m_pcbEnableClipPlane = new QCheckBox("Enable clip plane");
            m_pchShowRefLength   = new QCheckBox("Display reference length");
            m_pchSaveViewPoints  = new QCheckBox("Save viewpoints when exiting 3d views");
            pOtherLayout->addLayout(pGridLayout);

            pOtherLayout->addWidget(m_pchAutoAdjustScale);
            pOtherLayout->addWidget(m_pcbEnableClipPlane);
            pOtherLayout->addWidget(m_pchShowRefLength);
            pOtherLayout->addWidget(m_pchSaveViewPoints);
        }
        m_pGroupBox.last()->setLayout(pOtherLayout);
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
    //	lm.enableSubMenus(true, true, true, false);
    lm.initMenu(s_AxisStyle);
    lm.exec(QCursor::pos());

    s_AxisStyle = lm.theStyle();
    m_plbAxis->setTheStyle(s_AxisStyle);
}


void W3dPrefs::onHighlight()
{
    LineMenu lm(nullptr, false);
    //	lm.enableSubMenus(true, true, true, false);
    lm.initMenu(s_HighStyle);
    lm.exec(QCursor::pos());

    s_HighStyle = lm.theStyle();
    m_plbHighlight->setTheStyle(s_HighStyle);
}


void W3dPrefs::onSelect()
{
    LineMenu lm(nullptr, false);
    //	lm.enableSubMenus(true, true, true, false);
    lm.initMenu(s_SelectStyle);
    lm.exec(QCursor::pos());

    s_SelectStyle = lm.theStyle();
    m_plbSelect->setTheStyle(s_SelectStyle);
}


void W3dPrefs::onWind()
{
    LineMenu lm(nullptr, false);
    //	lm.enableSubMenus(true, true, true, false);
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


void W3dPrefs::onMasses(QColor clr)
{
    if(clr.isValid()) s_MassColor = clr;
    m_pcmbMassColor->setColor(s_MassColor);

    update();
}


void W3dPrefs::onWaterColor(QColor clr)
{
    if(clr.isValid()) s_WaterColor = clr;
    m_pcmbWaterColor->setColor(s_WaterColor);

    update();
}


void W3dPrefs::onBackPanelClr()
{
    s_bUseBackClr = m_pchBackPanelClr->isChecked();
    m_pcmbFusePanelClr->setEnabled(!s_bUseBackClr);
    m_pcmbWingPanelClr->setEnabled(!s_bUseBackClr);
    m_pcmbFlapPanelClr->setEnabled(!s_bUseBackClr);
    m_pcmbWakePanelClr->setEnabled(!s_bUseBackClr);
    if(s_bUseBackClr)
    {
        m_pcmbFusePanelClr->setColor(s_BackColor);
        m_pcmbWingPanelClr->setColor(s_BackColor);
        m_pcmbFlapPanelClr->setColor(s_BackColor);
        m_pcmbWakePanelClr->setColor(s_BackColor);
        s_WingPanelColor = s_BackColor;
        s_FusePanelColor = s_BackColor;
        s_FlapPanelColor = s_BackColor;
        s_WakePanelColor = s_BackColor;
    }
}


void W3dPrefs::onFusePanelClr(QColor clr)
{
    if(clr.isValid()) s_FusePanelColor = clr;
    m_pcmbFusePanelClr->setColor(s_FusePanelColor);

    update();
}


void W3dPrefs::onWingPanelClr(QColor clr)
{

    if(clr.isValid()) s_WingPanelColor = clr;
    m_pcmbWingPanelClr->setColor(s_WingPanelColor);

    update();
}


void W3dPrefs::onFlapPanelClr(QColor clr)
{
    if(clr.isValid()) s_FlapPanelColor = clr;
    m_pcmbFlapPanelClr->setColor(s_FlapPanelColor);

    update();
}


void W3dPrefs::onWakePanelClr(QColor clr)
{   
    if(clr.isValid()) s_WakePanelColor = clr;
    m_pcmbWakePanelClr->setColor(s_WakePanelColor);

    update();
}


void W3dPrefs::onVortonClr(QColor clr)
{
    if(clr.isValid()) s_VortonColor = clr;
    m_pcmbVortonColour->setColor(s_VortonColor);

    update();
}


void W3dPrefs::onShowWake()
{
}


void W3dPrefs::onOther3dChanged()
{
    m_pdeSpinDamping->setEnabled(m_pchSpinAnimation->isChecked());
    m_pieAnimationTime->setEnabled(m_pchAnimateTransitions->isChecked());
}


void W3dPrefs::saveSettings(QSettings &settings)
{
    settings.beginGroup("3DPrefs");
    {
        s_AxisStyle.saveSettings(     settings, "AxisStyle");
        s_HighStyle.saveSettings(     settings, "HighlightStyle");
        s_SelectStyle.saveSettings(   settings, "SelectionStyle");
        s_WindStyle.saveSettings(     settings, "WindStyle");
        s_PanelStyle.saveSettings(    settings, "PanelStyle");
        s_OutlineStyle.saveSettings(  settings, "OutlineStyle");
        s_LiftStyle.saveSettings(     settings, "LiftForceStyle");
        s_MomentStyle.saveSettings(   settings, "MomentStyle");
        s_VDragStyle.saveSettings(    settings, "VDragStyle");
        s_IDragStyle.saveSettings(    settings, "IDragStyle");
        s_VelocityStyle.saveSettings( settings, "VelocityStyle");
        s_CpStyle.saveSettings(       settings, "CpStyle");
        s_TransStyle.saveSettings(    settings, "TopStyle");
        s_StreamStyle.saveSettings(   settings, "StreamStyle");
        s_FlowStyle.saveSettings(     settings, "FlowLineStyle");

        settings.setValue("NContourLines", s_NContourLines);
        s_ContourLineStyle.saveSettings(   settings, "ContourLineStyle");

        settings.setValue("VortonColour", s_VortonColor);
        settings.setValue("VortonRelRadius", s_VortonRadius);

        settings.setValue("UseWingColour", s_bUseWingColour);

        settings.setValue("showWakePanels", s_bWakePanels);

        settings.setValue("WaterColor",  s_WaterColor);
        settings.setValue("bShowGround", s_bShowGround);
        settings.setValue("GroundBoxX",  s_BoxX);
        settings.setValue("GroundBoxY",  s_BoxY);
        settings.setValue("GroundBoxZ",  s_BoxZ);


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

        settings.setValue("OccTessellator", Fuse::isOccTessellator());
        settings.setValue("ChordwiseRes", s_iChordwiseRes);
        settings.setValue("BodyAxialRes", s_iBodyAxialRes);
        settings.setValue("BodyHoopRes",  s_iBodyHoopRes);
        settings.setValue("SailXRes",     s_iSailXRes);
        settings.setValue("SailZRes",     s_iSailZRes);

        s_OccTessParams.saveSettings(settings);

        settings.setValue("ArcBallRadius", ArcBall::sphereRadius());

        settings.setValue("SpinAnimation",      s_bSpinAnimation);
        settings.setValue("SpinDamping",        s_SpinDamping);

        settings.setValue("AnimateTransitions", gl3dView::bAnimateTransitions());
        settings.setValue("TransitionTime",     gl3dView::transitionTime());
        settings.setValue("ZAnimAngle",         gl3dView::zAnimAngle());
    }
    settings.endGroup();
}


void W3dPrefs::loadSettings(QSettings &settings)
{
    resetDefaults();
    settings.beginGroup("3DPrefs");
    {
        s_AxisStyle.loadSettings(     settings, "AxisStyle");
        s_HighStyle.loadSettings(     settings, "HighlightStyle");
        s_SelectStyle.loadSettings(   settings, "SelectionStyle");
        s_WindStyle.loadSettings(     settings, "WindStyle");
        s_PanelStyle.loadSettings(    settings, "PanelStyle");
        s_OutlineStyle.loadSettings(  settings, "OutlineStyle");
        s_LiftStyle.loadSettings(     settings, "LiftForceStyle");
        s_MomentStyle.loadSettings(   settings, "MomentStyle");
        s_VDragStyle.loadSettings(    settings, "VDragStyle");
        s_IDragStyle.loadSettings(    settings, "IDragStyle");
        s_VelocityStyle.loadSettings( settings, "VelocityStyle");
        s_CpStyle.loadSettings(       settings, "CpStyle");
        s_TransStyle.loadSettings(    settings, "TopStyle");
        s_StreamStyle.loadSettings(   settings, "StreamStyle");
        s_FlowStyle.loadSettings(     settings, "FlowLineStyle");

        s_NContourLines = settings.value("NContourLines", s_NContourLines).toInt();
        s_ContourLineStyle.loadSettings(   settings, "ContourLineStyle");

        s_VortonColor = settings.value("VortonColour", s_VortonColor).value<QColor>();
        s_VortonRadius = settings.value("VortonRelRadius", s_VortonRadius).toDouble();

        s_bUseWingColour = settings.value("UseWingColour", false).toBool();

        s_WaterColor  = settings.value("WaterColor", s_WaterColor).value<QColor>();
        s_bShowGround = settings.value("bShowGround", s_bShowGround).toBool();
        s_BoxX        = settings.value("GroundBoxX",  s_BoxX).toDouble();
        s_BoxY        = settings.value("GroundBoxY",  s_BoxY).toDouble();
        s_BoxZ        = settings.value("GroundBoxZ",  s_BoxZ).toDouble();

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

        Fuse::setOccTessellator(settings.value("OccTessellator", true).toBool());

        s_iChordwiseRes = settings.value("ChordwiseRes", s_iChordwiseRes).toInt();
        s_iBodyAxialRes = settings.value("BodyAxialRes", s_iBodyAxialRes).toInt();
        s_iBodyHoopRes  = settings.value("BodyHoopRes",  s_iBodyHoopRes).toInt();
        s_iSailXRes     = settings.value("SailXRes",     s_iSailXRes).toInt();
        s_iSailZRes     = settings.value("SailZRes",     s_iSailZRes).toInt();

        s_OccTessParams.loadSettings(settings);

        ArcBall::setSphereRadius(settings.value("ArcBallRadius", 0.8).toDouble());

        W3dPrefs::setSpinAnimation(settings.value("SpinAnimation", s_bSpinAnimation).toBool());
        W3dPrefs::setSpinDamping(settings.value("SpinDamping", s_SpinDamping).toDouble());

        gl3dView::setAnimationTransitions(settings.value("AnimateTransitions", gl3dView::bAnimateTransitions()).toBool());
        gl3dView::setTransitionTime(      settings.value("TransitionTime",     gl3dView::transitionTime()).toInt());
        gl3dView::setZAnimAngle(          settings.value("ZAnimAngle",         gl3dView::zAnimAngle()).toDouble());
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

    s_SelectStyle   = LineStyle(true, Line::SOLID, 5, QColor(255,75,75),  Line::NOSYMBOL);
    s_HighStyle     = LineStyle(true, Line::SOLID, 5, QColor(75,75,255),  Line::NOSYMBOL);

    s_WaterColor = QColor(51,77,89,100);
    s_MassColor = QColor(95,128,99);

    s_PanelStyle.m_Stipple    = Line::SOLID;
    s_PanelStyle.m_Width      = 1;
    s_PanelStyle.m_Color      = QColor(87,87,87);
    s_AxisStyle.m_Stipple     = Line::DASHDOT;
    s_AxisStyle.m_Width       = 1;
    s_AxisStyle.m_Color       = QColor(150,150,150);
    s_WindStyle.m_Stipple     = Line::DASHDOT;
    s_WindStyle.m_Width       = 3;
    s_WindStyle.m_Color       = QColor(75,75,75);
    s_OutlineStyle.m_Stipple  = Line::SOLID;
    s_OutlineStyle.m_Width    = 1;
    s_OutlineStyle.m_Color    = QColor(41, 41, 41);
    s_LiftStyle.m_Stipple     = Line::SOLID;
    s_LiftStyle.m_Width       = 3;
    s_LiftStyle.m_Color       = QColor(105, 105, 105);
    s_MomentStyle.m_Stipple   = Line::SOLID;
    s_MomentStyle.m_Width     = 2;
    s_MomentStyle.m_Color     = QColor(200, 100, 100);

    s_IDragStyle.m_Stipple    = Line::DASH;
    s_IDragStyle.m_Width      = 2;
    s_IDragStyle.m_Color      = QColor(215,100,125);
    s_VDragStyle.m_Stipple    = Line::DASH;
    s_VDragStyle.m_Width      = 2;
    s_VDragStyle.m_Color      = QColor(215,125,100);

    s_VelocityStyle.m_Stipple = Line::SOLID;
    s_VelocityStyle.m_Width   = 2;
    s_VelocityStyle.m_Color   = QColor(255, 100, 100);

    s_CpStyle.m_Stipple = Line::SOLID;
    s_CpStyle.m_Width   = 1;
    s_CpStyle.m_Color   = QColor(255,0,0);

    s_FlowStyle.m_Stipple = Line::SOLID;
    s_FlowStyle.m_Width   = 2;
    s_FlowStyle.m_Color   = QColor(101, 101, 231, 153);

    s_StreamStyle.m_Stipple  = Line::DASH;
    s_StreamStyle.m_Width    = 1;
    s_StreamStyle.m_Color    = QColor(105, 105, 105);

    s_TransStyle.m_Stipple = Line::SOLID;
    s_TransStyle.m_Width   = 2;
    s_TransStyle.m_Color   = QColor(171, 103, 220);

    s_bUseBackClr = false;
    s_WingPanelColor = QColor(231,231,231);
    s_FusePanelColor = QColor(241,241,241);
    s_FlapPanelColor = QColor(227,227,227);
    s_WakePanelColor = QColor(215,215,215, 105);

    s_BoxX = s_BoxY = 10.0;
    s_BoxZ = 0.03;

    s_NContourLines = 11;

    s_VortonColor = QColor(191,191,191);
    s_VortonRadius = 0.010;

    s_iChordwiseRes= 27;
    s_iBodyAxialRes=57;
    s_iBodyHoopRes=29;
    s_iSailXRes = 37;
    s_iSailZRes = 31;

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
    m_pdeBoxX->setValue(s_BoxX*Units::mtoUnit());
    m_pdeBoxY->setValue(s_BoxY*Units::mtoUnit());
    m_pdeBoxZ->setValue(s_BoxZ*Units::mtoUnit());
    m_pLabXUnit->setText(Units::lengthUnitLabel());
    m_pLabYUnit->setText(Units::lengthUnitLabel());
    m_pLabZUnit->setText(Units::lengthUnitLabel());
}


void W3dPrefs::updateGradientBtn()
{
    QString style("background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,");
    QString strange;
    for(int i=0; i<11; i++ )
    {
        double fi = double(i)/double(11);
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
