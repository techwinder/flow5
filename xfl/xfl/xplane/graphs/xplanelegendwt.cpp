/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include <QVBoxLayout>
#include <QLabel>

#include "xplanelegendwt.h"

#include <xfl/xplane/xplane.h>
#include <xfl/xplane/controls/planetreeview.h>
#include <xflcore/displayoptions.h>

#include <xflgraph/graph/graph.h>
#include <xflobjects/objects3d/analysis/planeopp.h>
#include <xflobjects/objects3d/analysis/wpolar.h>
#include <xflobjects/objects3d/objects3d.h>
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflwidgets/line/legendbtn.h>
#include <xflwidgets/line/linemenu.h>
#include <xflwidgets/wt_globals.h>

XPlane* XPlaneLegendWt::s_pXPlane = nullptr;


XPlaneLegendWt::XPlaneLegendWt(QWidget *pParent) : LegendWt(pParent)
{
    setAutoFillBackground(true);
}


void XPlaneLegendWt::makeLegend(bool bHighlight)
{
    QString sheet = QString::asprintf("background: %s;", DisplayOptions::backgroundColor().name(QColor::HexRgb).toStdString().c_str());
    setStyleSheet(sheet);
    setAutoFillBackground(true);

    switch(s_pXPlane->m_eView)
    {
        case XPlane::WOPPVIEW:
            makePOppLegendBtns(bHighlight);
            break;
        case XPlane::STABPOLARVIEW:
        case XPlane::WPOLARVIEW:
            makeWPolarLegendBtns(s_pXPlane->m_eView, bHighlight);
            break;
        case XPlane::W3DVIEW:
            break;
        case XPlane::STABTIMEVIEW:
            makeGraphLegendBtns(bHighlight);
            break;
        case XPlane::CPVIEW:
        case XPlane::OTHERVIEW:
            break;
    }
}


void XPlaneLegendWt::makeWPolarLegendBtns(XPlane::enumViews eXPlaneView, bool bHighlight)
{
    QVector<Plane const*> planelist;
    for (int j=0; j<Objects3d::nPlanes(); j++)
    {
        Plane const*pPlane = Objects3d::planeAt(j);
        for (int i=0; i<Objects3d::nPolars(); i++)
        {
            WPolar const*pWPolar = Objects3d::wPolarAt(i);
            if(pWPolar->planeName().compare(pPlane->name())==0 && pWPolar->hasPoints() && pWPolar->isVisible())
            {
                if(eXPlaneView==XPlane::STABPOLARVIEW)
                {
                    if(pWPolar->isStabilityPolar())
                    {
                        planelist.append(pPlane);
                        break;
                    }
                }
                else
                {
                    planelist.append(pPlane);
                    break;
                }
            }
        }
    }
    QString stylestring = QString::asprintf("color: %s; font-family: %s; font-weight: bold; font-size: %dpt;",
                                            DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                            DisplayOptions::textFontStruct().family().toStdString().c_str(),
                                            DisplayOptions::textFontStruct().pointSize());

    m_XflObjectMap.clear();

    QVBoxLayout *pLegendLayout = dynamic_cast<QVBoxLayout*>(layout());
    if(!pLegendLayout)
    {
        pLegendLayout = new QVBoxLayout;
        setLayout(pLegendLayout);
    }
    else
    {
        clearLayout(pLegendLayout);
    }

    for (int j=0; j<planelist.count(); j++)
    {
        Plane const *pPlane = planelist.at(j);
        QLabel *pPlaneLab = new QLabel(pPlane->name());
        pPlaneLab->setAttribute(Qt::WA_NoSystemBackground);
        pPlaneLab->setStyleSheet(stylestring);
        pLegendLayout->addWidget(pPlaneLab);

        for (int i=0; i<Objects3d::nPolars(); i++)
        {
            bool bShow = false;
            WPolar *pWPolar = Objects3d::wPolarAt(i);
            if (pWPolar->planeName()==pPlane->name() && pWPolar->hasPoints() && pWPolar->isVisible())
            {
                if(eXPlaneView==XPlane::STABPOLARVIEW)
                {
                    if(pWPolar->isStabilityPolar()) bShow = true;
                }
                else
                {
                    bShow = true;
                }
            }

            if(bShow)
            {
                LegendBtn *pLegendBtn = new LegendBtn;
                LineStyle ls(pWPolar->theStyle());
                ls.m_Tag = pWPolar->name();
                pLegendBtn->setStyle(ls);
                pLegendBtn->setBackground(true);
                if(pWPolar==s_pXPlane->m_pCurWPolar && Graph::isHighLighting() && bHighlight)
                    pLegendBtn->setHighLight(true);
                m_XflObjectMap.insert(pLegendBtn, pWPolar);

                connect(pLegendBtn, SIGNAL(clickedLB(LineStyle)),   SLOT(onClickedWPolarBtn()));
                connect(pLegendBtn, SIGNAL(clickedLine(LineStyle)), SLOT(onClickedWPolarBtnLine(LineStyle)));
                pLegendLayout->addWidget(pLegendBtn);
            }
        }
        pLegendLayout->addSpacing(16);
    }
    pLegendLayout->addStretch();
}


