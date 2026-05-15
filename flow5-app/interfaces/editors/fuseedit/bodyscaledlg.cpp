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


#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "bodyscaledlg.h"


#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>

BodyScaleDlg::BodyScaleDlg(QWidget *pParent ): QDialog(pParent)
{
    setWindowTitle(tr("Object scale"));
    m_XFactor = 1.0;
    m_YFactor = 1.0;
    m_ZFactor = 1.0;
    m_bFrameOnly = false;
    m_FrameID = 0;
    setupLayout();
}


void BodyScaleDlg::initDialog(bool bFrameOnly)
{
    m_pfeXScaleFactor->setValue(m_XFactor);
    m_pfeYScaleFactor->setValue(m_YFactor);
    m_pfeZScaleFactor->setValue(m_ZFactor);

    m_pfeXScaleFactor->setFocus();

    m_pieFrameID->setEnabled(false);
    m_pieFrameID->setValue(m_FrameID+1);

    m_bFrameOnly = bFrameOnly;
    if(!m_bFrameOnly)
    {
        m_prbBody->setChecked(true);
        m_prbFrame->setChecked(false);
    }
    else
    {
        m_prbBody->setChecked(false);
        m_prbFrame->setChecked(true);
        m_pfeXScaleFactor->setEnabled(false);
    }
    m_prbBody->setEnabled(false);
    m_prbFrame->setEnabled(false);

    enableControls();
}


void BodyScaleDlg::setupLayout()
{
    QGridLayout *pTopLayout = new QGridLayout;
    {
        m_prbBody  = new QRadioButton(tr("Whole object"));
        m_prbFrame = new QRadioButton(tr("Frame only"));
        m_pieFrameID = new IntEdit(10);

        pTopLayout->addWidget(m_prbBody,    1, 1);
        pTopLayout->addWidget(m_prbFrame,   2, 1);
        pTopLayout->addWidget(m_pieFrameID, 2, 2);
    }

    QGridLayout *pScaleLayout = new QGridLayout;
    {
        m_pfeXScaleFactor = new FloatEdit(1.0);
        m_pfeYScaleFactor = new FloatEdit(2.000);
        m_pfeZScaleFactor = new FloatEdit(3.);
        m_pfeXScaleFactor->setDigits(3);
        m_pfeYScaleFactor->setDigits(3);
        m_pfeZScaleFactor->setDigits(3);
        QLabel *plab0 = new QLabel("Scale factor");
        QLabel *plab1 = new QLabel("X scale");
        QLabel *plab2 = new QLabel("Y scale");
        QLabel *plab3 = new QLabel("Z scale");
        pScaleLayout->addWidget(plab0,             1, 2);
        pScaleLayout->addWidget(plab1,             2, 1, Qt::AlignRight);
        pScaleLayout->addWidget(plab2,             3, 1, Qt::AlignRight);
        pScaleLayout->addWidget(plab3,             4, 1, Qt::AlignRight);
        pScaleLayout->addWidget(m_pfeXScaleFactor, 2, 2);
        pScaleLayout->addWidget(m_pfeYScaleFactor, 3, 2);
        pScaleLayout->addWidget(m_pfeZScaleFactor, 4, 2);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    {
        m_pButtonBox->setCursor(Qt::ArrowCursor);
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pTopLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pScaleLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);


    connect(m_prbBody,  SIGNAL(clicked()), SLOT(onRadioBtn()));
    connect(m_prbFrame, SIGNAL(clicked()), SLOT(onRadioBtn()));
}


void BodyScaleDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)      onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}


void BodyScaleDlg::enableControls()
{

}



void BodyScaleDlg::keyPressEvent(QKeyEvent *event)
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



void BodyScaleDlg::onRadioBtn()
{
    if(m_prbBody->isChecked())
    {
        m_pieFrameID->setEnabled(false);
        m_pfeXScaleFactor->setEnabled(true);
        m_bFrameOnly = false;
    }
    else
    {
        m_pieFrameID->setEnabled(true);
        m_pfeXScaleFactor->setEnabled(false);
        m_bFrameOnly = true;
    }
}


void BodyScaleDlg::onOK()
{
    m_FrameID = m_pieFrameID->value()-1;

    m_XFactor = m_pfeXScaleFactor->value();
    m_YFactor = m_pfeYScaleFactor->value();
    m_ZFactor = m_pfeZScaleFactor->value();

    QDialog::accept();
}


void BodyScaleDlg::onEditingFinished()
{
    m_FrameID = m_pieFrameID->value()-1;
    //    pBodyDlg->m_pFrame = pBodyDlg->m_pFuseXfl->setActiveFrame(m_FrameID);
    //    pBodyDlg->updateView();
}




void BodyScaleDlg::enableFrameID(bool bEnable)
{
    m_pieFrameID->setEnabled(bEnable);

}



