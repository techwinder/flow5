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
#include <QGroupBox>
#include <QTabWidget>
#include <QButtonGroup>
#include <QtConcurrent/QtConcurrent>

#include "crossflowctrls.h"

#include <globals/mainframe.h>



#include <api/boat.h>
#include <api/boatopp.h>
#include <api/boatpolar.h>
#include <api/planeopp.h>
#include <api/planepolar.h>
#include <api/planexfl.h>
#include <api/units.h>
#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/widgets/customwts/exponentialslider.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/line/linemenu.h>
#include <modules/xplane/glview/gl3dxplaneview.h>
#include <modules/xplane/xplane.h>
#include <modules/xsail/view/gl3dxsailview.h>
#include <modules/xsail/xsail.h>

XPlane *CrossFlowCtrls::s_pXPlane = nullptr;
XSail  *CrossFlowCtrls::s_pXSail  = nullptr;


double CrossFlowCtrls::s_Width  = 1.0;
double CrossFlowCtrls::s_Height = 1.0;
double CrossFlowCtrls::s_XPos   = 1.0;

int CrossFlowCtrls::s_nVelocitySamples  = 15;
int CrossFlowCtrls::s_nPressureSamples = 51;
int CrossFlowCtrls::s_nVorticitySamples = 51;

int CrossFlowCtrls::s_OmegaDir = 0;

double CrossFlowCtrls::s_OmegaCoef = 1000.0;

bool CrossFlowCtrls::s_bAutoOmegaScale = true;
double CrossFlowCtrls::s_OmegaMin = -1.0;
double CrossFlowCtrls::s_OmegaMax =  1.0;



CrossFlowCtrls::CrossFlowCtrls(QWidget *pMainFrame) : QWidget(pMainFrame)
{
    setAttribute(Qt::WA_DeleteOnClose, false);
    setWindowTitle("3d Scales Settings");
    setWindowFlags(Qt::Tool);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    m_pgl3dXPlaneView = nullptr;
    m_pgl3dXSailView = nullptr;

    m_bGridVelocity = false;
    m_bVorticityMap = false;

    setupLayout();

    connectSignals();
}


void CrossFlowCtrls::connectSignals()
{
    //Wake
    connect(m_pchCrossFlowVel,      SIGNAL(clicked(bool)),         SLOT(onMakeCrossFlowPlane()));
    connect(m_pchVorticityMap,      SIGNAL(clicked(bool)),         SLOT(onMakeCrossFlowPlane()));
    connect(m_pchAutoOmegaScale,    SIGNAL(clicked(bool)),         SLOT(onMakeCrossFlowPlane()));
    connect(m_pchAutoOmegaScale,    SIGNAL(clicked(bool)),         SLOT(onOmegaScale()));
    connect(m_pcbOmegaDir,          SIGNAL(activated(int)),        SLOT(onMakeCrossFlowPlane()));
    connect(m_pdeHeight,            SIGNAL(floatChanged(float)),   SLOT(onMakeCrossFlowPlane()));
    connect(m_pdeOmegaMax,          SIGNAL(floatChanged(float)),   SLOT(onOmegaScale()));
    connect(m_pdeOmegaMin,          SIGNAL(floatChanged(float)),   SLOT(onOmegaScale()));
    connect(m_pdeWidth,             SIGNAL(floatChanged(float)),   SLOT(onMakeCrossFlowPlane()));
    connect(m_pslPlanePos,          SIGNAL(sliderMoved(int)),      SLOT(onMakeCrossFlowPlane()));
    connect(m_pieVorticitySamples,  SIGNAL(intChanged(int)),       SLOT(onMakeCrossFlowPlane()));
    connect(m_pieVelocitySamples,   SIGNAL(intChanged(int)),       SLOT(onMakeCrossFlowPlane()));
}


void CrossFlowCtrls::set3dXPlaneView(gl3dXPlaneView *pView)
{
    m_pgl3dXPlaneView = pView;
    m_pgl3dXPlaneView->setCrossFlowCtrls(this);
}


void CrossFlowCtrls::set3dXSailView(gl3dXSailView *pView)
{
    m_pgl3dXSailView = pView;
    m_pgl3dXSailView->setCrossFlowCtrls(this);
}


void CrossFlowCtrls::updateUnits()
{
    //wake
    m_pLabLen2->setText(Units::lengthUnitQLabel());
}


