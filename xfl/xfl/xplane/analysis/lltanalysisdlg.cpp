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

#define _MATH_DEFINES_DEFINED

#include "lltanalysisdlg.h"

#include <QApplication>
#include <QDir>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrentRun>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>

#include "lltanalysisdlg.h"
#include <xflcore/displayoptions.h>

#include <xflcore/xflcore.h>
#include <xflcore/saveoptions.h>
#include <xfl/xplane/xplane.h>
#include <xfl/xplane/analysis/analysis3dsettings.h>
#include <xflobjects/analysis3d/llttask.h>
#include <xflobjects/analysis3d/planetask.h>
#include <xflgraph/containers/graphwt.h>
#include <xflgraph/graph/graph.h>
#include <xflgraph/controls/graphoptions.h>
#include <xflobjects/objects3d/objects3d.h>
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflobjects/objects3d/analysis/wpolar.h>
#include <xflobjects/objects3d/analysis/planeopp.h>
#include <xflobjects/objects3d/wing/wingxfl.h>
#include <xflwidgets/customwts/plaintextoutput.h>


QByteArray LLTAnalysisDlg::s_HSplitterSizes;
QByteArray LLTAnalysisDlg::s_Geometry;
XPlane *LLTAnalysisDlg::s_pXPlane = nullptr;

LLTAnalysisDlg::LLTAnalysisDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("LLT Analysis");

    setupLayout();

    m_pTheLLTTask = nullptr;

    m_pIterGraph = new Graph();
    m_pIterGraph->setCurveModel(new CurveModel());
    m_pIterGraph->setName("LLT iterations");
    m_pIterGraph->setXMin(0.0);             m_pIterGraph->setXMax(LLTTask::maxIter());
    m_pIterGraph->setYMin(0, 0.0);          m_pIterGraph->setYMax(0, 1.0);
    m_pIterGraph->setYMin(1, 0.0);          m_pIterGraph->setYMax(1, 1.0);

    m_pGraphWt->setGraph(m_pIterGraph);

    m_bHasErrors = false;

    m_pIterGraph->setAuto(true);


//    m_pIterGraph->setXMajGrid(true, QColor(120,120,120),2,1);
//    m_pIterGraph->setYMajGrid(true, QColor(120,120,120),2,1);

//    m_pIterGraph->setScaleType(GRAPH::RESETTING);

//    m_pIterGraph->setYTitle("|Da|");

    m_pLastPOpp = nullptr;

    m_bFinished   = true;
}


LLTAnalysisDlg::~LLTAnalysisDlg()
{
    if(m_pTheLLTTask) delete m_pTheLLTTask;
    if(m_pIterGraph)
    {
        delete m_pIterGraph->curveModel();
        delete m_pIterGraph;
    }
}


void LLTAnalysisDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            onCancelAnalysis();
            pEvent->accept();
            return;
        }
        default:
            pEvent->ignore();
    }
}


void LLTAnalysisDlg::onCancelAnalysis()
{
    if(m_pTheLLTTask) m_pTheLLTTask->setCancelled(true);
    if(m_bFinished) accept();
}


void LLTAnalysisDlg::onClearCurves()
{
    m_pIterGraph->deleteCurves();
    update();
}


void LLTAnalysisDlg::setupLayout()
{
    m_pHSplitter = new QSplitter;
    {
        m_pGraphWt = new GraphWt(this);
        m_pGraphWt->showLegend(true);

        QFrame *pfrRight = new QFrame;
        {
            QVBoxLayout *pRightLayout = new QVBoxLayout;
            {

                m_ppto = new PlainTextOutput;
                QHBoxLayout *pctrlLayout = new QHBoxLayout;
                {
                    m_ppbClearCurves = new QPushButton("Clear curves");
                    connect(m_ppbClearCurves, SIGNAL(clicked()), SLOT(onClearCurves()));

                    m_ppbCancel = new QPushButton("Close");
                    connect(m_ppbCancel, SIGNAL(clicked()), SLOT(onCancelAnalysis()));

                    m_pchKeepOpenOnErrors = new QCheckBox("Keep this window opened on errors");
                    pctrlLayout->addWidget(m_pchKeepOpenOnErrors);
                    pctrlLayout->addStretch();
                    pctrlLayout->addWidget(m_ppbClearCurves);
                    pctrlLayout->addWidget(m_ppbCancel);
                }
                pRightLayout->addWidget(m_ppto,1);
                pRightLayout->addLayout(pctrlLayout);
            }
            pfrRight->setLayout(pRightLayout);
        }

        m_pHSplitter->setChildrenCollapsible(false);
        m_pHSplitter->addWidget(m_pGraphWt);
        m_pHSplitter->addWidget(pfrRight);
    }

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        pMainLayout->addWidget(m_pHSplitter);
    }

    setLayout(pMainLayout);
}


