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
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMenu>
#include <QHeaderView>
#include <QColorDialog>
#include <QFileDialog>

#include "planestldlg.h"


#include <core/stlreaderdlg.h>
#include <core/xflcore.h>
#include <interfaces/editors/inertia/planestlinertiadlg.h>
#include <interfaces/graphs/graph/curve.h>
#include <interfaces/mesh/panelcheckdlg.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dplanestlview.h>
#include <interfaces/widgets/color/colorbtn.h>
#include <interfaces/widgets/customdlg/doublevaluedlg.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/customwts/formattextoutput.h>

#include <api/planestl.h>
#include <api/units.h>
#include <api/utils.h>

bool PlaneSTLDlg::s_bGuessOpposite = false;
QByteArray PlaneSTLDlg::s_VSplitterSizes;
QByteArray PlaneSTLDlg::s_STLGeometry;


PlaneSTLDlg::PlaneSTLDlg(QWidget *pParent) : PlaneDlg(pParent)
{
    setWindowTitle("STL plane editor");
    setupLayout();
    connectSignals();
}


void PlaneSTLDlg::setupLayout()
{
    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        QVBoxLayout * pDefLayout = new QVBoxLayout;
        {
            m_pVSplitter = new QSplitter(Qt::Vertical);
            {
                m_pVSplitter->setChildrenCollapsible(false);
                m_pLeftTabWt = new QTabWidget;
                {
                    QFrame *pMetaFrame = new QFrame;
                    {
                        QGridLayout *pMetaLayout = new QGridLayout;
                        {
                            QLabel *plabArea = new QLabel("Ref. area:");
                            QLabel *plabSpan = new QLabel("Ref. span:");
                            QLabel *plabChord = new QLabel("Ref. chord:");
                            m_pfeRefArea = new FloatEdit;
                            m_pfeRefSpan = new FloatEdit;
                            m_pfeRefChord = new FloatEdit;
                            QLabel *plabAreaUnit = new QLabel(Units::areaUnitQLabel());
                            QLabel *plabLen1 = new QLabel(Units::lengthUnitQLabel());
                            QLabel *plabLen2 = new QLabel(Units::lengthUnitQLabel());
                            m_pcbColor = new ColorBtn;

                            pMetaLayout->addWidget(m_pleName,         2,1,1,2);
                            pMetaLayout->addWidget(m_pcbColor,        2,3);
                            pMetaLayout->addWidget(m_pleDescription,  3,1,1,3);
                            pMetaLayout->addWidget(plabArea,          4,1,Qt::AlignVCenter |Qt::AlignRight);
                            pMetaLayout->addWidget(m_pfeRefArea,      4,2);
                            pMetaLayout->addWidget(plabAreaUnit,      4,3);
                            pMetaLayout->addWidget(plabChord,         5,1,Qt::AlignVCenter |Qt::AlignRight);
                            pMetaLayout->addWidget(m_pfeRefChord,     5,2);
                            pMetaLayout->addWidget(plabLen1,          5,3);
                            pMetaLayout->addWidget(plabSpan,          6,1,Qt::AlignVCenter |Qt::AlignRight);
                            pMetaLayout->addWidget(m_pfeRefSpan,      6,2);
                            pMetaLayout->addWidget(plabLen2,          6,3);
                        }
                        pMetaFrame->setLayout(pMetaLayout);
                    }

                    QFrame *pTransformBox = new QFrame;
                    {
                        QGridLayout *pTransformLayout = new QGridLayout;
                        {
                            QLabel *plabRotateUnit = new QLabel("<p>&deg;</p>");
                            QLabel *plabLength4 = new QLabel(Units::lengthUnitQLabel());

                            m_pfeRotate     = new FloatEdit(90.0);
                            m_pfeScale      = new FloatEdit(1.0);
                            m_pfeTranslate  = new FloatEdit(0.0);
                            m_ppbTranslateX = new QPushButton("Translate X");
                            m_ppbTranslateY = new QPushButton("Translate Y");
                            m_ppbTranslateZ = new QPushButton("Translate Z");
                            m_ppbRotateX    = new QPushButton("Rotate X");
                            m_ppbRotateY    = new QPushButton("Rotate Y");
                            m_ppbRotateZ    = new QPushButton("Rotate Z");
                            m_ppbScale      = new QPushButton("Scale");

                            pTransformLayout->addWidget(m_ppbRotateX,    2,1);
                            pTransformLayout->addWidget(m_ppbRotateY,    2,2);
                            pTransformLayout->addWidget(m_ppbRotateZ,    2,3);
                            pTransformLayout->addWidget(m_pfeRotate,     2,4);
                            pTransformLayout->addWidget(plabRotateUnit,  2,5);

                            pTransformLayout->addWidget(m_ppbTranslateX, 3,1);
                            pTransformLayout->addWidget(m_ppbTranslateY, 3,2);
                            pTransformLayout->addWidget(m_ppbTranslateZ, 3,3);
                            pTransformLayout->addWidget(m_pfeTranslate,  3,4);
                            pTransformLayout->addWidget(plabLength4,     3,5);

                            pTransformLayout->addWidget(m_ppbScale,      4,3);
                            pTransformLayout->addWidget(m_pfeScale,      4,4);

                            pTransformLayout->setRowStretch(             5,1);
                        }
                        pTransformBox->setLayout(pTransformLayout);
                    }

                    QFrame *pTEPage = new QFrame;
                    {
                        QVBoxLayout *pMeshLayout = new QVBoxLayout;
                        {
                            QFrame *pTEBox = new QFrame;
                            {
                                QGridLayout *pTEBoxLayout = new QGridLayout;
                                {
                                    QPushButton *ppbMesh = new QPushButton("Mesh actions");
                                    {
                                        QMenu *pMeshMenu = new QMenu("Mesh");
                                        {
                                            m_pConvertTriangles = new QAction("Rebuild mesh from STL triangles");
                                            pMeshMenu->addAction(m_pConvertTriangles);
                                            pMeshMenu->addSeparator();
                                            pMeshMenu->addAction(m_pCheckMesh);
                                            pMeshMenu->addAction(m_pConnectPanels);
                                            pMeshMenu->addAction(m_pCheckFreeEdges);
                                            pMeshMenu->addSeparator();
                                            pMeshMenu->addAction(m_pClearHighlighted);
                                            pMeshMenu->addSeparator();
                                            pMeshMenu->addAction(m_pMergeFuseToWingNodes);
                                            pMeshMenu->addSeparator();
                                            pMeshMenu->addAction(m_pCenterOnPanel);
                                        }
                                        ppbMesh->setMenu(pMeshMenu);
                                    }


                                    QLabel *plabMaxTE = new QLabel("Max. T.E. angle for guesses:");
                                    m_ppbGuessTE = new QPushButton("Guess T.E.");
                                    m_pfeTEAngle = new FloatEdit(s_TEMaxAngle);
                                    QLabel *plabDegree = new QLabel("<p>&deg;</p>");

                                    m_ppbTopTEPanels = new QPushButton("Top panels");
                                    m_ppbTopTEPanels->setCheckable(true);
                                    m_ppbBotTEPanels = new QPushButton("Bottom panels");
                                    m_ppbBotTEPanels->setCheckable(true);
                                    m_pchGuessOpposite = new QCheckBox("Guess opposite");
                                    m_pchGuessOpposite->setChecked(s_bGuessOpposite);

                                    m_ppbCheckTE = new QPushButton("Check T.E.");

                                    pTEBoxLayout->addWidget(ppbMesh,           1, 1);
                                    pTEBoxLayout->addWidget(plabMaxTE,         2, 1);
                                    pTEBoxLayout->addWidget(m_pfeTEAngle,      2, 2);
                                    pTEBoxLayout->addWidget(plabDegree,        2, 3);

                                    pTEBoxLayout->addWidget(new QLabel("Automatic detection:"),3,1);
                                    pTEBoxLayout->addWidget(m_ppbGuessTE,       3,2);

                                    pTEBoxLayout->addWidget(new QLabel("Manual selection:"), 4,1);
                                    pTEBoxLayout->addWidget(m_ppbTopTEPanels,   4,2);
                                    pTEBoxLayout->addWidget(m_pchGuessOpposite, 4,3);
                                    pTEBoxLayout->addWidget(m_ppbBotTEPanels,   5,2);

                                    pTEBoxLayout->addWidget(new QLabel("Verification:"), 6,1);
                                    pTEBoxLayout->addWidget(m_ppbCheckTE,       6,2);
                                }
                                pTEBox->setLayout(pTEBoxLayout);
                            }

                            QLabel *pFlow5Link = new QLabel;
                            pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Modelling/STL_Planes.html#Stepbystep>https://flow5.tech/docs/flow5_doc/Modelling/STL_Planes.html</a>");
                            pFlow5Link->setOpenExternalLinks(true);
                            pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
                            pFlow5Link->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

                            pMeshLayout->addWidget(pTEBox);
                            pMeshLayout->addStretch();
                            pMeshLayout->addWidget(pFlow5Link);
                        }
                        pTEPage->setLayout(pMeshLayout);
                    }

                    m_pLeftTabWt->addTab(pMetaFrame,    "Meta-data");
                    m_pLeftTabWt->addTab(pTransformBox, "Transformations");
                    m_pLeftTabWt->addTab(pTEPage,       "Mesh and trailing edges");
                }

                m_pVSplitter->addWidget(m_pLeftTabWt);
                m_pVSplitter->addWidget(m_ppto);
                m_pVSplitter->setStretchFactor(0, 1);
                m_pVSplitter->setStretchFactor(1, 3);
            }
            pDefLayout->addWidget(m_pVSplitter);
            pDefLayout->addWidget(m_pButtonBox);
        }

        QVBoxLayout *pFuseViewLayout = new QVBoxLayout;
        {
            m_pglPlaneView = new gl3dPlaneSTLView;
            m_pglPlaneView->showPartFrame(false);
            m_pglControls  = new gl3dGeomControls(m_pglPlaneView, PlaneLayout, true);

            pFuseViewLayout->addWidget(m_pglPlaneView);
            pFuseViewLayout->addWidget(m_pglControls);
        }

        pMainLayout->addLayout(pDefLayout);
        pMainLayout->addLayout(pFuseViewLayout);
        pMainLayout->setStretchFactor(pDefLayout,1);
        pMainLayout->setStretchFactor(pFuseViewLayout,5);
    }

    setLayout(pMainLayout);
}


