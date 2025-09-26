/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once


#include <QAbstractButton>
#include <xflcore/linestyle.h>


class LegendBtn : public QWidget
{
    Q_OBJECT

    public:
        LegendBtn(QWidget *parent = nullptr);

        void setTag(QString const &tag) {m_LineStyle.m_Tag = tag;}

        void setStyle(LineStyle ls);

        bool isCurrent() const {return m_bIsCurrent;}
        void setCurrent(bool bCurrent) {m_bIsCurrent=bCurrent;}
        bool hasBackGround() const {return m_bHasBackGround;}
        void setBackground(bool bBack) {m_bHasBackGround=bBack;}

        QColor lineColor() const {return m_LineStyle.m_Color;}
        void setLineColor(QColor clr) {m_LineStyle.m_Color=clr;}

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



