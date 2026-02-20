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
#include <QLabel>
#include <QPushButton>

#include "lecircledlg.h"
#include <globals/mainframe.h>
#include <interfaces/widgets/customwts/floatedit.h>


LECircleDlg::LECircleDlg(MainFrame *pMainFrame): XflDialog(pMainFrame)
{
    setWindowTitle(tr("L.E. Circle"));
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



