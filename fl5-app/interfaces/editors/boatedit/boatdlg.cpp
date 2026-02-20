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
#include <BRepBuilderAPI_MakeShell.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>



#include <api/boat.h>
#include <api/flow5events.h>
#include <api/fuseocc.h>
#include <api/fusestl.h>
#include <api/fusexfl.h>
#include <api/occ_globals.h>
#include <api/planexfl.h>
#include <api/quad3d.h>
#include <api/sail.h>
#include <api/sailnurbs.h>
#include <api/sailobjects.h>
#include <api/sailocc.h>
#include <api/sailspline.h>
#include <api/sailstl.h>
#include <api/sailwing.h>
#include <api/units.h>



#include <core/saveoptions.h>
#include <core/stlreaderdlg.h>
#include <core/xflcore.h>
#include <interfaces/editors/boatedit/boatdlg.h>
#include <interfaces/editors/boatedit/sailcadreaderdlg.h>
#include <interfaces/editors/boatedit/sailnurbsdlg.h>
#include <interfaces/editors/boatedit/sailoccdlg.h>
#include <interfaces/editors/boatedit/sailsplinedlg.h>
#include <interfaces/editors/boatedit/sailstldlg.h>
#include <interfaces/editors/boatedit/sailwingdlg.h>
#include <interfaces/editors/fuseedit/bodyscaledlg.h>
#include <interfaces/editors/fuseedit/fuseoccdlg.h>
#include <interfaces/editors/fuseedit/fusestldlg.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fusexfldefdlg.h>
#include <interfaces/mesh/gmesh_globals.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dboatview.h>
#include <interfaces/widgets/customdlg/newnamedlg.h>
#include <interfaces/widgets/customdlg/selectiondlg.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/customwts/xfldelegate.h>
#include <api/xmlfusereader.h>
#include <api/xmlboatreader.h>
#include <api/xmlsailreader.h>
#include <modules/xobjects.h>

QByteArray BoatDlg::s_WindowGeometry;

QByteArray BoatDlg::s_HSplitterSizes;
QByteArray BoatDlg::s_VSplitterSizes;

Quaternion BoatDlg::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);


