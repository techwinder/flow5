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

#include <QStandardItem>
#include <QHeaderView>

#include "objecttreeview.h"
#include <api/objects_global.h>
#include <api/planexfl.h>
#include <api/utils.h>
#include <api/wingxfl.h>
#include <api/xml_globals.h>

#include <core/enums_core.h>
#include <api/units.h>
#include <core/xflcore.h>
#include <modules/xobjects.h>


ObjectTreeView::ObjectTreeView(QWidget *pParent) : QTreeView (pParent)
{
    header()->setSectionResizeMode(QHeaderView::Interactive);

    header()->setStretchLastSection(true);
    header()->setDefaultAlignment(Qt::AlignCenter);

    setEditTriggers(QAbstractItemView::AllEditTriggers);
    setSelectionBehavior (QAbstractItemView::SelectRows);

    setWindowTitle(tr("Objects"));
}


void ObjectTreeView::readWingTree(WingXfl *pWing)
{
    QString object, field, value;
    QStandardItemModel *m_pModel = dynamic_cast<QStandardItemModel*>(model());
    QModelIndex indexlevel = m_pModel->index(0,0);
    object = indexlevel.sibling(indexlevel.row(),0).data().toString();
    field = indexlevel.sibling(indexlevel.row(),1).data().toString();
    value = indexlevel.sibling(indexlevel.row(),2).data().toString();
    if(object.compare("Wing", Qt::CaseInsensitive)==0)
    {
        pWing->setWingType(xml::wingType(value));
    }
    indexlevel = m_pModel->index(0,0, indexlevel);

    do
    {
        QStandardItem *pItem = m_pModel->itemFromIndex(indexlevel);

        if(!pItem) return;
        if(pItem->child(0,0))
        {
            object = indexlevel.sibling(indexlevel.row(),0).data().toString();

            if(object.compare("Fin data", Qt::CaseInsensitive)==0)
            {
                QModelIndex subIndex = pItem->child(0,0)->index();
                do
                {
                    subIndex = subIndex.sibling(subIndex.row()+1,0);
                }while(subIndex.isValid());
            }
            else if(object.compare("Inertia", Qt::CaseInsensitive)==0)  readInertiaTree(pWing->inertia(), pItem->child(0,0)->index());
            else if(object.compare("Sections", Qt::CaseInsensitive)==0)
            {
                QModelIndex subIndex = pItem->child(0,0)->index();
                do
                {
                    QStandardItem *pSubItem = m_pModel->itemFromIndex(subIndex);
                    readWingSectionTree(pWing, pSubItem->child(0,0)->index());
                    subIndex = subIndex.sibling(subIndex.row()+1,0);
                }while(subIndex.isValid());
            }
        }
        else
        {
            //no more children
            object = indexlevel.sibling(indexlevel.row(),0).data().toString();
            field = indexlevel.sibling(indexlevel.row(),1).data().toString();
            bool bValue = indexlevel.sibling(indexlevel.row(),2).data().toBool();

            if     (field.compare("Symmetric",         Qt::CaseInsensitive)==0) pWing->setsymmetric(bValue);
            else if(field.compare("Two sided",         Qt::CaseInsensitive)==0) pWing->setTwoSided(bValue);
            else if(field.compare("Closed inner side", Qt::CaseInsensitive)==0) pWing->setClosedInnerSide(bValue);

        }

        indexlevel = indexlevel.sibling(indexlevel.row()+1,0);

    } while(indexlevel.isValid());
}


void ObjectTreeView::readColorTree(QColor &color, QModelIndex indexLevel)
{
    QString object, field, value;
    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();
        field = indexLevel.sibling(indexLevel.row(),1).data().toString();
        value = indexLevel.sibling(indexLevel.row(),2).data().toString();

        QModelIndex dataIndex = indexLevel.sibling(indexLevel.row(),2);

        if      (field.compare("red",   Qt::CaseInsensitive)==0)  color.setRed(dataIndex.data().toInt());
        else if (field.compare("green", Qt::CaseInsensitive)==0)  color.setGreen(dataIndex.data().toInt());
        else if (field.compare("blue",  Qt::CaseInsensitive)==0)  color.setBlue(dataIndex.data().toInt());
        else if (field.compare("alpha", Qt::CaseInsensitive)==0)  color.setAlpha(dataIndex.data().toInt());

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
    }
    while(indexLevel.isValid());
}


