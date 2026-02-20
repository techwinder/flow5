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


#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>


#include "foiltegapdlg.h"
#include <interfaces/widgets/customwts/floatedit.h>
#include <api/foil.h>
#include <interfaces/editors/foiledit/foilwt.h>


double FoilTEGapDlg::s_BlendLength = 0.2;

FoilTEGapDlg::FoilTEGapDlg(QWidget *pParent) : FoilDlg(pParent)
{
    setWindowTitle(tr("Trailing edge gap"));

    s_BlendLength = 0.8;

    setupLayout();

    connect(m_pdeGap,   SIGNAL(floatChanged(float)), SLOT(onChanged()));
    connect(m_pdeBlend, SIGNAL(floatChanged(float)), SLOT(onChanged()));
}


void FoilTEGapDlg::setupLayout()
{
    m_pOverlayFrame = new QFrame(m_pFoilWt);
    {
        m_pOverlayFrame->setCursor(Qt::ArrowCursor);
        m_pOverlayFrame->setFrameShape(QFrame::NoFrame);
        m_pOverlayFrame->setPalette(m_Palette);
        m_pOverlayFrame->setAutoFillBackground(false);

        QGridLayout *pParamsLayout = new QGridLayout;
        {
            QLabel *pLab1 = new QLabel("T.E. Gap Value=");
            pLab1->setPalette(m_Palette);
            pLab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            pLab1->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

            QLabel *pLab2 = new QLabel("% chord");
            pLab2->setPalette(m_Palette);
            pLab2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            m_pdeGap = new FloatEdit(0,3);

            QLabel *pLab3 = new QLabel("Characteristic blending distance from L.E.=");
            pLab3->setPalette(m_Palette);
            pLab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            pLab3->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

            QLabel *pLab4 = new QLabel("% chord");
            pLab4->setPalette(m_Palette);
            pLab4->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            m_pdeBlend = new FloatEdit;

            pParamsLayout->addWidget(pLab1,      1,1);
            pParamsLayout->addWidget(m_pdeGap,   1,2);
            pParamsLayout->addWidget(pLab2,      1,3);
            pParamsLayout->addWidget(pLab3,      2,1);
            pParamsLayout->addWidget(m_pdeBlend, 2,2);
            pParamsLayout->addWidget(pLab4,      2,3);
        }
        m_pOverlayFrame->setLayout(pParamsLayout);
    }


    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pFoilWt);
        //        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);
}


void FoilTEGapDlg::showEvent(QShowEvent *pEvent)
{
    FoilDlg::showEvent(pEvent);
    resizeEvent(nullptr);
}


void FoilTEGapDlg::resizeEvent(QResizeEvent *)
{
    int h = m_pFoilWt->height();
    int w = m_pFoilWt->width();

    QPoint pos1(5, h-m_pOverlayFrame->height()-5);
    m_pOverlayFrame->move(pos1);

    QPoint pos2(w-m_pButtonBox->width()-5, h-m_pButtonBox->height()-5);
    m_pButtonBox->move(pos2);
}


void FoilTEGapDlg::initDialog(Foil *pFoil)
{
    FoilDlg::initDialog(pFoil);
    m_pFoilWt->addFoil(pFoil);

    m_pdeBlend->setMin(  0.0);
    m_pdeBlend->setMax(100.0);

    double gap = m_pRefFoil->TEGap();
    m_pdeGap->setValue(gap*100.0);
    m_pdeBlend->setValue(s_BlendLength*100.0);
}


void FoilTEGapDlg::onApply()
{
    //reset everything and retry
    resetFoil();

    double gap = m_pdeGap->value()/100.0;
    s_BlendLength  = m_pdeBlend->value()/100;

    double dg = (gap-m_pRefFoil->TEGap());
    double length = m_pRefFoil->length();

    CubicSpline CS = m_pRefFoil->cubicSpline(); //  make it non-const

    for(int i=0; i<CS.ctrlPointCount(); i++)
    {
        double arg = m_pRefFoil->TE().x - CS.controlPoint(i).x;
        //decay exponentially
        double dth = exp(-arg/(1.0-s_BlendLength)*length);
//        double u = CS.tmin() + CS.t(i)/(CS.tmax()-CS.tmin());
        double u = double(i) / double(CS.ctrlPointCount()-1);
        if(u<m_pRefFoil->CSfracLE())
        {
            // top surface
            m_pBufferFoil->setBaseNode(i, m_pBufferFoil->xb(i), m_pBufferFoil->yb(i) + dth * dg/2.0);
        }
        else
        {
            // bot surface
            m_pBufferFoil->setBaseNode(i, m_pBufferFoil->xb(i), m_pBufferFoil->yb(i) - dth * dg/2.0);
        }
    }

    m_pBufferFoil->initGeometry();

    m_pBufferFoil->setName(m_pRefFoil->name()+QString::asprintf("_TEgap %.2f%%", m_pBufferFoil->TEGap()*100.0).toStdString());

    m_bModified = true;

    m_pFoilWt->update();
}



