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
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>


#include "planexflinertiadlg.h"

#include <interfaces/editors/inertia/partinertiadelegate.h>
#include <interfaces/editors/inertia/partinertiadlg.h>
#include <interfaces/editors/inertia/partinertiamodel.h>
#include <interfaces/editors/inertia/planeinertiamodel.h>
#include <interfaces/editors/inertia/pointmasstable.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dplanexflview.h>
#include <core/saveoptions.h>
#include <api/units.h>
#include <api/objects3d.h>
#include <api/planexfl.h>
#include <interfaces/widgets/customdlg/stringvaluedlg.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/xfldelegate.h>

PlaneXflInertiaDlg::PlaneXflInertiaDlg(QWidget *pParent) : PlaneInertiaDlg(pParent)
{
    m_pPlaneXfl = nullptr;
    setupLayout();
    connectSignals();
}


void PlaneXflInertiaDlg::setupLayout()
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
                    m_prbInertiaFromParts = new QRadioButton(tr("Auto"));
                    m_prbInertiaManual = new QRadioButton(tr("Manual"));
                    m_prbInertiaFromParts->setText(tr("From parts"));
                    pInputSourceLayout->addWidget(m_prbInertiaFromParts);
                    pInputSourceLayout->addWidget(m_prbInertiaManual);
                    pInputSourceLayout->addStretch();
                }
                m_pVSplitter = new QSplitter(Qt::Vertical);
                {
                    m_pVSplitter->setChildrenCollapsible(false);
                    m_pswInputSource = new QStackedWidget;
                    {
                        //___________Volume Mass, Center of gravity, and inertias__________
                        m_pcptPart = new CPTableView(this);
                        m_pcptPart->setEditable(false);
                        m_pcptPart->setWindowTitle(tr("Wing List"));
                        m_pcptPart->setWordWrap(false);
                        m_pPartTableDelegate = new XflDelegate(this);
                        m_pPartTableDelegate->setActionColumn(5);
                        m_pPartTableDelegate->setDigits({-1,3,3,3,3,-1});
                        m_pPartTableDelegate->setItemTypes({XflDelegate::STRING, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::ACTION});
                        m_pcptPart->setItemDelegate(m_pPartTableDelegate);

                        m_pswInputSource->addWidget(m_pcptPart);
                        m_pswInputSource->addWidget(m_pInertiaManTable);
                    }

                    m_pVSplitter->addWidget(m_pswInputSource);
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
                m_pgl3dPlaneView = new gl3dPlaneXflView;

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


void PlaneXflInertiaDlg::initDialog(Plane *pPlane)
{
    m_pPlane = pPlane;
    m_pPlaneXfl = dynamic_cast<PlaneXfl*>(pPlane);
    gl3dPlaneXflView *pgl3dPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pgl3dPlaneView);
    pgl3dPlaneXflView->setPlane(m_pPlaneXfl);

    m_pPartInertiaModel = new PlaneInertiaModel(m_pPlaneXfl, nullptr);
    connect(m_pPartInertiaModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(onDataChanged(QModelIndex,QModelIndex)));
    m_pcptPart->setModel(m_pPartInertiaModel);

    if(m_pPlane->bAutoInertia())
    {
        m_prbInertiaFromParts->setChecked(true);
        m_pswInputSource->setCurrentIndex(0);
    }
    else
    {
        m_prbInertiaManual->setChecked(true);
        m_pswInputSource->setCurrentIndex(1);
    }

    PlaneInertiaDlg::initDialog(pPlane);
}


void PlaneXflInertiaDlg::connectSignals()
{
    connectBaseSignals();

    connect(m_pcptPart,            SIGNAL(clicked(QModelIndex)),       SLOT(onItemClicked(QModelIndex)));
    connect(m_pcptPart,            SIGNAL(doubleClicked(QModelIndex)), SLOT(onItemDoubleClicked(QModelIndex)));
    connect(m_prbInertiaManual,    SIGNAL(clicked()),                  SLOT(onInertiaSource()));
    connect(m_prbInertiaFromParts, SIGNAL(clicked()),                  SLOT(onInertiaSource()));
}


