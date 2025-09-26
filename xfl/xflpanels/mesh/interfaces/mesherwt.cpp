/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include <QThread>
#include <QVBoxLayout>

#include <TopExp_Explorer.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopoDS.hxx>

#include "mesherwt.h"


#include <xflcore/units.h>
#include <xflocc/occ_globals.h>
#include <xflobjects/sailobjects/sails/sail.h>
#include <xflpanels/mesh/interfaces/panelcheckdlg.h>
#include <xflpanels/mesh/shell/afmesher.h>
#include <xflpanels/mesh/xflmesh.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/plaintextoutput.h>


MesherWt::MesherWt(QWidget *pParent) : QGroupBox("Mesher", pParent)
{
    m_pParent = pParent; // to post an animation event
    m_pMesher = nullptr;
    m_pSail = nullptr;
    m_bIsMeshing = false;
    m_bSplittableInnerPSLG = false;
    m_bMakexzSymmetric = false;

    setupLayout();
    connectSignals();
}


void MesherWt::setupLayout()
{
    QString tip;
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGridLayout *pParamsLayout = new QGridLayout;
        {           
            m_pchPickEdge = new QCheckBox("Pick edge");

            QLabel *pLabEdgeSize   = new QLabel("Max. edge length:");
            QLabel *pLabLengthUnit = new QLabel(Units::lengthUnitLabel());
            pLabEdgeSize->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            pLabLengthUnit->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            QLabel *pLabMaxPanels  = new QLabel("Max. panel count:");
            pLabMaxPanels->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            QLabel *pLabMaxPanelUnit =new QLabel("/face");

            QLabel *pLabMergeDist   = new QLabel("Node merge distance:");
            QLabel *pLabLengthUnit2 = new QLabel(Units::lengthUnitLabel());
            pLabMergeDist->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            pLabLengthUnit2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            QLabel *pLabSearchFact = new QLabel("Node search radius coef.:");
            pLabSearchFact->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

            QLabel *pLabGrowthFact = new QLabel("Triangle growth factor:");
            pLabGrowthFact->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

            m_pdeMaxEdgeSize = new FloatEdit(1.0);
            tip = "<p>The approximate max length used to split edges.</p>";
            m_pdeMaxEdgeSize->setToolTip(tip);

            m_pieMaxPanelCount = new IntEdit;
            tip = "<p>The mesh operation will stop if the number of triangular panels\n"
                     "on any face exceeds this value.\n"
                     "Intended to prevent excessive mesh sizes and lengthy operations.\n"
                     "Recommendation 1000-3000.</p>";
            m_pieMaxPanelCount->setToolTip(tip);

            m_pdeNodeMergeDistance = new FloatEdit;
            tip = "<p>Nodes closer than this value will be merged.\n"
                     "Set a positive value to eliminate quasi-null triangles.\n"
                     "Recommendation: ~0.1mm or greater</p>";
            m_pdeNodeMergeDistance ->setToolTip(tip);

            m_pdeSearchRadiusFactor = new FloatEdit;
            tip = "<p>Defines the coefficient K used to calculate the radius "
                     "of the circle in which existing nodes will be searched for "
                     "when building a new triangle.<br>"
                     "The mesher will use the closest node in the radius "
                     "r = min(MaxEdgeLength/2.0, triangle_base_length*K)<br>"
                     "Small values of K lead to more equilateral triangles but larger mesh sizes. "
                     "High values of K lead to more progressive increase of the triangle sizes.<br>"
                     "Recommendation: 0.3 < K < 1</p>";
            m_pdeSearchRadiusFactor->setToolTip(tip);

            m_pdeGrowthFactor = new FloatEdit;
            tip= "Controls the growth of triangles as the mesh front progresses away from the edges.\n"
                 "A value of 1.0 will lead to approximately equilateral triangles.\n"
                 "Start with 1.0 and increase this value to reduce the number of triangles.\n"
                 "Recommendation: 1.0 < K < 3.0.";
            m_pdeGrowthFactor->setToolTip(tip);

            m_pchDelaunayFlip = new QCheckBox("Make Delaunay flips");
            tip = "<p>https://en.wikipedia.org/wiki/Delaunay_triangulation#Visual_Delaunay_definition:_Flipping <br>"
                     "Will eliminate many skinny triangles, but may occasionally damage the geometry.<br>"
                     "Recommendation: activate!</p>";
            m_pchDelaunayFlip->setToolTip(tip);

            m_pchSplitInnerPSLG = new QCheckBox("Splittable inner edges");
            m_pchSplitInnerPSLG->setToolTip("<p>In the case of FACEs with nested holes, allows the inner EDGEs to be split.<br>"
                                            "Recommendation: activate only in the case of FACEs with inner holes.</p>");

            m_ppbMakeTriMesh = new QPushButton("Make mesh");
            m_ppbMakeTriMesh->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_M));

            pParamsLayout->addWidget(m_pchPickEdge,                1,1,1,3);
            pParamsLayout->addWidget(pLabEdgeSize,                 2,1);
            pParamsLayout->addWidget(m_pdeMaxEdgeSize,             2,2);
            pParamsLayout->addWidget(pLabLengthUnit,               2,3);
            pParamsLayout->addWidget(pLabMaxPanels,                3,1);
            pParamsLayout->addWidget(m_pieMaxPanelCount,           3,2);
            pParamsLayout->addWidget(pLabMaxPanelUnit,             3,3);
            pParamsLayout->addWidget(pLabMergeDist,                4,1);
            pParamsLayout->addWidget(m_pdeNodeMergeDistance,       4,2);
            pParamsLayout->addWidget(pLabLengthUnit2,              4,3);
            pParamsLayout->addWidget(pLabSearchFact,               5,1);
            pParamsLayout->addWidget(m_pdeSearchRadiusFactor,      5,2);
            pParamsLayout->addWidget(pLabGrowthFact,               6,1);
            pParamsLayout->addWidget(m_pdeGrowthFactor,            6,2);
            pParamsLayout->addWidget(m_pchDelaunayFlip,            7,1,1,3);
            pParamsLayout->addWidget(m_pchSplitInnerPSLG,          8,1,1,3);
            pParamsLayout->addWidget(m_ppbMakeTriMesh,             9,1,1,3);
            pParamsLayout->setColumnStretch(3,1);
        }

        m_pTraceBox = new QGroupBox("Debug TopoDS_Face mesh");
        {
            QGridLayout *pTraceLayout = new QGridLayout;
            {
                QLabel *plabFaceTrace = new QLabel("Face index:");
                plabFaceTrace->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                m_pieFaceIdx = new IntEdit(-1);
                m_pieFaceIdx->setToolTip("<p>Index of the face to mesh;<br>"
                                           "-1 to mesh all faces.</p>");
                QLabel *plabIter = new QLabel("Iterations:");
                plabIter->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                m_pieIter = new IntEdit(-1);
                m_pieIter->setToolTip("<p>The max. number of mesh iterations = approx. number of triangles;<br>"
                                        "-1 to make all triangles.</p>");
                pTraceLayout->addWidget(plabFaceTrace,   2, 1, Qt::AlignRight);
                pTraceLayout->addWidget(m_pieFaceIdx,    2, 2);
                pTraceLayout->addWidget(plabIter,        2, 3, Qt::AlignRight);
                pTraceLayout->addWidget(m_pieIter,       2, 4);

                m_pchAnimate = new QCheckBox("Animate");
                m_pchAnimate->setToolTip("<p>Animation fun in the shape view;<br>"
                                         "Activate and start the mesh.<br>"
                                         "Uncheck to interrupt.</p>");
                m_pieAnimInterval = new IntEdit(AFMesher::animationPause());
                m_pieAnimInterval->setMin(0);
                m_pieAnimInterval->setToolTip("<p>Animation fun in the shape view;<br>"
                                             "Delay between view updates in ms.</p>");
                m_pieAnimInterval->setEnabled(false);

                QLabel *plabms = new QLabel("(ms)");
                plabms->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                pTraceLayout->addWidget(m_pchAnimate,      3,1);
                pTraceLayout->addWidget(m_pieAnimInterval, 3,2);
                pTraceLayout->addWidget(plabms,            3,3);
            }
            m_pTraceBox->setLayout(pTraceLayout);
            m_pTraceBox->setVisible(false);
        }

        QLabel *pFlow5Link = new QLabel;
        pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Modelling/AFMesher.html#Stepbystep>https://flow5.tech/docs/flow5_doc/Modelling/AFMesher.html</a>");
        pFlow5Link->setOpenExternalLinks(true);
        pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
        pFlow5Link->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

        pMainLayout->addLayout(pParamsLayout);
        pMainLayout->addWidget(m_pTraceBox);
        pMainLayout->addWidget(pFlow5Link);
        pMainLayout->addStretch();
    }
    setLayout(pMainLayout);

    m_pdeMaxEdgeSize->setValue(AFMesher::maxEdgeLength()*Units::mtoUnit());
    m_pieMaxPanelCount->setValue(AFMesher::maxPanelCount());
    m_pdeNodeMergeDistance->setValue(XflMesh::nodeMergeDistance()*Units::mtoUnit());
    m_pchDelaunayFlip->setChecked(true);
    m_pdeSearchRadiusFactor->setValue(AFMesher::searchRadiusFactor());
    m_pdeGrowthFactor->setValue(AFMesher::growthFactor());
}


