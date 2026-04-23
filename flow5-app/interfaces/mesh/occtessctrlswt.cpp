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

#include <interfaces/mesh/occtessctrlswt.h>
#include <api/units.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>



OccTessCtrlsWt::OccTessCtrlsWt(QWidget *parent) : QWidget(parent)
{
    m_bChanged = false;
    setupLayout();
    connectSignals();
}


void OccTessCtrlsWt::setupLayout()
{
    QString strLength = Units::lengthUnitQLabel();

    QGridLayout *pMainLayout = new QGridLayout;
    {
        QLabel *plabDeflectionType = new QLabel("Deflection type:");
        plabDeflectionType->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        m_prbLinAbs = new QRadioButton("Absolute");
        m_prbLinRel = new QRadioButton("Relative");

        m_plabLinDefAbs = new QLabel("Absolute linear deflection:");
        m_plabLinDefAbs->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        m_plabAbsUnit = new QLabel(strLength);
        m_plabAbsUnit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        m_pfeLinDefAbs  = new FloatEdit(0.0, 3);
        m_pfeLinDefAbs->setToolTip("<p>The linear deflection is the maximum distance between an edge of the "
                                   "mesh and the corresponding surface. This will always be non-zero for a "
                                   "curved surface.<br>"
                                   "Caution: too small a value will lead to large tessellation sizes"
                                   "and slow load times.</p>");

        m_plabLinDefRel = new QLabel("Relative linear deflection:");
        m_plabLinDefRel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        m_plabRelUnit = new QLabel("% edge length");
        m_plabRelUnit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        m_pfeLinDefRel  = new FloatEdit(0.0, 3);
        m_pfeLinDefRel->setToolTip("<p>The linear deflection is the maximum distance between an edge of the "
                                   "mesh and the corresponding surface. This will always be non-zero for a "
                                   "curved surface.<br>"
                                   "Recommendation ~1 to 3&deg;</p>");

        QLabel *plabAngDeflection = new QLabel("Angular deflection:");
        plabAngDeflection->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        QLabel *plabAngUnit = new QLabel("<p>&deg;</p>");
        plabAngUnit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_pfeAngDeviation  = new FloatEdit(0.0, 3);
        m_pfeAngDeviation->setToolTip("<p>Angular deflection is the upper limit to the angle between subsequent segments in a polyline "
                                      "https://www.opencascade.com/doc/occt-7.1.0/overview/html/occt_user_guides__modeling_algos.html#occt_modalg_11_2<br>"
                                      "Recommendation: ~10&deg;</p>");


        pMainLayout->addWidget(plabDeflectionType, 2,1);
        pMainLayout->addWidget(m_prbLinAbs,        2,2);
        pMainLayout->addWidget(m_prbLinRel,        2,3);

        pMainLayout->addWidget(m_plabLinDefAbs,    3,1);
        pMainLayout->addWidget(m_pfeLinDefAbs,     3,2);
        pMainLayout->addWidget(m_plabAbsUnit,      3,3,Qt::AlignLeft|Qt::AlignVCenter);

        pMainLayout->addWidget(m_plabLinDefRel,    4,1);
        pMainLayout->addWidget(m_pfeLinDefRel,     4,2);
        pMainLayout->addWidget(m_plabRelUnit,      4,3,Qt::AlignLeft|Qt::AlignVCenter);

        pMainLayout->addWidget(plabAngDeflection,  5,1);
        pMainLayout->addWidget(m_pfeAngDeviation,  5,2);
        pMainLayout->addWidget(plabAngUnit,        5,3, Qt::AlignLeft|Qt::AlignVCenter);

        pMainLayout->setColumnStretch(1,1);
    }


    setLayout(pMainLayout);
}


void OccTessCtrlsWt::connectSignals()
{
    connect(m_prbLinAbs,            SIGNAL(clicked(bool)),     SLOT(onParamChanged()));
    connect(m_prbLinRel,            SIGNAL(clicked(bool)),     SLOT(onParamChanged()));

    connect(m_pfeLinDefAbs,       SIGNAL(floatChanged(float)), SLOT(onParamChanged()));
    connect(m_pfeLinDefRel,       SIGNAL(floatChanged(float)), SLOT(onParamChanged()));
    connect(m_pfeAngDeviation,    SIGNAL(floatChanged(float)), SLOT(onParamChanged()));
}


void OccTessCtrlsWt::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);
    m_bChanged = false;
    setControls();
}


void OccTessCtrlsWt::initWt(OccMeshParams const &params)
{
    m_Params = params;

    m_prbLinAbs->setChecked(!m_Params.isRelativeDeflection());
    m_prbLinRel->setChecked(m_Params.isRelativeDeflection());

    m_pfeLinDefAbs->setValue(m_Params.deflectionAbsolute()*Units::mtoUnit());
    m_pfeLinDefRel->setValue(m_Params.deflectionRelative()*100.0);

    m_pfeAngDeviation->setValue(m_Params.angularDeviation());
}


void OccTessCtrlsWt::readParams()
{
    m_Params.setDefRelative(m_prbLinRel->isChecked());
    double defabs = m_pfeLinDefAbs->value()/Units::mtoUnit();
    defabs = std::max(defabs, 0.001); // to prevent errors and lengthy tessellation operations
    m_Params.setDefAbsolute(defabs);
    double defrel = m_pfeLinDefRel->value()/100.0;
    defrel = std::max(defrel, 0.01); // to prevent errors and lengthy tessellation operations
    m_Params.setDefRelative(defrel);
    m_Params.setAngularDeviation(m_pfeAngDeviation->value());
}


void OccTessCtrlsWt::onParamChanged()
{
    setControls();
    m_bChanged = true;
}


void OccTessCtrlsWt::setControls()
{
    if(m_prbLinAbs->isChecked())
    {
        m_Params.setDefRelative(false);
        m_pfeLinDefRel->setEnabled(false);
        m_plabRelUnit->setEnabled(false);
        m_plabLinDefRel->setEnabled(false);
        m_plabAbsUnit->setEnabled(true);
        m_pfeLinDefAbs->setEnabled(true);
        m_plabLinDefAbs->setEnabled(true);
    }
    else
    {
        m_Params.setDefRelative(true);
        m_pfeLinDefRel->setEnabled(true);
        m_plabRelUnit->setEnabled(true);
        m_plabLinDefRel->setEnabled(true);
        m_plabAbsUnit->setEnabled(false);
        m_pfeLinDefAbs->setEnabled(false);
        m_plabLinDefAbs->setEnabled(false);
    }
}


// length unit may have been changed in preferences
void OccTessCtrlsWt::updateUnits()
{
    m_plabAbsUnit->setText(Units::lengthUnitQLabel());
    m_pfeLinDefAbs->setValue(m_Params.deflectionAbsolute()*Units::mtoUnit());
}


