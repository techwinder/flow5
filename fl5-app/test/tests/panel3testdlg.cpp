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


#include <QGridLayout>
#include <QFormLayout>

#include "panel3testdlg.h"
#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <api/vector3d.h>
#include <test/test3d/gl3dpanelfield.h>
#include <interfaces/graphs/containers/graphwt.h>
#include <interfaces/graphs/controls/graphoptions.h>
#include <interfaces/graphs/graph/graph.h>
#include <api/panel3.h>
#include <api/panel4.h>
#include <api/vortex.h>
#include <api/testpanels.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>

LineStyle Panel3TestDlg::s_LS = {true, Line::SOLID, 2, fl5Color(100,155,221), Line::NOSYMBOL};

bool Panel3TestDlg::s_b1TriPanel(true);
bool Panel3TestDlg::s_bP3(true);


QByteArray Panel3TestDlg::s_Geometry;
QByteArray Panel3TestDlg::s_HSplitterSizes;
Vortex::enumVortex Panel3TestDlg::s_VortexModel;
double Panel3TestDlg::s_CoreRadius = 0.0001;
xfl::enumDistribution s_XDistrib=xfl::EXP;
xfl::enumDistribution s_YDistrib=xfl::TANH;
int Panel3TestDlg::s_NXPanels = 5;
int Panel3TestDlg::s_NYPanels = 3;
double Panel3TestDlg::s_Side = 1.0;
bool Panel3TestDlg::s_bSource = true;

PANELMETHOD Panel3TestDlg::s_Method = NASA4023;

double Panel3TestDlg::s_X0=  1.5;
double Panel3TestDlg::s_Y0= -2.0;
double Panel3TestDlg::s_Z0=  0.0;
double Panel3TestDlg::s_X1=  1.5;
double Panel3TestDlg::s_Y1=  2.0;
double Panel3TestDlg::s_Z1=  0.0;
int Panel3TestDlg::s_nCurvePts = 100;

bool Panel3TestDlg::s_bPotential = true;
int Panel3TestDlg::s_VComp = 0;


Panel3TestDlg::Panel3TestDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Panel3 test");
    setWindowFlag(Qt::WindowStaysOnTopHint);

    m_pGraphWt = new GraphWt(this);
    {
        m_pGraphWt->showLegend(true);
        m_pGraphWt->enableContextMenu(true);
        m_pGraph = new Graph;
        m_pGraphWt->setGraph(m_pGraph);
        m_pGraph->setCurveModel(new CurveModel);
        GraphOptions::resetGraphSettings(*m_pGraph);
        m_pGraph->setLegendVisible(true);
        m_pGraph->setLegendPosition(Qt::AlignHCenter | Qt::AlignTop);
        m_pGraph->setScaleType(GRAPH::RESETTING);
        m_pGraph->setAuto(true);
    }

    setupLayout();
    connectSignals();
}


Panel3TestDlg::~Panel3TestDlg()
{
    if(m_pGraph)
    {
        if(m_pGraph->curveModel()) m_pGraph->deleteCurveModel();
        delete m_pGraph;
        m_pGraph = nullptr;
    }
}


void Panel3TestDlg::setupLayout()
{
    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        m_pHSplitter = new QSplitter;
        {
            m_pHSplitter->setChildrenCollapsible(false);

            QFrame *pFrame = makeControls();
            m_pgl3dPanelView = new gl3dPanelField;
            m_pHSplitter->addWidget(pFrame);
            m_pHSplitter->addWidget(m_pGraphWt);
            m_pHSplitter->addWidget(m_pgl3dPanelView);
        }
        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);
}


