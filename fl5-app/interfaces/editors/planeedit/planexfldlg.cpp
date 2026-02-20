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
#include <QButtonGroup>
#include <QFileDialog>

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include "planexfldlg.h"



#include <api/constants.h>
#include <api/frame.h>
#include <api/fuse.h>
#include <api/fuseocc.h>
#include <api/fusesections.h>
#include <api/fusestl.h>
#include <api/fusexfl.h>
#include <api/geom_global.h>
#include <api/units.h>
#include <api/objects2d.h>
#include <api/objects3d.h>
#include <api/objects_global.h>
#include <api/occ_globals.h>
#include <api/panel3.h>
#include <api/part.h>
#include <api/planexfl.h>
#include <api/quad3d.h>
#include <api/segment2d.h>
#include <api/vector3d.h>
#include <api/wingxfl.h>
#include <api/xmlfusereader.h>
#include <api/xmlwingreader.h>

#include <core/saveoptions.h>
#include <core/stlreaderdlg.h>
#include <core/xflcore.h>
#include <interfaces/editors/fuseedit/flatfaceconverterdlg.h>
#include <interfaces/editors/fuseedit/fusemesherdlg.h>
#include <interfaces/editors/fuseedit/fuseoccdlg.h>
#include <interfaces/editors/fuseedit/fusestldlg.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fusexfldefdlg.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fusexflobjectdlg.h>
#include <interfaces/editors/importobjectdlg.h>
#include <interfaces/editors/inertia/partinertiadlg.h>
#include <interfaces/editors/inertia/planexflinertiadlg.h>
#include <interfaces/editors/planeedit/planepartdelegate.h>
#include <interfaces/editors/planeedit/planepartmodel.h>
#include <interfaces/editors/wingedit/wingdefdlg.h>
#include <interfaces/editors/wingedit/wingobjectdlg.h>
#include <interfaces/editors/wingedit/wingscaledlg.h>
#include <interfaces/exchange/stlwriterdlg.h>
#include <interfaces/mesh/afmesher.h>
#include <interfaces/mesh/gmesh_globals.h>
#include <interfaces/mesh/gmesherwt.h>
#include <interfaces/mesh/mesherwt.h>
#include <interfaces/mesh/meshevent.h>
#include <interfaces/mesh/tesscontrolsdlg.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dplanexflview.h>
#include <interfaces/widgets/customdlg/doublevaluedlg.h>
#include <interfaces/widgets/customdlg/intvaluedlg.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/formattextoutput.h>
#include <modules/xobjects.h>


int PlaneXflDlg::s_iActivePage = 1;

double PlaneXflDlg::s_StitchPrecision = 1.0e-4;
double PlaneXflDlg::s_NodeMergeDistance = 0.05;

QByteArray PlaneXflDlg::s_HSplitterSizes;
QByteArray PlaneXflDlg::s_PartSplitterSizes;
QByteArray PlaneXflDlg::s_XflGeometry;

#define NPOINTS 19

PlaneXflDlg::PlaneXflDlg(QWidget *pParent) : PlaneDlg(pParent)
{
    setWindowTitle(tr("Xfl plane editor"));

    m_pPlane = nullptr;

    m_StackPos = 0;

    makeActions();
    setupLayout();
    connectSignals();

}


void PlaneXflDlg::connectSignals()
{
    connectBaseSignals();

    connect(m_pClearOutput,           SIGNAL(triggered(bool)), SLOT(onClearOutput()));

    connect(m_pInsertWing,            SIGNAL(triggered(bool)), SLOT(onInsertWing()));
    connect(m_pInsertWingOther,       SIGNAL(triggered(bool)), SLOT(onImportOtherWing()));
    connect(m_pInsertWingXml,         SIGNAL(triggered(bool)), SLOT(onInsertWingFromXml()));
    connect(m_pInsertWingVSP,         SIGNAL(triggered(bool)), SLOT(onInsertWingFromVSP()));
    connect(m_pInsertElev,            SIGNAL(triggered(bool)), SLOT(onInsertElevator()));
    connect(m_pInsertFin,             SIGNAL(triggered(bool)), SLOT(onInsertFin()));
    connect(m_pInsertFuseXflSpline,   SIGNAL(triggered(bool)), SLOT(onInsertFuseXfl()));
    connect(m_pInsertFuseXflFlat,     SIGNAL(triggered(bool)), SLOT(onInsertFuseXfl()));
    connect(m_pInsertFuseXflSections, SIGNAL(triggered(bool)), SLOT(onInsertFuseXfl()));
    connect(m_pInsertFuseXml,         SIGNAL(triggered(bool)), SLOT(onInsertFuseXml()));
    connect(m_pInsertFuseOther,       SIGNAL(triggered(bool)), SLOT(onImportOtherFuse()));
    connect(m_pInsertFuseOcc,         SIGNAL(triggered(bool)), SLOT(onInsertFuseOcc()));
    connect(m_pInsertFuseStl,         SIGNAL(triggered(bool)), SLOT(onInsertFuseStl()));
    connect(m_pInsertEllWing,         SIGNAL(triggered(bool)), SLOT(onInsertEllipticWing()));
    connect(m_pInsertSTLSphere,       SIGNAL(triggered(bool)), SLOT(onInsertSTLSphereFuse()));
    connect(m_pInsertSTLCylinder,     SIGNAL(triggered(bool)), SLOT(onInsertSTLCylinderFuse()));
    connect(m_pInsertCADSphere,       SIGNAL(triggered(bool)), SLOT(onInsertCADShape()));
    connect(m_pInsertCADCylinder,     SIGNAL(triggered(bool)), SLOT(onInsertCADShape()));
    connect(m_pInsertCADBox,          SIGNAL(triggered(bool)), SLOT(onInsertCADShape()));
    connect(m_pRemovePart,            SIGNAL(triggered(bool)), SLOT(onRemovePart()));
    connect(m_pDuplicatePart,         SIGNAL(triggered(bool)), SLOT(onDuplicatePart()));
    connect(m_pEditPartDef,           SIGNAL(triggered(bool)), SLOT(onEditPart()));
//    connect(m_pEditPartObject,        SIGNAL(triggered(bool)), SLOT(onEditPart()));
    connect(m_pExportMeshSTL,         SIGNAL(triggered(bool)), SLOT(onExportMeshToSTLFile()));
    connect(m_pScalePlane,            SIGNAL(triggered(bool)), SLOT(onScalePlane()));

    connect(m_plwWings,               SIGNAL(itemSelectionChanged()), SLOT(onThinListClick()));

    connect(m_pResetFuse,             SIGNAL(triggered()),                                       SLOT(onResetFuse()));
    connect(m_pTessellation,          SIGNAL(triggered()),                                       SLOT(onTessellation()));

    connect(m_prbThin,                SIGNAL(clicked(bool)),                                     SLOT(onThinThick()));
    connect(m_prbThick,               SIGNAL(clicked(bool)),                                     SLOT(onThinThick()));

    connect(m_pLeftTabWidget,         SIGNAL(currentChanged(int)),                               SLOT(onTabChanged(int)));

    connect(m_pcptParts,              SIGNAL(clicked(QModelIndex)),                              SLOT(onPartItemClicked(QModelIndex)));
    connect(m_pcptParts,              SIGNAL(dataPasted()),                                      SLOT(onPartDataChanged()));
    connect(m_pPartModel,             SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(onPartDataChanged()));
    connect(m_pPartModel,             SIGNAL(wingNamesChanged()),                                SLOT(onNamesChanged()));
    connect(m_pchHighlightSel,        SIGNAL(clicked(bool)),                                     SLOT(onHighlightSel(bool)));


    connect(m_pMoveUp,                SIGNAL(triggered(bool)),                                   SLOT(onMovePartUp()));
    connect(m_pMoveDown,              SIGNAL(triggered(bool)),                                   SLOT(onMovePartDown()));
    connect(m_pPartInertia,           SIGNAL(triggered(bool)),                                   SLOT(onPartInertia()));
    connect(m_pPartScale,             SIGNAL(triggered(bool)),                                   SLOT(onScalePart()));

    connect(m_pHSplitter,             SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved(int,int)));

    connect(m_pglPlaneView,           SIGNAL(partSelected(Part*)),            SLOT(onSelectPart(Part*)));
    connect(m_pglPlaneView,           SIGNAL(pickedNodePair(QPair<int,int>)), SLOT(onPickedNodePair(QPair<int,int>)));

    connect(m_pGMesherWt,             SIGNAL(outputMsg(QString)), m_ppto, SLOT(onAppendQText(QString)));
    connect(m_pGMesherWt,             SIGNAL(updateFuseView()),           SLOT(onUpdateMesh()));

    connect(m_pRestoreFuseMesh,       SIGNAL(triggered()),    SLOT(onResetFuseMesh()));
    connect(m_pFuseMesher,            SIGNAL(triggered()),    SLOT(onFuseMeshDlg()));

    connect(m_ppbMoveNode,            SIGNAL(clicked(bool)),  SLOT(onMergeNodes(bool)));

    connect(m_ppbSelectPanels,        SIGNAL(clicked(bool)),  SLOT(onSelectPanels(bool)));
    connect(m_ppbDeletePanel,         SIGNAL(clicked(bool)),  SLOT(onDeleteP3Selection()));

    connect(m_ppbMakeP3,              SIGNAL(clicked(bool)),  SLOT(onMakeP3(bool)));
    connect(m_ppbMakeP3Strip,         SIGNAL(clicked(bool)),  SLOT(onMakeP3Strip(bool)));

    connect(m_ppbUndo,                SIGNAL(clicked()), SLOT(onUndo()));
    connect(m_ppbRedo,                SIGNAL(clicked()), SLOT(onRedo()));
}


