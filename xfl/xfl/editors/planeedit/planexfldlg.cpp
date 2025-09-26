/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include <QApplication>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMenu>
#include <QHeaderView>
#include <QFileDialog>

#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include "planexfldlg.h"

#include <xfl/opengl/gl3dgeomcontrols.h>
#include <xfl/opengl/gl3dplanexflview.h>

#include <xflcore/saveoptions.h>
#include <xflcore/stlreaderdlg.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflgeom/geom2d/segment2d.h>
#include <xflgeom/geom3d/frame.h>
#include <xflgeom/geom3d/quad3d.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflgeom/geom_globals/geom_global.h>
#include <xflmath/constants.h>
#include <xfl/editors/fuseedit/flatfaceconverterdlg.h>
#include <xfl/editors/fuseedit/fuseoccdlg.h>
#include <xfl/editors/fuseedit/fusestldlg.h>
#include <xfl/editors/fuseedit/xflfuseedit/fusexfldefdlg.h>
#include <xfl/editors/fuseedit/xflfuseedit/fusexflobjectdlg.h>
#include <xfl/editors/fuseedit/fusemesherdlg.h>
#include <xfl/editors/importobjectdlg.h>
#include <xfl/editors/inertia/partinertiadlg.h>
#include <xfl/editors/inertia/planexflinertiadlg.h>
#include <xfl/editors/planeedit/planepartdelegate.h>
#include <xfl/editors/planeedit/planepartmodel.h>
#include <xfl/editors/wingedit/wingdefdlg.h>
#include <xfl/editors/wingedit/wingobjectdlg.h>
#include <xfl/editors/wingedit/wingscaledlg.h>
#include <xflobjects/exchange/stlwriterdlg.h>
#include <xflfoil/objects2d/objects2d.h>
#include <xflobjects/objects3d/fuse/fuse.h>
#include <xflobjects/objects3d/fuse/fuseocc.h>
#include <xflobjects/objects3d/fuse/fusesections.h>
#include <xflobjects/objects3d/objects3d.h>
#include <xflocc/occmeshcontrols.h>
#include <xflobjects/objects3d/fuse/fusestl.h>
#include <xflobjects/objects3d/fuse/fusexfl.h>
#include <xflobjects/objects3d/part.h>
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflobjects/objects3d/wing/wingxfl.h>
#include <xflobjects/objects_globals/objects_global.h>
#include <xflocc/occ_globals.h>
#include <xflpanels/mesh/interfaces/mesherwt.h>
#include <xflpanels/mesh/shell/afmesher.h>
#include <xflpanels/panels/panel3.h>
#include <xflscript/xml/fuse/xmlfusereader.h>
#include <xflscript/xml/xplane/xmlwingreader.h>
#include <xflwidgets/customdlg/doublevaluedlg.h>
#include <xflwidgets/customdlg/intvaluedlg.h>
#include <xflwidgets/customwts/cptableview.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/plaintextoutput.h>

bool PlaneXflDlg::s_bParallelCut = true;

int PlaneXflDlg::s_iActivePage = 1;

double PlaneXflDlg::s_StitchPrecision = 1.0e-4;
double PlaneXflDlg::s_NodeMergeDistance = 0.05;

QByteArray PlaneXflDlg::s_HSplitterSizes;
QByteArray PlaneXflDlg::s_PartSplitterSizes;
QByteArray PlaneXflDlg::s_XflGeometry;

#define NPOINTS 19

PlaneXflDlg::PlaneXflDlg(QWidget *pParent) : PlaneDlg(pParent)
{
    setWindowTitle("Xfl plane editor");

    m_pPlane = nullptr;

    m_StackPos = 0;

    makeActions();
    setupLayout();
    connectSignals();
}


void PlaneXflDlg::connectSignals()
{
    connectBaseSignals();

    connect(m_pClearOutput,           SIGNAL(triggered(bool)), SLOT(onClearOutput()));

    connect(m_pInsertWing,            SIGNAL(triggered(bool)), SLOT(onInsertWing()));
    connect(m_pInsertWingOther,       SIGNAL(triggered(bool)), SLOT(onImportOtherWing()));
    connect(m_pInsertWingXml,         SIGNAL(triggered(bool)), SLOT(onInsertWingFromXml()));
    connect(m_pInsertWingVSP,         SIGNAL(triggered(bool)), SLOT(onInsertWingFromVSP()));
    connect(m_pInsertElev,            SIGNAL(triggered(bool)), SLOT(onInsertElevator()));
    connect(m_pInsertFin,             SIGNAL(triggered(bool)), SLOT(onInsertFin()));
    connect(m_pInsertFuseXflSpline,   SIGNAL(triggered(bool)), SLOT(onInsertFuseXfl()));
    connect(m_pInsertFuseXflFlat,     SIGNAL(triggered(bool)), SLOT(onInsertFuseXfl()));
    connect(m_pInsertFuseXflSections, SIGNAL(triggered(bool)), SLOT(onInsertFuseXfl()));
    connect(m_pInsertFuseXml,         SIGNAL(triggered(bool)), SLOT(onInsertFuseXml()));
    connect(m_pInsertFuseOther,       SIGNAL(triggered(bool)), SLOT(onImportOtherFuse()));
    connect(m_pInsertFuseOcc,         SIGNAL(triggered(bool)), SLOT(onInsertFuseOcc()));
    connect(m_pInsertFuseStl,         SIGNAL(triggered(bool)), SLOT(onInsertFuseStl()));
    connect(m_pInsertEllWing,         SIGNAL(triggered(bool)), SLOT(onInsertEllipticWing()));
    connect(m_pInsertSTLSphere,       SIGNAL(triggered(bool)), SLOT(onInsertSTLSphereFuse()));
    connect(m_pInsertSTLCylinder,     SIGNAL(triggered(bool)), SLOT(onInsertSTLCylinderFuse()));
    connect(m_pInsertCADSphere,       SIGNAL(triggered(bool)), SLOT(onInsertCADShape()));
    connect(m_pInsertCADCylinder,     SIGNAL(triggered(bool)), SLOT(onInsertCADShape()));
    connect(m_pInsertCADBox,          SIGNAL(triggered(bool)), SLOT(onInsertCADShape()));
    connect(m_pRemovePart,            SIGNAL(triggered(bool)), SLOT(onRemovePart()));
    connect(m_pDuplicatePart,         SIGNAL(triggered(bool)), SLOT(onDuplicatePart()));
    connect(m_pEditPartDef,           SIGNAL(triggered(bool)), SLOT(onEditPart()));
    connect(m_pEditPartObject,        SIGNAL(triggered(bool)), SLOT(onEditPart()));
    connect(m_pExportMeshSTL,         SIGNAL(triggered(bool)), SLOT(onExportMeshToSTLFile()));
    connect(m_pScalePlane,            SIGNAL(triggered(bool)), SLOT(onScalePlane()));

    connect(m_plwWingListWt,          SIGNAL(clicked(QModelIndex)), SLOT(onPartListClicked(QModelIndex)));
    connect(m_plwFuseListWt,          SIGNAL(clicked(QModelIndex)), SLOT(onListClick()));
    connect(m_plwFuseListWt,          SIGNAL(clicked(QModelIndex)), SLOT(onPartListClicked(QModelIndex)));

    QItemSelectionModel *pSelModel = m_plwWingListWt->selectionModel();
    connect(pSelModel,                SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onListClick()));

    connect(m_ppbCutFuse,             SIGNAL(clicked(bool)),  SLOT(onCutFuse()));

    connect(m_pResetFuse,             SIGNAL(triggered()),    SLOT(onResetFuse()));
    connect(m_pTessellation,          SIGNAL(triggered()),    SLOT(onTessellation()));

    connect(m_pLeftTabWidget,         SIGNAL(currentChanged(int)), SLOT(onTabChanged(int)));

    connect(m_pcptParts,              SIGNAL(clicked(QModelIndex)),                              SLOT(onPartItemClicked(QModelIndex)));
    connect(m_pcptParts,              SIGNAL(dataPasted()),                                      SLOT(onPartDataChanged()));
    connect(m_pPartModel,             SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(onPartDataChanged()));
    connect(m_pPartModel,             SIGNAL(wingNamesChanged()),                                SLOT(onNamesChanged()));
    connect(m_pchHighlightSel,        SIGNAL(clicked(bool)),                                     SLOT(onHighlightSel(bool)));


    connect(m_pMoveUp,               SIGNAL(triggered(bool)), SLOT(onMovePartUp()));
    connect(m_pMoveDown,             SIGNAL(triggered(bool)), SLOT(onMovePartDown()));
    connect(m_pPartInertia,          SIGNAL(triggered(bool)), SLOT(onPartInertia()));
    connect(m_pPartScale,            SIGNAL(triggered(bool)), SLOT(onScalePart()));

    connect(m_pHSplitter,            SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved(int,int)));

    connect(m_pglPlaneView,          SIGNAL(partSelected(Part*)),            SLOT(onSelectPart(Part*)));
    connect(m_pglPlaneView,          SIGNAL(pickedNodePair(QPair<int,int>)), SLOT(onPickedNodePair(QPair<int,int>)));


    connect(m_pMesherWt,             SIGNAL(outputMsg(QString)), m_ppto, SLOT(onAppendThisPlainText(QString)));
    connect(m_pMesherWt,             SIGNAL(updateFuseView()),           SLOT(onUpdateMesh()));

    connect(m_pRestoreFuseMesh,      SIGNAL(triggered()),    SLOT(onResetFuseMesh()));
    connect(m_pFuseMesher,           SIGNAL(triggered()),    SLOT(onFuseMeshDlg()));

    connect(m_ppbMoveNode,           SIGNAL(clicked(bool)),  SLOT(onMergeNodes(bool)));

    connect(m_ppbSelectPanels,       SIGNAL(clicked(bool)),  SLOT(onSelectPanels(bool)));
    connect(m_ppbDeletePanel,        SIGNAL(clicked(bool)),  SLOT(onDeleteP3Selection()));

    connect(m_ppbMakeP3,             SIGNAL(clicked(bool)),  SLOT(onMakeP3(bool)));
    connect(m_ppbMakeP3Strip,        SIGNAL(clicked(bool)),  SLOT(onMakeP3Strip(bool)));

    connect(m_ppbUndo,               SIGNAL(clicked()), SLOT(onUndo()));
    connect(m_ppbRedo,               SIGNAL(clicked()), SLOT(onRedo()));
}


void PlaneXflDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent const*pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        updateOutput(pMsgEvent->msg());
    }
    else if(pEvent->type() == MESH_UPDATE_EVENT)
    {
        Fuse *pFuse = m_pPlaneXfl->fuse(0);
        if(!pFuse) return;
        QString strange;
        strange = "   Making mesh from triangles\n";
        pFuse->triMesh().makeMeshFromTriangles(m_pMesherWt->triangles(), 0, xfl::FUSESURFACE, strange, "      ");
        updateOutput(strange);

        pFuse->setMaxElementSize(AFMesher::maxEdgeLength());
        strange  = QString::asprintf("\nTriangle count = %d\n", pFuse->nPanel3());
        strange += QString::asprintf("Node count     = %d\n", int(pFuse->nodes().size()));
        strange += "\n_______\n\n";
        updateOutput(strange);
        onUpdateMesh();
        m_bChanged = true;
    }
    else
        QDialog::customEvent(pEvent);
}


void PlaneXflDlg::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QMenu *pPlaneCtxMenu = new QMenu("context menu", this);
    {
        pPlaneCtxMenu->addAction(m_pPlaneInertia);
        pPlaneCtxMenu->addSeparator();
        pPlaneCtxMenu->addAction(m_pScalePlane);
        pPlaneCtxMenu->addSeparator();
        QMenu *pCheckMeshMenu = pPlaneCtxMenu->addMenu("Mesh");
        {
            m_pRestoreFuseMesh  = new QAction("Restore default mesh",this);
            m_pFuseMesher    = new QAction("Fuselage mesher");
            pCheckMeshMenu->addAction(m_pCheckMesh);
            pCheckMeshMenu->addAction(m_pConnectPanels);
            pCheckMeshMenu->addAction(m_pCheckFreeEdges);
            pCheckMeshMenu->addAction(m_pClearHighlighted);
            pCheckMeshMenu->addSeparator();
            pCheckMeshMenu->addAction(m_pMergeFuseToWingNodes);
            pCheckMeshMenu->addSeparator();
            pCheckMeshMenu->addAction(m_pCenterOnPanel);
            pCheckMeshMenu->addSeparator();
            pCheckMeshMenu->addAction(m_pRestoreFuseMesh);
            pCheckMeshMenu->addSeparator();
            pCheckMeshMenu->addAction(m_pFuseMesher);
        }
        pPlaneCtxMenu->addSeparator();

        QAction *m_pBackImageLoad = new QAction("Load", this);
        connect(m_pBackImageLoad, SIGNAL(triggered()), m_pglPlaneView, SLOT(onLoadBackImage()));

        QAction *m_pBackImageClear = new QAction("Clear", this);
        connect(m_pBackImageClear, SIGNAL(triggered()), m_pglPlaneView, SLOT(onClearBackImage()));

        QAction *m_pBackImageSettings = new QAction("Settings", this);
        connect(m_pBackImageSettings, SIGNAL(triggered()), m_pglPlaneView, SLOT(onBackImageSettings()));

        pPlaneCtxMenu->addSeparator();
        QMenu *pBackImageMenu = pPlaneCtxMenu->addMenu("Background image");
        {
            pBackImageMenu->addAction(m_pBackImageLoad);
            pBackImageMenu->addAction(m_pBackImageClear);
            pBackImageMenu->addAction(m_pBackImageSettings);
        }

    }
    pPlaneCtxMenu->exec(pEvent->globalPos());
}


void PlaneXflDlg::initDialog(Plane *pPlane, bool bAcceptName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    PlaneDlg::initDialog(pPlane, bAcceptName);

    PlaneXfl* pPlaneXfl = dynamic_cast<PlaneXfl*>(pPlane);

    m_pPlaneXfl = pPlaneXfl;
    m_bAcceptName = bAcceptName;

    m_pPartDelegate->setPlane(m_pPlaneXfl);
    m_pPartModel->setPlane(m_pPlaneXfl);
    m_pPartModel->updateData();

    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->setPlane(m_pPlaneXfl);
    if(m_pPlaneXfl->mainWing())
        m_pglPlaneView->setReferenceLength(m_pPlaneXfl->mainWing()->planformSpan()/2.0);
    else
    {
        if(m_pPlaneXfl->hasFuse())  m_pglPlaneView->setReferenceLength(m_pPlaneXfl->fuse(0)->length()*2);
        else                     m_pglPlaneView->setReferenceLength(1.0);
    }
    m_pglPlaneView->reset3dScale();

    for(int iw=0;    iw<pPlaneXfl->nWings();   iw++)    pPlaneXfl->wing(iw)->setVisible(true);
    for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++) pPlaneXfl->fuse(ifuse)->setVisible(true);


    onUpdatePlaneProps();

    setControls();

    if(s_iActivePage>=0 && s_iActivePage<4) m_pLeftTabWidget->setCurrentIndex(s_iActivePage);

    QApplication::restoreOverrideCursor();
}


