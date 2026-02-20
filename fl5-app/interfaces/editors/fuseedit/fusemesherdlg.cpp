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



#ifdef Q_OS_LINUX
#include <unistd.h>
#endif
#ifdef Q_OS_WIN
#include <windows.h>
#define sleep(s) Sleep(s)
#endif
#ifdef Q_OS_MAC
#include <unistd.h>
#endif

#

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QButtonGroup>
#include <QLabel>
#include <QTabWidget>
#include <QAction>
#include <QMenu>


#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include "fusemesherdlg.h"



#include <core/xflcore.h>
#include <interfaces/mesh/afmesher.h>
#include <interfaces/mesh/gmesherwt.h>
#include <interfaces/mesh/gmesh_globals.h>
#include <interfaces/mesh/mesherwt.h>
#include <interfaces/mesh/meshevent.h>
#include <interfaces/mesh/panelcheckdlg.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dfuseview.h>
#include <interfaces/opengl/fl5views/gl3dshapeview.h>
#include <interfaces/widgets/customdlg/doublevaluedlg.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>

#include <api/fusexfl.h>
#include <api/occ_globals.h>
#include <api/panel3.h>
#include <api/units.h>


bool FuseMesherDlg::s_bfl5Mesher(false);

int FuseMesherDlg::s_ViewIndex = 1;

QByteArray FuseMesherDlg::s_Geometry;
QByteArray FuseMesherDlg::s_HSplitterSizes;
QByteArray FuseMesherDlg::s_VSplitterSizes;

bool FuseMesherDlg::s_bOutline    = true;
bool FuseMesherDlg::s_bSurfaces   = true;
bool FuseMesherDlg::s_bVLMPanels  = false;
bool FuseMesherDlg::s_bAxes       = true;
bool FuseMesherDlg::s_bShowMasses = false;


Quaternion FuseMesherDlg::s_ab_quat_fuse(-0.212012, 0.148453, -0.554032, -0.79124);
Quaternion FuseMesherDlg::s_ab_quat_shape(-0.212012, 0.148453, -0.554032, -0.79124);

FuseMesherDlg::FuseMesherDlg(QWidget *pParent) : QDialog(pParent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Fuse mesh tester"));
    setWindowFlag(Qt::WindowMinMaxButtonsHint);

    m_pFuse= nullptr;

    setupLayout();
    connectSignals();
}


