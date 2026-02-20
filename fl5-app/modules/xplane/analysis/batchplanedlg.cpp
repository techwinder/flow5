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
#include <QDesktopServices>
#include <QDateTime>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

#include "batchplanedlg.h"




#include <api/fl5core.h>
#include <api/flow5events.h>
#include <api/llttask.h>
#include <api/objects3d.h>
#include <api/panelanalysis.h>
#include <api/plane.h>
#include <api/planepolar.h>
#include <api/planetask.h>
#include <api/planexfl.h>
#include <api/task3d.h>

#include <core/displayoptions.h>
#include <core/fontstruct.h>
#include <api/units.h>
#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/controls/analysisrangetable.h>
#include <interfaces/controls/t8rangetable.h>
#include <interfaces/editors/analysis3ddef/t1234578polardlg.h>
#include <interfaces/editors/analysis3ddef/t6polardlg.h>
#include <interfaces/script/xflexecutor.h>
#include <interfaces/widgets/customdlg/newnamedlg.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/mvc/expandabletreeview.h>
#include <interfaces/widgets/mvc/objecttreedelegate.h>
#include <interfaces/widgets/mvc/objecttreeitem.h>
#include <interfaces/widgets/mvc/objecttreemodel.h>
#include <modules/xplane/analysis/analysis3dsettings.h>
#include <modules/xplane/controls/planeexplorer.h>


QByteArray BatchPlaneDlg::s_Geometry;

QByteArray BatchPlaneDlg::s_HMainSplitterSizes;
QByteArray BatchPlaneDlg::s_VLeftSplitterSizes;

QMap<QString, bool> BatchPlaneDlg::s_Analyses;
bool BatchPlaneDlg::s_bStorePOpps = false;


BatchPlaneDlg::BatchPlaneDlg(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Batch mode"));

    setupLayout();
    connectSignals();

    m_pExecutor = nullptr;

    m_bChanged = false;
    m_bHasErrors = false;
}


BatchPlaneDlg::~BatchPlaneDlg()
{
    delete m_pDelegate;
}


void BatchPlaneDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_psplHMain = new QSplitter(Qt::Horizontal, this);
        {
            QFrame *pfrLeft = new QFrame;
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
                pfrLeft->setLayout(pLeftLayout);
            }

            QFrame*pfrMiddle = new QFrame;
            {
                QVBoxLayout *pMidLayout = new QVBoxLayout;
                {
                    m_pTabWidget = new QTabWidget;
                    {
                        m_pT12RangeTable = new AnalysisRangeTable(this);
                        m_pT12RangeTable->setName("BatchModeDlg::t12");
                        m_pT12RangeTable->setPolarType(xfl::T1POLAR);
                        m_pTabWidget->addTab(m_pT12RangeTable, "T12");

                        m_pT3RangeTable = new AnalysisRangeTable(this);
                        m_pT3RangeTable->setName("BatchModeDlg::t3");
                        m_pT3RangeTable->setPolarType(xfl::T3POLAR);
                        m_pTabWidget->addTab(m_pT3RangeTable, "T3");

                        m_pT5RangeTable = new AnalysisRangeTable(this);
                        m_pT5RangeTable->setName("BatchModeDlg::t5");
                        m_pT5RangeTable->setPolarType(xfl::T5POLAR);
                        m_pTabWidget->addTab(m_pT5RangeTable, "T5");

                        m_pT6RangeTable = new AnalysisRangeTable(this);
                        m_pT6RangeTable->setName("T6");
                        m_pT6RangeTable->setPolarType(xfl::T6POLAR);
                        m_pTabWidget->addTab(m_pT6RangeTable, "T6");

                        m_pT7RangeTable = new AnalysisRangeTable(this);
                        m_pT7RangeTable->setName("BatchModeDlg::t7");
                        m_pT7RangeTable->setPolarType(xfl::T7POLAR);
                        m_pTabWidget->addTab(m_pT7RangeTable, "T7");

                        m_pT8Table = new T8RangeTable(this);
                        m_pT8Table->setName("BatchModeDlg::t8");
                        m_pTabWidget->addTab(m_pT8Table, "T8");
                    }

                    QHBoxLayout *pOptionLayout = new QHBoxLayout;
                    {
                        m_pchStorePOpps      = new QCheckBox(tr("Store operating points"));
                        m_pchStorePOpps->setToolTip(tr("<p>"
                                                    "If activated, the operating points will be stored at the end of the calculation. "
                                                    "The results are stored in the polar in all cases."
                                                    "</p>"));
                        m_pchStabDerivatives = new QCheckBox(tr("Compute derivatives"));
                        m_pchStabDerivatives->setToolTip(tr("<p>"
                                                         "If activated, stability derivatives and eigenthings will be "
                                                         "computed during a T12358 run.<br>"
                                                         "Deactivate to save a little computation time."
                                                         "</p>"));
                        pOptionLayout->addWidget(m_pchStorePOpps);
                        pOptionLayout->addStretch();
                        pOptionLayout->addWidget(m_pchStabDerivatives);
                    }


                    pMidLayout->addWidget(m_pTabWidget);
                    pMidLayout->addLayout(pOptionLayout);
                    pMidLayout->setStretchFactor(m_pTabWidget,1);
                }
                pfrMiddle->setLayout(pMidLayout);
            }

            QFrame *pfrRight = new QFrame;
            {
                QVBoxLayout *pRangeFrameLayout = new QVBoxLayout;
                {
                    m_ppto = new PlainTextOutput;

                    QVBoxLayout *pStatusBtnLayout = new QVBoxLayout;
                    {
                        QLabel *pFlow5Link = new QLabel;
                        pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Tutorials/Batch.html>https://flow5.tech/.../Batch.html</a>");
                        pFlow5Link->setOpenExternalLinks(true);
                        pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
                        m_plabStatus = new QLabel("Not running.");
                        m_plabStatus->setFont(DisplayOptions::tableFont());

                        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
                        {
                            QPushButton *ppbClear = new QPushButton("Clear output");
                            connect(ppbClear, SIGNAL(clicked()), m_ppto, SLOT(clear()));
                            m_ppbAnalyze =  new QPushButton("Calculate");
                            m_ppbAnalyze->setDefault(true);
                            m_ppbAnalyze->setAutoDefault(true);
                            m_pButtonBox->addButton(ppbClear, QDialogButtonBox::ActionRole);
                            m_pButtonBox->addButton(m_ppbAnalyze, QDialogButtonBox::ActionRole);
                            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
                        }

                        pStatusBtnLayout->addWidget(pFlow5Link);
                        pStatusBtnLayout->addWidget(m_plabStatus);
                        pStatusBtnLayout->addWidget(m_pButtonBox);
                    }

                    pRangeFrameLayout->addWidget(m_ppto);
                    pRangeFrameLayout->addLayout(pStatusBtnLayout);
                    pRangeFrameLayout->setStretchFactor(m_ppto,1);
                }
                pfrRight->setLayout(pRangeFrameLayout);
            }
            m_psplHMain->addWidget(pfrLeft);
            m_psplHMain->addWidget(pfrMiddle);
            m_psplHMain->addWidget(pfrRight);
            m_psplHMain->setChildrenCollapsible(false);
        }

        pMainLayout->addWidget(m_psplHMain);
    }
    setLayout(pMainLayout);
}


void BatchPlaneDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton) reject();
    else if(pButton == m_ppbAnalyze) calculate();
}


void BatchPlaneDlg::reject()
{
    if(m_pExecutor && m_pExecutor->isRunning())
    {
        m_pExecutor->onCancel();
    }

    QDialog::reject();
}


void BatchPlaneDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch(pEvent->key())
    {
        case Qt::Key_L:
            QDesktopServices::openUrl(QUrl::fromLocalFile(SaveOptions::lastLogFileName()));
            break;
        default:
            QDialog::keyPressEvent(pEvent);
    }
}


void BatchPlaneDlg::connectSignals()
{
    connect(m_psplHMain,           SIGNAL(splitterMoved(int,int)),  SLOT(onResizeColumns()));
    connect(m_pchStorePOpps,       SIGNAL(clicked(bool)),           SLOT(onOption()));
    connect(m_pchStabDerivatives,  SIGNAL(clicked(bool)),           SLOT(onOption()));
    connect(m_pStruct->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentRowChanged(QModelIndex,QModelIndex)));
}


void BatchPlaneDlg::onOption()
{
    s_bStorePOpps = m_pchStorePOpps->isChecked();
    Analysis3dSettings::setStabDerivatives(m_pchStabDerivatives->isChecked());
    if(m_pExecutor)
        m_pExecutor->setMakePOpps(s_bStorePOpps);
}


void BatchPlaneDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent const *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        m_ppto->onAppendQText(pMsgEvent->msg());
    }
    else if(pEvent->type()==TASK3D_END_EVENT)
    {
//        onAnalysisFinished();
    }
    else
        QDialog::customEvent(pEvent);
}


void BatchPlaneDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_bStorePOpps  = m_pchStorePOpps->isChecked();
    Analysis3dSettings::setStabDerivatives(m_pchStabDerivatives->isChecked());

    s_Geometry = saveGeometry();

    s_HMainSplitterSizes  = m_psplHMain->saveState();
    s_VLeftSplitterSizes  = m_psplVLeft->saveState();
}


void BatchPlaneDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);

    if(s_HMainSplitterSizes.length()>0)  m_psplHMain->restoreState(s_HMainSplitterSizes);
    if(s_VLeftSplitterSizes.length()>0)  m_psplVLeft->restoreState(s_VLeftSplitterSizes);

    initDialog();
    onResizeColumns();
}


void BatchPlaneDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("BatchPlaneDlg");
    {
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();
        s_HMainSplitterSizes      = settings.value("HSplitterSizes").toByteArray();
        s_VLeftSplitterSizes      = settings.value("VLeftSplitterSizes").toByteArray();
    }
    settings.endGroup();
}


void BatchPlaneDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("BatchPlaneDlg");
    {
        settings.setValue("WindowGeom", s_Geometry);
        settings.setValue("HSplitterSizes",      s_HMainSplitterSizes);
        settings.setValue("VLeftSplitterSizes",  s_VLeftSplitterSizes);
    }
    settings.endGroup();
}


void BatchPlaneDlg::onEditAnalysis()
{
    PlanePolar *pWPolar = nullptr;
    if(!pWPolar) return;

    int res=-1;

    if(pWPolar->isStabilityPolar())
    {
/*        StabPolarDlg dlg(this);
        dlg.initPolar3dDlg(nullptr, pWPolar);
        res = dlg.exec();*/
    }
    else if(pWPolar->isControlPolar())
    {
        T6PolarDlg dlg(this);
        dlg.initPolar3dDlg(nullptr, pWPolar);
        res = dlg.exec();
    }
    else
    {
        T1234578PolarDlg dlg(this);
        dlg.initPolar3dDlg(nullptr, pWPolar);
        res = dlg.exec();
    }

    if (res != QDialog::Accepted)
    {
        return;
    }
}


