/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once


#include <QAbstractTableModel>

class WingXfl;

class WingSectionModel : public QAbstractTableModel
{
    public:
        WingSectionModel(WingXfl *pWing, QObject *parent);
        int rowCount(const QModelIndex &parent = QModelIndex()) const override ;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        //	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
        Qt::ItemFlags flags(const QModelIndex &index) const override;

        void updateData();
        void setEditSide(bool bRight) {m_bRightSide=bRight;}
        void setWing(WingXfl*pWing) {m_pWing=pWing;}


        void setActionColumn(int iColumn) {m_iActionColumn=iColumn;}
        int actionColumn() const {return m_iActionColumn;}

    private:
        WingXfl *m_pWing;
        bool m_bRightSide;
        int m_iActionColumn;
};






