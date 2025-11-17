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
#include <QClipboard>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include "wingdefdlg.h"


#include <api/foil.h>
#include <api/objects2d.h>
#include <api/objects3d.h>
#include <api/planexfl.h>
#include <api/polar.h>
#include <api/surface.h>
#include <api/units.h>
#include <api/wingxfl.h>

#include <interfaces/editors/inertia/partinertiadlg.h>
#include <interfaces/editors/planeedit/planexfldlg.h>
#include <interfaces/editors/wingedit/wingdlg.h>
#include <interfaces/editors/wingedit/wingscaledlg.h>
#include <interfaces/editors/wingedit/wingsectiondelegate.h>
#include <interfaces/exchange/cadexportdlg.h>
#include <interfaces/exchange/stlwriterdlg.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dwingview.h>
#include <interfaces/widgets/color/colorbtn.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <api/xmlwingwriter.h>


QByteArray WingDefDlg::s_HSplitterSizes;
QByteArray WingDefDlg::s_VSplitterSizes;


WingDefDlg::WingDefDlg(QWidget *pParent) : WingDlg(pParent)
{
    makeWingTable();
    setupLayout();
    connectSignals();

    m_pTableContextMenu = new QMenu("Section",this);
    {
        m_pTableContextMenu->addAction(m_pInsertBefore);
        m_pTableContextMenu->addAction(m_pInsertAfter);
        m_pTableContextMenu->addAction(m_pInsertNBefore);
        m_pTableContextMenu->addAction(m_pInsertNAfter);
        m_pTableContextMenu->addAction(m_pDuplicateSection);
        m_pTableContextMenu->addAction(m_pDeleteSection);
        m_pTableContextMenu->addAction(m_pResetSection);
        m_pTableContextMenu->addSeparator();
        m_pTableContextMenu->addAction(m_pCopyAction);
        m_pTableContextMenu->addAction(m_pPasteAction);
    }
}


WingDefDlg::~WingDefDlg()
{
}


void WingDefDlg::connectSignals()
{
    connectWingSignals();

    connect(m_pHSplitter,        SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(m_pVSplitter,        SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));

    connect(m_pchTwoSided,       SIGNAL(clicked()),              SLOT(onWingSides()));
    connect(m_pchCloseInnerSide, SIGNAL(clicked()),              SLOT(onWingSides()));
    connect(m_pchsymmetric,      SIGNAL(clicked()),              SLOT(onWingSides()));
    connect(m_pieTipStrips,      SIGNAL(intChanged(int)),        SLOT(onTipStrips()));
    connect(m_prbRightSide,      SIGNAL(clicked()),              SLOT(onSide()));
    connect(m_prbLeftSide,       SIGNAL(clicked()),              SLOT(onSide()));

    connect(m_pcptSections,      SIGNAL(customContextMenuRequested(QPoint)), SLOT(onWingTableContextMenu(QPoint)));
    connect(m_pcptSections,      SIGNAL(clicked(QModelIndex)),               SLOT(onWingTableClicked(QModelIndex)));
    connect(m_pcptSections->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onRowChanged(QModelIndex,QModelIndex)));
    connect(m_pcptSections,      SIGNAL(dataPasted()),                                      SLOT(onCellChanged()));
    connect(m_pSectionModel,     SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(onCellChanged()));
}


void WingDefDlg::makeWingTable()
{
    m_pcptSections = new CPTableView(this);
    m_pcptSections->setEditable(true);
    m_pcptSections->setWindowTitle("Wing definition");
    m_pcptSections->setWordWrap(false);

    m_pcptSections->setContextMenuPolicy(Qt::CustomContextMenu);
    m_pcptSections->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_pcptSections->setEditTriggers(QAbstractItemView::AllEditTriggers);

    QHeaderView *pVHeader = m_pcptSections->verticalHeader();
    pVHeader->setDefaultSectionSize(m_pcptSections->fontHeight()*2);
    pVHeader->setSectionResizeMode(QHeaderView::Fixed);

    QHeaderView *pHorizontalHeader = m_pcptSections->horizontalHeader();
    pHorizontalHeader->setStretchLastSection(true);

    m_pSectionModel = new WingSectionModel(nullptr, this);
    m_pcptSections->setModel(m_pSectionModel);

    m_pSectionDelegate = new WingSectionDelegate(this);
    m_pcptSections->setItemDelegate(m_pSectionDelegate);

    QVector<int>precision({3,3,3,3,3,0,0,0,0,0});
    m_pSectionDelegate->setPrecision(precision);
}


