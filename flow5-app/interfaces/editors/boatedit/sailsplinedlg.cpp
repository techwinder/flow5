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


#include <QMessageBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QToolBar>
#include <QPushButton>
#include <QScrollArea>

#include "sailsplinedlg.h"


#include <api/bspline.h>
#include <api/sail.h>
#include <api/sailspline.h>
#include <api/spline.h>
#include <api/units.h>
#include <api/utils.h>

#include <core/xflcore.h>
#include <interfaces/editors/boatedit/sailsectionview.h>
#include <interfaces/editors/fuseedit/bodytransdlg.h>
#include <interfaces/exchange/cadexportdlg.h>
#include <interfaces/mesh/slg3d.h>
#include <interfaces/opengl/fl5views/gl3dboatview.h>
#include <interfaces/opengl/fl5views/gl3dsailview.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/customwts/xfldelegate.h>



SailSplineDlg::SailSplineDlg(QWidget *pParent) : ThinSailDlg(pParent)
{
    setWindowTitle("Spline sail editor");
    makeTables();
    setupLayout();
    connectSignals();
}


void SailSplineDlg::connectSignals()
{
    connectBaseSignals();

    connect(m_pcptSections,      SIGNAL(clicked(QModelIndex)),   SLOT(onSectionItemClicked(QModelIndex)));
    connect(m_pcptSections,      SIGNAL(dataPasted()),           SLOT(onSectionDataChanged()));
    connect(m_pSectionDelegate,  SIGNAL(closeEditor(QWidget*)), SLOT(onSectionDataChanged()));
    connect(m_pcptSections->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentSectionChanged(QModelIndex)));

    connect(m_pcptPoints,        SIGNAL(clicked(QModelIndex)),   SLOT(onPointItemClicked(QModelIndex)));
    connect(m_pcptPoints,        SIGNAL(dataPasted()),           SLOT(onPointDataChanged()));
    connect(m_pPointDelegate,    SIGNAL(closeEditor(QWidget*)), SLOT(onPointDataChanged()));

    connect(m_p2dSectionView,    SIGNAL(mouseDragReleased()),    SLOT(onUpdate()));

    connect(m_pieBSplineDeg,     SIGNAL(editingFinished()),      SLOT(onBSplineDegreeChanged()));

    connect(m_pcbSailType,       SIGNAL(activated(int)),         SLOT(onConvertSplines(int)));

    connect(m_pTabWidget,        SIGNAL(currentChanged(int)),    SLOT(onResizeTableColumns()));
    connect(m_pViewHSplitter,    SIGNAL(splitterMoved(int,int)), SLOT(onResizeTableColumns()));
}


void SailSplineDlg::keyPressEvent(QKeyEvent *pEvent)
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
    ThinSailDlg::keyPressEvent(pEvent);
}


void SailSplineDlg::initDialog(Sail *pSail)
{
    ThinSailDlg::initDialog(pSail);

    updateSailDataOutput();
    setSailData();
    setControls();

    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);
    Spline *pSpline =pSS->spline(0);

    switch(pSS->splineType())
    {
        default:
        case Spline::BSPLINE:
        {
            m_pcbSailType->setCurrentIndex(0);
            break;
        }
        case Spline::CUBIC:
        {
            m_pcbSailType->setCurrentIndex(1);
            break;
        }
        case Spline::BEZIER:
        {
            m_pcbSailType->setCurrentIndex(2);
            break;
        }
        case Spline::POINT:
        {
            m_pcbSailType->setCurrentIndex(3);
            break;
        }
    }

    m_pieBSplineDeg->setValue(pSpline->degree());

    fillSectionModel();
    fillPointModel();

    QModelIndex index = m_pSectionModel->index(m_iActiveSection, 0);
    if(index.isValid())
    {
        m_pcptSections->setCurrentIndex(index);
    }

    m_ppbTETop->setEnabled(false);
    m_ppbTEBotMid->setText(tr("Mid. panels"));
    m_pchGuessOpposite->setEnabled(false);
    m_pfeTEAngle->setEnabled(false);
}


