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


#include <QDateTime>
#include <QHeaderView>
#include <QCompleter>
#include <QVBoxLayout>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileSystemModel>



#include <api/fl5core.h>
#include <api/flow5events.h>
#include <api/llttask.h>
#include <api/objects3d.h>
#include <api/plane.h>
#include <api/planepolar.h>
#include <api/planetask.h>
#include <api/planexfl.h>
#include <api/task3d.h>
#include <api/xmlplanepolarreader.h>
#include <api/xmlplanepolarwriter.h>

#include <core/displayoptions.h>
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
#include <interfaces/widgets/customwts/xfldelegate.h>
#include <modules/xplane/analysis/analysis3dsettings.h>
#include <modules/xplane/analysis/batchxmldlg.h>
#include <modules/xplane/xplane.h>


XPlane *BatchXmlDlg::s_pXPlane = nullptr;

QByteArray BatchXmlDlg::s_Geometry;

QByteArray BatchXmlDlg::s_HSplitterSizes;
QByteArray BatchXmlDlg:: s_SplitterPlaneSizes;
QByteArray BatchXmlDlg::s_SplitterWPolarSizes;

QMap<QString, bool> BatchXmlDlg::s_Analyses;
bool BatchXmlDlg::s_bStorePOpps = false;

int BatchXmlDlg::s_LastTabIndex=0;

BatchXmlDlg::BatchXmlDlg(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Batch analysis"));

    setupLayout();
    connectSignals();

    m_pExecutor = nullptr;

    m_bChanged = false;
    m_bHasErrors = false;
}


void BatchXmlDlg::makeTables()
{
    QFont fnt(DisplayOptions::tableFont());
    fnt.setPointSize(DisplayOptions::tableFont().pointSize());

    m_pcpPlaneTable = new CPTableView;
    m_pcpPlaneTable->setCharSize(3,5);
    m_pcpPlaneTable->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    m_pcpPlaneTable->setEditable(false);
    m_pcpPlaneTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pcpPlaneTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_pcpPlaneTable->setFont(fnt);
    m_pcpPlaneTable->horizontalHeader()->setFont(fnt);
    m_pcpPlaneTable->verticalHeader()->setFont(fnt);

    m_pPlaneModel = new ActionItemModel(this);
    m_pPlaneModel->setName("Plane model");
    m_pPlaneModel->setActionColumn(-1);
    m_pPlaneModel->setRowCount(0);//temporary
    m_pPlaneModel->setColumnCount(2);
    m_pPlaneModel->setHeaderData(0, Qt::Horizontal, QString());
    m_pPlaneModel->setHeaderData(1, Qt::Horizontal, tr("Plane name"));
    m_pPlaneModel->setHeaderData(2, Qt::Horizontal, QString());
    m_pcpPlaneTable->setModel(m_pPlaneModel);

    m_pPlaneDelegate = new XflDelegate(this);
    m_pPlaneDelegate->setName("Plane delegate");
    m_pPlaneDelegate->setCheckColumn(0);
    m_pPlaneDelegate->setActionColumn(-1);
    m_pPlaneDelegate->setDigits({-1,-1,-1});
    m_pPlaneDelegate->setItemTypes({XflDelegate::CHECKBOX,XflDelegate::STRING,XflDelegate::STRING});
    m_pcpPlaneTable->setItemDelegate(m_pPlaneDelegate);

    m_pcpAnalysisTable = new CPTableView;
    m_pcpAnalysisTable->setToolTip(tr("Define and activate the generic analyses to associate to the planes.\n") +
                                   tr("Note that if the plane name is defined, the analysis will only be created\n") +
                                   tr("for that specific plane. Leave the plane name blank otherwise."));
    m_pcpAnalysisTable->setCharSize(3,5);
    m_pcpAnalysisTable->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    m_pcpAnalysisTable->setEditable(false);
    m_pcpAnalysisTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pcpAnalysisTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pcpAnalysisTable->setFont(fnt);
    m_pcpAnalysisTable->horizontalHeader()->setFont(fnt);
    m_pcpAnalysisTable->verticalHeader()->setFont(fnt);

    m_pAnalysisModel = new ActionItemModel(this);
    m_pAnalysisModel->setName("Analysis model");
    m_pAnalysisModel->setRowCount(0);//temporary
    m_pAnalysisModel->setColumnCount(3);
    m_pAnalysisModel->setHeaderData(0, Qt::Horizontal, QString());
    m_pAnalysisModel->setHeaderData(1, Qt::Horizontal, tr("File name"));
    m_pAnalysisModel->setHeaderData(2, Qt::Horizontal, QString());
    m_pAnalysisModel->setActionColumn(2);
    m_pcpAnalysisTable->setModel(m_pAnalysisModel);

    m_pAnalysisDelegate = new XflDelegate(this);
    m_pAnalysisDelegate->setName("Analysis delegate");
    m_pAnalysisDelegate->setCheckColumn(0);
    m_pAnalysisDelegate->setActionColumn(2);
    m_pAnalysisDelegate->setDigits({-1,-1,-1});
    m_pAnalysisDelegate->setItemTypes({XflDelegate::CHECKBOX,XflDelegate::STRING,XflDelegate::ACTION});

    m_pcpAnalysisTable->setItemDelegate(m_pAnalysisDelegate);
}