void PlaneXflInertiaDlg::onInertiaSource()
{
    m_pPlane->setAutoInertia(m_prbInertiaFromParts->isChecked());
    if(m_pPlane->bAutoInertia())
    {
        m_pswInputSource->setCurrentIndex(0);
        m_pPlane->computeStructuralInertia();
    }
    else
    {
        m_pswInputSource->setCurrentIndex(1);
    }

    m_pInertiaManTable->setEnabled(!m_pPlane->bAutoInertia());
    m_pStructInertiaModel->setInertia(&m_pPlane->inertia());
    m_pStructInertiaModel->setEditable(!m_pPlane->bAutoInertia());
    onResizeColumns();
}


void PlaneXflInertiaDlg::onDataChanged(QModelIndex topleft, QModelIndex)
{
    if(topleft.column()==0)
    {
        m_pcptPart->setFocus();
    }
    onRedraw();
}


void PlaneXflInertiaDlg::onResizeColumns()
{
    // resize part table columns
    int np = 5; //action column
    QHeaderView *pHHeader = m_pcptPart->horizontalHeader();
    pHHeader->setSectionResizeMode(np, QHeaderView::Stretch);
    pHHeader->resizeSection(np, 1);

    double wp = double(m_pcptPart->width())/100.0;
    int wname  = int(19.0*wp);
    int wnum   = int(13.0*wp);
    m_pcptPart->setColumnWidth(0, wname);
    m_pcptPart->setColumnWidth(1, wnum);
    m_pcptPart->setColumnWidth(2, wnum);
    m_pcptPart->setColumnWidth(3, wnum);
    m_pcptPart->setColumnWidth(4, wnum);

    PlaneInertiaDlg::onResizeColumns();
}


void PlaneXflInertiaDlg::onItemClicked(QModelIndex index)
{
    switch(index.column())
    {
        case 5:
        {
            if(index.row()<m_pPlaneXfl->nWings())       editWingInertia(index.row());
            else if(index.row()==m_pPlaneXfl->nWings()) editFuseInertia();
            onRedraw();
            break;
        }
        default:
            break;
    }
}


void PlaneXflInertiaDlg::onItemDoubleClicked(QModelIndex index)
{
    if(index.row()<m_pPlaneXfl->nWings())       editWingInertia(index.row());
    else if(index.row()==m_pPlaneXfl->nWings()) editFuseInertia();
    onRedraw();
}


void PlaneXflInertiaDlg::onImportExisting()
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


    if(pRefPlane->isXflType() && m_pPlane->bAutoInertia())
    {
        m_pPlaneXfl->setAutoInertia(true);
        m_prbInertiaFromParts->setChecked(true);
        m_prbInertiaManual->setChecked(false);
        for(int iw=0; iw<m_pPlane->nWings(); iw++)
        {
            WingXfl *pWing = m_pPlaneXfl->wing(iw);
            if(iw<pRefPlane->nWings())
                pWing->setInertia(pRefPlane->wingAt(iw)->inertia());
        }

        for(int ib=0; ib<m_pPlaneXfl->nFuse(); ib++)
        {
            Fuse *pFuse = m_pPlaneXfl->fuse(ib);
            if(ib<pRefPlane->nFuse())
                pFuse->setInertia(pRefPlane->fuseAt(ib)->inertia());
        }

        m_pswInputSource->setCurrentIndex(0);
        m_pPlane->computeStructuralInertia();

        m_pPartInertiaModel->updateData();

        m_pcptPart->setEnabled(true);
        m_pInertiaManTable->setEnabled(false);
    }
    else
    {
        m_pPlaneXfl->setAutoInertia(false);
        m_prbInertiaFromParts->setChecked(false);
        m_prbInertiaManual->setChecked(true);
        m_pStructInertiaModel->updateData();
        m_pInertiaManTable->setEnabled(true);
        m_pInertiaManTable->update();
        m_pswInputSource->setCurrentIndex(1);
    }

    updateTotalInertia();

    m_bChanged = true;
}