void PlaneXflDlg::setControls()
{
    gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    m_pchHighlightSel->setChecked(pglPlaneXflView->isHighlightingPart());
    m_pdeStitchPrecision->setValue(s_StitchPrecision*Units::mtoUnit());
    m_pchCutParallel->setChecked(s_bParallelCut);
}


void PlaneXflDlg::makeActions()
{
    m_pEditPartDef           = new QAction("Edit",                       this);
    m_pEditPartObject        = new QAction("Edit (advanced)",            this);
    m_pInsertWing            = new QAction("new",                        this);
    m_pInsertWingOther       = new QAction("from other plane",           this);
    m_pInsertWingXml         = new QAction("from XML file",              this);
    m_pInsertWingVSP         = new QAction("from VSP export",            this);
    m_pInsertElev            = new QAction("Elevator",                   this);
    m_pInsertFin             = new QAction("Fin",                        this);
    m_pInsertFuseXflSpline   = new QAction("NURBS type",                 this);
    m_pInsertFuseXflFlat     = new QAction("Quad faces",                 this);
    m_pInsertFuseXflSections = new QAction("Interpolated (in progress)", this);

    m_pInsertFuseXml         = new QAction("from XML file",                   this);
    m_pInsertFuseOther       = new QAction("from other plane",                this);
    m_pInsertFuseOcc         = new QAction("from CAD file",                   this);
    m_pInsertFuseStl         = new QAction("from STL file",                   this);
    m_pInsertEllWing         = new QAction("Insert elliptical wing",          this);
    m_pInsertSTLSphere       = new QAction("Insert STL sphere fuselage",      this);
    m_pInsertSTLCylinder     = new QAction("Insert STL cylinder fuselage",    this);
    m_pInsertCADSphere       = new QAction("Insert CAD sphere fuselage",      this);
    m_pInsertCADCylinder     = new QAction("Insert CAD cylinder fuselage",    this);
    m_pInsertCADBox          = new QAction("Insert CAD box fuselage",         this);
    m_pRemovePart            = new QAction("Remove",                          this);
    m_pDuplicatePart         = new QAction("Duplicate",                       this);

    m_pResetFuse             = new QAction("Restore geometry and mesh",  this);
    m_pTessellation          = new QAction("Fuse tessellation",          this);

    m_pPartInertia   = new QAction("Inertia", this);
    m_pPartScale     = new QAction("Scale", this);
    m_pMoveUp        = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowUp),  "Move Up", this);
    m_pMoveDown      = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowDown),  "Move Down", this);

    m_pExportMeshSTL = new QAction("Export mesh to STL", this);

    m_pPartMenu = new QMenu("Selected Part", this);
    {
        m_pPartMenu->addAction(m_pEditPartDef);
        m_pPartMenu->addAction(m_pEditPartObject);
        m_pPartMenu->addAction(m_pRemovePart);
        m_pPartMenu->addAction(m_pDuplicatePart);
        m_pPartMenu->addSeparator();
        m_pPartMenu->addAction(m_pPartInertia);
        m_pPartMenu->addAction(m_pPartScale);
        m_pPartMenu->addSeparator();
        m_pPartMenu->addAction(m_pTessellation);
        m_pPartMenu->addAction(m_pFlipNormals);
        m_pPartMenu->addSeparator();
        m_pPartMenu->addAction(m_pMoveUp);
        m_pPartMenu->addAction(m_pMoveDown);
    }
}


PlaneXflDlg::~PlaneXflDlg()
{
}


