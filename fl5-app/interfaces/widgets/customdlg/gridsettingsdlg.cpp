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

#include <QGridLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>


#include "gridsettingsdlg.h"
#include <interfaces/widgets/line/linemenu.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/customwts/floatedit.h>


QByteArray GridSettingsDlg::s_WindowGeometry;


GridSettingsDlg::GridSettingsDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle("Grid options");
    m_bWithUnit = true;
    setupLayout();
}


void GridSettingsDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();
}


void GridSettingsDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);
}


void GridSettingsDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            done(0);
            break;
        }
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
        default:
            pEvent->ignore();
    }
}


void GridSettingsDlg::initDialog(Grid const &grid, bool bWithUnit)
{
    m_Grid = grid;
    m_bWithUnit = bWithUnit;

    m_pGridControl->initControls(&m_Grid);
}


void GridSettingsDlg::setupLayout()
{
    m_pGridControl = new GridControl;

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pGridControl);
        pMainLayout->addStretch();
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void GridSettingsDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)
    {
        m_Grid.showYAxis(m_pGridControl->m_pchYAxisShow->isChecked());
        accept();
    }
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)     reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)    reject();
}