void PlaneXflInertiaDlg::onCopyInertiaToClipboard()
{
    if(!m_pPlane) return;

    QClipboard *pClipBoard = QApplication::clipboard();

    QString sep = SaveOptions::textSeparator() + " ";
    QString strInertia, strange;

    strInertia = "Inertia properties for "+QString::fromStdString(m_pPlaneXfl->name())+"\n\n";
    strange = QString::asprintf("Total mass = %11.5g ", m_pPlaneXfl->totalMass()*Units::kgtoUnit());
    strange += Units::massUnitQLabel() + "\n";

    strInertia += "Part inertias:\n";
    strInertia += "    Mass (" + Units::massUnitQLabel()   + ")" + sep;
    strInertia += "         x (" + Units::lengthUnitQLabel() + ")" + sep;
    strInertia += "         y (" + Units::lengthUnitQLabel() + ")" + sep;
    strInertia += "         z (" + Units::lengthUnitQLabel() + ")" + sep;
    strInertia += "   Ixx (" + Units::inertiaUnitQLabel() + ")" + sep;
    strInertia += "   Iyy (" + Units::inertiaUnitQLabel() + ")" + sep;
    strInertia += "   Izz (" + Units::inertiaUnitQLabel() + ")" + sep;
    strInertia += "   Ixz (" + Units::inertiaUnitQLabel() + ")" + sep;
    strInertia += "\n";

    for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
    {
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->wing(iw)->totalMass()*Units::kgtoUnit());
        strInertia += strange + sep;
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->wing(iw)->CoG_t().x*Units::mtoUnit());
        strInertia += strange + sep;
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->wing(iw)->CoG_t().y*Units::mtoUnit());
        strInertia += strange + sep;
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->wing(iw)->CoG_t().z*Units::mtoUnit());
        strInertia += strange + sep ;
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->wing(iw)->Ixx_t()*Units::kgm2toUnit());
        strInertia += strange + sep ;
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->wing(iw)->Iyy_t()*Units::kgm2toUnit());
        strInertia += strange + sep ;
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->wing(iw)->Izz_t()*Units::kgm2toUnit());
        strInertia += strange + sep ;
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->wing(iw)->Ixz_t()*Units::kgm2toUnit());
        strInertia += strange + sep ;
        strange = QString::asprintf("  %s", m_pPlaneXfl->wing(iw)->name().c_str());

        strInertia += strange+"\n";
    }

    for(int ifuse=0; ifuse<m_pPlaneXfl->nFuse(); ifuse++)
    {
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->fuse(ifuse)->totalMass()*Units::kgtoUnit());
        strInertia += strange + sep;
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->fuse(ifuse)->CoG_t().x*Units::mtoUnit());
        strInertia += strange + sep;
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->fuse(ifuse)->CoG_t().y*Units::mtoUnit());
        strInertia += strange + sep;
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->fuse(ifuse)->CoG_t().z*Units::mtoUnit());
        strInertia += strange + sep ;
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->fuse(ifuse)->Ixx_t()*Units::kgm2toUnit());
        strInertia += strange + sep ;
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->fuse(ifuse)->Iyy_t()*Units::kgm2toUnit());
        strInertia += strange + sep ;
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->fuse(ifuse)->Izz_t()*Units::kgm2toUnit());
        strInertia += strange + sep ;
        strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->fuse(ifuse)->Ixz_t()*Units::kgm2toUnit());
        strInertia += strange + sep ;
        strange = QString::asprintf("  %s", m_pPlaneXfl->fuse(ifuse)->name().c_str());

        strInertia += strange+"\n";
    }

    strInertia += "\n";

    if(m_pPlaneXfl->pointMassCount()>0)
    {
        strInertia += "Additional point masses:\n";
        strInertia += "     Mass (" + Units::massUnitQLabel() + ")" + sep;
        strInertia += "        x ("    + Units::lengthUnitQLabel() + ")" + sep;
        strInertia += "        y ("    + Units::lengthUnitQLabel() + ")" + sep;
        strInertia += "        z ("    + Units::lengthUnitQLabel() + ")" + sep;
        strInertia += "\n";
        for(int im=0; im<m_pPlaneXfl->pointMassCount(); im++)
        {
            strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->pointMassAt(im).mass()*Units::kgtoUnit());
            strInertia += strange + sep;
            strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->pointMassAt(im).position().x*Units::mtoUnit());
            strInertia += strange + sep;
            strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->pointMassAt(im).position().y*Units::mtoUnit());
            strInertia += strange + sep;
            strange = QString::asprintf("  %11.5g ", m_pPlaneXfl->pointMassAt(im).position().z*Units::mtoUnit());
            strInertia += strange + sep ;
            strange = QString::asprintf("  %s", m_pPlaneXfl->pointMassAt(im).tag().c_str());
            strInertia += strange + "\n";
        }
        strInertia += "\n";
    }

    strInertia += "Total:\n";

    strange = QString::asprintf("   Mass = %11.5g ", m_pPlaneXfl->totalMass()*Units::kgtoUnit());
    strange += Units::massUnitQLabel() + "\n";
    strInertia += strange;

    strInertia += "   CoG: " + sep;
    strange = QString::asprintf("%13.5g", m_pPlaneXfl->CoG_t().x * Units::mtoUnit());
    strInertia += strange + sep;
    strange = QString::asprintf("%13.5g", m_pPlaneXfl->CoG_t().y * Units::mtoUnit());
    strInertia += strange + sep;
    strange = QString::asprintf("%13.5g", m_pPlaneXfl->CoG_t().z * Units::mtoUnit());
    strInertia += strange + sep + Units::lengthUnitQLabel();
    strInertia += "\n";

    strInertia += "   Inertia in the part's CoG frame:\n";
    strange = QString::asprintf("      Ixx_cog= %13.5g", m_pPlaneXfl->Ixx_t()*Units::kgm2toUnit());
    strInertia += strange + sep+ Units::inertiaUnitQLabel() + "\n";
    strange = QString::asprintf("      Iyy_cog= %13.5g", m_pPlaneXfl->Iyy_t()*Units::kgm2toUnit());
    strInertia += strange + sep+ Units::inertiaUnitQLabel() + "\n";
    strange = QString::asprintf("      Izz_cog= %13.5g", m_pPlaneXfl->Izz_t()*Units::kgm2toUnit());
    strInertia += strange + sep+ Units::inertiaUnitQLabel() + "\n";
    strange = QString::asprintf("      Ixz_cog= %13.5g", m_pPlaneXfl->Ixz_t()*Units::kgm2toUnit());
    strInertia += strange + sep+ Units::inertiaUnitQLabel() + "\n";

    pClipBoard->setText(strInertia);

    m_pButtonBox->button(QDialogButtonBox::Save)->setFocus();
}



