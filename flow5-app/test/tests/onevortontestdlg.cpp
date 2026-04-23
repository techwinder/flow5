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

#include "onevortontestdlg.h"
#include <test/test3d/gl3dvortonfield.h>
#include <core/displayoptions.h>
#include <interfaces/graphs/graph/graph.h>
#include <interfaces/graphs/controls/graphoptions.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/floatedit.h>

bool OneVortonTestDlg::s_bGradV = false;
bool OneVortonTestDlg::s_bDiff  = false;
double OneVortonTestDlg::s_X0= 0.0;
double OneVortonTestDlg::s_Y0= 0.0;
double OneVortonTestDlg::s_Z0=-1.0;

double OneVortonTestDlg::s_X1= 0.0;
double OneVortonTestDlg::s_Y1= 0.0;
double OneVortonTestDlg::s_Z1= 1.0;

int OneVortonTestDlg::s_iGradDir = 0;
int OneVortonTestDlg::s_iVPlotDir = 0;
int OneVortonTestDlg::s_nCurvePts = 100;

QByteArray OneVortonTestDlg::s_Geometry;
QByteArray OneVortonTestDlg::s_HSplitterSizes;
Quaternion OneVortonTestDlg::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);

OneVortonTestDlg::OneVortonTestDlg() : QDialog()
{
    setWindowTitle("One vorton influence");

    setupLayout();

    m_Vorton.resize(1);
    Vector3d u(-0.5,1,1);
    u.normalize();
    m_Vorton.front().setVortex(u, 10.0);
    m_pglVortonField->setVortons(m_Vorton);
}


OneVortonTestDlg::~OneVortonTestDlg()
{
    if(m_pGraphWt && m_pGraphWt->graph())
    {
        m_pGraphWt->graph()->deleteCurveModel();
        delete m_pGraphWt->graph();
    }
}


