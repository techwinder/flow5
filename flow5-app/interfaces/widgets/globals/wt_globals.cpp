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

#include <cmath>

#include <QApplication>
#include <QLayout>
#include <QLayoutItem>
#include <QPalette>
#include <QFontMetrics>
#include <QTabWidget>
#include <QStackedWidget>

#include <interfaces/widgets/globals/wt_globals.h>


void wt::drawCheckBox(QPainter *painter, bool bChecked, QRect const & r, int side, bool bBackground, bool bContour,
                  const QColor &crossclr, const QColor &backclr)
{
    if(bChecked) drawCheckBox(painter, Qt::Checked,   r, side, bBackground, bContour, crossclr, backclr, Qt::black);
    else         drawCheckBox(painter, Qt::Unchecked, r, side, bBackground, bContour, crossclr, backclr, Qt::black);
}


void wt::drawCheckBox(QPainter *painter, Qt::CheckState state, QRect const & r, int side, bool bBackground, bool bContour,
                  QColor const &crossclr, QColor const &backclr, QColor const &ContourClr)
{
    int h3 = int(double(side)/4.0);

    QPoint center = r.center();
//    QRect contourrect = QRect(center.x()-h3-3, center.y()-h3-3, 2*h3+4, 2*h3+4);
//    QRect crossrect   = QRect(center.x()-h3,   center.y()-h3,   2*h3,   2*h3);
//    QRect contourrect = QRect(center.x()-h3-2, center.y()-h3-2, 2*(h3+2), 2*(h3+2));

    QRect crossrect   = QRect(center.x()-h3,   center.y()-h3,   2*h3,   2*h3);
    QRect contourrect = crossrect.adjusted(-4,-4,3,3);

    painter->save();

    if(bBackground)
    {
        painter->fillRect(r, QBrush(backclr));
    }
    painter->setBackgroundMode(Qt::TransparentMode);

    if(state==Qt::Checked)
    {
        QPen checkPen;

        checkPen.setColor(crossclr);
        checkPen.setWidth(2);
        painter->setPen(checkPen);
        painter->drawLine(crossrect.left(), crossrect.bottom(), crossrect.right(), crossrect.top());
        painter->drawLine(crossrect.left(), crossrect.top(),    crossrect.right(), crossrect.bottom());
    }
    else if(state==Qt::PartiallyChecked)
    {
        painter->setPen(QPen(Qt::gray));
        painter->setBrush(QBrush(Qt::gray));
        painter->drawRoundedRect(contourrect, 1, 1, Qt::AbsoluteSize);
    }
    else if(state==Qt::Unchecked)
    {
        // don't draw anything
    }

    if(bContour)
    {
        QPen contourpen(ContourClr);
        contourpen.setWidth(1);
        painter->setPen(contourpen);
        painter->drawRect(contourrect);
    }

    painter->restore();

}


QString wt::formatDouble(double d, int decimaldigits, bool bLocalize)
{
    QString str;
    QString format = bLocalize ? "%L1" : "%1";
    if(decimaldigits<0)
    {
        str=QString(format).arg(d,0,'g');
    }
    else
    {
        if ((fabs(d)<=1.e-30 || fabs(d)>=pow(10.0, -decimaldigits)) && d <10000000.0)
        {
            str=QString(format).arg(d,0,'f', decimaldigits);
        }
        else
        {
            str=QString(format).arg(d,0,'g', decimaldigits+1);
        }
    }

    return str;
}


void wt::removeLayout(QWidget* pWidget)
{
    qDeleteAll(pWidget->children());
}


void wt::clearLayout(QLayout *pLayout)
{
    QLayoutItem *pChild = nullptr;
    while ((pChild=pLayout->takeAt(0)) != nullptr)
    {
        if(pChild->layout() != nullptr)
            wt::clearLayout( pChild->layout() );
        else if(pChild->widget() != nullptr)
            delete pChild->widget();
        delete pChild;
    }
}


void wt::setLayoutStyle(QLayout *pLayout, QPalette const &palette)
{
    for (int i=0; i<pLayout->count(); i++)
    {
        QWidget *pWidget = pLayout->itemAt(i)->widget();
        if (pWidget)
            wt::setWidgetStyle(pWidget, palette);
        else
        {
            QLayout *pSubLayout =  pLayout->itemAt(i)->layout();
            if(pSubLayout)
                setLayoutStyle(pSubLayout, palette);
        }
    }
}


void wt::setWidgetStyle(QWidget *pWidget, QPalette const &palette)
{
    pWidget->setPalette(palette);
//    pWidget->setAttribute(Qt::WA_PaintUnclipped);
    pWidget->setAutoFillBackground(false);
    if(pWidget->layout())
        setLayoutStyle(pWidget->layout(), palette);

    QStackedWidget *pStackWt = dynamic_cast<QStackedWidget*>(pWidget);
    if(pStackWt)
    {
        for (int j=0; j<pStackWt->count(); j++)
        {
            wt::setWidgetStyle(pStackWt->widget(j), palette);
        }
    }

    QTabWidget *pTabWt = dynamic_cast<QTabWidget*>(pWidget);
    if(pTabWt)
    {
        for (int j=0; j<pTabWt->count(); j++)
        {
            wt::setWidgetStyle(pTabWt->widget(j), palette);
        }
    }
}
