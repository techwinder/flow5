/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#include "foiltabledelegate.h"
#include <xflfoil/objects2d/objects2d.h>
#include <xflfoil/objects2d/foil.h>
#include <xflcore/xflcore.h>
#include <xflwidgets/wt_globals.h>




FoilTableDelegate::FoilTableDelegate(QObject *pParent) : QStyledItemDelegate(pParent)
{
}


void FoilTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVector<Foil*> foils = Objects2d::sortedFoils();

    QFontMetrics fm(option.font);

    if(index.row()>=foils.size()) return;

    Foil const *pFoil = foils.at(index.row());
    if(!pFoil) return;

    QColor backcolor, crosscolor;
    // using QItemDelegate::drawBackground() code
    if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
            cg = QPalette::Inactive;

        backcolor = option.palette.color(cg, QPalette::Highlight);
        crosscolor = option.palette.color(cg, QPalette::HighlightedText);
    }
    else
    {
        backcolor = option.palette.base().color();
        crosscolor = option.palette.windowText().color();
    }

    if(index.column()==9)
    {
//        QItemDelegate::drawBackground(painter, option, index); // paint the background, using palette colors, including stylesheet mods
        drawCheckBox(painter, pFoil->isVisible(), option.rect, fm.height(), false, false, crosscolor, backcolor);
    }
    else if(index.column()==10)
    {
//        QItemDelegate::drawBackground(painter, option, index); // paint the background, using palette colors, including stylesheet mods
        drawCheckBox(painter, pFoil->isCamberLineVisible(), option.rect, fm.height(), false, false, crosscolor, backcolor);
    }
    else if(index.column()==11)
    {
//        QItemDelegate::drawBackground(painter, option, index); // paint the background, using palette colors, including stylesheet mods
        QColor linecolor = pFoil->lineColor();
        Line::enumPointStyle pointstyle = pFoil->pointStyle();

        QRect r = option.rect;

        painter->save();

        QPen LinePen(linecolor);
        LinePen.setStyle(xfl::getStyle(pFoil->lineStipple()));
        LinePen.setWidth(pFoil->lineWidth());
        painter->setPen(LinePen);
        painter->drawLine(r.left()+5, r.top()+r.height()/2, r.right()-5, r.top()+r.height()/2);

        LinePen.setStyle(Qt::SolidLine);
        painter->setPen(LinePen);


        xfl::drawSymbol(*painter, pointstyle, backcolor, linecolor, r.center());

        painter->restore();

//        drawFocus(painter, myOption, myOption.rect);
    }
    else
    {
        QString strange;
        QStyleOptionViewItem myOption = option;
//        myOption.backgroundBrush.setColor(backcolor);

        if(index.column()==0) // the foil name
        {
            myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
            strange = index.model()->data(index, Qt::DisplayRole).toString();
        }
        else if(index.column()==5)
        {
            myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            strange = QString("%L1").arg(index.model()->data(index, Qt::DisplayRole).toInt());
        }
        else
        {
            double dble = index.model()->data(index, Qt::DisplayRole).toDouble();
            myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            if(xfl::isLocalized())
                strange = QString("%L1").arg(dble ,0,'f', 2);
            else
                strange = QString::asprintf("%.2f", dble);

        }

        painter->drawText(myOption.rect, myOption.displayAlignment, strange);
    }
}