void PlaneXflDlg::keyPressEvent(QKeyEvent *pEvent)
{
    /*    bool bShift = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;*/
    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;
    bool bAlt = false;
    if(pEvent->modifiers() & Qt::AltModifier)   bAlt =true;

    switch (pEvent->key())
    {
        case(Qt::Key_Escape):
        {
            if(!cancelPicks())
                reject();
            return;
            break;
        }
        case Qt::Key_1:
        {
            if(bCtrl)
            {
                m_pLeftTabWidget->setCurrentIndex(0);
            }
            break;
        }
        case Qt::Key_2:
        {
            if(bCtrl)
            {
                m_pLeftTabWidget->setCurrentIndex(1);
            }
            break;
        }
        case Qt::Key_3:
        {
            if(bCtrl)
            {
                m_pLeftTabWidget->setCurrentIndex(2);
            }
            break;
        }
        case Qt::Key_4:
        {
            if(bCtrl)
            {
                m_pLeftTabWidget->setCurrentIndex(3);
            }
            break;
        }
        case Qt::Key_5:
        {
            if(bCtrl)
            {
                m_pLeftTabWidget->setCurrentIndex(4);
            }
            break;
        }
        case Qt::Key_B:
        {
            if(bCtrl)
            {
                Fuse *pFuse = m_pPlaneXfl->fuse(0);
                if(pFuse) editFuse(0);
                onUpdatePlane();

                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_C:
        {
            if(bAlt)
            {
                m_pglPlaneView->clearHighlightList();
                m_pglPlaneView->updatePartFrame(m_pPlane);
                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_E:
        {
            if(bCtrl)
            {
                WingXfl *pStab = m_pPlaneXfl->stab();
                if(pStab)
                {
                    editWing(pStab);
                }

                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_F:
        {
            if(bCtrl)
            {
                WingXfl *pFin = m_pPlaneXfl->fin();
                if(pFin)
                {
                    editWing(pFin);
                }

                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_S:
        {
            if(bCtrl)
                onOK();
            break;
        }
        case Qt::Key_W:
        {
            if(bCtrl)
            {
                WingXfl *pWing = m_pPlaneXfl->mainWing();
                if(pWing)
                {
                    editWing(pWing);
                }

                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_F12:
        {
            onPlaneInertia();
            pEvent->accept();
            return;
        }
        default: break;
    }
    PlaneDlg::keyPressEvent(pEvent);
}


bool PlaneXflDlg::cancelPicks()
{
    bool bCancelled = false;
    if(m_pglControls->getDistance())
    {
        m_pglControls->stopDistance();
        m_pglPlaneView->clearMeasure();
        bCancelled = true;
    }

    if(m_ppbMoveNode->isChecked() || m_ppbSelectPanels->isChecked()|| m_ppbMakeP3->isChecked() || m_ppbMakeP3Strip->isChecked())
    {
        endPanelMods();
        m_ppbMoveNode->setChecked(false);
        m_ppbSelectPanels->setChecked(false);
        m_ppbMakeP3->setChecked(false);
        m_ppbMakeP3Strip->setChecked(false);
        bCancelled = true;
    }

    if(m_pglPlaneView->hasSelectedPart())
    {
        m_pglPlaneView->clearSelectedParts();
        m_pglPlaneView->update();
        bCancelled = true;
    }

    return bCancelled;
}


void PlaneXflDlg::showEvent(QShowEvent *pEvent)
{
    PlaneDlg::showEvent(pEvent);

    restoreGeometry(s_XflGeometry);

    if(s_HSplitterSizes.length()>0)  m_pHSplitter->restoreState(s_HSplitterSizes);
    if(s_PartSplitterSizes.length()>0)  m_pPartSplitter->restoreState(s_PartSplitterSizes);

    m_pdeStitchPrecision->setValue(s_StitchPrecision*Units::mtoUnit());

    onResizeColumns();
}


void PlaneXflDlg::resizeEvent(QResizeEvent *pEvent)
{
    PlaneDlg::resizeEvent(pEvent);
    onSplitterMoved(0,0);
}


void PlaneXflDlg::hideEvent(QHideEvent *pEvent)
{
    readParams();

    m_pPlane = nullptr;

    s_HSplitterSizes = m_pHSplitter->saveState();
    s_PartSplitterSizes = m_pPartSplitter->saveState();

    s_iActivePage = m_pLeftTabWidget->currentIndex();

    s_XflGeometry = saveGeometry();

    PlaneDlg::hideEvent(pEvent);
}


void PlaneXflDlg::readParams()
{
    if(!m_pPlane) return;
    s_StitchPrecision  = m_pdeStitchPrecision->value()/Units::mtoUnit();
    s_bParallelCut = m_pchCutParallel->isChecked();
}


void PlaneXflDlg::onOK(int iExitCode)
{
    for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
        m_pPlaneXfl->wing(iw)->setVisible(true);
    for(int ifuse=0; ifuse<m_pPlaneXfl->nFuse(); ifuse++)
        m_pPlaneXfl->fuse(ifuse)->setVisible(true);

    m_pPlaneXfl->setName(m_pleName->text());
    m_pPlaneXfl->setDescription(m_pleDescription->toPlainText());

    m_pPlaneXfl->makeTriMesh(true);

    done(iExitCode);
}


void PlaneXflDlg::onUpdatePlaneProps()
{
    if(!m_pPlane) return;

    QString strange, str1, str2;
    str1 = QString::asprintf("Quads          = %5d\n", m_pPlaneXfl->nPanel4());
    str2 = QString::asprintf("Triangles      = %5d",   m_pPlaneXfl->nPanel3());
    strange = m_pPlaneXfl->planeData(true) + "\n" + str1 + str2;

    m_pglPlaneView->setBotLeftOutput(strange);
}


void PlaneXflDlg::updateOutput(QString const &msg)
{
    m_ppto->onAppendThisPlainText(msg);
}


void PlaneXflDlg::onSplitterMoved(int , int)
{
    onResizeColumns();
}


void PlaneXflDlg::onPlaneInertia()
{
    PlaneXflInertiaDlg dlg(this);
    dlg.hideSaveAsNew();
    dlg.initDialog(m_pPlaneXfl);
    dlg.exec();

    m_bChanged = true;
    m_pglPlaneView->update();
}


void PlaneXflDlg::onInsertFuseXml()
{
    QString PathName;
    PathName = QFileDialog::getOpenFileName(this, "Open XML file",
                                            SaveOptions::xmlPlaneDirName(),
                                             "Fuse XML file (*.xml)");
    if(!PathName.length()) return;

    QFile XFile(PathName);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange =  "Could not open the file\n" +PathName;
        QMessageBox::warning(this, "Warning", strange);
        return;
    }

    XmlFuseReader fusereader(XFile);
    fusereader.readXMLFuseFile();
    XFile.close();

    if(fusereader.hasError())
    {
        QString errorMsg = fusereader.errorString() + QString("\nline %1 column %2").arg(fusereader.lineNumber()).arg(fusereader.columnNumber());
        QMessageBox::warning(this, "XML read", errorMsg, QMessageBox::Ok);
        //        m_pBody->duplicate(&memBody);
        return;
    }

    Fuse *pFuse = fusereader.fuseXfl();
    pFuse->makeFuseGeometry();
    QString logmsg;
    pFuse->makeDefaultTriMesh(logmsg, QString());

    m_pPlaneXfl->addFuse(pFuse);

    setControls();
    updateData();

    onUpdatePlane();
    m_bChanged = true;
}


void PlaneXflDlg::onImportOtherFuse()
{
    ImportObjectDlg dlg(this, false);
    if(dlg.exec()==QDialog::Accepted)
    {
        Plane const*pPlane = Objects3d::planeAt(dlg.planeName());
        if(!pPlane) return;
        for(int ifuse=0; ifuse<pPlane->nFuse(); ifuse++)
        {
            if(pPlane->fuseAt(ifuse)->name()==dlg.objectName())
            {
                Fuse *pFuse = pPlane->fuseAt(ifuse)->clone();
                m_pPlaneXfl->addFuse(pFuse);
                setControls();
                updateData();

                onUpdatePlane();
                m_bChanged = true;
                break;
            }
        }
    }
}


void PlaneXflDlg::onImportOtherWing()
{
    ImportObjectDlg dlg(this, true);
    if(dlg.exec()==QDialog::Accepted)
    {
        Plane const*pPlane = Objects3d::planeAt(dlg.planeName());
        if(!pPlane) return;
        for(int iw=0; iw<pPlane->nWings(); iw++)
        {
            QString wingname = dlg.objectName();
            WingXfl const *pWing= pPlane->wingAt(iw);
            if(pWing->name()==wingname)
            {
                WingXfl *pNewWing = new WingXfl();
                pNewWing->duplicate(pWing);
                m_pPlaneXfl->addWing(pNewWing);
                setControls();
                updateData();

                onUpdatePlane();
                m_bChanged = true;
                break;
            }
        }
    }
}


void PlaneXflDlg::onInsertFuseOcc()
{
    double dimension(0);
    QString logmsg;

    updateOutput("Importing CAD file...\n");

    QString filter = "CAD Files (*.brep *stp *step *igs *iges)";
    QString filename = QFileDialog::getOpenFileName(nullptr, "CAD File", SaveOptions::CADDirName(), filter);
    if(!filename.length()) return;
    QFileInfo fi(filename);

    FuseOcc *pFuseOcc = new FuseOcc;
    pFuseOcc->setPartName(fi.baseName());

    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool bImport = occ::importCADShapes(filename, pFuseOcc->shapes(), dimension, s_StitchPrecision, logmsg, this);
    updateOutput(logmsg+"\n");

    if(!bImport)
    {
        delete pFuseOcc;
        QApplication::restoreOverrideCursor();
        return;
    }

    m_pPlaneXfl->addFuse(pFuseOcc);

    m_ppto->appendPlainText("---Making shells from shapes-----\n");

    pFuseOcc->makeShellsFromShapes();

    if(pFuseOcc->shellCount()==0)
    {
        QString strange  = "Imported SHAPE does not contain any SHELL: use the fuselage editor to fix the shapes.\n";
        updateOutput(strange);
    }
    else
    {
        m_ppto->appendPlainText("---Making shell triangulation-----\n");
        logmsg.clear();
        updateOutput("Making shell triangulation\n");
        pFuseOcc->makeShellTriangulation(logmsg, "   ");
        pFuseOcc->saveBaseTriangulation();
        pFuseOcc->computeSurfaceProperties(logmsg, "   ");
        updateOutput(logmsg+"\n");
        m_ppto->appendPlainText("---updating plane-----\n");
        onUpdatePlane();
    }

    setControls();

    updateData();

    QApplication::restoreOverrideCursor();
    m_bChanged = true;
}


void PlaneXflDlg::onInsertFuseStl()
{
    StlReaderDlg dlg(this);
    if(dlg.exec()==QDialog::Rejected)
    {
        return;
    }

    if(dlg.triangleList().size()==0)
    {
        updateOutput("STL import: triangle list is empty\n");
        return; // nothing imported
    }

    updateOutput(dlg.logMsg());

    Fuse *pFuse = m_pPlaneXfl->makeNewFuse(Fuse::Stl);

    QString strong;
    strong = QString::asprintf("STL_type_fuse_%d", m_pPlaneXfl->stlFuseCount());
    pFuse->setPartName(strong);

    pFuse->setBaseTriangles(dlg.triangleList());
    pFuse->setModTriangles(dlg.triangleList());
    pFuse->makeTriangleNodes();
    pFuse->makeNodeNormals();

    QString logmsg;
    updateOutput("   Making mesh panels from stl triangles\n");
    pFuse->makeDefaultTriMesh(logmsg, QString());
    updateOutput(logmsg);

    setControls();

    updateData();

    onUpdatePlane();

    m_bChanged = true;
}


void PlaneXflDlg::onInsertSTLCylinderFuse()
{
    DoubleValueDlg dlg(this, {2.0*Units::mtoUnit(), 0.1*Units::mtoUnit(), 37, 31}, {"Length", "Radius", "Ny", "NHoop"},
                       {Units::lengthUnitLabel(), Units::lengthUnitLabel(), QString(), QString()});
    if(dlg.exec()!=QDialog::Accepted) return;

    double length = dlg.value(0)/Units::mtoUnit();
    double radius = dlg.value(1)/Units::mtoUnit();
    int ny = int(dlg.value(2));
    int nh = int(dlg.value(3));

    QVector<Triangle3d> triangles;
    // use the AF mesher to mesh the lateral faces

    nh = std::max(nh, 3);
    ny = std::max(ny, 2);

    QVector<Node>facepts(nh);

    double ymin = -length/2.0;
    double dy = length/double(ny);

    double theta = 2.0*PI/double(nh);
    QVector<Node> node((ny+1)*nh);
    int in = 0;
    for(int iy=0; iy<=ny; iy++)
    {
        double diy = double(iy);
        double dTh = (iy%2==0) ? 0.0 : theta/2.0;
        for(int ih=0; ih<nh; ih++)
        {
            double di = double(ih);
            node[in].setPosition(radius*cos(theta*di + dTh), ymin + diy*dy, radius*sin(theta*di+dTh));
            node[in].setNormal(node[in]);
            in++;
        }
        if(iy==0)
        {
            for(int i=0; i<nh; i++)
            {
                facepts[i] = node[i];
                facepts[i].setNormal(0.0,-1.0,0.0);
            }
        }
    }

    int it = 0;
    int nLA=0, nLB=0, nTA=0, nTB=0;
    triangles.resize(nh*ny*2);
    for(int i=0; i<ny; i++)
    {
        for(int h=0; h<nh; h++)
        {
            nLA = i*nh + (h+1)%nh;      // wrap around
            nLB = i*nh + (h+1)%nh + nh; // wrap around
            nTA = i*nh +  h;
            nTB = i*nh +  h+nh;
            if(i%2==0)
            {
                // left triangle
                triangles[it++].setTriangle(node.at(nLA), node.at(nTA), node.at(nTB));
                //right triangle
                triangles[it++].setTriangle(node.at(nLB), node.at(nLA), node.at(nTB));
            }
            else
            {
                // left triangle
                triangles[it++].setTriangle(node.at(nLA), node.at(nTA), node.at(nLB));
                //right triangle
                triangles[it++].setTriangle(node.at(nLB), node.at(nTA), node.at(nTB));
            }
        }
    }

    //left tip patch - no simple algorithm, using the advancing front mesher
    TopoDS_Face face;
    QString strange;
    occ::makeFaceFromNodeStrip(facepts, face, strange);


    SLG3d slg;
    slg.resize(nh);
    for(int i=0; i<facepts.size(); i++)
    {
        slg[i].setNodes(facepts.at(i), facepts.at((i+1)%nh));
    }

    AFMesher mesher;
    int maxpanelcount = 1000;
    double maxedgelength = radius*2.0*PI/double(nh);
    mesher.setMaxIterations(1000);
    QVector<Triangle3d> facetrianglesleft, facetrianglesright;
    mesher.makeTriangles(face, slg, facetrianglesleft, maxedgelength, maxpanelcount, strange);

    int nFlips=0, nIter=0;
    mesher.makeDelaunayFlips(facetrianglesleft, nFlips, nIter);

    facetrianglesright = facetrianglesleft;
    Vector3d O, axis(0.0,1.0,0.0);
    for(int i=0; i<facetrianglesleft.size(); i++)
    {
        Triangle3d &t3 = facetrianglesright[i];
        t3.translate(0.0, length, 0.0);
        t3.normal().reverse();
        if((ny%2)==1) t3.rotate(O, axis, (theta/2.0)*180.0/PI);
        t3.reverseOrientation();
//        for(int in=0; in<3; in++) t3.nodeNormal(in).reverse();
    }
    triangles.append(facetrianglesleft);
    triangles.append(facetrianglesright);

    Fuse *pFuse = m_pPlaneXfl->makeNewFuse(Fuse::Stl);
    pFuse->setColor(QColor(235,155,105));
    QString strong;
    strong = QString::asprintf("Cylinder_STL_fuse_%d", m_pPlaneXfl->stlFuseCount());
    pFuse->setPartName(strong);

    pFuse->setBaseTriangles(triangles);
    pFuse->setModTriangles(triangles);
    pFuse->makeTriangleNodes();
    pFuse->makeNodeNormals();

    QString logmsg;
    updateOutput("   Making mesh panels from stl triangles\n");
    pFuse->makeDefaultTriMesh(logmsg, QString());
    updateOutput(logmsg);

    setControls();

    updateData();

    onUpdatePlane();

    m_bChanged = true;
}

void PlaneXflDlg::onInsertEllipticWing()
{
    DoubleValueDlg dlg(this, {0.3*Units::mtoUnit(), 3.0*Units::mtoUnit(), 15},
                             {"Root chord:", "Span:", "N Panels:"},
                             {Units::lengthUnitLabel(), Units::lengthUnitLabel(), QString()});
    if(dlg.exec()!=QDialog::Accepted)  return;

    double a = dlg.value(0) / Units::mtoUnit();       // root chord
    double b = dlg.value(1) / Units::mtoUnit() / 2.0; // half span
    int nsecs = int(dlg.value(2));
    nsecs = std::max(2, nsecs);    //at least two sections

    WingXfl *pWing = new WingXfl;
    pWing->clearWingSections();
    QVector<double> frac;
    xfl::getPointDistribution(frac, nsecs, xfl::INV_EXP);
    for(int i=0; i<frac.size(); i++)
    {
        double r = frac.at(i);
        double bs = b *r;
        double as = a* sqrt(1.0-bs*bs/b/b);
        as = std::max(as, a/20);
        pWing->appendWingSection(as, 0.0, bs, 0, (a-as)/2.0, 5, 1, xfl::COSINE, xfl::UNIFORM, QString(), QString());
    }

    if(!m_pPlaneXfl->hasMainWing()) pWing->setWingType(WingXfl::Main);
    else                            pWing->setWingType(WingXfl::OtherWing);
    m_pPlaneXfl->addWing(pWing);
    pWing->setVisible(true);

    updateData();
    onUpdatePlane();
}


void PlaneXflDlg::onInsertSTLSphereFuse()
{
    IntValueDlg dlg(this);
    dlg.setValue(2);
    dlg.setLeftLabel("Number of icosahedron splits (0<=n<=4)");
    if(dlg.exec()!=QDialog::Accepted) return;

    int nSplit = std::min(dlg.value(), 4);
    nSplit = std::max(0, nSplit);

    QVector<Triangle3d> triangles;
    makeSphere(1.0, nSplit, triangles);

    Fuse *pFuse = m_pPlaneXfl->makeNewFuse(Fuse::Stl);

    QString strong;
    strong = QString::asprintf("Sphere_STL_fuse_%d", m_pPlaneXfl->stlFuseCount());
    pFuse->setPartName(strong);

    pFuse->setBaseTriangles(triangles);
    pFuse->setModTriangles(triangles);
    pFuse->makeTriangleNodes();
    pFuse->makeNodeNormals();

    QString logmsg;
    updateOutput("   Making mesh panels from STL triangles\n");
    pFuse->makeDefaultTriMesh(logmsg, QString());
    updateOutput(logmsg);

    setControls();

    updateData();
    onUpdatePlane();

    m_bChanged = true;
}


void PlaneXflDlg::onInsertWingFromXml()
{
    WingXfl *pNewWing = importWingFromXML();
    if(!pNewWing) return;

    m_pPlaneXfl->addWing(pNewWing);

    updateData();
    onUpdatePlane();
}


void PlaneXflDlg::onInsertWingFromVSP()
{
    QStringList filters;
    filters << "CSV text file (*.csv)";

    QString pathname = QFileDialog::getOpenFileName(this,  "OpenVSP csv definition file",
                                                   SaveOptions::lastDirName(),
                                                   filters.first());

    if(pathname.isEmpty()) return;
    QFileInfo fi(pathname);

    if(!fi.exists() || fi.suffix().compare("csv", Qt::CaseInsensitive)!=0)
    {
        m_ppto->onAppendThisPlainText("Invalid CSV file - aborting\n");
        return;
    }

    QFile VSPFile(pathname);
    if(!VSPFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_ppto->onAppendThisPlainText("Could not read the file - aborting\n");
        return;
    }

    SaveOptions::setLastDirName(fi.absolutePath());

    QStringList wingnames;
    QTextStream instream(&VSPFile);

    QString strVSP = instream.readAll(); // need a copy to parse multiple times
    VSPFile.close();

    QTextStream stream(&strVSP);
    QString strange;
    do
    {
        strange = stream.readLine();
        if(strange.isNull()) break;
        if(strange.contains("Geom Name", Qt::CaseSensitive))
        {
            QStringList fields = strange.split(",");
            if(fields.count()>=2)
            {
                if(!wingnames.contains(fields.at(1)))
                    wingnames.append(fields.at(1));
            }
        }

    }while (!strange.isNull());


    m_ppto->onAppendThisPlainText(QString::asprintf("Found %d wings:\n", int(wingnames.size())));
    for(int i=0; i<wingnames.size(); i++)
    {
        m_ppto->onAppendThisPlainText(QString("   ")+wingnames.at(i)+"\n");
    }
/*    QVector<Wing*> wings;
    for(int i=0; i<wingnames.size(); i++)
    {
        wings.append(new WingXfl(WingXfl::OtherWing));
        wings.last()->setName(wingnames.at(i));
    }
*/

    //read all the foil names
    stream.seek(0);
    QStringList airfoilfilenames;
    do
    {
        strange = stream.readLine();
        if(strange.isNull()) break;
        if(strange.contains("Airfoil File Name", Qt::CaseSensitive))
        {
            QStringList fields = strange.split(",");
            if(fields.count()>=2)
            {
                if(!airfoilfilenames.contains(fields.at(1)))
                    airfoilfilenames.append(fields.at(1));
            }
        }

    }while (!strange.isNull());

    m_ppto->onAppendThisPlainText(QString::asprintf("Found %d airfoil files to load:\n", int(airfoilfilenames.size())));
    for(int i=0; i<airfoilfilenames.size(); i++)
    {
        m_ppto->onAppendThisPlainText(QString("   ")+airfoilfilenames.at(i)+"\n");
    }


    QStringList filter = {"*.dat"};
    QStringList files = xfl::findFiles(fi.absolutePath(), filter, false);
    for(int i=0; i<files.size(); i++)
    {
        Foil *pFoil = new Foil;
        if(readVSPFoilFile(files.at(i), pFoil))
            Objects2d::insertThisFoil(pFoil);
        else
            delete  pFoil;
    }

/*    do
    {
        strange = stream.readLine();
        if(strange.isNull()) break;
        if(strange.contains("########################################", Qt::CaseSensitive))
        {
            QString wingname;
            int index(0);
            WingSection *ws = new WingSection;
            readVSPSection(stream, wingname, index, ws);
        }

    }while (!strange.isNull());*/
}


void PlaneXflDlg::readVSPSection(QTextStream &stream, QString &wingname, int &index, WingSection &ws)
{
    QString strange;
    do
    {
        strange = stream.readLine();
        QStringList fields = strange.split(",");
        if(fields.count()>2)
        {
            if     (fields.first().contains("Geom Name"))          wingname = fields.at(1);
            else if(fields.first().contains("Airfoil Index"))      index = fields.at(1).toInt();
            else if(fields.first().contains("Leading Edge Point") && fields.size()==4)
            {
//                ws.setOffset(fields.at(1).toDouble(), fields.at(1).toDouble(), fields.at(3).toDouble());
            }
            else if(fields.first().contains("Airfoil File Name"))
            {
                ws.setLeftFoilName(fields.at(1));
                ws.setRightFoilName(fields.at(1));
            }
        }
    }
    while(!strange.contains("#######"));
}


void PlaneXflDlg::onInsertWing()
{
    if(!m_pPlaneXfl->hasMainWing()) m_pPlaneXfl->addWing(WingXfl::Main);
    else                            m_pPlaneXfl->addWing(WingXfl::OtherWing);

    updateData();
    onUpdatePlane();
}


void PlaneXflDlg::onInsertElevator()
{
    WingXfl *pWing = m_pPlaneXfl->addWing(WingXfl::Elevator);
    if(pWing)
    {
        pWing->setWingType(WingXfl::Elevator);
        pWing->setTwoSided(true);
        pWing->setPartName("Elevator");
    }
    int n = m_pPlaneXfl->nWings()-1;
    if(n>=0)
    {
        double span = 1.0;
        double chord = 0.2;
        if(m_pPlaneXfl->hasMainWing())
        {
            span = m_pPlaneXfl->planformSpan();
            chord = m_pPlaneXfl->mac();
        }
        if(pWing)
        {
            pWing->scaleChord(chord/2.0);
            pWing->scaleSpan(span/5.0);
        }
        m_pPlaneXfl->wing(n)->m_rx = 0.0;
        m_pPlaneXfl->wing(n)->m_LE.set(span/3.0, 0.0, 0.0);
    }

    updateData();

    onUpdatePlane();
}


void PlaneXflDlg::onInsertFin()
{
    WingXfl *pWing = m_pPlaneXfl->addWing(WingXfl::Fin);
    if(pWing)
    {
        pWing->setWingType(WingXfl::Fin);
        pWing->setTwoSided(false);
        pWing->setPartName("Fin");
    }
    int n = m_pPlaneXfl->nWings()-1;
    if(n>=0)
    {
        double span = 1.0;
        double chord = 0.2;
        if(m_pPlaneXfl->hasMainWing())
        {
            span = m_pPlaneXfl->planformSpan();
            chord = m_pPlaneXfl->mac();
        }
        if(pWing)
        {
            pWing->scaleChord(chord/2.0);
            pWing->scaleSpan(span/5.0);
        }
        m_pPlaneXfl->m_Wing[n].m_rx = -90.0;
        m_pPlaneXfl->m_Wing[n].m_LE.set(span/3.0, 0.0, 0.0);
    }

    updateData();

    onUpdatePlane();
}


void PlaneXflDlg::onUpdatePlane()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    m_pPlaneXfl->makePlane(true, false, true);

    onUpdatePlaneProps();

    m_pglPlaneView->setPlaneReferenceLength(m_pPlane);

    m_pglPlaneView->updatePartFrame(m_pPlane);
    gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->resetgl3dPlane();
    m_pglPlaneView->update();

    QApplication::restoreOverrideCursor();
}


void PlaneXflDlg::onUpdateMesh()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    onClearHighlighted();

    m_pPlaneXfl->makeTriMesh(true);
    m_bChanged = true;

    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();
    onUpdatePlaneProps();

    QApplication::restoreOverrideCursor();
}


void PlaneXflDlg::onUpdateHighlightedPanels()
{
    Fuse *pFuse = m_pPlaneXfl->fuse(0);
    if(!pFuse) return;

    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();
}


void PlaneXflDlg::editWing(WingXfl *pWing, bool bAdvanced)
{
    if(!pWing) return;
    WingXfl saveWing;
    saveWing.duplicate(pWing);

    WingDlg *pWingDlg = nullptr;
    if(!bAdvanced) pWingDlg = new WingDefDlg(this);
    else           pWingDlg = new WingObjectDlg(this);
    pWingDlg->hideSaveAsNew();

    pWingDlg->initDialog(pWing);

    if(pWingDlg->exec()==QDialog::Accepted)
    {
        m_bChanged |= pWingDlg->bChanged();
        m_bDescriptionChanged |= pWingDlg->bDescriptionChanged();
        onUpdatePlane();
    }
    else   pWing->duplicate(saveWing);

    delete pWingDlg;
}


void PlaneXflDlg::editFuse(int iFuse, bool bAdvanced)
{
    Fuse *pFuse = m_pPlaneXfl->fuse(iFuse);

    if(pFuse->isSplineType() || pFuse->isFlatFaceType())
    {
        FuseXfl *pFuse = dynamic_cast<FuseXfl*>(m_pPlaneXfl->fuse(iFuse));
        Fuse* pMemBody = pFuse->clone();

        FuseXflDlg *pXflFuseDlg = nullptr;
        if(bAdvanced) pXflFuseDlg = new FuseXflObjectDlg(this);
        else          pXflFuseDlg = new FuseXflDefDlg(this);

        pXflFuseDlg->enableName(false);
        pXflFuseDlg->initDialog(pFuse);
        pXflFuseDlg->hideSaveAsNew();
        if(pXflFuseDlg->exec() != QDialog::Accepted)
        {
            pFuse->duplicateFuse(*pMemBody);
            delete pMemBody;
            delete pXflFuseDlg;
            return;
        }
        m_bChanged |= pXflFuseDlg->bChanged();
        m_bDescriptionChanged |= pXflFuseDlg->bDescriptionChanged();
        delete pMemBody;
        delete pXflFuseDlg;
    }
    else if(pFuse->isSectionType())
    {
        FuseSections memBody;
        memBody.duplicateFuse(*pFuse);
        FuseSections* pFuseSections = dynamic_cast<FuseSections*>(pFuse);

        FuseXflDlg *pXflFuseDlg = nullptr;
        pXflFuseDlg = new FuseXflDefDlg(this);

        pXflFuseDlg->enableName(false);
        pXflFuseDlg->initDialog(pFuseSections);
        pXflFuseDlg->hideSaveAsNew();
        if(pXflFuseDlg->exec() != QDialog::Accepted)
        {
            pFuseSections->duplicateFuse(memBody);
            delete pXflFuseDlg;
            return;
        }
        m_bChanged |= pXflFuseDlg->bChanged();
        m_bDescriptionChanged |= pXflFuseDlg->bDescriptionChanged();
        delete pXflFuseDlg;
    }
    else if(pFuse->isOccType())
    {
        FuseOcc *pOccBody = dynamic_cast<FuseOcc*>(pFuse);
        FuseOcc memBody(*pOccBody);
        FuseOccDlg obDlg(this);
        obDlg.hideSaveAsNew();
        obDlg.initDialog(pOccBody);
        if(obDlg.exec() != QDialog::Accepted)
        {
            pOccBody->duplicateFuse(memBody);
            return;
        }
        m_bChanged |= obDlg.bChanged();
        m_bDescriptionChanged |= obDlg.bDescriptionChanged();
    }
    else if(pFuse->isStlType())
    {
        FuseStl *pStlFuse = dynamic_cast<FuseStl*>(pFuse);
        FuseStl memFuseStl(*pStlFuse);
        FuseStlDlg sbDlg(this);
        sbDlg.hideSaveAsNew();

        sbDlg.initDialog(pStlFuse);
        if(sbDlg.exec() != QDialog::Accepted)
        {
            pStlFuse->duplicateFuse(memFuseStl);
            return;
        }
        m_bChanged |= sbDlg.bChanged();
        m_bDescriptionChanged |= sbDlg.bDescriptionChanged();
    }

/*    QString strange;
    if(pFuse) pFuse->makeDefaultTriMesh(strange, QString());*/

    onUpdatePlane();
}



void PlaneXflDlg::scaleWing(WingXfl *pWing)
{
    WingScaleDlg dlg(this);
    dlg.initDialog(pWing->planformSpan(),
                   pWing->chord(0),
                   pWing->averageSweep(),
                   pWing->twist(pWing->sectionCount()-1),
                   pWing->planformArea(),
                   pWing->aspectRatio(),
                   pWing->taperRatio());

    if(QDialog::Accepted == dlg.exec())
    {
        if (dlg.m_bSpan || dlg.m_bChord || dlg.m_bSweep || dlg.m_bTwist || dlg.m_bArea || dlg.m_bAR)
        {
            if(dlg.m_bSpan)  pWing->scaleSpan(dlg.m_NewSpan);
            if(dlg.m_bChord) pWing->scaleChord(dlg.m_NewChord);
            if(dlg.m_bSweep) pWing->scaleSweep(dlg.m_NewSweep);
            if(dlg.m_bTwist) pWing->scaleTwist(dlg.m_NewTwist);
            if(dlg.m_bArea)  pWing->scaleArea(dlg.m_NewArea);
            if(dlg.m_bAR)    pWing->scaleAR(dlg.m_NewAR);
            if(dlg.m_bTR)    pWing->scaleTR(dlg.m_NewTR);
        }

        m_bChanged = true;

        updateData();

        onUpdatePlane();
    }
}


void PlaneXflDlg::scaleFuse(Fuse*pFuse)
{
    if(!pFuse) return;

    DoubleValueDlg dlg(this, {1.0}, {"Scale factor:"}, {QString()});
    if(dlg.exec()==QDialog::Accepted)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        pFuse->scale(dlg.value(0), dlg.value(0), dlg.value(0));

        updateData();

        onUpdatePlane();

        m_bChanged = true;
        QApplication::restoreOverrideCursor();
    }
}


void PlaneXflDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save)    == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
    else if(pButton==m_ppbSaveAsNew)                                      onOK(10);
}


void PlaneXflDlg::onHighlightSel(bool bSel)
{
    gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->highlightSelectedPart(bSel);
    m_pglPlaneView->update();
}


void PlaneXflDlg::onNamesChanged()
{
    m_bDescriptionChanged = true;
    m_pglPlaneView->updatePartFrame(m_pPlane);
}


void PlaneXflDlg::onTabChanged(int iNewTab)
{
    onClearHighlighted();
    if(m_ppbMoveNode->isChecked())
    {
        m_ppbMoveNode->setChecked(false);
        onMergeNodes(false);
    }

    switch(iNewTab)
    {
        case 1:
        {
            onResizeColumns();
            break;
        }
        case 2:
        {
            //fill the tables
            m_plwWingListWt->clear();
            m_plwFuseListWt->clear();
            for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
            {
                m_plwWingListWt->addItem(m_pPlaneXfl->wing(iw)->name());
            }
            for(int ifuse=0; ifuse<m_pPlaneXfl->nFuse(); ifuse++)
            {
                m_plwFuseListWt->addItem(m_pPlaneXfl->fuse(ifuse)->name());
            }
            onListClick();
            break;
        }
        case 3:
        {
            // initialize the flow5 mesher
            Fuse *pFuse =nullptr;
            // get the fuse name currently selected in the assembly table, if any
            QListWidgetItem *pItem = m_plwFuseListWt->currentItem();
            if(pItem)
            {
                QString fusename = pItem->text();
                pFuse = m_pPlaneXfl->fuse(fusename);
            }
            if(!pFuse)
            {
                // get the firtst fuse, if any
                pFuse = m_pPlaneXfl->fuse(0);
            }

            if(!pFuse)
            {
                m_pMesherWt->setEnabled(false);
            }
            else
            {
                m_pMesherWt->setEnabled(true);
                FuseXfl *pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);
                if(pFuseXfl)
                    m_pMesherWt->initDialog(pFuseXfl->rightSideShells(), pFuse->maxElementSize(), true, false);
                else
                    m_pMesherWt->initDialog(pFuse->shells(), pFuse->maxElementSize(), false, false);
                AFMesher::setTraceFaceIndex(-1);
            }
            break;
        }
        case 4:
        {
            Fuse *pFuse =nullptr;
            // get the fuse name currently selected in the assembly table, if any
            QListWidgetItem *pItem = m_plwFuseListWt->currentItem();
            if(pItem)
            {
                QString fusename = pItem->text();
                pFuse = m_pPlaneXfl->fuse(fusename);
            }
            if(!pFuse)
            {
                // get the firtst fuse, if any
                pFuse = m_pPlaneXfl->fuse(0);
            }

            m_UndoStack.clear();
            m_pMeshCorrectionsFrame->setEnabled(pFuse);
            if(pFuse) takePicture();

            break;
        }
    }

    m_ppbCheckMenuBtn->setChecked(false);
    m_ppbMoveNode->setChecked(false);
    m_ppbSelectPanels->setChecked(false);
    m_ppbMakeP3->setChecked(false);
    m_ppbMakeP3Strip->setChecked(false);

    m_pglPlaneView->clearSelectedParts();
    m_pglPlaneView->update();
}


void PlaneXflDlg::onListClick()
{
    QList<QListWidgetItem*> selectedtools = m_plwWingListWt->selectedItems();
//    QList<QListWidgetItem*> selectedfuse = m_plwFuseListWt->selectedItems();

    Fuse *pFuse = nullptr;
    for(int ifuse=0; ifuse<m_plwFuseListWt->count(); ifuse++)
    {
        QListWidgetItem *pItem = m_plwFuseListWt->item(ifuse);
        if(pItem && pItem->isSelected())
        {
            pFuse = m_pPlaneXfl->fuse(ifuse);
            break;
        }
    }

    m_ppbCutFuse->setEnabled(pFuse && selectedtools.size()>0);
    m_ppbFuseMenuButton->setEnabled(pFuse);
}


void PlaneXflDlg::onPartListClicked(QModelIndex)
{
    m_pglPlaneView->clearSelectedParts();
    for(int iw=0; iw<m_plwWingListWt->count(); iw++)
    {
        QListWidgetItem *pItem = m_plwWingListWt->item(iw);
        if(pItem && pItem->isSelected())
        {
            WingXfl *pWing = m_pPlaneXfl->wing(iw);
            m_pglPlaneView->setSelectedPart(pWing->uniqueIndex());
        }
    }
    for(int ifuse=0; ifuse<m_plwFuseListWt->count(); ifuse++)
    {
        QListWidgetItem *pItem = m_plwFuseListWt->item(ifuse);
        if(pItem && pItem->isSelected())
        {
            Fuse const *pFuse = m_pPlaneXfl->fuse(ifuse);
            m_pglPlaneView->setSelectedPart(pFuse->uniqueIndex());
        }
    }
    m_pglPlaneView->update();
}


/**
 * Cuts the left and right shells with each of the wings and updates the two half shells.
 */
void PlaneXflDlg::onCutFuse()
{
    QString strange;

    readParams();

    // reset the shells
    Fuse *pFuse = nullptr;
    Vector3d fusepos;
    for(int ifuse=0; ifuse<m_plwFuseListWt->count(); ifuse++)
    {
        QListWidgetItem *pItem = m_plwFuseListWt->item(ifuse);
        if(pItem && pItem->isSelected())
        {
            pFuse = m_pPlaneXfl->fuse(ifuse);
            fusepos = m_pPlaneXfl->fusePos(ifuse);
            break;
        }
    }
    if(!pFuse)
    {
        updateOutput("No fuse selected.\n\n");
        return;
    }

    s_StitchPrecision = m_pdeStitchPrecision->value()/Units::mtoUnit();
    strange = QString::asprintf("Stitching faces with precision %g ", s_StitchPrecision*Units::mtoUnit());
    strange += Units::lengthUnitLabel() + "\n";
    updateOutput(strange);

    TopTools_ListOfShape WingShapeList;
    WingShapeList.Clear();

//    int iWing=0;
    TopoDS_Shape WingShape;
    for(int iw=0; iw<m_plwWingListWt->count(); iw++)
    {
        QListWidgetItem *pItem = m_plwWingListWt->item(iw);
        if(pItem && pItem->isSelected())
        {
            QString logmsg;
            WingXfl const *pWing = m_pPlaneXfl->wingAt(iw);

            // check foils
            QStringList foilist;
            for(int is=0; is<pWing->sectionCount(); is++)
            {
                foilist.append(pWing->section(is).leftFoilName());
                foilist.append(pWing->section(is).rightFoilName());
            }
            foilist.removeDuplicates();

            bool bOpen = false;
            for(int ifoil=0; ifoil<foilist.size(); ifoil++)
            {
                Foil const *pFoil = Objects2d::foil(foilist.at(ifoil));
                if(!pFoil)
                {
                    logmsg += "   Missing foil" +foilist.at(ifoil)+" in wing " + pWing->name() + "\n";
                    logmsg += "   ... Aborting operation\n";
                    updateOutput(logmsg);
                    return;
                }

                if(fabs(pFoil->TEGap())>1.e-6)
                {
                    logmsg += "   The foil " + foilist.at(ifoil) + " has an open trailing edge\n";
                    bOpen = true;
                    break;
                }
            }
            if(bOpen)
            {
                logmsg += "   Cannot make a solid of wing " + pWing->name() + " due to its open trailing edge\n";
//                iWing++;
                updateOutput(logmsg);
                return;
            }

            occ::makeWingShape(pWing, s_StitchPrecision, WingShape, logmsg);
            updateOutput(logmsg);

            if(!pWing->isTwoSided() && pWing->isClosedInnerSide())
            {
                logmsg  = "   WARNING: " + pWing->name() + " is one-sided and has a closed right tip\n";
                logmsg += "            the resulting mesh may be self intersecting.\n\n";
                updateOutput(logmsg);
            }

            if(!WingShape.IsNull())
            {
                WingShapeList.Append(WingShape);
            }

//            iWing++;
        }
    }

    if(WingShapeList.Size()<1)
    {
        updateOutput("   No wing selected, no cut to perform\n");
        return;
    }

    //restore the shells before attempting to cut
    if(pFuse->isXflType())
    {
        FuseXfl *pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);
        pFuseXfl->makeShape(strange); // will build both the shapes and the shells
    }
    else
        pFuse->makeShellsFromShapes();

    if(pFuse->isXflType())
    {
        cutFuseXflRightShapes(pFuse, fusepos, WingShapeList); //saves 1/2 the cutting time
    }
    else
    {
        cutFuseShapes(pFuse, fusepos, WingShapeList);
    }

    if(pFuse->shells().Extent())
    {
    }
    else
    {
        updateOutput("Restoring the uncut shells\n");
        pFuse->makeShellsFromShapes();
    }

    pFuse->makeDefaultTriMesh(strange, QString()); // just for xfl and stl

    m_pPlaneXfl->makeTriMesh(true);

    gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->resetgl3dFuse();
    m_pglPlaneView->update();
    m_bChanged = true;
}


void PlaneXflDlg::cutFuseShapes(Fuse *pFuse, Vector3d const &fusepos, TopoDS_ListOfShape &tools)
{
    QString strong, strange;
    if(pFuse->shapeCount()<=0)
    {
        updateOutput("   Fuse has no topology shapes to cut.\n");
        return;
    }
    pFuse->makeShellsFromShapes();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    updateOutput("Cutting body shape with selected wings...\n");

    // make cutting list
   TopTools_ListIteratorOfListOfShape itwing;

    // translate the wings by -fusepos
    if(fabs(fusepos.x)>LENGTHPRECISION || fabs(fusepos.y)>LENGTHPRECISION || fabs(fusepos.z)>LENGTHPRECISION)
    {
        gp_Trsf Translation;
        Translation.SetTranslation(gp_Vec(-fusepos.x, -fusepos.y, -fusepos.z));
        BRepBuilderAPI_Transform thetranslator(Translation);

        for(TopTools_ListIteratorOfListOfShape shapeit(tools); shapeit.More(); shapeit.Next())
        {
            thetranslator.Perform(shapeit.Value(), Standard_True);
            shapeit.Value() = thetranslator.Shape();
        }
    }

    pFuse->clearShells();

    QElapsedTimer t; t.start();
    // BRepAlgoAPI_Cut sometimes fails when there is more than one shape in the arguments
    for(TopTools_ListIteratorOfListOfShape shellit(pFuse->shapes()); shellit.More(); shellit.Next())
    {
//        TopoDS_ListOfShape singleshell;
//        singleshell.Append(shellit.Value());

        TopoDS_ListOfShape faces;
        TopExp_Explorer shellexplorer;
        for (shellexplorer.Init(shellit.Value(), TopAbs_SHELL); shellexplorer.More(); shellexplorer.Next())
        {
            faces.Append(shellexplorer.Current());
        }

        BRepAlgoAPI_Cut theKnife;
        theKnife.SetArguments(faces);
        theKnife.SetTools(tools);
        theKnife.SetRunParallel(s_bParallelCut);
//        theKnife.SetFuzzyValue(1.e-4);
        strange = QString::asprintf("   Using fuzzy value %g", theKnife.FuzzyValue()*Units::mtoUnit());
        strange += Units::lengthUnitLabel() + "\n";
        updateOutput(strange);

        // run the algorithm
        theKnife.Build();

        if (theKnife.HasErrors() || !theKnife.IsDone())
        {
            updateOutput("Error cutting shape with wings\n");
            QApplication::restoreOverrideCursor();
            return;
        }

        TopoDS_Shape compound = theKnife.Shape();

        TopExp_Explorer compexplorer;
        int iShell=0;

        for (compexplorer.Init(compound, TopAbs_SHELL); compexplorer.More(); compexplorer.Next())
        {
            strange = QString::asprintf("   Cutting shell %d\n", iShell+1);
            updateOutput(strange);

            if(compexplorer.Current().Orientation()==TopAbs_REVERSED)
                pFuse->appendShell(compexplorer.Current().Reversed());
            else
                pFuse->appendShell(compexplorer.Current());

            iShell++;
        }
    }

    strange = QString::asprintf("   Time to cut the fuse: %.3f s\n\n", double(t.elapsed())/1000.0);
    updateOutput(strange);

    strong = QString::asprintf("   Cut operation has produced %d shell(s)\n",pFuse->shells().Extent());
    updateOutput(strong);
    int iShell=0;

    strange.clear();
    for(TopTools_ListIteratorOfListOfShape shellit(pFuse->shells()); shellit.More(); shellit.Next())
    {
        strange += QString::asprintf("   Shell %d:\n", iShell);
        occ::listShapeContent(shellit.Value(), strong, "      ");
        strange += strong;
        iShell++;
    }
    updateOutput(strange+"\n");


    pFuse->clearTriangles();
    pFuse->clearTriangleNodes();

    QString logmsg;

    pFuse->makeShellTriangulation(logmsg, "   ");
    updateOutput(logmsg);

    strong = QString::asprintf("   New triangulation has %d elements\n", pFuse->triangleCount());
    updateOutput(strong+"\n\n");

    QApplication::restoreOverrideCursor();
}


/**
 * In the case of an xfl type fuse, only cut the right shells and duplicate.
 * Saves significant cutting time.
 */
void PlaneXflDlg::cutFuseXflRightShapes(Fuse *pFuse, Vector3d const &fusepos, TopoDS_ListOfShape &tools)
{
    QString strong, strange;
    FuseXfl *pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);

    QString logmsg;
    // remake the shells which may have been modified by a previous cut
//    logmsg = "Making fuse shape\n";
//    pFuseXfl->makeShape(logmsg);

    updateOutput(logmsg+"\n");
    logmsg .clear();

    int nshells = pFuseXfl->m_RightSideShell.Size();
    if(nshells<0)
    {
        updateOutput("   Fuse has no TopoDS_Shell to cut.\n");
        return;
    }

/*    strange = QString::asprintf("   Fuse half-side is made of %d shells\n", nshells);
    updateAssyOutput(strange);

    int ishell=0;
    for(TopTools_ListIteratorOfListOfShape shapeit(pFuseXfl->rightSideShells()); shapeit.More(); shapeit.Next())
    {
        strange = QString::asprintf("   Sub-shell %d:\n", ishell);
        listShapeContent(shapeit.Value(), strange, "      ");
        updateAssyOutput(strange);
        ishell++;
    }
    updateAssyOutput("\n");*/

    QApplication::setOverrideCursor(Qt::WaitCursor);
    updateOutput("Cutting fuse shape with selected wings...");

    // make cutting list
    TopTools_ListOfShape toollist;
    TopTools_ListIteratorOfListOfShape itwing;

    // translate the wings by -fusepos
    if(fabs(fusepos.x)>LENGTHPRECISION || fabs(fusepos.y)>LENGTHPRECISION || fabs(fusepos.z)>LENGTHPRECISION)
    {
        gp_Trsf Translation;
        Translation.SetTranslation(gp_Vec(-fusepos.x, -fusepos.y, -fusepos.z));
        BRepBuilderAPI_Transform thetranslator(Translation);

        for(TopTools_ListIteratorOfListOfShape shapeit(tools); shapeit.More(); shapeit.Next())
        {
            thetranslator.Perform(shapeit.Value(), Standard_True);
            shapeit.Value() = thetranslator.Shape();
        }
    }


    BRepAlgoAPI_Cut theKnife;
    theKnife.SetRunParallel(s_bParallelCut);

//    double fuzz = theKnife.FuzzyValue();
//    theKnife.SetFuzzyValue(1.e-4);
    theKnife.SetArguments(pFuseXfl->rightSideShells());
    theKnife.SetTools(tools);

    QElapsedTimer t; t.start();

    // run the algorithm
    theKnife.Build();

    if (theKnife.HasErrors() || !theKnife.IsDone())
    {
        updateOutput("Error cutting shape with wings\n");
        QApplication::restoreOverrideCursor();
        return;
    }

    updateOutput("   DONE\n");
    strange = QString::asprintf("   Time to cut the fuse: %.3f s\n\n", double(t.elapsed())/1000.0);
    updateOutput(strange);

    TopoDS_Shape compound = theKnife.Shape();
    TopExp_Explorer compexplorer;
    int iShell=0;
    pFuseXfl->clearShells();
    pFuseXfl->clearRightShells();

    for (compexplorer.Init(compound, TopAbs_SHELL); compexplorer.More(); compexplorer.Next())
    {
        strange = QString::asprintf("   shell %d content:\n", iShell+1);
        updateOutput(strange);

        TopoDS_Shell rightsideshell = TopoDS::Shell(compexplorer.Current());
        pFuseXfl->appendRightSideShell(rightsideshell);
        occ::listShapeContent(compexplorer.Current(), strange, "      ");
        updateOutput(strange);
        iShell++;
    }

    strong = QString::asprintf("   Cut operation has produced %d shell(s)\n",iShell);
    updateOutput(strong);

    pFuseXfl->makeShellTriangulation(logmsg, "   ");

    updateOutput(logmsg);

    strong = QString::asprintf("   The new fuse tessellation has %d elements\n", pFuseXfl->triangleCount());
    strong += "\n______\n\n";
    updateOutput(strong);

    QApplication::restoreOverrideCursor();
}


void PlaneXflDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("PlaneXflDlg");
    {
        s_XflGeometry       = settings.value("Geometry", QByteArray()).toByteArray();
        s_HSplitterSizes    = settings.value("HSplitterSizes").toByteArray();
        s_PartSplitterSizes = settings.value("VSplitterSizes").toByteArray();

        s_StitchPrecision   = settings.value("StitchPrecision",   s_StitchPrecision  ).toDouble();
        s_NodeMergeDistance = settings.value("NodeMergeDistance", s_NodeMergeDistance).toDouble();
        s_bParallelCut      = settings.value("ParallelCut",       s_bParallelCut).toBool();
    }
    settings.endGroup();
}


void PlaneXflDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("PlaneXflDlg");
    {
        settings.setValue("Geometry", s_XflGeometry);

        settings.setValue("HSplitterSizes", s_HSplitterSizes);
        settings.setValue("VSplitterSizes", s_PartSplitterSizes);

        settings.setValue("StitchPrecision",   s_StitchPrecision);
        settings.setValue("NodeMergeDistance", s_NodeMergeDistance);
        settings.setValue("ParallelCut",       s_bParallelCut);
    }
    settings.endGroup();
}


WingXfl* PlaneXflDlg::importWingFromXML()
{
    QString path_to_file;
    path_to_file = QFileDialog::getOpenFileName(this,
                                                QString("Open XML File"),
                                                SaveOptions::xmlPlaneDirName(),
                                                "Plane XML file (*.xml)");
    QFile xmlfile(path_to_file);

    if (!xmlfile.open(QIODevice::ReadOnly))
    {
        QString strange = "Could not open the file\n"+xmlfile.fileName();
        QMessageBox::warning(this, "Warning", strange);
        return nullptr;
    }

    XmlWingReader wingreader(xmlfile);
    if(!wingreader.readXMLWingFile())
    {
        QString strong;
        QString errorMsg;
        errorMsg = "Failed to read the file "+path_to_file+"\n";
        strong = QString::asprintf("error on line %d column %d", int(wingreader.lineNumber()), int(wingreader.columnNumber()));
        errorMsg += wingreader.errorString() + " at " + strong;
        QMessageBox::warning(this, "XML read", errorMsg, QMessageBox::Ok);
        return nullptr;
    }

    m_bChanged = true;

    return wingreader.wing();
}


