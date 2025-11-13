/****************************************************************************

    flow5
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
#include <xflfoil/objects2d/foil.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflfoil/editors/foilwt.h>

#include <xflmath/mathelem.h>

int NacaFoilDlg::s_nPanels = 100;


NacaFoilDlg::NacaFoilDlg(QWidget *pParent) : FoilDlg(pParent)
{
    setWindowTitle("NACA foils");

    setupLayout();

    m_Digits = 9;
    m_pleNumber->setText(QString("%1").arg(m_Digits,4,10,QChar('0')));
    m_piePanels->setValue(s_nPanels);
    m_piePanels->setMin(10);

    m_pBufferFoil->setName("Naca_0009");
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

    QVector<double> frac;
    int npts = s_nPanels/2;
    xfl::getPointDistribution(frac, npts-1, xfl::COSINE);

/*    m_pBufferFoil->resizeBaseArrays(npts);
    for(int i=0; i<npts; i++)
    {
        m_pBufferFoil->m_BaseTop[i].x = frac[i];
        m_pBufferFoil->m_BaseBot[i].x = frac[i];
    }*/

    m_pBufferFoil->setDefaultMidLinePointDistribution(npts);

    m_pBufferFoil->setLE(Vector2d(0.0,0.0));
    m_pBufferFoil->setTE(Vector2d(1.0,0.0));

    bool bSuccess(false);

    if(log10(double(m_Digits))>4)
        bSuccess = makeNaca5(m_Digits);
    else
        bSuccess = makeNaca4(m_Digits);
    if(!bSuccess)  makeNaca4(9);


    m_pBufferFoil->initGeometry();
    QString str;
    if(m_Digits>0 && log10(m_Digits)<4)
        str = QString("%1").arg(m_Digits,4,10,QChar('0'));
    else
        str = QString("%1").arg(m_Digits);
    str = "NACA "+ str;

    m_pBufferFoil->setName(str);

    m_bModified = true;
}


bool NacaFoilDlg::makeNaca4(int digits)
{
    int d = digits;
    int n3 = d % 100;
    d = (d-n3)/100;
    int n2 = d % 10;
    int n1 = (d-n2)/10;

    double m  = double(n1)/100.0; // max. camber
    double p  = double(n2)/10.0;  // max. camber position
    double t  = double(n3)/100.0; // max. thickness

    makeThickness(t);

    QVector<Node2d> cbl = m_pBufferFoil->baseCbLine();
    if(n1==0 || n2==0)
    {
        // no camber, or camber max pos is undefined
        for(int i=0; i<cbl.size(); i++)
        {
            cbl[i].y = 0.0;
        }
    }
    else
    {
        double a0 = m/p/p;
        double a1 =m/(1.0-p)/(1.0-p);
        for(int i=0; i<cbl.size(); i++)
        {
            double x = cbl[i].x;
            if(x<=p)
            {
                cbl[i].y = a0 * (2.0*p*x-x*x);
            }
            else
            {
                cbl[i].y = a1 * ((1.0-2.0*p) + 2.0*p*x-x*x);
            }
        }
    }

    m_pBufferFoil->setBaseCamberLine(cbl);
    m_pBufferFoil->makeBaseFromCamberAndThickness();
    m_pBufferFoil->rebuildPointSequenceFromBase();

    return true;
}


bool NacaFoilDlg::makeNaca5(int digits)
{
    int d = digits;
    int n3 = d % 100;
    d = (d-n3)/100;
    int pre =d;
    int n2 = d % 10;

    if(n2!=0 && n2!=1) return false;

    //    double xf = double(n1)/10.0/2.0;  // max. camber position
    double t  = double(n3)/100.0; // max. thickness

    double m=0.0;
    double k1 = 0.0;
    double k2k1=0.0;
    switch(pre)
    {
        case 210:
            m=0.0580;   k1=361.4;   break;
        case 220:
            m=0.1260;   k1=51.64;   break;
        case 230:
            m=0.2025;   k1=15.957;  break;
        case 240:
            m=0.2900;   k1=6.643;   break;
        case 250:
            m=0.3910;   k1=3.230;   break;
            //reflexed airfoils
        case 221:
            m=0.130;    k1=51.990;  k2k1=0.000764; break;
        case 231:
            m=0.217;    k1=15.793;  k2k1=0.00677;  break;
        case 241:
            m=0.318;    k1=6.520;   k2k1=0.0303;   break;
        case 251:
            m=0.441;    k1=3.191;   k2k1=0.1355;   break;
        default:
            return false;
    }


    QVector<Node2d> cbl = m_pBufferFoil->baseCbLine();
    for(int i=0; i<cbl.size(); i++)
    {
        double x = cbl[i].x;
        if(x<=m)
        {
            if(n2==0)
                cbl[i].y = k1/6.0*(x*x*x - 3.0*m*x*x + m*m*(3.0-m)*x);
            else
                cbl[i].y = k1/6.0*((x-m)*(x-m)*(x-m) - k2k1*(1.0-m)*(1.0-m)*(1.0-m)*x - m*m*m*(x-1.0));
        }
        else
        {
            if(n2==0)
                cbl[i].y = k1/6.0 *m*m*m* (1.0-x);
            else
                cbl[i].y = k1/6.0*(k2k1*(x-m)*(x-m)*(x-m) - k2k1*(1.0-m)*(1.0-m)*(1.0-m)*x - m*m*m*(x-1.0));
        }
    }

    m_pBufferFoil->setBaseCamberLine(cbl);
    makeThickness(t);
    m_pBufferFoil->makeBaseFromCamberAndThickness();
    m_pBufferFoil->rebuildPointSequenceFromBase();

    return true;
}


void NacaFoilDlg::makeThickness(double t)
{
    QVector<double> th    = m_pBufferFoil->thickness();
    QVector<Node2d> const &cbl = m_pBufferFoil->baseCbLine();
    for(int i=0; i<th.size(); i++)
    {
        double x = cbl[i].x;
        th[i] = 0.2969*sqrt(x) - 0.1260*x -0.3516*x*x + 0.2843*x*x*x -0.1015*x*x*x*x;
        th[i] *= 5.0*t *2.0;
    }

    m_pBufferFoil->setThickness(th);
}


