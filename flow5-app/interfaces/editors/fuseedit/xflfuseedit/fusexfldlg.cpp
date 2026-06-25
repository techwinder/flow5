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



#include <QAction>
#include <QColorDialog>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QTime>
#include <QVBoxLayout>
#include <QVBoxLayout>

#include "fusexfldlg.h"

#include <api/frame.h>
#include <api/fusenurbs.h>
#include <api/fusesections.h>
#include <api/fusexfl.h>
#include <api/quad3d.h>
#include <api/units.h>
#include <api/xmlfusewriter.h>


#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/editors/fuseedit/bodyscaledlg.h>
#include <interfaces/editors/fuseedit/bodytransdlg.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fuseframewt.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fuselinewt.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dfuseview.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/customwts/xfldelegate.h>


QByteArray FuseXflDlg::s_MainSplitterSizes;
QByteArray FuseXflDlg::s_TableSplitterSizes;
int FuseXflDlg::s_PageIndex = 0;

Grid FuseXflDlg::s_BodyLineGrid;
Grid FuseXflDlg::s_FrameGrid;



bool FuseXflDlg::s_bShowCtrlPoints=false;

QByteArray FuseXflDlg::s_VViewSplitterSizes, FuseXflDlg::s_HViewSplitterSizes;

QByteArray FuseXflDlg::s_Geometry;


FuseXflDlg::FuseXflDlg(QWidget *pParent) : FuseDlg(pParent)
{
    setWindowTitle(tr("Xfl fuse editor"));

    m_pFuseXfl = nullptr;
    m_StackPos  = 0; //the current position on the stack
    m_bChanged = false;

    makeCommonWts();

    m_pPointDelegate = nullptr;
    m_pFrameDelegate = nullptr;

    m_pPointModel = nullptr;
    m_pFrameModel = nullptr;

    createActions();

    makeTables();
    setTableUnits();

    setupLayout();
    connectSignals();
}


FuseXflDlg::~FuseXflDlg()
{
    clearStack(-1);
}


void FuseXflDlg::createActions()
{
    m_pResetFuse     = new QAction(tr("Restore geometry and mesh"), this);
    m_pScaleBody     = new QAction(tr("Scale"), this);
    m_pExportBodyXML = new QAction(tr("Export body geometry to an XML file"), this);
    m_pTranslateBody = new QAction(tr("Translate"), this);

    QMenu *pBodyMenu = new QMenu(tr("Actions..."), this);
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
    }
    m_ppbMenuButton->setMenu(pBodyMenu);
}


void FuseXflDlg::connectSignals()
{
    connectBaseSignals();

    // action signals
    connect(m_pResetFuse,      SIGNAL(triggered()),           SLOT(onResetFuse()));
    connect(m_pScaleBody,      SIGNAL(triggered()),           SLOT(onScaleFuse()));
    connect(m_pExportBodyXML,  SIGNAL(triggered()),           SLOT(onExportFuseToXML()));
    connect(m_pTranslateBody,  SIGNAL(triggered()),           SLOT(onTranslateFuse()));

    // view signals
    connect(m_pFuseLineView,   SIGNAL(scaleFuse(bool)),       SLOT(onScaleFuse(bool)));
    connect(m_pFuseLineView,   SIGNAL(translateFuse()),       SLOT(onTranslateFuse()));
    connect(m_pFuseLineView,   SIGNAL(insertFrame(Vector3d)), SLOT(onInsertFrame(Vector3d)));
    connect(m_pFuseLineView,   SIGNAL(removeFrame(int)),      SLOT(onRemoveFrame(int)));

    connect(m_pFrameView,      SIGNAL(scaleBody(bool)),       SLOT(onScaleFuse(bool)));
    connect(m_pFrameView,      SIGNAL(insertPoint(Vector3d)), SLOT(onInsertPoint(Vector3d)));
    connect(m_pFrameView,      SIGNAL(removePoint(int)),      SLOT(onRemovePoint(int)));

    connect(m_ppbUndo,         SIGNAL(clicked()),              SLOT(onUndo()));
    connect(m_ppbRedo,         SIGNAL(clicked()),              SLOT(onRedo()));

    connect(m_pslBunchAmp,     SIGNAL(sliderReleased()),       SLOT(onNURBSPanels()));

    connect(m_pieNHoopPanels,  SIGNAL(intChanged(int)),        SLOT(onNURBSPanels()));
    connect(m_pieNXPanels,     SIGNAL(intChanged(int)),        SLOT(onNURBSPanels()));
    connect(m_pcbXDegree,      SIGNAL(activated(int)),         SLOT(onSelChangeXDegree(int)));
    connect(m_pcbHoopDegree,   SIGNAL(activated(int)),         SLOT(onSelChangeHoopDegree(int)));

    // table signals
    connect(m_pcptFrameTable,  SIGNAL(clicked(QModelIndex)),   SLOT(onFrameItemClicked(QModelIndex)));
    connect(m_pcptFrameTable,  SIGNAL(dataPasted()),           SLOT(onFrameCellChanged()));
    connect(m_pFrameDelegate,  SIGNAL(closeEditor(QWidget*)),  SLOT(onFrameCellChanged()));
    connect(m_pcptFrameTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onSelectFrame(QModelIndex)));

    connect(m_pcptPointTable,  SIGNAL(clicked(QModelIndex)),   SLOT(onPointItemClicked(QModelIndex)));
    connect(m_pcptPointTable,  SIGNAL(dataPasted()),           SLOT(onPointCellChanged()));
    connect(m_pPointDelegate,  SIGNAL(closeEditor(QWidget*)),  SLOT(onPointCellChanged()));

    connect(m_pcptFrameTable,  SIGNAL(tableResized()),         SLOT(onResizeTables()));

    connect(m_pFuseLineView,   SIGNAL(selectedChanged(int)),   SLOT(onFrameClickedIn2dView()));
    connect(m_pFrameView,      SIGNAL(selectedChanged(int)),   SLOT(onPointClickedIn2dView()));

    connect(m_pFuseLineView,   SIGNAL(mouseDragReleased()),    SLOT(onUpdateFuseDlg()));
    connect(m_pFrameView,      SIGNAL(mouseDragReleased()),    SLOT(onUpdateFuseDlg()));

    connect(m_pFrameView,      SIGNAL(frameSelected(int)),     SLOT(onSelectFrame(int)));

    connect(m_pfeFitPrecision, SIGNAL(floatChanged(float)),    SLOT(onFitPrecision()));
}