QFrame* Panel3TestDlg::makeControls()
{
    QFrame *pFrame = new QFrame(this);
    {
        QPalette palette;
        palette.setColor(QPalette::Text, DisplayOptions::textColor());
        palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
        palette.setColor(QPalette::Window, QColor(125,125,125,65));
        pFrame->setCursor(Qt::ArrowCursor);
        pFrame->setPalette(palette);
        pFrame->setAutoFillBackground(false);

        QVBoxLayout * pFrameLayout =new QVBoxLayout;
        {
            QGroupBox *pTypeBox = new QGroupBox("Panel type");
            {
                QHBoxLayout *pTypeLayout = new QHBoxLayout;
                {
                    m_prbP3 = new QRadioButton("Triangles");
                    m_prbP4 = new QRadioButton("Quads");
                    pTypeLayout->addStretch();
                    pTypeLayout->addWidget(m_prbP3);
                    pTypeLayout->addStretch();
                    pTypeLayout->addWidget(m_prbP4);
                    pTypeLayout->addStretch();

                    m_prbP3->setChecked(s_bP3);
                    m_prbP4->setChecked(!s_bP3);
                }
                pTypeBox->setLayout(pTypeLayout);
            }

            QGroupBox *pPanelsBox = new QGroupBox("Panels");
            {
                QFormLayout * pPanelBoxLayout = new QFormLayout;
                {
                    m_pchSinglePanel = new QCheckBox("One panel only");
                    m_pchSinglePanel->setChecked(s_b1TriPanel);

                    m_pieNXPanels = new IntEdit(s_NXPanels);
                    m_pieNYPanels = new IntEdit(s_NYPanels);

                    m_pcbXDistrib = new QComboBox;
                    m_pcbYDistrib = new QComboBox;
                    QStringList items({"UNIFORM","COSINE", "SINE", "INV_SINE","TANH","EXP","INV_EXP"});
                    m_pcbXDistrib->addItems(items);
                    m_pcbYDistrib->addItems(items);
                    switch(s_XDistrib)
                    {
                        default:
                        case xfl::UNIFORM:  m_pcbXDistrib->setCurrentIndex(0);   break;
                        case xfl::COSINE:   m_pcbXDistrib->setCurrentIndex(1);   break;
                        case xfl::SINE:     m_pcbXDistrib->setCurrentIndex(2);   break;
                        case xfl::INV_SINE: m_pcbXDistrib->setCurrentIndex(3);   break;
                        case xfl::TANH:     m_pcbXDistrib->setCurrentIndex(4);   break;
                        case xfl::EXP:      m_pcbXDistrib->setCurrentIndex(5);   break;
                        case xfl::INV_EXP:  m_pcbXDistrib->setCurrentIndex(6);   break;
                    }
                    switch(s_YDistrib)
                    {
                        default:
                        case xfl::UNIFORM:  m_pcbYDistrib->setCurrentIndex(0);   break;
                        case xfl::COSINE:   m_pcbYDistrib->setCurrentIndex(1);   break;
                        case xfl::SINE:     m_pcbYDistrib->setCurrentIndex(2);   break;
                        case xfl::INV_SINE: m_pcbYDistrib->setCurrentIndex(3);   break;
                        case xfl::TANH:     m_pcbYDistrib->setCurrentIndex(4);   break;
                        case xfl::EXP:      m_pcbYDistrib->setCurrentIndex(5);   break;
                        case xfl::INV_EXP:  m_pcbYDistrib->setCurrentIndex(6);   break;
                    }

                    pPanelBoxLayout->addRow("", m_pchSinglePanel);
                    pPanelBoxLayout->addRow("NX rows=", m_pieNXPanels);
                    pPanelBoxLayout->addRow("NY cols=", m_pieNYPanels);
                    pPanelBoxLayout->addRow("XDistrib:",  m_pcbXDistrib);
                    pPanelBoxLayout->addRow("YDistrib:",  m_pcbYDistrib);
                }
                pPanelsBox->setLayout(pPanelBoxLayout);
            }

            QGroupBox *pSingBox = new QGroupBox("Singularity");
            {
                QGridLayout*pSingLayout = new QGridLayout;
                {
                    m_prbSource   = new QRadioButton("Source");
                    m_prbDoublet  = new QRadioButton("Doublet");
                    m_prbSource->setChecked(s_bSource);
                    m_prbDoublet->setChecked(!s_bSource);

                    QLabel *pLabVortexModel = new QLabel("Vortex model");
                    m_pcbVortexModel = new QComboBox;
                    QStringList items;
                    items << "POTENTIAL" << "CUT_OFF" << "LAMB_OSEEN" << "RANKINE" << "SCULLY" << "VATISTAS";
                    m_pcbVortexModel->addItems(items);
                    m_pcbVortexModel->setCurrentIndex(s_VortexModel);

                    m_pdeCoreRadius = new FloatEdit(s_CoreRadius);
                    pSingLayout->addWidget(m_prbSource,                1,1);
                    pSingLayout->addWidget(m_prbDoublet,               1,2);
                    pSingLayout->addWidget(new QLabel("Core radius="), 2,1, Qt::AlignVCenter | Qt::AlignRight);
                    pSingLayout->addWidget(m_pdeCoreRadius,            2,2);
                    pSingLayout->addWidget(pLabVortexModel,            3,1);
                    pSingLayout->addWidget(m_pcbVortexModel,           3,2);
                    pSingLayout->setColumnStretch(3,1);
                }
                pSingBox->setLayout(pSingLayout);
            }

            QGroupBox *pMethodBox = new QGroupBox("Method");
            {
                QHBoxLayout *pMethodLayout = new QHBoxLayout;
                {
                    m_prbBasis  = new QRadioButton("Basis");
                    m_prbN4023  = new QRadioButton("N4023");
                    m_prbVortex = new QRadioButton("Vortex");
                    m_prbBasis->setChecked(s_Method==BASIS);
                    m_prbN4023->setChecked(s_Method==NASA4023);
                    m_prbVortex->setChecked(s_Method==VORTEX);
                    pMethodLayout->addStretch();
                    pMethodLayout->addWidget(m_prbN4023);
                    pMethodLayout->addWidget(m_prbBasis);
                    pMethodLayout->addWidget(m_prbVortex);
                    pMethodLayout->addStretch();
                }
                pMethodBox->setLayout(pMethodLayout);
            }

            QGroupBox *pVarBox = new QGroupBox("Variable");
            {
                QGridLayout *pVarLayout = new QGridLayout;
                {
                    m_prbPotential = new QRadioButton("Potential");
                    m_prbPotential->setChecked(s_bPotential);
                    m_prbVelocity  = new QRadioButton("Velocity");
                    m_prbVelocity->setChecked(!s_bPotential);
                    m_pcbVcomp = new QComboBox;
                    m_pcbVcomp->addItems({"Vn", "|V|", "Vx", "Vy", "Vz"});
                    m_pcbVcomp->setCurrentIndex(s_VComp);
                    pVarLayout->addWidget(m_prbPotential, 1 ,1);
                    pVarLayout->addWidget(m_prbVelocity,  2, 1);
                    pVarLayout->addWidget(m_pcbVcomp,     2, 2);
                    pVarLayout->setColumnStretch(3,1);

                }
                pVarBox->setLayout(pVarLayout);
            }

            QGroupBox *pGraphBox = new QGroupBox("Curve");
            {

                QGridLayout *pGraphCtrlsLayout= new QGridLayout;
                {
                    m_pdeX0 = new FloatEdit(s_X0);
                    m_pdeY0 = new FloatEdit(s_Y0);
                    m_pdeZ0 = new FloatEdit(s_Z0);
                    m_pdeX1 = new FloatEdit(s_X1);
                    m_pdeY1 = new FloatEdit(s_Y1);
                    m_pdeZ1 = new FloatEdit(s_Z1);

                    m_pieCurvePts = new IntEdit(s_nCurvePts);

                    m_pLineBtn = new LineBtn;
                    m_pLineBtn->setTheStyle(s_LS);
                    pGraphCtrlsLayout->addWidget(new QLabel("X0="), 1,1, Qt::AlignRight);
                    pGraphCtrlsLayout->addWidget(new QLabel("Y0="), 2,1, Qt::AlignRight);
                    pGraphCtrlsLayout->addWidget(new QLabel("Z0="), 3,1, Qt::AlignRight);
                    pGraphCtrlsLayout->addWidget(new QLabel("X1="), 1,4, Qt::AlignRight);
                    pGraphCtrlsLayout->addWidget(new QLabel("Y1="), 2,4, Qt::AlignRight);
                    pGraphCtrlsLayout->addWidget(new QLabel("Z1="), 3,4, Qt::AlignRight);
                    pGraphCtrlsLayout->addWidget(m_pdeX0,1,2);
                    pGraphCtrlsLayout->addWidget(m_pdeY0,2,2);
                    pGraphCtrlsLayout->addWidget(m_pdeZ0,3,2);
                    pGraphCtrlsLayout->addWidget(m_pdeX1,1,5);
                    pGraphCtrlsLayout->addWidget(m_pdeY1,2,5);
                    pGraphCtrlsLayout->addWidget(m_pdeZ1,3,5);
                    pGraphCtrlsLayout->addWidget(new QLabel("Curve size="), 5, 1, Qt::AlignRight);
                    pGraphCtrlsLayout->addWidget(m_pieCurvePts,5,2,1,2);
                    pGraphCtrlsLayout->addWidget(m_pLineBtn,5,4,1,3);
                    pGraphCtrlsLayout->setColumnStretch(3,1);
                    pGraphCtrlsLayout->setColumnStretch(6,1);
                }

                pGraphBox->setLayout(pGraphCtrlsLayout);
            }

            m_ppto = new PlainTextOutput;
            m_ppto->setReadOnly(false);
            pFrameLayout->addWidget(pTypeBox);
            pFrameLayout->addWidget(pPanelsBox);
            pFrameLayout->addWidget(pSingBox);
            pFrameLayout->addWidget(pMethodBox);
            pFrameLayout->addWidget(pVarBox);
            pFrameLayout->addWidget(pGraphBox);
            pFrameLayout->addWidget(m_ppto);
        }
        pFrame->setLayout(pFrameLayout);
    }
    return pFrame;
}


