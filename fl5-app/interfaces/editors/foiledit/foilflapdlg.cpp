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


#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>

#include "foilflapdlg.h"

#include <interfaces/editors/foiledit/foilwt.h>
#include <api/foil.h>
#include <api/geom_params.h>
#include <interfaces/widgets/customwts/floatedit.h>


FoilFlapDlg::FoilFlapDlg(QWidget *pParent) : FoilDlg(pParent)
{
    setWindowTitle("Flap settings");

    m_bTEFlap     = false;
    m_TEFlapAngle = 0.0;
    m_TEXHinge    = 0.7;
    m_TEYHinge    = 0.5;
    m_bLEFlap     = false;
    m_LEFlapAngle = 0.0;
    m_LEXHinge    = 0.2;
    m_LEYHinge    = 0.5;

    setupLayout();


    connect(m_pchLEFlapCheck, SIGNAL(clicked(bool)),       SLOT(onLEFlapCheck()));
    connect(m_pchTEFlapCheck, SIGNAL(clicked(bool)),       SLOT(onTEFlapCheck()));

    connect(m_pfeLEXHinge,    SIGNAL(floatChanged(float)), SLOT(onChanged()));
    connect(m_pfeLEYHinge,    SIGNAL(floatChanged(float)), SLOT(onChanged()));
    connect(m_pfeTEXHinge,    SIGNAL(floatChanged(float)), SLOT(onChanged()));
    connect(m_pfeTEYHinge,    SIGNAL(floatChanged(float)), SLOT(onChanged()));
    connect(m_pfeLEFlapAngle, SIGNAL(floatChanged(float)), SLOT(onChanged()));
    connect(m_pfeTEFlapAngle, SIGNAL(floatChanged(float)), SLOT(onChanged()));
}


void FoilFlapDlg::setupLayout()
{
    m_pOverlayFrame = new QFrame(m_pFoilWt);
    {
        m_pOverlayFrame->setCursor(Qt::ArrowCursor);
        m_pOverlayFrame->setFrameShape(QFrame::NoFrame);
        m_pOverlayFrame->setPalette(m_Palette);
        m_pOverlayFrame->setAutoFillBackground(false);
        m_pOverlayFrame->setAttribute(Qt::WA_NoSystemBackground);

        QGridLayout *pFlapDataLayout = new QGridLayout;
        {
            m_pchLEFlapCheck = new QCheckBox("L.E. flap");
            m_pchTEFlapCheck = new QCheckBox("T.E. flap");
            m_pchLEFlapCheck->setPalette(m_Palette);
            m_pchTEFlapCheck->setPalette(m_Palette);

            m_pfeLEXHinge    = new FloatEdit;
            m_pfeLEYHinge    = new FloatEdit;
            m_pfeTEXHinge    = new FloatEdit;
            m_pfeTEYHinge    = new FloatEdit;
            m_pfeTEFlapAngle = new FloatEdit;
            m_pfeLEFlapAngle = new FloatEdit;

            QLabel *plab1 = new QLabel("Flap angle");
            QLabel *plab2 = new QLabel("<p>&deg; (+ is down)</p>");
            QLabel *plab3 = new QLabel("Hinge x position");
            QLabel *plab4 = new QLabel("% Chord");
            QLabel *plab5 = new QLabel("Hinge y position");
            QLabel *plab6 = new QLabel("% thickness");
            plab1->setPalette(m_Palette);
            plab2->setPalette(m_Palette);
            plab3->setPalette(m_Palette);
            plab4->setPalette(m_Palette);
            plab5->setPalette(m_Palette);
            plab6->setPalette(m_Palette);
            plab1->setAttribute(Qt::WA_NoSystemBackground);
            plab2->setAttribute(Qt::WA_NoSystemBackground);
            plab3->setAttribute(Qt::WA_NoSystemBackground);
            plab4->setAttribute(Qt::WA_NoSystemBackground);
            plab5->setAttribute(Qt::WA_NoSystemBackground);
            plab6->setAttribute(Qt::WA_NoSystemBackground);

            m_pchMakePermanent = new QCheckBox("Make deflection permanent");
            QString tip("<p>"
                        "If activated, the foil's geometry will be modified to include the deflected T.E. flap.<br>"
                        "This option should be avoided in the general case. It is of interest only in the special case where a plane needs to be "
                        "built later on with a non-zero T.E. flap angle, to intersect the flapped wing with the fuselage<br>"
                        "Starting in v7.50, it is recommended to deactivate this option and to set the T.E. flap angles in the foil and plane polars."
                        "</p>");
            m_pchMakePermanent->setToolTip(tip);

            QLabel *pFlow5Link = new QLabel;
            pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Modelling/TEFlaps.html>https://flow5.tech/.../TEFlaps.html</a>");
            pFlow5Link->setOpenExternalLinks(true);
//            pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
            pFlow5Link->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);
            pFlow5Link->setAttribute(Qt::WA_NoSystemBackground);


            pFlapDataLayout->addWidget(m_pchLEFlapCheck,   1, 2);
            pFlapDataLayout->addWidget(m_pchTEFlapCheck,   1, 3);

            pFlapDataLayout->addWidget(plab3,              3, 1);
            pFlapDataLayout->addWidget(m_pfeLEXHinge,      3, 2);
            pFlapDataLayout->addWidget(m_pfeTEXHinge,      3, 3);
            pFlapDataLayout->addWidget(plab4,              3, 4);
            pFlapDataLayout->addWidget(plab5,              4, 1);
            pFlapDataLayout->addWidget(m_pfeLEYHinge,      4, 2);
            pFlapDataLayout->addWidget(m_pfeTEYHinge,      4, 3);
            pFlapDataLayout->addWidget(plab6,              4, 4);

            pFlapDataLayout->addWidget(plab1,              6, 1);
            pFlapDataLayout->addWidget(m_pfeLEFlapAngle,   6, 2);
            pFlapDataLayout->addWidget(m_pfeTEFlapAngle,   6, 3);
            pFlapDataLayout->addWidget(plab2,              6, 4);

            pFlapDataLayout->addWidget(m_pchMakePermanent, 7, 3, 1, 2);
            pFlapDataLayout->addWidget(pFlow5Link,         8, 1, 1, 3);

            pFlapDataLayout->setColumnStretch(4,1);
        }
        m_pOverlayFrame->setLayout(pFlapDataLayout);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pFoilWt);
    }
    setLayout(pMainLayout);
}


