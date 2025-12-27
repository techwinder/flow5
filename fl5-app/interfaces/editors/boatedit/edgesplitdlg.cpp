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


#include <QFormLayout>



#include "edgesplitdlg.h"
#include <interfaces/widgets/customwts/intedit.h>
#include <api/edgesplit.h>


EdgeSplitDlg::EdgeSplitDlg(QWidget *pParent, EdgeSplit &es) : XflDialog(pParent)
{
    setupLayout();
    setButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    m_pieNSegs->setValue(es.nSegs());
    switch (es.distrib())
    {
        default:
        case xfl::UNIFORM:   m_pcbDistType->setCurrentIndex(0);  break;
        case xfl::COSINE:    m_pcbDistType->setCurrentIndex(1);  break;
        case xfl::SINE:      m_pcbDistType->setCurrentIndex(2);  break;
        case xfl::INV_SINE:  m_pcbDistType->setCurrentIndex(3);  break;
        case xfl::TANH:      m_pcbDistType->setCurrentIndex(4);  break;
        case xfl::EXP:       m_pcbDistType->setCurrentIndex(5);  break;
        case xfl::INV_EXP:   m_pcbDistType->setCurrentIndex(6);  break;
    }
}


void EdgeSplitDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QFormLayout *pEdgeLayout = new QFormLayout;
        {
            m_pieNSegs = new IntEdit;
            m_pieNSegs->setToolTip("Set the number of triangles on this EDGE.\n"
                                   "Set the value to -1 to let the mesher define the number\n"
                                   "using the global max. edge length setting.");
            m_pcbDistType = new QComboBox;
            m_pcbDistType->addItems({"UNIFORM", "COSINE", "SINE", "INV_SINE", "TANH", "EXP", "INV_EXP"});
            pEdgeLayout->addRow("Number of segments:", m_pieNSegs);
            pEdgeLayout->addRow("Distribution type:", m_pcbDistType);
        }

        pMainLayout->addLayout(pEdgeLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


int EdgeSplitDlg::nSegs() const
{
    return m_pieNSegs->value();
}


xfl::enumDistribution EdgeSplitDlg::distrib() const
{
    switch (m_pcbDistType->currentIndex())
    {
        default:
        case 0:   return xfl::UNIFORM;   break;
        case 1:   return xfl::COSINE;    break;
        case 2:   return xfl::SINE;      break;
        case 3:   return xfl::INV_SINE;  break;
        case 4:   return xfl::TANH;      break;
        case 5:   return xfl::EXP;       break;
        case 6:   return xfl::INV_EXP;   break;
    }
}