void FuseXflDlg::setTableUnits()
{
    QString length;
    length = Units::lengthUnitQLabel();

    m_pFrameModel->setHeaderData(0, Qt::Horizontal, "x ("+length+")");
    m_pFrameModel->setHeaderData(1, Qt::Horizontal, tr("NPanels"));
    m_pFrameModel->setHeaderData(2, Qt::Horizontal, tr("Actions"));

    m_pPointModel->setHeaderData(0, Qt::Horizontal, "y ("+length+")");
    m_pPointModel->setHeaderData(1, Qt::Horizontal, "z ("+length+")");
    m_pPointModel->setHeaderData(2, Qt::Horizontal, tr("NPanels"));
    m_pPointModel->setHeaderData(3, Qt::Horizontal, tr("Actions"));
}


void FuseXflDlg::fillFrameDataTable()
{
    if(!m_pFuseXfl) return;

    if(!m_pFuseXfl->isSectionType())
    {
        m_pFrameModel->setRowCount(m_pFuseXfl->frameCount());
        for(int row=0; row<m_pFuseXfl->frameCount(); row++)
        {
            QModelIndex ind;

            ind = m_pFrameModel->index(row, 0, QModelIndex());
            m_pFrameModel->setData(ind, m_pFuseXfl->frame(row).position().x * Units::mtoUnit());

            ind = m_pFrameModel->index(row, 1, QModelIndex());
            m_pFrameModel->setData(ind, m_pFuseXfl->xPanels(row));
        }
    }
    else
    {
        QModelIndex ind;
        FuseSections const *pFuseSections = dynamic_cast<FuseSections const *>(m_pFuseXfl);
        m_pFrameModel->setRowCount(pFuseSections->nSections());
        for(int row=0; row<pFuseSections->nSections(); row++)
        {
            std::vector<Vector3d> const &sec = pFuseSections->sectionAt(row);
            ind = m_pFrameModel->index(row, 0, QModelIndex());
            m_pFrameModel->setData(ind, sec.front().x * Units::mtoUnit());

            ind = m_pFrameModel->index(row, 1, QModelIndex());
            m_pFrameModel->setData(ind, m_pFuseXfl->xPanels(row));
        }
    }
}


void FuseXflDlg:: fillPointDataTable()
{
    if(!m_pFuseXfl) return;

    if(!m_pFuseXfl->isSectionType())
    {
        if(m_pFuseXfl->activeFrameIndex()<0 || m_pFuseXfl->activeFrameIndex()>=m_pFuseXfl->frameCount()) return;

        Frame const &pActiveFrame = m_pFuseXfl->activeFrame();

        m_pPointModel->setRowCount(m_pFuseXfl->sideLineCount());
        for(int row=0; row<m_pFuseXfl->sideLineCount(); row++)
        {
            QModelIndex ind;

            ind = m_pPointModel->index(row, 0, QModelIndex());
            m_pPointModel->setData(ind, pActiveFrame.ctrlPointAt(row).y * Units::mtoUnit());

            ind = m_pPointModel->index(row, 1, QModelIndex());
            m_pPointModel->setData(ind, pActiveFrame.ctrlPointAt(row).z * Units::mtoUnit());

            ind = m_pPointModel->index(row, 2, QModelIndex());
            m_pPointModel->setData(ind, m_pFuseXfl->m_hPanels.at(row));
        }
    }
    else
    {
        FuseSections const *pFuseSections = dynamic_cast<FuseSections const *>(m_pFuseXfl);
        if(pFuseSections->activeSectionIndex()<0 || pFuseSections->activeSectionIndex()>=pFuseSections->nSections()) return;

        std::vector<Vector3d> const &section = pFuseSections->activeSection();

        m_pPointModel->setRowCount(pFuseSections->pointCount());
        for(uint row=0; row<section.size(); row++)
        {
            QModelIndex ind;

            ind = m_pPointModel->index(row, 0, QModelIndex());
            m_pPointModel->setData(ind, section.at(row).y * Units::mtoUnit());

            ind = m_pPointModel->index(row, 1, QModelIndex());
            m_pPointModel->setData(ind, section.at(row).z * Units::mtoUnit());

/*pFuseXfl            ind = m_pPointModel->index(row, 2, QModelIndex());
            m_pPointModel->setData(ind, pFuseSections->m_hPanels.at(row));*/
        }
    }
}


void FuseXflDlg::onFrameClickedIn2dView()
{
    if(!m_pFuseXfl->isSectionType())
        setFrame(m_pFuseXfl->activeFrameIndex());
    else
    {
        FuseSections const *pFuseSections = dynamic_cast<FuseSections const *>(m_pFuseXfl);
        setFrame(pFuseSections->activeSectionIndex());
    }
    fillPointDataTable();
    m_pFrameView->update();
}