void BatchPlaneDlg::initDialog()
{
    ObjectTreeItem *pRootItem = m_pModel->rootItem();
    pRootItem->setName("Planes");

    m_pStruct->selectionModel()->blockSignals(true);

    for(int iPlane=0; iPlane<Objects3d::nPlanes(); iPlane++)
    {
        Plane const *pPlane = Objects3d::planeAt(iPlane);
        if(!pPlane) continue;

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

    m_pStruct->selectionModel()->blockSignals(false);

    m_pStruct->onPolarLevel();
    m_pT12RangeTable->fillTable();
    m_pT3RangeTable->fillTable();
    m_pT5RangeTable->fillTable();
    m_pT6RangeTable->fillTable();
    m_pT7RangeTable->fillTable();
    m_pT8Table->fillRangeTable();

    m_pchStorePOpps->setChecked(s_bStorePOpps);
    m_pchStabDerivatives->setChecked(Analysis3dSettings::bStabDerivatives());

    m_pButtonBox->setFocus();
}


void BatchPlaneDlg::onResizeColumns()
{
/*    m_pT12RangeTable->onResizeColumns();
    m_pT3RangeTable->onResizeColumns();
    m_pT5RangeTable->onResizeColumns();
    m_pT6RangeTable->onResizeColumns();
    m_pT7RangeTable->onResizeColumns();*/

    update();
}


/**
* Updates the text output in the dialog box and the log file.
* @param msg the text message to append to the output widget and to the log file.
*/
void BatchPlaneDlg::onMessage(QString const &msg)
{
    m_ppto->onAppendQText(msg);
}


void BatchPlaneDlg::calculate()
{
    if(m_pExecutor)
    {
        onMessage("         -----Analysis cancel request-----   \n\n");
        m_pExecutor->onCancel();
        return;
    }

    QApplication::setOverrideCursor(Qt::BusyCursor);

    QString strange, log;
    m_bChanged = true;

    //readData
    s_bStorePOpps  = m_pchStorePOpps->isChecked();

    // read operating ranges
    std::vector<double> t12opps, t3opps, t5opps, t6opps, t7opps;
    std::vector<T8Opp> txopps;

    for(int i=0; i<AnalysisRangeTable::t12Range().size(); i++)
    {
        AnalysisRange const &range = AnalysisRangeTable::t12Range().at(i);
        if(range.isActive())
        {
            std::vector<double> vals = range.values();
            t12opps.insert(t12opps.end(), vals.begin(), vals.end());
        }
    }

    for(int i=0; i<AnalysisRangeTable::t3Range().size(); i++)
    {
        AnalysisRange const &range = AnalysisRangeTable::t3Range().at(i);
        if(range.isActive())
        {
            std::vector<double> vals = range.values();
            t3opps.insert(t3opps.end(), vals.begin(), vals.end());
        }
    }

    for(int i=0; i<AnalysisRangeTable::t5Range().size(); i++)
    {
        AnalysisRange const &range = AnalysisRangeTable::t5Range().at(i);
        if(range.isActive())
        {
            std::vector<double> vals = range.values();
            t5opps.insert(t5opps.end(), vals.begin(), vals.end());
        }
    }

    log += "T6 operating range:\n";
    for(int i=0; i<AnalysisRangeTable::t6Range().size(); i++)
    {
        AnalysisRange const &range = AnalysisRangeTable::t6Range().at(i);
        if(range.isActive())
        {
            std::vector<double> vals = range.values();
            t6opps.insert(t6opps.end(), vals.begin(), vals.end());
        }
    }

    log += "T7 operating range:\n";
    for(int i=0; i<AnalysisRangeTable::t7Range().size(); i++)
    {
        AnalysisRange const &range = AnalysisRangeTable::t7Range().at(i);
        if(range.isActive())
        {
            std::vector<double> vals = range.values();
            t7opps.insert(t7opps.end(), vals.begin(), vals.end());
        }
    }

    log += "T8 operating range:\n";
    for(uint i=0; i<T8RangeTable::t8range().size(); i++)
    {
        T8Opp const &range = T8RangeTable::t8range().at(i);
        if(range.isActive()) txopps.push_back(range);
    }

    if( t12opps.size()==0 && t3opps.size()==0 && t5opps.size()==0 &&
        t6opps.size()==0  && t7opps.size()==0 && txopps.size()==0)
    {
        onMessage("No operating point ranges selected for analysis - aborting.\n\n");
        QApplication::restoreOverrideCursor();
        return;
    }

    QList<Plane*> planelist;
    QList<PlanePolar*> polarlist;
    for(int ip=0; ip<Objects3d::nPlanes(); ip++)
    {
        Objects3d::planeAt(ip)->setActive(false);
    }

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
                    Plane *pPlane = Objects3d::plane(parentItem->name().toStdString());
                    PlanePolar *pWPolar = Objects3d::wPolar(pPlane, pSelectedItem->name().toStdString());
                    if(pPlane && pWPolar)
                    {
                        if(!planelist.contains(pPlane)) planelist.append(pPlane);
                        pPlane->setActive(true);
                        if(!polarlist.contains(pWPolar)) polarlist.append(pWPolar);
                    }
                }
            }
        }
    }

    for(Plane *pPlane : planelist)
    {
        if(!pPlane) continue;
        if(pPlane->isActive() && pPlane->isXflType())
        {
            PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(pPlane);
            if(!pPlaneXfl->isInitialized())
            {
                pPlaneXfl->makePlane(true, false, true);
            }
        }
    }

    if(planelist.isEmpty())
    {
        onMessage("No planes to analyze - aborting\n\n");
        QApplication::restoreOverrideCursor();
        return;
    }
    else
    {
        strange = "Planes to analyze:\n";
        for(int ip=0; ip<planelist.size(); ip++)
            strange += "   " + QString::fromStdString(planelist.at(ip)->name()) + "\n";
        onMessage(strange+"\n\n");
    }

    //run
    m_pExecutor = new XflExecutor();
    m_pExecutor->setEventDestination(nullptr);

    QString logFileName = SaveOptions::newLogFileName();
    SaveOptions::setLastLogFileName(logFileName);
    m_pExecutor->setLogFile        (logFileName, QString::fromStdString(fl5::versionName(true)));
    m_pExecutor->setMakePOpps(s_bStorePOpps);
    m_pExecutor->setStabDerivatives(Analysis3dSettings::bStabDerivatives());
    m_pExecutor->setT12Range(AnalysisRangeTable::t12Range());
    m_pExecutor->setT3Range( AnalysisRangeTable::t3Range());
    m_pExecutor->setT5Range( AnalysisRangeTable::t5Range());
    m_pExecutor->setT6Range( AnalysisRangeTable::t6Range());
    m_pExecutor->setT7Range( AnalysisRangeTable::t7Range());
    m_pExecutor->setT8Range( T8RangeTable::t8range());
    m_pExecutor->setPlanes(planelist);
    m_pExecutor->setWPolars(polarlist);

    m_pExecutor->makePlaneTasks(strange);
    onMessage(strange);

    m_pButtonBox->button(QDialogButtonBox::Close)->setEnabled(false);
    m_ppbAnalyze->setText("Cancel");

    QApplication::restoreOverrideCursor();

    Task3d::setCancelled(false);
    TriMesh::setCancelled(false);
    PanelAnalysis::setMultiThread(xfl::isMultiThreaded());
    PanelAnalysis::setMaxThreadCount(xfl::maxThreadCount());

    onMessage(QString::fromStdString(fl5::versionName(true)) + "\n");
    QDateTime dt = QDateTime::currentDateTime();
    QString str = dt.toString();
    onMessage(str+"\n\n");

    m_bHasErrors = false;


    connect(m_pExecutor, &XflExecutor::taskStarted,   this,         &BatchPlaneDlg::onPlaneTaskStarted);
    connect(m_pExecutor, &XflExecutor::taskFinished,  this,         &BatchPlaneDlg::onAnalysisFinished);
    connect(m_pExecutor, &XflExecutor::outputMessage, this,         &BatchPlaneDlg::onMessage);

    //run the instance asynchronously
    // Launch the task async to keep the UI responsive
    QFuture<void> future = QtConcurrent::run(&XflExecutor::onRunExecutor, m_pExecutor);
    (void)future;

