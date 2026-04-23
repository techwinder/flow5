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


#include <QFormLayout>

#include "vortontestdlg.h"

#include <test/test3d/gl3dvortonfield.h>
#include <interfaces/graphs/containers/graphwt.h>
#include <interfaces/graphs/graph/graph.h>
#include <interfaces/graphs/controls/graphoptions.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <api/planeopp.h>

int VortonTestDlg::s_nXPanels=5, VortonTestDlg::s_nYPanels=3;
double VortonTestDlg::s_Width = 3.0;
double VortonTestDlg::s_Length = 5.0;
int VortonTestDlg::s_nVtnSide = 5; // Number of vortons/side

bool VortonTestDlg::s_bGradV = false;
int VortonTestDlg::s_iGradDir = 0;
double VortonTestDlg::s_X0=  2.5;
double VortonTestDlg::s_Y0= -3.0;
double VortonTestDlg::s_Z0=  0.0;

double VortonTestDlg::s_X1= 2.5;
double VortonTestDlg::s_Y1= 3.0;
double VortonTestDlg::s_Z1= 0.0;

int VortonTestDlg::s_nCurvePts = 100;
int VortonTestDlg::s_iVPlotDir = 0; // the velocity component to plot in the graph
int VortonTestDlg::s_iVelSrc = 0;

QByteArray VortonTestDlg::s_Geometry;
QByteArray VortonTestDlg::s_HSplitterSizes;
Quaternion VortonTestDlg::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);

VortonTestDlg::VortonTestDlg(PlaneOpp const *pPOpp) : QDialog()
{
    setWindowTitle("Equivalent vorton field");

    setupLayout();

    if(pPOpp)
    {
        for(int ip=0; ip<pPOpp->vortonRows(); ip++)
            m_Vortons.insert(m_Vortons.end(), pPOpp->m_Vorton.at(ip).begin(), pPOpp->m_Vorton.at(ip).end());
    }
    else
    {
        onMakePanels();
    }

    onRecalc();
}


VortonTestDlg::~VortonTestDlg()
{
    if(m_pGraphWt && m_pGraphWt->graph())
    {
        m_pGraphWt->graph()->deleteCurveModel();
        delete m_pGraphWt->graph();
    }
}


void VortonTestDlg::setupLayout()
{
    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        m_pHSplitter = new QSplitter;
        {
            m_pHSplitter->setOrientation(Qt::Horizontal);
            m_pHSplitter->setChildrenCollapsible(false);
            m_pglVortonField = new gl3dVortonField;

            m_pGraphWt = new GraphWt(this);
            {
                m_pGraphWt->showLegend(true);
                Graph *pGraph = new Graph;
                m_pGraphWt->setGraph(pGraph);
                pGraph->setCurveModel(new CurveModel);
                GraphOptions::resetGraphSettings(*pGraph);
                pGraph->setLegendVisible(true);
                pGraph->setLegendPosition(Qt::AlignHCenter | Qt::AlignTop);
                pGraph->setScaleType(GRAPH::RESETTING);
                pGraph->setAuto(true);
            }

            QFrame *pCtrlFrame = makeControls();
            connectSignals();


            m_pHSplitter->addWidget(pCtrlFrame);
            m_pHSplitter->addWidget(m_pGraphWt);
            m_pHSplitter->addWidget(m_pglVortonField);
        }
        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);
}