void CrossFlowCtrls::setupLayout()
{
    QVBoxLayout *pWakeLayout = new QVBoxLayout;
    {
        QGroupBox *pPlaneGeomBox = new QGroupBox(tr("Crossflow plane geometry"));
        {
            QVBoxLayout *pPlaneGeomLayout = new QVBoxLayout;
            {
                QHBoxLayout *pXPosLayout = new QHBoxLayout;
                {
                    QLabel *pLabXPos = new QLabel(tr("X"));
                    m_pslPlanePos = new QSlider(Qt::Horizontal);
                    m_pslPlanePos->setRange(0, 1000);
                    m_pslPlanePos->setTickInterval(100);
                    m_pslPlanePos->setTickPosition(QSlider::TicksBelow);
                    m_pslPlanePos->setValue(0);
                    m_pslPlanePos->setSingleStep(1);
                    m_pLabLen1 = new QLabel(Units::lengthUnitQLabel());

                    pXPosLayout->addWidget(pLabXPos);
                    pXPosLayout->addWidget(m_pslPlanePos);
                    pXPosLayout->addWidget(m_pLabLen1);
                }

                QGridLayout *pPlaneParamsLayout = new QGridLayout;
                {
                    m_pLabLen2  = new QLabel(Units::lengthUnitQLabel());
                    m_pdeWidth  = new FloatEdit;
                    m_pdeHeight = new FloatEdit;

                    pPlaneParamsLayout->addWidget(new QLabel(tr("Y")),     1, 2, Qt::AlignCenter);
                    pPlaneParamsLayout->addWidget(new QLabel(tr("Z")),     1, 3, Qt::AlignCenter);
                    pPlaneParamsLayout->addWidget(new QLabel(tr("Size:")), 2, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pPlaneParamsLayout->addWidget(m_pdeWidth,          2, 2);
                    pPlaneParamsLayout->addWidget(m_pdeHeight,         2, 3);
                    pPlaneParamsLayout->addWidget(m_pLabLen2,          2, 4);
                    pPlaneParamsLayout->setColumnStretch(4,1);
                }
                pPlaneGeomLayout->addLayout(pXPosLayout);
                pPlaneGeomLayout->addLayout(pPlaneParamsLayout);
            }
            pPlaneGeomBox->setLayout(pPlaneGeomLayout);
        }

        QGroupBox *pVelocityBox = new QGroupBox(tr("Velocity vectors"));
        {
            QVBoxLayout *pParamLayout = new QVBoxLayout;
            {
            QString tip(tr("<p>Scale the vectors using the velocity slider in the scales tab</p>"));
            m_pchCrossFlowVel = new QCheckBox(tr("Perturbation velocities"));
                m_pchCrossFlowVel->setToolTip(tip);

                QHBoxLayout *pSamplesLayout =new QHBoxLayout;
                {
                    QLabel *pLabVel = new QLabel(tr("Samples:"));
                    m_pieVelocitySamples = new IntEdit;
                    m_pieVelocitySamples->setToolTip(tr("<p>The number of velocity vectors in each of the y and z directions.<br>"
                                                      "Scale the vectors using the velocity slider in the scales tab.</p>"));
                    pSamplesLayout->addWidget(pLabVel);
                    pSamplesLayout->addWidget(m_pieVelocitySamples);
                    pSamplesLayout->addStretch();
                }
                pParamLayout->addWidget(m_pchCrossFlowVel);
                pParamLayout->addLayout(pSamplesLayout);
            }
            pVelocityBox->setLayout(pParamLayout);
        }

        QGroupBox *pVorticityBox = new QGroupBox(tr("Vorticity"));
        {
            QVBoxLayout *pVorticityLayout = new QVBoxLayout;
            {
                QHBoxLayout *pComponentLayout = new QHBoxLayout;
                {
                    m_pchVorticityMap = new QCheckBox(tr("Crossflow vorticity (VPW only)"));
                    m_pcbOmegaDir = new QComboBox;
                    m_pcbOmegaDir->addItems({tr("X"), tr("Y"), tr("Z"), tr("Norm")});
                    m_pcbOmegaDir->setCurrentIndex(s_OmegaDir);
                    pComponentLayout->addWidget(m_pchVorticityMap);
                    pComponentLayout->addStretch();
                    pComponentLayout->addWidget(m_pcbOmegaDir);
                }

                QGridLayout *pParamLayout = new QGridLayout;
                {
                    m_pchAutoOmegaScale = new QCheckBox(tr("Auto scale"));
                    m_pdeOmegaMin = new FloatEdit(s_OmegaMin);
                    m_pdeOmegaMax = new FloatEdit(s_OmegaMax);

                    m_pieVorticitySamples = new IntEdit;
                    m_pieVorticitySamples->setToolTip(tr("<p>Number of colour samples in the y and z directions.<br>"
                                                      "Recommendation: N = 50 to 200 </p>"));

                    pParamLayout->addWidget(new QLabel(tr("Min.")),       3, 2, Qt::AlignCenter);
                    pParamLayout->addWidget(new QLabel(tr("Max.")),       3, 3, Qt::AlignCenter);
                    pParamLayout->addWidget(m_pchAutoOmegaScale,      4, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamLayout->addWidget(m_pdeOmegaMin,            4, 2);
                    pParamLayout->addWidget(m_pdeOmegaMax,            4, 3);

                    pParamLayout->addWidget(new QLabel(tr("Samples:")),   5, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamLayout->addWidget(m_pieVorticitySamples,    5, 2, Qt::AlignRight | Qt::AlignVCenter);

                    pParamLayout->setRowStretch(7,1);
                    pParamLayout->setColumnStretch(4,1);
                }
                pVorticityLayout->addLayout(pComponentLayout);
                pVorticityLayout->addLayout(pParamLayout);
            }
            pVorticityBox->setLayout(pVorticityLayout);
        }

        pWakeLayout->addWidget(pPlaneGeomBox);
        pWakeLayout->addWidget(pVelocityBox);
        pWakeLayout->addWidget(pVorticityBox);
        pWakeLayout->addStretch();
    }
    setLayout(pWakeLayout);
}


void CrossFlowCtrls::initWidget()
{
    updateUnits();

    setWakeData();
    enableWakeControls();
}


void CrossFlowCtrls::setWakeData()
{
    m_pchCrossFlowVel->setChecked(m_bGridVelocity);
    m_pchVorticityMap->setChecked(m_bVorticityMap);

    m_pdeWidth->setValue(s_Width*Units::mtoUnit());
    m_pdeHeight->setValue(s_Height*Units::mtoUnit());
    m_pslPlanePos->setValue(int(s_XPos*1000.0));

    m_pieVelocitySamples->setValue(s_nVelocitySamples);

    m_pieVorticitySamples->setValue(s_nVorticitySamples);

    m_pchAutoOmegaScale->setChecked(s_bAutoOmegaScale);
    m_pdeOmegaMin->setValue(s_OmegaMin);
    m_pdeOmegaMax->setValue(s_OmegaMax);
    m_pdeOmegaMin->setEnabled(!s_bAutoOmegaScale);
    m_pdeOmegaMax->setEnabled(!s_bAutoOmegaScale);
//    m_pgl3dXPlaneView->m_LegendOverlay.setManualRange(s_OmegaMin*s_OmegaCoef, s_OmegaMax*s_OmegaCoef);

    m_pcbOmegaDir->setCurrentIndex(s_OmegaDir);
}


void CrossFlowCtrls::enableWakeControls()
{
    m_pieVelocitySamples->setEnabled(m_bGridVelocity);


    m_pcbOmegaDir->setEnabled(m_bVorticityMap);
    m_pchAutoOmegaScale->setEnabled(m_bVorticityMap);
    m_pdeOmegaMin->setEnabled(!s_bAutoOmegaScale && m_bVorticityMap);
    m_pdeOmegaMax->setEnabled(!s_bAutoOmegaScale && m_bVorticityMap);
    m_pieVorticitySamples->setEnabled(m_bVorticityMap);
}


void CrossFlowCtrls::readWakeData()
{
    m_bGridVelocity = m_pchCrossFlowVel->isChecked();
    m_bVorticityMap = m_pchVorticityMap->isChecked();

    m_pgl3dXPlaneView->showColourLegend(m_bVorticityMap);

    s_Width  = m_pdeWidth->value()/Units::mtoUnit();
    s_Height = m_pdeHeight->value()/Units::mtoUnit();
    s_XPos   = double(m_pslPlanePos->value()/1000.0);

    s_nVelocitySamples  = std::min(m_pieVelocitySamples->value(),  500);
    s_nVorticitySamples = std::min(m_pieVorticitySamples->value(), 500);

    s_OmegaDir = m_pcbOmegaDir->currentIndex();
}


/** unused */
/*void CrossFlowCtrls::onApply()
{
    s_bAutoGammaScale = m_pchAutoGammaScale->isChecked();
    s_GammaMax = m_pdeGammaMin->value();
    s_GammaMax = m_pdeGammaMax->value();
    s_bAutoCpScale = m_pchAutoCpScale->isChecked();
    s_CpMax = m_pdeCpMin->value();
    s_CpMax = m_pdeCpMax->value();
    s_bAutoPressureScale = m_pchAutoPressureScale->isChecked();
    s_PressureMin = m_pdePressureMin->value()/Units::PatoUnit();
    s_PressureMax = m_pdePressureMax->value()/Units::PatoUnit();

    emit update3dScales();
}*/


void CrossFlowCtrls::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dScales");
    {
        //wake
        s_Width      = settings.value("YWidth",          s_Width).toDouble();
        s_Height     = settings.value("ZHeight",         s_Height).toDouble();
        s_XPos       = settings.value("XPos",            s_XPos).toDouble();

        s_nVelocitySamples  = settings.value("NVelSamples",   s_nVelocitySamples).toInt();
        s_nPressureSamples  = settings.value("NPressSamples", s_nPressureSamples).toInt();
        s_nVorticitySamples = settings.value("NVortSamples",  s_nVorticitySamples).toInt();

        s_bAutoOmegaScale = settings.value("AutoOmegaScale", true).toBool();
        s_OmegaMin        = settings.value("LegendOmegaMin", s_OmegaMin).toDouble();
        s_OmegaMax        = settings.value("LegendOmegaMax", s_OmegaMax).toDouble();
        s_OmegaDir        = settings.value("OmegaDir",       s_OmegaDir).toInt();
    }
    settings.endGroup();
}


void CrossFlowCtrls::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dScales");
    {
        // wake
        settings.setValue("YWidth",         s_Width);
        settings.setValue("ZHeight",        s_Height);
        settings.setValue("XPos",           s_XPos);

        settings.setValue("NVelSamples",    s_nVelocitySamples);
        settings.setValue("NPressSamples",  s_nPressureSamples);
        settings.setValue("NVortSamples",   s_nVorticitySamples);

        settings.setValue("AutoOmegaScale", s_bAutoOmegaScale);
        settings.setValue("LegendOmegaMin", s_OmegaMin);
        settings.setValue("LegendOmegaMax", s_OmegaMax);
        settings.setValue("OmegaDir",       s_OmegaDir);

    }
    settings.endGroup();
}


void CrossFlowCtrls::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            break;
        }
        case Qt::Key_Escape:
        {
            if(parent())
            {
                QWidget *pParentWt = dynamic_cast<QWidget*>(parent());
                pParentWt->hide();
                pEvent->accept();
            }
            break;
        }
        default:
            pEvent->ignore();
    }
}


