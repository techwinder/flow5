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


#include <QApplication>
#include <QDir>
#include <QDateTime>
#include <QTimer>
#include <QHBoxLayout>
#include <QFontDatabase>
#include <QShowEvent>
#include <QPushButton>
#include <QtConcurrent/QtConcurrent>

#include "xfoilanalysisdlg.h"


#include <api/flow5events.h>
#include <api/foil.h>
#include <api/fl5core.h>
#include <api/objects2d.h>
#include <api/oppoint.h>
#include <api/polar.h>
#include <api/xfoiltask.h>

#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/graphs/graph/curve.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <modules/xdirect/xdirect.h>


QByteArray XFoilAnalysisDlg::s_WindowGeometry;

XFoilAnalysisDlg::XFoilAnalysisDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("XFoil Analysis");

    m_pXFoilTask = nullptr;

    m_pXFile   = nullptr;
    m_pFoil    = nullptr;
    m_pPolar   = nullptr;
    m_pLastOpp = nullptr;

    m_bErrors     = false;

    setupLayout();
}


XFoilAnalysisDlg::~XFoilAnalysisDlg()
{
    if(m_pXFoilTask) delete m_pXFoilTask;
    if(m_pXFile) delete m_pXFile;
}


void XFoilAnalysisDlg::setupLayout()
{
    m_ppto = new PlainTextOutput;

    QHBoxLayout *pButtonsLayout = new QHBoxLayout;
    {
        m_ppbCancel = new QPushButton("Cancel");

        connect(m_ppbCancel, SIGNAL(clicked()), SLOT(onCancelClose()));

        m_pchKeepOpen = new QCheckBox("Keep this window opened on errors");
        connect(m_pchKeepOpen, SIGNAL(toggled(bool)), SLOT(onKeepOpen(bool)));

        pButtonsLayout->addWidget(m_pchKeepOpen);
        pButtonsLayout->addStretch();
        pButtonsLayout->addWidget(m_ppbCancel);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_ppto);
        pMainLayout->addLayout(pButtonsLayout);
        setLayout(pMainLayout);
    }
}


void XFoilAnalysisDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);
}


void XFoilAnalysisDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();
}


void XFoilAnalysisDlg::initializeAnalysis(Foil *pFoil, Polar* pPolar, QVector<AnalysisRange> const &ranges)
{
    m_pFoil  = pFoil;
    m_pPolar = pPolar;

    m_pchKeepOpen->setChecked(XDirect::bKeepOpenOnErrors());

    QString FileName = SaveOptions::newLogFileName();
    SaveOptions::setLastLogFileName(FileName);

    m_pXFile = new QFile(FileName);
    if (!m_pXFile->open(QIODevice::WriteOnly | QIODevice::Text)) m_pXFile = nullptr;
    setFileHeader();

    if(!m_pXFoilTask)
    {
        m_pXFoilTask = new XFoilTask();
    }

    m_pXFoilTask->setAoAAnalysis(XDirect::bAlpha());


    m_pXFoilTask->clearRanges();
    for (AnalysisRange rg : ranges)
    {
        m_pXFoilTask->appendRange(rg);
    }

    m_pXFoilTask->initialize(m_pFoil, m_pPolar, XDirect::bStoreOpps());

    m_ppto->clear();

    m_pLastOpp = nullptr;
}


void XFoilAnalysisDlg::onCancelClose()
{
    XFoil::s_bCancel= true;
    XFoilTask::setCancelled(true);
    if(m_pXFoilTask) m_pXFoilTask->setAnalysisStatus(xfl::CANCELLED);
    else
        reject();
}


void XFoilAnalysisDlg::onKeepOpen(bool bChecked)
{
    XDirect::setKeepOpenOnErrors(bChecked);
}


