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


#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHideEvent>

#include "analysisseldlg.h"

#include <core/displayoptions.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <api/objects3d.h>
#include <api/plane.h>
#include <api/planepolar.h>
#include <api/sailobjects.h>
#include <api/boat.h>
#include <api/boatpolar.h>

#include <interfaces/widgets/mvc/expandabletreeview.h>
#include <interfaces/widgets/mvc/objecttreedelegate.h>
#include <interfaces/widgets/mvc/objecttreeitem.h>
#include <interfaces/widgets/mvc/objecttreemodel.h>

QByteArray AnalysisSelDlg::s_Geometry;


AnalysisSelDlg::AnalysisSelDlg(QWidget *parent) : QDialog(parent)
{
    m_Object = FOIL;
    setupLayout();
}


void AnalysisSelDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_plabTitle = new QLabel(tr("Select the analyses to duplicate:"));

        m_pStruct = new ExpandableTreeView;
        m_pStruct->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
        m_pStruct->setSelectionMode(QAbstractItemView::SingleSelection);
        m_pStruct->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_pStruct->setUniformRowHeights(true);
        m_pStruct->setRootIsDecorated(true);
        m_pStruct->setSelectionMode(QAbstractItemView::MultiSelection);

        QStringList labels;
        labels << tr("Object") << "1234567"<< "";

        m_pModel = new ObjectTreeModel(this);
        m_pModel->setHeaderData(0, Qt::Horizontal, tr("Objects"), Qt::DisplayRole);
        m_pModel->setHeaderData(1, Qt::Horizontal, "1234567890123", Qt::EditRole);
        m_pModel->setHeaderData(1, Qt::Horizontal, "1234567890123", Qt::DisplayRole);
        m_pModel->setHeaderData(2, Qt::Horizontal, "123", Qt::DisplayRole);
        m_pModel->setHeaderData(2, Qt::Horizontal, Qt::AlignRight, Qt::TextAlignmentRole);
        m_pStruct->setModel(m_pModel);
        m_pStruct->setRootIndex(QModelIndex());

        m_pStruct->hideColumn(1);
        m_pStruct->hideColumn(2);
        m_pStruct->header()->hide();
        m_pStruct->header()->setStretchLastSection(false);
        m_pStruct->header()->hide();
        m_pStruct->header()->setStretchLastSection(false);
        m_pStruct->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        m_pStruct->header()->setSectionResizeMode(1, QHeaderView::Fixed);
        m_pStruct->header()->setSectionResizeMode(2, QHeaderView::Fixed);
        int av = DisplayOptions::treeFontStruct().averageCharWidth();
        m_pStruct->header()->resizeSection(1, 7*av);
        m_pStruct->header()->resizeSection(2, 3*av);

        m_pDelegate = new ObjectTreeDelegate(this);
        m_pStruct->setItemDelegate(m_pDelegate);


        m_pDelegate = new ObjectTreeDelegate(this);
        m_pDelegate->showStyle(false);

        m_pStruct->setItemDelegate(m_pDelegate);

        QItemSelectionModel *selectionModel = new QItemSelectionModel(m_pModel);
        m_pStruct->setSelectionModel(selectionModel);
        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
        }

        pMainLayout->addWidget(m_plabTitle);
        pMainLayout->addWidget(m_pStruct);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void AnalysisSelDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok)     == pButton) accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton) reject();
}


void AnalysisSelDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
    pEvent->ignore();
}


void AnalysisSelDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
    pEvent->ignore();
}