void SailSplineDlg::setupLayout()
{
//    QString strLength = Units::lengthUnitLabel();

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QFrame *pLeftFrame = new QFrame;
    {
        QVBoxLayout *pLeftLayout = new QVBoxLayout;
        {
            m_pInternalSplitter = new QSplitter(Qt::Vertical);
            {
                m_pInternalSplitter->setChildrenCollapsible(false);

                m_pTabWidget = new QTabWidget(this);
                {
                    QWidget *pGeneralPage   = new QWidget(this);
                    {
                        QVBoxLayout *pGeneralLayout = new QVBoxLayout;
                        {
                            QGroupBox *pSplineTypeBox = new QGroupBox("Spline type");
                            {
                                QGridLayout *pSplineTypeLayout = new QGridLayout;
                                {
                                    QLabel *pLabSplineType = new QLabel("Spline type");

                                    m_pcbSailType = new QComboBox;
                                    m_pcbSailType->addItems({"BSPLINE", "CUBICSPLINE", "BEZIERSPLINE", "POINTSPLINE"});

                                    QLabel *pLabDegree = new QLabel("Spline degree");
                                    m_pieBSplineDeg = new IntEdit(2);
                                    QString tip= QString("The degree must be strictly less than the number of control points");
                                    m_pieBSplineDeg->setToolTip(tip);
                                    pSplineTypeLayout->addWidget(pLabSplineType,      1, 1, Qt::AlignRight | Qt::AlignVCenter);
                                    pSplineTypeLayout->addWidget(m_pcbSailType,  1, 2);
                                    pSplineTypeLayout->addWidget(pLabDegree,          2, 1, Qt::AlignRight | Qt::AlignVCenter);
                                    pSplineTypeLayout->addWidget(m_pieBSplineDeg,   2, 2);
                                    pSplineTypeLayout->setColumnStretch(1,1);
                                    pSplineTypeLayout->setColumnStretch(3,1);
                                }
                                pSplineTypeBox->setLayout(pSplineTypeLayout);
                            }

                            pGeneralLayout->addWidget(m_pfrMeta);
                            pGeneralLayout->addWidget(m_pSurfaceBox);
                            pGeneralLayout->addWidget(pSplineTypeBox);
                            pGeneralLayout->addStretch();
                        }
                        pGeneralPage->setLayout(pGeneralLayout);
                    }

                    m_pViewVSplitter = new QSplitter(Qt::Vertical);
                    {
                        m_pViewVSplitter->setChildrenCollapsible(false);
                        m_pViewVSplitter->addWidget(m_pfr2dView);
                        m_pViewVSplitter->addWidget(m_pSectionTableSplitter);
                    }


                    m_pTabWidget->addTab(pGeneralPage, "Meta-data");
                    m_pTabWidget->addTab(m_pViewVSplitter, "Geometry");
                    m_pTabWidget->addTab(m_pfrMesh, "Mesh");
                    m_pTabWidget->addTab(m_pfrTE, "Trailing edge");
                    m_pTabWidget->setTabToolTip(0, "Ctrl+1");
                    m_pTabWidget->setTabToolTip(1, "Ctrl+2");
                    m_pTabWidget->setTabToolTip(2, "Ctrl+3");
                    m_pTabWidget->setTabToolTip(3, "Ctrl+4");
                }
                m_pButtonBox->addButton(m_ppbSailOps, QDialogButtonBox::ActionRole);

                m_pInternalSplitter->addWidget(m_pTabWidget);
                m_pInternalSplitter->addWidget(m_ppto);
                m_pInternalSplitter->setStretchFactor(0,1);
                m_pInternalSplitter->setStretchFactor(1,5);
            }
            pLeftLayout->addWidget(m_pInternalSplitter);
            pLeftLayout->addWidget(m_pButtonBox);
        }
        pLeftFrame->setLayout(pLeftLayout);
    }


    m_pViewHSplitter = new QSplitter(Qt::Horizontal);
    {
        m_pViewHSplitter->setChildrenCollapsible(false);
        m_pViewHSplitter->addWidget(pLeftFrame);
        m_pViewHSplitter->addWidget(m_pfr3dView);
        m_pViewHSplitter->setStretchFactor(0, 3);
        m_pViewHSplitter->setStretchFactor(1, 1);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pViewHSplitter);
    }

    setLayout(pMainLayout);
}


void SailSplineDlg::setSailData()
{
    ThinSailDlg::setSailData();
    fillSectionModel();
}