void XFoilAnalysisDlg::setFileHeader()
{
    if(!m_pXFile) return;
    QTextStream out(m_pXFile);

    out << "\n";
    out << QString::fromStdString(fl5::versionName(true));
    out << "\n";
    out << QString::fromStdString(m_pFoil->name());
    out << "\n";
    if(m_pPolar)
    {
        out << QString::fromStdString(m_pPolar->name());
        out << "\n";
    }

    QDateTime dt = QDateTime::currentDateTime();
    QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");

    out << str;
    out << "\n___________________________________\n\n";
}


void XFoilAnalysisDlg::onOutputMessage(QString const &msg)
{
    m_ppto->onAppendQText(msg);
}


void XFoilAnalysisDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("XFoilAnalysisDlg");
    {
        s_WindowGeometry = settings.value("WindowGeom").toByteArray();
    }
    settings.endGroup();
}


void XFoilAnalysisDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("XFoilAnalysisDlg");
    {
        settings.setValue("WindowGeom", s_WindowGeometry);
    }
    settings.endGroup();
}


QSize XFoilAnalysisDlg::sizeHint() const
{
    QFont fnt;
    QFontMetrics fm(fnt);
    return QSize(125*fm.averageCharWidth(), 50*fm.height());
}


void XFoilAnalysisDlg::onTaskFinished()
{
    if(!m_pXFoilTask) return;

    QString strong="\n";
    onOutputMessage(strong);
    if(m_pXFoilTask->isCancelled())
    {
        strong = "Analysis cancelled\n\n";
    }
    else if (!m_pXFoilTask->hasErrors())
        strong = "Analysis completed successfully\n\n";
    else if (m_pXFoilTask->hasErrors())
        strong = "Analysis completed ...errors encountered\n\n";
    m_ppto->onAppendQText(strong);

    // retrieve and store the operating points before deleting the task
    bool bStoreOpps = XDirect::bStoreOpps();
    m_pLastOpp = nullptr;
    for(OpPoint *pOpp : m_pXFoilTask->operatingPoints())
    {
        Objects2d::addOpPoint(pOpp, bStoreOpps);
        if(!bStoreOpps) delete pOpp;
        else m_pLastOpp = pOpp;
    }
    m_pXFoilTask->clearOpps(); // you never know


    m_ppbCancel->setText("Close");

    m_bErrors = m_pXFoilTask->hasErrors();

    if(m_pXFile)
    {
        QTextStream stream(m_pXFile);
        stream << m_ppto->toPlainText();
        m_pXFile->close();
    }

    delete m_pXFoilTask;
    m_pXFoilTask = nullptr;

    emit analysisFinished();

    update();
}


void XFoilAnalysisDlg::customEvent(QEvent * pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent const *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        m_ppto->onAppendQText(pMsgEvent->msg());
    }
    else if(pEvent->type() == XFOIL_TASK_END_EVENT)
    {
        onTaskFinished();
    }
}


void XFoilAnalysisDlg::start()
{
    m_ppbCancel->setText("Cancel");

    // Launch the task async to keep the UI responsive
    QFuture<void> future = QtConcurrent::run(&XFoilAnalysisDlg::runAsync, this);
    (void)future;
}


void XFoilAnalysisDlg::runAsync()
{
    m_pXFoilTask->setAnalysisStatus(xfl::RUNNING);

    std::thread p(&XFoilTask::run, m_pXFoilTask);

    while(!m_pXFoilTask->isFinished())
    {
        // Access the Q under the lock:
        std::unique_lock<std::mutex> lck(m_pXFoilTask->m_mtx);
        m_pXFoilTask->m_cv.wait(lck, [this]() {return !m_pXFoilTask->m_theMsgQueue.empty();} );

        std::string xoppreport = m_pXFoilTask->m_theMsgQueue.front();
        m_pXFoilTask->m_theMsgQueue.pop();

        // forward to the UI thread for user notification
        MessageEvent *pMsgEvent = new MessageEvent(xoppreport);
        qApp->postEvent(this, pMsgEvent);
    }


    p.join();

    qApp->postEvent(this, new XFoilTaskEvent(m_pXFoilTask)); // done and clean
}








