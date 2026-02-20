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
#include <QColorDialog>

#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dwingview.h>
#include <core/saveoptions.h>
#include <api/units.h>
#include <core/xflcore.h>
#include <interfaces/editors/inertia/partinertiadlg.h>
#include <interfaces/editors/planeedit/planexfldlg.h>
#include <interfaces/editors/translatedlg.h>
#include <interfaces/editors/wingedit/wingdlg.h>
#include <interfaces/editors/wingedit/wingscaledlg.h>
#include <interfaces/editors/wingedit/wingsectiondelegate.h>
#include <interfaces/exchange/stlwriterdlg.h>
#include <interfaces/exchange/wingexportdlg.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <api/polar.h>
#include <api/objects3d.h>
#include <api/planexfl.h>
#include <api/surface.h>
#include <api/wingxfl.h>
#include <api/occ_globals.h>
#include <api/xmlwingwriter.h>
#include <interfaces/widgets/color/colorbtn.h>
#include <interfaces/widgets/customdlg/intvaluedlg.h>

#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>

QByteArray WingDlg::s_Geometry;

double WingDlg::s_MaxEdgeLength=0.5;
double WingDlg::s_MaxEdgeDeflection = 45; //degrees
double WingDlg::s_QualityBound = sqrt(2.0);
int WingDlg::s_MaxMeshIter = 1;

bool WingDlg::s_bAxes       = true;
bool WingDlg::s_bOutline    = true;
bool WingDlg::s_bSurfaces   = true;
bool WingDlg::s_bVLMPanels  = false;
bool WingDlg::s_bShowMasses = false;
bool WingDlg::s_bFoilNames  = false;

Quaternion WingDlg::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);


WingDlg::WingDlg(QWidget *pParent) : XflDialog(pParent)
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

    m_pBackImageLoad     = new QAction(tr("Load"),  this);
    m_pBackImageClear    = new QAction(tr("Clear"), this);
    m_pBackImageSettings = new QAction(tr("Settings"),  this);

    m_pResetMesh            = new QAction(tr("Reset mesh"),  this);
    m_pTranslateWing        = new QAction(tr("Translate"),   this);
    m_pScaleWing            = new QAction(tr("Scale"),       this);
    m_pInertia              = new QAction(tr("Inertia"),     this);
    m_pExportToXml          = new QAction(tr("to XML file"), this);
    m_pExportToCADFile      = new QAction(tr("to CAD file"), this);
    m_pExportToStl          = new QAction(tr("to STL file"), this);

    makeCommonWts();
}


void WingDlg::makeCommonWts()
{
    m_pglWingView = new gl3dWingView(this);
    m_pglWingView->showPartFrame(false);

    m_pglControls = new gl3dGeomControls(m_pglWingView, WingLayout, false);

    m_plabPlaneName    = new QLabel;
    m_plabPlaneName->setStyleSheet("font: bold");

    m_pleWingName      = new QLineEdit(tr("WingName"));
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


void WingDlg::onSurfaceColor()
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


void WingDlg::onWingColor(QColor clr)
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


void WingDlg::readParams()
{
    if(!m_pWing) return;
    m_pWing->setName(m_pleWingName->text().toStdString());
    QString strange = m_ppteDescription->toPlainText();
    m_pWing->setDescription(strange.toStdString());
}


void WingDlg::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QRect r = m_pglWingView->geometry();

    QPoint pt = pEvent->pos();
    if(r.contains(pt))
    {
        QMenu *pContextMenu = new QMenu(tr("GraphMenu"));
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
        pContextMenu->addSeparator();
        QMenu *pBackImageMenu = pContextMenu->addMenu(tr("Background image"));
        {
            pBackImageMenu->addAction(m_pBackImageLoad);
            pBackImageMenu->addAction(m_pBackImageClear);
            pBackImageMenu->addAction(m_pBackImageSettings);
        }
        pContextMenu->exec(QCursor::pos());
    }
    update();
    pEvent->accept();
}


