/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include "xfltreeview.h"

XflTreeView::XflTreeView(QWidget *pParent) : QTreeView(pParent)
{

}

QModelIndex XflTreeView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    if(cursorAction == QAbstractItemView::MoveNext)
    {
        QModelIndex index = currentIndex();
        if (index.isValid())
        {
            if (index.column()+1 < model()->columnCount())
                return index.sibling(index.row(), index.column()+1);
            else if(index.row()+1 < model()->rowCount(index.parent()))
                return index.sibling(index.row()+1, 0);
            else
                return QModelIndex();
        }
    }
    else if(cursorAction == QAbstractItemView::MovePrevious)
    {
        QModelIndex index = currentIndex();
        if (index.isValid())
        {
            if(index.column()>= 1)
                return index.sibling(index.row(), index.column()-1);
            else if(index.row()>= 1)
                return index.sibling(index.row()-1, model()->columnCount()-1);
            else
                return QModelIndex();
        }
    }
    return QTreeView::moveCursor(cursorAction, modifiers);
}
