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
#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QContextMenuEvent>
#include <QAction>
#include <QLineEdit>
#include <QHeaderView>

#include "boattreeview.h"

#include <globals/mainframe.h>
#include <modules/xsail/xsail.h>
#include <modules/xsail/menus/xsailmenus.h>

#include <api/units.h>
#include <api/boatopp.h>
#include <api/boatpolar.h>
#include <api/objects_global.h>
#include <api/boat.h>
#include <api/sailobjects.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/line/linemenu.h>
#include <interfaces/widgets/mvc/expandabletreeview.h>

#include <interfaces/widgets/mvc/objecttreedelegate.h>
#include <interfaces/widgets/mvc/objecttreeitem.h>
#include <interfaces/widgets/mvc/objecttreemodel.h>

QByteArray BoatTreeView::s_SplitterSizes;
XSail *BoatTreeView::s_pXSail = nullptr;
int BoatTreeView::s_Width=351;

BoatTreeView::BoatTreeView(QWidget *pParent) : QWidget(pParent)
{
    m_pStruct = nullptr;
    m_pModel  = nullptr;

    m_Selection = BoatTreeView::NOBOAT;

    setupLayout();

    QStringList labels;
    labels << "Object" << "1234567"<< "";

    m_pModel = new ObjectTreeModel(this);
    m_pModel->setHeaderData(0, Qt::Horizontal, "Objects", Qt::DisplayRole);
    m_pModel->setHeaderData(1, Qt::Horizontal, "1234567890123", Qt::EditRole);
    m_pModel->setHeaderData(1, Qt::Horizontal, "1234567890123", Qt::DisplayRole);
    m_pModel->setHeaderData(2, Qt::Horizontal, "123", Qt::DisplayRole);
    m_pModel->setHeaderData(2, Qt::Horizontal, Qt::AlignRight, Qt::TextAlignmentRole);

    m_pStruct->setModel(m_pModel);
    connect(m_pStruct->m_pleFilter, SIGNAL(returnPressed()), this, SLOT(onSetFilter()));

    m_pStruct->header()->hide();
    m_pStruct->header()->setStretchLastSection(false);
    m_pStruct->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_pStruct->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // so that ExpandableTreeView::sizeHintForColumn is used
    m_pStruct->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // so that ExpandableTreeView::sizeHintForColumn is used

    m_pDelegate = new ObjectTreeDelegate(this);
    m_pStruct->setItemDelegate(m_pDelegate);


    connect(m_pStruct, SIGNAL(pressed(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
    connect(m_pStruct, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onItemDoubleClicked(QModelIndex)));
//    connect(m_pStruct, SIGNAL(activated(QModelIndex)), this, SLOT(onActivated(QModelIndex)));
//    connect(m_pStruct, SIGNAL(clicked(QModelIndex)), this, SLOT(onActivated(QModelIndex)));
//    connect(m_pStruct->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onCurrentChanged(QModelIndex, QModelIndex)));
    connect(m_pStruct->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(onCurrentRowChanged(QModelIndex,QModelIndex)));

}


BoatTreeView::~BoatTreeView()
{
}


void BoatTreeView::showEvent(QShowEvent *pEvent)
{
    m_pMainSplitter->restoreState(s_SplitterSizes);
    pEvent->accept();
}


void BoatTreeView::hideEvent(QHideEvent *)
{
    s_Width = width();

    s_SplitterSizes = m_pMainSplitter->saveState();
}


void BoatTreeView::resizeEvent(QResizeEvent *pEvent)
{
    s_Width = width();
    updateGeometry(); // Notifies the layout system that the sizeHint()  has changed
    pEvent->accept();
}


void BoatTreeView::updateObjectView()
{
    fillModelView();
    selectObjects();
}


void BoatTreeView::setObjectProperties()
{
    std::string props;
    switch(m_Selection)
    {
        case BoatTreeView::BOAT:
        {
            if(s_pXSail->m_pCurBoat)
            {
                props = s_pXSail->m_pCurBoat->properties(true);
                break;
            }
            break;
        }
        case BoatTreeView::BTPOLAR:
        {
            if(s_pXSail->m_pCurBtPolar && s_pXSail->m_pCurBoat)
            {
                s_pXSail->m_pCurBtPolar->getProperties(props, xfl::TXT, false);
                break;
            }
            break;
        }
        case BoatTreeView::BOATOPP:
        {
            if(s_pXSail->m_pCurBtPolar && s_pXSail->m_pCurBtOpp)
            {
                s_pXSail->m_pCurBtOpp->getProperties(s_pXSail->curBoat(), s_pXSail->m_pCurBtPolar->density(), props, true);
                break;
            }
            break;
        }

        default:
        {
            props.clear();
            break;
        }
    }
    m_ppto->setStdText(props);
}


void BoatTreeView::setPropertiesFont(QFont const &fnt)
{
    m_ppto->setFont(fnt);
}


void BoatTreeView::setTreeFontStruct(const FontStruct &fntstruct)
{
    m_pStruct->setFont(fntstruct.font());
}


void BoatTreeView::setupLayout()
{
    m_pStruct = new ExpandableTreeView;
    m_pStruct->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pStruct->setUniformRowHeights(true);
    m_pStruct->setRootIsDecorated(true);
    connect(m_pStruct, SIGNAL(switchAll(bool)), SLOT(onSwitchAll(bool)));

    m_ppto = new PlainTextOutput;

    m_pMainSplitter = new QSplitter;
    m_pMainSplitter->setOrientation(Qt::Vertical);
    {
        m_pMainSplitter->setChildrenCollapsible(true);
        m_pMainSplitter->addWidget(m_pStruct);
        m_pMainSplitter->addWidget(m_ppto);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pStruct->cmdWidget());
        pMainLayout->addWidget(m_pMainSplitter);
    }
    setLayout(pMainLayout);
}


void BoatTreeView::fillModelView()
{
    m_pModel->removeRows(0, m_pModel->rowCount());

    ObjectTreeItem *pRootItem = m_pModel->rootItem();

    m_pStruct->selectionModel()->blockSignals(true);

    for(int iPlane=0; iPlane<SailObjects::nBoats(); iPlane++)
    {
        Boat const *pBoat = SailObjects::boat(iPlane);
        if(!pBoat) continue;

        LineStyle ls(pBoat->theStyle());
        ObjectTreeItem *pBoatItem = m_pModel->appendRow(pRootItem, pBoat->name(), pBoat->theStyle(), boatState(pBoat));
        fillBtPolars(pBoatItem, pBoat);
    }
    m_pStruct->selectionModel()->blockSignals(false);

    setOverallCheckStatus();
}


void BoatTreeView::fillBtPolars(ObjectTreeItem *pBoatItem, Boat const*pBoat)
{
    if(!pBoat || !pBoatItem) return;

    for(int iPolar=0; iPolar<SailObjects::nBtPolars(); iPolar++)
    {
        BoatPolar *pBtPolar = SailObjects::btPolar(iPolar);
        if(!pBtPolar) continue;
        if(pBtPolar && pBtPolar->boatName().compare(pBoat->name())==0)
        {
            LineStyle ls(pBtPolar->theStyle());
            ls.m_bIsEnabled = true;
            m_pModel->appendRow(pBoatItem, pBtPolar->name(), ls, btPolarState(pBtPolar));

            addBtOpps(pBtPolar);
        }
    }
}


void BoatTreeView::addBtOpps(BoatPolar* pBtPolar)
{
    if(!pBtPolar) pBtPolar = s_pXSail->m_pCurBtPolar;
    if(!pBtPolar) return;

    m_pStruct->selectionModel()->blockSignals(true);

    bool bAdded = false;

    //find this polar's plane parent
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        // find the polar's parent Plane
        if(pPlaneItem->name().toStdString().compare(pBtPolar->boatName())==0)
        {
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pBtPolarItem = pPlaneItem->child(jr);
                if(pBtPolarItem->name().toStdString().compare(pBtPolar->name())==0)
                {
                    m_Selection = BoatTreeView::BTPOLAR; /** @todo remove */

                    QModelIndex polarindex = m_pModel->index(jr, 0, pPlaneItem);
                    m_pModel->removeRows(0, pBtPolarItem->rowCount(), polarindex);

                    for(int iOpp=SailObjects::nBtOpps()-1; iOpp>=0; iOpp--)
                    {
                        BoatOpp const *pBtOpp = SailObjects::btOpp(iOpp);
                        if(pBtOpp->boatName().compare(pBtPolar->boatName())==0 && pBtOpp->polarName().compare(pBtPolar->name())==0)
                        {
                            QString strange;
                            strange = QString::asprintf("%7.3f", pBtOpp->ctrl());

                            LineStyle ls(pBtOpp->theStyle());
                            ls.m_bIsEnabled = false;
                            m_pModel->appendRow(pBtPolarItem, strange, ls, Qt::Unchecked);

                        }
                    }
                    bAdded = true;
                    break;
                }
            }
        }
        if(bAdded) break;
    }

