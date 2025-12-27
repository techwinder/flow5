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

#include <QHBoxLayout>
#include <QButtonGroup>


#include "stab3dctrls.h"

#include <api/constants.h>
#include <api/geom_global.h>
#include <api/planeopp.h>
#include <api/planexfl.h>
#include <api/units.h>
#include <api/utils.h>
#include <api/planepolar.h>
#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <modules/xplane/glview/gl3dxplaneview.h>
#include <modules/xplane/xplane.h>


XPlane *Stab3dCtrls::s_pXPlane=nullptr;
double Stab3dCtrls::s_dt(0.001);
double Stab3dCtrls::s_Amplitude(0.5);

Stab3dCtrls::Stab3dCtrls(QWidget *pParent) : QWidget(pParent)
{
    setWindowTitle("Stability 3d controls");

    m_iActiveMode = -1;

    memset(m_vabs, 0, sizeof(m_vabs));
    memset(m_phi,  0, sizeof(m_phi));

    m_ModeNormFactor = 1.0;
    m_ModeElapsedTime = 0.0;
    memset(m_ModeState, 0, 6*sizeof(double));

    setupLayout();
    connectSignals();
}


void Stab3dCtrls::setupLayout()
{
    QVBoxLayout *pStabLayout = new QVBoxLayout;
    {
        QGridLayout *pModeLayout = new QGridLayout;
        {
            QButtonGroup *pGroup = new QButtonGroup;
            {
                m_prbNoMode = new QRadioButton("No mode");
                pGroup->addButton(m_prbNoMode);

                for(int imode=0; imode<NMODES; imode++)
                {
                    m_prbMode[imode] = new QRadioButton(QString::asprintf("%d", imode%4+1));
                    pGroup->addButton(m_prbMode[imode]);
                }
            }

            QLabel *plabLong = new QLabel("Longitudinal mode:");
            QLabel *plabLat  = new QLabel("Lateral mode:");

            pModeLayout->addWidget(m_prbNoMode,  1, 3, 1, 4);
            pModeLayout->addWidget(plabLong,     2, 1);
            for(int imode=0; imode<4; imode++)
                pModeLayout->addWidget(m_prbMode[imode],   2, imode+3);
            pModeLayout->addWidget(plabLat,      3, 1);
            for(int imode=0; imode<4; imode++)
                pModeLayout->addWidget(m_prbMode[imode+4], 3, imode+3);

            pModeLayout->setColumnStretch(1,1);
            pModeLayout->setColumnStretch(2,1);
            pModeLayout->setColumnStretch(7,1);
        }

        QHBoxLayout *pAmplitudeLayout = new QHBoxLayout;
        {
            QLabel *plabAmplitude = new QLabel("Amplitude:");

            m_pslAnimAmplitude = new QSlider(Qt::Horizontal);
            m_pslAnimAmplitude->setToolTip("<p>Sets an arbitrary normalization factor used to display the mode state.<br>"
                                           "The 100% factor is calculated so that:"
                                           "<ul>"
                                           "<li>the maximum displacement does not exceed 20% of the span length,</li>"
                                           "<li>the maximum rotation around any axis does not exceed 20&deg;.</li>"
                                           "</ul></p>");
            m_pslAnimAmplitude->setRange(0, 100);
            m_pslAnimAmplitude->setTickInterval(10);
            m_pslAnimAmplitude->setTickPosition(QSlider::TicksBelow);
            m_pslAnimAmplitude->setSliderPosition(int(s_Amplitude*100.0));

            QLabel*plab0   = new QLabel("0%");
            QLabel*plab100 = new QLabel("100%");

            pAmplitudeLayout->addWidget(plabAmplitude);
            pAmplitudeLayout->addSpacing(13);
            pAmplitudeLayout->addWidget(plab0);
            pAmplitudeLayout->addWidget(m_pslAnimAmplitude);
            pAmplitudeLayout->addWidget(plab100);
        }

        QHBoxLayout *pAnimationCommandsLayout = new QHBoxLayout;
        {
            m_ppbAnimate = new QPushButton("Animate");
            m_ppbAnimate->setCheckable(true);
            m_ppbAnimateRestart = new QPushButton("Restart");
            pAnimationCommandsLayout->addWidget(m_ppbAnimate);
            pAnimationCommandsLayout->addWidget(m_ppbAnimateRestart);
        }

        QHBoxLayout *pStepLayout = new  QHBoxLayout;
        {
            m_pfeModeStep = new FloatEdit(s_dt);
            m_pfeModeStep->setToolTip("<p>The time increment used to solve numerically the differential equations of motion. "
                                      "The display is updated after each time increment."
                                      "</p>");
            QLabel *pStepLabel = new QLabel("Time step:");
            QLabel *pStepUnit  = new QLabel("s");
            pStepLayout->addWidget(pStepLabel);
            pStepLayout->addWidget(m_pfeModeStep);
            pStepLayout->addWidget(pStepUnit);
            pStepLayout->addStretch();
        }

        QFont fixedfont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        m_plabdof = new QLabel;
        m_plabdof->setFont(fixedfont);

        pStabLayout->addLayout(pModeLayout);
        pStabLayout->addLayout(pAmplitudeLayout);
        pStabLayout->addLayout(pStepLayout);
        pStabLayout->addStretch();
        pStabLayout->addWidget(m_plabdof);
        pStabLayout->addLayout(pAnimationCommandsLayout);
    }

    setLayout(pStabLayout);
}


