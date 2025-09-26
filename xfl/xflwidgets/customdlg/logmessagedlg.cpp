/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStringList>
#include <QShowEvent>
#include <QHideEvent>

#include "logmessagedlg.h"

#include <xflwidgets/customwts/plaintextoutput.h>


LogMessageDlg::LogMessageDlg(QWidget *pParent) : QWidget(pParent)
{
    setWindowTitle("All-purpose log message window");
    setupLayout();
}


void LogMessageDlg::setOnTop()
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
}


void LogMessageDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = pEvent->modifiers() & Qt::ControlModifier;

    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            onClose();
            return;
        }
        case Qt::Key_W:
        {
            if(bCtrl)
            {
                hide();
            }
            break;
        }
        default:
            pEvent->ignore();
            break;
    }
}


void LogMessageDlg::onClose()
{
    hide();
    emit closeLog();
}


void LogMessageDlg::clearText()
{
    m_ppto->clear();
}


void LogMessageDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)  onClose();
    else if (m_ppbClearButton == pButton)  clearText();
}



void LogMessageDlg::setupLayout()
{
    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    {
        m_ppbClearButton= new QPushButton("Clear text");
        m_pButtonBox->addButton(m_ppbClearButton, QDialogButtonBox::ActionRole);

        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout * pMainLayout = new QVBoxLayout(this);
    {
        m_ppto = new PlainTextOutput;
        m_ppto->setReadOnly(true);
        m_ppto->setLineWrapMode(QPlainTextEdit::NoWrap);
        m_ppto->setWordWrapMode(QTextOption::NoWrap);
        pMainLayout->addWidget(m_ppto);
        pMainLayout->addSpacing(20);
        pMainLayout->addWidget(m_pButtonBox);

    }

    setLayout(pMainLayout);
}


void LogMessageDlg::initDialog(QString const &title, QString const &props)
{
    setWindowTitle(title);
    m_ppto->insertPlainText(props);
}


void LogMessageDlg::setOutputFont(QFont const &fnt)
{
    m_ppto->setFont(fnt);
}


void LogMessageDlg::onAppendMessage(QString const &msg)
{
    m_ppto->onAppendThisPlainText(msg);
    m_ppto->viewport()->update();
    m_ppto->update();
    update();
}



