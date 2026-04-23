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



#define _MATH_DEFINES_DEFINED


#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>



#include "wingseldlg.h"

#include <api/enums_objects.h>

bool WingSelDlg::s_bMainWing  = true;
bool WingSelDlg::s_bElevator  = true;
bool WingSelDlg::s_bFin       = true;
bool WingSelDlg::s_bOtherWing = true;


WingSelDlg::WingSelDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowFlags(Qt::Tool);
    setupLayout();

    m_pchWing[0]->setChecked(s_bMainWing);
    m_pchWing[1]->setChecked(s_bElevator);
    m_pchWing[2]->setChecked(s_bFin);
    m_pchWing[3]->setChecked(s_bOtherWing);
}



void WingSelDlg::setupLayout()
{
    QVBoxLayout *pWingLayout = new QVBoxLayout;
    {
        m_pchWing.resize(4);
        m_pchWing[0] = new QCheckBox(tr("Main wing"));
        m_pchWing[1] = new QCheckBox(tr("Elevator"));
        m_pchWing[2] = new QCheckBox(tr("Fin"));
        m_pchWing[3] = new QCheckBox(tr("Other wings"));

        for(int iw=0; iw<4; iw++) pWingLayout->addWidget(m_pchWing[iw]);

    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        {
            connect(pButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);
        }
        pMainLayout->addLayout(pWingLayout);
        pMainLayout->addWidget(pButtonBox);
    }

    setLayout(pMainLayout);
}


void WingSelDlg::accept()
{
    s_bMainWing  = m_pchWing[0]->isChecked();
    s_bElevator  = m_pchWing[1]->isChecked();
    s_bFin       = m_pchWing[2]->isChecked();
    s_bOtherWing = m_pchWing[3]->isChecked();
    QDialog::accept();
}


void WingSelDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("WingSelDlg");
    {
        s_bMainWing  = settings.value("MainWingCurve",  true).toBool();
        s_bElevator  = settings.value("ElevatorCurve",  true).toBool();
        s_bFin       = settings.value("FinCurve",       true).toBool();
        s_bOtherWing = settings.value("OtherWingCurve", true).toBool();
    }
    settings.endGroup();
}


void WingSelDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("WingSelDlg");
    {
        settings.setValue("MainWingCurve",  s_bMainWing);
        settings.setValue("ElevatorCurve",  s_bElevator);
        settings.setValue("FinCurve",       s_bFin);
        settings.setValue("OtherWingCurve", s_bOtherWing);
    }
    settings.endGroup();
}