void Panel3TestDlg::connectSignals()
{
    connect(m_prbP3,          SIGNAL(clicked(bool)),             SLOT(onMakeViews()));
    connect(m_prbP4,          SIGNAL(clicked(bool)),             SLOT(onMakeViews()));

    connect(m_pchSinglePanel, SIGNAL(clicked(bool)),             SLOT(onMakeViews()));

    connect(m_pcbXDistrib,    SIGNAL(currentIndexChanged(int)),  SLOT(onMakeViews()));
    connect(m_pcbYDistrib,    SIGNAL(currentIndexChanged(int)),  SLOT(onMakeViews()));
    connect(m_pieNXPanels,    SIGNAL(intChanged(int)),            SLOT(onMakeViews()));
    connect(m_pieNYPanels,    SIGNAL(intChanged(int)),            SLOT(onMakeViews()));
    connect(m_pcbVortexModel, SIGNAL(currentIndexChanged(int)),  SLOT(onMakeViews()));
    connect(m_pdeCoreRadius,  SIGNAL(floatChanged(float)),            SLOT(onMakeViews()));
    connect(m_prbSource,      SIGNAL(clicked(bool)),             SLOT(onMakeViews()));
    connect(m_prbDoublet,     SIGNAL(clicked(bool)),             SLOT(onMakeViews()));
    connect(m_prbVelocity,    SIGNAL(clicked(bool)),             SLOT(onMakeViews()));
    connect(m_prbPotential,   SIGNAL(clicked(bool)),             SLOT(onMakeViews()));
    connect(m_pcbVcomp,       SIGNAL(activated(int)),            SLOT(onMakeViews()));

    connect(m_prbBasis,       SIGNAL(clicked(bool)),             SLOT(onMakeViews()));
    connect(m_prbN4023,       SIGNAL(clicked(bool)),             SLOT(onMakeViews()));
    connect(m_prbVortex,      SIGNAL(clicked(bool)),             SLOT(onMakeViews()));


    connect(m_pdeX0,          SIGNAL(floatChanged(float)),            SLOT(onMakeViews()));
    connect(m_pdeY0,          SIGNAL(floatChanged(float)),            SLOT(onMakeViews()));
    connect(m_pdeZ0,          SIGNAL(floatChanged(float)),            SLOT(onMakeViews()));
    connect(m_pdeX1,          SIGNAL(floatChanged(float)),            SLOT(onMakeViews()));
    connect(m_pdeY1,          SIGNAL(floatChanged(float)),            SLOT(onMakeViews()));
    connect(m_pdeZ1,          SIGNAL(floatChanged(float)),            SLOT(onMakeViews()));
    connect(m_pieCurvePts,    SIGNAL(intChanged(int)),            SLOT(onMakeViews()));
    connect(m_pLineBtn,       SIGNAL(clickedLB(LineStyle)), SLOT(onCurveStyle(LineStyle)));
}


