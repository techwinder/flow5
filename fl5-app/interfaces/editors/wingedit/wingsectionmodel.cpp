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

#include "wingsectionmodel.h"

#include <api/wingxfl.h>
#include <api/units.h>
#include <api/utils.h>

#include <core/xflcore.h>

WingSectionModel::WingSectionModel(WingXfl *pWing, QObject *parent)
    : QAbstractTableModel(parent)
{
    m_pWing = pWing;
    m_bRightSide = true;
    m_iActionColumn = 10;
}


int WingSectionModel::rowCount(const QModelIndex & /*parent */) const
{
    if(!m_pWing) return 0;
    return m_pWing->nSections();
}


int WingSectionModel::columnCount(const QModelIndex & /*parent */) const
{
    return 11;
}


Qt::ItemFlags WingSectionModel::flags(const QModelIndex &index) const
{
//    if (index.column()==0) return QAbstractTableModel::flags(index) | Qt::ItemIsUserCheckable;
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}


QVariant WingSectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==Qt::DisplayRole && orientation==Qt::Horizontal)
    {
        QString str = Units::lengthUnitQLabel();
        switch (section)
        {
            case 0:
                return QString("y ("+str+")");

            case 1:
                return QString("chord ("+str+")");

            case 2:
                return QString("offset ("+str+")");

            case 3:
                return QString("dihedral")+ DEGch;

            case 4:
                return QString("twist")+ DEGch;

            case 5:
                return QString("foil");

            case 6:
                return QString("x-panels");

            case 7:
                return QString("x-dist");

            case 8:
                return QString("y-panels");

            case 9:
                return QString("y-dist");

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


QVariant WingSectionModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    if(!m_pWing) return QVariant(); // you never know
    if(row<0 || row>=m_pWing->nSections()) return QVariant();// you never know

    WingSection ws = m_pWing->section(row);

    switch(role)
    {
        case Qt::DisplayRole:
        {
            switch(col)
            {
                case 0:
                {
                    return ws.yPosition() * Units::mtoUnit();
                }
                case 1:
                {
                    return ws.chord() * Units::mtoUnit();
                }
                case 2:
                {
                    return ws.offset() * Units::mtoUnit();
                }
                case 3:
                {
                    return ws.dihedral();
                }
                case 4:
                {
                    return ws.twist();
                }
                case 5:
                {
                    if(m_bRightSide) return QString::fromStdString(ws.rightFoilName());
                    else             return QString::fromStdString(ws.leftFoilName());
                }
                case 6:
                {
                    return ws.nXPanels();
                }
                case 7:
                {
                    return QString::fromStdString(xfl::distributionType(ws.xDistType()));
                }
                case 8:
                {
                    return ws.nYPanels();
                }
                case 9:
                {
                    return QString::fromStdString(xfl::distributionType(ws.yDistType()));
                }
                case 10:
                {
                    return "...";
                }
                default:
                {
                    QString strange;
                    strange = QString::asprintf("row=%d col=%d", index.row(), index.column());
                    return strange;
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
            if (col==10)
            {
                QPalette pal;
                QBrush ForegrouncClr(pal.buttonText());
                return ForegrouncClr;
            }
            break;
        }
        case Qt::BackgroundRole:
        {
            if (col==10)
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
            break;
        }
        case Qt::ToolTipRole:
        {
            QString strange;
            switch(col)
            {
                case 0:
                {
                    strange = QString("<p>The span position of the wing section</p>");
                    break;
                }
                case 1:
                {
                    strange = QString("<p>The chord at this wing section</p>");
                    break;
                }
                case 2:
                {
                    strange = QString("<p>The x-offset of the leading edge of this wing sction</p>");
                    break;
                }
                case 3:
                {
                    strange = QString("<p>The dihedral of the outside panel of this wing section</p>");
                    break;
                }
                case 4:
                {
                    strange = QString("<p>The twist of this wing section, applied at the chord's &frac14; point</p>");
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
                    strange = QString("<p>The number of quad panels in the chordwise direction "
                                      "for the panel outside this wing section.</p>");
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
                    strange = QString::asprintf("<p>a tooltip for cell (%d, %d)</p>", index.row(), index.column());
                    break;
                }
            }
            return strange;
        }
    }
    return QVariant();
}


bool WingSectionModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!m_pWing) return true;
    bool bChanged = false;
    WingSection *pws = m_pWing->pSection(index.row());
    if(role==Qt::EditRole)
    {
        bChanged = true;
        switch(index.column())
        {
            case 0:
            {
                pws->m_YPosition = value.toDouble() / Units::mtoUnit();
                break;
            }
            case 1:
            {
                pws->m_Chord = value.toDouble() / Units::mtoUnit();
                break;
            }
            case 2:
            {
                pws->m_Offset = value.toDouble() / Units::mtoUnit();
                break;
            }
            case 3:
            {
                pws->m_Dihedral = value.toDouble();
                break;
            }
            case 4:
            {
                pws->m_Twist = value.toDouble();
                break;
            }
            case 5:
            {
                if(!m_pWing->isSymmetric())
                {
                    if(m_bRightSide) pws->setRightFoilName(value.toString().toStdString());
                    else             pws->setLeftFoilName(value.toString().toStdString());
                }
                else
                {
                    pws->setRightFoilName(value.toString().toStdString());
                    pws->setLeftFoilName(value.toString().toStdString());
                }
                break;
            }
            case 6:
            {
                int nx = value.toInt();
                for(int iws=0; iws<m_pWing->nSections(); iws++)
                {
                    m_pWing->pSection(iws)->setNX(nx);
                }
//                updateData();
                break;
            }
            case 7:
            {
                pws->m_XPanelDist = xfl::distributionType(value.toString().toStdString());
                break;
            }
            case 8:
            {
                pws->m_NYPanels = value.toInt();
                break;
            }
            case 9:
            {
                pws->m_YPanelDist = xfl::distributionType(value.toString().toStdString());
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


/** custom method to update the qtableview if the underlying plane object has changed */
void WingSectionModel::updateData()
{
    beginResetModel();
    endResetModel();
}





