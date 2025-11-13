/****************************************************************************

    flow5
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

#include <QDateTime>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QModelIndex>
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrentRun>

#include "xfoilbatchdlg.h"


#include <xfl/controls/analysisrangetable.h>
#include <xfl/xdirect/xdirect.h>
#include <xfl/xdirect/xdirect.h>
#include <xflcore/analysisrange.h>
#include <xflcore/displayoptions.h>
#include <xflcore/flow5events.h>
#include <xflcore/saveoptions.h>
#include <xflcore/xflcore.h>
#include <xflfoil/analysis2d/xfoiltask.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/objects2d.h>
#include <xflfoil/objects2d/oppoint.h>
#include <xflfoil/objects2d/polar.h>
#include <xflwidgets/customwts/actionitemmodel.h>
#include <xflwidgets/customwts/cptableview.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/plaintextoutput.h>
#include <xflwidgets/customwts/xfldelegate.h>


bool XFoilBatchDlg::s_bAlpha    = true;

bool XFoilBatchDlg::s_bInitBL   = false;

xfl::enumPolarType XFoilBatchDlg::s_PolarType = xfl::T1POLAR;

double XFoilBatchDlg::s_XTop   = 1.0;
double XFoilBatchDlg::s_XBot   = 1.0;

bool XFoilBatchDlg::s_bUpdatePolarView = false;
XDirect * XFoilBatchDlg::s_pXDirect;

QVector<bool> XFoilBatchDlg::s_ActiveList;
QVector<double> XFoilBatchDlg::s_ReList;
QVector<double> XFoilBatchDlg::s_MachList;
QVector<double> XFoilBatchDlg::s_NCritList;

QByteArray XFoilBatchDlg::s_Geometry;
QByteArray XFoilBatchDlg::s_HSplitterSizes;


XFoilBatchDlg::XFoilBatchDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Multi-threaded batch analysis");

    setWindowFlag(Qt::WindowMinMaxButtonsHint);
    m_pXFile = nullptr;
    m_pFoil = nullptr;

    m_bCancel         = false;
    m_bIsRunning      = false;

    XFoil::setCancel(false);
    XFoilTask::setSkipOpp(false);
    XFoilTask::setSkipPolar(false);

    connect(&m_Timer, SIGNAL(timeout()), SLOT(onTimerEvent()));

    m_nTaskDone    = 0;
    m_nTaskStarted = 0;
    m_nAnalysis    = 0;

    setupLayout();
    connectBaseSignals();
}


XFoilBatchDlg::~XFoilBatchDlg()
{
    //clean up the rest of the analysis in case of cancellation
    for(int ia=m_AnalysisPair.count()-1; ia>=0; ia--)
    {
        FoilAnalysis *pAnalysis = m_AnalysisPair.last();
        m_AnalysisPair.removeLast();
        delete pAnalysis;
    }

    if(m_pXFile)  delete m_pXFile;
    m_pXFile = nullptr;
}


void XFoilBatchDlg::setupLayout()
{
    m_pHSplitter = new QSplitter(Qt::Horizontal);
    {
        m_pHSplitter->setChildrenCollapsible(false);

        QTabWidget *pLeftTabWt = new QTabWidget;
        {
            m_pfrRangeVars = new QFrame;
            {
                QHBoxLayout *pRangeSpecLayout = new QHBoxLayout;
                {
                    QLabel *Spec = new QLabel("Specify:");
                    m_prbAlpha = new QRadioButton(ALPHACHAR);
                    m_prbCl = new QRadioButton("Cl");
                    pRangeSpecLayout->addWidget(Spec);
                    pRangeSpecLayout->addWidget(m_prbAlpha);
                    pRangeSpecLayout->addWidget(m_prbCl);
                    pRangeSpecLayout->addStretch();
                }

                m_pAnalysisRangeTable = new AnalysisRangeTable(this);
                m_pAnalysisRangeTable->setFoilPolar(true);
                m_pAnalysisRangeTable->setName("Batch foil ranges"); // debug use only


                QVBoxLayout *pRangeVarsGroupLayout = new QVBoxLayout;
                {
                    pRangeVarsGroupLayout->addLayout(pRangeSpecLayout);
                    pRangeVarsGroupLayout->addWidget(m_pAnalysisRangeTable);
                    m_pfrRangeVars->setLayout(pRangeVarsGroupLayout);
                }
            }
            m_plwNameList = new QListWidget;
            m_plwNameList->setSelectionMode(QAbstractItemView::MultiSelection);

            m_pfrPolars = new QFrame;
            {
                QVBoxLayout *pPolarsLayout = new QVBoxLayout;
                {
                    QGroupBox *pPolarTypeBox = new QGroupBox("Polar type");
                        {
                        QHBoxLayout *pPolarTypeLayout =new QHBoxLayout;
                        {
                            m_prbT1 = new QRadioButton("T1");
                            m_prbT1->setToolTip("Fixed speed polar");
                            m_prbT2 = new QRadioButton("T2");
                            m_prbT2->setToolTip("Fixed lift polar");
                            m_prbT3 = new QRadioButton("T3");
                            m_prbT3->setToolTip("Rubber chord polar");
                            pPolarTypeLayout->addStretch();
                            pPolarTypeLayout->addWidget(m_prbT1);
                            pPolarTypeLayout->addStretch();
                            pPolarTypeLayout->addWidget(m_prbT2);
                            pPolarTypeLayout->addStretch();
                            pPolarTypeLayout->addWidget(m_prbT3);
                            pPolarTypeLayout->addStretch();
                        }
                        pPolarTypeBox->setLayout(pPolarTypeLayout);
                    }

                    m_pcptReTable = new CPTableView(this);
                    {
                        m_pcptReTable->setEditable(true);
                        m_pcptReTable->setEditTriggers(QAbstractItemView::CurrentChanged |
                                                       QAbstractItemView::DoubleClicked |
                                                       QAbstractItemView::SelectedClicked |
                                                       QAbstractItemView::EditKeyPressed |
                                                       QAbstractItemView::AnyKeyPressed);
                        m_pReModel = new ActionItemModel(this);
                        m_pReModel->setRowCount(5);//temporary
                        m_pReModel->setColumnCount(5);
                        m_pReModel->setActionColumn(4);
                        m_pReModel->setHeaderData(0, Qt::Horizontal, QString());
                        m_pReModel->setHeaderData(1, Qt::Horizontal, "Re");
                        m_pReModel->setHeaderData(2, Qt::Horizontal, "Mach");
                        m_pReModel->setHeaderData(3, Qt::Horizontal, "NCrit");
                        m_pReModel->setHeaderData(4, Qt::Horizontal, "Actions");

                        m_pcptReTable->setModel(m_pReModel);

                        int n = m_pReModel->actionColumn();
                        QHeaderView *pHHeader = m_pcptReTable->horizontalHeader();
                        pHHeader->setSectionResizeMode(n, QHeaderView::Stretch);
                        pHHeader->resizeSection(n, 1);

                        m_pFloatDelegate = new XflDelegate(this);
                        m_pFloatDelegate->setCheckColumn(0);
                        m_pFloatDelegate->setActionColumn(4);
                        m_pFloatDelegate->setDigits({-1,0,2,2,-1});
                        m_pFloatDelegate->setItemTypes({XflDelegate::CHECKBOX, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::ACTION});
                        m_pcptReTable->setItemDelegate(m_pFloatDelegate);

                        m_pInsertBeforeAct    = new QAction("Insert before", this);
                        m_pInsertAfterAct    = new QAction("Insert after", this);
                        m_pDeleteAct        = new QAction("Delete", this);
                    }

                    QGroupBox *m_pgbTransVars = new QGroupBox("Forced Transitions");
                    {
                        QGridLayout *pTransVars = new QGridLayout;
                        {
                            pTransVars->setColumnStretch(0,4);
                            pTransVars->setColumnStretch(1,1);
                            QLabel *plabTopTrans = new QLabel("Top transition location (x/c)");
                            QLabel *plabBotTrans = new QLabel("Bottom transition location (x/c)");
                            plabTopTrans->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
                            plabBotTrans->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
                            m_pdeXTopTr = new FloatEdit(1.00);
                            m_pdeXBotTr = new FloatEdit(1.00);

                            pTransVars->addWidget(plabTopTrans, 2, 1);
                            pTransVars->addWidget(m_pdeXTopTr  , 2, 2);
                            pTransVars->addWidget(plabBotTrans, 3, 1);
                            pTransVars->addWidget(m_pdeXBotTr,   3, 2);
                        }
                        m_pgbTransVars->setLayout(pTransVars);
                    }


                    pPolarsLayout->addWidget(pPolarTypeBox);
                    pPolarsLayout->addWidget(m_pcptReTable);
                    pPolarsLayout->addWidget(m_pgbTransVars);
                }
                m_pfrPolars->setLayout(pPolarsLayout);
            }

            pLeftTabWt->addTab(m_plwNameList,  "Foils");
            pLeftTabWt->addTab(m_pfrPolars,    "Polars");
            pLeftTabWt->addTab(m_pfrRangeVars, "Operating points");

            connect(pLeftTabWt, SIGNAL(currentChanged(int)), SLOT(onResizeColumns()));
        }

        QFrame *pRightFrame = new QFrame;
        {
            QVBoxLayout *pRightSideLayout = new QVBoxLayout;
            {
                m_ppto = new PlainTextOutput;
                m_ppto->setReadOnly(true);
                m_ppto->setLineWrapMode(QPlainTextEdit::NoWrap);
                m_ppto->setWordWrapMode(QTextOption::NoWrap);
                m_ppto->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                m_ppto->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
                QFontMetrics fm(DisplayOptions::tableFont());
                m_ppto->setMinimumWidth(67*fm.averageCharWidth());

                m_pfrOptions = new QFrame;
                {
                    QVBoxLayout *pOptionsLayout = new QVBoxLayout;
                    {
                        m_pchInitBL          = new QCheckBox("Initialize B.L. between polars");
                        m_pchStoreOpp        = new QCheckBox("Store operating points");

                        m_pchUpdatePolarView = new QCheckBox("Update polar view");
                        m_pchUpdatePolarView->setToolTip("Update the polar graphs after the completion of each foil/polar pair.\nUncheck for increased analysis speed.");

                        pOptionsLayout->addWidget(m_pchInitBL);
                        pOptionsLayout->addWidget(m_pchStoreOpp);
                        pOptionsLayout->addWidget(m_pchUpdatePolarView);
                    }
                    m_pfrOptions->setLayout(pOptionsLayout);
                }

                m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
                {
                    QPushButton *ppbClearBtn = new QPushButton("Clear Output");
                    connect(ppbClearBtn, SIGNAL(clicked()), m_ppto, SLOT(clear()));
                    m_pButtonBox->addButton(ppbClearBtn, QDialogButtonBox::ActionRole);

                    m_ppbAnalyze   = new QPushButton("Analyze");
                    m_pButtonBox->addButton(m_ppbAnalyze, QDialogButtonBox::ActionRole);

                    connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
                }


                pRightSideLayout->addWidget(m_pfrOptions);
                pRightSideLayout->addWidget(m_ppto);
                pRightSideLayout->addWidget(m_pButtonBox);
            }
            pRightFrame->setLayout(pRightSideLayout);
        }
        m_pHSplitter->addWidget(pLeftTabWt);
        m_pHSplitter->addWidget(pRightFrame);
    }

    QHBoxLayout *pBoxesLayout = new QHBoxLayout;
    {
        pBoxesLayout->addWidget(m_pHSplitter);
    }

    setLayout(pBoxesLayout);
}


void XFoilBatchDlg::onAnalyze()
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
    s_bInitBL = m_pchInitBL->isChecked();

    m_ppbAnalyze->setFocus();

    QString strong;

    QVector<Foil*> foils;
    readFoils(foils);

    if(foils.isEmpty())
    {
        strong ="No foil defined for analysis\n\n";
        m_ppto->insertPlainText(strong);
        cleanUp();
        return;
    }

    m_ppbAnalyze->setText("Cancel");

    //    QThreadPool::globalInstance()->setExpiryTimeout(60000);//ms

    //build an array of all analysis pairs to run
    m_nAnalysis = 0;
    m_nTaskDone = 0;
    m_nTaskStarted = 0;

    for(int ifoil=0; ifoil<foils.count(); ifoil++)
    {
        Foil *pFoil = foils.at(ifoil);
        if(pFoil)
        {
            for (int iRe=0; iRe<s_ActiveList.size(); iRe++)
            {
                if(s_ActiveList.at(iRe))
                {
                    FoilAnalysis *pAnalysis = new FoilAnalysis;
                    m_AnalysisPair.append(pAnalysis);
                    pAnalysis->pFoil = pFoil;

                    pAnalysis->pPolar = Objects2d::createPolar(pFoil, s_PolarType,
                                                               s_ReList.at(iRe), s_MachList.at(iRe), s_NCritList.at(iRe),
                                                               s_XTop, s_XBot);
                    m_nAnalysis++;
                }
            }
        }
    }
    strong = QString::asprintf("Found %d foil/polar pairs to analyze\n", m_nAnalysis);
    m_ppto->insertPlainText(strong);

    // Start as many threads as the user has requested
    // This is a complicated way of doing things; QFuture and QRunnable are simpler

    XFoilTask::setCancelled(false);

    m_ppto->insertPlainText("\nStarted/Done/Total\n");

    if(m_Timer.isActive())  m_Timer.stop();
    m_Timer.start(30);
}


/**
 * A timer event used to check at regular intervals if any threads are inactive
 * in which case it launches a task if any are left.
 * Also checks if the user has pressed the Cancel button
*/
void XFoilBatchDlg::onTimerEvent()
{
    QString strong;
    //time to launch another analysis, if any are left

    if(m_AnalysisPair.size()<=0 || m_bCancel)
    {
        //nothing left to launch...
        if(m_nTaskDone>=m_nAnalysis || m_bCancel)
        {
            QThreadPool::globalInstance()->waitForDone();

            if(m_bCancel) strong = "\n_____Analysis cancelled_____\n";
            else          strong = "\n_____Analysis completed_____\n";
            m_ppto->insertPlainText(strong);
            m_ppto->ensureCursorVisible();

            m_Timer.stop();
            cleanUp();

            if(s_pXDirect->isPolarView() && s_bUpdatePolarView)
            {
                s_pXDirect->createPolarCurves();
                s_pXDirect->updateView();
            }
        }
    }
    else if(m_bIsRunning)
    {
        QString strong;
        if(m_AnalysisPair.count())
        {
            XFoilTask *pXFoilTask = new XFoilTask(this);
            //take the last analysis in the array
            FoilAnalysis *pAnalysis = m_AnalysisPair.at(m_AnalysisPair.size()-1);

            pAnalysis->pPolar->setVisible(true);

            //initiate the task
            pXFoilTask->setAoAAnalysis(s_bAlpha);
            pXFoilTask->setAnalysisRanges(m_pAnalysisRangeTable->ranges());
            pXFoilTask->initializeTask(pAnalysis->pFoil, pAnalysis->pPolar, true, s_bInitBL, true);

            //launch it
            m_nTaskStarted++;
            strong = "Starting "+pAnalysis->pFoil->name()+" / "+pAnalysis->pPolar->name() + "\n";
            updateOutput(strong);

//            QThreadPool::globalInstance()->start(pXFoilTask);

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
            QFuture<void> future = QtConcurrent::run(&XFoilTask::run, pXFoilTask);
#else
            QtConcurrent::run(pXFoilTask, &XFoilTask::run);
#endif
            //remove it from the array of pairs to analyze
            pAnalysis = m_AnalysisPair.last();
            m_AnalysisPair.removeLast();
            delete pAnalysis;
        }
    }
}


