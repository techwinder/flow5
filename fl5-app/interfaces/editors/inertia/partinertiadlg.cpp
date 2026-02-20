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

#include <QApplication>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QTextStream>
#include <QFileDialog>
#include <QAction>
#include <QMenu>
#include <QClipboard>

#include "partinertiadelegate.h"
#include "partinertiadlg.h"
#include "partinertiamodel.h"

#include <core/saveoptions.h>
#include <api/units.h>
#include <core/xflcore.h>
#include <interfaces/editors/inertia/pointmasstable.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dfuseview.h>
#include <interfaces/opengl/fl5views/gl3dplanexflview.h>
#include <interfaces/opengl/fl5views/gl3dwingview.h>
#include <options/prefsdlg.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <api/constants.h>
#include <api/pointmass.h>
#include <api/planexfl.h>


QByteArray PartInertiaDlg::s_Geometry;

QByteArray PartInertiaDlg::s_HSplitterSizes;
QByteArray PartInertiaDlg::s_VSplitterSizes;

Quaternion PartInertiaDlg::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);

PartInertiaDlg::PartInertiaDlg(WingXfl *pWing, Fuse *pFuse, QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Inertia Properties"));
    setWindowFlag(Qt::WindowMinMaxButtonsHint);

    if(pWing)
    {
        m_pPart = pWing;
    }
    else if(pFuse)
    {
        m_pPart = pFuse;
    }
    else m_pPart = nullptr;

    if(m_pPart && m_pPart->bAutoInertia()) m_pPart->computeStructuralInertia(Vector3d());

    setupLayout();
    connectSignals();
    initDialog();

    if(pWing)
    {
        m_pgl3dWingView->setReferenceLength(pWing->planformSpan());
    }
    else if(pFuse)
    {
        m_pgl3dFuseView->setReferenceLength(pFuse->length());
    }

    m_bChanged = false;
}


void PartInertiaDlg::initDialog()
{
    setWindowTitle("Inertia properties for "+QString::fromStdString(m_pPart->name()));

    if(!m_pPart)
    {
        return;
    }

    if(m_pPart->isWing())
    {
        WingXfl *pWing = dynamic_cast<WingXfl*>(m_pPart);
        m_pgl3dWingView->setWing(pWing);
        m_p3dViewStack->setCurrentWidget(m_pgl3dWingView);
    }
    if(m_pPart->isFuse())
    {
        Fuse *pFuse = dynamic_cast<Fuse*>(m_pPart);
        m_pgl3dFuseView->setFuse(pFuse);
        m_p3dViewStack->setCurrentWidget(m_pgl3dFuseView);
    }
    m_ppmtMasses->setInertia(&m_pPart->inertia());
    m_ppmtMasses->fillMassModel();

    m_pStructInertiaModel->setInertia(&m_pPart->inertia());

    m_pchAutoInertia->setChecked(m_pPart->bAutoInertia());
    m_pfeStructMass->setEnabled(m_pPart->bAutoInertia());
    m_pfeStructMass->setValue(m_pPart->structuralMass()*Units::kgtoUnit());

    m_pStructInertiaTable->setEnabled(!m_pPart->bAutoInertia());
    m_pStructInertiaModel->setEditable(!m_pPart->bAutoInertia());
    m_pStructInertiaTable->update();
    m_pStructInertiaModel->updateData();

    updateTotalInertia();
    setFocus();
}


