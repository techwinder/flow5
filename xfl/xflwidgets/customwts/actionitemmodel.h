/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QStandardItemModel>



class ActionItemModel : public QStandardItemModel
{
    Q_OBJECT

    public:
        ActionItemModel(QObject *pParent=nullptr);
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        void updateData();

        void setActionColumn(int iColumn) {m_iActionColumn=iColumn;}
        int actionColumn() const {return m_iActionColumn;}

        void setName(QString const &name) {m_ModelName=name;}

    private:
        int m_iActionColumn;

        QString m_ModelName; // debug info
};


