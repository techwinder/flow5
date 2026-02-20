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


#define _MATH_DEFINES_DEFINED

#include <QVBoxLayout>
#include <QGroupBox>

#include "opp3dscalesctrls.h"


#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <interfaces/widgets/customwts/exponentialslider.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <modules/xsail/view/gl3dxsailview.h>
#include <core/xflcore.h>

#include <api/units.h>
#include <api/utils.h>



XPlane *Opp3dScalesCtrls::s_pXPlane(nullptr);
XSail *Opp3dScalesCtrls::s_pXSail(nullptr);

bool Opp3dScalesCtrls::s_bAutoGammaScale = true;
double Opp3dScalesCtrls::s_GammaMin = -1.0;
double Opp3dScalesCtrls::s_GammaMax =  1.0;

bool Opp3dScalesCtrls::s_bAutoCpScale = true;
double Opp3dScalesCtrls::s_CpMin = -1.0;
double Opp3dScalesCtrls::s_CpMax =  1.0;

bool Opp3dScalesCtrls::s_bAutoPressureScale = true;
double Opp3dScalesCtrls::s_PressureMin = -1.0;
double Opp3dScalesCtrls::s_PressureMax =  1.0;


double Opp3dScalesCtrls::s_PanelForceScale = 1.0;
double Opp3dScalesCtrls::s_LiftScale       = 1.0;
double Opp3dScalesCtrls::s_PartForceScale  = 1.0;
double Opp3dScalesCtrls::s_MomentScale     = 1.0;
double Opp3dScalesCtrls::s_DragScale       = 1.0;


Opp3dScalesCtrls::Opp3dScalesCtrls(QWidget *parent) : QWidget{parent}
{
    m_pgl3dXPlaneView = nullptr;
    m_pgl3dXSailView = nullptr;

    setupLayout();
    connectSignals();
}

