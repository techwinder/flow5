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


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStringList>
#include <QShowEvent>
#include <QHideEvent>

#include "logmessagedlg.h"

#include <interfaces/widgets/customwts/formattextoutput.h>


LogMessageDlg::LogMessageDlg(QWidget *pParent) : QWidget(pParent)
{
    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    setWindowTitle("All-purpose log message window");
    setupLayout();
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
        m_ppto = new FormatTextOutput;
        m_ppto->setReadOnly(true);
//        m_ppto->setLineWrapMode(QPlainTextEdit::NoWrap);
//        m_ppto->setWordWrapMode(QTextOption::NoWrap);
        pMainLayout->addWidget(m_ppto);
//        pMainLayout->addSpacing(20);
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


void LogMessageDlg::onAppendPlainText(QString const &msg)
{
    m_ppto->onAppendQText(msg);
    m_ppto->viewport()->update();
    m_ppto->update();
    update();
}


void LogMessageDlg::onAppendHtmlText(QString const &msg)
{
    m_ppto->appendHtmlText(msg);
    m_ppto->viewport()->update();
    m_ppto->update();
    update();
}


