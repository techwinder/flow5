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

#include <QVBoxLayout>
#include <QPushButton>

#include "extradragdlg.h"
#include <interfaces/editors/analysis3ddef/extradragwt.h>

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


void ExtraDragDlg::initDialog(PlanePolar const *pWPolar)
{
    m_pExtraDragWt->initWt(pWPolar);
}


void ExtraDragDlg::setExtraDragData(PlanePolar *pWPolar)
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