void PlaneXflDlg::setupLayout()
{
    m_pScalePlane  = new QAction(tr("Scale plane"));
    QMenu *pCheckMeshMenu = m_ppbActions->menu();
    pCheckMeshMenu->addSeparator();
    pCheckMeshMenu->addAction(m_pScalePlane);

    m_pHSplitter = new QSplitter(Qt::Horizontal);
    {
        m_pHSplitter->setChildrenCollapsible(false);
        QFrame *pLeftSideWidget = new QFrame;
        {
            QVBoxLayout *pLeftSideLayout = new QVBoxLayout;
            {
                m_pPartSplitter = new QSplitter(Qt::Vertical);
                {
                    m_pPartSplitter->setChildrenCollapsible(false);
                    m_pLeftTabWidget = new QTabWidget;
                    {
                        QFrame *pMetaFrame = new QFrame;
                        {
                            QVBoxLayout *pDefinitionLayout = new QVBoxLayout;
                            {
                                m_pleDescription->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

                                pDefinitionLayout->addWidget(m_pleName);
                                pDefinitionLayout->addWidget(m_pleDescription);
                            }
                            pMetaFrame->setLayout(pDefinitionLayout);
                        }

                        QFrame *pfrParts = new QFrame;
                        {
                            QVBoxLayout *pPartLayout = new QVBoxLayout;
                            {
                                makePartTable();
                                QHBoxLayout *pEditCmdLayout = new QHBoxLayout;
                                {
                                    QPushButton *ppbInsertBtn = new QPushButton(tr("Insert"));
                                    QMenu *pInsertMenu = new QMenu(this);
                                    {
                                        QMenu *pInsertWingMenu = pInsertMenu->addMenu(tr("Wing"));
                                        {
                                            pInsertWingMenu->addAction(m_pInsertWing);
                                            pInsertWingMenu->addAction(m_pInsertWingOther);
                                            pInsertWingMenu->addAction(m_pInsertWingXml);
                                            pInsertWingMenu->addAction(m_pInsertWingVSP);
                                        }

                                        pInsertMenu->addAction(m_pInsertElev);
                                        pInsertMenu->addAction(m_pInsertFin);
                                        pInsertMenu->addSeparator();

                                        QMenu*pFuseMenu = pInsertMenu->addMenu(tr("Fuselage"));
                                        {
                                            QMenu *pFuseXflMenu = pFuseMenu->addMenu(tr("xfl-type fuselage"));
                                            {
                                                pFuseXflMenu->addAction(m_pInsertFuseXflSpline);
                                                pFuseXflMenu->addAction(m_pInsertFuseXflFlat);
                                                pFuseXflMenu->addAction(m_pInsertFuseXflSections);
                                                pFuseXflMenu->addAction(m_pInsertFuseXml);
                                            }
                                            pFuseMenu->addAction(m_pInsertFuseOther);
                                            pFuseMenu->addAction(m_pInsertFuseOcc);
                                            pFuseMenu->addAction(m_pInsertFuseStl);
                                        }
                                        pInsertMenu->addSeparator();
                                        QMenu *pTestMenu = pInsertMenu->addMenu(tr("Testing"));
                                        {
                                            pTestMenu->addAction(m_pInsertEllWing);
                                            pTestMenu->addSeparator();
                                            QMenu *pFuseMenu = pTestMenu->addMenu(tr("Fuselage"));
                                            {
                                                pFuseMenu->addAction(m_pInsertSTLSphere);
                                                pFuseMenu->addAction(m_pInsertSTLCylinder);
                                                pFuseMenu->addSeparator();
//                                                pFuseMenu->addAction(m_pInsertCADSphere);
//                                                pFuseMenu->addAction(m_pInsertCADCylinder);
                                                pFuseMenu->addAction(m_pInsertCADBox);
                                            }
                                        }

                                        ppbInsertBtn->setMenu(pInsertMenu);
                                    }

                                    m_pchHighlightSel = new QCheckBox(tr("Highlight selected part"));

                                    pEditCmdLayout->addWidget(ppbInsertBtn);
                                    pEditCmdLayout->addStretch();
                                    pEditCmdLayout->addWidget(m_pchHighlightSel);
                                }
                                pPartLayout->addWidget(m_pcptParts);
                                pPartLayout->addLayout(pEditCmdLayout);
                            }
                            pfrParts->setLayout(pPartLayout);
                        }

                        QFrame *pfrThinThick = new QFrame;
                        {
                            QVBoxLayout *pThinThickLayout = new QVBoxLayout;
                            {

                                QHBoxLayout *pSelLayout = new QHBoxLayout;
                                {
                                    QLabel *plabAssyType = new QLabel(tr("Target assembly:"));
                                    QButtonGroup *pGroup = new QButtonGroup;
                                    {
                                        m_prbThin  = new QRadioButton(tr("Thin  surfaces"));
                                        m_prbThick = new QRadioButton(tr("Thick surfaces"));
                                        m_prbThin->setToolTip(tr("<p>Assemble the fuselage and wings for THIN surface calculations;<br>"
                                                              "The fuselage mesh will conform to the mid-camber line of the selected wings."
                                                              "</p>"));
                                        m_prbThick->setToolTip(tr("<p>Assemble the fuselage and wings for THICK surface calculations;<br>"
                                                              "The fuselage mesh will conform to the top and bottom surfaces of the selected wings."
                                                              "</p>"));

                                        pGroup->addButton(m_prbThin);
                                        pGroup->addButton(m_prbThick);
                                    }
                                    pSelLayout->addWidget(plabAssyType);
                                    pSelLayout->addWidget(m_prbThin);
                                    pSelLayout->addWidget(m_prbThick);
                                    pSelLayout->addStretch();
                                }

                                QLabel *plabConnectedWings = new QLabel(tr("Select the wings connected to the fuselage:"));
                                m_plwWings = new QListWidget;
                                m_plwWings->setSelectionMode(QAbstractItemView::MultiSelection);

                                pThinThickLayout->addLayout(pSelLayout);
                                pThinThickLayout->addWidget(plabConnectedWings);
                                pThinThickLayout->addWidget(m_plwWings);
                            }
                            pfrThinThick->setLayout(pThinThickLayout);
                        }

                        QFrame *pfrGmesher = new QFrame;
                        {
                            QVBoxLayout *pGMesherLayout = new QVBoxLayout;
                            {
                                QLabel *plabGmsh = new QLabel("<b>Gmsh</b>");

                                QLabel *plabGmshLink = new QLabel;
                                plabGmshLink->setText("<a href=https://gmsh.info>https://gmsh.info</a>");
                                plabGmshLink->setOpenExternalLinks(true);
                                plabGmshLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);


                                m_pGMesherWt = new GMesherWt(this);

                                pGMesherLayout->addWidget(plabGmsh);
                                pGMesherLayout->addWidget(plabGmshLink);
                                pGMesherLayout->addWidget(m_pGMesherWt);
                            }
                            pfrGmesher->setLayout(pGMesherLayout);
                        }

                        m_pMeshCorrectionsFrame = new QFrame;
                        {
                            QLabel *plabInfo = new QLabel(tr("Use the actions below to modify the fuselage's mesh<br>"
                                                          "and to connect it to the wing's mesh."));
                            QVBoxLayout *pCorrectLayout = new QVBoxLayout;
                            {
                                QGridLayout *pModLayout = new QGridLayout;
                                {
                                    QLabel *plabTrans = new QLabel(tr("Node translation:"));
                                    m_ppbMoveNode = new QPushButton(tr("Move node"));
                                    QString tip = tr("<p>Use this option to move a fuselage node and to merge it with another. "
                                                     "Select first the source node to move, then the destination node.</p<");
                                    m_ppbMoveNode->setToolTip(tip);
                                    m_ppbMoveNode->setCheckable(true);


                                    QLabel *plabDelete = new QLabel(tr("Panel deletion:"));
                                    m_ppbSelectPanels = new QPushButton(tr("Select"));
                                    m_ppbSelectPanels->setCheckable(true);
                                    m_ppbDeletePanel = new QPushButton(tr("Delete"));

                                    QLabel *plabMake = new QLabel(tr("Panel creation:"));
                                    m_ppbMakeP3 = new QPushButton(tr("Single"));
                                    m_ppbMakeP3->setCheckable(true);

                                    m_ppbMakeP3Strip = new QPushButton(tr("Strip"));
                                    m_ppbMakeP3Strip->setCheckable(true);

                                    m_pchMakeP3Opposite = new QCheckBox(tr("Make opposite"));

                                    pModLayout->addWidget(plabInfo,            0, 1, 1 ,3);
                                    pModLayout->addWidget(plabTrans,           1, 1);
                                    pModLayout->addWidget(m_ppbMoveNode,       1, 2);
                                    pModLayout->addWidget(plabDelete,          2, 1);
                                    pModLayout->addWidget(m_ppbSelectPanels,   2, 2);
                                    pModLayout->addWidget(m_ppbDeletePanel,    2, 3);
                                    pModLayout->addWidget(plabMake,            3, 1);
                                    pModLayout->addWidget(m_ppbMakeP3,         3, 2);
                                    pModLayout->addWidget(m_ppbMakeP3Strip,    3, 3);
                                    pModLayout->addWidget(m_pchMakeP3Opposite, 3, 4);
                                }

                                QHBoxLayout *pUndoRedoLayout = new QHBoxLayout;
                                {
                                    m_ppbUndo = new QPushButton(QIcon(":/icons/OnUndo.png"), tr("Undo"));
                                    m_ppbUndo->setShortcut(Qt::Key_Undo);
                                    m_ppbRedo = new QPushButton(QIcon(":/icons/OnRedo.png"), tr("Redo"));
                                    m_ppbUndo->setShortcut(Qt::Key_Redo);

                                    m_plabStackInfo = new QLabel("stackpos: 0/0");
                                    pUndoRedoLayout->addStretch();
                                    pUndoRedoLayout->addWidget(m_ppbUndo);
                                    pUndoRedoLayout->addWidget(m_ppbRedo);
                                    pUndoRedoLayout->addWidget(m_plabStackInfo);
                                    pUndoRedoLayout->addStretch();
                                }

                                m_ppbCheckMenuBtn = new QPushButton(tr("Mesh actions"));
                                {
                                    QMenu *pCheckMeshMenu = new QMenu(tr("Mesh actions"));
                                    {
                                        m_pRestoreFuseMesh  = new QAction(tr("Restore default mesh"),this);
                                        m_pFuseMesher    = new QAction(tr("Fuselage mesher"));
                                        pCheckMeshMenu->addAction(m_pCheckMesh);
                                        pCheckMeshMenu->addAction(m_pConnectPanels);
                                        pCheckMeshMenu->addAction(m_pCheckFreeEdges);
                                        pCheckMeshMenu->addAction(m_pClearHighlighted);
                                        pCheckMeshMenu->addSeparator();
                                        pCheckMeshMenu->addAction(m_pMergeFuseToWingNodes);
                                        pCheckMeshMenu->addSeparator();
                                        pCheckMeshMenu->addAction(m_pCenterOnPanel);
                                        pCheckMeshMenu->addSeparator();
                                        pCheckMeshMenu->addAction(m_pRestoreFuseMesh);
                                        pCheckMeshMenu->addSeparator();
                                        pCheckMeshMenu->addAction(m_pFuseMesher);
                                    }
                                    m_ppbCheckMenuBtn->setMenu(pCheckMeshMenu);
                                }
                                QLabel *pFlow5Link = new QLabel;
                                pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Modelling/Connections.html>https://flow5.tech/docs/flow5_doc/Modelling/Connections.html</a>");
                                pFlow5Link->setOpenExternalLinks(true);
                                pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);

                                pCorrectLayout->addLayout(pModLayout);
                                pCorrectLayout->addLayout(pUndoRedoLayout);
                                pCorrectLayout->addStretch();
                                pCorrectLayout->addWidget(m_ppbCheckMenuBtn);
                                pCorrectLayout->addWidget(pFlow5Link);
                            }
                            m_pMeshCorrectionsFrame->setLayout(pCorrectLayout);
                        }

                        m_pLeftTabWidget->addTab(pMetaFrame,               tr("Meta"));
                        m_pLeftTabWidget->addTab(pfrParts,                 tr("Parts"));
                        m_pLeftTabWidget->addTab(pfrThinThick,             tr("Assembly"));
                        m_pLeftTabWidget->addTab(pfrGmesher,               tr("Fuselage mesh"));
                        m_pLeftTabWidget->addTab(m_pMeshCorrectionsFrame , tr("Mesh connections"));

                        m_pLeftTabWidget->setTabToolTip(0, "Ctrl+1");
                        m_pLeftTabWidget->setTabToolTip(1, "Ctrl+2");
                        m_pLeftTabWidget->setTabToolTip(2, "Ctrl+3");
                        m_pLeftTabWidget->setTabToolTip(3, "Ctrl+4");
                        m_pLeftTabWidget->setTabToolTip(4, "Ctrl+5");
                    }

                    m_pPartSplitter->addWidget(m_pLeftTabWidget);
                    m_pPartSplitter->addWidget(m_ppto);
                    m_pPartSplitter->setStretchFactor(0,1);
                    m_pPartSplitter->setStretchFactor(1,5);
                }

                pLeftSideLayout->addWidget(m_pPartSplitter);
                pLeftSideLayout->addWidget(m_pButtonBox);
                pLeftSideWidget->setLayout(pLeftSideLayout);
            }
        }

        m_pglPlaneViewFrame = new QFrame;
        {
            QVBoxLayout *pTWPageLayout = new QVBoxLayout;
            {
                m_pglPlaneView = new gl3dPlaneXflView;
                m_pglControls  = new gl3dGeomControls(m_pglPlaneView, PlaneLayout, true);
                connect(m_pglControls->m_ptbDistance, SIGNAL(clicked()), this, SLOT(onNodeDistance()));
                pTWPageLayout->addWidget(m_pglPlaneView);
                pTWPageLayout->addWidget(m_pglControls);
            }
            m_pglPlaneViewFrame->setLayout(pTWPageLayout);
        }

        m_pHSplitter->addWidget(pLeftSideWidget);
#ifdef QT_DEBUG
        m_pHSplitter->addWidget(m_pglPlaneViewFrame);
#else
        m_pHSplitter->addWidget(m_pglPlaneViewFrame);
#endif
        m_pHSplitter->setStretchFactor(0,1);
        m_pHSplitter->setStretchFactor(1,2);
    }

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);
}


void PlaneXflDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent const*pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        updateOutput(pMsgEvent->msg());
    }
    else if(pEvent->type() == MESH_UPDATE_EVENT)
    {
        MeshEvent *pMeshEvent = dynamic_cast<MeshEvent*>(pEvent);

        Fuse *pFuse = m_pPlaneXfl->fuse(0);
        if(!pFuse) return;
        std::string str;
        str = "   Making mesh from triangles\n";
        pFuse->triMesh().makeMeshFromTriangles(pMeshEvent->triangles(), 0, xfl::FUSESURFACE, str, "      ");
        updateStdOutput(str);

        QString strange;
        pFuse->setMaxElementSize(AFMesher::maxEdgeLength());
        strange  = QString::asprintf("\nTriangle count = %d\n", pFuse->nPanel3());
        strange += QString::asprintf("Node count     = %d\n", int(pFuse->nodes().size()));
        strange += "\n_______\n\n";
        updateOutput(strange);
        onUpdateMesh();
        m_bChanged = true;
    }
    else
        QDialog::customEvent(pEvent);
}


void PlaneXflDlg::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QMenu *pPlaneCtxMenu = new QMenu(tr("context menu"), this);
    {
        pPlaneCtxMenu->addAction(m_pPlaneInertia);
        pPlaneCtxMenu->addSeparator();
        pPlaneCtxMenu->addAction(m_pScalePlane);
        pPlaneCtxMenu->addSeparator();
        QMenu *pCheckMeshMenu = pPlaneCtxMenu->addMenu(tr("Mesh"));
        {
            m_pRestoreFuseMesh  = new QAction(tr("Restore default mesh"),this);
            m_pFuseMesher       = new QAction(tr("Fuselage mesher"));
            pCheckMeshMenu->addAction(m_pCheckMesh);
            pCheckMeshMenu->addAction(m_pConnectPanels);
            pCheckMeshMenu->addAction(m_pCheckFreeEdges);
            pCheckMeshMenu->addAction(m_pClearHighlighted);
            pCheckMeshMenu->addSeparator();
            pCheckMeshMenu->addAction(m_pMergeFuseToWingNodes);
            pCheckMeshMenu->addSeparator();
            pCheckMeshMenu->addAction(m_pCenterOnPanel);
            pCheckMeshMenu->addSeparator();
            pCheckMeshMenu->addAction(m_pRestoreFuseMesh);
            pCheckMeshMenu->addSeparator();
            pCheckMeshMenu->addAction(m_pFuseMesher);
        }
        pPlaneCtxMenu->addSeparator();

        QAction *m_pBackImageLoad = new QAction(tr("Load"), this);
        connect(m_pBackImageLoad, SIGNAL(triggered()), m_pglPlaneView, SLOT(onLoadBackImage()));

        QAction *m_pBackImageClear = new QAction(tr("Clear"), this);
        connect(m_pBackImageClear, SIGNAL(triggered()), m_pglPlaneView, SLOT(onClearBackImage()));

        QAction *m_pBackImageSettings = new QAction(tr("Settings"), this);
        connect(m_pBackImageSettings, SIGNAL(triggered()), m_pglPlaneView, SLOT(onBackImageSettings()));

        pPlaneCtxMenu->addSeparator();
        QMenu *pBackImageMenu = pPlaneCtxMenu->addMenu(tr("Background image"));
        {
            pBackImageMenu->addAction(m_pBackImageLoad);
            pBackImageMenu->addAction(m_pBackImageClear);
            pBackImageMenu->addAction(m_pBackImageSettings);
        }

    }
    pPlaneCtxMenu->exec(pEvent->globalPos());
}


void PlaneXflDlg::initDialog(Plane *pPlane, bool bAcceptName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    PlaneDlg::initDialog(pPlane, bAcceptName);

    PlaneXfl* pPlaneXfl = dynamic_cast<PlaneXfl*>(pPlane);

    m_pPlaneXfl = pPlaneXfl;
    m_bAcceptName = bAcceptName;

    m_pPartDelegate->setPlane(m_pPlaneXfl);
    m_pPartModel->setPlane(m_pPlaneXfl);
    m_pPartModel->updateData();

    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->setPlane(m_pPlaneXfl);
    if(m_pPlaneXfl->mainWing())
        m_pglPlaneView->setReferenceLength(m_pPlaneXfl->mainWing()->planformSpan()/2.0);
    else
    {
        if(m_pPlaneXfl->hasFuse())  m_pglPlaneView->setReferenceLength(m_pPlaneXfl->fuse(0)->length()*2);
        else                        m_pglPlaneView->setReferenceLength(1.0);
    }
    m_pglPlaneView->reset3dScale();

    for(int iw=0;    iw<pPlaneXfl->nWings();   iw++)    pPlaneXfl->wing(iw)->setVisible(true);
    for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++) pPlaneXfl->fuse(ifuse)->setVisible(true);


    onUpdatePlane();

    setControls();

    if(s_iActivePage>=0 && s_iActivePage<4) m_pLeftTabWidget->setCurrentIndex(s_iActivePage);

    QApplication::restoreOverrideCursor();
}


void PlaneXflDlg::setControls()
{
    gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    m_pchHighlightSel->setChecked(pglPlaneXflView->isHighlightingPart());

    m_prbThick->setChecked(m_pPlaneXfl->isThickBuild());
    m_prbThin->setChecked(!m_pPlaneXfl->isThickBuild());
}