void BatchXmlDlg::setupLayout()
{
    makeTables();

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pHSplitter = new QSplitter(Qt::Horizontal, this);
        {
            QFrame *pPlaneFrame = new QFrame;
            {
                QVBoxLayout *pPlaneFrameLayout = new QVBoxLayout;
                {
                    QHBoxLayout *pActionsLayout = new QHBoxLayout;
                    {
                        QPushButton *pPlaneMenuBtn = new QPushButton(tr("Actions"));
                        {
                            pPlaneMenuBtn->setAutoDefault(false);
                            pPlaneMenuBtn->setDefault(false);
                            QMenu *m_pPlaneMenu = new QMenu(this);
                            {
                                QAction *pPlaneSel = new QAction(tr("Select all"), this);
                                pPlaneSel->setData(true);
                                connect(pPlaneSel, SIGNAL(triggered()), SLOT(onToggleAllPlaneSel()));

                                QAction *pPlaneDesel = new QAction(tr("De-select all"), this);
                                pPlaneDesel->setData(false);
                                connect(pPlaneDesel, SIGNAL(triggered()), SLOT(onToggleAllPlaneSel()));

                                m_pPlaneMenu->addAction(pPlaneSel);
                                m_pPlaneMenu->addAction(pPlaneDesel);
                            }
                            pPlaneMenuBtn->setMenu(m_pPlaneMenu);
                        }
                        pActionsLayout->addStretch();
                        pActionsLayout->addWidget(pPlaneMenuBtn);
                    }

                    m_pPlaneSplitter = new QSplitter(Qt::Vertical, this);
                    {
                        m_pptoPlaneProps = new PlainTextOutput;
                        m_pPlaneSplitter->addWidget(m_pcpPlaneTable);
                        m_pPlaneSplitter->addWidget(m_pptoPlaneProps);
                    }
                    pPlaneFrameLayout->addLayout(pActionsLayout);
                    pPlaneFrameLayout->addWidget(m_pPlaneSplitter);
                }
                pPlaneFrame->setLayout(pPlaneFrameLayout);
            }

            QFrame *pWPolarFrame = new QFrame;
            {
                QVBoxLayout *pWPolarFrameLayout = new QVBoxLayout;
                {
                    QHBoxLayout* pTitleBarLayout = new QHBoxLayout;
                    {
                        m_pleWPolarDir = new QLineEdit;
                        m_pleWPolarDir->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
                        QFileSystemModel *m_pDirModel = new QFileSystemModel;
                        m_pDirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
                        m_pDirModel->setRootPath(SaveOptions::xmlWPolarDirName());
                        QCompleter *m_pCompleter = new QCompleter(this);
                        m_pCompleter->setMaxVisibleItems(20);
                        m_pCompleter->setModel(m_pDirModel);
                        m_pleWPolarDir->setCompleter(m_pCompleter);

                        QPushButton *pWPolarMenuBtn = new QPushButton(tr("Actions"));
                        {
                            pWPolarMenuBtn->setDefault(false);
                            pWPolarMenuBtn->setAutoDefault(false);
                            QMenu *m_pWPolarMenu = new QMenu(this);
                            {
                                int senddata = -1;

                                QAction *pSelDirectory = new QAction(tr("Change directory"), this);
                                connect(pSelDirectory, SIGNAL(triggered()), this, SLOT(onWPolarXmlDir()));

                                QAction *pRefreshDirectory = new QAction(tr("Refresh list"), this);
                                connect(pRefreshDirectory, SIGNAL(triggered()), this, SLOT(onListDirAnalyses()));

                                QAction *pAnalysisSel = new QAction(tr("Select all"), this);
                                pAnalysisSel->setData(true);
                                connect(pAnalysisSel, SIGNAL(triggered()), SLOT(onToggleAllAnalysisSel()));

                                QAction *pAnalysisDesel = new QAction(tr("De-select all"), this);
                                pAnalysisDesel->setData(false);
                                connect(pAnalysisDesel, SIGNAL(triggered()), SLOT(onToggleAllAnalysisSel()));

                                QAction *pDefineWPolar = new QAction(tr("Add a T123 analysis"), this);
                                senddata = 0;
                                pDefineWPolar->setData(senddata);
                                pDefineWPolar->setShortcut(Qt::Key_F6);
                                connect(pDefineWPolar,          SIGNAL(triggered()), this, SLOT(onDefineWPolar()));

                                QAction *pDefineControlPolar = new QAction(tr("Add a T6 analysis"), this);
                                senddata = 1;
                                pDefineControlPolar->setData(senddata);
                                pDefineControlPolar->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F6));;
                                connect(pDefineControlPolar,    SIGNAL(triggered()), this, SLOT(onDefineWPolar()));

                                QAction *pDefineStabPolar = new QAction(tr("Add a T7 analysis"), this);
                                senddata = 2;
                                pDefineStabPolar->setData(senddata);
                                pDefineStabPolar->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F6));
                                connect(pDefineStabPolar,       SIGNAL(triggered()), this, SLOT(onDefineWPolar()));

                                m_pWPolarMenu->addAction(pSelDirectory);
                                m_pWPolarMenu->addAction(pRefreshDirectory);
                                m_pWPolarMenu->addSeparator();
                                m_pWPolarMenu->addAction(pAnalysisSel);
                                m_pWPolarMenu->addAction(pAnalysisDesel);
                                m_pWPolarMenu->addSeparator();
                                m_pWPolarMenu->addAction(pDefineWPolar);
                                m_pWPolarMenu->addAction(pDefineControlPolar);
                                m_pWPolarMenu->addAction(pDefineStabPolar);
                            }
                            pWPolarMenuBtn->setMenu(m_pWPolarMenu);
                        }
                        pTitleBarLayout->addWidget(m_pleWPolarDir);
                        pTitleBarLayout->addWidget(pWPolarMenuBtn);

                    }

                    m_pWPolarSplitter = new QSplitter(Qt::Vertical, this);
                    {
                        m_pptoAnalysisProps = new PlainTextOutput;

                        m_pWPolarSplitter->addWidget(m_pcpAnalysisTable);
                        m_pWPolarSplitter->addWidget(m_pptoAnalysisProps);
                    }

                    pWPolarFrameLayout->addLayout(pTitleBarLayout);
                    pWPolarFrameLayout->addWidget(m_pWPolarSplitter);
                }
                pWPolarFrame->setLayout(pWPolarFrameLayout);
            }

            QFrame *pRightFrame = new QFrame;
            {
                QVBoxLayout *pRangeFrameLayout = new QVBoxLayout;
                {
                    QVBoxLayout *pTopLayout = new QVBoxLayout;
                    {
                        m_pTabWidget = new QTabWidget;
                        {
                            m_pT12RangeTable = new AnalysisRangeTable(this);
                            m_pT12RangeTable->setName("BatchPlaneDlg::t12");
                            m_pT12RangeTable->setPolarType(xfl::T1POLAR);
                            m_pTabWidget->addTab(m_pT12RangeTable, "T12");

                            m_pT3RangeTable = new AnalysisRangeTable(this);
                            m_pT3RangeTable->setName("BatchPlaneDlg::t3");
                            m_pT3RangeTable->setPolarType(xfl::T3POLAR);
                            m_pTabWidget->addTab(m_pT3RangeTable, "T3");

                            m_pT6RangeTable = new AnalysisRangeTable(this);
                            m_pT6RangeTable->setName("T6");
                            m_pT6RangeTable->setPolarType(xfl::T6POLAR);
                            m_pTabWidget->addTab(m_pT6RangeTable, "T6");

                            m_pT7RangeTable = new AnalysisRangeTable(this);
                            m_pT7RangeTable->setName("BatchPlaneDlg::t7");
                            m_pT7RangeTable->setPolarType(xfl::T7POLAR);
                            m_pTabWidget->addTab(m_pT7RangeTable, "T7");

                            m_pTXRangeTable = new T8RangeTable(this);
                            m_pTXRangeTable->setName("BatchPlaneDlg::t8");
                            m_pTabWidget->addTab(m_pTXRangeTable, "T8");

                            m_pTabWidget->setCurrentIndex(s_LastTabIndex);
                        }

                        m_pchStorePOpps  = new QCheckBox(tr("Store operating points"));
                        m_pchStorePOpps->setChecked(s_bStorePOpps);


                        pTopLayout->addWidget(m_pTabWidget);
                        pTopLayout->addWidget(m_pchStorePOpps);
                        pTopLayout->setStretchFactor(m_pTabWidget,1);
                    }

                    m_pptoAnalysisOutput = new PlainTextOutput;
                    QVBoxLayout *pStatusBtnLayout = new QVBoxLayout;
                    {
                        m_plabStatus = new QLabel(tr("Not running."));
                        m_plabStatus->setFont(DisplayOptions::tableFont());
                        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
                        {
                            QPushButton *ppbClear = new QPushButton(tr("Clear output"));
                            connect(ppbClear, SIGNAL(clicked()), m_pptoAnalysisOutput, SLOT(clear()));
                            m_ppbAnalyze =  new QPushButton(tr("Calculate"));
                            m_ppbAnalyze->setDefault(true);
                            m_ppbAnalyze->setAutoDefault(true);
                            m_pButtonBox->addButton(ppbClear, QDialogButtonBox::ActionRole);
                            m_pButtonBox->addButton(m_ppbAnalyze, QDialogButtonBox::ActionRole);
                            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
                        }
                        pStatusBtnLayout->addWidget(m_plabStatus);
                        pStatusBtnLayout->addStretch();
                        pStatusBtnLayout->addWidget(m_pButtonBox);
                    }

                    pRangeFrameLayout->addLayout(pTopLayout);
                    pRangeFrameLayout->addWidget(m_pptoAnalysisOutput);
                    pRangeFrameLayout->addLayout(pStatusBtnLayout);
                    pRangeFrameLayout->setStretchFactor(pTopLayout, 3);
                    pRangeFrameLayout->setStretchFactor(m_pptoAnalysisOutput, 6);
                    pRangeFrameLayout->setStretchFactor(pStatusBtnLayout, 0);
                }
                pRightFrame->setLayout(pRangeFrameLayout);
            }
            m_pHSplitter->addWidget(pPlaneFrame);
            m_pHSplitter->addWidget(pWPolarFrame);
            m_pHSplitter->addWidget(pRightFrame);
            m_pHSplitter->setChildrenCollapsible(false);
        }

        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);
}


void BatchXmlDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton) reject();
    else if(pButton == m_ppbAnalyze) onAnalyze();
}


void BatchXmlDlg::reject()
{
    if(m_pExecutor && m_pExecutor->isRunning())
    {
        m_pExecutor->onCancel();
    }

    QDialog::reject();
}


void BatchXmlDlg::connectSignals()
{
    connect(m_pcpPlaneTable,     SIGNAL(pressed(QModelIndex)),     SLOT(onPlaneClicked(QModelIndex)));
    connect(m_pcpAnalysisTable,  SIGNAL(pressed(QModelIndex)),     SLOT(onAnalysisClicked(QModelIndex)));
    connect(m_pcpAnalysisTable,  SIGNAL(doubleClick(QModelIndex)), SLOT(onAnalysisDoubleClicked(QModelIndex)));
    connect(m_pHSplitter,        SIGNAL(splitterMoved(int,int)),   SLOT(onResizeColumns()));
    connect(m_pchStorePOpps,     SIGNAL(clicked(bool)),            SLOT(onStorePOpps()));
    connect(m_pleWPolarDir,      SIGNAL(editingFinished()),        SLOT(onListDirAnalyses()));
}


void BatchXmlDlg::onStorePOpps()
{
    s_bStorePOpps = m_pchStorePOpps->isChecked();
    if(m_pExecutor) m_pExecutor->setMakePOpps(s_bStorePOpps);
}


void BatchXmlDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);

    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    if(s_SplitterPlaneSizes.length()>0) m_pPlaneSplitter->restoreState(s_SplitterPlaneSizes);
    if(s_SplitterWPolarSizes.length()>0) m_pWPolarSplitter->restoreState(s_SplitterWPolarSizes);

    initDialog();
    onResizeColumns();
}


void BatchXmlDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_bStorePOpps  = m_pchStorePOpps->isChecked();
    s_LastTabIndex = m_pTabWidget->currentIndex();
    s_Geometry = saveGeometry();

    s_HSplitterSizes  = m_pHSplitter->saveState();
    s_SplitterPlaneSizes  = m_pPlaneSplitter->saveState();
    s_SplitterWPolarSizes = m_pWPolarSplitter->saveState();
}


void BatchXmlDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("BatchXmlDlg");
    {
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();
        s_HSplitterSizes      = settings.value("HSplitterSizes").toByteArray();
        s_SplitterPlaneSizes  = settings.value("PlaneSplitterSizes").toByteArray();
        s_SplitterWPolarSizes = settings.value("WPolarSplitterSizes").toByteArray();
    }
    settings.endGroup();
}


void BatchXmlDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("BatchXmlDlg");
    {
        settings.setValue("WindowGeom", s_Geometry);

        settings.setValue("HSplitterSizes",      s_HSplitterSizes);
        settings.setValue("PlaneSplitterSizes",  s_SplitterPlaneSizes);
        settings.setValue("WPolarSplitterSizes", s_SplitterWPolarSizes);
    }
    settings.endGroup();
}


void BatchXmlDlg::onWPolarXmlDir()
{
    QString dirName = QFileDialog::getExistingDirectory(this, tr("Select directory"),
                                                        SaveOptions::xmlWPolarDirName(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dirName.length())
    {
        SaveOptions::setXmlWPolarDirName(dirName);
        m_pleWPolarDir->setText(SaveOptions::xmlWPolarDirName());
        onListDirAnalyses();
    }
}


void BatchXmlDlg::onToggleAllPlaneSel()
{
    QAction *pAction = qobject_cast<QAction *>(sender());
    if(!pAction) return;

    bool bActive = pAction->data().toBool();

    for(int row=0; row<m_pPlaneModel->rowCount(); row++)
    {
        QModelIndex index0 = m_pPlaneModel->index(row, 0);
        QModelIndex index1 = m_pPlaneModel->index(row, 1);
        Plane const *pPlane = Objects3d::planeAt(m_pPlaneModel->data(index1).toString().toStdString());
        if(!pPlane) continue;

        pPlane->setActive(bActive);
        m_pPlaneModel->setData(index0, bActive, Qt::UserRole);
    }
    m_pPlaneModel->updateData();
    update();
}


void BatchXmlDlg::onToggleAllAnalysisSel()
{
    QAction *pAction = qobject_cast<QAction *>(sender());
    if(!pAction) return;

    bool bActive = pAction->data().toBool();

    for(int row=0; row<m_pAnalysisModel->rowCount(); row++)
    {
        QModelIndex index0 = m_pAnalysisModel->index(row, 0);
        QModelIndex index1 = m_pAnalysisModel->index(row, 1);
        s_Analyses[m_pAnalysisModel->data(index1).toString()] = bActive;
        m_pAnalysisModel->setData(index0, bActive, Qt::UserRole);
    }
    m_pAnalysisModel->updateData();
    update();
}


void BatchXmlDlg::onPlaneClicked(QModelIndex const &index)
{
    if(!index.isValid()) return;

    QModelIndex sibling = m_pPlaneModel->index(index.row(), 1);
    QString name = m_pPlaneModel->data(sibling).toString();

    Plane const *pPlane = Objects3d::plane(name.toStdString());
    if(!pPlane) return;

    m_pptoPlaneProps->setStdText(pPlane->planeData(false) + "\n\n" + pPlane->description());

    if(index.column()==0)
    {
        // toggle active flag
        pPlane->setActive(!pPlane->isActive());
        m_pPlaneModel->setData(index, pPlane->isActive(), Qt::UserRole);
        m_pPlaneModel->updateData();
    }
    update();
}


void BatchXmlDlg::onAnalysisClicked(QModelIndex const &index)
{
    if(!index.isValid()) return;

    QModelIndex sibling = m_pAnalysisModel->index(index.row(), 1);
    QString name = m_pAnalysisModel->data(sibling).toString();

    QString pathname = SaveOptions::xmlWPolarDirName() + QDir::separator()+name;

    PlanePolar *pWPolar = readXmlWPolarFile(pathname);

    updateAnalysisProperties(pWPolar);

    if(index.column()==0)
    {
        // toggle active flag
        bool bIsActive = s_Analyses.value(name);
        bIsActive = !bIsActive;
        s_Analyses[name] = bIsActive;
        m_pAnalysisModel->setData(index, bIsActive, Qt::UserRole);
        m_pAnalysisModel->updateData();
    }
    else if(index.column() == m_pAnalysisModel->actionColumn())
    {
        QRect itemrect = m_pcpAnalysisTable->visualRect(index);
        QPoint menupos = m_pcpAnalysisTable->mapToGlobal(itemrect.topLeft());
        QMenu *pRowMenu = new QMenu(tr("Analysis"),this);
        {
            QAction *pEditAnalysis = new QAction(tr("Edit analysis"), this);
            connect(pEditAnalysis, SIGNAL(triggered(bool)), SLOT(onEditAnalysis()));

            QAction *pClearPlaneName = new QAction(tr("Clear plane name"), this);
            connect(pClearPlaneName, SIGNAL(triggered(bool)), SLOT(onClearPlaneName()));

            QAction *pRenameFile = new QAction(tr("Rename"), this);
            connect(pRenameFile, SIGNAL(triggered(bool)), SLOT(onRenameFile()));

            QAction *pDuplicateFile = new QAction(tr("Duplicate"), this);
            connect(pDuplicateFile, SIGNAL(triggered(bool)), SLOT(onDuplicateFile()));

            QAction *pDeleteFile = new QAction(tr("Delete"), this);
            connect(pDeleteFile, SIGNAL(triggered(bool)), SLOT(onDeleteFile()));

            pRowMenu->addAction(pClearPlaneName);
            pRowMenu->addAction(pEditAnalysis);
            pRowMenu->addSeparator();
            QMenu *pFileMenu = pRowMenu->addMenu(tr("File"));
            {
                pFileMenu->addAction(pRenameFile);
                pFileMenu->addAction(pDuplicateFile);
                pFileMenu->addAction(pDeleteFile);
            }
        }
        pRowMenu->exec(menupos);
    }
}


void BatchXmlDlg::onAnalysisDoubleClicked(QModelIndex const &index)
{
    if(!index.isValid()) return;

    QModelIndex sibling = m_pAnalysisModel->index(index.row(), 1);
    QString name = m_pAnalysisModel->data(sibling).toString();

    QString pathname = SaveOptions::xmlWPolarDirName() + QDir::separator()+name;

    PlanePolar *pWPolar = readXmlWPolarFile(pathname);

    updateAnalysisProperties(pWPolar);
    onEditAnalysis();
}


void BatchXmlDlg::updateAnalysisProperties(PlanePolar const *pWPolar)
{
    if(pWPolar)
    {
        std::string strangeprops;
        pWPolar->getProperties(strangeprops, nullptr);
        QString strange;
        strange  = "Plane name:    " + QString::fromStdString(pWPolar->planeName())+"\n";
        strange += "Analysis name: " + QString::fromStdString(pWPolar->name())+"\n";
        m_pptoAnalysisProps->setPlainText(strange);
        m_pptoAnalysisProps->onAppendStdText(strangeprops);
        m_pptoAnalysisProps->moveCursor(QTextCursor::Start);
        m_pptoAnalysisProps->ensureCursorVisible();

        delete pWPolar; // no further use
    }
    else
        m_pptoAnalysisProps->clear();
}



void BatchXmlDlg::onRenameFile()
{
    QModelIndexList indexList = m_pcpAnalysisTable->selectionModel()->selectedIndexes();

    int row = -1;
    if(indexList.size()>0) row = indexList.at(0).row();
    if(row<0 || row>s_Analyses.size()) return;

    QModelIndex sibling = indexList.front().sibling(indexList.front().row(), 1);
    QString name = m_pAnalysisModel->data(sibling).toString();
    QString filename = SaveOptions::xmlWPolarDirName() + QDir::separator() + name;

    NewNameDlg dlg(name, this);
    if(dlg.exec() != QDialog::Accepted) return;

    QString newfilename = SaveOptions::xmlWPolarDirName() + QDir::separator() + dlg.newName();

    if(!newfilename.endsWith(".xml", Qt::CaseInsensitive)) newfilename += ".xml";


    QFileInfo finew(newfilename);
    if(finew.exists())
    {
        int resp = QMessageBox::question(this, tr("Exit"), tr("Replace the existing file?"),
                                         QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                                         QMessageBox::Yes);
        if(resp!=QMessageBox::Yes) return;
    }

    QFile::rename(filename, newfilename);

    s_Analyses[dlg.newName()] = s_Analyses[name];
    s_Analyses.remove(name);
    fillAnalysisModel();
}


void BatchXmlDlg::onDuplicateFile()
{
    QModelIndexList indexList = m_pcpAnalysisTable->selectionModel()->selectedIndexes();

    int row = -1;
    if(indexList.size()>0) row = indexList.at(0).row();
    if(row<0 || row>s_Analyses.size()) return;

    QModelIndex sibling = indexList.front().sibling(indexList.front().row(), 1);
    QString name = m_pAnalysisModel->data(sibling).toString();
    QString filename = SaveOptions::xmlWPolarDirName() + QDir::separator() + name;

    QString newfilename = filename;

    QString filter = tr("XML file (*.xml)");
    newfilename = QFileDialog::getSaveFileName(this, tr("Export analysis definition to xml file"),
                                               newfilename,
                                               filter,
                                               &filter);
    if(!newfilename.length()) return;

    PlanePolar *pWPolar = readXmlWPolarFile(filename);
    if (!pWPolar)
    {
        QMessageBox::warning(this, tr("Write error"), tr("Failed to read the file") + filename);
        return;
    }

    QFile XFile(newfilename);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Write error"), tr("Failed to save the new file"));
        return;
    }

    QFileInfo finew(newfilename);

    XmlPlanePolarWriter wpolarWriter(XFile);
    wpolarWriter.writeXMLWPolar(pWPolar);

    s_Analyses[finew.fileName()] = true;
    fillAnalysisModel();
}


