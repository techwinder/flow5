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


#include <QGridLayout>
#include <QHeaderView>
#include <QResizeEvent>
#include <QMenu>

#include "graphtilectrls.h"
#include <interfaces/graphs/controls/graphtiles.h>
#include <globals/mainframe.h>
#include <core/displayoptions.h>
#include <interfaces/graphs/controls/graphtilevariableset.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/xfldelegate.h>
#include <interfaces/widgets/customwts/cptableview.h>

MainFrame *GraphTileCtrls::s_pMainFrame = nullptr;
XPlane *GraphTileCtrls::s_pXPlane = nullptr;
XDirect * GraphTileCtrls::s_pXDirect = nullptr;


GraphTileCtrls::GraphTileCtrls(GraphTiles *pParent) : QWidget(pParent)
{
    m_pGraphTileWt = nullptr;
    makeCommonWts();
}


void GraphTileCtrls::setMaxGraphs(int nGraphs)
{
    if(nGraphs==1)
    {
        m_prbTwoGraphs->setEnabled(false);
        m_prbFourGraphs->setEnabled(false);
        m_prbAllGraphs->setEnabled(false);
    }
    else if(nGraphs==2)
    {
        m_prbFourGraphs->setEnabled(false);
        m_prbAllGraphs->setEnabled(false);
        for(int ig=2; ig<5; ig++) m_prbGraph[ig]->setEnabled(false);
    }
    else if(nGraphs==4)
    {
        m_prbAllGraphs->setEnabled(false);
        for(int ig=4; ig<5; ig++) m_prbGraph[ig]->setEnabled(false);
    }
}


void GraphTileCtrls::makeCommonWts()
{
    m_pcptVariableSet = new CPTableView(this);
    m_pcptVariableSet->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pcptVariableSet->setEditable(true);
    m_pcptVariableSet->setCharSize(3,7);
    m_pcptVariableSet->setWindowTitle("Variable sets");
    m_pcptVariableSet->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_pVariableSetModel = new ActionItemModel(this);
    m_pVariableSetModel->setRowCount(1);//temporary
    m_pVariableSetModel->setColumnCount(2);
    m_pVariableSetModel->setActionColumn(1);
    m_pVariableSetModel->setHeaderData(0, Qt::Horizontal, "Variable set");
    m_pVariableSetModel->setHeaderData(1, Qt::Horizontal, "Actions");

    m_pcptVariableSet->setModel(m_pVariableSetModel);

    QHeaderView *pHHeader = m_pcptVariableSet->horizontalHeader();
    pHHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    pHHeader->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    QHeaderView *pVHeader = m_pcptVariableSet->verticalHeader();
    pVHeader->setDefaultSectionSize(DisplayOptions::tableFontStruct().height()*2);
    pVHeader->setSectionResizeMode(QHeaderView::Fixed);

    m_pActionDelegate = new XflDelegate(this);
    m_pActionDelegate->setActionColumn(1);
    m_pcptVariableSet->setItemDelegate(m_pActionDelegate);
    QVector<int>precision = {-1,-1};
    m_pActionDelegate->setDigits(precision);
    m_pActionDelegate->setItemTypes({XflDelegate::STRING, XflDelegate::ACTION});

    QString strange;
    for(int ig=0; ig<5; ig++)
    {
        strange = QString::asprintf("Graph %d", ig+1);
        m_prbGraph[ig] = new QRadioButton(strange, this);
        strange = QString::asprintf("(%d)", ig+1);
        m_prbGraph[ig]->setToolTip(strange);
    }

    m_prbTwoGraphs  = new QRadioButton("2 graphs");
    m_prbTwoGraphs->setToolTip("(T)");
    m_prbFourGraphs = new QRadioButton("4 graphs");
    m_prbFourGraphs->setToolTip("(F)");
    m_prbAllGraphs  = new QRadioButton("All graphs");
    m_prbAllGraphs->setToolTip("(A)");
}


