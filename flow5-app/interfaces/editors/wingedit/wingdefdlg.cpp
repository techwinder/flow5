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
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QFileDialog>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>

#include "wingdefdlg.h"

#include <api/foil.h>
#include <api/objects2d.h>
#include <api/objects3d.h>
#include <api/occ_globals.h>
#include <api/planexfl.h>
#include <api/polar.h>
#include <api/surface.h>
#include <api/units.h>
#include <api/wingxfl.h>
#include <api/xmlwingwriter.h>

#include <api/xmlwingwriter.h>
#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/editors/inertia/partinertiadlg.h>
#include <interfaces/editors/planeedit/planexfldlg.h>
#include <interfaces/editors/translatedlg.h>
#include <interfaces/editors/wingedit/wingscaledlg.h>
#include <interfaces/editors/wingedit/wingsectiondelegate.h>
#include <interfaces/editors/wingedit/wingsectionmodel.h>
#include <interfaces/exchange/cadexportdlg.h>
#include <interfaces/exchange/stlwriterdlg.h>
#include <interfaces/exchange/wingexportdlg.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dwingview.h>
#include <interfaces/widgets/color/colorbtn.h>
#include <interfaces/widgets/customdlg/intvaluedlg.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>

QByteArray WingDefDlg::s_Geometry;

double WingDefDlg::s_MaxEdgeLength=0.5;
double WingDefDlg::s_MaxEdgeDeflection = 45; //degrees
double WingDefDlg::s_QualityBound = sqrt(2.0);
int WingDefDlg::s_MaxMeshIter = 1;

bool WingDefDlg::s_bAxes       = true;
bool WingDefDlg::s_bOutline    = true;
bool WingDefDlg::s_bSurfaces   = true;
bool WingDefDlg::s_bVLMPanels  = false;
bool WingDefDlg::s_bShowMasses = false;
bool WingDefDlg::s_bFoilNames  = false;

Quaternion WingDefDlg::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);

QByteArray WingDefDlg::s_HSplitterSizes;
QByteArray WingDefDlg::s_VSplitterSizes;


WingDefDlg::WingDefDlg(QWidget *pParent) : XflDialog(pParent)
{
    setWindowTitle(tr("Wing editor"));
    setWindowFlag(Qt::WindowMinMaxButtonsHint);
    m_pWing = nullptr;

    m_iSection   = -1;
    m_bRightSide               = true;
    m_bChanged                 = false;
    m_bDescriptionChanged      = false;


    m_pInsertBefore     = new QAction(tr("Insert before"),                   this);
    m_pInsertBefore->setData(1);
    m_pInsertAfter      = new QAction(tr("Insert after"),                    this);
    m_pInsertAfter->setData(1);
    m_pInsertNBefore    = new QAction(tr("Insert multiple sections before"), this);
    m_pInsertNBefore->setData(-1);
    m_pInsertNAfter     = new QAction(tr("Insert multiple sections after"),  this);
    m_pInsertNAfter->setData(-1);
    m_pDuplicateSection = new QAction(tr("Duplicate section"),               this);
    m_pDeleteSection    = new QAction(tr("Delete section"),                  this);
    m_pResetSection     = new QAction(tr("Reset section"),                   this);
    m_pCopyAction       = new QAction(tr("Copy"),                            this);
    m_pCopyAction->setShortcut(Qt::Key_Copy);
    m_pPasteAction      = new QAction(tr("Paste"), this);
    m_pPasteAction->setShortcut(Qt::Key_Paste);

    m_pResetMesh            = new QAction(tr("Reset mesh"),  this);
    m_pTranslateWing        = new QAction(tr("Translate"),   this);
    m_pScaleWing            = new QAction(tr("Scale"),       this);
    m_pInertia              = new QAction(tr("Inertia"),     this);
    m_pExportToXml          = new QAction(tr("to XML file"), this);
    m_pExportToCADFile      = new QAction(tr("to CAD file"), this);
    m_pExportToStl          = new QAction(tr("to STL file"), this);

    makeCommonWts();

    makeWingTable();
    setupLayout();
    connectSignals();

    m_pTableContextMenu = new QMenu(tr("Section"),this);
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
    m_pcptSections->setWindowTitle(tr("Wing definition"));
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
                    m_pchTwoSided   = new QCheckBox(tr("Two-sided"));
                    m_pchsymmetric  = new QCheckBox(tr("Symmetric"));
                    m_prbRightSide  = new QRadioButton(tr("Right side"));
                    m_prbLeftSide   = new QRadioButton(tr("Left side"));

                    m_pchCloseInnerSide = new QCheckBox(tr("Close inner side"));

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
                    QLabel *plabWingDescription = new QLabel(tr("Description:"));

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
                m_pieTipStrips->setToolTip(tr("<p>Number of horizontal panel strips at the wing tips.<br>"
                                           "Recommendation: 1</p>"));
                pTipStripLayout->addWidget(new QLabel(tr("Tip strips")));
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
    m_iSection = 0;

    m_pWing = pWing;
    if(!m_pWing) return;

    m_pleWingName->setText(QString::fromStdString(m_pWing->name()));
    m_ppteDescription->setPlainText(QString::fromStdString(m_pWing->description()));
    m_pglWingView->setWing(m_pWing);
    computeGeometry();

    //    m_pcmbWingColor->setColor(m_pWing->color());
    m_pcbColor->setColor(xfl::fromfl5Clr(m_pWing->color()));

    setControls();


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
    XflDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);

    m_bChanged = false;

    resizeEvent(nullptr);
    m_pglWingView->setFlags(s_bOutline, s_bSurfaces, s_bVLMPanels, s_bAxes, s_bShowMasses, s_bFoilNames, false, false, false);
    m_pglControls->setControls();
    m_pglWingView->resetglWing();

    m_pglWingView->restoreViewPoint(s_ab_quat);
    m_pglWingView->reset3dScale();


    m_pglWingView->update();

    m_pButtonBox->setFocus();

    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    if(s_VSplitterSizes.length()>0) m_pVSplitter->restoreState(s_VSplitterSizes);
}


