/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <algorithm>

#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>

#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepTools.hxx>

#include <interfaces/mesh/gmesh_globals.h>


#include "gmesherwt.h"

#include <api/flow5events.h>
#include <api/frame.h>
#include <api/fuse.h>
#include <api/fuseocc.h>
#include <api/fusexfl.h>
#include <api/nurbssurface.h>
#include <api/occ_globals.h>
#include <api/sail.h>
#include <api/sailnurbs.h>
#include <api/sailocc.h>
#include <api/sailspline.h>
#include <api/sailstl.h>
#include <api/sailwing.h>
#include <api/units.h>
#include <api/utils.h>
#include <api/wingxfl.h>


#include <core/xflcore.h>
#include <interfaces/mesh/gmesh_globals.h>
#include <interfaces/mesh/gmesher.h>
#include <interfaces/mesh/meshevent.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/widgets/customdlg/doublevaluedlg.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/globals/wt_globals.h>


int GMesherWt::s_idxAlgo = 0;

GMesherWt::GMesherWt(QWidget *pParent) : QFrame{pParent}
{
    m_pParent = pParent;
    m_pFuse = nullptr;
    m_pSail = nullptr;
    m_iLoggerStack = 0;

    m_bThickSurfaces   = false;
    m_bMakexzSymmetric = false;
    m_bIsMeshing       = false;

    setupLayout();

    m_pWorker = new GMesher;
    m_pWorker->moveToThread(&m_MeshThread);
    connect(&m_MeshThread, &QThread::finished,          m_pWorker,  &QObject::deleteLater);
    connect(this,          &GMesherWt::meshCurrent,     m_pWorker,  &GMesher::onMeshCurrentModel);
    connect(m_pWorker,     &GMesher::displayMessage,    m_ppto, &PlainTextOutput::onAppendQText, Qt::BlockingQueuedConnection);
    connect(m_pWorker,     &GMesher::meshDone,          this,       &GMesherWt::onHandleMeshResults);

    m_MeshThread.start();

    connect(&m_LogTimer, SIGNAL(timeout()), SLOT(onCheckLogger()));


    gmsh::option::setNumber("General.NumThreads", 16);

    std::string list;
    gmesh::listMainOptions(list);
    m_ppto->onAppendStdText(list);
    m_ppto->appendEOL();
}


GMesherWt::~GMesherWt()
{
    m_MeshThread.quit();
    m_MeshThread.wait();
}