void Stab3dCtrls::connectSignals()
{    
    connect(m_prbNoMode, SIGNAL(clicked()), SLOT(onModeSelection()));
    for(int imode=0; imode<NMODES; imode++)
        connect(m_prbMode[imode], SIGNAL(clicked()), SLOT(onModeSelection()));

    connect(m_ppbAnimate,           SIGNAL(clicked()),             SLOT(onAnimateMode()));
    connect(m_pslAnimAmplitude,     SIGNAL(sliderMoved(int)),      SLOT(onAnimationAmplitude(int)));
    connect(m_ppbAnimateRestart,    SIGNAL(clicked()),             SLOT(onAnimateRestart()));
}


void Stab3dCtrls::loadSettings(QSettings &settings)
{
    settings.beginGroup("Stab3dCtrls");
    {
        s_dt  = settings.value("TimeStep",  s_dt).toDouble();
        s_Amplitude = settings.value("Amplitude", s_Amplitude).toDouble();
    }
    settings.endGroup();
}


void Stab3dCtrls::saveSettings(QSettings &settings)
{
    settings.beginGroup("Stab3dCtrls");
    {
        settings.setValue("TimeStep",  s_dt);
        settings.setValue("Amplitude", s_Amplitude);
    }
    settings.endGroup();

}


void Stab3dCtrls::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            if(m_ppbAnimate->isChecked())
            {
                stopAnimate();
                break;
            }
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
    QWidget::keyPressEvent(pEvent);
}


void Stab3dCtrls::setControls()
{
    m_prbNoMode->setChecked(m_iActiveMode<0);
    for(int imode=0; imode<NMODES; imode++)
        m_prbMode[imode]->setChecked(m_iActiveMode==imode);

    m_pslAnimAmplitude->setSliderPosition(int(s_Amplitude*100.0));
    m_pfeModeStep->setValue(s_dt);

    updateDof();
}


void Stab3dCtrls::onModeSelection()
{
    if(m_prbNoMode->isChecked())
    {
        m_iActiveMode = -1;
        stopAnimate();
    }
    else
    {
        for(int imode=0; imode<NMODES; imode++)
        {
            if(m_prbMode[imode]->isChecked())
            {
                m_iActiveMode = imode;
                break;
            }
        }
    }
    setMode(s_pXPlane->curPOpp());
}


