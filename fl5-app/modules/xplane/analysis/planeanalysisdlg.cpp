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


#include <QApplication>
#include <QHBoxLayout>
#include <QDateTime>
#include <QLabel>
#include <QKeyEvent>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

#include "planeanalysisdlg.h"

#include <api/analysisrange.h>
#include <api/fl5core.h>
#include <api/flow5events.h>
#include <api/objects3d.h>
#include <api/panelanalysis.h>
#include <api/planeopp.h>
#include <api/planepolar.h>
#include <api/planetask.h>
#include <api/planexfl.h>
#include <api/utils.h>
#include <api/xfoiltask.h>

#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <modules/xplane/analysis/analysis3dsettings.h>
#include <modules/xplane/glview/gl3dxplaneview.h>
#include <modules/xplane/xplane.h>


XPlane *PlaneAnalysisDlg::s_pXPlane = nullptr;

QByteArray PlaneAnalysisDlg::s_Geometry;


PlaneAnalysisDlg::PlaneAnalysisDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Plane analysis");
    setWindowFlag(Qt::WindowStaysOnTopHint);// | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setWindowFlag(Qt::WindowMinMaxButtonsHint);
    setupLayout();

    m_pActiveTask = nullptr;

    m_pLastPOpp = nullptr;
//    m_pLastLivePOpp = nullptr;

    m_bHasErrors = false;
}


void PlaneAnalysisDlg::setupLayout()
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    m_ppto = new PlainTextOutput;
    m_ppto->setReadOnly(true);

    m_plabTaskInfo = new QLabel;
    m_pButtonBox = new QDialogButtonBox();
    {
        m_pchLiveVortons = new QCheckBox("Live VPW update");
        if(s_pXPlane && s_pXPlane->curPlPolar())
        {
            m_pchLiveVortons->setChecked(Task3d::bLiveUpdate() && s_pXPlane->curPlPolar()->bVortonWake());
            m_pchLiveVortons->setEnabled(s_pXPlane->curPlPolar()->bVortonWake());
        }
        m_pButtonBox->addButton(m_pchLiveVortons, QDialogButtonBox::ActionRole);
        connect(m_pchLiveVortons, SIGNAL(clicked(bool)), SLOT(onLiveVortons()));

        m_pchKeepOpenOnErrors = new QCheckBox("Keep opened on errors");
        m_pButtonBox->addButton(m_pchKeepOpenOnErrors, QDialogButtonBox::ActionRole);
        connect(m_pchKeepOpenOnErrors, SIGNAL(clicked(bool)), SLOT(onKeepOpenErrors()));

        m_ppbStopIter = new QPushButton("End iterations");
        m_pButtonBox->addButton(m_ppbStopIter, QDialogButtonBox::ActionRole);
        connect(m_ppbStopIter, SIGNAL(clicked(bool)), SLOT(onStopIterations()));

        m_ppbCloseDialog = new QPushButton("Cancel/Close");
        m_pButtonBox->addButton(m_ppbCloseDialog, QDialogButtonBox::ActionRole);
        connect(m_ppbCloseDialog, SIGNAL(clicked(bool)), SLOT(onCancelClose()));
        m_ppbCloseDialog->setDefault(true);
        m_ppbCloseDialog->setAutoDefault(true);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_ppto);
        pMainLayout->addWidget(m_plabTaskInfo);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void PlaneAnalysisDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            if(m_pActiveTask && m_pActiveTask->isRunning())
                m_pActiveTask->cancelTask();
            else
                close();
            pEvent->accept();
            return;
        }
        default:
            pEvent->ignore();
    }
}


void PlaneAnalysisDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);

    if(s_pXPlane && s_pXPlane->curPlPolar())
    {
        m_ppbStopIter->setVisible(s_pXPlane && s_pXPlane->curPlPolar()->isType6() && s_pXPlane->curPlPolar()->bVortonWake());
    }
    m_pchKeepOpenOnErrors->setChecked(Analysis3dSettings::keepOpenOnErrors());
    m_ppbCloseDialog->setFocus();
}


void PlaneAnalysisDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
    Analysis3dSettings::setKeepOpenOnErrors(m_pchKeepOpenOnErrors->isChecked());
}


void PlaneAnalysisDlg::cleanUp()
{
    onOutputMessage("Cleaning up\n");

    QString logFileName = SaveOptions::newLogFileName();

    SaveOptions::setLastLogFileName(logFileName);

    QFile pXFile(logFileName);

    QString strange = QString::asprintf("Elapsed: %.2f s\n", double(m_Clock.elapsed())/1000.0);
    onOutputMessage(strange);

    if(pXFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream outstream(&pXFile);
        outstream << m_ppto->toPlainText();
        outstream << "\n";


        outstream.flush();
        pXFile.close();
    }

    if(m_pActiveTask) delete m_pActiveTask;
    m_pActiveTask = nullptr;
}


void PlaneAnalysisDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent const *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        m_ppto->onAppendQText(pMsgEvent->msg());

        if(m_pActiveTask && (m_pActiveTask->wPolar()->isType6() || m_pActiveTask->wPolar()->isType7()))
        {
            double ctrl = m_pActiveTask->ctrl();
            int nrhs = m_pActiveTask->nRHS();
            int qrhs = std::min(m_pActiveTask->qRHS(), nrhs-1);
            if(qrhs>=0)
                m_plabTaskInfo->setText(QString::asprintf("Processing %d/%d - ctrl parameter = %9.3f", qrhs+1, nrhs, ctrl));
        }
    }
    else if(pEvent->type()==TASK3D_END_EVENT)
    {
        onTaskFinished();
    }
    else
        QDialog::customEvent(pEvent);
}


void PlaneAnalysisDlg::onOutputMessage(QString const &msg)
{
    m_ppto->onAppendQText(msg);
    m_ppto->update();
}


void PlaneAnalysisDlg::onTaskFinished()
{
    if(!m_pActiveTask) return;

    m_pLastPOpp = nullptr;

    if(m_pActiveTask->planeOppList().size())
        m_pLastPOpp = m_pActiveTask->planeOppList().back();


    QString strong="\n";
    onOutputMessage(strong);
    if(m_pActiveTask->isCancelled())
    {
        strong = "Analysis cancelled\n\n";
    }
    else if (!m_pActiveTask->hasErrors())
        strong = "Panel analysis completed successfully\n\n";
    else if (m_pActiveTask->hasErrors())
        strong = "Panel analysis completed ... Errors encountered\n\n";
    onOutputMessage(strong);

    m_bHasErrors = m_pActiveTask->hasErrors();
    m_ppbCloseDialog->setText("Close");

    PlanePolar *pWPolar = m_pActiveTask->wPolar();
    cleanUp();

    emit analysisFinished(pWPolar);
}


void PlaneAnalysisDlg::onKeepOpenErrors()
{
    Analysis3dSettings::setKeepOpenOnErrors(m_pchKeepOpenOnErrors->isChecked());
}


void PlaneAnalysisDlg::onStopIterations()
{
    if(m_pActiveTask && m_pActiveTask->isRunning())
    {
        if(m_pActiveTask->wPolar()->isType6() && m_pActiveTask->wPolar()->bVortonWake())
        {
            m_pActiveTask->stopVPWIterations();
        }
    }
}


void PlaneAnalysisDlg::onCancelClose()
{
    if(m_pActiveTask && m_pActiveTask->isRunning())
    {
        cancelTask(m_pActiveTask);
        m_ppbCloseDialog->setText("Close");
    }
    else
        close();
}


void PlaneAnalysisDlg::cancelTask(PlaneTask *pTask)
{
    TriMesh::setCancelled(true);
    XFoilTask::setCancelled(true);
    if(pTask) pTask->cancelTask();
}