void XFoilBatchDlg::updateOutput(QString const&str)
{
    QString strong;
    strong = QString::asprintf("%3d/%3d/%3d  ", m_nTaskStarted, m_nTaskDone, m_nAnalysis);
    m_ppto->insertPlainText(strong + str);
    m_ppto->ensureCursorVisible();
}


void XFoilBatchDlg::cleanUp()
{
    if(m_pXFile->isOpen())
    {
        QTextStream out(m_pXFile);
        out<<m_ppto->toPlainText();
        m_pXFile->close();
    }
    m_pButtonBox->button(QDialogButtonBox::Close)->setEnabled(true);
    m_ppbAnalyze->setText("Analyze");
    m_bIsRunning = false;
    m_bCancel    = false;
    XFoil::setCancel(false);
    m_pButtonBox->button(QDialogButtonBox::Close)->setFocus();

    //in case we cancelled, delete all Analyses that are left
    for(int ia=m_AnalysisPair.count()-1; ia>=0; ia--)
    {
        FoilAnalysis *pAnalysis = m_AnalysisPair.at(ia);
        delete pAnalysis;
    }
    m_AnalysisPair.clear();

    qApp->restoreOverrideCursor();
}


void XFoilBatchDlg::customEvent(QEvent * pEvent)
{
    if(pEvent->type() == XFOIL_END_TASK_EVENT)
    {
        XFoilTaskEvent*pXFEvent = static_cast<XFoilTaskEvent *>(pEvent);
        m_nTaskDone++; //one down, more to go
        QString str = "   ...Finished "+ (pXFEvent->foil())->name()+" / "
                      +(pXFEvent->polar())->name()+"\n";
        updateOutput(str);

        if(s_bUpdatePolarView)
        {
            s_pXDirect->createPolarCurves();
            s_pXDirect->updateView();
        }
        delete pXFEvent->task();
    }
    else if(pEvent->type() == XFOIL_END_OPP_EVENT)
    {
        XFoilOppEvent *pOppEvent = dynamic_cast<XFoilOppEvent*>(pEvent);
        if(XFoilTask::bStoreOpps()) Objects2d::insertOpPoint(pOppEvent->theOpPoint());
        else                        delete pOppEvent->theOpPoint();
    }
}


