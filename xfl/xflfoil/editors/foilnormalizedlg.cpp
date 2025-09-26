/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/


#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "foilnormalizedlg.h"

#include <xflcore/xflcore.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/plaintextoutput.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/editors/foilwt.h>


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
    strange += QString::asprintf("Camber line angle = %f", angle) + DEGCHAR + EOLCHAR;
    m_ppto->onAppendThisPlainText(strange+EOLCHAR);
}


void FoilNormalizeDlg::onNormalize()
{
    m_ppto->onAppendThisPlainText("Normalizing...\n");
    m_pBufferFoil->normalizeGeometry();
    m_pBufferFoil->initGeometry();
    m_ppto->onAppendThisPlainText("New properties:\n");
    updateProperties();
    m_pFoilWt->update();
}


void FoilNormalizeDlg::onDerotate()
{
    m_ppto->onAppendThisPlainText("Derotating...\n");
    m_pBufferFoil->deRotate();
    m_pBufferFoil->initGeometry();
    m_ppto->onAppendThisPlainText("New properties:\n");
    updateProperties();
    m_pFoilWt->update();
}


void FoilNormalizeDlg::onReset()
{
    m_ppto->onAppendThisPlainText("Resetting foil...\n");
    resetFoil();
    updateProperties();
    m_pFoilWt->update();
}

