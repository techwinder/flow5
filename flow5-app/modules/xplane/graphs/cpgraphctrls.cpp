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


#include <QVBoxLayout>

#include "cpgraphctrls.h"
#include <globals/mainframe.h>
#include <modules/xplane/graphs/cpviewwt.h>
#include <modules/xplane/glview/gl3dxplaneview.h>
#include <modules/xplane/xplane.h>
#include <modules/xsail/xsail.h>
#include <core/displayoptions.h>
#include <interfaces/graphs/graph/graph.h>
#include <api/boatpolar.h>
#include <api/planepolar.h>
#include <api/planexfl.h>
#include <api/boat.h>
#include <api/sail.h>
#include <interfaces/widgets/customwts/exponentialslider.h>
#include <interfaces/widgets/customwts/floatedit.h>


CpGraphCtrls::CpGraphCtrls(MainFrame *pMainFrame, XPlane *pXPlane, XSail *pXSail) : QWidget(pMainFrame)
{
    setWindowTitle(tr("Cp Graph Controls"));
    setWindowFlags(Qt::Tool);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    m_iStrip = 0;
    m_SpanRelativePos    = 0.0;
    m_CpLineStyle.m_Color = fl5Color(100,100,100);
    m_CpLineStyle.m_Stipple = Line::DASH;
    m_CpLineStyle.m_Width = 1;
    m_CpLineStyle.m_Symbol = Line::NOSYMBOL;

    s_pMainFrame = pMainFrame;
    s_pXPlane = pXPlane;
    s_pXSail = pXSail;

    setupLayout();
    connectSignals();

}


void CpGraphCtrls::connectSignals()
{
    connect(m_ppbClearCpCurves,   SIGNAL(clicked()),                this,      SLOT(onClearCpCurves()));
    connect(m_ppbKeepCpCurve,     SIGNAL(clicked()),                s_pXPlane, SLOT(onAddCpSectionCurve()));
    connect(m_pslCpSectionSlider, SIGNAL(sliderMoved(int)),         this,      SLOT(onCpSectionSlider(int)));
    connect(m_peslScale,          SIGNAL(sliderMoved(int)),         this,      SLOT(onCpScale(int)));
}


void CpGraphCtrls::setupLayout()
{
    QVBoxLayout *pCpParamsLayout = new QVBoxLayout;
    {
        m_pcbWingList = new QComboBox;

        QGridLayout *pPositionLayout = new QGridLayout;
        {
            QLabel *plabPos = new QLabel(tr("Strip:"));
            m_pslCpSectionSlider = new QSlider(Qt::Horizontal);
            m_pslCpSectionSlider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            m_pslCpSectionSlider->setMinimum(-100);
            m_pslCpSectionSlider->setMaximum(100);
            m_pslCpSectionSlider->setSliderPosition(0);
            m_pslCpSectionSlider->setTickInterval(10);
            m_pslCpSectionSlider->setTickPosition(QSlider::TicksBelow);

            QLabel *plabScale = new QLabel(tr("3d scale:"));
            m_peslScale = new ExponentialSlider(Qt::Horizontal);
            m_peslScale->setRange(0, 100);
            m_peslScale->setExpValue(30);


            pPositionLayout->addWidget(plabPos,              1, 1);
            pPositionLayout->addWidget(m_pslCpSectionSlider, 1, 2);
            pPositionLayout->addWidget(plabScale,            2, 1);
            pPositionLayout->addWidget(m_peslScale,          2, 2);
        }

        m_ppbKeepCpCurve  = new QPushButton(tr("Keep"));

        m_ppbClearCpCurves = new QPushButton(tr("Delete All"));

        QLabel *pFlow5Link = new QLabel;
        pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Post-processing/Cp3d.html>https://flow5.tech/.../Cp3d.html</a>");
        pFlow5Link->setOpenExternalLinks(true);
        pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
        pFlow5Link->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);


        pCpParamsLayout->addWidget(m_pcbWingList);
        pCpParamsLayout->addLayout(pPositionLayout);

        pCpParamsLayout->addStretch();
        pCpParamsLayout->addWidget(m_ppbKeepCpCurve);
        pCpParamsLayout->addWidget(m_ppbClearCpCurves);
        pCpParamsLayout->addWidget(pFlow5Link);
    }
    setLayout(pCpParamsLayout);
}


void CpGraphCtrls::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);
    setControls();
}


void CpGraphCtrls::hideEvent(QHideEvent *pEvent)
{
    QWidget::hideEvent(pEvent);
}


void CpGraphCtrls::setControls()
{
    m_pslCpSectionSlider->setValue(int(m_SpanRelativePos*100.0));
    //fill wing sel list
    m_pcbWingList->clear();

    if(MainFrame::xflApp()==xfl::XPLANE)
    {

        if(s_pXPlane->curPlane() && s_pXPlane->curPlane()->isXflType())
        {
            PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(s_pXPlane->curPlane());
            for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
            {
                m_pcbWingList->addItem(QString::fromStdString(pPlaneXfl->wing(iw)->name()));
            }
            m_pcbWingList->setCurrentIndex(0);

            WingXfl const *pWing = pPlaneXfl->wingAt(0);
            if(pWing)
            {
                m_pslCpSectionSlider->setMinimum(0);
                if(s_pXPlane->curPlPolar() && s_pXPlane->curPlPolar()->isTriLinearMethod())
                    m_pslCpSectionSlider->setMaximum(pWing->nStations());
                else
                    m_pslCpSectionSlider->setMaximum(pWing->nStations()-1);
                m_iStrip = pWing->nStations()/2;
                m_pslCpSectionSlider->setSliderPosition(m_iStrip);
            }
        }

        if(!s_pXPlane->isCpView()) setEnabled(false);
        else
        {
            m_pcbWingList->setEnabled(s_pXPlane->curPOpp());
            m_ppbKeepCpCurve->setEnabled(s_pXPlane->curPOpp());
            m_pslCpSectionSlider->setEnabled(s_pXPlane->curPOpp());
        }
    }
}


void CpGraphCtrls::onCpSectionSlider(int pos)
{
    m_iStrip = pos;
    int iRange = m_pslCpSectionSlider->maximum()-m_pslCpSectionSlider->minimum();
    double range = double(iRange);

    m_SpanRelativePos = -1.0 + 2.0*double(pos)/range;
    s_pMainFrame->resetCpCurves();
    s_pMainFrame->updateView();
}


double CpGraphCtrls::CpSectionScale() const
{
    return m_peslScale->expValuef()/100.0;
}


void CpGraphCtrls::onCpScale(int)
{
    s_pMainFrame->resetCpCurves();
    s_pMainFrame->updateView();
}


void CpGraphCtrls::onClearCpCurves()
{
    s_pMainFrame->cpGraph()->deleteCurves();
    s_pMainFrame->resetCpCurves();
    s_pMainFrame->updateView();
}