void WingDefDlg::hideEvent(QHideEvent *pEvent)
{
    XflDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();

    s_bOutline    = m_pglWingView->bOutline();
    s_bSurfaces   = m_pglWingView->bSurfaces();
    s_bVLMPanels  = m_pglWingView->bVLMPanels();
    s_bAxes       = m_pglWingView->bAxes();
    s_bShowMasses = m_pglWingView->bMasses();
    s_bFoilNames  = m_pglWingView->bFoilNames();

    m_pglWingView->saveViewPoint(s_ab_quat);

    s_HSplitterSizes  = m_pHSplitter->saveState();
    s_VSplitterSizes  = m_pVSplitter->saveState();
}


void WingDefDlg::resizeEvent(QResizeEvent *)
{
    if(m_pWing)    m_pglWingView->setReferenceLength(m_pWing->planformSpan());
    //    m_pglWingView->reset3dScale();

    int n = m_pSectionModel->actionColumn();
    QHeaderView *pHHeader = m_pcptSections->horizontalHeader();
    //pHHeader->setDefaultSectionSize(1);
    pHHeader->setSectionResizeMode(n, QHeaderView::Stretch);
    pHHeader->resizeSection(n, 1);

    double w = double(m_pcptSections->width())/100.0;
    int wFoil  = int(17.0*w);
    int wCols  = int(7.5*w);

    m_pcptSections->setColumnWidth( 0, wCols);
    m_pcptSections->setColumnWidth( 1, wCols);
    m_pcptSections->setColumnWidth( 2, wCols);
    m_pcptSections->setColumnWidth( 3, wCols);
    m_pcptSections->setColumnWidth( 4, wCols);
    m_pcptSections->setColumnWidth( 5, wFoil);
    m_pcptSections->setColumnWidth( 6, wCols);
    m_pcptSections->setColumnWidth( 7, wCols);
    m_pcptSections->setColumnWidth( 8, wCols);
    m_pcptSections->setColumnWidth( 9, wCols);
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
        m_pWing->setSymmetric(true);
        m_bRightSide          = true;
        for(int i=0; i<m_pWing->nSections(); i++)
        {
            m_pWing->setLeftFoilName(i, m_pWing->rightFoilName(i));
        }
    }
    else
    {
        m_pWing->setSymmetric(false);
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
        m_pWing->appendSection();
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

    m_pWing->setName(m_pleWingName->text().toStdString());
    QString strange = m_ppteDescription->toPlainText();
    m_pWing->setDescription(strange.toStdString());

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


void WingDefDlg::makeCommonWts()
{
    m_pglWingView = new gl3dWingView(this);
    m_pglWingView->showPartFrame(false);

    m_pglControls = new gl3dGeomControls(m_pglWingView, WingLayout, false);

    m_plabPlaneName    = new QLabel;
    m_plabPlaneName->setStyleSheet("font: bold");

    m_pleWingName      = new QLineEdit("Wing name");
    m_pcbColor         = new ColorBtn;

    m_ppteDescription = new QPlainTextEdit;
    m_ppteDescription->setToolTip(tr("Enter here a short description for the wing"));
    QFont font;
    QFontMetrics fm(font);
    m_ppteDescription->setMaximumHeight(fm.height()*5);

    m_pButtonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Discard);
    {
        m_ppbActionMenuButton = new QPushButton(tr("Actions"));
        {
            QMenu *pWingMenu = new QMenu(tr("Actions"), this);
            pWingMenu->addAction(m_pResetMesh);
            pWingMenu->addSeparator();
            QMenu *pExportMenu = pWingMenu->addMenu(tr("Export"));
            pExportMenu->addAction(m_pExportToCADFile);
            pExportMenu->addAction(m_pExportToStl);
            pExportMenu->addAction(m_pExportToXml);
            pWingMenu->addSeparator();
            pWingMenu->addAction(m_pInertia);
            //            pWingMenu->addAction(m_pTranslateWing);
            pWingMenu->addAction(m_pScaleWing);
            m_ppbActionMenuButton->setMenu(pWingMenu);
        }
        m_ppbSaveAsNew = new QPushButton(tr("Save as"));
        m_pButtonBox->addButton(m_ppbActionMenuButton, QDialogButtonBox::ActionRole);
        m_pButtonBox->addButton(m_ppbSaveAsNew,        QDialogButtonBox::ActionRole);
    }
}


