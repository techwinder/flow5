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
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>


#include "intvaluesdlg.h"


IntValuesDlg::IntValuesDlg(QWidget *pParent, QVector<int> const &values, QStringList const &labels) : QDialog(pParent)
{
    setupLayout(values.size());

    for(int i=0; i<values.size(); i++)
        m_pIntEdit[i]->setValue(values.at(i));

    QString strange;
    for(int i=0; i<values.size(); i++)
    {
        if(i<labels.size()) strange = labels.at(i);
        else strange = QString::asprintf("Label_%d", i+1);
        m_pLabel[i]->setText(strange);
    }
}


void IntValuesDlg::setupLayout(int nValues)
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QFormLayout *pIntLayout = new QFormLayout;
        {
            m_pLabel.clear();
            m_pIntEdit.clear();
            for(int i=0; i<nValues; i++)
            {
                m_pLabel.push_back(new QLabel);
                m_pIntEdit.push_back(new IntEdit);
                pIntLayout->addRow(m_pLabel.back(), m_pIntEdit.back());
            }
        }

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        {
            connect(m_pButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(m_pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }

        pMainLayout->addLayout(pIntLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void IntValuesDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    if(m_pIntEdit.size()>0)
    {
        m_pIntEdit.front()->selectAll();
        m_pIntEdit.front()->setFocus();
    }
}


void IntValuesDlg::keyPressEvent(QKeyEvent *pEvent)
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


void IntValuesDlg::setLabel(int iLabel, QString label)
{
    if(iLabel>=0 && iLabel<m_pLabel.size()) m_pLabel[iLabel]->setText(label);
}


