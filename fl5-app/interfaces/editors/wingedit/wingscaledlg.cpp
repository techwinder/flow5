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

#include "wingscaledlg.h"

#include <api/wingxfl.h>
#include <api/units.h>
#include <api/utils.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <core/xflcore.h>


WingScaleDlg::WingScaleDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Scale wing");
    m_bSweep = m_bSpan = m_bChord = m_bTwist = m_bArea = m_bAR = m_bTR = false;

    m_NewSweep = m_NewChord = m_NewTwist = m_NewSpan = 1.0;
    m_RefSweep = m_RefChord = m_RefTwist = m_RefSpan = 1.0;

    m_RefArea = m_RefAR = m_RefTR = 1.0;
    m_NewArea = m_NewAR = m_NewTR = 1.0;

    setupLayout();
}


void WingScaleDlg::setupLayout()
{
    QGridLayout *pScaleLayout = new QGridLayout;
    {
        m_pchSpan  = new QCheckBox("Span");
        m_pchChord = new QCheckBox("Chord");
        m_pchSweep = new QCheckBox("Sweep");
        m_pchTwist = new QCheckBox("Twist");
        m_pchScaleArea = new QCheckBox("Area");
        m_pchScaleAR   = new QCheckBox("Aspect ratio");
        m_pchScaleTR   = new QCheckBox("Taper ratio");

        m_pdeNewSpan  = new FloatEdit(0,3);
        m_pdeNewChord = new FloatEdit(0,3);
        m_pdeNewTwist = new FloatEdit(0,3);
        m_pdeNewSweep = new FloatEdit(0,3);
        m_pdeNewArea  = new FloatEdit(0,3);
        m_pdeNewAR    = new FloatEdit(0,3);
        m_pdeNewTR    = new FloatEdit(0,3);

        m_plabRefSpan  = new QLabel("0.000");
        m_plabRefChord = new QLabel("0.000");
        m_plabRefSweep = new QLabel("0.000");
        m_plabRefTwist = new QLabel("0.000");
        m_plabRefArea  = new QLabel("0.000");
        m_plabRefAR    = new QLabel("0.000");
        m_plabRefTR    = new QLabel("0.000");

        m_plabRefSpan->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabRefChord->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabRefSweep->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabRefTwist->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabRefArea->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabRefAR->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabRefTR->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        m_plabSpanRatio  = new QLabel("1.000");
        m_plabChordRatio = new QLabel("1.000");
        m_plabSweepRatio = new QLabel("1.000");
        m_plabTwistRatio = new QLabel("1.000");
        m_plabAreaRatio  = new QLabel("1.000");
        m_plabARRatio    = new QLabel("1.000");
        m_plabTRRatio    = new QLabel("1.000");

        m_plabSpanRatio->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabChordRatio->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabSweepRatio->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabTwistRatio->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabAreaRatio->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabARRatio->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_plabTRRatio->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        QLabel *plab11 = new QLabel("Reference");
        QLabel *plab12 = new QLabel("New");
        QLabel *plab13 = new QLabel("Ratio");
        plab11->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        plab12->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        plab13->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        pScaleLayout->addWidget(plab11,          1,2);
        pScaleLayout->addWidget(plab12,          1,3);
        pScaleLayout->addWidget(plab13,          1,5);

        pScaleLayout->addWidget(m_pchSpan,       2,1);
        pScaleLayout->addWidget(m_plabRefSpan,   2,2);
        pScaleLayout->addWidget(m_pdeNewSpan,    2,3);
        pScaleLayout->addWidget(m_plabSpanRatio, 2,5);

        pScaleLayout->addWidget(m_pchChord,      3,1);
        pScaleLayout->addWidget(m_plabRefChord,  3,2);
        pScaleLayout->addWidget(m_pdeNewChord,   3,3);
        pScaleLayout->addWidget(m_plabChordRatio,3,5);

        pScaleLayout->addWidget(m_pchSweep,      4,1);
        pScaleLayout->addWidget(m_plabRefSweep,  4,2);
        pScaleLayout->addWidget(m_pdeNewSweep,   4,3);
        pScaleLayout->addWidget(m_plabSweepRatio,4,5);

        pScaleLayout->addWidget(m_pchTwist,      5,1);
        pScaleLayout->addWidget(m_plabRefTwist,  5,2);
        pScaleLayout->addWidget(m_pdeNewTwist,   5,3);
        pScaleLayout->addWidget(m_plabTwistRatio,5,5);

        pScaleLayout->addWidget(m_pchScaleArea,  6,1);
        pScaleLayout->addWidget(m_plabRefArea,   6,2);
        pScaleLayout->addWidget(m_pdeNewArea,    6,3);
        pScaleLayout->addWidget(m_plabAreaRatio, 6,5);

        pScaleLayout->addWidget(m_pchScaleAR,    7,1);
        pScaleLayout->addWidget(m_plabRefAR,     7,2);
        pScaleLayout->addWidget(m_pdeNewAR,      7,3);
        pScaleLayout->addWidget(m_plabARRatio,   7,5);

        pScaleLayout->addWidget(m_pchScaleTR,    8,1);
        pScaleLayout->addWidget(m_plabRefTR,     8,2);
        pScaleLayout->addWidget(m_pdeNewTR,      8,3);
        pScaleLayout->addWidget(m_plabTRRatio,   8,5);

        QString unitLabel = Units::lengthUnitQLabel();
        QLabel *pLengthUnit1 = new QLabel(unitLabel);
        QLabel *pLengthUnit2 = new QLabel(unitLabel);

        QLabel *pAngleUnit1 = new QLabel("<p>&deg;</p>");
        QLabel *pAngleUnit2 = new QLabel("<p>&deg;</p>");

        unitLabel = Units::areaUnitQLabel();
        QLabel *pAreaUnit = new QLabel(unitLabel);
        pScaleLayout->addWidget(pLengthUnit1, 2, 4);
        pScaleLayout->addWidget(pLengthUnit2, 3, 4);
        pScaleLayout->addWidget(pAngleUnit1,  4, 4);
        pScaleLayout->addWidget(pAngleUnit2,  5, 4);
        pScaleLayout->addWidget(pAreaUnit,    6, 4);
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

    connect(m_pchSpan,      SIGNAL(clicked()), this, SLOT(onClickedCheckBox()));
    connect(m_pchChord,     SIGNAL(clicked()), this, SLOT(onClickedCheckBox()));
    connect(m_pchSweep,     SIGNAL(clicked()), this, SLOT(onClickedCheckBox()));
    connect(m_pchTwist,     SIGNAL(clicked()), this, SLOT(onClickedCheckBox()));
    connect(m_pchScaleArea, SIGNAL(clicked()), this, SLOT(onClickedCheckBox()));
    connect(m_pchScaleAR,   SIGNAL(clicked()), this, SLOT(onClickedCheckBox()));
    connect(m_pchScaleTR,   SIGNAL(clicked()), this, SLOT(onClickedCheckBox()));

    connect(m_pdeNewSpan,  SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeNewChord, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeNewSweep, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeNewTwist, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeNewArea, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeNewAR, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeNewTR, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
}



void WingScaleDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)      accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}




void WingScaleDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
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
            event->ignore();
    }
}
void WingScaleDlg::initDialog(WingXfl *pWing)
{
    initDialog(pWing->planformSpan(),
               pWing->chord(0),
               pWing->averageSweep(),
               pWing->twist(pWing->nSections()-1),
               pWing->planformArea(),
               pWing->aspectRatio(),
               pWing->taperRatio());
}


void WingScaleDlg::initDialog(double const &RefSpan, double const &RefChord, double const &RefSweep, double const &RefTwist,
                              double const &RefArea, double const &RefAR, double const &RefTR)
{
    m_RefSpan  = RefSpan;
    m_RefChord = RefChord;
    m_RefSweep = RefSweep;
    m_RefTwist = RefTwist;
    m_RefArea  = RefArea;
    m_RefAR    = RefAR;
    m_RefTR    = RefTR;

    m_NewSpan  = RefSpan;
    m_NewChord = RefChord;
    m_NewSweep = RefSweep;
    m_NewTwist = RefTwist;
    m_NewArea  = RefArea;
    m_NewAR    = RefAR;
    m_NewTR    = RefTR;

    m_pchSpan->setChecked(m_bSpan);//(false)
    m_pchChord->setChecked(m_bChord);
    m_pchTwist->setChecked(m_bTwist);
    m_pchSweep->setChecked(m_bSweep);
    m_pchScaleArea->setChecked(m_bArea);
    m_pchScaleAR->setChecked(m_bAR);
    m_pchScaleTR->setChecked(m_bTR);

    QString strong;

    strong = QString("%1").arg(m_RefSpan * Units::mtoUnit(),8,'f',3);
    m_plabRefSpan->setText(strong);

    strong = QString("%1").arg(m_RefChord * Units::mtoUnit(),8,'f',3);
    m_plabRefChord->setText(strong);

    strong = QString("%1").arg(m_RefSweep,8,'f',2);
    strong += DEGch;
    m_plabRefSweep->setText(strong);

    strong = QString("%1").arg(m_RefTwist,8,'f',2);
    m_plabRefTwist->setText(strong);

    strong = QString ("%1").arg(m_RefArea *Units::m2toUnit(), 8,'f',3);
    m_plabRefArea->setText(strong);

    strong = QString ("%1").arg(m_RefAR , 8,'f',3);
    m_plabRefAR->setText(strong);

    strong = QString ("%1").arg(m_RefTR , 8,'f',3);
    m_plabRefTR->setText(strong);

    m_pdeNewSpan->setValue(m_NewSpan * Units::mtoUnit());
    m_pdeNewChord->setValue(m_NewChord * Units::mtoUnit());
    m_pdeNewSweep->setValue(m_NewSweep);
    m_pdeNewTwist->setValue(m_NewTwist);
    m_pdeNewArea->setValue(m_NewArea*Units::m2toUnit());
    m_pdeNewAR->setValue(m_NewAR);
    m_pdeNewTR->setValue(m_NewTR);

    setResults();
    enableControls();
}