void WingDefDlg::setupLayout()
{
    QFrame *pLeftSideFrame = new QFrame;
    {
        QVBoxLayout *pLeftSideLayout = new QVBoxLayout;
        {
            QFrame *pfrWingSym = new QFrame;
            {
                QHBoxLayout *pSectionLayout = new QHBoxLayout;
                {
                    m_pchTwoSided   = new QCheckBox("two-sided");
                    m_pchsymmetric  = new QCheckBox("symmetric");
                    m_prbRightSide  = new QRadioButton("Right Side");
                    m_prbLeftSide   = new QRadioButton("Left Side");

                    m_pchCloseInnerSide = new QCheckBox("Close inner side");

                    pSectionLayout->addWidget(m_pchTwoSided);
                    pSectionLayout->addWidget(m_pchsymmetric);
                    pSectionLayout->addWidget(m_prbRightSide);
                    pSectionLayout->addWidget(m_prbLeftSide);
                    pSectionLayout->addStretch();
                    pSectionLayout->addWidget(m_pchCloseInnerSide);
                }
                pfrWingSym->setLayout(pSectionLayout);
            }

            m_pVSplitter = new QSplitter(Qt::Vertical, this);
            {
                m_pVSplitter->setChildrenCollapsible(false);
                m_pVSplitter->setChildrenCollapsible(true);
                m_pVSplitter->addWidget(m_pcptSections);
                m_pVSplitter->addWidget(m_pglWingView);
                m_pVSplitter->setStretchFactor(0,1);
                m_pVSplitter->setStretchFactor(1,5);
                m_pVSplitter->setStretchFactor(2,11);
            }
            pLeftSideLayout->addWidget(pfrWingSym);
            pLeftSideLayout->addWidget(m_pVSplitter);
        }
        pLeftSideFrame->setLayout(pLeftSideLayout);
    }

    QFrame *pfrRightSide = new QFrame;
    {
        QVBoxLayout *pRightSideLayout = new QVBoxLayout(this);
        {
            QFrame *pfrMeta = new QFrame;
            {
                pfrMeta->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
                QVBoxLayout *pMetaLayout = new QVBoxLayout;
                {
                    QLabel *plabWingDescription = new QLabel("Description:");

                    pMetaLayout->addWidget(m_plabPlaneName);
                    pMetaLayout->addWidget(m_pleWingName);
                    pMetaLayout->addWidget(m_pcbColor);
                    pMetaLayout->addWidget(plabWingDescription);
                    pMetaLayout->addWidget(m_ppteDescription);
                }
                pfrMeta->setLayout(pMetaLayout);
            }
            QHBoxLayout *pTipStripLayout = new QHBoxLayout;
            {
                m_pieTipStrips = new IntEdit();
                m_pieTipStrips->setToolTip("Number of horizontal panel strips at the wing tips.\n"
                                           "Recommendation 2-5.");
                pTipStripLayout->addWidget(new QLabel("Tip strips"));
                pTipStripLayout->addWidget(m_pieTipStrips);
                pTipStripLayout->addStretch();
            }

            QVBoxLayout *pBottomLayout = new QVBoxLayout;
            {
                pBottomLayout->addStretch();
                pBottomLayout->addWidget(m_pglControls);
                pBottomLayout->addStretch();
                pBottomLayout->addWidget(m_pButtonBox);
            }
            pRightSideLayout->addWidget(pfrMeta);
            pRightSideLayout->addLayout(pTipStripLayout);
            pRightSideLayout->addStretch();
            pRightSideLayout->addLayout(pBottomLayout);
        }

        pfrRightSide->setLayout(pRightSideLayout);
    }

    m_pHSplitter = new QSplitter(Qt::Horizontal, this);
    {
        m_pHSplitter->setChildrenCollapsible(false);
        m_pHSplitter->addWidget(pLeftSideFrame);
        m_pHSplitter->addWidget(pfrRightSide);
        m_pHSplitter->setStretchFactor(0,5);
        m_pHSplitter->setStretchFactor(1,1);
        m_pHSplitter->setChildrenCollapsible(true);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);
}


void WingDefDlg::initDialog(WingXfl*pWing)
{
    WingDlg::initDialog(pWing);

    m_pSectionModel->setWing(pWing);
    updateData();
    m_pSectionDelegate->setWingSectionArray(m_pWing->sections());

    setWingProps();

    QModelIndex index = m_pSectionModel->index(0,0);
    m_pcptSections->setCurrentIndex(index);

    m_pieTipStrips->setValue(pWing->nTipStrips());

    m_bChanged = m_bDescriptionChanged = false;
}


