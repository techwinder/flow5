/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include <QApplication>
#include <QKeyEvent>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QHeaderView>
#include <QMenu>
#include <QCursor>
#include <QAction>
#include <QFileDialog>

#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_MakeShell.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <Geom_BSplineSurface.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

#include <xfl/opengl/gl3dboatview.h>
#include <xfl/opengl/gl3dgeomcontrols.h>

#include <xflcore/flow5events.h>
#include <xflcore/saveoptions.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflgeom/geom3d/quad3d.h>
#include <xfl/editors/boatedit/boatdlg.h>
#include <xfl/editors/boatedit/nurbssaildlg.h>
#include <xfl/editors/boatedit/occsaildlg.h>
#include <xfl/editors/boatedit/sailcadreaderdlg.h>
#include <xfl/editors/boatedit/splinesaildlg.h>
#include <xfl/editors/boatedit/stlsaildlg.h>
#include <xfl/editors/boatedit/wingsaildlg.h>
#include <xfl/editors/fuseedit/bodyscaledlg.h>
#include <xfl/editors/fuseedit/fuseoccdlg.h>
#include <xfl/editors/fuseedit/fusestldlg.h>
#include <xfl/editors/fuseedit/xflfuseedit/fusexfldefdlg.h>
#include <xflcore/stlreaderdlg.h>
#include <xflobjects/objects3d/fuse/fuseocc.h>
#include <xflobjects/objects3d/fuse/fusestl.h>
#include <xflobjects/objects3d/fuse/fusexfl.h>
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflocc/occ_globals.h>
#include <xflobjects/sailobjects/boat.h>
#include <xflobjects/sailobjects/sailobjects.h>
#include <xflobjects/sailobjects/sails/nurbssail.h>
#include <xflobjects/sailobjects/sails/occsail.h>
#include <xflobjects/sailobjects/sails/sail.h>
#include <xflobjects/sailobjects/sails/splinesail.h>
#include <xflobjects/sailobjects/sails/stlsail.h>
#include <xflobjects/sailobjects/sails/wingsail.h>
#include <xflscript//xml/xsail/xmlboatreader.h>
#include <xflscript//xml/xsail/xmlsailreader.h>
#include <xflscript/xml/fuse/xmlfusereader.h>
#include <xflwidgets/customdlg/newnamedlg.h>
#include <xflwidgets/customdlg/selectiondlg.h>
#include <xflwidgets/customwts/xfldelegate.h>
#include <xflwidgets/customwts/actionitemmodel.h>
#include <xflwidgets/customwts/cptableview.h>
#include <xflwidgets/customwts/plaintextoutput.h>

QByteArray BoatDlg::s_WindowGeometry;

QByteArray BoatDlg::s_HSplitterSizes;
QByteArray BoatDlg::s_VSplitterSizes;

Quaternion BoatDlg::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);


BoatDlg::BoatDlg(QWidget *pParent) : XflDialog(pParent)
{
    setWindowTitle("Boat editor");
    setWindowFlag(Qt::WindowMinMaxButtonsHint);

    m_pBoat = nullptr;
    m_pglBoatView = nullptr;
    m_pHullDelegate = nullptr;
    m_pSailDelegate = nullptr;

    m_bChanged = m_bDescriptionChanged = false;

    setupLayout();
    connectSignals();
}


BoatDlg::~BoatDlg()
{
}


void BoatDlg::setSail(int iSelect) {if(iSelect>=0) m_pcptSails->selectRow(iSelect);}
void BoatDlg::setHull(int iSelect) {if(iSelect>=0) m_pcptHulls->selectRow(iSelect);}


