/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QAbstractTableModel>


class Part;
class Inertia;

class PartInertiaModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        PartInertiaModel(QObject *parent);

        int rowCount(const QModelIndex &parent = QModelIndex()) const override ;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        //	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
        Qt::ItemFlags flags(const QModelIndex &index) const override;

        void updateData();
        void setEditable(bool bEditable) {m_bEditable=bEditable;}
        void setInertia(Inertia *pInertia) {m_pInertia=pInertia;}

    private:
        Inertia *m_pInertia;
        bool m_bEditable;
};