void FuseMesherDlg::setupLayout()
{
    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        m_pHSplitter = new QSplitter(Qt::Horizontal);
        {
            m_pHSplitter->setChildrenCollapsible(true);
            m_ptabViewWt = new QTabWidget;
            {
                QFrame *p3dViewFrame = new QFrame;
                {
                    QVBoxLayout *p3dViewLayout = new QVBoxLayout;
                    {
                        m_pglFuseView = new gl3dFuseView(this);
                        m_pgl3dFuseControls = new gl3dGeomControls(m_pglFuseView, Qt::Horizontal, true);
                        p3dViewLayout->addWidget(m_pglFuseView);
                        p3dViewLayout->addWidget(m_pgl3dFuseControls);
                    }
                    p3dViewFrame->setLayout(p3dViewLayout);
                }

                QFrame *pOccViewFrame = new QFrame;
                {
                    QVBoxLayout *pOccViewLayout = new QVBoxLayout;
                    {
                        m_pglShapeView = new gl3dShapeView(this);
                        m_pgl3dShapeControls = new gl3dGeomControls(m_pglShapeView, Qt::Horizontal, true);
                        pOccViewLayout->addWidget(m_pglShapeView);
                        pOccViewLayout->addWidget(m_pgl3dShapeControls);
                    }
                    pOccViewFrame->setLayout(pOccViewLayout);
                }

                m_ptabViewWt->addTab(p3dViewFrame,"Fuse view");
                m_ptabViewWt->addTab(pOccViewFrame,"Shape view");
            }

            m_pVSplitter = new QSplitter(Qt::Vertical);
            {
                m_pVSplitter->setChildrenCollapsible(false);
                QFrame *pCmdFrame = new QFrame;
                {
                    QVBoxLayout *pCmdLayout = new QVBoxLayout;
                    {

                        QFrame *pfrFreeMesh = new QFrame;
                        {
                            QVBoxLayout *pMeshLayout = new QVBoxLayout;
                            {
                                QHBoxLayout *pMeshSelLayout = new QHBoxLayout;
                                {
                                    QButtonGroup *pGroup = new QButtonGroup;
                                    {
                                        m_prbfl5Mesher = new QRadioButton("flow5 mesher (deprecated)");
                                        m_prbGMesher   = new QRadioButton("Gmsh");
                                        m_prbfl5Mesher->setChecked(s_bfl5Mesher);
                                        m_prbGMesher->setChecked(!s_bfl5Mesher);

                                        pGroup->addButton(m_prbfl5Mesher);
                                        pGroup->addButton(m_prbGMesher);
                                    }
                                    pMeshSelLayout->addStretch();
                                    pMeshSelLayout->addWidget(m_prbfl5Mesher);
                                    pMeshSelLayout->addWidget(m_prbGMesher);
                                    pMeshSelLayout->addStretch();
                                }

                                m_pMesherWt = new MesherWt(this);
                                m_pMesherWt->showPickEdge(false);
                    #ifdef QT_DEBUG
                                m_pMesherWt->showDebugBox(true);
                    #endif
                                m_pGMesherWt = new GMesherWt(this);
                                m_pMesherWt->setVisible(s_bfl5Mesher);
                                m_pGMesherWt->setVisible(!s_bfl5Mesher);
                                pMeshLayout->addLayout(pMeshSelLayout);
                                pMeshLayout->addWidget(m_pMesherWt);
                                pMeshLayout->addWidget(m_pGMesherWt);
                            }
                            pfrFreeMesh->setLayout(pMeshLayout);
                        }


                        QHBoxLayout *pCheckNodesLayout = new QHBoxLayout;
                        {
                            m_ppbMoveNode = new QPushButton("Move node");
                            QString tip = "<p>Use this option to move a fuselage node and to merge it with another.<br>"
                                             "Select first the source node to move, then the destination node.</p>";
                            m_ppbMoveNode->setToolTip(tip);
                            m_ppbMoveNode->setCheckable(true);
                            m_ppbUndoLastMerge = new QPushButton("Undo last");
                            QPushButton *pCheckMenuBtn = new QPushButton("Actions");
                            {
                                QMenu *pCheckMeshMenu = new QMenu("Actions");
                                {
                                    m_pCheckMesh        = new QAction("Check mesh", this);
                                    m_pConnectPanels    = new QAction("Connect panels", this);
                                    m_pConnectPanels->setShortcut(QKeySequence(Qt::ALT | Qt::Key_C));
                                    m_pCheckFreeEdges   = new QAction("Check free edges", this);
                                    m_pCheckFreeEdges->setShortcut(QKeySequence(Qt::ALT | Qt::Key_G));
                                    m_pClearHighlighted = new QAction("Clear highlighted", this);
                                    m_pClearHighlighted->setShortcut(QKeySequence(Qt::ALT | Qt::Key_L));
                                    m_pRestoreFuseMesh = new QAction("Restore default mesh",this);
                                    m_pCleanDoubleNode = new QAction("Clean double nodes", this);

                                    pCheckMeshMenu->addAction(m_pCheckMesh);
                                    pCheckMeshMenu->addAction(m_pConnectPanels);
                                    pCheckMeshMenu->addAction(m_pCheckFreeEdges);
                                    pCheckMeshMenu->addAction(m_pClearHighlighted);
                                    pCheckMeshMenu->addSeparator();
                                    pCheckMeshMenu->addAction(m_pCleanDoubleNode);
                                    pCheckMeshMenu->addSeparator();
                                    pCheckMeshMenu->addAction(m_pRestoreFuseMesh);
                                }
                                pCheckMenuBtn->setMenu(pCheckMeshMenu);
                            }

                            pCheckNodesLayout->addWidget(pCheckMenuBtn);
                            pCheckNodesLayout->addStretch();
                            pCheckNodesLayout->addWidget(m_ppbMoveNode);
                            pCheckNodesLayout->addWidget(m_ppbUndoLastMerge);
                        }

                        pCmdLayout->addWidget(pfrFreeMesh);
                        pCmdLayout->addLayout(pCheckNodesLayout);
                    }
                    pCmdFrame->setLayout(pCmdLayout);
                }

                QFrame *pBotFrame = new QFrame;
                {
                    QVBoxLayout *pBotLayout = new QVBoxLayout;
                    {
                        m_ppto = new PlainTextOutput;

                        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
                        {
                            QPushButton *pClearOutput = new QPushButton("Clear output");
                            m_pButtonBox->addButton(pClearOutput, QDialogButtonBox::ActionRole);
                            connect(pClearOutput, SIGNAL(clicked()), m_ppto,  SLOT(clear()));
                            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
                        }
                        pBotLayout->addWidget(m_ppto);
                        pBotLayout->addWidget(m_pButtonBox);
                        pBotLayout->setStretchFactor(m_ppto, 1);
                    }
                    pBotFrame->setLayout(pBotLayout);
                }

                m_pVSplitter->addWidget(pCmdFrame);
                m_pVSplitter->addWidget(pBotFrame);
            }


            m_pHSplitter->addWidget(m_ptabViewWt);
            m_pHSplitter->addWidget(m_pVSplitter);
            m_pHSplitter->setStretchFactor(0,5);
            m_pHSplitter->setStretchFactor(1,1);

        }
        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);
}