//    onSetOverallCheckStatus();
    m_pStruct->selectionModel()->blockSignals(false);
}


void BoatTreeView::loadSettings(QSettings &settings)
{
    settings.beginGroup("PartInertiaDlg");
    {
        s_SplitterSizes = settings.value("HSplitterSizes").toByteArray();
    }
    settings.endGroup();
}


void BoatTreeView::saveSettings(QSettings &settings)
{
    settings.beginGroup("PartInertiaDlg");
    {
        settings.setValue("HSplitterSizes", s_SplitterSizes);
    }
    settings.endGroup();
}


void BoatTreeView::onCurrentRowChanged(QModelIndex curidx, QModelIndex )
{
    setObjectFromIndex(curidx);
    s_pXSail->updateView();
}


void BoatTreeView::onItemClicked(const QModelIndex &index)
{
    Boat *m_pBoat         = s_pXSail->m_pCurBoat;
    BoatPolar *m_pBtPolar = s_pXSail->m_pCurBtPolar;
    BoatOpp *m_pBtOpp     = s_pXSail->m_pCurBtOpp;

    if(index.column()==1)
    {
        ObjectTreeItem *pItem = m_pModel->itemFromIndex(index);

        if(m_pBtOpp)
        {
            if(s_pXSail->is3dView())
            {
                LineStyle ls(m_pBtOpp->theStyle());
                LineMenu *pLineMenu = new LineMenu(nullptr);
                pLineMenu->initMenu(ls);
                pLineMenu->exec(QCursor::pos());
                ls = pLineMenu->theStyle();
                m_pBtOpp->setLineStipple(ls.m_Stipple);
                m_pBtOpp->setLineWidth(ls.m_Width);
                m_pBtOpp->setLineColor(ls.m_Color);
                m_pBtOpp->setPointStyle(ls.m_Symbol);
                pItem->setTheStyle(ls);
                s_pXSail->resetCurves();
                emit s_pXSail->projectModified();
            }
        }
        else if(m_pBtPolar)
        {
            LineStyle ls(m_pBtPolar->theStyle());
            LineMenu *pLineMenu = new LineMenu(nullptr);
            pLineMenu->initMenu(ls);
            pLineMenu->exec(QCursor::pos());
            ls = pLineMenu->theStyle();

            SailObjects::setBPolarStyle(m_pBtPolar, ls, pLineMenu->styleChanged(), pLineMenu->widthChanged(), pLineMenu->colorChanged(), pLineMenu->pointsChanged());
            setCurveParams();

            pItem->setTheStyle(ls);
            s_pXSail->resetCurves();
            emit s_pXSail->projectModified();
        }
        else if(m_pBoat)
        {
            LineStyle ls(m_pBoat->theStyle());
            LineMenu *pLineMenu = new LineMenu(nullptr);
            pLineMenu->initMenu(ls);
            pLineMenu->exec(QCursor::pos());
            ls = pLineMenu->theStyle();

            SailObjects::setBoatStyle(m_pBoat, ls, pLineMenu->styleChanged(), pLineMenu->widthChanged(), pLineMenu->colorChanged(), pLineMenu->pointsChanged());
            setCurveParams();

            pItem->setTheStyle(ls);
            s_pXSail->resetCurves();
            emit s_pXSail->projectModified();

            s_pXSail->resetCurves();
        }
    }
    else if (index.column()==2)
    {
        if(m_pBtOpp)
        {
            if(s_pXSail->is3dView())
            {
                ObjectTreeItem *pItem = m_pModel->itemFromIndex(index);
                if(pItem)
                {
                    m_pBtOpp->setVisible(!m_pBtOpp->isVisible());
//                    pItem->setCheckState(m_pPOpp->isVisible() ? Qt::Checked : Qt::Unchecked);
                    setCurveParams();
                    s_pXSail->resetCurves();
                    emit s_pXSail->projectModified();
                }
            }
        }
        else if(m_pBtPolar)
        {
            ObjectTreeItem *pItem = m_pModel->itemFromIndex(index);
            if(pItem)
            {
                Qt::CheckState state = btPolarState(m_pBtPolar);

                if(state==Qt::PartiallyChecked || state==Qt::Unchecked)
                    SailObjects::setBPolarVisible(m_pBtPolar, true);
                else
                    SailObjects::setBPolarVisible(m_pBtPolar, false);

                setCurveParams();
                s_pXSail->resetCurves();
                emit s_pXSail->projectModified();
            }
        }
        else if(m_pBoat)
        {
            ObjectTreeItem *pItem = m_pModel->itemFromIndex(index);
            if(pItem)
            {
                Qt::CheckState state = boatState(m_pBoat);
                if(state==Qt::PartiallyChecked || state==Qt::Unchecked)
                    SailObjects::setBoatVisible(m_pBoat, true);
                else if(state==Qt::Checked)
                    SailObjects::setBoatVisible(m_pBoat, false);

                setCurveParams();
                s_pXSail->resetCurves();
                emit s_pXSail->projectModified();
            }
        }
        setOverallCheckStatus();
    }

    s_pXSail->updateView();
}