void FoilFlapDlg::initDialog(Foil *pFoil)
{
    FoilDlg::initDialog(pFoil);

    m_pFoilWt->addFoil(pFoil);
    m_pFoilWt->showHinges(true);

    m_pchTEFlapCheck->setChecked(pFoil->hasTEFlap());

    enableTEFlap(pFoil->hasTEFlap());
    m_pfeTEFlapAngle->setValue(pFoil->TEFlapAngle());
    m_pfeTEXHinge->setValue(pFoil->TEXHinge()*100.0);
    m_pfeTEYHinge->setValue(pFoil->TEYHinge()*100.0);

    m_pchLEFlapCheck->setChecked(pFoil->hasLEFlap());
    enableLEFlap(pFoil->hasLEFlap());
    m_pfeLEFlapAngle->setValue(pFoil->LEFlapAngle());
    m_pfeLEXHinge->setValue(pFoil->LEXHinge()*100.0);
    m_pfeLEYHinge->setValue(pFoil->LEYHinge()*100.0);

    m_pchMakePermanent->setChecked(false);
}


void FoilFlapDlg::readParams()
{
    m_bLEFlap = m_pchLEFlapCheck->isChecked();
    m_LEFlapAngle = m_pfeLEFlapAngle->value();
    m_LEXHinge    = m_pfeLEXHinge->value()/100.0;
    m_LEYHinge    = m_pfeLEYHinge->value()/100.0;

    m_bTEFlap = m_pchTEFlapCheck->isChecked();
    m_TEFlapAngle = m_pfeTEFlapAngle->value();
    m_TEXHinge    = m_pfeTEXHinge->value()/100.0;
    m_TEYHinge    = m_pfeTEYHinge->value()/100.0;

    if(m_LEXHinge>=m_TEXHinge && m_bLEFlap && m_bTEFlap)
    {
        QMessageBox::information(window(), "Warning", "The trailing edge hinge must be downstream of the leading edge hinge");
        m_pfeLEXHinge->setFocus();
        m_pfeLEXHinge->selectAll();
    }
}


void FoilFlapDlg::onApply()
{
    //reset everything and retry
    resetFoil();

    readParams();

    m_pBufferFoil->setTEFlapData(m_bTEFlap, m_TEXHinge, m_TEYHinge, m_TEFlapAngle);
    m_pBufferFoil->setLEFlapData(m_bLEFlap, m_LEXHinge, m_LEYHinge, m_LEFlapAngle);
    m_pBufferFoil->initGeometry();

    m_bModified = true;

    update();
}


void FoilFlapDlg::onOK()
{
    readParams();

    onApply();

    if(m_pchMakePermanent->isChecked() &&
            m_pBufferFoil->hasTEFlap() &&
            fabs(m_pBufferFoil->TEFlapAngle())>FLAPANGLEPRECISION)
    {
        m_pBufferFoil->makeModPermanent();
        m_pBufferFoil->initGeometry();
    }
    else
    {
        m_pBufferFoil->setTEFlapAngle(0.0);
    }

    accept();
}


void FoilFlapDlg::showEvent(QShowEvent *pEvent)
{
    FoilDlg::showEvent(pEvent);
    resizeEvent(nullptr);
}


void FoilFlapDlg::resizeEvent(QResizeEvent *pEvent)
{
    FoilDlg::resizeEvent(pEvent);

    int h = m_pFoilWt->height();
//    int w = m_pFoilWidget->width();

    QPoint pos1(5, h-m_pOverlayFrame->height()-5);
    m_pOverlayFrame->move(pos1);
}


void FoilFlapDlg::enableLEFlap(bool bEnable)
{
    m_pfeLEFlapAngle->setEnabled(bEnable);
}


void FoilFlapDlg::enableTEFlap(bool bEnable)
{
    m_pfeTEFlapAngle->setEnabled(bEnable);
    m_pchMakePermanent->setEnabled(bEnable);
}


void FoilFlapDlg::onTEFlapCheck()
{
    if(m_pchTEFlapCheck->isChecked())
    {
        enableTEFlap(true);
        m_pfeTEFlapAngle->setFocus();
    }
    else
        enableTEFlap(false);
    onApply();
}


void FoilFlapDlg::onLEFlapCheck()
{
    if(m_pchLEFlapCheck->isChecked())
    {
        enableLEFlap(true);
        m_pfeLEFlapAngle->setFocus();
    }
    else
        enableLEFlap(false);
    onApply();
}



