/****************************************************************************

    flow5
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

#include <QMessageBox>
#include <QVBoxLayout>
#include <QToolBar>
#include <QPushButton>
#include <QScrollArea>
#include <QHeaderView>

#include "nurbssaildlg.h"

#include <xfl/editors/boatedit/sailsectionview.h>
#include <xfl/editors/fuseedit/bodytransdlg.h>
#include <xfl/opengl/gl3dboatview.h>
#include <xfl/opengl/gl3dsailview.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflgeom/geom3d/nurbssurface.h>
#include <xflobjects/sailobjects/sails/nurbssail.h>
#include <xflobjects/sailobjects/sails/sail.h>
#include <xflwidgets/customwts/xfldelegate.h>
#include <xflwidgets/customwts/actionitemmodel.h>
#include <xflwidgets/customwts/cptableview.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/plaintextoutput.h>


NURBSSailDlg::NURBSSailDlg(QWidget *pParent) : SailDlg(pParent)
{
    setWindowTitle("NURBS sail editor");

    makeTables();
    setupLayout();
    connectSignals();
}


void NURBSSailDlg::connectSignals()
{
    connectBaseSignals();

    connect(m_pcptSections,     SIGNAL(clicked(QModelIndex)),   SLOT(onSectionItemClicked(QModelIndex)));
    connect(m_pcptSections,     SIGNAL(dataPasted()),           SLOT(onSectionDataChanged()));
    connect(m_pSectionDelegate, SIGNAL(closeEditor(QWidget*)), SLOT(onSectionDataChanged()));
    connect(m_pcptSections->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentSectionChanged(QModelIndex)));

    connect(m_pcptPoints,       SIGNAL(clicked(QModelIndex)),   SLOT(onPointItemClicked(QModelIndex)));
    connect(m_pcptPoints,       SIGNAL(dataPasted()),           SLOT(onPointDataChanged()));
    connect(m_pPointDelegate,   SIGNAL(closeEditor(QWidget*)),  SLOT(onPointDataChanged()));

    connect(m_p2dSectionView,   SIGNAL(mouseDragReleased()),    SLOT(onUpdate()));

    connect(m_pieNXDegree,      SIGNAL(editingFinished()),      SLOT(onNurbsMetaChanged()));
    connect(m_pieNZDegree,      SIGNAL(editingFinished()),      SLOT(onNurbsMetaChanged()));

    connect(m_pdeEdgeWeightu,   SIGNAL(editingFinished()),      SLOT(onNurbsMetaChanged()));
    connect(m_pdeEdgeWeightv,   SIGNAL(editingFinished()),      SLOT(onNurbsMetaChanged()));


    connect(m_pTabWidget,       SIGNAL(currentChanged(int)),    SLOT(onResizeTableColumns()));
    connect(m_pViewHSplitter,   SIGNAL(splitterMoved(int,int)), SLOT(onResizeTableColumns()));
}


void NURBSSailDlg::setupLayout()
{
    QString strLength;
    Units::getLengthUnitLabel(strLength);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QFrame *pLeftFrame = new QFrame;
    {
        QVBoxLayout *pRightLayout = new QVBoxLayout;
        {
            m_pInternalSplitter = new QSplitter(Qt::Vertical);
            {
                m_pTabWidget = new QTabWidget(this);
                {
                    QWidget *pGeneralPage   = new QWidget(this);
                    {
                        QVBoxLayout *pGeneralLayout = new QVBoxLayout;
                        {
                            QGroupBox *pNURBSSailWidget = new QGroupBox("NURBS parameters");
                            {
                                QGridLayout *pNURBSDataLayout = new QGridLayout;
                                {
                                    QLabel *pLabX = new QLabel("x");
                                    QLabel *pLabZ = new QLabel("z");
                                    QLabel *pLabDegree = new QLabel("Degree=");
                                    QLabel *pLabEdgeWeight = new QLabel("Edge Weight =");

                                    m_pieNXDegree = new IntEdit(2);
                                    m_pieNZDegree = new IntEdit(2);

                                    m_pdeEdgeWeightu = new FloatEdit(1.0, 1);
                                    m_pdeEdgeWeightv = new FloatEdit(1.0, 1);

                                    m_pieNXDegree->setToolTip("<p>The x-degree must be strictly less than the number of control points</p>");
                                    m_pieNZDegree->setToolTip("<p>The z-degree must be strictly less than the number of sections</p>");
                                    QLabel *pLimitLab = new QLabel(m_pieNXDegree->toolTip()+"\n"+m_pieNZDegree->toolTip());

                                    pNURBSDataLayout->addWidget(pLabX,              1, 2, Qt::AlignCenter);
                                    pNURBSDataLayout->addWidget(pLabZ,              1, 3, Qt::AlignCenter);
                                    pNURBSDataLayout->addWidget(pLabDegree,         2, 1, Qt::AlignRight | Qt::AlignVCenter);
                                    pNURBSDataLayout->addWidget(m_pieNXDegree,      2, 2);
                                    pNURBSDataLayout->addWidget(m_pieNZDegree,      2, 3);
                                    pNURBSDataLayout->addWidget(pLabEdgeWeight,     3, 1, Qt::AlignRight | Qt::AlignVCenter);
                                    pNURBSDataLayout->addWidget(m_pdeEdgeWeightu,   3, 2);
                                    pNURBSDataLayout->addWidget(m_pdeEdgeWeightv,   3, 3);
                                    pNURBSDataLayout->addWidget(pLimitLab,          4, 1, 1, 3);

                                    pNURBSDataLayout->setColumnStretch(1,1);
                                    pNURBSDataLayout->setColumnStretch(4,1);
                                    pNURBSDataLayout->setRowStretch(5,2);
                                    pNURBSSailWidget->setLayout(pNURBSDataLayout);
                                }
                            }

                            pGeneralLayout->addWidget(m_pMetaFrame);
                            pGeneralLayout->addWidget(pNURBSSailWidget);
                            pGeneralLayout->addStretch();
                        }
                        pGeneralPage->setLayout(pGeneralLayout);
                    }

                    m_pViewVSplitter = new QSplitter(Qt::Vertical);
                    {
                        m_pViewVSplitter->setChildrenCollapsible(false);
                        m_pViewVSplitter->addWidget(m_p2dViewFrame);
                        m_pViewVSplitter->addWidget(m_pSectionTableSplitter);
                    }

                    m_pTabWidget->addTab(pGeneralPage,     "Meta");
                    m_pTabWidget->addTab(m_pViewVSplitter, "Geometry");
                    m_pTabWidget->addTab(m_pfrMesh,        "Mesh");
                    m_pTabWidget->addTab(m_pfrTE,          "Trailing edge");
                    m_pTabWidget->setTabToolTip(0, "Ctrl+1");
                    m_pTabWidget->setTabToolTip(1, "Ctrl+2");
                    m_pTabWidget->setTabToolTip(2, "Ctrl+3");
                    m_pTabWidget->setTabToolTip(3, "Ctrl+4");
                }

                m_pInternalSplitter->addWidget(m_pTabWidget);
                m_pInternalSplitter->addWidget(m_pptoOutput);
                m_pInternalSplitter->setStretchFactor(0,1);
                m_pInternalSplitter->setStretchFactor(1,5);
            }
            m_pButtonBox->addButton(m_ppbSailOps, QDialogButtonBox::ActionRole);

            pRightLayout->addWidget(m_pInternalSplitter);
            pRightLayout->addWidget(m_pButtonBox);
        }
        pLeftFrame->setLayout(pRightLayout);
    }

    m_pViewHSplitter = new QSplitter(Qt::Horizontal);
    {
        m_pViewHSplitter->setChildrenCollapsible(false);
        m_pViewHSplitter->addWidget(pLeftFrame);
        m_pViewHSplitter->addWidget(m_p3dViewFrame);
        m_pViewHSplitter->setStretchFactor(0, 3);
        m_pViewHSplitter->setStretchFactor(1, 1);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pViewHSplitter);
    }

    setLayout(pMainLayout);
}


void NURBSSailDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_1:
        {
            if(bCtrl)
            {
                m_pTabWidget->setCurrentIndex(0);
            }
            break;
        }
        case Qt::Key_2:
        {
            if(bCtrl)
            {
                m_pTabWidget->setCurrentIndex(1);
            }
            break;
        }
        case Qt::Key_3:
        {
            if(bCtrl)
            {
                m_pTabWidget->setCurrentIndex(2);
            }
            break;
        }
        case Qt::Key_4:
        {
            if(bCtrl)
            {
                m_pTabWidget->setCurrentIndex(3);
            }
            break;
        }
        default: break;
    }
    SailDlg::keyPressEvent(pEvent);
}


void NURBSSailDlg::onSwapUV()
{
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    NURBSSurface memnurbs(pNS->nurbs());
    int nFr = memnurbs.frameCount();
    int nc = memnurbs.framePointCount();
    pNS->nurbs().setFrameCount(nc);
    pNS->nurbs().setFramePointCount(nFr);
    for(int isec=0; isec<nFr; isec++)
    {
        for(int ic=0; ic<nc; ic++)
        {
            pNS->nurbs().frame(ic).setCtrlPoint(isec, memnurbs.frame(isec).ctrlPointAt(ic));
        }
    }

    for(int i=0; i<pNS->nurbs().frameCount(); i++)
    {
        pNS->frame(i).setZPosition(pNS->frame(i).firstControlPoint().z);
    }
    pNS->nurbs().setKnots();

    updateTriMesh();
    updateSailGeometry();
    updateSailDataOutput();
    updateSailSectionOutput();
    fillSectionModel();
    fillPointModel();
    updateView();
}


void NURBSSailDlg::onReverseVSections()
{
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    NURBSSurface memnurbs(pNS->nurbs());
    int nFr = memnurbs.frameCount();
    for(int isec=0; isec<memnurbs.frameCount(); isec++)
    {
        pNS->nurbs().frame(isec).copyFrame(memnurbs.frame(nFr-isec-1));
    }
    pNS->nurbs().setKnots();

    updateTriMesh();
    updateSailGeometry();
    updateSailDataOutput();
    updateSailSectionOutput();
    fillSectionModel();
    fillPointModel();
    updateView();
}


void NURBSSailDlg::onReverseHPoints()
{
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    NURBSSurface memnurbs(pNS->nurbs());
    int nc = memnurbs.framePointCount();
    for(int isec=0; isec<memnurbs.frameCount(); isec++)
    {
        Frame &fr = pNS->nurbs().frame(isec);
        for(int ic=0; ic<nc; ic++)
        {
            fr.ctrlPoint(ic) = memnurbs.frame(isec).ctrlPointAt(nc-ic-1);
        }
    }
    updateTriMesh();
    updateSailGeometry();
    updateSailDataOutput();
    updateSailSectionOutput();
    fillSectionModel();
    fillPointModel();
    updateView();
}


void NURBSSailDlg::onNurbsMetaChanged()
{
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);

    // z is the u-direction
    // x is the v-direction
    int udegree = m_pieNZDegree->value();
    int vdegree = m_pieNXDegree->value();

    if(udegree<1)
    {
        m_pieNZDegree->blockSignals(true);
        udegree=std::max(1, pNS->nurbs().uDegree());
        m_pieNZDegree->setValue(udegree);
        m_pieNZDegree->blockSignals(false);
    }
    if(vdegree<1)
    {
        m_pieNXDegree->blockSignals(true);
        vdegree=std::max(1, pNS->nurbs().vDegree());
        m_pieNXDegree->setValue(vdegree);
        m_pieNXDegree->blockSignals(false);
    }

    if(udegree>=pNS->frameCount())
    {
        m_pieNZDegree->blockSignals(true);
        udegree=std::min(pNS->frameCount()-1, pNS->nurbs().uDegree());
        m_pieNZDegree->setValue(udegree);
        m_pieNZDegree->blockSignals(false);
    }
    if(vdegree>=pNS->framePointCount())
    {
        m_pieNXDegree->blockSignals(true);
        vdegree=std::min(pNS->framePointCount()-1, pNS->nurbs().vDegree());
        m_pieNXDegree->setValue(vdegree);
        m_pieNXDegree->blockSignals(false);
    }

    pNS->nurbs().setuDegree(udegree);
    pNS->nurbs().setvDegree(vdegree);
    pNS->nurbs().setuEdgeWeight(m_pdeEdgeWeightu->value());
    pNS->nurbs().setvEdgeWeight(m_pdeEdgeWeightv->value());

    m_bChanged = true;

    updateTriMesh();
    updateSailGeometry();
    updateSailDataOutput();
    updateSailSectionOutput();

    updateView();
}


void NURBSSailDlg::setSailData()
{
    SailDlg::setSailData();

    fillSectionModel();
    fillPointModel();
}


void NURBSSailDlg::initDialog(Sail *pSail)
{
    SailDlg::initDialog(pSail);

    updateSailDataOutput();
    setSailData();
    setControls();

    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    pNS->setActiveFrameIndex(m_iActiveSection);
    QModelIndex index = m_pSectionModel->index(m_iActiveSection, 0);
    if(index.isValid())
    {
        m_pcptSections->setCurrentIndex(index);
    }

    m_ppbTETop->setEnabled(false);
    m_ppbTEBotMid->setText("Mid. panels");
    m_pchGuessOpposite->setEnabled(false);
    m_pfeTEAngle->setEnabled(false);
}


void NURBSSailDlg::updateSailSectionOutput()
{
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    Frame &pFrame = pNS->activeFrame();
    if(pFrame.nCtrlPoints()<2)
    {
        m_p2dSectionView->clearOutputInfo();
        return;
    }

    QString info, props;

    info = QString::asprintf("Leading angle  = %7.2f", pNS->leadingAngle(m_iActiveSection));
    props = info + DEGCHAR + ("\n");


    info = QString::asprintf("Trailing angle = %7.2f", pNS->trailingAngle(m_iActiveSection));
    props += info + DEGCHAR;

    m_p2dSectionView->setOutputInfo(props);
}


void NURBSSailDlg::setControls()
{
    SailDlg::setControls();

    m_pfrThickness->setEnabled(false);
    m_prbThin->setChecked(true);
    m_prbThick->setChecked(false);

    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    m_pieNXDegree->setValue(pNS->nurbs().vDegree());
    m_pieNZDegree->setValue(pNS->nurbs().uDegree());

    m_pdeEdgeWeightu->setValue(pNS->nurbs().uEdgeWeight());
    m_pdeEdgeWeightv->setValue(pNS->nurbs().vEdgeWeight());
}


void NURBSSailDlg::resizeSectionTableColumns()
{
    // get size from event, resize columns here
    int wc    = int(double(m_pcptSections->width()) *0.9/ double(m_pSectionModel->columnCount()));
    for(int ic=0; ic<m_pSectionModel->columnCount(); ic++)
        m_pcptSections->setColumnWidth(ic, wc);
}


void NURBSSailDlg::makeTables()
{
    makeBaseTables();

    m_pSectionModel = new ActionItemModel(this);
    m_pSectionModel->setRowCount(3);//temporary
    m_pSectionModel->setColumnCount(3);
    m_pSectionModel->setHeaderData(0, Qt::Horizontal, "z ("+Units::lengthUnitLabel()+")");
    m_pSectionModel->setHeaderData(1, Qt::Horizontal, "angle (" + DEGCHAR + ")");
    m_pSectionModel->setHeaderData(2, Qt::Horizontal, "Actions");
    m_pSectionModel->setActionColumn(2);
    m_pcptSections->setModel(m_pSectionModel);
    QItemSelectionModel *pSelSectionModel = new QItemSelectionModel(m_pSectionModel);
    m_pcptSections->setSelectionModel(pSelSectionModel);
    m_pSectionDelegate = new XflDelegate(this);
    m_pcptSections->setItemDelegate(m_pSectionDelegate);

    m_pcptSections->horizontalHeader()->setStretchLastSection(true);
    m_pSectionDelegate->setDigits({3,1,0});
    m_pSectionDelegate->setItemTypes({XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::ACTION});
    m_pSectionDelegate->setActionColumn(2);

    QModelIndex index = m_pSectionModel->index(m_iActiveSection, 0, QModelIndex());
    m_pcptSections->setCurrentIndex(index);
}


void NURBSSailDlg::fillPointModel()
{
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);

    if(m_iActiveSection<0 || m_iActiveSection>=pNS->nurbs().frameCount()) return;
    Frame const &pSection = pNS->nurbs().frame(m_iActiveSection);

    if(m_iActiveSection<0 || m_iActiveSection>=pNS->nurbs().frameCount()) return;

    QModelIndex ind;

//    m_pPointModel->blockSignals(true); // avoid sending the dataChanged signal
    m_pPointModel->setRowCount(pSection.nCtrlPoints());

    for(int ip=0; ip<pSection.nCtrlPoints(); ip++)
    {
        ind = m_pPointModel->index(ip, 0, QModelIndex());
        m_pPointModel->setData(ind, pSection.ctrlPointAt(ip).x * Units::mtoUnit());

        ind = m_pPointModel->index(ip, 1, QModelIndex());
        m_pPointModel->setData(ind, pSection.ctrlPointAt(ip).y * Units::mtoUnit());
    }
//    m_pPointModel->blockSignals(false);
}


void NURBSSailDlg::fillSectionModel()
{
    if(!m_pSail) return;
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);

    QModelIndex ind;

    m_pSectionModel->setRowCount(pNS->frameCount());

    for(int is=0; is<pNS->frameCount(); is++)
    {
        Frame const &pFrame = pNS->frame(is);

        ind = m_pSectionModel->index(is, 0, QModelIndex());
        m_pSectionModel->setData(ind, pFrame.position().z * Units::mtoUnit());

        ind = m_pSectionModel->index(is, 1, QModelIndex());
        m_pSectionModel->setData(ind, pFrame.angle());
    }
}


void NURBSSailDlg::readSectionData()
{
    if(!m_pSail) return;
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);

    double d=0;
    bool bOK=false;

    QString strong;

    for(int is=0; is<m_pSectionModel->rowCount(); is++)
    {
        if(is>=pNS->pNurbs()->nFrames()) break;

        Frame &pSection = pNS->pNurbs()->frame(is);

        QStandardItem *pItem = m_pSectionModel->item(is,0);
        if(pItem)
        {
            strong = pItem->text();
            strong.replace(" ","");
            d = strong.toDouble(&bOK) / Units::mtoUnit();
            if(bOK)
            {
                pSection.setuPosition(2, d);
            }
        }

        pItem = m_pSectionModel->item(is,1);
        if(pItem)
        {
            strong = pItem->text();
            strong.replace(" ","");
            d = strong.toDouble(&bOK);
            if(bOK)
            {
                pSection.setAngle(d);
            }
        }
    }
}


void NURBSSailDlg::readData()
{
    SailDlg::readData();

    if(!m_pSail) return;
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);

    Vector3d LE = pNS->nurbs().leadingEdgeAxis();
    pNS->setLuffAngle(atan2(LE.x, LE.z) * 180./PI);

    pNS->pNurbs()->setvDegree(int(m_pieNXDegree->value()));
    //the degree may have been adjusted, so set the returned value
    m_pieNXDegree->setValue(pNS->pNurbs()->vDegree());

    pNS->pNurbs()->setuDegree(int(m_pieNZDegree->value()));
    //the degree may have been adjusted, so set the returned value
    m_pieNZDegree->setValue(pNS->pNurbs()->uDegree());

    pNS->pNurbs()->setuEdgeWeight(m_pdeEdgeWeightu->value());
    pNS->pNurbs()->setvEdgeWeight(m_pdeEdgeWeightv->value());

    readPointData();
    readSectionData();// read section data after to set z position
}


void NURBSSailDlg::readPointData()
{
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);

    double d=0;
    bool bOK=false;
    QString strong;
    QStandardItem *pItem;
    if(m_iActiveSection<0 || m_iActiveSection>=pNS->nurbs().frameCount()) return;

    Frame &pSection = pNS->nurbs().frame(m_iActiveSection);

    for (int row=0; row< m_pPointModel->rowCount(); row++)
    {
        if(row >= pSection.nCtrlPoints()) break;

        pItem = m_pPointModel->item(row,0);
        if(pItem)
        {
            strong = pItem->text();
            strong.replace(" ","");
            d = strong.toDouble(&bOK);
            if(bOK) pSection.ctrlPoint(row).x = d / Units::mtoUnit();
        }

        pItem = m_pPointModel->item(row,1);
        if(pItem)
        {
            strong = pItem->text();
            strong.replace(" ","");
            d = strong.toDouble(&bOK);
            if(bOK) pSection.ctrlPoint(row).y = d / Units::mtoUnit();
        }

        pSection.ctrlPoint(row).z = pNS->nurbs().frame(m_iActiveSection).position().z;

    }
}


void NURBSSailDlg::onCurrentSectionChanged(const QModelIndex &index)
{
    if(!index.isValid()) return;
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    m_iActiveSection = index.row();
    pNS->setActiveFrameIndex(m_iActiveSection);

    fillPointModel();

    m_pglSailView->resetglSectionHighlight();
    updateView();
}


void NURBSSailDlg::onSectionItemClicked(const QModelIndex &index)
{
    if(!index.isValid()) return;
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);

    m_iActiveSection = index.row();
    pNS->setActiveFrameIndex(m_iActiveSection);

    if(index.column() == m_pSectionModel->actionColumn())
    {
        QRect itemrect = m_pcptSections->visualRect(index);
        QPoint menupos = m_pcptSections->mapToGlobal(itemrect.topLeft());
        QMenu *pRowMenu = new QMenu("Section",this);

        QAction *pInsertBefore = new QAction("Insert before", this);
        connect(pInsertBefore, SIGNAL(triggered(bool)), this, SLOT(onInsertSectionBefore()));
        pRowMenu->addAction(pInsertBefore);

        QAction *pInsertAfter = new QAction("Insert after", this);
        connect(pInsertAfter, SIGNAL(triggered(bool)), this, SLOT(onInsertSectionAfter()));
        pRowMenu->addAction(pInsertAfter);

        QAction *pDeleteRow = new QAction("Delete", this);
        connect(pDeleteRow, SIGNAL(triggered(bool)), this, SLOT(onDeleteSection()));
        pRowMenu->addAction(pDeleteRow);

        QAction *pTranslate = new QAction("Translate", this);
        connect(pTranslate, SIGNAL(triggered(bool)), this, SLOT(onTranslateSection()));
        pRowMenu->addAction(pTranslate);

        QAction *pScaleSection = new QAction("Scale", this);
        connect(pScaleSection, SIGNAL(triggered(bool)), this, SLOT(onScaleSection()));
        pRowMenu->addAction(pScaleSection);

        pRowMenu->exec(menupos);
    }


    fillPointModel();

    setControls();
    m_pglSailView->resetglSectionHighlight();
    updateView();
}


void NURBSSailDlg::onPointItemClicked(const QModelIndex &index)
{
    if(!index.isValid()) return;

    onCurrentPointChanged(index);

    if(index.column() == m_pPointModel->actionColumn())
    {
        QRect itemrect = m_pcptPoints->visualRect(index);
        QPoint menupos = m_pcptPoints->mapToGlobal(itemrect.topLeft());

        QMenu *pRowMenu = new QMenu("Section",this);

        QAction *pInsertBefore = new QAction("Insert before", this);
        connect(pInsertBefore, SIGNAL(triggered(bool)), this, SLOT(onInsertPointBefore()));
        pRowMenu->addAction(pInsertBefore);

        QAction *pInsertAfter = new QAction("Insert after", this);
        connect(pInsertAfter, SIGNAL(triggered(bool)), this, SLOT(onInsertPointAfter()));
        pRowMenu->addAction(pInsertAfter);

        QAction *pDeleteRow = new QAction("Delete row", this);
        connect(pDeleteRow, SIGNAL(triggered(bool)), this, SLOT(onDeletePoint()));

        pRowMenu->addAction(pDeleteRow);
        pRowMenu->exec(menupos);
    }
}


void NURBSSailDlg::onCurrentPointChanged(const QModelIndex &index)
{
    if(!index.isValid()) return;

    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    int iCtrlPt = index.row();
    pNS->nurbs().frame(m_iActiveSection).setSelected(iCtrlPt);

    m_p2dSectionView->update();
    m_pglSailView->resetglSectionHighlight();
    m_pglSailView->update();
}


void NURBSSailDlg::onTranslateSection()
{
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    if(!m_pSail || m_iActiveSection<0 || m_iActiveSection>=pNS->nurbs().frameCount()) return;

    if(pNS->activeFrameIndex()<0) return;
    Frame &pFrame = pNS->activeFrame();

    BodyTransDlg dlg;
    dlg.initDialog();
    dlg.setFrameOnly(true);
    dlg.enableDirections(true, true, false);
    dlg.setFrameId(m_iActiveSection+1);
    dlg.enableFrameID(false);
    dlg.checkFrameId(true);
    if(dlg.exec()==QDialog::Rejected) return;

    double tx = dlg.dx();
    double ty = dlg.dy();
//    double tz = dlg.dz();
    pFrame.translate(tx,ty,0.0);

    m_bChanged = true;
    fillPointModel();
    setControls();
    updateTriMesh();
    updateSailGeometry();
    m_pglSailView->resetglSail();
    updateView();
}


void NURBSSailDlg::onInsertSectionAfter()
{
    if(!m_pSail) return;
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);

    pNS->createSection(m_iActiveSection+1);

    m_iActiveSection++;

    m_bChanged = true;
    fillSectionModel();
    m_pcptSections->selectRow(m_iActiveSection);
    setControls();
    updateTriMesh();
    updateSailGeometry();
    m_pglSailView->resetglSail();
    updateView();
}


void NURBSSailDlg::onInsertSectionBefore()
{
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    if(!m_pSail || m_iActiveSection<0 || m_iActiveSection>=pNS->nurbs().frameCount()) return;

    pNS->createSection(m_iActiveSection);

    m_bChanged = true;
    fillSectionModel();
    m_pcptSections->selectRow(m_iActiveSection);
    setControls();
    updateTriMesh();
    updateSailGeometry();
    m_pglSailView->resetglSail();
    updateView();
}


void NURBSSailDlg::onDeleteSection()
{
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    if(!m_pSail || m_iActiveSection<0 || m_iActiveSection>=pNS->nurbs().frameCount()) return;

    if(pNS->nurbs().frameCount()<=2)
    {
        QMessageBox::warning(window(), "Warning", "At least two sections are required to define the sail");
        return;
    }

    if(pNS->frameCount()<=pNS->nurbs().uDegree()+1)
    {
        QMessageBox::warning(this, "Warning", "Cannot remove: the number of sections must be at least equal to the z degree+1");
        return;
    }

    pNS->deleteSection(m_iActiveSection);

    fillSectionModel();

    if(m_iActiveSection>=pNS->nurbs().frameCount()) m_iActiveSection--;
    if(m_iActiveSection<0) m_iActiveSection=0;

    pNS->pNurbs()->setuDegree(int(m_pieNZDegree->value()));
    pNS->makeSurface();
    updateView();

    m_bChanged = true;
    m_pcptSections->selectRow(m_iActiveSection);
    setControls();
    updateTriMesh();
    updateSailGeometry();
    updateView();

}


void NURBSSailDlg::onInsertPointBefore()
{
    if(!m_pSail) return;
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    NURBSSurface *pNurbs = pNS->pNurbs();

    int iPt = m_pcptPoints->currentIndex().row();
    for(int ifr=0; ifr<pNurbs->frameCount(); ifr++)
    {
        Frame &pFrame = pNurbs->frame(ifr);
        pFrame.insertPoint(iPt);
        pFrame.point(iPt).z = pFrame.zPos();
    }

    m_bChanged = true;
    fillPointModel();
    m_pcptPoints->selectRow(iPt);
    setControls();
    updateTriMesh();
    updateSailGeometry();
    updateView();
}


void NURBSSailDlg::onInsertPointAfter()
{
    if(!m_pSail) return;
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    NURBSSurface *pNurbs = pNS->pNurbs();

    int iPt = m_pcptPoints->currentIndex().row();
    for(int ifr=0; ifr<pNurbs->frameCount(); ifr++)
    {
        Frame &pFrame = pNurbs->frame(ifr);
        pFrame.insertPoint(iPt+1);
        pFrame.point(iPt).z = pFrame.zPos();
    }

    m_bChanged = true;
    fillPointModel();
    m_pcptPoints->selectRow(iPt);
    setControls();
    updateTriMesh();
    updateSailGeometry();
    updateView();
}


void NURBSSailDlg::onDeletePoint()
{
    if(!m_pSail) return;
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    NURBSSurface *pNurbs = pNS->pNurbs();

    if(pNS->framePointCount()<=pNS->nurbs().vDegree()+1)
    {
        QString strange("Cannot remove: the number of point must be at least equal to the x degree+1");
        QMessageBox::warning(this, "Warning", strange);
        return;
    }

    int iPt = m_pcptPoints->currentIndex().row();
    for(int ifr=0; ifr<pNurbs->frameCount(); ifr++)
    {
        Frame &pFrame = pNurbs->frame(ifr);
        pFrame.removePoint(iPt);
    }

    m_bChanged = true;
    fillPointModel();
    m_pcptPoints->selectRow(iPt);
    setControls();
    updateTriMesh();
    updateSailGeometry();
    updateView();
}


/** Aligns the intermediate sections between the leading points
 * of the top and bottom sections */
