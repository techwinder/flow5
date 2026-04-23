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



#include <QCoreApplication>
#include <QPainter>
#include <QIcon>


#include "linemenu.h"
#include <core/xflcore.h>
#include <interfaces/widgets/line/linepicker.h>

LineMenu::LineMenu(QWidget *pParent, bool bShowPointStyleMenu) : QMenu(pParent)
{
    setTitle(tr("Style"));
    m_pParentMenu = nullptr;
    m_pPointStyleMenu = m_pLineStyleMenu = m_pLineWidthMenu = m_pLineColorMenu = nullptr;
    m_bEnableLineStyle = m_bEnablePointStyle = true;
    m_bEnableLineWidth = m_bEnableLineColor = false;

    m_pck = new LinePicker(this);
    fillMenu(bShowPointStyleMenu);

    m_bStyleChanged = m_bWidthChanged = m_bColorChanged = m_bPointsChanged = false;
}


LineMenu::~LineMenu()
{
    delete m_pck;
    m_pck = nullptr;
}


void LineMenu::enableSubMenus()
{
    if(m_pPointStyleMenu) m_pPointStyleMenu->setEnabled(m_bEnablePointStyle);
    m_pLineStyleMenu->setEnabled(m_bEnableLineStyle);
    m_pLineWidthMenu->setEnabled(m_bEnableLineWidth);
    m_pLineColorMenu->setEnabled(m_bEnableLineColor);
}


void LineMenu::enableSubMenus(bool blineStyle, bool bLineWidth, bool bLineColor, bool bPointStyle)
{
    m_bEnableLineStyle = blineStyle;
    m_bEnableLineWidth = bLineWidth;
    m_bEnableLineColor = bLineColor;
    m_bEnablePointStyle = bPointStyle;
    enableSubMenus();
}


void LineMenu::initMenu(LineStyle ls)
{
    m_bStyleChanged = m_bWidthChanged = m_bColorChanged = m_bPointsChanged = false;
    m_theStyle = ls;

    m_pck->initDialog(m_theStyle);

    updateLineActions();
}


void LineMenu::initMenu(Line::enumLineStipple style, int width, const fl5Color &color, Line::enumPointStyle pointStyle)
{
    m_bStyleChanged = m_bWidthChanged = m_bColorChanged = m_bPointsChanged = false;
    m_theStyle.m_Stipple  = style;
    m_theStyle.m_Width  = width;
    m_theStyle.m_Color = color;
    m_theStyle.m_Symbol = pointStyle;

    m_pck->initDialog(m_theStyle);

    updateLineActions();
}


void LineMenu::initMenu(Line::enumLineStipple style, int width, const QColor &color, Line::enumPointStyle pointStyle)
{
    initMenu(style, width, xfl::tofl5Clr(color), pointStyle);
}


void LineMenu::updateLineActions()
{
    emit(lineChanged(m_theStyle));

    for(int i=0; i<NLINESTYLES; i++)
    {
        m_LineStyleAction[i].lineBtn().setTheStyle(m_theStyle);
        m_LineStyleAction[i].lineBtn().setStipple(LineStyle::convertLineStyle(i));
        m_LineStyleAction[i].lineBtn().setCurrent(i==m_theStyle.m_Stipple);
    }
    for(int i=0; i<NLINEWIDTHS; i++)
    {
        m_LineWidthAction[i].lineBtn().setTheStyle(m_theStyle);
        m_LineWidthAction[i].lineBtn().setWidth(i+1);
        m_LineWidthAction[i].lineBtn().setCurrent(i+1==m_theStyle.m_Width);
    }
    for(int i=0; i<NPOINTSTYLES; i++)
    {
        m_SymbolAction[i].lineBtn().setTheStyle(m_theStyle);
        m_SymbolAction[i].lineBtn().setPointStyle(LineStyle::convertSymbol(i));
        m_SymbolAction[i].lineBtn().setCurrent(i==m_theStyle.m_Symbol);
    }
}


void LineMenu::showPointStyle(bool bShow)
{
    m_pPointStyleMenu->setVisible(bShow);
    m_pPointStyleMenu->hide();
    m_pPointStyleMenu->setEnabled(false);
}


void LineMenu::fillMenu(bool bShowPointStyleMenu)
{
    if(bShowPointStyleMenu)
    {
        m_pPointStyleMenu = addMenu(tr("Symbols"));
        for(int i=0; i<NPOINTSTYLES; i++)
        {
            m_SymbolAction[i].setCheckable(true);
            m_SymbolAction[i].lineBtn().setBackground(true);
            connect(m_SymbolAction+i, SIGNAL(triggered(bool)), this, SLOT(onPointStyle()));
            m_pPointStyleMenu->addAction(m_SymbolAction+i);
        }
    }

    m_pLineStyleMenu = addMenu(tr("Style"));
    for(int i=0; i<NLINESTYLES; i++)
    {
        m_LineStyleAction[i].setCheckable(true);
        m_LineStyleAction[i].lineBtn().setBackground(true);
        connect(m_LineStyleAction+i, SIGNAL(triggered(bool)), this, SLOT(onLineStyle()));
        m_pLineStyleMenu->addAction(m_LineStyleAction+i);
    }

    m_pLineWidthMenu = addMenu(tr("Thickness"));
    for(int i=0; i<NLINEWIDTHS; i++)
    {
        m_LineWidthAction[i].setCheckable(true);
        m_LineWidthAction[i].lineBtn().setBackground(true);
        connect(m_LineWidthAction+i, SIGNAL(triggered(bool)), this, SLOT(onLineWidth()));
        m_pLineWidthMenu->addAction(m_LineWidthAction+i);
    }

    m_pLineColorMenu = addMenu(tr("Colour"));
    connect(m_pck, SIGNAL(colorChanged(QColor)), this, SLOT(onColorChanged(QColor)));
    QWidgetAction *pColorAction = new QWidgetAction(this);
    pColorAction->setDefaultWidget(m_pck);
    m_pLineColorMenu->addAction(pColorAction);
}


void LineMenu::onPointStyle()
{
    m_bPointsChanged = true;
    LineAction *pLineAction = qobject_cast<LineAction *>(sender());
    m_theStyle.m_Symbol = pLineAction->lineBtn().btnPointStyle();
    updateLineActions();

    if(m_pParentMenu) m_pParentMenu->close();
    else close();
}


void LineMenu::onLineStyle()
{
    m_bStyleChanged = true;
    LineAction *pLineAction = qobject_cast<LineAction *>(sender());
    m_theStyle.m_Stipple = pLineAction->lineBtn().btnStyle();
    updateLineActions();
    if(m_pParentMenu) m_pParentMenu->close();
    else close();
}


void LineMenu::onLineWidth()
{
    m_bWidthChanged = true;
    LineAction *pLineAction = qobject_cast<LineAction *>(sender());
    m_theStyle.m_Width = pLineAction->lineBtn().btnWidth();
    updateLineActions();
    if(m_pParentMenu) m_pParentMenu->close();
    else close();
}


void LineMenu::onColorChanged(QColor clr)
{
    m_bColorChanged = true;
    m_theStyle.m_Color = xfl::tofl5Clr(clr);
    updateLineActions();
    if(m_pParentMenu) m_pParentMenu->close();
    else close();
}


void LineMenu::setColor(QColor const &color)
{
    m_theStyle.m_Color=xfl::tofl5Clr(color);
}


