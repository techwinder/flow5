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

#define _MATH_DEFINES_DEFINED


#include <QFont>
#include <QBrush>
#include <QPalette>


#include "planepartmodel.h"

#include <api/objects_global.h>
#include <api/planexfl.h>
#include <api/utils.h>
#include <api/xml_globals.h>

#include <api/units.h>
#include <core/xflcore.h>
#include <modules/xobjects.h>


PlanePartModel::PlanePartModel(QObject *parent)    : QAbstractTableModel(parent)
{
    m_pPlane = nullptr;
}


int PlanePartModel::rowCount(const QModelIndex & /*parent */) const
{
    if(!m_pPlane) return 0;
    return m_pPlane->nWings()+m_pPlane->fuseCount();
}


int PlanePartModel::columnCount(const QModelIndex & /*parent */) const
{
    return 8;
}


Qt::ItemFlags PlanePartModel::flags(const QModelIndex &index) const
{
//    if (index.column()==0) return QAbstractTableModel::flags(index) | Qt::ItemIsUserCheckable;
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}


QVariant PlanePartModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==Qt::DisplayRole && orientation==Qt::Horizontal)
    {
        QString str = Units::lengthUnitQLabel();
        switch (section)
        {
            case 0:
                return "Type";

            case 1:
                return "Name";

            case 2:
                return "x ("+str+")";

            case 3:
                return "y ("+str+")";

            case 4:
                return "z ("+str+")";

            case 5:
                return "Rx("+DEGch + ")";

            case 6:
                return "Ry("+DEGch + ")";

            case 7:
                return "Action";

            default:
                return "";
        }
    }
    else if(role==Qt::DisplayRole && orientation==Qt::Vertical)
    {
        QString rowName;
        rowName = QString::asprintf("%d", section);
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


QVariant PlanePartModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

        if(!m_pPlane) return QVariant();

    switch(role)
    {
        case Qt::DisplayRole:
        {
            if(index.row()<m_pPlane->nWings())
            {
                switch(col)
                {
                    case 0:
                    {
                        return xml::wingType(m_pPlane->wing(row)->wingType());
                    }
                    case 1:
                    {
                        return QString::fromStdString(m_pPlane->wing(row)->name());
                    }
                    case 2:
                    {
                        return m_pPlane->wingLE(row).x * Units::mtoUnit();
                    }
                    case 3:
                    {
                        return m_pPlane->wingLE(row).y * Units::mtoUnit();
                    }
                    case 4:
                    {
                        return m_pPlane->wingLE(row).z * Units::mtoUnit();
                    }
                    case 5:
                    {
                        return m_pPlane->rxAngle(row);
                    }
                    case 6:
                    {
                        return m_pPlane->ryAngle(row);
                    }
                    case 7:
                    {
                        return QString("...");
                    }
                    default:
                    {
                        QString strange;
                        strange = QString::asprintf("row=%d col=%d", index.row(), index.column());
                        return strange;
                    }
                }
            }
            else if(row>=m_pPlane->nWings() && m_pPlane->hasFuse())
            {
                int ifuse = row-m_pPlane->nWings();
                switch(col)
                {
                    case 0:
                    {
                        return "FUSE";
                    }
                    case 1:
                    {
                        return QString::fromStdString(m_pPlane->fuse(ifuse)->name());
                    }
                    case 2:
                    {
                        return m_pPlane->fusePos(ifuse).x * Units::mtoUnit();
                    }
                    case 3:
                    {
                        return m_pPlane->fusePos(ifuse).y * Units::mtoUnit();
                    }
                    case 4:
                    {
                        return m_pPlane->fusePos(ifuse).z * Units::mtoUnit();
                    }
                    case 5:
                    {
                        return 0;
                    }
                    case 6:
                    {
                        return 0;
                    }
                    case 7:
                    {
                        return QString("...");
                    }
                    default:
                    {
                        QString strange;
                        strange = QString::asprintf("row=%d col=%d", index.row(), index.column());
                        return strange;
                    }
                }
            }
            break;
        }
        case Qt::FontRole:
        {
            if (col==0 || col==7)
            {
                QFont boldFont;
                boldFont.setBold(true);
                return boldFont;
            }
            break;
        }
        case Qt::ForegroundRole:
        {
            if (col==7)
            {
                QPalette pal;
                QBrush ForegrouncClr(pal.buttonText());
                return ForegrouncClr;
            }
            break;
        }
        case Qt::BackgroundRole:
        {
            if (col==7)
            {
                QPalette pal;
                QBrush BackgroundClr(pal.button());
                return BackgroundClr;
            }
            break;
        }
        case Qt::TextAlignmentRole:
        {
            if(col==0)
                return Qt::AlignCenter;
            else if(col==1)
                return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
            else if(col==7)
                return QVariant(Qt::AlignVCenter | Qt::AlignHCenter);
            else
                return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        }
        case Qt::CheckStateRole:
        {
            break;
        }
        case Qt::ToolTipRole:
        {
            QString strange;
            switch(col)
            {
                case 0:
                {
                    strange = QString("<p>The MAINWING (Ctrl+W) is required to set the reference dimensions.<br>"
                                          "If defined, the first ELEVATOR (Ctrl+E) in the list is used to calculate the tail volume.<br>"
                                          "FINs (Ctrl+F) will be defined as one-sided only by default.<br>"
                                          "Other values are for information only.<br>"
                                          "The shortcut for the fuselage editor is Ctrl+B(ody).</p>");
                    break;
                }
                case 1:
                {
                    strange = QString("<p>A suitable name to identify the part.<br>"
                                      "Used as information only.</p>");
                    break;
                }
                case 2:
                {
                    strange = QString("<p>The x-coordinate of the part in the plane's reference frame.</p>");
                    break;
                }
                case 3:
                {
                    strange = QString("<p>The y-coordinate of the part in the plane's reference frame.</p>");
                    break;
                }
                case 4:
                {
                    strange = QString("<p>The z-coordinate of the part in the plane's reference frame.</p>");
                    break;
                }
                case 5:
                {
                    strange = QString("<p>The angle of rotation of the part around the x-axis.</p>");
                    break;
                }
                case 6:
                {
                    strange = QString("<p>The angle of rotation of the part around the y-axis.</p>");
                    break;
                }
                case 7:
                {
                    strange = QString("<p>Click to open up the corresponding part editor.</p>");
                    break;
                }
                default:
                {
                    break;
                }
            }
            return strange;
        }
    }
    return QVariant();
}