BoatDlg::BoatDlg(QWidget *pParent) : XflDialog(pParent)
{
    setWindowTitle(tr("Boat editor"));
    setWindowFlag(Qt::WindowMinMaxButtonsHint);

    m_pBoat = nullptr;
    m_pglBoatView = nullptr;
    m_pHullDelegate = nullptr;
    m_pSailDelegate = nullptr;

    m_bChanged = m_bDescriptionChanged = false;

    setupLayout();
    connectSignals();

    setWindowFlag(Qt::WindowStaysOnTopHint);

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

                m_pleBtName->setToolTip(tr("Enter the boat's name"));
                m_pteBtDescription = new QTextEdit;
                m_pteBtDescription->setToolTip(tr("Use this field to enter a short text to describe the boat"));
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
                            m_pcptSails->setWindowTitle(tr("Sail definition"));
                            m_pcptSails->setSelectionBehavior(QAbstractItemView::SelectRows);
                            m_pcptSails->setEditable(true);

                            m_ppbAddSail    = new QPushButton(tr("Add sail"));

                            QMenu *pSailSelMenu = new QMenu(tr("Actions..."),this);
                            {
                                QAction *pNURBSSail    = new QAction(tr("NURBS type sail"), this);
                                pNURBSSail->setData(QVariant(0));
                                QAction *pBSplineSail   = new QAction(tr("BSpline"), this);
                                pBSplineSail->setData(QVariant(1));
                                QAction *pBezierSail   = new QAction(tr("Bezier"), this);
                                pBezierSail->setData(QVariant(2));
                                QAction *pPointSail    = new QAction(tr("points"), this);
                                pPointSail->setData(QVariant(3));
                                QAction *pCubicSail    = new QAction(tr("cubic"), this);
                                pCubicSail->setData(QVariant(4));
                                QAction *pWingSail    = new QAction(tr("Thick wing type sail"), this);
                                pWingSail->setData(QVariant(5));
                                QAction *pImportSail    = new QAction(tr("from other boat"), this);
                                QAction *pImportXMLSail = new QAction(tr("from XML file"), this);
                                QAction *pImportSTLSail = new QAction(tr("from STL file"), this);
                                QAction *pImportCADSail = new QAction(tr("from CAD file"), this);
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
                                QMenu *pSplineMenu = pSailSelMenu->addMenu(tr("Spline type sail"));
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
                            m_pcptHulls->setWindowTitle(tr("Hull definition"));
                            m_pcptHulls->setSelectionMode(QAbstractItemView::SingleSelection);
                            m_pcptHulls->setSelectionBehavior(QAbstractItemView::SelectRows);
                            m_pcptHulls->setEditable(true);

                            m_ppbAddHull  = new QPushButton(tr("Add hull"));
                            QMenu *pHullSelMenu = new QMenu(tr("Actions..."),this);
                            {
                                int n=0;
                                QAction *pFlatPanelsHull   = new QAction(tr("flat quads type"), this);
                                pFlatPanelsHull->setData(n);
                                QAction *pNURBSHull   = new QAction(tr("NURBS type"), this);
                                n=1;
                                pNURBSHull->setData(n);
                                QAction *pGetHull        = new QAction(tr("from other boat"), this);
                                pGetHull->setToolTip(tr("Select an existing hull from another boat"));
                                QAction *pImportHullXML  = new QAction(tr("from XML file"), this);
                                QAction *pImportHullCAD  = new QAction(tr("from CAD file"), this);
                                QAction *pImportHullSTL  = new QAction(tr("from STL file"), this);
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

                            QLabel *pLabHullWarning = new QLabel(tr("<p>WARNING:<br>"
                                                                 "Hulls should only be included if fully emerged as in the case of the AC75.<br>"
                                                                 "Hull/water and hull/sail intersections are IGNORED.</p>"));
                            pHullPageLayout->addWidget(m_pcptHulls);
                            pHullPageLayout->addWidget(pLabHullWarning);
                            pHullPageLayout->addWidget(m_ppbAddHull);

                        }
                        pHullBox->setLayout(pHullPageLayout);
                    }

                    m_pPartTabWt->addTab(pSailBox, tr("Sails"));
                    m_pPartTabWt->addTab(pHullBox, tr("Hulls"));
                }

                m_ppto = new PlainTextOutput;
                m_pVPartSplitter->addWidget(m_pPartTabWt);
                m_pVPartSplitter->addWidget(m_ppto);
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
    m_pleBtName->setText(QString::fromStdString(m_pBoat->name()));
    m_pleBtName->setEnabled(bEnableName);
    m_pteBtDescription->setPlainText(QString::fromStdString(m_pBoat->description()));
    strLen = Units::lengthUnitQLabel();

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
        m_ppto->onAppendQText(pMsgEvent->msg());
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
        QMenu *pBackImageMenu = pContextMenu->addMenu(tr("Background image"));
        {
            pBackImageMenu->addAction(m_pBackImageLoad);
            pBackImageMenu->addAction(m_pBackImageClear);
            pBackImageMenu->addAction(m_pBackImageSettings);
        }

        /** @todo issue with dialog modality */
        QAction *p3dLightAct = new  QAction(QIcon(":/icons/light.png"), tr("3d light options"), this);
        connect(p3dLightAct, SIGNAL(triggered()), m_pglBoatView, SLOT(onSetupLight()));
        pContextMenu->addAction(p3dLightAct);
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
    pFuse->setName(strong.toStdString());
    pFuse->makeFuseGeometry();
    std::string logmsg;
    pFuse->makeDefaultTriMesh(logmsg, "");

    fillHullList();
    QModelIndex index = m_pHullModel->index(m_pBoat->nHulls()-1, 0, QModelIndex());
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
    pFuse->setName(strong.toStdString());
    pFuse->makeFuseGeometry();
    std::string logmsg;
    pFuse->makeDefaultTriMesh(logmsg, "");
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
    if(iSelect<0 || iSelect>=m_pBoat->nHulls()) return;
    Fuse *pCurHull = m_pBoat->hull(iSelect);
    if(!pCurHull) return;

    for(int is=m_pBoat->nHulls()-1;is>=0;is--)
    {
        if(is==iSelect)
        {
            //delete
            m_pBoat->deleteHull(is);
            break;
        }
    }
    fillHullList();
    if(m_pBoat->nHulls())
    {
        QModelIndex index = m_pHullModel->index(0, 0, QModelIndex());
        m_pcptHulls->setCurrentIndex(index);
    }
    setControls();
    m_pglBoatView->setBoatReferenceLength(m_pBoat);
    m_pglBoatView->resetglHull();
    m_pglBoatView->update();
    m_bChanged = true;
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
    std::string strlog;

    m_ppto->appendPlainText("Importing CAD file...\n");

    QString filter = "CAD Files (*.brep *.stp *.step *.igs *.iges)";
    QString filename = QFileDialog::getOpenFileName(nullptr, "CAD File", SaveOptions::CADDirName(), filter);
    if(!filename.length()) return;
    QFileInfo fi(filename);

    FuseOcc *pNewHullOcc = new FuseOcc;

    pNewHullOcc->setName(fi.baseName().toStdString());

    bool bImport = occ::importCADShapes(filename.toStdString(), pNewHullOcc->shapes(), dimension, strlog);
    m_ppto->onAppendQText(QString::fromStdString(strlog)+"\n");

    if(!bImport)
    {
        delete pNewHullOcc;
        return;
    }

    pNewHullOcc->makeShellsFromShapes();

    logmsg.clear();
    m_ppto->appendPlainText("Making shell triangulation\n");

    gmesh::makeFuseTriangulation(pNewHullOcc, logmsg, "   ");
    pNewHullOcc->saveBaseTriangulation();
    pNewHullOcc->computeSurfaceProperties(strlog, "   ");
    logmsg += QString::fromStdString(strlog);
    m_ppto->onAppendQText(logmsg+"\n");

    m_ppto->onAppendStdText("Making default triangular mesh\n");
    logmsg.clear();
    pNewHullOcc->makeDefaultTriMesh(strlog, "");
    m_ppto->onAppendStdText(strlog+"\n");

    m_pBoat->appendHull(pNewHullOcc);
    fillHullList();
    QModelIndex index = m_pHullModel->index(m_pBoat->nHulls()-1, 0, QModelIndex());
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
        m_ppto->appendPlainText("STL import: triangle list is empty\n");
        return; // nothing imported
    }
    m_ppto->appendPlainText(dlg.logMsg());

    QApplication::setOverrideCursor(Qt::WaitCursor);

    FuseStl *pNewHullStl = new FuseStl;
    QString strong;
    strong = QString::asprintf("STL_type_hull_%d", m_pBoat->stlFuseCount());
    pNewHullStl->setName(strong.toStdString());
    pNewHullStl->setBaseTriangles(dlg.triangleList());
    pNewHullStl->setTriangles(dlg.triangleList());
    pNewHullStl->makeTriangleNodes();
    pNewHullStl->makeNodeNormals();

    m_pBoat->appendHull(pNewHullStl);
    fillHullList();
    QModelIndex index = m_pHullModel->index(m_pBoat->nHulls()-1, 0, QModelIndex());
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
    for(int k=0; k<SailObjects::nBoats(); k++)
    {
        Boat *pBoat = SailObjects::boat(k);
        for(int j=0; j<pBoat->nHulls(); j++)
        {
            Fuse*pHull = pBoat->hull(j);
            NameList.append(QString::fromStdString(pBoat->name()) + " / " + QString::fromStdString(pHull->name()));
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


            Boat const*pBoat = SailObjects::boat(BoatName.toStdString());
            if(!pBoat) return;

            Fuse *pHull = pBoat->hull(HullName.toStdString());
            if(!pHull) return;

            Fuse *pNewHull = pHull->clone();
            pNewHull->setName(HullName.toStdString());

            m_pBoat->appendHull(pNewHull);
            fillHullList();
            QModelIndex index = m_pHullModel->index(m_pBoat->nHulls()-1, 0, QModelIndex());
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
    if(iSelect<0 || iSelect>=m_pBoat->nHulls()) return;
    Fuse *pCurHull = m_pBoat->hull(iSelect);
    if(!pCurHull) return;

    Fuse *pNewFuse = pCurHull->clone();

    m_pBoat->appendHull(pNewFuse);
    fillHullList();
    QModelIndex index = m_pHullModel->index(m_pBoat->nHulls()-1, 0, QModelIndex());
    m_pcptHulls->setCurrentIndex(index);
    setControls();

    m_pglBoatView->setBoatReferenceLength(m_pBoat);
    m_pglBoatView->resetglHull();
    m_pglBoatView->update();

    m_bChanged = true;

}


void BoatDlg::onRenameHull()
{
    int iHull = m_pcptHulls->currentIndex().row();
    if(iHull<0 || iHull>=m_pHullModel->rowCount()) return;

    NewNameDlg dlg(QString::fromStdString(m_pBoat->hull(iHull)->name()), this);
    if(dlg.exec()==QDialog::Accepted)
    {
        m_pBoat->hull(iHull)->setName(dlg.newName().toStdString());
        fillHullList();
        m_bDescriptionChanged = true;
    }
}


void BoatDlg::onEditHull()
{
    //Get a pointer to the currently selected Hull
    int iSelect = m_pcptHulls->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->nHulls()) return;
    editHull(iSelect);
}


