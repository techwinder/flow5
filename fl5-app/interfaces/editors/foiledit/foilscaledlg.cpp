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


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

#include "foilscaledlg.h"
#include <core/displayoptions.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <api/foil.h>
#include <interfaces/editors/foiledit/foilwt.h>
#include <QDebug>



FoilScaleDlg::FoilScaleDlg(QWidget *pParent) : FoilDlg(pParent)
{
    setWindowTitle(tr("Foil Geometry"));

    setupLayout();

    connect(m_pfeCamber,       SIGNAL(floatChanged(float)), SLOT(onCamber()));
    connect(m_pfeXCamber,      SIGNAL(floatChanged(float)), SLOT(onXCamber()));
    connect(m_pfeThickness,    SIGNAL(floatChanged(float)), SLOT(onThickness()));
    connect(m_pfeXThickness,   SIGNAL(floatChanged(float)), SLOT(onXThickness()));

    connect(m_pslCamberSlide,  SIGNAL(sliderMoved(int)),    SLOT(onCamberSlide(int)));
    connect(m_pslXCamberSlide, SIGNAL(sliderMoved(int)),    SLOT(onXCamberSlide(int)));
    connect(m_pslThickSlide,   SIGNAL(sliderMoved(int)),    SLOT(onThickSlide(int)));
    connect(m_pslXThickSlide,  SIGNAL(sliderMoved(int)),    SLOT(onXThickSlide(int)));
}