void WingDefDlg::onSurfaceColor()
{
    QColor clr = QColorDialog::getColor(xfl::fromfl5Clr(m_pWing->color()), this, tr("Surface colour"), QColorDialog::ShowAlphaChannel);

    if(clr.isValid())
    {
        m_pWing->setColor(xfl::tofl5Clr(clr));
        m_pcbColor->setColor(clr);
        m_bDescriptionChanged = true;
    }
    m_pglWingView->resetglWing();
    m_pglWingView->update();
}


void WingDefDlg::onWingColor(QColor clr)
{
    if(!m_pWing) return;

    if(clr.isValid())
    {
        m_pWing->setColor(xfl::tofl5Clr(clr));
        m_bDescriptionChanged = true;
    }

    m_pglWingView->resetglWing();
    m_pglWingView->update();
}



void WingDefDlg::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QRect r = m_pglWingView->geometry();

    QPoint pt = pEvent->pos();
    if(r.contains(pt))
    {
        QMenu *pContextMenu = new QMenu("GraphMenu");
        QMenu *pSectionMenu = pContextMenu->addMenu(tr("Selected section"));
        {
            pSectionMenu->addAction(m_pInsertBefore);
            pSectionMenu->addAction(m_pInsertAfter);
            pSectionMenu->addAction(m_pDuplicateSection);
            pSectionMenu->addAction(m_pDeleteSection);
            pSectionMenu->addAction(m_pResetSection);
        }
        pSectionMenu->setEnabled(m_iSection>=0);
        pContextMenu->addSeparator();
        QMenu *pWingMenu = pContextMenu->addMenu(tr("Wing"));
        {
            pWingMenu->addAction(m_pScaleWing);
            pWingMenu->addAction(m_pInertia);
            pWingMenu->addSeparator();
            pWingMenu->addAction(m_pExportToXml);
            pWingMenu->addAction(m_pExportToCADFile);
            pWingMenu->addAction(m_pExportToStl);
        }
        pContextMenu->exec(QCursor::pos());
    }
    update();
    pEvent->accept();
}