void BoatDlg::editHull(int iFuse)
{
    std::string logmsg;
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
    m_pBoat->hull(iFuse)->makeDefaultTriMesh(logmsg, "");

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
            pNewSail = new SailNurbs;
            pNewSail->setName(QString("NURBS Sail %1").arg(m_pBoat->nSails()+1).toStdString());
            break;
        case 1:
            pNewSail = new SailSpline(Spline::BSPLINE);
            pNewSail->setName(QString("BSpline Sail %1").arg(m_pBoat->nSails()+1).toStdString());
            break;
        case 2:
            pNewSail = new SailSpline(Spline::BEZIER);
            pNewSail->setName(QString("Bezier spline Sail %1").arg(m_pBoat->nSails()+1).toStdString());
            break;
        case 3:
            pNewSail = new SailSpline(Spline::POINT);
            pNewSail->setName(QString("point spline Sail %1").arg(m_pBoat->nSails()+1).toStdString());
            break;
        case 4:
            pNewSail = new SailSpline(Spline::CUBIC);
            pNewSail->setName(QString("Cubic spline Sail %1").arg(m_pBoat->nSails()+1).toStdString());
            break;
        case 5:
            pNewSail = new SailWing();
            pNewSail->setName(QString("Wing Sail %1").arg(m_pBoat->nSails()+1).toStdString());
            break;
    }

    pNewSail->makeDefaultSail();
    m_pBoat->appendSail(pNewSail);
    m_pBoat->makeRefTriMesh(true, true);

    fillSailList();
    QModelIndex index = m_pSailModel->index(m_pBoat->nSails()-1, 0, QModelIndex());
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

    SailStl *pStlSail = new SailStl;
    pStlSail->setName("STL sail");
    pStlSail->setTriangles(STLdlg.triangleList());
    Objects3d::makeSailTriangulation(pStlSail);

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
                        m_ppto->onAppendQText(QString::asprintf("Adding shell %d as a sail\n", ishape));
                        
                        SailOcc *pNewSail = new SailOcc;
                        pNewSail->setName(QString::asprintf("Sail %d", ishape+1).toStdString());
                        pNewSail->appendShape(shell);
                        pNewSail->shapesToBreps();
                        Objects3d::makeSailTriangulation(pNewSail);

                        pNewSail->computeProperties();
                        m_pBoat->appendSail(pNewSail);
                    }
                    else
                    {
                        m_ppto->onAppendQText(QString::asprintf("Discarding null SHELL %d\n", ishape));
                        
                    }
                }
                catch(Standard_TypeMismatch &ex)
                {
                    m_ppto->onAppendQText("   Shells not made: "+QString(ex.GetMessageString())+"\n");
                    
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

                    Handle(Geom_Surface) hSurf = aGAS.Surface();
                    if(hSurf.IsNull())
                    {
                        m_ppto->onAppendQText("Failed to build the FACE's geometric surface.\n");
                        
                        break;
                    }

                    BRepBuilderAPI_MakeShell bodystitcher(hSurf);

                    switch(bodystitcher.Error())
                    {
                        case BRepBuilderAPI_ShellDone:
                        {
                            m_ppto->onAppendQText("Shell has been built from face sueccessfully\n");
                            
                            break;
                        }
                        case BRepBuilderAPI_EmptyShell:
                        {
                            m_ppto->onAppendQText("Failed to build SHELL - No initialization of the algorithm: only an empty constructor was used\n");
                            
                            break;
                        }
                        case BRepBuilderAPI_DisconnectedShell:
                        {
                            m_ppto->onAppendQText("Failed to build SHELL\n");
                            
                            break;
                        }
                        case BRepBuilderAPI_ShellParametersOutOfRange:
                        {
                            m_ppto->onAppendQText("Failed to build SHELL - The parameters given to limit the surface are out of bounds.\n");
                            
                            break;
                        }
                    }

                    if(bodystitcher.IsDone() && !bodystitcher.Shape().IsNull())
                    {
                        QString strange = QString::asprintf("Adding FACE %d as a sail\n", ishape+1);
                        std::string str;
                        occ::listShapeContent(bodystitcher.Shape(), str);
                        m_ppto->onAppendQText(strange + QString::fromStdString(str) +"\n");
                        
                        SailOcc *pNewSail = new SailOcc;
                        pNewSail->setName(QString::asprintf("Sail %d", ishape+1).toStdString());
                        pNewSail->appendShape(bodystitcher.Shape());
                        pNewSail->shapesToBreps();
                        Objects3d::makeSailTriangulation(pNewSail);
                        pNewSail->computeProperties();
                        m_pBoat->appendSail(pNewSail);
                    }
                    else
                    {
                        m_ppto->onAppendQText(QString::asprintf("Discarding null FACE %d\n", ishape+1));
                        
                    }
                }
                catch(Standard_TypeMismatch &ex)
                {
                    m_ppto->onAppendQText("Shells not made: "+QString(ex.GetMessageString())+"\n");
                    
                }
                catch(...)
                {
                    m_ppto->onAppendQText(QString::asprintf("Unknown error when converting FACE %d to a SHELL \n", ishape+1));
                    
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
        m_ppto->appendPlainText(errorMsg+"\n\n");
    }

    m_pglBoatView->setBoatReferenceLength(m_pBoat);
    m_bChanged = true;
    m_pglBoatView->resetglSail();
    m_pglBoatView->update();
}


