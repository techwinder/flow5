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


#include <QHeaderView>
#include <QVBoxLayout>

#include "planestlinertiadlg.h"


#include <interfaces/editors/inertia/partinertiamodel.h>
#include <interfaces/editors/inertia/pointmasstable.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dplanestlview.h>
#include <interfaces/opengl/fl5views/gl3dplanexflview.h>
#include <core/saveoptions.h>
#include <api/units.h>
#include <api/objects3d.h>
#include <api/planestl.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customdlg/stringvaluedlg.h>

PlaneStlInertiaDlg::PlaneStlInertiaDlg(QWidget *pParent) : PlaneInertiaDlg(pParent)
{
    m_pPlaneSTL = nullptr;
    setupLayout();
    connectSignals();
}


void PlaneStlInertiaDlg::setupLayout()
{
    QString strMass, strLength;
    strMass = Units::massUnitQLabel();
    strLength = Units::lengthUnitQLabel();

    m_pHSplitter = new QSplitter;
    {
        m_pHSplitter->setChildrenCollapsible(false);

        QWidget *pLeftWidget = new QWidget;
        {
            m_plabPlaneName = new QLabel;
            m_plabPlaneName->setStyleSheet("font: bold");

            QVBoxLayout * pLeftLayout = new QVBoxLayout;
            {
                QHBoxLayout *pInputSourceLayout = new QHBoxLayout;
                {
                    m_pchAutoEstimation = new QCheckBox("Auto estimation");
                    QLabel *plabStructMass = new QLabel("Structural mass");
                    QLabel *pLabMassUnit = new QLabel(Units::massUnitQLabel());
                    m_pdeStructMass = new FloatEdit;
                    pInputSourceLayout->addWidget(m_pchAutoEstimation);
                    pInputSourceLayout->addStretch();
                    pInputSourceLayout->addWidget(plabStructMass);
                    pInputSourceLayout->addWidget(m_pdeStructMass);
                    pInputSourceLayout->addWidget(pLabMassUnit);
                }
                m_pVSplitter = new QSplitter(Qt::Vertical);
                {
                    m_pVSplitter->setChildrenCollapsible(false);
                    m_pVSplitter->addWidget(m_pInertiaManTable);
                    m_pVSplitter->addWidget(m_pfrPointMass);
                }

                pLeftLayout->addWidget(m_plabPlaneName);
                pLeftLayout->addLayout(pInputSourceLayout);
                pLeftLayout->addWidget(m_pVSplitter);
                pLeftLayout->addWidget(m_pfrTotalMass);
                pLeftLayout->addWidget(m_pButtonBox);
            }
            pLeftWidget->setLayout(pLeftLayout);
        }

        QWidget *pRightWidget = new QWidget;
        {
            QVBoxLayout *pRightLayout = new QVBoxLayout;
            {
                m_pgl3dPlaneView = new gl3dPlaneSTLView;

                m_pgl3dControls = new gl3dGeomControls(m_pgl3dPlaneView, InertiaLayout, true);
                m_pgl3dControls->showMasses(true);
                m_pgl3dControls->showHighlightCtrl(false);

                pRightLayout->addWidget(m_pgl3dPlaneView);
                pRightLayout->addWidget(m_pgl3dControls);
            }
            pRightWidget->setLayout(pRightLayout);
        }

        m_pHSplitter->addWidget(pLeftWidget);
        m_pHSplitter->addWidget(pRightWidget);
        m_pHSplitter->setStretchFactor(0, 1);
        m_pHSplitter->setStretchFactor(1, 3);
    }

    QHBoxLayout*pMainLayout = new QHBoxLayout;
    {
        pMainLayout->addWidget(m_pHSplitter);
    }

    setLayout(pMainLayout);
}


void PlaneStlInertiaDlg::connectSignals()
{
    connectBaseSignals();
    connect(m_pchAutoEstimation, SIGNAL(clicked(bool)),  SLOT(onAutoInertia()));
    connect(m_pdeStructMass,     SIGNAL(floatChanged(float)), SLOT(onAutoInertia()));
}


void PlaneStlInertiaDlg::initDialog(Plane *pPlane)
{
    m_pPlaneSTL = dynamic_cast<PlaneSTL*>(pPlane);
    gl3dPlaneSTLView *pgl3dPlaneStlView = dynamic_cast<gl3dPlaneSTLView*>(m_pgl3dPlaneView);
    pgl3dPlaneStlView->setPlane(m_pPlaneSTL);

    m_pdeStructMass->setValue(pPlane->inertia().structuralMass()*Units::kgtoUnit());
    m_pchAutoEstimation->setChecked(m_pPlaneSTL->bAutoInertia());

    PlaneInertiaDlg::initDialog(pPlane);
}


void PlaneStlInertiaDlg::onImportExisting()
{
    // list existing planes
    QStringList list;
    for(int ip=0; ip<Objects3d::nPlanes(); ip++)
    {
        list.append(QString::fromStdString(Objects3d::planeAt(ip)->name()));
    }
    StringValueDlg dlg(this, list);
    if(dlg.exec()!=QDialog::Accepted) return;

    Plane const* pRefPlane = Objects3d::plane(dlg.selectedString().toStdString());
    if(!pRefPlane) return;

    m_pPlane->setInertia(pRefPlane->inertia());
    m_ppmtMasses->setInertia(&m_pPlane->inertia());
    m_ppmtMasses->fillMassModel();

    m_pPlaneSTL->setAutoInertia(false);
    m_pchAutoEstimation->setChecked(false);
    m_pdeStructMass->setValue(pRefPlane->structuralMass()*Units::kgtoUnit());
    m_pdeStructMass->setEnabled(false);
    m_pStructInertiaModel->setInertia(&m_pPlane->inertia());
    m_pStructInertiaModel->updateData();
    m_pInertiaManTable->setEnabled(true);

    updateTotalInertia();

    m_bChanged = true;
}


void PlaneStlInertiaDlg::onCopyInertiaToClipboard()
{

}


void PlaneStlInertiaDlg::onExportToAVL()
{

}


void PlaneStlInertiaDlg::onAutoInertia()
{
    m_pPlaneSTL->setAutoInertia(m_pchAutoEstimation->isChecked());
    m_pdeStructMass->setEnabled(m_pPlaneSTL->bAutoInertia());
    m_pInertiaManTable->setEnabled(!m_pPlaneSTL->bAutoInertia());

    if(m_pPlaneSTL->bAutoInertia())
    {
        double mass = m_pdeStructMass->value()/Units::kgtoUnit();
        m_pPlaneSTL->inertia().setStructuralMass(mass);
        m_pPlaneSTL->computeStructuralInertia();
        m_pStructInertiaModel->updateData();

        updateTotalInertia();
        m_bChanged = true;
        onRedraw();
    }
    else
    {
        m_pPlaneSTL->inertia().setStructuralMass(m_pdeStructMass->value()/Units::kgtoUnit());
    }
    m_pInertiaManTable->update();
    updateTotalInertia();
}


void PlaneStlInertiaDlg::onOK(int iExitCode)
{
    m_ppmtMasses->readPointMassData();

    done(iExitCode);
}