/**
* Exports the mass and inertia data to AVL format
*/
void PlaneXflInertiaDlg::onExportToAVL()
{
    QString filter =".mass";

    QString FileName, strong;

    Vector3d CoG;

    FileName = QString::fromStdString(m_pPlaneXfl->name());
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
    out << "#   "+QString::fromStdString(m_pPlaneXfl->name())+"\n";
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

    // we write out each object contribution individually
    double CoGIxx=0, CoGIyy=0, CoGIzz=0, CoGIxz = 0.0;
    for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
    {
        WingXfl *pWing = m_pPlaneXfl->wing(iw);
        if(!pWing) continue;

        if(pWing->bAutoInertia()) pWing->computeStructuralInertia(m_pPlaneXfl->wingLE(iw));

        // Apply Huyghens/Steiner theorem
        Vector3d const &T = m_pPlaneXfl->wingLE(iw);
        double d2 = T.x*T.x+T.y*T.y+T.z*T.z;
        CoG += T;
        CoGIxx += pWing->structuralMass() *(d2-T.x*T.x);
        CoGIyy += pWing->structuralMass() *(d2-T.y*T.y);
        CoGIzz += pWing->structuralMass() *(d2-T.z*T.z);
        CoGIxz += pWing->structuralMass() *(  -T.x*T.z);

        strong = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 ! ")
                 .arg(pWing->inertia().structuralMass() /Munit, 10, 'g', 3)
                 .arg((T.x+pWing->CoG_s().x)/Lunit, 10, 'g', 3)
                 .arg((T.y+pWing->CoG_s().y)/Lunit, 10, 'g', 3)
                 .arg((T.z+pWing->CoG_s().z)/Lunit, 10, 'g', 3)
                 .arg(pWing->Ixx_s()/Iunit,10, 'g', 3)
                 .arg(pWing->Iyy_s()/Iunit,10, 'g', 3)
                 .arg(pWing->Izz_s()/Iunit,10, 'g', 3)
                 .arg(0.0,10, 'g', 3)
                 .arg(pWing->Ixz_s()/Iunit,10, 'g', 3)
                 .arg(0.0,10, 'g', 3);
        strong += QString::fromStdString(pWing->name());
        out << strong+"\n";

    }
    (void)CoGIxx;
    (void)CoGIyy;
    (void)CoGIzz;
    (void)CoGIxz;

    if(m_pPlaneXfl->hasFuse())
    {
        Fuse *pFuse = m_pPlaneXfl->fuse(0);
        Vector3d const &T = m_pPlaneXfl->fusePos(0);
        pFuse->computeStructuralInertia(Vector3d());
        strong = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 ! Body's inertia")
                 .arg(m_pPlaneXfl->fuse(0)->structuralMass() /Munit, 10, 'g', 3)
                 .arg((T.x+pFuse->CoG_s().x)/Lunit, 10, 'g', 3)
                 .arg((T.y+pFuse->CoG_s().y)/Lunit, 10, 'g', 3)
                 .arg((T.z+pFuse->CoG_s().z)/Lunit, 10, 'g', 3)
                 .arg(pFuse->Ixx_s()/Iunit,10, 'g', 3)
                 .arg(pFuse->Iyy_s()/Iunit,10, 'g', 3)
                 .arg(pFuse->Izz_s()/Iunit,10, 'g', 3)
                 .arg(0.0,10, 'g', 3)
                 .arg(pFuse->Ixz_s() /Iunit,10, 'g', 3)
                 .arg(0.0,10, 'g', 3);
        out << strong+"\n";
    }

    for (int i=0; i<m_pPlaneXfl->pointMassCount(); i++)
    {
        if(m_pPlaneXfl->pointMassAt(i).mass()>0.0)
        {
            strong = QString("%1 %2 %3 %4      0.000      0.000      0.000")
                     .arg(m_pPlaneXfl->pointMassAt(i).mass() / Munit,    10, 'g', 3)
                     .arg(m_pPlaneXfl->pointMassAt(i).position().x/Lunit, 10, 'g', 3)
                     .arg(m_pPlaneXfl->pointMassAt(i).position().y/Lunit, 10, 'g', 3)
                     .arg(m_pPlaneXfl->pointMassAt(i).position().z/Lunit, 10, 'g', 3);
            strong += " ! " + QString::fromStdString(m_pPlaneXfl->pointMassAt(i).tag());
            out << strong+"\n";
        }
    }


    // need to write the point masses for the objects
    //Main Wing
    for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
    {
        WingXfl *pWing = m_pPlaneXfl->wing(iw);
        Vector3d const &T = m_pPlaneXfl->wingLE(iw);
        if(pWing)
        {
            for (int i=0; i<pWing->pointMassCount(); i++)
            {
                if(m_pPlaneXfl->wing(0)->pointMassAt(i).mass()>0.0)
                {
                    strong = QString("%1 %2 %3 %4      0.000      0.000      0.000")
                             .arg(pWing->pointMassAt(i).mass() / Munit,    10, 'g', 3)
                             .arg((pWing->pointMassAt(i).position().x+T.x)/Lunit, 10, 'g', 3)
                             .arg((pWing->pointMassAt(i).position().y+T.y)/Lunit, 10, 'g', 3)
                             .arg((pWing->pointMassAt(i).position().z+T.z)/Lunit, 10, 'g', 3);

                    strong += " ! " + QString::fromStdString(pWing->pointMassAt(i).tag());
                    out << strong+"\n";
                }
            }
        }
    }

    if(m_pPlaneXfl->hasFuse())
    {
        //fuse
        Fuse const *pFuse = m_pPlaneXfl->fuseAt(0);
        Vector3d const &T = m_pPlaneXfl->fusePos(0);
        for (int i=0; i<pFuse->pointMassCount(); i++)
        {
            if(pFuse->pointMassAt(i).mass()>0.0)
            {
                strong = QString("%1 %2 %3 %4      0.000      0.000      0.000")
                         .arg(pFuse->pointMassAt(i).mass() / Munit,    10, 'g', 3)
                         .arg((pFuse->pointMassAt(i).position().x+T.x)/Lunit, 10, 'g', 3)
                         .arg((pFuse->pointMassAt(i).position().y+T.y)/Lunit, 10, 'g', 3)
                         .arg((pFuse->pointMassAt(i).position().z+T.z)/Lunit, 10, 'g', 3);
                strong += " ! " + QString::fromStdString(pFuse->pointMassAt(i).tag());
                out << strong+"\n";
            }
        }
    }


    XFile.close();

    m_pButtonBox->button(QDialogButtonBox::Save)->setFocus();
}


