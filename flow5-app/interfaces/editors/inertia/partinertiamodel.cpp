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




#include <QFont>
#include <QBrush>

#include "partinertiamodel.h"
#include <api/inertia.h>

#include <api/units.h>

PartInertiaModel::PartInertiaModel(QObject *parent)
    :QAbstractTableModel(parent)
{
    m_bEditable =  false;
    m_pInertia = nullptr;
}


int PartInertiaModel::rowCount(const QModelIndex & /*parent */) const
{
    if(!m_pInertia) return 0;
    //mass, 3 Cog components, 6 tensor components
    return 10;
}


int PartInertiaModel::columnCount(const QModelIndex & /*parent */) const
{
    return 3;
}


Qt::ItemFlags PartInertiaModel::flags(const QModelIndex &index) const
{
    if (index.column()==1) return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    else                   return QAbstractTableModel::flags(index);
}


QVariant PartInertiaModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==Qt::DisplayRole && orientation==Qt::Horizontal)
    {
//        QString strMass = Units::massUnitLabel();
//        QString strLength = Units::lengthUnitLabel();
//        QString strInertia = Units::inertiaUnitLabel();
        switch (section)
        {
            case 0:
                return QString(" ");

            case 1:
                return QString("Value");

            case 2:
                return QString("Unit");

            default:
                return QString();

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


QVariant PartInertiaModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if(!m_pInertia) return QVariant();

    switch(role)
    {
        case Qt::EditRole:
        {
            switch(col)
            {
                case 1:
                {
                    if      (row==0)
                    {
                        return m_pInertia->structuralMass() * Units::kgtoUnit();
                    }
                    else if (row==1) return m_pInertia->CoG_s().x       * Units::mtoUnit();
                    else if (row==2) return m_pInertia->CoG_s().y       * Units::mtoUnit();
                    else if (row==3) return m_pInertia->CoG_s().z       * Units::mtoUnit();
                    else if (row==4) return m_pInertia->Ixx_s()         * Units::kgm2toUnit();
                    else if (row==5) return m_pInertia->Ixy_s()         * Units::kgm2toUnit();
                    else if (row==6) return m_pInertia->Ixz_s()         * Units::kgm2toUnit();
                    else if (row==7) return m_pInertia->Iyy_s()         * Units::kgm2toUnit();
                    else if (row==8) return m_pInertia->Iyz_s()         * Units::kgm2toUnit();
                    else if (row==9) return m_pInertia->Izz_s()         * Units::kgm2toUnit();
                    else return 0.0;

                }
                default:
                {
                    QString strange;
                    strange = QString::asprintf("row=%d col=%d", index.row(), index.column());
                    return QVariant();
                }
            }
        }
        case Qt::DisplayRole:
        {
            QString strange;

            switch(col)
            {
                case 0: // label
                {
                    if     (row==0) return QString("Mass=");
                    else if(row==1) return QString("CoG_x=");
                    else if(row==2) return QString("CoG_y=");
                    else if(row==3) return QString("CoG_z=");
                    else if(row==4) return QString("Ixx=");
                    else if(row==5) return QString("Ixy=");
                    else if(row==6) return QString("Ixz=");
                    else if(row==7) return QString("Iyy=");
                    else if(row==8) return QString("Iyz=");
                    else if(row==9) return QString("Izz=");
                    else return QVariant();
                }
                case 1:
                {
                    if      (row==0) strange = QString::asprintf("%.g", m_pInertia->structuralMass() * Units::kgtoUnit());
                    else if (row==1) strange = QString::asprintf("%.g", m_pInertia->CoG_s().x        * Units::mtoUnit());
                    else if (row==2) strange = QString::asprintf("%.g", m_pInertia->CoG_s().y        * Units::mtoUnit());
                    else if (row==3) strange = QString::asprintf("%.g", m_pInertia->CoG_s().z        * Units::mtoUnit());
                    else if (row==4) strange = QString::asprintf("%.g", m_pInertia->Ixx_s()          * Units::kgm2toUnit());
                    else if (row==5) strange = QString::asprintf("%.g", m_pInertia->Ixy_s()          * Units::kgm2toUnit());
                    else if (row==6) strange = QString::asprintf("%.g", m_pInertia->Ixz_s()          * Units::kgm2toUnit());
                    else if (row==7) strange = QString::asprintf("%.g", m_pInertia->Iyy_s()          * Units::kgm2toUnit());
                    else if (row==8) strange = QString::asprintf("%.g", m_pInertia->Iyz_s()          * Units::kgm2toUnit());
                    else if (row==9) strange = QString::asprintf("%.g", m_pInertia->Izz_s()          * Units::kgm2toUnit());
                    return strange;
                }
                case 2:
                {
                    if     (row==0) return Units::massUnitQLabel();
                    else if(row==1) return Units::lengthUnitQLabel();
                    else if(row==2) return Units::lengthUnitQLabel();
                    else if(row==3) return Units::lengthUnitQLabel();
                    else if(row==4) return Units::inertiaUnitQLabel();
                    else if(row==5) return Units::inertiaUnitQLabel();
                    else if(row==6) return Units::inertiaUnitQLabel();
                    else if(row==7) return Units::inertiaUnitQLabel();
                    else if(row==8) return Units::inertiaUnitQLabel();
                    else if(row==9) return Units::inertiaUnitQLabel();
                    break;
                }
                default:
                {
                    strange = QString::asprintf("row=%d col=%d", index.row(), index.column());
                    return strange;
                }
            }

            break;
        }
        case Qt::TextAlignmentRole:
        {
            if(col==0 || col==1) return QVariant(Qt::AlignRight | Qt::AlignVCenter);
            else                 return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }

        case Qt::ToolTipRole:
        {
            QString strange = QString();
            switch (index.row())
            {
                case 0:
                    strange = "<p>The part's structural mass excluding point masses</p>";
                    break;
                case 1:
                case 2:
                case 3:
                    strange = "<p>The position of the CoG of the part's structural mass excluding point masses</p>";
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                    strange = "<p>The inertia tensor components of the part's structural mass "
                              "in the frame centered at the structure's CoG, and excluding point masses.</p>";
                    break;
                default:
                    strange = QString();
                    break;
            }
            return strange;
        }
    }
    return QVariant();
}


bool PartInertiaModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!m_pInertia) return false;
    int row = index.row();
    int col = index.column();
    if(role==Qt::EditRole)
    {
        if(col==1)
        {
            if     (row==0) m_pInertia->setStructuralMass(value.toDouble()/Units::kgtoUnit());
            else if(row==1) m_pInertia->m_CoG_s.x = value.toDouble()/Units::mtoUnit();
            else if(row==2) m_pInertia->m_CoG_s.y = value.toDouble()/Units::mtoUnit();
            else if(row==3) m_pInertia->m_CoG_s.z = value.toDouble()/Units::mtoUnit();
            else if(row==4) m_pInertia->setIxx_s(value.toDouble()/Units::kgm2toUnit());
            else if(row==5) m_pInertia->setIxy_s(value.toDouble()/Units::kgm2toUnit());
            else if(row==6) m_pInertia->setIxz_s(value.toDouble()/Units::kgm2toUnit());
            else if(row==7) m_pInertia->setIyy_s(value.toDouble()/Units::kgm2toUnit());
            else if(row==8) m_pInertia->setIyz_s(value.toDouble()/Units::kgm2toUnit());
            else if(row==9) m_pInertia->setIzz_s(value.toDouble()/Units::kgm2toUnit());
            emit dataChanged(index,index);
        }
    }
    else if (role==Qt::CheckStateRole)
    {
        if (index.column()==2315)
        {
        }
    }

    return false;

//    return QAbstractItemModel::setData(index,value,role);
}


/** custom method to update the qtableview if the underlying object has changed */
void PartInertiaModel::updateData()
{
//    QModelIndex idxTL = index(0,0);
//    QModelIndex idxBR = index(rowCount(), columnCount());
    beginResetModel();
    endResetModel();
//    emit(dataChanged(idxTL, idxBR));
}