void PartInertiaDlg::reject()
{
    if(m_bChanged && xfl::bConfirmDiscard())
    {
        QString strong = "Discard the changes?";
        int Ans = QMessageBox::question(this, "Question", strong,
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


void PartInertiaDlg::updateTotalInertia()
{
    //display the results
    m_pfeTotalMass->setValue(m_pPart->totalMass()*Units::kgtoUnit());

    Vector3d cogt = m_pPart->CoG_t();
    m_pfeXTotalCoG->setValue(cogt.x*Units::mtoUnit());
    m_pfeYTotalCoG->setValue(cogt.y*Units::mtoUnit());
    m_pfeZTotalCoG->setValue(cogt.z*Units::mtoUnit());

    m_pfeTotalIxx->setValue(m_pPart->Ixx_t()*Units::kgm2toUnit());
    m_pfeTotalIyy->setValue(m_pPart->Iyy_t()*Units::kgm2toUnit());
    m_pfeTotalIzz->setValue(m_pPart->Izz_t()*Units::kgm2toUnit());
    m_pfeTotalIxz->setValue(m_pPart->Ixz_t()*Units::kgm2toUnit());
}


void PartInertiaDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            //            m_pButtonBox->button(QDialogButtonBox::Save)->setFocus();
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


void PartInertiaDlg::onStructInertiaCellChanged()
{
    updateTotalInertia();
    m_bChanged = true;
    onRedraw();
}


void PartInertiaDlg::onPointMassCellChanged()
{
    updateTotalInertia();

    m_bChanged = true;
    onRedraw();
}


void PartInertiaDlg::onPointMassDataPasted()
{
    m_ppmtMasses->readPointMassData();
    updateTotalInertia();

    m_bChanged = true;
    onRedraw();
}


void PartInertiaDlg::onAutoInertia()
{
    m_pPart->setAutoInertia(m_pchAutoInertia->isChecked());
    m_pfeStructMass->setEnabled(m_pPart->bAutoInertia());
    m_pStructInertiaTable->setEnabled(!m_pPart->bAutoInertia());

    if(m_pPart->bAutoInertia())
    {
        double mass = m_pfeStructMass->value()/Units::kgtoUnit();
        m_pPart->setStructuralMass(mass);
        m_pPart->computeStructuralInertia(Vector3d());
        m_pStructInertiaModel->updateData();

        updateTotalInertia();
        m_bChanged = true;
        onRedraw();
    }
    m_pStructInertiaTable->update();
    updateTotalInertia();
}


QString PartInertiaDlg::toString(Inertia const &in) const
{
    QString sep =  " ";
    QString strInertia, strange;

    strInertia += "Structure only:\n";
    strange = QString::asprintf("   mass = %11.5g ", in.structuralMass()*Units::kgtoUnit());
    strange += Units::massUnitQLabel() + "\n";
    strInertia += strange;

    strInertia += "   CoG: " + sep;
    strange = QString::asprintf("%13.5g", in.CoG_s().x * Units::mtoUnit());
    strInertia += strange + sep;
    strange = QString::asprintf("%13.5g", in.CoG_s().y * Units::mtoUnit());
    strInertia += strange + sep;
    strange = QString::asprintf("%13.5g", in.CoG_s().z * Units::mtoUnit());
    strInertia += strange + sep + Units::lengthUnitQLabel();
    strInertia += "\n";

    strInertia += "   inertia in the structure's CoG frame:\n";
    strange = QString::asprintf("      Ixx_struct_cog= %13.5g", in.Ixx_s()*Units::kgm2toUnit());
    strInertia += strange + sep+ Units::inertiaUnitQLabel() + "\n";
    strange = QString::asprintf("      Iyy_struct_cog= %13.5g", in.Iyy_s()*Units::kgm2toUnit());
    strInertia += strange + sep+ Units::inertiaUnitQLabel() + "\n";
    strange = QString::asprintf("      Izz_struct_cog= %13.5g", in.Izz_s()*Units::kgm2toUnit());
    strInertia += strange + sep+ Units::inertiaUnitQLabel() + "\n";
    strange = QString::asprintf("      Ixz_struct_cog= %13.5g", in.Ixz_s()*Units::kgm2toUnit());
    strInertia += strange + sep+ Units::inertiaUnitQLabel() + "\n\n";

    if(in.pointMassCount()>0)
    {
        strInertia += "Additional point masses:\n";
        strInertia += "     Mass (" + Units::massUnitQLabel() + ")" + sep;
        strInertia += "        x ("    + Units::lengthUnitQLabel() + ")" + sep;
        strInertia += "        y ("    + Units::lengthUnitQLabel() + ")" + sep;
        strInertia += "        z ("    + Units::lengthUnitQLabel() + ")" + sep;
        strInertia += "\n";
        for(int im=0; im<in.pointMassCount(); im++)
        {
            strange = QString::asprintf("  %11.5g ", in.pointMassAt(im).mass()*Units::kgtoUnit());
            strInertia += strange + sep;
            strange = QString::asprintf("  %11.5g ", in.pointMassAt(im).position().x*Units::mtoUnit());
            strInertia += strange + sep;
            strange = QString::asprintf("  %11.5g ", in.pointMassAt(im).position().y*Units::mtoUnit());
            strInertia += strange + sep;
            strange = QString::asprintf("  %11.5g ", in.pointMassAt(im).position().z*Units::mtoUnit());
            strInertia += strange + sep ;
            strange = QString::asprintf("  %s", in.pointMassAt(im).tag().c_str());
            strInertia += strange + "\n";
        }
        strInertia += "\n";
    }

    return strInertia;
}


void PartInertiaDlg::onCopyInertiaToClipboard()
{
    QClipboard *pClipBoard = QApplication::clipboard();

    QString sep = SaveOptions::textSeparator() + " ";
    QString strInertia, strange;

    strInertia = "Inertia properties for "+QString::fromStdString(m_pPart->name())+"\n\n";

    strInertia += toString(m_pPart->inertia());

    strInertia += "Total:\n";

    strange = QString::asprintf("   Mass = %11.5g ", m_pPart->totalMass()*Units::kgtoUnit());
    strange += Units::massUnitQLabel() + "\n";
    strInertia += strange;

    strInertia += "   CoG: " + sep;
    strange = QString::asprintf("%13.5g", m_pPart->CoG_t().x * Units::mtoUnit());
    strInertia += strange + sep;
    strange = QString::asprintf("%13.5g", m_pPart->CoG_t().y * Units::mtoUnit());
    strInertia += strange + sep;
    strange = QString::asprintf("%13.5g", m_pPart->CoG_t().z * Units::mtoUnit());
    strInertia += strange + sep + Units::lengthUnitQLabel();
    strInertia += "\n";

    strInertia += "   Inertia in the part's CoG frame:\n";
    strange = QString::asprintf("      Ixx_cog= %13.5g", m_pPart->Ixx_t()*Units::kgm2toUnit());
    strInertia += strange + sep+ Units::inertiaUnitQLabel() + "\n";
    strange = QString::asprintf("      Iyy_cog= %13.5g", m_pPart->Iyy_t()*Units::kgm2toUnit());
    strInertia += strange + sep+ Units::inertiaUnitQLabel() + "\n";
    strange = QString::asprintf("      Izz_cog= %13.5g", m_pPart->Izz_t()*Units::kgm2toUnit());
    strInertia += strange + sep+ Units::inertiaUnitQLabel() + "\n";
    strange = QString::asprintf("      Ixz_cog= %13.5g", m_pPart->Ixz_t()*Units::kgm2toUnit());
    strInertia += strange + sep+ Units::inertiaUnitQLabel() + "\n";

    pClipBoard->setText(strInertia);

    m_pButtonBox->button(QDialogButtonBox::Save)->setFocus();
}


/**
* Exports the mass and inertia data to AVL format
*/
void PartInertiaDlg::onExportToAVL()
{
    if (!m_pPart) return;
    QString filter =".mass";

    QString FileName, strong;

    FileName = QString::fromStdString(m_pPart->name()).trimmed();

    FileName.replace('/', '_');
    FileName.replace(' ', '_');
    FileName += ".mass";
    FileName = QFileDialog::getSaveFileName(this, "Export Mass Properties",SaveOptions::lastDirName() + "/"+FileName,
                                            "AVL Mass File (*.mass)", &filter);
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastDirName(FileName.left(pos));

    pos = FileName.lastIndexOf(".");
    if(pos<0) FileName += ".mass";

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;
    QTextStream out(&XFile);
//    out.setCodec("UTF-8");

    double Lunit = 1./Units::mtoUnit();
    double Munit = 1./Units::kgtoUnit();
    double Iunit = 1./Units::kgm2toUnit();

    out << "#-------------------------------------------------\n";
    out << "#\n";
    out << "#   "+QString::fromStdString(m_pPart->name())+"\n";

    out << "#\n";
    out << "#  Dimensional unit and parameter data.\n";
    out << "#  Mass & Inertia breakdown.\n";
    out << "#-------------------------------------------------\n";
    out << "#  Names and scalings for units to be used for trim and eigenmode calculations.\n";
    out << "#  The Lunit and Munit values scale the mass, xyz, and inertia table data below.\n";
    out << "#  Lunit value will also scale all lengths and areas in the AVL input file.\n";
    out << "#\n";
    strong = QString("Lunit = %1 m").arg(Lunit,10,'f',4);
    out << strong+"\n";
    strong = QString("Munit = %1 kg").arg(Munit,10,'f',4);
    out << strong+"\n";
    out << "Tunit = 1.0 s\n";
    out << "#-------------------------\n";
    out << "#  Gravity and density to be used as default values in trim setup (saves runtime typing).\n";
    out << "#  Must be in the unit names given above (i.e. m,kg,s).\n";
    out << "g   = 9.81\n";
    out << "rho = 1.225\n";
    out << "#-------------------------\n";
    out << "#  Mass & Inertia breakdown.\n";
    out << "#  x y z  is location of item's own CG.\n";
    out << "#  Ixx... are item's inertias about item's own CG.\n";
    out << "#\n";
    out << "#  x,y,z system here must be exactly the same one used in the .avl input file\n";
    out << "#     (same orientation, same origin location, same length units)\n";
    out << "#\n";
    out << "#     mass          x          y          z        Ixx        Iyy        Izz        Ixy        Ixz        Iyz \n";

    // in accordance with AVL input format,
    // we need to convert the inertia in the wing's CoG system
    // by applying Huyghen/Steiner's theorem

    Inertia const &inert = m_pPart->inertia();
    strong = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10! Inertia of both left and right wings")
             .arg(inert.structuralMass() /Munit, 10, 'g', 3)
             .arg(inert.CoG_s().x/Lunit, 10, 'g', 3)
             .arg(inert.CoG_s().y/Lunit, 10, 'g', 3)  //should be zero
             .arg(inert.CoG_s().z/Lunit, 10, 'g', 3)
             .arg(inert.Ixx_s()/Iunit,  10, 'g', 3)
             .arg(inert.Iyy_s()/Iunit,  10, 'g', 3)
             .arg(inert.Izz_s()/Iunit,  10, 'g', 3)
             .arg(0.0,  10, 'g', 3)
             .arg(inert.Ixz_s()/Iunit,  10, 'g', 3).arg(0.0,  10, 'g', 3);
    out << strong+"\n";

    for (int i=0; i<inert.pointMassCount(); i++)
    {
        PointMass const &pm = inert.pointMassAt(i);
        if(pm.mass()>0.0)
        {
            strong = QString("%1 %2 %3 %4      0.000      0.000      0.000")
                     .arg(pm.mass() / Munit,    10, 'g', 3)
                     .arg(pm.position().x/Lunit, 10, 'g', 3)
                     .arg(pm.position().y/Lunit, 10, 'g', 3)
                     .arg(pm.position().z/Lunit, 10, 'g', 3);
            strong += " ! " + QString::fromStdString(pm.tag());
            out << strong+"\n";
        }
    }
    XFile.close();

    m_pButtonBox->button(QDialogButtonBox::Save)->setFocus();
}


void PartInertiaDlg::onOK()
{
    readData();
    accept();
}


void PartInertiaDlg::readData()
{
    m_pPart->setAutoInertia(m_pchAutoInertia->isChecked());
    m_ppmtMasses->readPointMassData();
}


void PartInertiaDlg::setupLayout()
{
    QString strMass, strLength;
    strMass = Units::massUnitQLabel();
    strLength = Units::lengthUnitQLabel();

    m_pHSplitter = new QSplitter;
    {
        m_pHSplitter->setChildrenCollapsible(false);
        QWidget *pLeftWidget = new QWidget;
        {
            QVBoxLayout * pLeftLayout = new QVBoxLayout;
            {
                m_plabPlaneName = new QLabel;
                m_plabPlaneName->setStyleSheet("font: bold");

                m_pVSplitter = new QSplitter(Qt::Vertical);
                {
                    m_pVSplitter->setChildrenCollapsible(false);
                    //___________Volume Mass, Center of gravity, and inertias__________
                    QFrame *pObjectMassFrame = new QFrame;
                    {
                        QVBoxLayout *pObjectMassLayout = new QVBoxLayout;
                        {
                            QLabel *plab0 = new QLabel("Object Mass - Structural only, excluding point masses");
                            plab0->setStyleSheet("QLabel { font-weight: bold;}");

                            QHBoxLayout *pMassLayout = new QHBoxLayout;
                            {
                                QLabel *plabMass = new QLabel("Structural mass");
                                plabMass->setAlignment(Qt::AlignRight);
                                m_pfeStructMass = new FloatEdit(0.,3);
                                QLabel *plabMassUnit1 = new QLabel(Units::massUnitQLabel());
                                m_pchAutoInertia = new QCheckBox("Auto estimation of inertia");
                                pMassLayout->addWidget(m_pchAutoInertia);
                                pMassLayout->addStretch();
                                pMassLayout->addWidget(plabMass);
                                pMassLayout->addWidget(m_pfeStructMass);
                                pMassLayout->addWidget(plabMassUnit1);
                            }

                            m_pStructInertiaTable = new CPTableView(this);
                            m_pStructInertiaTable->setEditable(true);
                            m_pStructInertiaModel = new PartInertiaModel(this);
                            m_pStructInertiaTable->setModel(m_pStructInertiaModel);

                            m_pPartInertiaDelegate = new PartInertiaDelegate(this);
                            QVector<int> precision= {-1,3,-1};
                            m_pPartInertiaDelegate->setPrecision(precision);
                            m_pStructInertiaTable->setItemDelegate(m_pPartInertiaDelegate);

                            pObjectMassLayout->addWidget(plab0);
                            pObjectMassLayout->addLayout(pMassLayout);
                            pObjectMassLayout->addWidget(m_pStructInertiaTable);
                        }
                        pObjectMassFrame->setLayout(pObjectMassLayout);
                    }

                    //___________________Point Masses__________________________
                    QFrame *pPointMassFrame = new QFrame;
                    {
                        //                        pPointMassFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                        QVBoxLayout *pPointMassLayout = new QVBoxLayout;
                        {
                            QLabel *pPointMassesLabel = new QLabel("Additional point masses");
                            pPointMassesLabel->setStyleSheet("QLabel { font-weight: bold;}");

                            m_ppmtMasses = new PointMassTable(this);

                            pPointMassLayout->addWidget(pPointMassesLabel);
                            pPointMassLayout->addWidget(m_ppmtMasses);
                        }
                        pPointMassFrame->setLayout(pPointMassLayout);
                    }

                    m_pVSplitter->addWidget(pObjectMassFrame);
                    m_pVSplitter->addWidget(pPointMassFrame);
                }
                //________________Total Mass, Center of gravity, and inertias__________
                QFrame *pTotalMassBox = new QFrame;
                {
                    QVBoxLayout *pTotalLayout = new QVBoxLayout;
                    {
                        QLabel *pTotalMassLabel = new QLabel("Total Inertia = Structural + point masses");
                        pTotalMassLabel->setStyleSheet("QLabel { font-weight: bold;}");
                        QHBoxLayout *pTotalMassLayout = new QHBoxLayout;
                        {
                            QGroupBox *pTotalCoGBox = new QGroupBox("Center of gravity");
                            {
                                QGridLayout *pTotalCoGLayout = new QGridLayout;
                                {
                                    QLabel *plabXTotal = new QLabel("X_CoG=");
                                    QLabel *plabYTotal = new QLabel("Y_CoG=");
                                    QLabel *plabZTotal = new QLabel("Z_CoG=");
                                    m_pfeXTotalCoG = new FloatEdit(0.0f,3);
                                    m_pfeYTotalCoG = new FloatEdit(0.0f,3);
                                    m_pfeZTotalCoG = new FloatEdit(0.0f,3);
                                    m_pfeXTotalCoG->setEnabled(false);
                                    m_pfeYTotalCoG->setEnabled(false);
                                    m_pfeZTotalCoG->setEnabled(false);
                                    QLabel *plabLengthUnit20 = new QLabel(Units::lengthUnitQLabel());
                                    QLabel *plabLengthUnit21 = new QLabel(Units::lengthUnitQLabel());
                                    QLabel *plabLengthUnit22 = new QLabel(Units::lengthUnitQLabel());
                                    QLabel *plabTotalMassLabel   = new QLabel("Total Mass=");
                                    QLabel *plabMassUnit2        = new QLabel(Units::massUnitQLabel());
                                    m_pfeTotalMass         = new FloatEdit(1.0f,3);
                                    m_pfeTotalMass->setEnabled(false);

                                    pTotalCoGLayout->addWidget(plabTotalMassLabel,  1,1);
                                    pTotalCoGLayout->addWidget(m_pfeTotalMass,      1,2);
                                    pTotalCoGLayout->addWidget(plabMassUnit2,       1,3);
                                    pTotalCoGLayout->addWidget(plabXTotal,          2,1, Qt::AlignRight);
                                    pTotalCoGLayout->addWidget(plabYTotal,          3,1, Qt::AlignRight);
                                    pTotalCoGLayout->addWidget(plabZTotal,          4,1, Qt::AlignRight);
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
                                    pTotalInertiaLayout->addWidget(plabTotIxx,        1,1, Qt::AlignRight);
                                    pTotalInertiaLayout->addWidget(plabTotIyy,        2,1, Qt::AlignRight);
                                    pTotalInertiaLayout->addWidget(plabTotIzz,        3,1, Qt::AlignRight);
                                    pTotalInertiaLayout->addWidget(plabTotIxz,        4,1, Qt::AlignRight);
                                    pTotalInertiaLayout->addWidget(m_pfeTotalIxx,     1,2);
                                    pTotalInertiaLayout->addWidget(m_pfeTotalIyy,     2,2);
                                    pTotalInertiaLayout->addWidget(m_pfeTotalIzz,     3,2);
                                    pTotalInertiaLayout->addWidget(m_pfeTotalIxz,     4,2);
                                    pTotalInertiaLayout->addWidget(plabInertiaUnit10, 1,3);
                                    pTotalInertiaLayout->addWidget(plabInertiaUnit20, 2,3);
                                    pTotalInertiaLayout->addWidget(plabInertiaUnit30, 3,3);
                                    pTotalInertiaLayout->addWidget(plabInertiaUnit40, 4,3);
                                    pTotalInertiaLayout->setColumnStretch(1,1);
                                    pTotalInertiaLayout->setColumnStretch(2,2);
                                    pTotalInertiaLayout->setColumnStretch(3,1);
                                }
                                pTotalInertiaBox->setLayout(pTotalInertiaLayout);
                            }
                            pTotalMassLayout->addWidget(pTotalCoGBox);
                            pTotalMassLayout->addWidget(pTotalInertiaBox);
                        }
                        pTotalLayout->addWidget(pTotalMassLabel);
                        pTotalLayout->addLayout(pTotalMassLayout);
                    }
                    pTotalMassBox->setLayout(pTotalLayout);
                }

                //__________________Control buttons___________________
                m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
                {
                    m_ppbExportToAVL = new QPushButton("to AVL file");
                    m_pButtonBox->addButton(m_ppbExportToAVL, QDialogButtonBox::ActionRole);

                    m_ppbExportToClipboard = new QPushButton("to clipboard");
                    m_pButtonBox->addButton(m_ppbExportToClipboard, QDialogButtonBox::ActionRole);
                }

                pLeftLayout->addWidget(m_plabPlaneName);
                pLeftLayout->addWidget(m_pVSplitter);
                pLeftLayout->addWidget(pTotalMassBox);
                pLeftLayout->addWidget(m_pButtonBox);
            }
            pLeftWidget->setLayout(pLeftLayout);
        }

        QWidget *pRightWidget = new QWidget;
        {
            QVBoxLayout *pRightLayout = new QVBoxLayout;
            {
                m_p3dViewStack = new QStackedWidget;

                m_pgl3dWingView = new gl3dWingView;
                m_pgl3dWingView->showPartFrame(false);

                m_pgl3dFuseView = new gl3dFuseView;
                m_pgl3dFuseView->showPartFrame(false);

                m_p3dViewStack->addWidget(m_pgl3dWingView);
                m_p3dViewStack->addWidget(m_pgl3dFuseView);

                // not quite achieved polymorphism yet
                if     (m_pPart->isWing())  m_pgl3dControls = new gl3dGeomControls(m_pgl3dWingView, InertiaLayout, true);
                else if(m_pPart->isFuse())  m_pgl3dControls = new gl3dGeomControls(m_pgl3dFuseView, InertiaLayout, true);

                if(m_pgl3dControls) m_pgl3dControls->showMasses(true);

                pRightLayout->addWidget(m_p3dViewStack);
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


void PartInertiaDlg::connectSignals()
{
    connect(m_pButtonBox,          SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));

    connect(m_pHSplitter,          SIGNAL(splitterMoved(int,int)),    SLOT(onResizeColumns()));
    connect(m_pVSplitter,          SIGNAL(splitterMoved(int,int)),    SLOT(onResizeColumns()));

    connect(m_pfeStructMass,       SIGNAL(editingFinished()),         SLOT(onAutoInertia()));

    connect(m_pchAutoInertia,      SIGNAL(clicked(bool)),             SLOT(onAutoInertia()));
    connect(m_ppmtMasses,          SIGNAL(pointMassChanged()),        SLOT(onPointMassCellChanged()));
    connect(m_ppmtMasses,          SIGNAL(dataPasted()),              SLOT(onPointMassDataPasted()));
    connect(m_ppmtMasses->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onMassRowChanged(QModelIndex)));


    connect(m_pStructInertiaTable, SIGNAL(dataPasted()),              SLOT(onStructInertiaCellChanged()));
    connect(m_pStructInertiaModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(onStructInertiaCellChanged()));
}


void PartInertiaDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)     onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
    else if (pButton == m_ppbExportToAVL)                                 onExportToAVL();
    else if (pButton == m_ppbExportToClipboard)                           onCopyInertiaToClipboard();
}


void PartInertiaDlg::onResizeColumns()
{
    int n = 2; //unit column
    QHeaderView *pHHeader = m_pStructInertiaTable->horizontalHeader();
    pHHeader->setSectionResizeMode(n, QHeaderView::Stretch);
    pHHeader->resizeSection(n, 1);

    double w = double(m_pStructInertiaTable->width())/100.0;
    //    int wcol = (int)(35.0*w);

    m_pStructInertiaTable->setColumnWidth(0, int(30.0*w));
    m_pStructInertiaTable->setColumnWidth(1, int(40.0*w));

    m_ppmtMasses->resizeColumns();
}


void PartInertiaDlg::resizeEvent(QResizeEvent *pEvent)
{
    QDialog::resizeEvent(pEvent);
    onResizeColumns();
}


void PartInertiaDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);

    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    if(s_VSplitterSizes.length()>0) m_pVSplitter->restoreState(s_VSplitterSizes);

    onResizeColumns();

    if     (m_pPart->isWing()) m_pgl3dWingView->reset3dScale();
    else if(m_pPart->isFuse()) m_pgl3dFuseView->reset3dScale();

    m_pgl3dFuseView->restoreViewPoint(s_ab_quat);
    m_pgl3dWingView->restoreViewPoint(s_ab_quat);
}


void PartInertiaDlg::hideEvent(QHideEvent*pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();

    s_HSplitterSizes  = m_pHSplitter->saveState();
    s_VSplitterSizes  = m_pVSplitter->saveState();

    m_pgl3dFuseView->saveViewPoint(s_ab_quat);
    m_pgl3dWingView->saveViewPoint(s_ab_quat);
}


void PartInertiaDlg::onRedraw()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if     (m_pPart->isWing()) m_pgl3dWingView->update();
    else if(m_pPart->isFuse()) m_pgl3dFuseView->update();

    QApplication::restoreOverrideCursor();
}


void PartInertiaDlg::onMassRowChanged(QModelIndex index)
{
    int row = index.row();
    if(m_pPart->isWing())
    {
        m_pgl3dWingView->setSelectedPartMass(row);
        m_pgl3dWingView->update();
    }
    else if(m_pPart->isFuse())
    {
        m_pgl3dFuseView->setSelectedPartMass(row);
        m_pgl3dFuseView->update();
    }
}


void PartInertiaDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("PartInertiaDlg");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();
        s_HSplitterSizes = settings.value("HSplitterSize").toByteArray();
        s_VSplitterSizes = settings.value("VSplitterSize").toByteArray();
    }
    settings.endGroup();
}


void PartInertiaDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("PartInertiaDlg");
    {
        settings.setValue("WindowGeometry", s_Geometry);
        settings.setValue("HSplitterSize", s_HSplitterSizes);
        settings.setValue("VSplitterSize", s_VSplitterSizes);
    }
    settings.endGroup();
}



