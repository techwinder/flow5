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
#include <QHBoxLayout>
#include <QColorDialog>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>

#include "fusedlg.h"


#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/editors/inertia/partinertiadlg.h>
#include <interfaces/editors/rotatedlg.h>
#include <interfaces/editors/scaledlg.h>
#include <interfaces/editors/translatedlg.h>
#include <interfaces/exchange/cadexportdlg.h>
#include <interfaces/exchange/stlwriterdlg.h>
#include <interfaces/mesh/tesscontrolsdlg.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/widgets/color/colorbtn.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <modules/xobjects.h>

#include <api/fuse.h>
#include <api/part.h>
#include <api/objects_global.h>
#include <api/occ_globals.h>
#include <api/units.h>
#include <interfaces/mesh/gmesh_globals.h>

bool FuseDlg::s_bOutline    = true;
bool FuseDlg::s_bSurfaces   = true;
bool FuseDlg::s_bVLMPanels  = false;
bool FuseDlg::s_bAxes       = true;
bool FuseDlg::s_bShowMasses = false;

Quaternion FuseDlg::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);

QVector<Vector3d> FuseDlg::s_DebugPts;

FuseDlg::FuseDlg(QWidget *pParent) : XflDialog(pParent)
{
    setWindowFlag(Qt::WindowMinMaxButtonsHint);

    m_bChanged = m_bDescriptionChanged = false;
    m_bEnableName = true;
    createBaseActions();
    makeWidgets();
}


void FuseDlg::createBaseActions()
{
    m_pExportMeshToSTL      = new QAction("Export mesh to an STL File", this);
    m_pExportTrianglesToSTL = new QAction("Export triangulation to an STL File", this);
    m_pExportToCADFile      = new QAction("Export body geometry to a CAD file", this);
    m_pFuseInertia          = new QAction("Inertia", this);
    m_pFuseInertia->setShortcut(Qt::Key_F12);
    m_pTessSettings         = new QAction("Tessellation", this);
    m_pTessSettings->setShortcut(QKeySequence(Qt::ALT | Qt::Key_T));

    m_pBackImageLoad     = new QAction("Load",     this);
    m_pBackImageClear    = new QAction("Clear",    this);
    m_pBackImageSettings = new QAction("Settings", this);
}


void FuseDlg::connectBaseSignals()
{
    connect(m_pcbColor,              SIGNAL(clicked()),         SLOT(onFuseColor()));
    connect(m_pleName,               SIGNAL(editingFinished()), SLOT(onMetaDataChanged()));
    connect(m_pteDescription,        SIGNAL(textChanged()),     SLOT(onMetaDataChanged()));
    connect(m_pTessSettings,         SIGNAL(triggered()),       SLOT(onTessellation()));

    connect(m_pExportMeshToSTL,      SIGNAL(triggered()),       SLOT(onExportMeshToSTLFile()));
    connect(m_pExportTrianglesToSTL, SIGNAL(triggered()),       SLOT(onExportTrianglesToSTLFile()));
    connect(m_pExportToCADFile,      SIGNAL(triggered()),       SLOT(onExportFuseToCADFile()));
    connect(m_pFuseInertia,          SIGNAL(triggered()),       SLOT(onFuseInertia()));

    connect(m_pTranslate,            SIGNAL(triggered()),       SLOT(onTranslate()));
    connect(m_pScale,                SIGNAL(triggered()),       SLOT(onScale()));
    connect(m_pRotate,               SIGNAL(triggered()),       SLOT(onRotate()));

    connect(m_pglControls->m_ptbDistance, SIGNAL(clicked()), SLOT(onNodeDistance()));
    connect(m_pglFuseView,   SIGNAL(pickedNodePair(QPair<int,int>)), SLOT(onPickedNodePair(QPair<int,int>)));

    connect(m_pBackImageLoad,        SIGNAL(triggered()), m_pglFuseView, SLOT(onLoadBackImage()));
    connect(m_pBackImageClear,       SIGNAL(triggered()), m_pglFuseView, SLOT(onClearBackImage()));
    connect(m_pBackImageSettings,    SIGNAL(triggered()), m_pglFuseView, SLOT(onBackImageSettings()));
}


void FuseDlg::onNodeDistance()
{
    m_pglFuseView->setHighlighting(false);
    m_pglControls->setHighlighting(false);
    m_pglFuseView->setPicking(m_pglControls->getDistance() ? xfl::MESHNODE : xfl::NOPICK);
    if(!m_pglControls->getDistance()) m_pglFuseView->clearMeasure();
    m_pglFuseView->setSurfacePick(xfl::NOSURFACE);
    m_pglFuseView->update();
}


void FuseDlg::onPickedNodePair(QPair<int, int> nodepair)
{
    if(nodepair.first <0 || nodepair.first >=int(m_pFuse->nodes().size())) return;
    if(nodepair.second<0 || nodepair.second>=int(m_pFuse->nodes().size())) return;
    Node nsrc  = m_pFuse->nodes().at(nodepair.first);
    Node ndest = m_pFuse->nodes().at(nodepair.second);

    Segment3d seg(nsrc, ndest);
    m_pglFuseView->setMeasure(seg);

    m_pglFuseView->resetPickedNodes();
    m_pglFuseView->update();
}


