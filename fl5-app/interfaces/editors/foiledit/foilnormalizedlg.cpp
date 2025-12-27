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


#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "foilnormalizedlg.h"

#include <api/constants.h>
#include <api/foil.h>
#include <api/utils.h>
#include <core/xflcore.h>
#include <interfaces/editors/foiledit/foilwt.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>


FoilNormalizeDlg::FoilNormalizeDlg(QWidget *pParent) : FoilDlg(pParent)
{
    setWindowTitle("Foil normalization");
    setupLayout();
    connectSignals();
}


void FoilNormalizeDlg::setupLayout()
{
    m_pOverlayFrame = new QFrame(m_pFoilWt);
    {
        m_pOverlayFrame->setCursor(Qt::ArrowCursor);
        m_pOverlayFrame->setFrameShape(QFrame::NoFrame);
//        m_pOverlayFrame->setPalette(m_Palette);
        m_pOverlayFrame->setAutoFillBackground(true);
//        m_pOverlayFrame->setAttribute(Qt::WA_NoSystemBackground);

        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            m_ppto = new PlainTextOutput;
//            m_ppto->updateColors(false);
            m_ppto->setCharDimensions(50,15);

            QHBoxLayout *pBtnLayout = new QHBoxLayout;
            {
                m_ppbDerotate  = new QPushButton("De-rotate");
                m_ppbNormalize = new QPushButton("Normalize");
                pBtnLayout->addWidget(m_ppbDerotate);
                pBtnLayout->addWidget(m_ppbNormalize);
            }

            pFrameLayout->addWidget(m_ppto);
            pFrameLayout->addLayout(pBtnLayout);
        }
        m_pOverlayFrame->setLayout(pFrameLayout);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pFoilWt);
    }
    setLayout(pMainLayout);

    m_pButtonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Discard |
                                     QDialogButtonBox::Reset);
    m_pButtonBox->addButton(m_ppbMenuBtn, QDialogButtonBox::ActionRole);

//    m_pButtonBox->addButton(m_ppbDerotate,  QDialogButtonBox::ActionRole);
//    m_pButtonBox->addButton(m_ppbNormalize, QDialogButtonBox::ActionRole);
}


void FoilNormalizeDlg::connectSignals()
{
    connect(m_ppbDerotate,  SIGNAL(clicked()), SLOT(onDerotate()));
    connect(m_ppbNormalize, SIGNAL(clicked()), SLOT(onNormalize()));
}


void FoilNormalizeDlg::initDialog(Foil *pFoil)
{
    FoilDlg::initDialog(pFoil);
    pFoil->setVisible(true);
    m_pFoilWt->addFoil(m_pRefFoil);
    updateProperties();
}


void FoilNormalizeDlg::showEvent(QShowEvent *pEvent)
{
    FoilDlg::showEvent(pEvent);
    resizeEvent(nullptr);
}


void FoilNormalizeDlg::resizeEvent(QResizeEvent *pEvent)
{
    FoilDlg::resizeEvent(pEvent);
    int h = m_pFoilWt->height();

    QPoint pos1(5, h-m_pOverlayFrame->height()-5);
    m_pOverlayFrame->move(pos1);
}

void FoilNormalizeDlg::updateProperties()
{
    // find current angle
    double angle = atan2(m_pBufferFoil->TE().y-m_pBufferFoil->LE().y, m_pBufferFoil->TE().x-m_pBufferFoil->LE().x)*180.0/PI;
    double length = sqrt(  (m_pBufferFoil->TE().x-m_pBufferFoil->LE().x)*(m_pBufferFoil->TE().x-m_pBufferFoil->LE().x)
                         + (m_pBufferFoil->TE().y-m_pBufferFoil->LE().y)*(m_pBufferFoil->TE().y-m_pBufferFoil->LE().y));
    QString strange;
    strange += QString::asprintf("L.E. position: x=%9f  y=%9f\n", m_pBufferFoil->LE().x, m_pBufferFoil->LE().y);
    strange += QString::asprintf("T.E. position: x=%9f  y=%9f\n", m_pBufferFoil->TE().x, m_pBufferFoil->TE().y);
    strange += QString::asprintf("Foil length       = %f\n", length);
    strange += QString::asprintf("Camber line angle = %f", angle) + DEGch + EOLch;
    m_ppto->onAppendQText(strange+EOLch);
}


void FoilNormalizeDlg::onNormalize()
{
    m_ppto->onAppendQText("Normalizing...\n");
    m_pBufferFoil->normalizeGeometry();
    m_pBufferFoil->initGeometry();
    m_ppto->onAppendQText("New properties:\n");
    updateProperties();
    m_pFoilWt->update();
}


void FoilNormalizeDlg::onDerotate()
{
    m_ppto->onAppendQText("Derotating...\n");
    m_pBufferFoil->deRotate();
    m_pBufferFoil->initGeometry();
    m_ppto->onAppendQText("New properties:\n");
    updateProperties();
    m_pFoilWt->update();
}


void FoilNormalizeDlg::onReset()
{
    m_ppto->onAppendQText("Resetting foil...\n");
    resetFoil();
    updateProperties();
    m_pFoilWt->update();
}

