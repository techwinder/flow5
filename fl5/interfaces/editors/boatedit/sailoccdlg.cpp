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

#include <QVBoxLayout>

#include <QAction>
#include <QMenu>

#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

#include "sailoccdlg.h"

#include <interfaces/editors/boatedit/edgesplitdlg.h>
#include <interfaces/editors/shapedlg.h>
#include <interfaces/mesh/afmesher.h>
#include <interfaces/mesh/gmesherwt.h>
#include <interfaces/mesh/mesherwt.h>
#include <interfaces/mesh/meshevent.h>
#include <interfaces/mesh/tesscontrolsdlg.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dsailview.h>
#include <interfaces/widgets/customdlg/doublevaluedlg.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <modules/xobjects.h>

#include <api/sailocc.h>
#include <api/occ_globals.h>
#include <api/flow5events.h>
#include <api/units.h>




SailOccDlg::SailOccDlg(QWidget *pParent) : ExternalSailDlg(pParent)
{
    setWindowTitle("CAD sail editor");

    m_pTabWidget = nullptr;

    setupLayout();
    connectSignals();
    m_pglSailControls->enableCtrlPts(false);
}


void SailOccDlg::hideEvent(QHideEvent *pEvent)
{
    ExternalSailDlg::hideEvent(pEvent);
    SailOcc *pOccSail = dynamic_cast<SailOcc*>(m_pSail);
    pOccSail->setMaxElementSize(AFMesher::maxEdgeLength());
}


void SailOccDlg::initDialog(Sail *pSail)
{
    SailDlg::initDialog(pSail);

    SailOcc *pOccSail = dynamic_cast<SailOcc*>(m_pSail);
    if(pOccSail->shapes().Extent()==1)
    {
        TopoDS_Shape const &sh = pOccSail->shapes().First();
        pOccSail->m_EdgeSplit.resize(occ::nFaces(sh));
        for(uint iface=0; iface<pOccSail->m_EdgeSplit.size(); iface++)
        {
            TopoDS_Face face;
            occ::getShapeFace(sh, iface, face);
            pOccSail->m_EdgeSplit[iface].resize(occ::nEdges(face));
        }
        onMakeEdgeSplits();
    }

    m_pgbMeshType->setEnabled(false);
    m_prbRuledMesh->setChecked(false);
    m_prbFreeMesh->setChecked(true);

    m_prbThin->setChecked(pOccSail->isThinSurface());
    m_prbThick->setChecked(!pOccSail->isThinSurface());

    m_pglSailView->setReferenceLength(2.0*pOccSail->size());

    m_pfeTEAngle->setValue(s_TEMaxAngle);

    setControls();
    setSailData();
}