void Opp3dScalesCtrls::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGroupBox *pgbCpScales = new QGroupBox(tr("Colour scales"));
        {
            QGridLayout *pCpScaleLayout = new QGridLayout;
            {
                QLabel *plabGamma    = new QLabel(GAMMAch);
                QLabel *plabCp       = new QLabel("Cp");
                QLabel *plabPressure = new QLabel("Pressure");
                plabGamma->setAlignment(   Qt::AlignCenter);
                plabCp->setAlignment(      Qt::AlignCenter);
                plabPressure->setAlignment(Qt::AlignCenter);
                m_pchAutoGammaScale    = new QCheckBox("Auto");
                m_pchAutoCpScale       = new QCheckBox("Auto");
                m_pchAutoPressureScale = new QCheckBox("Auto");
                m_pfeGammaMin    = new FloatEdit(-1.0f);
                m_pfeGammaMax    = new FloatEdit( 1.0f);
                m_pfeCpMin       = new FloatEdit(-1.0f);
                m_pfeCpMax       = new FloatEdit( 1.0f);
                m_pfePressureMin = new FloatEdit(-1.0f);
                m_pfePressureMax = new FloatEdit( 1.0f);

                m_plabPressureUnit1 = new QLabel(Units::pressureUnitQLabel());
                m_plabPressureUnit2 = new QLabel(Units::pressureUnitQLabel());
                QLabel *plabMin = new QLabel("Min:");
                QLabel *plabMax = new QLabel("Max:");

                pCpScaleLayout->addWidget(plabGamma,                2, 2);
                pCpScaleLayout->addWidget(plabCp,                   2, 3);
                pCpScaleLayout->addWidget(plabPressure,             2, 4);
                pCpScaleLayout->addWidget(m_pchAutoGammaScale,      3, 2);
                pCpScaleLayout->addWidget(m_pchAutoCpScale,         3, 3);
                pCpScaleLayout->addWidget(m_pchAutoPressureScale,   3, 4);
                pCpScaleLayout->addWidget(plabMax,                  4, 1, Qt::AlignVCenter |Qt::AlignRight);
                pCpScaleLayout->addWidget(m_pfeGammaMax,            4, 2);
                pCpScaleLayout->addWidget(m_pfeCpMax,               4, 3);
                pCpScaleLayout->addWidget(m_pfePressureMax,         4, 4);
                pCpScaleLayout->addWidget(m_plabPressureUnit1,      4, 5);
                pCpScaleLayout->addWidget(plabMin,                  5, 1, Qt::AlignVCenter |Qt::AlignRight);
                pCpScaleLayout->addWidget(m_pfeGammaMin,            5, 2);
                pCpScaleLayout->addWidget(m_pfeCpMin,               5, 3);
                pCpScaleLayout->addWidget(m_pfePressureMin,         5, 4);
                pCpScaleLayout->addWidget(m_plabPressureUnit2,      5, 5);
                pCpScaleLayout->setColumnStretch(1,1);
                pCpScaleLayout->setColumnStretch(5,1);
            }
            pgbCpScales->setLayout(pCpScaleLayout);
        }

        QGroupBox *pScaleBox = new QGroupBox(tr("Display length scales"));
        {
            //        m_pScaleBox->setPalette(palette);
            pScaleBox->setAutoFillBackground(true);
            QGridLayout *pScaleLayout = new QGridLayout;
            {
                m_pesPanelForce= new ExponentialSlider(false, 7.0, Qt::Horizontal);
                m_pesPanelForce->setMinimum(0);
                m_pesPanelForce->setMaximum(10000);
                m_pesPanelForce->setSliderPosition(5000);
                m_pesPanelForce->setTickInterval(500);
                m_pesPanelForce->setTickPosition(QSlider::TicksBelow);

                m_pesLiftScale  = new ExponentialSlider(false, 7.0, Qt::Horizontal);
                m_pesLiftScale->setMinimum(0);
                m_pesLiftScale->setMaximum(10000);
                m_pesLiftScale->setSliderPosition(5000);
                m_pesLiftScale->setTickInterval(500);
                m_pesLiftScale->setTickPosition(QSlider::TicksBelow);

                m_pesPartForce  = new ExponentialSlider(false, 7.0, Qt::Horizontal);
                m_pesPartForce->setMinimum(0);
                m_pesPartForce->setMaximum(10000);
                m_pesPartForce->setSliderPosition(5000);
                m_pesPartForce->setTickInterval(500);
                m_pesPartForce->setTickPosition(QSlider::TicksBelow);

                m_pesMoment  = new ExponentialSlider(false, 7.0, Qt::Horizontal);
                m_pesMoment->setMinimum(0);
                m_pesMoment->setMaximum(1000);
                m_pesMoment->setSliderPosition(500);
                m_pesMoment->setTickInterval(50);
                m_pesMoment->setTickPosition(QSlider::TicksBelow);

                m_pesDrag = new ExponentialSlider(false, 7.0, Qt::Horizontal);
                m_pesDrag->setMinimum(0);
                m_pesDrag->setMaximum(1000);
                m_pesDrag->setSliderPosition(500);
                m_pesDrag->setTickInterval(50);
                m_pesDrag->setTickPosition(QSlider::TicksBelow);

                m_pesVelocity  = new ExponentialSlider(false, 7.0, Qt::Horizontal);
                m_pesVelocity->setMinimum(0);
                m_pesVelocity->setMaximum(1000);
                m_pesVelocity->setSliderPosition(500);
                m_pesVelocity->setTickInterval(50);
                m_pesVelocity->setTickPosition(QSlider::TicksBelow);

                QLabel *pLab1 = new QLabel("Part forces:");
                QLabel *pLab2 = new QLabel("Panel forces:");
                QLabel *pLab3 = new QLabel("Strip lift:");
                QLabel *pLab4 = new QLabel("Moments:");
                QLabel *pLab5 = new QLabel("Drag:");
                QLabel *pLab6 = new QLabel("Velocity:");

                pScaleLayout->addWidget(pLab1,             2, 1, Qt::AlignVCenter | Qt::AlignRight);
                pScaleLayout->addWidget(pLab2,             3, 1, Qt::AlignVCenter | Qt::AlignRight);
                pScaleLayout->addWidget(pLab3,             4, 1, Qt::AlignVCenter | Qt::AlignRight);
                pScaleLayout->addWidget(pLab4,             5, 1, Qt::AlignVCenter | Qt::AlignRight);
                pScaleLayout->addWidget(pLab5,             6, 1, Qt::AlignVCenter | Qt::AlignRight);
                pScaleLayout->addWidget(pLab6,             7, 1, Qt::AlignVCenter | Qt::AlignRight);
                pScaleLayout->addWidget(m_pesPartForce,    2, 2);
                pScaleLayout->addWidget(m_pesPanelForce,   3, 2);
                pScaleLayout->addWidget(m_pesLiftScale,    4, 2);
                pScaleLayout->addWidget(m_pesMoment,       5, 2);
                pScaleLayout->addWidget(m_pesDrag,         6, 2);
                pScaleLayout->addWidget(m_pesVelocity,     7, 2);
            }
            pScaleBox->setLayout(pScaleLayout);
        }

        pMainLayout->addWidget(pgbCpScales);
        pMainLayout->addWidget(pScaleBox);
        pMainLayout->addStretch();
    }
    setLayout(pMainLayout);
}