void CrossFlowCtrls::makeXPlaneVelocityVectors()
{
    Polar3d const *pPolar3d = s_pXPlane->curPlPolar();
    Opp3d const *pOpp3d = s_pXPlane->curPOpp();
    if(!pOpp3d) return;

    double xmax(0.0);
    if(pOpp3d && pOpp3d->m_Vorton.size()) xmax = pOpp3d->m_Vorton.back().front().position().x;
    else                                  xmax = pPolar3d->wakeLength();
    m_pLabLen1->setText(QString::asprintf("%.2f", xmax*Units::mtoUnit()*1.1)+Units::lengthUnitQLabel());

    double xpos = xmax*(s_XPos*1.2-0.1);

    double cosa = 1.0;
    double sina = 0.0;
    m_GridNodesVel.resize(s_nVelocitySamples*s_nVelocitySamples);
    m_GridVectors.resize( s_nVelocitySamples*s_nVelocitySamples);

    for(int irow=0; irow<s_nVelocitySamples; irow++)
    {
        // rotate the plane to be perpendicular to the freestream flow
        double zpos = (double(irow)/double(s_nVelocitySamples-1)-0.5)*s_Height;
        double x = xpos*cosa - zpos*sina;
        double z = xpos*sina + zpos*cosa;
        for(int icol=0; icol<s_nVelocitySamples; icol++)
        {
            double y = (double(icol)/double(s_nVelocitySamples-1)-0.5)*s_Width*2;
            int idx = irow*s_nVelocitySamples + icol;
            m_GridNodesVel[idx].set(x, y, z);
        }
    }

    if(pOpp3d->isQuadMethod())
        m_pgl3dXPlaneView->computeP4VelocityVectors(pOpp3d, m_GridNodesVel, m_GridVectors, xfl::isMultiThreaded());
    else
        m_pgl3dXPlaneView->computeP3VelocityVectors(pOpp3d, m_GridNodesVel, m_GridVectors, xfl::isMultiThreaded());

    gl3dXPlaneView::s_bResetglGridVelocities = true;
    m_pgl3dXPlaneView->update();
}