void XFoilBatchDlg::connectBaseSignals()
{
//    connect(m_pAnalysisRangeTable, SIGNAL(pressed(QModelIndex)), SLOT(onSetControls()));

    connect(m_prbAlpha,           SIGNAL(clicked(bool)),                        SLOT(onAcl()));
    connect(m_prbCl,              SIGNAL(clicked(bool)),                        SLOT(onAcl()));
    connect(m_pchUpdatePolarView, SIGNAL(clicked(bool)),                        SLOT(onUpdatePolarView()));

    connect(m_pcptReTable,        SIGNAL(clicked(QModelIndex)),                 SLOT(onReTableClicked(QModelIndex)));
    connect(m_pReModel,           SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onCellChanged(QModelIndex,QModelIndex)));
    connect(m_pDeleteAct,         SIGNAL(triggered(bool)),                      SLOT(onDelete()));
    connect(m_pInsertBeforeAct,   SIGNAL(triggered(bool)),                      SLOT(onInsertBefore()));
    connect(m_pInsertAfterAct,    SIGNAL(triggered(bool)),                      SLOT(onInsertAfter()));

    connect(m_pHSplitter,         SIGNAL(splitterMoved(int,int)),               SLOT(onResizeColumns()));
}


void XFoilBatchDlg::onButton(QAbstractButton *pButton)
{
    if      (pButton == m_pButtonBox->button(QDialogButtonBox::Close)) onClose();
    else if (pButton == m_ppbAnalyze)                                  onAnalyze();
}


void XFoilBatchDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(m_pButtonBox->button(QDialogButtonBox::Close)->hasFocus())   done(1);
            else if(m_ppbAnalyze->hasFocus())  onAnalyze();
            else                               m_ppbAnalyze->setFocus();
            break;
        }
        case Qt::Key_Escape:
        {
            if(m_bIsRunning)
            {
                m_bCancel = true;
                XFoilTask::setCancelled(true);
                XFoil::setCancel(true);
            }
            else
            {
                onClose(); // will close the dialog box
            }
            break;
        }
        default:
            pEvent->ignore();
    }
    pEvent->accept();
}


void XFoilBatchDlg::initDialog()
{
    if(!XDirect::curFoil()) return;

    blockSignals(true);

    if(s_ActiveList.size()==0 || s_ReList.size()==0 || s_MachList.size()==0 || s_NCritList.size()==0)
        initReList();

    for(int i=0; i<Objects2d::nFoils(); i++)
    {
        Foil const *pFoil = Objects2d::foilAt(i);
        if(pFoil)
        {
            m_plwNameList->addItem(pFoil->name());
            if(m_pFoil==pFoil)
            {
                QListWidgetItem *pItem =  m_plwNameList->item(i);
                pItem->setSelected(true);
            }
        }
    }

    m_ppto->clear();
    m_ppto->setFont(DisplayOptions::tableFont());

    switch (s_PolarType)
    {
        default:
        case xfl::T1POLAR:            m_prbT1->setChecked(true);   break;
        case xfl::T2POLAR:            m_prbT2->setChecked(true);   break;
        case xfl::T3POLAR:   m_prbT3->setChecked(true);   break;
    }


    m_pdeXTopTr->setValue(s_XTop);
    m_pdeXBotTr->setValue(s_XBot);

    if(s_bAlpha) m_prbAlpha->setChecked(true);
    else         m_prbCl->setChecked(true);
    onAcl();

    m_pchInitBL->setChecked(true);
    m_pchStoreOpp->setChecked(XFoilTask::bStoreOpps());
    m_pchUpdatePolarView->setChecked(s_bUpdatePolarView);

    fillReModel();
    m_pAnalysisRangeTable->fillTable();
    blockSignals(false);
}