/*    QThread *pThread = new QThread;
    m_pExecutor->moveToThread(pThread);
    connect(pThread,     &QThread::started,           m_pExecutor,  &XflExecutor::onRunExecutor);
    connect(pThread,     &QThread::finished,          pThread,      &QThread::deleteLater);
    pThread->start(xfl::threadPriority());*/

}


void BatchPlaneDlg::onAnalysisFinished()
{
    if(m_pExecutor)
    {
//        m_pExecutor->thread()->exit(0);
        delete m_pExecutor;
        m_pExecutor = nullptr;
    }
    onMessage("_____Plane analyses completed_____\n\n");

    m_plabStatus->setText("Not running.");
    m_ppbAnalyze->setText("Calculate");
    m_pButtonBox->button(QDialogButtonBox::Close)->setEnabled(true);
}


void BatchPlaneDlg::onPlaneTaskStarted(int iTask)
{
    if(!m_pExecutor) return;
    int total = m_pExecutor->planeTasks().size();
    QString strange;

    strange = QString::asprintf("(%d/%d) ", iTask+1, total);
    Task3d  *pTask = m_pExecutor->planeTasks().at(iTask);
    PlaneTask const *pPlaneTask = dynamic_cast<PlaneTask const*>(pTask);
    if(pPlaneTask)
        strange += QString::fromStdString(pPlaneTask->plane()->name() + " / "+pPlaneTask->wPolar()->name());

    LLTTask *pLLTTask = dynamic_cast<LLTTask*>(pTask);
    if(pLLTTask)
        strange += QString::fromStdString(pLLTTask->plane()->name() + " / "+pLLTTask->wPolar()->name());

    m_plabStatus->setText(strange);
}


void BatchPlaneDlg::onCurrentRowChanged(QModelIndex currentidx, QModelIndex )
{
    setObjectProperties(currentidx);
}


void BatchPlaneDlg::setObjectProperties(QModelIndex index)
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
            Plane *pPlane  = Objects3d::plane(parentItem->name().toStdString());
            PlanePolar *pWPolar = Objects3d::wPolar(pPlane, pSelectedItem->name().toStdString());

            if(pPlane && pWPolar)
            {
                std::string str;
                pWPolar->getProperties(str, pPlane);
                props = QString::fromStdString(str);
            }
        }
        else
        {
            Q_ASSERT(pSelectedItem->isObjectLevel());
            Plane *pPlane = Objects3d::plane(pSelectedItem->name().toStdString());
            if(pPlane)
            {
                if(pPlane->description().length())
                {
                    props = QString::fromStdString(pPlane->description() + "\n\n");
                }
                props += QString::fromStdString(pPlane->planeData(false));
            }
        }
    }

    m_pptoObjectProps->setPlainText(props);
}