QFrame *VortonTestDlg::makeControls()
{
    QFrame *pFrame = new QFrame;
    {
        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            QGroupBox *pPanelBox = new QGroupBox("Panels and vortons");
            {
                QFormLayout *pVtnLayout = new QFormLayout;
                {
                    m_pdeWidth    = new FloatEdit(s_Width);
                    m_pdeLength   = new FloatEdit(s_Length);
                    m_pieNXPanels = new IntEdit(s_nXPanels);
                    m_pieNYPanels = new IntEdit(s_nYPanels);
                    m_pieVtnSide = new IntEdit(s_nVtnSide);
                    m_pdeVtnCoreSize = new FloatEdit(0.1f);
                    pVtnLayout->addRow("x-Length",         m_pdeLength);
                    pVtnLayout->addRow("y-Width",          m_pdeWidth);
                    pVtnLayout->addRow("Nx Panels",        m_pieNXPanels);
                    pVtnLayout->addRow("Ny Panels",        m_pieNYPanels);
                    pVtnLayout->addRow("Vortons/side",     m_pieVtnSide);
                    pVtnLayout->addRow("Vorton core size", m_pdeVtnCoreSize);
                }
                pPanelBox->setLayout(pVtnLayout);
            }

            QGroupBox *pGraphBox = new QGroupBox("Graph");
            {
                QVBoxLayout *pGraphLayout = new QVBoxLayout;
                {
                    QFormLayout *pGraphCtrlsLayout= new QFormLayout;
                    {
                        m_pdeX0 = new FloatEdit(s_X0);
                        m_pdeY0 = new FloatEdit(s_Y0);
                        m_pdeZ0 = new FloatEdit(s_Z0);
                        m_pdeX1 = new FloatEdit(s_X1);
                        m_pdeY1 = new FloatEdit(s_Y1);
                        m_pdeZ1 = new FloatEdit(s_Z1);

                        m_pcbVPlotDir = new QComboBox;
                        m_pcbVPlotDir->addItems({"X", "Y", "Z", "Norm"});
                        m_pcbVPlotDir->setCurrentIndex(s_iVPlotDir%4);

                        m_pieCurvePts = new IntEdit(s_nCurvePts);
                        pGraphCtrlsLayout->addRow("X0", m_pdeX0);
                        pGraphCtrlsLayout->addRow("Y0", m_pdeY0);
                        pGraphCtrlsLayout->addRow("Z0", m_pdeZ0);
                        pGraphCtrlsLayout->addRow("X1", m_pdeX1);
                        pGraphCtrlsLayout->addRow("Y1", m_pdeY1);
                        pGraphCtrlsLayout->addRow("Z1", m_pdeZ1);
                        pGraphCtrlsLayout->addRow("VDir", m_pcbVPlotDir);
                        pGraphCtrlsLayout->addRow("n graph pts", m_pieCurvePts);
                    }
                    QGroupBox *pVarSelBox = new QGroupBox("Variable");
                    {
                        QHBoxLayout *pVarSelLayout = new QHBoxLayout;
                        {
                            m_prbV     = new QRadioButton("V");
                            m_prbGradV = new QRadioButton("dV/");
                            m_prbV->setChecked(!s_bGradV);
                            m_prbGradV->setChecked(s_bGradV);

                            m_pcbGradDir = new QComboBox;
                            m_pcbGradDir->addItems({"dx","dy","dz"});
                            m_pcbGradDir->setCurrentIndex(s_iGradDir%3);
                            m_pcbGradDir->setEnabled(s_bGradV);

                            pVarSelLayout->addWidget(m_prbV);
                            pVarSelLayout->addStretch();
                            pVarSelLayout->addWidget(m_prbGradV);
                            pVarSelLayout->addWidget(m_pcbGradDir);
                        }
                        pVarSelBox->setLayout(pVarSelLayout);
                    }

                    pGraphLayout->addWidget(pVarSelBox);
                    pGraphLayout->addLayout(pGraphCtrlsLayout);
                }
                pGraphBox->setLayout(pGraphLayout);
            }

            QGroupBox *p3dBox = new QGroupBox("3d view");
            {
                QFormLayout *p3dCtrlsLayout= new QFormLayout;
                {
                    QLabel *pTitleLab = new QLabel("Velocity from:");
                    m_prbVelSrc[0] = new QRadioButton("Panels");
                    m_prbVelSrc[1] = new QRadioButton("Vortons");

                    m_prbVelSrc[0]->setChecked(s_iVelSrc==0);
                    m_prbVelSrc[1]->setChecked(s_iVelSrc==1);

                    p3dCtrlsLayout->addRow(pTitleLab);
                    p3dCtrlsLayout->addRow(m_prbVelSrc[0]);
                    p3dCtrlsLayout->addRow(m_prbVelSrc[1]);
                }
                p3dBox->setLayout(p3dCtrlsLayout);
            }

            QPushButton *pRecalBtn = new QPushButton("Recalc.");
            connect(pRecalBtn, SIGNAL(clicked()), SLOT(onRecalc()));

            pFrameLayout->addWidget(pPanelBox);
            pFrameLayout->addWidget(pGraphBox);
            pFrameLayout->addWidget(p3dBox);
            pFrameLayout->addWidget(pRecalBtn);
            pFrameLayout->addStretch();
        }
        pFrame->setLayout(pFrameLayout);
    }
    return pFrame;
}