void FuseDlg::initDialog(Fuse *pFuse)
{
    m_pFuse = pFuse;
    m_pglControls->disableFoilCtrl();

    m_pleName->setText(QString::fromStdString(pFuse->name()));
    m_pcbColor->setColor(xfl::fromfl5Clr(pFuse->color()));
    m_pteDescription->setText(QString::fromStdString(pFuse->description()));
}


void FuseDlg::showEvent(QShowEvent *pEvent)
{
    XflDialog::showEvent(pEvent);
    m_pglFuseView->setFlags(s_bOutline, s_bSurfaces, s_bVLMPanels, s_bAxes, s_bShowMasses, false, false, false, false);
    m_pglControls->setControls();
    m_pglFuseView->restoreViewPoint(s_ab_quat);
}


void FuseDlg::hideEvent(QHideEvent *pEvent)
{
    XflDialog::hideEvent(pEvent);
    s_bOutline    = m_pglFuseView->bOutline();
    s_bSurfaces   = m_pglFuseView->bSurfaces();
    s_bVLMPanels  = m_pglFuseView->bVLMPanels();
    s_bAxes       = m_pglFuseView->bAxes();
    s_bShowMasses = m_pglFuseView->bMasses();

    m_pglFuseView->saveViewPoint(s_ab_quat);
}


void FuseDlg::accept()
{
    m_pFuse->setName(m_pleName->text().toStdString());
    m_pFuse->setDescription(m_pteDescription->toPlainText().toStdString());
    m_pFuse->setColor(xfl::tofl5Clr(m_pcbColor->color()));

    QDialog::accept();
}


void FuseDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)       accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)    reject();
    else if (m_ppbSaveAsNew==pButton)                                       done(10);
}


void FuseDlg::makeWidgets()
{        
    m_pScale     = new QAction("Scale",     this);
    m_pTranslate = new QAction("Translate", this);
    m_pRotate    = new QAction("Rotate",    this);

    m_pButtonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Discard);
    {
        m_ppbSaveAsNew = new QPushButton("Save as");
        m_pButtonBox->addButton(m_ppbSaveAsNew, QDialogButtonBox::ActionRole);
    }

    m_pglFuseView = new gl3dFuseView;
    m_pglFuseView->showPartFrame(false);
    m_pglControls = new gl3dGeomControls(m_pglFuseView, FuseLayout, true);

    m_pMetaFrame = new QFrame;
    {
        QGridLayout *pMetaLayout = new QGridLayout;
        {
            m_pcbColor = new ColorBtn;

            m_pleName = new QLineEdit;
            m_pleName->setClearButtonEnabled(true);

            m_pteDescription = new QTextEdit;
            m_pteDescription->setToolTip("Enter here a short description for the fuselage");

            pMetaLayout->addWidget(m_pleName,        2,1,1,3);
            pMetaLayout->addWidget(m_pcbColor,  2,4);
            pMetaLayout->addWidget(m_pteDescription, 3,1,2,4);
        }
        m_pMetaFrame->setLayout(pMetaLayout);
    }
}


void FuseDlg::keyPressEvent(QKeyEvent *pEvent)
{
/*    bool bShift = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;*/
    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            if(m_pglControls->getDistance())
            {
                m_pglControls->stopDistance();
                m_pglFuseView->stopPicking();
                m_pglFuseView->setPicking(xfl::NOPICK);
                m_pglFuseView->clearMeasure();
                return;
            }
            else
            {
                reject();
                return;
            }
        }
        case Qt::Key_S:
        {
            if(bCtrl)
                accept();
            break;
        }
        default:
            break;
    }
    XflDialog::keyPressEvent(pEvent);
}


void FuseDlg::onMetaDataChanged()
{
     m_bDescriptionChanged = true;
}


void FuseDlg::onFuseColor()
{
    QColor clr = QColorDialog::getColor(xfl::fromfl5Clr(m_pFuse->color()), this, "Fuse colour", QColorDialog::ShowAlphaChannel);
    if(clr.isValid())
    {
        m_pFuse->setColor(xfl::tofl5Clr(clr));
        m_pcbColor->setColor(clr);
    }

    updateView();
    m_bDescriptionChanged = true;
}


void FuseDlg::updateView()
{
    m_pglFuseView->update();
}


void FuseDlg::onFuseInertia()
{
    if(!m_pFuse) return;
    PartInertiaDlg dlg(nullptr, m_pFuse, this);
//    dlg.move(pos().x()+25, pos().y()+25);
    if(dlg.exec()==QDialog::Accepted) m_bChanged=true;
    m_pFuse->computeStructuralInertia(Vector3d());
    m_bChanged = true;
    updateView();
}