void NURBSSailDlg::onAlignLuffPoints()
{
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    int N = pNS->frameCount();
    if(N<=2) return; // nothing to align

    Frame &pBot = pNS->frame(0);
    Frame &pTop = pNS->frame(N-1);
    Vector3d bot(pBot.firstControlPoint().x, pBot.firstControlPoint().y, pBot.position().z);
    Vector3d top(pTop.firstControlPoint().x, pTop.firstControlPoint().y, pTop.position().z);
    double dx = top.x-bot.x;
    double dy = top.y-bot.y;
    double dz = top.z-bot.z;

    for(int is=1; is<pNS->frameCount()-1; is++)
    {
        Frame &pFrame = pNS->frame(is);

        // find the target position
        Vector3d pos =pFrame.position();
        double tau = (pos.z-bot.z) / dz;
        Vector3d targetpos(bot.x+tau*dx, bot.y+tau*dy, bot.z+tau*dz);
        //define the translation
        double tx = targetpos.x-pFrame.firstControlPoint().x;
        double ty = targetpos.y-pFrame.firstControlPoint().y;
        for(int ic=0; ic<pFrame.nCtrlPoints(); ic++)
        {
            pFrame.ctrlPoint(ic).translate(tx, ty, 0.0);
        }
    }

    pNS->makeSurface();
    fillPointModel();
    setControls();
    updateTriMesh();
    updateSailGeometry();
    updateView();
}


void NURBSSailDlg::onSelectCtrlPoint(int iPoint)
{
    QModelIndex index = m_pPointModel->index(iPoint, 0);
    m_pcptPoints->setCurrentIndex(index);

    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    Frame &pFrame = pNS->activeFrame();
    pFrame.setHighlighted(iPoint);

    m_pglSailView->update();
}


void NURBSSailDlg::onSelectSection(int iSection)
{
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
    if(iSection<0 || iSection>=pNS->frameCount()) return;

    pNS->setActiveFrameIndex(iSection);
    m_iActiveSection = iSection;

    QModelIndex index = m_pSectionModel->index(m_iActiveSection, 0);
    if(index.isValid())
    {
        m_pcptSections->setCurrentIndex(index);
    }

    fillPointModel();
    m_pglSailView->resetglSectionHighlight();
    updateView();
}

