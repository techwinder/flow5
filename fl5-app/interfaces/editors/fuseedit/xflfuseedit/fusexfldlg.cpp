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


#include <QFileDialog>
#include <QVBoxLayout>
#include <QTime>
#include <QMessageBox>

#include "fusexfldlg.h"


#include <api/fusenurbs.h>
#include <api/fusesections.h>
#include <api/fusexfl.h>
#include <api/units.h>
#include <api/xmlfusewriter.h>

#include <core/saveoptions.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/editors/fuseedit/bodyscaledlg.h>
#include <interfaces/editors/fuseedit/bodytransdlg.h>
#include <interfaces/editors/fuseedit/flatfaceconverterdlg.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fuseframewt.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fuselinewt.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>


bool FuseXflDlg::s_bShowCtrlPoints=false;

QByteArray FuseXflDlg::s_VViewSplitterSizes, FuseXflDlg::s_HViewSplitterSizes;

QByteArray FuseXflDlg::s_Geometry;


FuseXflDlg::FuseXflDlg(QWidget *pParent) : FuseDlg(pParent)
{
    setWindowTitle(tr("Xfl Fuse Editor"));

    m_pFuseXfl = nullptr;
    m_StackPos  = 0; //the current position on the stack
    m_bChanged = false;

    makeCommonWts();
}


FuseXflDlg::~FuseXflDlg()
{
    clearStack(-1);
}


void FuseXflDlg::createActions()
{
    m_pResetFuse     = new QAction("Restore geometry and mesh", this);
    m_pScaleBody     = new QAction("Scale", this);
    m_pExportBodyXML = new QAction("Export body geometry to an XML file", this);
    m_pTranslateBody = new QAction("Translate", this);
    m_pToFlatFace    = new QAction("to flat face type", this);

    QMenu *pBodyMenu = new QMenu("Actions...",this);
    {
        pBodyMenu->addAction(m_pResetFuse);
        pBodyMenu->addSeparator();
        pBodyMenu->addAction(m_pExportBodyXML);
        pBodyMenu->addSeparator();
        pBodyMenu->addAction(m_pExportToCADFile);
        pBodyMenu->addAction(m_pExportMeshToSTL);
        pBodyMenu->addAction(m_pExportTrianglesToSTL);
        pBodyMenu->addSeparator();
        pBodyMenu->addAction(m_pFuseInertia);
        pBodyMenu->addSeparator();
        pBodyMenu->addAction(m_pTranslateBody);
        pBodyMenu->addAction(m_pScaleBody);
        pBodyMenu->addSeparator();
        pBodyMenu->addAction(m_pTessSettings);
        pBodyMenu->addSeparator();
        pBodyMenu->addAction(m_pToFlatFace);
    }
    m_ppbMenuButton->setMenu(pBodyMenu);
}


void FuseXflDlg::connectFuseXflSignals()
{
    connectBaseSignals();

    // action signals
    connect(m_pResetFuse,      SIGNAL(triggered()),         SLOT(onResetFuse()));
    connect(m_pScaleBody,      SIGNAL(triggered()),         SLOT(onScaleFuse()));
    connect(m_pExportBodyXML,  SIGNAL(triggered()),         SLOT(onExportFuseToXML()));
    connect(m_pTranslateBody,  SIGNAL(triggered()),         SLOT(onTranslateFuse()));
    connect(m_pToFlatFace,     SIGNAL(triggered()),         SLOT(onConvertToFlatFace()));

    // view signals
    connect(m_pFuseLineView, SIGNAL(scaleFuse(bool)),       SLOT(onScaleFuse(bool)));
    connect(m_pFuseLineView, SIGNAL(translateFuse()),       SLOT(onTranslateFuse()));
    connect(m_pFuseLineView, SIGNAL(insertFrame(Vector3d)), SLOT(onInsertFrame(Vector3d)));
    connect(m_pFuseLineView, SIGNAL(removeFrame(int)),      SLOT(onRemoveFrame(int)));

    connect(m_pFrameView,    SIGNAL(scaleBody(bool)),       SLOT(onScaleFuse(bool)));
    connect(m_pFrameView,    SIGNAL(insertPoint(Vector3d)), SLOT(onInsertPoint(Vector3d)));
    connect(m_pFrameView,    SIGNAL(removePoint(int)),      SLOT(onRemovePoint(int)));
}


void FuseXflDlg::onRemoveFrame(int iFrame)
{
    if(m_pFuseXfl->isSplineType() && (m_pFuseXfl->frameCount()<=m_pFuseXfl->nurbs().uDegree()+1))
    {
        QString strange("Cannot remove: the number of frames must be at least equal to the x degree+1");
        QMessageBox::warning(this, "Warning", strange);
        return;
    }


    m_pFuseXfl->removeFrame(iFrame);
    updateFuseDlg();
    takePicture();

    m_bChanged = true;
}


