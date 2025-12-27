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



//performs an automatic evaluation of the object's CoG and inertia properties

#include <QApplication>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QClipboard>

#include "planeinertiadlg.h"

#include <interfaces/editors/inertia/partinertiadelegate.h>
#include <interfaces/editors/inertia/partinertiadlg.h>
#include <interfaces/editors/inertia/partinertiamodel.h>
#include <interfaces/editors/inertia/pointmasstable.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dplanexflview.h>
#include <options/prefsdlg.h>
#include <core/saveoptions.h>
#include <api/units.h>
#include <core/xflcore.h>
#include <api/constants.h>
#include <api/pointmass.h>
#include <api/planexfl.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>


QByteArray PlaneInertiaDlg::s_Geometry;

QByteArray PlaneInertiaDlg::s_HSplitterSizes;
QByteArray PlaneInertiaDlg::s_VSplitterSizes;

Quaternion PlaneInertiaDlg::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);


PlaneInertiaDlg::PlaneInertiaDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Inertia Properties");
    setWindowFlag(Qt::WindowMinMaxButtonsHint);

    m_pPlane = nullptr;

    m_bChanged = false;

    makeCommonWts();
}


PlaneInertiaDlg::~PlaneInertiaDlg()
{
}


/**
* Computes the inertia in the frame of reference with origin at the CoG.
* Assumes that the data has been read.
*/
void PlaneInertiaDlg::updateTotalInertia()
{
    // update the plane
    if(m_pPlane->bAutoInertia())
        m_pPlane->computeStructuralInertia();

    //display the results
    m_pfeTotalMass->setValue(m_pPlane->totalMass()*Units::kgtoUnit());

    m_pfeXTotalCoG->setValue(m_pPlane->CoG_t().x*Units::mtoUnit());
    m_pfeYTotalCoG->setValue(m_pPlane->CoG_t().y*Units::mtoUnit());
    m_pfeZTotalCoG->setValue(m_pPlane->CoG_t().z*Units::mtoUnit());

    m_pfeTotalIxx->setValue(m_pPlane->Ixx_t()*Units::kgm2toUnit());
    m_pfeTotalIyy->setValue(m_pPlane->Iyy_t()*Units::kgm2toUnit());
    m_pfeTotalIzz->setValue(m_pPlane->Izz_t()*Units::kgm2toUnit());
    m_pfeTotalIxz->setValue(m_pPlane->Ixz_t()*Units::kgm2toUnit());
}


void PlaneInertiaDlg::initDialog(Plane *pPlane)
{
    m_pPlane = pPlane;
    setWindowTitle("Inertia properties");

    if(pPlane)
        m_plabPlaneName->setText(QString::fromStdString(pPlane->name()));

    m_pInertiaManTable->setEnabled(!m_pPlane->bAutoInertia());
    m_pStructInertiaModel->setInertia(&m_pPlane->inertia());
    m_pStructInertiaModel->setEditable(!m_pPlane->bAutoInertia());
    m_pStructInertiaModel->updateData();
    m_pInertiaManTable->update();

    m_ppmtMasses->setInertia(&m_pPlane->inertia());
    m_ppmtMasses->fillMassModel();

    updateTotalInertia();
    setFocus();
}


void PlaneInertiaDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            m_pButtonBox->button(QDialogButtonBox::Save)->setFocus();
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        case Qt::Key_S:
        {
            if(bCtrl)
                onOK();
            break;
        }
        default:
            pEvent->ignore();
    }
}


void PlaneInertiaDlg::reject()
{
    if(m_bChanged && xfl::bConfirmDiscard())
    {
        int Ans = QMessageBox::question(this, "Question", "Discard the changes?",
                                        QMessageBox::Yes | QMessageBox::Cancel);
        if (QMessageBox::Yes == Ans)
        {
            done(QDialog::Rejected);
            return;
        }
        else return;
    }

    done(QDialog::Rejected);
}


void PlaneInertiaDlg::onManInertiaCellChanged(QModelIndex )
{
    updateTotalInertia();
    m_bChanged = true;
    onRedraw();
}


void PlaneInertiaDlg::onPointMassCellChanged()
{
    updateTotalInertia();
    m_bChanged = true;
    onRedraw();
}


