/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QDebug>
#include <QStringList>

#include <xflwidgets/mvc/objecttreeitem.h>


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


QVariant ObjectTreeItem::data(int column) const
{
    if(column<0 || column>2) return QVariant();
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