void LLTAnalysisDlg::initDialog(PlaneXfl *pPlane, WPolar *pWPolar, QVector<double> const &opplist)
{
    m_pLastPOpp = nullptr;

    m_pIterGraph->deleteCurves();
    m_pIterGraph->resetLimits();
    m_pIterGraph->setXMin(0.0);
    m_pIterGraph->setXMax(double(LLTTask::maxIter()));
    m_pIterGraph->setXo(0.0);
    m_pIterGraph->setXUnit(int(LLTTask::maxIter()/10.0));

    m_pIterGraph->setYo(0, 0.0);
    m_pIterGraph->setYMin(0,0.0);
    m_pIterGraph->setYMax(0,1.0);
    m_pIterGraph->setAuto(true);

    m_pIterGraph->setLegendVisible(true);
    m_pIterGraph->setLegendPosition(Qt::AlignTop | Qt::AlignHCenter);

    m_pTheLLTTask = new LLTTask;
    m_pTheLLTTask->setEventDestination(nullptr);
    connect(m_pTheLLTTask, &Task3d::outputMessage, this, &LLTAnalysisDlg::onOutputMessage);
    m_pTheLLTTask->setObjects(pPlane, pWPolar);
    m_pTheLLTTask->setLLTRange(opplist);
    m_pTheLLTTask->initializeGeom();
}


void LLTAnalysisDlg::analyze()
{
    if(!m_pTheLLTTask->plane() || !m_pTheLLTTask->wPolar()) return;
    //all set to launch the analysis

    WingXfl *pWing = m_pTheLLTTask->plane()->mainWing();

    if(!pWing) return;

    m_ppbCancel->setText("Cancel");
    m_bFinished = false;
    m_pLastPOpp = nullptr;

    m_ppto->clear();

    QString strange, log;
    strange = pWing->name()+"\n";
    log = strange;
    strange = m_pTheLLTTask->wPolar()->name()+"\n";
    log += strange;

    strange = "Launching analysis....\n\n";
    log += strange;
    strange = QString("Max iterations     = %1\n").arg(LLTTask::maxIter());
    log += strange;
    strange = ALPHACHAR + QString(" precision       = %1 ").arg(LLTTask::convergencePrecision(),0,'f',6) + DEGCHAR + "\n";
    log += strange;
    strange = QString("Number of stations = %1\n").arg(LLTTask::nSpanStations());
    log += strange;
    strange = QString("Relaxation factor  = %1\n\n").arg(LLTTask::relaxationFactor(),0,'f',1);
    log += strange;
    onOutputMessage(log);

    connect(m_pTheLLTTask, SIGNAL(taskFinished()),     this,   SLOT(onTaskFinished()));

    //run the instance asynchronously
/*    QFuture<void> future = QtConcurrent::run(m_pTheLLTTask, &LLTAnalysis::run);
    while(future.isRunning())
    {
        
        QThread::msleep(200);
        QDateTime dt = QDateTime::currentDateTime();
        QString str = dt.toString("dd.MM.yyyy  hh:mm:ss.zzz");
    }*/

    QThread *pThread = new QThread;
    m_pTheLLTTask->moveToThread(pThread);
    connect(pThread,       SIGNAL(started()),          m_pTheLLTTask, SLOT(run()));
    connect(pThread,       SIGNAL(finished()),         pThread,          SLOT(deleteLater()));

    pThread->start();
    pThread->setPriority(xfl::threadPriority());

//    m_pTheLLTTask->run();
//    cleanUp();
}