void PlaneInertiaDlg::onPointMassDataPasted()
{
    m_ppmtMasses->readPointMassData();
    onPointMassCellChanged();
}


void PlaneInertiaDlg::onMassRowChanged(QModelIndex index)
{
    m_pgl3dPlaneView->setSelectedPlaneMass(index.row());
    m_pgl3dPlaneView->update();
}


void PlaneInertiaDlg::connectBaseSignals()
{
    connect(m_pStructInertiaModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(onManInertiaCellChanged(QModelIndex)));

    connect(m_ppmtMasses->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onMassRowChanged(QModelIndex)));
    connect(m_ppmtMasses, SIGNAL(pointMassChanged()), SLOT(onPointMassCellChanged()));
    connect(m_ppmtMasses, SIGNAL(dataPasted()),       SLOT(onPointMassDataPasted()));

    connect(m_pImportInertia,     SIGNAL(triggered()), SLOT(onImportExisting()));
    connect(m_pExportToAVL,       SIGNAL(triggered()), SLOT(onExportToAVL()));
    connect(m_pExportToClipboard, SIGNAL(triggered()), SLOT(onCopyInertiaToClipboard()));

    connect(m_pHSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(onResizeColumns()));
}


void PlaneInertiaDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)          onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
    else if (pButton == m_ppbSaveAsNew)                                 onOK(10);
}


void PlaneInertiaDlg::onResizeColumns()
{
    QHeaderView *pHHeader = m_pInertiaManTable->horizontalHeader();
    pHHeader->setStretchLastSection(true);
    double w = double(m_pInertiaManTable->width())*0.95;
    m_pInertiaManTable->setColumnWidth(0, int(w/3));
    m_pInertiaManTable->setColumnWidth(1, int(w/3));
//    m_pInertiaManTable->setColumnWidth(2, int(w/3));

    m_ppmtMasses->resizeColumns();
}


void PlaneInertiaDlg::resizeEvent(QResizeEvent *pEvent)
{
    onResizeColumns();
    pEvent->accept();
}


void PlaneInertiaDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);

    m_pgl3dPlaneView->reset3dScale();

    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    if(s_VSplitterSizes.length()>0) m_pVSplitter->restoreState(s_VSplitterSizes);

    onResizeColumns();

    m_pgl3dPlaneView->restoreViewPoint(s_ab_quat);

    pEvent->accept();
}


void PlaneInertiaDlg::hideEvent(QHideEvent*pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
    s_HSplitterSizes  = m_pHSplitter->saveState();
    s_VSplitterSizes  = m_pVSplitter->saveState();

    m_pgl3dPlaneView->saveViewPoint(s_ab_quat);

    pEvent->accept();
}


void PlaneInertiaDlg::onRedraw()
{
    m_pgl3dPlaneView->update();
}


void PlaneInertiaDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("PlaneInertiaDlg");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();
        s_HSplitterSizes = settings.value("HSplitterSize").toByteArray();
        s_VSplitterSizes = settings.value("VSplitterSize").toByteArray();
    }
    settings.endGroup();
}



void PlaneInertiaDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("PlaneInertiaDlg");
    {
        settings.setValue("WindowGeometry", s_Geometry);
        settings.setValue("HSplitterSize", s_HSplitterSizes);
        settings.setValue("VSplitterSize", s_VSplitterSizes);
    }
    settings.endGroup();
}


