/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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