void PlaneXflInertiaDlg::editWingInertia(int iWing)
{
    if(iWing>=m_pPlaneXfl->nWings() || !m_pPlaneXfl->wing(iWing)) return;

    WingXfl workwing;
    workwing.duplicate(m_pPlaneXfl->wing(iWing));
    workwing.createSurfaces(Vector3d(), m_pPlaneXfl->rxAngle(iWing), m_pPlaneXfl->ryAngle(iWing));

    //update the inertia
    workwing.computeStructuralInertia(m_pPlaneXfl->wingLE(iWing)); //wing surfaces are defined in the plane's body axis

    PartInertiaDlg dlg(&workwing, nullptr, this);
    if(dlg.exec()!=QDialog::Accepted)
    {
        return;
    }
    m_pPlaneXfl->wing(iWing)->duplicate(workwing);
    updateTotalInertia();
    m_pgl3dPlaneView->update();
}


void PlaneXflInertiaDlg::editFuseInertia()
{
    if(!m_pPlaneXfl->fuse(0) || !m_pPlaneXfl->fuse(0)->isFuse()) return;
    Fuse *pFuse = m_pPlaneXfl->fuse(0);

    // update the inertia
    if(pFuse->bAutoInertia()) pFuse->computeStructuralInertia(Vector3d());

    // save the inertia properties
    bool bAutoInertia = pFuse->bAutoInertia();
    Inertia meminertia = pFuse->inertia();

    PartInertiaDlg dlg(nullptr, pFuse, this);

    if(dlg.exec()==QDialog::Accepted)
    {
        m_bChanged=true;
        updateTotalInertia();
    }
    else
    {
        // restore
        pFuse->setAutoInertia(bAutoInertia);
        pFuse->setInertia(meminertia);
    }
}