void PlaneSTLDlg::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QMenu *pPlaneCtxMenu = new QMenu("context menu", this);
    {
        pPlaneCtxMenu->addAction(m_pPlaneInertia);
        pPlaneCtxMenu->addSeparator();
        QMenu *pMeshMenu = pPlaneCtxMenu->addMenu("Mesh");
        {
            pMeshMenu->addAction(m_pCheckMesh);
            pMeshMenu->addAction(m_pConnectPanels);
            pMeshMenu->addAction(m_pCheckFreeEdges);
            pMeshMenu->addAction(m_pClearHighlighted);
            pMeshMenu->addSeparator();
//            pMeshMenu->addAction(m_pCleanDoubleNode);
//            pMeshMenu->addSeparator();
            pMeshMenu->addAction(m_pCenterOnPanel);
        }
        pPlaneCtxMenu->addSeparator();

        QAction *m_pBackImageLoad = new QAction("Load", this);
        connect(m_pBackImageLoad, SIGNAL(triggered()), m_pglPlaneView, SLOT(onLoadBackImage()));

        QAction *m_pBackImageClear = new QAction("Clear", this);
        connect(m_pBackImageClear, SIGNAL(triggered()), m_pglPlaneView, SLOT(onClearBackImage()));

        QAction *m_pBackImageSettings = new QAction("Settings", this);
        connect(m_pBackImageSettings, SIGNAL(triggered()), m_pglPlaneView, SLOT(onBackImageSettings()));

        pPlaneCtxMenu->addSeparator();
        QMenu *pBackImageMenu = pPlaneCtxMenu->addMenu("Background image");
        {
            pBackImageMenu->addAction(m_pBackImageLoad);
            pBackImageMenu->addAction(m_pBackImageClear);
            pBackImageMenu->addAction(m_pBackImageSettings);
        }
    }
    pPlaneCtxMenu->exec(pEvent->globalPos());
}


