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
#include <QColorDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QHeaderView>



#include <api/frame.h>
#include <api/fusesections.h>
#include <api/fusexfl.h>
#include <api/quad3d.h>
#include <api/units.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fuseframewt.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fuselinewt.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fusexfldefdlg.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dfuseview.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/customwts/xfldelegate.h>


QByteArray FuseXflDefDlg::s_MainSplitterSizes;
QByteArray FuseXflDefDlg::s_TableSplitterSizes;
int FuseXflDefDlg::s_PageIndex = 0;

Grid FuseXflDefDlg::s_BodyLineGrid;
Grid FuseXflDefDlg::s_FrameGrid;


FuseXflDefDlg::FuseXflDefDlg(QWidget *pParent) : FuseXflDlg(pParent)
{
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


FuseXflDefDlg::~FuseXflDefDlg()
{
}


void FuseXflDefDlg::connectSignals()
{
    connectFuseXflSignals();

    connect(m_ppbUndo, SIGNAL(clicked()), SLOT(onUndo()));
    connect(m_ppbRedo, SIGNAL(clicked()), SLOT(onRedo()));

    connect(m_pslBunchAmp,        SIGNAL(sliderReleased()),  SLOT(onNURBSPanels()));

    connect(m_pieNHoopPanels,     SIGNAL(intChanged(int)), SLOT(onNURBSPanels()));
    connect(m_pieNXPanels,        SIGNAL(intChanged(int)), SLOT(onNURBSPanels()));
    connect(m_pcbXDegree,         SIGNAL(activated(int)),    SLOT(onSelChangeXDegree(int)));
    connect(m_pcbHoopDegree,      SIGNAL(activated(int)),    SLOT(onSelChangeHoopDegree(int)));

    // table signals
    connect(m_pcptFrameTable, SIGNAL(clicked(QModelIndex)),   SLOT(onFrameItemClicked(QModelIndex)));
    connect(m_pcptFrameTable, SIGNAL(dataPasted()),           SLOT(onFrameCellChanged()));
    connect(m_pFrameDelegate,  SIGNAL(closeEditor(QWidget*)), SLOT(onFrameCellChanged()));
    connect(m_pcptFrameTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onSelectFrame(QModelIndex)));

    connect(m_pcptPointTable, SIGNAL(clicked(QModelIndex)),   SLOT(onPointItemClicked(QModelIndex)));
    connect(m_pcptPointTable, SIGNAL(dataPasted()),           SLOT(onPointCellChanged()));
    connect(m_pPointDelegate,  SIGNAL(closeEditor(QWidget*)), SLOT(onPointCellChanged()));

    connect(m_pcptFrameTable, SIGNAL(tableResized()), SLOT(onResizeTables()));

    connect(m_pFuseLineView, SIGNAL(selectedChanged(int)),  SLOT(onFrameClickedIn2dView()));
    connect(m_pFrameView,    SIGNAL(selectedChanged(int)),  SLOT(onPointClickedIn2dView()));

    connect(m_pFuseLineView, SIGNAL(mouseDragReleased()),   SLOT(onUpdateFuseDlg()));
    connect(m_pFrameView,    SIGNAL(mouseDragReleased()),   SLOT(onUpdateFuseDlg()));

    connect(m_pFrameView,    SIGNAL(frameSelected(int)),    SLOT(onSelectFrame(int)));

    connect(m_pdeFitPrecision, SIGNAL(floatChanged(float)), SLOT(onFitPrecision()));
}


void FuseXflDefDlg::setTableUnits()
{
    QString length;
    length = Units::lengthUnitQLabel();

    m_pFrameModel->setHeaderData(0, Qt::Horizontal, "x ("+length+")");
    m_pFrameModel->setHeaderData(1, Qt::Horizontal, "NPanels");
    m_pFrameModel->setHeaderData(2, Qt::Horizontal, "Actions");

    m_pPointModel->setHeaderData(0, Qt::Horizontal, "y ("+length+")");
    m_pPointModel->setHeaderData(1, Qt::Horizontal, "z ("+length+")");
    m_pPointModel->setHeaderData(2, Qt::Horizontal, "NPanels");
    m_pPointModel->setHeaderData(3, Qt::Horizontal, "Actions");
}


void FuseXflDefDlg::fillFrameDataTable()
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


void FuseXflDefDlg:: fillPointDataTable()
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


void FuseXflDefDlg::onFrameClickedIn2dView()
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


void FuseXflDefDlg::readFrameSectionData(int sel)
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