bool PlanePartModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
//    emit dataChanged(index,index);
    if(!m_pPlane) return true;
    bool bChanged = false;
    bool bWingNamesChanged = false;
    if(role==Qt::EditRole)
    {
        if(index.row()<m_pPlane->nWings())
        {
            switch(index.column())
            {
                case 0:
                {
                    if(m_pPlane->wing(index.row())->wingType() != xml::wingType(value.toString()))
                    {
                        m_pPlane->wing(index.row())->setWingType(xml::wingType(value.toString()));
                        bChanged = true;
                    }
                    break;
                }
                case 1:
                {
                    if(m_pPlane->wing(index.row())->name() != value.toString().toStdString())
                    {
                        m_pPlane->wing(index.row())->setName(value.toString().trimmed().toStdString());
                        bWingNamesChanged = true;
                    }
                    break;
                }
                case 2:
                {
                    if(fabs(m_pPlane->wingLE(index.row()).x-value.toDouble()/Units::mtoUnit())>PRECISION)
                    {
                        Vector3d LE = m_pPlane->wingLE(index.row());
                        LE.x = value.toDouble() / Units::mtoUnit();
                        m_pPlane->setWingLE(index.row(), LE);
                        bChanged = true;
                    }
                    break;
                }
                case 3:
                {
                if(fabs(m_pPlane->wingLE(index.row()).y-value.toDouble()/Units::mtoUnit())>PRECISION)
                    {
                        Vector3d LE = m_pPlane->wingLE(index.row());
                        LE.y = value.toDouble() / Units::mtoUnit();
                        m_pPlane->setWingLE(index.row(), LE);
                        bChanged = true;
                    }
                    break;
                }
                case 4:
                {
                if(fabs(m_pPlane->wingLE(index.row()).z-value.toDouble()/Units::mtoUnit())>PRECISION)
                    {
                        Vector3d LE = m_pPlane->wingLE(index.row());
                        LE.z = value.toDouble() / Units::mtoUnit();
                        m_pPlane->setWingLE(index.row(), LE);
                        bChanged = true;
                    }
                    break;
                }
                case 5:
                {
                    if(fabs(m_pPlane->rxAngle(index.row())-value.toDouble())>PRECISION)
                    {
                        m_pPlane->setRxAngle(index.row(), value.toDouble());
                        bChanged = true;
                    }
                    break;
                }
                case 6:
                {
                    if(fabs(m_pPlane->ryAngle(index.row())-value.toDouble())>PRECISION)
                    {
                        m_pPlane->setRyAngle(index.row(), value.toDouble());
                        bChanged = true;
                    }
                    break;
                }
                default:
                    return false;
            }
        }
        else if(index.row()>=m_pPlane->nWings() && m_pPlane->hasFuse())
        {
            int ifuse = index.row()-m_pPlane->nWings();
            switch(index.column())
            {
                case 0:
                {
                    break;
                }
                case 1:
                {
                    if(m_pPlane->fuse(ifuse)->name() != value.toString().toStdString())
                    {
                        m_pPlane->fuse(ifuse)->setName(value.toString().trimmed().toStdString());
                        bChanged = true;
                    }
                    break;
                }
                case 2:
                {
                    if(fabs(m_pPlane->fusePos(ifuse).x-value.toDouble() / Units::mtoUnit())>PRECISION)
                    {
                        Vector3d pos = m_pPlane->fusePos(ifuse);
                        pos.x = value.toDouble() / Units::mtoUnit();
                        m_pPlane->setFusePos(ifuse, pos);
                        bChanged = true;
                    }
                    break;
                }
                case 3:
                {
                    if(fabs(m_pPlane->fusePos(ifuse).y-value.toDouble() / Units::mtoUnit())>PRECISION)
                    {
                        Vector3d pos = m_pPlane->fusePos(ifuse);
                        pos.y = value.toDouble() / Units::mtoUnit();
                        m_pPlane->setFusePos(ifuse, pos);
                        bChanged = true;
                    }
                    break;
                }
                case 4:
                {
                    if(fabs(m_pPlane->fusePos(ifuse).z-value.toDouble() / Units::mtoUnit())>PRECISION)
                    {
                        Vector3d pos = m_pPlane->fusePos(ifuse);
                        pos.z = value.toDouble() / Units::mtoUnit();
                        m_pPlane->setFusePos(ifuse, pos);
                        bChanged = true;
                    }
                    break;
                }
                default:
                    return false;
            }
        }
    }
    else if (role==Qt::CheckStateRole)
    {
        if (index.column()==2315)
        {
            return false;
        }
    }

    if(bWingNamesChanged) emit wingNamesChanged();
    if(bChanged)          emit dataChanged(index,index);

    return true;
//    return QAbstractItemModel::setData(index,value,role);
}


/** custom method to update the qtableview if the underlying plane object has changed */
void PlanePartModel::updateData()
{
    beginResetModel();
    endResetModel();
}