void CrossFlowCtrls::makeXSailVelocityVectors()
{
    double xmax = 1.0;
    Polar3d const *pPolar3d = s_pXSail->curBtPolar();
    Opp3d const *pOpp3d = s_pXSail->curBtOpp();

    if(!pOpp3d) return;

    if(pOpp3d && pOpp3d->m_Vorton.size()) xmax = pOpp3d->m_Vorton.back().front().position().x;
    else                                  xmax = pPolar3d->wakeLength();
    m_pLabLen1->setText(QString::asprintf("%.2f", xmax*Units::mtoUnit()*1.1)+Units::lengthUnitQLabel());

    double xpos = xmax*(s_XPos*1.2-0.1);

    double cosb = cos(pOpp3d->beta()*PI/180.0);
    double sinb = sin(pOpp3d->beta()*PI/180.0);

    m_GridNodesVel.resize(s_nVelocitySamples*s_nVelocitySamples);
    m_GridVectors.resize( s_nVelocitySamples*s_nVelocitySamples);

    Vector3d VT;
    for(int irow=0; irow<s_nVelocitySamples; irow++)
    {
        // rotate the plane to be perpendicular to the freestream flow
        double z = double(irow)/double(s_nVelocitySamples-1)*s_Height -1;
        for(int icol=0; icol<s_nVelocitySamples; icol++)
        {
            double ypos = (double(icol)/double(s_nVelocitySamples-1)-0.5)*s_Width*2;
            double x = xpos*cosb - ypos*sinb;
            double y = xpos*sinb + ypos*cosb;
            int idx = irow*s_nVelocitySamples + icol;
            m_GridNodesVel[idx].set(x, y, z);
        }
    }

    m_pgl3dXSailView->computeP3VelocityVectors(pOpp3d, m_GridNodesVel, m_GridVectors);
    gl3dXSailView::s_bResetglGridVelocities = true;
    m_pgl3dXSailView->update();
}