void FoilScaleDlg::setupLayout()
{
    m_pOverlayFrame = new QFrame(m_pFoilWt);
    {
        m_pOverlayFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        m_pOverlayFrame->setAutoFillBackground(false);
        m_pOverlayFrame->setFrameShape(QFrame::NoFrame);
        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            QGroupBox *pCamberGroup = new QGroupBox("Camber");
            {
                pCamberGroup->setPalette(m_Palette);
                pCamberGroup->setAutoFillBackground(false);
                pCamberGroup->setCursor(Qt::ArrowCursor);
                QGridLayout *pCamberLayout = new QGridLayout;
                {
                    m_pslCamberSlide = new QSlider;
                    m_pslCamberSlide->setPalette(m_SliderPalette);
                    m_pslCamberSlide->setAutoFillBackground(true);
                    m_pslCamberSlide->setOrientation(Qt::Horizontal);
                    m_pslCamberSlide->setTickPosition(QSlider::TicksBelow);
                    m_pslCamberSlide->setMinimumWidth(DisplayOptions::tableFontStruct().averageCharWidth()*50);
                    m_pfeCamber =new FloatEdit;
                    m_pslXCamberSlide = new QSlider;
                    m_pslXCamberSlide->setAutoFillBackground(true);
                    m_pslXCamberSlide->setOrientation(Qt::Horizontal);
                    m_pslXCamberSlide->setTickPosition(QSlider::TicksBelow);
                    m_pslXCamberSlide->setMinimumWidth(DisplayOptions::tableFontStruct().averageCharWidth()*50);
                    m_pslXCamberSlide->setPalette(m_SliderPalette);
                    m_pfeXCamber = new FloatEdit;
                    QLabel *lab1 = new QLabel("Value");
                    QLabel *lab2 = new QLabel("%");
                    QLabel *lab3 = new QLabel("0%");
                    QLabel *lab4 = new QLabel("10%");
                    QLabel *lab5 = new QLabel("x-pos");
                    QLabel *lab6 = new QLabel("%");
                    QLabel *lab7 = new QLabel("0%");
                    QLabel *lab8 = new QLabel("100%");
                    lab1->setPalette(m_Palette);
                    lab2->setPalette(m_Palette);
                    lab3->setPalette(m_Palette);
                    lab4->setPalette(m_Palette);
                    lab5->setPalette(m_Palette);
                    lab6->setPalette(m_Palette);
                    lab7->setPalette(m_Palette);
                    lab8->setPalette(m_Palette);
                    lab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    lab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    lab5->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    lab7->setAlignment(Qt::AlignRight | Qt::AlignVCenter);


                    pCamberLayout->addWidget(lab1,                1,1);
                    pCamberLayout->addWidget(m_pfeCamber,         1,2);
                    pCamberLayout->addWidget(lab2,                1,3);
                    pCamberLayout->addWidget(lab3,                1,5);
                    pCamberLayout->addWidget(m_pslCamberSlide,    1,6);
                    pCamberLayout->addWidget(lab4,                1,7);

                    pCamberLayout->addWidget(lab5,                2,1);
                    pCamberLayout->addWidget(m_pfeXCamber,        2,2);
                    pCamberLayout->addWidget(lab6,                2,3);
                    pCamberLayout->addWidget(lab7,                2,5);
                    pCamberLayout->addWidget(m_pslXCamberSlide,   2,6);
                    pCamberLayout->addWidget(lab8,                2,7);
                    pCamberLayout->setColumnMinimumWidth(4, DisplayOptions::tableFontStruct().averageCharWidth()*5);

                }
                pCamberGroup->setLayout(pCamberLayout);
            }

            QGroupBox *pThicknessGroup = new QGroupBox("Thickness");
            {
                pThicknessGroup->setPalette(m_Palette);
                pThicknessGroup->setAutoFillBackground(false);
                pThicknessGroup->setCursor(Qt::ArrowCursor);
                QGridLayout *pThicknessLayout = new QGridLayout;
                {
                    m_pslThickSlide = new QSlider;
                    m_pslThickSlide->setPalette(m_SliderPalette);
                    m_pslThickSlide->setAutoFillBackground(true);
                    m_pslThickSlide->setOrientation(Qt::Horizontal);
                    m_pslThickSlide->setTickPosition(QSlider::TicksBelow);
                    m_pslThickSlide->setMinimumWidth(DisplayOptions::tableFontStruct().averageCharWidth()*50);
                    m_pfeThickness =new FloatEdit;
                    m_pslXThickSlide = new QSlider;
                    m_pslXThickSlide->setAutoFillBackground(true);
                    m_pslXThickSlide->setPalette(m_SliderPalette);
                    m_pslXThickSlide->setOrientation(Qt::Horizontal);
                    m_pslXThickSlide->setTickPosition(QSlider::TicksBelow);
                    m_pslXThickSlide->setMinimumWidth(DisplayOptions::tableFontStruct().averageCharWidth()*50);
                    m_pfeXThickness = new FloatEdit;
                    QLabel *lab11 = new QLabel("Value");
                    QLabel *lab12 = new QLabel("%");
                    QLabel *lab13 = new QLabel("0%");
                    QLabel *lab14 = new QLabel("20%");
                    QLabel *lab15 = new QLabel("x-pos");
                    QLabel *lab16 = new QLabel("%");
                    QLabel *lab17 = new QLabel("0%");
                    QLabel *lab18 = new QLabel("100%");
                    lab11->setPalette(m_Palette);
                    lab12->setPalette(m_Palette);
                    lab13->setPalette(m_Palette);
                    lab14->setPalette(m_Palette);
                    lab15->setPalette(m_Palette);
                    lab16->setPalette(m_Palette);
                    lab17->setPalette(m_Palette);
                    lab18->setPalette(m_Palette);

                    lab11->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    lab15->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    lab13->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    lab17->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

                    pThicknessLayout->addWidget(lab11,              1,1);
                    pThicknessLayout->addWidget(m_pfeThickness,   1,2);
                    pThicknessLayout->addWidget(lab12,              1,3);
                    pThicknessLayout->addWidget(lab13,              1,5);
                    pThicknessLayout->addWidget(m_pslThickSlide,  1,6);
                    pThicknessLayout->addWidget(lab14,              1,7);

                    pThicknessLayout->addWidget(lab15,              2,1);
                    pThicknessLayout->addWidget(m_pfeXThickness,  2,2);
                    pThicknessLayout->addWidget(lab16,              2,3);
                    pThicknessLayout->addWidget(lab17,              2,5);
                    pThicknessLayout->addWidget(m_pslXThickSlide, 2,6);
                    pThicknessLayout->addWidget(lab18,              2,7);

                    pThicknessLayout->setColumnMinimumWidth(4, DisplayOptions::tableFontStruct().averageCharWidth()*5);
                }
                pThicknessGroup->setLayout(pThicknessLayout);
            }

            pCamberGroup->setStyleSheet("QGroupBox{font-weight: bold;}");
            pThicknessGroup->setStyleSheet("QGroupBox{font-weight: bold;}");

            pFrameLayout->addWidget(pCamberGroup);
            pFrameLayout->addWidget(pThicknessGroup);
        }
        m_pOverlayFrame->setLayout(pFrameLayout);
    }


    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pFoilWt);
        setLayout(pMainLayout);
    }

    m_pfeCamber->setDigits(2);
    m_pfeXCamber->setDigits(2);
    m_pfeThickness->setDigits(2);
    m_pfeXThickness->setDigits(2);

    m_pslCamberSlide->setRange(0,100);
    m_pslCamberSlide->setTickInterval(5);
    m_pslXCamberSlide->setRange(0,1000);
    m_pslXCamberSlide->setTickInterval(100);
    m_pslThickSlide->setRange(0,200);
    m_pslThickSlide->setTickInterval(5);
    m_pslXThickSlide->setRange(0,1000);
    m_pslXThickSlide->setTickInterval(100);
}


void FoilScaleDlg::showEvent(QShowEvent *pEvent)
{
    (void)pEvent;
//    m_pFoilWidget->setMinimumSize(QSize(m_pOverlayFrame->width(), m_pFoilWidget->height()));
    resizeEvent(nullptr);
}


void FoilScaleDlg::resizeEvent(QResizeEvent *)
{
    int h = m_pFoilWt->height();
    int w = m_pFoilWt->width();

    QPoint pos1(5, h-m_pOverlayFrame->height()-5);
    m_pOverlayFrame->move(pos1);

    QPoint pos2(w-m_pButtonBox->width()-5, h-m_pButtonBox->height()-5);
    m_pButtonBox->move(pos2);
}