void MesherWt::loadSettings(QSettings &settings)
{
    settings.beginGroup("MesherWt");
    {
        AFMesher::setMaxEdgeLength(settings.value("MaxEdgeLength", AFMesher::maxEdgeLength()).toDouble());

        int count = settings.value("MaxPanelCount",     AFMesher::maxPanelCount()).toInt();
        if(count==0) count = 1000; // fixing past errors
        AFMesher::setMaxPanelCount(count);
        double radius = settings.value("SearchRadiusCoef",  AFMesher::searchRadiusFactor()).toDouble();
        if(fabs(radius)<0.05) radius = 1.0; // fixing past errors
        AFMesher::setSearchRadiusFactor(radius);
        double growth = settings.value("GrowthFactor",      AFMesher::growthFactor()).toDouble();
        if(fabs(growth)<0.05) growth = 1.0; // fixing past errors
        AFMesher::setGrowthFactor(growth);
        double nodemergedist = settings.value("NodeMergeDistance", XflMesh::nodeMergeDistance()).toDouble();
        if(nodemergedist<1.e-6) nodemergedist=1.e-4;
        XflMesh::setNodeMergeDistance(nodemergedist);
    }
    settings.endGroup();
}


void MesherWt::saveSettings(QSettings &settings)
{
    settings.beginGroup("MesherWt");
    {
        settings.setValue("MaxEdgeLength",     AFMesher::maxEdgeLength());
        settings.setValue("MaxPanelCount",     AFMesher::maxPanelCount());
        settings.setValue("SearchRadiusCoef",  AFMesher::searchRadiusFactor());
        settings.setValue("GrowthFactor",      AFMesher::growthFactor());
        settings.setValue("NodeMergeDistance", XflMesh::nodeMergeDistance());
    }
    settings.endGroup();
}