void BoatDlg::setupLayout()
{
    QFrame *pLeftFrame = new QFrame;
    {
        QVBoxLayout *pLeftLayout = new QVBoxLayout;
        {
            QVBoxLayout *pBtMetaLayout = new QVBoxLayout;
            {
                m_pleBtName = new QLineEdit;
                m_pleBtName->setClearButtonEnabled(true);

                m_pleBtName->setToolTip("Enter the boat's name");
                m_pteBtDescription = new QTextEdit;
                m_pteBtDescription->setToolTip("Use this field to enter a short text to describe the boat");
                QFont fnt;
                QFontMetrics fm(fnt);
                m_pteBtDescription->setMaximumHeight(fm.height()*3);
                pBtMetaLayout->addWidget(m_pleBtName);
                pBtMetaLayout->addWidget(m_pteBtDescription);
            }

            m_pVPartSplitter = new QSplitter(Qt::Vertical);
            {
                m_pVPartSplitter->setChildrenCollapsible(false);
                m_pPartTabWt = new QTabWidget;
                {
                    QFrame *pSailBox= new QFrame;
                    {
                        QVBoxLayout *pSailPageLayout = new QVBoxLayout;
                        {
                            m_pcptSails = new CPTableView;
                            m_pcptSails->setWindowTitle("Sail definition");
                            m_pcptSails->setSelectionBehavior(QAbstractItemView::SelectRows);
                            m_pcptSails->setEditable(true);

                            m_ppbAddSail    = new QPushButton("Add sail");

                            QMenu *pSailSelMenu = new QMenu("Actions...",this);
                            {
                                QAction *pNURBSSail    = new QAction("NURBS type sail", this);
                                pNURBSSail->setData(QVariant(0));
                                QAction *pBSplineSail   = new QAction("BSpline", this);
                                pBSplineSail->setData(QVariant(1));
                                QAction *pBezierSail   = new QAction("Bezier", this);
                                pBezierSail->setData(QVariant(2));
                                QAction *pPointSail    = new QAction("points", this);
                                pPointSail->setData(QVariant(3));
                                QAction *pCubicSail    = new QAction("cubic", this);
                                pCubicSail->setData(QVariant(4));
                                QAction *pWingSail    = new QAction("Thick wing type sail", this);
                                pWingSail->setData(QVariant(5));
                                QAction *pImportSail    = new QAction("from other boat", this);
                                QAction *pImportXMLSail = new QAction("from XML file", this);
                                QAction *pImportSTLSail = new QAction("from STL file", this);
                                QAction *pImportCADSail = new QAction("from CAD file", this);
                                pImportCADSail->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));

                                connect(pNURBSSail,     SIGNAL(triggered()), SLOT(onAddSail()));
                                connect(pBSplineSail,   SIGNAL(triggered()), SLOT(onAddSail()));
                                connect(pBezierSail,    SIGNAL(triggered()), SLOT(onAddSail()));
                                connect(pPointSail,     SIGNAL(triggered()), SLOT(onAddSail()));
                                connect(pCubicSail,     SIGNAL(triggered()), SLOT(onAddSail()));
                                connect(pWingSail,      SIGNAL(triggered()), SLOT(onAddSail()));
                                connect(pImportSail,    SIGNAL(triggered()), SLOT(onImportSailFromBoat()));
                                connect(pImportXMLSail, SIGNAL(triggered()), SLOT(onImportSailFromXml()));
                                connect(pImportSTLSail, SIGNAL(triggered()), SLOT(onImportSailFromSTL()));
                                connect(pImportCADSail, SIGNAL(triggered()), SLOT(onImportSailFromCAD()));
                                pSailSelMenu->addAction(pNURBSSail);
                                QMenu *pSplineMenu = pSailSelMenu->addMenu("Spline type sail");
                                pSplineMenu->addAction(pBSplineSail);
                                pSplineMenu->addAction(pBezierSail);
                                pSplineMenu->addAction(pPointSail);
                                pSplineMenu->addAction(pCubicSail);
                                pSailSelMenu->addAction(pWingSail);
                                pSailSelMenu->addSeparator();
                                pSailSelMenu->addAction(pImportSail);
                                pSailSelMenu->addAction(pImportXMLSail);
                                pSailSelMenu->addAction(pImportSTLSail);
                                pSailSelMenu->addAction(pImportCADSail);
                                m_ppbAddSail->setMenu(pSailSelMenu);
                            }

                            pSailPageLayout->addWidget(m_pcptSails);
                            pSailPageLayout->addWidget(m_ppbAddSail);

                        }
                        pSailBox->setLayout(pSailPageLayout);
                    }

                    QFrame *pHullBox = new QFrame;
                    {
                        QVBoxLayout *pHullPageLayout = new QVBoxLayout;
                        {
                            m_pcptHulls = new CPTableView;
                            m_pcptHulls->setWindowTitle("Hull definition");
                            m_pcptHulls->setSelectionMode(QAbstractItemView::SingleSelection);
                            m_pcptHulls->setSelectionBehavior(QAbstractItemView::SelectRows);
                            m_pcptHulls->setEditable(true);

                            m_ppbAddHull  = new QPushButton("Add hull");
                            QMenu *pHullSelMenu = new QMenu("Actions...",this);
                            {
                                int n=0;
                                QAction *pFlatPanelsHull   = new QAction("flat quads type", this);
                                pFlatPanelsHull->setData(n);
                                QAction *pNURBSHull   = new QAction("NURBS type", this);
                                n=1;
                                pNURBSHull->setData(n);
                                QAction *pGetHull        = new QAction("from other boat", this);
                                pGetHull->setToolTip("Select an existing hull from another boat");
                                QAction *pImportHullXML  = new QAction("from XML file", this);
                                QAction *pImportHullCAD  = new QAction("from CAD file", this);
                                QAction *pImportHullSTL  = new QAction("from STL file", this);

                                connect(pFlatPanelsHull, SIGNAL(triggered(bool)), SLOT(onAddHull()));
                                connect(pNURBSHull,      SIGNAL(triggered(bool)), SLOT(onAddHull()));
                                connect(pGetHull,        SIGNAL(triggered(bool)), SLOT(onGetHull()));
                                connect(pImportHullXML,  SIGNAL(triggered(bool)), SLOT(onImportHullXML()));
                                connect(pImportHullCAD,  SIGNAL(triggered(bool)), SLOT(onImportHullCAD()));
                                connect(pImportHullSTL,  SIGNAL(triggered(bool)), SLOT(onImportHullSTL()));

                                pHullSelMenu->addAction(pGetHull);
                                pHullSelMenu->addSeparator();
                                pHullSelMenu->addAction(pFlatPanelsHull);
                                pHullSelMenu->addAction(pNURBSHull);
                                pHullSelMenu->addSeparator();
                                pHullSelMenu->addAction(pImportHullXML);
                                pHullSelMenu->addAction(pImportHullCAD);
                                pHullSelMenu->addAction(pImportHullSTL);

                                m_ppbAddHull->setMenu(pHullSelMenu);
                            }

                            QLabel *pLabHullWarning = new QLabel("<p>WARNING:<br>"
                                                                 "Hulls should only be included if fully emerged as in the case of the AC75.<br>"
                                                                 "Hull/water and hull/sail intersections are IGNORED.</p>");
                            pHullPageLayout->addWidget(m_pcptHulls);
                            pHullPageLayout->addWidget(pLabHullWarning);
                            pHullPageLayout->addWidget(m_ppbAddHull);

                        }
                        pHullBox->setLayout(pHullPageLayout);
                    }

                    m_pPartTabWt->addTab(pSailBox, "Sails");
                    m_pPartTabWt->addTab(pHullBox, "Hulls");
                }

                m_pptoOutput = new PlainTextOutput;
                m_pVPartSplitter->addWidget(m_pPartTabWt);
                m_pVPartSplitter->addWidget(m_pptoOutput);
            }

            m_pButtonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Discard);

            pLeftLayout->addLayout(pBtMetaLayout);
            pLeftLayout->addWidget(m_pVPartSplitter);

            pLeftLayout->addWidget(m_pButtonBox);
        }
        pLeftFrame->setLayout(pLeftLayout);
    }

    QFrame *pRightFrame = new QFrame;
    {
        QVBoxLayout *pRightLayout = new QVBoxLayout;
        {
            m_pglBoatView = new gl3dBoatView;

            m_pglBoatCtrls = new gl3dGeomControls(m_pglBoatView, BoatLayout, false);
            m_pglBoatCtrls->enableCtrlPts(false);
            pRightLayout->addWidget(m_pglBoatView);
            pRightLayout->addWidget(m_pglBoatCtrls);
        }
        pRightFrame->setLayout(pRightLayout);
    }

    m_pViewHSplitter = new QSplitter(Qt::Horizontal);
    {
        m_pViewHSplitter->setChildrenCollapsible(false);
        m_pViewHSplitter->addWidget(pLeftFrame);
        m_pViewHSplitter->addWidget(pRightFrame);
    }

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        pMainLayout->addWidget(m_pViewHSplitter);
    }

    setLayout(pMainLayout);
}


void BoatDlg::connectSignals()
{
    connect(m_pViewHSplitter, SIGNAL(splitterMoved(int,int)), SLOT(onResizeTableColumns()));
    connect(m_pPartTabWt,     SIGNAL(currentChanged(int)),    SLOT(onResizeTableColumns()));

    connect(m_pglBoatView,      SIGNAL(pickedNodePair(QPair<int,int>)), SLOT(onPickedNodePair(QPair<int,int>)));
    connect(m_pglBoatCtrls->m_ptbDistance, SIGNAL(clicked()), SLOT(onNodeDistance()));
}


void BoatDlg::showEvent(QShowEvent *pEvent)
{
    XflDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);
    if(s_HSplitterSizes.length()>0) m_pViewHSplitter->restoreState(s_HSplitterSizes);
    if(s_VSplitterSizes.length()>0) m_pVPartSplitter->restoreState(s_VSplitterSizes);

    onResizeTableColumns();

    m_pglBoatView->restoreViewPoint(s_ab_quat);
}


void BoatDlg::hideEvent(QHideEvent *pEvent)
{
    XflDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();

    s_HSplitterSizes = m_pViewHSplitter->saveState();
    s_VSplitterSizes = m_pVPartSplitter->saveState();

    m_pglBoatView->saveViewPoint(s_ab_quat);
}


void BoatDlg::resizeEvent(QResizeEvent *)
{
    onResizeTableColumns();
}


void BoatDlg::onResizeTableColumns()
{
    double w = double(m_pcptSails->width()) / 100.0;
    int ColumnWidth = int(w*20);
    m_pcptSails->setColumnWidth(0,ColumnWidth);
    m_pcptSails->setColumnWidth(1,ColumnWidth);
    m_pcptSails->setColumnWidth(2,ColumnWidth);
    m_pcptSails->setColumnWidth(3,ColumnWidth);

    w = double(m_pcptHulls->width()) / 100.0;
    ColumnWidth = int(w*17);
    m_pcptHulls->setColumnWidth(0,ColumnWidth);
    m_pcptHulls->setColumnWidth(1,ColumnWidth);
    m_pcptHulls->setColumnWidth(2,ColumnWidth);
    m_pcptHulls->setColumnWidth(3,ColumnWidth);

}


