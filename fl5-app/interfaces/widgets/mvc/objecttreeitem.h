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

#include <QList>
#include <QVariant>
#include <QStandardItem>

#include <api/linestyle.h>

class ObjectTreeItem
{
    friend class ObjectTreeModel;

    public:
        explicit ObjectTreeItem(const QString &name, const LineStyle &ls, Qt::CheckState state, ObjectTreeItem *parentItem = 0);
        ~ObjectTreeItem();

        ObjectTreeItem *child(int row) const {return m_pChildItems.value(row);}
        int rowCount() const {return m_pChildItems.count();}
        int columnCount() const {return 3;}
        QVariant data(int column) const;
        int row() const;
        ObjectTreeItem *parentItem() {return m_pParentItem;}

        void setParent(ObjectTreeItem*pParent) {m_pParentItem=pParent;}
        void setName(QString const &name) {m_Name=name;}
        void setTheStyle(LineStyle const &ls) {m_LS=ls;}
        void setCheckState(Qt::CheckState state) {m_CheckState=state;}

        void setData(int column, QVariant const &var);

        QString const &name() const {return m_Name;}
        void setLevel(int level) {m_Level = level;}
        int level() const {return m_Level;}
        bool isObjectLevel() const {return m_Level==1;}
        bool isPolarLevel()  const {return m_Level==2;}
        bool isOppLevel()    const {return m_Level==3;}

    private:
        // these methods should only be accessed through the model so as to call begin/end removeRows()
        void removeChidren();
        void insertRow(int iRow, ObjectTreeItem *pItem);
        ObjectTreeItem *insertRow(int iRow, QString const &name, LineStyle const &ls, Qt::CheckState state);
        void removeRow(int iRow);
        ObjectTreeItem *appendRow(QString const &name, LineStyle const &ls, Qt::CheckState state);

    private:
        QList<ObjectTreeItem*> m_pChildItems;
        ObjectTreeItem *m_pParentItem;

        QString m_Name;
        LineStyle m_LS;
        Qt::CheckState m_CheckState;
        int m_Level; /// 0 is root, 1 is plane/foil, 2 is polar, 3 is Opp, 4 is mode
};