void FoilScaleDlg::initDialog(Foil *pFoil)
{
    FoilDlg::initDialog(pFoil);

    pFoil->setVisible(true);
    m_pFoilWt->addFoil(pFoil);
    m_pFoilWt->addFoil(m_pBufferFoil);
    m_pBufferFoil->showCamberLine(true);

    double Camber     = pFoil->maxCamber();
    double Thickness  = pFoil->maxThickness();
    double XCamber    = pFoil->xCamber();
    double XThickness = pFoil->xThickness();

    if(fabs(Camber) <0.0001)
    {
        m_pslCamberSlide->setEnabled(false);
        m_pfeCamber->setEnabled(false);
    }

    m_pfeCamber->setValue(Camber*100.0);
    m_pfeThickness->setValue(Thickness*100.0);
    m_pfeXCamber->setValue(XCamber*100.0);
    m_pfeXThickness->setValue(XThickness*100.0);

    m_pslCamberSlide->setSliderPosition(int(Camber*1000.0));
    m_pslThickSlide->setSliderPosition(int(Thickness*1000.0));

    m_pslXCamberSlide->setSliderPosition(int(XCamber*1000.0));
    m_pslXThickSlide->setSliderPosition(int(XThickness*1000.0));
}


void FoilScaleDlg::onReset()
{
    FoilDlg::onReset();

    double Camber      = m_pBufferFoil->maxCamber();
    double Thickness   = m_pBufferFoil->maxThickness();
    double XCamber     = m_pBufferFoil->xCamber();
    double XThickness  = m_pBufferFoil->xThickness();

    m_pfeThickness->setValue(Thickness*100.0);
    m_pfeCamber->setValue(Camber*100.0);
    m_pslThickSlide->setSliderPosition(int(Thickness*1000.0));
    m_pslCamberSlide->setSliderPosition(int(Camber*1000.0));

    m_pfeXThickness->setValue(XThickness*100.0);
    m_pfeXCamber->setValue(XCamber*100.0);
    m_pslXThickSlide->setSliderPosition(int(XThickness*1000.0));
    m_pslXCamberSlide->setSliderPosition(int(XCamber*1000.0));

    m_bModified = false;

    update();
}


void FoilScaleDlg::onCamber()
{
    double fCamber = m_pfeCamber->value();
    m_pslCamberSlide->setValue(int(fCamber*10.0));
    onApply();
}


void FoilScaleDlg::onThickness()
{
    double Thickness = m_pfeThickness->value();
    m_pslThickSlide->setValue(int(Thickness*10.0));
    onApply();
}


void FoilScaleDlg::onThickSlide(int pos)
{
    double Thickness = double(pos)/10.0;
    m_pfeThickness->setValue(Thickness);
    onApply();
}


void FoilScaleDlg::onCamberSlide(int pos)
{
    double fCamber = double(pos)/10.0;
    m_pfeCamber->setValue(fCamber);
    onApply();
}


void FoilScaleDlg::onXCamberSlide(int pos)
{
    double XCamber = double(pos)/10.0;
    m_pfeXCamber->setValue(XCamber);
    onApply();
}


void FoilScaleDlg::onXCamber()
{
    double XCamber = m_pfeXCamber->value();
    m_pslXCamberSlide->setValue(int(XCamber*10.0));
    onApply();
}


void FoilScaleDlg::onXThickSlide(int pos)
{
    double XThickness = double(pos)/10.0;
    m_pfeXThickness->setValue(XThickness);
    onApply();
}


void FoilScaleDlg::onXThickness()
{
    double XThickness = m_pfeXThickness->value();
    m_pslXThickSlide->setValue(int(XThickness*10.0));
    onApply();
}


void FoilScaleDlg::onApply()
{
    //reset everything and apply
    resetFoil();

    m_pBufferFoil->applyBase();
    m_pBufferFoil->showCamberLine(true);

    double thickness  = m_pfeThickness->value() /100.0;
    double camber     = m_pfeCamber->value()    /100.0;
    double Xthickness = m_pfeXThickness->value()/100.0;
    double Xcamber    = m_pfeXCamber->value()   /100.0;
    m_pslCamberSlide->setSliderPosition( int(camber    *100.0*10.0));
    m_pslThickSlide->setSliderPosition(  int(thickness *100.0*10.0));
    m_pslXCamberSlide->setSliderPosition(int(Xcamber   *100.0*10.0));
    m_pslXThickSlide->setSliderPosition( int(Xthickness*100.0*10.0));

    Xcamber    = m_pBufferFoil->baseCbLine().front().x + (m_pBufferFoil->baseCbLine().back().x-m_pBufferFoil->baseCbLine().front().x) * Xcamber;
    Xthickness = m_pBufferFoil->baseCbLine().front().x + (m_pBufferFoil->baseCbLine().back().x-m_pBufferFoil->baseCbLine().front().x) * Xthickness;

    m_pBufferFoil->setThickness(Xthickness, thickness);

    m_pBufferFoil->setCamber(Xcamber, camber);

    m_pBufferFoil->makeBaseFromCamberAndThickness();
    m_pBufferFoil->rebuildPointSequenceFromBase();
    m_pBufferFoil->applyBase();

    m_bModified = true;

    update();
}