void BoatDlg::initDialog(Boat *pBoat, bool bEnableName)
{
    if(!pBoat) return;

    QString strLen;

    m_pBoat = pBoat;
    m_pBoat->makeRefTriMesh(true, true);

    m_pglBoatView->setBoat(pBoat);
    m_pleBtName->setText(m_pBoat->name());
    m_pleBtName->setEnabled(bEnableName);
    m_pteBtDescription->setPlainText(m_pBoat->description());
    Units::getLengthUnitLabel(strLen);

    m_pSailModel = new ActionItemModel(this);
    m_pSailModel->setRowCount(10);//temporary
    m_pSailModel->setColumnCount(4);
    m_pSailModel->setActionColumn(3);
    m_pSailModel->setHeaderData(0, Qt::Horizontal, "Sail Name");
    m_pSailModel->setHeaderData(1, Qt::Horizontal, "x ("+strLen+")");
    m_pSailModel->setHeaderData(2, Qt::Horizontal, "z ("+strLen+")");
    m_pSailModel->setHeaderData(3, Qt::Horizontal, "Actions");
    m_pcptSails->setModel(m_pSailModel);

    QHeaderView *pHorizontalHeader = m_pcptSails->horizontalHeader();
    m_pcptSails->setColumnWidth(0, 80);
    m_pcptSails->setColumnWidth(1, 50);
    m_pcptSails->setColumnWidth(2, 50);
    pHorizontalHeader->setStretchLastSection(true);

    m_pSailDelegate = new XflDelegate(this);
    m_pSailDelegate->setActionColumn(3);
    m_pcptSails->setItemDelegate(m_pSailDelegate);
    m_pSailDelegate->setDigits({-1,3,3,-1});
    m_pSailDelegate->setItemTypes({XflDelegate::STRING, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::ACTION});
    connect(m_pSailDelegate, SIGNAL(commitData(QWidget*)),  SLOT(onSailCellChanged(QWidget*)));
    connect(m_pcptSails,     SIGNAL(clicked(QModelIndex)),  SLOT(onSailItemClicked(QModelIndex)));

    fillSailList();

    m_pHullModel = new ActionItemModel(this);
    m_pHullModel->setRowCount(10);//temporary
    m_pHullModel->setColumnCount(5);
    m_pHullModel->setActionColumn(4);
    m_pHullModel->setHeaderData(0, Qt::Horizontal, "Hull Name");
    m_pHullModel->setHeaderData(1, Qt::Horizontal, "x ("+strLen+")");
    m_pHullModel->setHeaderData(2, Qt::Horizontal, "y ("+strLen+")");
    m_pHullModel->setHeaderData(3, Qt::Horizontal, "z ("+strLen+")");
    m_pHullModel->setHeaderData(4, Qt::Horizontal, "Actions");
    m_pcptHulls->setModel(m_pHullModel);

    pHorizontalHeader = m_pcptHulls->horizontalHeader();
    m_pcptHulls->setColumnWidth(0,80);
    m_pcptHulls->setColumnWidth(1,40);
    m_pcptHulls->setColumnWidth(2,40);
    m_pcptHulls->setColumnWidth(3,40);
    pHorizontalHeader->setStretchLastSection(true);

    m_pHullDelegate = new XflDelegate(this);
    m_pHullDelegate->setActionColumn(4);
    m_pcptHulls->setItemDelegate(m_pHullDelegate);
    m_pHullDelegate->setDigits({-1,3,3,3,-1});
    m_pHullDelegate->setItemTypes({XflDelegate::STRING, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::ACTION});
    connect(m_pHullDelegate,  SIGNAL(commitData(QWidget*)),  SLOT(onHullCellChanged(QWidget*)));
    connect(m_pcptHulls,      SIGNAL(clicked(QModelIndex)),  SLOT(onHullItemClicked(QModelIndex)));

    setControls();
    fillHullList();
}


void BoatDlg::keyPressEvent(QKeyEvent *pEvent)
{
/*    bool bShift = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;*/
    bool bCtrl  = false;
    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_Return:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
            }
            else
            {
                accept();
            }
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        case Qt::Key_M:
        {
            if(bCtrl)
            {
                onEditMainSail();
                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_J:
        {
            if(bCtrl) onEditJib();
            {
                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_H:
        {
            if(bCtrl)
            {
                Fuse *pFuse = m_pBoat->hull();
                if(pFuse) editHull(0);
                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_S:
        {
            if(bCtrl)
                accept();
            break;
        }
//        default:
//            pEvent->ignore();
    }
}


void BoatDlg::onEditMainSail()
{
    Sail *pSail = m_pBoat->mainSail();
    if(pSail)
    {
        editSail(pSail);
        m_pBoat->makeRefTriMesh(true, true);
        m_pglBoatView->resetglSail();
        m_pglBoatView->update();
    }
}


void BoatDlg::onEditJib()
{
    Sail *pSail = m_pBoat->jib();
    if(pSail)
    {
        editSail(pSail);
        m_pBoat->makeRefTriMesh(true, true);
        m_pglBoatView->resetglSail();
        m_pglBoatView->update();
    }
}


void BoatDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent const *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        m_pptoOutput->onAppendThisPlainText(pMsgEvent->msg());

    }
    else
        QDialog::customEvent(pEvent);
}


void BoatDlg::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QMenu *pContextMenu = new QMenu("GraphMenu");
    {
        QAction *pEditMain = new QAction("Edit main sail", this);
        connect(pEditMain, SIGNAL(triggered(bool)), SLOT(onEditMainSail()));
        pEditMain->setEnabled(m_pBoat->mainSail());

        QAction *pEditJib = new QAction("Edit jib", this);
        connect(pEditJib, SIGNAL(triggered(bool)), SLOT(onEditJib()));
        pEditJib->setEnabled(m_pBoat->jib());

        pContextMenu->addAction(pEditMain);
        pContextMenu->addAction(pEditJib);

        QAction *m_pBackImageLoad = new QAction("Load", this);
        connect(m_pBackImageLoad, SIGNAL(triggered()), m_pglBoatView, SLOT(onLoadBackImage()));

        QAction *m_pBackImageClear = new QAction("Clear", this);
        connect(m_pBackImageClear, SIGNAL(triggered()), m_pglBoatView, SLOT(onClearBackImage()));

        QAction *m_pBackImageSettings = new QAction("Settings", this);
        connect(m_pBackImageSettings, SIGNAL(triggered()), m_pglBoatView, SLOT(onBackImageSettings()));

        pContextMenu->addSeparator();
        QMenu *pBackImageMenu = pContextMenu->addMenu("Background image");
        {
            pBackImageMenu->addAction(m_pBackImageLoad);
            pBackImageMenu->addAction(m_pBackImageClear);
            pBackImageMenu->addAction(m_pBackImageSettings);
        }

        /** @todo issue with dialog modality */
/*        QAction *p3dLightAct = new  QAction(QIcon(":/icons/light.png"), "3d light options"), this);
        p3dLightAct->setStatusTip("Define the light options in 3D view"));
        connect(p3dLightAct, SIGNAL(triggered()), m_pglSailView, SLOT(onSetupLight()));
        pContextMenu->addAction(p3dLightAct);*/
    }
//    pContextMenu->exec(pEvent->pos());
    pContextMenu->exec(QCursor::pos());
    update();
    pEvent->accept();
}


void BoatDlg::onAddHull()
{
    Fuse *pFuse = nullptr;
    QAction *pAction = qobject_cast<QAction *>(sender());
    if(pAction->data()==0) pFuse = m_pBoat->makeNewHull(Fuse::FlatFace);
    else                   pFuse = m_pBoat->makeNewHull(Fuse::NURBS);

    QString strong = QString::asprintf("xfl_type_hull_%d", m_pBoat->xflFuseCount());
    pFuse->setPartName(strong);
    pFuse->makeFuseGeometry();
    QString logmsg;
    pFuse->makeDefaultTriMesh(logmsg, QString());

    fillHullList();
    QModelIndex index = m_pHullModel->index(m_pBoat->hullCount()-1, 0, QModelIndex());
    m_pcptHulls->setCurrentIndex(index);

    setControls();

    m_pglBoatView->setBoatReferenceLength(m_pBoat);
    m_pglBoatView->resetglHull();
    m_pglBoatView->update();

    m_bChanged = true;}


void BoatDlg::onInsertFuseXfl()
{
    Fuse *pFuse = m_pBoat->makeNewHull(Fuse::NURBS);
    QString strong;
    strong = QString::asprintf("xfl_type_fuse_%d", m_pBoat->xflFuseCount());
    pFuse->setPartName(strong);
    pFuse->makeFuseGeometry();
    QString logmsg;
    pFuse->makeDefaultTriMesh(logmsg, QString());
//    m_pOccMeshControls->setMeshSettings(m_pBoat->fuse(0));
    setControls();


    m_pglBoatView->setBoatReferenceLength(m_pBoat);
    m_bChanged = true;
    m_pglBoatView->resetglHull();
    m_pglBoatView->update();

//    listPartUniqueIndexes();
}


void BoatDlg::onDeleteHull()
{
    int iSelect = m_pcptHulls->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->hullCount()) return;
    Fuse *pCurHull = m_pBoat->hull(iSelect);
    if(!pCurHull) return;

//    QString strange = "Are you sure you want to delete the Hull "+pCurHull->partName()+"?";
//    int resp = QMessageBox::question(this, "Delete Hull"), strange, QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
//    if(resp != QMessageBox::Yes) return;

    for(int is=m_pBoat->hullCount()-1;is>=0;is--)
    {
        if(is==iSelect)
        {
            //delete
            m_pBoat->removeHull(is);
            break;
        }
    }
    fillHullList();
    if(m_pBoat->hullCount())
    {
        QModelIndex index = m_pHullModel->index(0, 0, QModelIndex());
        m_pcptHulls->setCurrentIndex(index);
    }
    setControls();
    m_pglBoatView->setBoatReferenceLength(m_pBoat);
    m_bChanged = true;
    m_pglBoatView->resetglHull();
    m_pglBoatView->update();
}