bool WingDefDlg::checkWing()
{
    if(!m_pWing->name().length())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Please enter a name for the wing"));
        return false;
    }

    for (int k=1; k<m_pWing->nSections(); k++)
    {
        if(m_pWing->yPosition(k) + LENGTHPRECISION < m_pWing->yPosition(k-1))
        {
            QMessageBox::warning(this, tr("Warning"), tr("<p>The panel sequence is inconsistent. "
                                                         "The sections should be ordered from root to tip.</p>"));
            return false;
        }
    }

    for (int k=0; k<m_pWing->nSections(); k++)
    {
        if(fabs(m_pWing->chord(k))<LENGTHPRECISION)
        {
            QMessageBox::warning(this, tr("Warning"), tr("Zero length chords will cause a division by zero and should be avoided."));
            return false;
        }
    }

    for (int k=1; k<m_pWing->nSections(); k++)
    {
        Foil const*pLeftFoil = Objects2d::foil(m_pWing->m_Section.at(k).m_LeftFoilName);
        Foil const*pRightFoil = Objects2d::foil(m_pWing->m_Section.at(k).m_RightFoilName);
        if(pLeftFoil)
        {
            if((pLeftFoil->TEXHinge()>=0.99&& pLeftFoil->hasTEFlap()) ||(pLeftFoil->LEXHinge()<0.01&&pLeftFoil->hasLEFlap()))
            {
                QMessageBox::warning(this, tr("Warning"), QString::fromStdString(pLeftFoil->name())+": " +tr("Zero length flaps will cause a division by zero and should be avoided."));
                return false;
            }
        }
        if(pRightFoil)
        {
            if((pRightFoil->TEXHinge()>=0.99&& pRightFoil->hasTEFlap()) ||(pRightFoil->LEXHinge()<0.01&&pRightFoil->hasLEFlap()))
            {
                QMessageBox::warning(this, tr("Warning"), QString::fromStdString(pRightFoil->name())+": "+tr("Zero length flaps will cause a division by zero and should be avoided."));
                return false;
            }
        }
    }

    /*    int NYPanels = 0;
    for(int j=0; j<m_pWing->nSections()-1; j++)
    {
        NYPanels += m_pWing->nYPanels(j);
    }*/

    return true;
}


void WingDefDlg::computeGeometry()
{
    m_pWing->computeGeometry();
    m_pWing->createSurfaces(Vector3d(0.0,0.0,0.0), 0.0, 0.0);
}


void WingDefDlg::connectWingSignals()
{
    connect(m_pleWingName,           SIGNAL(editingFinished()), SLOT(onMetaDataChanged()));
    connect(m_ppteDescription,      SIGNAL(textChanged()),     SLOT(onMetaDataChanged()));
    //    connect(m_pcmbWingColor,         SIGNAL(clickedCB(QColor)), SLOT(onWingColor(QColor)));
    connect(m_pcbColor,              SIGNAL(clicked()),         SLOT(onSurfaceColor()));

    connect(m_pInsertBefore,         SIGNAL(triggered()), SLOT(onInsertNBefore()));
    connect(m_pInsertAfter,          SIGNAL(triggered()), SLOT(onInsertNAfter()));
    connect(m_pInsertNBefore,        SIGNAL(triggered()), SLOT(onInsertNBefore()));
    connect(m_pInsertNAfter,         SIGNAL(triggered()), SLOT(onInsertNAfter()));
    connect(m_pDuplicateSection,     SIGNAL(triggered()), SLOT(onDuplicateSection()));
    connect(m_pDeleteSection,        SIGNAL(triggered()), SLOT(onDeleteSection()));
    connect(m_pResetSection,         SIGNAL(triggered()), SLOT(onResetSection()));

    connect(m_pCopyAction,           SIGNAL(triggered(bool)), SLOT(onCopy()));
    connect(m_pPasteAction,          SIGNAL(triggered(bool)), SLOT(onPaste()));

    connect(m_pInertia,              SIGNAL(triggered()), SLOT(onInertia()));
    connect(m_pTranslateWing,        SIGNAL(triggered()), SLOT(onTranslateWing()));
    connect(m_pScaleWing,            SIGNAL(triggered()), SLOT(onScaleWing()));
    connect(m_pExportToXml,          SIGNAL(triggered()), SLOT(onExportWingToXML()));
    connect(m_pExportToCADFile,      SIGNAL(triggered()), SLOT(onExportWingToCADFile()));
    connect(m_pExportToStl,          SIGNAL(triggered()), SLOT(onExportWingToStlFile()));

    connect(m_pglWingView,           SIGNAL(pickedNodePair(QPair<int,int>)), SLOT(onPickedNodePair(QPair<int,int>)));
    connect(m_pglControls->m_ptbDistance, SIGNAL(clicked()), SLOT(onNodeDistance()));
}