void PlaneSTLDlg::keyPressEvent(QKeyEvent *pEvent)
{
/*    bool bShift = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;*/
    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;
/*    bool bAlt = false;
    if(pEvent->modifiers() & Qt::AltModifier)   bAlt =true;*/

    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            m_pButtonBox->button(QDialogButtonBox::Save)->setFocus();
            break;
        }
        case Qt::Key_Escape:
        {
            if(m_pglControls->getDistance())
            {
                m_pglControls->stopDistance();
                m_pglPlaneView->clearMeasure();
            }
            else  if(!deselectButtons())
                reject();
            break;
        }
        case Qt::Key_S:
        {
            if(bCtrl)
                accept();
            break;
        }
        default:
            PlaneDlg::keyPressEvent(pEvent);
            break;
    }
}


bool PlaneSTLDlg::deselectButtons()
{
    bool bChecked = false;
    bChecked |= m_ppbBotTEPanels->isChecked();
    bChecked |= m_ppbTopTEPanels->isChecked();

    m_ppbBotTEPanels->setChecked(false);
    m_ppbTopTEPanels->setChecked(false);
    gl3dPlaneSTLView *pglPlaneSTLView = dynamic_cast<gl3dPlaneSTLView*>(m_pglPlaneView);
    pglPlaneSTLView->selectPanels(false);

    if(!m_pglControls->isHighlighting())
        pglPlaneSTLView->stopPicking();
    return bChecked;
}


void PlaneSTLDlg::setControls()
{
    m_ppbScale->setEnabled(m_pPlaneSTL->triangleCount()>0);
    m_ppbTranslateX->setEnabled(m_pPlaneSTL->triangleCount()>0);
    m_ppbTranslateY->setEnabled(m_pPlaneSTL->triangleCount()>0);
    m_ppbTranslateZ->setEnabled(m_pPlaneSTL->triangleCount()>0);
    m_ppbRotateX->setEnabled(m_pPlaneSTL->triangleCount()>0);
    m_ppbRotateY->setEnabled(m_pPlaneSTL->triangleCount()>0);
    m_ppbRotateZ->setEnabled(m_pPlaneSTL->triangleCount()>0);
}


