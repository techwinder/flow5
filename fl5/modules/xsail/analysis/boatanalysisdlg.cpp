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
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

#include "boatanalysisdlg.h"

#include <api/boat.h>
#include <api/boatopp.h>
#include <api/boatpolar.h>
#include <api/boattask.h>
#include <api/fl5core.h>
#include <api/flow5events.h>
#include <api/objects3d.h>
#include <api/panelanalysis.h>
#include <api/sailobjects.h>

#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <modules/xplane/analysis/analysis3dsettings.h>
#include <modules/xsail/xsail.h>

XSail *BoatAnalysisDlg::s_pXSail = nullptr;

QByteArray BoatAnalysisDlg::s_WindowGeometry;


BoatAnalysisDlg::BoatAnalysisDlg() : QDialog()
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);// | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setupLayout();
    m_pActiveTask = nullptr;

    m_bHasErrors = false;
}


BoatAnalysisDlg::~BoatAnalysisDlg()
{
}


void BoatAnalysisDlg::setupLayout()
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    m_ppto = new PlainTextOutput;
    m_ppto->setReadOnly(true);

    m_plabTaskInfo = new QLabel;

    m_pButtonBox = new QDialogButtonBox();
    {
        m_pchLiveVortons = new QCheckBox("Live VPW update");
        m_pchLiveVortons->setChecked(Task3d::bLiveUpdate());
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


void BoatAnalysisDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            if(m_pActiveTask && m_pActiveTask->isRunning())
                m_pActiveTask->onCancel();
            else
                close();
            pEvent->accept();
            return;
        }
        default:
            pEvent->ignore();
    }
}


void BoatAnalysisDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        onOutputMessage(pMsgEvent->msg());
        if(m_pActiveTask)
        {
            double ctrl = m_pActiveTask->ctrlParam();
            int nrhs = m_pActiveTask->nRHS();
            int qrhs = std::min(m_pActiveTask->qRHS(), nrhs);
            if(qrhs>=0)
                m_plabTaskInfo->setText(QString::asprintf("Processing %d/%d - ctrl parameter = %9.3f", qrhs+1, nrhs, ctrl));
        }
    }

    else if(pEvent->type()==TASK3D_END_EVENT)
    {
        onTaskFinished();
    }
    else if (pEvent->type() == VPW_UPDATE_EVENT)
    {
//        VPWUpdateEvent const *pVPWEvent = dynamic_cast<VPWUpdateEvent*>(pEvent);
//        s_pXSail->setLiveVortons(pVPWEvent->m_Ctrl, pVPWEvent->m_Vortons);
    }
    else
        QDialog::customEvent(pEvent);
}


void BoatAnalysisDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);

    restoreGeometry(s_WindowGeometry);
    m_pchKeepOpenOnErrors->setChecked(Analysis3dSettings::keepOpenOnErrors());
    m_ppbCloseDialog->setFocus();
}


void BoatAnalysisDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);

    s_WindowGeometry = saveGeometry();

    Analysis3dSettings::setKeepOpenOnErrors(m_pchKeepOpenOnErrors->isChecked());
}


void BoatAnalysisDlg::onOutputMessage(QString const &msg)
{
    m_ppto->onAppendQText(msg);
    m_ppto->update();
}


void BoatAnalysisDlg::deleteTask(BoatTask *pTask)
{
    if(pTask) delete pTask;
}


void BoatAnalysisDlg::cleanUp()
{
    QString logFileName = SaveOptions::newLogFileName();
    SaveOptions::setLastLogFileName(logFileName);

    QFile pXFile(logFileName);

    if(pXFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream outstream(&pXFile);
        outstream << m_ppto->toPlainText();
        outstream << "\n";
        outstream << "Elapsed: "<<double(m_Clock.elapsed())/1000.0<<"s";
        outstream << "\n";

        outstream.flush();
        pXFile.close();
    }


    if(m_pActiveTask) delete m_pActiveTask;
    m_pActiveTask = nullptr;
}


void BoatAnalysisDlg::onTaskFinished()
{
    if(m_pActiveTask)
    {
        m_BtOppList = m_pActiveTask->BtOppList(); // keep track of the pointers before destroying the task;
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
    }

    m_ppbCloseDialog->setText("Close");
    cleanUp();
    emit analysisFinished();
}