void BoatTreeView::onItemDoubleClicked(const QModelIndex &filteredindex)
{
    setObjectFromIndex(filteredindex);

//    s_pXSail->m_pAnalysisControls->setAnalysisParams();
    s_pXSail->updateView();
    if(m_Selection==BoatTreeView::BOAT)
    {
        s_pXSail->onEditCurBoat();
    }
    else if(m_Selection==BoatTreeView::BTPOLAR)
    {
        s_pXSail->onEditCurBtPolar();
    }
}


/**
 * Sets the new current object, a Boat, a WPolar, a BoatOpp or or a Mode
 * from the new index
 */
void BoatTreeView::setObjectFromIndex(QModelIndex index)
{
    ObjectTreeItem *pSelectedItem = nullptr;

    if(index.column()==0)
    {
        pSelectedItem = m_pModel->itemFromIndex(index);
    }
    else if(index.column()>=1)
    {
        QModelIndex ind = index.siblingAtColumn(0);
        pSelectedItem = m_pModel->itemFromIndex(ind);
    }

    if(!pSelectedItem) return;

    m_pStruct->selectionModel()->blockSignals(true);
    if(pSelectedItem->level()==1)
    {
        Boat *m_pBoat = SailObjects::boat(pSelectedItem->name().toStdString());
        s_pXSail->setBoat(m_pBoat);
        s_pXSail->m_pCurBtPolar = nullptr;
        s_pXSail->m_pCurBtOpp = nullptr;
        m_Selection = BoatTreeView::BOAT;
    }
    else if(pSelectedItem->level()==2)
    {
        ObjectTreeItem *pPlaneItem = pSelectedItem->parentItem();
        Boat *m_pBoat         = SailObjects::boat(pPlaneItem->name().toStdString());
        BoatPolar *m_pBtPolar = SailObjects::btPolar(m_pBoat, pSelectedItem->name().toStdString());
        s_pXSail->setBoat(m_pBoat);
        s_pXSail->setBtPolar(m_pBtPolar);
        s_pXSail->m_pCurBtOpp = nullptr;

        m_Selection = BoatTreeView::BTPOLAR;
    }
    else if(pSelectedItem->level()==3)
    {
        ObjectTreeItem *pWPolarItem = pSelectedItem->parentItem();
        ObjectTreeItem *pPlaneItem = pWPolarItem->parentItem();
        Boat      *m_pBoat    = SailObjects::boat(pPlaneItem->name().toStdString());
        BoatPolar *m_pBtPolar = SailObjects::btPolar(m_pBoat, pWPolarItem->name().toStdString());
        BoatOpp   *m_pBtOpp   = SailObjects::getBoatOpp(m_pBoat, m_pBtPolar, pSelectedItem->name().toDouble());
        m_Selection = BoatTreeView::BOATOPP;

        if(m_pBoat!=s_pXSail->m_pCurBoat)
        {
            s_pXSail->setBoat(m_pBoat);
            s_pXSail->setBtPolar(m_pBtPolar);
        }
        else if(m_pBtPolar != s_pXSail->m_pCurBtPolar) s_pXSail->setBtPolar(m_pBtPolar);
        if(m_pBtOpp)
        {
            s_pXSail->setBtOpp(m_pBtOpp);
            s_pXSail->resetCurves();
        }
    }
    else m_Selection = BoatTreeView::NOBOAT;

    s_pXSail->setControls();
    setObjectProperties();
    m_pStruct->selectionModel()->blockSignals(false);
    s_pXSail->setControls();
    setObjectProperties();
    m_pStruct->selectionModel()->blockSignals(false);
}


