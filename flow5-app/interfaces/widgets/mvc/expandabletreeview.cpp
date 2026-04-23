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

#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDebug>

#include "expandabletreeview.h"
#include <interfaces/widgets/customwts/crosscheckbox.h>
#include <interfaces/widgets/mvc/objecttreeitem.h>
#include <interfaces/widgets/mvc/objecttreemodel.h>
#include <core/displayoptions.h>


ExpandableTreeView::ExpandableTreeView(QWidget *pParent) : QTreeView(pParent)
{
    initETV();

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setUniformRowHeights(true);
    setRootIsDecorated(true);
#ifdef Q_OS_WIN
    setStyleSheet("QTreeView::item { border: none;}");
#endif
    header()->hide();
    header()->setStretchLastSection(false);
}


void ExpandableTreeView::initETV()
{
    m_pLevelMinus   = new QAction(QIcon(":/icons/level-.png"),  "Collapse selected item",        this);
    m_pLevelPlus    = new QAction(QIcon(":/icons/level+.png"),  "Expand selected item",          this);
    m_pLevel0Action = new QAction(QIcon(":/icons/level0.png"),  "Object level\tAlt+0",           this);
    m_pLevel0Action->setShortcut(QKeySequence(Qt::ALT | Qt::Key_0));
    m_pLevel1Action = new QAction(QIcon(":/icons/level1.png"),  "Polar level\tAlt+1",            this);
    m_pLevel1Action->setShortcut(QKeySequence(Qt::ALT | Qt::Key_1));
    m_pLevel2Action = new QAction(QIcon(":/icons/level2.png"),  "Operating point level\tAlt+2",  this);
    m_pLevel2Action->setShortcut(QKeySequence(Qt::ALT | Qt::Key_2));

    m_pfrControls = new QFrame;
    {
        int av = DisplayOptions::treeFontStruct().averageCharWidth();
        m_pleFilter = new QLineEdit;
        m_pleFilter->setClearButtonEnabled(true);
        m_pleFilter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        m_pleFilter->setToolTip("<p>Enter here the words to be used as filters for objects and polars and press Enter. "
                                "This will apply a filter to the polar graphs.<br>"
                                "If only one word is entered it will be used to filter objects OR polars.<br>"
                                "If two words are entered, e.g. \'planefilter T2\' the first word will be "
                                "used to filter planes AND the second will be used to filter polars.<br>"
                                "The filter is case-insensitive.</p");
        m_pchHideShowAll = new CrossCheckBox;
        m_pchHideShowAll->setWidthHint(3*av);
        m_pchHideShowAll->setToolTip("Hide or show all polars or operating point objects in the graphs.");
        QHBoxLayout *pHLayout = new QHBoxLayout;
        {
            QToolButton *pLevelPlus   = new QToolButton(this);
            QToolButton *pLevelMinus  = new QToolButton(this);
            QToolButton *pLevel0      = new QToolButton(this);
            QToolButton *pLevel1      = new QToolButton(this);
            QToolButton *pLevel2      = new QToolButton(this);

            pLevelPlus->setDefaultAction(m_pLevelPlus);
            pLevelMinus->setDefaultAction(m_pLevelMinus);
            pLevel0->setDefaultAction(m_pLevel0Action);
            pLevel1->setDefaultAction(m_pLevel1Action);
            pLevel2->setDefaultAction(m_pLevel2Action);

            pHLayout->addSpacing(3*av);
            pHLayout->addWidget(pLevel0);
            pHLayout->addWidget(pLevel1);
            pHLayout->addWidget(pLevel2);
            pHLayout->addSpacing(3*av);

            pHLayout->addWidget(pLevelMinus);
            pHLayout->addWidget(pLevelPlus);
            pHLayout->addSpacing(3*av);

            pHLayout->addWidget(m_pleFilter);
            pHLayout->addSpacing(av);
            pHLayout->addWidget(m_pchHideShowAll);
        }
        m_pfrControls->setLayout(pHLayout);
    }

    connect(m_pLevel0Action,  SIGNAL(triggered(bool)), SLOT(onObjectLevel()));
    connect(m_pLevel1Action,  SIGNAL(triggered(bool)), SLOT(onPolarLevel()));
    connect(m_pLevel2Action,  SIGNAL(triggered(bool)), SLOT(onOpPointLevel()));
    connect(m_pLevelMinus,    SIGNAL(triggered(bool)), SLOT(onLevelMinus()));
    connect(m_pLevelPlus,     SIGNAL(triggered(bool)), SLOT(onLevelPlus()));

    connect(m_pchHideShowAll, SIGNAL(clicked(bool)),   SLOT(onHideShowAll(bool)));
}