void PlaneXflDlg::makeActions()
{
    m_pEditPartDef           = new QAction(tr("Edit"),                       this);
//    m_pEditPartObject        = new QAction(tr("Edit (advanced)"),            this);
    m_pInsertWing            = new QAction(tr("new"),                        this);
    m_pInsertWingOther       = new QAction(tr("from other plane"),           this);
    m_pInsertWingXml         = new QAction(tr("from XML file"),              this);
    m_pInsertWingVSP         = new QAction(tr("from VSP export"),            this);
    m_pInsertElev            = new QAction(tr("Elevator"),                   this);
    m_pInsertFin             = new QAction(tr("Fin"),                        this);
    m_pInsertFuseXflSpline   = new QAction(tr("NURBS type"),                 this);
    m_pInsertFuseXflFlat     = new QAction(tr("Quad faces"),                 this);
    m_pInsertFuseXflSections = new QAction(tr("Interpolated (in progress)"), this);

    m_pInsertFuseXml         = new QAction(tr("from XML file"),                   this);
    m_pInsertFuseOther       = new QAction(tr("from other plane"),                this);
    m_pInsertFuseOcc         = new QAction(tr("from CAD file"),                   this);
    m_pInsertFuseStl         = new QAction(tr("from STL file"),                   this);
    m_pInsertEllWing         = new QAction(tr("Insert elliptical wing"),          this);
    m_pInsertSTLSphere       = new QAction(tr("Insert STL sphere fuselage"),      this);
    m_pInsertSTLCylinder     = new QAction(tr("Insert STL cylinder fuselage"),    this);
    m_pInsertCADSphere       = new QAction(tr("Insert CAD sphere fuselage"),      this);
    m_pInsertCADCylinder     = new QAction(tr("Insert CAD cylinder fuselage"),    this);
    m_pInsertCADBox          = new QAction(tr("Insert CAD box fuselage"),         this);
    m_pRemovePart            = new QAction(tr("Remove"),                          this);
    m_pDuplicatePart         = new QAction(tr("Duplicate"),                       this);

    m_pResetFuse             = new QAction(tr("Restore geometry and mesh"),  this);
    m_pTessellation          = new QAction(tr("Fuse tessellation"),          this);

    m_pPartInertia   = new QAction(tr("Inertia"), this);
    m_pPartScale     = new QAction(tr("Scale"), this);
    m_pMoveUp        = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowUp),  tr("Move Up"), this);
    m_pMoveDown      = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowDown),  tr("Move Down"), this);

    m_pExportMeshSTL = new QAction(tr("Export mesh to STL"), this);

    m_pPartMenu = new QMenu(tr("Selected Part"), this);
    {
        m_pPartMenu->addAction(m_pEditPartDef);
//        m_pPartMenu->addAction(m_pEditPartObject);
        m_pPartMenu->addAction(m_pRemovePart);
        m_pPartMenu->addAction(m_pDuplicatePart);
        m_pPartMenu->addSeparator();
        m_pPartMenu->addAction(m_pPartInertia);
        m_pPartMenu->addAction(m_pPartScale);
        m_pPartMenu->addSeparator();
        m_pPartMenu->addAction(m_pResetFuse);
        m_pPartMenu->addAction(m_pTessellation);
        m_pPartMenu->addAction(m_pFlipNormals);
        m_pPartMenu->addSeparator();
        m_pPartMenu->addAction(m_pMoveUp);
        m_pPartMenu->addAction(m_pMoveDown);
    }
}


PlaneXflDlg::~PlaneXflDlg()
{
}