void BoatDlg::onImportHullXML()
{
    //    Body memBody;
    //    memBody.duplicate(m_pBody);

    QString PathName;
    PathName = QFileDialog::getOpenFileName(this, "Open XML File",
                                            SaveOptions::xmlPlaneDirName(),
                                            "Plane XML file (*.xml)");
    if(!PathName.length())        return ;

    QFile XFile(PathName);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange = "Could not open the file\n"+PathName;
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
    if(fusereader.fuseXfl())
    {
        FuseXfl *pFuseXfl = fusereader.fuseXfl();
        pFuseXfl->makeFuseGeometry();
        m_pBoat->appendHull(pFuseXfl);
    }

    m_pglBoatView->setBoatReferenceLength(m_pBoat);
    m_bChanged = true;
    m_pglBoatView->resetglHull();
    m_pglBoatView->update();
}


void BoatDlg::onImportHullCAD()
{
    double dimension=0;
    QString logmsg;

    m_pptoOutput->appendPlainText("Importing CAD file...\n");

    QString filter = "CAD Files (*.brep *.stp *.step *.igs *.iges)";
    QString filename = QFileDialog::getOpenFileName(nullptr, "CAD File", SaveOptions::CADDirName(), filter);
    if(!filename.length()) return;
    QFileInfo fi(filename);

    FuseOcc *pNewHullOcc = new FuseOcc;

    pNewHullOcc->setPartName(fi.baseName());

    bool bImport = occ::importCADShapes(filename, pNewHullOcc->shapes(), dimension, 1.e-4, logmsg, this);
    m_pptoOutput->appendPlainText(logmsg+"\n");

    if(!bImport)
    {
        delete pNewHullOcc;
        return;
    }

    pNewHullOcc->makeShellsFromShapes();

    logmsg.clear();
    m_pptoOutput->appendPlainText("Making shell triangulation\n");
    pNewHullOcc->makeShellTriangulation(logmsg, "   ");
    pNewHullOcc->saveBaseTriangulation();
    pNewHullOcc->computeSurfaceProperties(logmsg, "   ");
    m_pptoOutput->appendPlainText(logmsg+"\n");

    m_pptoOutput->appendPlainText("Making default triangular mesh\n");
    logmsg.clear();
    pNewHullOcc->makeDefaultTriMesh(logmsg, QString());
    m_pptoOutput->appendPlainText(logmsg+"\n");

    m_pBoat->appendHull(pNewHullOcc);
    fillHullList();
    QModelIndex index = m_pHullModel->index(m_pBoat->hullCount()-1, 0, QModelIndex());
    m_pcptHulls->setCurrentIndex(index);
    setControls();

    m_pglBoatView->setBoatReferenceLength(m_pBoat);
    m_bChanged = true;
    m_pglBoatView->resetglHull();
    m_pglBoatView->update();
}


void BoatDlg::onImportHullSTL()
{
    StlReaderDlg dlg(this);
    if(dlg.exec()==QDialog::Rejected)
    {
        return;
    }

    if(dlg.triangleList().size()==0)
    {
        m_pptoOutput->appendPlainText("STL import: triangle list is empty\n");
        return; // nothing imported
    }
    m_pptoOutput->appendPlainText(dlg.logMsg());

    QApplication::setOverrideCursor(Qt::WaitCursor);

    FuseStl *pNewHullStl = new FuseStl;
    QString strong;
    strong = QString::asprintf("STL_type_hull_%d", m_pBoat->stlFuseCount());
    pNewHullStl->setPartName(strong);
    pNewHullStl->setBaseTriangles(dlg.triangleList());
    pNewHullStl->setModTriangles(dlg.triangleList());
    pNewHullStl->makeTriangleNodes();
    pNewHullStl->makeNodeNormals();

    m_pBoat->appendHull(pNewHullStl);
    fillHullList();
    QModelIndex index = m_pHullModel->index(m_pBoat->hullCount()-1, 0, QModelIndex());
    m_pcptHulls->setCurrentIndex(index);
    setControls();

    m_pglBoatView->setBoatReferenceLength(m_pBoat);
    m_bChanged = true;
    m_pglBoatView->resetglHull();
    m_pglBoatView->update();

    QApplication::restoreOverrideCursor();
}


void BoatDlg::onInsertFuseStl()
{
}


void BoatDlg::onGetHull()
{
    SelectionDlg dlg(this);
    QStringList NameList;
    QString BoatName, HullName;

    NameList.clear();
    for(int k=0; k<SailObjects::boatCount(); k++)
    {
        Boat *pBoat = SailObjects::boat(k);
        for(int j=0; j<pBoat->hullCount(); j++)
        {
            Fuse*pHull = pBoat->hull(j);
            NameList.append(pBoat->name() + " / " + pHull->name());
        }
    }

    dlg.initDialog("Select hull to import", NameList, QStringList(), true);
    if(dlg.exec()==QDialog::Accepted)
    {
        int pos = dlg.selection().indexOf(" / ");
        if(pos>0)
        {
            m_bChanged = true;
            BoatName = dlg.selection().left(pos);
            HullName = dlg.selection().right(dlg.selection().length()-pos-3);


            Boat const*pBoat = SailObjects::boat(BoatName);
            if(!pBoat) return;

            Fuse *pHull = pBoat->hull(HullName);
            if(!pHull) return;

            Fuse *pNewHull = pHull->clone();
            pNewHull->setPartName(HullName);

            m_pBoat->appendHull(pNewHull);
            fillHullList();
            QModelIndex index = m_pHullModel->index(m_pBoat->hullCount()-1, 0, QModelIndex());
            m_pcptHulls->setCurrentIndex(index);
            setControls();

            m_pglBoatView->setBoatReferenceLength(m_pBoat);
            m_pglBoatView->resetglHull();
            m_pglBoatView->update();

            m_bChanged = true;
        }
    }
}



void BoatDlg::onDuplicateHull()
{
    int iSelect = m_pcptHulls->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->hullCount()) return;
    Fuse *pCurHull = m_pBoat->hull(iSelect);
    if(!pCurHull) return;

    Fuse *pNewFuse = pCurHull->clone();

    m_pBoat->appendHull(pNewFuse);
    fillHullList();
    QModelIndex index = m_pHullModel->index(m_pBoat->hullCount()-1, 0, QModelIndex());
    m_pcptHulls->setCurrentIndex(index);
    setControls();
    m_bChanged = true;
}