void GMesherWt::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGridLayout*pMeshOptionLayout = new QGridLayout;
        {
            QLabel *plabLength1 = new QLabel(Units::lengthUnitQLabel());
            QLabel *plabLength2 = new QLabel(Units::lengthUnitQLabel());

            QLabel *plabMinSize = new QLabel("Min. element size=");
            QLabel *plabMaxSize = new QLabel("Max. element size=");
            m_pfeMinSize = new FloatEdit(0.01f); // meters
            m_pfeMinSize->setToolTip("<p>The minimum acceptable size of the triangles. "
                                     "This value should be stricly positive to prevent "
                                     "excessively fine meshes, high number of elements. and long meshing times."
                                     "</p>"
                                     "<p>"
                                     "Recommendation: start with a large value and reduce progressively."
                                     "</p>"
                                     "<p>"
                                     "Min.acceptable value= 0.1 mm"
                                     "</p>");
            m_pfeMaxSize = new FloatEdit(10.0f); // meters
            m_pfeMaxSize->setToolTip("<p>The maximum acceptable size of the triangles. "
                                     "Reduce the value to increase the mesh density.</p>");

            QLabel *plabFromCurvature = new QLabel("<p>Nbr. of elements=</p>");
            m_pieFromCurvature = new IntEdit(20);
            m_pieFromCurvature->setToolTip("<p> Automatically compute mesh element sizes from curvature, "
                                        "using the value as the target number of elements per 2&pi;.<br>"
                                        "This is the main parameter that should be used to control mesh density.<br>"
                                        "Recommendation: between 10 and 50.</p>");
            QLabel *plab2Pi = new QLabel("<p>/2&pi;</p>");

            QLabel *plabAlgo = new QLabel("Algorithm:");
            m_pcbMeshAlgo = new QComboBox;
            {
                m_pcbMeshAlgo->addItem("MeshAdapt",                  1);
                m_pcbMeshAlgo->addItem("Automatic",                  2);
                m_pcbMeshAlgo->addItem("Initial mesh only",          3);
                m_pcbMeshAlgo->addItem("Delaunay",                   5);
                m_pcbMeshAlgo->addItem("Frontal-Delaunay",           6);
                m_pcbMeshAlgo->addItem("BAMG",                       7);
                m_pcbMeshAlgo->addItem("Frontal-Delaunay for Quads", 8);
                m_pcbMeshAlgo->addItem("Packing of Parallelograms",  9);
                m_pcbMeshAlgo->addItem("Quasi-structured Quad",     11);

                m_pcbMeshAlgo->setCurrentIndex(s_idxAlgo);

                QString strange = "<p>"
                                  "Extract from https://gmsh.info/doc/texinfo/gmsh.html#Choosing-the-right-unstructured-algorithm"
                                  "</p>"
                                  "<p>"
                                  "For all 2D unstructured algorithms a Delaunay mesh that contains all the points of the 1D mesh is initially constructed "
                                  "using a divide-and-conquer algorithm. Missing edges are recovered using edge swaps."
                                  "</p>"
                                  "<p>"
                                  "After this initial step several algorithms can be applied to generate the final mesh:"
                                  "<ul>"
                                  "   <li>The MeshAdapt algorithm is based on local mesh modifications. "
                                  "       This technique makes use of edge swaps, splits, and collapses: long edges are split, short edges are collapsed, "
                                  "       and edges are swapped if a better geometrical configuration is obtained.</li>"
                                  "   <li>The Delaunay algorithm is inspired by the work of the GAMMA team at INRIA. "
                                  "       New points are inserted sequentially at the circumcenter of the element that has the largest "
                                  "       adimensional circumradius. The mesh is then reconnected using an anisotropic Delaunay criterion.</li>"
                                  "   <li>The Frontal-Delaunay algorithm is inspired by the work of S. Rebay.</li>"
                                  "   <li>Other experimental algorithms with specific features are also available. "
                                  "       In particular, Frontal-Delaunay for Quads is a variant of the Frontal-Delaunay "
                                  "       algorithm aiming at generating right-angle triangles suitable for recombination; "
                                  "       and BAMG allows to generate anisotropic triangulations.</li>"
                                  "</ul>"
                                  "</p>"
                                  "<p>"
                                  "For very complex curved surfaces the MeshAdapt algorithm is the most robust. "
                                  "When high element quality is important, the Frontal-Delaunay algorithm should be tried. "
                                  "For very large meshes of plane surfaces the Delaunay algorithm is the fastest; "
                                  "it usually also handles complex mesh size fields better than the Frontal-Delaunay. "
                                  "When the Delaunay or Frontal-Delaunay algorithms fail, MeshAdapt is automatically triggered. "
                                  "The “Automatic” algorithm uses Delaunay for plane surfaces and MeshAdapt for all other surfaces."
                                  "</p>";
                m_pcbMeshAlgo->setToolTip(strange);

            }

            m_ppbMesh = new QPushButton("Mesh");
            connect(m_ppbMesh, SIGNAL(clicked()), SLOT(onMesh()));


            pMeshOptionLayout->addWidget(plabMinSize,        1, 1);
            pMeshOptionLayout->addWidget(m_pfeMinSize,       1, 2);
            pMeshOptionLayout->addWidget(plabLength1,        1, 3);
            pMeshOptionLayout->addWidget(plabMaxSize,        2, 1);
            pMeshOptionLayout->addWidget(m_pfeMaxSize,       2, 2);
            pMeshOptionLayout->addWidget(plabLength2,        2, 3);
            pMeshOptionLayout->addWidget(plabFromCurvature,  3, 1);
            pMeshOptionLayout->addWidget(m_pieFromCurvature, 3, 2);
            pMeshOptionLayout->addWidget(plab2Pi,            3, 3);
            pMeshOptionLayout->addWidget(plabAlgo,           4, 1);
            pMeshOptionLayout->addWidget(m_pcbMeshAlgo,      4, 2);
            pMeshOptionLayout->addWidget(m_ppbMesh,          5, 1, 1, 2);
            pMeshOptionLayout->setColumnStretch(4,1);
        }

        QLabel *plabGmshOutput = new QLabel("Gmsh:");
        m_ppto = new PlainTextOutput;

        pMainLayout->addLayout(pMeshOptionLayout);
        pMainLayout->addWidget(plabGmshOutput);
        pMainLayout->addWidget(m_ppto);
    }
    setLayout(pMainLayout);
}