void WingScaleDlg::onClickedCheckBox()
{
    readData();
    enableControls();
}


void WingScaleDlg::onOK()
{
    readData();
    accept();
}

void WingScaleDlg::onEditingFinished()
{
    readData();
    setResults();
}



void WingScaleDlg::enableControls()
{
    m_pdeNewTR->setEnabled(m_bTR);

    m_pdeNewSpan->setEnabled(m_bSpan);
    m_pdeNewChord->setEnabled(m_bChord);
    m_pdeNewSweep->setEnabled(m_bSweep);
    m_pdeNewTwist->setEnabled(m_bTwist);
    m_pdeNewArea->setEnabled(m_bArea);
    m_pdeNewAR->setEnabled(m_bAR);
}


void WingScaleDlg::readData()
{
    m_bSpan  = m_pchSpan->isChecked();
    m_bChord = m_pchChord->isChecked();
    m_bSweep = m_pchSweep->isChecked();
    m_bTwist = m_pchTwist->isChecked();
    m_bArea  = m_pchScaleArea->isChecked();
    m_bAR    = m_pchScaleAR->isChecked();
    m_bTR    = m_pchScaleTR->isChecked();

    m_NewSpan  = m_pdeNewSpan->value()  / Units::mtoUnit();
    m_NewChord = m_pdeNewChord->value() / Units::mtoUnit();
    m_NewSweep = m_pdeNewSweep->value();
    m_NewTwist = m_pdeNewTwist->value();
    m_NewArea  = m_pdeNewArea->value() /Units::m2toUnit();
    m_NewAR    = m_pdeNewAR->value();
    m_NewTR    = m_pdeNewTR->value();
}


void WingScaleDlg::setResults()
{
    QString strong;

    if(m_RefSpan>0.0)  strong = QString("%1").arg(m_NewSpan/m_RefSpan, 6,'f',3);
    else               strong =" 1.000";
    m_plabSpanRatio->setText(strong);

    if(m_RefChord>0.0) strong = QString("%1").arg(m_NewChord/m_RefChord, 6,'f',3);
    else               strong =" 1.000";
    m_plabChordRatio->setText(strong);

    if(m_RefSweep>0.0) strong = QString("%1").arg(m_NewSweep/m_RefSweep, 6,'f',3);
    else               strong =" 1.000";
    m_plabSweepRatio->setText(strong);

    if(m_RefTwist>0.0) strong = QString("%1").arg(m_NewTwist/m_RefTwist, 6,'f',3);
    else               strong =" 1.000";
    m_plabTwistRatio->setText(strong);

    if(m_RefArea>0.0)  strong = QString("%1").arg(m_NewArea/m_RefArea, 6,'f',3);
    else               strong =" 1.000";
    m_plabAreaRatio->setText(strong);

    if(m_RefAR>0.0)    strong = QString("%1").arg(m_NewAR/m_RefAR, 6,'f',3);
    else               strong =" 1.000";
    m_plabARRatio->setText(strong);

    if(m_RefTR>0.0)    strong = QString("%1").arg(m_NewTR/m_RefTR, 6,'f',3);
    else               strong =" 1.000";
    m_plabTRRatio->setText(strong);
}