void ObjectTreeView::readInertiaTree(Inertia &inertia, QModelIndex indexLevel)
{
    inertia.clearPointMasses();

    QStandardItemModel *m_pModel = dynamic_cast<QStandardItemModel*>(model());

    QString object, field, value;
    QModelIndex dataIndex;
    do
    {
        QStandardItem *pItem = m_pModel->itemFromIndex(indexLevel);
        if(pItem->child(0,0))
        {
            object = indexLevel.sibling(indexLevel.row(),0).data().toString();
            if(object.indexOf("Point_mass_", Qt::CaseInsensitive)>=0)
            {
                PointMass ppm ;
                readPointMassTree(ppm, pItem->child(0,0)->index());
                inertia.appendPointMass(ppm);
            }
        }
        else
        {
            //no more children
            object = indexLevel.sibling(indexLevel.row(),0).data().toString();
            field = indexLevel.sibling(indexLevel.row(),1).data().toString();
            value = indexLevel.sibling(indexLevel.row(),2).data().toString();
            dataIndex = indexLevel.sibling(indexLevel.row(),2);

            if     (field.compare("Volume Mass", Qt::CaseInsensitive)==0)   inertia.setStructuralMass(dataIndex.data().toDouble()/Units::kgtoUnit());
        }

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

    } while(indexLevel.isValid());
}


void ObjectTreeView::readPointMassTree(PointMass &pm, QModelIndex indexLevel)
{
    QString object, field, value;
    QModelIndex dataIndex;
//    QStandardItemModel *m_pModel = dynamic_cast<QStandardItemModel*>(model());

    // not expecting any more children
    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();
        field = indexLevel.sibling(indexLevel.row(),1).data().toString();
        value = indexLevel.sibling(indexLevel.row(),2).data().toString();
        dataIndex = indexLevel.sibling(indexLevel.row(),2);

        if      (field.compare("mass", Qt::CaseInsensitive)==0) pm.setMass(dataIndex.data().toDouble()/Units::kgtoUnit());
        else if (field.compare("tag",  Qt::CaseInsensitive)==0) pm.setTag(value.toStdString());
        else if (field.compare("x",    Qt::CaseInsensitive)==0) pm.setXPosition(dataIndex.data().toDouble()/Units::mtoUnit());
        else if (field.compare("y",    Qt::CaseInsensitive)==0) pm.setYPosition(dataIndex.data().toDouble()/Units::mtoUnit());
        else if (field.compare("z",    Qt::CaseInsensitive)==0) pm.setZPosition(dataIndex.data().toDouble()/Units::mtoUnit());

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

    } while(indexLevel.isValid());
}


void ObjectTreeView::readVectorTree(Vector3d &V, QModelIndex indexLevel)
{
    QString object, field, value;
    QModelIndex dataIndex;
    // not expecting any more children
    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();
        field = indexLevel.sibling(indexLevel.row(),1).data().toString();
        value = indexLevel.sibling(indexLevel.row(),2).data().toString();
        dataIndex = indexLevel.sibling(indexLevel.row(),2);

        if (field.compare("x", Qt::CaseInsensitive)==0)      V.x = dataIndex.data().toDouble()/Units::mtoUnit();
        else if (field.compare("y", Qt::CaseInsensitive)==0) V.y = dataIndex.data().toDouble()/Units::mtoUnit();
        else if (field.compare("z", Qt::CaseInsensitive)==0) V.z = dataIndex.data().toDouble()/Units::mtoUnit();

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
    } while(indexLevel.isValid());
}


