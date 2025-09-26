/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QPainter>

#include "objecttreedelegate.h"

#include <xflcore/linestyle.h>
#include <xflcore/xflcore.h>
#include <xflcore/displayoptions.h>
#include <xflwidgets/wt_globals.h>



ObjectTreeDelegate::ObjectTreeDelegate(QObject *pParent)  : QStyledItemDelegate(pParent)
{
    m_bShowStyle = true;
}


void ObjectTreeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int col = index.column();

    QColor linecolor;

    /*           QColor backcolor;
    if (option.state & QStyle::State_Selected) backcolor = option.palette.highlight().color();
    else                                       backcolor = option.palette.base().color();*/

    QColor backcolor;
    if (option.showDecorationSelected && ((option.state & QStyle::State_Selected) || (option.state & QStyle::State_MouseOver)))
//    if (option.showDecorationSelected && (option.state & QStyle::State_MouseOver))
    {
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
            cg = QPalette::Inactive;

        backcolor = option.palette.color(cg, QPalette::Highlight);
    }
    else
    {
        backcolor = option.palette.base().color();
    }
    painter->fillRect(option.rect, backcolor);

    if(col==0)
    {
        QStyleOptionViewItem myOption = option;
        myOption.font = DisplayOptions::treeFontStruct().font();
        QStyledItemDelegate::paint(painter, myOption, index);
    }
    else if(col==1 && m_bShowStyle)
    {
        if (index.data(Qt::DisplayRole).canConvert<LineStyle>())
        {
            LineStyle ls = qvariant_cast<LineStyle>(index.data());
            QRect r = option.rect;

            QStyledItemDelegate::paint(painter, option, index); // paint the background, using palette colors, including stylesheet mods

            painter->setRenderHint(QPainter::Antialiasing);
            painter->setRenderHint(QPainter::SmoothPixmapTransform);


            if(ls.m_bIsEnabled)
            {
                painter->save();
                QPen LinePen;

                linecolor = ls.m_Color;
                LinePen.setStyle(xfl::getStyle(ls.m_Stipple));
                LinePen.setWidth(ls.m_Width);

                LinePen.setColor(linecolor);
                painter->setPen(LinePen);
                painter->drawLine(r.left()+5, r.center().y(), r.right()-5, r.center().y());

                LinePen.setStyle(Qt::SolidLine);
                painter->setPen(LinePen);

                if(ls.m_Symbol>0 && ls.m_bIsEnabled) xfl::drawSymbol(*painter, ls.m_Symbol, backcolor, linecolor, r.center());
                painter->restore();
            }
        }
        else
        {
            QStyledItemDelegate::paint(painter, option, index);
        }
    }
    else if(col==2 && m_bShowStyle)
    {
        QFontMetrics fm(option.font);
        Qt::CheckState checkstate=Qt::PartiallyChecked;

        if     (index.data()==2) checkstate=Qt::Checked;
        else if(index.data()==1) checkstate=Qt::PartiallyChecked;
        else if(index.data()==0) checkstate=Qt::Unchecked;


        QColor crosscolor;
        if (option.showDecorationSelected && (option.state & QStyle::State_Selected))
        {
            QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                      ? QPalette::Normal : QPalette::Disabled;
            if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
                cg = QPalette::Inactive;

            crosscolor = option.palette.color(cg, QPalette::HighlightedText);
        }
        else
        {
            crosscolor = option.palette.windowText().color();
        }

        if(option.state & QStyle::State_Selected)
            drawCheckBox(painter, checkstate, option.rect, fm.height(), true, false, crosscolor, backcolor, Qt::black);
        else
            drawCheckBox(painter, checkstate, option.rect, fm.height(), false, false, crosscolor, backcolor, Qt::black);
    }
    else
    {
        // hopefully never reached;
        Q_ASSERT(false);
        QStyledItemDelegate::paint(painter, option, index);
    }
}
