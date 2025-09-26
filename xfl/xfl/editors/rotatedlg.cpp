/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QVBoxLayout>


#include "rotatedlg.h"

#include <xflwidgets/customwts/floatedit.h>

double RotateDlg::s_Angle;
int RotateDlg::s_iAxis;

QByteArray RotateDlg::s_WindowGeometry;

RotateDlg::RotateDlg(QWidget *pParent) : XflDialog(pParent)
{
    setupLayout();
}

void RotateDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGridLayout *pAxisLayout = new QGridLayout;
        {
            QLabel *pTitle = new QLabel("Rotation axis:");
            m_prbX = new QRadioButton("X");
            m_prbY = new QRadioButton("Y");
            m_prbZ = new QRadioButton("Z");
            m_prbX->setChecked(s_iAxis==0);
            m_prbY->setChecked(s_iAxis==1);
            m_prbZ->setChecked(s_iAxis==2);

            pAxisLayout->addWidget(pTitle, 1, 1);
            pAxisLayout->addWidget(m_prbX, 2, 2);
            pAxisLayout->addWidget(m_prbY, 3, 2);
            pAxisLayout->addWidget(m_prbZ, 4, 2);
        }

        QHBoxLayout *pPosLayout = new QHBoxLayout;
        {
            QLabel *plabAngle = new QLabel("Angle:");
            m_pfeAngle = new FloatEdit(s_Angle);
            QLabel *plabDegree = new QLabel("<p>&deg;</p>");

            pPosLayout->addStretch();
            pPosLayout->addWidget(plabAngle);
            pPosLayout->addWidget(m_pfeAngle);
            pPosLayout->addWidget(plabDegree);
            pPosLayout->addStretch();
        }
        m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        pMainLayout->addLayout(pAxisLayout);
        pMainLayout->addLayout(pPosLayout);
        pMainLayout->addStretch();
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void RotateDlg::showEvent(QShowEvent *pEvent)
{
    XflDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);
    m_pButtonBox->setFocus();
}


void RotateDlg::hideEvent(QHideEvent*pEvent)
{
    XflDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();
    if     (m_prbX->isChecked()) s_iAxis=0;
    else if(m_prbY->isChecked()) s_iAxis=1;
    else if(m_prbZ->isChecked()) s_iAxis=2;

    s_Angle = m_pfeAngle->value();
}


int RotateDlg::axis() const
{
    if      (m_prbX->isChecked()) return 0;
    else if (m_prbY->isChecked()) return 1;
    else if (m_prbZ->isChecked()) return 2;
    return -1;
}


double RotateDlg::angle() const
{
    return m_pfeAngle->value();
}



