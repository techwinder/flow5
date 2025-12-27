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