void Stab3dCtrls::setMode(PlaneOpp const *pPOpp)
{
    if(!pPOpp || !pPOpp->isType7() || m_iActiveMode<0)
    {
        m_vabs[0] = m_vabs[1] = m_vabs[2] = m_vabs[3] = 0.0;
        m_phi[0]  = m_phi[1]  = m_phi[2]  = m_phi[3]  = 0.0;
    }
    else
    {
        m_vabs[0] = std::abs(pPOpp->m_EigenVector[m_iActiveMode][0]);
        m_vabs[1] = std::abs(pPOpp->m_EigenVector[m_iActiveMode][1]);
        m_vabs[2] = std::abs(pPOpp->m_EigenVector[m_iActiveMode][2]);
        m_vabs[3] = std::abs(pPOpp->m_EigenVector[m_iActiveMode][3]);
        m_phi[0]  = std::arg(pPOpp->m_EigenVector[m_iActiveMode][0]);
        m_phi[1]  = std::arg(pPOpp->m_EigenVector[m_iActiveMode][1]);
        m_phi[2]  = std::arg(pPOpp->m_EigenVector[m_iActiveMode][2]);
        m_phi[3]  = std::arg(pPOpp->m_EigenVector[m_iActiveMode][3]);
    }

    onAnimateRestart();
}


void Stab3dCtrls::onAnimateMode()
{
    if(!s_pXPlane || !s_pXPlane->is3dView()) return;

    if(m_ppbAnimate->isChecked())
    {
        s_dt = m_pfeModeStep->value();
        s_pXPlane->m_pTimerMode->start(1);
    }
    else
    {
        s_pXPlane->stopAnimateMode();
    }
}


void Stab3dCtrls::onAnimationAmplitude(int val)
{
    if(!s_pXPlane) return;
    s_Amplitude = double(val)/100.0;
    s_pXPlane->onAnimateModeSingle(false);
}


void Stab3dCtrls::onAnimateRestart()
{
    PlaneOpp const *pPOpp = s_pXPlane->curPOpp();
    Plane const *pPlane = s_pXPlane->m_pCurPlane;

    m_ModeNormFactor=1.0;
    m_ModeElapsedTime = 0.0;

    if(!pPOpp || !pPlane)
    {
        m_ModeState[0] = 0.0;
        m_ModeState[1] = 0.0;
        m_ModeState[2] = 0.0;
        m_ModeState[3] = 0.0;
        m_ModeState[4] = 0.0;
        m_ModeState[5] = 0.0;
        s_pXPlane->updateView();
        return;
    }

    getPosition(pPOpp, m_iActiveMode, 0.0);

    //max 20% span
    double norm1 = std::max(fabs(m_ModeState[0]), fabs(m_ModeState[1]));
    norm1 = std::max(norm1, fabs(m_ModeState[2]));
    if(norm1>PRECISION)  norm1 = pPlane->planformSpan() *.20 / norm1;
    else                 norm1 = 1.0;

    //max 20°
    double norm2 = std::max(fabs(m_ModeState[3]), fabs(m_ModeState[4]));
    norm2 = std::max(norm2, fabs(m_ModeState[5]));
    if(norm2>PRECISION)  norm2 = PI*(20.0/180.0)/ norm2;
    else                 norm2 = 1.0;

    m_ModeNormFactor = std::min(norm1, norm2);
    //set initial mode positions, i.e. t=0
    s_pXPlane->onAnimateModeSingle(false);
}


void Stab3dCtrls::incrementTime(PlaneOpp const*pPOpp, bool bStep)
{
    int iMode = m_iActiveMode;

    if(m_ModeElapsedTime>=1000)
    {
        stopAnimate(); // safety limit
        return;
    }

    //read the data
    s_dt = m_pfeModeStep->value();

    getPosition(pPOpp, iMode, m_ModeElapsedTime);

    for(int i=0; i<6; i++) m_ModeState[i] *= m_ModeNormFactor * s_Amplitude;

    //increase the time for the next update
    if(bStep) m_ModeElapsedTime += s_dt;

    updateDof();
}


