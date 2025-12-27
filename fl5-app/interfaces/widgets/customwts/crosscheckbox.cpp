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

#include "crosscheckbox.h"
#include <interfaces/widgets/globals/wt_globals.h>

CrossCheckBox::CrossCheckBox(QWidget *pParent) : QWidget(pParent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_State = Qt::Unchecked;
    m_WidthHint = 32;
}


void CrossCheckBox::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QColor backcolor    = palette().window().color();
    QColor crosscolor   = palette().windowText().color();
    QColor contourcolor = palette().mid().color();


//    wt::drawCheckBox(&painter, m_State, rect(), DisplayOptions::treeFontStruct().height(), false, true, crosscolor, backcolor, contourcolor);
    wt::drawCheckBox(&painter, m_State, rect(),  height()*0.8, false, true, crosscolor, backcolor, contourcolor);

}


QSize CrossCheckBox::minimumSizeHint() const
{
    return sizeHint();
}


QSize CrossCheckBox::sizeHint() const
{
//    return QSize(DisplayOptions::treeFontStruct().height()*1.5, DisplayOptions::treeFontStruct().height()*1.5);
    return QSize(m_WidthHint, m_WidthHint);
}


void CrossCheckBox::mouseReleaseEvent(QMouseEvent *)
{
    if(m_State==Qt::Checked)
    {
        m_State = Qt::Unchecked;
        emit clicked(false);
    }
    else
    {
        m_State = Qt::Checked;
        emit clicked(true);
    }
    update();
}