void FuseDlg::onScale()
{
    ScaleDlg dlg(this);
    if(dlg.exec()!=QDialog::Accepted) return;

    m_pFuse->scale(dlg.XFactor(), dlg.YFactor(), dlg.ZFactor());

    m_pglFuseView->resetFuse();
    m_pglFuseView->setReferenceLength(m_pFuse->length());
    updateView();
    updateProperties();

    m_bChanged = true;
}


void FuseDlg::onTranslate()
{
    TranslateDlg dlg(this);
    if(dlg.exec()!=QDialog::Accepted) return;

    m_pFuse->translate(dlg.translationVector());
    m_pglFuseView->resetFuse();
    updateView();
    updateProperties();

    m_bChanged = true;
}


void FuseDlg::onRotate()
{
    RotateDlg dlg(this);
    if(dlg.exec()!=QDialog::Accepted) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    Vector3d O, axis;
    if      (dlg.axis()==0) axis.set(1.0,0.0,0.0);
    else if (dlg.axis()==1) axis.set(0.0,1.0,0.0);
    else if (dlg.axis()==2) axis.set(0.0,0.0,1.0);;
    m_pFuse->rotate(O, axis, dlg.angle());
    m_pglFuseView->resetFuse();
    updateView();
    updateProperties();

    m_bChanged = true;
    QApplication::restoreOverrideCursor();
}


void FuseDlg::onExportFuseToCADFile()
{
    if(!m_pFuse)return;   // is there anything to export?
    m_pFuse->makeFuseGeometry();

    CADExportDlg dlg(this);
    dlg.init(m_pFuse->shapes(), QString::fromStdString(m_pFuse->name()));
    dlg.exec();
}


void FuseDlg::onExportMeshToSTLFile()
{
    if(!m_pFuse) return;

    QString filter ="STL File (*.stl)";
    QString FileName;

    FileName = QString::fromStdString(m_pFuse->name()).trimmed();
    FileName.replace('/', '_');
    FileName.replace(' ', '_');

    QFileDialog Fdlg(this);
    FileName = Fdlg.getSaveFileName(this, "Export to STL File",
                                    SaveOptions::CADDirName() + "/"+FileName+".stl",
                                    "STL File (*.stl)",
                                    &filter);

    if(!FileName.length()) return;


    bool bBinary = true;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastDirName(FileName.left(pos));

    pos = FileName.indexOf(".stl", Qt::CaseInsensitive);
    if(pos<0) FileName += ".stl";

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(window(), "Warning", "Could not open the file for writing");
        return;
    }

    if(bBinary)
    {
        QDataStream out(&XFile);
        Objects3d::exportTriMesh(out,1.0, m_pFuse->triMesh());
    }
    else
    {
//        QTextStream out(&XFile);
    }

    XFile.close();
}


/**
 * Exports the part's triangulation to an STL file
 * For an STL-type Body, the geometry's triangulation is the same as the mesh
 * For other types, the triangulation is the array of triangular panels
 */
void FuseDlg::onExportTrianglesToSTLFile()
{
    if(!m_pFuse) return;

    QString filter ="STL File (*.stl)";
    QString FileName;

    FileName = QString::fromStdString(m_pFuse->name()).trimmed();
    FileName.replace('/', '_');
    FileName.replace(' ', '_');

    QFileDialog Fdlg(this);
    FileName = Fdlg.getSaveFileName(this, "Export to STL File",
                                    SaveOptions::CADDirName() + "/"+FileName+".stl",
                                    "STL File (*.stl)",
                                    &filter);

    if(!FileName.length()) return;


    bool bBinary = true;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastDirName(FileName.left(pos));

    pos = FileName.indexOf(".stl", Qt::CaseInsensitive);
    if(pos<0) FileName += ".stl";

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(window(), "Warning", "Could not open the file for writing");
        return;
    }

    if(bBinary)
    {
        QDataStream out(&XFile);
        out.setByteOrder(QDataStream::LittleEndian);
//        m_pFuse->exportStlTriangulation(out,1);
        Objects3d::exportTriangulation(out, 1.0, m_pFuse->triangles());
    }
    else
    {
//        QTextStream out(&XFile);
    }

    XFile.close();
}


void FuseDlg::onTessellation()
{
    TessControlsDlg dlg(this);
    dlg.initDialog(m_pFuse->occTessParams(), m_pFuse->gmshTessParams(), Part::isOccTessellator(), true);
    if(dlg.exec()==QDialog::Accepted)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        Fuse::setOccTessellator(dlg.bOcc());
        m_pFuse->setOccTessParams(dlg.occParameters());
        m_pFuse->setGmshTessParams(dlg.gmshParameters());

        m_pFuse->clearOccTriangulation();
        m_bDescriptionChanged = true;
        QString strange;
//        m_pFuse->makeShellTriangulation(strange, QString());

        gmesh::makeFuseTriangulation(m_pFuse, strange);
        m_pglFuseView->resetFuse();
        updateView();
        updateOutput("\n"+strange+"\n");
        QApplication::restoreOverrideCursor();
    }
}