void XPlaneLegendWt::makePOppLegendBtns(bool bHighlight)
{
    QString boldstylestring = QString::asprintf("color: %s; font-family: %s; font-weight: bold; font-size: %dpt;",
                                                DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                                DisplayOptions::textFontStruct().family().toStdString().c_str(),
                                                DisplayOptions::textFontStruct().pointSize());
    QString stylestring = QString::asprintf("color: %s; font-family: %s; font-size: %dpt;",
                                            DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                            DisplayOptions::textFontStruct().family().toStdString().c_str(),
                                            DisplayOptions::textFontStruct().pointSize());

    //list the planes with a non-void wpolar
    bool bPOppVisible=false;
    QVector<Plane const*> planelist;
    for (int j=0; j<Objects3d::nPlanes(); j++)
    {
        Plane const *pPlane = Objects3d::planeAt(j);
        for (int i=0; i<Objects3d::nPOpps(); i++)
        {
            PlaneOpp const*pPOpp = Objects3d::POppAt(i);
            bPOppVisible = pPOpp->isVisible();
            if(s_pXPlane->curPOppOnly()) bPOppVisible = bPOppVisible && pPOpp==s_pXPlane->m_pCurPOpp;
            if(pPOpp->planeName().compare(pPlane->name())==0 && bPOppVisible)
            {
                planelist.append(pPlane);
                break;
            }
        }
    }

    m_XflObjectMap.clear();
    int row=1;

    QGridLayout *pLegendLayout = dynamic_cast<QGridLayout*>(layout());
    if(!pLegendLayout)
    {
        pLegendLayout = new QGridLayout;
        pLegendLayout->setColumnStretch(1,1);
        pLegendLayout->setColumnStretch(2,1);
        pLegendLayout->setColumnStretch(3,35);
        setLayout(pLegendLayout);
    }
    else
    {
        clearLayout(pLegendLayout);
    }

    for (int ipl=0; ipl<planelist.count(); ipl++)
    {
        Plane const *pPlane = planelist.at(ipl);
        QLabel *pPlaneLab = new QLabel(pPlane->name());
        pPlaneLab->setAttribute(Qt::WA_NoSystemBackground);
        pPlaneLab->setStyleSheet(boldstylestring);

        pLegendLayout->addWidget(pPlaneLab, row, 1, 1, 3);
        pLegendLayout->setRowStretch(row,0);
        row++;

        for(int ipl=0; ipl<Objects3d::nPolars(); ipl++)
        {
            WPolar const *pWPolar = Objects3d::wPolarAt(ipl);
            if(pWPolar->planeName()==pPlane->name())
            {
                // display the label only if the wpolar has children popps
                for (int ipo=Objects3d::nPOpps()-1; ipo>=0; ipo--)
                {
                    PlaneOpp *pPOpp = Objects3d::POppAt(ipo);
                    bPOppVisible = pPOpp->isVisible();
                    if(s_pXPlane->curPOppOnly()) bPOppVisible = bPOppVisible && pPOpp==s_pXPlane->m_pCurPOpp;

                    if (pPOpp->planeName()==pPlane->name() && pPOpp->polarName()==pWPolar->name() && bPOppVisible)
                    {
                        QLabel *pWPolarLab = new QLabel(pWPolar->name());
                        pWPolarLab->setAttribute(Qt::WA_NoSystemBackground);
                        pWPolarLab->setStyleSheet(stylestring);

                        pLegendLayout->addWidget(pWPolarLab, row, 2, 1, 2);
                        pLegendLayout->setRowStretch(row,0);
                        row++;
                        break;
                    }
                }
                for (int ipo=Objects3d::nPOpps()-1; ipo>=0; ipo--)
                {
                    PlaneOpp *pPOpp = Objects3d::POppAt(ipo);
                    bPOppVisible = pPOpp->isVisible();
                    if(s_pXPlane->curPOppOnly()) bPOppVisible = bPOppVisible && pPOpp==s_pXPlane->m_pCurPOpp;

                    if (pPOpp->planeName()==pPlane->name() && pPOpp->polarName()==pWPolar->name() && bPOppVisible)
                    {
                        LegendBtn *pLegendBtn = new LegendBtn;
                        LineStyle ls(pPOpp->theStyle());
                        ls.m_Tag = pPOpp->title(false);
                        pLegendBtn->setStyle(ls);
                        pLegendBtn->setBackground(true);
                        if(pPOpp==s_pXPlane->m_pCurPOpp && Graph::isHighLighting() && bHighlight)
                            pLegendBtn->setHighLight(true);
                        m_XflObjectMap.insert(pLegendBtn, pPOpp);

                        connect(pLegendBtn, SIGNAL(clickedLB(LineStyle)), SLOT(onClickedPOppBtn()));
        //                connect(pLegendBtn, SIGNAL(clickedRightLB(LineStyle)), SLOT(onClickedRightPOppBtn(LineStyle)));
                        connect(pLegendBtn, SIGNAL(clickedLine(LineStyle)), SLOT(onClickedPOppBtnLine(LineStyle)));
                        pLegendLayout->addWidget(pLegendBtn, row, 3, 1, 1);
                        pLegendLayout->setRowStretch(row,0);
                        row++;
                    }
                }// finished inventory
//                pLegendLayout->addSpacing(8);
            }
        }
//        pLegendLayout->addSpacing(16);
    }
    pLegendLayout->setRowStretch(row,1);
}