void BoatTreeView::insertBtPolar(BoatPolar* pBtPolar)
{
    if(!pBtPolar) pBtPolar = s_pXSail->curBtPolar();
    if(!pBtPolar) return;

    // the Plane item is the polar's parent item
    m_pStruct->selectionModel()->blockSignals(true);

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pBoatItem = m_pModel->item(ir);

        // find the polar's parent Plane
        if(pBoatItem->name().toStdString().compare(pBtPolar->boatName())==0)
        {
            ObjectTreeItem *pNewBtPolarItem = nullptr;
            for(int jr=0; jr<pBoatItem->rowCount(); jr++)
            {
                ObjectTreeItem *pOldPolarItem = pBoatItem->child(jr);
                if(pOldPolarItem->name().toStdString().compare(pBtPolar->name())==0)
                {
                    pNewBtPolarItem = pOldPolarItem;
                }
                else if(pOldPolarItem->name().toStdString().compare(pBtPolar->name())>0)
                {
                    //insert before
                    pNewBtPolarItem = m_pModel->insertRow(pBoatItem, jr, pBtPolar->name(), pBtPolar->theStyle(), btPolarState(pBtPolar));
                }
                if(pNewBtPolarItem) break;
            }
            if(!pNewBtPolarItem)
            {
                //append
                pNewBtPolarItem = m_pModel->appendRow(pBoatItem, pBtPolar->name(), pBtPolar->theStyle(), btPolarState(pBtPolar));
            }

            if(pNewBtPolarItem)
            {
                addBtOpps(pBtPolar);

                // set the curve data
                LineStyle ls(pBtPolar->theStyle());
                ls.m_bIsEnabled = s_pXSail->isPolarView();
                pNewBtPolarItem->setTheStyle(ls);
                pNewBtPolarItem->setCheckState(btPolarState(pBtPolar));
            }
            m_pStruct->selectionModel()->blockSignals(false);
            return;
        }
    }

    setOverallCheckStatus();
    m_pStruct->selectionModel()->blockSignals(false);
}


void BoatTreeView::insertBoat(Boat* pBoat)
{
    if(!pBoat) pBoat = s_pXSail->curBoat();
    if(!pBoat) return;

    m_pStruct->selectionModel()->blockSignals(true);

    bool bInserted = false;
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pItem = m_pModel->item(ir);
        // insert alphabetically
        if(pItem->name().toStdString().compare(pBoat->name())==0)
        {
            //A boat of that name already exists
            m_pStruct->selectionModel()->blockSignals(false);
            return;
        }
        else if(pItem->name().toStdString().compare(pBoat->name())>0)
        {
            //insert before
            m_pModel->insertRow(m_pModel->rootItem(), ir, pBoat->name(), pBoat->theStyle(), boatState(pBoat));
            bInserted = true;
            break;
        }
    }
    if(!bInserted)
    {
        //not inserted, append
        m_pModel->appendRow(m_pModel->rootItem(), pBoat->name(), pBoat->theStyle(), boatState(pBoat));
    }

    for(int iwp=0; iwp<SailObjects::nBtPolars(); iwp++)
    {
        BoatPolar *pBtPolar = SailObjects::btPolar(iwp);
        if(pBtPolar->boatName().compare(pBoat->name())==0)
        {
            insertBtPolar(pBtPolar);
        }
    }

    setOverallCheckStatus();
    m_pStruct->selectionModel()->blockSignals(false);
}