void BoatDlg::onMoveSailUp()
{
    int iSelect = m_pcptSails->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->nSails()) return;

    if(iSelect==0) return; // can't move up

    Sail *pPrevSail = m_pBoat->sail(iSelect-1);
    m_pBoat->removeSailAt(iSelect-1);
    m_pBoat->insertSailAt(iSelect, pPrevSail);

    fillSailList();
    m_bChanged = true;
}


void BoatDlg::onMoveSailDown()
{
    int iSelect = m_pcptSails->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->nSails()) return;

    if(iSelect==m_pBoat->nSails()-1) return; // can't move down

//    Sail *pCurSail = m_pBoat->sail(iSelect);
    Sail *pNextSail = m_pBoat->sail(iSelect+1);
    m_pBoat->removeSailAt(iSelect+1);
    m_pBoat->insertSailAt(iSelect, pNextSail);

    fillSailList();
    m_bChanged = true;
}


void BoatDlg::onMoveHullUp()
{
    int iSelect = m_pcptHulls->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->nHulls()) return;

    if(iSelect==0) return; // can't move up

    Fuse *pPrevHull = m_pBoat->hull(iSelect-1);
    m_pBoat->removeHullAt(iSelect-1);
    m_pBoat->insertHullAt(iSelect, pPrevHull);

    fillHullList();
    m_bChanged = true;
}


