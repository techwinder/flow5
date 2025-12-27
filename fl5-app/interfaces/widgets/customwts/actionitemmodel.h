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