void PlaneXflDlg::onInsertFuseXfl()
{
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    Fuse::enumType type = Fuse::NURBS;
    if     (pSenderAction==m_pInsertFuseXflFlat)     type=Fuse::FlatFace;
    else if(pSenderAction==m_pInsertFuseXflSections) type=Fuse::Sections;

    Fuse *pFuse = m_pPlaneXfl->makeNewFuse(type);
    QString strong;
    strong = QString::asprintf("xfl_fuse_%d", m_pPlaneXfl->xflFuseCount());
    pFuse->setPartName(strong);
    pFuse->makeFuseGeometry();
    QString logmsg;
    pFuse->makeDefaultTriMesh(logmsg, QString());

    setControls();

    updateData();

    onUpdatePlane();

    m_bChanged = true;
//    listPartUniqueIndexes();
}


void PlaneXflDlg::onTessellation()
{
    OccMeshControls dlg;
    Fuse *pFuse = activeFuse();
    if(!pFuse) return;

    dlg.initDialog(pFuse->occMeshParams(), true);
    if(dlg.exec()==QDialog::Accepted)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        updateOutput("Making new fuse tessellation\n");
        pFuse->setOccMeshParams(dlg.occParameters());
        pFuse->clearOccTriangulation();
        m_bChanged = true;
        QString strange;
        pFuse->makeShellTriangulation(strange, "   ");
        updateOutput(strange);

        gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
        pglPlaneXflView->resetgl3dFuse();
        m_pglPlaneView->update();

        QApplication::restoreOverrideCursor();
    }
}


void PlaneXflDlg::onFlipNormals()
{
    Fuse *pFuse = activeFuse();
    if(!pFuse) return;
    pFuse->triangulation().flipNormals();
    gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->resetgl3dFuse();

    m_pglPlaneView->update();
}


void PlaneXflDlg::updateData()
{
    m_pPartModel->updateData();

    QModelIndex index = m_pPartModel->index(m_pPlaneXfl->nWings(), 0);
    m_pcptParts->setCurrentIndex(index);
}


void PlaneXflDlg::onEditPart()
{
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    bool bAdvanced = false;
    if     (pSenderAction == m_pEditPartDef)    bAdvanced = false;
    else if(pSenderAction == m_pEditPartObject) bAdvanced = true;

    int row = selectedPart();
    editPart(row, bAdvanced);
}


void PlaneXflDlg::onResetFuse()
{
    QString strange;

    updateOutput("\n");

    Fuse *pFuse = nullptr;
    Vector3d fusepos;
    for(int ifuse=0; ifuse<m_plwFuseListWt->count(); ifuse++)
    {
        QListWidgetItem *pItem = m_plwFuseListWt->item(ifuse);
        if(pItem && pItem->isSelected())
        {
            pFuse = m_pPlaneXfl->fuse(ifuse);
            fusepos = m_pPlaneXfl->fusePos(ifuse);
            break;
        }
    }

    if(!pFuse)
    {
        updateOutput("No fuse selected, no reset possible.\n\n");
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if(pFuse->isXflType())
    {
        FuseXfl *pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);
        pFuseXfl->makeShape(strange); // will build both the shapes and the shells
    }
    else
        pFuse->makeShellsFromShapes();

    pFuse->makeFuseGeometry();

    pFuse->makeDefaultTriMesh(strange, QString());
    m_pPlaneXfl->makeTriMesh(true);
    gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->resetgl3dFuse();


    strange = QString::asprintf("The fuse %s has been reset.\n", pFuse->name().toStdString().c_str());
    updateOutput(strange+"\n");

    m_pglPlaneView->update();
    QApplication::restoreOverrideCursor();
}