void XFoilBatchDlg::onAcl()
{
    if(s_PolarType==xfl::T4POLAR) return;
    s_bAlpha = m_prbAlpha->isChecked();
    if(s_bAlpha) m_pAnalysisRangeTable->setRangeType(AnalysisRange::ALPHA);
    else         m_pAnalysisRangeTable->setRangeType(AnalysisRange::CL);
    m_pAnalysisRangeTable->fillTable();
}


void XFoilBatchDlg::onSpecChanged()
{
    readParams();
}


void XFoilBatchDlg::onClose()
{
    if(m_bIsRunning) return;

    m_bCancel = true;
    XFoilTask::setCancelled(true);
    QThreadPool::globalInstance()->waitForDone();

    // leave things as they were
    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());

    readParams();

    accept();
}


void XFoilBatchDlg::reject()
{
    if(m_bIsRunning)
    {
        m_bCancel    = true;
        XFoil::setCancel(true);
    }
    else
    {
        QDialog::reject();
    }
}


void XFoilBatchDlg::onInitBL(int)
{
    s_bInitBL = m_pchInitBL->isChecked();
}


void XFoilBatchDlg::readParams()
{
    s_bAlpha = m_prbAlpha->isChecked();

    if     (m_prbT2->isChecked()) s_PolarType = xfl::T2POLAR;
    else if(m_prbT3->isChecked()) s_PolarType = xfl::T3POLAR;
    else                          s_PolarType = xfl::T1POLAR;

    s_XTop   = m_pdeXTopTr->value();
    s_XBot   = m_pdeXBotTr->value();

    QVector<AnalysisRange> foilrange;
    m_pAnalysisRangeTable->readTable(foilrange); // result also is stored in AnalysisRangeTable static variable

    s_bInitBL = m_pchInitBL->isChecked();
    XFoilTask::setStoreOpps(m_pchStoreOpp->isChecked());
}


void XFoilBatchDlg::setFileHeader()
{
    if(!m_pXFile) return;
    QTextStream out(m_pXFile);

    out << "\n";
    out << xfl::versionName(true);
    out << "\n";

    QDateTime dt = QDateTime::currentDateTime();
    QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");

    out << str;
    out << "\n___________________________________\n\n";
}


