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
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

#include "targetcurvedlg.h"
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>


TargetCurveDlg::TargetCurveDlg(QWidget *pParent) : QDialog(pParent)
{
    setupLayout();
}


void TargetCurveDlg::initDialog(bool bShowElliptic, bool bShowBell, bool bMaxCl, double curveExp, LineStyle const &ls)
{
    m_pchShowEllipticCurve->setChecked(bShowElliptic);
    m_pchShowBellCurve->setChecked(bShowBell);
    m_pdeExptEdit->setValue(curveExp);
    m_prbRadio1->setChecked(bMaxCl);
    m_prbRadio2->setChecked(!bMaxCl);
    m_plbCurveStyle->setTheStyle(ls);
}


void TargetCurveDlg::setupLayout()
{
    QVBoxLayout*pMainLayout = new QVBoxLayout;
    {
        QGroupBox *pEllBox = new QGroupBox;
        {
            QHBoxLayout *pEllLayout = new QHBoxLayout;
            {
                QLabel *pEllipticLabel = new QLabel(QString::fromUtf8("y=sqrt(1-(2x/b)²)"));
                m_pchShowEllipticCurve = new QCheckBox("Show elliptic curve");
                pEllLayout->addWidget(m_pchShowEllipticCurve);
                pEllLayout->addWidget(pEllipticLabel);
            }
            pEllBox->setLayout(pEllLayout);
        }

        QGroupBox *pBellBox = new QGroupBox;
        {
            QGridLayout *pBellLayout = new QGridLayout;
            {
                QLabel *pBellLabel = new QLabel(QString::fromUtf8("y=(1-(2x/b)²)^p"));
                m_pchShowBellCurve = new QCheckBox("Show bell curve");
                pBellLayout->addWidget(m_pchShowBellCurve,1,1);
                pBellLayout->addWidget(pBellLabel,1,2);

                QLabel *pExpLabel = new QLabel("Bell curve exponent:");
                pExpLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_pdeExptEdit = new FloatEdit(1, 2);
                pBellLayout->addWidget(pExpLabel,2,1);
                pBellLayout->addWidget(m_pdeExptEdit,2,2);
            }
            pBellBox->setLayout(pBellLayout);
        }

        QGroupBox *pctrlCLBox = new QGroupBox("Cl adjustment");
        {
            QHBoxLayout *pCLLayout = new QHBoxLayout;
            {
                m_prbRadio1 = new QRadioButton("Max local Cl");
                m_prbRadio2 = new QRadioButton("Wing CL");
                pCLLayout->addWidget(m_prbRadio1);
                pCLLayout->addWidget(m_prbRadio2);
            }
            pctrlCLBox->setLayout(pCLLayout);
        }

        QGroupBox *pStyleBox = new QGroupBox;
        {
            QHBoxLayout *pStyleLayout = new QHBoxLayout;
            {
                QLabel*pCurveStyleLab = new QLabel("Style");
                m_plbCurveStyle = new LineBtn;
                pStyleLayout->addWidget(pCurveStyleLab);
                pStyleLayout->addWidget(m_plbCurveStyle);
                connect(m_plbCurveStyle, SIGNAL(clickedLB(LineStyle)), SLOT(onCurveStyle()));
            }
            pStyleBox->setLayout(pStyleLayout);
        }
        pMainLayout->addWidget(pEllBox);
        pMainLayout->addWidget(pBellBox);
        pMainLayout->addWidget(pctrlCLBox);
        pMainLayout->addWidget(pStyleBox);
    }

    QDialogButtonBox *m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    {
        connect(m_pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);
    }

    pMainLayout->addStretch();
    pMainLayout->addWidget(m_pButtonBox);

    setLayout(pMainLayout);
}


void TargetCurveDlg::onCurveStyle()
{
    LineMenu lineMenu(this, false);
    lineMenu.initMenu(m_plbCurveStyle->theLineStyle());
    lineMenu.exec(QCursor::pos());
    m_plbCurveStyle->setTheStyle(lineMenu.theStyle());
}

double TargetCurveDlg::bellCurveExp() const {return m_pdeExptEdit->value();}

bool TargetCurveDlg::bShowBellCurve() const {return m_pchShowBellCurve->isChecked();}
bool TargetCurveDlg::bShowEllipticCurve() const {return m_pchShowEllipticCurve->isChecked();}
bool TargetCurveDlg::bMaxCl() const {return m_prbRadio1->isChecked();}

LineStyle TargetCurveDlg::lineStyle() const {return m_plbCurveStyle->theLineStyle();}

