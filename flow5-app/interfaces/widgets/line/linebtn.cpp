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


#include <QStyleOption>
#include <QMouseEvent>
#include <QPen>
#include <QPainter>

#include "linebtn.h"
#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/view2d/paint2d.h>


LineBtn::LineBtn(QWidget *parent) : QAbstractButton(parent)
{
    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    szPolicyExpanding.setVerticalPolicy(QSizePolicy::Minimum);
    setSizePolicy(szPolicyExpanding);

    m_LineStyle.m_Color.setRgb(200,200,200);
    m_LineStyle.m_Stipple = Line::SOLID;
    m_LineStyle.m_Width = 1;
    m_LineStyle.m_Symbol = Line::NOSYMBOL;

    m_bHasBackGround = false;
    m_bIsCurrent     = false;
    m_bMouseHover    = false;

    setMouseTracking(true);
}


LineBtn::LineBtn(LineStyle ls, QWidget *parent) : QAbstractButton(parent)
{
    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    szPolicyExpanding.setVerticalPolicy(QSizePolicy::Minimum);
    setSizePolicy(szPolicyExpanding);

    m_LineStyle = ls;

    m_bHasBackGround = false;
    m_bIsCurrent     = false;
    m_bMouseHover    = false;

    setMouseTracking(true);
}


bool LineBtn::event(QEvent* pEvent)
{
    if (pEvent->type() == QEvent::Enter)
    {
        m_bMouseHover = true;
        update();
    }
    if (pEvent->type()==QEvent::Leave)
    {
        m_bMouseHover = false;
        update();
    }
    return QWidget::event(pEvent); // Or whatever parent class you have.
}


void LineBtn::mousePressEvent(QMouseEvent *pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        emit clickedLB(m_LineStyle);
        pEvent->accept();
        return;
    }
    else
        QWidget::mousePressEvent(pEvent);
}


QSize LineBtn::sizeHint() const
{
    QFont font; // the application's default font, avoid custom font to ensure fixed size widget
    QFontMetrics fm(font);
    int w = 13 * fm.averageCharWidth();
    int h = int(double(fm.height()*1.5));
    return QSize(w, h);
}


QColor LineBtn::btnColor() const
{
    return xfl::fromfl5Clr(m_LineStyle.m_Color);
}


void LineBtn::setBtnColor(fl5Color const &color)
{
    m_LineStyle.m_Color = color;
    update();
}


void LineBtn::setBtnColor(QColor const &color)
{
    m_LineStyle.m_Color = xfl::tofl5Clr(color);
    update();
}


void LineBtn::setStipple(Line::enumLineStipple stipple)
{
    m_LineStyle.m_Stipple = stipple;
    update();
}


void LineBtn::setWidth(int width)
{
    m_LineStyle.m_Width = width;
    update();
}


void LineBtn::setPointStyle(Line::enumPointStyle pointstyle)
{
    m_LineStyle.m_Symbol = pointstyle;
    update();
}


void LineBtn::setTheStyle(Line::enumLineStipple style, int width, QColor const &color, Line::enumPointStyle pointstyle)
{
    setTheStyle(style, width, xfl::tofl5Clr(color), pointstyle);
    update();
}


void LineBtn::setTheStyle(Line::enumLineStipple style, int width, fl5Color const &color, Line::enumPointStyle pointstyle)
{
    m_LineStyle.m_Stipple = style;
    m_LineStyle.m_Width = width;
    m_LineStyle.m_Color = color;
    m_LineStyle.m_Symbol = pointstyle;
    update();
}


void LineBtn::setTheStyle(LineStyle const &ls)
{
    m_LineStyle = ls;
    update();
}


void LineBtn::paintEvent(QPaintEvent *pEvent)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);

    QRect r = rect();

    painter.setBackgroundMode(Qt::OpaqueMode);

    QPalette myPal;
    QColor backcolor= myPal.window().color();
    if(m_bMouseHover && isEnabled())
    {
        backcolor = myPal.highlight().color();
    }
    else if(m_bHasBackGround)
    {
        backcolor = DisplayOptions::backgroundColor();
    }

    painter.setBackgroundMode(Qt::TransparentMode);

    if(m_bHasBackGround)
    {
        QPen contourpen(backcolor,1,Qt::SolidLine);
        QBrush colorbrush(backcolor);
        painter.setBrush(colorbrush);

        painter.setPen(contourpen);
        painter.drawRect(r);
    }

    QPen LinePen;
    QColor linecolor;
    LinePen.setStyle(xfl::getStyle(m_LineStyle.m_Stipple));
    LinePen.setWidth(m_LineStyle.m_Width);
    if(isEnabled()) linecolor = xfl::fromfl5Clr(m_LineStyle.m_Color);
    else            linecolor = Qt::darkGray;

    LinePen.setColor(linecolor);
    painter.setPen(LinePen);
/*    if(m_LineStyle.m_Stipple==Line::NOLINE && m_LineStyle.m_Symbol==Line::NOSYMBOL)
    {
        QPen textpen(m_LineStyle.m_Color);
        painter.setPen(textpen);
        painter.drawText(r, Qt::AlignCenter, "-none-");
    }
    else*/
    {
        painter.drawLine(r.left()+5, r.center().y(), r.width()-5, r.center().y());
    }
    LinePen.setStyle(Qt::SolidLine);
    painter.setPen(LinePen);
    xfl::drawSymbol(painter, m_LineStyle.m_Symbol, DisplayOptions::backgroundColor(), linecolor, r.center());

    if(m_bIsCurrent)
    {
        QPalette myPal;
        QPen contourpen(myPal.highlight().color());
        contourpen.setStyle(Qt::DotLine);
        contourpen.setWidth(2);
        painter.setPen(contourpen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(r.marginsRemoved(QMargins(2,2,2,2)));
    }

    painter.restore();
    pEvent->accept();
}