void CrossFlowCtrls::makeOmegaMap()
{
    if(!m_bVorticityMap) return;
    if(!s_pXPlane) return;
    Opp3d const *pPOpp = s_pXPlane->curPOpp();
    if(!pPOpp) return;

    double xmax = 1.0;
    if(pPOpp && pPOpp->m_Vorton.size())
    {
        xmax = pPOpp->m_Vorton.back().front().position().x;
        m_pLabLen1->setText(QString::asprintf("%.2f", xmax*Units::mtoUnit()*1.1)+Units::lengthUnitQLabel());
    }

    double xpos = xmax*(s_XPos*1.2-0.1);

    double cosa = 1.0;
    double sina = 0.0;

    m_GridNodesOmega.resize(s_nVorticitySamples*s_nVorticitySamples);
    m_OmegaField.resize(s_nVorticitySamples*s_nVorticitySamples);

    if(xfl::isMultiThreaded())
    {
        QFutureSynchronizer<void> futureSync;
        for(int irow=0; irow<s_nVorticitySamples; irow++)
        {
            // rotate the plane to be perpendicular to the freestream flow
            double zpos = (double(irow)/double(s_nVorticitySamples-1)-0.5)*s_Height;
            double x = xpos*cosa - zpos*sina;
            double z = xpos*sina + zpos*cosa;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            futureSync.addFuture(QtConcurrent::run(this, &CrossFlowCtrls::makeVorticityRow,
                                                   irow, x, z, pPOpp));
#else
            futureSync.addFuture(QtConcurrent::run(&CrossFlowCtrls::makeVorticityRow, this,
                                                   irow, x, z, pPOpp));
#endif
        }
        futureSync.waitForFinished();
    }
    else
    {
        for(int irow=0; irow<s_nVorticitySamples; irow++)
        {
            // rotate the plane to be perpendicular to the freestream flow
            double zpos = (double(irow)/double(s_nVorticitySamples-1)-0.5)*s_Height;
            double x = xpos*cosa - zpos*sina;
            double z = xpos*sina + zpos*cosa;
            makeVorticityRow(irow, x, z, pPOpp);
        }
    }

    if(s_bAutoOmegaScale)
    {
        s_OmegaMin =  1e10;
        s_OmegaMax = -1e10;
        for(int idx=0; idx<m_OmegaField.size(); idx++)
        {
            s_OmegaMin = std::min(s_OmegaMin, m_OmegaField[idx]);
            s_OmegaMax = std::max(s_OmegaMax, m_OmegaField[idx]);
        }

        m_pdeOmegaMin->setValue(s_OmegaMin);
        m_pdeOmegaMax->setValue(s_OmegaMax);
        m_pgl3dXPlaneView->m_ColourLegend.setRange(s_OmegaMin*s_OmegaCoef, s_OmegaMax*s_OmegaCoef);
    }
}