void BoatTreeView::removeBtPolarBtOpps(BoatPolar* pBtPolar)
{
    if(!pBtPolar) return;

    m_pStruct->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        // find the polar's parent Plane item
        if(pPlaneItem->name().toStdString().compare(pBtPolar->boatName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pPlaneItem->child(jr);
                if(pPolarItem->name().toStdString().compare(pBtPolar->name())==0)
                {
                    QModelIndex polarindex = m_pModel->index(jr, 0, pPlaneItem);
                    m_pModel->removeRows(0, pPolarItem->rowCount(), polarindex);
                    break;
                }
            }
        }
    }
//    onSetOverallCheckStatus();
    m_pStruct->selectionModel()->blockSignals(false);
}


void BoatTreeView::removeBtOpps(Boat* pBoat)
{
    for(int ip=0; ip<SailObjects::nBtPolars(); ip++)
    {
        BoatPolar *pBtPolar = SailObjects::btPolar(ip);
        if(pBtPolar->boatName()==pBoat->name())
            removeBtPolarBtOpps(pBtPolar);
    }
}


void BoatTreeView::selectBoat(Boat* pBoat)
{
    if(!pBoat) pBoat = s_pXSail->m_pCurBoat;
    if(!pBoat) return;

//    m_pStruct->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        // find the polar's parent Plane
        if(pPlaneItem->name().toStdString().compare(pBoat->name())==0)
        {
            m_Selection = BoatTreeView::BOAT;
            if(m_pModel->index(ir, 0).isValid())
            {
                QModelIndex index = m_pModel->index(ir, 0);
                m_pStruct->setCurrentIndex(index);
                m_pStruct->scrollTo(index);
                if(pPlaneItem->rowCount()>0) m_pStruct->expand(index);
            }

            break;
        }
    }
//    m_pStruct->selectionModel()->blockSignals(false);
}


void BoatTreeView::selectBtPolar(BoatPolar* pBtPolar)
{
    //qDebug("Select WPolar");
    if(!pBtPolar) pBtPolar = s_pXSail->curBtPolar();
    if(!pBtPolar) return;

    bool bSelected=false;

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem const *pBoatItem = m_pModel->item(ir);

        QModelIndex planeindex = m_pModel->index(ir, 0);
        Q_ASSERT(planeindex.isValid());
        // find the polar's parent Plane
        if(pBoatItem->name().toStdString().compare(pBtPolar->boatName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pBoatItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pBoatItem->child(jr);
                if(pPolarItem->name().toStdString().compare(pBtPolar->name())==0)
                {
                    m_Selection = BoatTreeView::BTPOLAR;
                    QModelIndex polarindex = m_pModel->index(jr, 0, planeindex);
                    if(polarindex.isValid())
                    {
                        m_pStruct->setCurrentIndex(polarindex);
                        m_pStruct->selectionModel()->select(polarindex, QItemSelectionModel::Rows);
                        m_pStruct->scrollTo(polarindex);

                        bSelected = true;
                    }
                    break;
                }
            }
        }
        if(bSelected) break;
    }

    setObjectProperties();
    update();
}


void BoatTreeView::selectBtOpp(BoatOpp *pBtOpp)
{
//qDebug("Select BoatOpp");
    if(!pBtOpp) pBtOpp = s_pXSail->m_pCurBtOpp;
    if(!pBtOpp) return;

    bool bSelected = false;

    //    m_pStruct->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pBoatItem = m_pModel->item(ir);
        // find the polar's parent Plane
        if(pBoatItem->name().toStdString().compare(pBtOpp->boatName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pBoatItem->rowCount(); jr++)
            {
                //                const QModelIndex &oldWPolarChild = pPlaneItem->index().child(jr, 0);
                //                PlaneTreeItem *pPolarItem = m_pModel->itemFromIndex(oldWPolarChild);
                ObjectTreeItem *pPolarItem = pBoatItem->child(jr);

                if(pPolarItem->name().toStdString().compare(pBtOpp->polarName())==0)
                {
                    //find the POpp item
                    for(int jr=0; jr<pPolarItem->rowCount(); jr++)
                    {
                        ObjectTreeItem *pPOppItem = pPolarItem->child(jr);
                        QModelIndex poppChild = m_pModel->index(jr,0, pPolarItem);

                        bool bOK=false;
                        QString strange = pPOppItem->name().trimmed();
                        double val = strange.toDouble(&bOK);

                        if(bOK && poppChild.isValid())
                        {
                            bSelected = fabs(val-pBtOpp->ctrl())<0.0005;
                            if(bSelected)
                            {
                                m_Selection = BoatTreeView::BOATOPP;
                                m_pStruct->setCurrentIndex(poppChild);
                                m_pStruct->scrollTo(poppChild);
                                break;
                            }
                        }
                    }
                }
                if(bSelected) break;
            }
        }
        if(bSelected) break;
    }
    setObjectProperties();
}


