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

#include <QFormLayout>

#include "separatorsdlg.h"

bool SeparatorsDlg::s_bWhiteSpace = true;
bool SeparatorsDlg::s_bTab        = true;
bool SeparatorsDlg::s_bComma      = false;
bool SeparatorsDlg::s_bSemiColon  = false;

SeparatorsDlg::SeparatorsDlg(QWidget *pParent) : QDialog(pParent)
{
    setupLayout();
}


void SeparatorsDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QLabel *pLabTitle = new QLabel("Select the text separators to use");
        m_pchWhiteSpace = new QCheckBox("WhiteSpace");
        m_pchWhiteSpace->setChecked(s_bWhiteSpace);
        m_pchTab        = new QCheckBox("tab");
        m_pchTab->setChecked(s_bTab);
        m_pchComma      = new QCheckBox("comma ',' ");
        m_pchComma->setChecked(s_bComma);
        m_pchSemiColon  = new QCheckBox("semi-colon ';'");
        m_pchSemiColon->setChecked(s_bSemiColon);
        QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(pButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

        pMainLayout->addWidget(pLabTitle);
        pMainLayout->addWidget(m_pchWhiteSpace);
        pMainLayout->addWidget(m_pchTab);
        pMainLayout->addWidget(m_pchComma);
        pMainLayout->addWidget(m_pchSemiColon);
        pMainLayout->addWidget(pButtonBox);
    }
    setLayout(pMainLayout);
}


void SeparatorsDlg::hideEvent(QHideEvent *)
{
    s_bWhiteSpace = m_pchWhiteSpace->isChecked();
    s_bTab        = m_pchTab->isChecked();
    s_bComma      = m_pchComma->isChecked();
    s_bSemiColon  = m_pchSemiColon->isChecked();
}


void SeparatorsDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("Separators");
    {
        s_bWhiteSpace = settings.value("WhiteSpace", s_bWhiteSpace).toBool();
        s_bTab        = settings.value("TabSep",     s_bTab).toBool();
        s_bComma      = settings.value("CommaSep",   s_bComma).toBool();
        s_bSemiColon  = settings.value("SemiColSep", s_bSemiColon).toBool();
    }
    settings.endGroup();

}


void SeparatorsDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("Separators");
    {
        settings.setValue("WhiteSpace",             s_bWhiteSpace);
        settings.setValue("TabSep",                 s_bTab);
        settings.setValue("CommaSep",               s_bComma);
        settings.setValue("SemiColSep",             s_bSemiColon);
    }
    settings.endGroup();
}