void SailSplineDlg::updateSailSectionOutput()
{
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);
    Spline *pSpline = nullptr;
    if(m_iActiveSection>=0 && m_iActiveSection<pSS->sectionCount()) pSpline = pSS->spline(m_iActiveSection);
    if(!pSpline || pSpline->ctrlPointCount()<2)
    {
        m_p2dSectionView->clearOutputInfo();
        return;
    }

    QString info, props;
    double dx = pSpline->controlPoint(1).x - pSpline->firstCtrlPoint().x;
    double dy = pSpline->controlPoint(1).y - pSpline->firstCtrlPoint().y;
    double leadingangle = atan2(dy, dx)*180.0/PI;
    info = QString::asprintf("Leading angle  = %7.2f", leadingangle);
    props = info + DEGch + "\n";

    int n = pSpline->ctrlPointCount();
    dx = pSpline->lastCtrlPoint().x - pSpline->controlPoint(n-2).x;
    dy = pSpline->lastCtrlPoint().y - pSpline->controlPoint(n-2).y;
    double trailingangle = atan2(dy, dx)*180.0/PI;
    info = QString::asprintf("Trailing angle = %7.2f", trailingangle);
    props += info + DEGch;

    /*
     props += "\n";
    double c=0, xc=0;
    for(int ic=0; ic<int(pSpline->outputSize()); ic++)
    {
        if(fabs(pSpline->outputPt(ic).y)>c)
        {
            c = pSpline->outputPt(ic).y;
            xc= pSpline->outputPt(ic).x;
        }
    }
    info = QString::asprintf("Max. camber = %7.2f", c*Units::mtoUnit());
    props += info + Units::lengthUnitLabel() + "\n";
    info = QString::asprintf("Camber pos. = %7.2f", xc*Units::mtoUnit());
    props += info + Units::lengthUnitLabel();*/

    m_p2dSectionView->setOutputInfo(props);
}


void SailSplineDlg::fillSectionModel()
{
    if(!m_pSail) return;
    SailSpline *pNS = dynamic_cast<SailSpline*>(m_pSail);

    QModelIndex ind;

    m_pSectionModel->setRowCount(pNS->sectionCount());

    for(int is=0; is<pNS->sectionCount(); is++)
    {
//        Spline const *pSpline = pNS->spline(is);

        ind = m_pSectionModel->index(is, 0, QModelIndex());
        m_pSectionModel->setData(ind, pNS->sectionPosition(is).z * Units::mtoUnit());

        ind = m_pSectionModel->index(is, 1, QModelIndex());
        m_pSectionModel->setData(ind, pNS->sectionAngle(is));
    }
}


void SailSplineDlg::readSectionData()
{
    if(!m_pSail) return;
    SailSpline *pNS = dynamic_cast<SailSpline*>(m_pSail);

    double d=0;
    bool bOK=false;

    QString strong;
    QStandardItem *pItem=nullptr;

    for(int is=0; is<m_pSectionModel->rowCount(); is++)
    {

        pItem = m_pSectionModel->item(is,0);
        if(pItem)
        {
            strong = pItem->text();
            strong.replace(" ","");
            d = strong.toDouble(&bOK);
            if(bOK)
            {
                pNS->setZPosition(is, d/Units::mtoUnit());
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
                pNS->setSectionAngle(is, d);
            }
        }
    }
}


void SailSplineDlg::onCurrentSectionChanged(const QModelIndex &index)
{
    if(!index.isValid()) return;
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);
    pSS->setActiveSection(index.row());
    m_iActiveSection = index.row();

    fillPointModel();
    m_pcptPoints->update();
    m_pglSailView->resetglSectionHighlight();
    updateView();
}


void SailSplineDlg::onSectionItemClicked(const QModelIndex &index)
{
    if(!index.isValid()) return;
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);
    pSS->setActiveSection(index.row());
    m_iActiveSection = index.row();

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

//    setControls();

    fillPointModel();
    m_pglSailView->resetglSectionHighlight();
    updateView();
}


void SailSplineDlg::updateSailGeometry()
{
    ThinSailDlg::updateSailGeometry();
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);

    if(m_iActiveSection>=0&& m_iActiveSection<pSS->sectionCount())
    {
        pSS->updateActiveSpline();
    }
}