void Opp3dScalesCtrls::connectSignals()
{
    connect(m_pchAutoGammaScale,    SIGNAL(clicked()),             SLOT(onGammaScale()));
    connect(m_pfeGammaMin,          SIGNAL(floatChanged(float)),   SLOT(onGammaScale()));
    connect(m_pfeGammaMax,          SIGNAL(floatChanged(float)),   SLOT(onGammaScale()));
    connect(m_pchAutoCpScale,       SIGNAL(clicked()),             SLOT(onCpScale()));
    connect(m_pfeCpMin,             SIGNAL(floatChanged(float)),   SLOT(onCpScale()));
    connect(m_pfeCpMax,             SIGNAL(floatChanged(float)),   SLOT(onCpScale()));
    connect(m_pchAutoPressureScale, SIGNAL(clicked()),             SLOT(onPressureScale()));
    connect(m_pfePressureMin,       SIGNAL(floatChanged(float)),   SLOT(onPressureScale()));
    connect(m_pfePressureMax,       SIGNAL(floatChanged(float)),   SLOT(onPressureScale()));

    connect(m_pesPanelForce,        SIGNAL(sliderMoved(int)),      SLOT(onPanelForceScale()));
    connect(m_pesLiftScale,         SIGNAL(sliderMoved(int)),      SLOT(onLiftScale()));
    connect(m_pesPartForce,         SIGNAL(sliderMoved(int)),      SLOT(onPartForceScale()));
    connect(m_pesMoment,            SIGNAL(sliderMoved(int)),      SLOT(onMomentScale()));
    connect(m_pesDrag,              SIGNAL(sliderMoved(int)),      SLOT(onDragScale()));
    connect(m_pesVelocity,          SIGNAL(sliderMoved(int)),      SLOT(onVelocityScale()));
}


void Opp3dScalesCtrls::set3dXSailView(gl3dXSailView *pView)
{
    m_pgl3dXSailView = pView;
    m_pgl3dXSailView->m_pOpp3dScalesCtrls = this;
}


void Opp3dScalesCtrls::loadSettings(QSettings &settings)
{
    settings.beginGroup("Opp3dScalesCtrls");
    {
        s_PanelForceScale    = settings.value("PanelForceScale",   s_PanelForceScale).toDouble();
        s_LiftScale          = settings.value("LiftScale",         s_LiftScale).toDouble();
        s_PartForceScale     = settings.value("PartForceScale",    s_PartForceScale).toDouble();
        s_MomentScale        = settings.value("MomentScale",       s_MomentScale).toDouble();
        s_DragScale          = settings.value("DragScale",         s_DragScale).toDouble();
        gl3dXflView::setVelocityScale(settings.value("VelocityScale", gl3dXflView::velocityScale()).toDouble());

        s_bAutoGammaScale    = settings.value("AutoGammaScale",    s_bAutoGammaScale).toBool();
        s_GammaMin           = settings.value("GammaMin",          s_GammaMin).toDouble();
        s_GammaMax           = settings.value("GammaMax",          s_GammaMax).toDouble();
        s_bAutoCpScale       = settings.value("AutoCpScale",       s_bAutoCpScale).toBool();
        s_CpMin              = settings.value("LegendCpMin",       s_CpMin).toDouble();
        s_CpMax              = settings.value("LegendCpMax",       s_CpMax).toDouble();
        s_bAutoPressureScale = settings.value("AutoPressureScale", s_bAutoPressureScale).toBool();
        s_PressureMin        = settings.value("LegendPressureMin", s_PressureMin).toDouble();
        s_PressureMax        = settings.value("LegendPressureMax", s_PressureMax).toDouble();

    }
    settings.endGroup();
}


void Opp3dScalesCtrls::saveSettings(QSettings &settings)
{
    settings.beginGroup("Opp3dScalesCtrls");
    {
        settings.setValue("PanelForceScale",  s_PanelForceScale);
        settings.setValue("LiftScale",        s_LiftScale);
        settings.setValue("PartForceScale",   s_PartForceScale);
        settings.setValue("MomentScale",      s_MomentScale);
        settings.setValue("DragScale",        s_DragScale);
        settings.setValue("VelocityScale", gl3dXflView::velocityScale());

        settings.setValue("AutoGammaScale",    s_bAutoGammaScale);
        settings.setValue("GammaMin",          s_GammaMin);
        settings.setValue("GammMax",           s_GammaMax);
        settings.setValue("AutoCpScale",       s_bAutoCpScale);
        settings.setValue("LegendCpMin",       s_CpMin);
        settings.setValue("LegendCpMax",       s_CpMax);
        settings.setValue("AutoPressureScale", s_bAutoPressureScale);
        settings.setValue("LegendPressureMin", s_PressureMin);
        settings.setValue("LegendPressureMax", s_PressureMax);
    }
    settings.endGroup();
}