void XPlaneLegendWt::onClickedWPolarBtn()
{
    LegendBtn *pLegendBtn = dynamic_cast<LegendBtn*>(sender());
    WPolar *pWPolar = dynamic_cast<WPolar*>(m_XflObjectMap[pLegendBtn]);

    s_pXPlane->setPlane(pWPolar->planeName());
    s_pXPlane->setWPolar(pWPolar);
    s_pXPlane->m_pPlaneTreeView->selectWPolar(pWPolar, false);
    s_pXPlane->updateView();
}


void XPlaneLegendWt::onClickedWPolarBtnLine(LineStyle ls)
{
    LegendBtn *pLegendBtn = dynamic_cast<LegendBtn*>(sender());
    WPolar *pWPolar = dynamic_cast<WPolar*>(m_XflObjectMap[pLegendBtn]);

    LineMenu *pLineMenu = new LineMenu(nullptr);
    pLineMenu->initMenu(ls);
    pLineMenu->exec(QCursor::pos());
    ls = pLineMenu->theStyle();
    pWPolar->setTheStyle(ls);
    s_pXPlane->resetCurves();
    s_pXPlane->updateView();
    s_pXPlane->m_pPlaneTreeView->setCurveParams();

    emit s_pXPlane->projectModified();
}


void XPlaneLegendWt::onClickedPOppBtnLine(LineStyle ls)
{
    LegendBtn *pLegendBtn = dynamic_cast<LegendBtn*>(sender());
    PlaneOpp *pPOpp = dynamic_cast<PlaneOpp*>(m_XflObjectMap[pLegendBtn]);
    LineMenu *pLineMenu = new LineMenu(nullptr);
    pLineMenu->initMenu(ls);
    pLineMenu->exec(QCursor::pos());
    ls = pLineMenu->theStyle();
    pPOpp->setTheStyle(ls);
    s_pXPlane->resetCurves();
    s_pXPlane->m_pPlaneTreeView->setCurveParams();
    emit (s_pXPlane->projectModified());
    s_pXPlane->updateView();
}


void XPlaneLegendWt::onClickedPOppBtn()
{
    LegendBtn *pLegendBtn = dynamic_cast<LegendBtn*>(sender());
    PlaneOpp *pPOpp = dynamic_cast<PlaneOpp*>(m_XflObjectMap[pLegendBtn]);

    s_pXPlane->setPlane(pPOpp->planeName());
    s_pXPlane->setWPolar(pPOpp->polarName());
    s_pXPlane->setPlaneOpp(pPOpp); // will call createcurves and makelegend
    s_pXPlane->m_pPlaneTreeView->selectPlaneOpp(pPOpp);

    s_pXPlane->updateView();
}