void XFoilBatchDlg::writeString(QString const&strong)
{
    if(!m_pXFile || !m_pXFile->isOpen()) return;
    QTextStream ds(m_pXFile);
    ds << strong;
}


void XFoilBatchDlg::outputReList()
{
    m_ppto->appendPlainText("Reynolds numbers to analyze:\n");

    for(int i=0; i<s_ReList.count(); i++)
    {
        if(s_ActiveList.at(i))
        {
            QString strong = QString::asprintf("   Re = %10.0f  /  Mach = %5.3f  /  NCrit = %5.2f\n", s_ReList.at(i), s_MachList.at(i), s_NCritList.at(i));
            m_ppto->appendPlainText(strong);
        }
    }

    m_ppto->appendPlainText("\n");
}


void XFoilBatchDlg::onResizeColumns()
{
    double w = double(m_pcptReTable->width())*.93;
    int wCols  = int(w/10);
    m_pcptReTable->setColumnWidth(0, wCols);
    m_pcptReTable->setColumnWidth(1, 2*wCols);
    m_pcptReTable->setColumnWidth(2, 2*wCols);
    m_pcptReTable->setColumnWidth(3, 2*wCols);
    m_pcptReTable->setColumnWidth(4, 2*wCols);
    update();
}


void XFoilBatchDlg::resizeEvent(QResizeEvent*pEvent)
{
    QDialog::resizeEvent(pEvent);
    onResizeColumns();
}


void XFoilBatchDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);

    if(m_pHSplitter) m_pHSplitter->restoreState(s_HSplitterSizes);

    onResizeColumns();
}


void XFoilBatchDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
    if(m_pHSplitter) s_HSplitterSizes = m_pHSplitter->saveState();
}


void XFoilBatchDlg::onUpdatePolarView()
{
    s_bUpdatePolarView = m_pchUpdatePolarView->isChecked();
    s_pXDirect->updateView();
}


void XFoilBatchDlg::initReList()
{
    s_ActiveList.resize(12);
    s_ReList.resize(12);
    s_MachList.resize(12);
    s_NCritList.resize(12);

    s_ActiveList.fill(true);

    s_ReList[0]  =   30000.0;
    s_ReList[1]  =   40000.0;
    s_ReList[2]  =   60000.0;
    s_ReList[3]  =   80000.0;
    s_ReList[4]  =  100000.0;
    s_ReList[5]  =  130000.0;
    s_ReList[6]  =  160000.0;
    s_ReList[7]  =  200000.0;
    s_ReList[8]  =  300000.0;
    s_ReList[9]  =  500000.0;
    s_ReList[10] = 1000000.0;
    s_ReList[11] = 3000000.0;

    s_MachList.fill(0);
    s_NCritList.fill(9);
}


void XFoilBatchDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("XFoilBatchDlg");
    {
        int iType   = settings.value("PolarType",    1).toInt();
        if     (iType==2)  s_PolarType = xfl::T2POLAR;
        else if(iType==3)  s_PolarType = xfl::T3POLAR;
        else               s_PolarType = xfl::T1POLAR;

        s_bInitBL   = settings.value("bInitBL",      s_bInitBL).toBool();

        s_bAlpha    = settings.value("bAlpha",       s_bAlpha).toBool();

        s_XTop      = settings.value("XTrTop",       s_XTop).toDouble();
        s_XBot      = settings.value("XTrBot",       s_XBot).toDouble();

        if(settings.contains("NReynolds"))
        {
            int NRe = settings.value("NReynolds").toInt();
            s_ActiveList.clear();
            s_ReList.clear();
            s_MachList.clear();
            s_NCritList.clear();
            for (int i=0; i<NRe; i++)
            {
                QString str0 = QString("ActiveList%1").arg(i);
                QString str1 = QString("ReList%1").arg(i);
                QString str2 = QString("MaList%1").arg(i);
                QString str3 = QString("NcList%1").arg(i);
                if(settings.contains(str0)) s_ActiveList.append(settings.value(str0).toBool());
                if(settings.contains(str1)) s_ReList.append(settings.value(str1).toDouble());
                if(settings.contains(str2)) s_MachList.append(settings.value(str2).toDouble());
                if(settings.contains(str3)) s_NCritList.append(settings.value(str3).toDouble());
            }
        }

        s_HSplitterSizes = settings.value("HSplitterSizes").toByteArray();
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void XFoilBatchDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("XFoilBatchDlg");
    {
        settings.setValue("bInitBL",      s_bInitBL);

        switch (s_PolarType)
        {
            default:
            case xfl::T1POLAR:            settings.setValue("PolarType", 1);   break;
            case xfl::T2POLAR:            settings.setValue("PolarType", 2);   break;
            case xfl::T3POLAR:   settings.setValue("PolarType", 3);   break;
        }

        settings.setValue("bAlpha",       s_bAlpha);

        settings.setValue("XTrTop",       s_XTop);
        settings.setValue("XTrBot",       s_XBot);

        settings.setValue("NReynolds", s_ReList.count());
        if(s_ActiveList.size()!=s_ReList.size()) s_ActiveList.resize(s_ReList.size());
        if(s_MachList.size()  !=s_ReList.size()) s_MachList.resize(s_ReList.size());
        if(s_NCritList.size() !=s_ReList.size()) s_NCritList.resize(s_ReList.size());


        for (int i=0; i<s_ReList.count(); i++)
        {
            QString str0 = QString("ActiveList%1").arg(i);
            QString str1 = QString("ReList%1").arg(i);
            QString str2 = QString("MaList%1").arg(i);
            QString str3 = QString("NcList%1").arg(i);
            settings.setValue(str0, s_ActiveList.at(i));
            settings.setValue(str1, s_ReList.at(i));
            settings.setValue(str2, s_MachList.at(i));
            settings.setValue(str3, s_NCritList.at(i));
        }

        settings.setValue("VSplitterSizes",  s_HSplitterSizes);
        settings.setValue("WindowGeom",      s_Geometry);
    }
    settings.endGroup();
}


