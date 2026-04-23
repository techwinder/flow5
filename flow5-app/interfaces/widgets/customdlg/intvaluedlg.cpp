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

#include "intvaluedlg.h"
#include <interfaces/widgets/customwts/intedit.h>

QByteArray IntValueDlg::s_WindowGeometry;


IntValueDlg::IntValueDlg(QWidget *pParent) : XflDialog (pParent)
{
    setupLayout();
}


void IntValueDlg::setValue(int intvalue)
{
    m_pieIntEdit->setValue(intvalue);
}


int IntValueDlg::value() const
{
    return m_pieIntEdit->value();
}


void IntValueDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QHBoxLayout *pValueLayout = new QHBoxLayout;
        {
            m_pLeftLabel = new QLabel("Enter value:");
            m_pieIntEdit = new IntEdit;
            m_pRightLabel = new QLabel;
            pValueLayout->addWidget(m_pLeftLabel);
            pValueLayout->addWidget(m_pieIntEdit);
            pValueLayout->addWidget(m_pRightLabel);
        }

        m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        pMainLayout->addLayout(pValueLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void IntValueDlg::showEvent(QShowEvent *pEvent)
{
    m_pieIntEdit->selectAll();
    m_pieIntEdit->setFocus();
    restoreGeometry(s_WindowGeometry);
    QDialog::showEvent(pEvent);
}


void IntValueDlg::hideEvent(QHideEvent *)
{
    s_WindowGeometry = saveGeometry();
}


void IntValueDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
            }
            else
            {
                QDialog::accept();
            }
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
        default:
            pEvent->ignore();
    }
}