void ObjectTreeView::readWingSectionTree(WingXfl *pWing, QModelIndex indexLevel)
{
    QString object, field, value;
    QModelIndex dataIndex;

    // not expecting any more children
    WingSection pWS;
    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();
        field = indexLevel.sibling(indexLevel.row(),1).data().toString();
        value = indexLevel.sibling(indexLevel.row(),2).data().toString();


        dataIndex = indexLevel.sibling(indexLevel.row(),2);

        if      (field.compare("span position", Qt::CaseInsensitive)==0)        pWS.setYPosition(dataIndex.data().toDouble()/Units::mtoUnit());
        else if (field.compare("chord", Qt::CaseInsensitive)==0)                pWS.setChord(dataIndex.data().toDouble()/Units::mtoUnit());
        else if (field.compare("offset", Qt::CaseInsensitive)==0)               pWS.setXOffset(dataIndex.data().toDouble()/Units::mtoUnit());
        else if (field.compare("dihedral", Qt::CaseInsensitive)==0)             pWS.setDihedral(dataIndex.data().toDouble());
        else if (field.compare("twist", Qt::CaseInsensitive)==0)                pWS.setTwist(dataIndex.data().toDouble());
        else if (field.compare("x-panels", Qt::CaseInsensitive)==0)             pWS.setNX(dataIndex.data().toInt());
        else if (field.compare("y-panels", Qt::CaseInsensitive)==0)             pWS.setNY(dataIndex.data().toInt());
        else if (field.compare("x-distribution", Qt::CaseInsensitive)==0)       pWS.setXDistType(xfl::distributionType(value.toStdString()));
        else if (field.compare("y-distribution", Qt::CaseInsensitive)==0)       pWS.setYDistType(xfl::distributionType(value.toStdString()));
        else if (field.compare("Left side foil name", Qt::CaseInsensitive)==0)  pWS.setLeftFoilName( value.toStdString());
        else if (field.compare("Right side foil name", Qt::CaseInsensitive)==0) pWS.setRightFoilName( value.toStdString());

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
    } while(indexLevel.isValid());

    pWing->appendWingSection(pWS);
}


void ObjectTreeView::fillWingTreeView(const WingXfl *pWing)
{
    QStandardItemModel *pModel = dynamic_cast<QStandardItemModel*>(model());
    pModel->removeRows(0, pModel->rowCount());

    QList<QStandardItem*> wingFolder = xfl::prepareRow("Wing", "Type", xml::wingType(pWing->wingType()));
    wingFolder.at(2)->setData(xfl::WINGTYPE, Qt::UserRole);


//    QStandardItem *pItem = m_pModel->itemFromIndex(rootIndex());
    QStandardItem *pRootItem = pModel->invisibleRootItem();
    pRootItem->appendRow(wingFolder);

    QModelIndex rootindex = pModel->index(0,0);
    if(rootindex.isValid())
        expand(rootindex);

    QList<QStandardItem*> dataItem;

    dataItem = xfl::prepareBoolRow("Symmetric", "Symmetric", pWing->isSymmetric());
    wingFolder.front()->appendRow(dataItem);
    dataItem = xfl::prepareBoolRow("Two sided", "Two sided", pWing->isTwoSided());
    wingFolder.front()->appendRow(dataItem);
    dataItem = xfl::prepareBoolRow("Closed inner side", "Closed inner side", pWing->isClosedInnerSide());
    wingFolder.front()->appendRow(dataItem);

    QList<QStandardItem*> wingInertiaFolder = xfl::prepareRow("Inertia");
    wingFolder.front()->appendRow(wingInertiaFolder);
    {
        fillInertia(pWing->inertia(), wingInertiaFolder.front());
    }

    QList<QStandardItem*> wingSectionFolder = xfl::prepareRow("Sections");
    wingFolder.front()->appendRow(wingSectionFolder);
    {
/*        if(m_pPlane->wing(m_enumActiveWingType)==pWing)
        {
            m_pStruct->expand(m_pModel->indexFromItem(wingSectionFolder.front()));
        }*/
        for(int iws=0; iws<pWing->nSections(); iws++)
        {
            WingSection const &wingsec = pWing->section(iws);

            QList<QStandardItem*> sectionFolder = xfl::prepareRow(QString("Section_%1").arg(iws+1));
            wingSectionFolder.front()->appendRow(sectionFolder);
            {
                dataItem = xfl::prepareDoubleRow("", "span position", wingsec.yPosition()*Units::mtoUnit(), Units::lengthUnitQLabel());
                sectionFolder.front()->appendRow(dataItem);

                dataItem = xfl::prepareDoubleRow("", "chord", wingsec.chord()*Units::mtoUnit(), Units::lengthUnitQLabel());
                sectionFolder.front()->appendRow(dataItem);

                dataItem = xfl::prepareDoubleRow("", "offset", wingsec.offset()*Units::mtoUnit(), Units::lengthUnitQLabel());
                sectionFolder.front()->appendRow(dataItem);

                dataItem = xfl::prepareDoubleRow("", "dihedral", wingsec.dihedral(), DEGch);
                sectionFolder.front()->appendRow(dataItem);

                dataItem = xfl::prepareDoubleRow("", "twist", wingsec.twist(), DEGch);
                sectionFolder.front()->appendRow(dataItem);

                dataItem = xfl::prepareIntRow("", "x-panels", wingsec.nXPanels());
                sectionFolder.front()->appendRow(dataItem);

                dataItem = xfl::prepareRow("", "x-distribution", QString::fromStdString(xfl::distributionType(wingsec.xDistType())));
                dataItem.at(2)->setData(xfl::PANELDISTRIBUTION, Qt::UserRole);
                sectionFolder.front()->appendRow(dataItem);

                dataItem = xfl::prepareIntRow("", "y-panels",wingsec.nYPanels());
                sectionFolder.front()->appendRow(dataItem);

                dataItem = xfl::prepareRow("", "y-distribution", QString::fromStdString(xfl::distributionType(wingsec.yDistType())));
                sectionFolder.front()->appendRow(dataItem);
                dataItem.at(2)->setData(xfl::PANELDISTRIBUTION, Qt::UserRole);

                dataItem = xfl::prepareRow("", "Left side foil name", wingsec.m_LeftFoilName.length() ? QString::fromStdString(wingsec.m_LeftFoilName) : "No left foil defined");
                dataItem.at(2)->setData(xfl::FOILNAME, Qt::UserRole);
                sectionFolder.front()->appendRow(dataItem);

                dataItem = xfl::prepareRow("", "Right side foil name", wingsec.m_RightFoilName.length() ? QString::fromStdString(wingsec.m_RightFoilName) : "No right foil defined");
                dataItem.at(2)->setData(xfl::FOILNAME, Qt::UserRole);
                sectionFolder.front()->appendRow(dataItem);
            }
        }
    }
}