void FuseXflDefDlg::onPointItemClicked(const QModelIndex &index)
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
        QMenu *pRowMenu = new QMenu("Section",this);

        QAction *pInsertBefore = new QAction("Insert before", this);
        connect(pInsertBefore, SIGNAL(triggered(bool)), this, SLOT(onInsertPointBefore()));
        pRowMenu->addAction(pInsertBefore);

        QAction *pInsertAfter = new QAction("Insert after", this);
        connect(pInsertAfter, SIGNAL(triggered(bool)), this, SLOT(onInsertPointAfter()));
        pRowMenu->addAction(pInsertAfter);

        QAction *pDeleteRow = new QAction("Delete", this);
        connect(pDeleteRow, SIGNAL(triggered(bool)), this, SLOT(onRemoveSelectedPoint()));
        pRowMenu->addAction(pDeleteRow);

        pRowMenu->exec(menupos);
    }
}


void FuseXflDefDlg::onControlPoints()
{
//    m_pFuseXfl->m_bInterpolatedNURBS = m_prbInterpolationPts->isChecked();
    updateFuseXfl();
    m_pglFuseView->resetFuse();
    updateView();
    m_bChanged = true;
}



/** a frame has been clicked in the frame view */
void FuseXflDefDlg::onSelectFrame(int iFrame)
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


void FuseXflDefDlg::onSelectFrame(QModelIndex const &index)
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


void FuseXflDefDlg::setFrame(int iFrame)
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
void FuseXflDefDlg::onFrameItemClicked(const QModelIndex &index)
{
    onSelectFrame(index);

    if(index.column() == m_pFrameModel->actionColumn())
    {
        QRect itemrect = m_pcptFrameTable->visualRect(index);
        QPoint menupos = m_pcptFrameTable->mapToGlobal(itemrect.topLeft());
        QMenu *pRowMenu = new QMenu("Section",this);

        QAction *pInsertBefore = new QAction("Insert before", this);
        connect(pInsertBefore, SIGNAL(triggered(bool)), this, SLOT(onInsertFrameBefore()));
        pRowMenu->addAction(pInsertBefore);

        QAction *pInsertAfter = new QAction("Insert after", this);
        connect(pInsertAfter, SIGNAL(triggered(bool)), this, SLOT(onInsertFrameAfter()));
        pRowMenu->addAction(pInsertAfter);

        QAction *pDeleteRow = new QAction("Delete", this);
        connect(pDeleteRow, SIGNAL(triggered(bool)), this, SLOT(onRemoveSelectedFrame()));
        pRowMenu->addAction(pDeleteRow);

        pRowMenu->exec(menupos);
    }
}


void FuseXflDefDlg::onInsertFrameBefore()
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


void FuseXflDefDlg::onRemoveSelectedFrame()
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


void FuseXflDefDlg::onPointClickedIn2dView()
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


void FuseXflDefDlg::onFrameCellChanged()
{
    for(int ip=0; ip<m_pFrameModel->rowCount(); ip++)
        readFrameSectionData(ip);
    updateFuseXfl();
    takePicture();
    m_pglFuseView->resetFuse();
    updateView();

    m_bChanged = true;
}


void FuseXflDefDlg::onPointCellChanged()
{
    for(int ip=0; ip<m_pPointModel->rowCount(); ip++)
        readPointSectionData(ip);

    updateFuseXfl();
    takePicture();
    m_pglFuseView->resetFuse();
    updateView();

    m_bChanged = true;
}


void FuseXflDefDlg::onInsertPointBefore()
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


void FuseXflDefDlg::onInsertPointAfter()
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


void FuseXflDefDlg::onInsertFrameAfter()
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


void FuseXflDefDlg::onRemoveSelectedPoint()
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


void FuseXflDefDlg::onNURBSPanels()
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