void PlaneSTLDlg::connectSignals()
{
    connectBaseSignals();

    connect(m_pfeRefArea,     SIGNAL(floatChanged(float)),    SLOT(onReferenceDims()));
    connect(m_pfeRefChord,    SIGNAL(floatChanged(float)),    SLOT(onReferenceDims()));
    connect(m_pfeRefSpan,     SIGNAL(floatChanged(float)),    SLOT(onReferenceDims()));

    connect(m_ppbTranslateX,  SIGNAL(clicked(bool)),     SLOT(onTranslate()));
    connect(m_ppbTranslateY,  SIGNAL(clicked(bool)),     SLOT(onTranslate()));
    connect(m_ppbTranslateZ,  SIGNAL(clicked(bool)),     SLOT(onTranslate()));
    connect(m_ppbRotateX,     SIGNAL(clicked(bool)),     SLOT(onRotate()));
    connect(m_ppbRotateY,     SIGNAL(clicked(bool)),     SLOT(onRotate()));
    connect(m_ppbRotateZ,     SIGNAL(clicked(bool)),     SLOT(onRotate()));
    connect(m_ppbScale,       SIGNAL(clicked(bool)),     SLOT(onScale()));

    connect(m_pConvertTriangles, SIGNAL(triggered()),  SLOT(onMakeMeshFromTriangles()));

    gl3dPlaneSTLView *pglPlaneSTLView = dynamic_cast<gl3dPlaneSTLView*>(m_pglPlaneView);

    connect(m_ppbGuessTE,         SIGNAL(clicked(bool)),      SLOT(onGuessTEPanels()));
    connect(m_ppbTopTEPanels,     SIGNAL(clicked(bool)),      SLOT(onTopTEPanels()));
    connect(m_ppbBotTEPanels,     SIGNAL(clicked(bool)),      SLOT(onBotTEPanels()));
    connect(pglPlaneSTLView,      SIGNAL(panelSelected(int)), SLOT(onPanelSelected(int)));

    connect(m_ppbCheckTE,         SIGNAL(clicked(bool)),      SLOT(onCheckTEPanels()));

    connect(pglPlaneSTLView,      SIGNAL(pickedNodePair(QPair<int,int>)), SLOT(onPickedNodePair(QPair<int,int>)));
    connect(m_pglControls->m_ptbDistance, SIGNAL(clicked()), SLOT(onNodeDistance()));

    connect(m_pcbColor,          SIGNAL(clicked()),          SLOT(onSetColor()));
}


void PlaneSTLDlg::initDialog(Plane *pPlane, bool bAcceptName)
{
    PlaneDlg::initDialog(pPlane, bAcceptName);

    m_pPlaneSTL = dynamic_cast<PlaneSTL*>(pPlane);

    m_pcbColor->setColor(xfl::fromfl5Clr(m_pPlaneSTL->surfaceColor()));

    gl3dPlaneSTLView *pglPlaneSTLView = dynamic_cast<gl3dPlaneSTLView*>(m_pglPlaneView);
    pglPlaneSTLView->setPlane(m_pPlaneSTL);
    pglPlaneSTLView->resetgl3dPlane();
    pglPlaneSTLView->setPlaneReferenceLength(m_pPlaneSTL);
    m_pglPlaneView->reset3dScale();
    m_pglPlaneView->update();

    m_pfeRefChord->setValue(m_pPlaneSTL->refChord()*Units::mtoUnit());
    m_pfeRefSpan->setValue(m_pPlaneSTL->refSpan()*Units::mtoUnit());
    m_pfeRefArea->setValue(m_pPlaneSTL->refArea()*Units::m2toUnit());

    onUpdatePlaneProps();
    setControls();
}


void PlaneSTLDlg::showEvent(QShowEvent *pEvent)
{
    PlaneDlg::showEvent(pEvent);
    restoreGeometry(s_STLGeometry);
    if(s_VSplitterSizes.length()>0) m_pVSplitter->restoreState(s_VSplitterSizes);

    if(!m_pPlaneSTL->isInitialized()) QTimer::singleShot(150, this, SLOT(onInitializePlane()));
}


void PlaneSTLDlg::hideEvent(QHideEvent *pEvent)
{
    PlaneDlg::hideEvent(pEvent);
    s_STLGeometry = saveGeometry();
    s_VSplitterSizes  = m_pVSplitter->saveState();

    s_TEMaxAngle = m_pfeTEAngle->value();
    s_bGuessOpposite = m_pchGuessOpposite->isChecked();
}


void PlaneSTLDlg::onInitializePlane()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    updateOutput("Making triangle nodes\n");
    m_pPlaneSTL->makeTriangleNodes();
    updateOutput("Making node normals\n");
    m_pPlaneSTL->makeNodeNormals();
    updateOutput("Making mesh from triangles\n");
    std::string log, prefix("   ");
    m_pPlaneSTL->m_RefTriMesh.makeMeshFromTriangles(m_pPlaneSTL->m_Triangulation.triangles(), 0, xfl::NOSURFACE, log, prefix);
    for(int i3=0; i3<m_pPlaneSTL->m_RefTriMesh.nPanels(); i3++) m_pPlaneSTL->m_RefTriMesh.panel(i3).setFromSTL(true);
    m_pPlaneSTL->restoreMesh();
    updateStdOutput(log);

    updateOutput("\nComputing surface properties:\n");
    m_pPlaneSTL->computeSurfaceProperties();
