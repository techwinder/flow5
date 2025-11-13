/****************************************************************************

    flow5
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


