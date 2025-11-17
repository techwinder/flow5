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


#include "panelanalysistest.h"

#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <api/p3linanalysis.h>
#include <api/p3unianalysis.h>
#include <api/p4analysis.h>
#include <api/units.h>
#include <core/xflcore.h>
#include <core/displayoptions.h>
#include <interfaces/graphs/controls/graphoptions.h>
#include <interfaces/graphs/graph/graph.h>
#include <api/boatopp.h>
#include <api/planeopp.h>
#include <api/polar3d.h>
#include <api/planexfl.h>
#include <api/boat.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>

LineStyle PanelAnalysisTest::s_LS = {true, Line::SOLID, 1, xfl::Orchid, Line::NOSYMBOL};

bool PanelAnalysisTest::s_bPotential = true;
int PanelAnalysisTest::s_VComp = 0;
int PanelAnalysisTest::s_PanelId = 0;
double PanelAnalysisTest::s_ZMax = 0.01;
double PanelAnalysisTest::s_ZInc = 0.001;
QByteArray PanelAnalysisTest::s_Geometry;

PanelAnalysisTest::PanelAnalysisTest() : GraphWt()
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlag(Qt::WindowStaysOnTopHint);

    m_pgl3dXflView = nullptr;

    m_pPolar3d    = nullptr;
    m_pOpp3d      = nullptr;

    m_pP3Analysis = nullptr;
    m_pP4Analysis = nullptr;

    Graph *pGraph = new Graph;
    pGraph->setName("Panel result");
    pGraph->setCurveModel(new CurveModel);
    pGraph->setLegendVisible(true);
    pGraph->setLegendPosition(Qt::AlignHCenter | Qt::AlignCenter);
    pGraph->setScaleType(GRAPH::RESETTING);
    pGraph->setAuto(true);
    GraphOptions::resetGraphSettings(*pGraph);
    pGraph->setLegendPosition(Qt::AlignTop | Qt::AlignHCenter);

    setGraph(pGraph);

    makeControls();
}


PanelAnalysisTest::~PanelAnalysisTest()
{
    if(m_pP3Analysis) delete m_pP3Analysis;
    m_pP3Analysis = nullptr;

    if(m_pP4Analysis) delete m_pP4Analysis;
    m_pP4Analysis = nullptr;

    if(m_pGraph) delete m_pGraph;
    m_pGraph = nullptr;

    disconnect(m_pgl3dXflView, SIGNAL(panelSelected(int)), nullptr, nullptr);
    m_pgl3dXflView->clearDebugPoints();
    m_pgl3dXflView->update();
}


