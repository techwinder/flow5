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


#include <QFont>
#include <QBrush>
#include <QPalette>

#include "actionitemmodel.h"



ActionItemModel::ActionItemModel(QObject *pParent) : QStandardItemModel(pParent)
{
    m_iActionColumn = -1;
}


QVariant ActionItemModel::data(const QModelIndex &index, int role) const
{
    int col = index.column();

    if(col==m_iActionColumn)
    {
        switch(role)
        {
            case Qt::DisplayRole:
            {
                return QString("...");
            }
            case Qt::FontRole:
            {
                QFont boldFont;
                boldFont.setBold(true);
                return boldFont;
            }
            case Qt::ForegroundRole:
            {
                QPalette pal;
                return QBrush(pal.buttonText());
            }
            case Qt::BackgroundRole:
            {
                QPalette pal;
                return QBrush(pal.button());
            }
            case Qt::TextAlignmentRole:
            {
                return Qt::AlignCenter;
            }
        }
    }
    return QStandardItemModel::data(index, role);
}


/** custom method to update the qtableview if the underlying object has changed */
void ActionItemModel::updateData()
{
    beginResetModel();
    endResetModel();
}