void Stab3dCtrls::getPosition(PlaneOpp const*pPOpp, int iMode, double t)
{
    if(iMode<0 || iMode>7)
    {
        memset(m_ModeState, 0, 6*sizeof(double));
        return;
    }

    double const *vabs = m_vabs;
    double const *phi  = m_phi;

    // calculate the new state
    double sigma = pPOpp->m_EigenValue[iMode].real();
    double omega = pPOpp->m_EigenValue[iMode].imag();
    double s2 = sigma*sigma;
    double o2 = omega*omega;

    if(t>=1000) stopAnimate(); // safety limit

    if(s2+o2>PRECISION)
    {
        if(iMode<4) // longitudinal
        {
            //x, z, theta are evaluated by direct integration of u, w, q
            m_ModeState[1] = 0.0;
            m_ModeState[3] = 0.0;
            m_ModeState[5] = 0.0;
            m_ModeState[0] = vabs[0]*exp(sigma*t)/(s2+o2) * (sigma*cos(omega*t+phi[0])+omega*sin(omega*t+phi[0]));
            m_ModeState[2] = vabs[1]*exp(sigma*t)/(s2+o2) * (sigma*cos(omega*t+phi[1])+omega*sin(omega*t+phi[1]));
            m_ModeState[4] = vabs[2]*exp(sigma*t)/(s2+o2) * (sigma*cos(omega*t+phi[2])+omega*sin(omega*t+phi[2]));
            //        m_ModeState[4] = norm*vabs[3]*exp(sigma*t)*cos(omega*t+phi[3]);

            //add u0 x theta_sum to z component
            double theta_sum = vabs[3]*exp(sigma*t)/(s2+o2) * (sigma*cos(omega*t+phi[3])+omega*sin(omega*t+phi[3]));
            m_ModeState[2] -= theta_sum *pPOpp->QInf();
        }
        else
        {
            //y, phi, psi evaluation
            m_ModeState[0] = 0.0;
            m_ModeState[2] = 0.0;
            m_ModeState[4] = 0.0;

            // integrate (v+u0.psi.cos(theta0)) to get y
            m_ModeState[1] = vabs[0]*exp(sigma*t)/(s2+o2) * (sigma*cos(omega*t+phi[0])+omega*sin(omega*t+phi[0]));

            //integrate psi = integrate twice r (thanks Matlab !)
            double psi_sum =   sigma * ( sigma * cos(omega*t+phi[2]) + omega * sin(omega*t+phi[2]))
                    + omega * (-omega * cos(omega*t+phi[2]) + sigma * sin(omega*t+phi[2]));
            psi_sum *= vabs[2] * exp(sigma*t)/(s2+o2)/(s2+o2);

            m_ModeState[1] += pPOpp->QInf() * psi_sum;

            // get directly phi from fourth eigenvector component (alternatively integrate p+r.tan(theta0));
            m_ModeState[3] = vabs[3]*exp(sigma*t)*cos(omega*t+phi[3]);

            // integrate once 'p+r.sin(theta0)' to get heading angle
            m_ModeState[5] = vabs[2]*exp(sigma*t)/(s2+o2) * (sigma*cos(omega*t+phi[2])+omega*sin(omega*t+phi[2]));
        }
    }
    else
    {
        //something went wrong somewhere
        memset(m_ModeState, 0, 6*sizeof(double));
    }
}


void Stab3dCtrls::updateDof()
{
    QString strange;
    strange   = QString::asprintf("x  = %9.5f ", m_ModeState[0] * Units::mtoUnit()) + Units::lengthUnitQLabel() + EOLch;
    strange  += QString::asprintf("y  = %9.5f ", m_ModeState[1] * Units::mtoUnit()) + Units::lengthUnitQLabel() + EOLch;
    strange  += QString::asprintf("z  = %9.5f ", m_ModeState[2] * Units::mtoUnit()) + Units::lengthUnitQLabel() + EOLch;
    strange  += QString::asprintf("rx = %9.5f",  m_ModeState[3]*180.0/PI) + DEGch + EOLch;
    strange  += QString::asprintf("ry = %9.5f",  m_ModeState[4]*180.0/PI) + DEGch + EOLch;
    strange  += QString::asprintf("rz = %9.5f",  m_ModeState[5]*180.0/PI) + DEGch + EOLch;

    m_plabdof->setText(strange);
    m_plabdof->adjustSize();
}


void Stab3dCtrls::stopAnimate()
{
    s_pXPlane->stopAnimateMode();
    m_ppbAnimate->setChecked(false);
}

