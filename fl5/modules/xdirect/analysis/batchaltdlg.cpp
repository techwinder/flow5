/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois

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

#include <iostream>

#include <QGroupBox>
#include <QHBoxLayout>
#include<QHeaderView>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrentRun>

#include "batchaltdlg.h"

#include <api/fl5core.h>
#include <api/analysisrange.h>
#include <api/flow5events.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <api/oppoint.h>
#include <api/polar.h>
#include <api/utils.h>
#include <api/xfoiltask.h>


#include <core/displayoptions.h>
#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/controls/analysisrangetable.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/customwts/xfldelegate.h>
#include <interfaces/widgets/mvc/expandabletreeview.h>
#include <interfaces/widgets/mvc/objecttreedelegate.h>
#include <interfaces/widgets/mvc/objecttreeitem.h>
#include <interfaces/widgets/mvc/objecttreemodel.h>
#include <modules/xdirect/analysis/polarnamemaker.h>
#include <modules/xdirect/xdirect.h>


QByteArray BatchAltDlg::s_VLeftSplitterSizes;


BatchAltDlg::BatchAltDlg(QWidget *pParent) : BatchDlg(pParent)
{
    setWindowTitle("Multi-threaded batch analysis");

    setupLayout();
    connectBaseSignals();
    connectSignals();
}


BatchAltDlg::~BatchAltDlg()
{
    if(m_pXFile)  delete m_pXFile;
    m_pXFile = nullptr;
}


void BatchAltDlg::setupLayout()
{
    QFrame *pFrame = new QFrame;
    {
        QVBoxLayout *pLeftLayout = new QVBoxLayout;
        {
            m_psplVLeft = new QSplitter(Qt::Vertical, this);
            {
                m_pStruct = new ExpandableTreeView;
                m_pStruct->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
                m_pStruct->setSelectionMode(QAbstractItemView::MultiSelection);
                m_pStruct->setEditTriggers(QAbstractItemView::NoEditTriggers);
                m_pStruct->setUniformRowHeights(true);
                m_pStruct->setRootIsDecorated(true);
                m_pStruct->setFont(DisplayOptions::treeFont());

                QStringList labels;
                labels << "Object"  << "1234567"<< "";

                m_pModel = new ObjectTreeModel(this);
                m_pModel->setHeaderData(0, Qt::Horizontal, "Objects", Qt::DisplayRole);
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
                m_pDelegate->showStyle(false);
                m_pStruct->setItemDelegate(m_pDelegate);

                QItemSelectionModel *selectionModel = new QItemSelectionModel(m_pModel);
                m_pStruct->setSelectionModel(selectionModel);
                m_pptoObjectProps = new PlainTextOutput;

                m_psplVLeft->addWidget(m_pStruct);
                m_psplVLeft->addWidget(m_pptoObjectProps);

            }
            pLeftLayout->addWidget(m_pStruct->cmdWidget());
            pLeftLayout->addWidget(m_psplVLeft);
        }
        pFrame->setLayout(pLeftLayout);
    }

    m_pLeftTabWt->addTab(pFrame, "Analyses");
    m_pLeftTabWt->addTab(m_pfrRangeVars, "Operating points");

    QHBoxLayout *pBoxesLayout = new QHBoxLayout;
    {
        pBoxesLayout->addWidget(m_pHSplitter);
    }

    setLayout(pBoxesLayout);
}


void BatchAltDlg::connectSignals()
{

    connect(m_pStruct->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentRowChanged(QModelIndex,QModelIndex)));
}


void BatchAltDlg::initDialog()
{
    BatchDlg::initDialog();

    ObjectTreeItem *pRootItem = m_pModel->rootItem();
    pRootItem->setName("Planes");


    for(int ifoil=0; ifoil<Objects2d::nFoils(); ifoil++)
    {
        Foil const *pFoil = Objects2d::foilAt(ifoil);
        if(!pFoil) continue;

        LineStyle ls(pFoil->theStyle());
        ObjectTreeItem *pPlaneItem = m_pModel->appendRow(pRootItem, pFoil->name(), pFoil->theStyle(), Qt::Unchecked);

        for(int iPolar=0; iPolar<Objects2d::nPolars(); iPolar++)
        {
            Polar *pPolar = Objects2d::polarAt(iPolar);
            if(!pPolar) continue;
            if(pPolar && pPolar->foilName().compare(pFoil->name())==0)
            {
                LineStyle ls(pPolar->theStyle());
                ls.m_bIsEnabled = true;
                m_pModel->appendRow(pPlaneItem, pPolar->name(), ls, Qt::Unchecked);
            }
        }
    }


    m_pStruct->onPolarLevel();

    m_pButtonBox->setFocus();

}


void BatchAltDlg::showEvent(QShowEvent *pEvent)
{
    BatchDlg::showEvent(pEvent);

    if(s_VLeftSplitterSizes.length()>0)  m_psplVLeft->restoreState(s_VLeftSplitterSizes);
}