void OneVortonTestDlg::setupLayout()
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    palette.setColor(QPalette::Text,       DisplayOptions::textColor());

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        m_pHSplitter = new QSplitter;
        {
            m_pHSplitter->setChildrenCollapsible(false);

            QFrame *pFrame = new QFrame(this);
            {
                pFrame->setCursor(Qt::ArrowCursor);
                QVBoxLayout *pFrameLayout = new QVBoxLayout;
                {
                    QFormLayout *pGraphCtrlsLayout= new QFormLayout;
                    {
                        m_pdeVtnCoreSize = new FloatEdit(0.1f);

                        m_pcbVPlotDir = new QComboBox;
                        m_pcbVPlotDir->addItems({"X", "Y", "Z", "Norm"});
                        m_pcbVPlotDir->setCurrentIndex(s_iVPlotDir%4);
                        m_pieCurvePts = new IntEdit(s_nCurvePts);

                        m_pdeX0 = new FloatEdit(s_X0);
                        m_pdeY0 = new FloatEdit(s_Y0);
                        m_pdeZ0 = new FloatEdit(s_Z0);
                        m_pdeX1 = new FloatEdit(s_X1);
                        m_pdeY1 = new FloatEdit(s_Y1);
                        m_pdeZ1 = new FloatEdit(s_Z1);

                        pGraphCtrlsLayout->addRow("Vorton core size", m_pdeVtnCoreSize);
                        pGraphCtrlsLayout->addRow("VDir", m_pcbVPlotDir);
                        pGraphCtrlsLayout->addRow("n curve pts", m_pieCurvePts);
                        pGraphCtrlsLayout->addRow("X0", m_pdeX0);
                        pGraphCtrlsLayout->addRow("Y0", m_pdeY0);
                        pGraphCtrlsLayout->addRow("Z0", m_pdeZ0);
                        pGraphCtrlsLayout->addRow("X1", m_pdeX1);
                        pGraphCtrlsLayout->addRow("Y1", m_pdeY1);
                        pGraphCtrlsLayout->addRow("Z1", m_pdeZ1);
                    }

                    QGroupBox *pVarSelBox = new QGroupBox("Show gradient");
                    {
                        QHBoxLayout *pVarSelLayout = new QHBoxLayout;
                        {
                            m_pchbGradV = new QCheckBox("dV/");
                            m_pchbGradV->setChecked(s_bGradV);

                            m_pcbGradDir = new QComboBox;
                            m_pcbGradDir->addItems({"dx","dy","dz"});
                            m_pcbGradDir->setCurrentIndex(s_iGradDir%3);
                            m_pcbGradDir->setEnabled(s_bGradV);

                            m_pchbDiff = new QCheckBox("Diff");
                            m_pchbDiff->setChecked(s_bDiff);

                            pVarSelLayout->addWidget(m_pchbGradV);
                            pVarSelLayout->addWidget(m_pcbGradDir);
                            pVarSelLayout->addWidget(m_pchbDiff);
                            pVarSelLayout->addStretch();
                        }
                        pVarSelBox->setLayout(pVarSelLayout);
                    }

                    pFrameLayout->addLayout(pGraphCtrlsLayout);
                    pFrameLayout->addWidget(pVarSelBox);
                    pFrameLayout->addStretch();
                }
                pFrame->setLayout(pFrameLayout);
            }
            pFrame->setPalette(palette);

            m_pGraphWt = new GraphWt(this);
            {
                Graph *pGraph = new Graph;
                pGraph->setCurveModel(new CurveModel);
                GraphOptions::resetGraphSettings(*pGraph);
                pGraph->setLegendVisible(true);
                pGraph->setLegendPosition(Qt::AlignHCenter | Qt::AlignTop);
                pGraph->setScaleType(GRAPH::RESETTING);
                pGraph->setAuto(true);
                m_pGraphWt->setGraph(pGraph);
            }
            m_pglVortonField = new gl3dVortonField;

            m_pHSplitter->addWidget(pFrame);
            m_pHSplitter->addWidget(m_pGraphWt);
            m_pHSplitter->addWidget(m_pglVortonField);
        }

        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);

    connect(m_pdeVtnCoreSize, SIGNAL(floatChanged(float)), SLOT(onRecalc()));
    connect(m_pchbGradV,      SIGNAL(clicked()),      SLOT(onRecalc()));
    connect(m_pieCurvePts,    SIGNAL(intChanged(int)), SLOT(onRecalc()));
    connect(m_pdeX0,          SIGNAL(floatChanged(float)), SLOT(onRecalc()));
    connect(m_pdeY0,          SIGNAL(floatChanged(float)), SLOT(onRecalc()));
    connect(m_pdeZ0,          SIGNAL(floatChanged(float)), SLOT(onRecalc()));
    connect(m_pdeX1,          SIGNAL(floatChanged(float)), SLOT(onRecalc()));
    connect(m_pdeY1,          SIGNAL(floatChanged(float)), SLOT(onRecalc()));
    connect(m_pdeZ1,          SIGNAL(floatChanged(float)), SLOT(onRecalc()));
    connect(m_pchbDiff,       SIGNAL(clicked(bool)),  SLOT(onRecalc()));
    connect(m_pcbGradDir,     SIGNAL(activated(int)), SLOT(onRecalc()));
    connect(m_pcbVPlotDir,    SIGNAL(activated(int)), SLOT(onRecalc()));
}


void OneVortonTestDlg::keyPressEvent(QKeyEvent *pEvent)
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