bool GMesherWt::readMeshSize()
{
    GmshParams m_GmshParams;

    double const MINSIZE = 0.0001;
    double size = m_pfeMinSize->value()/Units::mtoUnit();
    if(size<MINSIZE)
    {
        QString str = QString::asprintf("%g", MINSIZE*Units::mtoUnit()) + Units::lengthUnitQLabel();
        QMessageBox::warning(this, tr("Warning"), tr("The min. element size must be at least ") + str);
        m_pfeMinSize->setValue(MINSIZE*Units::mtoUnit());
        m_pfeMinSize->selectAll();
        m_pfeMinSize->setFocus();
        return false;
    }

    m_GmshParams.m_MinSize = m_pfeMinSize->value()/Units::mtoUnit();
    m_GmshParams.m_MaxSize = m_pfeMaxSize->value()/Units::mtoUnit();
    m_GmshParams.m_nCurvature = m_pieFromCurvature->value();
    if     (m_pFuse) m_pFuse->setGmshParams(m_GmshParams);
    else if(m_pSail) m_pSail->setGmshParams(m_GmshParams);

    gmsh::option::setNumber("Mesh.MeshSizeMin", m_GmshParams.m_MinSize);
    gmsh::option::setNumber("Mesh.MeshSizeMax", m_GmshParams.m_MaxSize);
    gmsh::option::setNumber("Mesh.MeshSizeFromCurvature", m_GmshParams.m_nCurvature);

    return true;
}


void GMesherWt::initWt(Fuse *pFuse, bool bMakexzSymmetric, bool bThickSurfaces)
{
    m_pFuse = pFuse;

    m_bThickSurfaces = bThickSurfaces;
    m_bMakexzSymmetric = bMakexzSymmetric;

    GmshParams m_GmshParams = pFuse->gmshParams();
    m_pfeMinSize->setValue(m_GmshParams.m_MinSize*Units::mtoUnit());
    m_pfeMaxSize->setValue(m_GmshParams.m_MaxSize*Units::mtoUnit());
    m_pieFromCurvature->setValue(m_GmshParams.m_nCurvature);
}


void GMesherWt::initWt(Sail *pSail)
{
    m_pSail = pSail;
    GmshParams m_GmshParams = m_pSail->gmshParams();
    m_pfeMinSize->setValue(m_GmshParams.m_MinSize*Units::mtoUnit());
    m_pfeMaxSize->setValue(m_GmshParams.m_MaxSize*Units::mtoUnit());
    m_pieFromCurvature->setValue(m_GmshParams.m_nCurvature);
}


void GMesherWt::loadSettings(QSettings &settings)
{
    settings.beginGroup("GMesherWt");
    {
        s_idxAlgo = settings.value("Algorithm", s_idxAlgo).toInt();
    }
    settings.endGroup();
}


void GMesherWt::saveSettings(QSettings &settings)
{
    settings.beginGroup("GMesherWt");
    {
        settings.setValue("Algorithm", s_idxAlgo);
    }
    settings.endGroup();
}


void GMesherWt::onKillMeshThread()
{
    // Don't.
    //
    // Warning: This function is dangerous and its use is discouraged.
    // The thread can be terminated at any point in its code path.
    // Threads can be terminated while modifying data.
    // There is no chance for the thread to clean up after itself, unlock any held mutexes, etc.
    // In short, use this function only if absolutely necessary.

    m_MeshThread.terminate();
    m_MeshThread.wait();
}


int GMesherWt::meshAlgo()
{
    bool bOK(false);
    int iAlgo = m_pcbMeshAlgo->currentData().toInt(&bOK);

    s_idxAlgo = m_pcbMeshAlgo->currentIndex();

    if(!bOK)
    {
        s_idxAlgo = 0;
        iAlgo = 1;
    }

    emit outputMsg("Using mesh algo.: " + m_pcbMeshAlgo->currentText()+EOLch);
    return iAlgo;
}