void Panel3TestDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
            QTimer::singleShot(0, this, SLOT(close()));
            break;
        case Qt::Key_W:
            if(bCtrl)
                QTimer::singleShot(0, this, SLOT(close()));
            break;
        default:
            break;
    }
    QWidget::keyPressEvent(pEvent);
}


void Panel3TestDlg::onCurveStyle(LineStyle ls)
{
    LineMenu *pLineMenu = new LineMenu(nullptr);
    pLineMenu->initMenu(ls);
    pLineMenu->exec(QCursor::pos());
    s_LS = pLineMenu->theStyle();
    m_pLineBtn->setTheStyle(s_LS);
    onMakeViews();
}


void Panel3TestDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);

    onMakeViews();
}


void Panel3TestDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    readData();
    s_Geometry = saveGeometry();
    s_HSplitterSizes  = m_pHSplitter->saveState();
}


void Panel3TestDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("Panel3TestWt");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();
        s_HSplitterSizes = settings.value("HSplitSize").toByteArray();

        s_bP3     = settings.value("Triangles", s_bP3).toBool();
        s_b1TriPanel = settings.value("OnePanel", s_b1TriPanel).toBool();

        int idist = 0;
        idist = settings.value("XDistrib", 0).toInt();
        if     (idist==1)  s_XDistrib = xfl::COSINE;
        else if(idist==2)  s_XDistrib = xfl::SINE;
        else if(idist==-2) s_XDistrib = xfl::INV_SINE;
        else if(idist==3)  s_XDistrib = xfl::INV_SINH;
        else if(idist==4)  s_XDistrib = xfl::TANH;
        else if(idist==5)  s_XDistrib = xfl::EXP;
        else if(idist==6)  s_XDistrib = xfl::INV_EXP;
        else               s_XDistrib = xfl::UNIFORM;
        idist = settings.value("YDistrib", 0).toInt();
        if     (idist==1)  s_YDistrib = xfl::COSINE;
        else if(idist==2)  s_YDistrib = xfl::SINE;
        else if(idist==-2) s_YDistrib = xfl::INV_SINE;
        else if(idist==3)  s_YDistrib = xfl::INV_SINH;
        else if(idist==4)  s_YDistrib = xfl::TANH;
        else if(idist==5)  s_YDistrib = xfl::EXP;
        else if(idist==6)  s_YDistrib = xfl::INV_EXP;
        else               s_YDistrib = xfl::UNIFORM;


        s_X0 = settings.value("X0",  0.0).toDouble();
        s_Y0 = settings.value("Y0",  0.0).toDouble();
        s_Z0 = settings.value("Z0", -1.0).toDouble();
        s_X1 = settings.value("X1",  0.0).toDouble();
        s_Y1 = settings.value("Y1",  0.0).toDouble();
        s_Z1 = settings.value("Z1",  1.0).toDouble();

        switch (settings.value("VortexModel", Vortex::vortexModel()).toInt())
        {
            default:
            case 0: s_VortexModel = Vortex::POTENTIAL;  break;
            case 1: s_VortexModel = Vortex::CUT_OFF;    break;
            case 2: s_VortexModel = Vortex::LAMB_OSEEN; break;
            case 3: s_VortexModel = Vortex::RANKINE;    break;
            case 4: s_VortexModel = Vortex::SCULLY;     break;
            case 5: s_VortexModel = Vortex::VATISTAS;   break;

        }

        s_CoreRadius = settings.value("CoreRadius", s_CoreRadius).toDouble();

        s_NXPanels   = settings.value("NXPanels",    s_NXPanels).toInt();
        s_NYPanels   = settings.value("NYPanels",    s_NYPanels).toInt();
        s_nCurvePts  = settings.value("CurvePts",    s_nCurvePts).toInt();
        s_Side       = settings.value("Side",        s_Side).toDouble();
        s_bSource    = settings.value("bSource",     s_bSource).toBool();
        s_bPotential = settings.value("bPotential",  s_bPotential).toBool();
        s_VComp      = settings.value("VComp",       s_VComp).toDouble();
        xfl::loadLineSettings(settings, s_LS, "CurveStyle");
     }
    settings.endGroup();
}