void LLTAnalysisDlg::onTaskFinished()
{
    if(!m_pTheLLTTask) return;
    m_pTheLLTTask->thread()->exit(0);

    storePOpps();

    QString strong="\n";
    outputMessage(strong);
    if(m_pTheLLTTask->isCancelled())
    {
        strong = "Analysis cancelled\n\n";
    }
    else if (!m_pTheLLTTask->hasErrors())
        strong = "LLT analysis completed successfully\n\n";
    else if (m_pTheLLTTask->hasErrors())
        strong = "LLT analysis completed ... Errors encountered\n\n";
    outputMessage(strong);

    m_bHasErrors = m_pTheLLTTask->hasErrors();
    m_ppbCancel->setText("Close");

    emit analysisFinished(m_pTheLLTTask->wPolar());

    cleanUp();
}


void LLTAnalysisDlg::cleanUp()
{
    QString strange;

    m_bFinished = true;
    strange = "\n_________\nAnalysis completed";

    strange+= "\n";

//    m_pTheTask->m_ptheLLTAnalysis->traceLog(strange);

    QString logFileName = SaveOptions::newLogFileName();
    SaveOptions::setLastLogFileName(logFileName);

    QFile pXFile(logFileName);

    if(pXFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream outstream(&pXFile);
        outstream << m_ppto->toPlainText();
        outstream << "\n";
//        QDateTime dt = QDateTime::currentDateTime();
//        QString str = dt.toString();
//        outstream << "\n";

        outstream.flush();
        pXFile.close();
    }

    delete m_pTheLLTTask;
    m_pTheLLTTask = nullptr;
    m_ppbCancel->setText("Close");
    m_ppbCancel->setFocus();
}


void LLTAnalysisDlg::storePOpps()
{
    // WPolar has been populated with results by the LLTTask
    // Store the POpps if requested

    if(PlaneOpp::bStoreOpps3d())
    {
        m_pLastPOpp = s_pXPlane->storePOpps(m_pTheLLTTask->planeOppList());
    }
    else
    {
        m_pTheLLTTask->clearPOppList();
        m_pLastPOpp = nullptr;
    }
}


void LLTAnalysisDlg::updateView()
{
    m_pGraphWt->update();
    update();
}


void LLTAnalysisDlg::onOutputMessage(QString const &msg)
{
    m_ppto->insertPlainText(msg);
    m_ppto->ensureCursorVisible();
}


void LLTAnalysisDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);

    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);

    GraphOptions::resetGraphSettings(*m_pIterGraph);


    m_pchKeepOpenOnErrors->setChecked(Analysis3dSettings::keepOpenOnErrors());
    m_ppbCancel->setFocus();
}


void LLTAnalysisDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    Analysis3dSettings::setKeepOpenOnErrors(m_pchKeepOpenOnErrors->isChecked());
    s_Geometry = saveGeometry();
    s_HSplitterSizes  = m_pHSplitter->saveState();
}


void LLTAnalysisDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent const *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        m_ppto->onAppendThisPlainText(pMsgEvent->msg());
    }
    else if(pEvent->type() == LLT_OPP_EVENT)
    {
        LLTOppEvent *pOppEvent = static_cast<LLTOppEvent*>(pEvent);

        Curve *pCurve = m_pIterGraph->addCurve();
        pCurve->setName(ALPHACHAR + QString::asprintf("=%.2f", pOppEvent->alpha()) + DEGCHAR);
        pCurve->setPoints(pOppEvent->xc(), pOppEvent->yc());
        m_pIterGraph->invalidate();
        m_pIterGraph->resetYLimits();
        m_pGraphWt->update();
    }
}


void LLTAnalysisDlg::outputMessage(const QString &msg)
{
    m_ppto->onAppendThisPlainText(msg);
}


bool LLTAnalysisDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("LLTAnalysisDlg");
    {
        settings.setValue("Geometry", s_Geometry);
        settings.setValue("HSplitterSize", s_HSplitterSizes);
    }
    settings.endGroup();

    return true;
}


bool LLTAnalysisDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("LLTAnalysisDlg");
    {
        s_Geometry = settings.value("Geometry").toByteArray();
        s_HSplitterSizes = settings.value("HSplitterSize").toByteArray();
    }
    settings.endGroup();
    return true;
}