void SailOccDlg::setupLayout()
{
    QFrame *pLeftFrame = new QFrame;
    {
        QVBoxLayout *pLeftLayout = new QVBoxLayout;
        {
            m_pTabWidget = new QTabWidget(this);
            {
                QFrame *pMetaFrame = new QFrame;
                {
                    QVBoxLayout *pMetaLayout = new QVBoxLayout;
                    {
                        pMetaLayout->addWidget(m_pfrMeta);
                        pMetaLayout->addWidget(m_pCornersBox);
                    }
                    pMetaFrame->setLayout(pMetaLayout);
                }

                m_pTabWidget->addTab(pMetaFrame, "Meta");
                m_pTabWidget->addTab(m_pfrMesh,  "Mesh");
                m_pTabWidget->addTab(m_pfrTE,    "Trailing edge");
                m_pTabWidget->setCurrentIndex(0);
            }

            QPushButton *ppbSailActions = new QPushButton("Sail actions");
            {
                QMenu *ppbSailMenu = new QMenu("Sail actions");
                {
                    ppbSailMenu->addAction(m_pDefinitions);
                    ppbSailMenu->addSeparator();
                    QMenu *pTessMenu = ppbSailMenu->addMenu("Tessellation");
                    {
                        m_pTessSettings         = new QAction("Settings",     this);
                        m_pTessSettings->setShortcut(QKeySequence(Qt::ALT | Qt::Key_T));
                        m_pFlipTessNormals      = new QAction("Flip normals", this);
                        pTessMenu->addAction(m_pTessSettings);
                        pTessMenu->addAction(m_pFlipTessNormals);
                    }

                    ppbSailMenu->addSeparator();
                    ppbSailMenu->addAction(m_pTranslate);
                    ppbSailMenu->addAction(m_pRotate);
                    ppbSailMenu->addAction(m_pScaleShape);
                    ppbSailMenu->addAction(m_pScaleSize);
                    ppbSailMenu->addSeparator();
                    ppbSailMenu->addAction(m_pFlipXZ);
#ifdef QT_DEBUG
                    QAction *pEditShapes = new QAction("Shapes", this);
                    pEditShapes->setShortcut(Qt::Key_F9);
                    connect(pEditShapes, SIGNAL(triggered()), SLOT(onShapes()));
                    ppbSailMenu->addSeparator();
                    ppbSailMenu->addAction(pEditShapes);
#endif
                }
                ppbSailActions->setMenu(ppbSailMenu);
                m_pButtonBox->addButton(ppbSailActions, QDialogButtonBox::ActionRole);
            }
            pLeftLayout->addWidget(m_pTabWidget);
            pLeftLayout->addWidget(m_ppto);
            pLeftLayout->addWidget(m_pFlow5Link);
            pLeftLayout->addWidget(m_pButtonBox);
            pLeftLayout->setStretchFactor(m_pTabWidget,1);
            pLeftLayout->setStretchFactor(m_ppto,10);
            pLeftLayout->setStretchFactor(m_pTabWidget,1);
        }
        pLeftFrame->setLayout(pLeftLayout);
    }

    m_pExternalSplitter = new QSplitter(Qt::Horizontal);
    {
        m_pExternalSplitter->setChildrenCollapsible(false);
        m_pExternalSplitter->addWidget(pLeftFrame);
        m_pExternalSplitter->addWidget(m_pfr3dView);
        m_pExternalSplitter->setStretchFactor(0, 3);
        m_pExternalSplitter->setStretchFactor(1, 1);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pExternalSplitter);
    }

    setLayout(pMainLayout);
}


void SailOccDlg::connectSignals()
{
    ExternalSailDlg::connectSignals();


    connect(m_pMesherWt,        SIGNAL(updateFuseView()),     SLOT(onUpdateSailView()));
    connect(m_pMesherWt,        SIGNAL(outputMsg(QString)), m_ppto, SLOT(onAppendQText(QString)));
    connect(m_pMesherWt->m_pfeMaxEdgeSize,  SIGNAL(floatChanged(float)),  SLOT(onMakeEdgeSplits()));
    connect(m_pMesherWt->m_pchPickEdge,     SIGNAL(clicked(bool)),   SLOT(onPickEdge(bool)));

    connect(m_pGMesherWt,       SIGNAL(updateFuseView()),     SLOT(onUpdateSailView()));
    connect(m_pGMesherWt,       SIGNAL(outputMsg(QString)), m_ppto, SLOT(onAppendQText(QString)));


    connect(m_pglSailView, SIGNAL(pickedEdge(int,int)), SLOT(onPickedEdge(int,int)));

    connect(m_pFlipTessNormals, SIGNAL(triggered()),          SLOT(onFlipTessNormals()));
    connect(m_pTessSettings,    SIGNAL(triggered()),          SLOT(onTessellation()));

    connect(m_pTabWidget,       SIGNAL(currentChanged(int)),  SLOT(onTabChanged(int)));
}


void SailOccDlg::keyPressEvent(QKeyEvent *pEvent)
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
         default: break;
    }
    ExternalSailDlg::keyPressEvent(pEvent);
}


void SailOccDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        m_ppto->onAppendQText(pMsgEvent->msg());
    }
    else if(pEvent->type() == MESH_UPDATE_EVENT)
    {      
        MeshEvent *pMeshEvent = dynamic_cast<MeshEvent*>(pEvent);
        QString strange;
        SailOcc *pOccSail = dynamic_cast<SailOcc*>(m_pSail);
        pOccSail->clearTEIndexes();
        pOccSail->setRefTriangles(pMeshEvent->triangles());
        pOccSail->makeTriPanels(Vector3d());

        strange = QString::asprintf("\nPanel count = %d\n", pOccSail->triMesh().nPanels());
        strange = QString::asprintf("\nNode count  = %d\n", pOccSail->triMesh().nNodes());
        strange += "\n_______\n\n";
        m_ppto->onAppendQText(strange);

        pOccSail->setMaxElementSize(AFMesher::maxEdgeLength());
        m_bChanged = true;
    }
    else
        QDialog::customEvent(pEvent);
}


