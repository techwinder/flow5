/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>

#include "translatedlg.h"
#include <xflwidgets/customwts/floatedit.h>
#include <xflcore/units.h>

QByteArray TranslateDlg::s_WindowGeometry;


TranslateDlg::TranslateDlg(QWidget *pParent) : XflDialog(pParent)
{
    setupLayout();
}


void TranslateDlg::setupLayout()
{
    QGridLayout *pPosLayout = new QGridLayout;
    {
        QLabel *plabTitle = new QLabel("Enter the translation vector:");
        m_pdeX = new FloatEdit;
        m_pdeY = new FloatEdit;
        m_pdeZ = new FloatEdit;
        QLabel *plabX = new QLabel("x=");
        QLabel *plabY = new QLabel("y=");
        QLabel *plabZ = new QLabel("z=");
        plabX->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        plabY->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        plabZ->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QLabel *pLenX = new QLabel(Units::lengthUnitLabel());
        QLabel *plenY = new QLabel(Units::lengthUnitLabel());
        QLabel *plenZ = new QLabel(Units::lengthUnitLabel());
        pLenX->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        plenY->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        plenZ->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        pPosLayout->addWidget(plabTitle, 1,1,1,3);

        pPosLayout->addWidget(plabX,     2,1);
        pPosLayout->addWidget(m_pdeX,    2,2);
        pPosLayout->addWidget(pLenX,     2,3);

        pPosLayout->addWidget(plabY,     3,1);
        pPosLayout->addWidget(m_pdeY,    3,2);
        pPosLayout->addWidget(plenY,     3,3);

        pPosLayout->addWidget(plabZ,     4,1);
        pPosLayout->addWidget(m_pdeZ,    4,2);
        pPosLayout->addWidget(plenZ,     4,3);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok |QDialogButtonBox::Cancel);

        pMainLayout->addLayout(pPosLayout);
        pMainLayout->addStretch();
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void TranslateDlg::accept()
{
    m_Translation.x = m_pdeX->value() / Units::mtoUnit();
    m_Translation.y = m_pdeY->value() / Units::mtoUnit();
    m_Translation.z = m_pdeZ->value() / Units::mtoUnit();
    QDialog::accept();
}



void TranslateDlg::showEvent(QShowEvent *pEvent)
{
    XflDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);
    m_pButtonBox->setFocus();
}


void TranslateDlg::hideEvent(QHideEvent*pEvent)
{
    XflDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();
}


