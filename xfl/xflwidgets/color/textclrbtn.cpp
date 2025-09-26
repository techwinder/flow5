/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#include <QPen>
#include <QPainter>
#include <QFontMetrics>


#include <xflwidgets/color/textclrbtn.h>


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