void BoatDlg::onRenameHull()
{
    int iHull = m_pcptHulls->currentIndex().row();
    if(iHull<0 || iHull>=m_pHullModel->rowCount()) return;

    NewNameDlg dlg(m_pBoat->hull(iHull)->name(), this);
    if(dlg.exec()==QDialog::Accepted)
    {
        m_pBoat->hull(iHull)->setPartName(dlg.newName());
        fillHullList();
        m_bDescriptionChanged = true;
    }
}


void BoatDlg::onEditHull()
{
    //Get a pointer to the currently selected Hull
    int iSelect = m_pcptHulls->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->hullCount()) return;
    editHull(iSelect);
}


void BoatDlg::editHull(int iFuse)
{
    QString logmsg;
    if(m_pBoat->hull(iFuse)->isXflType())
    {
        FuseXfl *pFuse = dynamic_cast<FuseXfl*>(m_pBoat->hull(iFuse));
        Fuse const *pMemBody = pFuse->clone();

        FuseXflDefDlg glbDlg(this);
        glbDlg.hideSaveAsNew();
        glbDlg.enableName(false);
        glbDlg.initDialog(pFuse);

        if(glbDlg.exec() != QDialog::Accepted)
        {
            pFuse->duplicateFuse(*pMemBody);
            delete pMemBody;
            return;
        }
        delete pMemBody;
    }
    else if(m_pBoat->hull(iFuse)->isOccType())
    {
        FuseOcc *pOccBody = dynamic_cast<FuseOcc*>(m_pBoat->hull(iFuse));
        FuseOcc memBody(*pOccBody);
        FuseOccDlg obDlg(this);
        obDlg.hideSaveAsNew();
        obDlg.initDialog(pOccBody);
        if(obDlg.exec() != QDialog::Accepted)
        {
            pOccBody->duplicateFuse(memBody);
            return;
        }
    }
    else if(m_pBoat->hull(iFuse)->isStlType())
    {
        FuseStl *pStlFuse = dynamic_cast<FuseStl*>(m_pBoat->hull(iFuse));
        FuseStl memFuseStl(*pStlFuse);
        FuseStlDlg sbDlg(this);
        sbDlg.hideSaveAsNew();
        sbDlg.initDialog(pStlFuse);
        if(sbDlg.exec() != QDialog::Accepted)
        {
            pStlFuse->duplicateFuse(memFuseStl);
            return;
        }
    }

    m_bChanged = true;
    m_pBoat->hull(iFuse)->makeFuseGeometry();
    m_pBoat->hull(iFuse)->makeDefaultTriMesh(logmsg, QString());

    fillHullList(); // in case of a change of name
    m_pglBoatView->resetglHull();
    m_pglBoatView->setBoatReferenceLength(m_pBoat);
    m_pglBoatView->update();
}


void BoatDlg::onAddSail()
{
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;
    int isail = pSenderAction->data().toInt();
    Sail *pNewSail=nullptr;
    switch (isail)
    {
        default:
        case 0:
            pNewSail = new NURBSSail;
            pNewSail->setName(QString("NURBS Sail %1").arg(m_pBoat->sailCount()+1));
            break;
        case 1:
            pNewSail = new SplineSail(Spline::BSPLINE);
            pNewSail->setName(QString("BSpline Sail %1").arg(m_pBoat->sailCount()+1));
            break;
        case 2:
            pNewSail = new SplineSail(Spline::BEZIER);
            pNewSail->setName(QString("Bezier spline Sail %1").arg(m_pBoat->sailCount()+1));
            break;
        case 3:
            pNewSail = new SplineSail(Spline::POINT);
            pNewSail->setName(QString("point spline Sail %1").arg(m_pBoat->sailCount()+1));
            break;
        case 4:
            pNewSail = new SplineSail(Spline::CUBIC);
            pNewSail->setName(QString("Cubic spline Sail %1").arg(m_pBoat->sailCount()+1));
            break;
        case 5:
            pNewSail = new WingSail();
            pNewSail->setName(QString("Wing Sail %1").arg(m_pBoat->sailCount()+1));
            break;
    }

    pNewSail->makeDefaultSail();
    m_pBoat->appendSail(pNewSail);
    m_pBoat->makeRefTriMesh(true, true);

    fillSailList();
    QModelIndex index = m_pSailModel->index(m_pBoat->sailCount()-1, 0, QModelIndex());
    m_pcptSails->setCurrentIndex(index);

    setControls();
    m_pglBoatView->resetglSail();
    m_pglBoatView->update();
    m_pglBoatView->setBoatReferenceLength(m_pBoat);
    m_bChanged = true;
}