void SailSplineDlg::fillPointModel()
{
    SailSpline const *pSS = dynamic_cast<SailSpline const*>(m_pSail);
    Spline const *pSpline = pSS->splineAt(m_iActiveSection);
    if(!pSpline) return;


    QModelIndex ind;

//    m_pPointModel->blockSignals(true); // avoid sending the dataChanged signal
    m_pPointModel->setRowCount(pSpline->ctrlPointCount());

    for(int ip=0; ip<pSpline->ctrlPointCount(); ip++)
    {
        Vector2d const &pt = pSpline->controlPoint(ip);
        ind = m_pPointModel->index(ip, 0, QModelIndex());
        m_pPointModel->setData(ind, pt.x*Units::mtoUnit());

        ind = m_pPointModel->index(ip, 1, QModelIndex());
        m_pPointModel->setData(ind, pt.y*Units::mtoUnit());
    }
//    m_pPointModel->blockSignals(false);
}


void SailSplineDlg::readPointData()
{
    double d=0;
    bool bOK=false;
    QString strong;
    QStandardItem *pItem;

    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);
    Spline *pSpline = pSS->spline(m_iActiveSection);
    if(!pSpline) return;

    for (int ic=0; ic<m_pPointModel->rowCount(); ic++)
    {
        Vector2d pt = pSpline->controlPoint(ic);
        pItem = m_pPointModel->item(ic,0);
        if(pItem)
        {
            strong = pItem->text();
            strong.replace(" ","");
            d = strong.toDouble(&bOK);
            if(bOK) pt.x = d / Units::mtoUnit();
        }

        pItem = m_pPointModel->item(ic,1);
        if(pItem)
        {
            strong = pItem->text();
            strong.replace(" ","");
            d = strong.toDouble(&bOK);
            if(bOK) pt.y = d / Units::mtoUnit();
        }
        pSpline->setCtrlPoint(ic, pt.x, pt.y);
    }
}


void SailSplineDlg::onCurrentPointChanged(const QModelIndex &index)
{
    if(!index.isValid()) return;

    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);
    Spline *pSpline = pSS->spline(m_iActiveSection);
    if(!pSpline) return;

    int iCtrlPt = index.row();
    pSpline->setSelectedPoint(iCtrlPt);

    m_p2dSectionView->update();
    m_pglSailView->resetglSectionHighlight();
    m_pglSailView->update();
}


void SailSplineDlg::onPointItemClicked(const QModelIndex &index)
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

void SailSplineDlg::onTranslateSection()
{
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);
    if(!m_pSail || m_iActiveSection<0 || m_iActiveSection>=pSS->sectionCount()) return;
    Spline *pSpline = pSS->spline(m_iActiveSection);
    if(!pSpline) return;

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
    pSpline->translate(tx,ty);

    m_bChanged = true;
    fillPointModel();
    setControls();
    updateTriMesh();
    updateSailGeometry();
    updateView();
}


void SailSplineDlg::onInsertSectionBefore()
{
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);
    if(!m_pSail || m_iActiveSection<0 || m_iActiveSection>=pSS->sectionCount()) return;

    pSS->createSection(m_iActiveSection);

    fillSectionModel();

    m_bChanged = true;
    setControls();
    updateTriMesh();
    updateSailGeometry();
    updateView();
}


void SailSplineDlg::onInsertSectionAfter()
{
    if(!m_pSail) return;
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);

    pSS->createSection(m_iActiveSection+1);
    m_iActiveSection++;

    fillSectionModel();
    m_pcptSections->selectRow(m_iActiveSection);
    m_bChanged = true;
    setControls();
    updateTriMesh();
    updateSailGeometry();
    updateView();
}


void SailSplineDlg::resizeSectionTableColumns()
{
    // get size from event, resize columns here
    int wc    = int(double(m_pcptSections->width()) *0.9/ double(m_pSectionModel->columnCount()));
    for(int ic=0; ic<m_pSectionModel->columnCount(); ic++)
        m_pcptSections->setColumnWidth(ic, wc);
}


