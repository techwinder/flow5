/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "newnamedlg.h"

QByteArray NewNameDlg::s_WindowGeometry;

NewNameDlg::NewNameDlg(QString const &name, QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Name Dialog");

    setupLayout();

    m_pleName->setText(name);
    m_pleName->selectAll();
    QFont font;
    QFontMetrics fm(font);
    int w=0;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    w = fm.horizontalAdvance(name);
#else
    w = fm.width(m_OldName);
#endif
    m_pleName->setMinimumWidth(w+50);

}


void NewNameDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_plabQuestion = new QLabel("Name:");
        m_pleName = new QLineEdit(this);

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
        }
        pMainLayout->addWidget(m_plabQuestion);
        pMainLayout->addWidget(m_pleName);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void NewNameDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok)     == pButton) accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton) reject();
}


void NewNameDlg::accept()
{
    m_NewName = m_pleName->text();
    QDialog::accept();
}


void NewNameDlg::keyPressEvent(QKeyEvent *pEvent)
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


void NewNameDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);
}


void NewNameDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();
}