void PlaneXflDlg::editPart(int row, bool bAdvanced)
{
    if(row<m_pPlaneXfl->nWings())
    {
        WingXfl *pWing = m_pPlaneXfl->wing(row);
        editWing(pWing, bAdvanced);
    }
    else if(row>=m_pPlaneXfl->nWings())
    {
        int iFuse = row-m_pPlaneXfl->nWings();
        editFuse(iFuse, bAdvanced);
    }
}


void PlaneXflDlg::onRemovePart()
{
    int row = selectedPart();
    if (row<0 || row>=m_pPlaneXfl->nParts()) return;

    int iFuse = row-m_pPlaneXfl->nWings();
    QString partname;
    if(row>=0 && row<m_pPlaneXfl->nWings())
    {
        partname = m_pPlaneXfl->wing(row)->name();
    }
    else if(row>=m_pPlaneXfl->nWings() && m_pPlaneXfl->hasFuse())
    {
        partname = m_pPlaneXfl->fuse(iFuse)->name();
    }

    if(row>=0 && row<m_pPlaneXfl->nWings())
    {
        m_pPlaneXfl->removeWing(row);
    }
    else if(row>=m_pPlaneXfl->nWings())
    {
        m_pPlaneXfl->removeFuse(iFuse);
    }

    m_pPartMenu->setTitle("Selected part");

    setControls();
    onUpdatePlane();

    m_pPartModel->updateData();
    m_bChanged = true;
}


void PlaneXflDlg::onMovePartDown()
{
    int row = selectedPart();

    //switch row and row+1;
    if(row<m_pPlaneXfl->nWings())
    {
        if(row==m_pPlaneXfl->nWings()-1) return;
        m_pPlaneXfl->swapWings(row, row+1);
    }
    else if(row<m_pPlaneXfl->nFuse())
    {
        if(row==m_pPlaneXfl->nFuse()-1) return;
        int iFuse=row-m_pPlaneXfl->nWings();
        m_pPlaneXfl->swapFuses(iFuse, iFuse+1);
    }

    m_pPartModel->updateData();
    onUpdatePlane();
    m_bChanged = true;
}


void PlaneXflDlg::onMovePartUp()
{
    int row = selectedPart();
    if(row==0) return;

    //switch row and row-1;
    if(row<m_pPlaneXfl->nWings())
    {
        m_pPlaneXfl->swapWings(row, row-1);
    }
    else
    {
        int iFuse=row-m_pPlaneXfl->nWings();
        if(iFuse>0) m_pPlaneXfl->swapFuses(iFuse, iFuse-1);
    }

    m_pPartModel->updateData();
    onUpdatePlane();
    m_bChanged = true;
}


void PlaneXflDlg::onResizeColumns()
{
    double w = double(m_pcptParts->width())/100.0;
    int wtype  = int(12.0*w);
    int wname  = int(15.0*w);
    int wdist  = int(10.0*w);

    m_pcptParts->setColumnWidth(0, wtype);
    m_pcptParts->setColumnWidth(1, wname);
    m_pcptParts->setColumnWidth(2, wdist);
    m_pcptParts->setColumnWidth(3, wdist);
    m_pcptParts->setColumnWidth(4, wdist);
    m_pcptParts->setColumnWidth(5, wdist);
    m_pcptParts->setColumnWidth(6, wdist);
    m_pcptParts->horizontalHeader()->setStretchLastSection(true);
}


void PlaneXflDlg::onPartItemClicked(QModelIndex index)
{
    if(!index.isValid())
    {
    }
    else
    {
        if(!m_pPlane) return;
        QString strange;
        int row = index.row();
        bool bShift = (QApplication::queryKeyboardModifiers() & Qt::SHIFT);
        if(!bShift) m_pglPlaneView->clearSelectedParts();
        if(row<m_pPlaneXfl->nWings())
        {
            WingXfl const *pWing = m_pPlaneXfl->wing(row);
            if(pWing)
            {
                updateOutput("\n" + pWing->name() + "\n");

                pWing->getProperties(strange, "   ");
                updateOutput(strange + "\n");
                m_pglPlaneView->setSelectedPart(pWing->uniqueIndex());
            }
            else      m_pglPlaneView->setSelectedPart(-1);
        }
        else if(row>=m_pPlaneXfl->nWings())
        {
            int iFuse = row-m_pPlaneXfl->nWings();
            Fuse *pFuse = m_pPlaneXfl->fuse(iFuse);
            if(pFuse)
            {
                updateOutput("\n" + pFuse->name() + "\n");
                pFuse->getProperties(strange, "   ");
                updateOutput(strange + "\n");
                m_pglPlaneView->setSelectedPart(pFuse->uniqueIndex());
            }
            else      m_pglPlaneView->setSelectedPart(-1);
        }
        m_pglPlaneView->update();

        if(index.column()==7)
        {
            m_pcptParts->selectRow(index.row());
            QRect itemrect = m_pcptParts->visualRect(index);
            QPoint menupos = m_pcptParts->mapToGlobal(itemrect.topLeft());
            m_pPartMenu->setEnabled(index.row()>=0);
            m_pFlipNormals->setEnabled(index.row()>=m_pPlaneXfl->nWings());
            m_pTessellation->setEnabled(index.row()>=m_pPlaneXfl->nWings());
            m_pPartMenu->exec(menupos, m_pEditPartDef);
        }
    }
}


void PlaneXflDlg::onSelectPart(Part *pPart)
{
    if(pPart->isWing())
    {
        for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
        {
            if(m_pPlaneXfl->wing(iw)==pPart)
            {
                m_pcptParts->selectRow(iw);
                m_plwWingListWt->setCurrentRow(iw);
            }
        }
    }
    else if(pPart->isFuse())
    {
        for(int ifuse=0; ifuse<m_pPlaneXfl->nFuse(); ifuse++)
        {
            if(m_pPlaneXfl->fuse(ifuse)==pPart)
            {
                m_pcptParts->selectRow(m_pPlaneXfl->nWings()+ifuse);
                m_plwFuseListWt->setCurrentRow(ifuse);
            }
        }
    }
}


void PlaneXflDlg::onPartDataChanged()
{
    m_bChanged = true;

    gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->resetgl3dFuse();
    pglPlaneXflView->resetgl3dPlane();

    onUpdatePlane();
}


int PlaneXflDlg::selectedPart()
{
    QModelIndexList indexList = m_pcptParts->selectionModel()->selectedIndexes();

    int row = -1;
    if(indexList.size()>0) row = indexList.at(0).row();
    return row;
}


void PlaneXflDlg::onPartInertia()
{
    int row = selectedPart();

    if(row<m_pPlaneXfl->nWings())
    {
        PartInertiaDlg dlg(m_pPlaneXfl->wing(row), nullptr, this);
        if(dlg.exec()==QDialog::Accepted) m_bChanged=true;
    }
    else if(m_pPlaneXfl->fuse(0))
    {
        Fuse*pFuse = m_pPlaneXfl->fuse(0);
        PartInertiaDlg dlg(nullptr, pFuse, this);

        if(dlg.exec()==QDialog::Accepted) m_bChanged=true;
    }

    m_pPartModel->updateData();
    m_pglPlaneView->update();
}


void PlaneXflDlg::onDuplicatePart()
{
    int row = selectedPart();
    if(row<m_pPlaneXfl->nWings())
    {
        m_pPlaneXfl->duplicateWing(row);
    }
    else
    {
        m_pPlaneXfl->duplicateFuse(row-m_pPlaneXfl->nWings());
    }
    m_pPartModel->updateData();

    onUpdatePlane();
    m_bChanged = true;
}


void PlaneXflDlg::onScalePart()
{
    int row = selectedPart();

    if(row<m_pPlaneXfl->nWings())
    {
        scaleWing(m_pPlaneXfl->wing(row));
    }
    else
    {
        scaleFuse(m_pPlaneXfl->fuse(0));
    }
}


Fuse *PlaneXflDlg::activeFuse()
{
    Fuse *pFuse=nullptr;
    if(m_pLeftTabWidget->currentIndex()==1)
    {
        int row = m_pcptParts->currentIndex().row();
        if(row<m_pPlaneXfl->nWings()) return nullptr;
        else pFuse = m_pPlaneXfl->fuse(row-m_pPlaneXfl->nWings());
    }
    else if(m_pLeftTabWidget->currentIndex()>1)
    {
        for(int ifuse=0; ifuse<m_plwFuseListWt->count(); ifuse++)
        {
            QListWidgetItem *pItem = m_plwFuseListWt->item(ifuse);
            if(pItem && pItem->isSelected())
            {
                pFuse = m_pPlaneXfl->fuse(ifuse);
                break;
            }
        }
    }
    return pFuse;
}


void PlaneXflDlg::makePartTable()
{
    m_pcptParts = new CPTableView(this);
    m_pcptParts->setEditable(true);
    m_pcptParts->setWindowTitle("Object List");
    m_pcptParts->setWordWrap(false);
    m_pcptParts->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QHeaderView *pVHeader = m_pcptParts->verticalHeader();
    pVHeader->setDefaultSectionSize(m_pcptParts->fontHeight()*2);
    pVHeader->setSectionResizeMode(QHeaderView::Fixed);

    m_pPartModel = new PlanePartModel(this);
    m_pcptParts->setModel(m_pPartModel);

    m_pPartDelegate = new PlanePartDelegate(this);
    m_pcptParts->setItemDelegate(m_pPartDelegate);

    m_pPartDelegate->setPrecision({0,0,3,3,3,2,2,0});
}