/** Saves the static variables to the specified settings file.
 * @param settings a reference to the QSettings object*/
void Panel3TestDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("Panel3TestWt");
    {
        settings.setValue("WindowGeometry", s_Geometry);
        settings.setValue("HSplitSize",     s_HSplitterSizes);

        settings.setValue("Triangles", s_bP3);
        settings.setValue("OnePanel", s_b1TriPanel);

        int idist = 0;
        switch(s_XDistrib)
        {
            case xfl::COSINE:      idist = 1;  break;
            case xfl::SINE:        idist = 2;  break;
            case xfl::INV_SINE:    idist =-2;  break;
            case xfl::INV_SINH:    idist = 3;  break;
            case xfl::TANH:        idist = 4;  break;
            case xfl::EXP:         idist = 5;  break;
            case xfl::INV_EXP:     idist = 6;  break;
            case xfl::UNIFORM:
            default:               idist = 0;  break;
        }
        settings.setValue("XDistrib",   idist);

        switch(s_YDistrib)
        {
            case xfl::COSINE:      idist = 1;  break;
            case xfl::SINE:        idist = 2;  break;
            case xfl::INV_SINE:    idist =-2;  break;
            case xfl::INV_SINH:    idist = 3;  break;
            case xfl::TANH:        idist = 4;  break;
            case xfl::EXP:         idist = 5;  break;
            case xfl::INV_EXP:     idist = 6;  break;
            case xfl::UNIFORM:
            default:               idist = 0;  break;
        }
        settings.setValue("YDistrib",   idist);


        settings.setValue("VortexModel", s_VortexModel);
        settings.setValue("CoreRadius", s_CoreRadius);

        settings.setValue("NXPanels",   s_NXPanels);
        settings.setValue("NYPanels",   s_NYPanels);
        settings.setValue("Side",       s_Side);
        settings.setValue("bSource",    s_bSource);
        settings.setValue("bPotential", s_bPotential);
        settings.setValue("X0", s_X0);
        settings.setValue("Y0", s_Y0);
        settings.setValue("Z0", s_Z0);
        settings.setValue("X1", s_X1);
        settings.setValue("Y1", s_Y1);
        settings.setValue("Z1", s_Z1);
        settings.setValue("CurvePts",   s_nCurvePts);
        settings.setValue("VComp",      s_VComp);
        xfl::saveLineSettings(settings, s_LS, "CurveStyle");
     }
    settings.endGroup();
}