/**
 * Removes the Boat defined by the pointer and returns the name
 * of the previous Boat in the list, or of the next Boat if none
 * @param pBoat a pointer to the Boat object to be removed
 * @return the name of the next Boat to select
 */
QString BoatTreeView::removeBoat(Boat *pBoat)
{
    if(!pBoat) return "";
    return removeBoat(QString::fromStdString(pBoat->name()));
}


/**
 * Removes the Boat defined by the name and returns the name
 * of the previous Boat in the list, or of the next Boat if none
 * @param BoatName the name of the Boat object to be removed
 * @return the name of the next Boat to select
 */
QString BoatTreeView::removeBoat(QString const &BoatName)
{
    if(!BoatName.length()) return "";

    m_pStruct->selectionModel()->blockSignals(true);

    int irow = 0;
    for(irow=0; irow<m_pModel->rowCount(); irow++)
    {
        ObjectTreeItem const *pItem = m_pModel->item(irow);
        // scan
        if(pItem->name().compare(BoatName)==0)
        {
            // plane found
            m_pModel->removeRow(irow);
            break;
        }
    }

    setOverallCheckStatus();
    m_pStruct->selectionModel()->blockSignals(false);

    if(irow+1<SailObjects::nBoats())
        return QString::fromStdString(SailObjects::boat(irow+1)->name());
    else if(irow-1>=0)
        return QString::fromStdString(SailObjects::boat(irow-1)->name());

    return QString();
}


QString BoatTreeView::removeBtPolar(BoatPolar const* pBtPolar)
{
    if(!pBtPolar) return QString();

    m_pStruct->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        QModelIndex planeindex = m_pModel->index(ir, 0);
        // find the polar's parent Plane
        if(pPlaneItem->name().toStdString().compare(pBtPolar->boatName())==0)
        {
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pOldPolarItem = pPlaneItem->child(jr);

                if(pOldPolarItem)
                {
                    if(pOldPolarItem->name().toStdString().compare(pBtPolar->name())==0)
                    {
                        m_pModel->removeRow(jr, planeindex);
                        m_pStruct->selectionModel()->blockSignals(false);

                        // find the previous item, or the next one if this polar is the first
                        if(pPlaneItem->rowCount())
                        {
                            jr =std::min(jr, pPlaneItem->rowCount()-1);
                            return pPlaneItem->child(jr)->name();
                        }
                        return "";
                    }
                }
            }
        }
    }

    setOverallCheckStatus();
    m_pStruct->selectionModel()->blockSignals(false);
    return QString(); /** @todo need to do better than that */
}


void BoatTreeView::removeBoatOpp(BoatOpp *pBtOpp)
{
    if(!pBtOpp) return;

    m_pStruct->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pBoatItem = m_pModel->item(ir);
        Boat *m_pBoat = SailObjects::boat(pBoatItem->name().toStdString());
        // find the polar's parent Plane
        if(pBoatItem->name().toStdString().compare(pBtOpp->boatName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pBoatItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pBoatItem->child(jr);
                if(pPolarItem->name().toStdString().compare(pBtOpp->polarName())==0)
                {
                    QModelIndex polarindex = m_pModel->index(jr, 0, pBoatItem);

                    BoatPolar const *pWPolar = SailObjects::btPolar(m_pBoat, pPolarItem->name().toStdString());
                    if(!pWPolar) continue;

                    //find the POpp item
                    for(int jr=0; jr<pPolarItem->rowCount(); jr++)
                    {
                        ObjectTreeItem *poppItem = pPolarItem->child(jr);
                        double val = poppItem->name().toDouble();

                        if(fabs(val-pBtOpp->ctrl())<0.0005)
                        {
                            m_pModel->removeRow(jr,polarindex);
                            break;
                        }

                        m_pStruct->update();
                    }
                }
            }
        }
    }

//    onSetOverallCheckStatus();
    m_pStruct->selectionModel()->blockSignals(false);
}


void BoatTreeView::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QModelIndex idx = m_pStruct->currentIndex();

    ObjectTreeItem *pItem = m_pModel->itemFromIndex(idx);
//    onActivated(idx);

    QString strong;

    if(!pItem) return;

    if(pItem->level()==1)
    {
        // no parent, we have a plane
        Boat *m_pBoat = SailObjects::boat(pItem->name().toStdString());
        if(m_pBoat) strong  = QString::fromStdString(m_pBoat->name());
        else        strong.clear();
    }
    else if(pItem->level()==2)
    {
        //we have a WPolar;
        ObjectTreeItem *pParent = pItem->parentItem();
        Boat *m_pBoat = SailObjects::boat(pParent->name().toStdString());
        BoatPolar *m_pBtPolar = SailObjects::btPolar(m_pBoat, pItem->name().toStdString());
        if(m_pBtPolar) strong = QString::fromStdString(m_pBtPolar->name());
        else           strong.clear();
    }
    else if(pItem->level()==3)
    {
        //we have a POpp selected;
        ObjectTreeItem *pParent = pItem->parentItem();
        ObjectTreeItem *pParent2 = pParent->parentItem();
        Boat *pBoat  = SailObjects::boat(pParent2->name().toStdString());
        BoatPolar *pBtPolar = SailObjects::btPolar(pBoat, pParent->name().toStdString());
        BoatOpp *pBtOpp   = SailObjects::getBoatOpp(pBoat, pBtPolar, pItem->name().toDouble());
        if(pBtOpp) strong = pItem->name();
        else       strong.clear();
    }

    if     (m_Selection==BoatTreeView::BOATOPP)
        s_pXSail->m_pMenus->m_pCurBtOppMenu->exec(pEvent->globalPos());
    else if(m_Selection==BoatTreeView::BTPOLAR)
        s_pXSail->m_pMenus->m_pCurBtPlrMenu->exec(pEvent->globalPos());
    else if(m_Selection==BoatTreeView::BOAT)
        s_pXSail->m_pMenus->m_pCurBoatMenu->exec(pEvent->globalPos());

    pEvent->accept();
}


