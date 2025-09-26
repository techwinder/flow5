/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QVBoxLayout>
#include <QPushButton>

#include "extradragdlg.h"
#include <xfl/editors/analysis3ddef/extradragwt.h>

QByteArray ExtraDragDlg::s_Geometry;


ExtraDragDlg::ExtraDragDlg(QWidget *pParent) : QDialog(pParent)
{
    setupLayout();
}


void ExtraDragDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pExtraDragWt = new ExtraDragWt;

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
        }
        pMainLayout->addWidget(m_pExtraDragWt);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void ExtraDragDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)      accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}


void ExtraDragDlg::initDialog(WPolar const *pWPolar)
{
    m_pExtraDragWt->initWt(pWPolar);
}


void ExtraDragDlg::setExtraDragData(WPolar *pWPolar)
{
    m_pExtraDragWt->setExtraDragData(*pWPolar);
}


void ExtraDragDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void ExtraDragDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void ExtraDragDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("ExtraDragDlg");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();
    }
    settings.endGroup();
}


void ExtraDragDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("ExtraDragDlg");
    {
        settings.setValue("WindowGeometry", s_Geometry);
    }
    settings.endGroup();
}