/**
 * Collapse all items up to plane/foil level
 */
void ExpandableTreeView::onObjectLevel()
{
    QModelIndex currentIndex = this->currentIndex();
    ObjectTreeModel *pModel = dynamic_cast<ObjectTreeModel*>(model());

    ObjectTreeItem *pCurItem = pModel->itemFromIndex(currentIndex);
    ObjectTreeItem *pRootItem = pModel->rootItem();

    if(pCurItem)
    {
        while (pCurItem->level()>1)
        {
            pCurItem=pCurItem->parentItem();
        }
    }

    collapseAll();

    if(pCurItem)
    {
        QModelIndex newIndex = pModel->index(pRootItem, pCurItem);
        setCurrentIndex(newIndex);
        scrollTo(newIndex);
    }
}


void ExpandableTreeView::onPolarLevel()
{
    QModelIndex currentIndex = this->currentIndex();
    ObjectTreeModel *pModel = dynamic_cast<ObjectTreeModel*>(model());

    ObjectTreeItem *pCurItem = pModel->itemFromIndex(currentIndex);
    int itemDepth=0;
    if(pCurItem)
    {
        while (pCurItem->level()>1)
        {
            itemDepth++;
            pCurItem=pCurItem->parentItem();
        }
    }

    //collapse and expand all level 1
    collapseAll();
    ObjectTreeItem *pRootItem = pModel->rootItem();
    for(int i0=0; i0<pRootItem->rowCount(); i0++)
    {
        QModelIndex foilindex = pModel->index(i0,0, pRootItem);
        expand(foilindex);
    }


    if(pCurItem)
    {
        QModelIndex ind = pModel->index(0,0,pRootItem);
        expand(ind);
    }

    // select the appropriate index
    // there is no function to tell us the current selection's depth nor if it is hidden unfortunately
    QModelIndex newIndex;
    if(itemDepth==2)
    {
        newIndex = currentIndex.parent();
    }
    else
    {
        newIndex = currentIndex;
    }
    setCurrentIndex(newIndex);
    scrollTo(newIndex);
}


void ExpandableTreeView::onOpPointLevel()
{
    QModelIndex currentIndex = this->currentIndex();

    ObjectTreeModel *pModel = dynamic_cast<ObjectTreeModel*>(model());
    ObjectTreeItem *pCurItem = pModel->itemFromIndex(currentIndex);

    //expand all levels
    ObjectTreeItem *pRootItem = pModel->rootItem();
    for(int i0=0; i0<pRootItem->rowCount(); i0++)
    {
        QModelIndex objectindex = pModel->index(i0,0);
        expand(objectindex);
        ObjectTreeItem *pObjectItem = pModel->itemFromIndex(objectindex);
        for(int i1=0; i1< pObjectItem->rowCount(); i1++)
        {
            QModelIndex polarindex = pModel->index(i1,0,pObjectItem);
            if(polarindex.isValid())
            {
                expand(polarindex);
            }
        }
    }

    if(pCurItem)
    {
        QModelIndex ind = pModel->index(0,0,pCurItem);
        setCurrentIndex(ind);
        scrollTo(ind);
    }
}