void VortonTestDlg::connectSignals()
{
    connect(m_pdeWidth,     SIGNAL(floatChanged(float)), SLOT(onMakePanels()));
    connect(m_pdeLength,    SIGNAL(floatChanged(float)), SLOT(onMakePanels()));
    connect(m_pieNXPanels,  SIGNAL(intChanged(int)),       SLOT(onMakePanels()));
    connect(m_pieNYPanels,  SIGNAL(intChanged(int)),       SLOT(onMakePanels()));
    connect(m_pieVtnSide,   SIGNAL(intChanged(int)),       SLOT(onMakePanels()));

    connect(m_pdeVtnCoreSize, SIGNAL(floatChanged(float)), SLOT(onRecalc()));

    connect(m_pdeX0,        SIGNAL(floatChanged(float)), SLOT(onRecalc()));
    connect(m_pdeY0,        SIGNAL(floatChanged(float)), SLOT(onRecalc()));
    connect(m_pdeZ0,        SIGNAL(floatChanged(float)), SLOT(onRecalc()));
    connect(m_pdeX1,        SIGNAL(floatChanged(float)), SLOT(onRecalc()));
    connect(m_pdeY1,        SIGNAL(floatChanged(float)), SLOT(onRecalc()));
    connect(m_pdeZ1,        SIGNAL(floatChanged(float)), SLOT(onRecalc()));
    connect(m_pieCurvePts,  SIGNAL(intChanged(int)), SLOT(onRecalc()));
    connect(m_pcbVPlotDir,  SIGNAL(activated(int)),  SLOT(onRecalc()));
    connect(m_pcbGradDir,   SIGNAL(activated(int)),  SLOT(onRecalc()));
    connect(m_prbV,         SIGNAL(clicked()),       SLOT(onRecalc()));
    connect(m_prbGradV,     SIGNAL(clicked()),       SLOT(onRecalc()));
    connect(m_prbVelSrc[0], SIGNAL(clicked()),       SLOT(onRecalc()));
    connect(m_prbVelSrc[1], SIGNAL(clicked()),       SLOT(onRecalc()));
}


void VortonTestDlg::readData()
{
    s_bGradV = m_prbGradV->isChecked();
    m_pcbGradDir->setEnabled(s_bGradV);
    s_iGradDir = m_pcbGradDir->currentIndex();

    s_X0 = m_pdeX0->value();
    s_Y0 = m_pdeY0->value();
    s_Z0 = m_pdeZ0->value();
    s_X1 = m_pdeX1->value();
    s_Y1 = m_pdeY1->value();
    s_Z1 = m_pdeZ1->value();

    s_iVPlotDir = m_pcbVPlotDir->currentIndex();
    s_nCurvePts = m_pieCurvePts->value();

    if     (m_prbVelSrc[0]->isChecked()) s_iVelSrc=0;
    else if(m_prbVelSrc[1]->isChecked()) s_iVelSrc=1;

    m_pglVortonField->setVortonCoreSize(m_pdeVtnCoreSize->value());
}


void VortonTestDlg::onRecalc()
{
    readData();

    Vector3d P0(s_X0, s_Y0, s_Z0);
    Vector3d P1(s_X1, s_Y1, s_Z1);

    m_pglVortonField->setPlotLine({P0, P1});

    m_pglVortonField->makePlotLineVelocities(s_iVelSrc, s_nCurvePts);
    m_pglVortonField->makeVorticityColorMap();

    makeGraph(P0, P1, s_nCurvePts);
    update();
    m_pglVortonField->update();
}


void VortonTestDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);

    switch(pEvent->key())
    {
        case Qt::Key_W:
        {
            if(bCtrl) close();
            break;
        }

        default:
            QDialog::keyPressEvent(pEvent);
    }
}


void VortonTestDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);

    restoreGeometry(s_Geometry);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    m_pglVortonField->restoreViewPoint(s_ab_quat);
}


void VortonTestDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    readData();

    s_Geometry = saveGeometry();
    s_HSplitterSizes  = m_pHSplitter->saveState();

    m_pglVortonField->saveViewPoint(s_ab_quat);
}


void VortonTestDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("VortonTest");
    {
        s_Geometry       = settings.value("WindowGeometry").toByteArray();
        s_HSplitterSizes = settings.value("HSplitSize").toByteArray();

        s_nXPanels = settings.value("NXPanels", s_nXPanels).toInt();
        s_nYPanels = settings.value("NYPanels", s_nYPanels).toInt();
        s_Length   = settings.value("XLength",  s_Length).toDouble();
        s_Width    = settings.value("YWidth",   s_Width).toDouble();

        s_bGradV = settings.value("GradV", s_bGradV).toBool();
        s_iGradDir = settings.value("GradDir", s_iGradDir).toInt();
        s_X0 = settings.value("X0",  s_X0).toDouble();
        s_Y0 = settings.value("Y0",  s_Y0).toDouble();
        s_Z0 = settings.value("Z0",  s_Z0).toDouble();
        s_X1 = settings.value("X1",  s_X1).toDouble();
        s_Y1 = settings.value("Y1",  s_Y1).toDouble();
        s_Z1 = settings.value("Z1",  s_Z1).toDouble();

        s_nCurvePts = settings.value("CurvePts",   s_nCurvePts).toInt();
        s_iVPlotDir = settings.value("VPlotDir",   s_iVPlotDir).toInt();
        s_nVtnSide  = settings.value("VortonSide", s_nVtnSide).toInt();
        s_iVelSrc   = settings.value("VelSrc",     s_iVelSrc).toInt();
    }
    settings.endGroup();
}


void VortonTestDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("VortonTest");
    {
        settings.setValue("WindowGeometry", s_Geometry);
        settings.setValue("HSplitSize",     s_HSplitterSizes);

        settings.setValue("NXPanels", s_nXPanels);
        settings.setValue("NYPanels", s_nYPanels);
        settings.setValue("XLength",  s_Length);
        settings.setValue("YWidth",   s_Width);

        settings.setValue("GradV",   s_bGradV);
        settings.setValue("GradDir", s_iGradDir);
        settings.setValue("X0", s_X0);
        settings.setValue("Y0", s_Y0);
        settings.setValue("Z0", s_Z0);
        settings.setValue("X1", s_X1);
        settings.setValue("Y1", s_Y1);
        settings.setValue("Z1", s_Z1);

        settings.setValue("CurvePts",   s_nCurvePts);
        settings.setValue("VPlotDir",   s_iVPlotDir);
        settings.setValue("VortonSide", s_nVtnSide);
        settings.setValue("VelSrc",     s_iVelSrc);
    }
    settings.endGroup();
}


void VortonTestDlg::onMakePanels()
{
    s_nXPanels = m_pieNXPanels->value();
    s_nYPanels = m_pieNYPanels->value();
    s_Length   = m_pdeLength->value();
    s_Width    = m_pdeWidth->value();
    s_nVtnSide = m_pieVtnSide->value();
    double xl = s_Length;
    double yl = s_Width/2.0;
    Node vertex[4];
    vertex[0].set( 0, -yl, 0);
    vertex[1].set(xl, -yl, 0);
    vertex[2].set(xl,  yl, 0);
    vertex[3].set( 0,  yl, 0);
    for(int i=0; i<4; i++) vertex[i].setNormal(0.0,0.0,1.0);

    makePanels(vertex);

    m_pglVortonField->setPanels(m_Panels);
    m_pglVortonField->setVortons(m_Vortons);

    onRecalc();
}