bool PlaneAnalysisDlg::isAnalysisRunning() const
{
    if(!m_pActiveTask) return false;
    return !m_pActiveTask->isFinished();
}


bool PlaneAnalysisDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("PlaneAnalysisDlg");
    {
        settings.setValue("Geometry", s_Geometry);
    }
    settings.endGroup();

    return true;
}


bool PlaneAnalysisDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("PlaneAnalysisDlg");
    {
        s_Geometry = settings.value("Geometry").toByteArray();
    }
    settings.endGroup();
    return true;
}


bool PlaneAnalysisDlg::bIsRunning() const
{
    return m_pActiveTask && m_pActiveTask->isRunning();
}


void PlaneAnalysisDlg::onLiveVortons()
{
    PlaneTask::setLiveUpdate(m_pchLiveVortons->isChecked());
}


PlaneTask* PlaneAnalysisDlg::analyze(Plane *pPlane, PlanePolar *pPlPolar, std::vector<double> const &opplist, std::vector<T8Opp> const &ranges)
{
    if(pPlane->name().compare(pPlPolar->planeName())!=0)
    {
        return nullptr;
    }
    Task3d::setCancelled(false);
    TriMesh::setCancelled(false);
    PanelAnalysis::setMultiThread(xfl::isMultiThreaded());
    PanelAnalysis::setMaxThreadCount(xfl::maxThreadCount());

    m_Clock.start(); // put pressure on something (Jerry)

    m_ppbCloseDialog->setText("Cancel");
    m_ppto->clear();
    m_plabTaskInfo->clear();

    onOutputMessage(QString::fromStdString(fl5::versionName(true)) + EOLch);

    QDateTime dt = QDateTime::currentDateTime();
    QString str = dt.toString();
    onOutputMessage(str+"\n\n");

    onOutputMessage("Launching analysis\n\n");

    update();


    m_pActiveTask = new PlaneTask;
    m_pActiveTask->setKeepOpps(XPlane::bStoreOpps3d());

    m_pActiveTask->setObjects(pPlane, pPlPolar);

    m_pActiveTask->setComputeDerivatives(XPlane::bStoreOpps3d() && Analysis3dSettings::bStabDerivatives());

    if (pPlPolar->isType123() || pPlPolar->isType4() || pPlPolar->isType5())
                                    m_pActiveTask->setOppList(opplist);
    else if(pPlPolar->isType6())     m_pActiveTask->setCtrlOppList(opplist);
    else if(pPlPolar->isType7())     m_pActiveTask->setStabOppList(opplist);
    else if(pPlPolar->isType8())     m_pActiveTask->setT8OppList(ranges);

    m_bHasErrors = false;

    // Launch the task async to keep the UI responsive
    QFuture<void> future = QtConcurrent::run(&PlaneAnalysisDlg::runAsync, this);
    (void)future;

    return m_pActiveTask;
}


void PlaneAnalysisDlg::runAsync()
{
    m_pActiveTask->setAnalysisStatus(xfl::RUNNING);

    std::thread p(&PlaneTask::run, m_pActiveTask);

    while(!m_pActiveTask->isFinished() || !m_pActiveTask->m_theMsgQueue.empty())
    {
        // Access the Q under the lock:
        std::unique_lock<std::mutex> lck(m_pActiveTask->m_mtx);
        m_pActiveTask->m_cv.wait(lck, [this]() {return !m_pActiveTask->m_theMsgQueue.empty();} );
        VPWReport report = m_pActiveTask->m_theMsgQueue.front();
        m_pActiveTask->m_theMsgQueue.pop();

        // forward to the UI thread for user notification
        qApp->postEvent(this, new MessageEvent(report.message()));

        if(report.m_Vortons.size()!=0)
            s_pXPlane->setLiveVortons(report.m_Ctrl, report.m_Vortons);
    }

    p.join();

    qApp->postEvent(this, new QEvent(TASK3D_END_EVENT)); // done and clean
}
