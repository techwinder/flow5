/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "lecircledlg.h"
#include <xfl/globals/mainframe.h>
#include <xflwidgets/customwts/floatedit.h>


LECircleDlg::LECircleDlg(MainFrame *pMainFrame): XflDialog(pMainFrame)
{
    setWindowTitle("L.E. Circle");
    setupLayout();
    s_pMainFrame = pMainFrame;
}


void LECircleDlg::setupLayout()
{
    QHBoxLayout *pLERadiusLayout = new QHBoxLayout;
    {
        m_pdeRadius = new FloatEdit(0.0,3);
        QLabel *plab0 = new QLabel("r=");
        QLabel *plab1 = new QLabel("% Chord");
        plab0->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        pLERadiusLayout->addStretch(1);
        pLERadiusLayout->addWidget(plab0);
        pLERadiusLayout->addWidget(m_pdeRadius);
        pLERadiusLayout->addWidget(plab1);
    }

    m_pchShow = new QCheckBox("Show");

    setButtons(QDialogButtonBox::Ok);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pchShow);
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pLERadiusLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);
}


void LECircleDlg::initDialog()
{
    m_pdeRadius->setValue(m_Radius*100.0);
    m_pchShow->setChecked(m_bShowRadius);
}


void LECircleDlg::accept()
{
    m_Radius = m_pdeRadius->value()/100.0;
    m_bShowRadius = m_pchShow->isChecked();
    XflDialog::accept();
}