QFrame* PanelAnalysisTest::makeControls()
{
    QFrame *pFrame = new QFrame(this);
    {
        QPalette palette;
        palette.setColor(QPalette::Text,       DisplayOptions::textColor());
        palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
        palette.setColor(QPalette::Window,     QColor(125,125,125,65));
        pFrame->setCursor(Qt::ArrowCursor);
        pFrame->setPalette(palette);
        pFrame->setAutoFillBackground(false);

        QVBoxLayout * pFrameLayout =new QVBoxLayout;
        {
            QHBoxLayout *pPanelIdLayout = new QHBoxLayout;
            {
                QLabel *pLabPanelId = new QLabel("Panel index");
                m_piePanelId = new IntEdit(s_PanelId);
                pPanelIdLayout->addWidget(pLabPanelId);
                pPanelIdLayout->addWidget(m_piePanelId);
                pPanelIdLayout->addStretch();
            }

            QFrame *pVarFrame = new QFrame;
            {
                QGridLayout *pVarLayout = new QGridLayout;
                {
                    m_prbPotential = new QRadioButton("Potential");
                    m_prbPotential->setChecked(s_bPotential);
                    m_prbVelocity  = new QRadioButton("Velocity");
                    m_prbVelocity->setChecked(!s_bPotential);
                    m_pcbVcomp = new QComboBox;
                    m_pcbVcomp->addItems({"Vn", "|V|", "V.wind"});
                    m_pcbVcomp->setCurrentIndex(s_VComp);
                    pVarLayout->addWidget(m_prbPotential, 1 ,1);
                    pVarLayout->addWidget(m_prbVelocity,  2, 1);
                    pVarLayout->addWidget(m_pcbVcomp,     2, 2);

                }
                pVarFrame->setLayout(pVarLayout);
            }

            QGroupBox *pGraphBox = new QGroupBox("Curve");
            {
                QGridLayout *pGraphDataLayout = new QGridLayout;
                {
                    QLabel *pLabZMax = new QLabel("+/-z");
                    QLabel *pLabZInc = new QLabel("dz");
                    QLabel *pLabLen0 = new QLabel(QUnits::lengthUnitLabel());
                    QLabel *pLabLen1 = new QLabel(QUnits::lengthUnitLabel());
                    pLabZMax->setPalette(palette);
                    pLabZInc->setPalette(palette);
                    m_pdeZMax    = new FloatEdit(s_ZMax*Units::mtoUnit());
                    m_pdeZInc    = new FloatEdit(s_ZInc*Units::mtoUnit());

                    m_pLineBtn = new LineBtn;
                    m_pLineBtn->setTheStyle(s_LS);
                    m_pLineBtn->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
                    connect(m_pLineBtn, SIGNAL(clickedLB(LineStyle)), SLOT(onCurveStyle(LineStyle)));

                    pGraphDataLayout->addWidget(pLabZMax,       1, 1, Qt::AlignVCenter | Qt::AlignRight);
                    pGraphDataLayout->addWidget(m_pdeZMax,      1, 2);
                    pGraphDataLayout->addWidget(pLabLen0,       1, 3, Qt::AlignLeft | Qt::AlignVCenter);
                    pGraphDataLayout->addWidget(pLabZInc ,      2, 1, Qt::AlignVCenter | Qt::AlignRight);
                    pGraphDataLayout->addWidget(m_pdeZInc,      2, 2);
                    pGraphDataLayout->addWidget(pLabLen1,       2, 3, Qt::AlignLeft | Qt::AlignVCenter);
                    pGraphDataLayout->addWidget(m_pLineBtn,     3, 1, 1, 2);
                }
                pGraphBox->setLayout(pGraphDataLayout);
            }

            m_pptoOutput = new PlainTextOutput;
            m_pptoOutput->setMinimumWidth(300);
            m_pptoOutput->setMinimumHeight(350);
            pFrameLayout->addLayout(pPanelIdLayout);
            pFrameLayout->addWidget(pVarFrame);
            pFrameLayout->addWidget(pGraphBox);
            pFrameLayout->addWidget(m_pptoOutput);
        }
        pFrame->setLayout(pFrameLayout);
    }

    connect(m_piePanelId,   SIGNAL(intChanged(int)), SLOT(onMakeGraph()));
    connect(m_pdeZMax,      SIGNAL(floatChanged(float)), SLOT(onMakeGraph()));
    connect(m_pdeZInc,      SIGNAL(floatChanged(float)), SLOT(onMakeGraph()));
    connect(m_prbVelocity,  SIGNAL(clicked(bool)),  SLOT(onMakeGraph()));
    connect(m_prbPotential, SIGNAL(clicked(bool)),  SLOT(onMakeGraph()));
    connect(m_pcbVcomp,     SIGNAL(activated(int)), SLOT(onMakeGraph()));
    return pFrame;
}


void PanelAnalysisTest::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            QTimer::singleShot(0, this, SLOT(close()));
            return;
        }

        default: break;
    }
    GraphWt::keyPressEvent(pEvent);
}


void PanelAnalysisTest::onCurveStyle(LineStyle ls)
{
    LineMenu *pLineMenu = new LineMenu(nullptr);
    pLineMenu->initMenu(ls);
    pLineMenu->exec(QCursor::pos());
    s_LS = pLineMenu->theStyle();
    m_pLineBtn->setTheStyle(s_LS);
    onMakeGraph();
}