void FuseXflDlg::onInsertFrame(Vector3d const &pos)
{
    if(!m_pFuseXfl->isSectionType())
        m_pFuseXfl->insertFrame(pos);
    else
    {
        FuseSections *pFuseSecs = dynamic_cast<FuseSections*>(m_pFuseXfl);
        pFuseSecs->insertFrame(pos);
    }

    m_bChanged = true;
    updateFuseDlg();
    takePicture();

    m_bChanged = true;
}


void FuseXflDlg::onRemovePoint(int iPt)
{
    m_pFuseXfl->removeSideLine(iPt);

    m_bChanged = true;
    updateFuseDlg();
    takePicture();

    m_bChanged = true;
}


void FuseXflDlg::onInsertPoint(Vector3d const &pos)
{
    m_pFuseXfl->insertPoint(pos);

    m_bChanged = true;
    updateFuseDlg();
    takePicture();

    m_bChanged = true;
}


void FuseXflDlg::initDialog(Fuse *pFuse)
{
    FuseDlg::initDialog(pFuse);

    FuseXfl const*pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);
    if(!pFuseXfl) return;

    m_pToFlatFace->setEnabled(pFuseXfl->isSplineType());

    m_pFuseLineView->setUnitFactor(Units::mtoUnit());
    m_pFrameView->setUnitFactor(Units::mtoUnit());

//    m_pglControls->showCtrlPointsCtrl(true);

    m_pglFuseView->showOutline(s_bOutline);
    m_pglFuseView->showSurfaces(s_bSurfaces);
    m_pglFuseView->showPanels(s_bVLMPanels);
    m_pglFuseView->showMasses(s_bShowMasses);
    m_pglFuseView->showControlPoints(s_bShowCtrlPoints);
}


void FuseXflDlg::setBody(FuseXfl *pFuseXfl)
{
    if(pFuseXfl) m_pFuseXfl = pFuseXfl;
    m_pFuse = pFuseXfl;

    m_pFuseXfl->makeQuadMesh(0, Vector3d()); // needed to initialize wetted area calculation

    if(pFuseXfl)
    {
        m_pglFuseView->setFuse(pFuseXfl);
        m_pglFuseView->setReferenceLength(pFuseXfl->length());
    }
    m_pglFuseView->reset3dScale();

    m_pFuseLineView->setXflFuse(m_pFuseXfl);
    m_pFrameView->setBody(m_pFuseXfl);

    takePicture();
}


void FuseXflDlg::accept()
{
    std::string strange;

    m_pFuseXfl->makeFuseGeometry();
    m_pFuseXfl->makeDefaultTriMesh(strange, "");
    m_pFuseXfl->makeQuadMesh(0, m_pFuseXfl->position());
    FuseDlg::accept();
}


void FuseXflDlg::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QMenu *pBodyMenu = new QMenu("context menu");
    {
        pBodyMenu->addAction(m_pExportBodyXML);
        pBodyMenu->addSeparator();
        pBodyMenu->addAction(m_pExportToCADFile);
        pBodyMenu->addAction(m_pExportMeshToSTL);
        pBodyMenu->addAction(m_pExportTrianglesToSTL);
        pBodyMenu->addSeparator();
        pBodyMenu->addAction(m_pFuseInertia);
        pBodyMenu->addSeparator();
        pBodyMenu->addAction(m_pTranslateBody);
        pBodyMenu->addAction(m_pScaleBody);
        pBodyMenu->addSeparator();
        pBodyMenu->addAction(m_pToFlatFace);
        pBodyMenu->addSeparator();
        QMenu *pBackImageMenu = pBodyMenu->addMenu(tr("Background image"));
        {
            pBackImageMenu->addAction(m_pBackImageLoad);
            pBackImageMenu->addAction(m_pBackImageClear);
            pBackImageMenu->addAction(m_pBackImageSettings);
        }
    }

    pBodyMenu->exec(pEvent->globalPos());
}


void FuseXflDlg::onScaleFuse()
{
    onScaleFuse(false);
}


void FuseXflDlg::onScaleFuse(bool bFrameOnly)
{
    if(!m_pFuseXfl) return;
    FuseSections *pFuseSecs = nullptr;
    if(m_pFuseXfl->isSectionType()) pFuseSecs = dynamic_cast<FuseSections*>(m_pFuseXfl);

    BodyScaleDlg dlg(this);
    dlg.move(QCursor::pos());

    int iFr = -1;
    if(pFuseSecs) iFr = pFuseSecs->activeSectionIndex();
    else          iFr = m_pFuseXfl->activeFrameIndex();
    dlg.setFrameIndex(iFr);

    dlg.initDialog(bFrameOnly);

    if(dlg.exec()==QDialog::Accepted)
    {
        if(dlg.bFrameOnly())
        {
            m_pFuseXfl->scaleFrame(dlg.YFactor(), dlg.ZFactor(), iFr);
        }
        else
        {
            m_pFuseXfl->scale(dlg.XFactor(), dlg.YFactor(), dlg.ZFactor());
        }

        updateProperties();
        updateFuseDlg();
        takePicture();

        m_bChanged = true;
    }
}


