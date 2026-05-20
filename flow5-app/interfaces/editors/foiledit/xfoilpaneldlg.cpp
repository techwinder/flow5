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

#include <QMessageBox>
#include <QGridLayout>
#include <QLabel>

#include "xfoilpaneldlg.h"

#include <api/foil.h>
#include <xfoil.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/editors/foiledit/foilwt.h>


int XFoilPanelDlg::s_npan(149);
double XFoilPanelDlg::s_cvpar(1.0);
double XFoilPanelDlg::s_cterat(0.15);
double XFoilPanelDlg::s_ctrrat(1.0);
double XFoilPanelDlg::s_xsref1(1.0);
double XFoilPanelDlg::s_xsref2(1.0);
double XFoilPanelDlg::s_xpref1(1.0);
double XFoilPanelDlg::s_xpref2(1.0);



XFoilPanelDlg::XFoilPanelDlg(QWidget *pParent) : FoilDlg(pParent)
{
    setWindowTitle(tr("XFoil panel refinement"));
    setupLayout();
}


void XFoilPanelDlg::setupLayout()
{
    m_pfrXFoilPanel = new QFrame(m_pFoilWt);
    {
        m_pfrXFoilPanel->setCursor(Qt::ArrowCursor);
        m_pfrXFoilPanel->setAutoFillBackground(false);
        m_pfrXFoilPanel->setPalette(m_Palette);
        m_pfrXFoilPanel->setAttribute(Qt::WA_NoSystemBackground);

        QGridLayout *pInputDataLayout = new QGridLayout;
        {
            QLabel *plab1 = new QLabel("Number of panels");
            QLabel *plab2 = new QLabel("Panel bunching parameter");
            QLabel *plab3 = new QLabel("TE/LE panel density ratio");
            QLabel *plab4 = new QLabel("Refined area/LE panel density ratio");
            QLabel *plab5 = new QLabel("Top side refined area x/c limits");
            QLabel *plab6 = new QLabel("Bottom side refined area x/c limits");

            pInputDataLayout->addWidget(plab1, 1, 1);
            pInputDataLayout->addWidget(plab2, 2, 1);
            pInputDataLayout->addWidget(plab3, 3, 1);
            pInputDataLayout->addWidget(plab4, 4, 1);
            pInputDataLayout->addWidget(plab5, 5, 1);
            pInputDataLayout->addWidget(plab6, 6, 1);


            m_pieNPanels = new IntEdit(100, this);
            m_pieNPanels->setMax(IQX);

            m_pfeCVpar  = new FloatEdit;
            m_pfeCTErat = new FloatEdit;
            m_pfeCTRrat = new FloatEdit;
            m_pfeXsRef1 = new FloatEdit;
            m_pfeXsRef2 = new FloatEdit;
            m_pfeXpRef1 = new FloatEdit;
            m_pfeXpRef2 = new FloatEdit;

            pInputDataLayout->addWidget(m_pieNPanels, 1, 2);
            pInputDataLayout->addWidget(m_pfeCVpar,   2, 2);
            pInputDataLayout->addWidget(m_pfeCTErat,  3, 2);
            pInputDataLayout->addWidget(m_pfeCTRrat,  4, 2);
            pInputDataLayout->addWidget(m_pfeXsRef1,  5, 2);
            pInputDataLayout->addWidget(m_pfeXsRef2,  5, 3);
            pInputDataLayout->addWidget(m_pfeXpRef1,  6, 2);
            pInputDataLayout->addWidget(m_pfeXpRef2,  6, 3);

            connect(m_pieNPanels, SIGNAL(intChanged(int)),     SLOT(onChanged()));
            connect(m_pfeCVpar,   SIGNAL(floatChanged(float)), SLOT(onChanged()));
            connect(m_pfeCTErat,  SIGNAL(floatChanged(float)), SLOT(onChanged()));
            connect(m_pfeCTRrat,  SIGNAL(floatChanged(float)), SLOT(onChanged()));
            connect(m_pfeXsRef1,  SIGNAL(floatChanged(float)), SLOT(onChanged()));
            connect(m_pfeXsRef2,  SIGNAL(floatChanged(float)), SLOT(onChanged()));
            connect(m_pfeXpRef1,  SIGNAL(floatChanged(float)), SLOT(onChanged()));
            connect(m_pfeXpRef2,  SIGNAL(floatChanged(float)), SLOT(onChanged()));
        }
        m_pfrXFoilPanel->setLayout(pInputDataLayout);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pFoilWt);
        setLayout(pMainLayout);
    }
}


void XFoilPanelDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("XFoilPanelDlg");
    {
        s_npan   = settings.value("NPan",   s_npan).toInt();
        s_cvpar  = settings.value("CVPar",  s_cvpar).toDouble();
        s_cterat = settings.value("CTErat", s_cterat).toDouble();
        s_ctrrat = settings.value("CTrRAT", s_ctrrat).toDouble();
        s_xsref1 = settings.value("XsRef1", s_xsref1).toDouble();
        s_xsref2 = settings.value("XsRef2", s_xsref2).toDouble();
        s_xpref1 = settings.value("XpRef1", s_xpref1).toDouble();
        s_xpref2 = settings.value("XpRef2", s_xpref2).toDouble();
    }
    settings.endGroup();
}


void XFoilPanelDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("XFoilPanelDlg");
    {
        settings.setValue("NPan",   s_npan);
        settings.setValue("CVPar",  s_cvpar);
        settings.setValue("CTErat", s_cterat);
        settings.setValue("CTrRAT", s_ctrrat);
        settings.setValue("XsRef1", s_xsref1);
        settings.setValue("XsRef2", s_xsref2);
        settings.setValue("XpRef1", s_xpref1);
        settings.setValue("XpRef2", s_xpref2);
    }
    settings.endGroup();
}

void XFoilPanelDlg::initDialog(Foil *pFoil)
{
    FoilDlg::initDialog(pFoil);
    if(pFoil) m_pBufferFoil->setName(pFoil->name() + " - repaneled");
    m_pFoilWt->addFoil(m_pBufferFoil);

    onReset();

    m_pBufferFoil->show();
    s_npan   = m_pRefFoil->nBaseNodes();

    m_pieNPanels->setValue(s_npan);
    m_pfeCVpar->setValue(s_cvpar);
    m_pfeCTErat->setValue(s_cterat);
    m_pfeCTRrat->setValue(s_ctrrat);
    m_pfeXsRef1->setValue(s_xsref1);
    m_pfeXsRef2->setValue(s_xsref2);
    m_pfeXpRef1->setValue(s_xpref1);
    m_pfeXpRef2->setValue(s_xpref2);
    m_pieNPanels->setFocus();

    onApply();
}


void XFoilPanelDlg::onReset()
{
    FoilDlg::onReset();
//    fillCoordinates();
    update();
}


void XFoilPanelDlg::showEvent(QShowEvent *pEvent)
{
    FoilDlg::showEvent(pEvent);
    resizeEvent(nullptr);
    m_pButtonBox->setFocus();
}


void XFoilPanelDlg::resizeEvent(QResizeEvent *pEvent)
{
    FoilDlg::resizeEvent(pEvent);
    int h  = m_pFoilWt->height();
    int h1 = m_pfrXFoilPanel->height();
    QPoint pos1(0, h-h1);
    m_pfrXFoilPanel->move(pos1);
}


void XFoilPanelDlg::onApply()
{
    //reset everything and retry

    for (int i=0; i< m_pRefFoil->nBaseNodes(); i++)
    {
        m_XFoil.xb[i+1] = m_pRefFoil->xb(i);
        m_XFoil.yb[i+1] = m_pRefFoil->yb(i);
    }
    m_XFoil.nb = m_pRefFoil->nBaseNodes();

    m_XFoil.lflap = false;
    m_XFoil.lbflap = false;

    if(m_XFoil.PreProcess())
    {
        m_XFoil.CheckAngles();
    }

    readParams();

    m_XFoil.pangen();

    if(m_XFoil.n>IQX)
    {
        QString strange = QString(tr("The total number of panels cannot exceed %1")).arg(IQX);
        QMessageBox::information(this, tr("Warning"), strange);
        //reset everything and retry
        for (int i=0; i< m_pRefFoil->nBaseNodes(); i++)
        {
            m_XFoil.x[i+1] = m_pRefFoil->xb(i);
            m_XFoil.y[i+1] = m_pRefFoil->yb(i);
        }
        m_XFoil.n = m_pRefFoil->nBaseNodes();
    }
    else
    {
        m_pBufferFoil->resizePointArrays(m_XFoil.n);
        for (int j=0; j< m_XFoil.n; j++)
        {
            m_pBufferFoil->setBaseNode(j, m_XFoil.x[j+1], m_XFoil.y[j+1]);
        }
        m_pBufferFoil->initGeometry();
        m_pBufferFoil->setFlaps();
    }
    m_bModified = true;

    update();
}



void XFoilPanelDlg::readParams()
{
    s_npan   = m_pieNPanels->value();
    s_cvpar  = m_pfeCVpar->value();
    s_cterat = m_pfeCTErat->value();
    s_ctrrat = m_pfeCTRrat->value();
    s_xsref1 = m_pfeXsRef1->value();
    s_xsref2 = m_pfeXsRef2->value();
    s_xpref1 = m_pfeXpRef1->value();
    s_xpref2 = m_pfeXpRef2->value();


    m_XFoil.npan   = s_npan;
    m_XFoil.cvpar  = s_cvpar;
    m_XFoil.cterat = s_cterat;
    m_XFoil.ctrrat = s_ctrrat;
    m_XFoil.xsref1 = s_xsref1;
    m_XFoil.xsref2 = s_xsref2;
    m_XFoil.xpref1 = s_xpref1;
    m_XFoil.xpref2 = s_xpref2;
}







