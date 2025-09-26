/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED


#include <QDialogButtonBox>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QAction>
#include <QMenu>


#include "fuseoccdlg.h"
#include <xfl/opengl/gl3dgeomcontrols.h>
#include <xflcore/units.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflmath/constants.h>
#include <xfl/editors/fuseedit/shapefixerdlg.h>
#include <xflobjects/exchange/cadexportdlg.h>
#include <xflobjects/objects3d/fuse/fuseocc.h>
#include <xflocc/occ_globals.h>
#include <xflpanels/mesh/interfaces/mesherwt.h>
#include <xflpanels/mesh/interfaces/panelcheckdlg.h>
#include <xflpanels/mesh/shell/afmesher.h>
#include <xflwidgets/customdlg/doublevaluedlg.h>
#include <xflwidgets/customdlg/intvaluedlg.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/plaintextoutput.h>

QByteArray FuseOccDlg::s_Geometry;
QByteArray FuseOccDlg::s_HSplitterSizes;

#define NPOINTS 19

FuseOccDlg::FuseOccDlg(QWidget *pParent) : FuseDlg(pParent)
{
    setWindowTitle("Occ fuse editor");

    m_pFuseOcc = nullptr;

    createActions();
    setupLayout();
    connectSignals();
}


FuseOccDlg::~FuseOccDlg()
{
}


void FuseOccDlg::initDialog(Fuse*pFuse)
{
    FuseDlg::initDialog(pFuse);

    FuseOcc *pFuseOcc = dynamic_cast<FuseOcc*>(pFuse);

    m_pFuseOcc = pFuseOcc;

    m_pFuseMesherWt->initDialog(m_pFuseOcc->shells(), m_pFuseOcc->maxElementSize(), false, false);
    m_pglFuseView->setFuse(m_pFuseOcc);

    updateProperties();
}


void FuseOccDlg::createActions()
{
    m_pFlipTessNormals = new QAction("Flip normals", this);
}


