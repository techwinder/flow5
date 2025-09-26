/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#define _MATH_DEFINES_DEFINED



#include <QFont>
#include <QBrush>
#include <QPalette>

#include "planeinertiamodel.h"
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflcore/units.h>

PlaneInertiaModel::PlaneInertiaModel(PlaneXfl *pPlane, QObject *parent)
    :QAbstractTableModel(parent)
{
    m_pPlane = pPlane;
    m_iActionColumn = 5;
}


int PlaneInertiaModel::rowCount(const QModelIndex & /*parent */) const
{
    if(!m_pPlane) return 0;
    if(m_pPlane->hasFuse())    return m_pPlane->nWings()+1;
    else                    return m_pPlane->nWings()+0;
}


int PlaneInertiaModel::columnCount(const QModelIndex & /*parent */) const
{
    return 6;
}


Qt::ItemFlags PlaneInertiaModel::flags(const QModelIndex &index) const
{
//    if (index.column()==1) return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    return QAbstractTableModel::flags(index);
}


QVariant PlaneInertiaModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==Qt::DisplayRole && orientation==Qt::Horizontal)
    {
        QString strMass;
        Units::getMassUnitLabel(strMass);
        QString strLength;
        Units::getLengthUnitLabel(strLength);
        QString strInertia;
        Units::getInertiaUnitLabel(strInertia);
        switch (section)
        {
            case 0:
                return "Name";

            case 1:
                return "Mass ("+strMass+")";

            case 2:
                return "CoG_x ("+strLength+")";

            case 3:
                return "CoG_y ("+strLength+")";

            case 4:
                return "CoG_z ("+strLength+")";

            case 5:
                return "";

            default:
                return "";

        }
    }
    else if(role==Qt::DisplayRole && orientation==Qt::Vertical)
    {
        QString rowName;
        rowName = QString::asprintf("%d", section+1);
        return rowName;
    }
    else if(role==Qt::ToolTipRole && orientation==Qt::Horizontal)
    {
        QString strange;
        strange = QString::asprintf("a tooltip for header section %d", section);
        return strange;
    }
    return QVariant();
}


QVariant PlaneInertiaModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if(!m_pPlane) return QVariant();

    switch(role)
    {
        case Qt::DisplayRole:
        {
            QString strange;
            if(index.row()<m_pPlane->nWings())
            {
                WingXfl const *pWing = m_pPlane->wingAt(index.row());
                switch(col)
                {
                    case 0:
                    {
                        return pWing->name();
                    }
                    case 1:
                    {
                        strange = QString::asprintf("%7.3f", pWing->totalMass() * Units::kgtoUnit());
                        return strange;
                    }
                    case 2:
                    {
                        strange = QString::asprintf("%7.3f", (pWing->CoG_t().x + pWing->position().x) * Units::mtoUnit());
                        return strange;
                    }
                    case 3:
                    {
                        strange = QString::asprintf("%7.3f", (pWing->CoG_t().y + pWing->position().y) * Units::mtoUnit());
                        return strange;
                    }
                    case 4:
                    {
                        strange = QString::asprintf("%7.3f", (pWing->CoG_t().z + pWing->position().z) * Units::mtoUnit());
                        return strange;
                    }
                    case 5:
                    {
                        return QString("Edit...");
                    }
                    default:
                    {
                        strange = QString::asprintf("row=%d col=%d", index.row(), index.column());
                        return strange;
                    }
                }
            }
            else if(row==m_pPlane->nWings() && m_pPlane->hasFuse())
            {
                Fuse const*pFuse = m_pPlane->fuseAt(0);
                switch(col)
                {
                    case 0:
                    {
                        return pFuse->name();
                    }
                    case 1:
                    {
                        strange = QString::asprintf("%7.3f", pFuse->totalMass() * Units::kgtoUnit());
                        return strange;
                    }
                    case 2:
                    {
                        strange = QString::asprintf("%7.3f", (pFuse->CoG_t().x+pFuse->position().x) * Units::mtoUnit());
                        return strange;
                    }
                    case 3:
                    {
                        strange = QString::asprintf("%7.3f", (pFuse->CoG_t().y+pFuse->position().y) * Units::mtoUnit());
                        return strange;
                    }
                    case 4:
                    {
                        strange = QString::asprintf("%7.3f", (pFuse->CoG_t().z+pFuse->position().z) * Units::mtoUnit());
                        return strange;
                    }
                    case 5:
                    {
                        return QString("Edit...");
                    }
                    default:
                    {
                        strange = QString::asprintf("row=%d col=%d", index.row(), index.column());
                        return strange;
                    }
                }
            }
            break;
        }
        case Qt::FontRole:
        {
            if (col==0 || col==m_iActionColumn)
            {
                QFont boldFont;
                boldFont.setBold(true);
                return boldFont;
            }
            break;
        }
        case Qt::ForegroundRole:
        {
            QPalette pal;
            return QBrush(pal.buttonText());
        }
        case Qt::BackgroundRole:
        {
            if (col==m_iActionColumn)
            {
                QPalette pal;
                return QBrush(pal.button());
            }
            break;
        }
        case Qt::TextAlignmentRole:
        {
            if(col==0 || col==m_iActionColumn) return Qt::AlignCenter;
            else                               return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        }
        case Qt::CheckStateRole:
        {
            break;
        }
        case Qt::ToolTipRole:
        {
            QString strange;
            strange = QString::asprintf("a tooltip for cell (%d, %d)", index.row(), index.column());
            return strange;
        }
    }
    return QVariant();
}


bool PlaneInertiaModel::setData(const QModelIndex& index, const QVariant& , int role)
{
    if(role==Qt::EditRole)
    {
        return false;
    }
    else if (role==Qt::CheckStateRole)
    {
        if (index.column()==2315)
        {
        }
    }

    emit dataChanged(index,index);
    return false;

//    return QAbstractItemModel::setData(index,value,role);
}


/** custom method to update the qtableview if the underlying object has changed */
void PlaneInertiaModel::updateData()
{
//    QModelIndex idxTL = index(0,0);
//    QModelIndex idxBR = index(rowCount(), columnCount());
    beginResetModel();
    endResetModel();
//    emit(dataChanged(idxTL, idxBR));
}






