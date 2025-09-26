/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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

