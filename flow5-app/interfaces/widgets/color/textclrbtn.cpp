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



#include <QPen>
#include <QPainter>
#include <QFontMetrics>


#include <interfaces/widgets/color/textclrbtn.h>


TextClrBtn::TextClrBtn(QWidget *parent)
    : QAbstractButton(parent)
{
    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    szPolicyExpanding.setVerticalPolicy(QSizePolicy::Minimum);
    setSizePolicy(szPolicyExpanding);

    m_bContour = true;

    m_TextColor = Qt::yellow;
    m_BackgroundColor = Qt::white;
}


void TextClrBtn::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        emit clickedTB();
    }
    else
        QWidget::mouseReleaseEvent(pEvent);
}


QSize TextClrBtn::sizeHint() const
{
    QFontMetrics fm(m_TextFont);
    int w = 13 * fm.averageCharWidth();
    int h = int((fm.height()*3)/2);
    return QSize(w, h);
}


void TextClrBtn::setTextColor(QColor const & TextColor)
{
    m_TextColor = TextColor;
    update();
}


void TextClrBtn::setBackgroundColor(QColor const & TextColor)
{
    m_BackgroundColor = TextColor;
    update();
}


void TextClrBtn::setFont(QFont const & font)
{
    m_TextFont = font;
    update();
}


void TextClrBtn::paintEvent(QPaintEvent *)
{
    QRect r = rect();

    QPainter painter(this);
    painter.save();

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(m_TextFont);

    QBrush backBrush(m_BackgroundColor);
    painter.setBrush(backBrush);
    painter.setBackgroundMode(Qt::TransparentMode);

    QColor ContourColor = Qt::lightGray;
    if(isEnabled())
    {
        if(isDown()) ContourColor = Qt::red;
        else         ContourColor = Qt::gray;
    }

    if(m_bContour)
    {
        QPen ContourPen(ContourColor);
        painter.setPen(ContourPen);
    }

/*    if(m_bRoundedRect)
    {
        r.adjust(0,2,-1,-3);
        painter.drawRoundedRect(r,5,40);
    }
    else*/
    {
        painter.drawRect(rect());
    }
    QPen LinePen(m_TextColor);
    painter.setPen(LinePen);
    painter.drawText(r, Qt::AlignCenter, text());

    painter.restore();
}
