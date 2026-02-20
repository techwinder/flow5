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

#include <interfaces/widgets/customwts/floatedit.h>

#include "scaledlg.h"

ScaleDlg::ScaleDlg(QWidget *pParent) : XflDialog(pParent)
{
    setupLayout();
}


void ScaleDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGridLayout *pScaleLayout = new QGridLayout;
        {
            m_pdeXFactor = new FloatEdit(1.0);
            m_pdeYFactor = new FloatEdit(1.0);
            m_pdeZFactor = new FloatEdit(1.0);
            QLabel *plab0 = new QLabel(tr("Scale factor"));
            QLabel *plab1 = new QLabel(tr("X Scale:"));
            QLabel *plab2 = new QLabel(tr("Y Scale:"));
            QLabel *plab3 = new QLabel(tr("Z Scale:"));
            pScaleLayout->addWidget(plab0,        1,2, Qt::AlignCenter);
            pScaleLayout->addWidget(plab1,        2,1, Qt::AlignRight | Qt::AlignVCenter);
            pScaleLayout->addWidget(plab2,        3,1, Qt::AlignRight | Qt::AlignVCenter);
            pScaleLayout->addWidget(plab3,        4,1, Qt::AlignRight | Qt::AlignVCenter);
            pScaleLayout->addWidget(m_pdeXFactor, 2,2);
            pScaleLayout->addWidget(m_pdeYFactor, 3,2);
            pScaleLayout->addWidget(m_pdeZFactor, 4,2);
        }
        m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok |QDialogButtonBox::Cancel);

        pMainLayout->addLayout(pScaleLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


double ScaleDlg::XFactor() const {return m_pdeXFactor->value();}
double ScaleDlg::YFactor() const {return m_pdeYFactor->value();}
double ScaleDlg::ZFactor() const {return m_pdeZFactor->value();}