void FuseOccDlg::setupLayout()
{
    QFrame *pLeftFrame = new QFrame;
    {
        QVBoxLayout *pLeftSideLayout = new QVBoxLayout;
        {
            QTabWidget*pTabViewWidget = new QTabWidget;
            {
                QWidget *pDefinitionTab = new QWidget;
                {
                    pDefinitionTab->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
                    QVBoxLayout *pDefLayout = new QVBoxLayout;
                    {
                        QGroupBox *pShapeFixBox = new QGroupBox("Shape healing");
                        {
                            QVBoxLayout *pShapeFixLayout = new QVBoxLayout;
                            {
                                QLabel *pFlow5Link = new QLabel;
                                pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Modelling/Fuse_CAD.html#stepbystep>https://flow5.tech/docs/flow5_doc/Modelling/Fuse_CAD.html</a>");
                                pFlow5Link->setOpenExternalLinks(true);
                                pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
                                pFlow5Link->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);
                                m_ppbShapeFix = new QPushButton("Fix shapes");
                                m_ppbShapeFix->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
                                pShapeFixLayout->addWidget(m_ppbShapeFix);
                                pShapeFixLayout->addWidget(pFlow5Link);
                            }
                            pShapeFixBox->setLayout(pShapeFixLayout);
                        }

                        pDefLayout->addWidget(m_pMetaFrame);
                        pDefLayout->addWidget(pShapeFixBox);
                        pDefLayout->addStretch();
                    }
                    pDefinitionTab->setLayout(pDefLayout);
                }

                QWidget *pMeshTab = new QWidget;
                {
                    QVBoxLayout *pMeshLayout = new QVBoxLayout;
                    {
                        m_pFuseMesherWt = new MesherWt(this);

                        QHBoxLayout *pActionLayout = new QHBoxLayout;
                        {
                            QPushButton *m_ppbCheckMenuBtn = new QPushButton("Actions");
                            {
                                QMenu *pCheckMeshMenu = new QMenu("Actions");
                                {
                                    m_pCheckMesh        = new QAction("Check mesh", this);
                                    m_pCheckFreeEdges   = new QAction("Check free edges", this);
                                    m_pCheckFreeEdges->setShortcut(QKeySequence(Qt::ALT | Qt::Key_G));
                                    m_pConnectPanels    = new QAction("Connect panels", this);
                                    m_pConnectPanels->setShortcut(QKeySequence(Qt::ALT | Qt::Key_C));
                                    m_pClearHighlighted = new QAction("Clear highlighted", this);
                                    m_pClearHighlighted->setShortcut(QKeySequence(Qt::ALT | Qt::Key_L));
                                    m_pCleanDoubleNode  = new QAction("Clean double nodes");
                                    m_pCenterOnPanel    = new QAction("Center view on panel", this);
                                    m_pRestoreFuseMesh  = new QAction("Restore default mesh",this);
                                    pCheckMeshMenu->addAction(m_pCheckMesh);
                                    pCheckMeshMenu->addAction(m_pConnectPanels);
                                    pCheckMeshMenu->addAction(m_pCheckFreeEdges);
                                    pCheckMeshMenu->addAction(m_pClearHighlighted);
                                    pCheckMeshMenu->addSeparator();
                                    pCheckMeshMenu->addAction(m_pCleanDoubleNode);
                                    pCheckMeshMenu->addSeparator();
                                    pCheckMeshMenu->addAction(m_pCenterOnPanel);
                                    pCheckMeshMenu->addSeparator();
                                    pCheckMeshMenu->addAction(m_pRestoreFuseMesh);

                                }
                                m_ppbCheckMenuBtn->setMenu(pCheckMeshMenu);
                            }
                            pActionLayout->addWidget(m_ppbCheckMenuBtn);
                            pActionLayout->addStretch();
                        }

                        pMeshLayout->addWidget(m_pFuseMesherWt);
                        pMeshLayout->addLayout(pActionLayout);
                    }
                    pMeshTab->setLayout(pMeshLayout);
                }

                pTabViewWidget->addTab(pDefinitionTab, "Geometry");
                pTabViewWidget->addTab(pMeshTab, "Mesh");
            }

            m_pptoOutput = new PlainTextOutput;

            QPushButton *pMenuButton = new QPushButton("Actions");
            {
                QMenu *pBodyMenu = new QMenu("Actions...", this);
                {
                    pBodyMenu->addAction(m_pScale);
                    pBodyMenu->addAction(m_pTranslate);
                    pBodyMenu->addAction(m_pRotate);
                    pBodyMenu->addSeparator();
                    pBodyMenu->addAction(m_pExportToCADFile);
                    pBodyMenu->addAction(m_pExportMeshToSTL);
                    pBodyMenu->addAction(m_pExportTrianglesToSTL);
                    pBodyMenu->addSeparator();
                    pBodyMenu->addAction(m_pFuseInertia);
                    pBodyMenu->addSeparator();
                    pBodyMenu->addAction(m_pTessSettings);
                    pBodyMenu->addAction(m_pFlipTessNormals);

                    QAction *pClearOutput = new QAction("Clear output", this);
                    connect(pClearOutput, SIGNAL(triggered()), m_pptoOutput, SLOT(clear()));
                    pBodyMenu->addSeparator();
                    pBodyMenu->addAction(pClearOutput);
                }
                pMenuButton->setMenu(pBodyMenu);
            }
            m_pButtonBox->addButton(pMenuButton, QDialogButtonBox::ActionRole);

            pLeftSideLayout->addWidget(pTabViewWidget);
            pLeftSideLayout->addWidget(m_pptoOutput);
            pLeftSideLayout->addWidget(m_pButtonBox);
            pLeftSideLayout->setStretchFactor(pTabViewWidget,1);
            pLeftSideLayout->setStretchFactor(m_pptoOutput,5);
            pLeftSideLayout->setStretchFactor(m_pButtonBox,1);
        }
        pLeftFrame->setLayout(pLeftSideLayout);
    }

    QFrame *pRightFrame = new QFrame;
    {
        QVBoxLayout *pTWViewLayout = new QVBoxLayout;
        {
            pTWViewLayout->addWidget(m_pglFuseView);
            pTWViewLayout->addWidget(m_pglControls);
        }
        pRightFrame->setLayout(pTWViewLayout);
    }

    m_pHSplitter = new QSplitter(Qt::Horizontal, this);
    {
        m_pHSplitter->addWidget(pLeftFrame);
        m_pHSplitter->addWidget(pRightFrame);
        m_pHSplitter->setStretchFactor(0,1);
        m_pHSplitter->setStretchFactor(1,5);
        m_pHSplitter->setChildrenCollapsible(false);
    }

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);
}