void FuseMesherDlg::initDialog(Fuse *pFuse)
{
    if(!pFuse) return;
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_pFuse = pFuse;
    FuseXfl *pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);
    if(pFuseXfl)
    {
        m_pMesherWt->initWt(pFuseXfl->rightSideShells(), pFuse->maxElementSize(), true, false);
    }
    else
    {
        m_pMesherWt->initWt(pFuse->shells(), pFuse->maxElementSize(), false, false);
    }
    m_pGMesherWt->initWt(m_pFuse, pFuse->isXflType(), false);

    m_ptabViewWt->setCurrentIndex(s_ViewIndex);

    m_pglFuseView->setFuse(pFuse);

    std::string strange;
    pFuse->getProperties(strange, "");
    m_pglFuseView->setBotLeftOutput(strange);
    m_pglFuseView->showPartFrame(false);

    m_pglShapeView->setFuse(pFuse);

    m_pglShapeView->showPartFrame(false);

    m_ppbUndoLastMerge->setEnabled(false);

    QApplication::restoreOverrideCursor();
}


void FuseMesherDlg::connectSignals()
{
    connect(m_prbfl5Mesher,      SIGNAL(clicked(bool)),        SLOT(onSelMesher()));
    connect(m_prbGMesher,        SIGNAL(clicked(bool)),        SLOT(onSelMesher()));

    connect(m_pMesherWt,         SIGNAL(updateFuseView()),     SLOT(onUpdate3dView()));
    connect(m_pMesherWt,         SIGNAL(outputMsg(QString)),   m_ppto, SLOT(onAppendQText(QString)));

    connect(m_pGMesherWt,        SIGNAL(updateFuseView()),     SLOT(onUpdate3dView()));
    connect(m_pGMesherWt,        SIGNAL(outputMsg(QString)),   m_ppto, SLOT(onAppendQText(QString)));

    connect(m_ppbMoveNode,       SIGNAL(clicked(bool)),        SLOT(onMergeNodes(bool)));
    connect(m_ppbUndoLastMerge,  SIGNAL(clicked(bool)),        SLOT(onUndoLastMerge()));

    connect(m_pCheckMesh,        SIGNAL(triggered()),          SLOT(onCheckMesh()));
    connect(m_pClearHighlighted, SIGNAL(triggered()),          SLOT(onClearHighlighted()));
    connect(m_pRestoreFuseMesh,  SIGNAL(triggered()),          SLOT(onResetFuseMesh()));
    connect(m_pCleanDoubleNode,  SIGNAL(triggered()),          SLOT(onDoubleNodes()));

    connect(m_pConnectPanels,    SIGNAL(triggered()),          SLOT(onConnectTriangles()));
    connect(m_pCheckFreeEdges,   SIGNAL(triggered()),          SLOT(onCheckFreeEdges()));


    connect(m_pglFuseView, SIGNAL(pickedNodePair(QPair<int,int>)), SLOT(onPickedNodePair(QPair<int,int>)));
}


