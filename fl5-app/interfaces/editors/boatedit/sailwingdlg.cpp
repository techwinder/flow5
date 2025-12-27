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
#include <QMenu>
#include <QScrollArea>

#include "sailwingdlg.h"

#include <api/constants.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <api/sailwing.h>
#include <api/units.h>
#include <api/utils.h>
#include <api/wingsailsection.h>
#include <core/xflcore.h>
#include <interfaces/editors/boatedit/sailsectionview.h>
#include <interfaces/editors/boatedit/wingsailsectiondelegate.h>
#include <interfaces/editors/boatedit/wingsailsectionmodel.h>
#include <interfaces/editors/fuseedit/bodytransdlg.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dsailview.h>
#include <interfaces/widgets/customdlg/intvaluedlg.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>



SailWingDlg::SailWingDlg(QWidget *pParent) : SailDlg(pParent)
{
    setWindowTitle("Wing sail editor");
    makeTables();
    setupLayout();
    connectSignals();
}


void SailWingDlg::setupLayout()
{
//    QString strLength = Units::lengthUnitLabel();

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QFrame *pLeftFrame = new QFrame;
    {
        QVBoxLayout *pRightLayout = new QVBoxLayout;
        {
            m_pTabWidget = new QTabWidget(this);
            {
                QFrame *pMetaFrame = new QFrame;
                {
                    QVBoxLayout*pMetaLayout = new QVBoxLayout;
                    {
                        pMetaLayout->addWidget(m_pfrMeta);
                        pMetaLayout->addWidget(m_ppto);
                        pMetaLayout->setStretchFactor(m_pfrMeta, 1);
                        pMetaLayout->setStretchFactor(m_ppto, 5);
                    }
                    pMetaFrame->setLayout(pMetaLayout);
                }

                m_pViewVSplitter = new QSplitter(Qt::Vertical);
                {
                    m_pViewVSplitter->setChildrenCollapsible(false);
                    m_pViewVSplitter->addWidget(m_pfr2dView);
                    m_pViewVSplitter->addWidget(m_pcptSections);
                }

                m_pTabWidget->addTab(pMetaFrame, "Meta-data");
                m_pTabWidget->addTab(m_pViewVSplitter, "Sections");
                m_pTabWidget->setTabToolTip(0, "Ctrl+1");
                m_pTabWidget->setTabToolTip(1, "Ctrl+2");
            }

            m_pButtonBox->addButton(m_ppbSailOps, QDialogButtonBox::ActionRole);

            pRightLayout->addWidget(m_pTabWidget);
            pRightLayout->addWidget(m_pButtonBox);
        }
        pLeftFrame->setLayout(pRightLayout);
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


void SailWingDlg::connectSignals()
{
    connectBaseSignals();
    connect(m_p2dSectionView, SIGNAL(objectModified()),   SLOT(onUpdate()));

    connect(m_pcptSections,       SIGNAL(clicked(QModelIndex)),   SLOT(onSectionItemClicked(QModelIndex)));
    connect(m_pcptSections,       SIGNAL(dataPasted()),           SLOT(onSectionDataChanged()));
    connect(m_pWSSectionDelegate, SIGNAL(closeEditor(QWidget*)),  SLOT(onSectionDataChanged()));
    connect(m_pcptSections->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentSectionChanged(QModelIndex)));

    connect(m_pViewHSplitter,     SIGNAL(splitterMoved(int,int)), SLOT(onResizeTableColumns()));
    connect(m_pTabWidget,         SIGNAL(currentChanged(int)),    SLOT(onResizeTableColumns()));
}


void SailWingDlg::initDialog(Sail *pSail)
{
    SailDlg::initDialog(pSail);

    SailWing *pWS = dynamic_cast<SailWing*>(m_pSail);

    m_pWSSectionModel->setWingSail(pWS);
    m_pWSSectionDelegate->setWingSail(pWS);
    fillSectionModel();

    pWS->setActiveSection(m_iActiveSection);

    QModelIndex index = m_pWSSectionModel->index(m_iActiveSection, 0);
    if(index.isValid())
    {
        m_pcptSections->setCurrentIndex(index);
    }
    m_pglSailControls->enableCtrlPts(false);

    setSailData();

    setControls();
}


void SailWingDlg::accept()
{
    readData();

    if(m_pSail->refArea()<0.00001)
    {
        m_ppto->onAppendQText("The reference dimensions cannot be null\n");
        m_pTabWidget->setCurrentIndex(0);
        m_pfeRefArea->selectAll();
        m_pfeRefArea->setFocus();
        return;
    }

    if(m_pSail->refChord()<0.00001)
    {
        m_ppto->onAppendQText("The reference dimensions cannot be null\n");
        m_pTabWidget->setCurrentIndex(0);
        m_pfeRefChord->selectAll();
        m_pfeRefChord->setFocus();
        return;
    }

    m_pSail->makeSurface();
    XflDialog::accept();
}


void SailWingDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl  = false;
    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;

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
        default:
            SailDlg::keyPressEvent(pEvent);
    }
}