void FuseXflDlg::readFrameSectionData(int sel)
{
    if(sel<0 || sel>=m_pFrameModel->rowCount()) return;

    FuseSections *pFuseSections = nullptr;
    if(m_pFuseXfl->isSectionType()) pFuseSections = dynamic_cast<FuseSections *>(m_pFuseXfl);

    bool bOK=false;

    QStandardItem *pItem = m_pFrameModel->item(sel,0);
    if(!pItem) return;

    QString strong = pItem->text();
    strong.replace(" ","");
    double x = strong.toDouble(&bOK);
    if(bOK)
    {
        if(!m_pFuseXfl->isSectionType())
        {
            m_pFuseXfl->frame(sel).setuPosition(m_pFuseXfl->nurbs().uAxis(), x / Units::mtoUnit());
            for(int ic=0; ic<m_pFuseXfl->frame(sel).nCtrlPoints(); ic++)
            {
                m_pFuseXfl->frame(sel).ctrlPoint(ic).x  = x / Units::mtoUnit();
            }
        }
        else
            if(pFuseSections) pFuseSections->setSectionXPosition(sel, x / Units::mtoUnit());
    }

    pItem = m_pFrameModel->item(sel,1);
    if(pItem)
    {
        strong = pItem->text();
        strong.replace(" ","");
        int k = strong.toInt(&bOK);
        if(bOK) m_pFuseXfl->m_xPanels[sel] = k;
    }
}


void FuseXflDlg::onPointItemClicked(const QModelIndex &index)
{
    if(m_pFuseXfl->activeFrameIndex()<0) return;
    Frame const &pActiveFrame = m_pFuseXfl->activeFrame();

    if(!index.isValid()) return;

    pActiveFrame.setSelected(index.row());
    pActiveFrame.setHighlighted(index.row());
    updateView();

    if(index.column() == m_pPointModel->actionColumn())
    {
        QRect itemrect = m_pcptPointTable->visualRect(index);
        QPoint menupos = m_pcptPointTable->mapToGlobal(itemrect.topLeft());
        QMenu *pRowMenu = new QMenu(tr("Section"),this);

        QAction *pInsertBefore = new QAction(tr("Insert before"), this);
        connect(pInsertBefore, SIGNAL(triggered(bool)), this, SLOT(onInsertPointBefore()));
        pRowMenu->addAction(pInsertBefore);

        QAction *pInsertAfter = new QAction(tr("Insert after"), this);
        connect(pInsertAfter, SIGNAL(triggered(bool)), this, SLOT(onInsertPointAfter()));
        pRowMenu->addAction(pInsertAfter);

        QAction *pDeleteRow = new QAction(tr("Delete"), this);
        connect(pDeleteRow, SIGNAL(triggered(bool)), this, SLOT(onRemoveSelectedPoint()));
        pRowMenu->addAction(pDeleteRow);

        pRowMenu->exec(menupos);
    }
}


void FuseXflDlg::onControlPoints()
{
//    m_pFuseXfl->m_bInterpolatedNURBS = m_prbInterpolationPts->isChecked();
    updateFuseXfl();
    m_pglFuseView->resetFuse();
    updateView();
    m_bChanged = true;
}



/** a frame has been clicked in the frame view */
void FuseXflDlg::onSelectFrame(int iFrame)
{
    if(!m_pFuseXfl->isSectionType())
    {
        m_pFuseXfl->setActiveFrameIndex(iFrame);
        if(iFrame>=0 && iFrame<m_pFuseXfl->frameCount())
        {
            m_pcptFrameTable->selectRow(m_pFuseXfl->activeFrameIndex());
            fillPointDataTable();
        }
    }
    else
    {
        FuseSections const *pFuseSections = dynamic_cast<FuseSections const *>(m_pFuseXfl);
        pFuseSections->setActiveSectionIndex(iFrame);
        if(iFrame>=0 && iFrame<pFuseSections->nSections())
        {
            m_pcptFrameTable->selectRow(m_pFuseXfl->activeFrameIndex());
            fillPointDataTable();
        }
    }

    m_pglFuseView->resetFrameHighlight();
    updateView();
}


void FuseXflDlg::onSelectFrame(QModelIndex const &index)
{
    int newframerow = index.row();
    if(!m_pFuseXfl->isSectionType())
        m_pFuseXfl->setActiveFrameIndex(newframerow);
    else
    {
        FuseSections const *pFuseSections = dynamic_cast<FuseSections const *>(m_pFuseXfl);
        pFuseSections->setActiveSectionIndex(newframerow);
    }
    fillPointDataTable();

    m_pglFuseView->resetFrameHighlight();
    updateView();
}


void FuseXflDlg::setFrame(int iFrame)
{
    if(!m_pFuseXfl) return;
    if(!m_pFuseXfl->isSectionType())
    {
        m_pFuseXfl->setActiveFrameIndex(iFrame);

        m_pcptFrameTable->selectRow(m_pFuseXfl->activeFrameIndex());
    }
    else
    {
        FuseSections const *pFuseSections = dynamic_cast<FuseSections const *>(m_pFuseXfl);
        pFuseSections->setActiveSectionIndex(iFrame);
    }

    fillPointDataTable();
    m_pglFuseView->resetFrameHighlight();
    updateView();
}


/** slot isn't called if an editor is opened?
* so backup with onSelectFrame slot
*/
void FuseXflDlg::onFrameItemClicked(const QModelIndex &index)
{
    onSelectFrame(index);

    if(index.column() == m_pFrameModel->actionColumn())
    {
        QRect itemrect = m_pcptFrameTable->visualRect(index);
        QPoint menupos = m_pcptFrameTable->mapToGlobal(itemrect.topLeft());
        QMenu *pRowMenu = new QMenu(tr("Section"),this);

        QAction *pInsertBefore = new QAction(tr("Insert before"), this);
        connect(pInsertBefore, SIGNAL(triggered(bool)), this, SLOT(onInsertFrameBefore()));
        pRowMenu->addAction(pInsertBefore);

        QAction *pInsertAfter = new QAction(tr("Insert after"), this);
        connect(pInsertAfter, SIGNAL(triggered(bool)), this, SLOT(onInsertFrameAfter()));
        pRowMenu->addAction(pInsertAfter);

        QAction *pDeleteRow = new QAction(tr("Delete"), this);
        connect(pDeleteRow, SIGNAL(triggered(bool)), this, SLOT(onRemoveSelectedFrame()));
        pRowMenu->addAction(pDeleteRow);

        pRowMenu->exec(menupos);
    }
}