void FuseMesherDlg::onUpdate3dView()
{
    if(m_pFuse)
    {
        std::string strange;
        m_pFuse->getProperties(strange, "");
        m_pglFuseView->setBotLeftOutput(strange);
    }

    m_pglFuseView->resetFuse();
    m_pglFuseView->update();
    m_pglShapeView->update();
}


void FuseMesherDlg::onUpdateHighlightedPanels()
{
    m_pglFuseView->resetPanels();
    m_pglFuseView->update();
}


void FuseMesherDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("FuseMesherDlg");
    {
        s_Geometry       = settings.value("WindowGeometry").toByteArray();
        s_HSplitterSizes = settings.value("HSplitSize").toByteArray();
        s_VSplitterSizes = settings.value("VSplitSize").toByteArray();
        s_ViewIndex      = settings.value("ViewIndex3d", s_ViewIndex).toInt();

        AFMesher::setAnimationPause(settings.value("AnimInterval", AFMesher::animationPause()).toInt());

        s_bfl5Mesher     = settings.value("bfl5Mesher",   s_bfl5Mesher).toBool();
        s_bOutline       = settings.value("bOutline",     s_bOutline).toBool();
        s_bSurfaces      = settings.value("bSurfaces",    s_bSurfaces).toBool();
        s_bVLMPanels     = settings.value("bVLMPanels",   s_bVLMPanels).toBool();
        s_bAxes          = settings.value("bAxes",        s_bAxes).toBool();
        s_bShowMasses    = settings.value("bShowMasses",  s_bShowMasses).toBool();

    }
    settings.endGroup();
}


void FuseMesherDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("FuseMesherDlg");
    {
        settings.setValue("WindowGeometry", s_Geometry);
        settings.setValue("HSplitSize",     s_HSplitterSizes);
        settings.setValue("VSplitSize",     s_VSplitterSizes);
        settings.setValue("ViewIndex3d",    s_ViewIndex);

        settings.setValue("AnimInterval",   AFMesher::animationPause());

        settings.setValue("bfl5Mesher",     s_bfl5Mesher);

        settings.setValue("bOutline",       s_bOutline);
        settings.setValue("bSurfaces",      s_bSurfaces);
        settings.setValue("bVLMPanels",     s_bVLMPanels);
        settings.setValue("bAxes",          s_bAxes);
        settings.setValue("bShowMasses",    s_bShowMasses);
    }
    settings.endGroup();
}


void FuseMesherDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save)    == pButton) accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton) reject();
}


void FuseMesherDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    if(s_VSplitterSizes.length()>0) m_pVSplitter->restoreState(s_VSplitterSizes);

    m_pglFuseView->setFlags(s_bOutline, s_bSurfaces, s_bVLMPanels, s_bAxes, s_bShowMasses, false, false, false, false);
    m_pglShapeView->setFlags(s_bOutline, s_bSurfaces, true, s_bAxes, s_bShowMasses, false, false, false, false);
    m_pgl3dFuseControls->setControls();
    m_pgl3dShapeControls->setControls();
    m_pglFuseView->restoreViewPoint(s_ab_quat_fuse);
    m_pglShapeView->restoreViewPoint(s_ab_quat_shape);
}


void FuseMesherDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    m_pFuse->setMaxElementSize(AFMesher::maxEdgeLength());

    s_Geometry = saveGeometry();
    s_HSplitterSizes  = m_pHSplitter->saveState();
    s_VSplitterSizes  = m_pVSplitter->saveState();
    s_ViewIndex = m_ptabViewWt->currentIndex();

    s_bOutline    = m_pglFuseView->bOutline();
    s_bSurfaces   = m_pglFuseView->bSurfaces();
    s_bVLMPanels  = m_pglFuseView->bVLMPanels();
    s_bAxes       = m_pglFuseView->bAxes();
    s_bShowMasses = m_pglFuseView->bMasses();

    m_pglFuseView->saveViewPoint(s_ab_quat_fuse);
    m_pglShapeView->saveViewPoint(s_ab_quat_shape);
}


void FuseMesherDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);

    switch(pEvent->key())
    {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        {
            m_pMesherWt->setFocus();
            break;
        }
        case Qt::Key_C:
        {
            if(bCtrl)
            {
                m_pglShapeView->clearSLG();
                m_pglShapeView->clearTriangles();
                m_pglShapeView->resetGeometry();
                m_pglShapeView->update();
            }
            break;
        }
        case Qt::Key_Escape:
        {
            if(m_pMesherWt->isMeshing())
            {
                AFMesher::cancelTriangulation();
                return;
            }

            if(m_ppbMoveNode->isChecked())
            {
                m_ppbMoveNode->setChecked(false);
                onMergeNodes(false);
                break;
            }
//            break;//no escape
        }
        [[fallthrough]];
        default:
            QDialog::keyPressEvent(pEvent);
    }
}