void BoatDlg::onMoveHullDown()
{
    int iSelect = m_pcptHulls->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->nHulls()) return;

    if(iSelect==m_pBoat->nHulls()-1) return; // can't move down

    Fuse *pNextHull = m_pBoat->hull(iSelect+1);
    m_pBoat->removeHullAt(iSelect+1);
    m_pBoat->insertHullAt(iSelect, pNextHull);

    fillHullList();
    m_bChanged = true;
}


void BoatDlg::onDeleteSail()
{
    int iSelect = m_pcptSails->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->nSails()) return;
    Sail *pCurSail = m_pBoat->sail(iSelect);
    if(!pCurSail) return;

    for(int is=m_pBoat->nSails()-1;is>=0;is--)
    {
        if(is==iSelect)
        {
            m_pBoat->deleteSail(is);
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
    if(iSelect<0 || iSelect>=m_pBoat->nSails()) return;
    Sail const *pCurSail = m_pBoat->sail(iSelect);
    if(!pCurSail) return;

    Sail *pNewSail = pCurSail->clone();

    if(pNewSail)
    {
//        pNewSail->duplicate(pCurSail);
        m_pBoat->appendSail(pNewSail);
        pNewSail->setName(QString("Sail %1").arg(m_pBoat->nSails()).toStdString());
        QModelIndex index = m_pSailModel->index(m_pBoat->nSails()-1, 0, QModelIndex());
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

    NewNameDlg dlg(QString::fromStdString(m_pBoat->sail(iSail)->name()), this);
    dlg.setQuestion("Enter the sail's new name");
    if(dlg.exec()==QDialog::Accepted)
    {
        m_pBoat->sail(iSail)->setName(dlg.newName().toStdString());
        fillSailList();
        m_bDescriptionChanged = true;
    }
}


void BoatDlg::onEditSail()
{
    int iSelect = m_pcptSails->currentIndex().row();
    if(iSelect<0 || iSelect>=m_pBoat->nSails()) return;

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
    SailSpline *pMemSail = new SailSpline;
    pMemSail->duplicate(pSail);

    SailSpline *pSS = dynamic_cast<SailSpline*>(pSail);

    SailSplineDlg dlg(this);
    dlg.initDialog(pSS);
    if(dlg.exec()!=QDialog::Accepted)
    {
        pSail->duplicate(pMemSail);
        delete pMemSail;
        return;
    }
    pSail->makeSurface();
    Objects3d::makeSailTriangulation(pSail);
    m_pglBoatView->resetglSail();
}


void BoatDlg::editOccSail(Sail *pSail)
{
    SailOcc *pMemSail = new SailOcc;
    pMemSail->duplicate(pSail);

    SailOcc *pOccSail = dynamic_cast<SailOcc*>(pSail);

    SailOccDlg dlg(this);
    dlg.initDialog(pOccSail);
    if(dlg.exec()!=QDialog::Accepted)
    {
        pSail->duplicate(pMemSail);
        delete pMemSail;
        return;
    }
    Objects3d::makeSailTriangulation(pSail);
    m_pglBoatView->resetglSail();
}


void BoatDlg::editStlSail(Sail *pSail)
{
    SailStl *pMemSail = new SailStl;
    pMemSail->duplicate(pSail);

    SailStl *pSTL = dynamic_cast<SailStl*>(pSail);

    SailStlDlg dlg(this);
    dlg.initDialog(pSTL);
    if(dlg.exec()!=QDialog::Accepted)
    {
        pSail->duplicate(pMemSail);
        delete pMemSail;
        return;
    }
    Objects3d::makeSailTriangulation(pSail);
    m_pglBoatView->resetglSail();
}


void BoatDlg::editNurbsSail(Sail *pSail)
{
    SailNurbs *pMemSail = new SailNurbs;
    pMemSail->duplicate(pSail);

    SailNurbs *pNS = dynamic_cast<SailNurbs*>(pSail);

    SailNurbsDlg dlg(this);
    dlg.initDialog(pNS);
    if(dlg.exec()!=QDialog::Accepted)
    {
        pSail->duplicate(pMemSail);
        delete pMemSail;
        return;
    }
    pSail->makeSurface();
    Objects3d::makeSailTriangulation(pSail);
    m_pglBoatView->resetglSail();
}


void BoatDlg::editWingSail(Sail *pSail)
{
    SailWing *pMemSail = new SailWing;
    pMemSail->duplicate(pSail);

    SailWing *pWS = dynamic_cast<SailWing*>(pSail);

    SailWingDlg dlg(this);
    dlg.initDialog(pWS);
    if(dlg.exec()!=QDialog::Accepted)
    {
        pSail->duplicate(pMemSail);
        delete pMemSail;
        return;
    }
    pSail->makeSurface();
    Objects3d::makeSailTriangulation(pSail);
    m_pglBoatView->resetglSail();
}


void BoatDlg::onImportSailFromBoat()
{
    SelectionDlg dlg;
    QStringList NameList;
    QString BoatName, SailName;

    NameList.clear();
    for(int k=0; k<SailObjects::nBoats(); k++)
    {
        Boat *pBoat = SailObjects::boat(k);
        for(int j=0; j<pBoat->nSails(); j++)
        {
            Sail *pSail = pBoat->sail(j);
            NameList.append(QString::fromStdString(pBoat->name() + " / " + pSail->name()));
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

            SailNurbs *pNewSail = new SailNurbs;
            if(pNewSail)
            {
//                pNewSail->duplicate(pSail);
                pNewSail->setName(SailName.toStdString());

                m_pBoat->appendSail(pNewSail);
                QModelIndex index = m_pSailModel->index(m_pBoat->nSails()-1, 0, QModelIndex());
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
    m_pBoat->setName(m_pleBtName->text().toStdString());
    m_pBoat->setDescription(m_pteBtDescription->toPlainText().toStdString());
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
    if(row>=0 && row<m_pBoat->nSails())
    {
        Sail *pSail = m_pBoat->sail(index.row());
        std::string strange, frontspacer;
        pSail->properties(strange, frontspacer);
        m_ppto->onAppendStdText(strange+"\n\n");
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
    if(row>=0 && row<m_pBoat->nHulls())
    {
        Fuse *pHull = m_pBoat->hull(index.row());
        std::string strange;
        pHull->getProperties(strange, "");
        m_ppto->onAppendStdText(strange+"\n\n");
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
            pSail->setName(index.data().toString().toStdString());

            index = m_pSailModel->index(is,1, QModelIndex());
            double x = index.data().toDouble()/Units::mtoUnit();

            index = m_pSailModel->index(is,2, QModelIndex());
            double z = index.data().toDouble()/Units::mtoUnit();

            pSail->setPosition(x, 0.0, z);
        }
    }
    for(int ib=0; ib<m_pHullModel->rowCount(); ib++)
    {
        Fuse *pHull = m_pBoat->hull(ib);
        if(pHull)
        {
            QModelIndex index = m_pHullModel->index(ib, 0, QModelIndex());
            pHull->setName(index.data().toString().toStdString());

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
    m_pHullModel->setRowCount(m_pBoat->nHulls());
    for(int ih=0; ih<m_pBoat->nHulls(); ih++)
    {
        Fuse *pHull = m_pBoat->hull(ih);
        if(pHull)
        {
            ind = m_pHullModel->index(ih, 0, QModelIndex());
            m_pHullModel->setData(ind, QString::fromStdString(pHull->name()));

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
    m_pSailModel->setRowCount(m_pBoat->nSails());
    for(int is=0; is<m_pBoat->nSails(); is++)
    {
        Sail *pSail = m_pBoat->sail(is);

        if(pSail)
        {
            ind = m_pSailModel->index(is, 0, QModelIndex());
            m_pSailModel->setData(ind, QString::fromStdString(pSail->name()));

            ind = m_pSailModel->index(is, 1, QModelIndex());
            m_pSailModel->setData(ind, pSail->position().x * Units::mtoUnit());

            ind = m_pSailModel->index(is, 2, QModelIndex());
            m_pSailModel->setData(ind, pSail->position().z * Units::mtoUnit());
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
    strange += QString::asprintf("   distance = %13g ", seg.length()*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n";
    strange += QString::asprintf("         dx = %13g ", seg.segment().x*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n";
    strange += QString::asprintf("         dy = %13g ", seg.segment().y*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n";
    strange += QString::asprintf("         dz = %13g ", seg.segment().z*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n\n";

    m_ppto->onAppendQText(strange);

    m_pglBoatView->resetPickedNodes();
    m_pglBoatView->update();
}

