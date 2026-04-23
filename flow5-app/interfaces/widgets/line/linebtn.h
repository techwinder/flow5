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


class LineBtn : public QAbstractButton
{
    Q_OBJECT

    public:
        LineBtn(QWidget *parent = nullptr);
        LineBtn(LineStyle ls, QWidget *parent = nullptr);

        void setTheStyle(const LineStyle &ls);
        void setTheStyle(Line::enumLineStipple style, int width, const fl5Color &color, Line::enumPointStyle pointstyle);
        void setTheStyle(Line::enumLineStipple style, int width, const QColor &color, Line::enumPointStyle pointstyle);

        void setStipple(Line::enumLineStipple stipple);
        void setWidth(int width);
        void setPointStyle(Line::enumPointStyle pointstyle);

        bool isCurrent() const {return m_bIsCurrent;}
        void setCurrent(bool bCurrent) {m_bIsCurrent=bCurrent;}

        bool hasBackGround() const {return m_bHasBackGround;}
        void setBackground(bool bBack) {m_bHasBackGround=bBack;}

        LineStyle const &theLineStyle() const {return m_LineStyle;}

        QColor btnColor() const;
        void setBtnColor(QColor const &clr);
        void setBtnColor(fl5Color const &clr);

        Line::enumLineStipple btnStyle()      const {return m_LineStyle.m_Stipple;}
        void setBtnStyle(Line::enumLineStipple iStyle) {m_LineStyle.m_Stipple = iStyle;}

        int btnWidth()      const {return m_LineStyle.m_Width;}
        void setBtnWidth(int iWidth) {m_LineStyle.m_Width=iWidth;}

        Line::enumPointStyle btnPointStyle() const {return m_LineStyle.m_Symbol;}
        void setBtnPointStyle(Line::enumPointStyle iSymbol) {m_LineStyle.m_Symbol=iSymbol;}

    signals:
        void clickedLB(LineStyle);

    private:
        void mousePressEvent(QMouseEvent *pEvent) override;
        void paintEvent(QPaintEvent *pEvent) override;
        QSize sizeHint() const override;

        bool event(QEvent* pEvent) override;


    private:
        bool m_bIsCurrent;
        bool m_bMouseHover;
        bool m_bHasBackGround;
        LineStyle m_LineStyle;

};