void FuseXflDlg::onInsertFrameBefore()
{
    int iSel = m_pSelectionModelFrame->currentIndex().row();
    m_pFuseXfl->insertFrameBefore(iSel);
    fillFrameDataTable();

    updateFuseXfl();
    m_pglFuseView->resetFuse();
    updateView();
    takePicture();
    m_bChanged = true;
}


void FuseXflDlg::onRemoveSelectedFrame()
{
    int iSel = m_pSelectionModelFrame->currentIndex().row();
    m_pFuseXfl->removeFrame(iSel);
    fillFrameDataTable();

    updateFuseXfl();
    takePicture();
    m_pglFuseView->resetFuse();
    updateView();
    m_bChanged = true;
}


void FuseXflDlg::onPointClickedIn2dView()
{
    if(!m_pFuseXfl->isSectionType())
    {
        if(m_pFuseXfl->activeFrameIndex()<0) return;
        Frame const &pActiveFrame = m_pFuseXfl->activeFrame();

        m_pcptPointTable->selectRow(pActiveFrame.selectedIndex());
    }
    else
    {
        FuseSections *pFuseSections = dynamic_cast<FuseSections*>(m_pFuseXfl);
        if(pFuseSections->activeSectionIndex()<0) return;
        if(pFuseSections->activePointIndex()<0) return;

        m_pcptPointTable->selectRow(pFuseSections->activePointIndex());
    }
    m_pglFuseView->update();
}


void FuseXflDlg::onFrameCellChanged()
{
    for(int ip=0; ip<m_pFrameModel->rowCount(); ip++)
        readFrameSectionData(ip);
    updateFuseXfl();
    takePicture();
    m_pglFuseView->resetFuse();
    updateView();

    m_bChanged = true;
}


void FuseXflDlg::onPointCellChanged()
{
    for(int ip=0; ip<m_pPointModel->rowCount(); ip++)
        readPointSectionData(ip);

    updateFuseXfl();
    takePicture();
    m_pglFuseView->resetFuse();
    updateView();

    m_bChanged = true;
}


void FuseXflDlg::onInsertPointBefore()
{
//    if(m_pFuseXfl->activeFrameIndex()<0) return;
    int iSel = m_pSelectionModelPoint->currentIndex().row();
    m_pFuseXfl->insertPoint(iSel);
    fillPointDataTable();

    updateFuseXfl();
    takePicture();
    m_pglFuseView->resetFuse();
    updateView();

    m_bChanged = true;
}


void FuseXflDlg::onInsertPointAfter()
{
//    if(m_pFuseXfl->activeFrameIndex()<0) return;
    int iSel = m_pSelectionModelPoint->currentIndex().row();
    m_pFuseXfl->insertPoint(iSel+1);
    fillPointDataTable();

    updateFuseXfl();
    takePicture();
    m_pglFuseView->resetFuse();
    updateView();

    m_bChanged = true;
}


void FuseXflDlg::onInsertFrameAfter()
{
    int iSel = m_pSelectionModelFrame->currentIndex().row();
    m_pFuseXfl->insertFrameAfter(iSel);

    fillFrameDataTable();

    updateFuseXfl();
    takePicture();
    m_pglFuseView->resetFuse();
    updateView();

    m_bChanged = true;
}


void FuseXflDlg::onRemoveSelectedPoint()
{
    int iSel = m_pSelectionModelPoint->currentIndex().row();

    if (iSel>=0)  m_pFuseXfl->removeSideLine(iSel);
    fillPointDataTable();

    updateFuseXfl();
    takePicture();
    m_pglFuseView->resetFuse();
    updateView();
    m_bChanged = true;
}


void FuseXflDlg::onNURBSPanels()
{
    if(!m_pFuseXfl) return;

    int val0 = m_pslBunchAmp->value();
    double amp = double(val0)/100.0; // k=0.0 --> uniform weight, k=1-->full varying weights;

    m_pFuseXfl->m_nurbs.setBunchParameters(amp, 0.0);

    m_pFuseXfl->m_nhNurbsPanels = m_pieNHoopPanels->value();
    m_pFuseXfl->m_nxNurbsPanels = m_pieNXPanels->value();
    m_pFuseXfl->setPanelPos();
    m_pFuseXfl->makeQuadMesh(0, Vector3d());
    std::string strange;
    m_pFuseXfl->makeDefaultTriMesh(strange, "");
    m_pglFuseView->resetFuse();

    takePicture();
    updateView();

    m_bChanged = true;
}


void FuseXflDlg::onSelChangeXDegree(int sel)
{
    if(!m_pFuseXfl) return;
    if (sel <0) return;

    int deg = sel+1;
    if(deg>=m_pFuseXfl->nurbs().frameCount())
    {
        QString strange = tr("<p>The degree must be less than the number of frames</p>");
        QMessageBox::warning(this, tr("Warning"), strange);
        deg=m_pFuseXfl->nurbs().frameCount();
        m_pcbXDegree->setCurrentIndex(m_pFuseXfl->nurbs().frameCount()-2);
    }

    m_pFuseXfl->m_nurbs.setuDegree(deg);
    m_pFuseXfl->setNURBSKnots();

    updateFuseXfl();
    takePicture();
    m_pglFuseView->resetFuse();
    updateView();

    m_bChanged = true;
}


void FuseXflDlg::onSelChangeHoopDegree(int sel)
{
    if(!m_pFuseXfl) return;
    if (sel<0) return;

    int deg = sel+1;
    if(deg>=m_pFuseXfl->nurbs().framePointCount())
    {
        QString strange(tr("<p>The degree must be less than the number of side lines</p>"));
        QMessageBox::warning(this, tr("Warning"), strange);
        deg=m_pFuseXfl->nurbs().framePointCount();
        m_pcbHoopDegree->setCurrentIndex(m_pFuseXfl->nurbs().framePointCount()-2);
    }

    m_pFuseXfl->m_nurbs.setvDegree(deg);
    m_pFuseXfl->setNURBSKnots();

    updateFuseXfl();
    takePicture();
    m_pglFuseView->resetFuse();
    updateView();
    m_bChanged = true;
}