void SailSplineDlg::makeTables()
{
    makeBaseTables();

    m_pSectionModel = new ActionItemModel(this);
    m_pSectionModel->setRowCount(3);//temporary
    m_pSectionModel->setColumnCount(3);
    m_pSectionModel->setHeaderData(0, Qt::Horizontal, "z ("+Units::lengthUnitQLabel()+")");
    m_pSectionModel->setHeaderData(1, Qt::Horizontal, "angle ("+ DEGch +")");
    m_pSectionModel->setHeaderData(2, Qt::Horizontal, "Actions");
    m_pSectionModel->setActionColumn(2);
    m_pcptSections->setModel(m_pSectionModel);
    QItemSelectionModel *pSelSectionModel = new QItemSelectionModel(m_pSectionModel);
    m_pcptSections->setSelectionModel(pSelSectionModel);
    m_pSectionDelegate = new XflDelegate(this);
    m_pcptSections->setItemDelegate(m_pSectionDelegate);

    m_pcptSections->horizontalHeader()->setStretchLastSection(true);
    m_pSectionDelegate->setDigits({3,1,0});
    m_pSectionDelegate->setItemTypes({XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::STRING});
    m_pSectionDelegate->setActionColumn(2);

    QModelIndex index = m_pSectionModel->index(m_iActiveSection, 0, QModelIndex());
    m_pcptSections->setCurrentIndex(index);
}


void SailSplineDlg::onDeleteSection()
{
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);
    if(!m_pSail || m_iActiveSection<0 || m_iActiveSection>=pSS->sectionCount()) return;

    if(pSS->sectionCount()<=2)
    {
        QMessageBox::warning(window(), tr("Warning"), tr("At least two sections are required to define the sail"));
        return;
    }

    pSS->deleteSection(m_iActiveSection);

    fillSectionModel();

    if(m_iActiveSection>=pSS->sectionCount()) m_iActiveSection--;
    if(m_iActiveSection<0) m_iActiveSection=0;

    if(m_pSail->bRuledMesh()) m_pSail->makeRuledMesh(Vector3d());
    else                      m_pSail->clearRefTriangles();

    setControls();
    updateTriMesh();
    updateSailGeometry();
    updateView();


    m_bChanged = true;
}


void SailSplineDlg::onInsertPointBefore()
{
    if(!m_pSail) return;
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);

    int iCurPt = m_pcptPoints->currentIndex().row();

    for(int is=0; is<pSS->sectionCount(); is++)
    {
        Spline *pSpline = pSS->spline(is);
        pSpline->insertCtrlPointAt(iCurPt);
    }

    m_bChanged = true;
    fillPointModel();
    m_pcptPoints->selectRow(iCurPt);
    setControls();

    updateSailGeometry();

    if(m_pSail->bRuledMesh()) m_pSail->makeRuledMesh(Vector3d());
    else                      m_pSail->clearRefTriangles();
    updateTriMesh();

    updateView();
}


void SailSplineDlg::onInsertPointAfter()
{
    if(!m_pSail) return;
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);

    int iCurPt = m_pcptPoints->currentIndex().row();

    for(int is=0; is<pSS->sectionCount(); is++)
    {
        Spline *pSpline = pSS->spline(is);
        pSpline->insertCtrlPointAt(iCurPt+1);
    }

    m_bChanged = true;
    fillPointModel();
    m_pcptPoints->selectRow(iCurPt);
    setControls();

    updateSailGeometry();

    if(m_pSail->bRuledMesh()) m_pSail->makeRuledMesh(Vector3d());
    else                      m_pSail->clearRefTriangles();
    updateTriMesh();
    updateView();
}


void SailSplineDlg::onDeletePoint()
{
    if(!m_pSail) return;
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);

    int iCurPt = m_pcptPoints->currentIndex().row();

    if(pSS->splineType()==Spline::BSPLINE && pSS->spline(0)->ctrlPointCount()<=3)
    {
        QMessageBox::warning(window(), "Warning", "At least three points are required to define each BSpline section");
        return;
    }
    else if(pSS->spline(0)->ctrlPointCount()<=2)
    {
        QMessageBox::warning(window(), "Warning", "At least two points are required to define each section");
        return;
    }

    for(int is=0; is<pSS->sectionCount(); is++)
    {
        Spline *pSpline = pSS->spline(is);
        pSpline->removeCtrlPoint(iCurPt);
        if(pSpline->isBSpline())
        {
            if(pSpline->degree()>=int(pSpline->ctrlPointCount()))
                pSpline->setDegree(pSpline->ctrlPointCount()-1);
            m_pieBSplineDeg->setValue(pSpline->degree());
        }
    }

    m_bChanged = true;
    fillPointModel();
    m_pcptPoints->selectRow(iCurPt);
    setControls();

    updateSailGeometry();

    if(m_pSail->bRuledMesh()) m_pSail->makeRuledMesh(Vector3d());
    else                      m_pSail->clearRefTriangles();
    updateTriMesh();

    updateView();
}


