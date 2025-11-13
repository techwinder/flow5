/****************************************************************************

    flow5
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


#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class ObjectTreeItem;
struct LineStyle;

class ObjectTreeModel : public QAbstractItemModel
{
    Q_OBJECT

    public:
        explicit ObjectTreeModel(QObject *parent = 0);
        ~ObjectTreeModel();

        QVariant data(const QModelIndex &index, int role) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex &index) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &ind= QModelIndex()) const override;

        ObjectTreeItem *rootItem() {return m_pRootItem;}
        ObjectTreeItem *item(int iRow);
        ObjectTreeItem *itemFromIndex(const QModelIndex &ind);
        QModelIndex index(int row, int column, const ObjectTreeItem *pParentItem);
        QModelIndex index(const ObjectTreeItem *pParentItem, const ObjectTreeItem *pItem);
        bool removeRows(int row, int count, const QModelIndex &parentindex = QModelIndex()) override;

        ObjectTreeItem* insertRow(ObjectTreeItem*pParentItem, int row, QString const &name, LineStyle const &ls, Qt::CheckState state);
        ObjectTreeItem* appendRow(ObjectTreeItem*pParentItem, QString const &name, LineStyle const &ls, Qt::CheckState state);

        void updateData();
        void updateData(QModelIndex idx);

    private:

        ObjectTreeItem *m_pRootItem;
};

