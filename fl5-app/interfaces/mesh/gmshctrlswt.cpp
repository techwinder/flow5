/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include "gmshctrlswt.h"

#include <api/units.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>


GmshCtrlsWt::GmshCtrlsWt(QWidget *pParent) : QWidget(pParent)
{
    m_bChanged = false;
    setupLayout();

    connectSignals();
}


void GmshCtrlsWt::setupLayout()
{
    QGridLayout *pMainLayout = new QGridLayout;
    {
        QLabel *plabMinSize = new QLabel("Minimum element size=");
        m_pfeMinSize = new FloatEdit;
        m_pfeMinSize->setToolTip("<p>The minimum acceptable size of the triangles. "
                                 "This value should be stricly positive to prevent "
                                 "excessively fine meshes, high number of elements. and long meshing times."
                                 "</p>"
                                 "<p>"
                                 "Min.acceptable value= 0.1 mm"
                                 "</p>");
        QLabel *plabMinUnit = new QLabel(Units::lengthUnitQLabel());

        QLabel *plabMaxSize = new QLabel("Maximum element size=");
        m_pfeMaxSize = new FloatEdit;
        m_pfeMaxSize->setToolTip("<p>The maximum acceptable size of the triangles. "
                                 "Reduce the value to increase the mesh density.</p>");
        QLabel *plabMaxUnit = new QLabel(Units::lengthUnitQLabel());


        QLabel *plabnCurvature = new QLabel("<p>Nbr. of elements=</p>");
        m_pienCurvature = new IntEdit;
        m_pienCurvature->setToolTip("<p> Automatically compute mesh element sizes from curvature, "
                                    "using the value as the target number of elements per 2&pi;.<br>"
                                    "This is the main parameter that should be used to control mesh density.<br>"
                                    "Recommendation: between 10 and 50.</p>");
        QLabel *plabnUnit = new QLabel("<p>/2&pi;</p>");

        pMainLayout->addWidget(plabMinSize,     1, 1);
        pMainLayout->addWidget(m_pfeMinSize,    1, 2);
        pMainLayout->addWidget(plabMinUnit,     1, 3);
        pMainLayout->addWidget(plabMaxSize,     2, 1);
        pMainLayout->addWidget(m_pfeMaxSize,    2, 2);
        pMainLayout->addWidget(plabMaxUnit,     2, 3);
        pMainLayout->addWidget(plabnCurvature,  3, 1);
        pMainLayout->addWidget(m_pienCurvature, 3, 2);
        pMainLayout->addWidget(plabnUnit,       3, 3);

        pMainLayout->setColumnStretch(1,1);
    }

    setLayout(pMainLayout);
}


void GmshCtrlsWt::connectSignals()
{
    connect(m_pfeMaxSize,    SIGNAL(floatChanged(float)), SLOT(onParamChanged()));
    connect(m_pfeMinSize,    SIGNAL(floatChanged(float)), SLOT(onParamChanged()));
    connect(m_pienCurvature, SIGNAL(intChanged(int)),     SLOT(onParamChanged()));
}


void GmshCtrlsWt::onParamChanged()
{
    m_bChanged = true;
}


void GmshCtrlsWt::initWt(GmshParams const &params)
{
    m_pfeMinSize->setValue(params.m_MinSize*Units::mtoUnit());
    m_pfeMaxSize->setValue(params.m_MaxSize*Units::mtoUnit());
    m_pienCurvature->setValue(params.m_nCurvature);
}


GmshParams GmshCtrlsWt::params() const
{
    GmshParams params;
    params.m_MinSize = m_pfeMinSize->value()/Units::mtoUnit();
    params.m_MaxSize = m_pfeMaxSize->value()/Units::mtoUnit();
    params.m_nCurvature = m_pienCurvature->value();

    params.m_MinSize = std::max(params.m_MinSize, 0.001); // failsafe
    return params;
}


