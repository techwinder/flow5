/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#include "lineaction.h"




LineAction::LineAction(QObject *pParent) : QWidgetAction(pParent)
{
    setDefaultWidget(&m_LineBtn);
    connect(&m_LineBtn, SIGNAL(clickedLB(LineStyle)), this, SLOT(onClickedLB(LineStyle)));
}



void LineAction::onClickedLB(LineStyle )
{
    QAction::trigger();
}