void GraphTileCtrls::setupLayout()
{
    QGridLayout *pMainLayout = new QGridLayout;
    {
        pMainLayout->addWidget(m_pcptVariableSet, 1,1,1,2);
        pMainLayout->addWidget(m_prbGraph[0],     2,1);
        pMainLayout->addWidget(m_prbGraph[1],     2,2);
        pMainLayout->addWidget(m_prbGraph[2],     3,1);
        pMainLayout->addWidget(m_prbGraph[3],     3,2);
        pMainLayout->addWidget(m_prbGraph[4],     4,1);
        pMainLayout->addWidget(m_prbTwoGraphs,    4,2);
        pMainLayout->addWidget(m_prbFourGraphs,   5,1);
        pMainLayout->addWidget(m_prbAllGraphs,    5,2);

    }
    setLayout(pMainLayout);
}


void GraphTileCtrls::connectSignals()
{
//    if(m_pGraphTileWt && m_pGraphTileWt->variableSetCount()>1)
//    {
        connect(m_pcptVariableSet,                   SIGNAL(clicked(QModelIndex)),                       SLOT(onVarSetClicked(QModelIndex)));
//        connect(m_pcptVariableSet->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentRowChanged(QModelIndex,QModelIndex)));
        connect(m_pVariableSetModel,                 SIGNAL(dataChanged(QModelIndex,QModelIndex)),       SLOT(onCellChanged(QModelIndex,QModelIndex)));
//    }

    connect(m_prbGraph[0],    SIGNAL(clicked(bool)), m_pGraphTileWt, SLOT(onGraph0()));
    connect(m_prbGraph[1],    SIGNAL(clicked(bool)), m_pGraphTileWt, SLOT(onGraph1()));
    connect(m_prbGraph[2],    SIGNAL(clicked(bool)), m_pGraphTileWt, SLOT(onGraph2()));
    connect(m_prbGraph[3],    SIGNAL(clicked(bool)), m_pGraphTileWt, SLOT(onGraph3()));
    connect(m_prbGraph[4],    SIGNAL(clicked(bool)), m_pGraphTileWt, SLOT(onGraph4()));
    connect(m_prbTwoGraphs,   SIGNAL(clicked(bool)), m_pGraphTileWt, SLOT(onTwoGraphs()));
    connect(m_prbFourGraphs,  SIGNAL(clicked(bool)), m_pGraphTileWt, SLOT(onFourGraphs()));
    connect(m_prbAllGraphs,   SIGNAL(clicked(bool)), m_pGraphTileWt, SLOT(onAllGraphs()));
}


void GraphTileCtrls::setGraphTileWt(GraphTiles *pTileWt)
{
    setParent(pTileWt);
    m_pGraphTileWt = pTileWt;
}


void GraphTileCtrls::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);
    QFont fnt(DisplayOptions::tableFont());
    fnt.setPointSize(DisplayOptions::tableFont().pointSize());
    m_pcptVariableSet->setFont(fnt);
    m_pcptVariableSet->horizontalHeader()->setFont(fnt);
    m_pcptVariableSet->verticalHeader()->setFont(fnt);
}


void GraphTileCtrls::setControls()
{
    fillVariableSetTable();

    checkGraphActions();
}


void GraphTileCtrls::fillVariableSetTable()
{
    m_pVariableSetModel->setRowCount(m_pGraphTileWt->variableSetCount());

    for(int il=0; il<m_pGraphTileWt->variableSetCount(); il++)
    {
        QModelIndex index = m_pVariableSetModel->index(il, 0, QModelIndex());
        QString strange = m_pGraphTileWt->variableSet(il).name();
        m_pVariableSetModel->setData(index, strange);
    }
}


void GraphTileCtrls::selectActiveVariableSet(int iSet)
{
    QModelIndex index = m_pVariableSetModel->index(iSet, 0, QModelIndex());
    m_pcptVariableSet->setCurrentIndex(index);
    m_pcptVariableSet->selectRow(index.row());
}


void GraphTileCtrls::checkGraphActions()
{
    for(int ig=0; ig<5; ig++)    m_prbGraph[ig]->setChecked(false);
    m_prbTwoGraphs->setChecked(false);
    m_prbFourGraphs->setChecked(false);
    m_prbAllGraphs->setChecked(false);

    if     (m_pGraphTileWt->isOneGraph())
    {
        if(m_pGraphTileWt->activeGraphIndex()>=0 && m_pGraphTileWt->activeGraphIndex()<5)
            m_prbGraph[m_pGraphTileWt->activeGraphIndex()]->setChecked(true);
    }
    else if(m_pGraphTileWt->isTwoGraphs())  m_prbTwoGraphs->setChecked(true);
    else if(m_pGraphTileWt->isFourGraphs()) m_prbFourGraphs->setChecked(true);
    else if(m_pGraphTileWt->isAllGraphs())  m_prbAllGraphs->setChecked(true);
}