void WingDefDlg::keyPressEvent(QKeyEvent *pEvent)
{
    /*    bool bShift = false;
    if(event->modifiers() & Qt::ShiftModifier)   bShift =true;*/
    bool bCtrl  = false;
    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;

    switch (pEvent->key())
    {
    case Qt::Key_F12:
    {
        onInertia();
        pEvent->accept();
        return;
    }
    case Qt::Key_Escape:
    {
        if(m_pglControls->getDistance())
        {
            m_pglControls->stopDistance();
            m_pglWingView->stopPicking();
            m_pglWingView->setPicking(xfl::NOPICK);
            m_pglWingView->clearMeasure();
            return;
        }
        break;
    }
    case Qt::Key_Delete:
    {
        onDeleteSection();
        pEvent->accept();
        return;
    }
    case Qt::Key_R:
    {
        m_pglWingView->on3dReset();
        return;
    }
    case Qt::Key_S:
    {
        if(bCtrl)
            onOK();
        break;
    }
    case Qt::Key_T:
    {
        if(bCtrl && m_pglWingView)
        {
            m_pglWingView->showTriangleOutline(!m_pglWingView->bTriangleOutline());
            m_pglWingView->update();
        }
        break;
    }
    case Qt::Key_H:
    {
        m_pglWingView->toggleHighlighting();
        return;
    }

    default:
        break;
    }
    XflDialog::keyPressEvent(pEvent);
}


void WingDefDlg::onMetaDataChanged()
{
    m_bDescriptionChanged=true;

    m_pWing->setName(m_pleWingName->text().toStdString());
    m_pWing->setDescription(m_ppteDescription->toPlainText().toStdString());
}


void WingDefDlg::onDeleteSection()
{
    if(m_iSection <0 || m_iSection>m_pWing->nSections()) return;
    if(m_iSection==0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("The first section cannot be deleted"));
        return;
    }

    int size = m_pWing->nSections();
    if(size<=2)
    {
        QMessageBox::warning(this, tr("Warning"), tr("The number of sections cannot be less than two"));
        return;
    }
    int ny = m_pWing->nYPanels(m_iSection-1) + m_pWing->nYPanels(m_iSection);

    m_pWing->removeSection(m_iSection);
    m_pWing->setNYPanels(m_iSection-1, ny);

    updateData();

    updateWingOutput();
}


void WingDefDlg::onInertia()
{
    WingXfl wing(*m_pWing);

    PartInertiaDlg dlg(&wing, nullptr, this);
    if(dlg.exec() == QDialog::Accepted)
    {
        m_pWing->copyInertia(wing);
        m_bChanged = true;
        m_pglWingView->update();
    }
}


void WingDefDlg::updateWingOutput()
{
    computeGeometry();
    setWingProps();

    m_bChanged = true;
    m_pglWingView->resetglSectionHighlight(m_iSection, m_bRightSide);
    m_pglWingView->resetglWing();

    m_pglWingView->update();
}


void WingDefDlg::onInsertNBefore()
{
    if(m_iSection>m_pWing->nSections()) return;

    if(m_iSection<=0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Insertion not possible before the first section"));
        return;
    }

    QAction *pAction = qobject_cast<QAction *>(sender());
    int nsec = pAction->data().toInt();

    if(nsec<0)
    {
        IntValueDlg dlg(this);
        dlg.setValue(1);
        dlg.setLeftLabel(tr("Number of sections to insert:"));
        if(dlg.exec()!=QDialog::Accepted) return;
        nsec = dlg.value();
        if(nsec<=0) return;
    }
    insertNSectionsAfter(m_iSection-1, nsec);

    setCurrentSection(m_iSection);

    updateData();
    updateWingOutput();
}