void GMesherWt::onCheckLogger()
{
    std::vector<std::string>  log;
    gmsh::logger::get(log);


    for(uint i=m_iLoggerStack; i<log.size(); i++)
    {
        m_ppto->onAppendQText(QString::fromStdString(log.at(i))+EOLch);
    }

    m_iLoggerStack = int(log.size());
}


void GMesherWt::convertFromGmsh()
{
    QString strange;
    std::vector<std::size_t> elementTags;
    std::vector<std::size_t> nodeTags;
    const int ElementType = 2; // triangles

    gmsh::model::mesh::getElementsByType(ElementType, elementTags, nodeTags);

    strange = QString::asprintf("Built %d type 2 elements\n", int(elementTags.size()));
    emit outputMsg(strange);

    if(elementTags.size()<=0)
    {
        emit outputMsg("No triangles to convert\n\n");
        return;
    }

    std::vector<std::size_t> nodetags;
    std::vector<double> coord;
    std::vector<double> parametricCoord;
    int dim = -1;
    int tag = -1;
    gmsh::model::mesh::getNodes(nodetags,
                                coord,
                                parametricCoord,
                                dim, tag,
                                false, false);
    strange = QString::asprintf("Built %d nodes and %d coordinates\n", int(nodetags.size()), int(coord.size()));
    emit outputMsg(strange);
    Q_ASSERT(coord.size() == 3*nodetags.size());

    if(nodetags.size()<=0)
    {
        emit outputMsg("Aborting\n\n");
        return;
    }

    // need to make an array of nodes for quick access
    // option 1: make a map (tag, position); slow?
    // option 2: make an array with index=tag;
    size_t maxtag = *std::max_element(nodetags.begin(), nodetags.end());

    m_Nodes.resize(maxtag+1);
    for(uint j=0; j<nodetags.size(); j++)
    {
        int inode = int(nodetags.at(j));
        m_Nodes[inode].set(&coord.at(3*j+0));
        m_Nodes[inode].setIndex(inode);
        m_Nodes[inode].clearNeighbourNodes();
        m_Nodes[inode].clearTriangles();
    }

//    convertTriangles(elementTags, node);

    std::vector<std::size_t> elementnodetags;
    int elementtype = -1;

    m_Triangles.resize(elementTags.size());

    double minsize(LARGEVALUE), maxsize(0);
    for(uint i=0; i<elementTags.size(); i++)
    {
        int ielementtag = int(elementTags.at(i));
        gmsh::model::mesh::getElement(ielementtag,
                                      elementtype,
                                      elementnodetags,
                                      dim,
                                      tag);
        Q_ASSERT(elementtype==2); // Triangle
        Q_ASSERT(dim==2); // Surface type element

        Triangle3d &t3d = m_Triangles[i];
        for(uint j=0; j<elementnodetags.size(); j++)
        {
            int nodetag = int(elementnodetags.at(j));
            m_Nodes[nodetag].addTriangleIndex(i);
            t3d.setVertex(j, m_Nodes.at(nodetag));
            t3d.setVertexIndex(j, nodetag);
        }
        t3d.setTriangle();
        minsize = std::min(minsize, t3d.minEdgeLength());
        maxsize = std::max(maxsize, t3d.maxEdgeLength());
    }

    // make the connections using the node information
    for(Triangle3d &t3d : m_Triangles)
    {
        t3d.clearConnections();
        for(int iedge=0; iedge<3; iedge++)
        {
            Node const &nd0 = m_Nodes.at(t3d.nodeIndex(iedge));
            Node const &nd1 = m_Nodes.at(t3d.nodeIndex(iedge+1));
            for(int nd0t=0; nd0t<nd0.triangleCount(); nd0t++)
            {
                int t3dindex = nd0.triangleIndex(nd0t);
                if(nd1.hasTriangleIndex(t3dindex))
                {
                    t3d.setNeighbour(t3dindex, iedge);
                    break;
                }
            }
        }
    }

    emit outputMsg(QString::asprintf("Min. element size = %g\n", minsize));
    emit outputMsg(QString::asprintf("Max. element size = %g\n", maxsize));
}


