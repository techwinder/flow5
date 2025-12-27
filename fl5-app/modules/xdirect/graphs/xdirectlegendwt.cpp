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
#include <QLabel>


#include <modules/xdirect/graphs/xdirectlegendwt.h>

#include <modules/xdirect/xdirect.h>
#include <modules/xdirect/controls/foilexplorer.h>

#include <interfaces/graphs/graph/graph.h>
#include <core/displayoptions.h>
#include <api/enums_objects.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <api/oppoint.h>
#include <api/polar.h>
#include <interfaces/widgets/line/legendbtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <interfaces/widgets/globals/wt_globals.h>

XDirect* XDirectLegendWt::s_pXDirect = nullptr;

XDirectLegendWt::XDirectLegendWt(QWidget *pParent) : LegendWt(pParent)
{
    setAutoFillBackground(true);
}


void XDirectLegendWt::makeLegend(bool bHighlight)
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    palette.setColor(QPalette::Window,     DisplayOptions::backgroundColor());
    palette.setColor(QPalette::Base,       DisplayOptions::backgroundColor());
//    setPalette(palette);

    if(s_pXDirect->isPolarView())
        makePolarLegendBtns(bHighlight);
    else
        makeOppLegendBtns(bHighlight);
}


void XDirectLegendWt::makePolarLegendBtns(bool bHighlight)
{
    setStyleSheet(QString::asprintf("background: %s;", DisplayOptions::backgroundColor().name(QColor::HexRgb).toStdString().c_str()));
    setAutoFillBackground(true);
    //list the foils with a non-void polar
    QVector<Foil*> foillist;
    for (int j=0; j<Objects2d::nFoils(); j++)
    {
        Foil *pFoil = Objects2d::foil(j);
        for (int i=0; i<Objects2d::nPolars(); i++)
        {
            Polar *pPolar = Objects2d::polarAt(i);
            /** @todo add polar filter test */
            if(pPolar->foilName().compare(pFoil->name())==0 && pPolar->hasData() && pPolar->isVisible())
            {
                foillist.append(pFoil);
                break;
            }
        }
    }

    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    QColor clr = DisplayOptions::backgroundColor();
    clr.setAlpha(0);
    palette.setColor(QPalette::Window, clr);
    palette.setColor(QPalette::Base, clr);


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
        wt::clearLayout(pLegendLayout);
    }

    for (int j=0; j<foillist.size(); j++)
    {
        Foil const *pFoil = foillist.at(j);
        QLabel *pFoilLab = new QLabel(QString::fromStdString(pFoil->name()));
        pFoilLab->setAttribute(Qt::WA_NoSystemBackground);
        pFoilLab->setStyleSheet(stylestring);
        pLegendLayout->addWidget(pFoilLab);

        for (int i=0; i<Objects2d::nPolars(); i++)
        {
            Polar *pPolar = Objects2d::polarAt(i);
            if (pPolar->foilName()==pFoil->name() && pPolar->hasData() && pPolar->isVisible())
            {
                LegendBtn *pLegendBtn = new LegendBtn;
                LineStyle ls(pPolar->theStyle());
                ls.m_Tag = pPolar->name();
                pLegendBtn->setStyle(ls);
                pLegendBtn->setBackground(true);
                if(pPolar==XDirect::curPolar() && Graph::isHighLighting() && bHighlight)
                    pLegendBtn->setHighLight(true);
                m_XflObjectMap.insert(pLegendBtn, pPolar);

                connect(pLegendBtn, SIGNAL(clickedLB(LineStyle)),   SLOT(onClickedPolarBtn()));
                connect(pLegendBtn, SIGNAL(clickedLine(LineStyle)), SLOT(onClickedPolarBtnLine(LineStyle)));
                pLegendLayout->addWidget(pLegendBtn);
            }
        }
        pLegendLayout->addSpacing(16);
    }
    pLegendLayout->addStretch();
}