void Panel3TestDlg::onMakeViews()
{
    readData();
    makePanels();
    setControls();
    makeGraph();

    m_pgl3dPanelView->clearPanels();
    if(s_bP3)  m_pgl3dPanelView->setPanel3(m_Panel3);
    else       m_pgl3dPanelView->setPanel4(m_Panel4);
    m_pgl3dPanelView->setSource(s_bSource);
    m_pgl3dPanelView->setCore(s_VortexModel, s_CoreRadius);
    m_pgl3dPanelView->setPlotLine({s_X0, s_Y0, s_Z0}, {s_X1, s_Y1, s_Z1});
    m_pgl3dPanelView->setMethod(s_Method);
    m_pgl3dPanelView->onMakeView();
}


void Panel3TestDlg::makePanels()
{
    m_Panel3.clear();
    m_Panel4.clear();


    if(s_b1TriPanel && s_bP3)
    {
        m_Panel3.push_back({{0.0,-0.5,0.0}, {1.0,-0.5,0.0}, {0.0,0.5,0.0}});
    }

/*    if(s_NXPanels == 1)
    {
        double theta = PI/3.0;
        double cost = cos(theta);
        double sint = sin(theta);
        Vector3d A{cost,sint,0};
        Vector3d B{-1,0,0};
        Vector3d C{cost,-sint,0};
        Vector3d D;

        if(s_bP3)
        {
//            m_Panel3.append({A,B,C});
         }
        else
        {
 //           Panel4 p4(A,C,A,B);
 //           m_Panel4.append(p4);
            m_Panel4.append({{-1,1,0}, {1,1,0}, {-1,-1,0}, {1,-1,0}});
        }
    }*/
    else
    {
        double side = 1.0;
        Node LA, LB, TA, TB;

        if(s_NXPanels<=0) s_NXPanels=1;
        if(s_NYPanels<=0) s_NYPanels=1;

        std::vector<double> xfrac, yfrac;
        xfl::getPointDistribution(xfrac, s_NXPanels, s_XDistrib);
        xfl::getPointDistribution(yfrac, s_NYPanels, s_YDistrib);

        for(uint iy=0; iy<yfrac.size()-1; iy++)
        {
            double w2 = side * double(s_NYPanels);
            double yl = (yfrac.at(iy)  -0.5)*w2;
            double yr = (yfrac.at(iy+1)-0.5)*w2;

            for(uint ix=0; ix<xfrac.size()-1; ix++)
            {
                double di  = xfrac.at(ix)   * double(s_NXPanels) * side;
                double di1 = xfrac.at(ix+1) * double(s_NXPanels) * side;
                LA.set( di*side, yl, 0);
                LB.set( di*side, yr, 0);
                TA.set(di1*side, yl, 0);
                TB.set(di1*side, yr, 0);
                LA.setNormal(0,0,1.0);
                LB.setNormal(0,0,1.0);
                TA.setNormal(0,0,1.0);
                TB.setNormal(0,0,1.0);

                if(s_bP3)
                {
                    m_Panel3.push_back({LB, LA, TA});
                    m_Panel3.push_back({LB, TA, TB});
                }
                else
                {
                    m_Panel4.push_back({LA, LB, TA, TB});
                }
            }
        }
    }
}