void PlaneXflDlg::keyPressEvent(QKeyEvent *pEvent)
{
    /*    bool bShift = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;*/
    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;
    bool bAlt = false;
    if(pEvent->modifiers() & Qt::AltModifier)   bAlt =true;

    switch (pEvent->key())
    {
        case(Qt::Key_Escape):
        {
            if(!cancelPicks())
                reject();
            return;
            break;
        }
        case Qt::Key_D:
        {
            if(bCtrl)
            {
                if(m_pPlaneXfl->nWings()>0)
                {
                    WingXfl MainWing(*m_pPlaneXfl->wingAt(0));
                    MainWing.createSurfaces(MainWing.position(), MainWing.rx(), MainWing.ry());

                    for (int j=0; j<MainWing.nSurfaces(); j++)
                        MainWing.surface(j).makeSideNodes(nullptr);

                    std::vector<std::vector<Node>> wires;
                    MainWing.makeMidWires(wires);

                    Q_ASSERT(wires.size()==3);

                    m_pglPlaneView->clearDebugPoints();
                    for(uint i=0; i<wires.front().size(); i++)
                    {
                        Node const &ndA = wires.front().at(i);
                        Node const &ndB = wires.at(1).at(i);
                        m_pglPlaneView->appendDebugPoint(ndA);
                        m_pglPlaneView->appendDebugVec(ndB-ndA);

                    }
                    for(uint i=0; i<wires.at(1).size(); i++)
                    {
                        Node const &ndA = wires.at(1).at(i);
                        Node const &ndB = wires.at(2).at(i);
                        m_pglPlaneView->appendDebugPoint(ndA);
                        m_pglPlaneView->appendDebugVec(ndB-ndA);

                    }

                    m_pglPlaneView->update();
                }
            }
            break;
        }
        case Qt::Key_1:
        {
            if(bCtrl)
            {
                m_pLeftTabWidget->setCurrentIndex(0);
            }
            break;
        }
        case Qt::Key_2:
        {
            if(bCtrl)
            {
                m_pLeftTabWidget->setCurrentIndex(1);
            }
            break;
        }
        case Qt::Key_3:
        {
            if(bCtrl)
            {
                m_pLeftTabWidget->setCurrentIndex(2);
            }
            break;
        }
        case Qt::Key_4:
        {
            if(bCtrl)
            {
                m_pLeftTabWidget->setCurrentIndex(3);
            }
            break;
        }
        case Qt::Key_5:
        {
            if(bCtrl)
            {
                m_pLeftTabWidget->setCurrentIndex(4);
            }
            break;
        }
        case Qt::Key_B:
        {
            if(bCtrl)
            {
                Fuse *pFuse = m_pPlaneXfl->fuse(0);
                if(pFuse) editFuse(0);
                onUpdatePlane();

                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_C:
        {
            if(bAlt)
            {
                m_pglPlaneView->clearHighlightList();
                m_pglPlaneView->updatePartFrame(m_pPlane);
                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_E:
        {
            if(bCtrl)
            {
                WingXfl *pStab = m_pPlaneXfl->stab();
                if(pStab)
                {
                    editWing(pStab);
                }

                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_F:
        {
            if(bCtrl)
            {
                WingXfl *pFin = m_pPlaneXfl->fin();
                if(pFin)
                {
                    editWing(pFin);
                }

                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_S:
        {
            if(bCtrl)
                onOK();
            break;
        }
        case Qt::Key_W:
        {
            if(bCtrl)
            {
                WingXfl *pWing = m_pPlaneXfl->mainWing();
                if(pWing)
                {
                    editWing(pWing);
                }

                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_F12:
        {
            onPlaneInertia();
            pEvent->accept();
            return;
        }
        default: break;
    }
    PlaneDlg::keyPressEvent(pEvent);
}


bool PlaneXflDlg::cancelPicks()
{
    bool bCancelled = false;
    if(m_pglControls->getDistance())
    {
        m_pglControls->stopDistance();
        m_pglPlaneView->clearMeasure();
        bCancelled = true;
    }

    if(m_ppbMoveNode->isChecked() || m_ppbSelectPanels->isChecked()|| m_ppbMakeP3->isChecked() || m_ppbMakeP3Strip->isChecked())
    {
        endPanelMods();
        m_ppbMoveNode->setChecked(false);
        m_ppbSelectPanels->setChecked(false);
        m_ppbMakeP3->setChecked(false);
        m_ppbMakeP3Strip->setChecked(false);
        bCancelled = true;
    }

    if(m_pglPlaneView->hasSelectedPart())
    {
        m_pglPlaneView->clearSelectedParts();
        m_pglPlaneView->update();
        bCancelled = true;
    }

    return bCancelled;
}


void PlaneXflDlg::showEvent(QShowEvent *pEvent)
{
    PlaneDlg::showEvent(pEvent);

    restoreGeometry(s_XflGeometry);

    if(s_HSplitterSizes.length()>0)  m_pHSplitter->restoreState(s_HSplitterSizes);
    if(s_PartSplitterSizes.length()>0)  m_pPartSplitter->restoreState(s_PartSplitterSizes);

    onResizeColumns();
}


void PlaneXflDlg::resizeEvent(QResizeEvent *pEvent)
{
    PlaneDlg::resizeEvent(pEvent);
    onSplitterMoved(0,0);
}


void PlaneXflDlg::hideEvent(QHideEvent *pEvent)
{
    readParams();

    m_pPlane = nullptr;

    s_HSplitterSizes = m_pHSplitter->saveState();
    s_PartSplitterSizes = m_pPartSplitter->saveState();

    s_iActivePage = m_pLeftTabWidget->currentIndex();

    s_XflGeometry = saveGeometry();

    PlaneDlg::hideEvent(pEvent);
}


void PlaneXflDlg::readParams()
{
    if(!m_pPlane) return;
}


void PlaneXflDlg::onOK(int iExitCode)
{
    for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
        m_pPlaneXfl->wing(iw)->setVisible(true);
    for(int ifuse=0; ifuse<m_pPlaneXfl->nFuse(); ifuse++)
        m_pPlaneXfl->fuse(ifuse)->setVisible(true);

    if(iExitCode==QDialog::Accepted)
        m_pPlaneXfl->setName(m_pleName->text().toStdString());
    else
        m_pPlaneXfl->setName(std::string()); // force rename

    m_pPlaneXfl->setDescription(m_pleDescription->toPlainText().toStdString());

    m_pPlaneXfl->makeTriMesh(m_pPlaneXfl->isThickBuild());

    done(iExitCode);
}


void PlaneXflDlg::onUpdatePlaneProps()
{
    if(!m_pPlane) return;

    QString strange, str1, str2;
    str1 = QString::asprintf("Quads          = %5d\n", m_pPlaneXfl->nPanel4());
    str2 = QString::asprintf("Triangles      = %5d",   m_pPlaneXfl->nPanel3());
    strange = QString::fromStdString(m_pPlaneXfl->planeData(true)) + "\n" + str1 + str2;

    m_pglPlaneView->setBotLeftOutput(strange);
}


void PlaneXflDlg::onSplitterMoved(int , int)
{
    onResizeColumns();
}


void PlaneXflDlg::onPlaneInertia()
{
    PlaneXflInertiaDlg dlg(this);
    dlg.hideSaveAsNew();
    dlg.initDialog(m_pPlaneXfl);
    dlg.exec();

    m_bChanged = true;
    m_pglPlaneView->update();
}


void PlaneXflDlg::onInsertFuseXml()
{
    QString PathName;
    PathName = QFileDialog::getOpenFileName(this, "Open XML file",
                                            SaveOptions::xmlPlaneDirName(),
                                             "Fuse XML file (*.xml)");
    if(!PathName.length()) return;

    QFile XFile(PathName);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange =  "<br><font color=red>Could not open the file:</font>" +PathName + "<br>";
        m_ppto->appendHtmlText(strange);
        return;
    }

    XmlFuseReader fusereader(XFile);
    fusereader.readXMLFuseFile();
    XFile.close();

    if(fusereader.hasError())
    {
        QString errorMsg = "<br><font color=red>Error reading file</font><br>"+ fusereader.errorString() +
                QString::asprintf(" at line %d column %d<br><br>", int(fusereader.lineNumber()), int(fusereader.columnNumber()));

        m_ppto->appendHtmlText(errorMsg);

        return;
    }

    FuseXfl *pFuseXfl = fusereader.fuseXfl();
    pFuseXfl->makeFuseGeometry();
    pFuseXfl->makeSurfaceTriangulation(50,30);
    pFuseXfl->saveBaseTriangulation();
    std::string logmsg;
    pFuseXfl->makeDefaultTriMesh(logmsg, "");

    m_pPlaneXfl->addFuse(pFuseXfl);

    setControls();
    updateData();

    onUpdatePlane();
    m_bChanged = true;
}


void PlaneXflDlg::onImportOtherFuse()
{
    ImportObjectDlg dlg(this, false);
    if(dlg.exec()==QDialog::Accepted)
    {
        Plane const*pPlane = Objects3d::planeAt(dlg.planeName().toStdString());
        if(!pPlane) return;
        for(int ifuse=0; ifuse<pPlane->nFuse(); ifuse++)
        {
            if(pPlane->fuseAt(ifuse)->name()==dlg.objectName().toStdString())
            {
                Fuse *pFuse = pPlane->fuseAt(ifuse)->clone();
                m_pPlaneXfl->addFuse(pFuse);
                setControls();
                updateData();

                onUpdatePlane();
                m_bChanged = true;
                break;
            }
        }
    }
}


void PlaneXflDlg::onImportOtherWing()
{
    ImportObjectDlg dlg(this, true);
    if(dlg.exec()==QDialog::Accepted)
    {
        Plane const*pPlane = Objects3d::planeAt(dlg.planeName().toStdString());
        if(!pPlane) return;
        for(int iw=0; iw<pPlane->nWings(); iw++)
        {
            QString wingname = dlg.objectName();
            WingXfl const *pWing= pPlane->wingAt(iw);
            if(pWing->name()==wingname.toStdString())
            {
                WingXfl *pNewWing = new WingXfl();
                pNewWing->duplicate(pWing);
                m_pPlaneXfl->addWing(pNewWing);
                setControls();
                updateData();

                onUpdatePlane();
                m_bChanged = true;
                break;
            }
        }
    }
}


void PlaneXflDlg::onInsertFuseOcc()
{
    double dimension(0);
    QString logmsg;
    std::string str;
    updateOutput("Importing CAD file...\n");

    QString filter = "CAD Files (*.brep *stp *step)";
    QString filename = QFileDialog::getOpenFileName(this, "CAD File", SaveOptions::CADDirName(), filter);
    if(!filename.length()) return;
    QFileInfo fi(filename);

    FuseOcc *pFuseOcc = new FuseOcc;
    pFuseOcc->setName(fi.baseName().toStdString());

    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool bImport = occ::importCADShapes(filename.toStdString(), pFuseOcc->shapes(), dimension, str);

    if(!bImport)
    {
        delete pFuseOcc;

        m_ppto->appendHtmlText("<font color=red>Error importing CAD file:</font> <br>"+QString::fromStdString(str)+"<br>");

        QApplication::restoreOverrideCursor();
        return;
    }
    updateStdOutput(str+"\n");

    m_pPlaneXfl->addFuse(pFuseOcc);

    m_ppto->appendPlainText("---Making shells from shapes-----\n\n");

    pFuseOcc->makeShellsFromShapes();

    if(pFuseOcc->shellCount()==0)
    {
        QString strange  = "<font color=red>Warning: </font>Imported SHAPE does not contain any SHELL: use the fuselage editor to fix the shapes.<br><br>";
        m_ppto->appendHtmlText(strange);
    }
    else
    {
        m_ppto->appendPlainText("---Making shell triangulation-----\n");
        std::string str;
        logmsg.clear();
        updateOutput("Making shell triangulation\n");
        gmesh::makeFuseTriangulation(pFuseOcc, logmsg, "   ");
        pFuseOcc->saveBaseTriangulation();
        pFuseOcc->computeSurfaceProperties(str, "   ");
        updateStdOutput(logmsg.toStdString() + str+"\n");
        m_ppto->appendPlainText("---updating plane-----\n");
        onUpdatePlane();
    }

    setControls();

    updateData();

    QApplication::restoreOverrideCursor();
    m_bChanged = true;
}


void PlaneXflDlg::onInsertFuseStl()
{
    StlReaderDlg dlg(this);
    if(dlg.exec()==QDialog::Rejected)
    {
        return;
    }

    if(dlg.triangleList().size()==0)
    {
        updateOutput("STL import: triangle list is empty\n");
        return; // nothing imported
    }

    updateOutput(dlg.logMsg());

    Fuse *pFuse = m_pPlaneXfl->makeNewFuse(Fuse::Stl);

    QString strong;
    strong = QString::asprintf("STL_type_fuse_%d", m_pPlaneXfl->stlFuseCount());
    pFuse->setName(strong.toStdString());
    pFuse->setBaseTriangles(dlg.triangleList());
    pFuse->setTriangles(dlg.triangleList());
    pFuse->makeTriangleNodes();
    pFuse->makeNodeNormals();

    std::string logmsg;
    updateOutput("   Making mesh panels from stl triangles\n");
    pFuse->makeDefaultTriMesh(logmsg, "");
    updateStdOutput(logmsg);

    setControls();

    updateData();

    onUpdatePlane();

    m_bChanged = true;
}


void PlaneXflDlg::onInsertSTLCylinderFuse()
{
    DoubleValueDlg dlg(this, {2.0*Units::mtoUnit(), 0.1*Units::mtoUnit(), 37, 31}, {"Length", "Radius", "Ny", "NHoop"},
                       {Units::lengthUnitQLabel(), Units::lengthUnitQLabel(), QString(), QString()});
    if(dlg.exec()!=QDialog::Accepted) return;

    double length = dlg.value(0)/Units::mtoUnit();
    double radius = dlg.value(1)/Units::mtoUnit();
    int ny = int(dlg.value(2));
    int nh = int(dlg.value(3));

    std::vector<Triangle3d> triangles;
    // use the AF mesher to mesh the lateral faces

    nh = std::max(nh, 3);
    ny = std::max(ny, 2);

    std::vector<Node>facepts(nh);

    double ymin = -length/2.0;
    double dy = length/double(ny);

    double theta = 2.0*PI/double(nh);
    QVector<Node> node((ny+1)*nh);
    int in = 0;
    for(int iy=0; iy<=ny; iy++)
    {
        double diy = double(iy);
        double dTh = (iy%2==0) ? 0.0 : theta/2.0;
        for(int ih=0; ih<nh; ih++)
        {
            double di = double(ih);
            node[in].setPosition(radius*cos(theta*di + dTh), ymin + diy*dy, radius*sin(theta*di+dTh));
            node[in].setNormal(node[in]);
            in++;
        }
        if(iy==0)
        {
            for(int i=0; i<nh; i++)
            {
                facepts[i] = node[i];
                facepts[i].setNormal(0.0,-1.0,0.0);
            }
        }
    }

    int it = 0;
    int nLA=0, nLB=0, nTA=0, nTB=0;
    triangles.resize(nh*ny*2);
    for(int i=0; i<ny; i++)
    {
        for(int h=0; h<nh; h++)
        {
            nLA = i*nh + (h+1)%nh;      // wrap around
            nLB = i*nh + (h+1)%nh + nh; // wrap around
            nTA = i*nh +  h;
            nTB = i*nh +  h+nh;
            if(i%2==0)
            {
                // left triangle
                triangles[it++].setTriangle(node.at(nLA), node.at(nTA), node.at(nTB));
                //right triangle
                triangles[it++].setTriangle(node.at(nLB), node.at(nLA), node.at(nTB));
            }
            else
            {
                // left triangle
                triangles[it++].setTriangle(node.at(nLA), node.at(nTA), node.at(nLB));
                //right triangle
                triangles[it++].setTriangle(node.at(nLB), node.at(nTA), node.at(nTB));
            }
        }
    }

    //left tip patch - no simple algorithm, using the advancing front mesher
    TopoDS_Face face;
    std::string strange;
    occ::makeFaceFromNodeStrip(facepts, face, strange);


    SLG3d slg;
    slg.resize(nh);
    for(uint i=0; i<facepts.size(); i++)
    {
        slg[i].setNodes(facepts.at(i), facepts.at((i+1)%nh));
    }

    AFMesher mesher;
    int maxpanelcount = 1000;
    double maxedgelength = radius*2.0*PI/double(nh);
    mesher.setMaxIterations(1000);
    std::vector<Triangle3d> facetrianglesleft, facetrianglesright;
    QString log;
    mesher.makeTriangles(face, slg, facetrianglesleft, maxedgelength, maxpanelcount, log);

    int nFlips=0, nIter=0;
    mesher.makeDelaunayFlips(facetrianglesleft, nFlips, nIter);

    facetrianglesright = facetrianglesleft;
    Vector3d O, axis(0.0,1.0,0.0);
    for(uint i=0; i<facetrianglesleft.size(); i++)
    {
        Triangle3d &t3 = facetrianglesright[i];
        t3.translate(0.0, length, 0.0);
        t3.normal().reverse();
        if((ny%2)==1) t3.rotate(O, axis, (theta/2.0)*180.0/PI);
        t3.reverseOrientation();
//        for(int in=0; in<3; in++) t3.nodeNormal(in).reverse();
    }
    triangles.insert(triangles.end(), facetrianglesleft.begin(), facetrianglesleft.end());
    triangles.insert(triangles.end(), facetrianglesright.begin(), facetrianglesright.end());

    Fuse *pFuse = m_pPlaneXfl->makeNewFuse(Fuse::Stl);
    QString strong;
    strong = QString::asprintf("Cylinder_STL_fuse_%d", m_pPlaneXfl->stlFuseCount());
    pFuse->setName(strong.toStdString());

    pFuse->setBaseTriangles(triangles);
    pFuse->setTriangles(triangles);
    pFuse->makeTriangleNodes();
    pFuse->makeNodeNormals();

    std::string logmsg;
    updateOutput("   Making mesh panels from stl triangles\n");
    pFuse->makeDefaultTriMesh(logmsg, "");
    updateStdOutput(logmsg);

    setControls();

    updateData();

    onUpdatePlane();

    m_bChanged = true;
}


void PlaneXflDlg::onInsertEllipticWing()
{
    DoubleValueDlg dlg(this, {0.3*Units::mtoUnit(), 3.0*Units::mtoUnit(), 15},
                             {"Root chord:", "Span:", "N Panels:"},
                             {Units::lengthUnitQLabel(), Units::lengthUnitQLabel(), QString()});
    if(dlg.exec()!=QDialog::Accepted)  return;

    double a = dlg.value(0) / Units::mtoUnit();       // root chord
    double b = dlg.value(1) / Units::mtoUnit() / 2.0; // half span
    int nsecs = int(dlg.value(2));
    nsecs = std::max(2, nsecs);    //at least two sections

    WingXfl *pWing = new WingXfl;
    pWing->clearWingSections();
    std::vector<double> frac;
    xfl::getPointDistribution(frac, nsecs, xfl::INV_EXP);
    for(uint i=0; i<frac.size(); i++)
    {
        double r = frac.at(i);
        double bs = b *r;
        double as = a* sqrt(1.0-bs*bs/b/b);
        as = std::max(as, a/20);
        pWing->appendWingSection(as, 0.0, bs, 0, (a-as)/2.0, 5, 1, xfl::COSINE, xfl::UNIFORM, std::string(), std::string());
    }

    if(!m_pPlaneXfl->hasMainWing()) pWing->setWingType(xfl::Main);
    else                            pWing->setWingType(xfl::OtherWing);
    m_pPlaneXfl->addWing(pWing);
    pWing->setVisible(true);

    updateData();
    onUpdatePlane();
}


void PlaneXflDlg::onInsertSTLSphereFuse()
{
    IntValueDlg dlg(this);
    dlg.setValue(2);
    dlg.setLeftLabel("Number of icosahedron splits (0<=n<=4)");
    if(dlg.exec()!=QDialog::Accepted) return;

    int nSplit = std::min(dlg.value(), 4);
    nSplit = std::max(0, nSplit);

    std::vector<Triangle3d> triangles;
    geom::makeSphere(1.0, nSplit, triangles);

    Fuse *pFuse = m_pPlaneXfl->makeNewFuse(Fuse::Stl);

    QString strong;
    strong = QString::asprintf("Sphere_STL_fuse_%d", m_pPlaneXfl->stlFuseCount());
    pFuse->setName(strong.toStdString());

    pFuse->setBaseTriangles(triangles);
    pFuse->setTriangles(triangles);
    pFuse->makeTriangleNodes();
    pFuse->makeNodeNormals();

    std::string logmsg;
    updateOutput("   Making mesh panels from STL triangles\n");
    pFuse->makeDefaultTriMesh(logmsg, "");
    updateStdOutput(logmsg);

    setControls();

    updateData();
    onUpdatePlane();

    m_bChanged = true;
}


void PlaneXflDlg::onInsertWingFromXml()
{
    QString path_to_file;
    path_to_file = QFileDialog::getOpenFileName(this,
                                                QString("Open XML File"),
                                                SaveOptions::xmlPlaneDirName(),
                                                "Plane XML file (*.xml)");
    QFile xmlfile(path_to_file);

    if (!xmlfile.open(QIODevice::ReadOnly))
    {
        QString strange = "<br><font color=red>Could not open the file:</font>"+xmlfile.fileName()+"<br><br>";
        m_ppto->appendHtmlText(strange);
        return ;
    }

    XmlWingReader wingreader(xmlfile);
    if(!wingreader.readXMLWingFile())
    {
        QString strong;
        QString errorMsg;
        errorMsg = "<br><font color=red>Failed to read the file "+xmlfile.fileName()+"</font><br>";
        strong = QString::asprintf(" at line %d column %d", int(wingreader.lineNumber()), int(wingreader.columnNumber()));
        errorMsg += wingreader.errorString() + strong +"<br>";
        m_ppto->appendHtmlText(errorMsg);
        return;
    }

    m_bChanged = true;

    WingXfl *pNewWing = wingreader.wing();

    if(!pNewWing) return;  //failsafe

    m_pPlaneXfl->addWing(pNewWing);

    updateData();
    onUpdatePlane();
}


void PlaneXflDlg::onInsertWingFromVSP()
{
    QStringList filters;
    filters << "CSV text file (*.csv)";

    QString pathname = QFileDialog::getOpenFileName(this,  "OpenVSP csv definition file",
                                                   SaveOptions::lastDirName(),
                                                   filters.front());

    if(pathname.isEmpty()) return;
    QFileInfo fi(pathname);

    if(!fi.exists() || fi.suffix().compare("csv", Qt::CaseInsensitive)!=0)
    {
        m_ppto->onAppendQText("Invalid CSV file - aborting\n");
        return;
    }

    QFile VSPFile(pathname);
    if(!VSPFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_ppto->onAppendQText("Could not read the file - aborting\n");
        return;
    }

    SaveOptions::setLastDirName(fi.absolutePath());

    QStringList wingnames;
    QTextStream instream(&VSPFile);

    QString strVSP = instream.readAll(); // need a copy to parse multiple times
    VSPFile.close();

    QTextStream stream(&strVSP);
    QString strange;
    do
    {
        strange = stream.readLine();
        if(strange.isNull()) break;
        if(strange.contains("Geom Name", Qt::CaseSensitive))
        {
            QStringList fields = strange.split(",");
            if(fields.count()>=2)
            {
                if(!wingnames.contains(fields.at(1)))
                    wingnames.append(fields.at(1));
            }
        }

    }while (!strange.isNull());


    m_ppto->onAppendQText(QString::asprintf("Found %d wings:\n", int(wingnames.size())));
    for(int i=0; i<wingnames.size(); i++)
    {
        m_ppto->onAppendQText(QString("   ")+wingnames.at(i)+"\n");
    }
/*    QVector<Wing*> wings;
    for(int i=0; i<wingnames.size(); i++)
    {
        wings.append(new WingXfl(xfl::OtherWing));
        wings.back()->setName(wingnames.at(i));
    }
*/

    //read all the foil names
    stream.seek(0);
    QStringList airfoilfilenames;
    do
    {
        strange = stream.readLine();
        if(strange.isNull()) break;
        if(strange.contains("Airfoil File Name", Qt::CaseSensitive))
        {
            QStringList fields = strange.split(",");
            if(fields.count()>=2)
            {
                if(!airfoilfilenames.contains(fields.at(1)))
                    airfoilfilenames.append(fields.at(1));
            }
        }

    }while (!strange.isNull());

    m_ppto->onAppendQText(QString::asprintf("Found %d airfoil files to load:\n", int(airfoilfilenames.size())));
    for(int i=0; i<airfoilfilenames.size(); i++)
    {
        m_ppto->onAppendQText(QString("   ")+airfoilfilenames.at(i)+"\n");
    }


    QStringList filter = {"*.dat"};
    QStringList files = xfl::findFiles(fi.absolutePath(), filter, false);
    for(int i=0; i<files.size(); i++)
    {
        Foil *pFoil = new Foil;
        if(Objects3d::readVSPFoilFile(files.at(i), pFoil))
            Objects2d::insertThisFoil(pFoil);
        else
            delete  pFoil;
    }

/*    do
    {
        strange = stream.readLine();
        if(strange.isNull()) break;
        if(strange.contains("########################################", Qt::CaseSensitive))
        {
            QString wingname;
            int index(0);
            WingSection *ws = new WingSection;
            readVSPSection(stream, wingname, index, ws);
        }

    }while (!strange.isNull());*/
}


void PlaneXflDlg::readVSPSection(QTextStream &stream, QString &wingname, int &index, WingSection &ws)
{
    QString strange;
    do
    {
        strange = stream.readLine();
        QStringList fields = strange.split(",");
        if(fields.count()>2)
        {
            if     (fields.front().contains("Geom Name"))          wingname = fields.at(1);
            else if(fields.front().contains("Airfoil Index"))      index = fields.at(1).toInt();
            else if(fields.front().contains("Leading Edge Point") && fields.size()==4)
            {
//                ws.setOffset(fields.at(1).toDouble(), fields.at(1).toDouble(), fields.at(3).toDouble());
            }
            else if(fields.front().contains("Airfoil File Name"))
            {
                ws.setLeftFoilName(fields.at(1).toStdString());
                ws.setRightFoilName(fields.at(1).toStdString());
            }
        }
    }
    while(!strange.contains("#######"));
}


void PlaneXflDlg::onInsertWing()
{
    if(!m_pPlaneXfl->hasMainWing()) m_pPlaneXfl->addWing(xfl::Main);
    else                            m_pPlaneXfl->addWing(xfl::OtherWing);

    updateData();
    onUpdatePlane();
}


void PlaneXflDlg::onInsertElevator()
{
    WingXfl *pWing = m_pPlaneXfl->addWing(xfl::Elevator);
    if(pWing)
    {
        pWing->setWingType(xfl::Elevator);
        pWing->setTwoSided(true);
        pWing->setName("Elevator");
    }
    int n = m_pPlaneXfl->nWings()-1;
    if(n>=0)
    {
        double span = 1.0;
        double chord = 0.2;
        if(m_pPlaneXfl->hasMainWing())
        {
            span = m_pPlaneXfl->planformSpan();
            chord = m_pPlaneXfl->mac();
        }
        if(pWing)
        {
            pWing->scaleChord(chord/2.0);
            pWing->scaleSpan(span/5.0);
        }
        m_pPlaneXfl->wing(n)->setRx(0.0);
        m_pPlaneXfl->wing(n)->setPosition(span/3.0, 0.0, 0.0);
    }

    updateData();

    onUpdatePlane();
}


void PlaneXflDlg::onInsertFin()
{
    WingXfl *pWing = m_pPlaneXfl->addWing(xfl::Fin);

    if(pWing)
    {
        pWing->setWingType(xfl::Fin);
        pWing->setTwoSided(false);
        pWing->setName("Fin");
    }
    int n = m_pPlaneXfl->nWings()-1;
    if(n>=0)
    {
        double span = 1.0;
        double chord = 0.2;
        if(m_pPlaneXfl->hasMainWing())
        {
            span = m_pPlaneXfl->planformSpan();
            chord = m_pPlaneXfl->mac();
        }
        if(pWing)
        {
            pWing->scaleChord(chord/2.0);
            pWing->scaleSpan(span/5.0);
        }
        m_pPlaneXfl->m_Wing[n].setRx(-90.0);
        m_pPlaneXfl->m_Wing[n].setPosition(span/3.0, 0.0, 0.0);
    }

    updateData();

    onUpdatePlane();
}


void PlaneXflDlg::onUpdatePlane()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    m_pPlaneXfl->makePlane(m_pPlaneXfl->isThickBuild(), false, true);

    onUpdatePlaneProps();

    m_pglPlaneView->setPlaneReferenceLength(m_pPlane);

    m_pglPlaneView->updatePartFrame(m_pPlane);

    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->resetgl3dPlane();
    m_pglPlaneView->update();

    QApplication::restoreOverrideCursor();
}


void PlaneXflDlg::onUpdateMesh()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    onClearHighlighted();

    m_pPlaneXfl->makeTriMesh(m_pPlaneXfl->isThickBuild());
    m_bChanged = true;

    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();
    onUpdatePlaneProps();

    QApplication::restoreOverrideCursor();
}


void PlaneXflDlg::onUpdateHighlightedPanels()
{
    Fuse *pFuse = m_pPlaneXfl->fuse(0);
    if(!pFuse) return;

    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();
}


void PlaneXflDlg::editWing(WingXfl *pWing, bool bAdvanced)
{
    if(!pWing) return;
    WingXfl modWing;
    modWing.duplicate(pWing);

    WingDlg *pWingDlg = nullptr;
    if(!bAdvanced) pWingDlg = new WingDefDlg(this);
    else           pWingDlg = new WingObjectDlg(this);
    pWingDlg->hideSaveAsNew();

    pWingDlg->initDialog(&modWing);

    if(pWingDlg->exec()==QDialog::Accepted)
    {
        m_bChanged |= pWingDlg->bChanged();
        m_bDescriptionChanged |= pWingDlg->bDescriptionChanged();
         pWing->duplicate(modWing);
         onUpdatePlane();
    }
//    else   pWing->duplicate(modWing);

    delete pWingDlg;
}


void PlaneXflDlg::editFuse(int iFuse, bool bAdvanced)
{
    Fuse *pFuse = m_pPlaneXfl->fuse(iFuse);

    if(pFuse->isSplineType() || pFuse->isFlatFaceType())
    {
        FuseXfl *pFuse = dynamic_cast<FuseXfl*>(m_pPlaneXfl->fuse(iFuse));
        Fuse* pMemBody = pFuse->clone();

        FuseXflDlg *pXflFuseDlg = nullptr;
        if(bAdvanced) pXflFuseDlg = new FuseXflObjectDlg(this);
        else          pXflFuseDlg = new FuseXflDefDlg(this);

        pXflFuseDlg->enableName(false);
        pXflFuseDlg->initDialog(pFuse);
        pXflFuseDlg->hideSaveAsNew();
        if(pXflFuseDlg->exec() != QDialog::Accepted)
        {
            pFuse->duplicateFuse(*pMemBody);
            delete pMemBody;
            delete pXflFuseDlg;
            return;
        }
        m_bChanged |= pXflFuseDlg->bChanged();
        m_bDescriptionChanged |= pXflFuseDlg->bDescriptionChanged();
        delete pMemBody;
        delete pXflFuseDlg;
    }
    else if(pFuse->isSectionType())
    {
        FuseSections memBody;
        memBody.duplicateFuse(*pFuse);
        FuseSections* pFuseSections = dynamic_cast<FuseSections*>(pFuse);

        FuseXflDlg *pXflFuseDlg = nullptr;
        pXflFuseDlg = new FuseXflDefDlg(this);

        pXflFuseDlg->enableName(false);
        pXflFuseDlg->initDialog(pFuseSections);
        pXflFuseDlg->hideSaveAsNew();
        if(pXflFuseDlg->exec() != QDialog::Accepted)
        {
            pFuseSections->duplicateFuse(memBody);
            delete pXflFuseDlg;
            return;
        }
        m_bChanged |= pXflFuseDlg->bChanged();
        m_bDescriptionChanged |= pXflFuseDlg->bDescriptionChanged();
        delete pXflFuseDlg;
    }
    else if(pFuse->isOccType())
    {
        FuseOcc *pOccBody = dynamic_cast<FuseOcc*>(pFuse);
        FuseOcc memBody(*pOccBody);
        FuseOccDlg obDlg(this);
        obDlg.hideSaveAsNew();
        obDlg.initDialog(pOccBody);
        if(obDlg.exec() != QDialog::Accepted)
        {
            pOccBody->duplicateFuse(memBody);
            return;
        }
        m_bChanged |= obDlg.bChanged();
        m_bDescriptionChanged |= obDlg.bDescriptionChanged();
    }
    else if(pFuse->isStlType())
    {
        FuseStl *pStlFuse = dynamic_cast<FuseStl*>(pFuse);
        FuseStl memFuseStl(*pStlFuse);
        FuseStlDlg sbDlg(this);
        sbDlg.hideSaveAsNew();

        sbDlg.initDialog(pStlFuse);
        if(sbDlg.exec() != QDialog::Accepted)
        {
            pStlFuse->duplicateFuse(memFuseStl);
            return;
        }
        m_bChanged |= sbDlg.bChanged();
        m_bDescriptionChanged |= sbDlg.bDescriptionChanged();
    }

/*    QString strange;
    if(pFuse) pFuse->makeDefaultTriMesh(strange, QString());*/

    onUpdatePlane();
}



void PlaneXflDlg::scaleWing(WingXfl *pWing)
{
    WingScaleDlg dlg(this);
    dlg.initDialog(pWing->planformSpan(),
                   pWing->chord(0),
                   pWing->averageSweep(),
                   pWing->twist(pWing->nSections()-1),
                   pWing->planformArea(),
                   pWing->aspectRatio(),
                   pWing->taperRatio());

    if(QDialog::Accepted == dlg.exec())
    {
        if (dlg.m_bSpan || dlg.m_bChord || dlg.m_bSweep || dlg.m_bTwist || dlg.m_bArea || dlg.m_bAR)
        {
            if(dlg.m_bSpan)  pWing->scaleSpan(dlg.m_NewSpan);
            if(dlg.m_bChord) pWing->scaleChord(dlg.m_NewChord);
            if(dlg.m_bSweep) pWing->scaleSweep(dlg.m_NewSweep);
            if(dlg.m_bTwist) pWing->scaleTwist(dlg.m_NewTwist);
            if(dlg.m_bArea)  pWing->scaleArea(dlg.m_NewArea);
            if(dlg.m_bAR)    pWing->scaleAR(dlg.m_NewAR);
            if(dlg.m_bTR)    pWing->scaleTR(dlg.m_NewTR);
        }

        m_bChanged = true;

        updateData();

        onUpdatePlane();
    }
}


void PlaneXflDlg::scaleFuse(Fuse*pFuse)
{
    if(!pFuse) return;

    DoubleValueDlg dlg(this, {1.0}, {"Scale factor:"}, {QString()});
    if(dlg.exec()==QDialog::Accepted)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        pFuse->scale(dlg.value(0), dlg.value(0), dlg.value(0));

        updateData();

        onUpdatePlane();

        m_bChanged = true;
        QApplication::restoreOverrideCursor();
    }
}


void PlaneXflDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save)    == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
    else if(pButton==m_ppbSaveAsNew)                                      onOK(10);
}


void PlaneXflDlg::onHighlightSel(bool bSel)
{
    gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->highlightSelectedPart(bSel);
    m_pglPlaneView->update();
}


void PlaneXflDlg::onNamesChanged()
{
    m_bDescriptionChanged = true;
    m_pglPlaneView->updatePartFrame(m_pPlane);
}


void PlaneXflDlg::onThinThick()
{
    m_pPlaneXfl->setThickBuild(m_prbThick->isChecked());
    onUpdatePlane();
    m_bChanged = true;
}


void PlaneXflDlg::onTabChanged(int iNewTab)
{
    onClearHighlighted();
    if(m_ppbMoveNode->isChecked())
    {
        m_ppbMoveNode->setChecked(false);
        onMergeNodes(false);
    }

    switch(iNewTab)
    {
        case 1:
        {
            onResizeColumns();
            break;
        }
        case 2:
        {
            QVector<WingXfl> const &thinwings = thinWingList(); // current selection
            QStringList wingnames;
            for(WingXfl const &wing : thinwings)  wingnames.push_back(QString::fromStdString(wing.name()));

            //fill the wing list - parts may have changed
            m_plwWings->clear();
            for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
            {
                m_plwWings->addItem(QString::fromStdString(m_pPlaneXfl->wing(iw)->name()));
            }

            //restore the existing selection
            for(int row=0; row<m_plwWings->count();row++)
            {
                QListWidgetItem *pItem = m_plwWings->item(row);
                if(pItem && wingnames.contains(pItem->text()))
                {
                    pItem->setSelected(true);
                }
            }

            break;
        }
        case 3:
        {            
            // initialize the flow5 mesher
            Fuse *pFuse =nullptr;

            if(!m_pPlaneXfl->isThickBuild())
            {
                QVector<WingXfl> const&thinwings = thinWingList(); // current selection
                m_ppto->onAppendQText("Conforming fuselage mesh to mid-camber lines of:\n");
                for(WingXfl const &wing : thinwings)
                {
                    m_ppto->onAppendQText("   " + QString::fromStdString(wing.name()) + EOLch);
                }
                m_ppto->appendEOL(2);
                m_pGMesherWt->setWings(thinwings);
            }
            else
            {
                QVector<WingXfl> const&thickwings = thinWingList(); // current selection
                m_ppto->onAppendQText("Conforming fuselage mesh to top and bottom lines of:\n");
                for(WingXfl const &wing : thickwings)
                {
                    m_ppto->onAppendQText("   " + QString::fromStdString(wing.name()) + EOLch);
                }
                m_ppto->appendEOL(2);
                m_pGMesherWt->setWings(thickwings);

/*                // get the fuse name currently selected in the assembly table, if any
                QListWidgetItem *pItem = m_plwFuseListWt->currentItem();
                if(pItem)
                {
                    QString fusename = pItem->text();
                    pFuse = m_pPlaneXfl->fuse(fusename.toStdString());
                }*/
            }

            if(!pFuse)
            {
                // get the first fuse, if any
                pFuse = m_pPlaneXfl->fuse(0);
            }

            if(!pFuse)
            {
                m_pGMesherWt->setEnabled(false);
            }
            else
            {
                m_pGMesherWt->setEnabled(true);
                m_pGMesherWt->initWt(pFuse, pFuse->isXflType(), m_pPlaneXfl->isThickBuild());

                AFMesher::setTraceFaceIndex(-1);
            }
            break;
        }
        case 4:
        {
            Fuse *pFuse =nullptr;

            // get the firtst fuse, if any
            pFuse = m_pPlaneXfl->fuse(0);

            m_UndoStack.clear();
            m_pMeshCorrectionsFrame->setEnabled(pFuse);
            if(pFuse) takePicture();

            break;
        }
    }

    m_ppbCheckMenuBtn->setChecked(false);
    m_ppbMoveNode->setChecked(false);
    m_ppbSelectPanels->setChecked(false);
    m_ppbMakeP3->setChecked(false);
    m_ppbMakeP3Strip->setChecked(false);

    m_pglPlaneView->clearSelectedParts();
    m_pglPlaneView->update();
}


QVector<WingXfl> PlaneXflDlg::thinWingList() const
{
    QVector<WingXfl> winglist;

    for(int row=0; row<m_plwWings->count(); row++)
    {
        QListWidgetItem *pItem = m_plwWings->item(row);
        if(pItem && pItem->isSelected())
        {
            WingXfl const *pWing = m_pPlaneXfl->wing(row);

            WingXfl ThinWing(*pWing);
            ThinWing.createSurfaces(ThinWing.position(), ThinWing.rx(), ThinWing.ry());

            for (int j=0; j<ThinWing.nSurfaces(); j++)
                ThinWing.surface(j).makeSideNodes(nullptr);

            winglist.push_back(ThinWing);
        }
    }

    return winglist;
}


void PlaneXflDlg::onThinListClick()
{
    QVector<WingXfl> winglist = thinWingList();
    m_pGMesherWt->setWings(winglist);

    if(m_pPlaneXfl->isThickBuild())
    {
        for(WingXfl const &wing : winglist)
        {
            if(!wing.isTwoSided())
            {
                if(wing.isClosedInnerSide())
                {
                    QString strange = "<font color=red>Warning:</font><i>"+
                             QString::fromStdString(wing.name()) +
                            "</i> is one-sided and closed at its inner section<br>";
                    m_ppto->appendHtmlText(strange);
                }
            }
        }
    }
}


/**
 * Unused in v7.54 - replaced by calls to gmsh API
 */
void PlaneXflDlg::onCutFuse()
{

    readParams();

    // reset the shells
    Fuse *pFuse = nullptr;
    Vector3d fusepos;

    if(!pFuse)
    {
        pFuse = m_pPlaneXfl->fuse(0);
        if(!pFuse)
        {
            updateOutput("No fuse selected.\n\n");
            return;
        }
    }

    TopTools_ListOfShape WingShapeList;
    WingShapeList.Clear();

    TopoDS_Shape WingShape;
    for(int iw=0; iw<m_plwWings->count(); iw++)
    {
        QListWidgetItem *pItem = m_plwWings->item(iw);
        if(pItem && pItem->isSelected())
        {
            std::string logmsg;
            WingXfl const *pWing = m_pPlaneXfl->wingAt(iw);

            // check foils
            std::vector<std::string> foilist;
            for(int is=0; is<pWing->nSections(); is++)
            {
                foilist.push_back(pWing->section(is).leftFoilName());
                foilist.push_back(pWing->section(is).rightFoilName());
            }
//            foilist.removeDuplicates();

            bool bOpen = false;
            for(uint ifoil=0; ifoil<foilist.size(); ifoil++)
            {
                Foil const *pFoil = Objects2d::foil(foilist.at(ifoil));
                if(!pFoil)
                {
                    logmsg += "   Missing foil" +foilist.at(ifoil)+" in wing " + pWing->name() + "\n";
                    logmsg += "   ... Aborting operation\n";
                    updateStdOutput(logmsg);
                    return;
                }

                if(fabs(pFoil->TEGap())>1.e-6)
                {
                    logmsg += "   The foil " + foilist.at(ifoil) + " has an open trailing edge\n";
                    bOpen = true;
                    break;
                }
            }
            if(bOpen)
            {
                logmsg += "   Cannot make a solid of wing " + pWing->name() + " due to its open trailing edge\n";
//                iWing++;
                updateStdOutput(logmsg);
                return;
            }

            occ::makeWingShape(pWing, s_StitchPrecision, WingShape, logmsg);
            updateStdOutput(logmsg);

            if(!pWing->isTwoSided() && pWing->isClosedInnerSide())
            {
                logmsg  = "   WARNING: " + pWing->name() + " is one-sided and has a closed right tip\n";
                logmsg += "            the resulting mesh may be self intersecting.\n\n";
                updateStdOutput(logmsg);
            }

            if(!WingShape.IsNull())
            {
                WingShapeList.Append(WingShape);
            }

//            iWing++;
        }
    }

    if(WingShapeList.Size()<1)
    {
        updateOutput("   No wing selected, no cut to perform\n");
        return;
    }

    //restore the shells before attempting to cut
    if(pFuse->isXflType())
    {
        std::string str;
        FuseXfl *pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);
        pFuseXfl->makeShape(str); // will build both the shapes and the shells
    }
    else
        pFuse->makeShellsFromShapes();

    if(pFuse->isXflType())
    {
        cutFuseXflRightShapes(pFuse, fusepos, WingShapeList); //saves 1/2 the cutting time
    }
    else
    {
        cutFuseShapes(pFuse, fusepos, WingShapeList);
    }

    if(pFuse->shells().Extent())
    {
    }
    else
    {
        updateOutput("Restoring the uncut shells\n");
        pFuse->makeShellsFromShapes();
    }

    std::string str;
    pFuse->makeDefaultTriMesh(str, ""); // just for xfl and stl

    m_pPlaneXfl->makeTriMesh(m_pPlaneXfl->isThickBuild());

    gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->resetgl3dFuse();
    m_pglPlaneView->update();
    m_bChanged = true;
}


bool PlaneXflDlg::makeFragments()
{
    if(!m_pPlaneXfl->hasMainWing()) return false;
    if(!m_pPlaneXfl->hasFuse()) return false;

    // build thin surface on wing mid-line
    WingXfl const &wing = *m_pPlaneXfl->mainWing();

    //Make the tools
    std::string logmsg;

    TopoDS_Shape SweptShape;
    if(!occ::makeWingSweepMidSection(&wing, SweptShape, logmsg))
        return false;

    TopTools_ListOfShape tools;
    tools.Append(SweptShape);


    // Time to fragment
    QString strong, strange;
    Fuse *pFuse = m_pPlaneXfl->fuse(0);

    if(pFuse->shapeCount()<=0)
    {
        updateOutput("   Fuse has no topology shapes to cut.\n");
        return false;
    }
    pFuse->makeShellsFromShapes();  /** @todo no need? */
    strange.clear();
    int iShell = 0;
    for(TopTools_ListIteratorOfListOfShape shellit(pFuse->shells()); shellit.More(); shellit.Next())
    {
        strange += QString::asprintf("   Shell %d:\n", iShell);
        std::string str;
        occ::listShapeContent(shellit.Value(), str, "      ");
        strange += QString::fromStdString(str);
        iShell++;
    }
//    updateOutput(strange+"\n");
qDebug("%s",strange.toStdString().c_str());


qDebug()<<"nshells="<<pFuse->shells().Extent();
qDebug()<<"nshapes="<<pFuse->shapes().Extent();

    updateOutput("Fragmenting body shape with selected wings...\n");

    // translate the wings by -fusepos
    if(fabs(pFuse->position().x)>LENGTHPRECISION || fabs(pFuse->position().y)>LENGTHPRECISION || fabs(pFuse->position().z)>LENGTHPRECISION)
    {
        gp_Trsf Translation;
        Translation.SetTranslation(gp_Vec(-pFuse->position().x, -pFuse->position().y, -pFuse->position().z));
        BRepBuilderAPI_Transform thetranslator(Translation);

        for(TopTools_ListIteratorOfListOfShape shapeit(tools); shapeit.More(); shapeit.Next())
        {
            thetranslator.Perform(shapeit.Value(), Standard_True);
            shapeit.Value() = thetranslator.Shape();
        }
    }

//    pFuse->clearShells();

    TopoDS_ListOfShape newShapes;
    for(TopTools_ListIteratorOfListOfShape shellit(pFuse->shapes()); shellit.More(); shellit.Next())
    {
        TopoDS_ListOfShape faces;
        TopExp_Explorer shellexplorer;
        for (shellexplorer.Init(shellit.Value(), TopAbs_SHELL); shellexplorer.More(); shellexplorer.Next())
        {
            faces.Append(shellexplorer.Current());
        }


        BRepAlgoAPI_Splitter aSplitter;
        aSplitter.SetArguments(faces);
        aSplitter.SetTools(tools); // or the shells maybe?
        aSplitter.SetRunParallel(true);
        aSplitter.Build();

        if (!aSplitter.IsDone())
        {
            return false;
        }

        TopoDS_Shape compound = aSplitter.Shape();

        newShapes.Append(compound);

std::string str;
occ::listShapeContent(compound, str, "", true);
qDebug("%s", str.c_str());

        TopExp_Explorer compexplorer;
        int iShell=0;

        for (compexplorer.Init(compound, TopAbs_SHELL); compexplorer.More(); compexplorer.Next())
        {
            strange = QString::asprintf("   Cutting shell %d\n", iShell+1);
            updateOutput(strange);

            if(compexplorer.Current().Orientation()==TopAbs_REVERSED)
                pFuse->appendShell(compexplorer.Current().Reversed());
            else
                pFuse->appendShell(compexplorer.Current());

            iShell++;
        }
    }

    // save the fragmented shapes
    pFuse->clearShapes();
    for(TopTools_ListIteratorOfListOfShape shapeit(newShapes); shapeit.More(); shapeit.Next())
    {
        pFuse->appendShape(shapeit.Value());
    }


    pFuse->clearTriangles();
    pFuse->clearTriangleNodes();


    QString logg;
    gmesh::makeFuseTriangulation(pFuse, logg, "   ");
    updateOutput(logg);

    strong = QString::asprintf("   New triangulation has %d elements\n", pFuse->nTriangles());
    updateOutput(strong+"\n\n");

    return true;
}


void PlaneXflDlg::cutFuseShapes(Fuse *pFuse, Vector3d const &fusepos, TopoDS_ListOfShape &tools)
{
    QString strong, strange;
    if(pFuse->shapeCount()<=0)
    {
        updateOutput("   Fuse has no topology shapes to cut.\n");
        return;
    }
    pFuse->makeShellsFromShapes();  /** @todo no need? */

    QApplication::setOverrideCursor(Qt::WaitCursor);
    updateOutput("Cutting body shape with selected wings...\n");

    // make cutting list

    // translate the wings by -fusepos
    if(fabs(fusepos.x)>LENGTHPRECISION || fabs(fusepos.y)>LENGTHPRECISION || fabs(fusepos.z)>LENGTHPRECISION)
    {
        gp_Trsf Translation;
        Translation.SetTranslation(gp_Vec(-fusepos.x, -fusepos.y, -fusepos.z));
        BRepBuilderAPI_Transform thetranslator(Translation);

        for(TopTools_ListIteratorOfListOfShape shapeit(tools); shapeit.More(); shapeit.Next())
        {
            thetranslator.Perform(shapeit.Value(), Standard_True);
            shapeit.Value() = thetranslator.Shape();
        }
    }

    pFuse->clearShells();

    QElapsedTimer t; t.start();
    // BRepAlgoAPI_Cut sometimes fails when there is more than one shape in the arguments
    for(TopTools_ListIteratorOfListOfShape shellit(pFuse->shapes()); shellit.More(); shellit.Next())
    {
//        TopoDS_ListOfShape singleshell;
//        singleshell.Append(shellit.Value());

        TopoDS_ListOfShape faces;
        TopExp_Explorer shellexplorer;
        for (shellexplorer.Init(shellit.Value(), TopAbs_SHELL); shellexplorer.More(); shellexplorer.Next())
        {
            faces.Append(shellexplorer.Current());
        }

        BRepAlgoAPI_Cut theKnife;
        theKnife.SetArguments(faces);
        theKnife.SetTools(tools);
        theKnife.SetRunParallel(true);
//        theKnife.SetFuzzyValue(1.0e-4);
        strange = QString::asprintf("   Using fuzzy value %g ", theKnife.FuzzyValue()*Units::mtoUnit());
        strange += Units::lengthUnitQLabel() + "\n";
        updateOutput(strange);

        // run the algorithm
        theKnife.Build();

        if (theKnife.HasErrors() || !theKnife.IsDone())
        {
            updateOutput("Error cutting shape with wings\n");
            QApplication::restoreOverrideCursor();
            return;
        }

        TopoDS_Shape compound = theKnife.Shape();

        TopExp_Explorer compexplorer;
        int iShell=0;

        for (compexplorer.Init(compound, TopAbs_SHELL); compexplorer.More(); compexplorer.Next())
        {
            strange = QString::asprintf("   Cutting shell %d\n", iShell+1);
            updateOutput(strange);

            if(compexplorer.Current().Orientation()==TopAbs_REVERSED)
                pFuse->appendShell(compexplorer.Current().Reversed());
            else
                pFuse->appendShell(compexplorer.Current());

            iShell++;
        }
    }

    strange = QString::asprintf("   Time to cut the fuse: %.3f s\n\n", double(t.elapsed())/1000.0);
    updateOutput(strange);

    strong = QString::asprintf("   Cut operation has produced %d shell(s)\n\n",pFuse->shells().Extent());
    updateOutput(strong);
    int iShell=0;

    strange.clear();
    for(TopTools_ListIteratorOfListOfShape shellit(pFuse->shells()); shellit.More(); shellit.Next())
    {
        strange += QString::asprintf("   Shell %d:\n", iShell);
        std::string str;
        occ::listShapeContent(shellit.Value(), str, "      ");
        strange += QString::fromStdString(str);
        iShell++;
    }
    updateOutput(strange+"\n");


    pFuse->clearTriangles();
    pFuse->clearTriangleNodes();

    QString logmsg;

    gmesh::makeFuseTriangulation(pFuse, logmsg, "   ");
    updateOutput(logmsg);

    strong = QString::asprintf("   New triangulation has %d elements\n", pFuse->nTriangles());
    updateOutput(strong+"\n\n");

    QApplication::restoreOverrideCursor();
}


/**
 * In the case of an xfl type fuse, only cut the right shells and duplicate.
 * Saves significant cutting time.
 */
void PlaneXflDlg::cutFuseXflRightShapes(Fuse *pFuse, Vector3d const &fusepos, TopoDS_ListOfShape &tools)
{
    QString strong, strange;
    FuseXfl *pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);

    QString logmsg;
    // remake the shells which may have been modified by a previous cut
//    logmsg = "Making fuse shape\n";
//    pFuseXfl->makeShape(logmsg);
//    updateStdOutput(logmsg+"\n");
//    logmsg .clear();

    int nshells = pFuseXfl->m_RightSideShell.Size();
    if(nshells<0)
    {
        updateOutput("   Fuse has no TopoDS_Shell to cut.\n");
        return;
    }

/*    strange = QString::asprintf("   Fuse half-side is made of %d shells\n", nshells);
    updateAssyOutput(strange);

    int ishell=0;
    for(TopTools_ListIteratorOfListOfShape shapeit(pFuseXfl->rightSideShells()); shapeit.More(); shapeit.Next())
    {
        strange = QString::asprintf("   Sub-shell %d:\n", ishell);
        listShapeContent(shapeit.Value(), strange, "      ");
        updateAssyOutput(strange);
        ishell++;
    }
    updateAssyOutput("\n");*/

    QApplication::setOverrideCursor(Qt::WaitCursor);
    updateOutput("Cutting fuse shape with selected wings...");

    // make cutting list
    TopTools_ListOfShape toollist;
    TopTools_ListIteratorOfListOfShape itwing;

    // translate the wings by -fusepos
    if(fabs(fusepos.x)>LENGTHPRECISION || fabs(fusepos.y)>LENGTHPRECISION || fabs(fusepos.z)>LENGTHPRECISION)
    {
        gp_Trsf Translation;
        Translation.SetTranslation(gp_Vec(-fusepos.x, -fusepos.y, -fusepos.z));
        BRepBuilderAPI_Transform thetranslator(Translation);

        for(TopTools_ListIteratorOfListOfShape shapeit(tools); shapeit.More(); shapeit.Next())
        {
            thetranslator.Perform(shapeit.Value(), Standard_True);
            shapeit.Value() = thetranslator.Shape();
        }
    }


    BRepAlgoAPI_Cut theKnife;
    theKnife.SetRunParallel(true);

//    double fuzz = theKnife.FuzzyValue();
//    theKnife.SetFuzzyValue(1.e-4);
    theKnife.SetArguments(pFuseXfl->rightSideShells());
    theKnife.SetTools(tools);

    QElapsedTimer t; t.start();

    // run the algorithm
    theKnife.Build();

    if (theKnife.HasErrors() || !theKnife.IsDone())
    {
        updateOutput("Error cutting shape with wings\n");
        QApplication::restoreOverrideCursor();
        return;
    }

    updateOutput("   DONE\n");
    strange = QString::asprintf("   Time to cut the fuse: %.3f s\n\n", double(t.elapsed())/1000.0);
    updateOutput(strange);

    TopoDS_Shape compound = theKnife.Shape();
    TopExp_Explorer compexplorer;
    int iShell=0;
    pFuseXfl->clearShells();
    pFuseXfl->clearRightShells();

    for (compexplorer.Init(compound, TopAbs_SHELL); compexplorer.More(); compexplorer.Next())
    {
        strange = QString::asprintf("   shell %d content:\n", iShell+1);
        updateOutput(strange);

        TopoDS_Shell rightsideshell = TopoDS::Shell(compexplorer.Current());
        pFuseXfl->appendRightSideShell(rightsideshell);
        std::string str;
        occ::listShapeContent(compexplorer.Current(), str, "      ");
        updateStdOutput(str);
        iShell++;
    }

    strong = QString::asprintf("   Cut operation has produced %d shell(s)\n\n",iShell);
    updateOutput(strong);

//    pFuseXfl->makeShellTriangulation(logmsg, "   ");

    gmesh::makeFuseTriangulation(pFuse, logmsg, "   ");

    updateOutput(logmsg);

    strong = QString::asprintf("   The new fuse tessellation has %d elements\n", pFuseXfl->nTriangles());
    strong += "\n______\n\n";
    updateOutput(strong);

    QApplication::restoreOverrideCursor();
}


void PlaneXflDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("PlaneXflDlg");
    {
        s_XflGeometry       = settings.value("Geometry", QByteArray()).toByteArray();
        s_HSplitterSizes    = settings.value("HSplitterSizes").toByteArray();
        s_PartSplitterSizes = settings.value("VSplitterSizes").toByteArray();

        s_StitchPrecision   = settings.value("StitchPrecision",   s_StitchPrecision  ).toDouble();
        s_NodeMergeDistance = settings.value("NodeMergeDistance", s_NodeMergeDistance).toDouble();

    }
    settings.endGroup();
}


void PlaneXflDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("PlaneXflDlg");
    {
        settings.setValue("Geometry",          s_XflGeometry);

        settings.setValue("HSplitterSizes",    s_HSplitterSizes);
        settings.setValue("VSplitterSizes",    s_PartSplitterSizes);

        settings.setValue("StitchPrecision",   s_StitchPrecision);
        settings.setValue("NodeMergeDistance", s_NodeMergeDistance);

    }
    settings.endGroup();
}



