/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QTreeView>



class XflTreeView : public QTreeView
{
    public:
        XflTreeView(QWidget *pParent=nullptr);

    private:
         QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
};


