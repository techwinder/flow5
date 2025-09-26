/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QVBoxLayout>
#include <QKeyEvent>


#include "logwt.h"
#include <xflwidgets/customwts/plaintextoutput.h>
#include <xflcore/flow5events.h>

QByteArray LogWt::s_Geometry;


LogWt::LogWt(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Analysis log report");

    m_bFinished = false;
    setupLayout();
}


void LogWt::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout();
    {
        m_pptoLogView = new PlainTextOutput;
        QHBoxLayout *pCtrlLayout = new QHBoxLayout;
        {
            m_ppbButton = new QPushButton("Cancel/Close");
            connect(m_ppbButton, SIGNAL(clicked(bool)), this, SLOT(onCancelClose()));
            pCtrlLayout->addStretch();
            pCtrlLayout->addWidget(m_ppbButton);
            pCtrlLayout->addStretch();
        }
        pMainLayout->addWidget(m_pptoLogView);
        pMainLayout->addLayout(pCtrlLayout);
    }
    setLayout(pMainLayout);
}


void LogWt::onOutputMessage(QString const &msg)
{
    m_pptoLogView->insertPlainText(msg);
    m_pptoLogView->textCursor().movePosition(QTextCursor::End);
    m_pptoLogView->ensureCursorVisible();
}


void LogWt::onCancelClose()
{
    if(m_bFinished)
    {
        hide();
    }
    else
    {
        m_ppbButton->setText("Cancelling...");
        m_ppbButton->setEnabled(false);
        onOutputMessage("\n_____________Cancel request emitted_____________\n\n");
        
    }
}


void LogWt::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
    case Qt::Key_Escape:
        m_ppbButton->animateClick();
        pEvent->accept();
        break;
    default:
        pEvent->ignore();
        break;
    }
}


void LogWt::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void LogWt::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void LogWt::setCancelButton(bool bCancel)
{
    if(bCancel) m_ppbButton->setText("Cancel");
    else        m_ppbButton->setText("Close");
}


void LogWt::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        onOutputMessage(pMsgEvent->msg());
    }
    else
        QWidget::customEvent(pEvent);
}


void LogWt::setFinished(bool bFinished)
{
    m_bFinished = bFinished;
    if(bFinished)
    {
        m_ppbButton->setText("Close");
        m_ppbButton->setEnabled(true);
        m_ppbButton->setFocus();
    }
}


void LogWt::loadSettings(QSettings &settings)
{
    settings.beginGroup("LogWt");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();
    }
    settings.endGroup();
}


void LogWt::saveSettings(QSettings &settings)
{
    settings.beginGroup("LogWt");
    {
        settings.setValue("WindowGeometry", s_Geometry);
    }
    settings.endGroup();
}