void XDirectLegendWt::makeOppLegendBtns(bool bHighlight)
{
    setStyleSheet(QString::asprintf("background: %s;", DisplayOptions::backgroundColor().name(QColor::HexRgb).toStdString().c_str()));
    setAutoFillBackground(true);

    QString stylestring = QString::asprintf("color: %s; font-family: %s; font-weight: bold; font-size: %dpt;",
                                            DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                            DisplayOptions::textFontStruct().family().toStdString().c_str(),
                                            DisplayOptions::textFontStruct().pointSize());

    bool bOppVisible=false;
    //list the foils with a non-void opp
    QVector<Foil*> foillist;
    for (int j=0; j<Objects2d::nFoils(); j++)
    {
        Foil *pFoil = Objects2d::foil(j);
        for (int i=0; i<Objects2d::nOpPoints(); i++)
        {
            OpPoint *pOpp = Objects2d::opPointAt(i);
            bOppVisible = pOpp->isVisible();
            if(s_pXDirect->curOppOnly()) bOppVisible &= (pOpp==XDirect::curOpp());
            if(pOpp->foilName().compare(pFoil->name())==0 && bOppVisible)
            {
                foillist.append(pFoil);
                break;
            }
        }
    }

    m_XflObjectMap.clear();

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

    for (int j=0; j<foillist.count(); j++)
    {
        Foil *pFoil = foillist.at(j);
        QLabel *pFoilLab = new QLabel(QString::fromStdString(pFoil->name()));
        pFoilLab->setAttribute(Qt::WA_NoSystemBackground);
        pFoilLab->setStyleSheet(stylestring);

        pLegendLayout->addWidget(pFoilLab);

        for (int i=0; i<Objects2d::nOpPoints(); i++)
        {
            OpPoint *pOpp = Objects2d::opPointAt(i);
            bOppVisible = pOpp->isVisible();
            if(s_pXDirect->curOppOnly()) bOppVisible &= (pOpp==XDirect::curOpp());
            if (pOpp->foilName()==pFoil->name() && bOppVisible)
            {
                LegendBtn *pLegendBtn = new LegendBtn;
                LineStyle ls(pOpp->theStyle());
                ls.m_Tag = pOpp->name();
                pLegendBtn->setStyle(ls);
                pLegendBtn->setBackground(true);
                if(pOpp==XDirect::curOpp() && Graph::isHighLighting() && bHighlight) pLegendBtn->setHighLight(true);
                m_XflObjectMap.insert(pLegendBtn, pOpp);

                connect(pLegendBtn, SIGNAL(clickedLB(LineStyle)), this, SLOT(onClickedOppBtn()));
                connect(pLegendBtn, SIGNAL(clickedLine(LineStyle)), this, SLOT(onClickedOppBtnLine(LineStyle)));
                pLegendLayout->addWidget(pLegendBtn);
            }
        }// finished inventory
        pLegendLayout->addSpacing(16);
    }
    pLegendLayout->addStretch();
}


void XDirectLegendWt::onClickedPolarBtn()
{
    LegendBtn *pLegendBtn = dynamic_cast<LegendBtn*>(sender());
    Polar *pPolar = dynamic_cast<Polar*>(m_XflObjectMap[pLegendBtn]);
    s_pXDirect->setFoil(Objects2d::foil(pPolar->foilName()));
    s_pXDirect->setPolar(pPolar);
    s_pXDirect->m_pFoilExplorer->selectPolar(pPolar);
    s_pXDirect->updateView();
}


void XDirectLegendWt::onClickedPolarBtnLine(LineStyle ls)
{
    LegendBtn *pLegendBtn = dynamic_cast<LegendBtn*>(sender());
    Polar *pPolar = dynamic_cast<Polar*>(m_XflObjectMap[pLegendBtn]);

    LineMenu *pLineMenu = new LineMenu(nullptr);
    pLineMenu->initMenu(ls);
    pLineMenu->exec(QCursor::pos());
    ls = pLineMenu->theStyle();
    pPolar->setLineStipple(ls.m_Stipple);
    pPolar->setLineWidth(ls.m_Width);
    pPolar->setLineColor(ls.m_Color);
    pPolar->setPointStyle(ls.m_Symbol);
    s_pXDirect->resetCurves();
    s_pXDirect->m_pFoilExplorer->setCurveParams();
    s_pXDirect->updateView();
    emit s_pXDirect->projectModified();
}


void XDirectLegendWt::onClickedOppBtnLine(LineStyle ls)
{
    LegendBtn *pLegendBtn = dynamic_cast<LegendBtn*>(sender());
    OpPoint *pOpp = dynamic_cast<OpPoint*>(m_XflObjectMap[pLegendBtn]);

    LineMenu *pLineMenu = new LineMenu(nullptr);
    pLineMenu->initMenu(ls);
    pLineMenu->exec(QCursor::pos());
    ls = pLineMenu->theStyle();
    pOpp->setLineStipple(ls.m_Stipple);
    pOpp->setLineWidth(ls.m_Width);
    pOpp->setLineColor(ls.m_Color);
    pOpp->setPointStyle(ls.m_Symbol);
    s_pXDirect->resetCurves();
    s_pXDirect->m_pFoilExplorer->setCurveParams();
    s_pXDirect->updateView();
    emit s_pXDirect->projectModified();
}


void XDirectLegendWt::onClickedOppBtn()
{
    LegendBtn *pLegendBtn = dynamic_cast<LegendBtn*>(sender());
    OpPoint *pOpp = dynamic_cast<OpPoint*>(m_XflObjectMap[pLegendBtn]);
    s_pXDirect->setFoil(Objects2d::foil(pOpp->foilName()));
    s_pXDirect->setPolar(Objects2d::polar(pOpp->foilName(), pOpp->polarName()));
    s_pXDirect->setOpp(pOpp);
    s_pXDirect->m_pFoilExplorer->selectOpPoint(pOpp);
    s_pXDirect->updateView();
}