void PanelAnalysisTest::setAnalysis(Plane const *pPlane, const Polar3d *pWPolar, PlaneOpp const*pPOpp)
{
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
    if(!pWPolar) return;
    m_pPolar3d = pWPolar;
    m_pOpp3d = pPOpp;

    if(m_pP4Analysis) delete m_pP4Analysis;
    m_pP4Analysis = nullptr;

    if(m_pP3Analysis) delete m_pP3Analysis;
    m_pP3Analysis = nullptr;

    if(m_pPolar3d->isQuadMethod())
    {
        m_pP4Analysis = new P4Analysis;
        m_pP4Analysis->setQuadMesh(pPlaneXfl->refQuadMesh());
        m_pP4Analysis->initializeAnalysis(pWPolar, 0);
    }
    else if(pWPolar->isTriangleMethod())
    {
        if(m_pPolar3d->isTriUniformMethod())
        {
            m_pP3Analysis = new P3UniAnalysis;
        }
        else if(m_pPolar3d->isTriLinearMethod())
        {
            m_pP3Analysis = new P3LinAnalysis;
        }
        m_pP3Analysis->setTriMesh(pPlaneXfl->triMesh());
        m_pP3Analysis->initializeAnalysis(pWPolar, 0);
    }
    onMakeGraph();
}


void PanelAnalysisTest::setAnalysis(const Boat *pBoat, Polar3d const *pBtPolar, BoatOpp const*pBtOpp)
{
    if(!pBtPolar) return;
    m_pPolar3d = pBtPolar;
    m_pOpp3d   = pBtOpp;

    if(m_pP4Analysis) delete m_pP4Analysis;
    m_pP4Analysis = nullptr;

    if(m_pP3Analysis) delete m_pP3Analysis;
    m_pP3Analysis = nullptr;

    if(m_pPolar3d->isQuadMethod())
    {
    }
    else if(pBtPolar->isTriangleMethod())
    {
        if(m_pPolar3d->isTriUniformMethod())
        {
            m_pP3Analysis = new P3UniAnalysis;
        }
        else if(m_pPolar3d->isTriLinearMethod())
        {
            m_pP3Analysis = new P3LinAnalysis;
        }
        m_pP3Analysis->setTriMesh(pBoat->triMesh());
        m_pP3Analysis->initializeAnalysis(pBtPolar, 0);
    }
    onMakeGraph();
}


void PanelAnalysisTest::getVelocityVector(Vector3d const &C, double const *Mu, double const *Sigma, double coreradius, Vector3d &velocity) const
{
    if(m_pP4Analysis)
        m_pP4Analysis->getVelocityVector(C, Mu, Sigma, velocity, coreradius, false, xfl::isMultiThreaded());
    else if(m_pP3Analysis)
        m_pP3Analysis->getVelocityVector(C, Mu, Sigma, velocity, coreradius, false, xfl::isMultiThreaded());
}


double PanelAnalysisTest::getPotential(Vector3d const &C, double const *Mu, double const *Sigma) const
{
    if(m_pP4Analysis)
    {
        return m_pP4Analysis->getPotential(C, Mu, Sigma);
    }
    else if(m_pP3Analysis)
    {
        return m_pP3Analysis->getPotential(C, Mu, Sigma);
    }
    return 0.0;
}


void PanelAnalysisTest::showEvent(QShowEvent *pEvent)
{
    GraphWt::showEvent(pEvent);
    restoreGeometry(s_Geometry);
    onMakeGraph();
}


void PanelAnalysisTest::hideEvent(QHideEvent *pEvent)
{
    GraphWt::hideEvent(pEvent);
    readData();
    s_Geometry = saveGeometry();
}


void PanelAnalysisTest::loadSettings(QSettings &settings)
{
    settings.beginGroup("PanelResultTest");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();

        s_bPotential = settings.value("bPotential", s_bPotential).toBool();
        s_PanelId    = settings.value("PanelId",    s_PanelId).toInt();
        s_ZMax       = settings.value("ZMax",       s_ZMax).toDouble();
        s_ZInc       = settings.value("ZInc",       s_ZInc).toDouble();
        s_VComp      = settings.value("VComp",       s_VComp).toDouble();
        xfl::loadLineSettings(settings, s_LS, "CurveStyle");
     }
    settings.endGroup();
}