void OneVortonTestDlg::readData()
{
    s_iVPlotDir = m_pcbVPlotDir->currentIndex();
    if(s_iVPlotDir>=3) m_pchbGradV->setChecked(false);

    s_nCurvePts = m_pieCurvePts->value();

    s_bDiff = m_pchbDiff->isChecked();
    s_bGradV = m_pchbGradV->isChecked();
    m_pcbGradDir->setEnabled(s_bGradV);
    s_iGradDir = m_pcbGradDir->currentIndex();

    m_pchbGradV->setEnabled(s_iVPlotDir<3);
    m_pchbDiff->setEnabled(s_iVPlotDir<3 && s_bGradV);
    m_pcbGradDir->setEnabled(s_iVPlotDir<3 && s_bGradV);

    s_X0 = m_pdeX0->value();
    s_Y0 = m_pdeY0->value();
    s_Z0 = m_pdeZ0->value();
    s_X1 = m_pdeX1->value();
    s_Y1 = m_pdeY1->value();
    s_Z1 = m_pdeZ1->value();
    Vector3d P0(s_X0, s_Y0, s_Z0);
    Vector3d P1(s_X1, s_Y1, s_Z1);

    m_pglVortonField->setVortonCoreSize(m_pdeVtnCoreSize->value());
    m_pglVortonField->setPlotLine({P0, P1});
    m_pglVortonField->resetVectors();
    m_pglVortonField->makePlotLineVelocities(1, s_nCurvePts);
    m_pglVortonField->makeVorticityColorMap();
    m_pglVortonField->update();
}


void OneVortonTestDlg::onRecalc()
{
    readData();

    double coresize = m_pdeVtnCoreSize->value();

    Vorton const &vorton = m_Vorton.front();
    Graph *pGraph = m_pGraphWt->graph();
    pGraph->showRightAxis(false);
    pGraph->setYVariableList({"V", "grad(V)", "Damp."});
//    m_pGraph->showRightAxis(true);
    pGraph->setYVariable(0,1);
    pGraph->setYVariable(1,2);
    pGraph->deleteCurves();

    Curve *pCurveV    = nullptr;
    Curve *pCurveGV   = nullptr;
    Curve *pCurveGVd  = nullptr;
    Curve *pCurveMoll = nullptr;

    pCurveV   = pGraph->addCurve();
    pCurveGV  = pGraph->addCurve();
    pCurveGVd = pGraph->addCurve();
    pCurveV->setVisible(!s_bGradV);
    pCurveGV->setVisible(s_bGradV);
    pCurveGVd->setVisible(s_bGradV && s_bDiff);

    pCurveMoll = pGraph->addCurve("Mollification factor");
    pCurveMoll->setColor(QColor(17,117,117));
    pCurveMoll->setVisible(false);

    QString strV;
    switch (s_iVPlotDir)
    {
        case 0: strV = "Vx";    break;
        case 1: strV = "Vy";    break;
        case 2: strV = "Vz";    break;
        default:
        case 3: strV = "V";     break;
    }

    pCurveV->setName(strV);
    pCurveV->setColor(Qt::red);
//    pCurveV->setWidth(2);

    strV = "d"+strV;
    switch(s_iGradDir)
    {
        default:
        case 0: strV += +"/dx";  break;
        case 1: strV += +"/dy";  break;
        case 2: strV += +"/dz";  break;
    }
    pCurveGV->setName(strV);
    pCurveGV->setColor(QColor(155,235,235));
//    pCurveGV->setWidth(2);
    pCurveGVd->setName(strV+"_diff");
    pCurveGVd->setWidth(2);
    pCurveGVd->setColor(Qt::darkYellow);

    Vector3d P0(s_X0, s_Y0, s_Z0);
    Vector3d P1(s_X1, s_Y1, s_Z1);

    Vector3d VelVtn, VelVtn1;
    Vector3d V;
    double dl = 0.0001;

    double G[] = {0,0,0,0,0,0,0,0,0};

    int icomp = s_iGradDir%3*3+s_iVPlotDir;

    for(int i=0; i<s_nCurvePts; i++)
    {
        double t = double(i)/double(s_nCurvePts-1);
        Vector3d pt = P0 + (P1-P0)*t;

        double r = vorton.position().distanceTo(pt);
        double moll = vorton.mollifiedInt(r/m_pdeVtnCoreSize->value());
        if(pCurveMoll) pCurveMoll->appendPoint(t, moll);
//qDebug("  %11g  %11g", t, moll);

        // -----Make the vorton-induced curve-----
        vorton.inducedVelocity(pt, coresize, VelVtn);


        // plot the velocity component
        if(s_iVPlotDir<3) pCurveV->appendPoint(t, VelVtn[s_iVPlotDir%3]);
        else              pCurveV->appendPoint(t, VelVtn.norm());

        // plot the velocity gradient component

        if(s_bGradV && s_iVPlotDir<3)
        {
            vorton.velocityGradient(pt, coresize, G);
            pCurveGV->appendPoint(t, G[icomp]);

            // by centered differentiation
            Vector3d ptm(pt.x-dl*(s_iGradDir==0), pt.y-dl*(s_iGradDir==1), pt.z-dl*(s_iGradDir==2));
            Vector3d ptp(pt.x+dl*(s_iGradDir==0), pt.y+dl*(s_iGradDir==1), pt.z+dl*(s_iGradDir==2));
            vorton.inducedVelocity(ptm, coresize, VelVtn);
            vorton.inducedVelocity(ptp, coresize, VelVtn1);
            pCurveGVd->appendPoint(t, (VelVtn1[s_iVPlotDir%3]-VelVtn[s_iVPlotDir%3])/dl/2.0);
        }

/*        qDebug(" %11g  %11g  %11g  %11g  %11g  %11g  r=%11g",
               t, pt.x, pt.y, pt.z, VelPan[s_iVPlotDir%3], VelVtn[s_iVPlotDir%3],
               VelPan[s_iVPlotDir%3]/VelVtn[s_iVPlotDir%3]);*/
    }
    m_pGraphWt->graph()->resetLimits();
    m_pGraphWt->graph()->invalidate();
    update();
}


void OneVortonTestDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);

    restoreGeometry(s_Geometry);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    m_pglVortonField->restoreViewPoint(s_ab_quat);
    onRecalc();
}


void OneVortonTestDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    readData();

    s_Geometry = saveGeometry();
    s_HSplitterSizes = m_pHSplitter->saveState();

    m_pglVortonField->saveViewPoint(s_ab_quat);
}


void OneVortonTestDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("OneVortonTest");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();
        s_HSplitterSizes = settings.value("HSplitSize").toByteArray();

        s_bGradV   = settings.value("GradV",   s_bGradV).toBool();
        s_iGradDir = settings.value("GradDir", s_iGradDir).toInt();
        s_bDiff    = settings.value("bDiff",   s_bDiff).toBool();
        s_X0 = settings.value("X0", s_X0).toDouble();
        s_Y0 = settings.value("Y0", s_Y0).toDouble();
        s_Z0 = settings.value("Z0", s_Z0).toDouble();
        s_X1 = settings.value("X1", s_X1).toDouble();
        s_Y1 = settings.value("Y1", s_Y1).toDouble();
        s_Z1 = settings.value("Z1", s_Z1).toDouble();

        s_nCurvePts = settings.value("CurvePts",   s_nCurvePts).toInt();
        s_iVPlotDir = settings.value("VPlotDir",   s_iVPlotDir).toInt();
     }
    settings.endGroup();
}


void OneVortonTestDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("OneVortonTest");
    {
        settings.setValue("WindowGeometry", s_Geometry);
        settings.setValue("HSplitSize",     s_HSplitterSizes);

        settings.setValue("GradV",   s_bGradV);
        settings.setValue("GradDir", s_iGradDir);
        settings.setValue("bDiff", s_bDiff);

        settings.setValue("X0", s_X0);
        settings.setValue("Y0", s_Y0);
        settings.setValue("Z0", s_Z0);
        settings.setValue("X1", s_X1);
        settings.setValue("Y1", s_Y1);
        settings.setValue("Z1", s_Z1);

        settings.setValue("CurvePts",   s_nCurvePts);
        settings.setValue("VPlotDir",   s_iVPlotDir);
     }
    settings.endGroup();
}