void PlaneXflDlg::setupLayout()
{
    m_pScalePlane  = new QAction("Scale plane");
    QMenu *pCheckMeshMenu = m_ppbActions->menu();
    pCheckMeshMenu->addSeparator();
    pCheckMeshMenu->addAction(m_pScalePlane);

    m_pHSplitter = new QSplitter(Qt::Horizontal);
    {
        m_pHSplitter->setChildrenCollapsible(false);
        QFrame *pLeftSideWidget = new QFrame;
        {
            QVBoxLayout *pLeftSideLayout = new QVBoxLayout;
            {
                m_pPartSplitter = new QSplitter(Qt::Vertical);
                {
                    m_pLeftTabWidget = new QTabWidget;
                    {
                        QFrame *pMetaFrame = new QFrame;
                        {
                            QVBoxLayout *pDefinitionLayout = new QVBoxLayout;
                            {
                                m_pleDescription->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

                                pDefinitionLayout->addWidget(m_pleName);
                                pDefinitionLayout->addWidget(m_pleDescription);
                            }
                            pMetaFrame->setLayout(pDefinitionLayout);
                        }

                        QFrame *pPartFrame = new QFrame;
                        {
                            QVBoxLayout *pPartLayout = new QVBoxLayout;
                            {
                                makePartTable();
                                QHBoxLayout *pEditCmdLayout = new QHBoxLayout;
                                {
                                    QPushButton *ppbInsertBtn = new QPushButton("Insert");
                                    QMenu *pInsertMenu = new QMenu(this);
                                    {
                                        QMenu *pInsertWingMenu = pInsertMenu->addMenu("Wing");
                                        {
                                            pInsertWingMenu->addAction(m_pInsertWing);
                                            pInsertWingMenu->addAction(m_pInsertWingOther);
                                            pInsertWingMenu->addAction(m_pInsertWingXml);
                                            pInsertWingMenu->addAction(m_pInsertWingVSP);
                                        }

                                        pInsertMenu->addAction(m_pInsertElev);
                                        pInsertMenu->addAction(m_pInsertFin);
                                        pInsertMenu->addSeparator();

                                        QMenu*pFuseMenu = pInsertMenu->addMenu("Fuselage");
                                        {
                                            QMenu *pFuseXflMenu = pFuseMenu->addMenu("xfl-type fuselage");
                                            {
                                                pFuseXflMenu->addAction(m_pInsertFuseXflSpline);
                                                pFuseXflMenu->addAction(m_pInsertFuseXflFlat);
                                                pFuseXflMenu->addAction(m_pInsertFuseXflSections);
                                                pFuseXflMenu->addAction(m_pInsertFuseXml);
                                            }
                                            pFuseMenu->addAction(m_pInsertFuseOther);
                                            pFuseMenu->addAction(m_pInsertFuseOcc);
                                            pFuseMenu->addAction(m_pInsertFuseStl);
                                        }
                                        pInsertMenu->addSeparator();
                                        QMenu *pTestMenu = pInsertMenu->addMenu("Testing");
                                        {
                                            pTestMenu->addAction(m_pInsertEllWing);
                                            pTestMenu->addSeparator();
                                            QMenu *pFuseMenu = pTestMenu->addMenu("Fuselage");
                                            {
                                                pFuseMenu->addAction(m_pInsertSTLSphere);
                                                pFuseMenu->addAction(m_pInsertSTLCylinder);
                                                pFuseMenu->addSeparator();
//                                                pFuseMenu->addAction(m_pInsertCADSphere);
//                                                pFuseMenu->addAction(m_pInsertCADCylinder);
                                                pFuseMenu->addAction(m_pInsertCADBox);
                                            }
                                        }

                                        ppbInsertBtn->setMenu(pInsertMenu);
                                    }

                                    m_pchHighlightSel = new QCheckBox("Highlight selected part");

                                    pEditCmdLayout->addWidget(ppbInsertBtn);
                                    pEditCmdLayout->addStretch();
                                    pEditCmdLayout->addWidget(m_pchHighlightSel);
                                }
                                pPartLayout->addWidget(m_pcptParts);
                                pPartLayout->addLayout(pEditCmdLayout);
                            }
                            pPartFrame->setLayout(pPartLayout);
                        }

                        QFrame *pTablesFrame = new QFrame;
                        {
                            QVBoxLayout *pTablesLayout = new QVBoxLayout;
                            {
                                QHBoxLayout *pObjectsLayout = new QHBoxLayout;
                                {
                                    QGroupBox * pCuttingWingsBox = new QGroupBox("Cutting wings");
                                    {
                                        QVBoxLayout *pCuttingWingsLayout = new QVBoxLayout;
                                        {
                                            m_plwWingListWt = new QListWidget;
                                            m_plwWingListWt->setSelectionMode(QAbstractItemView::MultiSelection);

                                            QGridLayout *pAssyParamLayout = new QGridLayout;
                                            {
                                                QString sewtip = "<p>OpenCascade documentation: "
                                                                 "Sewing allows creation of connected topology (shells and wires) "
                                                                 "from a set of separate topological elements (faces and edges). ";
                                                sewtip += "Working tolerance defines the maximal distance between topological elements "
                                                          "which can be sewn. It is not ultimate that such elements will be actually "
                                                          "sewn as many other criteria are applied to make the final decision. "
                                                          "Minimal tolerance defines the size of the smallest element (edge) in "
                                                          "the resulting shape. It is declared that no edges with size less than this "
                                                          "value are created after sewing. If encountered, such topology becomes degenerated.  ";

                                                sewtip +="The following recommendations can be proposed for tuning-up the sewing process: "
                                                         "<ul>"
                                                         "<li> Use as small working tolerance as possible. This will reduce the sewing time and, "
                                                         "  consequently, the number of incorrectly sewn edges for shells with free boundaries.</li>"
                                                         "<li> Use as large minimal tolerance as possible. This will reduce the number of small "
                                                         "  geometry in the shape, both original and appearing after cutting.</li>"
                                                         "<li> If it is expected to obtain a shell with holes (free boundaries) as a result "
                                                         "  of sewing, the working tolerance should be set to a value not greater than the "
                                                         "  size of the smallest element (edge) or smallest distance between elements of such "
                                                         "  free boundary. Otherwise the free boundary may be sewn only partially.</li>"
                                                         "<li> It should be mentioned that the Sewing algorithm is unable to understand which "
                                                         "  small (less than working tolerance) free boundary should be kept and which should "
                                                         "  be sewn.</li>"
                                                         "</ul>"
                                                         "</p>";

                                                QLabel *pLabSewPrecision = new QLabel("Sewing precision:");
                                                pLabSewPrecision->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                                                QLabel *pLabLength1 = new QLabel(Units::lengthUnitLabel());
                                                pLabLength1->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
                                                m_pdeStitchPrecision = new FloatEdit(0.001f);
                                                m_pdeStitchPrecision->setToolTip(sewtip);

                                                pAssyParamLayout->addWidget(pLabSewPrecision,       1,1);
                                                pAssyParamLayout->addWidget(m_pdeStitchPrecision, 1,2);
                                                pAssyParamLayout->addWidget(pLabLength1,            1,3);
                                            }

                                            pCuttingWingsLayout->addWidget(m_plwWingListWt);
                                            pCuttingWingsLayout->addLayout(pAssyParamLayout);
                                        }
                                        pCuttingWingsBox->setLayout(pCuttingWingsLayout);
                                    }

                                    QGroupBox *pFuseToCutBox = new QGroupBox("Fuselage to cut");
                                    {
                                        QVBoxLayout *pFuseToCutLayout = new QVBoxLayout;
                                        {
                                            m_plwFuseListWt = new QListWidget;
                                            m_ppbFuseMenuButton = new QPushButton("Fuselage actions");
                                            {
                                                QMenu *pFuseMenu = new QMenu("Fuselage actions", this);
                                                {
                                                    pFuseMenu->addAction(m_pResetFuse);
                                                    pFuseMenu->addAction(m_pTessellation);
                                                    pFuseMenu->addAction(m_pFlipNormals);
                                                }
                                                m_ppbFuseMenuButton->setMenu(pFuseMenu);
                                            }

                                            pFuseToCutLayout->addWidget(m_plwFuseListWt);
                                            pFuseToCutLayout->addWidget(m_ppbFuseMenuButton);
                                        }
                                        pFuseToCutBox->setLayout(pFuseToCutLayout);
                                    }
                                    pObjectsLayout->addWidget(pCuttingWingsBox);
                                    pObjectsLayout->addWidget(pFuseToCutBox);
                                }

                                m_pchCutParallel = new QCheckBox("Multithreaded cut operation");
                                m_ppbCutFuse = new QPushButton("Cut fuselage");
                                pTablesLayout->addLayout(pObjectsLayout);
                                pTablesLayout->addWidget(m_pchCutParallel);
                                pTablesLayout->addWidget(m_ppbCutFuse);
                            }
                            pTablesFrame->setLayout(pTablesLayout);
                        }

                        m_pMesherWt = new MesherWt(this);
                        m_pMesherWt->showPickEdge(false);

                        m_pMeshCorrectionsFrame = new QFrame;
                        {
                            QLabel *plabInfo = new QLabel("Use the actions below to modify the fuselage's mesh<br>"
                                                          "and to connect it to the wing's mesh.");
                            QVBoxLayout *pCorrectLayout = new QVBoxLayout;
                            {
                                QGridLayout *pModLayout = new QGridLayout;
                                {
                                    QLabel *plabTrans = new QLabel("Node translation:");
                                    m_ppbMoveNode = new QPushButton("Move node");
                                    QString tip = "<p>Use this option to move a fuselage node and to merge it with another."
                                                     "Select first the source node to move, then the destination node.</p>";
                                    m_ppbMoveNode->setToolTip(tip);
                                    m_ppbMoveNode->setCheckable(true);


                                    QLabel *plabDelete = new QLabel("Panel deletion:");
                                    m_ppbSelectPanels = new QPushButton("Select");
                                    m_ppbSelectPanels->setCheckable(true);
                                    m_ppbDeletePanel = new QPushButton("Delete");

                                    QLabel *plabMake = new QLabel("Panel creation:");
                                    m_ppbMakeP3 = new QPushButton("Single");
                                    m_ppbMakeP3->setCheckable(true);

                                    m_ppbMakeP3Strip = new QPushButton("Strip");
                                    m_ppbMakeP3Strip->setCheckable(true);

                                    m_pchMakeP3Opposite = new QCheckBox("Make opposite");

                                    pModLayout->addWidget(plabInfo,            0, 1, 1 ,3);
                                    pModLayout->addWidget(plabTrans,           1, 1);
                                    pModLayout->addWidget(m_ppbMoveNode,       1, 2);
                                    pModLayout->addWidget(plabDelete,          2, 1);
                                    pModLayout->addWidget(m_ppbSelectPanels,   2, 2);
                                    pModLayout->addWidget(m_ppbDeletePanel,    2, 3);
                                    pModLayout->addWidget(plabMake,            3, 1);
                                    pModLayout->addWidget(m_ppbMakeP3,         3, 2);
                                    pModLayout->addWidget(m_ppbMakeP3Strip,    3, 3);
                                    pModLayout->addWidget(m_pchMakeP3Opposite, 3, 4);
                                }

                                QHBoxLayout *pUndoRedoLayout = new QHBoxLayout;
                                {
                                    m_ppbUndo = new QPushButton(QIcon(":/icons/OnUndo.png"), "Undo");
                                    m_ppbUndo->setShortcut(Qt::Key_Undo);
                                    m_ppbRedo = new QPushButton(QIcon(":/icons/OnRedo.png"), "Redo");
                                    m_ppbUndo->setShortcut(Qt::Key_Redo);

                                    m_plabStackInfo = new QLabel("stackpos: 0/0");
                                    pUndoRedoLayout->addStretch();
                                    pUndoRedoLayout->addWidget(m_ppbUndo);
                                    pUndoRedoLayout->addWidget(m_ppbRedo);
                                    pUndoRedoLayout->addWidget(m_plabStackInfo);
                                    pUndoRedoLayout->addStretch();
                                }

                                m_ppbCheckMenuBtn = new QPushButton("Mesh actions");
                                {
                                    QMenu *pCheckMeshMenu = new QMenu("Mesh actions");
                                    {
                                        m_pRestoreFuseMesh  = new QAction("Restore default mesh",this);
                                        m_pFuseMesher    = new QAction("Fuselage mesher");
                                        pCheckMeshMenu->addAction(m_pCheckMesh);
                                        pCheckMeshMenu->addAction(m_pConnectPanels);
                                        pCheckMeshMenu->addAction(m_pCheckFreeEdges);
                                        pCheckMeshMenu->addAction(m_pClearHighlighted);
                                        pCheckMeshMenu->addSeparator();
                                        pCheckMeshMenu->addAction(m_pMergeFuseToWingNodes);
                                        pCheckMeshMenu->addSeparator();
                                        pCheckMeshMenu->addAction(m_pCenterOnPanel);
                                        pCheckMeshMenu->addSeparator();
                                        pCheckMeshMenu->addAction(m_pRestoreFuseMesh);
                                        pCheckMeshMenu->addSeparator();
                                        pCheckMeshMenu->addAction(m_pFuseMesher);
                                    }
                                    m_ppbCheckMenuBtn->setMenu(pCheckMeshMenu);
                                }
                                QLabel *pFlow5Link = new QLabel;
                                pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Modelling/Connections.html>https://flow5.tech/docs/flow5_doc/Modelling/Connections.html</a>");
                                pFlow5Link->setOpenExternalLinks(true);
                                pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);

                                pCorrectLayout->addLayout(pModLayout);
                                pCorrectLayout->addLayout(pUndoRedoLayout);
                                pCorrectLayout->addStretch();
                                pCorrectLayout->addWidget(m_ppbCheckMenuBtn);
                                pCorrectLayout->addWidget(pFlow5Link);
                            }
                            m_pMeshCorrectionsFrame->setLayout(pCorrectLayout);
                        }

                        m_pLeftTabWidget->addTab(pMetaFrame,               "Meta");
                        m_pLeftTabWidget->addTab(pPartFrame,               "Parts");
                        m_pLeftTabWidget->addTab(pTablesFrame,             "Assembly");
                        m_pLeftTabWidget->addTab(m_pMesherWt,              "Fuselage mesh");
                        m_pLeftTabWidget->addTab(m_pMeshCorrectionsFrame , "Mesh connections");

                        m_pLeftTabWidget->setTabToolTip(0, "Ctrl+1");
                        m_pLeftTabWidget->setTabToolTip(1, "Ctrl+2");
                        m_pLeftTabWidget->setTabToolTip(2, "Ctrl+3");
                        m_pLeftTabWidget->setTabToolTip(3, "Ctrl+4");
                        m_pLeftTabWidget->setTabToolTip(4, "Ctrl+5");
                    }

                    m_ppto = new PlainTextOutput;

                    m_pPartSplitter->addWidget(m_pLeftTabWidget);
                    m_pPartSplitter->addWidget(m_ppto);
                    m_pPartSplitter->setStretchFactor(0,1);
                    m_pPartSplitter->setStretchFactor(1,5);
                }

                pLeftSideLayout->addWidget(m_pPartSplitter);
                pLeftSideLayout->addWidget(m_pButtonBox);
                pLeftSideWidget->setLayout(pLeftSideLayout);
            }
        }

        m_pglPlaneViewFrame = new QFrame;
        {
            QVBoxLayout *pTWPageLayout = new QVBoxLayout;
            {
                m_pglPlaneView = new gl3dPlaneXflView;
                m_pglControls  = new gl3dGeomControls(m_pglPlaneView, PlaneLayout, true);
                connect(m_pglControls->m_ptbDistance, SIGNAL(clicked()), this, SLOT(onNodeDistance()));
                pTWPageLayout->addWidget(m_pglPlaneView);
                pTWPageLayout->addWidget(m_pglControls);
            }
            m_pglPlaneViewFrame->setLayout(pTWPageLayout);
        }

        m_pHSplitter->addWidget(pLeftSideWidget);
#ifdef QT_DEBUG
        m_pHSplitter->addWidget(m_pglPlaneViewFrame);
#else
        m_pHSplitter->addWidget(m_pglPlaneViewFrame);
#endif
        m_pHSplitter->setStretchFactor(0,1);
        m_pHSplitter->setStretchFactor(1,2);
    }

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);
}


void PlaneXflDlg::onClearOutput()
{
    m_ppto->clear();
}


void PlaneXflDlg::onResetFuseMesh()
{
    onClearHighlighted();

    if(!m_pPlaneXfl->fuse(0)) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString logmsg;
    m_pPlaneXfl->fuse(0)->makeDefaultTriMesh(logmsg, "   ");
    updateOutput(logmsg);

    m_pPlaneXfl->makeTriMesh(true);

    QString strange;
    strange = QString::asprintf("\nTriangle count=%d\n", m_pPlaneXfl->nPanel3());
    updateOutput(strange);
    strange = QString::asprintf(  "Node count    =%d\n", m_pPlaneXfl->nNodes());
    strange += "\n_______\n\n";
    updateOutput(strange);

    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();
    onUpdatePlaneProps();

    m_bChanged = true;
    QApplication::restoreOverrideCursor();
}


void PlaneXflDlg::onFuseMeshDlg()
{
    onClearHighlighted();

    if(!m_pPlaneXfl->fuse(0)) return;

    FuseMesherDlg *pFMDlg = new FuseMesherDlg(this);
    pFMDlg->initDialog(m_pPlaneXfl->fuse(0));
    if(pFMDlg->exec()!=QDialog::Accepted)
    {
        return;
    }

    m_pPlaneXfl->makeTriMesh(true);

    QString strange;
    strange = QString::asprintf("\nTriangle count=%d\n", m_pPlaneXfl->nPanel3());
    updateOutput(strange);
    strange = QString::asprintf(  "Node count    =%d\n", m_pPlaneXfl->nNodes());
    strange += "\n_______\n\n";
    updateOutput(strange);

    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();
    onUpdatePlaneProps();

    m_bChanged = true;
}


/**
 * Exports the part's triangular mesh to an STL file
 */
void PlaneXflDlg::onExportMeshToSTLFile()
{
    if(!m_pPlane) return;
    exportMeshToSTLFile(m_pPlaneXfl->triMesh(), m_pPlaneXfl->name(), SaveOptions::STLDirName(), Units::mtoUnit());
}


