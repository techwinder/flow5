/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#pragma once

#include <QComboBox>
#include <xflcore/linestyle.h>

class LineDelegate;

class LineCbBox : public QComboBox
{
    public:
        LineCbBox(QWidget *pParent=nullptr);
        void setLine(Line::enumLineStipple style, int width, QColor color, Line::enumPointStyle pointStyle);
        void setLine(LineStyle ls);

        bool points() const {return m_LineStyle.m_Symbol!=Line::NOSYMBOL;}

        LineDelegate *lineDelegate() {return m_pLineDelegate;}

    private:
        void paintEvent (QPaintEvent *pEvent)  override;

    private:
        LineStyle m_LineStyle;

        LineDelegate *m_pLineDelegate;
};



class LineDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

    public:
        LineDelegate (LineCbBox *pCbBox = nullptr);

        void setLineColor(QColor color) {m_LineColor = color;}
        void setLineStyle(int *style);
        void setLineWidth(int *width);
        void setPointStyle(QVector<Line::enumPointStyle> const &pointstyle) {m_PointStyle=pointstyle;}

    private:
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;


    private:
        LineCbBox * m_pCbBox; //pointer to the parent QLineComboBox
        QSize m_Size;

        QVector<int> m_LineStyle; // values depend on whether we have a line or width CbB
        QVector<int> m_LineWidth; // values depend on whether we have a line or width CbB
        QColor m_LineColor; // the same for all CbBox items
        QVector<Line::enumPointStyle> m_PointStyle;
};