//    m_Triangulation.computeSurfaceProperties(m_Length, m_Span, m_Height, m_WettedArea);

    QString strange;
    strange  = QString::asprintf("   Length      = %13g ", m_pPlaneSTL->m_Length * Units::mtoUnit()) + Units::lengthUnitQLabel() + EOLch;
    strange += QString::asprintf("   Span        = %13g ", m_pPlaneSTL->m_Span   * Units::mtoUnit()) + Units::lengthUnitQLabel() + EOLch;
    strange += QString::asprintf("   Height      = %13g ", m_pPlaneSTL->m_Height * Units::mtoUnit()) + Units::lengthUnitQLabel() + EOLch;
    strange += QString::asprintf("   Wetted area = %13g ", m_pPlaneSTL->m_WettedArea * Units::m2toUnit()) + Units::areaUnitQLabel() + EOLch + EOLch;

    updateOutput(strange);

    m_pPlaneSTL->setLineWidth(Curve::defaultLineWidth());
    m_pPlaneSTL->setInitialized(true);

    m_pglPlaneView->setPlaneReferenceLength(m_pPlaneSTL);
    m_pglPlaneView->reset3dScale();
    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();

    onUpdatePlaneProps();

    QApplication::restoreOverrideCursor();
}


void PlaneSTLDlg::onOK(int iExitCode)
{
    double chord = m_pfeRefChord->value()/Units::mtoUnit();
    double span = m_pfeRefSpan->value()/Units::mtoUnit();
    double area = m_pfeRefArea->value()/Units::m2toUnit();

    if(fabs(chord)<1.0e-4 || fabs(span)<1.0e-4 || fabs(area)<1.e-6)
    {
        QMessageBox::warning(this, "Warning", "Reference dimensions must be non-zero.");
        return;
    }

    m_pPlaneSTL->setRefChord(chord);
    m_pPlaneSTL->setRefSpan(span);
    m_pPlaneSTL->setRefArea(area);

    done(iExitCode);
}


void PlaneSTLDlg::onPlaneInertia()
{
    PlaneStlInertiaDlg dlg(this);
    dlg.initDialog(m_pPlaneSTL);
    dlg.exec();
    m_pglPlaneView->update();
    m_bChanged = true;

    onUpdatePlaneProps();
}


void PlaneSTLDlg::onUpdatePlane()
{
    onUpdatePlaneProps();

    m_pglPlaneView->setPlaneReferenceLength(m_pPlane);

    gl3dPlaneSTLView* pglPlaneStlView = dynamic_cast<gl3dPlaneSTLView*>(m_pglPlaneView);
    pglPlaneStlView->resetgl3dPlane();
    m_pglPlaneView->update();
}


void PlaneSTLDlg::onScale()
{
    deselectButtons();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    double scalefactor = m_pfeScale->value();
    m_pPlaneSTL->scale(scalefactor);

    m_pPlaneSTL->computeSurfaceProperties();

    onUpdatePlane();

    m_bChanged = true;
    QApplication::restoreOverrideCursor();
}


void PlaneSTLDlg::onTranslate()
{
    deselectButtons();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    double t = m_pfeTranslate->value()/Units::mtoUnit();
    QPushButton *pButton = static_cast<QPushButton *>(sender());
    Vector3d trans;
    if     (pButton==m_ppbTranslateX) trans.set(t,0.0,0.0);
    else if(pButton==m_ppbTranslateY) trans.set(0.0,t,0.0);
    else if(pButton==m_ppbTranslateZ) trans.set(0.0,0.0,t);

    m_pPlaneSTL->translate(trans);
    m_pPlaneSTL->computeSurfaceProperties();

    onUpdatePlane();

    m_bChanged = true;
    QApplication::restoreOverrideCursor();
}


void PlaneSTLDlg::onRotate()
{
    deselectButtons();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    double r = m_pfeRotate->value();
    QPushButton *pButton = static_cast<QPushButton *>(sender());
    Vector3d O, axis;
    if      (pButton==m_ppbRotateX) axis.set(1.0,0.0,0.0);
    else if (pButton==m_ppbRotateY) axis.set(0.0,1.0,0.0);
    else if (pButton==m_ppbRotateZ) axis.set(0.0,0.0,1.0);
    m_pPlaneSTL->rotate(O, axis, r);

    m_pPlaneSTL->computeSurfaceProperties();

    onUpdatePlane();

    m_bChanged = true;
    QApplication::restoreOverrideCursor();
}


void PlaneSTLDlg::onFlipNormals()
{
    deselectButtons();
    if(!m_pPlaneSTL) return;

    m_pPlaneSTL->setReversed(!m_pPlaneSTL->reversed());
    m_pPlaneSTL->triangulation().flipNormals();

    gl3dPlaneSTLView *pglPlaneSTLView = dynamic_cast<gl3dPlaneSTLView*>(m_pglPlaneView);
    pglPlaneSTLView->resetgl3dPlane();
    m_pglPlaneView->update();
    m_bChanged = true;
}


