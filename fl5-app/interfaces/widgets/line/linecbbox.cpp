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
#include <QPaintEvent>

#include "linecbbox.h"
#include <interfaces/view2d/paint2d.h>
#include <core/xflcore.h>

LineCbBox::LineCbBox(QWidget *pParent) : QComboBox(pParent)
{
    for (int i=0; i<5; i++)
    {
        addItem("item");
    }

    m_pLineDelegate = new LineDelegate(this);
    setItemDelegate(m_pLineDelegate);

    setParent(pParent);
    m_LineStyle.m_Stipple = Line::SOLID;
    m_LineStyle.m_Width = 1;
    m_LineStyle.m_Color = fl5Color(255,100,50);
    m_LineStyle.m_Symbol = Line::NOSYMBOL;

    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    szPolicyExpanding.setVerticalPolicy(QSizePolicy::Minimum);
    setSizePolicy(szPolicyExpanding);
}


void LineCbBox::setLine(Line::enumLineStipple style, int width, QColor const &color, Line::enumPointStyle pointStyle)
{
    setLine(style, width, xfl::tofl5Clr(color), pointStyle);
}


void LineCbBox::setLine(Line::enumLineStipple style, int width, fl5Color const &color, Line::enumPointStyle pointStyle)
{
    m_LineStyle.m_Stipple = style;
    m_LineStyle.m_Width = width;
    m_LineStyle.m_Color = color;
    m_LineStyle.m_Symbol = pointStyle;
}


void LineCbBox::setLine(LineStyle ls)
{
    m_LineStyle = ls;
    m_pLineDelegate->setLineColor(xfl::fromfl5Clr(ls.m_Color));
}


void LineCbBox::paintEvent (QPaintEvent *)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    painter.save();

    //    painter.setRenderHint(QPainter::Antialiasing);
    //    QColor ContourColor = Qt::gray;
    //    if(!isEnabled()) ContourColor = Qt::lightGray;

//    QRect r = pEvent->rect();
    QRect r = opt.rect;
    painter.setBrush(Qt::NoBrush);
    painter.setBackgroundMode(Qt::TransparentMode);

    QPen LinePen(xfl::fromfl5Clr(m_LineStyle.m_Color));
    LinePen.setStyle(xfl::getStyle(m_LineStyle.m_Stipple));
    LinePen.setWidth(m_LineStyle.m_Width);
    painter.setPen(LinePen);
    painter.drawLine(r.left()+5, r.center().y(), r.width()-30, r.center().y());

    if(m_LineStyle.m_Symbol>0)
    {
        LinePen.setStyle(Qt::SolidLine);
        painter.setPen(LinePen);

        QPalette palette;
        xfl::drawSymbol(painter, m_LineStyle.m_Symbol, palette.window().color(), m_LineStyle.m_Color, r.center());
    }

//    painter.setPen(Qt::blue);
//    painter.drawRect(r);

    painter.restore();
}

LineDelegate::LineDelegate(LineCbBox *pCbBox)
    : QAbstractItemDelegate(pCbBox)
{
    //initialize with something, just in case
    m_LineStyle.clear();
    m_LineWidth.clear();
    m_PointStyle.clear();
    for (int i=0; i<NLINESTYLES; i++)
    {
        m_LineStyle.push_back(i);
    }
    for (int i=0; i<NLINEWIDTHS; i++)
    {
        m_LineWidth.push_back(i+1);
    }
    for (int i=0; i<NPOINTSTYLES; i++)
    {
        m_PointStyle.push_back(Line::NOSYMBOL);
    }

    m_LineColor = QColor(0,255,0);
    m_Size.setHeight(15);
    m_Size.setWidth(50);

    m_pCbBox = pCbBox;
}


void LineDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected) painter->fillRect(option.rect, option.palette.highlight());

    painter->save();

    QPen LinePen(m_LineColor);
    switch(m_LineStyle[index.row()])
    {
        default:
        case 0:        LinePen.setStyle(Qt::SolidLine); break;
        case 1:        LinePen.setStyle(Qt::DashLine); break;
        case 2:        LinePen.setStyle(Qt::DotLine); break;
        case 3:        LinePen.setStyle(Qt::DashDotLine); break;
        case 4:        LinePen.setStyle(Qt::DashDotLine); break;
    }
    LinePen.setWidth(m_LineWidth[index.row()]);
    painter->setPen(LinePen);

    //    if (option.state & QStyle::State_Selected)  painter->setBrush(option.palette.highlightedText());
    //    else                                        painter->setBrush(QBrush(Qt::black));

    painter->drawLine(option.rect.x()+3,
                      option.rect.center().y(),
                      option.rect.width()-6,
                      option.rect.center().y());


    if(m_pCbBox && m_pCbBox->points())
    {
        LinePen.setStyle(Qt::SolidLine);
        painter->setPen(LinePen);
        xfl::drawSymbol(*painter, m_PointStyle[index.row()], QColor(255,255,255), Qt::white, option.rect.center());
    }
    painter->restore();
}


QSize LineDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    QFont fnt;
    QFontMetrics fm(fnt);
    int w = 7 * fm.averageCharWidth();
    int h = fm.height();
    return QSize(w, h);
}


void LineDelegate::setLineStyle(int *style)
{
    for (int i=0; i<NLINESTYLES; i++)    m_LineStyle[i] = style[i];
}


void LineDelegate::setLineWidth(int *width)
{
    for (int i=0; i<NLINEWIDTHS; i++)    m_LineWidth[i] = width[i];
}










