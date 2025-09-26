/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include "crosscheckbox.h"
#include <xflwidgets/wt_globals.h>

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


//    drawCheckBox(&painter, m_State, rect(), DisplayOptions::treeFontStruct().height(), false, true, crosscolor, backcolor, contourcolor);
    drawCheckBox(&painter, m_State, rect(),  height()*0.8, false, true, crosscolor, backcolor, contourcolor);

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