void GMesherWt::onHandleMeshResults(bool bError)
{
    while(QApplication::overrideCursor()!=nullptr)
        QApplication::restoreOverrideCursor();

    QApplication::setOverrideCursor(Qt::BusyCursor);
    m_LogTimer.stop();
    emit outputMsg("Handling mesh results...\n");
    if(bError) emit outputMsg(" ...Gmsh error reported...\n");

    std::string error;
    gmsh::logger::getLastError(error);

    if(error.length())
    {
        emit outputMsg("---ERROR---\n" + QString::fromStdString(error)+EOLch+EOLch);
    }

    onCheckLogger();
    gmsh::logger::stop();
    convertFromGmsh();


    QString strange;

    if(bError)
    {
        strange = "\n\n***********mesh error*********\n\n";
        emit outputMsg(strange);
    }

    double nT3d(0), nNodes(0);
    gmsh::option::getNumber("Mesh.NbTriangles", nT3d);
    gmsh::option::getNumber("Mesh.NbNodes", nNodes);
    strange = QString::asprintf("Gmsh count: %.0f triangles and %.0f nodes\n", nT3d, nNodes);
    emit outputMsg(strange);

    if(m_bMakexzSymmetric)
    {
        int nt = int(m_Triangles.size());
        strange = QString::asprintf("   Right side triangle count = %d\n", nt);
        emit outputMsg("\n" + strange + "   Making symmetric left side panels\n");
        for(int it=0; it<nt; it++)
        {
            Triangle3d t3 = m_Triangles.at(it);
            t3.makeXZsymmetric();
            m_Triangles.push_back(t3);
        }
        strange = QString::asprintf("   Total triangle count = %d\n", int(m_Triangles.size()));
        emit outputMsg(strange);
    }

    MeshEvent *pMeshEvent = new MeshEvent(m_Triangles);
    pMeshEvent->setFinal(true);
    qApp->postEvent(m_pParent, pMeshEvent);

    emit updateFuseView();

    m_bIsMeshing = false;
    m_ppbMesh->setText("Make mesh");
    QApplication::restoreOverrideCursor();
    setEnabled(true);
}


void GMesherWt::meshNURBSSail()
{
    SailNurbs *pNurbsSail = dynamic_cast<SailNurbs*>(m_pSail);
    if(!pNurbsSail) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_Triangles.clear();
    gmsh::logger::start();
    m_iLoggerStack = 0;
    gmsh::clear();
    gmsh::model::mesh::clear();

    gmsh::model::add("NURBS Surface");

    gmsh::option::setNumber("Mesh.Algorithm", meshAlgo());

    readMeshSize();
    onCheckLogger();

    std::vector<int> PointsU;
    for(int i=0; i<pNurbsSail->nurbs().nFrames(); i++)
    {
        Frame const &frame = pNurbsSail->nurbs().frameAt(i);
        for(int j=0; j<frame.nCtrlPoints(); j++)
        {
            Vector3d pt = frame.ctrlPointAt(j);
            if(fabs(frame.angle())>ANGLEPRECISION) //degrees
                pt.rotateY(frame.position(), frame.angle());

            PointsU.push_back(gmsh::model::occ::addPoint(pt.x, pt.y, pt.z));

        }
    }

    try
    {
        int tag = gmsh::model::occ::addBSplineSurface(PointsU, pNurbsSail->sideLineCount());
        emit outputMsg(QString::asprintf("gmsh NURBS tag = %d\n", tag));
    }
    catch(...)
    {
        emit outputMsg("Error making NurbsSail spline surface\n\n");
        return;
    }

    onCheckLogger();

    gmsh::model::occ::synchronize();

    QApplication::restoreOverrideCursor();

    emit outputMsg("Starting G-mesher in separate thread...\n");

    emit meshCurrent();
}