void BatchXmlDlg::onDeleteFile()
{
    QModelIndexList indexList = m_pcpAnalysisTable->selectionModel()->selectedIndexes();

    int row = -1;
    if(indexList.size()>0) row = indexList.at(0).row();
    if(row<0 || row>s_Analyses.size()) return;

    QModelIndex sibling = indexList.front().sibling(indexList.front().row(), 1);
    QString name = m_pAnalysisModel->data(sibling).toString();
    int resp = QMessageBox::question(this, tr("Exit"), tr("Really delete this file?"),
                                     QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                                     QMessageBox::Yes);
    if(resp!=QMessageBox::Yes) return;

    QString filename = SaveOptions::xmlWPolarDirName() + QDir::separator() + name;
    QFile file(filename);
    file.remove();
    s_Analyses.remove(name);
    fillAnalysisModel();
}


void BatchXmlDlg::onClearPlaneName()
{
    QModelIndexList indexList = m_pcpAnalysisTable->selectionModel()->selectedIndexes();

    int row = -1;
    if(indexList.size()>0) row = indexList.at(0).row();
    if(row<0 || row>s_Analyses.size()) return;

    QModelIndex sibling = indexList.front().sibling(indexList.front().row(), 1);
    QString name = m_pAnalysisModel->data(sibling).toString();
    QString filepath = SaveOptions::xmlWPolarDirName() + QDir::separator() + name;
    QFile file(filepath);

    PlanePolar *pWPolar = readXmlWPolarFile(filepath);
    if(!pWPolar) return;

    pWPolar->setPlaneName("");

    if(!filepath.length()) return;


    QFile XFile(filepath);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    XmlPlanePolarWriter wpolarWriter(XFile);
    wpolarWriter.writeXMLWPolar(pWPolar);

    XFile.close();
    updateAnalysisProperties(pWPolar);
}