void FuseXflDefDlg::onSelChangeXDegree(int sel)
{
    if(!m_pFuseXfl) return;
    if (sel <0) return;


    int deg = sel+1;
    if(deg>=m_pFuseXfl->nurbs().frameCount())
    {
        QString strange = "The degree must be less than the number of frames";
        QMessageBox::warning(this, "Warning", strange);
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


void FuseXflDefDlg::onSelChangeHoopDegree(int sel)
{
    if(!m_pFuseXfl) return;
    if (sel<0) return;

    int deg = sel+1;
    if(deg>=m_pFuseXfl->nurbs().framePointCount())
    {
        QString strange("The degree must be less than the number of side lines");
        QMessageBox::warning(this, "Warning", strange);
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


void FuseXflDefDlg::onEdgeWeight()
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


void FuseXflDefDlg::readPointSectionData(int sel)
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


void FuseXflDefDlg::enableStackBtns()
{
    m_ppbUndo->setEnabled(m_StackPos>0);
    m_ppbRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);
}


void FuseXflDefDlg::setControls()
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
        m_pNURBSParams->show();
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


void FuseXflDefDlg::initDialog(Fuse*pFuse)
{
    FuseXflDlg::initDialog(pFuse);

    m_ptwDefinition->setCurrentIndex(s_PageIndex);

    FuseXfl*pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);

    if(!pFuseXfl) return;

    if(pFuseXfl->isFlatFaceType()) m_ptwDefinition->removeTab(1);
    if(!pFuseXfl->isSplineType()) m_pUVParamsBox->hide();
    if(!pFuseXfl->isSectionType()) m_pFitBox->hide();
    else
    {
        FuseSections const *pFuseSections = dynamic_cast<FuseSections const *>(pFuseXfl);
        if(pFuseSections)
            m_pdeFitPrecision->setValue(pFuseSections->fitPrecision()*Units::mtoUnit());
    }

    setBody(pFuseXfl);
    setFrame(0);
}


void FuseXflDefDlg::setBody(FuseXfl *pFuseXfl)
{
    FuseXflDlg::setBody(pFuseXfl);
    setControls();
    blockSignalling(true);
    fillFrameDataTable();
    fillPointDataTable();
    blockSignalling(false);
}


void FuseXflDefDlg::setupLayout()
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
//                    m_pNURBSParams->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
                    QVBoxLayout *pParamsLayout = new QVBoxLayout;
                    {
                        m_pUVParamsBox = new QGroupBox("UV parameters");
                        {
                            QGridLayout *pSplineParamsLayout = new QGridLayout;
                            {
                                QLabel *pLab1 = new QLabel("x");
                                QLabel *pLab2 = new QLabel("Hoop");
                                QLabel *pLab3 = new QLabel("Degree");
                                m_pcbXDegree = new QComboBox;
                                m_pcbHoopDegree = new QComboBox;

/*                                pLab1->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
                                pLab2->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
                                pLab3->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
                                m_pcbXDegree->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
                                m_pcbHoopDegree->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
*/
                                pSplineParamsLayout->addWidget(pLab1,1,2, Qt::AlignCenter);
                                pSplineParamsLayout->addWidget(pLab2,1,3, Qt::AlignCenter);
                                pSplineParamsLayout->addWidget(pLab3,2,1, Qt::AlignRight);
                                pSplineParamsLayout->addWidget(m_pcbXDegree,2,2);
                                pSplineParamsLayout->addWidget(m_pcbHoopDegree,2,3);
                            }
                            m_pUVParamsBox->setLayout(pSplineParamsLayout);
                        }

                        m_pFitBox = new QGroupBox("NURBS fit");
                        {
                            QHBoxLayout *pFitLayout = new QHBoxLayout;
                            {
                                QLabel *pLabFit = new QLabel("Fit tolerance");
                                QLabel *pLabLengthUnit = new QLabel(Units::lengthUnitQLabel());
                                m_pdeFitPrecision = new FloatEdit;
                                QString tip("<p>Defines the precision with which the spline will be fit to the control points.<br>"
                                            "Reduce this precision for a better fit with the risk of potential NURBS oscillations.<br>"
                                            "Increase this precision to get a smoother NURBS with a less precise fit.</p>");
                                m_pdeFitPrecision->setToolTip(tip);

                                pFitLayout->addWidget(pLabFit);
                                pFitLayout->addWidget(m_pdeFitPrecision);
                                pFitLayout->addWidget(pLabLengthUnit);
                            }
                            m_pFitBox->setLayout(pFitLayout);
                        }

                        QGroupBox *pBunchParams = new QGroupBox("Quad panels");
                        {
                            QVBoxLayout *pMeshLayout = new QVBoxLayout;
                            {
                                QGridLayout *pSplineParamsLayout = new QGridLayout;
                                {
                                    QLabel *plab1 = new QLabel("x");
                                    QLabel *plab2 = new QLabel("Hoop");
                                    QLabel *plab4 = new QLabel("Quad panels");
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
                                    QLabel *plabCenter = new QLabel("Uniform");
                                    plabCenter->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
                                    QLabel *plabEndPoints = new QLabel("End points");
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

                        pParamsLayout->addWidget(m_pUVParamsBox);
                        pParamsLayout->addWidget(m_pFitBox);
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
                            QLabel *pLabelFrame = new QLabel("Frame positions");
                            pLabelFrame->setStyleSheet("font-weight: bold");
                            pLabelFrame->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
                            pFramePosLayout->addWidget(pLabelFrame);
                            pFramePosLayout->addWidget(m_pcptFrameTable);
                        }
                        pFramePosFrame->setLayout(pFramePosLayout);
                    }
                    QFrame *pFramePointFrame = new QFrame;
                    {
                        QVBoxLayout * pFramePointLayout = new QVBoxLayout;
                        {
                            QLabel *pLabelPoints = new QLabel("Active frame points");
                            pLabelPoints->setStyleSheet("font-weight: bold");
                            pLabelPoints->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
                            pFramePointLayout->addWidget(pLabelPoints);
                            pFramePointLayout->addWidget(m_pcptPointTable);
                        }
                        pFramePointFrame->setLayout(pFramePointLayout);
                    }

                    m_pTableSplitter->addWidget(pFramePosFrame);
                    m_pTableSplitter->addWidget(pFramePointFrame);
                }

                m_ptwDefinition->addTab(m_pMetaFrame,     "Meta");
                m_ptwDefinition->addTab(m_pNURBSParams,   "NURBS parameters");
                m_ptwDefinition->addTab(m_pTableSplitter, "Tables");
            }

            QHBoxLayout *pUndoRedoLayout = new QHBoxLayout;
            {
                m_ppbUndo = new QPushButton(QIcon(":/icons/OnUndo.png"), "Undo");
                m_ppbRedo = new QPushButton(QIcon(":/icons/OnRedo.png"), "Redo");
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


void FuseXflDefDlg::makeTables()
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


void FuseXflDefDlg::resizeEvent(QResizeEvent *pEvent)
{
    FuseXflDlg::resizeEvent(pEvent);

    onResizeTables();
    pEvent->accept();
}


void FuseXflDefDlg::showEvent(QShowEvent *pEvent)
{
    FuseXflDlg::showEvent(pEvent);
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
void FuseXflDefDlg::hideEvent(QHideEvent *pEvent)
{
    FuseXflDlg::hideEvent(pEvent);

    s_MainSplitterSizes = m_pMainHSplitter->saveState();
    s_TableSplitterSizes = m_pTableSplitter->saveState();

    s_PageIndex = m_ptwDefinition->currentIndex();

    s_BodyLineGrid.duplicate(m_pFuseLineView->grid());
    s_FrameGrid.duplicate(m_pFrameView->grid());

    pEvent->accept();
}


void FuseXflDefDlg::onResizeTables()
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


void FuseXflDefDlg::onConvertToFlatFace()
{
    FuseXflDlg::onConvertToFlatFace();

    if(m_pFuseXfl->isFlatFaceType())
    {
        if(m_ptwDefinition->count()>1) m_ptwDefinition->removeTab(0);
        updateFuseDlg();
    }
}


void FuseXflDefDlg::onUpdateFuseDlg()
{
    takePicture();
    updateFuseDlg();
}


void FuseXflDefDlg::updateFuseDlg()
{
    m_bChanged = true;

    fillFrameDataTable();
    fillPointDataTable();

    updateFuseXfl();
    m_pglFuseView->resetFuse();
    updateView();
}


void FuseXflDefDlg::setPicture()
{
    int iActiveFrameIndex = m_pFuseXfl->activeFrameIndex();

    Fuse const *pTmpBodyXfl = m_UndoStack.at(m_StackPos);
    m_pFuseXfl->duplicateFuse(*pTmpBodyXfl);
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


void FuseXflDefDlg::blockSignalling(bool bBlock)
{
    blockSignals(bBlock);
    m_pPointDelegate->blockSignals(bBlock);
    m_pFrameDelegate->blockSignals(bBlock);
    m_pcptPointTable->blockSignals(bBlock);
    m_pcptFrameTable->blockSignals(bBlock);

    m_pSelectionModelPoint->blockSignals(bBlock);
    m_pSelectionModelFrame->blockSignals(bBlock);
}


bool FuseXflDefDlg::loadSettings(QSettings &settings)
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


bool FuseXflDefDlg::saveSettings(QSettings &settings)
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


void FuseXflDefDlg::onFitPrecision()
{
    if(!m_pFuseXfl->isSectionType()) return;
    FuseSections *pFuseSections = dynamic_cast<FuseSections *>(m_pFuseXfl);
    pFuseSections->setFitPrecision(m_pdeFitPrecision->value()/Units::mtoUnit());
    updateFuseXfl();
    m_pglFuseView->resetFuse();
    updateView();

    m_bChanged = true;
}