void BoatDlg::onImportSailFromSTL()
{
    StlReaderDlg STLdlg(this);
    if(STLdlg.exec()==QDialog::Rejected) return;

    if(STLdlg.triangleList().size()==0)
    {
        return; // nothing imported
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QElapsedTimer t; t.start();

    STLSail *pStlSail = new STLSail;
    pStlSail->setName("STL sail");
    pStlSail->setTriangles(STLdlg.triangleList());
    pStlSail->makeTriangulation();

    m_pBoat->appendSail(pStlSail);
    m_pBoat->makeRefTriMesh(true, true);

    fillSailList();
    m_pglBoatView->resetglBoat();
    m_pglBoatView->update();
    m_bChanged = true;

    QApplication::restoreOverrideCursor();
}


void BoatDlg::onImportSailFromCAD()
{
    SailCadReaderDlg dlg(this);
    dlg.initDialog();
    if(dlg.exec()!=QDialog::Accepted) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    if(dlg.bShells())
    {
        //each imported SHELL is a sail
        int ishape=0;
        for(TopTools_ListIteratorOfListOfShape faceit(dlg.shapes()); faceit.More(); faceit.Next())
        {
            TopExp_Explorer shellexplorer;
            for (shellexplorer.Init(faceit.Value(), TopAbs_SHELL); shellexplorer.More(); shellexplorer.Next())
            {
                try
                {
                    TopoDS_Shell shell = TopoDS::Shell(shellexplorer.Current());
                    if(!shell.IsNull())
                    {
                        m_pptoOutput->onAppendThisPlainText(QString::asprintf("Adding shell %d as a sail\n", ishape));
                        
                        OccSail *pNewSail = new OccSail;
                        pNewSail->setName(QString::asprintf("Sail %d", ishape+1));
                        pNewSail->appendShape(shell);
                        pNewSail->makeTriangulation();
                        pNewSail->computeProperties();
                        m_pBoat->appendSail(pNewSail);
                    }
                    else
                    {
                        m_pptoOutput->onAppendThisPlainText(QString::asprintf("Discarding null SHELL %d\n", ishape));
                        
                    }
                }
                catch(Standard_TypeMismatch &ex)
                {
                    m_pptoOutput->onAppendThisPlainText("   Shells not made: "+QString(ex.GetMessageString())+"\n");
                    
                }
                catch(...)
                {

                }
                ishape++;
            }
        }
    }
    else
    {
        //each imported FACE is a sail
        int ishape=0;
        for(TopTools_ListIteratorOfListOfShape faceit(dlg.shapes()); faceit.More(); faceit.Next())
        {
            TopExp_Explorer faceexplorer;
            for (faceexplorer.Init(faceit.Value(), TopAbs_FACE); faceexplorer.More(); faceexplorer.Next())
            {
                try
                {
                    TopoDS_Face face = TopoDS::Face(faceexplorer.Current());
                    BRepAdaptor_Surface surfaceadaptor(face);
                    GeomAdaptor_Surface aGAS = surfaceadaptor.Surface();

                    Handle_Geom_Surface hSurf = aGAS.Surface();
                    if(hSurf.IsNull())
                    {
                        m_pptoOutput->onAppendThisPlainText("Failed to build the FACE's geometric surface.\n");
                        
                        break;
                    }

                    BRepBuilderAPI_MakeShell bodystitcher(hSurf);

                    switch(bodystitcher.Error())
                    {
                        case BRepBuilderAPI_ShellDone:
                        {
                            m_pptoOutput->onAppendThisPlainText("Shell has been built from face sueccessfully\n");
                            
                            break;
                        }
                        case BRepBuilderAPI_EmptyShell:
                        {
                            m_pptoOutput->onAppendThisPlainText("Failed to build SHELL - No initialization of the algorithm: only an empty constructor was used\n");
                            
                            break;
                        }
                        case BRepBuilderAPI_DisconnectedShell:
                        {
                            m_pptoOutput->onAppendThisPlainText("Failed to build SHELL\n");
                            
                            break;
                        }
                        case BRepBuilderAPI_ShellParametersOutOfRange:
                        {
                            m_pptoOutput->onAppendThisPlainText("Failed to build SHELL - The parameters given to limit the surface are out of bounds.\n");
                            
                            break;
                        }
                    }

                    if(bodystitcher.IsDone() && !bodystitcher.Shape().IsNull())
                    {
                        QString strange = QString::asprintf("Adding FACE %d as a sail\n", ishape+1);
                        occ::listShapeContent(bodystitcher.Shape(), strange);
                        m_pptoOutput->onAppendThisPlainText(strange +"\n");
                        
                        OccSail *pNewSail = new OccSail;
                        pNewSail->setName(QString::asprintf("Sail %d", ishape+1));
                        pNewSail->appendShape(bodystitcher.Shape());
                        pNewSail->makeTriangulation();
                        pNewSail->computeProperties();
                        m_pBoat->appendSail(pNewSail);
                    }
                    else
                    {
                        m_pptoOutput->onAppendThisPlainText(QString::asprintf("Discarding null FACE %d\n", ishape+1));
                        
                    }
                }
                catch(Standard_TypeMismatch &ex)
                {
                    m_pptoOutput->onAppendThisPlainText("Shells not made: "+QString(ex.GetMessageString())+"\n");
                    
                }
                catch(...)
                {
                    m_pptoOutput->onAppendThisPlainText(QString::asprintf("Unknown error when converting FACE %d to a SHELL \n", ishape+1));
                    
                }
                ishape++;
            }
        }
    }


    m_pBoat->makeRefTriMesh(true, true);

    fillSailList();
    m_bChanged = true;

    m_pglBoatView->setBoatReferenceLength(m_pBoat);
    m_pglBoatView->resetglBoat();
    m_pglBoatView->updatePartFrame(m_pBoat);
    m_pglBoatView->update();


    QApplication::restoreOverrideCursor();
}


void BoatDlg::onImportSailFromXml()
{
    QString path_to_file;
    path_to_file = QFileDialog::getOpenFileName(nullptr,
                                                QString("Open XML File"),
                                                SaveOptions::xmlPlaneDirName(),
                                                "Plane XML file (*.xml)");
    QFile xmlfile(path_to_file);

    if (!xmlfile.open(QIODevice::ReadOnly))
    {
        QString strange = "Could not open the file\n"+xmlfile.fileName();
        QMessageBox::warning(this, "Warning", strange);
        return;
    }

    XmlSailReader reader(xmlfile);
    if(reader.readXMLSailFile() && reader.sail())
    {
        Sail *pSail = reader.sail();
        m_pBoat->appendSail(pSail);
        m_pBoat->makeRefTriMesh(true, true);

        fillSailList();
        m_pglBoatView->resetglSail();
        m_pglBoatView->update();
        m_bChanged = true;
    }
    else
    {
        QString strong;
        QString errorMsg;
        errorMsg = "Failed to read the file "+path_to_file+"\n";
        strong = QString::asprintf("Line %d column %d",int(reader.lineNumber()),int(reader.columnNumber()));
        errorMsg += reader.errorString() + "\n" + strong;
        m_pptoOutput->appendPlainText(errorMsg+"\n\n");
    }

    m_pglBoatView->setBoatReferenceLength(m_pBoat);
    m_bChanged = true;
    m_pglBoatView->resetglSail();
    m_pglBoatView->update();
}


void BoatDlg::onMoveSailUp()
{
    int iSelect = m_pcptSails->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->sailCount()) return;

    if(iSelect==0) return; // can't move up

    Sail *pPrevSail = m_pBoat->sail(iSelect-1);
    m_pBoat->m_Sail.removeAt(iSelect-1);
    m_pBoat->m_Sail.insert(iSelect, pPrevSail);

    fillSailList();
    m_bChanged = true;
}


void BoatDlg::onMoveSailDown()
{
    int iSelect = m_pcptSails->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->sailCount()) return;

    if(iSelect==m_pBoat->sailCount()-1) return; // can't move down

//    Sail *pCurSail = m_pBoat->sail(iSelect);
    Sail *pNextSail = m_pBoat->sail(iSelect+1);
    m_pBoat->m_Sail.removeAt(iSelect+1);
    m_pBoat->m_Sail.insert(iSelect, pNextSail);

    fillSailList();
    m_bChanged = true;
}


void BoatDlg::onMoveHullUp()
{
    int iSelect = m_pcptHulls->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->hullCount()) return;

    if(iSelect==0) return; // can't move up

    Fuse *pPrevHull = m_pBoat->hull(iSelect-1);
    m_pBoat->m_Hull.removeAt(iSelect-1);
    m_pBoat->m_Hull.insert(iSelect, pPrevHull);

    fillHullList();
    m_bChanged = true;
}


void BoatDlg::onMoveHullDown()
{
    int iSelect = m_pcptHulls->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->hullCount()) return;

    if(iSelect==m_pBoat->hullCount()-1) return; // can't move down

    Fuse *pNextHull = m_pBoat->hull(iSelect+1);
    m_pBoat->m_Hull.removeAt(iSelect+1);
    m_pBoat->m_Hull.insert(iSelect, pNextHull);

    fillHullList();
    m_bChanged = true;
}


void BoatDlg::onDeleteSail()
{
    int iSelect = m_pcptSails->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->sailCount()) return;
    Sail *pCurSail = m_pBoat->sail(iSelect);
    if(!pCurSail) return;

    for(int is=m_pBoat->sailCount()-1;is>=0;is--)
    {
        if(is==iSelect)
        {
            m_pBoat->removeSail(is);
            break;
        }
        setControls();
    }
    fillSailList();
    m_pBoat->makeRefTriMesh(true, true);
    m_pglBoatView->setBoatReferenceLength(m_pBoat);
    m_bChanged = true;
    m_pglBoatView->resetglSail();
    m_pglBoatView->update();
}


void BoatDlg::onDuplicateSail()
{
    int iSelect = m_pcptSails->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->sailCount()) return;
    Sail const *pCurSail = m_pBoat->sail(iSelect);
    if(!pCurSail) return;

    Sail *pNewSail = pCurSail->clone();

    if(pNewSail)
    {
//        pNewSail->duplicate(pCurSail);
        m_pBoat->appendSail(pNewSail);
        pNewSail->setName(QString("Sail %1").arg(m_pBoat->sailCount()));
        QModelIndex index = m_pSailModel->index(m_pBoat->sailCount()-1, 0, QModelIndex());
        m_pcptSails->setCurrentIndex(index);
    }
    fillSailList();
    setControls();
    m_pglBoatView->resetglSail();
    m_pglBoatView->update();

    m_bChanged = true;
}


void BoatDlg::onRenameSail()
{
    int iSail = m_pcptSails->currentIndex().row();
    if(iSail<0 || iSail>=m_pSailModel->rowCount()) return;

    NewNameDlg dlg(m_pBoat->sail(iSail)->name(), this);
    dlg.setQuestion("Enter the sail's new name");
    if(dlg.exec()==QDialog::Accepted)
    {
        m_pBoat->sail(iSail)->setName(dlg.newName());
        fillSailList();
        m_bDescriptionChanged = true;
    }
}


