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


#include "foilnacadlg.h"


#include <interfaces/editors/foiledit/foilwt.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <api/mathelem.h>
#include <api/foil.h>
#include <api/objects2d.h>

int NacaFoilDlg::s_nPanels = 100;


NacaFoilDlg::NacaFoilDlg(QWidget *pParent) : FoilDlg(pParent)
{
    setWindowTitle("NACA foils");

    setupLayout();

    m_Digits = 9;
    m_pleNumber->setText(QString("%1").arg(m_Digits,4,10,QChar('0')));
    m_piePanels->setValue(s_nPanels);
    m_piePanels->setMin(10);

    m_pBufferFoil->setName(std::string("Naca_0009"));
}


void NacaFoilDlg::setupLayout()
{
    m_pOverlayFrame = new QFrame(m_pFoilWt);
    {
        m_pOverlayFrame->setCursor(Qt::ArrowCursor);
        m_pOverlayFrame->setPalette(m_Palette);
        m_pOverlayFrame->setAutoFillBackground(false);
        QGridLayout *pFormLayout = new QGridLayout;
        {
            QLabel *pDigitsLab = new QLabel("4 or 5 digits:");
            pDigitsLab->setPalette(m_Palette);
            m_pleNumber = new QLineEdit(this);

            QLabel *pNPanelsLab = new QLabel("Number of Panels:");
            pNPanelsLab->setPalette(m_Palette);
            m_piePanels = new IntEdit(100, this);
            m_piePanels->setMax(10000);

            pFormLayout->addWidget(pDigitsLab,1,1, Qt::AlignRight|Qt::AlignVCenter);
            pFormLayout->addWidget(m_pleNumber,1,2);
            pFormLayout->addWidget(pNPanelsLab,2,1);
            pFormLayout->addWidget(m_piePanels,2,2);
 //            pFormLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
        }
        m_pOverlayFrame->setLayout(pFormLayout);
    }

    m_plabMessage = new QLabel(m_pFoilWt);
    m_plabMessage->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_plabMessage->setMinimumWidth(200);
    m_plabMessage->setAlignment(Qt::AlignRight);
    m_plabMessage->setStyleSheet("QLabel { color : red; font-family: Hack; }");


    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pFoilWt);
    }

    setLayout(pMainLayout);

    connect(m_pleNumber, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_piePanels, SIGNAL(intChanged(int)),    this, SLOT(onEditingFinished()));
}


void NacaFoilDlg::showEvent(QShowEvent *pEvent)
{   
    FoilDlg::showEvent(pEvent);
    resizeEvent(nullptr);
    onApply();
}


void NacaFoilDlg::resizeEvent(QResizeEvent *pEvent)
{
    FoilDlg::resizeEvent(pEvent);

    int h = m_pFoilWt->height();
    int w = m_pFoilWt->width();


    QPoint pos1(5, h-m_pOverlayFrame->height()-5);
    m_pOverlayFrame->move(pos1);

    QPoint pos2(w-m_pButtonBox->width()-5, h-m_pButtonBox->height()-5);
    m_pButtonBox->move(pos2);

    QPoint pos3(w-200, h-m_plabMessage->height()-300);
    m_plabMessage->move(pos3);
}


void NacaFoilDlg::onEditingFinished()
{
    bool bOK=false;

    int d = m_pleNumber->text().toInt(&bOK);
    if(bOK) m_Digits = d;

    s_nPanels = m_piePanels->value();
    if(s_nPanels<10)
    {
        s_nPanels = 10;
        m_piePanels->setValue(s_nPanels);
    }

    onApply();

    m_pleNumber->setText(QString("%1").arg(m_Digits,4,10,QChar('0')));
    m_pFoilWt->update();
}


void NacaFoilDlg::onApply()
{
    if(m_Digits<1) return;

    Objects2d::makeNacaFoil(m_pBufferFoil, m_Digits, s_nPanels);

    QString str;
    if(m_Digits>0 && log10(m_Digits)<4)
        str = QString("%1").arg(m_Digits,4,10,QChar('0'));
    else
        str = QString("%1").arg(m_Digits);
    str = "NACA "+ str;

    m_pBufferFoil->setName(str.toStdString());

    m_bModified = true;
}



