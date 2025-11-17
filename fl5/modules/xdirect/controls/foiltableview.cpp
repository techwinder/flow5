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





#include <QHeaderView>


#include "foiltableview.h"

#include <api/foil.h>
#include <api/objects2d.h>
#include <api/utils.h>

#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/widgets/line/linemenu.h>
#include <modules/xdirect/controls/foiltabledelegate.h>
#include <modules/xdirect/menus/xdirectactions.h>
#include <modules/xdirect/view2d/dfoilwt.h>
#include <modules/xdirect/xdirect.h>


XDirect *FoilTableView::s_pXDirect=nullptr;
int FoilTableView::s_Height = 351;

FoilTableView::FoilTableView(QWidget *pParent) : CPTableView(pParent)
{
    makeFoilTable();
    connectSignals();
}


FoilTableView::FoilTableView()
{
    if(m_pFoilModel) delete m_pFoilModel;
    m_pFoilModel = nullptr;
}


void FoilTableView::makeFoilTable()
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditable(false);

    horizontalHeader()->setStretchLastSection(true);

    m_pFoilModel = new QStandardItemModel(this);
    m_pFoilModel->setRowCount(10);//temporary
    m_pFoilModel->setColumnCount(12);

    m_pFoilModel->setHeaderData(0,  Qt::Horizontal, "Name");
    m_pFoilModel->setHeaderData(1,  Qt::Horizontal, "Thickness (%)");
    m_pFoilModel->setHeaderData(2,  Qt::Horizontal, "at (%)");
    m_pFoilModel->setHeaderData(3,  Qt::Horizontal, "Camber (%)");
    m_pFoilModel->setHeaderData(4,  Qt::Horizontal, "at (%)");
    m_pFoilModel->setHeaderData(5,  Qt::Horizontal, "Points");
    m_pFoilModel->setHeaderData(6,  Qt::Horizontal, "TE Flap ("+DEGch+")");
    m_pFoilModel->setHeaderData(7,  Qt::Horizontal, "TE XHinge (%)");
    m_pFoilModel->setHeaderData(8,  Qt::Horizontal, "TE YHinge (%)");
    m_pFoilModel->setHeaderData(9,  Qt::Horizontal, "Show");
    m_pFoilModel->setHeaderData(10, Qt::Horizontal, "Camber line");
    m_pFoilModel->setHeaderData(11, Qt::Horizontal, "Style");
    setModel(m_pFoilModel);
    setWindowTitle("Foils");

    m_pFoilDelegate = new FoilTableDelegate(this);
    setItemDelegate(m_pFoilDelegate);

    hideColumn(6);
}


void FoilTableView::connectSignals()
{
    connect(this, SIGNAL(pressed(QModelIndex)), SLOT(onItemClicked(QModelIndex)));
    //    connect(m_pStruct, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onItemDoubleClicked(QModelIndex)));
    connect(this->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentRowChanged(QModelIndex,QModelIndex)));
}


void FoilTableView::updateTable()
{
    m_pFoilModel->setRowCount(Objects2d::nFoils());
    fillFoilTable();
    selectFoil(s_pXDirect->curFoil());
}


void FoilTableView::selectFoil(Foil *pFoil)
{
    if(!pFoil)
    {
        setCurrentIndex(QModelIndex());
        return;
    }

    for(int row=0; row<m_pFoilModel->rowCount(); row++)
    {
        QStandardItem *pItem = m_pFoilModel->item(row,0);
        if(!pItem) return;
        QString foilname = pItem->text();
        if(foilname==QString::fromStdString(pFoil->name()))
        {
            setCurrentIndex(m_pFoilModel->index(row,0));
            break;
        }
    }
}