void ObjectTreeView::fillColorTree(QColor clr, QStandardItem *pColorItem)
{
    QList<QStandardItem *>dataItem = xfl::prepareIntRow("", "red", clr.red());
    pColorItem->appendRow(dataItem);

    dataItem = xfl::prepareIntRow("", "green", clr.green());
    pColorItem->appendRow(dataItem);

    dataItem = xfl::prepareIntRow("", "blue", clr.blue());
    pColorItem->appendRow(dataItem);

    dataItem = xfl::prepareIntRow("", "alpha", clr.alpha());
    pColorItem->appendRow(dataItem);
}


void ObjectTreeView::fillInertia(Inertia const &inertia, QStandardItem * pWingInertiaFolder)
{
    QList<QStandardItem*> dataItem = xfl::prepareDoubleRow( "", "Volume mass", inertia.structuralMass()*Units::kgtoUnit(), Units::massUnitQLabel());
    dataItem.at(2)->setData(xfl::DOUBLEVALUE, Qt::UserRole);
    pWingInertiaFolder->appendRow(dataItem);

    for(int iwm=0; iwm<inertia.pointMassCount(); iwm++)
    {
        PointMass const &pm = inertia.pointMassAt(iwm);
        QList<QStandardItem*> wingPointMassFolder = xfl::prepareRow(QString("Point_mass_%1").arg(iwm+1));

        pWingInertiaFolder->appendRow(wingPointMassFolder);
        {
            QList<QStandardItem*> dataItem = xfl::prepareRow("", "Tag", QString::fromStdString(pm.tag()));
            dataItem.at(2)->setData(xfl::STRING, Qt::UserRole);
            wingPointMassFolder.front()->appendRow(dataItem);

            dataItem = xfl::prepareDoubleRow("", "mass", pm.mass()*Units::kgtoUnit(), Units::massUnitQLabel());
            dataItem.at(2)->setData(xfl::DOUBLEVALUE, Qt::UserRole);
            wingPointMassFolder.front()->appendRow(dataItem);

            dataItem = xfl::prepareDoubleRow("", "x", pm.position().x*Units::mtoUnit(), Units::lengthUnitQLabel());
            dataItem.at(2)->setData(xfl::DOUBLEVALUE, Qt::UserRole);
            wingPointMassFolder.front()->appendRow(dataItem);

            dataItem = xfl::prepareDoubleRow("", "y", pm.position().y*Units::mtoUnit(), Units::lengthUnitQLabel());;
            dataItem.at(2)->setData(xfl::DOUBLEVALUE, Qt::UserRole);
            wingPointMassFolder.front()->appendRow(dataItem);

            dataItem = xfl::prepareDoubleRow("", "z", pm.position().z*Units::mtoUnit(), Units::lengthUnitQLabel());
            dataItem.at(2)->setData(xfl::DOUBLEVALUE, Qt::UserRole);
            wingPointMassFolder.front()->appendRow(dataItem);
        }
    }
}



