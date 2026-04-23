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
#include <api/linestyle.h>


class LegendBtn : public QWidget
{
    Q_OBJECT

    public:
        LegendBtn(QWidget *parent = nullptr);

        void setTag(QString const &tag) {m_LineStyle.m_Tag = tag.toStdString();}

        void setStyle(LineStyle ls);

        bool isCurrent() const {return m_bIsCurrent;}
        void setCurrent(bool bCurrent) {m_bIsCurrent=bCurrent;}
        bool hasBackGround() const {return m_bHasBackGround;}
        void setBackground(bool bBack) {m_bHasBackGround=bBack;}

        QColor lineColor() const;
        void setLineColor(QColor const &clr);

        int lineStyle()      const {return m_LineStyle.m_Stipple;}
        void setLineStyle(Line::enumLineStipple iStyle) {m_LineStyle.m_Stipple = iStyle;}

        int lineWidth()      const {return m_LineStyle.m_Width;}
        void setLineWidth(int iWidth) {m_LineStyle.m_Width=iWidth;}

        int pointStyle() const {return m_LineStyle.m_Symbol;}
        void setPointStyle(Line::enumPointStyle iPointStyle) {m_LineStyle.m_Symbol=iPointStyle;}

        void setHighLight(bool bHigh) {m_bIsHighlighted=bHigh;}

        void paintButton(QPainter &painter);

    signals:
        void clickedLB(LineStyle);
        void clickedRightLB(LineStyle);
        void clickedLine(LineStyle);

    public:
        void mousePressEvent(QMouseEvent *pEvent) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;
        void resizeEvent(QResizeEvent *) override;
        void paintEvent(QPaintEvent *pEvent) override;
        QSize sizeHint() const override;

        bool event(QEvent* pEvent) override;


    private:
        bool m_bIsCurrent;
        bool m_bMouseHover;
        bool m_bHasBackGround;
        bool m_bIsHighlighted;

        LineStyle m_LineStyle;

        QRect m_LineRect, m_TagRect;
};



