/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGridLayout>
#include <QGroupBox>
#include "splineapproxdlg.h"

#include <api/objects2d.h>
#include <interfaces/widgets/customwts/intedit.h>

SplineApproxDlg::SplineApproxDlg(QWidget *pParent) : QDialog (pParent)
{
    setupLayout();
}


void SplineApproxDlg::initDialog(bool bCubic, int degree, int nCtrlPts)
{
    m_pieCtrlPoints->setValue(nCtrlPts);

    m_pcbSplineDegree->setEnabled(!bCubic);
    m_pcbSplineDegree->setCurrentIndex(degree-2);

    for(std::string const &name : Objects2d::foilNames())
        m_plwNames->addItem(QString::fromStdString(name));
}


void SplineApproxDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGridLayout *pParamsLayout = new QGridLayout;
        {
            QLabel *pLabDeg = new QLabel("Degree=");
            m_pcbSplineDegree = new QComboBox;
            for (int i=2; i<10; i++)
            {
                QString str = QString::asprintf("%7d",i);
                m_pcbSplineDegree->addItem(str);
            }
            QLabel *pLabCtrlPts = new QLabel("Nbr. of control points=");
            m_pieCtrlPoints = new IntEdit();
            pParamsLayout->addWidget(pLabDeg,             1, 1);
            pParamsLayout->addWidget(m_pcbSplineDegree,   1, 2);
            pParamsLayout->addWidget(pLabCtrlPts,         2, 1);
            pParamsLayout->addWidget(m_pieCtrlPoints,     2, 2);
            pParamsLayout->setColumnStretch(3,1);
        }


        m_plwNames = new QListWidget;


        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));

        pMainLayout->addLayout(pParamsLayout);
        pMainLayout->addWidget(m_plwNames);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void SplineApproxDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)      accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}

int SplineApproxDlg::degree() const
{
    return m_pcbSplineDegree->currentIndex()+2;
}


int SplineApproxDlg::nCtrlPoints() const
{
    return m_pieCtrlPoints->value();
}