void PlaneSTLDlg::onReferenceDims()
{
    m_pPlaneSTL->setRefArea(m_pfeRefArea->value()/Units::m2toUnit());
    m_pPlaneSTL->setRefChord(m_pfeRefChord->value()/Units::mtoUnit());
    m_pPlaneSTL->setRefSpan(m_pfeRefSpan->value()/Units::mtoUnit());

    onUpdatePlaneProps();
    m_bChanged = true;
}


/*
void PlaneSTLDlg::onSelectTENodes()
{
    if(m_ppbSelectTENodes->isChecked()) m_ppbUnSelectTENodes->setChecked(false);
    gl3dPlaneSTLView *pglPlaneSTLView = dynamic_cast<gl3dPlaneSTLView*>(m_pglPlaneView);
    pglPlaneSTLView->selectTENodes(m_ppbSelectTENodes->isChecked());
}


void PlaneSTLDlg::onUnSelectTENodes()
{
    if(m_ppbUnSelectTENodes->isChecked()) m_ppbSelectTENodes->setChecked(false);
    gl3dPlaneSTLView *pglPlaneSTLView = dynamic_cast<gl3dPlaneSTLView*>(m_pglPlaneView);
    pglPlaneSTLView->selectTENodes(m_ppbUnSelectTENodes->isChecked());
}*/


void PlaneSTLDlg::makeTESegments()
{
    QVector<Segment3d> tesegs;
    for(int i3=0; i3<m_pPlaneSTL->refTriMesh().nPanels(); i3++)
    {
        Panel3 &p3 = m_pPlaneSTL->panel3(i3);
        if(p3.isTrailing())
        {
            tesegs.append({p3.rightTrailingNode(), p3.leftTrailingNode()});
        }
    }
    m_pglPlaneView->setSegments(tesegs);
    m_pglPlaneView->update();
}


void PlaneSTLDlg::onTopTEPanels()
{
    m_pglPlaneView->stopPicking();
    gl3dPlaneSTLView *pglPlaneSTLView = dynamic_cast<gl3dPlaneSTLView*>(m_pglPlaneView);
    if(m_ppbTopTEPanels->isChecked())
    {
        m_ppbBotTEPanels->setChecked(false);
        pglPlaneSTLView->selectPanels(true);
        pglPlaneSTLView->setPicking(xfl::PANEL3);
    }
}


void PlaneSTLDlg::onBotTEPanels()
{
    m_pglPlaneView->stopPicking();
    gl3dPlaneSTLView *pglPlaneSTLView = dynamic_cast<gl3dPlaneSTLView*>(m_pglPlaneView);
    if(m_ppbBotTEPanels->isChecked())
    {
        m_ppbTopTEPanels->setChecked(false);
        pglPlaneSTLView->selectPanels(true);
        pglPlaneSTLView->setPicking(xfl::PANEL3);
    }
}


void PlaneSTLDlg::onPanelSelected(int i3)
{
    if(i3<0 || i3>=m_pPlaneSTL->refTriMesh().nPanels()) return;
    s_TEMaxAngle = m_pfeTEAngle->value();
    s_bGuessOpposite = m_pchGuessOpposite->isChecked();
    double ccrit = cos((180.0-s_TEMaxAngle)*PI/180.0);

    Panel3 &p3 = m_pPlaneSTL->refTriMesh().panel(i3);
    if(p3.neighbourCount()==0 && s_bGuessOpposite)
    {
        m_ppto->onAppendQText("Connect the panels before attempting to guess the T.E.\n");
    }

    if(m_ppbTopTEPanels->isChecked())
    {
        p3.setTrailing(!p3.isTrailing());
        if(p3.isTrailing()) p3.setSurfacePosition(xfl::TOPSURFACE);
        else                p3.setSurfacePosition(xfl::NOSURFACE);
        if(s_bGuessOpposite && p3.isTrailing())
        {
            for(int i3=0; i3<p3.neighbourCount(); i3++)
            {
                int idx = p3.neighbour(i3);
                if(idx>=0 && idx<m_pPlaneSTL->refTriMesh().nPanels())
                {
                    Panel3 &p3o = m_pPlaneSTL->refTriMesh().panel(idx);
                    double cos = p3.normal().dot(p3o.normal());
                    if(cos<ccrit)
                    {
                        p3o.setTrailing(true);
                        p3o.setSurfacePosition(xfl::BOTSURFACE);
                        p3.setOppositeIndex(p3o.index());
                        p3o.setOppositeIndex(p3.index());
                    }
                }
            }
        }

        m_pglPlaneView->resetgl3dMesh();
    }
    else if(m_ppbBotTEPanels->isChecked())
    {
        p3.setTrailing(!p3.isTrailing());
        if(p3.isTrailing()) p3.setSurfacePosition(xfl::BOTSURFACE);
        else                p3.setSurfacePosition(xfl::NOSURFACE);
        if(s_bGuessOpposite && p3.isTrailing())
        {
            for(int i3=0; i3<p3.neighbourCount(); i3++)
            {
                int idx = p3.neighbour(i3);
                if(idx>=0 && idx<m_pPlaneSTL->refTriMesh().nPanels())
                {
                    Panel3 &p3o = m_pPlaneSTL->refTriMesh().panel(idx);
                    double cos = p3.normal().dot(p3o.normal());
                    if(cos<ccrit)
                    {
                        p3o.setTrailing(true);
                        p3o.setSurfacePosition(xfl::TOPSURFACE);
                        p3.setOppositeIndex(p3o.index());
                        p3o.setOppositeIndex(p3.index());
                    }
                }
            }
        }
        m_pglPlaneView->resetgl3dMesh();
    }
    m_pglPlaneView->update();

    m_bChanged = true;
}


