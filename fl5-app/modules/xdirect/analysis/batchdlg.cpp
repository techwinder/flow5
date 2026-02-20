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

#include <QDateTime>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QMenu>
#include <QModelIndex>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrentRun>

#include "batchdlg.h"

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
#include <modules/xdirect/xdirect.h>
#include <modules/xdirect/analysis/polarnamemaker.h>


bool BatchDlg::s_bAlpha    = true;



bool BatchDlg::s_bUpdatePolarView = false;
XDirect * BatchDlg::s_pXDirect;

QByteArray BatchDlg::s_Geometry;
QByteArray BatchDlg::s_HSplitterSizes;


BatchDlg::BatchDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Batch analysis"));

    setWindowFlag(Qt::WindowMinMaxButtonsHint);
    m_pXFile = nullptr;
    m_pFoil = nullptr;

    m_bCancel         = false;
    m_bIsRunning      = false;

    XFoil::setCancel(false);

    m_nTaskDone    = 0;
    m_nTaskStarted = 0;
    m_nAnalysis    = 0;

    makeCommonWts();
    connectBaseSignals();
}


BatchDlg::~BatchDlg()
{
    if(m_pXFile)  delete m_pXFile;
    m_pXFile = nullptr;
}


void BatchDlg::makeCommonWts()
{
    m_pHSplitter = new QSplitter(Qt::Horizontal);
    {
        m_pHSplitter->setChildrenCollapsible(false);

        m_pLeftTabWt = new QTabWidget;
        {
            m_pfrRangeVars = new QTabWidget;
            {
                QFrame *pfrT12Range = new QFrame;
                {
                    QVBoxLayout *pT12Layout = new QVBoxLayout;
                    {
                        QHBoxLayout *pRangeSpecLayout = new QHBoxLayout;
                        {
                            QLabel *plabSpec = new QLabel(tr("Specify:"));
                            m_prbAlpha = new QRadioButton(ALPHAch);
                            m_prbCl = new QRadioButton("Cl");
                            pRangeSpecLayout->addWidget(plabSpec);
                            pRangeSpecLayout->addWidget(m_prbAlpha);
                            pRangeSpecLayout->addWidget(m_prbCl);
                            pRangeSpecLayout->addStretch();
                        }

                        m_pT12RangeTable = new AnalysisRangeTable(this);
                        m_pT12RangeTable->setFoilPolar(true);
                        m_pT12RangeTable->setName("Batch foil T12 ranges"); // debug use only

                        pT12Layout->addLayout(pRangeSpecLayout);
                        pT12Layout->addWidget(m_pT12RangeTable);
                    }

                    pfrT12Range->setLayout(pT12Layout);
                }

                m_pT4RangeTable = new AnalysisRangeTable(this);
                m_pT4RangeTable->setFoilPolar(true);
                m_pT4RangeTable->setName("Batch foil T4 ranges"); // debug use only

                m_pT6RangeTable = new AnalysisRangeTable(this);
                m_pT6RangeTable->setFoilPolar(true);
                m_pT6RangeTable->setName("Batch foil T6 ranges"); // debug use only

                m_pfrRangeVars->addTab(pfrT12Range, "Type 12");
                m_pfrRangeVars->addTab(m_pT4RangeTable, "Type 4");
                m_pfrRangeVars->addTab(m_pT6RangeTable, "Type 6");
            }

            m_pLeftTabWt->addTab(m_pfrRangeVars, "Operating points");
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
                        m_pchStoreOpp        = new QCheckBox(tr("Store operating points"));

                        m_pchUpdatePolarView = new QCheckBox(tr("Update polar view"));
                        m_pchUpdatePolarView->setToolTip(tr("Update the polar graphs after the completion of each foil/polar pair.\nUncheck for increased analysis speed."));

                        pOptionsLayout->addWidget(m_pchStoreOpp);
                        pOptionsLayout->addWidget(m_pchUpdatePolarView);
                    }
                    m_pfrOptions->setLayout(pOptionsLayout);
                }

                m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
                {
                    QPushButton *ppbClearBtn = new QPushButton(tr("Clear Output"));
                    connect(ppbClearBtn, SIGNAL(clicked()), m_ppto, SLOT(clear()));
                    m_pButtonBox->addButton(ppbClearBtn, QDialogButtonBox::ActionRole);

                    m_ppbAnalyze   = new QPushButton(tr("Calculate"));
                    m_pButtonBox->addButton(m_ppbAnalyze, QDialogButtonBox::ActionRole);

                    connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
                }


                pRightSideLayout->addWidget(m_pfrOptions);
                pRightSideLayout->addWidget(m_ppto);
                pRightSideLayout->addWidget(m_pButtonBox);
            }
            pRightFrame->setLayout(pRightSideLayout);
        }
        m_pHSplitter->addWidget(m_pLeftTabWt);
        m_pHSplitter->addWidget(pRightFrame);
    }
}