void PlaneXflDlg::onInsertFuseXfl()
{
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    Fuse::enumType type = Fuse::NURBS;
    if     (pSenderAction==m_pInsertFuseXflFlat)     type=Fuse::FlatFace;
    else if(pSenderAction==m_pInsertFuseXflSections) type=Fuse::Sections;

    Fuse *pFuse = m_pPlaneXfl->makeNewFuse(type);

    QString strong;
    strong = QString::asprintf("xfl_fuse_%d", m_pPlaneXfl->xflFuseCount());
    pFuse->setName(strong.toStdString());
    pFuse->makeFuseGeometry();

    QString logg;
    gmesh::makeFuseTriangulation(pFuse, logg, "   ");
    pFuse->saveBaseTriangulation();

    std::string logmsg;
    pFuse->makeDefaultTriMesh(logmsg, "");

    setControls();

    updateData();

    onUpdatePlane();

    m_bChanged = true;
}


void PlaneXflDlg::onTessellation()
{
    TessControlsDlg dlg(this);
    Fuse *pFuse = activeFuse();
    if(!pFuse) return;

    dlg.initDialog(pFuse->occTessParams(), pFuse->gmshTessParams(),  Part::isOccTessellator(), true);
    if(dlg.exec()==QDialog::Accepted)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        updateOutput("Making new fuse tessellation\n");
        Part::setOccTessellator(dlg.bOcc());
        pFuse->setOccTessParams(dlg.occParameters());
        pFuse->setGmshTessParams(dlg.gmshParameters());
        m_bChanged = true;

        QString strange;
        gmesh::makeFuseTriangulation(pFuse, strange, "   ");
        pFuse->saveBaseTriangulation();
        updateOutput(strange);

        gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
        pglPlaneXflView->resetgl3dFuse();
        m_pglPlaneView->update();

        QApplication::restoreOverrideCursor();
    }
}


