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

#define _MATH_DEFINES_DEFINED


#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>

#include "xsaillegendwt.h"


#include <modules/xsail/controls/boatexplorer.h>
#include <modules/xsail/xsail.h>

#include <interfaces/graphs/graph/graph.h>
#include <core/displayoptions.h>
#include <api/enums_objects.h>
#include <api/boatpolar.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <api/oppoint.h>
#include <api/polar.h>
#include <api/boat.h>
#include <api/sailobjects.h>
#include <interfaces/widgets/color/colorgraddlg.h>
#include <interfaces/widgets/line/legendbtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <interfaces/widgets/globals/wt_globals.h>

XSail* XSailLegendWt::s_pXSail = nullptr;

XSailLegendWt::XSailLegendWt(QWidget *pParent) : LegendWt(pParent)
{
    setAutoFillBackground(true);
}


void XSailLegendWt::makeLegend(bool bHighlight)
{
    if(s_pXSail->isPolarView())
        makePolarLegendBtns(bHighlight);
}


void XSailLegendWt::makePolarLegendBtns(bool bHighlight)
{
    setStyleSheet(QString::asprintf("background: %s;", DisplayOptions::backgroundColor().name(QColor::HexRgb).toStdString().c_str()));
    setAutoFillBackground(true);

    QVector<Boat*> Boatlist;
    for (int j=0; j<SailObjects::nBoats(); j++)
    {
        Boat *pBoat = SailObjects::boat(j);
        for (int i=0; i<SailObjects::nBtPolars(); i++)
        {
            BoatPolar *pBtPolar = SailObjects::btPolar(i);
            if(pBtPolar->boatName().compare(pBoat->name())==0 && pBtPolar->hasPoints() && isPolarVisible(pBtPolar))
            {
                Boatlist.append(pBoat);
                break;
            }
        }
    }

    m_XflObjectMap.clear();

    QString stylestring = QString::asprintf("color: %s; font-family: %s; font-weight: bold; font-size: %dpt;",
                                            DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                            DisplayOptions::textFontStruct().family().toStdString().c_str(),
                                            DisplayOptions::textFontStruct().pointSize());

    QVBoxLayout *pLegendLayout = dynamic_cast<QVBoxLayout*>(layout());
    if(!pLegendLayout)
    {
        pLegendLayout = new QVBoxLayout;
        setLayout(pLegendLayout);
    }
    else
    {
        wt::clearLayout(pLegendLayout);
    }

    for (int j=0; j<Boatlist.count(); j++)
    {
        Boat *pBoat = Boatlist.at(j);
        QLabel *pBoatLab = new QLabel(QString::fromStdString(pBoat->name()));
        pBoatLab->setAttribute(Qt::WA_NoSystemBackground);
        pBoatLab->setStyleSheet(stylestring);
        pLegendLayout->addWidget(pBoatLab);


        for (int i=0; i<SailObjects::nBtPolars(); i++)
        {

            BoatPolar *pBtPolar = SailObjects::btPolar(i);
            if (pBtPolar->boatName()==pBoat->name() && pBtPolar->hasPoints() && isPolarVisible(pBtPolar))
            {

                LegendBtn *pLegendBtn = new LegendBtn;
                LineStyle ls(pBtPolar->theStyle());
                ls.m_Tag = pBtPolar->name();
                pLegendBtn->setStyle(ls);
                pLegendBtn->setBackground(true);
                if(pBtPolar==s_pXSail->curBtPolar() && Graph::isHighLighting() && bHighlight)
                    pLegendBtn->setHighLight(true);
                m_XflObjectMap.insert(pLegendBtn, pBtPolar);

                connect(pLegendBtn, SIGNAL(clickedLB(LineStyle)), this, SLOT(onClickedPolarBtn()));
//                connect(pLegendBtn, SIGNAL(clickedRightLB(LineStyle)), this, SLOT(onClickedRightWPolarBtn(LineStyle)));
                connect(pLegendBtn, SIGNAL(clickedLine(LineStyle)), this, SLOT(onClickedPolarBtnLine(LineStyle)));
                pLegendLayout->addWidget(pLegendBtn);
            }
        }
        pLegendLayout->addSpacing(16);
    }
    pLegendLayout->addStretch();
}


bool XSailLegendWt::isPolarVisible(BoatPolar *pPolar)
{
    bool bPolarVisible = pPolar->isVisible();

    return bPolarVisible;
}


void XSailLegendWt::onClickedPolarBtn()
{
    LegendBtn *pLegendBtn = dynamic_cast<LegendBtn*>(sender());
    BoatPolar *pBtPolar = dynamic_cast<BoatPolar*>(m_XflObjectMap[pLegendBtn]);

    s_pXSail->setBoat(QString::fromStdString(pBtPolar->boatName()));
    s_pXSail->setBtPolar(pBtPolar);
    s_pXSail->m_pBoatExplorer->selectBtPolar(pBtPolar);
    s_pXSail->updateView();
}


void XSailLegendWt::onClickedPolarBtnLine(LineStyle ls)
{
    LegendBtn *pLegendBtn = dynamic_cast<LegendBtn*>(sender());
    BoatPolar *pBtPolar = dynamic_cast<BoatPolar*>(m_XflObjectMap[pLegendBtn]);

    LineMenu *pLineMenu = new LineMenu(nullptr);
    pLineMenu->initMenu(ls);
    pLineMenu->exec(QCursor::pos());
    ls = pLineMenu->theStyle();
    pBtPolar->setTheStyle(ls);
    s_pXSail->resetCurves();
    s_pXSail->updateView();
    s_pXSail->m_pBoatExplorer->setCurveParams();
    emit s_pXSail->projectModified();
}