void WingDefDlg::showEvent(QShowEvent *pEvent)
{
    WingDlg::showEvent(pEvent);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    if(s_VSplitterSizes.length()>0) m_pVSplitter->restoreState(s_VSplitterSizes);
}


void WingDefDlg::hideEvent(QHideEvent *pEvent)
{
    WingDlg::hideEvent(pEvent);
    s_HSplitterSizes  = m_pHSplitter->saveState();
    s_VSplitterSizes  = m_pVSplitter->saveState();
}


void WingDefDlg::resizeEvent(QResizeEvent *pEvent)
{
    WingDlg::resizeEvent(pEvent);

    int n = m_pSectionModel->actionColumn();
    QHeaderView *pHHeader = m_pcptSections->horizontalHeader();
    //pHHeader->setDefaultSectionSize(1);
    pHHeader->setSectionResizeMode(n, QHeaderView::Stretch);
    pHHeader->resizeSection(n, 1);

    double w = double(m_pcptSections->width())/100.0;
    int wFoil  = int(17.0*w);
    int wCols  = int(7.5*w);

    m_pcptSections->setColumnWidth(0, wCols);
    m_pcptSections->setColumnWidth(1, wCols);
    m_pcptSections->setColumnWidth(2, wCols);
    m_pcptSections->setColumnWidth(3, wCols);
    m_pcptSections->setColumnWidth(4, wCols);
    m_pcptSections->setColumnWidth(5, wFoil);
    m_pcptSections->setColumnWidth(6, wCols);
    m_pcptSections->setColumnWidth(7, wCols);
    m_pcptSections->setColumnWidth(8, wCols);
    m_pcptSections->setColumnWidth(9, wCols);
    m_pcptSections->setColumnWidth(10, wCols);
}


void WingDefDlg::onWingTableContextMenu(QPoint)
{
    m_pTableContextMenu->exec(QCursor::pos());
}


void WingDefDlg::onSide()
{
    m_bRightSide = m_prbRightSide->isChecked();
    m_pSectionModel->setEditSide(m_bRightSide);

    m_pSectionModel->updateData();

    m_bChanged = true;
    if(!m_pWing->isTwoSided()) m_bRightSide = false;
    m_pglWingView->resetglSectionHighlight(m_iSection, m_bRightSide);

    m_pglWingView->update();
}


void WingDefDlg::onWingSides()
{
    m_pWing->setTwoSided(m_pchTwoSided->isChecked());
    m_pWing->setClosedInnerSide(m_pchCloseInnerSide->isChecked());

    if(m_pchsymmetric->isChecked())
    {
        m_pWing->setsymmetric(true);
        m_bRightSide          = true;
        for(int i=0; i<m_pWing->nSections(); i++)
        {
            m_pWing->setLeftFoilName(i, m_pWing->rightFoilName(i));
        }
    }
    else
    {
        m_pWing->setsymmetric(false);
    }

    if(!m_pWing->isTwoSided()) m_bRightSide = false;

    computeGeometry();
    setWingProps();
    setControls();

    m_bChanged = true;
    m_pglWingView->resetglSectionHighlight(m_iSection, m_bRightSide);
    m_pglWingView->resetglWing();
    m_pglWingView->update();
}


void WingDefDlg::onTipStrips()
{
    int nStrips = m_pieTipStrips->value();
    nStrips = std::max(1, nStrips);
    nStrips = std::min(nStrips, 100);
    m_pWing->setNTipStrips(nStrips);

    computeGeometry();
    setWingProps();
    setControls();
    if(!m_pWing->isTwoSided()) m_bRightSide = false;

    m_bChanged = true;
    m_pglWingView->resetglSectionHighlight(m_iSection, m_bRightSide);
    m_pglWingView->resetglWing();
    m_pglWingView->update();
}


void WingDefDlg::updateData()
{
    m_pcptSections->closePersistentEditor(m_pcptSections->currentIndex());
    m_pSectionModel->updateData();
}


void WingDefDlg::setCurrentSection(int iSection)
{
    QModelIndex index = m_pSectionModel->index(iSection,0);
    m_pcptSections->setCurrentIndex(index);
    m_pcptSections->selectRow(index.row());
}



