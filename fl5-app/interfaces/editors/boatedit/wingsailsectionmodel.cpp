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

#ifdef Q_OS_WIN

#endif


#include "wingsailsectionmodel.h"
#include <api/sailwing.h>
#include <api/units.h>
#include <api/utils.h>
#include <core/xflcore.h>


WingSailSectionModel::WingSailSectionModel(QObject *pParent)
    :QAbstractTableModel(pParent)
{
    m_pWingSail = nullptr;
    m_iActionColumn = 9;
}


int WingSailSectionModel::rowCount(const QModelIndex & /*parent */) const
{
    if(!m_pWingSail) return 0;
    return m_pWingSail->sectionCount();
}


int WingSailSectionModel::columnCount(const QModelIndex & /*parent */) const
{
    return 11;
}


Qt::ItemFlags WingSailSectionModel::flags(const QModelIndex &index) const
{
//    if (index.column()==0) return QAbstractTableModel::flags(index) | Qt::ItemIsUserCheckable;
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}


QVariant WingSailSectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==Qt::DisplayRole && orientation==Qt::Horizontal)
    {
        QString str = Units::lengthUnitQLabel();
        switch (section)
        {
            case 0:
                return QString("x ("+str+")");
            case 1:
                return QString("z ("+str+")");
            case 2:
                return QString("Ry")+ DEGch;
            case 3:
                return QString("chord ("+str+")");
            case 4:
                return QString("twist")+ DEGch;
            case 5:
                return QString("foil");
            case 6:
                return QString("x-panels");
            case 7:
                return QString("x-dist");
            case 8:
                return QString("z-panels");
            case 9:
                return QString("z-dist");
            case 10:
                return QString("Actions");

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


QVariant WingSailSectionModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    if(!m_pWingSail) return QVariant(); // you never know
    if(row<0 || row>=m_pWingSail->sectionCount()) return QVariant();// you never know

    WingSailSection const &ws = m_pWingSail->section(row);

    switch(role)
    {
        case Qt::DisplayRole:
        {
            switch(col)
            {
                case 0:
                {
                    return m_pWingSail->xPosition(row) * Units::mtoUnit();
                }
                case 1:
                {
                    return m_pWingSail->zPosition(row) * Units::mtoUnit();
                }
                case 2:
                {
                    return m_pWingSail->sectionAngle(row);
                }
                case 3:
                {
                    return ws.chord() * Units::mtoUnit();
                }
                case 4:
                {
                    return ws.twist();
                }
                case 5:
                {
                    return QString::fromStdString(ws.foilName());
                }
                case 6:
                {
                    return ws.nxPanels();
                }
                case 7:
                {
                    return QString::fromStdString(xfl::distributionType(ws.xDistType()));
                }
                case 8:
                {
                    return ws.nzPanels();
                }
                case 9:
                {
                    return QString::fromStdString(xfl::distributionType(ws.zDistType()));
                }
                case 10:
                {
                    return "...";
                }
                default:
                {
                    return QString::asprintf("row=%d col=%d", index.row(), index.column());
                }

            }
        }
        case Qt::FontRole:
        {
            if (col==7003)
            {
                QFont boldFont;
                boldFont.setBold(true);
                return boldFont;
            }
            break;
        }

        case Qt::ForegroundRole:
        {
            if (col==m_iActionColumn)
            {
                QPalette pal;
                QBrush ForegrouncClr(pal.buttonText());
                return ForegrouncClr;
            }
            break;
        }
        case Qt::BackgroundRole:
        {
            if (col==m_iActionColumn)
            {
                QPalette pal;
                QBrush BackgroundClr(pal.button());
                return BackgroundClr;
            }
            break;
        }
        case Qt::TextAlignmentRole:
        {
            if(col==5 || col==7 || col==9) return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
            else if(col==10)               return Qt::AlignCenter;
            else                           return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        }
        case Qt::CheckStateRole:
        {
/*            if (col==0)
            {
                if(row<m_pPlane->nWings())
                {
                    if(m_pPlane->wing(row)) return Qt::Checked; else return Qt::Unchecked;
                }
                else if(row==m_pPlane->nWings())
                {
                    if(m_pPlane->hasBody()) return Qt::Checked; else return Qt::Unchecked;
                }
            }*/
            break;
        }
        case Qt::ToolTipRole:
        {
            QString strange;
            switch(col)
            {
                case 0:
                {
                    strange = QString("<p>The x-position of the wing section</p>");
                    break;
                }
                case 1:
                {
                    strange = QString("<p>The z-position of the wing section</p>");
                    break;
                }
                case 2:
                {
                    strange = QString("<p>angle of rotation around the y axis at the sections's leading edge</p>");
                    break;
                }
                case 3:
                {
                    strange = QString("<p>The chord at this wing section</p>");
                    break;
                }
                case 4:
                {
                    strange = QString("<p>The twist of this wing section, applied at the chord's quarter point</p>");
                    break;
                }
                case 5:
                {
                    strange = QString("<p>The foil at this wing section. <br>"
                                      "Will be either the right or left foil depending on which half wing is selected.</p>");
                    break;
                }
                case 6:
                {
                    strange = QString("<p>The number of quad panels in the chordwise direction for the panel outside this wing section.</p>");
                    break;
                }
                case 7:
                {
                    strange = QString("<p>The type of chordwise panel distribution for the panel outside this wing section.</p>");
                    break;
                }
                case 8:
                {
                    strange = QString("<p>The number of quad panels in the spanwise direction for the panel outside this wing section.</p>");
                    break;
                }
                case 9:
                {
                    strange = QString("<p>The type of spanwise panel distribution for the panel outside this wing section.</p>");
                    break;
                }
                default:
                {
                    strange = QString::asprintf("a tooltip for cell (%d, %d)", index.row(), index.column());
                    break;
                }
            }
            return strange;
        }
    }
    return QVariant();
}


bool WingSailSectionModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!m_pWingSail) return true;
    bool bChanged = false;

    int iSection  = index.row();
    WingSailSection &pws = m_pWingSail->section(iSection);

    if(role==Qt::EditRole)
    {
        bChanged = true;
        switch(index.column())
        {
            case 0:
            {
                m_pWingSail->setXPosition(iSection, value.toDouble() / Units::mtoUnit());
                break;
            }
            case 1:
            {
                m_pWingSail->setZPosition(iSection, value.toDouble() / Units::mtoUnit());
                break;
            }
            case 2:
            {
                m_pWingSail->setSectionAngle(iSection, value.toDouble());
                break;
            }
            case 3:
            {
                pws.setChord(value.toDouble() / Units::mtoUnit());
                break;
            }
            case 4:
            {
                pws.setTwist(value.toDouble());
                break;
            }
            case 5:
            {
                pws.setFoilName(value.toString().toStdString());
                break;
            }
            case 6:
            {
                pws.setNXPanels(value.toInt());
                break;
            }
            case 7:
            {
                pws.setXPanelDistType(xfl::distributionType(value.toString().toStdString()));
                break;
            }
            case 8:
            {
                pws.setNZPanels(value.toInt());
                break;
            }
            case 9:
            {
                pws.setZPanelDistType(xfl::distributionType(value.toString().toStdString()));
                break;
            }
            default:
                return false;
        }
    }
    else if (role==Qt::CheckStateRole)
    {
        if (index.column()==2315)
        {
            return false;
        }
    }

    if(bChanged) emit dataChanged(index,index);

    return true;
}


/** custom method to update the qtableview if the underlying sail object has changed */
void WingSailSectionModel::updateData()
{
    beginResetModel();
    endResetModel();
}





