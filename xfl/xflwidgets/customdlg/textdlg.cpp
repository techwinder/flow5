/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#include "textdlg.h"


#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "textdlg.h"

QByteArray TextDlg::s_WindowGeometry;

TextDlg::TextDlg(QString const &text, QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Text dialog");

    setupLayout();

    m_ppteText->setPlainText(text);
}


void TextDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_plabQuestion = new QLabel("Description:");
        m_ppteText = new QPlainTextEdit(this);

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
        }
        pMainLayout->addWidget(m_plabQuestion);
        pMainLayout->addWidget(m_ppteText);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void TextDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok)     == pButton) accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton) reject();
}


void TextDlg::accept()
{
    m_NewText = m_ppteText->toPlainText();
    QDialog::accept();
}


void TextDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus()) m_pButtonBox->setFocus();
            else accept();

            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
        default:
            pEvent->ignore();
    }
}


void TextDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);
}


void TextDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();
}