void MesherWt::connectSignals()
{
    connect(m_ppbMakeTriMesh,       SIGNAL(clicked(bool)),        SLOT(onMakeMesh()));
    connect(m_pdeMaxEdgeSize,       SIGNAL(floatChanged(float)),  SLOT(onMaxEdgeSize()));
    connect(m_pdeNodeMergeDistance, SIGNAL(floatChanged(float)),  SLOT(onReadParams()));
    connect(m_pieMaxPanelCount,     SIGNAL(intChanged(int)),      SLOT(onReadParams()));
    connect(m_pchDelaunayFlip,      SIGNAL(clicked(bool)),        SLOT(onReadParams()));

    connect(m_pieFaceIdx,           SIGNAL(intChanged(int)),      SLOT(onReadMeshDebugParams()));
    connect(m_pieIter,              SIGNAL(intChanged(int)),      SLOT(onReadMeshDebugParams()));
    connect(m_pchAnimate,           SIGNAL(clicked(bool)),        SLOT(onAnimation()));
    connect(m_pieAnimInterval,      SIGNAL(intChanged(int)),      SLOT(onAnimation()));
    connect(m_pchSplitInnerPSLG,    SIGNAL(clicked()),            SLOT(onSlittableInnerPSLG()));
}


void MesherWt::onSlittableInnerPSLG()
{
   m_bSplittableInnerPSLG = m_pchSplitInnerPSLG->isChecked();
}