void FuseOccDlg::connectSignals()
{
    connectBaseSignals();

    connect(m_ppbShapeFix,           SIGNAL(clicked(bool)),    SLOT(onShapeFix()));

    connect(m_pExportToCADFile,      SIGNAL(triggered()),       SLOT(onExportBodyToCADFile()));
    connect(m_pFlipTessNormals,      SIGNAL(triggered()),       SLOT(onFlipTessNormals()));

    connect(m_pFuseMesherWt,         SIGNAL(updateFuseView()),   SLOT(onUpdateFuseView()));
    connect(m_pFuseMesherWt,         SIGNAL(outputMsg(QString)), m_pptoOutput, SLOT(onAppendThisPlainText(QString)));

    connect(m_pCheckMesh,            SIGNAL(triggered()),       SLOT(onCheckMesh()));
    connect(m_pCenterOnPanel,        SIGNAL(triggered()),       SLOT(onCenterViewOnPanel()));
    connect(m_pClearHighlighted,     SIGNAL(triggered()),       SLOT(onClearHighlighted()));
    connect(m_pCleanDoubleNode,      SIGNAL(triggered()),       SLOT(onDoubleNodes()));
    connect(m_pConnectPanels,        SIGNAL(triggered()),       SLOT(onConnectTriangles()));
    connect(m_pCheckFreeEdges,       SIGNAL(triggered()),       SLOT(onCheckFreeEdges()));
}


void FuseOccDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent const *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        updateOutput(pMsgEvent->msg());
    }
    else if(pEvent->type() == MESH_UPDATE_EVENT)
    {
        if(!m_pFuseOcc) return;
        QString strange;
        strange = "   Making mesh from triangles\n";
        m_pFuseOcc->triMesh().makeMeshFromTriangles(m_pFuseMesherWt->triangles(), 0, xfl::FUSESURFACE, strange, "      ");
        updateOutput(strange);

        strange = QString::asprintf("\nTriangle count = %d\n", m_pFuseOcc->nPanel3());
        strange += QString::asprintf( "Node count     = %d\n", int(m_pFuseOcc->nodes().size()));
        strange += "\n_______\n\n";
        updateOutput(strange);
        updateProperties(false);

        m_pFuseOcc->setMaxElementSize(AFMesher::maxEdgeLength());

        m_bChanged = true;
    }
    else
        QDialog::customEvent(pEvent);
}


void FuseOccDlg::onUpdateFuseView()
{
    m_pglFuseView->resetFuse();
    m_pglFuseView->update();
}


void FuseOccDlg::updateOutput(QString const &strong)
{
    m_pptoOutput->onAppendThisPlainText(strong);
}


void FuseOccDlg::onFlipTessNormals()
{
    if(!m_pFuse) return;

    m_pFuse->triangulation().flipNormals();

    m_pglFuseView->resetFuse();
    updateView();
    m_bChanged = true;
}


void FuseOccDlg::onExportBodyToCADFile()
{
    if(!m_pFuseOcc)return ;// is there anything to export?
    CADExportDlg dlg(this);

    dlg.init(m_pFuseOcc->shells(), m_pFuseOcc->name());
    dlg.exec();
}


void FuseOccDlg::updateProperties(bool bFull)
{
    QString strong;
    m_pFuseOcc->computeSurfaceProperties(strong, QString());

    strong += QString::asprintf("\nTriangles       = %6d", m_pFuseOcc->nPanel3());
    m_pglFuseView->setBotLeftOutput(strong);

    QString logmsg("Fuselage properties:\n");
    m_pFuseOcc->getProperties(strong, QString("   "), bFull);
    logmsg += strong;
    updateOutput(logmsg+"\n\n");
}