void BatchDlg::cleanUp()
{
    for(uint it=0; it<m_Tasks.size(); it++)
        delete m_Tasks.at(it);

    m_Tasks.clear();

    if(m_pXFile->isOpen())
    {
        QTextStream out(m_pXFile);
        out<<m_ppto->toPlainText();
        m_pXFile->close();
    }
    m_pButtonBox->button(QDialogButtonBox::Close)->setEnabled(true);
    m_ppbAnalyze->setText(tr("Calculate"));
    m_bIsRunning = false;
    m_bCancel    = false;
    XFoil::setCancel(false);
    m_pButtonBox->button(QDialogButtonBox::Close)->setFocus();

    //in case we cancelled, delete all Analyses that are left


    qApp->restoreOverrideCursor();
}


void BatchDlg::customEvent(QEvent * pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent const *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        m_ppto->onAppendQText(pMsgEvent->msg());
    }
    else if(pEvent->type() == XFOIL_TASK_END_EVENT)
    {
        XFoilTaskEvent *pXFEvent = static_cast<XFoilTaskEvent *>(pEvent);
        m_nTaskDone++; //one down, more to go
        QString strong = QString::asprintf("%3d/%3d/%3d  ", m_nTaskStarted, m_nTaskDone, m_nAnalysis);
        std::string str = "   ...Finished "+ pXFEvent->task()->foil()->name()+" / "+ pXFEvent->task()->polar()->name()+"\n";

        m_ppto->onAppendQText(strong + QString::fromStdString(str));

        if(s_bUpdatePolarView)
        {
            s_pXDirect->createPolarCurves();
            s_pXDirect->updateView();
        }

        bool bStoreOpp = m_pchStoreOpp->isChecked();
        for(OpPoint *pOpp : pXFEvent->task()->operatingPoints())
        {
            Objects2d::addOpPoint(pOpp, bStoreOpp);
            if(!bStoreOpp) delete pOpp;
        }
    }
    else if(pEvent->type()==XFOIL_BATCH_END_EVENT)
    {
        std::string strong;
        if(m_bCancel) strong = "\n_____Analysis cancelled_____\n";
        else          strong = "\n_____Analysis completed_____\n";
        m_ppto->onAppendStdText(strong);

        cleanUp();

        if(s_pXDirect->isPolarView() && s_bUpdatePolarView)
        {
            s_pXDirect->createPolarCurves();
            s_pXDirect->updateView();
        }
    }
}


void BatchDlg::connectBaseSignals()
{
    connect(m_prbAlpha,           SIGNAL(clicked(bool)),                        SLOT(onAcl()));
    connect(m_prbCl,              SIGNAL(clicked(bool)),                        SLOT(onAcl()));
    connect(m_pchUpdatePolarView, SIGNAL(clicked(bool)),                        SLOT(onUpdatePolarView()));
}


void BatchDlg::onButton(QAbstractButton *pButton)
{
    if      (pButton == m_pButtonBox->button(QDialogButtonBox::Close)) onClose();
    else if (pButton == m_ppbAnalyze)                                  onAnalyze();
}


void BatchDlg::keyPressEvent(QKeyEvent *pEvent)
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


void BatchDlg::initDialog()
{
    if(!XDirect::curFoil()) return;

    m_ppto->clear();
    m_ppto->setFont(DisplayOptions::tableFont());

    m_pchStoreOpp->setChecked(XDirect::bStoreOpps());
    m_pchUpdatePolarView->setChecked(s_bUpdatePolarView);


    if(s_bAlpha) m_prbAlpha->setChecked(true);
    else         m_prbCl->setChecked(true);
    onAcl();

    m_pT4RangeTable->setRangeType(AnalysisRange::REYNOLDS);
    m_pT6RangeTable->setRangeType(AnalysisRange::THETA);

    m_pT4RangeTable->fillTable();
    m_pT6RangeTable->fillTable();
}


void BatchDlg::onAcl()
{
//    if(s_PolarType==xfl::T4POLAR) return;
    s_bAlpha = m_prbAlpha->isChecked();
    if(s_bAlpha) m_pT12RangeTable->setRangeType(AnalysisRange::ALPHA);
    else         m_pT12RangeTable->setRangeType(AnalysisRange::CL);
    m_pT12RangeTable->fillTable();
}