void GMesherWt::meshSplineSail()
{
    SailSpline *pSplineSail = dynamic_cast<SailSpline*>(m_pSail);
    if(!pSplineSail) return;


    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_Triangles.clear();
    gmsh::logger::start();
    m_iLoggerStack = 0;
    gmsh::clear();
    gmsh::model::mesh::clear();

    gmsh::model::add("Spline surface");
    gmsh::option::setNumber("Mesh.Algorithm", meshAlgo());

    onCheckLogger();

    std::vector<int> surface;
    int nCtrlPts = pSplineSail->firstSpline()->nCtrlPoints();

    for(int i=0; i<pSplineSail->nSections()-1; i++)
    {
        std::vector<int> PtsBot, PtsTop, PtsFront, PtsBack;

        for(int j=0; j<nCtrlPts; j++)
        {
            Vector3d pt = pSplineSail->sectionPoint(i, j);
            PtsBot.push_back(gmsh::model::occ::addPoint(pt.x, pt.y, pt.z));
        }

        for(int j=0; j<nCtrlPts; j++)
        {
            Vector3d pt = pSplineSail->sectionPoint(i+1, j);
            PtsTop.push_back(gmsh::model::occ::addPoint(pt.x, pt.y, pt.z));
        }

        Vector3d botfront = pSplineSail->sectionPoint(i,0);
        Vector3d botback  = pSplineSail->sectionPoint(i,nCtrlPts-1);
        Vector3d topfront = pSplineSail->sectionPoint(i+1,0);
        Vector3d topback  = pSplineSail->sectionPoint(i+1,nCtrlPts-1);

        PtsFront.push_back(gmsh::model::occ::addPoint(botfront.x, botfront.y, botfront.z));
        PtsFront.push_back(gmsh::model::occ::addPoint(topfront.x, topfront.y, topfront.z));

        PtsBack.push_back(gmsh::model::occ::addPoint(botback.x, botback.y, botback.z));
        PtsBack.push_back(gmsh::model::occ::addPoint(topback.x, topback.y, topback.z));

        int botspline   = gmsh::model::occ::addSpline(PtsBot);
        int topspline   = gmsh::model::occ::addSpline(PtsTop);
        int frontspline = gmsh::model::occ::addSpline(PtsFront);
        int backSpline  = gmsh::model::occ::addSpline(PtsBack);

        int loop = gmsh::model::occ::addCurveLoop({botspline, backSpline, topspline, frontspline});
        surface.push_back(gmsh::model::occ::addSurfaceFilling(loop));
    }

    try
    {        
        // 3 methods to glue the patches
        int method = 3; // all three methods work

        if (method == 1)
        {
            // healShapes performs surface sewing, which does the job; it will also
            // reorient the left patch so that its normal points along +z
            gmsh::vectorpair outDimTags;
            gmsh::model::occ::healShapes(outDimTags);
        }
        else if (method == 2)
        {
            // we can also fragment; here we also include points so that the original
            // corner points will be used as corners of the patches
            gmsh::vectorpair outDimTags;
            std::vector<gmsh::vectorpair> outDimTagsMap;

            gmsh::vectorpair dimTags0, dimTags2;
            gmsh::model::occ::getEntities(dimTags0, 0);
            gmsh::model::occ::getEntities(dimTags2, 2);
            gmsh::model::occ::fragment(dimTags0,
                                       dimTags2,
                                       outDimTags,
                                       outDimTagsMap);
        }
        else if(method == 3)
        {
            // this is the same as fragment, but just on the 2 surfaces
            gmsh::model::occ::removeAllDuplicates();
        }
    }
    catch(...)
    {
        emit outputMsg("Error making spline surface\n\n");
        setEnabled(true);
        return;
    }

    onCheckLogger();

    gmsh::model::occ::synchronize();

    QApplication::restoreOverrideCursor();

    emit outputMsg("Starting G-mesher in separate thread...\n");

    emit meshCurrent();
}


void GMesherWt::meshOccSail()
{
    SailOcc *pOccSail = dynamic_cast<SailOcc*>(m_pSail);
    if(!pOccSail) return;


    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_Triangles.clear();
    gmsh::logger::start();
    m_iLoggerStack = 0;
    gmsh::clear();
    gmsh::model::mesh::clear();


    gmsh::model::add("Occ Surfaces");

    gmsh::option::setNumber("Mesh.Algorithm", meshAlgo());

    for(uint i=0; i<pOccSail->bReps().size(); i++)
        gmesh::BReptoGmsh(pOccSail->bReps().at(i));

    onCheckLogger();

    gmsh::model::occ::synchronize();

    QApplication::restoreOverrideCursor();

    emit outputMsg("Starting G-mesher in separate thread...\n");

    emit meshCurrent();
}