void SailOccDlg::onUpdateSailView()
{
    m_pglSailView->resetglSail();
    m_pglSailView->update();
}


void SailOccDlg::onFlipTessNormals()
{
    m_pSail->flipTriangulationNormals();

    m_pglSailView->resetglSail();
    updateView();
    m_bChanged = true;
}


void SailOccDlg::onTessellation()
{
    TessControlsDlg dlg(this);
    SailOcc *pOccSail = dynamic_cast<SailOcc*>(m_pSail);
    dlg.initDialog(pOccSail->occTessParams(), pOccSail->gmshTessParams(), Part::isOccTessellator(), true);
    if(dlg.exec()==QDialog::Accepted && dlg.isChanged())
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        Part::setOccTessellator(dlg.bOcc());
        pOccSail->setOccTessParams(dlg.occParameters());
        pOccSail->setGmshTessParams(dlg.gmshParameters());
        m_bDescriptionChanged = true;
        Objects3d::makeSailTriangulation(pOccSail);
        m_pglSailView->resetglSail();
        updateView();
        m_ppto->onAppendQText("\n"+QString::fromStdString(pOccSail->logMsg())+"\n");
        QApplication::restoreOverrideCursor();
    }
}


void SailOccDlg::onShapes()
{
    SailOcc *pOccSail = dynamic_cast<SailOcc*>(m_pSail);
    ShapeDlg dlg(this);
    dlg.initDialog(pOccSail->shapes());
    if(dlg.exec()==QDialog::Accepted)
    {
        pOccSail->clearShapes();
        for(int ish=0; ish<dlg.shapes().size(); ish++)
        {
            pOccSail->appendShape(dlg.shape(ish));
        }

        Objects3d::makeSailTriangulation(pOccSail);
        m_pglSailView->resetglSail();
        m_pglSailView->update();
        m_bChanged = true;
    }
}


void SailOccDlg::onTabChanged(int iTab)
{
    deselectButtons();
    if(iTab==1) initMesher();
    else
    {
        m_pMesherWt->m_pchPickEdge->setChecked(false);
        onPickEdge(false);
    }
}


void SailOccDlg::initMesher()
{
    SailOcc *pOccSail = dynamic_cast<SailOcc*>(m_pSail);

    int iShell = 0;
    TopoDS_ListOfShape shells;
    for(TopTools_ListIteratorOfListOfShape shellitt(pOccSail->shapes()); shellitt.More(); shellitt.Next())
    {
        TopExp_Explorer shapeExplorer;
        for (shapeExplorer.Init(shellitt.Value(), TopAbs_SHELL); shapeExplorer.More(); shapeExplorer.Next())
        {
            try
            {
                TopoDS_Shell shell = TopoDS::Shell(shapeExplorer.Current());
                if(!shell.IsNull())
                {
                    shells.Append(shell);
                    iShell++;
                }
            }
            catch(Standard_TypeMismatch const &)
            {
            }
            catch(...)
            {
            }
        }
    }
    (void)iShell;

    m_pMesherWt->initWt(shells, pOccSail->maxElementSize(), false, false);
    m_pGMesherWt->initWt(pOccSail);
}


void SailOccDlg::onPickEdge(bool bPick)
{
    if(bPick)
        m_pglSailView->setPicking(xfl::SEGMENT3D);
    else
        m_pglSailView->stopPicking();

    m_pglSailView->showEdgeNodes(bPick);
    m_pglSailView->update();
}