void PlaneXflDlg::onPickedNodePair(QPair<int, int> nodepair)
{
    if(m_pglControls->getDistance())
    {
        if(nodepair.first <0 || nodepair.first >=m_pPlane->refTriMesh().nPanels()) return;
        if(nodepair.second<0 || nodepair.second>=m_pPlane->refTriMesh().nPanels()) return;
        Node nsrc  = m_pPlane->refTriMesh().node(nodepair.first);
        Node ndest = m_pPlane->refTriMesh().node(nodepair.second);

        Segment3d seg(nsrc, ndest);
        m_pglPlaneView->setMeasure(seg);

        QString strange = QString::asprintf("Node %d to node %d\n", nodepair.first, nodepair.second);
        strange += QString::asprintf("   distance = %13g ", seg.length()*Units::mtoUnit()) + Units::lengthUnitLabel() + "\n";
        strange += QString::asprintf("         dx = %13g ", seg.segment().x*Units::mtoUnit()) + Units::lengthUnitLabel() + "\n";
        strange += QString::asprintf("         dy = %13g ", seg.segment().y*Units::mtoUnit()) + Units::lengthUnitLabel() + "\n";
        strange += QString::asprintf("         dz = %13g ", seg.segment().z*Units::mtoUnit()) + Units::lengthUnitLabel() + "\n\n";
        updateOutput(strange);

        m_pglPlaneView->resetPickedNodes();
        m_pglPlaneView->update();
        return;
    }

    if(!m_ppbMoveNode->isChecked())
    {
        return;
    }

    if(!m_pPlaneXfl->hasFuse()) return;
    Fuse *pFuse = m_pPlaneXfl->fuse(0);

    // The node pair is given in plane indexes, convert to fuse indexes
    nodepair.first  -= pFuse->firstNodeIndex();
    nodepair.second -= pFuse->firstNodeIndex();

    if (nodepair.first<0 || nodepair.first>=pFuse->triMesh().nNodes() || nodepair.second<0 || nodepair.second>=pFuse->triMesh().nNodes())
    {
        m_ppto->onAppendThisPlainText("*****Internal index error picking nodes*****\n");
        return;
    }

    Node nsrc  = pFuse->triMesh().node(nodepair.first);
    Node ndest = pFuse->triMesh().node(nodepair.second);

    QString strange;
    strange = QString::asprintf("Moving fuselage node %d to location of node %d and merging\n", nodepair.first, nodepair.second);
    updateOutput(strange);

    // take a picture
    m_MeshStack.push(pFuse->triMesh());

    QString log;
    QApplication::setOverrideCursor(Qt::WaitCursor);

    pFuse->triMesh().mergeNodes(nodepair.first, nodepair.second, true, log, "   ");

    // remove the segments from the free edge list
    QVector<Segment3d> &freedges = m_pglPlaneView->segments();
    for(int iseg=freedges.size()-1; iseg>=0; iseg--)
    {
        Segment3d &seg = freedges[iseg];
        if(seg.vertex(0).isSame(nsrc) || seg.vertex(1).isSame(nsrc) || seg.vertex(0).isSame(ndest) || seg.vertex(1).isSame(ndest))
        {
            freedges.removeAt(iseg);
        }
    }

    //reset indexes, triangles have been reordered
    for(int i3=0; i3<pFuse->nPanel3(); i3++)
    {
        Panel3 &p3 = pFuse->triMesh().panel(i3);
        p3.setIndex(i3);
        Q_ASSERT(p3.isFusePanel());
    }

    pFuse->triMesh().makeNodeArrayFromPanels(0, strange, "  ");
    takePicture();

    onUpdateMesh();

    QApplication::restoreOverrideCursor();
    updateOutput(log);

    m_pglPlaneView->resetPickedNodes();

    m_pglPlaneView->update();

    m_bChanged = true;
}


void PlaneXflDlg::onScalePlane()
{
    QStringList labels("Scale factor:");
    QStringList rightlabels("");
    QVector<double> vals({1});
    DoubleValueDlg dlg(this, vals, labels, rightlabels);
    if(dlg.exec()!=QDialog::Accepted) return;

    m_pPlane->scale(dlg.value(0));

    m_bChanged = true;
    onUpdatePlane();
}


void PlaneXflDlg::onInsertCADShape()
{
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    Fuse *pFuseOcc = m_pPlaneXfl->makeNewFuse(Fuse::Occ);
    pFuseOcc->setColor(QColor(235,155,105));
    QString strong = QString::asprintf("Cylinder_CAD_fuse_%d", m_pPlaneXfl->stlFuseCount());
    pFuseOcc->setPartName(strong);

    if(pSenderAction==m_pInsertCADCylinder)
    {
        DoubleValueDlg dlg(this, {2.0*Units::mtoUnit(), 0.1*Units::mtoUnit()}, {"Length", "Radius"},
                           {Units::lengthUnitLabel(), Units::lengthUnitLabel()});
        if(dlg.exec()!=QDialog::Accepted) return;

        double length = dlg.value(0)/Units::mtoUnit();
        double radius = dlg.value(1)/Units::mtoUnit();

        BRepPrimAPI_MakeCylinder maker(radius, length);
        maker.Build();
        if(!maker.IsDone() || maker.Shape().IsNull())
        {
            delete pFuseOcc;
            m_ppto->onAppendThisPlainText("Error making cylinder");
            return;
        }
        pFuseOcc->appendShape(maker.Shape());
    }
    else if(pSenderAction==m_pInsertCADSphere)
    {
        DoubleValueDlg dlg(this, {1.0*Units::mtoUnit()}, {"Radius"}, {Units::lengthUnitLabel()});
        if(dlg.exec()!=QDialog::Accepted) return;

        double radius = dlg.value(0)/Units::mtoUnit();

        BRepPrimAPI_MakeSphere maker(radius);
        maker.Build();
        if(!maker.IsDone() || maker.Shape().IsNull())
        {
            delete pFuseOcc;
            m_ppto->onAppendThisPlainText("Error making sphere");
            return;
        }
        pFuseOcc->appendShape(maker.Shape());
    }
    else if(pSenderAction==m_pInsertCADBox)
    {
        DoubleValueDlg dlg(this, {1.0*Units::mtoUnit(), 1.0*Units::mtoUnit(), 1.0*Units::mtoUnit()}, {"Length", "Width", "Height"},
                           {Units::lengthUnitLabel(), Units::lengthUnitLabel(), Units::lengthUnitLabel()});
        if(dlg.exec()!=QDialog::Accepted) return;

        double x = dlg.value(0)/Units::mtoUnit();
        double y = dlg.value(1)/Units::mtoUnit();
        double z = dlg.value(2)/Units::mtoUnit();

        BRepPrimAPI_MakeBox maker(x, y, z);
        maker.Build();
        if(!maker.IsDone() || maker.Shape().IsNull())
        {
            delete pFuseOcc;
            m_ppto->onAppendThisPlainText("Error making box");
            return;
        }
        pFuseOcc->appendShape(maker.Shape());
    }
    m_ppto->appendPlainText("---Making shells from shapes-----\n");

    pFuseOcc->makeShellsFromShapes();

    if(pFuseOcc->shellCount()==0)
    {
        QString strange  = "Imported SHAPE does not contain any SHELL: use the fuselage editor to fix the shapes.\n";
        updateOutput(strange);
    }
    else
    {
        m_ppto->appendPlainText("---Making shell triangulation-----\n");
        QString logmsg;
        updateOutput("Making shell triangulation\n");
        pFuseOcc->makeShellTriangulation(logmsg, "   ");
        pFuseOcc->saveBaseTriangulation();
        pFuseOcc->computeSurfaceProperties(logmsg, "   ");
        updateOutput(logmsg+"\n");
        m_ppto->appendPlainText("---updating plane-----\n");
        onUpdatePlane();
    }

    setControls();

    updateData();

    m_bChanged = true;
}


#define MAXSTACKSIZE 25

void PlaneXflDlg::takePicture()
{
    //clear the downstream part of the stack which becomes obsolete
    clearStack(m_StackPos);

    Fuse* pFuse = m_pPlane->fuse(0);
    if(pFuse)
        m_UndoStack.append(pFuse->triMesh());

    if(m_UndoStack.size()==MAXSTACKSIZE+1) m_UndoStack.pop_front();

    // the new current position is the top of the stack
    m_StackPos = m_UndoStack.size()-1;

    enableStackBtns();
}


void PlaneXflDlg::onRedo()
{
    if(m_StackPos<m_UndoStack.size()-1)
    {
        m_StackPos++;
        setPicture();
    }
    enableStackBtns();
}


void PlaneXflDlg::onUndo()
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
    enableStackBtns();
}


void PlaneXflDlg::clearStack(int pos)
{
    for(int il=m_UndoStack.size()-1; il>pos; il--)
    {
        m_UndoStack.removeAt(il);     // remove from the stack
    }
    m_StackPos = m_UndoStack.size()-1;
}


void PlaneXflDlg::setPicture()
{
    Fuse *pFuse = m_pPlaneXfl->fuse(0);
    if(!pFuse) return;
    pFuse->setTriMesh(m_UndoStack.at(m_StackPos));

    onUpdateMesh();
}


void PlaneXflDlg::enableStackBtns()
{
    m_ppbUndo->setEnabled(m_StackPos>0);
    m_ppbRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);
    m_plabStackInfo->setText(QString::asprintf("stack: %d/%d", m_StackPos+1, int(m_UndoStack.size())));
}


void PlaneXflDlg::onMergeNodes(bool bIsMerging)
{
    m_ppbMakeP3->setChecked(false);
    m_ppbMakeP3Strip->setChecked(false);
    m_ppbSelectPanels->setChecked(false);
    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->clearP3Selection();
    pglPlaneXflView->resetgl3dP3Sel();

    m_pglPlaneView->resetPickedNodes();

    m_pglPlaneView->setPicking(bIsMerging ? xfl::MESHNODE : xfl::NOPICK);
    m_pglPlaneView->setSurfacePick(xfl::FUSESURFACE);
    m_pglPlaneView->update();
    m_bChanged = true;
}


void PlaneXflDlg::onMakeP3(bool bCheck)
{
    m_ppbMoveNode->setChecked(false);
    m_ppbSelectPanels->setChecked(false);
    m_ppbMakeP3Strip->setChecked(false);
    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->clearP3Selection();
    pglPlaneXflView->resetgl3dP3Sel();
    m_pglPlaneView->resetPickedNodes();

    Fuse *pFuse = m_pPlane->fuse(0);
    if(!pFuse)
    {
        m_ppto->appendPlainText("A fuselage is required to build the panel strip");
        m_ppbMakeP3Strip->setChecked(false);
        return;
    }

    m_pglPlaneView->resetPickedNodes();
    m_pglPlaneView->setPicking(bCheck ? xfl::MESHNODE : xfl::NOPICK);
    m_pglPlaneView->setSurfacePick(xfl::NOSURFACE); // =ANYSURFACE

    if(!bCheck)
    {
        endPanelMods();
    }

    m_pglPlaneView->update();
}


void PlaneXflDlg::onMakeP3Strip(bool bCheck)
{
    m_ppbMoveNode->setChecked(false);
    m_ppbSelectPanels->setChecked(false);
    m_ppbMakeP3->setChecked(false);

    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->clearP3Selection();
    pglPlaneXflView->resetgl3dP3Sel();
    m_pglPlaneView->resetPickedNodes();

    Fuse *pFuse = m_pPlane->fuse(0);
    if(!pFuse)
    {
        m_ppto->appendPlainText("A fuselage is required to build the panel strip");
        m_ppbMakeP3Strip->setChecked(false);
        return;
    }

    m_pglPlaneView->setPicking(bCheck ? xfl::MESHNODE : xfl::NOPICK);
    m_pglPlaneView->setSurfacePick(xfl::NOSURFACE); // =ANYSURFACE

    if(!bCheck)
    {
        endPanelMods();
    }

    m_pglPlaneView->update();
}


void PlaneXflDlg::onPickedNode(int iNode)
{
    if(!m_ppbMakeP3Strip->isChecked() && !m_ppbMakeP3->isChecked()) return;

    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);

    if(pglPlaneXflView->P3Node(0)<0)
    {
        pglPlaneXflView->setP3Node(0,iNode);
    }
    else if(pglPlaneXflView->P3Node(1)<0)
    {
        pglPlaneXflView->setP3Node(1, iNode);
    }
    else if(pglPlaneXflView->P3Node(2)<0) // always true
    {
        pglPlaneXflView->setP3Node(2, iNode);

        Fuse *pFuse = m_pPlane->fuse(0);
        if(!pFuse) return;

        Panel3 p3(m_pPlane->node(pglPlaneXflView->P3Node(0)), m_pPlane->node(pglPlaneXflView->P3Node(1)), m_pPlane->node(pglPlaneXflView->P3Node(2)));
        p3.setSurfacePosition(xfl::FUSESURFACE);
        pFuse->triMesh().addPanel(p3);
        if(m_pchMakeP3Opposite->isChecked())
        {
            p3.makeXZsymmetric();
            pFuse->triMesh().addPanel(p3);
        }

        QString strange;
        pFuse->triMesh().makeNodeArrayFromPanels(0, strange, "  ");
        takePicture();

        onUpdateMesh();

        if(m_ppbMakeP3Strip->isChecked())
            pglPlaneXflView->shiftP3Nodes();
        else
            pglPlaneXflView->resetPickedNodes();

        m_pglPlaneView->resetgl3dMesh();
        m_pglPlaneView->update();
        m_bChanged = true;
    }
}


void PlaneXflDlg::onSelectPanels(bool bSelect)
{
    m_ppbMoveNode->setChecked(false);
    m_ppbMakeP3->setChecked(false);
    m_ppbMakeP3Strip->setChecked(false);


    m_ppbMoveNode->setChecked(false);

    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->clearP3Selection();
    pglPlaneXflView->resetgl3dP3Sel();

    if(bSelect)
    {
        pglPlaneXflView->setP3Selecting(true);
        m_pglPlaneView->setPicking(xfl::PANEL3);
        m_pglPlaneView->setSurfacePick(xfl::FUSESURFACE);
    }
    else
        m_pglPlaneView->setPicking(xfl::NOPICK);

    m_pglPlaneView->update();
    m_bChanged = true;
}


void PlaneXflDlg::onDeleteP3Selection()
{
    Fuse *pFuse = m_pPlaneXfl->fuse(0);
    if(!pFuse) return;
    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);

    if(pglPlaneXflView->p3SelectionCount()==0)
    {
        m_ppto->appendPlainText("Nothing to delete\n");
        return;
    }

    QString strange;

//    pglPlaneXflView->setP3Selecting(false);

    for(int i3=pFuse->nPanel3()-1; i3>=0; i3--)
    {
        for(int isel=0; isel<pglPlaneXflView->p3SelectionCount(); isel++)
        {
            int iPanel3 = pglPlaneXflView->p3SelectionAt(isel);

            if(pFuse->panel3At(i3).index()+pFuse->firstPanel3Index()==iPanel3)
            {
                strange = QString::asprintf("Deleting panel %d\n", iPanel3);
                updateOutput(strange);

                pFuse->triMesh().removePanelAt(i3);

                pglPlaneXflView->p3SelRemove(isel);
                m_bChanged = true;
                break;
            }
        }
    }
    Q_ASSERT(pglPlaneXflView->p3SelectionCount()==0);

    strange = QString::asprintf("\nTriangle count = %d\n", pFuse->nPanel3());
    strange += QString::asprintf("Node count     = %d\n", int(pFuse->nodes().size()));
    strange += "\n_______\n\n";
    updateOutput(strange);

    pFuse->triMesh().makeNodeArrayFromPanels(0, strange, "  ");
    takePicture();

    onUpdateMesh();

    pglPlaneXflView->resetgl3dP3Sel();

//    m_ppbSelectPanels->setChecked(false);
}


bool PlaneXflDlg::endPanelMods()
{
    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->resetPickedNodes();
    pglPlaneXflView->clearP3Selection();
    pglPlaneXflView->resetgl3dP3Sel();
    m_pglPlaneView->setPicking(xfl::NOPICK);
    m_pglPlaneView->setSurfacePick(xfl::NOSURFACE); // =ANYSURFACE;
    onUpdateMesh();
    return true;
}
