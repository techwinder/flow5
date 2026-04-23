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

#include <QWidget>
#include <QStringList>
#include <QPushButton>
#include <QMenu>

#include <interfaces/widgets/line/linebtn.h>


#define NCOLORROWS 24
#define NCOLORCOLS 3

class TextClrBtn;

class LinePicker : public QWidget
{
    Q_OBJECT

    public:
        LinePicker(QWidget *pParent=nullptr);

        void initDialog(LineStyle const &ls);

        void setColor(QColor const &clr);
        void setColor(fl5Color const &clr);

        static void setColorList(QStringList const&colorlist, QStringList const &colornames);

        static QColor randomColor(bool bLightColor);


    signals:
        void colorChanged(QColor);

    public slots:
        void onClickedLB(LineStyle ls);

        void onOtherColor();

    private:
        void setupLayout();

    private:
        LineBtn m_lb[NCOLORROWS*NCOLORCOLS];

        TextClrBtn *m_pColorButton;

        LineStyle m_theStyle;

        static QStringList s_LineColorList;
        static QStringList s_LineColorNames;

};