void WingDefDlg::onInsertNAfter()
{
    if(m_iSection <0 || m_iSection>=m_pWing->nSections()) return;

    QAction *pAction = qobject_cast<QAction *>(sender());
    int nsec = pAction->data().toInt();

    if(nsec<0)
    {
        IntValueDlg dlg(this);
        dlg.setValue(1);
        dlg.setLeftLabel(tr("Number of sections to insert:"));
        if(dlg.exec()!=QDialog::Accepted) return;
        nsec = dlg.value();
        if(nsec<=0) return;
    }

    // address the case of the last section
    if(m_iSection==m_pWing->nSections()-1)
    {
        WingSection sec = m_pWing->section(m_iSection);
        m_pWing->appendSection();
        m_pWing->m_Section.back() = sec;
        m_pWing->m_Section.back().setYPosition(1.5*sec.yPosition());
        m_pWing->m_Section.back().setNY(sec.nYPanels()/2);
        m_pWing->section(m_iSection).setNY(sec.nYPanels()/2);

        if(nsec==1)
        {
            for(int i=1; i<m_pWing->nSections(); i++) m_pWing->setXPanelDist(i, m_pWing->xPanelDist(0));
            updateData();
            updateWingOutput();
            return;
        }
        nsec--;
    }

    insertNSectionsAfter(m_iSection, nsec);

    updateData();

    updateWingOutput();
}


void WingDefDlg::insertNSectionsAfter(int n0, int nsec)
{
    int ny =  int (double(m_pWing->nYPanels(n0))*1.0/double(nsec+1));
    ny = std::max(1,ny);
    m_pWing->section(n0).setNY(ny);

    WingSection s0 = m_pWing->section(n0);// avoid taking moving references
    WingSection s1 = m_pWing->section(n0+1);
    for(int isec=0; isec<nsec; isec++)
    {
        double tau = double(isec+1)/double(nsec+1);
        m_pWing->insertSection(n0+isec+1);
        WingSection &sec = m_pWing->section(n0+isec+1);
        sec.setYPosition(    s0.yPosition() *(1.0-tau) + s1.yPosition()*tau);
        sec.setChord(        s0.chord()     *(1.0-tau) + s1.chord()    *tau);
        sec.setXOffset(       s0.offset()    *(1.0-tau) + s1.offset()   *tau);
        sec.setTwist(        s0.twist()     *(1.0-tau) + s1.twist()    *tau);
        sec.setDihedral(     s0.dihedral()  *(1.0-tau) + s1.dihedral() *tau);
        sec.setXDistType(    s0.xDistType());
        sec.setNX(           s0.nXPanels());
        sec.setYDistType(    xfl::UNIFORM);
        sec.setNY(           ny);
        sec.setRightFoilName(s0.rightFoilName());
        sec.setLeftFoilName( s0.leftFoilName());
    }

    for(int i=1; i<m_pWing->nSections(); i++) m_pWing->setXPanelDist(i, m_pWing->xPanelDist(0));
}


void WingDefDlg::onDuplicateSection()
{
    if(m_iSection <0 || m_iSection>=m_pWing->nSections()) return;

    int n = m_iSection;

    m_pWing->insertSection(m_iSection+1);

    m_pWing->setYPosition(n+1, m_pWing->yPosition(n));
    m_pWing->setChord(n+1, m_pWing->chord(n));
    m_pWing->setOffset(n+1, m_pWing->offset(n));
    m_pWing->setTwist(n+1, m_pWing->twist(n));
    m_pWing->setDihedral(n+1, m_pWing->dihedral(n));
    m_pWing->setNXPanels(n+1, m_pWing->nXPanels(n));
    m_pWing->setNYPanels(n+1, m_pWing->nYPanels(n));
    m_pWing->setXPanelDist(n+1, m_pWing->xPanelDist(n));
    m_pWing->setYPanelDist(n+1, m_pWing->yPanelDist(n));
    m_pWing->setRightFoilName(n+1, m_pWing->rightFoilName(n));
    m_pWing->setLeftFoilName(n+1, m_pWing->leftFoilName(n));

    int ny = m_pWing->nYPanels(n);
    m_pWing->setNYPanels(n+1, ny);
    m_pWing->setNYPanels(n, 1);

    //    m_pWing->m_bVLMAutoMesh = true;

    int newsection = m_iSection+1;
    m_iSection = newsection;
    setCurrentSection(m_iSection);

    updateData();

    updateWingOutput();
}