void WingDefDlg::onWingTableClicked(QModelIndex index)
{
    if(!index.isValid()) return;
    switch(index.column())
    {
        case 10:
        {
            QRect itemrect = m_pcptSections->visualRect(index);
            /*                QPoint dlgpos = pos();
            QPoint tablepos = m_pPartTable->pos();
            QPoint menupos = dlgpos + tablepos +  itemrect.topLeft();*/
            QPoint menupos = m_pcptSections->mapToGlobal(itemrect.topLeft());
            QMenu *pWingTableRowMenu = new QMenu("Section", this);
            {
                pWingTableRowMenu->addAction(m_pInsertBefore);
                pWingTableRowMenu->addAction(m_pInsertAfter);
                pWingTableRowMenu->addAction(m_pInsertNBefore);
                pWingTableRowMenu->addAction(m_pInsertNAfter);
                pWingTableRowMenu->addAction(m_pDuplicateSection);
                pWingTableRowMenu->addAction(m_pDeleteSection);
                pWingTableRowMenu->addAction(m_pResetSection);
            }
            pWingTableRowMenu->popup(menupos, m_pInsertBefore);

            break;
        }
        default:
        {
            break;
        }
    }
}


void WingDefDlg::onRowChanged(const QModelIndex &currentindex, const QModelIndex &)
{
    if(currentindex.row()>=m_pWing->nSections())
    {
        //the user has filled a cell in the last line
        //so add an item before reading
        m_pWing->appendWingSection();
    }
    m_iSection = currentindex.row();
    if(!m_pWing->isTwoSided()) m_bRightSide = false;
    m_pglWingView->resetglSectionHighlight(m_iSection, m_bRightSide);

    m_pglWingView->update();
}


void WingDefDlg::onCellChanged()
{
    m_bChanged = true;

    // check for center gap
    if(m_pWing->isTwoSided() && fabs(m_pWing->section(0).yPosition())<LENGTHPRECISION)
    {
        m_pWing->setClosedInnerSide(false);
        m_pchCloseInnerSide->setChecked(false);
    }

    computeGeometry();
    setWingProps();

    m_pglWingView->setReferenceLength(m_pWing->planformSpan());
    m_pglWingView->resetglWing();
    m_pglWingView->update();
}


void WingDefDlg::onCopy()
{
    m_pcptSections->copySelection();
}


void WingDefDlg::onPaste()
{
    m_pcptSections->pasteClipboard();
    m_bChanged = true;
}


void WingDefDlg::readParams()
{
    if(!m_pWing) return;
    WingDlg::readParams();

    if(m_pWing->isFin())
        m_pWing->setTwoSided(m_pchTwoSided->isChecked());

    m_pWing->setClosedInnerSide(m_pchCloseInnerSide->isChecked());

    //Update Geometry
    computeGeometry();
}


void WingDefDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("WingDefDlg");
    {
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();
        s_HSplitterSizes    = settings.value("HSplitterSizes").toByteArray();
        s_VSplitterSizes    = settings.value("VSplitterSizes").toByteArray();

        s_bOutline    = settings.value("Outline",    s_bOutline).toBool();
        s_bSurfaces   = settings.value("Surfaces",   s_bSurfaces).toBool();
        s_bVLMPanels  = settings.value("MeshPanels", s_bVLMPanels).toBool();
        s_bShowMasses = settings.value("Masses",     s_bShowMasses).toBool();
        s_bFoilNames  = settings.value("FoilNames",  s_bFoilNames).toBool();
    }
    settings.endGroup();
}


void WingDefDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("WingDefDlg");
    {
        settings.setValue("WindowGeom", s_Geometry);

        settings.setValue("HSplitterSizes",  s_HSplitterSizes);
        settings.setValue("VSplitterSizes",    s_VSplitterSizes);

        settings.setValue("Outline",    s_bOutline);
        settings.setValue("Surfaces",   s_bSurfaces);
        settings.setValue("MeshPanels", s_bVLMPanels);
        settings.setValue("Masses",     s_bShowMasses);
        settings.setValue("FoilNames",  s_bFoilNames);
    }
    settings.endGroup();
}


void WingDefDlg::setControls()
{
    m_pleWingName->setEnabled(true);

    m_pchTwoSided->setChecked(m_pWing->isTwoSided());
    m_pchCloseInnerSide->setChecked(m_pWing->isClosedInnerSide());
    m_pchsymmetric->setChecked(m_pWing->isSymmetric());
    m_prbRightSide->setChecked(m_pWing->isSymmetric());
    m_prbRightSide->setChecked(m_bRightSide);
    m_prbLeftSide->setChecked(!m_bRightSide);
    m_prbLeftSide->setEnabled(!m_pWing->isSymmetric() && m_pWing->isTwoSided());
    m_prbRightSide->setEnabled(m_pWing->isTwoSided());
    m_pchsymmetric->setEnabled(m_pWing->isTwoSided());
}