void FuseXflDlg::onEdgeWeight()
{
    /*    if(!m_pBody) return;

    m_bChanged = true;

    double w= (double)m_pdeEdgeWeight->value()/100.0 + 1.0;
    m_pBody->setEdgeWeight(w, w);

    updateFuseXfl();
    onTakePicture();
    m_bResetglBody   = true;
    updateView();*/
}


void FuseXflDlg::readPointSectionData(int sel)
{
    if(!m_pFuseXfl->isSectionType())
    {
        if(m_pFuseXfl->activeFrameIndex()<0) return;
        Frame &pActiveFrame = m_pFuseXfl->activeFrame();


        if(sel>=m_pPointModel->rowCount()) return;
        if(sel<0 || sel>=pActiveFrame.nCtrlPoints()) return;

        double d=0;
        int k=0;

        bool bOK=false;
        QString strong;
        QStandardItem *pItem=nullptr;

        pItem = m_pPointModel->item(sel,0);
        if(pItem)
        {
            strong = pItem->text();
            strong.replace(" ","");
            d =strong.toDouble(&bOK);
            if(bOK) pActiveFrame.ctrlPoint(sel).y = d / Units::mtoUnit();
        }
        pItem = m_pPointModel->item(sel,1);
        if(pItem)
        {
            strong = pItem->text();
            strong.replace(" ","");
            d =strong.toDouble(&bOK);
            if(bOK) pActiveFrame.ctrlPoint(sel).z = d / Units::mtoUnit();
        }

        pItem = m_pPointModel->item(sel,2);
        if(pItem)
        {
            strong = pItem->text();
            strong.replace(" ","");
            k =strong.toInt(&bOK);
            if(bOK) m_pFuseXfl->m_hPanels[sel] = k;
        }
    }
    else
    {
        FuseSections *pFuseSections = dynamic_cast<FuseSections*>(m_pFuseXfl);
        if(pFuseSections->activeSectionIndex()<0) return;
        std::vector<Vector3d> &section = pFuseSections->activeSection();

        if(sel>=m_pPointModel->rowCount()) return;
        if(sel<0 || sel>=int(section.size())) return;

        double d=0;

        bool bOK=false;
        QString strong;
        QStandardItem *pItem=nullptr;

        pItem = m_pPointModel->item(sel,0);
        if(pItem)
        {
            strong = pItem->text();
            strong.replace(" ","");
            d =strong.toDouble(&bOK);
            if(bOK) section[sel].y = d / Units::mtoUnit();
        }
        pItem = m_pPointModel->item(sel,1);
        if(pItem)
        {
            strong = pItem->text();
            strong.replace(" ","");
            d =strong.toDouble(&bOK);
            if(bOK) section[sel].z = d / Units::mtoUnit();
        }
    }
}


void FuseXflDlg::enableStackBtns()
{
    m_ppbUndo->setEnabled(m_StackPos>0);
    m_ppbRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);
}


void FuseXflDlg::setControls()
{
    enableStackBtns();

    if(m_pFuseXfl->isFlatFaceType())
    {
        m_pNURBSParams->hide();
        m_pcptFrameTable->showColumn(1);
        m_pcptPointTable->showColumn(2);
    }
    else if(m_pFuseXfl->isSplineType() || m_pFuseXfl->isSectionType())
    {
        m_pcptFrameTable->hideColumn(1);
        m_pcptPointTable->hideColumn(2);
    }

    if(m_pFuseXfl)
    {
        int vamp = int(m_pFuseXfl->nurbs().bunchAmplitude()*100.0);
        m_pslBunchAmp->setValue(vamp);

        m_pieNXPanels->setValue(m_pFuseXfl->m_nxNurbsPanels);
        m_pieNHoopPanels->setValue(m_pFuseXfl->m_nhNurbsPanels);

        m_pcbXDegree->setCurrentIndex(m_pFuseXfl->m_nurbs.uDegree()-1);
        m_pcbHoopDegree->setCurrentIndex(m_pFuseXfl->m_nurbs.vDegree()-1);
    }
}


void FuseXflDlg::initDialog(Fuse*pFuse)
{
    FuseDlg::initDialog(pFuse);

    FuseXfl *pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);
    if(!pFuseXfl) return;

    m_pFuseLineView->setUnitFactor(Units::mtoUnit());
    m_pFrameView->setUnitFactor(Units::mtoUnit());

    //    m_pglControls->showCtrlPointsCtrl(true);

    m_pglFuseView->showOutline(s_bOutline);
    m_pglFuseView->showSurfaces(s_bSurfaces);
    m_pglFuseView->showPanels(s_bVLMPanels);
    m_pglFuseView->showMasses(s_bShowMasses);
    m_pglFuseView->showControlPoints(s_bShowCtrlPoints);

    m_ptwDefinition->setCurrentIndex(s_PageIndex);

    if(!pFuseXfl) return;

    if(pFuseXfl->isFlatFaceType()) m_ptwDefinition->removeTab(1);
    if(!pFuseXfl->isSplineType()) m_pgbUVParams->hide();
    if(!pFuseXfl->isSectionType()) m_pgbFit->hide();
    else
    {
        FuseSections const *pFuseSections = dynamic_cast<FuseSections const *>(pFuseXfl);
        if(pFuseSections)
            m_pfeFitPrecision->setValue(pFuseSections->fitPrecision()*Units::mtoUnit());
    }

    setBody(pFuseXfl);
    setFrame(0);
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

    setControls();
    blockSignalling(true);
    fillFrameDataTable();
    fillPointDataTable();
    blockSignalling(false);
}


