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


#include <QApplication>
#include <QHeaderView>
#include <QMenu>

#include "pointmasstable.h"
#include <api/units.h>
#include <api/part.h>
#include <interfaces/widgets/customwts/xfldelegate.h>

PointMassTable::PointMassTable(QWidget *pParent) : CPTableView(pParent)
{
    m_pInertia = nullptr;

    setEditable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pMassModel = new ActionItemModel(this);
    m_pMassModel->setRowCount(10);//temporary
    m_pMassModel->setColumnCount(6);
    m_pMassModel->setActionColumn(5);

    m_pMassModel->setHeaderData(0, Qt::Horizontal, "Mass ("+Units::massUnitQLabel()   +")");
    m_pMassModel->setHeaderData(1, Qt::Horizontal, "x ("   +Units::lengthUnitQLabel() +")");
    m_pMassModel->setHeaderData(2, Qt::Horizontal, "y ("   +Units::lengthUnitQLabel() +")");
    m_pMassModel->setHeaderData(3, Qt::Horizontal, "z ("   +Units::lengthUnitQLabel() +")");
    m_pMassModel->setHeaderData(4, Qt::Horizontal, "Description");
    m_pMassModel->setHeaderData(5, Qt::Horizontal, "Actions");

    int rc = m_pMassModel->rowCount();
    m_pMassModel->removeRows(0, rc);
    setModel(m_pMassModel);
    horizontalHeader()->setStretchLastSection(true);

    m_pMassDelegate = new XflDelegate(this);
    setItemDelegate(m_pMassDelegate);
    connect(m_pMassDelegate,  SIGNAL(closeEditor(QWidget*)), SLOT(onPointMassCellChanged(QWidget*)));

    m_pMassDelegate->setActionColumn(5);
    m_pMassDelegate->setDigits({3,3,3,3,-1});
    m_pMassDelegate->setItemTypes({XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::STRING, XflDelegate::ACTION});

    connect(this, SIGNAL(clicked(QModelIndex)), SLOT(onMassTableClicked(QModelIndex)));

    //make actions
    QAction *pInsertMassBefore = new QAction("Insert before", this);
    QAction *pInsertMassAfter  = new QAction("Insert after", this);
    QAction *pDeleteMassRow    = new QAction("Delete", this);
    QAction *pDuplicateMassRow = new QAction("Duplicate", this);
    QAction *pMoveUp           = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowUp),   "Move up",   this);
    QAction *pMoveDown         = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowDown), "Move down", this);

    m_pContextMenu = new QMenu("Point Mass",this);
    m_pContextMenu->addAction(pInsertMassBefore);
    m_pContextMenu->addAction(pInsertMassAfter);
    m_pContextMenu->addSeparator();
    m_pContextMenu->addAction(pDeleteMassRow);
    m_pContextMenu->addAction(pDuplicateMassRow);
    m_pContextMenu->addSeparator();
    m_pContextMenu->addAction(pMoveUp);
    m_pContextMenu->addAction(pMoveDown);
    connect(pInsertMassBefore, SIGNAL(triggered()), SLOT(onInsertMassBefore()));
    connect(pInsertMassAfter,  SIGNAL(triggered()), SLOT(onInsertMassAfter()));
    connect(pDeleteMassRow,    SIGNAL(triggered()), SLOT(onDeleteMassRow()));
    connect(pDuplicateMassRow, SIGNAL(triggered()), SLOT(onDuplicateMassRow()));
    connect(pMoveUp,           SIGNAL(triggered()), SLOT(onMoveUp()));
    connect(pMoveDown,         SIGNAL(triggered()), SLOT(onMoveDown()));
}


void PointMassTable::fillMassModel()
{
    QModelIndex index;
    int rowcount = 0;

    m_pMassModel->setRowCount(m_pInertia->pointMassCount()+1);
    rowcount =  m_pInertia->pointMassCount();

    int i=0;
    for(i=0; i<rowcount; i++)
    {
        PointMass const &pm = m_pInertia->pointMassAt(i);
//        if(pm.mass()>PRECISION || pm.tag().length())
        {
            index = m_pMassModel->index(i, 0, QModelIndex());
            m_pMassModel->setData(index, pm.mass()*Units::kgtoUnit());

            index = m_pMassModel->index(i, 1, QModelIndex());
            m_pMassModel->setData(index, pm.position().x*Units::mtoUnit());

            index = m_pMassModel->index(i, 2, QModelIndex());
            m_pMassModel->setData(index, pm.position().y*Units::mtoUnit());

            index = m_pMassModel->index(i, 3, QModelIndex());
            m_pMassModel->setData(index, pm.position().z*Units::mtoUnit());

            index = m_pMassModel->index(i, 4, QModelIndex());
            m_pMassModel->setData(index, QString::fromStdString(pm.tag()));

            index = m_pMassModel->index(i, 5, QModelIndex());
            m_pMassModel->setData(index, QString("..."));
        }
    }
    //add an extra empty line for a new mass
    index = m_pMassModel->index(i, 0, QModelIndex());
    m_pMassModel->setData(index, 0.0);

    index = m_pMassModel->index(i, 1, QModelIndex());
    m_pMassModel->setData(index, 0.0);

    index = m_pMassModel->index(i, 2, QModelIndex());
    m_pMassModel->setData(index, 0.0);

    index = m_pMassModel->index(i, 3, QModelIndex());
    m_pMassModel->setData(index, 0.0);

    index = m_pMassModel->index(i, 4, QModelIndex());
    m_pMassModel->setData(index, "");
}