void WingDefDlg::onResetSection()
{
    int n = m_iSection;

    if((0 < n) && (n < (m_pWing->nSections()-1)))
    {
        double ratio = (m_pWing->yPosition(n) - m_pWing->yPosition(n - 1)) / (m_pWing->yPosition(n + 1) - m_pWing->yPosition(n - 1));

        m_pWing->setChord( n, m_pWing->chord( n-1) + ratio * (m_pWing->chord( n+1) - m_pWing->chord(n-1)));
        m_pWing->setOffset(n, m_pWing->offset(n-1) + ratio * (m_pWing->offset(n+1) - m_pWing->offset(n-1)));
        m_pWing->setTwist( n, m_pWing->twist( n-1) + ratio * (m_pWing->twist( n+1) - m_pWing->twist(n-1)));

        updateData();

        updateWingOutput();
    }
}


void WingDefDlg::onOK()
{
    readParams();

    if(!checkWing()) return;

    if(m_pWing->m_bSymmetric)
    {
        for (int i=0; i<m_pWing->nSections(); i++)
        {
            m_pWing->setLeftFoilName(i, m_pWing->rightFoilName(i));
        }
    }

    m_pWing->computeGeometry();
    m_pWing->computeStructuralInertia(Vector3d());

    accept();
}


void WingDefDlg::onResetMesh()
{
    VLMSetAutoMesh();
    updateData();

    updateWingOutput();
}


void WingDefDlg::onTranslateWing()
{
    if(!m_pWing) return;
    TranslateDlg dlg(this);
    if(dlg.exec()!=QDialog::Accepted) return;

    for(int i=0; i<m_pWing->nSections(); i++)
    {
        m_pWing->pSection(i)->m_Offset += dlg.translationVector().x;
    }

    updateData();
    updateWingOutput();
}


void WingDefDlg::onScaleWing()
{
    WingScaleDlg dlg(this);
    dlg.initDialog(m_pWing->m_PlanformSpan,
                   m_pWing->chord(0),
                   m_pWing->averageSweep(),
                   m_pWing->twist(m_pWing->nSections()-1),
                   m_pWing->planformArea(),
                   m_pWing->aspectRatio(),
                   m_pWing->taperRatio());

    if(QDialog::Accepted == dlg.exec())
    {
        if (dlg.m_bSpan || dlg.m_bChord || dlg.m_bSweep || dlg.m_bTwist || dlg.m_bArea || dlg.m_bAR)
        {
            if(dlg.m_bSpan)  m_pWing->scaleSpan(dlg.m_NewSpan);
            if(dlg.m_bChord) m_pWing->scaleChord(dlg.m_NewChord);
            if(dlg.m_bSweep) m_pWing->scaleSweep(dlg.m_NewSweep);
            if(dlg.m_bTwist) m_pWing->scaleTwist(dlg.m_NewTwist);
            if(dlg.m_bArea) m_pWing->scaleArea(dlg.m_NewArea);
            if(dlg.m_bAR) m_pWing->scaleAR(dlg.m_NewAR);
            if(dlg.m_bTR) m_pWing->scaleTR(dlg.m_NewTR);
        }

        updateData();

        updateWingOutput();
    }
}


void WingDefDlg::accept()
{
    done(QDialog::Accepted);
}


void WingDefDlg::reject()
{
    if(m_bChanged && xfl::bConfirmDiscard())
    {
        QString strong = tr("Discard the changes?");
        int Ans = QMessageBox::question(this, tr("Question"), strong,
                                        QMessageBox::Yes | QMessageBox::Cancel);
        if (QMessageBox::Yes == Ans)
        {
            done(QDialog::Rejected);
            return;
        }
        else return;
    }

    done(QDialog::Rejected);
}


void WingDefDlg::setWingProps()
{
    if(!m_pWing) return;
    //Updates the wing's properties after a change of geometry
    std::string wingdata;
    m_pWing->getProperties(wingdata, "");

    m_pglWingView->setBotLeftOutput(wingdata);
}


void WingDefDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)     onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
    else if (m_ppbSaveAsNew==pButton)                                     done(10);
}


void WingDefDlg::onSplitterMoved()
{
    resizeEvent(nullptr);
}


