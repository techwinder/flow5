/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QAbstractTableModel>


class PlaneXfl;

class PlaneInertiaModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        PlaneInertiaModel(PlaneXfl *pPlane, QObject *parent);

        int rowCount(const QModelIndex &parent = QModelIndex()) const override ;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        //	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
        Qt::ItemFlags flags(const QModelIndex &index) const override;

        void updateData();

        void setActionColumn(int iCol) {m_iActionColumn=iCol;}
        int actionColumn() const {return m_iActionColumn;}

    private:
        PlaneXfl *m_pPlane;
        int m_iActionColumn;
};