void GMesherWt::meshFuseShellsThinSurfaces()
{
    if(!m_pFuse) return;
    QApplication::setOverrideCursor(Qt::WaitCursor);

//    const int DIM0 = 0; // points
//    const int DIM1 = 1; // lines
    const int DIM2 = 2; // faces

    m_Triangles.clear();
    gmsh::logger::start();
    m_iLoggerStack = 0;

    gmsh::clear();
    gmsh::model::mesh::clear();

    gmsh::model::add("Occ Surfaces");

    gmsh::option::setNumber("Mesh.Algorithm", meshAlgo());

    onCheckLogger();

    // converting to BREP and export+import
    TopoDS_ListOfShape const *pShells(nullptr);
    FuseXfl const*pFuseXfl = dynamic_cast<FuseXfl const*>(m_pFuse);
    if(pFuseXfl) pShells = &pFuseXfl->rightSideShells();
    else         pShells = &m_pFuse->shells();
    std::vector<std::string> brepstr;
    occ::shapesToBreps(*pShells, brepstr);
    gmesh::BRepstoGmsh(brepstr);


    if(!m_Wings.size())
    {
    }
    else
    {
        // get all the fuse faces
        gmsh::vectorpair fusedimtags;
        gmsh::model::occ::getEntities(fusedimtags, DIM2);

        // embed wings
        // make the fragmenting tools

        gmsh::vectorpair wingDimTags;

        for (int iw=0; iw<m_Wings.size(); iw++)
        {
            WingXfl const &wing = m_Wings.at(iw);

    //        Vector3d wingLE = pWing->position(); // no need, surfaces are built in position
            Vector3d fuseLE = m_pFuse->position();
            Vector3d t = Vector3d()-fuseLE;

            std::vector<int> wiretags;
            std::vector<std::vector<Node>> midwires;
            wing.makeMidWires(midwires);

            for(std::vector<Node> const &wire : midwires)
            {
                std::vector<int> pointtags;
                for(Node const &nd : wire)
                {
                    pointtags.push_back(gmsh::model::occ::addPoint(nd.x+t.x, nd.y+t.y, nd.z+t.z));
                }

                std::vector<int> linetags;
                for(uint it=0; it<pointtags.size()-1; it++)
                {
                    linetags.push_back(gmsh::model::occ::addLine(pointtags.at(it), pointtags.at(it+1)));
                }

                wiretags.push_back(gmsh::model::occ::addWire(linetags));
            }

            gmsh::vectorpair surfDimTags;
            gmsh::model::occ::addThruSections(wiretags, surfDimTags, -1, false, true);

            for(std::pair<int,int> pair : surfDimTags)
                wingDimTags.push_back(pair);
        }

        gmsh::vectorpair outDimTags;
        std::vector<gmsh::vectorpair> outDimTagsMap;

        // Fragment and embed automatically
        // Note: remove tools does not remove the wings from the model, needs manual removal
        gmsh::model::occ::fragment(fusedimtags, wingDimTags, outDimTags, outDimTagsMap, -1, true, true);

        // Remove all dim 2 objects except the original fuse faces.
        gmsh::vectorpair RedundantDimTags;
        for(std::pair<int,int> dimtag : outDimTags)
        {
            if(dimtag.first==DIM2)
            {
                bool bIsFuseDimTag = false;
                for(uint kf=0; kf<fusedimtags.size(); kf++)
                {
                    if(dimtag.second==fusedimtags.at(kf).second)
                    {
                        bIsFuseDimTag = true;
                        break;
                    }
                }

                if(!bIsFuseDimTag) RedundantDimTags.push_back(dimtag);
            }
        }
        gmsh::model::occ::remove(RedundantDimTags, true);
    }

    gmsh::model::occ::synchronize();

    emit outputMsg("Starting G-mesher in a separate thread...\n");
    emit meshCurrent();

}