void PanelAnalysisTest::saveSettings(QSettings &settings)
{
    settings.beginGroup("PanelResultTest");
    {
        settings.setValue("WindowGeometry", s_Geometry);

        settings.setValue("bPotential", s_bPotential);
        settings.setValue("PanelId",    s_PanelId);
        settings.setValue("ZMax",       s_ZMax);
        settings.setValue("ZInc",       s_ZInc);
        settings.setValue("VComp",      s_VComp);
        xfl::saveLineSettings(settings, s_LS, "CurveStyle");
     }
    settings.endGroup();
}


void PanelAnalysisTest::readData()
{
    s_VComp   = m_pcbVcomp->currentIndex();
    s_PanelId = m_piePanelId->value();
    s_ZMax    = m_pdeZMax->value()/Units::mtoUnit();
    s_ZInc    = m_pdeZInc->value()/Units::mtoUnit();
    s_bPotential = m_prbPotential->isChecked();
}


void PanelAnalysisTest::set3dView(gl3dXflView *p3dView)
{
    m_pgl3dXflView=p3dView;
    connect(m_pgl3dXflView, SIGNAL(panelSelected(int)), SLOT(onPickedIndex(int)));
}


/** From the gl3dXflView */
void PanelAnalysisTest::onPickedIndex(int idx)
{
    s_PanelId = idx;
    m_piePanelId->setValue(s_PanelId);
    onMakeGraph();
}


