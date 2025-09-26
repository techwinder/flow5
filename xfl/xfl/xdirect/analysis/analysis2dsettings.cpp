/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include <QKeyEvent>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QPushButton>

#include "analysis2dsettings.h"

#include <xfl/xdirect/xdirect.h>
#include <xflcore/xflcore.h>
#include <xflfoil/analysis2d/xfoiltask.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>



QByteArray Analysis2dSettings::s_WindowGeometry;

Analysis2dSettings::Analysis2dSettings(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("XFoil settings");
    setupLayout();
}


void Analysis2dSettings::setupLayout()
{
    QGridLayout *pSettingsLayout = new QGridLayout;
    {
        QString tip("<p><b>Viscous solution acceleration</b><br>"
                       "The execution of a viscous case requires the solution of a large "
                       "linear system every Newton iteration.  The coefficient matrix of "
                       "this system is 1/3 full, although most of its entries are very small. "
                       "Substantial savings in CPU time (factor of 4 or more) result when "
                       "these small entries are neglected.  SUBROUTINE BLSOLV which solves the "
                       "large Newton system ignores any off-diagonal element whose magnitude "
                       "is smaller than the variable VACCEL.<br>"
                       "A nonzero VACCEL parameter should in principle degrade the convergence rate "
                       "of the viscous solution and thus result in more Newton iterations, although "
                       "the effect is usually too small to notice.  For very low Reynolds number "
                       "cases (less than 100000), it MAY adversely affect the convergence rate "
                       "or stability, and one should try reducing VACCEL or even setting it "
                       "to zero if all other efforts at convergence are unsuccessful.<br>"
                       "The value of VACCEL has absolutely no effect on the final converged "
                       "viscous solution (if attained).</p>");
        m_pdeVAccel = new FloatEdit;
        m_pdeVAccel->setToolTip(tip);

        QLabel *plabVaccel  = new QLabel("VAccel=");
        QLabel *plabIterLim = new QLabel("Iteration limit=");
        m_pieIterLimit      = new IntEdit;

        QLabel *plabCdError = new QLabel("Discard points with Cd<");
        m_pfeCdError = new FloatEdit(0.001f);
        m_pfeCdError->setToolTip("<p>Operating points with drag coefficient less than this value will be considered to be spurious and will be discarded.<br>"
                                 "Recommendation: 0.001</p>");

        m_pchInitBL         = new QCheckBox("Initialize B.L. after an unconverged iteration");
        m_pchFullReport     = new QCheckBox("Show full log report after an XFoil analysis");
        m_pchKeepErrorsOpen = new QCheckBox("Keep XFoil interface open if analysis errors");

        pSettingsLayout->addWidget(plabVaccel,          1,1, Qt::AlignRight);
        pSettingsLayout->addWidget(m_pdeVAccel,         1,2);
        pSettingsLayout->addWidget(plabIterLim ,        2,1, Qt::AlignRight);
        pSettingsLayout->addWidget(m_pieIterLimit,      2,2);

        pSettingsLayout->addWidget(plabCdError,         3,1, Qt::AlignRight);
        pSettingsLayout->addWidget(m_pfeCdError,        3,2);

        pSettingsLayout->addWidget(m_pchInitBL,         4,1);
        pSettingsLayout->addWidget(m_pchFullReport,     5,1);
        pSettingsLayout->addWidget(m_pchKeepErrorsOpen, 6,1);
        pSettingsLayout->setColumnStretch(4,1);
        pSettingsLayout->setRowStretch(8,1);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::Reset);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pSettingsLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


/** close button action */
void Analysis2dSettings::reject()
{
    readData();
    QDialog::reject();
}


void Analysis2dSettings::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
                m_pButtonBox->button(QDialogButtonBox::Close)->setFocus();
            else {
                close();
            }
            break;
        }
        case Qt::Key_Escape:
        {
            QDialog::reject();
            break;
        }
        default:
            break;
    }
    pEvent->accept();
}


void Analysis2dSettings::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);
}


void Analysis2dSettings::hideEvent(QHideEvent*pEvent)
{
    QDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();
}


void Analysis2dSettings::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Reset) == pButton)
    {
        XDirect::setKeepOpenOnErrors(true);
        XFoil::setFullReport(false);
        XFoil::vaccel = 0.01;
        XFoilTask::setAutoInitBL(true);
        XFoilTask::setCdError(1.0e-3);
        XFoilTask::setMaxIterations(100);
        initWidget();
    }
    else if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)  reject();
}


void Analysis2dSettings::initWidget()
{
    m_pdeVAccel->setValue(XFoil::VAccel());
    m_pchInitBL->setChecked(XFoilTask::bAutoInitBL());
    m_pieIterLimit->setValue(XFoilTask::maxIterations());
    m_pchFullReport->setChecked(XFoil::fullReport());
    m_pchKeepErrorsOpen->setChecked(XDirect::bKeepOpenOnErrors());
    m_pfeCdError->setValue(XFoilTask::CdError());
}


void Analysis2dSettings::readData()
{
    XDirect::setKeepOpenOnErrors(m_pchKeepErrorsOpen->isChecked());
    XFoil::setFullReport(m_pchFullReport->isChecked());
    XFoil::vaccel = m_pdeVAccel->value();
    XFoilTask::setAutoInitBL(m_pchInitBL->isChecked());
    XFoilTask::setCdError(m_pfeCdError->value());
    XFoilTask::setMaxIterations(m_pieIterLimit->value());
}

