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

#include <QAbstractButton>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QFont>
#include <QColor>



class TextClrBtn : public QAbstractButton
{
    Q_OBJECT

    public:
        TextClrBtn(QWidget *parent = nullptr);

        void setFont(QFont const &font);
        void setTextColor(QColor const & TextColor);
        void setBackgroundColor(QColor const & TextColor);
        QColor textColor() const { return m_TextColor;}

        void setContour(bool bContour) {m_bContour=bContour;}

    signals:
        void clickedTB();

    public:
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void paintEvent(QPaintEvent *pEvent) override;
        QSize sizeHint() const override;

    private:
        QColor m_TextColor;
        QColor m_BackgroundColor;
        QFont m_TextFont;

        bool m_bContour;
};