void GraphTileCtrls::onVarSetClicked(QModelIndex index)
{
    if(!index.isValid())
    {
    }
    else
    {
        int row = index.row();
        m_pGraphTileWt->setVariableSet(row);

        if(index.column()==m_pVariableSetModel->actionColumn())
        {
            m_pcptVariableSet->selectRow(index.row());
            QRect itemrect = m_pcptVariableSet->visualRect(index);
            QPoint menupos = m_pcptVariableSet->mapToGlobal(itemrect.topLeft());
            QMenu *pWingTableRowMenu = new QMenu("Section",this);

            QAction *m_pMoveUpAct        = new QAction("Move up",       this);
            QAction *m_pMoveDownAct        = new QAction("Move down",     this);
            QAction *m_pDeleteAct        = new QAction("Delete",        this);
            QAction *m_pInsertBeforeAct = new QAction("Insert before", this);
            QAction *m_pInsertAfterAct  = new QAction("Insert after",  this);

            connect(m_pDeleteAct,       SIGNAL(triggered(bool)), SLOT(onDelete()));
            connect(m_pMoveUpAct,       SIGNAL(triggered(bool)), SLOT(onMoveUp()));
            connect(m_pMoveDownAct,     SIGNAL(triggered(bool)), SLOT(onMoveDown()));
            connect(m_pInsertBeforeAct, SIGNAL(triggered(bool)), SLOT(onInsertBefore()));
            connect(m_pInsertAfterAct,  SIGNAL(triggered(bool)), SLOT(onInsertAfter()));

            pWingTableRowMenu->addAction(m_pInsertBeforeAct);
            pWingTableRowMenu->addAction(m_pInsertAfterAct);
            pWingTableRowMenu->addAction(m_pDeleteAct);
            pWingTableRowMenu->addSeparator();
            pWingTableRowMenu->addAction(m_pMoveUpAct);
            pWingTableRowMenu->addAction(m_pMoveDownAct);
            pWingTableRowMenu->exec(menupos, m_pInsertBeforeAct);
        }
    }
}


void GraphTileCtrls::onMoveUp()
{
    int row = m_pcptVariableSet->currentIndex().row();
    if(row<0 || row>=m_pGraphTileWt->variableSetCount()) return;
    m_pGraphTileWt->moveSetUp(row);
    fillVariableSetTable();
}


void GraphTileCtrls::onMoveDown()
{
    int row = m_pcptVariableSet->currentIndex().row();
    if(row<0 || row>=m_pGraphTileWt->variableSetCount()) return;
    m_pGraphTileWt->moveSetDown(row);
    fillVariableSetTable();
}


void GraphTileCtrls::onDelete()
{
    int row = m_pcptVariableSet->currentIndex().row();
    if(row<0 || row>=m_pGraphTileWt->variableSetCount()) return;
    if(m_pGraphTileWt->variableSetCount()<=1) return; // leave at least one row

    m_pGraphTileWt->removeVariableSet(row);

    fillVariableSetTable();
}


void GraphTileCtrls::onInsertBefore()
{
    int row = m_pcptVariableSet->currentIndex().row();
    if(row<0 || row>=m_pGraphTileWt->variableSetCount()) return;

    m_pGraphTileWt->duplicateVariableSet(row, true);

    fillVariableSetTable();
}


void GraphTileCtrls::onInsertAfter()
{
    int row = m_pcptVariableSet->currentIndex().row();
    if(row<0 || row>=m_pGraphTileWt->variableSetCount()) return;

    m_pGraphTileWt->duplicateVariableSet(row, false);

    fillVariableSetTable();
}


void GraphTileCtrls::onCellChanged(QModelIndex index,QModelIndex)
{
    int row = index.row();

    QString newname = m_pVariableSetModel->index(row, 0, QModelIndex()).data().toString();
    m_pGraphTileWt->setVariableSetName(row, newname);
}


void GraphTileCtrls::onCurrentRowChanged(QModelIndex index, QModelIndex)
{
    int row = index.row();
    m_pGraphTileWt->setVariableSet(row);
}


