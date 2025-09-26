/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QGridLayout>

#include <xflocc/occmeshcontrols.h>
#include <xflcore/units.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>



OccMeshControls::OccMeshControls(QWidget *parent) : QDialog(parent)
{
    setupLayout();
    connectSignals();
}


void OccMeshControls::setupLayout()
{
    QString strLength = Units::lengthUnitLabel();

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QString warning("Proceed with caution!\n"
                        "OCC tessellation at high precision can be lengthy\n"
                        "and produce an excessive number of triangles.");
        QLabel *pCautionLabel = new QLabel(warning);

        QGridLayout *pMeshLayout = new QGridLayout;
        {
            QLabel *plabDeflectionType = new QLabel(tr("Deflection type:"));
            plabDeflectionType->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            m_prbLinAbs = new QRadioButton(tr("Absolute"));
            m_prbLinRel = new QRadioButton(tr("Relative"));

            m_pLabLinDefAbs = new QLabel("Absolute linear deflection:");
            m_pLabLinDefAbs->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            m_plabAbsUnit = new QLabel(strLength);
            m_plabAbsUnit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

            m_pdeLinDefAbs  = new FloatEdit(0.0, 3);
            m_pdeLinDefAbs->setToolTip(tr("The linear deflection is the maximum distance between an edge of the\n"
                                            "mesh and the corresponding surface. This will always be non-zero for a\n"
                                            "curved surface."
                                            "Caution: too small a value will lead to large tessellation sizes"
                                            "and slow load times."));

            m_pLabLinDefRel = new QLabel("Relative linear deflection:");
            m_pLabLinDefRel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            m_plabRelUnit = new QLabel(tr("% edge length"));
            m_plabRelUnit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

            m_pdeLinDefRel  = new FloatEdit(0.0, 3);
            m_pdeLinDefRel->setToolTip(tr("The linear deflection is the maximum distance between an edge of the\n"
                                            "mesh and the corresponding surface. This will always be non-zero for a\n"
                                            "curved surface.\n"
                                            "Recommendation ~1 to 3°"));

            QLabel *plabAngDeflection = new QLabel("Angular deflection:");
            plabAngDeflection->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            QLabel *plabAngUnit = new QLabel("<p>&deg;</p>");
            plabAngUnit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
            m_pdeAngDeviation  = new FloatEdit(0.0, 3);
            m_pdeAngDeviation->setToolTip("Angular deflection is the upper limit to the angle between subsequent segments in a polyline\n"
                                            "https://www.opencascade.com/doc/occt-7.1.0/overview/html/occt_user_guides__modeling_algos.html#occt_modalg_11_2"
                                            "Recommendation: ~10°");


            pMeshLayout->addWidget(plabDeflectionType, 2,1);
            pMeshLayout->addWidget(m_prbLinAbs,        2,2);
            pMeshLayout->addWidget(m_prbLinRel,        2,3);

            pMeshLayout->addWidget(m_pLabLinDefAbs,3,1);
            pMeshLayout->addWidget(m_pdeLinDefAbs,3,2);
            pMeshLayout->addWidget(m_plabAbsUnit,3,3,Qt::AlignLeft|Qt::AlignVCenter);

            pMeshLayout->addWidget(m_pLabLinDefRel,4,1);
            pMeshLayout->addWidget(m_pdeLinDefRel,4,2);
            pMeshLayout->addWidget(m_plabRelUnit,4,3,Qt::AlignLeft|Qt::AlignVCenter);

            pMeshLayout->addWidget(plabAngDeflection,5,1);
            pMeshLayout->addWidget(m_pdeAngDeviation,5,2);
            pMeshLayout->addWidget(plabAngUnit,5,3, Qt::AlignLeft|Qt::AlignVCenter);

            pMeshLayout->setColumnStretch(1,1);
        }

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        {
            connect(m_pButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(m_pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }

        pMainLayout->addWidget(pCautionLabel);
        pMainLayout->addStretch();
        pMainLayout->addLayout(pMeshLayout);
        pMainLayout->addStretch();
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);
}


void OccMeshControls::connectSignals()
{
    connect(m_prbLinAbs,            SIGNAL(clicked(bool)),     SLOT(onParamChanged()));
    connect(m_prbLinRel,            SIGNAL(clicked(bool)),     SLOT(onParamChanged()));

    connect(m_pdeLinDefAbs,       SIGNAL(floatChanged(float)), SLOT(onParamChanged()));
    connect(m_pdeLinDefRel,       SIGNAL(floatChanged(float)), SLOT(onParamChanged()));
    connect(m_pdeAngDeviation,    SIGNAL(floatChanged(float)), SLOT(onParamChanged()));
}


void OccMeshControls::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    m_bChanged = false;
    setControls();
}


void OccMeshControls::initDialog(OccMeshParams const &params, bool bShowBtnBox)
{
    m_Params = params;

    m_prbLinAbs->setChecked(!m_Params.isRelativeDeflection());
    m_prbLinRel->setChecked(m_Params.isRelativeDeflection());

    m_pdeLinDefAbs->setValue(m_Params.deflectionAbsolute()*Units::mtoUnit());
    m_pdeLinDefRel->setValue(m_Params.deflectionRelative()*100.0);

    m_pdeAngDeviation->setValue(m_Params.angularDeviation());

    m_pButtonBox->setVisible(bShowBtnBox);
}


void OccMeshControls::readParams()
{
    m_Params.setDefRelative(m_prbLinRel->isChecked());
    double defabs = m_pdeLinDefAbs->value()/Units::mtoUnit();
    defabs = std::max(defabs, 0.001); // to prevent errors and lengthy tessellation operations
    m_Params.setDefAbsolute(defabs);
    double defrel = m_pdeLinDefRel->value()/100.0;
    defrel = std::max(defrel, 0.01); // to prevent errors and lengthy tessellation operations
    m_Params.setDefRelative(defrel);
    m_Params.setAngularDeviation(m_pdeAngDeviation->value());
}


void OccMeshControls::accept()
{
    readParams();
    QDialog::accept();
}


void OccMeshControls::onParamChanged()
{
    setControls();
    m_bChanged = true;
}


void OccMeshControls::setControls()
{
    if(m_prbLinAbs->isChecked())
    {
        m_Params.setDefRelative(false);
        m_pdeLinDefRel->setEnabled(false);
        m_plabRelUnit->setEnabled(false);
        m_pLabLinDefRel->setEnabled(false);
        m_plabAbsUnit->setEnabled(true);
        m_pdeLinDefAbs->setEnabled(true);
        m_pLabLinDefAbs->setEnabled(true);
    }
    else
    {
        m_Params.setDefRelative(true);
        m_pdeLinDefRel->setEnabled(true);
        m_plabRelUnit->setEnabled(true);
        m_pLabLinDefRel->setEnabled(true);
        m_plabAbsUnit->setEnabled(false);
        m_pdeLinDefAbs->setEnabled(false);
        m_pLabLinDefAbs->setEnabled(false);
    }
}


// length unit may have been changed in preferences
void OccMeshControls::updateUnits()
{
    m_plabAbsUnit->setText(Units::lengthUnitLabel());
    m_pdeLinDefAbs->setValue(m_Params.deflectionAbsolute()*Units::mtoUnit());
}