void SailWingDlg::resizeSectionTableColumns()
{
    // get size from event, resize columns here
    int wc    = int(double(m_pcptSections->width()) *0.87/ double(m_pWSSectionModel->columnCount()));
    m_pcptSections->setColumnWidth(0, wc);
    m_pcptSections->setColumnWidth(1, wc);
    m_pcptSections->setColumnWidth(2, wc);
    m_pcptSections->setColumnWidth(3, wc);
    m_pcptSections->setColumnWidth(4, wc);
    m_pcptSections->setColumnWidth(5, wc*2); // foil name
    m_pcptSections->setColumnWidth(6, wc);
    m_pcptSections->setColumnWidth(7, wc);
    m_pcptSections->setColumnWidth(8, wc);
    m_pcptSections->setColumnWidth(9, wc);
//    m_pcptSections->setColumnWidth(10, wc);  // Action column, stretched
}


void SailWingDlg::makeTables()
{
    //Set Frame Table
    m_pcptSections = new CPTableView(this);
    m_pcptSections->setEditable(true);
    m_pcptSections->setWindowTitle("Sail definition");
    m_pcptSections->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_pcptSections->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_pcptSections->horizontalHeader()->setStretchLastSection(true);

    m_pWSSectionModel = new WingSailSectionModel(this);
    m_pWSSectionModel->setActionColumn(10);
    m_pcptSections->setModel(m_pWSSectionModel);
    QItemSelectionModel *selSectionModel = new QItemSelectionModel(m_pWSSectionModel);
    m_pcptSections->setSelectionModel(selSectionModel);

    m_pWSSectionDelegate = new WingSailSectionDelegate(this);
    m_pcptSections->setItemDelegate(m_pWSSectionDelegate);

    QVector<int>precision({3,3,3,3,3,0,0,0,0});
    m_pWSSectionDelegate->setPrecision(precision);

    QModelIndex index = m_pWSSectionModel->index(m_iActiveSection, 0, QModelIndex());
    m_pcptSections->setCurrentIndex(index);

    m_pSectionTableSplitter = nullptr;
}


void SailWingDlg::updateSailSectionOutput()
{
    SailWing *pWS = dynamic_cast<SailWing*>(m_pSail);

    if(m_iActiveSection<0 && m_iActiveSection>=pWS->sectionCount())
    {
        m_p2dSectionView->clearOutputInfo();
        return;
    }
    WingSailSection const& sec = pWS->section(m_iActiveSection);

    Foil *pFoil = Objects2d::foil(sec.foilName());
    if(!pFoil)
    {
        m_p2dSectionView->clearOutputInfo();
        return;
    }

    QString info, props;
    info = QString::asprintf("Leading angle  = %7.2f", sec.twist() +atan(pFoil->camberSlope(0.0))*180.0/PI);
    props = info + DEGch + "\n";

    info = QString::asprintf("Trailing angle = %7.2f", sec.twist()+atan(pFoil->camberSlope(1.0))*180.0/PI);
    props += info + DEGch;

    m_p2dSectionView->setOutputInfo(props);
}


void SailWingDlg::setSailData()
{
    SailDlg::setSailData();
    fillSectionModel();
}


void SailWingDlg::setControls()
{
    SailDlg::setControls();

    m_pfrThickness->setEnabled(false);
    m_prbThin->setChecked(false);
    m_prbThick->setChecked(true);
}


void SailWingDlg::fillSectionModel()
{
    m_pWSSectionModel->updateData();
}


void SailWingDlg::readSectionData()
{
    //nothing to do, model is updated by MVC
}


void SailWingDlg::onAlignLuffPoints()
{
    SailWing *pWS = dynamic_cast<SailWing*>(m_pSail);
    int N = pWS->sectionCount();
    if(N<=2) return; // nothing to align

    Vector3d bot = pWS->sectionPosition(0);
    Vector3d top = pWS->sectionPosition(pWS->sectionCount()-1);
    double dx = top.x-bot.x;
    double dy = top.y-bot.y;
    double dz = top.z-bot.z;

    for(int is=1; is<pWS->sectionCount()-1; is++)
    {
        // find the target position
        Vector3d pos =pWS->sectionPosition(is);
        double tau = (pos.z-bot.z) / dz;
        Vector3d targetpos(bot.x+tau*dx, bot.y+tau*dy, bot.z+tau*dz);
        pWS->setSectionPosition(is, targetpos);
    }

    fillPointModel();
    setControls();
    updateSailGeometry();
    updateTriMesh();
    updateView();
}


void SailWingDlg::onUpdate()
{
    m_bChanged = true;

    updateSailGeometry();
    updateSailDataOutput();
    updateSailSectionOutput();
    updateView();
}