WingDlg::~WingDlg()
{
}


bool WingDlg::checkWing()
{
    if(!m_pWing->name().length())
    {
        QMessageBox::warning(this, "Warning", "Please enter a name for the wing");
        return false;
    }

    for (int k=1; k<m_pWing->nSections(); k++)
    {
        if(m_pWing->yPosition(k) + LENGTHPRECISION < m_pWing->yPosition(k-1))
        {
            QMessageBox::warning(this, "Warning", "Warning : Panel sequence is inconsistent. "
                                                  "The sections should be ordered from root to tip");
            return false;
        }
    }

    for (int k=0; k<m_pWing->nSections(); k++)
    {
        if(fabs(m_pWing->chord(k))<LENGTHPRECISION)
        {
            QMessageBox::warning(this, "Warning", "Zero length chords will cause a division by zero and should be avoided.");
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
                QMessageBox::warning(this, "Warning", QString::fromStdString(pLeftFoil->name())+": Zero length flaps will cause a division by zero and should be avoided.");
                return false;
            }
        }
        if(pRightFoil)
        {
            if((pRightFoil->TEXHinge()>=0.99&& pRightFoil->hasTEFlap()) ||(pRightFoil->LEXHinge()<0.01&&pRightFoil->hasLEFlap()))
            {
                QMessageBox::warning(this, "Warning", QString::fromStdString(pRightFoil->name())+": Zero length flaps will cause a division by zero and should be avoided.");
                return false;
            }
        }
    }

    int NYPanels = 0;
    for(int j=0; j<m_pWing->nSections()-1; j++)
    {
        NYPanels += m_pWing->nYPanels(j);
    }

    return true;
}


void WingDlg::computeGeometry()
{
    m_pWing->computeGeometry();
    m_pWing->createSurfaces(Vector3d(0.0,0.0,0.0), 0.0, 0.0);
}


void WingDlg::connectWingSignals()
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

    connect(m_pBackImageLoad,        SIGNAL(triggered()), m_pglWingView, SLOT(onLoadBackImage()));
    connect(m_pBackImageClear,       SIGNAL(triggered()), m_pglWingView, SLOT(onClearBackImage()));
    connect(m_pBackImageSettings,    SIGNAL(triggered()), m_pglWingView, SLOT(onBackImageSettings()));
}


void WingDlg::initDialog(WingXfl *pWing)
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
}


void WingDlg::keyPressEvent(QKeyEvent *pEvent)
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


void WingDlg::onMetaDataChanged()
{
    m_bDescriptionChanged=true;

    m_pWing->setName(m_pleWingName->text().toStdString());
    m_pWing->setDescription(m_ppteDescription->toPlainText().toStdString());
}


void WingDlg::onDeleteSection()
{
    if(m_iSection <0 || m_iSection>m_pWing->nSections()) return;
    if(m_iSection==0)
    {
        QMessageBox::warning(this, "Warning","The first section cannot be deleted");
        return;
    }

    int size = m_pWing->nSections();
    if(size<=2)
    {
        QMessageBox::warning(this, "Warning","The number of sections cannot be less than two");
        return;
    }
    int ny = m_pWing->nYPanels(m_iSection-1) + m_pWing->nYPanels(m_iSection);

    m_pWing->removeWingSection(m_iSection);
    m_pWing->setNYPanels(m_iSection-1, ny);

    updateData();

    updateWingOutput();
}


void WingDlg::onInertia()
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


void WingDlg::updateWingOutput()
{
    computeGeometry();
    setWingProps();

    m_bChanged = true;
    m_pglWingView->resetglSectionHighlight(m_iSection, m_bRightSide);
    m_pglWingView->resetglWing();

    m_pglWingView->update();
}