void XFoilBatchDlg::fillReModel()
{
    m_pReModel->setRowCount(s_ReList.count());
    m_pReModel->blockSignals(true);

    for (int i=0; i<s_ReList.count(); i++)
    {
        QModelIndex chindex = m_pReModel->index(i, 0, QModelIndex());
        m_pReModel->setData(chindex, s_ActiveList.at(i), Qt::UserRole);

        QModelIndex Xindex = m_pReModel->index(i, 1, QModelIndex());
        m_pReModel->setData(Xindex, s_ReList.at(i));

        QModelIndex Yindex = m_pReModel->index(i, 2, QModelIndex());
        m_pReModel->setData(Yindex, s_MachList.at(i));

        QModelIndex Zindex = m_pReModel->index(i, 3, QModelIndex());
        m_pReModel->setData(Zindex, s_NCritList.at(i));

        QModelIndex actionindex = m_pReModel->index(i, 4, QModelIndex());
        m_pReModel->setData(actionindex, QString("..."));
    }
    m_pReModel->blockSignals(false);
    m_pcptReTable->resizeRowsToContents();
}


void XFoilBatchDlg::setRowEnabled(int  row, bool bEnabled)
{
    for(int col=0; col<m_pReModel->columnCount(); col++)
    {
        QModelIndex ind = m_pReModel->index(row, col, QModelIndex());
        m_pReModel->setData(ind, bEnabled, Qt::UserRole); // used to display the row as enabled or disabled
    }
}


void XFoilBatchDlg::onDelete()
{
    if(m_pReModel->rowCount()<=1) return;

    QModelIndex index = m_pcptReTable->currentIndex();
    int sel = index.row();

    if(sel<0 || sel>=s_ReList.count()) return;

    s_ActiveList.removeAt(sel);
    s_ReList.removeAt(sel);
    s_MachList.removeAt(sel);
    s_NCritList.removeAt(sel);

    fillReModel();
    m_pcptReTable->closePersistentEditor(m_pcptReTable->currentIndex());
}


void XFoilBatchDlg::onInsertBefore()
{
    int sel = m_pcptReTable->currentIndex().row();

    s_ActiveList.insert(sel, true);
    s_ReList.insert(sel, 0.0);
    s_MachList.insert(sel, 0.0);
    s_NCritList.insert(sel, 0.0);

    if     (sel>0)   s_ReList[sel] = (s_ReList.at(sel-1)+s_ReList.at(sel+1)) /2.0;
    else if(sel==0)  s_ReList[sel] =  s_ReList.at(sel+1)                     /2.0;
    else             s_ReList[0]   = 100000.0;

    if(sel>=0)
    {
        s_MachList[sel]  = s_MachList.at(sel+1);
        s_NCritList[sel] = s_NCritList.at(sel+1);
    }
    else
    {
        sel = 0;
        s_MachList[sel]  = 0.0;
        s_NCritList[sel] = 0.0;
    }

    fillReModel();
    m_pcptReTable->closePersistentEditor(m_pcptReTable->currentIndex());


    QModelIndex index = m_pReModel->index(sel, 0, QModelIndex());
    m_pcptReTable->setCurrentIndex(index);
    m_pcptReTable->selectRow(index.row());
}


