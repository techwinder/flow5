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


#include <QGroupBox>
#include <QVBoxLayout>

#include "interpolatefoilsdlg.h"


#include <core/displayoptions.h>
#include <interfaces/editors/foiledit/foilwt.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>

#define SLIDERSCALE 10000

InterpolateFoilsDlg::InterpolateFoilsDlg(QWidget *pParent) : FoilDlg(pParent)
{
    setWindowTitle("Interpolate foils");

    m_pFoil1 = new Foil;
    m_pFoil2 = new Foil;

    setupLayout();

    connect(m_pcbFoil1,     SIGNAL(activated(int)),       SLOT(onSelChangeFoil1(int)));
    connect(m_pcbFoil2,     SIGNAL(activated(int)),       SLOT(onSelChangeFoil2(int)));
    connect(m_pdeFrac,      SIGNAL(floatChanged(float)),  SLOT(onFrac()));
    connect(m_pslMix,       SIGNAL(sliderMoved(int)),     SLOT(onSlider(int)));
}


InterpolateFoilsDlg::~InterpolateFoilsDlg()
{
    if(m_pFoil1) delete m_pFoil1;
    if(m_pFoil2) delete m_pFoil2;
    m_pFoil1 = nullptr;
    m_pFoil2 = nullptr;
}


void InterpolateFoilsDlg::showEvent(QShowEvent *pEvent)
{
    FoilDlg::showEvent(pEvent);
    resizeEvent(nullptr);
}


void InterpolateFoilsDlg::setupLayout()
{
    QFrame *pCtrlFrame = new QFrame;
    {
        pCtrlFrame->setCursor(Qt::ArrowCursor);
        QVBoxLayout *pCtrlLayout = new QVBoxLayout;
        {

            QHBoxLayout *pTopLayout = new QHBoxLayout;
            {
                QVBoxLayout *pFoil1Layout = new QVBoxLayout;
                {
                    m_pcbFoil1 = new QComboBox;
                    m_plabProps1 = new QLabel("properties");
                    m_plabProps1->setFont(DisplayOptions::tableFontStruct().font());
                    pFoil1Layout->addWidget(m_pcbFoil1);
                    pFoil1Layout->addWidget(m_plabProps1);
                }

                QVBoxLayout *pFoil2Layout = new QVBoxLayout;
                {
                    m_pcbFoil2 = new QComboBox;
                    m_plabProps2 = new QLabel("properties");
                    m_plabProps2->setFont(DisplayOptions::tableFontStruct().font());
                    pFoil2Layout->addWidget(m_pcbFoil2);
                    pFoil2Layout->addWidget(m_plabProps2);
                }

                QVBoxLayout *pFoil3Layout = new QVBoxLayout;
                {
                    QLabel *pInterLab = new QLabel("Buffer foil");

                    m_plabProps3 = new QLabel("properties");
                    m_plabProps3->setFont(DisplayOptions::tableFontStruct().font());

                    pFoil3Layout->addWidget(pInterLab);
                    pFoil3Layout->addWidget(m_plabProps3);
                }

                pTopLayout->addLayout(pFoil1Layout);
                pTopLayout->addStretch();
                pTopLayout->addLayout(pFoil3Layout);
                pTopLayout->addStretch();
                pTopLayout->addLayout(pFoil2Layout);
            }

            QHBoxLayout *pSliderLayout = new QHBoxLayout;
            {
                m_pdeFrac = new FloatEdit;
                QLabel *pLab = new QLabel("%");
                m_pslMix = new QSlider(Qt::Horizontal);
                m_pslMix->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum));
                m_pslMix->setMinimum(0);
                m_pslMix->setMaximum(SLIDERSCALE);
                m_pslMix->setTickInterval(SLIDERSCALE/10);
                m_pslMix->setTickPosition(QSlider::TicksBelow);
                pSliderLayout->addWidget(m_pdeFrac);
                pSliderLayout->addWidget(pLab);
                pSliderLayout->addWidget(m_pslMix);
            }
            pCtrlLayout->addLayout(pTopLayout);
            pCtrlLayout->addSpacing(15);
            pCtrlLayout->addLayout(pSliderLayout);
        }
        pCtrlFrame->setLayout(pCtrlLayout);
    }


    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(pCtrlFrame);
        pMainLayout->addSpacing(15);
        pMainLayout->addWidget(m_pFoilWt);
    }

    setLayout(pMainLayout);
}


