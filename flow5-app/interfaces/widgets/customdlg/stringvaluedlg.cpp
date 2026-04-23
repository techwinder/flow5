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

#include "stringvaluedlg.h"

QByteArray StringValueDlg::s_WindowGeometry;

StringValueDlg::StringValueDlg(QWidget *pParent, const QStringList &list) : QDialog(pParent)
{
    setupLayout();
    m_plwStrings->insertItems(0, list);
}


void StringValueDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_plwStrings = new QListWidget;
        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        {
            connect(m_pButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(m_pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }
        pMainLayout->addWidget(m_plwStrings);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);

    connect(m_plwStrings, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(onItemDoubleClicked(QListWidgetItem*)));
}


void StringValueDlg::onItemDoubleClicked(QListWidgetItem *pItem)
{
    if(pItem) accept();
}


QString StringValueDlg::selectedString() const
{
    QListWidgetItem const *pItem = m_plwStrings->currentItem();
    if(pItem) return pItem->text();
    return QString();
}


void StringValueDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);
}


void StringValueDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();
}