void PointMassTable::readPointMassData()
{
    QModelIndex index;
    bool bOK=false;

    m_pInertia->clearPointMasses();

    for (int i=0; i<m_pMassModel->rowCount(); i++)
    {
        index = m_pMassModel->index(i, 0, QModelIndex());
        double mass = index.data().toDouble(&bOK)/Units::kgtoUnit();

        index = m_pMassModel->index(i, 1, QModelIndex());
        double x = index.data().toDouble(&bOK)/Units::mtoUnit();

        index = m_pMassModel->index(i, 2, QModelIndex());
        double y = index.data().toDouble(&bOK)/Units::mtoUnit();

        index = m_pMassModel->index(i, 3, QModelIndex());
        double z = index.data().toDouble(&bOK)/Units::mtoUnit();

        index = m_pMassModel->index(i, 4, QModelIndex());
        QString tag = index.data().toString();

        if(fabs(mass)>PRECISION || fabs(x)>PRECISION || fabs(y)>PRECISION || fabs(z)>PRECISION || tag.length())
        {
            m_pInertia->appendPointMass({mass, Vector3d(x,y,z), tag.toStdString()});
        }
    }
}



void PointMassTable::onInsertMassBefore()
{
    int sel = currentIndex().row();
    if(sel<0 || sel>=m_pInertia->pointMassCount()) return;

    m_pInertia->insertPointMass(sel, {0.0, Vector3d(0.0,0.0,0.0), std::string()});

    fillMassModel();
    closePersistentEditor(currentIndex());

    QModelIndex index = m_pMassModel->index(sel, 0, QModelIndex());
    setCurrentIndex(index);
}


void PointMassTable::onInsertMassAfter()
{
    int sel = currentIndex().row();
    if(sel<0 || sel>=m_pInertia->pointMassCount()) return;

    int pos = sel+1;

    if(pos>m_pInertia->pointMassCount()) pos = m_pInertia->pointMassCount();
    m_pInertia->insertPointMass(pos, {0.0, Vector3d(0.0,0.0,0.0), std::string()});


    fillMassModel();
//    closePersistentEditor(currentIndex());

    QModelIndex index = m_pMassModel->index(pos, 0, QModelIndex());
    setCurrentIndex(index);
}


void PointMassTable::onDuplicateMassRow()
{
    int sel = currentIndex().row();
    if(sel<0 || sel>=m_pInertia->pointMassCount()) return;

    m_pInertia->insertPointMass(sel+1, m_pInertia->pointMass(sel));

    fillMassModel();

    QModelIndex index = m_pMassModel->index(sel+1, 0, QModelIndex());
    setCurrentIndex(index);
}


void PointMassTable::onDeleteMassRow()
{
    closePersistentEditor(currentIndex());
    int sel = currentIndex().row();

    if(sel>=0 && sel<m_pInertia->pointMassCount()) m_pInertia->removePointMass(sel);


    fillMassModel();
}


void PointMassTable::onMassTableClicked(QModelIndex index)
{
    if(!index.isValid())
    {
    }
    else
    {
        switch(index.column())
        {
            case 5:
            {
                QRect itemrect = visualRect(index);
                QPoint menupos = mapToGlobal(itemrect.topLeft());
                m_pContextMenu->exec(menupos);

                break;
            }
            default:
            {
                break;
            }
        }
    }
}


void PointMassTable::onPointMassCellChanged(QWidget *)
{
    readPointMassData();
    fillMassModel();
    emit pointMassChanged();
}


void PointMassTable::resizeColumns()
{
    int n = 5; //action column
    QHeaderView *pHHeader = horizontalHeader();
    pHHeader->setSectionResizeMode(n, QHeaderView::Stretch);
    pHHeader->resizeSection(n, 1);

    double w = double(width())/100.0;
    int wtag  = int(20.0*w);
    int wdist = int(14.0*w);

    setColumnWidth(0, wdist);
    setColumnWidth(1, wdist);
    setColumnWidth(2, wdist);
    setColumnWidth(3, wdist);
    setColumnWidth(4, wtag);
//    horizontalHeader()->setStretchLastSection(true);
}


void PointMassTable::onMoveUp()
{
    int sel = currentIndex().row();
    if(sel==0) return; // can't move up
    if(sel>=m_pInertia->pointMassCount()) return;

    PointMass pm(m_pInertia->pointMass(sel));
    m_pInertia->removePointMass(sel);
    m_pInertia->insertPointMass(sel-1, pm);

    fillMassModel();
    QModelIndex index = m_pMassModel->index(sel-1, 0, QModelIndex());
    setCurrentIndex(index);
}


void PointMassTable::onMoveDown()
{
    int sel = currentIndex().row();
    if(sel<0 || sel>=m_pInertia->pointMassCount()-1) return;

    PointMass pm(m_pInertia->pointMass(sel));
    m_pInertia->removePointMass(sel);
    m_pInertia->insertPointMass(sel+1, pm);

    fillMassModel();
    QModelIndex index = m_pMassModel->index(sel-1, 0, QModelIndex());
    setCurrentIndex(index);
}