void XFoilBatchDlg::onInsertAfter()
{
    int sel = m_pcptReTable->currentIndex().row()+1;

    s_ActiveList.insert(sel, true);
    s_ReList.insert(sel, 0.0);
    s_MachList.insert(sel, 0.0);
    s_NCritList.insert(sel, 0.0);

    if(sel==s_ReList.size()-1) s_ReList[sel] = s_ReList[sel-1]*2.0;
    else if(sel>0)             s_ReList[sel] = (s_ReList[sel-1]+s_ReList[sel+1]) /2.0;
    else if(sel==0)            s_ReList[sel] = s_ReList[sel+1]                   /2.0;

    if(sel>0)
    {
        s_MachList[sel]  = s_MachList[sel-1];
        s_NCritList[sel] = s_NCritList[sel-1];
    }
    else
    {
        sel = 0;
        s_MachList[sel]  = 0.0;
        s_NCritList[sel] = 0.0;
    }

    fillReModel();
    m_pcptReTable->closePersistentEditor(m_pcptReTable->currentIndex());

    QModelIndex index = m_pReModel->index(sel, 0, QModelIndex());
    m_pcptReTable->setCurrentIndex(index);
    m_pcptReTable->selectRow(index.row());
}


void XFoilBatchDlg::onCellChanged(QModelIndex topLeft, QModelIndex )
{
    s_ActiveList.clear();
    s_ReList.clear();
    s_MachList.clear();
    s_NCritList.clear();

    for (int i=0; i<m_pReModel->rowCount(); i++)
    {
        s_ActiveList.append(m_pReModel->index(i, 0, QModelIndex()).data(Qt::UserRole).toBool());
        s_ReList.append(    m_pReModel->index(i, 1, QModelIndex()).data().toDouble());
        s_MachList.append(  m_pReModel->index(i, 2, QModelIndex()).data().toDouble());
        s_NCritList.append( m_pReModel->index(i, 3, QModelIndex()).data().toDouble());
    }

    if(topLeft.column()==0)
    {
        sortRe();

        //and fill back the model
        fillReModel();
    }
}


/**
* Bubble sort algorithm for the arrays of Reynolds, Mach and NCrit numbers.
* The arrays are sorted by crescending Re numbers.
*/
void XFoilBatchDlg::sortRe()
{
    int indx(0), indx2(0);
    bool Chtemp(true), Chtemp2(true);
    double Retemp(0), Retemp2(0);
    double Matemp(0), Matemp2(0);
    double NCtemp(0), NCtemp2(0);
    int flipped(0);

    if (s_ReList.size()<=1) return;

    indx = 1;
    do
    {
        flipped = 0;
        for (indx2 = s_ReList.size() - 1; indx2 >= indx; --indx2)
        {
            Chtemp  = s_ActiveList.at(indx2);
            Chtemp2 = s_ActiveList.at(indx2 - 1);
            Retemp  = s_ReList.at(indx2);
            Retemp2 = s_ReList.at(indx2 - 1);
            Matemp  = s_MachList.at(indx2);
            Matemp2 = s_MachList.at(indx2 - 1);
            NCtemp  = s_NCritList.at(indx2);
            NCtemp2 = s_NCritList.at(indx2 - 1);
            if (Retemp2> Retemp)
            {
                s_ActiveList[indx2 - 1] = Chtemp;
                s_ActiveList[indx2]     = Chtemp2;
                s_ReList[indx2 - 1]     = Retemp;
                s_ReList[indx2]         = Retemp2;
                s_MachList[indx2 - 1]   = Matemp;
                s_MachList[indx2]       = Matemp2;
                s_NCritList[indx2 - 1]  = NCtemp;
                s_NCritList[indx2]      = NCtemp2;
                flipped = 1;
            }
        }
    } while ((++indx < s_ReList.size()) && flipped);
}


void XFoilBatchDlg::onReTableClicked(QModelIndex index)
{
    if(!index.isValid())  return;

    int row = index.row();
    if(row<0 || row>=m_pReModel->rowCount()) return;

    m_pcptReTable->selectRow(row);

    switch(index.column())
    {
        case 0:
        {
            bool bActive = m_pReModel->data(index, Qt::UserRole).toBool();
            if(row<s_ActiveList.size())
            {
                s_ActiveList[row] = !bActive; // toggle
                m_pReModel->setData(index, s_ActiveList.at(row), Qt::UserRole);
            }
            break;
        }
        case 4:
        {
            QRect itemrect = m_pcptReTable->visualRect(index);
            QPoint menupos = m_pcptReTable->mapToGlobal(itemrect.topLeft());
            QMenu *pReTableRowMenu = new QMenu("Section", this);
            pReTableRowMenu->addAction(m_pInsertBeforeAct);
            pReTableRowMenu->addAction(m_pInsertAfterAct);
            pReTableRowMenu->addAction(m_pDeleteAct);
            pReTableRowMenu->exec(menupos, m_pInsertBeforeAct);

            break;
        }
        default:  break;

    }
    update();
}


void XFoilBatchDlg::readFoils(QVector<Foil*> &foils)
{
    foils.clear();
    for(int i=0; i<m_plwNameList->count();i++)
    {
        QListWidgetItem *pItem = m_plwNameList->item(i);
        if(pItem && pItem->isSelected())
        {
            Foil *pFoil = Objects2d::foil(pItem->text());
            if(pFoil)
                foils.append(pFoil);
        }
    }
}



