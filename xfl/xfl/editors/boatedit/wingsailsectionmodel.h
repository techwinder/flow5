/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QAbstractTableModel>

class Sail;
class WingSail;

class WingSailSectionModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    WingSailSectionModel(QObject *pParent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
//	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void updateData();
    void setWingSail(WingSail*pWS) {m_pWingSail=pWS;}


    void setActionColumn(int iColumn) {m_iActionColumn=iColumn;}
    int actionColumn() const {return m_iActionColumn;}


private:
    WingSail *m_pWingSail;
    int m_iActionColumn;
};
