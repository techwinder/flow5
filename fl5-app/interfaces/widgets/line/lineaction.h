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



#pragma once

#include <api/linestyle.h>

#include <interfaces/widgets/line/linebtn.h>
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

