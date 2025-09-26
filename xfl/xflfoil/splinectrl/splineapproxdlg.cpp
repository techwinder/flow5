/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGridLayout>
#include <QGroupBox>
#include "splineapproxdlg.h"

#include <xflfoil/objects2d/objects2d.h>
#include <xflwidgets/customwts/intedit.h>

SplineApproxDlg::SplineApproxDlg(QWidget *pParent) : QDialog (pParent)
{
    setupLayout();
}


void SplineApproxDlg::initDialog(bool bCubic, int degree, int nCtrlPts)
{
    m_pieCtrlPoints->setValue(nCtrlPts);

    m_pcbSplineDegree->setEnabled(!bCubic);
    m_pcbSplineDegree->setCurrentIndex(degree-2);

    m_plwNames->addItems(Objects2d::foilNames());
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