void FuseXflDlg::setupLayout()
{
    QString str;

    QWidget *pLeftSideWidget = new QWidget;
    {
        QVBoxLayout *pLeftSideLayout = new QVBoxLayout;
        {
            m_ptwDefinition = new QTabWidget;
            {
                m_pNURBSParams = new QFrame;
                {
                    QVBoxLayout *pParamsLayout = new QVBoxLayout;
                    {
                        m_pgbUVParams = new QGroupBox(tr("UV parameters"));
                        {
                            QGridLayout *pSplineParamsLayout = new QGridLayout;
                            {
                                QLabel *plab1 = new QLabel("x");
                                QLabel *plab2 = new QLabel(tr("Hoop"));
                                QLabel *plab3 = new QLabel(tr("Degree"));
                                m_pcbXDegree = new QComboBox;
                                m_pcbHoopDegree = new QComboBox;

                                pSplineParamsLayout->addWidget(plab1,1,2, Qt::AlignCenter);
                                pSplineParamsLayout->addWidget(plab2,1,3, Qt::AlignCenter);
                                pSplineParamsLayout->addWidget(plab3,2,1, Qt::AlignRight);
                                pSplineParamsLayout->addWidget(m_pcbXDegree,2,2);
                                pSplineParamsLayout->addWidget(m_pcbHoopDegree,2,3);
                            }
                            m_pgbUVParams->setLayout(pSplineParamsLayout);
                        }

                        m_pgbFit = new QGroupBox(tr("NURBS fit"));
                        {
                            QHBoxLayout *pFitLayout = new QHBoxLayout;
                            {
                                QLabel *plabFit = new QLabel(tr("Fit tolerance"));
                                QLabel *plabLengthUnit = new QLabel(Units::lengthUnitQLabel());
                                m_pfeFitPrecision = new FloatEdit;
                                QString tip(tr("<p>Defines the precision with which the spline will be fit to the control points.<br>"
                                            "Reduce this precision for a better fit with the risk of potential NURBS oscillations.<br>"
                                            "Increase this precision to get a smoother NURBS with a less precise fit.</p>"));
                                m_pfeFitPrecision->setToolTip(tip);

                                pFitLayout->addWidget(plabFit);
                                pFitLayout->addWidget(m_pfeFitPrecision);
                                pFitLayout->addWidget(plabLengthUnit);
                            }
                            m_pgbFit->setLayout(pFitLayout);
                        }

                        QGroupBox *pBunchParams = new QGroupBox(tr("Panels"));
                        {
                            QVBoxLayout *pMeshLayout = new QVBoxLayout;
                            {
                                QGridLayout *pSplineParamsLayout = new QGridLayout;
                                {
                                    QLabel *plab1 = new QLabel("x");
                                    QLabel *plab2 = new QLabel(tr("Hoop"));
                                    QLabel *plab4 = new QLabel(tr("Number of panels"));
                                    m_pieNXPanels = new IntEdit;
                                    m_pieNHoopPanels = new IntEdit;

                                    pSplineParamsLayout->addWidget(plab1,            1, 2, Qt::AlignCenter);
                                    pSplineParamsLayout->addWidget(plab2,            1, 3, Qt::AlignCenter);
                                    pSplineParamsLayout->addWidget(plab4,            2, 1, Qt::AlignRight);
                                    pSplineParamsLayout->addWidget(m_pieNXPanels,    2, 2);
                                    pSplineParamsLayout->addWidget(m_pieNHoopPanels, 2, 3);
                                }

                                QGridLayout *pBunchParamsLayout = new QGridLayout;
                                {
                                    QLabel *plabCenter = new QLabel(tr("Uniform"));
                                    plabCenter->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
                                    QLabel *plabEndPoints = new QLabel(tr("End points"));
                                    plabEndPoints->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);

                                    m_pslBunchAmp = new QSlider(Qt::Horizontal);
                                    m_pslBunchAmp->setRange(0, 100);
                                    m_pslBunchAmp->setTickInterval(5);
                                    m_pslBunchAmp->setTickPosition(QSlider::TicksBelow);

                                    pBunchParamsLayout->addWidget(plabCenter,      2,1);
                                    pBunchParamsLayout->addWidget(m_pslBunchAmp,   2,2);
                                    pBunchParamsLayout->addWidget(plabEndPoints,   2,3);
                                }

                                pMeshLayout->addLayout(pSplineParamsLayout);
                                pMeshLayout->addSpacing(25);
                                pMeshLayout->addLayout(pBunchParamsLayout);
                            }
                            pBunchParams->setLayout(pMeshLayout);
                        }

                        pParamsLayout->addWidget(m_pgbUVParams);
                        pParamsLayout->addWidget(m_pgbFit);
                        pParamsLayout->addWidget(pBunchParams);
                        pParamsLayout->addStretch();
                    }

                    m_pNURBSParams->setLayout(pParamsLayout);
                }

                m_pTableSplitter = new QSplitter(Qt::Vertical);
                {
                    QFrame *pFramePosFrame = new QFrame;
                    {
                        QVBoxLayout * pFramePosLayout = new QVBoxLayout;
                        {
                            QLabel *plabFrame = new QLabel(tr("Frame positions"));
                            plabFrame->setStyleSheet("font-weight: bold");
                            plabFrame->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
                            pFramePosLayout->addWidget(plabFrame);
                            pFramePosLayout->addWidget(m_pcptFrameTable);
                        }
                        pFramePosFrame->setLayout(pFramePosLayout);
                    }
                    QFrame *pFramePointFrame = new QFrame;
                    {
                        QVBoxLayout * pFramePointLayout = new QVBoxLayout;
                        {
                            QLabel *plabPoints = new QLabel(tr("Active frame points"));
                            plabPoints->setStyleSheet("font-weight: bold");
                            plabPoints->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
                            pFramePointLayout->addWidget(plabPoints);
                            pFramePointLayout->addWidget(m_pcptPointTable);
                        }
                        pFramePointFrame->setLayout(pFramePointLayout);
                    }

                    m_pTableSplitter->addWidget(pFramePosFrame);
                    m_pTableSplitter->addWidget(pFramePointFrame);
                }

                m_ptwDefinition->addTab(m_pMetaFrame,     tr("Meta"));
                m_ptwDefinition->addTab(m_pNURBSParams,   tr("NURBS parameters"));
                m_ptwDefinition->addTab(m_pTableSplitter, tr("Tables"));
            }

            QHBoxLayout *pUndoRedoLayout = new QHBoxLayout;
            {
                m_ppbUndo = new QPushButton(QIcon(":/icons/OnUndo.png"), tr("Undo"));
                m_ppbRedo = new QPushButton(QIcon(":/icons/OnRedo.png"), tr("Redo"));
                pUndoRedoLayout->addWidget(m_ppbUndo);
                pUndoRedoLayout->addWidget(m_ppbRedo);
            }

            pLeftSideLayout->addWidget(m_ptwDefinition);
            pLeftSideLayout->addLayout(pUndoRedoLayout);
            pLeftSideLayout->addWidget(m_pButtonBox);
            pLeftSideLayout->setStretchFactor(m_pMetaFrame,1);
            pLeftSideLayout->setStretchFactor(m_ptwDefinition,20);
            pLeftSideLayout->setStretchFactor(m_pButtonBox,1);
        }
        pLeftSideWidget->setLayout(pLeftSideLayout);
    }

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        m_pMainHSplitter = new QSplitter(Qt::Horizontal);
        {
            m_pMainHSplitter->setChildrenCollapsible(false);
            m_pMainHSplitter->addWidget(pLeftSideWidget);
            m_pMainHSplitter->addWidget(m_pViewHSplitter);
            m_pMainHSplitter->setStretchFactor(0,1);
            m_pMainHSplitter->setStretchFactor(1,5);
        }
        pMainLayout->addWidget(m_pMainHSplitter);
    }
    setLayout(pMainLayout);

    for (int i=1; i<6; i++)
    {
        str = QString("%1").arg(i);
        m_pcbXDegree->addItem(str);
        m_pcbHoopDegree->addItem(str);
    }

    m_ppblRedraw->hide();
}