int WingDefDlg::VLMGetPanelTotal()
{
    double MinPanelSize=0;
    if(WingXfl::minSurfaceLength()>0.0) MinPanelSize = WingXfl::minSurfaceLength();
    else                             MinPanelSize = m_pWing->m_PlanformSpan/1000.0;

    int total = 0;
    for (int i=0; i<m_pWing->nSections()-1; i++)
    {
        //do not create a surface if its length is less than the critical size
        //            if(qAbs(m_pWing->TPos[j]-m_pWing->TPos(j+1))/m_pWing->m_Span >0.001){
        if (qAbs(m_pWing->yPosition(i)-m_pWing->yPosition(i+1)) > MinPanelSize)
            total +=m_pWing->nXPanels(i)*m_pWing->nYPanels(i);
    }
    //    if(!m_bMiddle) total *=2;
    if(!m_pWing->isFin()) return total*2;
    else                  return total;
}


bool WingDefDlg::VLMSetAutoMesh(int total)
{
    m_bChanged = true;
    //split (NYTotal) panels on each side proportionnaly to length, and space evenly
    //Set VLMMATSIZE/NYTotal panels along chord
    int NYTotal, size;

    if(!total)
    {
        size = int(2000/4);//why not? Too much refinement isn't worthwile
        NYTotal = 22;
    }
    else
    {
        size = total;
        NYTotal = int(sqrt(float(size)));
    }

    NYTotal *= 2;

    //    double d1, d2; //spanwise panel densities at i and i+1

    for (int i=0; i<m_pWing->nSections()-1;i++)
    {
        //        d1 = 5./2./m_pWing->m_Span/m_pWing->m_Span/m_pWing->m_Span *8. * pow(m_pWing->TPos[i],  3) + 0.5;
        //        d2 = 5./2./m_pWing->m_Span/m_pWing->m_Span/m_pWing->m_Span *8. * pow(m_pWing->TPos(i+1),3) + 0.5;
        //        m_pWing->NYPanels(i) = (int) (NYTotal * (0.8*d1+0.2*d2)* (m_pWing->TPos(i+1)-m_pWing->TPos(i))/m_pWing->m_Span);

        m_pWing->setNYPanels(i, int(qAbs(m_pWing->yPosition(i+1) - m_pWing->yPosition(i))* double(NYTotal)/m_pWing->m_PlanformSpan));

        m_pWing->setNXPanels(i, int(size/NYTotal));

        if(m_pWing->nYPanels(i)==0) m_pWing->setNYPanels(i, 1);
        if(m_pWing->nXPanels(i)==0) m_pWing->setNXPanels(i, 1);
    }

    return true;
}


void WingDefDlg::onExportWingToCADFile()
{
    std::string logmsg;
    TopoDS_Shape wingshape;
    occ::makeWingShape(m_pWing, 0.001, wingshape, logmsg);
    WingExportDlg dlg(this);
    dlg.init(m_pWing);
    dlg.exec();
}


void WingDefDlg::onExportWingToStlFile()
{
    if (!m_pWing) return;

    STLWriterDlg dlg(this);
    dlg.initDialog(nullptr, m_pWing, nullptr, nullptr);
    dlg.exec();
}


void WingDefDlg::onExportWingToXML()
{
    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = QString::fromStdString(m_pWing->name()).trimmed()+".xml";
    strong.replace(' ', '_');
    FileName = QFileDialog::getSaveFileName(this, "Export to xml file",
                                            SaveOptions::xmlPlaneDirName() +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;

    if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    XmlWingWriter wingwriter(XFile);
    wingwriter.writeXMLWing(*m_pWing, SaveOptions::bXmlWingFoils());

    XFile.close();
}


void WingDefDlg::onNodeDistance()
{
    m_pglWingView->setPicking(m_pglControls->getDistance() ? xfl::MESHNODE : xfl::NOPICK);
    if(!m_pglControls->getDistance()) m_pglWingView->clearMeasure();
    m_pglWingView->setSurfacePick(xfl::NOSURFACE);
    m_pglWingView->update();
}


void WingDefDlg::onPickedNodePair(QPair<int, int> nodepair)
{
    if(nodepair.first <0 || nodepair.first >=int(m_pglWingView->nodes().size())) return;
    if(nodepair.second<0 || nodepair.second>=int(m_pglWingView->nodes().size())) return;
    Node nsrc  = m_pglWingView->nodes().at(nodepair.first);
    Node ndest = m_pglWingView->nodes().at(nodepair.second);

    Segment3d seg(nsrc, ndest);
    m_pglWingView->setMeasure(seg);

    m_pglWingView->resetPickedNodes();
    m_pglWingView->update();
}