void ExpandableTreeView::onLevelPlus()
{
    QModelIndex curidx = currentIndex();
    if(curidx.isValid())
    {
        if(!isExpanded(curidx)) expand(curidx);
        else
        {
            ObjectTreeModel *pModel = dynamic_cast<ObjectTreeModel*>(model());

            ObjectTreeItem *pItem = pModel->itemFromIndex(curidx);
            for(int iRow=0; iRow<pItem->rowCount(); iRow++)
            {
                QModelIndex idx = pModel->index(iRow, 0, pItem);

                if(idx.isValid()) expand(idx);
                else
                {
                    qDebug()<<"invvvalllid index";
                }
            }
        }
    }
}


void ExpandableTreeView::onLevelMinus()
{
    QModelIndex currentIndex = this->currentIndex();

    ObjectTreeModel *pModel = dynamic_cast<ObjectTreeModel*>(model());

    ObjectTreeItem *pCurItem = pModel->itemFromIndex(currentIndex);
    // find the top level item
    int itemDepth=0;
    if(!pCurItem) return;
    while (pCurItem->level()>1)
    {
        itemDepth++;
        pCurItem=pCurItem->parentItem();
    }
    //find the depth of expansion relative to the current item
    int expandeddepth =1;
    for(int iRow=0; iRow<pCurItem->rowCount(); iRow++)
    {
        ObjectTreeItem *pChildItem = pCurItem->child(iRow);
        QModelIndex childIndex = pModel->index(iRow, 0, pCurItem);
        if(pChildItem && pChildItem->rowCount()>0 && isExpanded(childIndex))
        {
            expandeddepth = 2;
        }
    }

    if(expandeddepth==2)
    {
        for(int iRow=0; iRow<pCurItem->rowCount(); iRow++)
        {
            QModelIndex childIndex = pModel->index(iRow, 0, pCurItem);
            collapse(childIndex);
        }
        expandeddepth--;
    }
    else if(expandeddepth==1)
    {
        QModelIndex index = pModel->index(pModel->rootItem(), pCurItem);
        collapse(index);
        expandeddepth--;
    }

    // select the appropriate index
    // there is no function to tell us the current selection's depth nor if it is hidden unfortunately
    QModelIndex newIndex;
    if(itemDepth==2)
    {
        if(expandeddepth==1) newIndex = currentIndex.parent();
        else                 newIndex = currentIndex.parent().parent();
    }
    else if (itemDepth==1)
    {
        if      (expandeddepth==1) newIndex = currentIndex;
        else if (expandeddepth==0) newIndex = currentIndex.parent();
    }
    else
    {
        newIndex = currentIndex;
    }
    setCurrentIndex(newIndex);
    scrollTo(newIndex);
}


void ExpandableTreeView::setOverallCheckedState(Qt::CheckState state)
{
    m_pchHideShowAll->setCheckState(state);
}


void ExpandableTreeView::onHideShowAll(bool bChecked)
{
    if(m_pchHideShowAll->checkState()==Qt::PartiallyChecked)
        m_pchHideShowAll->setCheckState(Qt::Checked); // prevent the partially checked state when clicking

    m_pleFilter->clear();
    emit switchAll(bChecked);
}


void ExpandableTreeView::enableSelectBox(bool bEnable)
{
    if(m_pchHideShowAll) m_pchHideShowAll->setEnabled(bEnable);
}


QSize ExpandableTreeView::sizeHint() const
{
    int w = 19 * DisplayOptions::treeFontStruct().averageCharWidth();
    int h = DisplayOptions::treeFontStruct().height();
    return QSize(w, 5*h);
}


/** "If you reimplement this function in a subclass, note that the value you return
 *   is only used when resizeColumnToContents() is called"*/
int ExpandableTreeView::sizeHintForColumn(int column) const
{
    if      (column==0) return 1*DisplayOptions::treeFontStruct().averageCharWidth();//unused, this is the stretched column
    else if (column==1) return 9*DisplayOptions::treeFontStruct().averageCharWidth();
    else if (column==2) return 2*DisplayOptions::treeFontStruct().averageCharWidth();
    return 5*DisplayOptions::treeFontStruct().averageCharWidth();
}

