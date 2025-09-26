/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QVBoxLayout>

#include <xflwidgets/customwts/floatedit.h>

#include "scaledlg.h"

ScaleDlg::ScaleDlg(QWidget *pParent) : XflDialog(pParent)
{
    setupLayout();
}


void ScaleDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGridLayout *pScaleLayout = new QGridLayout;
        {
            m_pdeXFactor = new FloatEdit(1.0);
            m_pdeYFactor = new FloatEdit(1.0);
            m_pdeZFactor = new FloatEdit(1.0);
            QLabel *plab0 = new QLabel("Scale factor");
            QLabel *plab1 = new QLabel("X Scale:");
            QLabel *plab2 = new QLabel("Y Scale:");
            QLabel *plab3 = new QLabel("Z Scale:");
            pScaleLayout->addWidget(plab0,        1,2, Qt::AlignCenter);
            pScaleLayout->addWidget(plab1,        2,1, Qt::AlignRight | Qt::AlignVCenter);
            pScaleLayout->addWidget(plab2,        3,1, Qt::AlignRight | Qt::AlignVCenter);
            pScaleLayout->addWidget(plab3,        4,1, Qt::AlignRight | Qt::AlignVCenter);
            pScaleLayout->addWidget(m_pdeXFactor, 2,2);
            pScaleLayout->addWidget(m_pdeYFactor, 3,2);
            pScaleLayout->addWidget(m_pdeZFactor, 4,2);
        }
        m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok |QDialogButtonBox::Cancel);

        pMainLayout->addLayout(pScaleLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


double ScaleDlg::XFactor() const {return m_pdeXFactor->value();}
double ScaleDlg::YFactor() const {return m_pdeYFactor->value();}
double ScaleDlg::ZFactor() const {return m_pdeZFactor->value();}