void Panel3TestDlg::readData()
{
    s_bP3 = m_prbP3->isChecked();
    s_b1TriPanel = m_pchSinglePanel->isChecked();
    m_prbBasis->setEnabled(s_bP3);
    if(!s_bP3)
    {
        if(m_prbBasis->isChecked()) m_prbN4023->setChecked(true);
    }

    s_XDistrib = xfl::distributionType(m_pcbXDistrib->currentText().toStdString());
    s_YDistrib = xfl::distributionType(m_pcbYDistrib->currentText().toStdString());

    s_NXPanels = m_pieNXPanels->value();
    s_NYPanels = m_pieNYPanels->value();

    s_bSource    = m_prbSource->isChecked();
    s_CoreRadius = m_pdeCoreRadius->value();

    switch(m_pcbVortexModel->currentIndex())
    {
        default:
        case 0: s_VortexModel = Vortex::POTENTIAL;  break;
        case 1: s_VortexModel = Vortex::CUT_OFF;    break;
        case 2: s_VortexModel = Vortex::LAMB_OSEEN; break;
        case 3: s_VortexModel = Vortex::RANKINE;    break;
        case 4: s_VortexModel = Vortex::SCULLY;     break;
        case 5: s_VortexModel = Vortex::VATISTAS;   break;
    }

    s_VComp      = m_pcbVcomp->currentIndex();
    s_bPotential = m_prbPotential->isChecked();

    s_X0 = m_pdeX0->value();
    s_Y0 = m_pdeY0->value();
    s_Z0 = m_pdeZ0->value();
    s_X1 = m_pdeX1->value();
    s_Y1 = m_pdeY1->value();
    s_Z1 = m_pdeZ1->value();
    s_nCurvePts = m_pieCurvePts->value();

    if     (m_prbBasis->isChecked())  s_Method=BASIS;
    else if(m_prbN4023->isChecked())  s_Method=NASA4023;
    else if(m_prbVortex->isChecked()) s_Method=VORTEX;
    else s_Method = NOPANELMETHOD;
}