void BatchXmlDlg::onEditAnalysis()
{
    QModelIndexList indexList = m_pcpAnalysisTable->selectionModel()->selectedIndexes();

    int row = -1;
    if(indexList.size()>0) row = indexList.at(0).row();
    if(row<0 || row>s_Analyses.size()) return;

    QModelIndex sibling = indexList.front().sibling(indexList.front().row(), 1);
    QString name = m_pAnalysisModel->data(sibling).toString();
    QString filepath = SaveOptions::xmlWPolarDirName() + QDir::separator() + name;
    QFile file(filepath);

    PlanePolar *pWPolar = readXmlWPolarFile(filepath);
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

    if(!filepath.length()) return;


    QFile XFile(filepath);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    XmlPlanePolarWriter wpolarWriter(XFile);
    wpolarWriter.writeXMLWPolar(&PlanePolarDlg::staticWPolar());

    XFile.close();

    updateAnalysisProperties(pWPolar);
}


void BatchXmlDlg::onListDirAnalyses()
{
    SaveOptions::setXmlWPolarDirName(m_pleWPolarDir->text());
    QStringList files = xfl::findFiles(SaveOptions::xmlWPolarDirName(), {"*.xml"}, false);

    // read the files, just to check if they are valid WPolars
    QStringList validnames;
    for(int i=0; i<files.size(); i++)
    {
        PlanePolar *pWPolar = readXmlWPolarFile(files.at(i)); // returns a valid pointer if successful, null otherwise
        if(pWPolar)
        {
            QFileInfo fi(files.at(i));
            validnames.append(fi.fileName());
            delete pWPolar;
        }
    }
    validnames.sort();

    // retrieve the existing status if compatible -
    QMap<QString, bool> analyses = s_Analyses;

    s_Analyses.clear();
    for(QString const&name : validnames)
    {
        s_Analyses.insert(name, false);
        if(analyses.contains(name))
        {
            s_Analyses[name] = analyses[name];
        }
    }

    fillAnalysisModel();
}


void BatchXmlDlg::fillAnalysisModel()
{
    m_pAnalysisModel->setRowCount(s_Analyses.size());

    QMapIterator<QString, bool> it(s_Analyses);
    int row = 0;
    while (it.hasNext())
    {
        it.next();

        QModelIndex ind;

        ind = m_pAnalysisModel->index(row, 0, QModelIndex());
        m_pAnalysisModel->setData(ind, it.value(), Qt::UserRole);

        ind = m_pAnalysisModel->index(row, 1, QModelIndex());
        m_pAnalysisModel->setData(ind, it.key());

        row++;
    }
}


void BatchXmlDlg::fillPlaneModel()
{
    m_pPlaneModel->setRowCount(Objects3d::nPlanes());
    for(int ip=0; ip<Objects3d::nPlanes(); ip++)
    {
        QModelIndex ind;
        for(int ip=0; ip<Objects3d::nPlanes(); ip++)
        {
            Plane const *pPlane = Objects3d::planeAt(ip);
            ind = m_pPlaneModel->index(ip, 0, QModelIndex());
            m_pPlaneModel->setData(ind, pPlane->isActive(), Qt::UserRole); // used to draw the checkbox

            ind = m_pPlaneModel->index(ip, 1, QModelIndex());
            m_pPlaneModel->setData(ind, QString::fromStdString(pPlane->name()));
        }
    }
}


PlanePolar * BatchXmlDlg::readXmlWPolarFile(QString path)
{
    QFile xmlFile(path);
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
//        QString strange = "Could not open the file "+xmlFile.fileName() +"\n";
        return nullptr;
    }


//        s_pMainFrame->displayMessage("Importing xml file " + fi.fileName() + "\n");

    XmlPlanePolarReader wpolarreader(xmlFile);
    wpolarreader.readXMLPolarFile();

    if(wpolarreader.hasError())
    {
        QString errorMsg = wpolarreader.errorString() + QString("\nline %1 column %2").arg(wpolarreader.lineNumber()).arg(wpolarreader.columnNumber());
        errorMsg +="\n";
//            s_pMainFrame->displayMessage(errorMsg);
//            s_pMainFrame->onShowLogWindow(true);
        return nullptr;
    }  
    return wpolarreader.wpolar();
}


void BatchXmlDlg::initDialog()
{
    m_pleWPolarDir->setText(SaveOptions::xmlWPolarDirName());

    onListDirAnalyses();

    fillPlaneModel();

    m_pT12RangeTable->fillTable();
    m_pT3RangeTable->fillTable();
    m_pT6RangeTable->fillTable();
    m_pT7RangeTable->fillTable();
    m_pTXRangeTable->fillRangeTable();
    m_pButtonBox->setFocus();
}


void BatchXmlDlg::onResizeColumns()
{
    int nCols = m_pPlaneModel->columnCount()-1;
    QHeaderView *pHHeader = m_pcpPlaneTable->horizontalHeader();
    pHHeader->setSectionResizeMode(nCols, QHeaderView::Stretch);
    pHHeader->resizeSection(nCols, 1); // 1 pixel to be resized automatically
    double w = double(m_pcpPlaneTable->width());
    int w0 = int(w/7.0);
    m_pcpPlaneTable->setColumnWidth(0, w0);

    nCols = m_pAnalysisModel->columnCount()-1;
    pHHeader = m_pcpAnalysisTable->horizontalHeader();
    pHHeader->resizeSection(0, 1); // 1 pixel to be resized automatically
    pHHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    pHHeader->resizeSection(2, 1); // 1 pixel to be resized automatically

    w = double(m_pcpAnalysisTable->width());
    w0 = int(w/10.0);

    m_pcpAnalysisTable->setColumnWidth(0, w0);
    m_pcpAnalysisTable->setColumnWidth(2, w0);

    update();
}