void BatchAltDlg::hideEvent(QHideEvent *pEvent)
{
    BatchDlg::hideEvent(pEvent);

    s_VLeftSplitterSizes  = m_psplVLeft->saveState();
}

void BatchAltDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("XFoilBatchDlg");
    {

    }
    settings.endGroup();
}


void BatchAltDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("XFoilBatchDlg");
    {

    }
    settings.endGroup();
}


void BatchAltDlg::onCurrentRowChanged(QModelIndex currentidx, QModelIndex )
{
    setObjectProperties(currentidx);
}


void BatchAltDlg::setObjectProperties(QModelIndex index)
{
    QString props;
    ObjectTreeItem *pSelectedItem = nullptr;

    if(index.column()==0)
    {
        pSelectedItem = m_pModel->itemFromIndex(index);
    }
    else if(index.column()>=1)
    {
        QModelIndex ind = index.sibling(index.row(), 0);
        pSelectedItem = m_pModel->itemFromIndex(ind);
    }

    if(pSelectedItem)
    {
        if(pSelectedItem->isPolarLevel())
        {
            ObjectTreeItem *parentItem = pSelectedItem->parentItem();
            Q_ASSERT(parentItem->isObjectLevel());
            //one parent, the user has clicked a WPolar
            Foil *pFoil = Objects2d::foil(parentItem->name().toStdString());
            Polar *pPolar = Objects2d::polar(pFoil, pSelectedItem->name().toStdString());

            if(pFoil && pPolar)
            {
                std::string str;
                str = pPolar->properties();
                props = QString::fromStdString(str);
            }
        }
        else
        {
            Q_ASSERT(pSelectedItem->isObjectLevel());
            Foil *pFoil = Objects2d::foil(pSelectedItem->name().toStdString());
            if(pFoil)
            {
                if(pFoil->description().length())
                {
                    props = QString::fromStdString(pFoil->description() + "\n\n");
                }
                props += QString::fromStdString(pFoil->properties(false));
            }
        }
    }

    m_pptoObjectProps->setPlainText(props);
}



void BatchAltDlg::onAnalyze()
{
    if(m_bIsRunning)
    {
        m_bCancel = true;
        XFoilTask::cancelAnalyses();
        XFoil::setCancel(true);
        return;
    }

    m_bCancel    = false;
    m_bIsRunning = true;

    m_pButtonBox->button(QDialogButtonBox::Close)->setEnabled(false);

    QString FileName = SaveOptions::newLogFileName();
    m_pXFile = new QFile(FileName);
    if (!m_pXFile->open(QIODevice::WriteOnly | QIODevice::Text)) m_pXFile = nullptr;

    readParams();

    setFileHeader();

    m_ppbAnalyze->setFocus();

    QString strong;


    m_AnalysisPair.clear();
    QList<Foil*> foils;
    QList<Polar*> polars;

    QModelIndexList indexes = m_pStruct->selectionModel()->selectedIndexes();
    for(QModelIndex const &index : indexes)
    {
        if (index.isValid())
        {
            ObjectTreeItem *pSelectedItem = m_pModel->itemFromIndex(index);
            if(pSelectedItem && pSelectedItem->isPolarLevel())
            {
                ObjectTreeItem *parentItem = pSelectedItem->parentItem();
                if(parentItem)
                {
                    Foil *pFoil = Objects2d::foil(parentItem->name().toStdString());
                    Polar *pPolar = Objects2d::polar(pFoil, pSelectedItem->name().toStdString());
                    if(pFoil && pPolar)
                    {
                        m_AnalysisPair.push_back({pFoil, pPolar});
                    }
                }
            }
        }
    }

    if(m_AnalysisPair.isEmpty())
    {
        strong ="No foil defined for analysis\n\n";
        m_ppto->insertPlainText(strong);
        cleanUp();
        return;
    }

    m_ppbAnalyze->setText("Cancel");

    m_nAnalysis = m_AnalysisPair.size();
    m_nTaskDone = 0;
    m_nTaskStarted = 0;

    strong = QString::asprintf("Found %d foil/polar pairs to analyze\n", m_nAnalysis);
    m_ppto->insertPlainText(strong);


    XFoilTask::setCancelled(false);

    // failsafe: clean up any remaining tasks from a previous batch
    if(m_Tasks.size()!=0)
    {
        m_ppto->appendPlainText("Uncleaned !\n");
        for(uint it=0; it<m_Tasks.size(); it++)
            delete m_Tasks.at(it);

        m_Tasks.clear();
    }

    QApplication::setOverrideCursor(Qt::BusyCursor);

    m_ppto->appendPlainText("\nStarted/Done/Total\n");

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
            QFuture<void> future = QtConcurrent::run(&BatchAltDlg::batchLaunch, this);
#else
            QtConcurrent::run(pXFoilTask, &XFoilTask::run);
#endif
}




