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

#include <QKeyEvent>
#include <QPushButton>
#include <QMessageBox>

#include "xfldialog.h"

#include <core/xflcore.h>

XflDialog::XflDialog(QWidget *pParent) : QDialog(pParent)
{
    setWindowFlag(Qt::WindowMinMaxButtonsHint);
    m_bChanged = false;

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::NoButton, this);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }
}


void XflDialog::setButtons(QDialogButtonBox::StandardButtons buttons)
{
    m_pButtonBox->setStandardButtons(buttons);
}


void XflDialog::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
                return;
            }
            else
            {
                accept();
                return;
            }
            break;
        }
        default:
            pEvent->ignore();
    }
    QDialog::keyPressEvent(pEvent);
}


void XflDialog::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok)              == pButton)
        accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Save)            == pButton) accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel)          == pButton) reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard)         == pButton) reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Apply)           == pButton) onApply();
    else if (m_pButtonBox->button(QDialogButtonBox::Reset)           == pButton) onReset();
    else if (m_pButtonBox->button(QDialogButtonBox::RestoreDefaults) == pButton) onDefaults();
    else if (m_pButtonBox->button(QDialogButtonBox::Close)           == pButton) reject();
}


void XflDialog::reject()
{
    if(m_bChanged && xfl::bConfirmDiscard())
    {
        QString strong = "Discard the changes?";
        int Ans = QMessageBox::question(this, "Question", strong,
                                        QMessageBox::Yes | QMessageBox::Cancel);
        if (QMessageBox::Yes == Ans)
        {
            done(QDialog::Rejected);
            return;
        }
        else return;
    }

    QDialog::reject();
}

