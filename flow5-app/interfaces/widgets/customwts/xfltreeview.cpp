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