/** Note: WRONG if the mesh has been rotated in the 3d view */
void PanelAnalysisTest::onMakeGraph()
{
    if(!m_pPolar3d || !m_pOpp3d) return;

    m_pptoOutput->clear();
    readData();

    Vector3d winddir = objects::windDirection(m_pOpp3d->alpha(), m_pOpp3d->beta());

    m_pGraph->setXVariableList({"z ("+QUnits::lengthUnitLabel()+")"});
    m_pGraph->setYVariableList({"phi", "V ("+QUnits::speedUnitLabel()+")"});

    m_pGraph->setYVariable(1,1);
    m_pGraph->deleteCurves();
    Curve *pCurvePhi[]{nullptr, nullptr, nullptr};
    Curve *pCurveV[]{nullptr, nullptr, nullptr};

    Vector3d pt[3];
    Vector3d V;
    double phi = 0.0;

    Panel  const *pPanel  = nullptr;
    Panel4 const *pPanel4 = nullptr;
    Panel3 const *pPanel3 = nullptr;
    if     (m_pP4Analysis)
    {
        pPanel4 = m_pP4Analysis->panel4(s_PanelId);
        pPanel = pPanel4;
    }
    else if(m_pP3Analysis)
    {
        pPanel3 = m_pP3Analysis->panel3(s_PanelId);
        pPanel = pPanel3;
    }

    int curvecount = 1;
    if(m_pP3Analysis && m_pP3Analysis->isTriLinMethod()) curvecount = 3;

    if(m_prbPotential->isChecked())
    {
        for(int i=0; i<curvecount; i++)
        {
            pCurvePhi[i] = m_pGraph->addCurve("phi "+QString::fromStdString(m_pPolar3d->name()+m_pOpp3d->title(false)));
            pCurvePhi[i]->setTheStyle(s_LS);
            pCurvePhi[i]->setColor(pCurvePhi[i]->qColor().darker(100+35*i));
        }
        m_pGraph->setYVariable(0,0);
    }
    else if(m_prbVelocity->isChecked())
    {
        for(int i=0; i<curvecount; i++)
        {
            pCurveV[i] = m_pGraph->addCurve("V ("+QUnits::speedUnitLabel()+") "+QString::fromStdString(m_pOpp3d->polarName()+m_pOpp3d->title(false)));
            pCurveV[i]->setTheStyle(s_LS);
            pCurveV[i]->setColor(pCurveV[i]->qColor().darker(100+35*i));
        }
        m_pGraph->setYVariable(0,1);
    }

    if(!pPanel)
    {
        m_pptoOutput->onAppendQText(QString::asprintf("Panel %d is not valid\n", s_PanelId));
        m_pGraph->deleteCurves();
        m_pGraph->resetLimits();
        m_pGraph->invalidate();
        update();
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    double const *mu    = m_pOpp3d->gamma().data();
    double const *sigma = m_pOpp3d->sigma().data();

    Vector3d VInf = objects::windDirection(m_pOpp3d->alpha(), m_pOpp3d->beta()) * m_pOpp3d->QInf();

    double g[]{0,0,0};
    // plot the curves in the panel's local reference frame;
    for(double z=-s_ZMax; z<s_ZMax; z+=s_ZInc)
//    double z=0;
    {
        if(pPanel3 && m_pP3Analysis && m_pP3Analysis->isTriLinMethod())
        {
            g[0]=2.0/3.0;  g[1]=1.0/6.0;  g[2]=1.0/6.0;
            pPanel3->cartesianCoords(g, pt[0]);
            pt[0] += pPanel->normal()*z;

            g[0]=1.0/6.0;  g[1]=2.0/3.0;  g[2]=1.0/6.0;
            pPanel3->cartesianCoords(g, pt[1]);
            pt[1] += pPanel->normal()*z;

            g[0]=1.0/6.0;  g[1]=1.0/6.0;  g[2]=2.0/3.0;
            pPanel3->cartesianCoords(g, pt[2]);
            pt[2] += pPanel->normal()*z;
        }
        else
        {
            pt[0].set(pPanel->CoG());
            pt[0] += pPanel->normal()*z;
        }

        for(int ic=0; ic<3; ic++)
        {
            if(pCurvePhi[ic])
            {
                phi = getPotential(pt[ic], mu, sigma);

                pCurvePhi[ic]->appendPoint(z*Units::mtoUnit(), phi);
//                qDebug(" z=%11g  phi=%11g", z, phi);
                m_pptoOutput->onAppendQText(QString::asprintf(" z=%11g  phi=%11g\n", z, phi));
            }
            if(pCurveV[ic])
            {
                getVelocityVector(pt[ic], mu, sigma, Vortex::coreRadius(), V);
                V += VInf;

                if(s_VComp==0)
                    pCurveV[ic]->appendPoint(z*Units::mtoUnit(), V.dot(pPanel->normal())*Units::mstoUnit());
                else if(s_VComp==1)
                    pCurveV[ic]->appendPoint(z*Units::mtoUnit(), V.norm()*Units::mstoUnit());
                else if(s_VComp==2)
                    pCurveV[ic]->appendPoint(z*Units::mtoUnit(), V.dot(winddir)*Units::mstoUnit());
//                qDebug(" z=%13g  Vn=%13g", z, V.dot(pPanel->normal()));
                m_pptoOutput->onAppendQText(QString::asprintf(" z=%11g  Vn=%11g\n", z, V.dot(pPanel->normal())));
            }
        }
    }
    m_pGraph->resetLimits();
    m_pGraph->invalidate();

    m_pgl3dXflView->clearDebugPoints();
    if(m_pP3Analysis && m_pP3Analysis->isTriUniMethod())
    {
        double Z = 0.05;
        double n = 10.0;
        for(double d=-Z; d<Z; d+=2.0*Z/n)
        {
            Vector3d C = pPanel->CoG()+ pPanel->normal() * d;
            getVelocityVector(C, mu, sigma, Vortex::coreRadius(), V);
            V += VInf;
            m_pgl3dXflView->appendDebugPoint(C);
            m_pgl3dXflView->appendDebugVec(V);
        }
        m_pgl3dXflView->update();
    }
    else
    {
        m_pgl3dXflView->appendDebugPoint(pPanel->CoG());
    }

/*    QString strange;
    strange = QString::asprintf("4.pi.mu  = %g\n", m_pOpp3d->gamma(s_PanelId)*4*PI);
    m_pptOuput->setPlainText(strange);

    strange = QString::asprintf("4.pi.sig = %g\n", m_pOpp3d->sigma(s_PanelId)*4*PI);
    m_pptOuput->onAppendThisPlainText(strange); */


    update();

    QApplication::restoreOverrideCursor();
}



