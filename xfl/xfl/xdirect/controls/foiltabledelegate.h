/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once


#include <QStyledItemDelegate>


class FoilTableDelegate : public QStyledItemDelegate
{
    public:
        FoilTableDelegate(QObject *pParent);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};









