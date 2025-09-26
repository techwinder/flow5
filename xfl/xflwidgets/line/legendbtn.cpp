/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QStyleOption>
#include <QMouseEvent>
#include <QPen>
#include <QPainter>

#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include "legendbtn.h"


LegendBtn::LegendBtn(QWidget *parent) : QWidget(parent)
{
    m_LineStyle.m_Color = Qt::darkGray;
    m_LineStyle.m_Stipple = Line::SOLID;
    m_LineStyle.m_Width = 1;
    m_LineStyle.m_Symbol = Line::NOSYMBOL;

    m_bHasBackGround = false;
    m_bIsCurrent     = false;
    m_bMouseHover    = false;
    m_bIsHighlighted = false;

    setAutoFillBackground(false);
    QString stylestring = QString::asprintf("color: %s; font: %s bold %dpt;",
                                            DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                            DisplayOptions::textFontStruct().family().toStdString().c_str(),
                                            DisplayOptions::textFontStruct().pointSize());
    setStyleSheet(stylestring);
    setMouseTracking(true);
}


void LegendBtn::resizeEvent(QResizeEvent *)
{
    m_LineRect.setRect(2*DisplayOptions::textFontStruct().averageCharWidth(), rect().top(), 7*DisplayOptions::textFontStruct().averageCharWidth(), rect().height());
    int spacer = DisplayOptions::textFontStruct().averageCharWidth();
    m_TagRect.setRect(m_LineRect.right()+spacer, rect().top(),
                      rect().width()-m_LineRect.width()-spacer, rect().height());
}


bool LegendBtn::event(QEvent* pEvent)
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


void LegendBtn::mousePressEvent(QMouseEvent *pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        pEvent->accept();
        if(m_LineRect.contains(pEvent->pos()))
            emit clickedLine(m_LineStyle);
        else
            emit clickedLB(m_LineStyle);
    }
    else
        QWidget::mousePressEvent(pEvent);
}


void LegendBtn::contextMenuEvent(QContextMenuEvent *pEvent)
{
    pEvent->accept();
    if(m_TagRect.contains(pEvent->pos()))
        emit clickedRightLB(m_LineStyle);
}


void LegendBtn::setStyle(LineStyle ls)
{
    m_LineStyle = ls;
    setToolTip(m_LineStyle.m_Tag);
    update();
}

QSize LegendBtn::sizeHint() const
{
    int labellength = int(DisplayOptions::textFontStruct().width(m_LineStyle.m_Tag)*1.1);
    int linelength = DisplayOptions::textFontStruct().averageCharWidth()*9;

    int h = int(double(DisplayOptions::textFontStruct().height())*1.15);

    return QSize(25+linelength+ labellength, h);
}


void LegendBtn::paintEvent(QPaintEvent *pEvent)
{
    QPainter painter(this);

    paintButton(painter);
    pEvent->accept();
}


void LegendBtn::paintButton(QPainter &painter)
{
    painter.save();
    QPalette palette;

    painter.setBackgroundMode(Qt::TransparentMode);

//    QColor backcolor = palette.window().color();
//    QColor textcolor = palette.windowText().color();

    QColor backcolor = DisplayOptions::backgroundColor();
    QColor textcolor = DisplayOptions::textColor();

    painter.setFont(DisplayOptions::textFontStruct().font());

    if(m_bIsCurrent)
    {
        QPen contourPen(palette.highlight().color());
        contourPen.setStyle(Qt::DotLine);
        painter.setPen(contourPen);
        painter.drawRect(rect().marginsRemoved(QMargins(2,2,2,2)));
        backcolor = palette.highlight().color();
    }

    QPen LinePen(m_LineStyle.m_Color);
    LinePen.setStyle(xfl::getStyle(m_LineStyle.m_Stipple));
    LinePen.setWidth(m_LineStyle.m_Width);
    if(m_bIsHighlighted)
    {
        LinePen.setWidth(m_LineStyle.m_Width+3);
    }

    painter.setPen(LinePen);

    painter.drawLine(m_LineRect.left(), m_LineRect.center().y(), m_LineRect.right(), m_LineRect.center().y());

    xfl::drawSymbol(painter, m_LineStyle.m_Symbol, backcolor, m_LineStyle.m_Color, m_LineRect.center());

    QPen textpen(textcolor);
    painter.setPen(textpen);

    if(m_LineStyle.m_Tag.length())
    {
//        qDebug()<<"paintnting legendbtn"<<font().family()<<font().pointSize()<<m_LineStyle.m_Tag;
        painter.drawText(m_TagRect, m_LineStyle.m_Tag);
    }

    painter.restore();
}
