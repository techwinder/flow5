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


#include <QDebug>
#include <QStringList>

#include <interfaces/widgets/mvc/objecttreeitem.h>


ObjectTreeItem::ObjectTreeItem(QString const &name, LineStyle const &ls, Qt::CheckState state, ObjectTreeItem *parent)
{
    m_pParentItem = parent;
    if(parent) m_Level = parent->m_Level+1;
    else m_Level = 0;

    m_Name = name;
    m_LS = ls;
    m_CheckState = state;
}


ObjectTreeItem::~ObjectTreeItem()
{
    qDeleteAll(m_pChildItems);
}


void ObjectTreeItem::setData(int column, QVariant const &var)
{
    switch (column)
    {
        case 0:
        {
            m_Name = var.toString();
            break;
        }
        case 1:
        {
            if (var.canConvert<LineStyle>())
            {
                m_LS = var.value<LineStyle>();
            }
            break;
        }
        case 2:
        {
            int n = var.toInt();

            if     (n==1) m_CheckState = Qt::PartiallyChecked;
            else if(n==2) m_CheckState = Qt::Checked;
            else          m_CheckState = Qt::Unchecked;
            break;
        }
        default: break;
    }
}


QVariant ObjectTreeItem::data(int column) const
{
    switch (column)
    {
        case 0: return m_Name;
        case 1: return QVariant::fromValue(m_LS);
        case 2:
        {
            int n=0;
            if     (m_CheckState==Qt::Checked)          n=2;
            else if(m_CheckState==Qt::PartiallyChecked) n=1;
            return n;
        }
        default: break;
    }
    return QVariant();
}


int ObjectTreeItem::row() const
{
    if (m_pParentItem)
        return m_pParentItem->m_pChildItems.indexOf(const_cast<ObjectTreeItem*>(this));

    return 0;
}


ObjectTreeItem* ObjectTreeItem::appendRow(const QString &name, const LineStyle &ls, Qt::CheckState state)
{
    ObjectTreeItem *pItem = new ObjectTreeItem(name, ls, state, this);
    m_pChildItems.append(pItem);
    return pItem;
}


void ObjectTreeItem::removeChidren()
{
    qDeleteAll(m_pChildItems);
    m_pChildItems.clear();
}


ObjectTreeItem *ObjectTreeItem::insertRow(int iRow, QString const &name, LineStyle const &ls, Qt::CheckState state)
{
    ObjectTreeItem *pItem = new ObjectTreeItem(name, ls, state, this);
    m_pChildItems.insert(iRow, pItem);
    return pItem;
}


void ObjectTreeItem::insertRow(int iRow, ObjectTreeItem *pItem)
{
    pItem->setParent(this);
    pItem->m_Level = m_Level+1;
    m_pChildItems.insert(iRow, pItem);
}


void ObjectTreeItem::removeRow(int iRow)
{
    if(iRow<0 || iRow>=m_pChildItems.size()) return;
    ObjectTreeItem *pItem = m_pChildItems.takeAt(iRow);
    if(pItem) delete pItem;
}