void FuseOccDlg::showEvent(QShowEvent *pEvent)
{
    FuseDlg::showEvent(pEvent);
    restoreGeometry(s_Geometry);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
}


void FuseOccDlg::hideEvent(QHideEvent *pEvent)
{
     s_Geometry = saveGeometry();
     s_HSplitterSizes  = m_pHSplitter->saveState();

     FuseDlg::hideEvent(pEvent);
}


void FuseOccDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("FuseOccDlg");
    {
        s_Geometry        = settings.value("WindowGeom", QByteArray()).toByteArray();
        s_HSplitterSizes  = settings.value("HSplitterSizes").toByteArray();
    }
    settings.endGroup();
}


void FuseOccDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("FuseOccDlg");
    {
        settings.setValue("WindowGeom", s_Geometry);
        settings.setValue("HSplitterSizes",  s_HSplitterSizes);
    }
    settings.endGroup();
}


void FuseOccDlg::onShapeFix()
{
    ShapeFixerDlg dlg;
    dlg.initDialog(m_pFuse->shapes());
    if(dlg.exec()!=QDialog::Accepted) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString strange;
    QString prefix("   ");
    updateOutput("Updating fuselage with fixed shapes:\n");

    m_pFuse->clearShapes();
    TopoDS_ListIteratorOfListOfShape iterator;
    int ishape(0);
    for (iterator.Initialize(dlg.shapes()); iterator.More(); iterator.Next())
    {
        m_pFuse->appendShape(iterator.Value());

        occ::listShapeContent(iterator.Value(), strange, prefix);
        strange = QString::asprintf("Shape %d:\n", ishape) + strange + "\n";
        m_pptoOutput->onAppendThisPlainText(strange);
        ishape++;
    }

    updateOutput("Making shells from shapes\n");
    m_pFuse->makeShellsFromShapes();

    updateOutput("Making shell triangulation\n");
    m_pFuse->makeShellTriangulation(strange, prefix);
    updateOutput("Tessellation:\n"+strange+"\n");

    updateProperties();

    m_pglFuseView->resetFuse();
    m_pglFuseView->update();

    m_pFuseMesherWt->initDialog(m_pFuseOcc->shells(), m_pFuseOcc->maxElementSize(), false, false);

    m_bChanged = true;
    QApplication::restoreOverrideCursor();
}


void FuseOccDlg::onConnectTriangles()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_pFuseMesherWt->onReadParams(); // update the node merge distance
    updateOutput(QString::asprintf("Connecting panels - node coincidence=%g", XflMesh::nodeMergeDistance()*Units::mtoUnit())+Units::lengthUnitLabel()+"\n");
    int n = m_pFuse->nPanel3();
    m_pFuse->triMesh().makeConnectionsFromNodePosition(0,n,XflMesh::nodeMergeDistance(), true);

    QString log(" ... done\n\n");
    updateOutput(log);
    QApplication::restoreOverrideCursor();
}


void FuseOccDlg::onCheckFreeEdges()
{
    QVector<Segment3d> freedges;
    m_pFuseOcc->triMesh().getFreeEdges(freedges);
    m_pglFuseView->setSegments(freedges);

    m_pglFuseView->resetgl3dMesh();
    m_pglFuseView->update();
    QString strange;
    strange = QString::asprintf("Found %d free edges\n\n", int(freedges.size()));
    updateOutput(strange);
}


void FuseOccDlg::onClearHighlighted()
{
    m_pglFuseView->clearHighlightList();
    m_pglFuseView->clearSegments();

    m_pglFuseView->resetgl3dMesh();
    m_pglFuseView->update();
}