void BoatTreeView::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Delete:
        {
            if     (m_Selection==BoatTreeView::BOATOPP) s_pXSail->onDeleteCurBtOpp();
            else if(m_Selection==BoatTreeView::BTPOLAR) s_pXSail->onDeleteCurBtPolar();
            else if(m_Selection==BoatTreeView::BOAT)    s_pXSail->onDeleteCurBoat();

            pEvent->accept();
            return;
        }
        default:
            s_pXSail->keyPressEvent(pEvent);;
    }
//    if(!pEvent->isAccepted()) s_pXSail->keyPressEvent(pEvent);
}


void BoatTreeView::selectCurrentObject()
{
//    qDebug("selectCurrentObject");
    if(s_pXSail->isPolarView())
    {
        if(s_pXSail->m_pCurBtPolar)     selectBtPolar(s_pXSail->m_pCurBtPolar);
        else if(s_pXSail->m_pCurBoat)   selectBoat(s_pXSail->m_pCurBoat);
    }
    else if (s_pXSail->is3dView())
    {
        selectBoat(s_pXSail->m_pCurBoat);
    }
}


void BoatTreeView::selectObjects()
{
//    qDebug("Select objects");
    m_pStruct->selectionModel()->blockSignals(true);
    if(s_pXSail->curBtOpp())        selectBtOpp();
    else if(s_pXSail->curBtPolar()) selectBtPolar(s_pXSail->curBtPolar());
    else                            selectBoat(s_pXSail->curBoat());
    setObjectProperties();
    m_pStruct->selectionModel()->blockSignals(false);
}


/** update the line properties for each polar and popp item in the treeview */
void BoatTreeView::setCurveParams()
{
    ObjectTreeItem *pRootItem = m_pModel->rootItem();
    for(int i0=0; i0<pRootItem->rowCount(); i0++)
    {
        QModelIndex planeindex = m_pModel->index(i0,0);
        ObjectTreeItem *pBoatItem = m_pModel->itemFromIndex(planeindex);
        Boat const *pBoat = nullptr;
        if(!pBoatItem) return;
        else           pBoat = SailObjects::boat(pBoatItem->name().toStdString());
        if(pBoat && pBoatItem)
        {
            pBoatItem->setTheStyle(pBoat->theStyle());
            pBoatItem->setCheckState(boatState(pBoat ));

            for(int i1=0; i1< pBoatItem->rowCount(); i1++)
            {
                if(pBoatItem->child(i1))
                {
                    ObjectTreeItem *pWPolarItem = pBoatItem->child(i1);
                    BoatPolar const *pWPolar = nullptr;
                    if(pWPolarItem) pWPolar = SailObjects::btPolar(pBoat, pWPolarItem->name().toStdString());
                    else            pWPolar = nullptr;

                    if(pWPolar)
                    {
                        pWPolarItem->setTheStyle(pWPolar->theStyle());

                        if(s_pXSail->isPolarView())
                        {
                            bool bCheck = pWPolar->isVisible();
                            bCheck = bCheck && (s_pXSail->isPolarView());
                            pWPolarItem->setCheckState(bCheck ? Qt::Checked : Qt::Unchecked);
                        }
                        else
                        {
                            pWPolarItem->setCheckState(btPolarState(pWPolar));
                        }
                    }
                }
            }
        }
    }
    m_pModel->updateData();
}


Qt::CheckState BoatTreeView::boatState(Boat const *pBoat) const
{
    bool bAll = true;
    bool bNone = true;
    if(s_pXSail->isPolarView())
    {
        for(int iplr=0; iplr<SailObjects::nBtPolars(); iplr++)
        {
            BoatPolar* pWPolar = SailObjects::btPolar(iplr);
            if(pBoat->hasBtPolar(pWPolar))
            {
                bAll = bAll && pWPolar->isVisible();
                bNone = bNone && !pWPolar->isVisible();
            }
        }
    }
    else
    {
        for(int iopp=0; iopp<SailObjects::nBtOpps(); iopp++)
        {
            BoatOpp *pOpp = SailObjects::btOpp(iopp);
            if(pBoat->hasBOpp(pOpp))
            {
                bAll = bAll && pOpp->isVisible();
                bNone = bNone && !pOpp->isVisible();
            }
        }
    }
    if(bNone)      return Qt::Unchecked;
    else if(bAll)  return Qt::Checked;
    else           return Qt::PartiallyChecked;
}