void BatchXmlDlg::onDefineWPolar()
{
    Polar3dDlg *pDlg = nullptr;
    QAction *pAction = qobject_cast<QAction *>(sender());
    if(!pAction) return;

    int id = pAction->data().toInt();
    switch(id)
    {
        case 0:
        {
            T1234578PolarDlg *pWDlg = new T1234578PolarDlg(this);
            pWDlg->initPolar3dDlg(nullptr);
            pDlg = pWDlg;
            break;
        }
        case 1:
        {
            T6PolarDlg *pCDlg = new T6PolarDlg(this);
            pCDlg->initPolar3dDlg(nullptr, nullptr);
            pDlg = pCDlg;
            break;
        }
        case 2:
        {
/*            StabPolarDlg *pSDlg = new StabPolarDlg(this);
            pSDlg->initPolar3dDlg(nullptr, nullptr);
            pDlg = pSDlg;*/
            break;
        }
        default: return;
    }

    if (pDlg->exec() != QDialog::Accepted) return;


    PlanePolar* pNewWPolar  = new PlanePolar;
    pNewWPolar->duplicateSpec(&PlanePolarDlg::staticWPolar());
    pNewWPolar->setPlaneName("");
    pNewWPolar->setName(PlanePolarDlg::staticWPolar().name());


    pNewWPolar->setReferenceChordLength(1.0);
    pNewWPolar->setReferenceSpanLength(1.0);
    pNewWPolar->setReferenceArea(1.0);

    pNewWPolar->setVisible(true);


    //export the polar to the xml format in  the target directory

    QString filter = tr("XML file (*.xml)");
    QString FileName, strong;

//    strong = m_pCurPlane->planeName()+"_"+m_pCurWPolar->polarName();
    strong = QString::fromStdString(pNewWPolar->name());
    strong.replace("/", "_");
    strong.replace(".", "_");

    QString pathname = SaveOptions::xmlWPolarDirName()+ QDir::separator() +strong;

    do
    {
        FileName = QFileDialog::getSaveFileName(this, tr("Export analysis definition to xml file"),
                                                pathname,
                                                filter,
                                                &filter);

        if(FileName.length())
        {
            if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";
            QFileInfo fileInfo(FileName);

            QFile XFile(FileName);
            if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

            XmlPlanePolarWriter wpolarWriter(XFile);
            wpolarWriter.writeXMLWPolar(pNewWPolar);

            XFile.close();

            // addd the polar to the list of polar names
            s_Analyses[fileInfo.fileName()] = false;
            fillAnalysisModel();
        }
        else
        {
            int resp = QMessageBox::question(this, tr("Exit"), tr("Discard the analysis ")+QString::fromStdString(pNewWPolar->name())+" ?",
                                             QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                                             QMessageBox::No);
            if(resp == QMessageBox::Yes)
                break;
        }
    }
    while(!FileName.length());

    delete pNewWPolar;
}


/**
* Updates the text output in the dialog box and the log file.
* @param msg the text message to append to the output widget and to the log file.
*/
void BatchXmlDlg::onMessage(QString const &msg)
{
    m_pptoAnalysisOutput->onAppendQText(msg);
    m_pptoAnalysisOutput->moveCursor(QTextCursor::End);
    m_pptoAnalysisOutput->ensureCursorVisible();
    
}