void FuseXflDlg::makeTables()
{
    m_pcptFrameTable = new CPTableView();
    m_pcptFrameTable->setEditable(true);
    m_pcptFrameTable->horizontalHeader()->setStretchLastSection(true);
    m_pcptFrameTable->setEditTriggers(QAbstractItemView::EditKeyPressed |
                                      QAbstractItemView::AnyKeyPressed  |
                                      QAbstractItemView::DoubleClicked  |
                                      QAbstractItemView::SelectedClicked);
    m_pFrameModel = new ActionItemModel(this);
    m_pFrameModel->setRowCount(10);//temporary
    m_pFrameModel->setColumnCount(3);
    m_pFrameModel->setActionColumn(2);
    m_pcptFrameTable->setModel(m_pFrameModel);
    m_pSelectionModelFrame = new QItemSelectionModel(m_pFrameModel);
    m_pcptFrameTable->setSelectionModel(m_pSelectionModelFrame);
    m_pFrameDelegate = new XflDelegate(this);
    m_pFrameDelegate->setActionColumn(2);
    m_pcptFrameTable->setItemDelegate(m_pFrameDelegate);
    m_pFrameDelegate->setDigits({5,0,0});
    m_pFrameDelegate->setItemTypes({XflDelegate::DOUBLE, XflDelegate::INTEGER, XflDelegate::ACTION});

    m_pcptPointTable = new CPTableView(this);
    m_pcptPointTable->setEditable(true);
    m_pcptPointTable->setEditTriggers(QAbstractItemView::EditKeyPressed |
                                       QAbstractItemView::AnyKeyPressed  |
                                       QAbstractItemView::DoubleClicked  |
                                       QAbstractItemView::SelectedClicked);

    m_pcptPointTable->horizontalHeader()->setStretchLastSection(true);
    m_pPointModel = new ActionItemModel(this);
    m_pPointModel->setRowCount(10);//temporary
    m_pPointModel->setColumnCount(4);
    m_pPointModel->setActionColumn(3);
    m_pcptPointTable->setModel(m_pPointModel);
    m_pSelectionModelPoint = new QItemSelectionModel(m_pPointModel);
    m_pcptPointTable->setSelectionModel(m_pSelectionModelPoint);
    m_pPointDelegate = new XflDelegate(this);
    m_pPointDelegate->setActionColumn(3);
    m_pcptPointTable->setItemDelegate(m_pPointDelegate);
    m_pPointDelegate->setDigits({5,5,0,0});
    m_pPointDelegate->setItemTypes({XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::INTEGER, XflDelegate::ACTION});
}


void FuseXflDlg::resizeEvent(QResizeEvent *pEvent)
{
    FuseDlg::resizeEvent(pEvent);

    onResizeTables();
    pEvent->accept();
}


void FuseXflDlg::showEvent(QShowEvent *pEvent)
{
    FuseDlg::showEvent(pEvent);
    if(s_MainSplitterSizes.length()>0)  m_pMainHSplitter->restoreState(s_MainSplitterSizes);
    if(s_TableSplitterSizes.length()>0)  m_pTableSplitter->restoreState(s_TableSplitterSizes);
    m_ptwDefinition->setCurrentIndex(s_PageIndex);

    m_pFuseLineView->setGrid(s_BodyLineGrid);
    m_pFrameView->setGrid(s_FrameGrid);

    m_pFuseLineView->resetDefaultScale();
    m_pFrameView->resetDefaultScale();
    m_pFuseLineView->setAutoUnits();
    m_pFrameView->setAutoUnits();

    setTableUnits();

    onResizeTables();

    updateView();
}


/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void FuseXflDlg::hideEvent(QHideEvent *pEvent)
{
    FuseDlg::hideEvent(pEvent);

    s_MainSplitterSizes = m_pMainHSplitter->saveState();
    s_TableSplitterSizes = m_pTableSplitter->saveState();

    s_PageIndex = m_ptwDefinition->currentIndex();

    s_BodyLineGrid.duplicate(m_pFuseLineView->grid());
    s_FrameGrid.duplicate(m_pFrameView->grid());

    pEvent->accept();
}


