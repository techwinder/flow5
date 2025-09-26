/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QPainter>
#include <QVBoxLayout>



#include <xfl/globals/mainframe.h>
#include <xfl/xdirect/controls/foiltableview.h>
#include <xfl/xdirect/controls/foiltreeview.h>
#include <xfl/xdirect/view2d/dfoillegendwt.h>
#include <xfl/xdirect/view2d/dfoilwt.h>
#include <xfl/xdirect/xdirect.h>

#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/objects2d.h>
#include <xflfoil/objects2d/oppoint.h>
#include <xflfoil/objects2d/polar.h>
#include <xflwidgets/line/legendbtn.h>
#include <xflwidgets/line/linemenu.h>


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
            pLegendBtn->setTag(pFoil->name());
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







