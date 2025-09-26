/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QApplication>
#include <QDir>
#include <QDateTime>
#include <QTimer>
#include <QHBoxLayout>
#include <QFontDatabase>
#include <QShowEvent>
#include <QPushButton>

#include "xfoilanalysisdlg.h"

#include <xfl/xdirect/xdirect.h>
#include <xflfoil/analysis2d/xfoiltask.h>
#include <xflcore/flow5events.h>
#include <xflcore/saveoptions.h>
#include <xflcore/xflcore.h>
#include <xflgraph/graph/curve.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/objects2d.h>
#include <xflfoil/objects2d/oppoint.h>
#include <xflfoil/objects2d/polar.h>

#include <xflwidgets/customwts/plaintextoutput.h>


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
        m_ppbSkip   = new QPushButton("Skip");
        m_ppbCancel = new QPushButton("Cancel");

        connect(m_ppbSkip,   SIGNAL(clicked()), this, SLOT(onSkipPoint()));
        connect(m_ppbCancel, SIGNAL(clicked()), this, SLOT(onCancelClose()));

        m_pchKeepOpen = new QCheckBox("Keep this window opened on errors");
        connect(m_pchKeepOpen, SIGNAL(toggled(bool)), this, SLOT(onKeepOpen(bool)));

        pButtonsLayout->addWidget(m_pchKeepOpen);
        pButtonsLayout->addStretch(1);
        pButtonsLayout->addWidget(m_ppbSkip);
        pButtonsLayout->addStretch(1);
        pButtonsLayout->addWidget(m_ppbCancel);
        pButtonsLayout->addStretch(1);
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
        m_pXFoilTask = new XFoilTask(this);

    connect(m_pXFoilTask,                  &XFoilTask::outputMessage,    this, &XFoilAnalysisDlg::onOutputMessage);

    m_pXFoilTask->setAoAAnalysis(XFoilTask::bAlpha());
    m_pXFoilTask->setAnalysisRanges(ranges);

    m_pXFoilTask->initializeTask(m_pFoil, m_pPolar,
                                 XFoilTask::bStoreOpps(), XFoilTask::bViscous(), XFoilTask::bInitBL());

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


void XFoilAnalysisDlg::onSkipPoint()
{
    XFoilTask::setSkipOpp(true);
}


void XFoilAnalysisDlg::setFileHeader()
{
    if(!m_pXFile) return;
    QTextStream out(m_pXFile);

    out << "\n";
    out << xfl::versionName(true);
    out << "\n";
    out << m_pFoil->name();
    out << "\n";
    if(m_pPolar)
    {
        out << m_pPolar->name();
        out << "\n";
    }

    QDateTime dt = QDateTime::currentDateTime();
    QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");

    out << str;
    out << "\n___________________________________\n\n";
}


void XFoilAnalysisDlg::analyze()
{
    m_ppbCancel->setText("Cancel");
    m_ppbSkip->setEnabled(true);

    //Launch the task
    m_pXFoilTask->setAnalysisStatus(xfl::RUNNING);

    //run the task asynchronously
    QThread *pThread = new QThread;
    m_pXFoilTask->moveToThread(pThread);
    connect(pThread,       SIGNAL(started()),          m_pXFoilTask, SLOT(run()));
//    connect(pThread,       SIGNAL(finished()),         pThread,       SLOT(deleteLater()));

    pThread->start();
    pThread->setPriority(xfl::threadPriority());
}


void XFoilAnalysisDlg::onOutputMessage(QString const &msg)
{
    m_ppto->onAppendThisPlainText(msg);
    m_ppto->update();
}


void XFoilAnalysisDlg::onTaskFinished()
{
    if(!m_pXFoilTask) return;

    if(m_pXFoilTask->thread())
        m_pXFoilTask->thread()->exit(0);

    QString strong="\n";
    onOutputMessage(strong);
    if(m_pXFoilTask->isCancelled())
    {
        strong = "Analysis cancelled\n\n";
    }
    else if (!m_pXFoilTask->hasErrors())
        strong = "Analysis completed successfully\n\n";
    else if (m_pXFoilTask->hasErrors())
        strong = "Analysis completed ... Errors encountered\n\n";
    onOutputMessage(strong);

    m_ppbCancel->setText("Close");
    m_ppbSkip->setEnabled(false);

    m_bErrors = m_pXFoilTask->m_bErrors;
    if(m_bErrors)
    {
        m_ppto->insertPlainText(" ...some points are unconverged");
        m_ppto->ensureCursorVisible();
    }

    m_pXFoilTask->flushOutStream();

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


void XFoilAnalysisDlg::onProgress()
{
    m_ppto->onAppendThisPlainText(m_pXFoilTask->logMessage());
    m_pXFoilTask->clearLog();
    update();
}


void XFoilAnalysisDlg::customEvent(QEvent * pEvent)
{
    if(pEvent->type() == XFOIL_END_TASK_EVENT)
    {
        onTaskFinished();
    }
    else if(pEvent->type() == XFOIL_END_OPP_EVENT)
    {
        XFoilOppEvent *pOppEvent = static_cast<XFoilOppEvent*>(pEvent);
        OpPoint *pNewOpp = Objects2d::addOpPoint(pOppEvent->foilPtr(), pOppEvent->polarPtr(), pOppEvent->theOpPoint(),
                                                 XFoilTask::bStoreOpps());

        if(pNewOpp)
        {
            pNewOpp->setVisible(true); // even if the parent polar is turned off
            m_pLastOpp = pNewOpp;
        }
    }
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