void SailOccDlg::onPickedEdge(int iFace, int iEdge)
{
    SailOcc *pOccSail = dynamic_cast<SailOcc*>(m_pSail);
    if(!pOccSail) return;
    if(!pOccSail->shapes().Extent()) return;

    if(iFace<0 || iFace>=int(pOccSail->m_EdgeSplit.size()))
    {
        m_ppto->onAppendQText("Face selection error\n");
        return;
    }
    if(iEdge<0 || iEdge>=int(pOccSail->m_EdgeSplit.at(iFace).size()))
    {
        m_ppto->onAppendQText("Edge selection error\n");
        return;
    }
    EdgeSplitDlg dlg(this, pOccSail->m_EdgeSplit[iFace][iEdge]);
    if(dlg.exec()!=QDialog::Accepted) return;
    pOccSail->m_EdgeSplit[iFace][iEdge].setNSegs(dlg.nSegs());
    pOccSail->m_EdgeSplit[iFace][iEdge].setDistrib(dlg.distrib());


    // Since OCC duplicates edges for each face, need to update all identical edges
    TopoDS_Edge refedge;
    occ::getEdge(pOccSail->shapes().First(), iFace, iEdge, refedge);

    TopExp_Explorer FaceExplorer;
    TopExp_Explorer EdgeExplorer;

    int iface=0, iedge=0;
    for(FaceExplorer.Init(pOccSail->shapes().First(), TopAbs_FACE); FaceExplorer.More(); FaceExplorer.Next())
    {
        TopoDS_Face const &face = TopoDS::Face(FaceExplorer.Current());
        if(iFace>=int(pOccSail->m_EdgeSplit.size())) break; //error somewhere, can't continue

        iedge = 0;
        for(EdgeExplorer.Init(face, TopAbs_EDGE); EdgeExplorer.More(); EdgeExplorer.Next())
        {
            if(iEdge>=int(pOccSail->m_EdgeSplit.at(iFace).size())) break; //error somewhere, can't continue

            TopoDS_Edge const &edge = TopoDS::Edge(EdgeExplorer.Current());
            if(occ::isSameEdge(edge, refedge))
            {
                pOccSail->m_EdgeSplit[iface][iedge] = pOccSail->m_EdgeSplit[iFace][iEdge];
            }
            iedge++;
        }
        iface++;
    }

    onMakeEdgeSplits();

    m_bChanged = true;
}


void SailOccDlg::onMakeEdgeSplits()
{
    SailOcc *pOccSail = dynamic_cast<SailOcc*>(m_pSail);
    TopExp_Explorer FaceExplorer;
    TopExp_Explorer EdgeExplorer;
    int iFace=0, iEdge=0;

    for(FaceExplorer.Init(pOccSail->shapes().First(), TopAbs_FACE); FaceExplorer.More(); FaceExplorer.Next())
    {
        TopoDS_Face const &face = TopoDS::Face(FaceExplorer.Current());

        if(iFace>=int(pOccSail->m_EdgeSplit.size()))
            break; //error somewhere, can't continue
        iEdge = 0;
        for(EdgeExplorer.Init(face, TopAbs_EDGE); EdgeExplorer.More(); EdgeExplorer.Next())
        {
            if(iEdge>=int(pOccSail->m_EdgeSplit.at(iFace).size())) break; //error somewhere, can't continue

            TopoDS_Edge const &edge = TopoDS::Edge(EdgeExplorer.Current());

            EdgeSplit &es = pOccSail->m_EdgeSplit[iFace][iEdge];

            if(es.nSegs()>0)
                es.makeSplit(edge);
            else
                es.makeUniformSplit(edge, AFMesher::maxEdgeLength());
            iEdge++;
        }
        iFace++;
    }
    updateEdgeNodes();

    m_pMesherWt->setEdgeSplit(pOccSail->m_EdgeSplit);
}


void SailOccDlg::updateEdgeNodes()
{
    m_pglSailView->clearNodes();
    std::vector<Node> nodes;
    makeEdgeNodes(nodes);
    m_pglSailView->setNodes(nodes);
    m_pglSailView->update();
}


void SailOccDlg::makeEdgeNodes(std::vector<Node> &nodes)
{
    SailOcc *pOccSail = dynamic_cast<SailOcc*>(m_pSail);
    TopExp_Explorer FaceExplorer;
    TopExp_Explorer EdgeExplorer;
    int iFace=0, iEdge=0;

    for(FaceExplorer.Init(pOccSail->shapes().First(), TopAbs_FACE); FaceExplorer.More(); FaceExplorer.Next())
    {
        TopoDS_Face const &face = TopoDS::Face(FaceExplorer.Current());
        if(iFace>=int(pOccSail->m_EdgeSplit.size())) break; //error somewhere, can't continue

        iEdge = 0;
        for(EdgeExplorer.Init(face, TopAbs_EDGE); EdgeExplorer.More(); EdgeExplorer.Next())
        {
            if(iEdge>=int(pOccSail->m_EdgeSplit.at(iFace).size())) break; //error somewhere, can't continue

            TopoDS_Edge const &edge = TopoDS::Edge(EdgeExplorer.Current());
            EdgeSplit &es = pOccSail->m_EdgeSplit[iFace][iEdge];
            es.makeNodes(edge, nodes);
            iEdge++;
        }
        iFace++;
    }
}