void PlaneXflDlg::onFlipNormals()
{
    Fuse *pFuse = activeFuse();
    if(!pFuse) return;
    pFuse->triangulation().flipNormals();
    gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->resetgl3dFuse();

    m_pglPlaneView->update();
}


void PlaneXflDlg::updateData()
{
    m_pPartModel->updateData();

    QModelIndex index = m_pPartModel->index(m_pPlaneXfl->nWings(), 0);
    m_pcptParts->setCurrentIndex(index);
}


void PlaneXflDlg::onEditPart()
{
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    bool bAdvanced = false;
//    if     (pSenderAction == m_pEditPartDef)    bAdvanced = false;
//    else if(pSenderAction == m_pEditPartObject) bAdvanced = true;

    int row = selectedPart();

    if(row<m_pPlaneXfl->nWings())
    {
        WingXfl *pWing = m_pPlaneXfl->wing(row);
        editWing(pWing, bAdvanced);
    }
    else if(row>=m_pPlaneXfl->nWings())
    {
        int iFuse = row-m_pPlaneXfl->nWings();
        editFuse(iFuse, bAdvanced);
    }
}


void PlaneXflDlg::onResetFuse()
{
    std::string strange;
    Fuse *pFuse = nullptr;

    int row = selectedPart();

    if(row<m_pPlaneXfl->nWings())
    {
        // not applicable
    }
    else if(row>=m_pPlaneXfl->nWings())
    {
        int iFuse = row-m_pPlaneXfl->nWings();
        pFuse = m_pPlaneXfl->fuse(iFuse);
    }

    if(!pFuse)
    {
        updateOutput("_nNo fuse selected, no reset possible.\n\n");
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if(pFuse->isXflType())
    {
        FuseXfl *pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);
        pFuseXfl->makeShape(strange); // will build both the shapes and the shells
    }
    else
        pFuse->makeShellsFromShapes();

    pFuse->makeFuseGeometry();

    pFuse->makeDefaultTriMesh(strange, "");
    m_pPlaneXfl->makeTriMesh(m_pPlaneXfl->isThickBuild());
    gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->resetgl3dFuse();
    QString str;
    gmesh::makeFuseTriangulation(pFuse, str, "   ");
    pFuse->saveBaseTriangulation();

    strange = "The fuse " + pFuse->name() + " has been reset.\n";
    updateStdOutput(strange+"\n");

    m_bChanged = true;
    m_pglPlaneView->update();
    QApplication::restoreOverrideCursor();
}