/** The vorticity at the edges is the jump in doublet density between two adjacent panels x4.PI */
void VortonTestDlg::makePanels(Node const *nd)
{
    Vector3d U;
    Node LA, LB, TA, TB;
    LA.setNormal(0.0,0.0,1.0);
    LB.setNormal(0.0,0.0,1.0);
    TA.setNormal(0.0,0.0,1.0);
    TB.setNormal(0.0,0.0,1.0);
    double xl = (nd[1].x-nd[0].x)/double(s_nXPanels);
    double yl = (nd[3].y-nd[0].y)/double(s_nYPanels);
    double gamx = -4.0*PI *xl/double(s_nVtnSide);
    double gamy = -4.0*PI *yl/double(s_nVtnSide);

    m_Panels.resize(s_nXPanels*s_nYPanels*2);
    m_Vortons.resize(s_nXPanels*s_nYPanels*4*s_nVtnSide);

    int iv=0;
    for(int ix=0; ix<s_nXPanels; ix++)
    {
        double dx  = double(ix)/double(s_nXPanels);
        double dx1 = double(ix+1)/double(s_nXPanels);

        LA.x = nd[0].x*(1.0-dx)  + nd[1].x*dx;
        TA.x = nd[0].x*(1.0-dx1) + nd[1].x*dx1;
        LB.x = nd[0].x*(1.0-dx)  + nd[1].x*dx;
        TB.x = nd[0].x*(1.0-dx1) + nd[1].x*dx1;

        for(int jy=0; jy<s_nYPanels; jy++)
        {
            double dy  = double(jy)/double(s_nYPanels);
            double dy1 = double(jy+1)/double(s_nYPanels);

            LA.y = nd[0].y*(1.0-dy)  + nd[3].y*dy;
            TA.y = nd[0].y*(1.0-dy)  + nd[3].y*dy;
            LB.y = nd[0].y*(1.0-dy1) + nd[3].y*dy1;
            TB.y = nd[0].y*(1.0-dy1) + nd[3].y*dy1;

            // make the panels
            m_Panels[ix*s_nYPanels*2 + jy*2  ].setFrame(LB, LA, TA);
            m_Panels[ix*s_nYPanels*2 + jy*2+1].setFrame(LB, TA, TB);

            //make the vortons
            U = (LA-TA).normalized();
            for(int k=0; k<s_nVtnSide; k++)
            {
                double tau = double(2*k+1)/double(s_nVtnSide*2);
                m_Vortons[iv++].setVorton(TA*(1.0-tau)+LA*tau, U, gamx);
            }
            U = (TA-TB).normalized();
            for(int k=0; k<s_nVtnSide; k++)
            {
//                double tau = double(k+1)/double(s_nVtnSide+1);
                double tau = double(2*k+1)/double(s_nVtnSide*2);
                m_Vortons[iv++].setVorton(TB*(1.0-tau)+TA*tau, U, gamy);
            }
            U = (TB-LB).normalized();
            for(int k=0; k<s_nVtnSide; k++)
            {
                double tau = double(2*k+1)/double(s_nVtnSide*2);
                m_Vortons[iv++].setVorton(LB*(1.0-tau)+TB*tau, U, gamx);
            }
            U = (LB-LA).normalized();
            for(int k=0; k<s_nVtnSide; k++)
            {
                double tau = double(2*k+1)/double(s_nVtnSide*2);
                m_Vortons[iv++].setVorton(LA*(1.0-tau)+LB*tau, U, gamy);
            }
        }
    }
    Q_ASSERT(iv==int(m_Vortons.size()));
}