void PlaneXflInertiaDlg::onOK(int iExitCode)
{
    m_ppmtMasses->readPointMassData();

    for(int imass=0; imass<m_pPlaneXfl->pointMassCount(); imass++)
    {
       PointMass const &pm = m_pPlaneXfl->pointMassAt(imass);
        for(int ipart=0; ipart<m_pPlaneXfl->nParts(); ipart++)
        {
            Part const *pPart = m_pPlaneXfl->partAt(ipart);
            {
                for(int im=0; im<pPart->pointMassCount(); im++)
                {
                    if(pm.isSame(pPart->pointMassAt(im), pPart->position()))
                    {
                        QString strange = QString::asprintf("Possible duplicate of point mass %d in part ", imass+1) + QString::fromStdString(pPart->name());
                        strange += "\nContinue?";
                        int Ans = QMessageBox::question(this, "Question", strange,
                                                        QMessageBox::Yes | QMessageBox::No);
                        if (Ans != QMessageBox::Yes)
                            return;
                    }
                }
            }
        }
    }

    double total = 0.0;
    if(m_pPlaneXfl->bAutoInertia())
    {
        for(int ipart=0; ipart<m_pPlaneXfl->nParts(); ipart++)
        {
            Part const *pPart = m_pPlaneXfl->partAt(ipart);
            total += pPart->totalMass();
        }
    }
    else
        total = m_pPlaneXfl->inertia().structuralMass();

    for(int imass=0; imass<m_pPlaneXfl->pointMassCount(); imass++)
    {
       PointMass const &pm = m_pPlaneXfl->pointMassAt(imass);
       total += pm.mass();
    }
   Q_ASSERT(fabs(m_pPlaneXfl->totalMass()- total)<0.001);

    done(iExitCode);
}