void AnalysisSelDlg::initDialog(Foil const*pCurFoil, Plane const*pCurPlane, Boat const*pCurBoat)
{
    ObjectTreeItem *pRootItem = m_pModel->rootItem();
    pRootItem->setName(tr("Objects"));

    if(pCurFoil)
    {
        m_Object = FOIL;
        m_plabTitle->setText(QString::fromStdString(pCurFoil->name())+tr(":\nSelect the analyses to duplicate"));
        for(int iFoil=0; iFoil<Objects2d::nFoils(); iFoil++)
        {
            Foil const *pFoil = Objects2d::foilAt(iFoil);
            if(!pFoil) continue;

            ObjectTreeItem *pFoilItem = m_pModel->appendRow(pRootItem, pFoil->name(), pFoil->theStyle(), Qt::Unchecked);

            for(int iPolar=0; iPolar<Objects2d::nPolars(); iPolar++)
            {
                Polar *pPolar = Objects2d::polarAt(iPolar);
                if(!pPolar) continue;
                if(pPolar && pPolar->foilName().compare(pFoil->name())==0)
                {
                    LineStyle ls(pPolar->theStyle());
                    ls.m_bIsEnabled = true;
                    m_pModel->appendRow(pFoilItem, pPolar->name(), ls, Qt::Unchecked);
                }
            }
        }

    }
    else if(pCurPlane)
    {
        m_Object = PLANE;
        m_plabTitle->setText(QString::fromStdString(pCurPlane->name())+tr(":\nSelect the analyses to duplicate"));
        for(int iPlane=0; iPlane<Objects3d::nPlanes(); iPlane++)
        {
            Plane const *pPlane = Objects3d::planeAt(iPlane);
            if(!pPlane) continue;

            LineStyle ls(pPlane->theStyle());
            ObjectTreeItem *pPlaneItem = m_pModel->appendRow(pRootItem, pPlane->name(), pPlane->theStyle(), Qt::Unchecked);

            for(int iPolar=0; iPolar<Objects3d::nPolars(); iPolar++)
            {
                PlanePolar *pWPolar = Objects3d::plPolarAt(iPolar);
                if(!pWPolar) continue;
                if(pWPolar && pWPolar->planeName().compare(pPlane->name())==0)
                {
                    LineStyle ls(pWPolar->theStyle());
                    ls.m_bIsEnabled = true;
                    m_pModel->appendRow(pPlaneItem, pWPolar->name(), ls, Qt::Unchecked);
                }
            }
        }
    }
    else if(pCurBoat)
    {
       m_Object = BOAT;
       m_plabTitle->setText(QString::fromStdString(pCurBoat->name())+tr(":\nSelect the analyses to duplicate"));
       for(int iBoat=0; iBoat<SailObjects::nBoats(); iBoat++)
       {
           Boat const *pBoat = SailObjects::boat(iBoat);
           if(!pBoat) continue;

           ObjectTreeItem *pPlaneItem = m_pModel->appendRow(pRootItem, pBoat->name(), pBoat->theStyle(), Qt::Unchecked);

           for(int iPolar=0; iPolar<SailObjects::nBtPolars(); iPolar++)
           {
               BoatPolar *pBtPolar = SailObjects::btPolar(iPolar);
               if(!pBtPolar) continue;
               if(pBtPolar && pBtPolar->boatName().compare(pBoat->name())==0)
               {
                   LineStyle ls(pBtPolar->theStyle());
                   ls.m_bIsEnabled = true;
                   m_pModel->appendRow(pPlaneItem, pBtPolar->name(), ls, Qt::Unchecked);
               }
           }
       }
    }

    m_pStruct->onPolarLevel();

    m_pButtonBox->setFocus();
}


void AnalysisSelDlg::accept()
{
    QModelIndexList indexes = m_pStruct->selectionModel()->selectedIndexes();
    for(int ip=0; ip<indexes.size(); ip++)
    {
        QModelIndex index = indexes.at(ip);
        if (index.isValid() && index.column()==0)
        {
            ObjectTreeItem *pSelectedItem = m_pModel->itemFromIndex(index);
            if(pSelectedItem && pSelectedItem->isPolarLevel())
            {
                ObjectTreeItem *parentItem = pSelectedItem->parentItem();
                if(parentItem)
                {
                    switch(m_Object)
                    {
                        case FOIL:
                        {
                            Foil *pFoil = Objects2d::foil(parentItem->name().toStdString());
                            Polar *pPolar = Objects2d::polar(pFoil, pSelectedItem->name().toStdString());
                            if(pFoil && pPolar) m_Selected2dPolars.append(pPolar);
                            break;
                        }
                        case PLANE:
                        {
                            Plane *pPlane  = Objects3d::plane(parentItem->name().toStdString());
                            PlanePolar *pWPolar = Objects3d::wPolar(pPlane, pSelectedItem->name().toStdString());
                            if(pPlane && pWPolar)
                            {
                                m_Selected3dPolars.append(pWPolar);
                            }
                            break;
                        }
                        case BOAT:
                        {
                            Boat *pBoat  = SailObjects::boat(parentItem->name().toStdString());
                            BoatPolar *pBtPolar = SailObjects::btPolar(pBoat, pSelectedItem->name().toStdString());
                            if(pBoat && pBtPolar)
                            {
                                m_Selected3dPolars.append(pBtPolar);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    QDialog::accept();
}
