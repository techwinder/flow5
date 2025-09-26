/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QPushButton>
#include <QHBoxLayout>
#include <QKeyEvent>

#include "moddlg.h"


ModDlg::ModDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Modification");
    m_Question = "";
    setupLayout();
}


void ModDlg::initDialog()
{
    m_plabQuestion->setText(m_Question);
}


void ModDlg::setupLayout()
{
    m_plabQuestion = new QLabel("Question here");

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard, this);
    {
        m_pSaveNewButton = new QPushButton("Save as new");
        {
            m_pButtonBox->addButton(m_pSaveNewButton, QDialogButtonBox::ActionRole);
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
        }
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_plabQuestion);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);
}


void ModDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)      accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton) reject();
    else if (m_pSaveNewButton == pButton)                                onSaveAsNew();
}


void ModDlg::onSaveAsNew()
{
    done(20);
}


void ModDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
            }
            else
            {
                QDialog::accept();
            }
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