void FoilTableView::fillFoilTable()
{
    // Because the QString default compare method is case-sensitive,
    // we need to make a re-ordered case-insensitive list of foils
    std::vector<Foil *> foils = Objects2d::sortedFoils();

    m_pFoilModel->setRowCount(Objects2d::nFoils());
    for(uint row=0; row<foils.size(); row++)
    {
        QModelIndex ind;
        Foil const*pFoil = foils.at(row);
        if(pFoil)
        {
            ind = m_pFoilModel->index(row, 0, QModelIndex());
            m_pFoilModel->setData(ind, QString::fromStdString(pFoil->name()));

            if(pFoil->description().length()) m_pFoilModel->setData(ind, QString::fromStdString(pFoil->description()), Qt::ToolTipRole);

            ind = m_pFoilModel->index(row, 1, QModelIndex());
            m_pFoilModel->setData(ind, pFoil->maxThickness()*100.0);

            ind = m_pFoilModel->index(row, 2, QModelIndex());
            m_pFoilModel->setData(ind, pFoil->xThickness()*100.0);

            ind = m_pFoilModel->index(row, 3, QModelIndex());
            m_pFoilModel->setData(ind, pFoil->maxCamber()*100.0);

            ind = m_pFoilModel->index(row, 4, QModelIndex());
            m_pFoilModel->setData(ind, pFoil->xCamber()*100.0);

            ind = m_pFoilModel->index(row, 5, QModelIndex());
            m_pFoilModel->setData(ind, pFoil->nNodes());

            if(pFoil->hasTEFlap())
            {
                ind = m_pFoilModel->index(row, 6, QModelIndex());
                m_pFoilModel->setData(ind, pFoil->TEFlapAngle());

                ind = m_pFoilModel->index(row, 7, QModelIndex());
                m_pFoilModel->setData(ind, pFoil->TEXHinge()*100.0);

                ind = m_pFoilModel->index(row, 8, QModelIndex());
                m_pFoilModel->setData(ind, pFoil->TEYHinge()*100.0);
            }
        }
    }
}


void FoilTableView::resizeColumns()
{
    int w = width();
    int w12 = int(double(w)*0.9/13);

    setColumnWidth(0,w12*4);
    setColumnWidth(1,w12);
    setColumnWidth(2,w12);
    setColumnWidth(3,w12);
    setColumnWidth(4,w12);
    setColumnWidth(5,w12);//points

    setColumnWidth(6,w12);   //TE Flap
    setColumnWidth(7,w12);   //TE XHinge
    setColumnWidth(8,w12);   //TE YHinge
    setColumnWidth(9,w12);   //Show
    setColumnWidth(10,w12);  //Camberline
}


void FoilTableView::hideEvent(QHideEvent *pEvent)
{
    s_Height = height();
    CPTableView::hideEvent(pEvent);
}


void FoilTableView::showEvent(QShowEvent *pEvent)
{
    fillFoilTable();
    resizeColumns();
    CPTableView::showEvent(pEvent);
}


void FoilTableView::resizeEvent(QResizeEvent *pEvent)
{
    resizeColumns();
    CPTableView::resizeEvent(pEvent);
}


