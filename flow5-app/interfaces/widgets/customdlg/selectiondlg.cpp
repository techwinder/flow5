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
#include <QHBoxLayout>
#include <QPushButton>
#include <QStandardItem>


#include "selectiondlg.h"

QByteArray SelectionDlg::s_Geometry;

SelectionDlg::SelectionDlg(QWidget *pParent) : XflDialog(pParent)
{
    setWindowTitle("Selection");

    m_strNameList.clear();

    setupLayout();
}


void SelectionDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_plabQuestion = new QLabel();
        m_plwNameList = new QListWidget;
        m_plwNameList->setSelectionMode(QAbstractItemView::MultiSelection);

        setButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        {
            m_ppbSelectAll = new QPushButton("Select All");
            m_pButtonBox->addButton(m_ppbSelectAll, QDialogButtonBox::ActionRole);
        }

        pMainLayout->addWidget(m_plabQuestion);
        pMainLayout->addWidget(m_plwNameList);
        pMainLayout->addWidget(m_pButtonBox);
    }

//    connect(m_plwNameList, SIGNAL(itemClicked(QListWidgetItem*)),       SLOT(onSelChangeList(QListWidgetItem*)));
    connect(m_plwNameList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(onDoubleClickList(QListWidgetItem*)));

    setLayout(pMainLayout);
}


void SelectionDlg::initDialog(QString const &question, QStringList const &namelist, QStringList const &selectionlist, bool bSingleSel)
{
    m_plabQuestion->setText(question);

    m_strNameList = namelist;
    m_SelectedList = selectionlist;
    m_plwNameList->addItems(namelist);

    if(bSingleSel)
    {
        m_plwNameList->setSelectionMode(QAbstractItemView::SingleSelection);
        m_ppbSelectAll->setEnabled(false);
    }
    else
        m_plwNameList->setSelectionMode(QAbstractItemView::MultiSelection);

    QAbstractItemModel *pItemModel = m_plwNameList->model();
    QItemSelectionModel *pSelModel = m_plwNameList->selectionModel();
    if(pSelModel && pItemModel)
    {
        for(int row=0; row<pItemModel->rowCount(); row++)
        {
            QModelIndex index = pItemModel->index(row, 0);
            QString name = pItemModel->data(index).toString();
            if(m_SelectedList.contains(name))
            {
                pSelModel->select(index, QItemSelectionModel::Select);
            }
        }
    }
}


void SelectionDlg::onButton(QAbstractButton*pButton)
{
    if (m_ppbSelectAll == pButton) onSelectAll();
    else XflDialog::onButton(pButton);
}


QString SelectionDlg::selection() const
{
    if(m_SelectedList.isEmpty()) return QString();
    else return m_SelectedList.front();
}


void SelectionDlg::onSelectAll()
{
    m_plwNameList->selectAll();
}


void SelectionDlg::accept()
{
    QListWidgetItem *pItem =  nullptr;
    
    m_SelectedList.clear();
    for(int i=0; i<m_plwNameList->count();i++)
    {
        pItem = m_plwNameList->item(i);
        if(pItem->isSelected())
        {
            m_SelectedList.append(pItem->text());
        }
    }

    XflDialog::accept();
}


void SelectionDlg::onDoubleClickList(QListWidgetItem *)
{
    accept();
}


void SelectionDlg::showEvent(QShowEvent *pEvent)
{
    XflDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void SelectionDlg::hideEvent(QHideEvent *pEvent)
{
    XflDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void SelectionDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("SelectionDlg");
    {
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void SelectionDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("SelectionDlg");
    {
        settings.setValue("WindowGeom", s_Geometry);
    }
    settings.endGroup();
}