void CrossFlowCtrls::makePressureRow(int irow, double x, double z, PlanePolar const *pWPolar, Opp3d const *pOpp3d)
{
    for(int icol=0; icol<s_nPressureSamples; icol++)
    {
        double y = (double(icol)/double(s_nPressureSamples-1)-0.5)*s_Width*2;

        int idx = irow*s_nPressureSamples + icol;
         m_GridNodesPressure[idx].set(x, y, z);
    }

    QVector<Vector3d> GridVectors;
    if(pOpp3d->isQuadMethod())
        m_pgl3dXPlaneView->computeP4VelocityVectors(pOpp3d, m_GridNodesPressure, GridVectors, false);
    else
        m_pgl3dXPlaneView->computeP3VelocityVectors(pOpp3d, m_GridNodesPressure, GridVectors, false);

    // compute pressure from Bernouilli's equation
    double qDyn = 1.0/2.0 * pWPolar->density() * pOpp3d->QInf()*pOpp3d->QInf();
    Vector3d VInf(pOpp3d->QInf(),0,0);
    for(int icol=0; icol<s_nPressureSamples; icol++)
    {
        int idx = irow*s_nPressureSamples + icol;
        double vel = (VInf + GridVectors.at(idx)).norm();
        m_PressureField[idx] = qDyn - 0.5 * pWPolar->density() * vel * vel;
    }
}


void CrossFlowCtrls::makeVorticityRow(int irow, double x, double z, Opp3d const *pOpp3d)
{
    Vector3d omega, omp;
    for(int icol=0; icol<s_nVorticitySamples; icol++)
    {
        double y = (double(icol)/double(s_nVorticitySamples-1)-0.5)*s_Width*2;

        int idx = irow*s_nVorticitySamples + icol;
        m_GridNodesOmega[idx].set(x, y, z);

        Vector3d const &pt = m_GridNodesOmega.at(idx);

        // from the vortons
        omega.set(0,0,0);
        for(uint ir=0; ir<pOpp3d->m_Vorton.size(); ir++)
        {
            for(uint jc=0; jc<pOpp3d->m_Vorton.at(ir).size(); jc++)
            {
                Vorton const &vtn = pOpp3d->m_Vorton.at(ir).at(jc);
                omp = vtn.vorticity(pt);
                omega += omp;
            }
        }

        switch (s_OmegaDir)
        {
            case 0:  m_OmegaField[idx] = omega.x;       break;
            case 1:  m_OmegaField[idx] = omega.y;       break;
            case 2:  m_OmegaField[idx] = omega.z;       break;
            default: m_OmegaField[idx] = omega.norm();  break;
        }
    }
}


void CrossFlowCtrls::onOmegaScale()
{
    s_bAutoOmegaScale = m_pchAutoOmegaScale->isChecked();
    s_OmegaMin = m_pdeOmegaMin->value();
    s_OmegaMax = m_pdeOmegaMax->value();

    m_pdeOmegaMin->setEnabled(!s_bAutoOmegaScale);
    m_pdeOmegaMax->setEnabled(!s_bAutoOmegaScale);

    m_pgl3dXPlaneView->m_ColourLegend.setRange(s_OmegaMin*s_OmegaCoef, s_OmegaMax*s_OmegaCoef);
    onMakeCrossFlowPlane();
}


void CrossFlowCtrls::onMakeCrossFlowPlane()
{
    readWakeData();
    enableWakeControls();

    if(m_bGridVelocity)
    {
        if      (MainFrame::xflApp()==xfl::XPLANE) makeXPlaneVelocityVectors();
        else if (MainFrame::xflApp()==xfl::XSAIL)  makeXSailVelocityVectors();
    }

    if(m_bVorticityMap)
    {
        makeOmegaMap();
        m_pgl3dXPlaneView->s_bResetglVorticity = true;
    }

    m_pgl3dXPlaneView->update();
}