void FuseMesherDlg::onCheckMesh()
{
    std::string log = "Checking panels\n";
    m_ppto->onAppendStdText(log);

    log.clear();
    PanelCheckDlg dlg(false);
    int res = dlg.exec();
    bool bCheck = dlg.checkSkinny() || dlg.checkMinArea() || dlg.checkMinAngles() || dlg.checkMinQuadWarp();
    if(res!=QDialog::Accepted)
    {
        return;
    }
    m_pglFuseView->resetPanels();

    m_pglFuseView->clearHighlightList();
    QVector<int> highlist = dlg.panelIndexes();
    m_pglFuseView->appendHighlightList(highlist);
    for(int i3=0; i3<highlist.size(); i3++)
    {
        outputPanelProperties(highlist.at(i3));
    }

    if(!bCheck)
    {
        m_pglFuseView->update();
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // check Triangles
    std::vector<int> skinnylist, anglelist, arealist, sizelist;

    m_pFuse->triMesh().checkPanels(log, dlg.checkSkinny(), dlg.checkMinAngles(), dlg.checkMinArea(), dlg.checkMinSize(),
                                   skinnylist, anglelist, arealist, sizelist,
                                   PanelCheckDlg::qualityFactor(), PanelCheckDlg::minAngle(), PanelCheckDlg::minArea(), PanelCheckDlg::minSize());

    QVector<int> qVec;
    if(dlg.checkSkinny())    qVec = QVector<int>(skinnylist.begin(), skinnylist.end());
    if(dlg.checkMinAngles()) qVec = QVector<int>(anglelist.begin(), anglelist.end());
    if(dlg.checkMinArea())   qVec = QVector<int>(arealist.begin(), arealist.end());
    if(dlg.checkMinSize())   qVec = QVector<int>(sizelist.begin(), sizelist.end());
    m_pglFuseView->appendHighlightList(qVec);

    m_ppto->onAppendStdText(log + "\n");


    QApplication::restoreOverrideCursor();
}


void FuseMesherDlg::onClearHighlighted()
{
    m_pglFuseView->clearHighlightList();
    m_pglFuseView->clearSegments();
    m_pglFuseView->resetPanels();
    m_pglFuseView->update();
}


void FuseMesherDlg::outputPanelProperties(int panelindex)
{
    std::string strange;

    // check index validity
    if(panelindex<0 || panelindex>=m_pFuse->nPanel3()) return;

    bool bLong = false;
#ifdef QT_DEBUG
    bLong = true;
#endif
    strange = m_pFuse->panel3At(panelindex).properties(bLong);
    strange +="\n\n";

    m_ppto->onAppendStdText(strange);
}


void FuseMesherDlg::onResetFuseMesh()
{
    std::string logmsg;
    m_pFuse->makeDefaultTriMesh(logmsg, "");

    m_pglFuseView->resetPanels();
    m_pglFuseView->update();
//    onUpdatePlaneProps();
}


void FuseMesherDlg::onNodeDistance()
{
    m_pglFuseView->setPicking(xfl::MESHNODE);
    m_pglFuseView->setSurfacePick(xfl::NOSURFACE);
    m_pglFuseView->update();
}


void FuseMesherDlg::onDoubleNodes()
{
    QStringList labels("Merge nodes closer than:");
    QStringList rightlabels(Units::lengthUnitQLabel());
    QVector<double> vals({XflMesh::nodeMergeDistance()*Units::mtoUnit()});
    DoubleValueDlg dlg(this, vals, labels, rightlabels);
    if(dlg.exec()!=QDialog::Accepted) return;
    XflMesh::setNodeMergeDistance(dlg.value(0)/Units::mtoUnit());


    QString strange;
    strange = QString::asprintf("Cleaning double nodes within precision %g", XflMesh::nodeMergeDistance()*Units::mtoUnit());
    strange += Units::lengthUnitQLabel() + "\n";
    m_ppto->onAppendQText(strange);

    std::string logmsg, prefix("   ");
    m_pFuse->triMesh().cleanDoubleNodes(XflMesh::nodeMergeDistance(), logmsg, prefix);
    m_ppto->onAppendStdText(logmsg);
}


void FuseMesherDlg::onMergeNodes(bool bIsMerging)
{
    m_pglFuseView->setPicking(bIsMerging ? xfl::MESHNODE : xfl::NOPICK);
    m_pglFuseView->update();
}


void FuseMesherDlg::onPickedNodePair(QPair<int, int> nodepair)
{
    if(!m_ppbMoveNode->isChecked())
    {
        m_pglFuseView->stopPicking();
        m_pglFuseView->setPicking(xfl::NOPICK);

        if(nodepair.first <0 || nodepair.first >=m_pFuse->triMesh().nPanels()) return;
        if(nodepair.second<0 || nodepair.second>=m_pFuse->triMesh().nPanels()) return;
        Node nsrc  = m_pFuse->triMesh().node(nodepair.first);
        Node ndest = m_pFuse->triMesh().node(nodepair.second);

        Segment3d seg(nsrc, ndest);
        m_pglFuseView->setSegments({seg});
        QString log = QString::asprintf("Node distance = %g", seg.length()*Units::mtoUnit()) + Units::lengthUnitQLabel();
        log += " (Alt+L to clear)\n";
        m_ppto->onAppendQText(log);

        m_pglFuseView->resetPickedNodes();
        m_pglFuseView->update();
        return;
    }
    QString strange;
    strange = QString::asprintf("Moving node %d to location of node %d and merging\n", nodepair.first, nodepair.second);
    m_ppto->onAppendQText(strange);

    // take a picture
    m_MeshStack.push(m_pFuse->triMesh());

    std::string log;
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_pFuse->triMesh().mergeNodes(nodepair.first, nodepair.second, true, log, "   ");
    //reset indexes, triangles have been reordered
    for(int i3=0; i3<m_pFuse->nPanel3(); i3++)
    {
        Panel3 &p3 = m_pFuse->triMesh().panel(i3);
        p3.setIndex(i3);
        Q_ASSERT(p3.isFusePanel());
    }

    std::string str;
    m_pFuse->triMesh().makeNodeArrayFromPanels(0, str, "  ");

    QApplication::restoreOverrideCursor();
    m_ppto->onAppendStdText(log);

    m_ppbUndoLastMerge->setEnabled(true);

    m_pglFuseView->resetPickedNodes();
    m_pglFuseView->resetPanels();
    str.clear();
    m_pFuse->getProperties(str, "");
    m_pglFuseView->setBotLeftOutput(str);
    m_pglFuseView->update();
}


void FuseMesherDlg::onUndoLastMerge()
{
    if(m_MeshStack.isEmpty()) return;
    std::string strange;
    // restore the picture
    m_pFuse->setTriMesh(m_MeshStack.back());
    m_MeshStack.pop();

    m_pglFuseView->resetPickedNodes();
    m_pglFuseView->resetPanels();
    m_pFuse->getProperties(strange, "");
    m_pglFuseView->setBotLeftOutput(strange);
    m_pglFuseView->update();
    m_ppbUndoLastMerge->setEnabled(!m_MeshStack.isEmpty());
}


void FuseMesherDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESH_UPDATE_EVENT)
    {
        MeshEvent *pMeshEvent = dynamic_cast<MeshEvent*>(pEvent);

        if(pMeshEvent->isFinal())
        {
            QString strange;
            std::string str;
            strange = "   Making mesh from triangles\n";
            m_pFuse->triMesh().makeMeshFromTriangles(pMeshEvent->triangles(), 0, xfl::FUSESURFACE, str, "      ");
            m_ppto->onAppendQText(strange+QString::fromStdString(str));

            strange = QString::asprintf("\nTriangle count = %d\n", m_pFuse->nPanel3());
            m_ppto->onAppendQText(strange);
            strange = QString::asprintf("Node count     = %d\n", int(m_pFuse->nodes().size()));
            strange += "\n_______\n\n";
            m_ppto->onAppendQText(strange);
//            gmsh::write("/home/techwinder/temp/gmesh.msh");
        }

        QVector<int> high;
        m_pglShapeView->setTriangles(pMeshEvent->triangles(), high);
        m_pglShapeView->setPSLG(pMeshEvent->SLG());
        m_pglShapeView->resetGeometry();
        m_pglShapeView->m_DebugPts = AFMesher::s_DebugPts;
        m_pglShapeView->update();

//        m_pglShapeView->repaint();
        
    }
    else if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        m_ppto->onAppendQText(pMsgEvent->msg());
    }
    else
        QDialog::customEvent(pEvent);
}


void FuseMesherDlg::onConnectTriangles()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_ppto->onAppendQText("Connecting panels...");
    onClearHighlighted();

    m_pFuse->triMesh().makeConnectionsFromNodePosition(false, xfl::isMultiThreaded());
    QString log(" ... done\n\n");
    m_ppto->onAppendQText(log);
    QApplication::restoreOverrideCursor();
}


void FuseMesherDlg::onCheckFreeEdges()
{
    std::vector<Segment3d> freeedges;
    m_pFuse->triMesh().getFreeEdges(freeedges);
    QVector<Segment3d> qVec(freeedges.begin(), freeedges.end());
    m_pglFuseView->setSegments(qVec);
    m_pglFuseView->resetPanels();
    m_pglFuseView->update();
    QString strange;
    strange = QString::asprintf("Found %d free edges\n\n", int(freeedges.size()));
    m_ppto->onAppendQText(strange);
}


void FuseMesherDlg::onSelMesher()
{
    s_bfl5Mesher = m_prbfl5Mesher->isChecked();
    m_pMesherWt->setVisible(s_bfl5Mesher);
    m_pGMesherWt->setVisible(!s_bfl5Mesher);
}