/** Enables/disables controls following user input */
void Panel3TestDlg::setControls()
{
    if(m_prbSource->isChecked())
    {
        if(m_prbVortex->isChecked())
        {
            m_prbN4023->setChecked(true);
            s_Method=NASA4023;
        }
        m_prbVortex->setEnabled(false);
    }
    else
    {
        m_prbVortex->setEnabled(true);
    }
    m_pcbVcomp->setEnabled(!s_bPotential);
}


void Panel3TestDlg::makeGraph()
{
    m_pGraph->setXVariableList({"z"});
    m_pGraph->setYVariableList({"phi", "V"});

    m_pGraph->setYVariable(1,1);
    m_pGraph->deleteCurves();
    Curve *pCurvePhi = nullptr;
    Curve *pCurveV   = nullptr;


    if(m_prbPotential->isChecked())
    {

        pCurvePhi = m_pGraph->addCurve();
        if(s_Method==BASIS)            pCurvePhi->setName("phi_basis");
        else if(s_Method==NASA4023)    pCurvePhi->setName("phi_N4023");
        pCurvePhi->setTheStyle(s_LS);
        m_pGraph->setYVariable(0,0);
    }
    else if(m_prbVelocity->isChecked())
    {
        pCurveV = m_pGraph->addCurve();
        if     (s_Method==BASIS)         pCurveV->setName("V_basis");
        else if(s_Method==NASA4023)      pCurveV->setName("V_N4023");
        else if(s_Method==VORTEX)        pCurveV->setName("V_Vortex");
        pCurveV->setTheStyle(s_LS);
        m_pGraph->setYVariable(0,1);
    }

    double phi(0);

    Vector3d pt;
    Vector3d V;

    Vector3d P0(s_X0, s_Y0, s_Z0);
    Vector3d P1(s_X1, s_Y1, s_Z1);

    double linelength = P0.distanceTo(P1);

    // plot the curves in the panel's local reference frame;
    for(int i=0; i<s_nCurvePts; i++)
    {
        double t = double(i)/double(s_nCurvePts-1);
        Vector3d pt = P0 + (P1-P0)*t;

        if(pCurvePhi)
        {
            if(m_Panel3.size())
                phi = potentialP3(pt, m_Panel3, s_Method, s_bSource, s_CoreRadius);
            else
                phi = potentialP4(pt, m_Panel4, s_Method, s_bSource, s_CoreRadius);

            pCurvePhi->appendPoint(t, phi);
            if(i==0)
            {
                QString strange = QString::asprintf("   x=%7g, y=%7g, z=%7g:", pt.x, pt.y, pt.z);
                if(m_Panel3.size())  strange += QString::asprintf("   phi3 = %11g\n", phi);
                else                 strange += QString::asprintf("   phi4 = %11g\n", phi);
                m_ppto->onAppendQText(strange);
            }
        }

        if(pCurveV)
        {
            if(m_Panel3.size())
                V = velocityP3(pt, m_Panel3, s_Method, s_bSource, s_CoreRadius, s_VortexModel);
            else
                V = velocityP4(pt, m_Panel4, s_Method, s_bSource, s_CoreRadius);
            if     (s_VComp==0) pCurveV->appendPoint(t*linelength, V.dot(m_Panel3.front().normal()));
            else if(s_VComp==1) pCurveV->appendPoint(t*linelength, V.norm());
            else if(s_VComp==2) pCurveV->appendPoint(t*linelength, V.x);
            else if(s_VComp==3) pCurveV->appendPoint(t*linelength, V.y);
            else if(s_VComp==4) pCurveV->appendPoint(t*linelength, V.z);
        }
    }
    m_pGraph->resetLimits();
    m_pGraph->invalidate();


    update();
}




