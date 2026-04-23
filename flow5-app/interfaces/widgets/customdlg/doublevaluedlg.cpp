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
#include <QLabel>
#include <QDialogButtonBox>

#include "doublevaluedlg.h"


QByteArray DoubleValueDlg::s_WindowGeometry;

DoubleValueDlg::DoubleValueDlg(QWidget *pParent, QVector<double> values, QStringList const &leftlabels, QStringList const &rightlabels) : XflDialog(pParent)
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGridLayout *pIntLayout = new QGridLayout;
        {
            m_pDoubleEdit.clear();
            for(int i=0; i<values.size(); i++)
            {
                QLabel *pLeftLabel = new QLabel;
                if(i<leftlabels.size()) pLeftLabel->setText(leftlabels.at(i));
                m_pDoubleEdit.push_back(new FloatEdit);
                QLabel *pRightLabel = new QLabel;
                if(i<rightlabels.size()) pRightLabel->setText(rightlabels.at(i));

                pIntLayout->addWidget(pLeftLabel,           i, 1);
                pIntLayout->addWidget(m_pDoubleEdit.back(), i, 2);
                pIntLayout->addWidget(pRightLabel,          i, 3);
            }
        }

        m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        pMainLayout->addLayout(pIntLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
    for(int i=0; i<values.size(); i++)
        m_pDoubleEdit[i]->setValue(values.at(i));
}


void DoubleValueDlg::showEvent(QShowEvent *pEvent)
{
    XflDialog::showEvent(pEvent);

    restoreGeometry(s_WindowGeometry);

    if(m_pDoubleEdit.size())
    {
        m_pDoubleEdit.front()->selectAll();
        m_pDoubleEdit.front()->setFocus();
    }
}


void DoubleValueDlg::hideEvent(QHideEvent *pEvent)
{
    XflDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();
}



void DoubleValueDlg::keyPressEvent(QKeyEvent *pEvent)
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

