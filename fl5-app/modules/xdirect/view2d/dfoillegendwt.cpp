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

#include <QPainter>
#include <QVBoxLayout>



#include <globals/mainframe.h>
#include <modules/xdirect/controls/foiltable.h>
#include <modules/xdirect/controls/foilexplorer.h>
#include <modules/xdirect/view2d/dfoillegendwt.h>
#include <modules/xdirect/view2d/dfoilwt.h>
#include <modules/xdirect/xdirect.h>

#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <api/oppoint.h>
#include <api/polar.h>
#include <interfaces/widgets/line/legendbtn.h>
#include <interfaces/widgets/line/linemenu.h>


XDirect* DFoilLegendWt::s_pXDirect=nullptr;


DFoilLegendWt::DFoilLegendWt(QWidget *pParent) : QWidget(pParent)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

//    m_SizeHint = QSize(30,30);

    setAutoFillBackground(true);
}


DFoilLegendWt::~DFoilLegendWt()
{
}


void DFoilLegendWt::selectFoil(Foil *pFoil)
{
    QList<LegendBtn*> list = m_FoilMap.keys();
//    for(int i=0; i<m_FoilMap.keys().size(); i++)
      for(int i=0; i<m_FoilMap.size(); i++)
    {
        list[i]->setHighLight(false);
    }

    LegendBtn *pLegendBtn = m_FoilMap.key(pFoil);
    if(pLegendBtn)
    {
        pLegendBtn->setHighLight(true);
    }
    update();
}


/**
 * Foil legend in direct design
 */
void DFoilLegendWt::makeDFoilLegend()
{
    QPalette palette;
    palette.setColor(QPalette::Window, DisplayOptions::backgroundColor());
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    setPalette(palette);

    m_FoilMap.clear();

    qDeleteAll(this->children());

    QVBoxLayout *pLegendLayout = new QVBoxLayout;
    int w = 0;
    int h = 0;
    int n=0;
    for (int i=0; i<Objects2d::nFoils(); i++)
    {
        Foil *pFoil = Objects2d::foil(i);
        if(pFoil->isVisible())
        {
            LegendBtn *pLegendBtn = new LegendBtn;
            LineStyle ls(pFoil->theStyle());
            pLegendBtn->setStyle(ls);
            pLegendBtn->setTag(QString::fromStdString(pFoil->name()));
            pLegendBtn->setBackground(false);
            if(pFoil==XDirect::curFoil())
                pLegendBtn->setHighLight(true);
            m_FoilMap.insert(pLegendBtn, pFoil);

//            connect(pLegendBtn, SIGNAL(clickedLB(LineStyle)), SLOT(onClickedFoilBtn()));
            pLegendLayout->addWidget(pLegendBtn);

           w = std::max(w, pLegendBtn->sizeHint().width());
            h +=  (pLegendBtn->sizeHint().height()*3)/2;
            n++;
        }
    }
    pLegendLayout->addStretch();
    setLayout(pLegendLayout);

    if(n==1) h = h*3/2;

    resize(QSize(w,h));
//    updateGeometry();
}


/*
// Causes too many recursive calls to setCurFoil()
void DFoilLegendWt::onClickedFoilBtn()
{
    LegendBtn *pLegendBtn = dynamic_cast<LegendBtn*>(sender());
    Foil *pFoil = dynamic_cast<Foil*>(m_FoilMap[pLegendBtn]);
    selectFoil(pFoil);
    s_pXDirect->setFoil(pFoil);
    s_pXDirect->m_pFoilTreeView->selectFoil(pFoil);
    s_pXDirect->m_pFoilTable->selectFoil(pFoil);
    s_pXDirect->updateView();
    update();
}*/