void BoatDlg::onEditSail()
{
    int iSelect = m_pcptSails->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->sailCount()) return;

    Sail *pSail = m_pBoat->sail(iSelect);
    if(!pSail) return;
    editSail(pSail);

    m_pBoat->makeRefTriMesh(true, true);
    m_pglBoatView->resetglSail();
    m_pglBoatView->update();
}


void BoatDlg::editSail(Sail *pSail)
{
    if     (pSail->isNURBSSail())  editNurbsSail(pSail);
    else if(pSail->isSplineSail()) editSplineSail(pSail);
    else if(pSail->isWingSail())   editWingSail(pSail);
    else if(pSail->isStlSail())    editStlSail(pSail);
    else if(pSail->isOccSail())    editOccSail(pSail);

    fillSailList();
    m_pglBoatView->setBoatReferenceLength(m_pBoat);
}


void BoatDlg::editSplineSail(Sail *pSail)
{
    SplineSail *pMemSail = new SplineSail;
    pMemSail->duplicate(pSail);

    SplineSail *pSS = dynamic_cast<SplineSail*>(pSail);

    SplineSailDlg dlg(this);
    dlg.initDialog(pSS);
    if(dlg.exec()!=QDialog::Accepted)
    {
        pSail->duplicate(pMemSail);
        delete pMemSail;
        return;
    }
    pSail->makeSurface();
    pSail->makeTriangulation();
    m_pglBoatView->resetglSail();
}


void BoatDlg::editOccSail(Sail *pSail)
{
    OccSail *pMemSail = new OccSail;
    pMemSail->duplicate(pSail);

    OccSail *pOccSail = dynamic_cast<OccSail*>(pSail);

    OccSailDlg dlg(this);
    dlg.initDialog(pOccSail);
    if(dlg.exec()!=QDialog::Accepted)
    {
        pSail->duplicate(pMemSail);
        delete pMemSail;
        return;
    }
    pSail->makeTriangulation();
    m_pglBoatView->resetglSail();
}


void BoatDlg::editStlSail(Sail *pSail)
{
    STLSail *pMemSail = new STLSail;
    pMemSail->duplicate(pSail);

    STLSail *pSTL = dynamic_cast<STLSail*>(pSail);

    STLSailDlg dlg(this);
    dlg.initDialog(pSTL);
    if(dlg.exec()!=QDialog::Accepted)
    {
        pSail->duplicate(pMemSail);
        delete pMemSail;
        return;
    }
    pSail->makeTriangulation();
    m_pglBoatView->resetglSail();
}


void BoatDlg::editNurbsSail(Sail *pSail)
{
    NURBSSail *pMemSail = new NURBSSail;
    pMemSail->duplicate(pSail);

    NURBSSail *pNS = dynamic_cast<NURBSSail*>(pSail);

    NURBSSailDlg dlg(this);
    dlg.initDialog(pNS);
    if(dlg.exec()!=QDialog::Accepted)
    {
        pSail->duplicate(pMemSail);
        delete pMemSail;
        return;
    }
    pSail->makeSurface();
    pSail->makeTriangulation();
    m_pglBoatView->resetglSail();
}


void BoatDlg::editWingSail(Sail *pSail)
{
    WingSail *pMemSail = new WingSail;
    pMemSail->duplicate(pSail);

    WingSail *pWS = dynamic_cast<WingSail*>(pSail);

    WingSailDlg dlg(this);
    dlg.initDialog(pWS);
    if(dlg.exec()!=QDialog::Accepted)
    {
        pSail->duplicate(pMemSail);
        delete pMemSail;
        return;
    }
    pSail->makeSurface();
    pSail->makeTriangulation();
    m_pglBoatView->resetglSail();
}


void BoatDlg::onImportSailFromBoat()
{
    SelectionDlg dlg;
    QStringList NameList;
    QString BoatName, SailName;

    NameList.clear();
    for(int k=0; k<SailObjects::boatCount(); k++)
    {
        Boat *pBoat = SailObjects::boat(k);
        for(int j=0; j<pBoat->sailCount(); j++)
        {
            Sail *pSail = pBoat->sail(j);
            NameList.append(pBoat->name() + " / " + pSail->name());
        }
    }

    dlg.initDialog("Select sail to import", NameList, QStringList(), true);
    if(dlg.exec()==QDialog::Accepted)
    {
        int pos = dlg.selection().indexOf(" / ");
        if(pos>0)
        {
            m_bChanged = true;
            BoatName = dlg.selection().left(pos).trimmed();
            SailName = dlg.selection().right(dlg.selection().length()-pos-3);

            NURBSSail *pNewSail = new NURBSSail;
            if(pNewSail)
            {
//                pNewSail->duplicate(pSail);
                pNewSail->setName(SailName);

                m_pBoat->appendSail(pNewSail);
                QModelIndex index = m_pSailModel->index(m_pBoat->sailCount()-1, 0, QModelIndex());
                m_pcptSails->setCurrentIndex(index);
                fillSailList();
                setControls();
            }
            m_pglBoatView->resetglSail();
            m_pglBoatView->update();

            m_bChanged = true;
        }
    }
    m_pglBoatView->setBoatReferenceLength(m_pBoat);
}


void BoatDlg::accept()
{
    m_pBoat->setName(m_pleBtName->text());
    m_pBoat->setDescription(m_pteBtDescription->toPlainText());
    readData();
    m_pBoat->makeRefTriMesh(true, true);
    QDialog::accept();
}


void BoatDlg::reject()
{
    if(m_bChanged && xfl::bConfirmDiscard())
    {
        QString strong = "Discard the changes?";
        int Ans = QMessageBox::question(this, "Question", strong,
                                        QMessageBox::Ok | QMessageBox::Cancel);
        if (QMessageBox::Ok == Ans)
        {
            done(QDialog::Rejected);
            return;
        }
        else return;
    }

    done(QDialog::Rejected);
}


void BoatDlg::onSailItemClicked(const QModelIndex &index)
{
    int row = index.row();
    if(row>=0 && row<m_pBoat->sailCount())
    {
        Sail *pSail = m_pBoat->sail(index.row());
        QString strange, frontspacer;
        pSail->properties(strange, frontspacer);
        m_pptoOutput->appendPlainText(strange+"\n\n");
    }

    if(index.column() == m_pSailModel->actionColumn())
    {
        QMenu *pRowMenu = new QMenu("Section",this);

        QAction *pRenameSail = new QAction("Rename", this);
        connect(pRenameSail, SIGNAL(triggered(bool)), this, SLOT(onRenameSail()));
        pRowMenu->addAction(pRenameSail);

        QAction *pEditSail = new QAction("Edit", this);
        connect(pEditSail, SIGNAL(triggered(bool)), this, SLOT(onEditSail()));
        pRowMenu->addAction(pEditSail);

        QAction *pDuplicate = new QAction("Duplicate", this);
        connect(pDuplicate, SIGNAL(triggered(bool)), this, SLOT(onDuplicateSail()));
        pRowMenu->addAction(pDuplicate);

        QAction *pDeleteSail = new QAction("Delete", this);
        connect(pDeleteSail, SIGNAL(triggered(bool)), this, SLOT(onDeleteSail()));
        pRowMenu->addAction(pDeleteSail);

        QAction *pMoveUp   = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowUp),   "Move Up", this);
        QAction *pMoveDown = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowDown), "Move Down", this);
        connect(pMoveUp,   SIGNAL(triggered(bool)), this, SLOT(onMoveSailUp()));
        connect(pMoveDown, SIGNAL(triggered(bool)), this, SLOT(onMoveSailDown()));

        pRowMenu->addSeparator();
        pRowMenu->addAction(pMoveUp);
        pRowMenu->addAction(pMoveDown);

        pRowMenu->exec(QCursor::pos());
    }

    setControls();
}