void WingDlg::onInsertNBefore()
{
    if(m_iSection>m_pWing->nSections()) return;

    if(m_iSection<=0)
    {
        QMessageBox::warning(this, "Warning", "Insertion not possible before the first section");
        return;
    }

    QAction *pAction = qobject_cast<QAction *>(sender());
    int nsec = pAction->data().toInt();

    if(nsec<0)
    {
        IntValueDlg dlg(this);
        dlg.setValue(1);
        dlg.setLeftLabel("Number of sections to insert:");
        if(dlg.exec()!=QDialog::Accepted) return;
        nsec = dlg.value();
        if(nsec<=0) return;
    }
    insertNSectionsAfter(m_iSection-1, nsec);

    setCurrentSection(m_iSection);

    updateData();
    updateWingOutput();
}


void WingDlg::onInsertNAfter()
{
    if(m_iSection <0 || m_iSection>=m_pWing->nSections()) return;

    QAction *pAction = qobject_cast<QAction *>(sender());
    int nsec = pAction->data().toInt();

    if(nsec<0)
    {
        IntValueDlg dlg(this);
        dlg.setValue(1);
        dlg.setLeftLabel("Number of sections to insert:");
        if(dlg.exec()!=QDialog::Accepted) return;
        nsec = dlg.value();
        if(nsec<=0) return;
    }

    // address the case of the last section
    if(m_iSection==m_pWing->nSections()-1)
    {
        WingSection sec = m_pWing->section(m_iSection);
        m_pWing->appendWingSection();
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


void WingDlg::insertNSectionsAfter(int n0, int nsec)
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


void WingDlg::onDuplicateSection()
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


void WingDlg::onResetSection()
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


void WingDlg::onOK()
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


void WingDlg::onResetMesh()
{
    VLMSetAutoMesh();
    updateData();

    updateWingOutput();
}


void WingDlg::onTranslateWing()
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


void WingDlg::onScaleWing()
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


void WingDlg::accept()
{
    done(QDialog::Accepted);
}


void WingDlg::reject()
{
    if(m_bChanged && xfl::bConfirmDiscard())
    {
        QString strong = "Discard the changes?";
        int Ans = QMessageBox::question(this, "Question", strong,
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


void WingDlg::setWingProps()
{
    if(!m_pWing) return;
    //Updates the wing's properties after a change of geometry
    std::string wingdata;
    m_pWing->getProperties(wingdata, "");

    m_pglWingView->setBotLeftOutput(wingdata);
}


void WingDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)     onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
    else if (m_ppbSaveAsNew==pButton)                                     done(10);
}


void WingDlg::showEvent(QShowEvent *pEvent)
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
}


void WingDlg::hideEvent(QHideEvent *pEvent)
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
}


void WingDlg::resizeEvent(QResizeEvent *)
{
    if(m_pWing)    m_pglWingView->setReferenceLength(m_pWing->planformSpan());
//    m_pglWingView->reset3dScale();
}

void WingDlg::onSplitterMoved()
{
    resizeEvent(nullptr);
}


int WingDlg::VLMGetPanelTotal()
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


bool WingDlg::VLMSetAutoMesh(int total)
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



void WingDlg::setControls()
{
}


void WingDlg::onExportWingToCADFile()
{
    std::string logmsg;
    TopoDS_Shape wingshape;
    occ::makeWingShape(m_pWing, 0.001, wingshape, logmsg);
    WingExportDlg dlg(this);
    dlg.init(m_pWing);
    dlg.exec();
}


void WingDlg::onExportWingToStlFile()
{
    if (!m_pWing) return;

    STLWriterDlg dlg(this);
    dlg.initDialog(nullptr, m_pWing, nullptr, nullptr);
    dlg.exec();
}


void WingDlg::onExportWingToXML()
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


void WingDlg::onNodeDistance()
{
    m_pglWingView->setPicking(m_pglControls->getDistance() ? xfl::MESHNODE : xfl::NOPICK);
    if(!m_pglControls->getDistance()) m_pglWingView->clearMeasure();
    m_pglWingView->setSurfacePick(xfl::NOSURFACE);
    m_pglWingView->update();
}


void WingDlg::onPickedNodePair(QPair<int, int> nodepair)
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