void SailWingDlg::onSectionDataChanged()
{
    m_bChanged = true;

    QModelIndex index = m_pcptSections->currentIndex();
    if(index.column()==6)
    {
        SailWing *pWS = dynamic_cast<SailWing*>(m_pSail);
        // make uniform number of panels in x direction
        int nx = pWS->section(m_iActiveSection).nxPanels();
        for(int is=0; is<pWS->sectionCount(); is++)
        {
            WingSailSection & sec = pWS->section(is);
            sec.setNXPanels(nx);
        }
        fillSectionModel();

        //restore the current selection
/*        QModelIndex idx = m_pWSSectionModel->index(index.row(), index.column());
        m_pcptSections->setCurrentIndex(idx);*/
    }

    updateSailGeometry();
    updateSailDataOutput();

    updateTriMesh();
    m_pglSailView->resetglSail();
    updateView();
}


void SailWingDlg::onCurrentSectionChanged(const QModelIndex &index)
{
    if(!index.isValid()) return;

    SailWing *pWS = dynamic_cast<SailWing*>(m_pSail);
    pWS->setActiveSection(index.row());
    m_iActiveSection = index.row();

    m_pglSailView->resetglSectionHighlight();
    updateView();
}


void SailWingDlg::onSectionItemClicked(const QModelIndex &index)
{
    if(!index.isValid()) return;
    SailWing *pWS = dynamic_cast<SailWing*>(m_pSail);
    pWS->setActiveSection(index.row());
    m_iActiveSection = index.row();
    int col = index.column();
    int actioncol  = m_pWSSectionModel->actionColumn();
    if(col==actioncol)
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

    m_pglSailView->resetglSectionHighlight();
    updateView();
}


void SailWingDlg::onInsertSectionBefore()
{
    SailWing *pWS = dynamic_cast<SailWing*>(m_pSail);
    if(!m_pSail || m_iActiveSection<0 || m_iActiveSection>=pWS->sectionCount()) return;

    pWS->createSection(m_iActiveSection);

    fillSectionModel();
    pWS->makeSurface();
    updateSailGeometry();
    updateTriMesh();

//    m_pglSailView->setDebugPoints(Surface::s_DebugPts, Surface::s_DebugVecs);

    m_bChanged = true;
    setControls();
    updateView();
}


void SailWingDlg::onInsertSectionAfter()
{
    if(!m_pSail) return;
    SailWing *pWS = dynamic_cast<SailWing*>(m_pSail);

    pWS->createSection(m_iActiveSection+1);
    m_iActiveSection++;

    fillSectionModel();
    pWS->makeSurface();
    updateSailGeometry();
    updateTriMesh();
    m_pglSailView->setDebugPoints(Surface::s_DebugPts, Surface::s_DebugVecs);
    m_pcptSections->selectRow(m_iActiveSection);
    m_bChanged = true;
    setControls();
    updateView();
}


void SailWingDlg::onDeleteSection()
{
    SailWing *pWS = dynamic_cast<SailWing*>(m_pSail);
     if(!m_pSail || m_iActiveSection<0 || m_iActiveSection>=pWS->sectionCount()) return;

    if(pWS->sectionCount()<=2)
    {
        QMessageBox::warning(window(), "Warning", "At least two sections are required to define the sail");
        return;
    }

    pWS->deleteSection(m_iActiveSection);

    fillSectionModel();

    if(m_iActiveSection>=pWS->sectionCount()) m_iActiveSection--;
    if(m_iActiveSection<0) m_iActiveSection=0;

    pWS->makeSurface();
    updateSailGeometry();
    updateTriMesh();

    m_bChanged = true;
    setControls();
    updateView();
}


void SailWingDlg::onTranslateSection()
{
    SailWing *pWS = dynamic_cast<SailWing*>(m_pSail);
    if(!m_pSail || m_iActiveSection<0 || m_iActiveSection>=pWS->sectionCount()) return;

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

    Vector3d newpos = pWS->sectionPosition(m_iActiveSection);
    newpos.x += tx;
    newpos.y += ty;
    pWS->setSectionPosition(m_iActiveSection, newpos);

    m_bChanged = true;

    setControls();
    updateSailGeometry();
    updateTriMesh();
    updateView();
}


void SailWingDlg::onSelectSection(int iSection)
{
    SailWing *pWS = dynamic_cast<SailWing*>(m_pSail);
    if(iSection<0 || iSection>=pWS->sectionCount()) return;

    pWS->setActiveSection(iSection);
    m_iActiveSection = iSection;

    QModelIndex index = m_pWSSectionModel->index(m_iActiveSection, 0);
    if(index.isValid())
    {
        m_pcptSections->setCurrentIndex(index);
    }

    fillPointModel();
    m_pglSailView->resetglSectionHighlight();

    updateSailSectionOutput();
    updateView();
}