void FuseXflDlg::onTranslateFuse()
{
    if(!m_pFuseXfl) return;

    BodyTransDlg dlg(this);
    dlg.setFrameId(m_pFuseXfl->activeFrameIndex());
    dlg.initDialog();

    if(dlg.exec()==QDialog::Accepted)
    {
        Vector3d T(dlg.dx(), dlg.dy(), dlg.dz());
        if(dlg.bFrameOnly())
            m_pFuseXfl->translateFrame(T, m_pFuseXfl->activeFrameIndex());
        else
            m_pFuseXfl->translate(T);

        takePicture();

        updateFuseDlg();
        m_bChanged = true;
    }
}


void FuseXflDlg::updateView()
{
    m_pglFuseView->update();
    m_pFrameView->update();
    m_pFuseLineView->update();
}


void FuseXflDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bShift = false;
    bool bCtrl  = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;
    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_Z:
        {
            if(bCtrl)
            {
                if(bShift)
                {
                    onRedo();
                }
                else onUndo();
                pEvent->accept();
            }
            else pEvent->ignore();
            break;
        }
        case Qt::Key_Y:
        {
            if(bCtrl)
            {
                onRedo();
                pEvent->accept();
            }
            else pEvent->ignore();
            break;
        }

        default:
            return FuseDlg::keyPressEvent(pEvent);
    }
    pEvent->ignore();
}


void FuseXflDlg::resizeEvent(QResizeEvent *pEvent)
{
    m_pFuseLineView->resetDefaultScale();
    m_pFrameView->resetDefaultScale();

    pEvent->accept();
}


void FuseXflDlg::showEvent(QShowEvent *pEvent)
{
    FuseDlg::showEvent(pEvent);

    restoreGeometry(s_Geometry);
    if(s_HViewSplitterSizes.length()>0) m_pViewHSplitter->restoreState(s_HViewSplitterSizes);
    if(s_VViewSplitterSizes.length()>0) m_pViewVSplitter->restoreState(s_VViewSplitterSizes);

    m_bChanged    = false;
    m_pglFuseView->resetFuse();
    m_pglFuseView->showControlPoints(s_bShowCtrlPoints);

    m_pFuseLineView->resetDefaultScale();
    m_pFrameView->resetDefaultScale();
    m_pFuseLineView->setAutoUnits();
    m_pFrameView->setAutoUnits();
}


/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void FuseXflDlg::hideEvent(QHideEvent *pEvent)
{
    FuseDlg::hideEvent(pEvent);

    s_Geometry = saveGeometry();

    s_bOutline  = m_pglFuseView->bOutline();
    s_bSurfaces = m_pglFuseView->bSurfaces();
    s_bVLMPanels = m_pglFuseView->bVLMPanels();
    s_bShowMasses = m_pglFuseView->bMasses();
    s_bShowCtrlPoints = m_pglFuseView->bCtrlPts();

    s_HViewSplitterSizes = m_pViewHSplitter->saveState();
    s_VViewSplitterSizes = m_pViewVSplitter->saveState();
    pEvent->accept();
}


/**
  * Clears the stack starting at a given position.
  * @param the first stack element to remove
  */
void FuseXflDlg::clearStack(int pos)
{
    for(int il=m_UndoStack.size()-1; il>pos; il--)
    {
        delete m_UndoStack.at(il);
        m_UndoStack.removeAt(il);     // remove from the stack
    }
    m_StackPos = m_UndoStack.size()-1;
}


/**
 * Copies the current Fuse object to a new Fuse and pushes it on the stack.
 */
void FuseXflDlg::takePicture()
{
    //this is a good time to update properties
    updateProperties();

    //clear the downstream part of the stack which becomes obsolete
    clearStack(m_StackPos);

    // append a copy of the current object

    Fuse* pFuseXfl = m_pFuseXfl->clone();
    pFuseXfl->duplicateFuse(*m_pFuseXfl);
    m_UndoStack.append(pFuseXfl);

    // the new current position is the top of the stack
    m_StackPos = m_UndoStack.size()-1;

    enableStackBtns(); // to enable/disable undo & redo buttons
}


void FuseXflDlg::onRedo()
{
    if(m_StackPos<m_UndoStack.size()-1)
    {
        m_StackPos++;
        setPicture();
    }
    enableStackBtns(); // to enable/disable undo & redo buttons
}


