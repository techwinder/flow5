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


#include <QHBoxLayout>
#include <QLabel>

#include "foilledlg.h"

#include <interfaces/widgets/customwts/floatedit.h>
#include <api/foil.h>
#include <interfaces/editors/foiledit/foilwt.h>


double FoilLEDlg::s_LErfac=1.0;
double FoilLEDlg::s_BlendLength=0.2;
double FoilLEDlg::s_LERadius=0.025;


FoilLEDlg::FoilLEDlg(QWidget *pParent) : FoilDlg(pParent)
{
    setWindowTitle("Leading edge");

    s_LErfac      = 1.0;
    s_BlendLength = 0.1;

    setupLayout();

    connect(m_pfeLEfactor, SIGNAL(floatChanged(float)), SLOT(onChanged()));
    connect(m_pfeBlend,    SIGNAL(floatChanged(float)), SLOT(onChanged()));
}


void FoilLEDlg::setupLayout()
{
    m_pOverlayFrame = new QFrame(m_pFoilWt);
    {
        m_pOverlayFrame->setCursor(Qt::ArrowCursor);
        m_pOverlayFrame->setFrameShape(QFrame::NoFrame);
        m_pOverlayFrame->setPalette(m_Palette);
        m_pOverlayFrame->setAutoFillBackground(false);

        QGridLayout *pParamsLayout = new QGridLayout;
        {
            m_pfeDisplayRadius = new FloatEdit(0.1f,3);
            m_pfeDisplayRadius->setToolTip("The radius of the LE Circle to display, for information only");
            connect(m_pfeDisplayRadius, SIGNAL(floatChanged(float)), this, SLOT(onLERadiusDisplay()));

            QLabel *pLabr = new QLabel("L.E. display circle radius=");
            pLabr->setPalette(m_Palette);

            QLabel *pLabChord = new QLabel("% Chord");
            pLabChord->setPalette(m_Palette);
            pLabr->setAlignment(Qt::AlignRight | Qt::AlignVCenter);


            QLabel *pLab1 = new QLabel("Approximate new/old ratio for L.E. radius");
            pLab1->setPalette(m_Palette);
            pLab1->setAlignment(Qt::AlignRight);

            QLabel *pLab2 = new QLabel("ratio");
            pLab2->setPalette(m_Palette);

            m_pfeLEfactor = new FloatEdit;


            QLabel *pLab3 = new QLabel("Characteristic blending distance from L.E.");
            pLab3->setPalette(m_Palette);
            pLab3->setAlignment(Qt::AlignRight);

            QLabel *pLab4 = new QLabel("% chord");
            pLab4->setPalette(m_Palette);

            m_pfeBlend = new FloatEdit;

            pParamsLayout->addWidget(pLabr,              1,1);
            pParamsLayout->addWidget(m_pfeDisplayRadius, 1,2);
            pParamsLayout->addWidget(pLabChord,          1,3);
            pParamsLayout->addWidget(pLab1,              2,1);
            pParamsLayout->addWidget(m_pfeLEfactor,      2,2);
            pParamsLayout->addWidget(pLab2,              2,3);
            pParamsLayout->addWidget(pLab3,              3,1);
            pParamsLayout->addWidget(m_pfeBlend,         3,2);
            pParamsLayout->addWidget(pLab4,              3,3);
        }
        m_pOverlayFrame->setLayout(pParamsLayout);
    }


    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pFoilWt);
    }
    setLayout(pMainLayout);
}



void FoilLEDlg::showEvent(QShowEvent *pEvent)
{
    FoilDlg::showEvent(pEvent);
    resizeEvent(nullptr);
}


void FoilLEDlg::resizeEvent(QResizeEvent *pEvent)
{
    FoilDlg::resizeEvent(pEvent);
    int h = m_pFoilWt->height();
//    int w = m_pFoilWidget->width();

    QPoint pos1(5, h-m_pOverlayFrame->height()-5);
    m_pOverlayFrame->move(pos1);
}


void FoilLEDlg::initDialog(Foil *pFoil)
{
    FoilDlg::initDialog(pFoil);
    m_pFoilWt->addFoil(m_pRefFoil);
    m_pfeLEfactor->setMin(  0.0);
    m_pfeLEfactor->setMax(100.0);

    m_pfeBlend->setMin(  0.001f);
    m_pfeBlend->setMax(100.0f);

    m_pfeDisplayRadius->setValue(s_LERadius*100.0);

    m_pfeLEfactor->setValue(s_LErfac);
    m_pfeBlend->setValue(s_BlendLength*100.0);
    m_pFoilWt->setLECircleRadius(s_LERadius);
}


void FoilLEDlg::onLERadiusDisplay()
{
    s_LERadius = m_pfeDisplayRadius->value()/100.0;
    m_pFoilWt->setLECircleRadius(s_LERadius);
    m_pFoilWt->update();
}


void FoilLEDlg::onApply()
{
    //reset everything and retry
    resetFoil();

    s_LErfac = m_pfeLEfactor->value();
    s_BlendLength  = m_pfeBlend->value()/100;

    std::vector<double> newthick = m_pBufferFoil->thickness();
    std::vector<Node2d> const& cbl = m_pBufferFoil->baseCbLine();

    for(uint i=0; i<cbl.size(); i++)
    {
        //decay exponentially
        double arg = cbl[i].x;
        double dcf = exp(-arg/s_BlendLength);
        newthick[i] += newthick[i] * dcf * (s_LErfac-1.0);
    }
    m_pBufferFoil->setThickness(newthick);
    m_pBufferFoil->makeBaseFromCamberAndThickness();
    m_pBufferFoil->rebuildPointSequenceFromBase();
    m_pBufferFoil->initGeometry();

    m_bModified = true;

    m_pFoilWt->update();
}