void MesherWt::onAnimation()
{
    m_pieAnimInterval->setEnabled(m_pchAnimate->isChecked());
    AFMesher::setAnimating(m_pchAnimate->isChecked());
    AFMesher::setAnimationPause(m_pieAnimInterval->value());
}


void MesherWt::onReadMeshDebugParams()
{
    AFMesher::setTraceFaceIndex(m_pieFaceIdx->value());
    AFMesher::setMaxIterations(m_pieIter->value());
}


void MesherWt::hideEvent(QHideEvent *)
{
    onReadParams();
}


void MesherWt::initDialog(TopoDS_Shape const &shape, QVector<QVector<EdgeSplit>> const &edgesplits, bool bSplittableInnerPSLG)
{
    m_bMakexzSymmetric = false;
    m_bSplittableInnerPSLG = bSplittableInnerPSLG;
    m_EdgeSplit = edgesplits;

    m_Shapes.Clear();
    m_Shapes.Append(shape);

    QString strange;
    occ::listShapeContent(shape, strange, "      ");
    emit outputMsg(strange+"\n");
    setControls();
}


void MesherWt::initDialog(QVector<TopoDS_Shape> const &shapes, bool bSplittableInnerPSLG)
{
    m_bMakexzSymmetric = false;
    m_Shapes.Clear();
    m_bSplittableInnerPSLG = bSplittableInnerPSLG;

    QString strange;
    for(int iShape=0; iShape<shapes.size(); iShape++)
    {
        TopExp_Explorer shapeExplorer;
        int iShell=0;
        for (shapeExplorer.Init(shapes.at(iShape), TopAbs_SHELL); shapeExplorer.More(); shapeExplorer.Next())
        {
            try
            {
                TopoDS_Shell shell = TopoDS::Shell(shapeExplorer.Current());
                if(!shell.IsNull())
                {
                    m_Shapes.Append(shell);
                    strange = QString::asprintf("   Shape %d / Shell %d ", iShape, iShell);
                    if     (shell.Orientation()==TopAbs_FORWARD)  strange += " is FORWARD";
                    else if(shell.Orientation()==TopAbs_REVERSED) strange += " is REVERSED";
                    emit outputMsg(strange+"\n");

                    occ::listShapeContent(shell, strange, "      ");
                    emit outputMsg(strange+"\n");

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
    setControls();
}


void MesherWt::initDialog(TopoDS_ListOfShape const &shells, double maxedgelength, bool bMakeXZSymmetric, bool bSplittableInnerPSLG)
{
    AFMesher::setMaxEdgeLength(maxedgelength);
    m_bMakexzSymmetric = bMakeXZSymmetric;
    m_bSplittableInnerPSLG = bSplittableInnerPSLG;
    m_Shapes.Clear();

    QString strange, strong;
    int iShell=0;
    double Xmin(1e10), Ymin(1e10), Zmin(1e10), Xmax(-1e10), Ymax(-1e10), Zmax(-1e10);

    for(TopTools_ListIteratorOfListOfShape iterator(shells); iterator.More(); iterator.Next())
    {
        m_Shapes.Append(iterator.Value());
        strange = QString::asprintf("   Shell %d ", iShell);
        if     (iterator.Value().Orientation()==TopAbs_FORWARD)  strange += " is FORWARD";
        else if(iterator.Value().Orientation()==TopAbs_REVERSED) strange += " is REVERSED";
        emit outputMsg(strange+"\n");

        occ::shapeBoundingBox(iterator.Value(), Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);
        occ::listShapeContent(iterator.Value(), strange);
        emit outputMsg(strange+"\n");
        iShell++;
    }
    strange = "Bounding box:\n";
    strong = QString::asprintf("   X=[%9g, %9g] ", Xmin*Units::mtoUnit(), Xmax *Units::mtoUnit());
    strange += strong + Units::lengthUnitLabel() +"\n";
    strong = QString::asprintf("   Y=[%9g, %9g] ", Ymin*Units::mtoUnit(), Ymax *Units::mtoUnit());
    strange += strong + Units::lengthUnitLabel() +"\n";
    strong = QString::asprintf("   Z=[%9g, %9g] ", Zmin*Units::mtoUnit(), Zmax *Units::mtoUnit());
    strange += strong + Units::lengthUnitLabel() +"\n\n";
    emit outputMsg(strange);
    setControls();
}


void MesherWt::initDialog(Sail *pSail)
{
    m_bMakexzSymmetric = false;
    m_bSplittableInnerPSLG = false;
    m_Shapes.Clear();

    m_pSail = pSail;

    setControls();
}


void MesherWt::setControls()
{
    m_pdeMaxEdgeSize->setValue(AFMesher::maxEdgeLength()*Units::mtoUnit());
    m_pieMaxPanelCount->setValue(AFMesher::maxPanelCount());
    m_pdeNodeMergeDistance->setValue(XflMesh::nodeMergeDistance()*Units::mtoUnit());
    m_pchDelaunayFlip->setChecked(AFMesher::bDelaunayFlips());
    m_pdeSearchRadiusFactor->setValue(AFMesher::searchRadiusFactor());
    m_pdeGrowthFactor->setValue(AFMesher::growthFactor());

    m_pieFaceIdx->setValue(AFMesher::traceFaceIndex());
    m_pieIter->setValue(AFMesher::maxIterations());

    m_pchAnimate->setChecked(AFMesher::isAnimating());
    m_pieAnimInterval->setEnabled(AFMesher::isAnimating());
    m_pieAnimInterval->setValue(AFMesher::animationPause());

    m_pchSplitInnerPSLG->setChecked(m_bSplittableInnerPSLG);
}


void MesherWt::onMaxEdgeSize()
{
    AFMesher::setMaxEdgeLength(     m_pdeMaxEdgeSize->value()/Units::mtoUnit());
    if(m_pSail) m_pSail->setMaxElementSize(m_pdeMaxEdgeSize->value()/Units::mtoUnit());
    emit updateShapeView();
}


void MesherWt::onReadParams()
{
    AFMesher::setMaxPanelCount(     m_pieMaxPanelCount->value());
    AFMesher::setMaxEdgeLength(     m_pdeMaxEdgeSize->value()/Units::mtoUnit());
    AFMesher::setSearchRadiusFactor(m_pdeSearchRadiusFactor->value());
    AFMesher::setGrowthFactor(      m_pdeGrowthFactor->value());
    XflMesh::setNodeMergeDistance(  m_pdeNodeMergeDistance->value()/Units::mtoUnit());
    AFMesher::setDelaunayFlips(     m_pchDelaunayFlip->isChecked());
}


void MesherWt::onMakeMesh()
{
    onReadParams();

    if(m_pSail)
    {
        onMakeSailMesh();
        return;
    }

    if(m_Shapes.Extent()<1)
    {
        AFMesher::setCancelled(true);
        m_ppbMakeTriMesh->setText("Make mesh");
        return;
    }
    if(m_bIsMeshing)
    {
        AFMesher::setCancelled(true);
        m_ppbMakeTriMesh->setText("Make mesh");
        emit outputMsg("   --- Cancelling mesh operation ---\n");
        
        return;
    }

    AFMesher::setCancelled(false);

    QString logmsg, strange;
    logmsg = "  Meshing shell with parameters\n";
    strange = QString::asprintf("      Max. edge length     = %g ", AFMesher::maxEdgeLength()*Units::mtoUnit());
    logmsg += strange + Units::lengthUnitLabel() + "\n";
    strange = QString::asprintf("      Node merge distance  = %g ", XflMesh::nodeMergeDistance()*Units::mtoUnit());
    logmsg += strange + Units::lengthUnitLabel() + "\n";
    strange = QString::asprintf("      Max. panels/face     = %d\n", AFMesher::maxPanelCount());
    logmsg += strange;

    emit outputMsg(logmsg+"\n");

    m_ppbMakeTriMesh->setText("Cancel mesh");
    

    m_bIsMeshing = true;

    if(m_pMesher) delete m_pMesher;
    m_pMesher = new AFMesher;
    m_pMesher->setParent(m_pParent);
    m_pMesher->clearShapes();
    m_pMesher->setSplittableInnerPSLG(m_bSplittableInnerPSLG);
    m_pMesher->setEdgeSplit(m_EdgeSplit);

    for(TopTools_ListIteratorOfListOfShape ShellIt(m_Shapes); ShellIt.More(); ShellIt.Next())
        m_pMesher->appendShape(ShellIt.Value());


    //run the instance asynchronously
    QThread *pThread = new QThread;
    m_pMesher->moveToThread(pThread);
    connect(m_pMesher, &AFMesher::meshFinished, this,      &MesherWt::onMeshFinished);
    connect(pThread,   &QThread::started,       m_pMesher, &AFMesher::triangulateShells);
    connect(pThread,   &QThread::finished,      pThread,   &QThread::deleteLater);

    QApplication::setOverrideCursor(Qt::BusyCursor);
    pThread->start();
//    m_pMesher->triangulateShells();
}


/** manual mesh */
void MesherWt::onMakeSailMesh()
{
    if(m_bIsMeshing)
    {
        m_bIsMeshing = false;
        AFMesher::s_bCancel = true;
        m_ppbMakeTriMesh->setText("Make mesh");
        QApplication::restoreOverrideCursor();
        return;
    }

    emit outputMsg("Making sail free mesh\n");
    m_pSail->clearTEIndexes();
    m_pSail->clearRefTriangles();
    m_pSail->updateStations();
    m_pSail->makeTriPanels(Vector3d());

    AFMesher::setCancelled(false);

    if(m_pMesher) delete m_pMesher;
    m_pMesher = new AFMesher;
    m_pMesher->setParent(m_pParent);
    m_pMesher->clearShapes();
    m_pMesher->setSail(m_pSail);
    m_pMesher->setEdgeSplit(m_EdgeSplit);

    connect(m_pMesher, &AFMesher::meshFinished, this,      &MesherWt::onMeshFinished);

    //run the instance asynchronously
    QThread *pThread = new QThread;
    m_pMesher->moveToThread(pThread);
    connect(pThread,   &QThread::started,  m_pMesher, &AFMesher::makeThinSailMesh);
    connect(pThread,   &QThread::finished, pThread,   &QThread::deleteLater);

    QApplication::setOverrideCursor(Qt::BusyCursor);
    pThread->start();
    m_bIsMeshing = true;
}


void MesherWt::onMeshFinished()
{
    QString strange;

    if(m_pMesher->bMeshError()) emit outputMsg("\n\n***********mesh error*********\n\n");

    m_Triangles = m_pMesher->triangles();

    if(m_bMakexzSymmetric)
    {
        strange = QString::asprintf("   Right side triangle count = %d\n", int(m_pMesher->triangles().size()));
        emit outputMsg("\n" + strange + "   Making symmetric left side panels\n");
        int nt = m_pMesher->triangles().size();
        for(int it=0; it<nt; it++)
        {
            Triangle3d t3 = m_pMesher->triangles().at(it);
            t3.makeXZsymmetric();
            m_Triangles.append(t3);
        }
        strange = QString::asprintf("   Total triangle count = %d\n", int(m_Triangles.size()));
        emit outputMsg(strange);
    }

    // done with the mesher
    if(m_pMesher)
    {
        delete m_pMesher;
        m_pMesher = nullptr;
    }

    MeshEvent *pMeshEvent = new MeshEvent(m_Triangles);
    pMeshEvent->setFinal(true);
    qApp->postEvent(m_pParent, pMeshEvent);
    
    emit updateFuseView();
    emit updateShapeView();
    

    AFMesher::setCancelled(false);
    m_bIsMeshing = false;
    m_ppbMakeTriMesh->setText("Make mesh");
    QApplication::restoreOverrideCursor();
}