void BoatDlg::onSailCellChanged(QWidget *)
{
    readData();
    m_pBoat->makeRefTriMesh(true, true);
    m_pglBoatView->resetglSail();
    m_pglBoatView->update();
    m_bChanged = true;
}


void BoatDlg::onHullItemClicked(const QModelIndex &index)
{
    int row = index.row();
    if(row>=0 && row<m_pBoat->hullCount())
    {
        Fuse *pHull = m_pBoat->hull(index.row());
        QString strange;
        pHull->getProperties(strange, QString());
        m_pptoOutput->appendPlainText(strange+"\n\n");
    }

    if(index.column() == m_pHullModel->actionColumn())
    {
        QMenu *pRowMenu = new QMenu("Section",this);

        QAction *pRenameHull = new QAction("Rename", this);
        connect(pRenameHull, SIGNAL(triggered(bool)), this, SLOT(onRenameHull()));
        pRowMenu->addAction(pRenameHull);

        QAction *pEditHull = new QAction("Edit", this);
        connect(pEditHull, SIGNAL(triggered(bool)), this, SLOT(onEditHull()));
        pRowMenu->addAction(pEditHull);

        QAction *pDuplicate = new QAction("Duplicate", this);
        connect(pDuplicate, SIGNAL(triggered(bool)), this, SLOT(onDuplicateHull()));
        pRowMenu->addAction(pDuplicate);

        QAction *pDeleteHull = new QAction("Delete", this);
        connect(pDeleteHull, SIGNAL(triggered(bool)), this, SLOT(onDeleteHull()));
        pRowMenu->addAction(pDeleteHull);

        QAction *pMoveUp   = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowUp), "Move Up", this);
        QAction *pMoveDown = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowDown), "Move Down", this);
        connect(pMoveUp,   SIGNAL(triggered(bool)), this, SLOT(onMoveHullUp()));
        connect(pMoveDown, SIGNAL(triggered(bool)), this, SLOT(onMoveHullDown()));

        pRowMenu->addSeparator();
        pRowMenu->addAction(pMoveUp);
        pRowMenu->addAction(pMoveDown);

        pRowMenu->exec(QCursor::pos());
    }
    setControls();
}


void BoatDlg::onHullCellChanged(QWidget *)
{
    readData();
    m_bChanged = true;
    m_pglBoatView->resetglHull();
    m_pglBoatView->update();
}


void BoatDlg::readData()
{
    for(int is=0; is<m_pSailModel->rowCount(); is++)
    {
        Sail *pSail = m_pBoat->sail(is);
        if(pSail)
        {
            QModelIndex index = m_pSailModel->index(is, 0, QModelIndex());
            pSail->setName(index.data().toString());

            index = m_pSailModel->index(is,1, QModelIndex());
            pSail->m_LE.x = index.data().toDouble()/Units::mtoUnit();

            index = m_pSailModel->index(is,2, QModelIndex());
            pSail->m_LE.z = index.data().toDouble()/Units::mtoUnit();
        }
    }
    for(int ib=0; ib<m_pHullModel->rowCount(); ib++)
    {
        Fuse *pHull = m_pBoat->hull(ib);
        if(pHull)
        {
            QModelIndex index = m_pHullModel->index(ib, 0, QModelIndex());
            pHull->setPartName(index.data().toString());

            index = m_pHullModel->index(ib,1, QModelIndex());
            double x = index.data().toDouble()/Units::mtoUnit();

            index = m_pHullModel->index(ib,2, QModelIndex());
            double y = index.data().toDouble()/Units::mtoUnit();

            index = m_pHullModel->index(ib,3, QModelIndex());
            double z = index.data().toDouble()/Units::mtoUnit();

            m_pBoat->setHullLE(ib, Vector3d(x,y,z));
        }
    }
}


void BoatDlg::fillHullList()
{
    QModelIndex ind;
    m_pHullModel->setRowCount(m_pBoat->hullCount());
    for(int ih=0; ih<m_pBoat->hullCount(); ih++)
    {
        Fuse *pHull = m_pBoat->hull(ih);
        if(pHull)
        {
            ind = m_pHullModel->index(ih, 0, QModelIndex());
            m_pHullModel->setData(ind, pHull->name());

            ind = m_pHullModel->index(ih, 1, QModelIndex());
            m_pHullModel->setData(ind, m_pBoat->hullLE(ih).x * Units::mtoUnit());

            ind = m_pHullModel->index(ih, 2, QModelIndex());
            m_pHullModel->setData(ind, m_pBoat->hullLE(ih).y * Units::mtoUnit());

            ind = m_pHullModel->index(ih, 3, QModelIndex());
            m_pHullModel->setData(ind, m_pBoat->hullLE(ih).z * Units::mtoUnit());
        }
    }
}


void BoatDlg::fillSailList()
{
    //fill the sail list
    QModelIndex ind;
    m_pSailModel->setRowCount(m_pBoat->sailCount());
    for(int is=0; is<m_pBoat->sailCount(); is++)
    {
        Sail *pSail = m_pBoat->sail(is);

        if(pSail)
        {
            ind = m_pSailModel->index(is, 0, QModelIndex());
            m_pSailModel->setData(ind, pSail->name());

            ind = m_pSailModel->index(is, 1, QModelIndex());
            m_pSailModel->setData(ind, pSail->m_LE.x * Units::mtoUnit());

            ind = m_pSailModel->index(is, 2, QModelIndex());
            m_pSailModel->setData(ind, pSail->m_LE.z * Units::mtoUnit());
        }
    }
}


void BoatDlg::setControls()
{
    m_pglBoatView->updatePartFrame(m_pBoat);
}


void BoatDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("BoatDlg");
    {
        s_WindowGeometry = settings.value("WindowGeometry").toByteArray();
        s_HSplitterSizes = settings.value("HSplitterSizes").toByteArray();
        s_VSplitterSizes = settings.value("VSplitterSizes").toByteArray();
    }
    settings.endGroup();
}


void BoatDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("BoatDlg");
    {
        settings.setValue("WindowGeometry", s_WindowGeometry);
        settings.setValue("HSplitterSizes", s_HSplitterSizes);
        settings.setValue("VSplitterSizes", s_VSplitterSizes);
     }
    settings.endGroup();
}


void BoatDlg::onNodeDistance()
{
    if(!m_pglBoatCtrls->getDistance()) m_pglBoatView->clearMeasure();

    m_pglBoatView->setPicking(m_pglBoatCtrls->getDistance() ? xfl::MESHNODE : xfl::NOPICK);
    m_pglBoatView->setSurfacePick(xfl::NOSURFACE);
    m_pglBoatView->update();
}


void BoatDlg::onPickedNodePair(QPair<int, int> nodepair)
{
    if(nodepair.first <0 || nodepair.first >=m_pBoat->refTriMesh().nPanels()) return;
    if(nodepair.second<0 || nodepair.second>=m_pBoat->refTriMesh().nPanels()) return;
    Node nsrc  = m_pBoat->refTriMesh().node(nodepair.first);
    Node ndest = m_pBoat->refTriMesh().node(nodepair.second);

    Segment3d seg(nsrc, ndest);
    m_pglBoatView->setMeasure(seg);

    QString strange;
    strange += QString::asprintf("   distance = %13g ", seg.length()*Units::mtoUnit()) + Units::lengthUnitLabel() + "\n";
    strange += QString::asprintf("         dx = %13g ", seg.segment().x*Units::mtoUnit()) + Units::lengthUnitLabel() + "\n";
    strange += QString::asprintf("         dy = %13g ", seg.segment().y*Units::mtoUnit()) + Units::lengthUnitLabel() + "\n";
    strange += QString::asprintf("         dz = %13g ", seg.segment().z*Units::mtoUnit()) + Units::lengthUnitLabel() + "\n\n";

    m_pptoOutput->onAppendThisPlainText(strange);

    m_pglBoatView->resetPickedNodes();
    m_pglBoatView->update();
}

