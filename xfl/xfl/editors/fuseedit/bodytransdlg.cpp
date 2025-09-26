/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include "bodytransdlg.h"
#include <xflcore/units.h>

#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>


BodyTransDlg::BodyTransDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle("Translation");
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
        m_pchFrameOnly = new QCheckBox("Frame only");
        m_pieFrameID = new IntEdit(0);
        pFrameIDLayout->addStretch();
        pFrameIDLayout->addWidget(m_pchFrameOnly);
        pFrameIDLayout->addWidget(m_pieFrameID);
    }

    QGridLayout *pTransLayout = new QGridLayout;
    {
        QLabel * XTrans = new QLabel("dX=");
        QLabel * YTrans = new QLabel("dY=");
        QLabel * ZTrans = new QLabel("dZ=");
        m_pdeXTransFactor = new FloatEdit(0.0,3);
        m_pdeYTransFactor = new FloatEdit(0.0,3);
        m_pdeZTransFactor = new FloatEdit(0.0,3);
        QString length;
        Units::getLengthUnitLabel(length);
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



