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


#include <QPushButton>
#include <QGridLayout>
#include <QLabel>


#include "sailscaledlg.h"

#include <interfaces/widgets/customwts/floatedit.h>
#include <api/wingxfl.h>
#include <api/sail.h>
#include <api/units.h>

SailScaleDlg::SailScaleDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Scale sail");
    m_bArea = m_bAR = m_bTwist = false;

    m_RefArea  = m_NewArea = 1.0;
    m_RefAR    = m_NewAR = 1.0;
    m_NewTwist = m_RefTwist = 1.0;

    setupLayout();
}


void SailScaleDlg::setupLayout()
{
    QGridLayout *pScaleLayout = new QGridLayout;
    {
        QLabel *plabRef  = new QLabel("Reference");
        QLabel *plabNew   = new QLabel("New");
        QLabel *plabRatio = new QLabel("Ratio");
        plabRef->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        plabNew->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        plabRatio->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

        m_pchScaleArea = new QCheckBox("Area");
        m_pchScaleAR   = new QCheckBox("Aspect ratio");
        m_pchTwist = new QCheckBox("Twist");

        m_pdeNewArea  = new FloatEdit;
        m_pdeNewAR    = new FloatEdit;
        m_pdeNewTwist = new FloatEdit;

        m_plabRefArea  = new QLabel("0.000");
        m_plabRefAR    = new QLabel("0.000");
        m_plabRefTwist = new QLabel("0.000");

        m_plabRefArea->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabRefAR->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabRefTwist->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        m_plabAreaRatio  = new QLabel("1.000");
        m_plabARRatio    = new QLabel("1.000");
        m_plabTwistRatio = new QLabel("1.000");

        m_plabAreaRatio->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabARRatio->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabTwistRatio->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        QLabel *plabDegree = new QLabel("<p>&deg;</p>");
        QLabel *pAreaUnit = new QLabel(Units::areaUnitQLabel());

        pScaleLayout->addWidget(plabRef,         1,2);
        pScaleLayout->addWidget(plabNew,         1,3);
        pScaleLayout->addWidget(plabRatio,       1,5);

        pScaleLayout->addWidget(m_pchScaleArea,  2,1);
        pScaleLayout->addWidget(m_plabRefArea,   2,2);
        pScaleLayout->addWidget(m_pdeNewArea,    2,3);
        pScaleLayout->addWidget(pAreaUnit,       2,4);
        pScaleLayout->addWidget(m_plabAreaRatio, 2,5);

        pScaleLayout->addWidget(m_pchScaleAR,    3,1);
        pScaleLayout->addWidget(m_plabRefAR,     3,2);
        pScaleLayout->addWidget(m_pdeNewAR,      3,3);
        pScaleLayout->addWidget(m_plabARRatio,   3,5);

        pScaleLayout->addWidget(m_pchTwist,      4,1);
        pScaleLayout->addWidget(m_plabRefTwist,  4,2);
        pScaleLayout->addWidget(m_pdeNewTwist,   4,3);
        pScaleLayout->addWidget(plabDegree,      4,4);
        pScaleLayout->addWidget(m_plabTwistRatio,4,5);

    }


    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pScaleLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);

    connect(m_pchTwist,     SIGNAL(clicked()),         SLOT(onClickedCheckBox()));
    connect(m_pchScaleArea, SIGNAL(clicked()),         SLOT(onClickedCheckBox()));
    connect(m_pchScaleAR,   SIGNAL(clicked()),         SLOT(onClickedCheckBox()));

    connect(m_pdeNewArea,   SIGNAL(editingFinished()), SLOT(onEditingFinished()));
    connect(m_pdeNewAR,     SIGNAL(editingFinished()), SLOT(onEditingFinished()));
    connect(m_pdeNewTwist,  SIGNAL(editingFinished()), SLOT(onEditingFinished()));
}


void SailScaleDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)      accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}


void SailScaleDlg::keyPressEvent(QKeyEvent *pEvent)
{
    // Prevent Return Key from closing App
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                //                updateResults();
                m_pButtonBox->setFocus();
                return;
            }
            else
            {
                accept();
                return;
            }
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


void SailScaleDlg::initDialog(Sail *pSail)
{
    m_RefArea  = m_NewArea  = pSail->area();
    m_RefAR    = m_NewAR    = pSail->aspectRatio();
    m_RefTwist = m_NewTwist = pSail->twist();

    m_pchScaleArea->setChecked(m_bArea);
    m_pchScaleAR->setChecked(m_bAR);
    m_pchTwist->setChecked(m_bTwist);

    m_pchTwist->setEnabled(!pSail->isOccSail());

    QString strong;

    strong = QString("%1").arg(m_RefTwist,8,'f',2);
    m_plabRefTwist->setText(strong);

    strong = QString ("%1").arg(m_RefArea *Units::m2toUnit(), 8,'f',3);
    m_plabRefArea->setText(strong);

    strong = QString ("%1").arg(m_RefAR , 8,'f',3);
    m_plabRefAR->setText(strong);

    m_pdeNewArea->setValue(m_NewArea*Units::m2toUnit());
    m_pdeNewAR->setValue(m_NewAR);
    m_pdeNewTwist->setValue(m_NewTwist);

    setResults();
    enableControls();
}


void SailScaleDlg::onClickedCheckBox()
{
    readData();
    enableControls();
}


void SailScaleDlg::onOK()
{
    readData();
    accept();
}


void SailScaleDlg::onEditingFinished()
{
    readData();
    setResults();
}


void SailScaleDlg::enableControls()
{
    m_pdeNewArea->setEnabled(m_bArea);
    m_pdeNewAR->setEnabled(m_bAR);
    m_pdeNewTwist->setEnabled(m_bTwist);
}


void SailScaleDlg::readData()
{
    m_bTwist = m_pchTwist->isChecked();
    m_bArea  = m_pchScaleArea->isChecked();
    m_bAR    = m_pchScaleAR->isChecked();

    m_NewTwist = m_pdeNewTwist->value();
    m_NewArea  = m_pdeNewArea->value() /Units::m2toUnit();
    m_NewAR    = m_pdeNewAR->value();
}


void SailScaleDlg::setResults()
{
    QString strong;

    if(m_RefTwist>0.0) strong = QString("%1").arg(m_NewTwist/m_RefTwist, 6,'f',3);
    else               strong =" 1.000";
    m_plabTwistRatio->setText(strong);

    if(m_RefArea>0.0)  strong = QString("%1").arg(m_NewArea/m_RefArea, 6,'f',3);
    else               strong =" 1.000";
    m_plabAreaRatio->setText(strong);

    if(m_RefAR>0.0)    strong = QString("%1").arg(m_NewAR/m_RefAR, 6,'f',3);
    else               strong =" 1.000";
    m_plabARRatio->setText(strong);

}