void PlaneInertiaDlg::makeCommonWts()
{
    m_pInertiaManTable = new CPTableView(this);
    m_pInertiaManTable->setEditable(true);
    m_pInertiaManTable->setWindowTitle("Wing list");
    m_pInertiaManTable->setWordWrap(false);
//    m_pInertiaManTable->sizePolicy().setVerticalStretch(2);
    m_pStructInertiaModel = new PartInertiaModel(this);
    m_pInertiaManTable->setModel(m_pStructInertiaModel);

    m_pPartInertiaDelegate = new PartInertiaDelegate(this);
    QVector<int> precision= {-1,3,-1};
    m_pPartInertiaDelegate->setPrecision(precision);
    m_pInertiaManTable->setItemDelegate(m_pPartInertiaDelegate);

    m_pfrPointMass = new QFrame;
    {
        QVBoxLayout *pPointMassLayout = new QVBoxLayout;
        {
            QLabel *pPointMassesLabel = new QLabel("Additional point masses");
            pPointMassesLabel->setStyleSheet("QLabel { font-weight: bold;}");

            m_ppmtMasses = new PointMassTable(this);

            pPointMassLayout->addWidget(pPointMassesLabel);
            pPointMassLayout->addWidget(m_ppmtMasses);
        }
        m_pfrPointMass->setLayout(pPointMassLayout);
    }

    //________________Total Mass, Center of gravity, and inertias__________
    m_pfrTotalMass = new QFrame;
    {
        QVBoxLayout *pTotalLayout = new QVBoxLayout;
        {
            QLabel *pTotalMassLabel = new QLabel("Total Mass = Volume + point masses");
            pTotalMassLabel->setStyleSheet("QLabel { font-weight: bold;}");
            QGroupBox *pTotalCoGBox = new QGroupBox("Center of gravity");
            {
                QGridLayout *pTotalCoGLayout = new QGridLayout;
                {
                    QLabel *TotalLabel = new QLabel("Center of gravity");
                    TotalLabel->setAlignment(Qt::AlignRight);
                    QLabel *pXTotalLab = new QLabel("CoG_x=");
                    QLabel *pYTotalLab = new QLabel("CoG_y=");
                    QLabel *pZTotalLab = new QLabel("CoG_z=");
                    m_pfeXTotalCoG = new FloatEdit(0.0f, 3);
                    m_pfeYTotalCoG = new FloatEdit(0.0f, 3);
                    m_pfeZTotalCoG = new FloatEdit(0.0f, 3);
                    m_pfeXTotalCoG->setEnabled(false);
                    m_pfeYTotalCoG->setEnabled(false);
                    m_pfeZTotalCoG->setEnabled(false);
                    QLabel *plabLengthUnit20 = new QLabel(Units::lengthUnitQLabel());
                    QLabel *plabLengthUnit21 = new QLabel(Units::lengthUnitQLabel());
                    QLabel *plabLengthUnit22 = new QLabel(Units::lengthUnitQLabel());
                    QLabel *plabTotalMass    = new QLabel("Total Mass=");
                    QLabel *plabMassUnit2    = new QLabel(Units::massUnitQLabel());
                    m_pfeTotalMass     = new FloatEdit(0.0f,3);
                    m_pfeTotalMass->setEnabled(false);
                    pTotalCoGLayout->addWidget(plabTotalMass,       1,1);
                    pTotalCoGLayout->addWidget(m_pfeTotalMass,      1,2);
                    pTotalCoGLayout->addWidget(plabMassUnit2,       1,3);
                    pTotalCoGLayout->addWidget(pXTotalLab,          2,1, Qt::AlignRight);
                    pTotalCoGLayout->addWidget(pYTotalLab,          3,1, Qt::AlignRight);
                    pTotalCoGLayout->addWidget(pZTotalLab,          4,1, Qt::AlignRight);
                    pTotalCoGLayout->addWidget(m_pfeXTotalCoG,      2,2);
                    pTotalCoGLayout->addWidget(m_pfeYTotalCoG,      3,2);
                    pTotalCoGLayout->addWidget(m_pfeZTotalCoG,      4,2);
                    pTotalCoGLayout->addWidget(plabLengthUnit20,    2,3);
                    pTotalCoGLayout->addWidget(plabLengthUnit21,    3,3);
                    pTotalCoGLayout->addWidget(plabLengthUnit22,    4,3);
                    pTotalCoGLayout->setColumnStretch(1,1);
                    pTotalCoGLayout->setColumnStretch(2,2);
                    pTotalCoGLayout->setColumnStretch(3,1);
                }
                pTotalCoGBox->setLayout(pTotalCoGLayout);
            }

            QGroupBox *pTotalInertiaBox = new QGroupBox("Inertia in CoG Frame");
            {
                QGridLayout *pTotalInertiaLayout = new QGridLayout;
                {
                    m_pfeTotalIxx = new FloatEdit(0,5);
                    m_pfeTotalIyy = new FloatEdit(0,5);
                    m_pfeTotalIzz = new FloatEdit(0,5);
                    m_pfeTotalIxz = new FloatEdit(0,5);
                    m_pfeTotalIxx->setEnabled(false);
                    m_pfeTotalIyy->setEnabled(false);
                    m_pfeTotalIzz->setEnabled(false);
                    m_pfeTotalIxz->setEnabled(false);
                    QLabel *plabTotIxx = new QLabel("Ixx=");
                    QLabel *plabTotIyy = new QLabel("Iyy=");
                    QLabel *plabTotIzz = new QLabel("Izz=");
                    QLabel *plabTotIxz = new QLabel("Ixz=");

                    QLabel *plabInertiaUnit10 = new QLabel(Units::inertiaUnitQLabel());
                    QLabel *plabInertiaUnit20 = new QLabel(Units::inertiaUnitQLabel());
                    QLabel *plabInertiaUnit30 = new QLabel(Units::inertiaUnitQLabel());
                    QLabel *plabInertiaUnit40 = new QLabel(Units::inertiaUnitQLabel());
                    pTotalInertiaLayout->addWidget(plabTotIxx,           1, 1, Qt::AlignRight);
                    pTotalInertiaLayout->addWidget(plabTotIyy,           2, 1, Qt::AlignRight);
                    pTotalInertiaLayout->addWidget(plabTotIzz,           3, 1, Qt::AlignRight);
                    pTotalInertiaLayout->addWidget(plabTotIxz,           4, 1, Qt::AlignRight);
                    pTotalInertiaLayout->addWidget(m_pfeTotalIxx,        1, 2);
                    pTotalInertiaLayout->addWidget(m_pfeTotalIyy,        2, 2);
                    pTotalInertiaLayout->addWidget(m_pfeTotalIzz,        3, 2);
                    pTotalInertiaLayout->addWidget(m_pfeTotalIxz,        4, 2);
                    pTotalInertiaLayout->addWidget(plabInertiaUnit10,    1, 3);
                    pTotalInertiaLayout->addWidget(plabInertiaUnit20,    2, 3);
                    pTotalInertiaLayout->addWidget(plabInertiaUnit30,    3, 3);
                    pTotalInertiaLayout->addWidget(plabInertiaUnit40,    4, 3);
                    pTotalInertiaLayout->setColumnStretch(1,1);
                    pTotalInertiaLayout->setColumnStretch(2,2);
                    pTotalInertiaLayout->setColumnStretch(3,1);
                }
                pTotalInertiaBox->setLayout(pTotalInertiaLayout);
            }
            QHBoxLayout *pTotalMassLayout = new QHBoxLayout;
            {
                pTotalMassLayout->addWidget(pTotalCoGBox);
                pTotalMassLayout->addWidget(pTotalInertiaBox);
            }
            pTotalLayout->addWidget(pTotalMassLabel);
            pTotalLayout->addLayout(pTotalMassLayout);
        }
        m_pfrTotalMass->setLayout(pTotalLayout);
    }


    //__________________Control buttons___________________
    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
    {
        QPushButton *ppbActtions = new QPushButton("Actions");
        {
            QMenu *pActionMenu = new QMenu;
            {
                m_pImportInertia = new QAction("Import from other plane");
                QMenu *pExportMenu = new QMenu("Export");
                {
                    m_pExportToAVL = new QAction("to AVL file", this);
                    m_pExportToClipboard = new QAction("to clipboard", this);
                    pExportMenu->addAction(m_pExportToAVL);
                    pExportMenu->addAction(m_pExportToClipboard);
                }
                pActionMenu->addAction(m_pImportInertia);
                pActionMenu->addSeparator();
                pActionMenu->addMenu(pExportMenu);
            }
            ppbActtions->setMenu(pActionMenu);
        }
        m_pButtonBox->addButton(ppbActtions, QDialogButtonBox::ActionRole);


        m_ppbSaveAsNew = new QPushButton("Save as");
        m_pButtonBox->addButton(m_ppbSaveAsNew, QDialogButtonBox::ActionRole);

        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }
}