/** aligns the intermediate sections between the leading points
 * of the top and bottom sections */
void SailSplineDlg::onAlignLuffPoints()
{
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);
    int N = pSS->sectionCount();
    if(N<=2) return; // nothing to align

    Spline *pBot = pSS->spline(0);
    Spline *pTop = pSS->spline(N-1);
    Vector3d bot(pBot->firstCtrlPoint().x, pBot->firstCtrlPoint().y, pSS->sectionPosition(0).z);
    Vector3d top(pTop->firstCtrlPoint().x, pTop->firstCtrlPoint().y, pSS->sectionPosition(N-1).z);
    double dx = top.x-bot.x;
    double dy = top.y-bot.y;
    double dz = top.z-bot.z;

    for(int is=1; is<pSS->sectionCount()-1; is++)
    {
        Spline *pSpline = pSS->spline(is);

        // find the target position
        Vector3d pos =pSS->sectionPosition(is);
        double tau = (pos.z-bot.z) / dz;
        Vector3d targetpos(bot.x+tau*dx, bot.y+tau*dy, bot.z+tau*dz);
        //define the translation
        double tx = targetpos.x-pSpline->firstCtrlPoint().x;
        double ty = targetpos.y-pSpline->firstCtrlPoint().y;
        for(int ic=0; ic<int(pSpline->ctrlPointCount()); ic++)
        {
            Vector2d Pt = pSpline->controlPoint(ic);
            Pt.translate(tx, ty);
            pSpline->setCtrlPoint(ic, Pt);
        }
        pSpline->updateSpline();
    }

    fillPointModel();
    setControls();
    updateTriMesh();
    updateSailGeometry();
    updateView();
}


void SailSplineDlg::onBSplineDegreeChanged()
{
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);
    if(pSS->splineType()!=Spline::BSPLINE) return; //ignore change

    int newdeg = m_pieBSplineDeg->value();
    Spline *pSpline = pSS->spline(0);
    if(newdeg>=int(pSpline->ctrlPointCount()))
    {
        QMessageBox::warning(window(), "Warning", "The degree must be less than the number of control points");
        m_pieBSplineDeg->setValue(pSpline->degree());
        return;
    }

    for(int is=0; is<pSS->sectionCount(); is++)
    {
        pSS->spline(is)->setDegree(newdeg);
    }
    updateTriMesh();
    updateSailGeometry();
    updateView();
}

//BSPLINE, CUBICSPLINE, BEZIERSPLINE, POINTSPLINE

void SailSplineDlg::onConvertSplines(int index)
{
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);
    switch(index)
    {
        case 0:
        {
            if(pSS->splineType()==Spline::BSPLINE) return; //nothing to change
            pSS->convertSplines(Spline::BSPLINE);
            break;
        }
        case 1:
        {
            if(pSS->splineType()==Spline::CUBIC) return; //nothing to change
            pSS->convertSplines(Spline::CUBIC);
            break;
        }
        case 2:
        {
            if(pSS->splineType()==Spline::BEZIER) return; //nothing to change
            pSS->convertSplines(Spline::BEZIER);
            break;
        }
        case 3:
        {
            if(pSS->splineType()==Spline::POINT) return; //nothing to change
            pSS->convertSplines(Spline::POINT);
            break;
        }
        default:
            return;
    }
    updateTriMesh();
    updateSailGeometry();
    updateView();
}


void SailSplineDlg::onSelectCtrlPoint(int iPoint)
{
    QModelIndex index = m_pPointModel->index(iPoint, 0);
    m_pcptPoints->setCurrentIndex(index);

    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);
    Spline *pSpline = pSS->spline(m_iActiveSection);
    pSpline->setHighlighted(iPoint);
    m_pglSailView->update();
}


void SailSplineDlg::onSelectSection(int iSection)
{
    SailSpline *pSS = dynamic_cast<SailSpline*>(m_pSail);
    if(iSection<0 || iSection>=pSS->sectionCount()) return;

    pSS->setActiveSection(iSection);
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


void SailSplineDlg::onUpdate()
{
    m_bChanged = true;

    fillPointModel();

    updateSailGeometry();

    if(m_pSail->bRuledMesh()) m_pSail->makeRuledMesh(Vector3d());
    else                      m_pSail->clearRefTriangles();

    updateTriMesh();
    updateSailDataOutput();
    updateSailSectionOutput();

    updateView();
}