void PlaneXflDlg::onRemovePart()
{
    int row = selectedPart();
    if (row<0 || row>=m_pPlaneXfl->nParts()) return;

    int iFuse = row-m_pPlaneXfl->nWings();
    QString partname;
    if(row>=0 && row<m_pPlaneXfl->nWings())
    {
        partname = QString::fromStdString(m_pPlaneXfl->wing(row)->name());
    }
    else if(row>=m_pPlaneXfl->nWings() && m_pPlaneXfl->hasFuse())
    {
        partname = QString::fromStdString(m_pPlaneXfl->fuse(iFuse)->name());
    }

    if(row>=0 && row<m_pPlaneXfl->nWings())
    {
        m_pPlaneXfl->removeWing(row);
    }
    else if(row>=m_pPlaneXfl->nWings())
    {
        m_pPlaneXfl->removeFuse(iFuse);
    }

    m_pPartMenu->setTitle(tr("Selected part"));

    setControls();
    onUpdatePlane();

    m_pPartModel->updateData();
    m_bChanged = true;
}


void PlaneXflDlg::onMovePartDown()
{
    int row = selectedPart();

    //switch row and row+1;
    if(row<m_pPlaneXfl->nWings())
    {
        if(row==m_pPlaneXfl->nWings()-1) return;
        m_pPlaneXfl->swapWings(row, row+1);
    }
    else if(row<m_pPlaneXfl->nFuse())
    {
        if(row==m_pPlaneXfl->nFuse()-1) return;
        int iFuse=row-m_pPlaneXfl->nWings();
        m_pPlaneXfl->swapFuses(iFuse, iFuse+1);
    }

    m_pPartModel->updateData();
    onUpdatePlane();
    m_bChanged = true;
}


void PlaneXflDlg::onMovePartUp()
{
    int row = selectedPart();
    if(row==0) return;

    //switch row and row-1;
    if(row<m_pPlaneXfl->nWings())
    {
        m_pPlaneXfl->swapWings(row, row-1);
    }
    else
    {
        int iFuse=row-m_pPlaneXfl->nWings();
        if(iFuse>0) m_pPlaneXfl->swapFuses(iFuse, iFuse-1);
    }

    m_pPartModel->updateData();
    onUpdatePlane();
    m_bChanged = true;
}


void PlaneXflDlg::onResizeColumns()
{
    double w = double(m_pcptParts->width())/100.0;
    int wtype  = int(12.0*w);
    int wname  = int(15.0*w);
    int wdist  = int(10.0*w);

    m_pcptParts->setColumnWidth(0, wtype);
    m_pcptParts->setColumnWidth(1, wname);
    m_pcptParts->setColumnWidth(2, wdist);
    m_pcptParts->setColumnWidth(3, wdist);
    m_pcptParts->setColumnWidth(4, wdist);
    m_pcptParts->setColumnWidth(5, wdist);
    m_pcptParts->setColumnWidth(6, wdist);
    m_pcptParts->horizontalHeader()->setStretchLastSection(true);
}


void PlaneXflDlg::onPartItemClicked(QModelIndex index)
{
    if(!index.isValid())
    {
    }
    else
    {
        if(!m_pPlane) return;
        std::string strange;
        int row = index.row();
        bool bShift = (QApplication::queryKeyboardModifiers() & Qt::SHIFT);
        if(!bShift) m_pglPlaneView->clearSelectedParts();
        if(row<m_pPlaneXfl->nWings())
        {
            WingXfl const *pWing = m_pPlaneXfl->wing(row);
            if(pWing)
            {
                updateOutput("\n" + QString::fromStdString(pWing->name()) + "\n");

                pWing->getProperties(strange, "   ");
                updateStdOutput(strange + "\n");
                m_pglPlaneView->setSelectedPart(pWing->uniqueIndex());
            }
            else      m_pglPlaneView->setSelectedPart(-1);
        }
        else if(row>=m_pPlaneXfl->nWings())
        {
            int iFuse = row-m_pPlaneXfl->nWings();
            Fuse *pFuse = m_pPlaneXfl->fuse(iFuse);
            if(pFuse)
            {
                updateStdOutput("\n" + pFuse->name() + "\n");
                pFuse->getProperties(strange, "   ");
                updateStdOutput(strange + "\n");
                m_pglPlaneView->setSelectedPart(pFuse->uniqueIndex());
            }
            else      m_pglPlaneView->setSelectedPart(-1);
        }
        m_pglPlaneView->update();

        if(index.column()==7)
        {
            m_pcptParts->selectRow(index.row());
            QRect itemrect = m_pcptParts->visualRect(index);
            QPoint menupos = m_pcptParts->mapToGlobal(itemrect.topLeft());
            m_pPartMenu->setEnabled(index.row()>=0);
            m_pResetFuse->setEnabled(index.row()>=m_pPlaneXfl->nWings());
            m_pFlipNormals->setEnabled(index.row()>=m_pPlaneXfl->nWings());
            m_pTessellation->setEnabled(index.row()>=m_pPlaneXfl->nWings());
            m_pPartMenu->exec(menupos, m_pEditPartDef);
        }
    }
}


void PlaneXflDlg::onSelectPart(Part *pPart)
{
    if(pPart->isWing())
    {
        for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
        {
            if(m_pPlaneXfl->wing(iw)==pPart)
            {
                m_pcptParts->selectRow(iw);
                m_plwWings->setCurrentRow(iw);
            }
        }
    }
    else if(pPart->isFuse())
    {
        for(int ifuse=0; ifuse<m_pPlaneXfl->nFuse(); ifuse++)
        {
            if(m_pPlaneXfl->fuse(ifuse)==pPart)
            {
                m_pcptParts->selectRow(m_pPlaneXfl->nWings()+ifuse);
            }
        }
    }
}


void PlaneXflDlg::onPartDataChanged()
{
    m_bChanged = true;

    gl3dPlaneXflView*pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->resetgl3dFuse();
    pglPlaneXflView->resetgl3dPlane();

    onUpdatePlane();
}


int PlaneXflDlg::selectedPart()
{
    QModelIndexList indexList = m_pcptParts->selectionModel()->selectedIndexes();

    int row = -1;
    if(indexList.size()>0) row = indexList.at(0).row();
    return row;
}


void PlaneXflDlg::onPartInertia()
{
    int row = selectedPart();

    if(row<m_pPlaneXfl->nWings())
    {
        PartInertiaDlg dlg(m_pPlaneXfl->wing(row), nullptr, this);
        if(dlg.exec()==QDialog::Accepted) m_bChanged=true;
    }
    else if(m_pPlaneXfl->fuse(0))
    {
        Fuse*pFuse = m_pPlaneXfl->fuse(0);
        PartInertiaDlg dlg(nullptr, pFuse, this);

        if(dlg.exec()==QDialog::Accepted) m_bChanged=true;
    }

    m_pPartModel->updateData();
    m_pglPlaneView->update();
}


void PlaneXflDlg::onDuplicatePart()
{
    int row = selectedPart();
    if(row<m_pPlaneXfl->nWings())
    {
        m_pPlaneXfl->duplicateWing(row);
    }
    else
    {
        m_pPlaneXfl->duplicateFuse(row-m_pPlaneXfl->nWings());
    }
    m_pPartModel->updateData();

    onUpdatePlane();
    m_bChanged = true;
}


void PlaneXflDlg::onScalePart()
{
    int row = selectedPart();

    if(row<m_pPlaneXfl->nWings())
    {
        scaleWing(m_pPlaneXfl->wing(row));
    }
    else
    {
        scaleFuse(m_pPlaneXfl->fuse(0));
    }
}


Fuse *PlaneXflDlg::activeFuse()
{
    Fuse *pFuse=nullptr;
    if(m_pLeftTabWidget->currentIndex()==1)
    {
        int row = m_pcptParts->currentIndex().row();
        if(row<m_pPlaneXfl->nWings()) return nullptr;
        else pFuse = m_pPlaneXfl->fuse(row-m_pPlaneXfl->nWings());
    }

    return pFuse;
}


void PlaneXflDlg::makePartTable()
{
    m_pcptParts = new CPTableView(this);
    m_pcptParts->setEditable(true);
    m_pcptParts->setWindowTitle(tr("Object List"));
    m_pcptParts->setWordWrap(false);
    m_pcptParts->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QHeaderView *pVHeader = m_pcptParts->verticalHeader();
    pVHeader->setDefaultSectionSize(m_pcptParts->fontHeight()*2);
    pVHeader->setSectionResizeMode(QHeaderView::Fixed);

    m_pPartModel = new PlanePartModel(this);
    m_pcptParts->setModel(m_pPartModel);

    m_pPartDelegate = new PlanePartDelegate(this);
    m_pcptParts->setItemDelegate(m_pPartDelegate);

    m_pPartDelegate->setPrecision({0,0,3,3,3,2,2,0});
}


void PlaneXflDlg::onClearOutput()
{
    m_ppto->clear();
}


void PlaneXflDlg::onResetFuseMesh()
{
    onClearHighlighted();

    if(!m_pPlaneXfl->fuse(0)) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    std::string logmsg;
    m_pPlaneXfl->fuse(0)->makeDefaultTriMesh(logmsg, "   ");
    updateStdOutput(logmsg);

    m_pPlaneXfl->makeTriMesh(m_pPlaneXfl->isThickBuild());

    QString strange;
    strange = QString::asprintf("\nTriangle count=%d\n", m_pPlaneXfl->nPanel3());
    updateOutput(strange);
    strange = QString::asprintf(  "Node count    =%d\n", m_pPlaneXfl->nNodes());
    strange += "\n_______\n\n";
    updateOutput(strange);

    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();
    onUpdatePlaneProps();

    m_bChanged = true;
    QApplication::restoreOverrideCursor();
}


void PlaneXflDlg::onFuseMeshDlg()
{
    onClearHighlighted();

    if(!m_pPlaneXfl->fuse(0)) return;

    FuseMesherDlg *pFMDlg = new FuseMesherDlg(this);
    pFMDlg->initDialog(m_pPlaneXfl->fuse(0));
    if(pFMDlg->exec()!=QDialog::Accepted)
    {
        return;
    }

    m_pPlaneXfl->makeTriMesh(m_pPlaneXfl->isThickBuild());

    QString strange;
    strange = QString::asprintf("\nTriangle count=%d\n", m_pPlaneXfl->nPanel3());
    updateOutput(strange);
    strange = QString::asprintf(  "Node count    =%d\n", m_pPlaneXfl->nNodes());
    strange += "\n_______\n\n";
    updateOutput(strange);

    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();
    onUpdatePlaneProps();

    m_bChanged = true;
}


/**
 * Exports the part's triangular mesh to an STL file
 */
void PlaneXflDlg::onExportMeshToSTLFile()
{
    if(!m_pPlane) return;
    /** @todo */
//    exportMeshToSTLFile(m_pPlaneXfl->triMesh(), m_pPlaneXfl->name(), SaveOptions::STLDirName(), Units::mtoUnit());
}