void BatchXmlDlg::onAnalyze()
{
    if(m_pExecutor)
    {
        onMessage("         -----Analysis cancel request-----   \n\n");
        m_pExecutor->onCancel();
        return;
    }

    QString strange, log;
    m_bChanged = true;

    s_bStorePOpps  = m_pchStorePOpps->isChecked();

    // read operating ranges
    std::vector<double> t12opps, t3opps, t6opps, t7opps;
    log = "T12 operating range:\n";
    for(int i=0; i<AnalysisRangeTable::t12Range().size(); i++)
    {
        AnalysisRange const &range = AnalysisRangeTable::t12Range().at(i);
        if(range.isActive())
        {
            t12opps.insert(t12opps.end(), range.values().begin(), range.values().end());
            strange = QString::asprintf("   min=%11.3g   max=%11.3g   inc=%11.3g\n", range.m_vStart, range.m_vEnd, range.m_vInc);
            log += strange;
        }
    }
    log += "T3 operating range:\n";
    for(int i=0; i<AnalysisRangeTable::t3Range().size(); i++)
    {
        AnalysisRange const &range = AnalysisRangeTable::t3Range().at(i);
        if(range.isActive())
        {
            t3opps.insert(t3opps.end(), range.values().begin(), range.values().end());
            strange = QString::asprintf("   min=%11.3g   max=%11.3g   inc=%11.3g\n", range.m_vStart, range.m_vEnd, range.m_vInc);
            log += strange;
        }
    }
    log += "T6 operating range:\n";
    for(int i=0; i<AnalysisRangeTable::t6Range().size(); i++)
    {
        AnalysisRange const &range = AnalysisRangeTable::t6Range().at(i);
        if(range.isActive())
        {
            t6opps.insert(t6opps.end(), range.values().begin(), range.values().end());
            strange = QString::asprintf("   min=%11.3g   max=%11.3g   inc=%11.3g\n", range.m_vStart, range.m_vEnd, range.m_vInc);
            log += strange;
        }
    }
    log += "T7 operating range:\n";
    for(int i=0; i<AnalysisRangeTable::t7Range().size(); i++)
    {
        AnalysisRange const &range = AnalysisRangeTable::t7Range().at(i);
        if(range.isActive())
        {
            t7opps.insert(t7opps.end(), range.values().begin(), range.values().end());
            strange = QString::asprintf("   min=%11.3g   max=%11.3g   inc=%11.3g\n", range.m_vStart, range.m_vEnd, range.m_vInc);
            log += strange;
        }
    }

    std::vector<T8Opp> xrange;
    log += "TX operating range:\n";
    for(uint i=0; i<T8RangeTable::t8range().size(); i++)
    {
        T8Opp const &range = T8RangeTable::t8range().at(i);
        if(range.isActive())
        {
            xrange.push_back(range);
            strange = QString::asprintf("   alpha=%.3g   beta=%.3g   VInf=%.3g", range.alpha(), range.beta(), range.Vinf()*Units::mstoUnit());
            log += strange + Units::lengthUnitQLabel()+"\n";
        }
    }

    if(t12opps.size()==0 && t3opps.size()==0 && t6opps.size()==0 && t7opps.size()==0 && xrange.size()==0)
    {
        onMessage("No operating points selected for analysis - aborting.\n\n");
        return;
    }

    QList<Plane*> planelist;
    for(int row=0; row<m_pPlaneModel->rowCount(); row++)
    {
        QModelIndex index1 = m_pPlaneModel->index(row, 1);
        Plane *pPlane = Objects3d::plane(m_pPlaneModel->data(index1).toString().toStdString());
        if(!pPlane) continue;
        if(pPlane->isActive() && pPlane->isXflType())
        {
            PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(pPlane);
            planelist.append(pPlaneXfl);

            if(!pPlaneXfl->isInitialized())
            {
                pPlaneXfl->makePlane(true, false, true);
            }
        }
    }
    if(planelist.isEmpty())
    {
        onMessage("No planes to analyze - aborting\n\n");
        return;
    }
    else
    {
        strange = "Planes to analyze:\n";
        for(int ip=0; ip<planelist.size(); ip++)
            strange += "   " + QString::fromStdString(planelist.at(ip)->name()) + "\n";
        onMessage(strange+"\n");
    }

    onMessage("\n");


    //run
    m_pExecutor = new XflExecutor();
    m_pExecutor->setEventDestination(nullptr);

    QString logFileName = SaveOptions::newLogFileName();
    SaveOptions::setLastLogFileName(logFileName);
    m_pExecutor->setLogFile(logFileName, QString::fromStdString(fl5::versionName(true)));
    m_pExecutor->setMakePOpps(s_bStorePOpps);
    m_pExecutor->setT12Range(AnalysisRangeTable::t12Range());
    m_pExecutor->setT3Range( AnalysisRangeTable::t3Range());
    m_pExecutor->setT6Range( AnalysisRangeTable::t6Range());
    m_pExecutor->setT7Range( AnalysisRangeTable::t7Range());
    m_pExecutor->setT8Range( T8RangeTable::t8range());
    m_pExecutor->setPlanes(planelist);

    m_pExecutor->makeWPolars(s_Analyses, SaveOptions::xmlWPolarDirName(), strange);
    onMessage(strange);

    m_pExecutor->makePlaneTasks(strange);
    onMessage(strange);

    m_pButtonBox->button(QDialogButtonBox::Close)->setEnabled(false);
    m_ppbAnalyze->setText(tr("Cancel"));

    Task3d::setCancelled(false);
    TriMesh::setCancelled(false);

    onMessage(QString::fromStdString(fl5::versionName(true)) + "\n");
    QDateTime dt = QDateTime::currentDateTime();
    QString str = dt.toString();
    onMessage(str+"\n\n");

    m_bHasErrors = false;

    //run the instance asynchronously
    QThread *pThread = new QThread;
    m_pExecutor->moveToThread(pThread);
    connect(pThread,     &QThread::started,           m_pExecutor,  &XflExecutor::onRunExecutor);
    connect(pThread,     &QThread::finished,          pThread,      &QThread::deleteLater);
    connect(m_pExecutor, &XflExecutor::taskStarted,   this,         &BatchXmlDlg::onPlaneTaskStarted);
    connect(m_pExecutor, &XflExecutor::taskFinished,  this,         &BatchXmlDlg::onAnalysisFinished);
    connect(m_pExecutor, &XflExecutor::outputMessage, this,         &BatchXmlDlg::onMessage);
    pThread->start(xfl::threadPriority());

}


void BatchXmlDlg::onAnalysisFinished()
{
    if(m_pExecutor)
    {
        m_pExecutor->thread()->exit(0);
        delete m_pExecutor;
    }
    m_pExecutor = nullptr;
    onMessage("_____Plane analyses completed_____\n\n");

    m_plabStatus->setText("Not running.");
    m_ppbAnalyze->setText("Calculate");
    m_pButtonBox->button(QDialogButtonBox::Close)->setEnabled(true);
}


void BatchXmlDlg::onPlaneTaskStarted(int iTask)
{
    if(!m_pExecutor) return;
    int total = m_pExecutor->planeTasks().size();
    QString strange;

    strange = QString::asprintf("%d/%d ", iTask+1, total);
    Task3d *pTask = m_pExecutor->planeTasks().at(iTask);

    PlaneTask const *pPlaneTask = dynamic_cast<PlaneTask const*>(pTask);
    if(pPlaneTask)
        strange += QString::fromStdString(pPlaneTask->plane()->name() + " / "+pPlaneTask->wPolar()->name());

    LLTTask *pLLTTask = dynamic_cast<LLTTask*>(pTask);
    if(pLLTTask)
        strange += QString::fromStdString(pLLTTask->plane()->name() + " / "+pLLTTask->wPolar()->name());

    m_plabStatus->setText(strange);
}




