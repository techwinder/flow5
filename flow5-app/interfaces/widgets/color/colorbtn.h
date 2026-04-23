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
#include <QPaintEvent>


class ColorBtn : public QAbstractButton
{
    Q_OBJECT

    public:
        ColorBtn(QWidget *pParent = nullptr);
        ColorBtn(QColor const &clr, QWidget *pParent = nullptr);

        void paintEvent (QPaintEvent * pEvent ) override;
        QSize minimumSizeHint() const  override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        bool event(QEvent* pEvent) override;

        QColor const &color() const {return m_Color;}
        void setColor(QColor const & color);

        bool isCurrent() const {return m_bIsCurrent;}
        void setCurrent(bool bCurrent) {m_bIsCurrent=bCurrent;}

        bool hasBackGround() const {return m_bHasBackGround;}
        void setBackground(bool bBack) {m_bHasBackGround=bBack;}


    signals:
        void clickedCB(QColor);

    public slots:
        void onSetColor(QColor clr);

    private:
        bool m_bIsCurrent;
        bool m_bMouseHover;
        bool m_bHasBackGround;
        QColor m_Color;
};




