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

#define _MATH_DEFINES_DEFINED


#include "lltanalysisdlg.h"

#include <QApplication>
#include <QDir>
#include <QtConcurrent/QtConcurrentRun>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>

#include "lltanalysisdlg.h"

#include <api/llttask.h>
#include <api/objects3d.h>
#include <api/planeopp.h>
#include <api/planetask.h>
#include <api/planexfl.h>
#include <api/trace.h>
#include <api/utils.h>
#include <api/wingxfl.h>
#include <api/planepolar.h>

#include <core/displayoptions.h>
#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/graphs/containers/graphwt.h>
#include <interfaces/graphs/controls/graphoptions.h>
#include <interfaces/graphs/graph/graph.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <modules/xplane/analysis/analysis3dsettings.h>
#include <modules/xplane/xplane.h>


QByteArray LLTAnalysisDlg::s_HSplitterSizes;
QByteArray LLTAnalysisDlg::s_Geometry;
XPlane *LLTAnalysisDlg::s_pXPlane = nullptr;

LLTAnalysisDlg::LLTAnalysisDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("LLT Analysis"));

    setupLayout();

    m_pLLTTask = nullptr;

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
    if(m_pLLTTask) delete m_pLLTTask;
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
    if(m_pLLTTask) m_pLLTTask->setCancelled(true);
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
                    m_ppbClearCurves = new QPushButton(tr("Clear curves"));
                    connect(m_ppbClearCurves, SIGNAL(clicked()), SLOT(onClearCurves()));

                    m_ppbCancel = new QPushButton(tr("Close"));
                    connect(m_ppbCancel, SIGNAL(clicked()), SLOT(onCancelAnalysis()));

                    m_pchKeepOpenOnErrors = new QCheckBox(tr("Keep this window opened on errors"));
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


void LLTAnalysisDlg::initDialog(PlaneXfl *pPlane, PlanePolar *pWPolar, std::vector<double> const &opplist)
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

    m_pLLTTask = new LLTTask;
    m_pLLTTask->setObjects(pPlane, pWPolar);
    m_pLLTTask->setLLTRange(opplist);
    m_pLLTTask->initializeGeom();
}


void LLTAnalysisDlg::onTaskFinished()
{
    if(!m_pLLTTask) return;

    // make sure we don't lose anything
    for(PlaneOpp *pPOpp : m_pLLTTask->planeOppList())
    {
        Objects3d::insertPlaneOpp(pPOpp);
        m_pLastPOpp = pPOpp;
    }


    QString strong="\n";
    outputMessage(strong);
    if(m_pLLTTask->isCancelled())
    {
        strong = tr("Analysis cancelled\n\n");
    }
    else if (!m_pLLTTask->hasErrors())
        strong = tr("LLT analysis completed successfully\n\n");
    else if (m_pLLTTask->hasErrors())
        strong = tr("LLT analysis completed ... Errors encountered\n\n");
    outputMessage(strong);

    m_bHasErrors = m_pLLTTask->hasErrors();
    m_ppbCancel->setText(tr("Close"));

    emit analysisFinished(m_pLLTTask->wPolar());

    cleanUp();
}


void LLTAnalysisDlg::cleanUp()
{
    QString strange;

    m_bFinished = true;
    strange = "\n_________\n" + tr("Analysis completed");

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

    delete m_pLLTTask;
    m_pLLTTask = nullptr;
    m_ppbCancel->setText(tr("Close"));
    m_ppbCancel->setFocus();
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
        m_ppto->onAppendQText(pMsgEvent->msg());
    }
    else if(pEvent->type() == LLT_OPP_EVENT)
    {
        LLTOppEvent *pOppEvent = static_cast<LLTOppEvent*>(pEvent);

        Curve *pCurve = m_pIterGraph->addCurve();
        pCurve->setName(ALPHAch + QString::asprintf("=%.2f", pOppEvent->alpha()) + DEGch);

        m_ppto->onAppendStdText(pOppEvent->message());


        QPolygonF points(pOppEvent->max_a().size());
        for(uint i=0; i<pOppEvent->max_a().size(); i++)
        {
            points[i] = QPointF(double(i), pOppEvent->max_a().at(i));
        }

        pCurve->setPoints(points);
        m_pIterGraph->invalidate();
        m_pIterGraph->resetYLimits();
        m_pGraphWt->update();
    }
    else if(pEvent->type()==TASK3D_END_EVENT)
    {
        onTaskFinished();
    }
    else
        QDialog::customEvent(pEvent);
}


void LLTAnalysisDlg::outputMessage(const QString &msg)
{
    m_ppto->onAppendQText(msg);
}


void LLTAnalysisDlg::outputStdMessage(const std::string &msg)
{
    m_ppto->onAppendStdText(msg);
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


void LLTAnalysisDlg::analyze()
{
    if(!m_pLLTTask->plane() || !m_pLLTTask->wPolar()) return;
    //all set to launch the analysis

    m_pLLTTask->setKeepOpps(XPlane::bStoreOpps3d());

    WingXfl *pWing = m_pLLTTask->plane()->mainWing();

    if(!pWing) return;

    m_ppbCancel->setText(tr("Cancel"));
    m_bFinished = false;
    m_pLastPOpp = nullptr;

    m_ppto->clear();

    QString strange, log;
    strange = QString::fromStdString(pWing->name())+"\n";
    log = strange;
    strange = QString::fromStdString(m_pLLTTask->wPolar()->name()+"\n");
    log += strange;

    strange = "Launching analysis....\n\n";
    log += strange;
    strange = QString::asprintf("Max iterations     = %d\n", LLTTask::maxIter());
    log += strange;
    strange = ALPHAch + QString::asprintf(" precision        = %g",LLTTask::convergencePrecision()) + DEGch + "\n";
    log += strange;
    strange = QString::asprintf("Number of stations = %d\n", LLTTask::nSpanStations());
    log += strange;
    strange = QString::asprintf("Relaxation factor  = %.1f\n\n", LLTTask::relaxationFactor());
    log += strange;
    onOutputMessage(log);

    // Launch the task async to keep the UI responsive
    QFuture<void> future = QtConcurrent::run(&LLTAnalysisDlg::runAsync, this);
    (void)future;
}


void LLTAnalysisDlg::runAsync()
{
    xfl::trace("running LLT ASync\n");

    m_pLLTTask->setAnalysisStatus(xfl::RUNNING);

    std::thread p(&LLTTask::run, m_pLLTTask);

    while(!m_pLLTTask->isFinished())
    {
        // Access the Q under the lock:
        std::unique_lock<std::mutex> lck(m_pLLTTask->m_mtx);
        m_pLLTTask->m_cv.wait(lck, [this]() {return !m_pLLTTask->m_theOppQueue.empty();} );
        LLTOppReport xoppreport = m_pLLTTask->m_theOppQueue.front();
        m_pLLTTask->m_theOppQueue.pop();

        // forward to the UI thread for user notification
        if(xoppreport.max_a().size())
        {
            LLTOppEvent *pOppEvent = new LLTOppEvent(xoppreport.alpha(), xoppreport.max_a(), xoppreport.message());
            qApp->postEvent(this, pOppEvent);
        }
        else
        {
            // just the message
            qApp->postEvent(this, new MessageEvent(xoppreport.message()));
        }
    }


    p.join();

    xfl::trace("LLT thread joined\n");

    qApp->postEvent(this, new QEvent(TASK3D_END_EVENT)); // done and clean
}