void BoatAnalysisDlg::onCancelClose()
{
    if(m_pActiveTask && m_pActiveTask->isRunning())
    {
        cancelTask(m_pActiveTask);
        m_ppbCloseDialog->setText("Close");
    }
    else
        close();
}


void BoatAnalysisDlg::cancelTask(BoatTask *pTask)
{
    if(pTask) pTask->onCancel();
}


bool BoatAnalysisDlg::isAnalysisRunning() const
{
    if(!m_pActiveTask) return false;
    return !m_pActiveTask->isFinished();
}


void BoatAnalysisDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("AnalysisDlg");
    {
        settings.setValue("AnalysisWindowGeom", s_WindowGeometry);
    }
    settings.endGroup();
}


void BoatAnalysisDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("AnalysisDlg");
    {
        s_WindowGeometry = settings.value("AnalysisWindowGeom").toByteArray();
    }
    settings.endGroup();
}


void BoatAnalysisDlg::onKeepOpenErrors()
{
    Analysis3dSettings::setKeepOpenOnErrors(m_pchKeepOpenOnErrors->isChecked());
}


void BoatAnalysisDlg::onStopIterations()
{
    if(m_pActiveTask && m_pActiveTask->isRunning())
    {
        if(m_pActiveTask->btPolar()->bVortonWake())
        {
            m_pActiveTask->stopVPWIterations();
        }
    }
}


void BoatAnalysisDlg::onLiveVortons()
{
    BoatTask::setLiveUpdate(m_pchLiveVortons->isChecked());
}


BoatTask* BoatAnalysisDlg::analyze(Boat *pBoat, BoatPolar *pBoatPolar, std::vector<double> const &opplist)
{
    if(pBoat->name().compare(pBoatPolar->boatName())!=0)
    {
        return nullptr;
    }

    Task3d::setCancelled(false);

    m_ppbCloseDialog->setText("Cancel");
    SailObjects::setLastBtOpp(nullptr);

    m_plabTaskInfo->clear();
    m_ppto->clear();
    onOutputMessage(QString::fromStdString(fl5::versionName(true))+"\n");

    QDateTime dt = QDateTime::currentDateTime();
    QString str = dt.toString();
    onOutputMessage(str+"\n\nLaunching analysis\n\n");

    m_pActiveTask = new BoatTask;
    if(!m_pActiveTask) return nullptr;

    m_pActiveTask->setObjects(pBoat, pBoatPolar);
    m_pActiveTask->setAnalysisRange(opplist);
    m_pActiveTask->initializeTask(this);

/*    m_pActiveTask->setEventDestination(this);
    connect(m_pActiveTask,                  &Task3d::outputMessage,    this, &BoatAnalysisDlg::onOutputMessage);
    connect(m_pActiveTask,                  &Task3d::taskFinished,     this,  &BoatAnalysisDlg::onTaskFinished);*/

    m_bHasErrors = false;

    m_Clock.start(); // put pressure on something (Jerry)


    // Launch the task async to keep the UI responsive
    QFuture<void> future = QtConcurrent::run(&BoatAnalysisDlg::runAsync, this);
    (void)future;

    return m_pActiveTask;
}


void BoatAnalysisDlg::runAsync()
{
    m_pActiveTask->setAnalysisStatus(xfl::RUNNING);

    std::thread p(&BoatTask::run, m_pActiveTask);

    while(!m_pActiveTask->isFinished())
    {
        // Access the Q under the lock:
        std::unique_lock<std::mutex> lck(m_pActiveTask->m_mtx);
        m_pActiveTask->m_cv.wait(lck, [this]() {return !m_pActiveTask->m_theMsgQueue.empty();} );
        VPWReport report = m_pActiveTask->m_theMsgQueue.front();
        m_pActiveTask->m_theMsgQueue.pop();

        if(report.m_Vortons.size()!=0)
            s_pXSail->setLiveVortons(report.m_Ctrl, report.m_Vortons);

        // forward to the UI thread for user notification
        qApp->postEvent(this, new MessageEvent(report.message()));
    }

    p.join();

    qApp->postEvent(this, new QEvent(TASK3D_END_EVENT)); // done and clean
}