void GMesherWt::meshFuseShellsThickSurfaces()
{
    if(!m_pFuse) return;
    QApplication::setOverrideCursor(Qt::WaitCursor);

//    const int DIM0 = 0; // points
//    const int DIM1 = 1; // lines
    const int DIM2 = 2; // faces

    m_Triangles.clear();
    gmsh::logger::start();
    m_iLoggerStack = 0;

    gmsh::clear();
    gmsh::model::mesh::clear();

    gmsh::model::add("Occ Surfaces");

    gmsh::option::setNumber("Mesh.Algorithm", meshAlgo());

    onCheckLogger();

    // converting to BREP and export+import
    TopoDS_ListOfShape const *pShells(nullptr);
    FuseXfl const*pFuseXfl = dynamic_cast<FuseXfl const*>(m_pFuse);
    if(pFuseXfl) pShells = &pFuseXfl->rightSideShells();
    else         pShells = &m_pFuse->shells();
    std::vector<std::string> brepstr;
    occ::shapesToBreps(*pShells, brepstr);
    gmesh::BRepstoGmsh(brepstr);


    if(!m_Wings.size())
    {
        // the fuselage has been cut, no need to embed the wings
    }
    else
    {
        // get all the fuse faces
        gmsh::vectorpair fusedimtags;
        gmsh::model::occ::getEntities(fusedimtags, DIM2);

        // embed wings
        // make the cutting tools

        gmsh::vectorpair wingDimTags;

        for (int iw=0; iw<m_Wings.size(); iw++)
        {
            WingXfl const &wing = m_Wings.at(iw);

    //        Vector3d wingLE = pWing->position(); // no need, surfaces are built in position
            Vector3d fuseLE = m_pFuse->position();
            Vector3d t = Vector3d()-fuseLE;

            std::vector<int> wiretags;
            std::vector<std::vector<Node>> sectionwires;
            wing.makeTopBotWires(sectionwires);

            for(std::vector<Node> const &wire : sectionwires)
            {
                std::vector<int> pointtags;
                for(Node const &nd : wire)
                {
                    pointtags.push_back(gmsh::model::occ::addPoint(nd.x+t.x, nd.y+t.y, nd.z+t.z));
                }

                std::vector<int> linetags;
                for(uint it=0; it<pointtags.size()-1; it++)
                {
                    linetags.push_back(gmsh::model::occ::addLine(pointtags.at(it), pointtags.at(it+1)));
                }

                wiretags.push_back(gmsh::model::occ::addCurveLoop(linetags));
            }

            gmsh::vectorpair surfDimTags;
//            gmsh::model::occ::addThruSections(wiretags, surfDimTags, -1, true, true, -1, "C1", "ChordLength", true);
            gmsh::model::occ::addThruSections(wiretags, surfDimTags, -1, true, true);

            for(std::pair<int,int> pair : surfDimTags)
                wingDimTags.push_back(pair);
        }

        gmsh::vectorpair outDimTags;
        std::vector<gmsh::vectorpair> outDimTagsMap;

        // Fragment and embed automatically
        // Note: remove tools does not remove the wings from the model, needs manual removal
        gmsh::model::occ::cut(fusedimtags, wingDimTags, outDimTags, outDimTagsMap, -1, true, true);

        // Remove all dim 2 objects except the original fuse faces.
        gmsh::vectorpair RedundantDimTags;
        for(std::pair<int,int> dimtag : outDimTags)
        {
            if(dimtag.first==DIM2)
            {
                bool bIsFuseDimTag = false;
                for(uint kf=0; kf<fusedimtags.size(); kf++)
                {
                    if(dimtag.second==fusedimtags.at(kf).second)
                    {
                        bIsFuseDimTag = true;
                        break;
                    }
                }

                if(!bIsFuseDimTag) RedundantDimTags.push_back(dimtag);
            }
        }
        gmsh::model::occ::remove(RedundantDimTags, true);
    }

    gmsh::model::occ::synchronize();

    emit outputMsg("Starting G-mesher in a separate thread...\n");
    emit meshCurrent();

}


void GMesherWt::onMesh()
{
    if(!readMeshSize()) return;

    m_ppto->clear();

    setEnabled(false);
    if(m_pFuse)
    {
        if(m_bThickSurfaces)
            meshFuseShellsThickSurfaces();
        else
            meshFuseShellsThinSurfaces();
    }
    else if(m_pSail)
    {
        m_pSail->clearTEIndexes();
        m_pSail->clearRefTriangles();

        if     (m_pSail->isNURBSSail())  meshNURBSSail();
        else if(m_pSail->isSplineSail()) meshSplineSail();
        else if(m_pSail->isOccSail())    meshOccSail();
    }
    else
        setEnabled(true);
}