void VortonTestDlg::makeGraph(Vector3d const &P0, Vector3d const &P1, int nPts)
{
    double coresize = m_pdeVtnCoreSize->value();

    Graph *pGraph = m_pGraphWt->graph();
    pGraph->deleteCurves();
    Curve *pCurvePan = pGraph->addCurve();
    Curve *pCurveVtn = pGraph->addCurve();
    QString  strP, strV;
    switch (s_iVPlotDir)
    {
        case 0:
            strP = "VPanel.x";
            strV = "VVorton.x";
            break;
        case 1:
            strP = "VPanel.y";
            strV = "VVorton.y";
            break;
        case 2:
            strP = "VPanel.z";
            strV = "VVorton.z";
            break;
        default:
        case 3:
            strP = "VPanel";
            strV = "VVorton";
            break;
    }
    if(s_bGradV)
    {
        strP = "grad("+strP+")";
        strV = "grad("+strV+")";
    }
    pCurvePan->setName(strP);
    pCurveVtn->setName(strV);

    pCurveVtn->setColor(Qt::red);
    pCurveVtn->setWidth(1);

    Vector3d VelPan, VelPan1;
    Vector3d VelVtn, VelVtn1;
    Vector3d V, Vu[3];
    double dl = 0.00001;
    double mu0=1, mu1=1;

    double G[] = {0,0,0,0,0,0,0,0,0};
    double gV=0.0;
    int icomp = s_iGradDir%3*3+s_iVPlotDir;

    for(int i=0; i<nPts; i++)
    {
        double t = double(i)/double(nPts-1);
        Vector3d pt = P0 + (P1-P0)*t;

        // -----Make the panel-induced curve-----
        VelPan.set(0,0,0);
        for(uint p=0; p<m_Panels.size(); p++)
        {
            m_Panels.at(p).doubletBasisVelocity(pt, Vu, false);
            VelPan += Vu[0]*mu0 + Vu[1]*mu0 + Vu[2]*mu1;
        }
        if(!s_bGradV)
        {
            // plot the velocity component
            if(s_iVPlotDir<3) pCurvePan->appendPoint(t, VelPan[s_iVPlotDir%3]);
            else              pCurvePan->appendPoint(t, VelPan.norm());
        }
        else
        {
            // plot the velocity gradient component
            VelPan1.set(0,0,0);
            Vector3d pt1(pt.x+dl*(s_iGradDir==0), pt.y+dl*(s_iGradDir==1), pt.z+dl*(s_iGradDir==2));
            for(uint p=0; p<m_Panels.size(); p++)
            {
                m_Panels.at(p).doubletBasisVelocity(pt1, Vu, false);
                VelPan1 += Vu[0]*mu0 + Vu[1]*mu0 + Vu[2]*mu1;
            }
            if(s_iVPlotDir<3) pCurvePan->appendPoint(t, (VelPan1[s_iVPlotDir%3]-VelPan[s_iVPlotDir%3])/dl);
        }

        // -----Make the vorton-induced curve-----
        VelVtn.set(0,0,0);
        for(uint ip=0; ip<m_Vortons.size(); ip++)
        {
            m_Vortons.at(ip).inducedVelocity(pt, coresize, V);
            VelVtn += V;
        }
        if(!s_bGradV)
        {
            // plot the velocity component
            if(s_iVPlotDir<3) pCurveVtn->appendPoint(t, VelVtn[s_iVPlotDir%3]);
            else              pCurveVtn->appendPoint(t, VelVtn.norm());
        }
        else
        {
            // plot the velocity gradient component
/*            // by forward differentiation
            VelVtn1.set(0,0,0);
            Vector3d pt1(pt.x+dl*(s_iGradDir==0), pt.y+dl*(s_iGradDir==1), pt.z+dl*(s_iGradDir==2));
            for(int ip=0; ip<m_Vortons.size(); ip++)
            {
                m_Vortons.at(ip).inducedVelocity(pt1, V);
                VelVtn1 += V;
            }
            if(s_iVPlotDir<3) pCurveVtn->appendPoint(t, (VelVtn1[s_iVPlotDir%3]-VelVtn[s_iVPlotDir%3])/dl);*/

            gV = 0.0;
            for(uint ip=0; ip<m_Vortons.size(); ip++)
            {
                m_Vortons.at(ip).velocityGradient(pt, coresize, G);
                gV += G[icomp];
            }
            pCurveVtn->appendPoint(t, gV);
        }

/*        qDebug(" %11g  %11g  %11g  %11g  %11g  %11g  r=%11g",
               t, pt.x, pt.y, pt.z, VelPan[s_iVPlotDir%3], VelVtn[s_iVPlotDir%3],
               VelPan[s_iVPlotDir%3]/VelVtn[s_iVPlotDir%3]);*/
    }
    pGraph->resetLimits();
    pGraph->invalidate();
}