void FuseXflDlg::onUndo()
{
    if(m_StackPos>0)
    {
        m_StackPos--;
        setPicture();
    }
    else
    {
        //nothing to restore
    }
    enableStackBtns(); // to enable/disable undo & redo buttons
}


void FuseXflDlg::updateProperties(bool bFull)
{
    if(!m_pFuseXfl) return;

    std::string log;
    m_pFuseXfl->getProperties(log, "", bFull);
    m_pglFuseView->setBotLeftOutput(log);
}


void FuseXflDlg::makeCommonWts()
{
    m_pViewHSplitter = new QSplitter(Qt::Horizontal, this);
    {
        m_pViewHSplitter->setChildrenCollapsible(false);
//        m_pViewHSplitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        m_pViewVSplitter = new QSplitter(Qt::Vertical, this);
        {
            m_pViewVSplitter->setChildrenCollapsible(false);

            m_pFuseLineView = new FuseLineWt(this);
//            m_pFuseLineView->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

            m_pViewVSplitter->addWidget(m_pFuseLineView);
            m_pViewVSplitter->addWidget(m_pglFuseView);
            m_pViewVSplitter->addWidget(m_pglControls);

            m_pViewVSplitter->setStretchFactor(0,5);
            m_pViewVSplitter->setStretchFactor(1,5);
            m_pViewVSplitter->setStretchFactor(2,1);
        }

        m_pFrameView = new FuseFrameWt(this);

        m_pViewHSplitter->addWidget(m_pViewVSplitter);
        m_pViewHSplitter->addWidget(m_pFrameView);
    }

//    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
    {
        m_ppblRedraw = new QPushButton("Regenerate\t(F4)");
        m_ppbMenuButton = new QPushButton("Actions");

        m_pButtonBox->addButton(m_ppblRedraw, QDialogButtonBox::ActionRole);
        m_pButtonBox->addButton(m_ppbMenuButton, QDialogButtonBox::ActionRole);
        m_pButtonBox->addButton(m_ppbSaveAsNew, QDialogButtonBox::ActionRole);
    }
}


void FuseXflDlg::onResetScales()
{
    m_pglFuseView->on3dReset();
    m_pFuseLineView->onResetScales();
    m_pFrameView->onResetScales();
    updateView();
}


void FuseXflDlg::onExportFuseToXML()
{
    if(!m_pFuseXfl)return ;// is there anything to export?

    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = QString::fromStdString(m_pFuseXfl->name()).trimmed();
    strong.replace(' ', '_');
    FileName = QFileDialog::getSaveFileName(window(), "Export plane definition to xml file",
                                            SaveOptions::xmlPlaneDirName() +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;
    int pos = FileName.indexOf(".xml", Qt::CaseInsensitive);
    if(pos<0) FileName += ".xml";


    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    XmlFuseWriter fusewriter(XFile);

    fusewriter.writeXMLBody(*m_pFuseXfl);

    XFile.close();
}


void FuseXflDlg::onConvertToFlatFace()
{
    if(!m_pFuse || !m_pFuse->isSplineType()) return;

    FlatFaceConverterDlg dlg(this);
    dlg.initDialog(m_pFuseXfl);
    if(dlg.exec()!=QDialog::Accepted) return;

    m_pFuseXfl->duplicateFuseXfl(*dlg.flatFaceFuse());

    m_pToFlatFace->setEnabled(false);
}


void FuseXflDlg::onResetFuse()
{
    std::string strange;

    m_bChanged = true;

    m_pFuseXfl->makeShellsFromShapes();

    m_pFuseXfl->makeFuseGeometry();

    m_pFuseXfl->makeDefaultTriMesh(strange, "");
    m_pFuseXfl->makeQuadMesh(0, Vector3d());

    updateFuseXfl();
    m_pglFuseView->resetFuse();
    updateView();
}


void FuseXflDlg::updateFuseXfl()
{
    // update data for 3d display
    std::string strong;

    if(m_pFuseXfl->isSectionType())
    {
/*        FuseSections *pFuseSecs = dynamic_cast<FuseSections*>(m_pFuseXfl);
        if(pFuseSecs->hasActiveSection())
        {
            QVector<Vector3d> const & sec = pFuseSecs->activeSection();
            qDebug("updateFuseXfl   %13g  %13g", sec.front().x, (sec.front().z+sec.back().z)/2.0);
        }*/
        m_pFuseXfl->makeNURBS();
    }


    m_pFuseXfl->makeDefaultTriMesh(strong, "   ");
    m_pFuseXfl->makeSurfaceTriangulation(W3dPrefs::bodyAxialRes(), W3dPrefs::bodyHoopRes());
    m_pFuseXfl->makeShape(strong);
    m_pFuseXfl->makeShellsFromShapes();
}