void Opp3dScalesCtrls::initWidget()
{
    updateUnits();

    m_pchAutoGammaScale->setChecked(s_bAutoGammaScale);
    m_pfeGammaMin->setValue(s_GammaMin);
    m_pfeGammaMax->setValue(s_GammaMax);
    m_pfeGammaMin->setEnabled(!s_bAutoGammaScale);
    m_pfeGammaMax->setEnabled(!s_bAutoGammaScale);

    m_pchAutoCpScale->setChecked(s_bAutoCpScale);
    m_pfeCpMin->setValue(s_CpMin);
    m_pfeCpMax->setValue(s_CpMax);
    m_pfeCpMin->setEnabled(!s_bAutoCpScale);
    m_pfeCpMax->setEnabled(!s_bAutoCpScale);

    m_pchAutoPressureScale->setChecked(s_bAutoPressureScale);

    m_pfePressureMin->setEnabled(!s_bAutoPressureScale);
    m_pfePressureMax->setEnabled(!s_bAutoPressureScale);

    m_pesPanelForce->setExpValue(s_PanelForceScale);
    m_pesLiftScale->setExpValue(s_LiftScale);
    m_pesPartForce->setExpValue(s_PartForceScale);
    m_pesMoment->setExpValue(s_MomentScale);
    m_pesDrag->setExpValue(s_DragScale);
    m_pesVelocity->setExpValue(gl3dXflView::velocityScale()*100.0);
}



void Opp3dScalesCtrls::updateUnits()
{
    QString str = Units::pressureUnitQLabel();
    m_plabPressureUnit1->setText(str);
    m_plabPressureUnit2->setText(str);
    m_pfePressureMin->setValue(s_PressureMin*Units::PatoUnit());
    m_pfePressureMax->setValue(s_PressureMax*Units::PatoUnit());
}


void Opp3dScalesCtrls::updateGammaRange(float GMin, float GMax)
{
    s_GammaMin = double(GMin);
    s_GammaMax = double(GMax);
    m_pfeGammaMin->setValue(s_GammaMin);
    m_pfeGammaMax->setValue(s_GammaMax);
}


void Opp3dScalesCtrls::updateCpRange(float CpMin, float CpMax)
{
    s_CpMin = double(CpMin);
    s_CpMax = double(CpMax);
    m_pfeCpMin->setValue(s_CpMin);
    m_pfeCpMax->setValue(s_CpMax);
}


void Opp3dScalesCtrls::updatePressureRange(float PressureMin, float PressureMax)
{
    s_PressureMin = double(PressureMin);
    s_PressureMax = double(PressureMax);
    m_pfePressureMin->setValue(s_PressureMin*Units::PatoUnit());
    m_pfePressureMax->setValue(s_PressureMax*Units::PatoUnit());
}


void Opp3dScalesCtrls::onGammaScale()
{
    s_bAutoGammaScale = m_pchAutoGammaScale->isChecked();
    s_GammaMax = m_pfeGammaMax->value();
    s_GammaMin = m_pfeGammaMin->value();
    m_pfeGammaMin->setEnabled(!s_bAutoGammaScale);
    m_pfeGammaMax->setEnabled(!s_bAutoGammaScale);

    emit update3dScales();
}


void Opp3dScalesCtrls::onCpScale()
{
    s_bAutoCpScale = m_pchAutoCpScale->isChecked();
    s_CpMax = m_pfeCpMax->value();
    s_CpMin = m_pfeCpMin->value();
    m_pfeCpMin->setEnabled(!s_bAutoCpScale);
    m_pfeCpMax->setEnabled(!s_bAutoCpScale);

    emit update3dScales();
}


void Opp3dScalesCtrls::onPressureScale()
{
    s_bAutoPressureScale = m_pchAutoPressureScale->isChecked();
    s_PressureMax = m_pfePressureMax->value()/Units::PatoUnit();
    s_PressureMin = m_pfePressureMin->value()/Units::PatoUnit();
    m_pfePressureMin->setEnabled(!s_bAutoPressureScale);
    m_pfePressureMax->setEnabled(!s_bAutoPressureScale);

    emit update3dScales();
}

void Opp3dScalesCtrls::onPanelForceScale()
{
    s_PanelForceScale    = m_pesPanelForce->expValue();
    emit update3dScales();
}


void Opp3dScalesCtrls::onPartForceScale()
{
    s_PartForceScale    = m_pesPartForce->expValue();
    emit update3dScales();
}


void Opp3dScalesCtrls::onLiftScale()
{
    s_LiftScale    = m_pesLiftScale->expValue();
    emit update3dScales();
}


void Opp3dScalesCtrls::onMomentScale()
{
    s_MomentScale    = m_pesMoment->expValue();
    emit update3dScales();
}


void Opp3dScalesCtrls::onDragScale()
{
    s_DragScale    = m_pesDrag->expValue();
    emit update3dScales();
}


void Opp3dScalesCtrls::onVelocityScale()
{
    gl3dXflView::setVelocityScale(m_pesVelocity->expValue()/100.0);
    emit update3dScales();
}