void InterpolateFoilsDlg::initDialogFoils()
{

    for(std::string const &name : Objects2d::foilNames())
    {
        m_pcbFoil1->addItem(QString::fromStdString(name));
        m_pcbFoil2->addItem(QString::fromStdString(name));
    }

    m_pcbFoil1->setCurrentIndex(0);
    m_pcbFoil2->setCurrentIndex(1);

    m_Frac = 50.0;
    m_pdeFrac->setValue(50);
    m_pslMix->setSliderPosition(SLIDERSCALE/2);

    m_pcbFoil1->setCurrentIndex(0);
    m_pcbFoil2->setCurrentIndex(1);

    setFoil1();
    setFoil2();
    showSelectedFoils();

    m_pFoilWt->setBufferFoil(m_pBufferFoil);
    m_pBufferFoil->setName(std::string("Interpolated foil"));
    m_pBufferFoil->setTheStyle(FoilWt::bufferFoilStyle());
    m_pBufferFoil->show();
    m_pBufferFoil->setFilled(FoilWt::isFilledBufferFoil());
    m_pFoilWt->m_pBufferLineMenu->initMenu(m_pBufferFoil->theStyle());

    onApply();
}


void InterpolateFoilsDlg::onSelChangeFoil1(int)
{
    setFoil1();
    showSelectedFoils();
    onApply();
}


void InterpolateFoilsDlg::onSelChangeFoil2(int)
{
    setFoil2();
    showSelectedFoils();
    onApply();
}


void InterpolateFoilsDlg::setFoil1()
{
    QString strong  = m_pcbFoil1->currentText();

    Foil const *pFoil1 = Objects2d::foil(strong.toStdString());
    if(pFoil1)
    {
        m_plabProps1->setText(QString::fromStdString(pFoil1->properties(false)));

        m_pFoil1->copy(pFoil1);
        m_pFoil1->setTheStyle(pFoil1->theStyle());
        m_pFoil1->setVisible(true);
    }
}


void InterpolateFoilsDlg::setFoil2()
{
    QString strong  = m_pcbFoil2->currentText();
    Foil const *pFoil2 = Objects2d::foil(strong.toStdString());

    if(pFoil2)
    {
       m_plabProps2->setText(QString::fromStdString(pFoil2->properties(false)));

        m_pFoil2->copy(pFoil2);
        m_pFoil2->setTheStyle(pFoil2->theStyle());
        m_pFoil2->setVisible(true);
    }
}


void InterpolateFoilsDlg::showSelectedFoils()
{
    m_pFoilWt->clearFoils();
    m_pFoilWt->addFoil(m_pBufferFoil);
    m_pFoilWt->addFoil(m_pFoil1);
    m_pFoilWt->addFoil(m_pFoil2);
}


void InterpolateFoilsDlg::onFrac()
{
    m_Frac = m_pdeFrac->value();
    m_pslMix->setSliderPosition(int(m_Frac/100.0*SLIDERSCALE));
    m_Frac = 100.0 - m_Frac;

    onApply();
}


void InterpolateFoilsDlg::onSlider(int val)
{
    val = m_pslMix->sliderPosition();
    m_Frac = double(val)/SLIDERSCALE*100.0;
    m_pdeFrac->setValue(m_Frac);
    onApply();
}


void InterpolateFoilsDlg::onApply()
{
    if(m_pFoil1->nNodes()>m_pFoil2->nNodes()) m_pBufferFoil->copy(m_pFoil1, false);
    else                            m_pBufferFoil->copy(m_pFoil2, false);
    m_pBufferFoil->setTheStyle(FoilWt::bufferFoilStyle());
    m_pBufferFoil->show();
    m_pBufferFoil->setFilled(FoilWt::isFilledBufferFoil());
    m_pBufferFoil->interpolate(m_pFoil1, m_pFoil2, m_Frac/100.0);
    m_pBufferFoil->makeBaseFromCamberAndThickness();
    m_pBufferFoil->rebuildPointSequenceFromBase();
    m_pBufferFoil->applyBase();

    m_plabProps3->setText(QString::fromStdString(m_pBufferFoil->properties(false)));

    update();
}