void PlaneSTLDlg::onGuessTEPanels()
{
    deselectButtons();

    if(m_pPlaneSTL->nPanel3()<=0) return;
    Panel3 &p3t = m_pPlaneSTL->refTriMesh().panel(0);
    if(p3t.neighbourCount()==0)
    {
        m_ppto->onAppendQText("Connect the panels before attempting to guess the T.E.\n");
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    s_TEMaxAngle = m_pfeTEAngle->value();
    double ccrit = cos((180.0-s_TEMaxAngle)*PI/180.0);

    int iTE=0;
    for(int i3=0; i3<m_pPlaneSTL->refTriMesh().nPanels(); i3++)
    {
        Panel3 &p3 = m_pPlaneSTL->refTriMesh().panel(i3);
        p3.setTrailing(false);
        p3.setSurfacePosition(xfl::NOSURFACE);
        p3.setOppositeIndex(-1);
    }

    double zcrit = 0.1;

    for(int i3t=0; i3t<m_pPlaneSTL->refTriMesh().nPanels(); i3t++)
    {
        Panel3 &p3t = m_pPlaneSTL->refTriMesh().panel(i3t);

        for(int i3b=0; i3b<p3t.neighbourCount(); i3b++)
        {
            int idx = p3t.neighbour(i3b);
            if(idx>=0 && idx<m_pPlaneSTL->refTriMesh().nPanels())
            {
                Panel3 &p3b = m_pPlaneSTL->refTriMesh().panel(idx);
                double cos = p3t.normal().dot(p3b.normal());
                if(p3b.oppositeIndex()<0 && cos<ccrit)
                {
                    // we have a TE
                    p3t.setTrailing(true);
                    p3t.setOppositeIndex(p3b.index());
                    p3b.setTrailing(true);
                    p3b.setOppositeIndex(p3t.index());

                    if(p3t.normal().z>zcrit && p3b.normal().z<-zcrit)
                    {
                        p3t.setSurfacePosition(xfl::TOPSURFACE);
                        p3b.setSurfacePosition(xfl::BOTSURFACE);
                    }
                    else if(p3b.normal().z>zcrit && p3t.normal().z<-zcrit)
                    {
                        p3t.setSurfacePosition(xfl::BOTSURFACE);
                        p3b.setSurfacePosition(xfl::TOPSURFACE);
                    }
                    else if(p3t.normal().y>zcrit && p3b.normal().y<-zcrit)
                    {
                        p3t.setSurfacePosition(xfl::TOPSURFACE);
                        p3b.setSurfacePosition(xfl::BOTSURFACE);
                    }
                    else if(p3b.normal().y>zcrit && p3t.normal().y<-zcrit)
                    {
                        p3t.setSurfacePosition(xfl::BOTSURFACE);
                        p3b.setSurfacePosition(xfl::TOPSURFACE);
                    }
                    else
                    {
                        //anything
                        p3t.setSurfacePosition(xfl::TOPSURFACE);
                        p3b.setSurfacePosition(xfl::BOTSURFACE);
                    }
                    iTE++;
                    break;
                }
            }
        }
    }

    std::vector<int> errorlist;
    m_pPlaneSTL->refTriMesh().connectTrailingEdges(errorlist); // re-orders vertices

    m_ppto->onAppendQText(QString::asprintf("Found %d pairs of TE panels.\n", iTE));
    QString strange;
    for(int idx : errorlist)
    {
        strange = QString::asprintf("   TE error at index = %d\n", idx);
        m_ppto->onAppendQText(strange);
    }

    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();

    m_bChanged = true;

    QApplication::restoreOverrideCursor();
}


void PlaneSTLDlg::onCheckTEPanels()
{
    deselectButtons();

    std::vector<int> errorlist;
    bool bNoError = m_pPlaneSTL->refTriMesh().connectTrailingEdges(errorlist);
    m_pPlaneSTL->triMesh() = m_pPlaneSTL->refTriMesh();

    if(errorlist.size())
    {
        QString log;
        log = "The following panels miss an opposite TE panel:\n";
        for(uint i=0; i<errorlist.size(); i++)
        {
            log += QString::asprintf("   %4d\n", errorlist.at(i));
        }
        updateOutput(log);
    }

/*    int nTop=0, nBot=0;
    for(int i3=0; i3<m_pPlaneSTL->refTriMesh().nPanels(); i3++)
    {
        Panel3 const &p3 = m_pPlaneSTL->panel3At(i3);
        if(p3.isTrailing())
        {
            if(p3.isTopPanel()) nTop++;
            if(p3.isBotPanel()) nBot++;
        }
    }
    if(nTop!=nBot)
    {
        QString log = QString::asprintf("The number of top trailing panels (%d) "
                                        "does not match the number of bottom trailing panels (%d)\n", nTop, nBot);
        updateOutput(log);
    }*/

    if(bNoError)
    {
        updateOutput("No TE error.\n");
    }

    QVector<int> qVec = QVector<int>(errorlist.begin(), errorlist.end());
    m_pglPlaneView->setHighlightList(qVec);
    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();

    updateOutput("\n");
}


void PlaneSTLDlg::onSetColor()
{
    QColor clr = QColorDialog::getColor(xfl::fromfl5Clr(m_pPlaneSTL->lineColor()), this, "Panel colour", QColorDialog::ShowAlphaChannel);
    if(clr.isValid())
    {
        m_pPlaneSTL->setSurfaceColor(xfl::tofl5Clr(clr));
        m_pcbColor->setColor(clr);
    }
    update();
    m_bDescriptionChanged = true;
}


void PlaneSTLDlg::onMakeMeshFromTriangles()
{
    QStringList labels("Node merge distance:");
    QStringList rightlabels({Units::lengthUnitQLabel()});
    QVector<double> vals({XflMesh::nodeMergeDistance()*Units::mtoUnit()});
    DoubleValueDlg dlg(this, vals, labels, rightlabels);

    if(dlg.exec()!=QDialog::Accepted)  return;
    XflMesh::setNodeMergeDistance(dlg.value(0)/Units::mtoUnit());

    QString strange("Converting STL triangles to mesh panels\n");
    strange += QString::asprintf("   Nodes closer than %g ", XflMesh::nodeMergeDistance()*Units::mtoUnit());
    strange += Units::lengthUnitQLabel() + " will be merged\n";
    m_ppto->onAppendQText(strange);

    deselectButtons();

    updateOutput("   Making STL triangle nodes\n");
    m_pPlaneSTL->makeTriangleNodes();
    updateOutput("   Making STL node normals\n");
    m_pPlaneSTL->makeNodeNormals();
    updateOutput("   Making mesh from STL triangles\n");
    std::string log, prefix("      ");
    m_pPlaneSTL->m_RefTriMesh.makeMeshFromTriangles(m_pPlaneSTL->m_Triangulation.triangles(), 0, xfl::NOSURFACE, log, prefix);
    for(int i3=0; i3<m_pPlaneSTL->m_RefTriMesh.nPanels(); i3++) m_pPlaneSTL->m_RefTriMesh.panel(i3).setFromSTL(true);
    m_pPlaneSTL->restoreMesh();
    updateStdOutput(log);

    updateStdOutput("   The mesh has been rebuilt.\n\n");

    onUpdatePlaneProps();

    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();
    m_bChanged = true;
}


void PlaneSTLDlg::onPickedNodePair(QPair<int, int> nodepair)
{
    if(nodepair.first <0 || nodepair.first >=m_pPlaneSTL->refTriMesh().nPanels()) return;
    if(nodepair.second<0 || nodepair.second>=m_pPlaneSTL->refTriMesh().nPanels()) return;
    Node nsrc  = m_pPlaneSTL->refTriMesh().node(nodepair.first);
    Node ndest = m_pPlaneSTL->refTriMesh().node(nodepair.second);

    Segment3d seg(nsrc, ndest);
    m_pglPlaneView->setMeasure(seg);

    QString strange;
    strange += QString::asprintf("   distance = %13g ", seg.length()   *Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n";
    strange += QString::asprintf("         dx = %13g ", seg.segment().x*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n";
    strange += QString::asprintf("         dy = %13g ", seg.segment().y*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n";
    strange += QString::asprintf("         dz = %13g ", seg.segment().z*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n\n";
    updateOutput(strange);

    m_pglPlaneView->resetPickedNodes();
    m_pglPlaneView->update();

    m_bChanged = true;
}


void PlaneSTLDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("PlaneSTLDlg");
    {
        s_STLGeometry = settings.value("Geometry", QByteArray()).toByteArray();
        s_VSplitterSizes      = settings.value("VSplitterSizes").toByteArray();
    }
    settings.endGroup();
}


void PlaneSTLDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("PlaneSTLDlg");
    {
        settings.setValue("Geometry", s_STLGeometry);

        settings.setValue("VSplitterSizes",      s_VSplitterSizes);
    }
    settings.endGroup();
}