void FoilTableView::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QModelIndex index = currentIndex();
    Foil *pFoil = setObjectFromIndex(index);
    if(!pFoil)
    {
        CPTableView::contextMenuEvent(pEvent);
        return;
    }

    QAction *pCopyAction = new QAction("Copy", this);
    pCopyAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_C));

    QAction *pPasteAction = new QAction("Paste", this);
    pPasteAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_V));
    pPasteAction->setEnabled(m_bIsEditable);

    connect(pCopyAction,  SIGNAL(triggered(bool)), this, SLOT(onCopySelection()));
    connect(pPasteAction, SIGNAL(triggered(bool)), this, SLOT(onPaste()));

    QMenu *pFoilTableMenu = new QMenu("context menu");
    {
        pFoilTableMenu->addAction(s_pXDirect->m_pActions->m_pGetFoilProps);
        pFoilTableMenu->addSeparator();
        pFoilTableMenu->addAction(s_pXDirect->m_pActions->m_pRenameCurFoil);
        pFoilTableMenu->addAction(s_pXDirect->m_pActions->m_pFoilDescription);
        pFoilTableMenu->addAction(s_pXDirect->m_pActions->m_pDeleteCurFoil);
        pFoilTableMenu->addAction(s_pXDirect->m_pActions->m_pDuplicateCurFoil);

        pFoilTableMenu->addSeparator();
        QMenu *pModifyMenu = pFoilTableMenu->addMenu("Modify");
        {
//            pModifyMenu->addAction(s_pXDirect->m_pActions->m_pNormalizeFoil);
            pModifyMenu->addAction(s_pXDirect->m_pActions->m_pDerotateFoil);
            pModifyMenu->addAction(s_pXDirect->m_pActions->m_pRefineGlobalFoil);
            pModifyMenu->addAction(s_pXDirect->m_pActions->m_pEditCoordsFoil);
            pModifyMenu->addAction(s_pXDirect->m_pActions->m_pScaleFoil);
            pModifyMenu->addAction(s_pXDirect->m_pActions->m_pSetTEGap);
            pModifyMenu->addAction(s_pXDirect->m_pActions->m_pSetLERadius);
            pModifyMenu->addAction(s_pXDirect->m_pActions->m_pSetFlap);
        }

        pFoilTableMenu->addSeparator();
        pFoilTableMenu->addAction(pCopyAction);
        pFoilTableMenu->addAction(pPasteAction);
        QMenu *pExportMenu = pFoilTableMenu->addMenu("Export");
        {
            pExportMenu->addAction(s_pXDirect->m_pActions->m_pExportCurFoilDat);
            pExportMenu->addAction(s_pXDirect->m_pActions->m_pExportCurFoilSVG);
        }
    }
    pFoilTableMenu->exec(pEvent->globalPos());
}


Foil* FoilTableView::setObjectFromIndex(QModelIndex index)
{
    QModelIndex ind0 = index.sibling(index.row(), 0);
    QStandardItem *pSelectedItem = m_pFoilModel->itemFromIndex(ind0);

    if(!pSelectedItem) return nullptr;

    Foil *pFoil = Objects2d::foil(pSelectedItem->text().toStdString());
    s_pXDirect->setFoil(pFoil);
    s_pXDirect->setControls();
    s_pXDirect->updateView();
    return pFoil;
}


void FoilTableView::onCurrentRowChanged(QModelIndex currentindex, QModelIndex)
{
    setObjectFromIndex(currentindex);
}


void FoilTableView::onItemClicked(const QModelIndex &index)
{
    Foil *pFoil = setObjectFromIndex(index);
    if(!pFoil) return;

    if(index.column()==9)
    {
        pFoil->setVisible(!pFoil->isVisible());
        emit s_pXDirect->projectModified();
    }
    else if(index.column()==10)
    {
        pFoil->showCamberLine(!pFoil->isCamberLineVisible());
        emit s_pXDirect->projectModified();
    }
    else if (index.column()==11)
    {
        LineStyle ls(pFoil->isVisible(), pFoil->lineStipple(), pFoil->lineWidth(), pFoil->lineColor(), pFoil->pointStyle());
        LineMenu *pLineMenu = new LineMenu(nullptr);
        pLineMenu->initMenu(ls);
        pLineMenu->exec(QCursor::pos());
        ls = pLineMenu->theStyle();
        Objects2d::setFoilStyle(pFoil, ls, pLineMenu->styleChanged(), pLineMenu->widthChanged(),
                                pLineMenu->colorChanged(), pLineMenu->pointsChanged(), true, xfl::darkFactor());

        emit s_pXDirect->projectModified();
    }
    s_pXDirect->resetCurves();
    s_pXDirect->m_pDFoilWt->updateView();
}


void FoilTableView::setTableFontStruct(const FontStruct &fntstruct)
{
    setFont(fntstruct.font());
    setFont(fntstruct.font());
    verticalHeader()->setFont(fntstruct.font());
    horizontalHeader()->setFont(fntstruct.font());
}