void FuseXflDlg::onResizeTables()
{
     if(m_pFuseXfl->isFlatFaceType())
    {
        double w = double(m_pcptFrameTable->width()) / 100.0;
        m_pcptFrameTable->setColumnWidth(0,int(w*37.0));
        m_pcptFrameTable->setColumnWidth(1,int(w*21.0));

        w = double(m_pcptPointTable->width()) / 100.0;
        int ColumnWidth = int(w*19);
        m_pcptPointTable->setColumnWidth(0,ColumnWidth);
        m_pcptPointTable->setColumnWidth(1,ColumnWidth);
        m_pcptPointTable->setColumnWidth(2,ColumnWidth);
    }
    else if(m_pFuseXfl->isSplineType() || m_pFuseXfl->isSectionType())
    {
        double w = double(m_pcptFrameTable->width()) / 100.0;
        m_pcptFrameTable->setColumnWidth(0,int(w*47.0));

        w = double(m_pcptPointTable->width()) / 100.0;
        int ColumnWidth = int(w*25);
        m_pcptPointTable->setColumnWidth(0,ColumnWidth);
        m_pcptPointTable->setColumnWidth(1,ColumnWidth);
        m_pcptPointTable->setColumnWidth(2,ColumnWidth);
    }
}


void FuseXflDlg::onUpdateFuseDlg()
{
    takePicture();
    updateFuseDlg();
}


void FuseXflDlg::updateFuseDlg()
{
    m_bChanged = true;

    fillFrameDataTable();
    fillPointDataTable();

    updateFuseXfl();
    m_pglFuseView->resetFuse();
    updateView();
}


void FuseXflDlg::setPicture()
{
    int iActiveFrameIndex = m_pFuseXfl->activeFrameIndex();

    Fuse const *pTmpBodyXfl = m_UndoStack.at(m_StackPos);
    m_pFuseXfl->duplicate(*pTmpBodyXfl);
    fillFrameDataTable();
    fillPointDataTable();

    // because signals are async, the picture may have been taken with or without
    // the modified geometry, so rebuild it
    updateFuseXfl();

    m_pFuseXfl->setActiveFrameIndex(iActiveFrameIndex);

    m_pFuseXfl->setNURBSKnots();

    m_pglFuseView->resetFuse();
    m_pglFuseView->resetFrameHighlight();

    updateView();
}


void FuseXflDlg::blockSignalling(bool bBlock)
{
    blockSignals(bBlock);
    m_pPointDelegate->blockSignals(bBlock);
    m_pFrameDelegate->blockSignals(bBlock);
    m_pcptPointTable->blockSignals(bBlock);
    m_pcptFrameTable->blockSignals(bBlock);

    m_pSelectionModelPoint->blockSignals(bBlock);
    m_pSelectionModelFrame->blockSignals(bBlock);
}


bool FuseXflDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("XflFuseDefDlg");
    {
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();

        s_MainSplitterSizes = settings.value("MainSplitterSizes").toByteArray();
        s_TableSplitterSizes = settings.value("TableSplitterSizes").toByteArray();

        s_HViewSplitterSizes = settings.value("HSplitterSizes").toByteArray();
        s_VViewSplitterSizes = settings.value("VSplitterSizes").toByteArray();
        settings.beginGroup("BodyLine");
        {
            s_BodyLineGrid.loadSettings(settings);
            settings.endGroup();
        }
        settings.beginGroup("BodyFrame");
        {
            s_FrameGrid.loadSettings(settings);
            settings.endGroup();
        }

    }
    settings.endGroup();
    return true;
}


bool FuseXflDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("XflFuseDefDlg");
    {
        settings.setValue("WindowGeom", s_Geometry);

        settings.setValue("MainSplitterSizes", s_MainSplitterSizes);
        settings.setValue("TableSplitterSizes", s_TableSplitterSizes);
        settings.setValue("HSplitterSizes", s_HViewSplitterSizes);
        settings.setValue("VSplitterSizes", s_VViewSplitterSizes);
        settings.beginGroup("BodyLine");
        {
            s_BodyLineGrid.saveSettings(settings);
            settings.endGroup();
        }
        settings.beginGroup("BodyFrame");
        {
            s_FrameGrid.saveSettings(settings);
            settings.endGroup();
        }
    }
    settings.endGroup();
    return true;
}


void FuseXflDlg::onFitPrecision()
{
    if(!m_pFuseXfl->isSectionType()) return;
    FuseSections *pFuseSections = dynamic_cast<FuseSections *>(m_pFuseXfl);
    pFuseSections->setFitPrecision(m_pfeFitPrecision->value()/Units::mtoUnit());
    updateFuseXfl();
    m_pglFuseView->resetFuse();
    updateView();

    m_bChanged = true;
}


void FuseXflDlg::onRemoveFrame(int iFrame)
{
    if(m_pFuseXfl->isSplineType() && (m_pFuseXfl->frameCount()<=m_pFuseXfl->nurbs().uDegree()+1))
    {
        QString strange(tr("<p>Cannot remove: the number of frames must be at least equal to the x degree + 1"));
        QMessageBox::warning(this, tr("Warning"), strange);
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
    pFuseXfl->duplicate(*m_pFuseXfl);
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
        m_ppblRedraw = new QPushButton(tr("Regenerate\t(F4)"));
        m_ppbMenuButton = new QPushButton(tr("Actions"));

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
    FileName = QFileDialog::getSaveFileName(window(), tr("Export fuselage definition to xml file"),
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


void FuseXflDlg::onResetFuse()
{
    std::string strange;

    m_bChanged = true;

    //    m_pFuseXfl->extractShellsFromShapes();

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
    m_pFuseXfl->makeShell(strong);
}


