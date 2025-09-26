/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#pragma once

#include <xflcore/linestyle.h>

#include <xflwidgets/line/linebtn.h>
#include <QWidgetAction>

class LineAction : public QWidgetAction
{
    Q_OBJECT

    public:
        LineAction(QObject *pParent=nullptr);
        LineBtn &lineBtn() {return m_LineBtn;}


    private slots:
        void onClickedLB(LineStyle ls);

    private:
        LineBtn m_LineBtn;
};

