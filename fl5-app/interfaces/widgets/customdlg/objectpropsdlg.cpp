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

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include <QShowEvent>
#include <QHideEvent>

#include "objectpropsdlg.h"


#include <interfaces/widgets/customwts/plaintextoutput.h>

QByteArray ObjectPropsDlg::s_Geometry;


ObjectPropsDlg::ObjectPropsDlg(QWidget *pParent) : QDialog(pParent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setupLayout();
}


void ObjectPropsDlg::setupLayout()
{
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    {
        connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }
    QVBoxLayout * pMainLayout = new QVBoxLayout(this);
    {
        m_ppto = new PlainTextOutput;
        pMainLayout->addWidget(m_ppto);
        pMainLayout->addSpacing(20);
        pMainLayout->addWidget(pButtonBox);
    }

    setLayout(pMainLayout);
}

void ObjectPropsDlg::initDialog(std::string const &title, std::string const &props)
{
    initDialog(QString::fromStdString(title), QString::fromStdString(props));
}


void ObjectPropsDlg::initDialog(QString const &title, QString const &props)
{
    setWindowTitle(title);
    m_ppto->insertPlainText(props);
    m_ppto->moveCursor(QTextCursor::Start);
}


void ObjectPropsDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void ObjectPropsDlg::hideEvent(QHideEvent*pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}







