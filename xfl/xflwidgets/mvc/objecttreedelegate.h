/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QStyledItemDelegate>
#include <xflcore/fontstruct.h>

class ObjectTreeDelegate : public QStyledItemDelegate
{
    public:
        ObjectTreeDelegate(QObject *pParent);
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

        void showStyle(bool bShow) {m_bShowStyle=bShow;}

        bool m_bShowStyle;

};







