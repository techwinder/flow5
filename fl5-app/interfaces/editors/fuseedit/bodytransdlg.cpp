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


#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include "bodytransdlg.h"

#include <api/units.h>

#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>


BodyTransDlg::BodyTransDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Translation"));
    m_XTrans = m_YTrans = m_ZTrans = 0.0;
    m_bFrameOnly = false;
    m_FrameID = 1;

    setupLayout();
}


void BodyTransDlg::initDialog()
{
    m_pdeXTransFactor->setValue(m_XTrans);
    m_pdeYTransFactor->setValue(m_YTrans);
    m_pdeZTransFactor->setValue(m_ZTrans);

    m_pdeYTransFactor->setEnabled(false);

    m_pchFrameOnly->setChecked(m_bFrameOnly);
    m_pieFrameID->setValue(m_FrameID+1);
    m_pieFrameID->setEnabled(m_bFrameOnly);
}


void BodyTransDlg::enableDirections(bool bx, bool by, bool bz)
{
    m_pdeXTransFactor->setEnabled(bx);
    m_pdeYTransFactor->setEnabled(by);
    m_pdeZTransFactor->setEnabled(bz);
}


void BodyTransDlg::enableFrameID(bool bEnable)
{
    m_pieFrameID->setValue(0);
    m_pieFrameID->setEnabled(bEnable);
    m_pchFrameOnly->setEnabled(bEnable);
}


void BodyTransDlg::checkFrameId(bool bForce)
{
    m_pchFrameOnly->setChecked(bForce);
}


void BodyTransDlg::setFrameId(int id)
{
    m_FrameID=id;
    m_pieFrameID->setValue(id);
}


void BodyTransDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
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
                onOK();
                return;
            }
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
        default:
            event->ignore();
    }
}


void BodyTransDlg::onOK()
{
    m_bFrameOnly = m_pchFrameOnly->isChecked();
    m_FrameID    = m_pieFrameID->value()-1;
    m_XTrans     = m_pdeXTransFactor->value() / Units::mtoUnit();
    m_YTrans     = m_pdeYTransFactor->value() / Units::mtoUnit();
    m_ZTrans     = m_pdeZTransFactor->value() / Units::mtoUnit();
    accept();
}



void BodyTransDlg::onFrameOnly()
{
    m_bFrameOnly = m_pchFrameOnly->isChecked();
    m_pieFrameID->setEnabled(m_bFrameOnly);
}


void BodyTransDlg::setupLayout()
{
    QHBoxLayout *pFrameIDLayout = new QHBoxLayout;
    {
        m_pchFrameOnly = new QCheckBox(tr("Frame only"));
        m_pieFrameID = new IntEdit(0);
        pFrameIDLayout->addStretch();
        pFrameIDLayout->addWidget(m_pchFrameOnly);
        pFrameIDLayout->addWidget(m_pieFrameID);
    }

    QGridLayout *pTransLayout = new QGridLayout;
    {
        QLabel * XTrans = new QLabel(tr("dX="));
        QLabel * YTrans = new QLabel(tr("dY="));
        QLabel * ZTrans = new QLabel(tr("dZ="));
        m_pdeXTransFactor = new FloatEdit(0.0,3);
        m_pdeYTransFactor = new FloatEdit(0.0,3);
        m_pdeZTransFactor = new FloatEdit(0.0,3);
        QString length;
        length = Units::lengthUnitQLabel();
        QLabel *pchLength1 = new QLabel(length);
        QLabel *pchLength2 = new QLabel(length);
        QLabel *pchLength3 = new QLabel(length);

        pTransLayout->addWidget(XTrans,1,1, Qt::AlignRight | Qt::AlignVCenter);
        pTransLayout->addWidget(YTrans,2,1, Qt::AlignRight | Qt::AlignVCenter);
        pTransLayout->addWidget(ZTrans,3,1, Qt::AlignRight | Qt::AlignVCenter);
        pTransLayout->addWidget(m_pdeXTransFactor,1,2);
        pTransLayout->addWidget(m_pdeYTransFactor,2,2);
        pTransLayout->addWidget(m_pdeZTransFactor,3,2);
        pTransLayout->addWidget(pchLength1,1,3);
        pTransLayout->addWidget(pchLength2,2,3);
        pTransLayout->addWidget(pchLength3,3,3);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }


    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pFrameIDLayout);
        pMainLayout->addLayout(pTransLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);

    connect(m_pchFrameOnly, SIGNAL(clicked()), SLOT(onFrameOnly()));

}



void BodyTransDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)      onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}