void PlaneXflDlg::onPickedNodePair(QPair<int, int> nodepair)
{
    if(m_pglControls->getDistance())
    {
        if(nodepair.first <0 || nodepair.first >=m_pPlane->refTriMesh().nPanels()) return;
        if(nodepair.second<0 || nodepair.second>=m_pPlane->refTriMesh().nPanels()) return;
        Node nsrc  = m_pPlane->refTriMesh().node(nodepair.first);
        Node ndest = m_pPlane->refTriMesh().node(nodepair.second);

        Segment3d seg(nsrc, ndest);
        m_pglPlaneView->setMeasure(seg);

        QString strange = QString::asprintf("Node %d to node %d\n", nodepair.first, nodepair.second);
        strange += QString::asprintf("   distance = %13g ", seg.length()*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n";
        strange += QString::asprintf("         dx = %13g ", seg.segment().x*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n";
        strange += QString::asprintf("         dy = %13g ", seg.segment().y*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n";
        strange += QString::asprintf("         dz = %13g ", seg.segment().z*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n\n";
        updateOutput(strange);

        m_pglPlaneView->resetPickedNodes();
        m_pglPlaneView->update();
        return;
    }

    if(!m_ppbMoveNode->isChecked())
    {
        return;
    }

    if(!m_pPlaneXfl->hasFuse()) return;
    Fuse *pFuse = m_pPlaneXfl->fuse(0);

    // The node pair is given in plane indexes, convert to fuse indexes
    nodepair.first  -= pFuse->firstNodeIndex();
    nodepair.second -= pFuse->firstNodeIndex();

    if (nodepair.first<0 || nodepair.first>=pFuse->triMesh().nNodes() || nodepair.second<0 || nodepair.second>=pFuse->triMesh().nNodes())
    {
        m_ppto->onAppendQText("*****Internal index error picking nodes*****\n");
        return;
    }

    Node nsrc  = pFuse->triMesh().node(nodepair.first);
    Node ndest = pFuse->triMesh().node(nodepair.second);

    QString strange;
    strange = QString::asprintf("Moving fuselage node %d to location of node %d and merging\n", nodepair.first, nodepair.second);
    updateOutput(strange);

    // take a picture
    m_MeshStack.push(pFuse->triMesh());

    std::string log;
    QApplication::setOverrideCursor(Qt::WaitCursor);

    pFuse->triMesh().mergeNodes(nodepair.first, nodepair.second, true, log, "   ");

    // remove the segments from the free edge list
    QVector<Segment3d> &freedges = m_pglPlaneView->segments();
    for(int iseg=freedges.size()-1; iseg>=0; iseg--)
    {
        Segment3d &seg = freedges[iseg];
        if(seg.vertex(0).isSame(nsrc) || seg.vertex(1).isSame(nsrc) || seg.vertex(0).isSame(ndest) || seg.vertex(1).isSame(ndest))
        {
            freedges.removeAt(iseg);
        }
    }

    //reset indexes, triangles have been reordered
    for(int i3=0; i3<pFuse->nPanel3(); i3++)
    {
        Panel3 &p3 = pFuse->triMesh().panel(i3);
        p3.setIndex(i3);
        Q_ASSERT(p3.isFusePanel());
    }

    std::string str;
    pFuse->triMesh().makeNodeArrayFromPanels(0, str, "  ");
    takePicture();

    onUpdateMesh();

    QApplication::restoreOverrideCursor();
    updateStdOutput(log);

    m_pglPlaneView->resetPickedNodes();

    m_pglPlaneView->update();

    m_bChanged = true;
}


void PlaneXflDlg::onScalePlane()
{
    QStringList labels("Scale factor:");
    QStringList rightlabels("");
    QVector<double> vals({1});
    DoubleValueDlg dlg(this, vals, labels, rightlabels);
    if(dlg.exec()!=QDialog::Accepted) return;

    m_pPlane->scale(dlg.value(0));

    m_bChanged = true;
    onUpdatePlane();
}


void PlaneXflDlg::onInsertCADShape()
{
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    Fuse *pFuseOcc = m_pPlaneXfl->makeNewFuse(Fuse::Occ);
    QString strong = QString::asprintf("CAD_shape_%d", m_pPlaneXfl->stlFuseCount());
    pFuseOcc->setName(strong.toStdString());

    if(pSenderAction==m_pInsertCADCylinder)
    {
        DoubleValueDlg dlg(this, {2.0*Units::mtoUnit(), 0.1*Units::mtoUnit()}, {"Length", "Radius"},
                           {Units::lengthUnitQLabel(), Units::lengthUnitQLabel()});
        if(dlg.exec()!=QDialog::Accepted) return;

        double length = dlg.value(0)/Units::mtoUnit();
        double radius = dlg.value(1)/Units::mtoUnit();

        BRepPrimAPI_MakeCylinder maker(radius, length);
        maker.Build();
        if(!maker.IsDone() || maker.Shape().IsNull())
        {
            delete pFuseOcc;
            m_ppto->appendHtmlText("<font color=red>Error making cylinder</font><br><br>");
            return;
        }
        pFuseOcc->appendShape(maker.Shape());
    }
    else if(pSenderAction==m_pInsertCADSphere)
    {
        DoubleValueDlg dlg(this, {1.0*Units::mtoUnit()}, {"Radius"}, {Units::lengthUnitQLabel()});
        if(dlg.exec()!=QDialog::Accepted) return;

        double radius = dlg.value(0)/Units::mtoUnit();

        BRepPrimAPI_MakeSphere maker(radius);
        maker.Build();
        if(!maker.IsDone() || maker.Shape().IsNull())
        {
            delete pFuseOcc;
            m_ppto->appendHtmlText("<font color=red>Error making sphere</font><br><br>");
            return;
        }
        pFuseOcc->appendShape(maker.Shape());
    }
    else if(pSenderAction==m_pInsertCADBox)
    {
        DoubleValueDlg dlg(this, {1.0*Units::mtoUnit(), 1.0*Units::mtoUnit(), 1.0*Units::mtoUnit()}, {"Length", "Width", "Height"},
                           {Units::lengthUnitQLabel(), Units::lengthUnitQLabel(), Units::lengthUnitQLabel()});
        if(dlg.exec()!=QDialog::Accepted) return;

        double x = dlg.value(0)/Units::mtoUnit();
        double y = dlg.value(1)/Units::mtoUnit();
        double z = dlg.value(2)/Units::mtoUnit();

        BRepPrimAPI_MakeBox maker(x, y, z);
        maker.Build();
        if(!maker.IsDone() || maker.Shape().IsNull())
        {
            delete pFuseOcc;
            m_ppto->appendHtmlText("<font color=red>Error making box</font><br><br>");
            return;
        }
        pFuseOcc->appendShape(maker.Shape());
    }
    m_ppto->appendPlainText("---Making shells from shapes-----\n");

    pFuseOcc->makeShellsFromShapes();

    if(pFuseOcc->shellCount()==0)
    {
        QString strange  = "Imported SHAPE does not contain any SHELL: use the fuselage editor to fix the shapes.\n";
        updateOutput(strange);
    }
    else
    {
        m_ppto->appendPlainText("---Making shell triangulation-----\n");
        QString logmsg;
        updateOutput("Making shell triangulation\n");
//        pFuseOcc->makeShellTriangulation(logmsg, "   ");
        gmesh::makeFuseTriangulation(pFuseOcc, logmsg, "   ");
        pFuseOcc->saveBaseTriangulation();
        std::string str;
        pFuseOcc->computeSurfaceProperties(str, "   ");
        updateStdOutput(str+"\n");
        m_ppto->appendPlainText("---updating plane-----\n");
        onUpdatePlane();
    }

    setControls();

    updateData();

    m_bChanged = true;
}


#define MAXSTACKSIZE 25

void PlaneXflDlg::takePicture()
{
    //clear the downstream part of the stack which becomes obsolete
    clearStack(m_StackPos);

    Fuse* pFuse = m_pPlane->fuse(0);
    if(pFuse)
        m_UndoStack.append(pFuse->triMesh());

    if(m_UndoStack.size()==MAXSTACKSIZE+1) m_UndoStack.pop_front();

    // the new current position is the top of the stack
    m_StackPos = m_UndoStack.size()-1;

    enableStackBtns();
}


void PlaneXflDlg::onRedo()
{
    if(m_StackPos<m_UndoStack.size()-1)
    {
        m_StackPos++;
        setPicture();
    }
    enableStackBtns();
}


void PlaneXflDlg::onUndo()
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
    enableStackBtns();
}


void PlaneXflDlg::clearStack(int pos)
{
    for(int il=m_UndoStack.size()-1; il>pos; il--)
    {
        m_UndoStack.removeAt(il);     // remove from the stack
    }
    m_StackPos = m_UndoStack.size()-1;
}


void PlaneXflDlg::setPicture()
{
    Fuse *pFuse = m_pPlaneXfl->fuse(0);
    if(!pFuse) return;
    pFuse->setTriMesh(m_UndoStack.at(m_StackPos));

    onUpdateMesh();
}


void PlaneXflDlg::enableStackBtns()
{
    m_ppbUndo->setEnabled(m_StackPos>0);
    m_ppbRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);
    m_plabStackInfo->setText(tr("stack: %1/%2").arg(m_StackPos+1).arg(int(m_UndoStack.size())));
}


void PlaneXflDlg::onMergeNodes(bool bIsMerging)
{
    m_ppbMakeP3->setChecked(false);
    m_ppbMakeP3Strip->setChecked(false);
    m_ppbSelectPanels->setChecked(false);
    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->clearP3Selection();
    pglPlaneXflView->resetgl3dP3Sel();

    m_pglPlaneView->resetPickedNodes();

    m_pglPlaneView->setPicking(bIsMerging ? xfl::MESHNODE : xfl::NOPICK);
    m_pglPlaneView->setSurfacePick(xfl::FUSESURFACE);
    m_pglPlaneView->update();
    m_bChanged = true;
}


void PlaneXflDlg::onMakeP3(bool bCheck)
{
    m_ppbMoveNode->setChecked(false);
    m_ppbSelectPanels->setChecked(false);
    m_ppbMakeP3Strip->setChecked(false);
    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->clearP3Selection();
    pglPlaneXflView->resetgl3dP3Sel();
    m_pglPlaneView->resetPickedNodes();

    Fuse *pFuse = m_pPlane->fuse(0);
    if(!pFuse)
    {
        m_ppto->appendPlainText("A fuselage is required to build the panel strip");
        m_ppbMakeP3Strip->setChecked(false);
        return;
    }

    m_pglPlaneView->resetPickedNodes();
    m_pglPlaneView->setPicking(bCheck ? xfl::MESHNODE : xfl::NOPICK);
    m_pglPlaneView->setSurfacePick(xfl::NOSURFACE); // =ANYSURFACE

    if(!bCheck)
    {
        endPanelMods();
    }

    m_pglPlaneView->update();
}


void PlaneXflDlg::onMakeP3Strip(bool bCheck)
{
    m_ppbMoveNode->setChecked(false);
    m_ppbSelectPanels->setChecked(false);
    m_ppbMakeP3->setChecked(false);

    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->clearP3Selection();
    pglPlaneXflView->resetgl3dP3Sel();
    m_pglPlaneView->resetPickedNodes();

    Fuse *pFuse = m_pPlane->fuse(0);
    if(!pFuse)
    {
        m_ppto->appendPlainText("A fuselage is required to build the panel strip");
        m_ppbMakeP3Strip->setChecked(false);
        return;
    }

    m_pglPlaneView->setPicking(bCheck ? xfl::MESHNODE : xfl::NOPICK);
    m_pglPlaneView->setSurfacePick(xfl::NOSURFACE); // =ANYSURFACE

    if(!bCheck)
    {
        endPanelMods();
    }

    m_pglPlaneView->update();
}


void PlaneXflDlg::onPickedNode(int iNode)
{
    if(!m_ppbMakeP3Strip->isChecked() && !m_ppbMakeP3->isChecked()) return;

    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);

    if(pglPlaneXflView->P3Node(0)<0)
    {
        pglPlaneXflView->setP3Node(0,iNode);
    }
    else if(pglPlaneXflView->P3Node(1)<0)
    {
        pglPlaneXflView->setP3Node(1, iNode);
    }
    else if(pglPlaneXflView->P3Node(2)<0) // always true
    {
        pglPlaneXflView->setP3Node(2, iNode);

        Fuse *pFuse = m_pPlane->fuse(0);
        if(!pFuse) return;

        Panel3 p3(m_pPlane->node(pglPlaneXflView->P3Node(0)), m_pPlane->node(pglPlaneXflView->P3Node(1)), m_pPlane->node(pglPlaneXflView->P3Node(2)));
        p3.setSurfacePosition(xfl::FUSESURFACE);
        pFuse->triMesh().addPanel(p3);
        if(m_pchMakeP3Opposite->isChecked())
        {
            p3.makeXZsymmetric();
            pFuse->triMesh().addPanel(p3);
        }

        std::string strange;
        pFuse->triMesh().makeNodeArrayFromPanels(0, strange, "  ");
        takePicture();

        onUpdateMesh();

        if(m_ppbMakeP3Strip->isChecked())
            pglPlaneXflView->shiftP3Nodes();
        else
            pglPlaneXflView->resetPickedNodes();

        m_pglPlaneView->resetgl3dMesh();
        m_pglPlaneView->update();
        m_bChanged = true;
    }
}


void PlaneXflDlg::onSelectPanels(bool bSelect)
{
    m_ppbMoveNode->setChecked(false);
    m_ppbMakeP3->setChecked(false);
    m_ppbMakeP3Strip->setChecked(false);


    m_ppbMoveNode->setChecked(false);

    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->clearP3Selection();
    pglPlaneXflView->resetgl3dP3Sel();

    if(bSelect)
    {
        pglPlaneXflView->setP3Selecting(true);
        m_pglPlaneView->setPicking(xfl::PANEL3);
        m_pglPlaneView->setSurfacePick(xfl::FUSESURFACE);
    }
    else
        m_pglPlaneView->setPicking(xfl::NOPICK);

    m_pglPlaneView->update();
    m_bChanged = true;
}


void PlaneXflDlg::onDeleteP3Selection()
{
    Fuse *pFuse = m_pPlaneXfl->fuse(0);
    if(!pFuse) return;
    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);

    if(pglPlaneXflView->p3SelectionCount()==0)
    {
        m_ppto->appendPlainText("Nothing to delete\n");
        return;
    }

    QString strange;

//    pglPlaneXflView->setP3Selecting(false);

    for(int i3=pFuse->nPanel3()-1; i3>=0; i3--)
    {
        for(int isel=0; isel<pglPlaneXflView->p3SelectionCount(); isel++)
        {
            int iPanel3 = pglPlaneXflView->p3SelectionAt(isel);

            if(pFuse->panel3At(i3).index()+pFuse->firstPanel3Index()==iPanel3)
            {
                strange = QString::asprintf("Deleting panel %d\n", iPanel3);
                updateOutput(strange);

                pFuse->triMesh().removePanelAt(i3);

                pglPlaneXflView->p3SelRemove(isel);
                m_bChanged = true;
                break;
            }
        }
    }
    Q_ASSERT(pglPlaneXflView->p3SelectionCount()==0);

    strange = QString::asprintf("\nTriangle count = %d\n", pFuse->nPanel3());
    strange += QString::asprintf("Node count     = %d\n", int(pFuse->nodes().size()));
    strange += "\n_______\n\n";
    updateOutput(strange);

    std::string str;
    pFuse->triMesh().makeNodeArrayFromPanels(0, str, "  ");
    takePicture();

    onUpdateMesh();

    pglPlaneXflView->resetgl3dP3Sel();

//    m_ppbSelectPanels->setChecked(false);
}


bool PlaneXflDlg::endPanelMods()
{
    gl3dPlaneXflView *pglPlaneXflView = dynamic_cast<gl3dPlaneXflView*>(m_pglPlaneView);
    pglPlaneXflView->resetPickedNodes();
    pglPlaneXflView->clearP3Selection();
    pglPlaneXflView->resetgl3dP3Sel();
    m_pglPlaneView->setPicking(xfl::NOPICK);
    m_pglPlaneView->setSurfacePick(xfl::NOSURFACE); // =ANYSURFACE;
    onUpdateMesh();
    return true;
}
