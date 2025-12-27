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


#include <interfaces/widgets/mvc/objecttreeitem.h>
#include <interfaces/widgets/mvc/objecttreemodel.h>



ObjectTreeModel::ObjectTreeModel(QObject *parent) : QAbstractItemModel(parent)
{
    LineStyle ls;
    ls.m_Tag = "the root item";
    m_pRootItem = new ObjectTreeItem("Root", ls, Qt::Unchecked, nullptr);
}


ObjectTreeModel::~ObjectTreeModel()
{
    delete m_pRootItem;
}


int ObjectTreeModel::columnCount(const QModelIndex &) const
{
    return 3;
}


bool ObjectTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;
    if (role != Qt::DisplayRole) return false;

    ObjectTreeItem *pItem = static_cast<ObjectTreeItem*>(index.internalPointer());
    if(!pItem) return false;

    pItem->setData(index.column(), value);

    emit dataChanged(index, index);
    return true;
}



QVariant ObjectTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole) return QVariant();

    ObjectTreeItem *pItem = static_cast<ObjectTreeItem*>(index.internalPointer());
    if(!pItem) return QVariant();

    return pItem->data(index.column());
}


ObjectTreeItem *ObjectTreeModel::item(int iRow)
{
    QModelIndex ind = index(iRow, 0);
    if (!ind.isValid()) return nullptr;

    ObjectTreeItem *pItem = static_cast<ObjectTreeItem*>(ind.internalPointer());
    return pItem;
}


ObjectTreeItem *ObjectTreeModel::itemFromIndex(QModelIndex const &ind)
{
    if (!ind.isValid()) return nullptr;

    ObjectTreeItem *pItem = static_cast<ObjectTreeItem*>(ind.internalPointer());
    return pItem;
}


Qt::ItemFlags ObjectTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}


QVariant ObjectTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_pRootItem->data(section);

    return QVariant();
}


QModelIndex ObjectTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ObjectTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = m_pRootItem;
    else
        parentItem = static_cast<ObjectTreeItem*>(parent.internalPointer());

    ObjectTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}


QModelIndex ObjectTreeModel::index(int row, int column, ObjectTreeItem const *pParentItem)
{
    if(!pParentItem) return QModelIndex();

    ObjectTreeItem *pChildItem = pParentItem->child(row);
    if (pChildItem)
        return createIndex(row, column, pChildItem);
    else
        return QModelIndex();
}


QModelIndex ObjectTreeModel::index(ObjectTreeItem const *pParentItem, ObjectTreeItem const *pItem)
{
    if(!pParentItem) return QModelIndex();
    if(!pItem) return QModelIndex();
    for(int ir=0; ir<pParentItem->rowCount(); ir++)
    {
        if(pParentItem->child(ir)==pItem)
        {
            return index(ir, 0, pParentItem);
        }
    }
    return QModelIndex();
}


QModelIndex ObjectTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ObjectTreeItem *pChildItem = static_cast<ObjectTreeItem*>(index.internalPointer());
    ObjectTreeItem *pParentItem = pChildItem->parentItem();

    if (pParentItem == m_pRootItem)
        return QModelIndex();

    return createIndex(pParentItem->row(), 0, pParentItem);
}


int ObjectTreeModel::rowCount(const QModelIndex &parent) const
{
    ObjectTreeItem *parentItem=nullptr;
    if (parent.column()>0) return 0;

    if (!parent.isValid()) parentItem = m_pRootItem;
    else                   parentItem = static_cast<ObjectTreeItem*>(parent.internalPointer());

    return parentItem->rowCount();
}


bool ObjectTreeModel::removeRows(int row, int count, const QModelIndex &parentindex)
{
    if(count<=0) return true;

    ObjectTreeItem *pItem = nullptr;
    if(!parentindex.isValid())
    {
        //remove all rows from the root item
        pItem = m_pRootItem;
    }
    else
        pItem = static_cast<ObjectTreeItem*>(parentindex.internalPointer());
    if(!pItem) return false; // something went wrong

    beginRemoveRows(parentindex, row, row+count-1);
    for(int i=count-1; i>=0; i--)
    {
        if(row+i<pItem->rowCount())
            pItem->removeRow(row+i);
    }
    endRemoveRows();

    return true;
}


ObjectTreeItem* ObjectTreeModel::insertRow(ObjectTreeItem*pParentItem, int row,
                                           QString const &name, LineStyle const &ls, Qt::CheckState state)
{
    if(!pParentItem) return nullptr;
    QModelIndex parentindex = index(pParentItem->parentItem(), pParentItem);
    beginInsertRows(parentindex, row, row);
    ObjectTreeItem *pNewItem = pParentItem->insertRow(row, name, ls, state);
    endInsertRows();
    return pNewItem;
}


ObjectTreeItem* ObjectTreeModel::insertRow(ObjectTreeItem*pParentItem, int row,
                                           std::string const &name, LineStyle const &ls, Qt::CheckState state)
{
    return insertRow(pParentItem, row, QString::fromStdString(name), ls, state);
}


ObjectTreeItem* ObjectTreeModel::appendRow(ObjectTreeItem*pParentItem, QString const &name, LineStyle const &ls, Qt::CheckState state)
{
    return insertRow(pParentItem, pParentItem->rowCount(), name, ls, state);
}


ObjectTreeItem* ObjectTreeModel::appendRow(ObjectTreeItem*pParentItem, std::string const &name, LineStyle const &ls, Qt::CheckState state)
{
    return insertRow(pParentItem, pParentItem->rowCount(), QString::fromStdString(name), ls, state);
}








