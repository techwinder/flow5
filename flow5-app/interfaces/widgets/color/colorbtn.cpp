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

#include <QMenu>
#include <QPainter>
#include <QStyleOption>
#include <QWidgetAction>

#include <interfaces/widgets/color/colorbtn.h>

ColorBtn::ColorBtn(QWidget *pParent) : QAbstractButton(pParent)
{
    m_bHasBackGround = false;
    m_bIsCurrent     = false;
    m_bMouseHover    = false;
    m_Color = Qt::darkGray;

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}


ColorBtn::ColorBtn(QColor const &clr, QWidget *pParent): QAbstractButton(pParent)
{
    m_bHasBackGround = false;
    m_bIsCurrent     = false;
    m_bMouseHover    = false;
    m_Color = clr;

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}


QSize ColorBtn::minimumSizeHint() const
{
    QFont font;
    QFontMetrics fm(font);
//    int w = std::max(25, 3*fm.averageCharWidth());
    int w = 3*fm.averageCharWidth();
    int h = int(double(fm.height())*1.5);
    return QSize(w, h);
}


void ColorBtn::setColor(QColor const & color)
{
    m_Color = color;
    update();
}


void ColorBtn::mousePressEvent(QMouseEvent *pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        emit clicked(false);
        emit clickedCB(m_Color);
        pEvent->accept();
        return;
    }
    else
    {
        QAbstractButton::mousePressEvent(pEvent);
    }
}


void ColorBtn::onSetColor(QColor clr)
{
    m_Color = clr;
    update();
}


bool ColorBtn::event(QEvent* pEvent)
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


void ColorBtn::paintEvent(QPaintEvent *pEvent)
{
    QColor paintcolor;

    if(isEnabled()) paintcolor = m_Color;
    else
    {
        if(isDown()) paintcolor = m_Color.lighter(150);
        else         paintcolor = Qt::lightGray;
    }

//    paintcolor.setAlpha(255);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBackgroundMode(Qt::TransparentMode);
    QRect r = rect();


    QPen blackPen(Qt::black, 1, Qt::SolidLine);
    QBrush colorbrush(paintcolor);
    painter.setBrush(colorbrush);
    painter.setPen(blackPen);
    painter.drawRoundedRect(r, 5, 25, Qt::RelativeSize);

    QPalette palette;

    if(m_bMouseHover && isEnabled())
    {
        QPen contourpen(palette.highlight().color());
        contourpen.setStyle(Qt::SolidLine);
        contourpen.setWidth(2);
        painter.setPen(contourpen);
        painter.drawRect(r.marginsRemoved(QMargins(2,2,2,2)));
    }
    else if(m_bIsCurrent)
    {
        QPen contourpen(palette.highlight().color());
        contourpen.setStyle(Qt::DotLine);
        contourpen.setWidth(3);
        painter.setPen(contourpen);
        painter.drawRect(r.marginsRemoved(QMargins(2,2,2,2)));
    }

    pEvent->accept();
}

