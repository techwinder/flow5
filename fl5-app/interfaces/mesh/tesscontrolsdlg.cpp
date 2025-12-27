/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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
#include <QButtonGroup>

#include "tesscontrolsdlg.h"


TessControlsDlg::TessControlsDlg(QWidget *pParent) : XflDialog(pParent)
{
    m_bOcc = true;
    m_bChanged = false;
    setupLayout();


    connect(m_prbOcc,  SIGNAL(clicked(bool)), SLOT(onTessellatorChanged()));
    connect(m_prbGmsh, SIGNAL(clicked(bool)), SLOT(onTessellatorChanged()));
}


void TessControlsDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {

        QString warning("<p>Proceed with caution!<br>"
                        "Tessellation at high precision can be a lengthy task "
                        "and produce an excessive number of triangles.</p>");
        QLabel *pCautionLabel = new QLabel(warning);
        pCautionLabel->setWordWrap(true);

        QHBoxLayout *pOptionLayout = new QHBoxLayout;
        {
            QButtonGroup *pGroup = new QButtonGroup;
            {
                m_prbOcc  = new QRadioButton("Occ");
                m_prbGmsh = new QRadioButton("Gmsh");
                pGroup->addButton(m_prbOcc);
                pGroup->addButton(m_prbGmsh);
            }
            pOptionLayout->addStretch();
            pOptionLayout->addWidget(m_prbOcc);
            pOptionLayout->addWidget(m_prbGmsh);
            pOptionLayout->addStretch();
        }

        m_pStackedWt = new QStackedWidget;
        {
            m_pOccWt = new OccTessCtrlsWt;
            m_pGmshWt = new GmshCtrlsWt;
            m_pStackedWt->addWidget(m_pOccWt);
            m_pStackedWt->addWidget(m_pGmshWt);
        }

        pMainLayout->addWidget(pCautionLabel);
        pMainLayout->addStretch();
        pMainLayout->addLayout(pOptionLayout);
        pMainLayout->addWidget(m_pStackedWt);
        pMainLayout->addWidget(m_pButtonBox);

        setButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    }

    setLayout(pMainLayout);
}


void TessControlsDlg::initDialog(OccMeshParams const &occparams, GmshParams const &gmshparams, bool bOcc, bool bShowBtnBox)
{
    m_pButtonBox->setVisible(bShowBtnBox);

    m_bOcc = bOcc;

    m_prbOcc->setChecked(m_bOcc);
    m_prbGmsh->setChecked(!m_bOcc);

    if(m_bOcc) m_pStackedWt->setCurrentWidget(m_pOccWt);
    else       m_pStackedWt->setCurrentWidget(m_pGmshWt);

    m_pOccWt->initWt(occparams);
    m_pGmshWt->initWt(gmshparams);
}


void TessControlsDlg::onTessellatorChanged()
{
    m_bOcc = m_prbOcc->isChecked();
    if(m_bOcc) m_pStackedWt->setCurrentWidget(m_pOccWt);
    else       m_pStackedWt->setCurrentWidget(m_pGmshWt);

    m_bChanged = true;
}