void FuseOccDlg::onDoubleNodes()
{
    QStringList labels("Merge nodes closer than:");
    QStringList rightlabels(Units::lengthUnitLabel());
    QVector<double> vals({XflMesh::nodeMergeDistance()*Units::mtoUnit()});
    DoubleValueDlg dlg(this, vals, labels, rightlabels);
    if(dlg.exec()!=QDialog::Accepted) return;
    XflMesh::setNodeMergeDistance(dlg.value(0)/Units::mtoUnit());

    QString strange;
    strange = QString::asprintf("Cleaning double nodes within precision %g", XflMesh::nodeMergeDistance()*Units::mtoUnit());
    strange += Units::lengthUnitLabel() + "\n";
    updateOutput(strange);

    QString logmsg, prefix("   ");

    m_pFuseOcc->triMesh().cleanDoubleNodes(XflMesh::nodeMergeDistance(), logmsg, prefix);
    updateOutput(logmsg);

    m_bChanged = true;
}


void FuseOccDlg::outputPanelProperties(int panelindex)
{
    QString strange;

    // check index validity
    if(panelindex<0 || panelindex>=m_pFuseOcc->nPanel3()) return;

    bool bLong = false;
#ifdef QT_DEBUG
    bLong = true;
#endif
    strange = m_pFuseOcc->panel3At(panelindex).properties(bLong);
    strange +="\n\n";

    updateOutput(strange);
}


void FuseOccDlg::onCheckMesh()
{
    QString log = "Checking panels\n";
    updateOutput(log);

    log.clear();
    PanelCheckDlg dlg(false);
    int res = dlg.exec();
    bool bCheck = dlg.checkSkinny() || dlg.checkMinArea() || dlg.checkMinAngles() || dlg.checkMinQuadWarp();
    if(res!=QDialog::Accepted)
    {
        return;
    }

    m_pglFuseView->resetgl3dMesh();

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
    QVector<int> skinnylist, anglelist, arealist, sizelist;

    m_pFuseOcc->triMesh().checkPanels(log, dlg.checkSkinny(), dlg.checkMinAngles(), dlg.checkMinArea(), dlg.checkMinSize(),
                                      skinnylist, anglelist, arealist, sizelist,
                                      PanelCheckDlg::qualityFactor(), PanelCheckDlg::minAngle(), PanelCheckDlg::minArea(), PanelCheckDlg::minSize());
    if(dlg.checkSkinny())    m_pglFuseView->appendHighlightList(skinnylist);
    if(dlg.checkMinAngles()) m_pglFuseView->appendHighlightList(anglelist);
    if(dlg.checkMinArea())   m_pglFuseView->appendHighlightList(arealist);
    if(dlg.checkMinSize())   m_pglFuseView->appendHighlightList(sizelist);

    updateOutput(log + "\n");

    QApplication::restoreOverrideCursor();
}


void FuseOccDlg::onCenterViewOnPanel()
{
    if(!m_pFuseOcc)return;

    IntValueDlg dlg(this);
    dlg.setValue(-1);
    dlg.setLeftLabel("Panel index:");
    if(dlg.exec()==QDialog::Accepted)
    {
        int ip = dlg.value();
        if(ip>=0 && ip<m_pFuseOcc->triMesh().nPanels())
        {
            m_pglFuseView->centerViewOn(m_pFuseOcc->triMesh().panelAt(ip).CoG());
            updateOutput(m_pFuseOcc->triMesh().panelAt(ip).properties(true)+"\n\n");
        }

        QVector<int> highlist = {ip};
        m_pglFuseView->setHighlightList(highlist);
        m_pglFuseView->resetgl3dMesh();
    }
}


void FuseOccDlg::onScale()
{
    FuseDlg::onScale();
    m_pFuseMesherWt->initDialog(m_pFuseOcc->shells(), m_pFuseOcc->maxElementSize(), false, false);
}


void FuseOccDlg::onTranslate()
{
    FuseDlg::onTranslate();
    m_pFuseMesherWt->initDialog(m_pFuseOcc->shells(), m_pFuseOcc->maxElementSize(), false, false);
}


void FuseOccDlg::onRotate()
{
    FuseDlg::onRotate();
    m_pFuseMesherWt->initDialog(m_pFuseOcc->shells(), m_pFuseOcc->maxElementSize(), false, false);
}