void BatchDlg::onSpecChanged()
{
    readParams();
}


void BatchDlg::onClose()
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


void BatchDlg::reject()
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


void BatchDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("BatchDlg");
    {
        s_HSplitterSizes = settings.value("HSplitterSizes").toByteArray();
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void BatchDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("BatchDlg");
    {
        settings.setValue("VSplitterSizes",  s_HSplitterSizes);
        settings.setValue("WindowGeom",      s_Geometry);
    }
    settings.endGroup();
}



void BatchDlg::readParams()
{
    s_bAlpha = m_prbAlpha->isChecked();

    s_bUpdatePolarView = m_pchUpdatePolarView->isChecked();
    XDirect::setStoreOpps(m_pchStoreOpp->isChecked());
}


void BatchDlg::setFileHeader()
{
    if(!m_pXFile) return;
    QTextStream out(m_pXFile);

    out << "\n";
    out << QString::fromStdString(fl5::versionName(true));
    out << "\n";

    QDateTime dt = QDateTime::currentDateTime();
    QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");

    out << str;
    out << "\n___________________________________\n\n";
}


void BatchDlg::writeString(QString const&strong)
{
    if(!m_pXFile || !m_pXFile->isOpen()) return;
    QTextStream ds(m_pXFile);
    ds << strong;
}


void BatchDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);

    if(m_pHSplitter) m_pHSplitter->restoreState(s_HSplitterSizes);
}


void BatchDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
    if(m_pHSplitter) s_HSplitterSizes = m_pHSplitter->saveState();
}


void BatchDlg::onUpdatePolarView()
{
    s_bUpdatePolarView = m_pchUpdatePolarView->isChecked();
    s_pXDirect->updateView();
}


void BatchDlg::batchLaunch()
{
    QString strong;

    std::vector<std::thread> threads;

    for(FoilAnalysis &analysis : m_AnalysisPair)
    {
        XFoilTask *pXFoilTask = new XFoilTask();
        m_Tasks.push_back(pXFoilTask);

        analysis.m_pPolar->setVisible(true);

        pXFoilTask->setAoAAnalysis(s_bAlpha);

        pXFoilTask->clearRanges();

        switch (analysis.m_pPolar->type())
        {
            case xfl::T1POLAR:
            case xfl::T2POLAR:
            {
                for (AnalysisRange const &rg : m_pT12RangeTable->ranges())
                {
                    pXFoilTask->appendRange(rg);
                }

                break;
            }
            case xfl::T4POLAR:
            {
                for (AnalysisRange const &rg : m_pT4RangeTable->ranges())
                {
                    pXFoilTask->appendRange(rg);
                }

                break;
            }
            case xfl::T6POLAR:
            {
                for (AnalysisRange const &rg : m_pT6RangeTable->ranges())
                {
                    pXFoilTask->appendRange(rg);
                }

                break;
            }
            default:
                break;
        }

        bool bStoreOpp = m_pchStoreOpp->isChecked();
        pXFoilTask->initialize(analysis.m_Foil, analysis.m_pPolar, bStoreOpp);

        //launch it
        m_nTaskStarted++;

        strong = QString::asprintf("%3d/%3d/%3d  ", m_nTaskStarted, m_nTaskDone, m_nAnalysis);
        strong += QString::fromStdString("Starting "+ analysis.m_Foil.name()+'/'+analysis.m_pPolar->name() + "\n");
        qApp->postEvent(this, new MessageEvent(strong));
        threads.push_back(std::thread(&XFoilTask::run, pXFoilTask));
    }

    std::vector<bool> bFinished(m_Tasks.size(), false);
    bool bAllFinished(false);
    do
    {
        bAllFinished = true;
        for(uint itask=0; itask<m_Tasks.size(); itask++)
        {
            XFoilTask*pTask = m_Tasks.at(itask);
            if(pTask->isFinished())
            {
                if(!bFinished.at(itask))
                {
                    // notify
                    XFoilTaskEvent *pTaskEvent = new XFoilTaskEvent(pTask);
                    qApp->postEvent(this, pTaskEvent);
                    // toggle flag
                    bFinished[itask] = true;
                }
            }
            else
                bAllFinished = false;
//            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    while(!bAllFinished);

    for(uint iblock=0; iblock<threads.size(); iblock++)
    {
        threads[iblock].join();

    }

    qApp->postEvent(this, new QEvent(XFOIL_BATCH_END_EVENT)); // done and clean
}