Qt::CheckState BoatTreeView::btPolarState(BoatPolar const* pWPolar) const
{
    if(s_pXSail->is3dView())
    {
        bool bAll = true;
        bool bNone = true;
        for(int iopp=0; iopp<SailObjects::nBtOpps(); iopp++)
        {
            BoatOpp *pBtOpp = SailObjects::btOpp(iopp);
            if(pWPolar->hasBtOpp(pBtOpp))
            {
                bAll = bAll && pBtOpp->isVisible();
                bNone = bNone && !pBtOpp->isVisible();
            }
        }
        if     (bNone) return Qt::Unchecked;
        else if(bAll)  return Qt::Checked;
        else           return Qt::PartiallyChecked;
    }
    else if(s_pXSail->isPolarView())
    {
        return pWPolar->isVisible() ? Qt::Checked : Qt::Unchecked;
    }
    return Qt::Unchecked;
}


void BoatTreeView::onSwitchAll(bool bChecked)
{
    if(s_pXSail->isPolarView())
    {
        if(bChecked) s_pXSail->onShowAllBtPolars();
        else         s_pXSail->onHideAllBtPolars();
    }
    m_pModel->updateData();
}


void BoatTreeView::setOverallCheckStatus()
{
    if(s_pXSail->isPolarView())
    {
        m_pStruct->enableSelectBox(true);
        bool bAllChecked   = true;
        bool bAllUnchecked = true;
        for(int io=0; io<SailObjects::nBtPolars(); io++)
        {
            BoatPolar const *pWPolar = SailObjects::btPolar(io);
            if(pWPolar->isVisible()) bAllUnchecked = false;
            else                     bAllChecked   = false;
        }

        if     (bAllChecked)   m_pStruct->setOverallCheckedState(Qt::Checked);
        else if(bAllUnchecked) m_pStruct->setOverallCheckedState(Qt::Unchecked);
        else                   m_pStruct->setOverallCheckedState(Qt::PartiallyChecked);
    }
    else
    {
        m_pStruct->enableSelectBox(false);
        m_pStruct->setOverallCheckedState(Qt::Unchecked);
    }
}


void BoatTreeView::onSetFilter()
{
    QString filter = m_pStruct->filter();
    QStringList filters = filter.split(QRegularExpression("\\s+"));


    if(filters.size()==0)
    {
        for(int jp=0; jp<SailObjects::nBtPolars(); jp++)
        {
            SailObjects::btPolar(jp)->setVisible(true);
        }
    }
    else if(filters.size()==1)
    {
        for(int jp=0; jp<SailObjects::nBtPolars(); jp++)
        {
            BoatPolar *pWPolar = SailObjects::btPolar(jp);
            bool bVisible = QString::fromStdString(pWPolar->name()).contains(filter);
            pWPolar->setVisible(bVisible);
        }

        for(int ip=0; ip<SailObjects::nBoats(); ip++)
        {
            Boat const *pBoat = SailObjects::boat(ip);
            if(QString::fromStdString(pBoat->name()).contains(filter))
            {
                for(int jp=0; jp<SailObjects::nBtPolars(); jp++)
                {
                    BoatPolar *pBtPolar = SailObjects::btPolar(jp);
                    if(pBtPolar->boatName()==pBoat->name())
                        pBtPolar->setVisible(true);
                }
            }
        }
    }
    else
    {
        QString boatfilter = filters.front();
        QString polarfilter = filters.at(1);
        for(int jp=0; jp<SailObjects::nBtPolars(); jp++)
        {
            SailObjects::btPolar(jp)->setVisible(false);
        }

        for(int ip=0; ip<SailObjects::nBoats(); ip++)
        {
            Boat const *pBoat = SailObjects::boat(ip);
            if(QString::fromStdString(pBoat->name()).contains(boatfilter))
            {
                for(int jp=0; jp<SailObjects::nBtPolars(); jp++)
                {
                    BoatPolar *pBtPolar = SailObjects::btPolar(jp);
                    if(pBtPolar->boatName()==pBoat->name())
                    {
                        bool bVisible = QString::fromStdString(pBtPolar->name()).contains(polarfilter);
                        pBtPolar->setVisible(bVisible);
                    }
                }
            }
        }
    }

    setCurveParams();

    m_pModel->updateData();
    setOverallCheckStatus();

    s_pXSail->resetCurves();
    s_pXSail->updateView();

    update();
    emit s_pXSail->projectModified();

}


void BoatTreeView::updateVisibilityBoxes()
{
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pBoatItem = m_pModel->item(ir);
        Boat *pBoat = SailObjects::boat(pBoatItem->name().toStdString());
        if(!pBoat) continue;

        QModelIndex checkindex = m_pModel->index(ir, 2);
        m_pModel->setData(checkindex, boatState(pBoat));

        for(int jr=0; jr<pBoatItem->rowCount(); jr++)
        {
            ObjectTreeItem *pWPolarItem = pBoatItem->child(jr);
            BoatPolar *pBtPolar = SailObjects::btPolar(pBoat, pWPolarItem->name().toStdString());
            if(!pBtPolar) continue;

            QModelIndex checkindex = m_pModel->index(jr, 2, pBoatItem);

            m_pModel->setData(checkindex, btPolarState(pBtPolar));
        }
    }
}





