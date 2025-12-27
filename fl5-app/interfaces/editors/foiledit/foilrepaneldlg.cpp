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



#include <QFormLayout>
#include <QLabel>
#include <QTime>

#include "foilrepaneldlg.h"

#include <core/displayoptions.h>
#include <interfaces/editors/foiledit/foilwt.h>
#include <api/cubicspline.h>
#include <api/foil.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>


FoilRepanelDlg::FoilRepanelDlg(QWidget *pParent) : FoilDlg(pParent)
{
    m_pCS = new CubicSpline;

    if(FoilWt::bufferFoilStyle().m_Symbol==Line::NOSYMBOL) FoilWt::bufferFoilStyle().m_Symbol=Line::BIGCROSS;
    setupLayout();

    m_pFoilWt->freezeSpline(true);
}


FoilRepanelDlg::~FoilRepanelDlg()
{
    if(m_pCS) delete m_pCS;
}


void FoilRepanelDlg::showEvent(QShowEvent *pEvent)
{
    FoilDlg::showEvent(pEvent);
    resizeEvent(nullptr);
    m_pButtonBox->setFocus();
}


void FoilRepanelDlg::resizeEvent(QResizeEvent *pEvent)
{
    FoilDlg::resizeEvent(pEvent);
    int h = m_pFoilWt->height();

    QPoint pos1(0, h-m_pBunchBox->height());
    m_pBunchBox->move(pos1);
}


void FoilRepanelDlg::setupLayout()
{
    setWindowTitle("Foil panel refinement");

    m_pBunchBox = new QFrame(m_pFoilWt);
    {
        m_pBunchBox->setCursor(Qt::ArrowCursor);
        m_pBunchBox->setAutoFillBackground(false);
        m_pBunchBox->setPalette(m_Palette);
        m_pBunchBox->setAttribute(Qt::WA_NoSystemBackground);
        QVBoxLayout *pBoxLayout = new QVBoxLayout;
        {
            QHBoxLayout *pNbLayout = new QHBoxLayout;
            {
                QLabel *plabNPanels = new QLabel("Number of panels");
                plabNPanels->setPalette(m_Palette);
                plabNPanels->setAttribute(Qt::WA_NoSystemBackground);
                m_pieNPanels = new IntEdit;
                QString tip= "<p>CAUTION: XFoil does not accept number of panels greater than 255. "
                             "Adjust the number of panels and the bunching parameters to "
                             "achieve the desired point distribution.</p>";
                m_pieNPanels->setToolTip(tip);

                m_plabWarning = new QLabel();
                m_plabWarning->setPalette(m_Palette);
                m_plabWarning->setAttribute(Qt::WA_NoSystemBackground);

                pNbLayout->addWidget(plabNPanels);
                pNbLayout->addWidget(m_pieNPanels);
                pNbLayout->addWidget(m_plabWarning);
                pNbLayout->addStretch();
            }

            QGridLayout *pBunchLayout = new QGridLayout;
            {
                QLabel *plabNoAmp = new QLabel("Uniform");
                plabNoAmp->setPalette(m_Palette);
                plabNoAmp->setAttribute(Qt::WA_NoSystemBackground);
                plabNoAmp->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
                QLabel *plabAmp = new QLabel("Bunched");
                plabAmp->setPalette(m_Palette);
                plabAmp->setAttribute(Qt::WA_NoSystemBackground);
                plabAmp->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
                m_pslBunchAmp = new QSlider(Qt::Horizontal);
                m_pslBunchAmp->setPalette(m_SliderPalette);
                m_pslBunchAmp->setRange(0, 100);
                m_pslBunchAmp->setTickInterval(5);
                m_pslBunchAmp->setMinimumWidth(DisplayOptions::tableFontStruct().averageCharWidth()*50);
                m_pslBunchAmp->setTickPosition(QSlider::TicksBelow);
                m_pslBunchAmp->setAutoFillBackground(true);

                pBunchLayout->addWidget(plabNoAmp,      2, 1);
                pBunchLayout->addWidget(m_pslBunchAmp,  2, 2, 1, 2);
                pBunchLayout->addWidget(plabAmp,        2, 4);

            }

            pBoxLayout->addLayout(pNbLayout);
            pBoxLayout->addLayout(pBunchLayout);
        }
        m_pBunchBox->setLayout(pBoxLayout);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pFoilWt);
        setLayout(pMainLayout);
    }

    connect(m_pieNPanels,   SIGNAL(intChanged(int)),   SLOT(onNPanels(int)));
    connect(m_pslBunchAmp,  SIGNAL(sliderMoved(int)),  SLOT(onApply()));
}


void FoilRepanelDlg::onBufferStyle(LineStyle ls)
{
    LineMenu lineMenu(nullptr);
    lineMenu.initMenu(m_pBufferFoil->theStyle());
    lineMenu.exec(QCursor::pos());
    ls = lineMenu.theStyle();
    m_pBufferFoil->setTheStyle(ls);
    update();
}


void FoilRepanelDlg::onNPanels(int npanels)
{
    if(npanels>255)
        m_plabWarning->setText("<p><font color=red>XFoil requires NPanels&lt;255</font></p>");
    else
        m_plabWarning->clear();
    onApply();
}


void FoilRepanelDlg::initDialog(Foil *pFoil)
{
    FoilDlg::initDialog(pFoil);

//    m_pFoilWt->addFoil(pFoil); // overloads the display
    if(pFoil) m_pBufferFoil->setName(pFoil->name() + " - repaneled");
    m_pFoilWt->addFoil(m_pBufferFoil);

    onReset();

    m_pRefFoil->makeCubicSpline(*m_pCS);
    m_pCS->computeArcLengths();

    m_pBufferFoil->show();

    m_pieNPanels->setFocus();

    onApply();
}


void FoilRepanelDlg::onReset()
{
    m_pCS->duplicate(m_pRefFoil->cubicSpline());
    m_pCS->setPointStyle(Line::BIGCIRCLE);
    m_pCS->showCtrlPts(false);
    m_pCS->updateSpline();

    m_pCS->setOutputSize(m_pRefFoil->nBaseNodes());
    m_pCS->makeCurve();

    m_pieNPanels->setValue(m_pRefFoil->nBaseNodes());

    int vamp = int(m_pCS->bunchAmplitude()*100.0);
    m_pslBunchAmp->setValue(vamp);

    update();
}


void FoilRepanelDlg::onApply()
{
    //reset everything and retry
    resetFoil();

    int nPanels = m_pieNPanels->value();
    nPanels = std::max(nPanels, 2);

    int val0 = m_pslBunchAmp->value();
    double amp = double(val0)/100.0;
    m_pCS->setBunchAmplitude(amp);

    m_pCS->rePanel(nPanels);

    m_pBufferFoil->setBunchParameters(m_pCS->bunchType(), m_pCS->bunchAmplitude()); // save the bunch parameters

    m_pBufferFoil->setBaseNodes(m_pCS->outputPts());
    m_pBufferFoil->applyBase();


//    m_pBufferFoil->setNodes(m_pCS->outputPts());

    update();

    m_bModified = true;
}


