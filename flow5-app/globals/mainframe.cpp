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


#include <QApplication>
#include <QScreen>
#include <QMenuBar>
#include <QClipboard>
#include <QToolTip>
#include <QProcess>

#include <QMessageBox>
#include <QKeyEvent>
#include <QToolBar>
#include <QDockWidget>
#include <QAction>


#include <QStatusBar>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>

#include <gmsh.h>

#ifdef OPENBLAS
    #include <openblas/cblas.h>
#elif defined INTEL_MKL
    #include <mkl.h>
#endif

#include "mainframe.h"


#include <core/displayoptions.h>
#include <core/saveoptions.h>
#include <core/trace.h>
#include <core/xflcore.h>
#include <globals/aboutf5.h>
#include <globals/creditsdlg.h>
#include <globals/gui_params.h>
#include <interfaces/controls/poppctrls/crossflowctrls.h>
#include <interfaces/controls/poppctrls/flowctrls.h>
#include <interfaces/controls/poppctrls/opp3dscalesctrls.h>
#include <interfaces/controls/poppctrls/streamlinesctrls.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/editors/boatedit/saildlg.h>
#include <interfaces/editors/foiledit/foildlg.h>
#include <interfaces/editors/fuseedit/fusemesherdlg.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fusexfldefdlg.h>
#include <interfaces/editors/planeedit/planexfldlg.h>
#include <interfaces/graphs/containers/fastgraphwt.h>
#include <interfaces/graphs/controls/graphdlg.h>
#include <interfaces/graphs/controls/graphoptions.h>
#include <interfaces/graphs/controls/graphtilectrls.h>
#include <interfaces/graphs/globals/graphsvgwriter.h>
#include <interfaces/mesh/gmesherwt.h>

#include <interfaces/opengl/globals/opengldlg.h>
#include <interfaces/opengl/testgl/gl2dcomplex.h>
#include <interfaces/opengl/testgl/gl2dfractal.h>
#include <interfaces/opengl/testgl/gl2dnewton.h>
#include <interfaces/opengl/testgl/gl2dquat.h>
#include <interfaces/opengl/testgl/gl3dattractors.h>
#include <interfaces/opengl/testgl/gl3dboids.h>
#include <interfaces/opengl/testgl/gl3dboids2.h>
#include <interfaces/opengl/testgl/gl3dflowvtx.h>
#include <interfaces/opengl/testgl/gl3dhydrogen.h>
#include <interfaces/opengl/testgl/gl3dlorenz.h>
#include <interfaces/opengl/testgl/gl3dlorenz2.h>
#include <interfaces/opengl/testgl/gl3dquat.h>
#include <interfaces/opengl/testgl/gl3dsagittarius.h>
#include <interfaces/opengl/testgl/gl3dshadow.h>
#include <interfaces/opengl/testgl/gl3dsolarsys.h>
#include <interfaces/opengl/testgl/gl3dspace.h>
#include <interfaces/opengl/testgl/gl3dsurface.h>
#include <interfaces/opengl/testgl/gl3dtestpoints.h>
#include <interfaces/opengl/testgl/gl3dtexture.h>
#include <interfaces/view2d/foilsvgwriter.h>
#include <interfaces/widgets/customdlg/logmessagedlg.h>
#include <interfaces/widgets/customdlg/objectpropsdlg.h>
#include <interfaces/widgets/customdlg/renamedlg.h>
#include <interfaces/widgets/customdlg/selectiondlg.h>
#include <interfaces/widgets/customdlg/separatorsdlg.h>
#include <interfaces/widgets/customwts/logwt.h>
#include <interfaces/widgets/line/linepicker.h>
#include <interfaces/widgets/view/section2doptions.h>
#include <modules/script/xflscriptexec.h>
#include <modules/xdirect/controls/analysis2dctrls.h>
#include <modules/xdirect/controls/foilexplorer.h>
#include <modules/xdirect/controls/foiltable.h>
#include <modules/xdirect/controls/oppointctrls.h>
#include <modules/xdirect/graphs/blgraphctrls.h>
#include <modules/xdirect/graphs/xdirectlegendwt.h>
#include <modules/xdirect/menus/xdirectactions.h>
#include <modules/xdirect/menus/xdirectmenus.h>
#include <modules/xdirect/mgt/foilplrlistdlg.h>
#include <modules/xdirect/view2d/dfoillegendwt.h>
#include <modules/xdirect/view2d/dfoilwt.h>
#include <modules/xdirect/view2d/oppointwt.h>
#include <modules/xdirect/xdirect.h>
#include <modules/xobjects.h>
#include <modules/xplane/analysis/analysis3dsettings.h>
#include <modules/xplane/analysis/planeanalysisdlg.h>
#include <modules/xplane/controls/analysis3dctrls.h>
#include <modules/xplane/controls/planeexplorer.h>
#include <modules/xplane/controls/popp3dctrls.h>
#include <modules/xplane/controls/stab3dctrls.h>
#include <modules/xplane/controls/stabtimectrls.h>
#include <modules/xplane/controls/xplanewt.h>
#include <modules/xplane/glview/gl3dxplaneview.h>
#include <modules/xplane/graphs/cpgraphctrls.h>
#include <modules/xplane/graphs/cpviewwt.h>
#include <modules/xplane/graphs/poppgraphctrls.h>
#include <modules/xplane/graphs/xplanelegendwt.h>
#include <modules/xplane/menus/xplaneactions.h>
#include <modules/xplane/menus/xplanemenus.h>
#include <modules/xplane/xplane.h>
#include <modules/xsail/controls/boatexplorer.h>
#include <modules/xsail/controls/gl3dxsailctrls.h>
#include <modules/xsail/controls/xsailanalysisctrls.h>
#include <modules/xsail/menus/xsailactions.h>
#include <modules/xsail/menus/xsailmenus.h>
#include <modules/xsail/view/gl3dxsailview.h>
#include <modules/xsail/xsail.h>
#include <options/prefsdlg.h>
#include <test/test3d/gl3daxesview.h>
#include <test/test3d/gl3dcontourview.h>
#include <test/test3d/gl3dflightview.h>
#include <test/test3d/gl3doptim2d.h>
#include <test/test3d/gl3dpanelfield.h>
#include <test/test3d/gl3dquadfield.h>
#include <test/test3d/gl3dsingularity.h>
#include <test/test3d/gl3dsurfaceplot.h>
#include <test/test3d/gl3dvortonfield.h>
#include <test/tests/attractor2d.h>
#include <test/tests/onevortontestdlg.h>
#include <test/tests/panel3testdlg.h>
#include <test/tests/panelanalysistest.h>
#include <test/tests/streamtestdlg.h>
#include <test/tests/threadtestdlg.h>
#include <test/tests/vortontestdlg.h>

#include <api/boat.h>
#include <api/boatpolar.h>
#include <api/fileio.h>
#include <api/fl5core.h>
#include <api/flow5-io.h>
#include <api/gmesh_globals.h>
#include <api/objects2d.h>
#include <api/objects2d_globals.h>
#include <api/objects3d.h>
#include <api/planepolar.h>
#include <api/planexfl.h>
#include <api/sailobjects.h>


xfl::enumApp MainFrame::s_iApp=xfl::NOAPP;

QString MainFrame::s_XflProjectPath;


bool MainFrame::s_bSaved = true;

MainFrame::MainFrame(QWidget *parent) : QMainWindow(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(QString::fromStdString(fl5::versionName(true)));

    uint t = QTime::currentTime().msec();
    std::srand(t);

    m_pLogMessageDlg = new LogMessageDlg;
//    m_pLogMessageDlg->setOutputFont(DisplayOptions::tableFont());

    m_pLogWt = new LogWt;

    //https://doc.qt.io/qt-6/qthread.html
    FileIO *worker = new FileIO;
    worker->moveToThread(&m_FileIOThread);
    connect(&m_FileIOThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &MainFrame::loadFile, worker, &FileIO::onLoadProject);
    connect(worker, &FileIO::displayMessage, m_pLogMessageDlg, &LogMessageDlg::onAppendPlainText, Qt::BlockingQueuedConnection);
    connect(worker, &FileIO::fileLoaded, this, &MainFrame::handleIOResults);
    m_FileIOThread.start();


    m_pScriptExecutor = nullptr;


    int oglversion = QSurfaceFormat::defaultFormat().majorVersion()*10+QSurfaceFormat::defaultFormat().minorVersion();

    if(oglversion<2)
    {
        QString strong = tr("flow5 requires OpenGL 3.3 or greater.") + EOLch;
        QString strange = tr("Your system provides by default OpenGL %1.%2")
                                    .arg(QSurfaceFormat::defaultFormat().majorVersion())
                                    .arg(QSurfaceFormat::defaultFormat().minorVersion());
        QMessageBox::warning(this, tr("Warning"), strong+strange);
    }

    setDefaultStaticFonts();
    GraphOptions::setDefaults(DisplayOptions::isDarkTheme());

    createDockWindows();
    createMainFrameActions();
    createMenus();
    createToolbars();
    createStatusBar();
    hideDockWindows();

    m_pXDirect->m_pFoilTable->setTableFontStruct(DisplayOptions::tableFontStruct());
    m_pXDirect->m_pFoilExplorer->setTreeFont(DisplayOptions::treeFontStruct().font());
    m_pXPlane->m_pPlaneExplorer->setTreeFont(DisplayOptions::treeFontStruct().font());
    m_pXSail->m_pBoatExplorer->setTreeFontStruct(DisplayOptions::treeFontStruct());

    setCentralWidget(m_pswCentralWidget);
    setActiveCentralWidget();

    setColorListFromFile();
    setPlainColorsFromFile();

    m_ImageFormat = xfl::PNG;

    m_GraphExportFilter = "Comma Separated Values (*.csv)";

    m_bSaveSettings = true;

    m_bManualCheck = false;

    loadSettings();

    updateRecentFileActions();

    QString strange;
    QScreen const *pScreen = QGuiApplication::primaryScreen();
    strange = QString::asprintf("Screen:\n   size=(%d,%d)\n   logical dots/inch=%.2f\n   pixel ratio = %.2f\n",
                                pScreen->size().width(), pScreen->size().height(),
                                pScreen->logicalDotsPerInch(),
                                pScreen->devicePixelRatio());
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    strange += QString::asprintf("Paint       device pixel ratio = %g\n\n", devicePixelRatio());
    strange += QString::asprintf("Application device pixel ratio = %g\n\n", qApp->devicePixelRatio());
#else
    strange += QString::asprintf("Application device pixel ratio = %d\n\n", devicePixelRatio());
#endif

    gl2dView::setImageSize(pScreen->size());

    displayMessage(strange + EOLch, false);

    strange = tr("Directories:") + "\n";
    strange += tr("   Last used :       ") + SaveOptions::lastDirName() + EOLch;
    strange += tr("   Foil .dat files:  ") + SaveOptions::datFoilDirName() + EOLch;
    strange += tr("   Polar .plr files: ") + SaveOptions::plrPolarDirName() + EOLch;
    strange += tr("   Plane .xml files: ") + SaveOptions::xmlPlaneDirName() + EOLch;
    strange += tr("   Polar .xml files: ") + SaveOptions::xmlWPolarDirName() + EOLch;
    strange += tr("   CAD files:        ") + SaveOptions::CADDirName() + EOLch;
    strange += tr("   STL files:        ") + SaveOptions::STLDirName() + EOLch;
    strange += tr("   Temporary files:  ") + SaveOptions::tempDirName() + EOLch;
    strange += tr("   File exports:     ") + SaveOptions::lastExportDirName() + EOLch;

    displayMessage(strange + EOLch, false);


    if(SaveOptions::bAutoSave())
    {
        m_SaveTimer.setInterval(SaveOptions::saveInterval()*60*1000);
        m_SaveTimer.start();
        connect(&m_SaveTimer, SIGNAL(timeout()), SLOT(onSaveTimer()));
    }

    s_bSaved     = true;
    s_iApp = xfl::NOAPP;

    setMenus();

    m_pFastGraphWt = new FastGraphWt();
    GraphOptions::resetGraphSettings(*m_pFastGraphWt->graph());

    connectSignals();

#ifdef OPENBLAS
    strange.clear();
    switch(openblas_get_parallel())
    {
//       https://github.com/OpenMathLib/OpenBLAS/wiki/Faq/a15b786986841d2e4e4e84e3f2ecff9c3b263b32
//      openblas_get_parallel() will return 0 for a single-threaded library, 1 if multithreading without OpenMP, 2 if built with USE_OPENMP=1
        case 0:  strange = "OpenBlas: single-threaded library";     break;
        case 1:  strange = "OpenBlas: multi-threading without OMP"; break;
        case 2:  strange = "OpenBlas: built with USE_OPENMP=1";     break;
        default: strange = "openblas_get_parallel() return error";
    }

        displayMessage(strange + EOLch + EOLch, false);
#elif defined INTEL_MKL
    strange.clear();
    int nt = mkl_get_max_threads();

    mkl_set_dynamic(xfl::isMKLDynamic());

    if (1 == mkl_get_dynamic())
    {
        strange += "MKL dynamic threading is enabled\n";
        strange += QString::asprintf("Intel MKL may use less than %i threads for a large problem", nt);
    }
    else
    {
        strange += "MKL dynamic threading is disabled\n";
        strange += QString::asprintf("Intel MKL should use %i threads for a large problem", nt);
    }
    displayMessage(strange + EOLch + EOLch, false);
#endif

    gmsh::initialize();
    gmsh::option::setNumber("General.Terminal", 0);
    gmsh::option::setNumber("Geometry.OCCParallel", 1.0);
    gmsh::option::setNumber("General.NumThreads", QThread::idealThreadCount());
//    gmsh::option::setNumber("Mesh.MaxNumThreads2D", QThread::idealThreadCount()); //Default value: 0; 0: use General.NumThreads


    std::string list;
    gmesh::listMainOptions(list);
    displayMessage(QString::fromStdString(list), false);

}


void MainFrame::testOpenGL()
{
    if(QSurfaceFormat::defaultFormat().majorVersion()<3)
    {
        displayMessage("The application requires OpenGL 3.1 or greater.\n", false);
    }
}


void MainFrame::connectSignals()
{
    connect(m_pXDirect, SIGNAL(curvesUpdated()),    SLOT(onCurvesUpdated()));
    connect(m_pXDirect, SIGNAL(projectModified()),  SLOT(onProjectModified()));

    connect(m_pXPlane,  SIGNAL(curvesUpdated()),    SLOT(onCurvesUpdated()));
    connect(m_pXPlane,  SIGNAL(projectModified()),  SLOT(onProjectModified()));

    connect(m_pXSail,   SIGNAL(curvesUpdated()),    SLOT(onCurvesUpdated()));
    connect(m_pXSail,   SIGNAL(projectModified()),  SLOT(onProjectModified()));

    m_pBLTiles->connectSignals();
    for(int igw=0; igw<m_pBLTiles->nGraphWts(); igw++)
    {
        connect(m_pBLTiles->graphWt(igw), SIGNAL(curveClicked(Curve*,int)),   m_pXDirect, SLOT(onCurveClicked(Curve*)));
        connect(m_pBLTiles->graphWt(igw), SIGNAL(curveDoubleClicked(Curve*)), m_pXDirect, SLOT(onCurveDoubleClicked(Curve*)));
    }

    m_pPolarTiles->connectSignals();
    for(int igw=0; igw<m_pPolarTiles->nGraphWts(); igw++)
    {
        connect(m_pPolarTiles->graphWt(igw), SIGNAL(curveClicked(Curve*,int)),   m_pXDirect, SLOT(onCurveClicked(Curve*)));
        connect(m_pPolarTiles->graphWt(igw), SIGNAL(curveDoubleClicked(Curve*)), m_pXDirect, SLOT(onCurveDoubleClicked(Curve*)));
    }

    m_pWPolarTiles->connectSignals();
    for(int igw=0; igw<m_pWPolarTiles->nGraphWts(); igw++)
    {
        connect(m_pWPolarTiles->graphWt(igw), SIGNAL(curveClicked(Curve*,int)),   m_pXPlane, SLOT(onCurveClicked(Curve*,int)));
        connect(m_pWPolarTiles->graphWt(igw), SIGNAL(curveDoubleClicked(Curve*)), m_pXPlane, SLOT(onCurveDoubleClicked(Curve*)));
    }

    m_pStabPolarTiles->connectSignals();
    for(int igw=0; igw<m_pStabPolarTiles->nGraphWts(); igw++)
    {
        connect(m_pStabPolarTiles->graphWt(igw), SIGNAL(curveClicked(Curve*,int)),   m_pXPlane, SLOT(onCurveClicked(Curve*,int)));
        connect(m_pStabPolarTiles->graphWt(igw), SIGNAL(curveDoubleClicked(Curve*)), m_pXPlane, SLOT(onCurveDoubleClicked(Curve*)));
    }

    m_pStabTimeTiles->connectSignals();
    for(int igw=0; igw<m_pStabTimeTiles->nGraphWts(); igw++)
    {
        connect(m_pStabTimeTiles->graphWt(igw), SIGNAL(curveClicked(Curve*,int)),   m_pXPlane, SLOT(onCurveClicked(Curve*,int)));
        connect(m_pStabTimeTiles->graphWt(igw), SIGNAL(curveDoubleClicked(Curve*)), m_pXPlane, SLOT(onCurveDoubleClicked(Curve*)));
    }

    m_pPOppTiles->connectSignals();
    for(int igw=0; igw<m_pPOppTiles->nGraphWts(); igw++)
    {
        connect(m_pPOppTiles->graphWt(igw), SIGNAL(curveClicked(Curve*,int)),   m_pXPlane, SLOT(onCurveClicked(Curve*,int)));
        connect(m_pPOppTiles->graphWt(igw), SIGNAL(curveDoubleClicked(Curve*)), m_pXPlane, SLOT(onCurveDoubleClicked(Curve*)));
    }

    m_pXSailTiles->connectSignals();

    for(int igw=0; igw<m_pXSailTiles->nGraphWts(); igw++)
    {
        connect(m_pXSailTiles->graphWt(igw), SIGNAL(curveClicked(Curve*,int)),   m_pXSail, SLOT(onCurveClicked(Curve*,int)));
        connect(m_pXSailTiles->graphWt(igw), SIGNAL(curveDoubleClicked(Curve*)), m_pXSail, SLOT(onCurveDoubleClicked(Curve*)));
    }
}


MainFrame::~MainFrame()
{
    gmsh::finalize();

    if(xfl::g_pTraceFile) xfl::g_pTraceFile->close();

    Objects2d::deleteObjects();

    while(m_GraphWidget.size()>0)
    {
        if(m_GraphWidget.back()->graph())
        {
            delete m_GraphWidget.back()->graph();
            m_GraphWidget.back()->setNullGraph();
        }
        //        m_GraphWidget.back()->close();
        delete m_GraphWidget.back();
        m_GraphWidget.pop_back();
    }

    delete m_pXPlane;
    delete m_pXDirect;

    for(int i=0; i<m_pRecentFileActs.size(); i++)
    {
        delete m_pRecentFileActs[i];
    }
    m_pRecentFileActs.clear();

    if(m_pFastGraphWt) delete m_pFastGraphWt;
    m_pFastGraphWt = nullptr;

    if(m_pLogWt) delete m_pLogWt;
    m_pLogWt = nullptr;


    if(m_pCpGraphCtrl)
    {
        m_pCpGraphCtrl->close();
        delete m_pCpGraphCtrl;
    }

    if(m_pCpViewWt)
    {
        m_pCpViewWt->close();
        delete m_pCpViewWt;
    }

    m_FileIOThread.quit();
    m_FileIOThread.wait();

    m_ScriptThread.quit();
    m_ScriptThread.wait();

    m_pLogMessageDlg->close();
    delete m_pLogMessageDlg;

}


void MainFrame::aboutFlow5()
{
    AboutFlow5 dlg(this);
    dlg.exec();
}


void MainFrame::aboutQt()
{
#ifndef QT_NO_MESSAGEBOX
    QMessageBox::aboutQt(
            #ifdef Q_OS_MAC
                this
            #else
                //            activeWindow()
                this
            #endif // Q_OS_MAC
                );
#endif // QT_NO_MESSAGEBOX
}


void MainFrame::addRecentFile(const QString &PathName)
{
    if(!PathName.endsWith(".fl5") && !PathName.endsWith(".xfl")) return;

    m_RecentFiles.removeAll(PathName);
    m_RecentFiles.prepend(PathName);
    while (m_RecentFiles.size() > MAXRECENTFILES)
        m_RecentFiles.pop_back();

    updateRecentFileActions();
}


void MainFrame::closeEvent(QCloseEvent *pEvent)
{
    if(m_pXPlane->isAnalysisRunning())
    {
        QMessageBox::warning(this, tr("Exit"), tr("Please wait for the analysis to finish before closing"));
        pEvent->ignore();
        return;
    }

    if(!s_bSaved)
    {
        int resp = QMessageBox::question(this, tr("Exit"), tr("Save the project before closing?"),
                                         QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                                         QMessageBox::Yes);
        if(resp == QMessageBox::Yes)
        {
            if(!saveProject(m_FilePath))
            {
                pEvent->ignore();
                return;
            }
            addRecentFile(m_FilePath);
        }
        else if (resp==QMessageBox::Cancel)
        {
            pEvent->ignore();
            return;
        }
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    deleteProject();

    hideDockWindows();

    //close all dockwindows and floating widgets
    statusBar()->showMessage("Closing app");
    m_VoidWidget.close();

    m_pBLTiles->close();
    m_pPolarTiles->close();
    m_pdwXDirect->close();
    m_pdwOpPoint->close();

    m_pDFoilWt->close();

    m_pdwFoilTree->close();
    m_pdwFoilTable->close();

    m_pXPlane->m_pgl3dXPlaneView->close();
    if(m_pXPlane->m_pXPlaneWt) m_pXPlane->m_pXPlaneWt->close();
    m_pdwAnalysis3d->close();
    m_pdwPlaneTree->close();
    m_pdwXPlaneResults3d->close();
    m_pdwStabTime->close();
    m_pdwCp3d->close();

    m_pWPolarTiles->close();
    m_pStabPolarTiles->close();
    m_pStabTimeTiles->close();
    m_pPOppTiles->close();
    m_pCpViewWt->close();

    m_pdwXSail->close();
    m_pdwXSail3dCtrls->close();
    m_pdwBoatTree->close();
    m_pXSailTiles->close();

    m_pdwGraphControls->close();

    m_pswCentralWidget->close();


    //clean the log files
    displayMessage("Cleaning log files\n\n", false);
    SaveOptions::cleanLogFiles();

    saveSettings();
    //    pEvent->accept();//continue closing
    QApplication::restoreOverrideCursor();

    QMainWindow::closeEvent(pEvent);
}


void MainFrame::createMainFrameActions()
{
    m_pNewProjectAct = new QAction(QIcon(":/icons/new.png"), tr("New project"), this);
    m_pNewProjectAct->setShortcut(QKeySequence::New);
    m_pNewProjectAct->setStatusTip(tr("Save and close the current project, create a new project"));
    connect(m_pNewProjectAct, SIGNAL(triggered()), SLOT(onNewProject()));

    m_pCloseProjectAct = new QAction(QIcon(":/icons/new.png"), tr("Close the project"), this);
    m_pCloseProjectAct->setStatusTip(tr("Save and close the current project"));
    connect(m_pCloseProjectAct, SIGNAL(triggered()), SLOT(onCloseProject()));

    m_pOpenAct = new QAction(QIcon(":/icons/open.png"), tr("Open a project"), this);
    m_pOpenAct->setShortcut(QKeySequence::Open);
    m_pOpenAct->setStatusTip(tr("Open an existing project file"));
    connect(m_pOpenAct, SIGNAL(triggered()), SLOT(onLoadProjectFile()));

    m_pSaveAct = new QAction(QIcon(":/icons/save.png"), tr("Save"), this);
    m_pSaveAct->setShortcut(QKeySequence::Save);
    m_pSaveAct->setStatusTip(tr("Save the project to disk"));
    connect(m_pSaveAct, SIGNAL(triggered()), SLOT(onSaveProject()));

    m_pLoadFoil = new QAction(QIcon(":/icons/OnLoadFoils.png"), tr("Load foil(s)"), this);
    m_pLoadFoil->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    m_pLoadFoil->setStatusTip(tr("Load foil(s) from .dat file"));
    connect(m_pLoadFoil, SIGNAL(triggered()), SLOT(onLoadFoilFile()));

    m_pLoadPlrFile = new QAction(tr("Load polar file(s)"), this);
    m_pLoadPlrFile->setStatusTip(tr("Load a .plr file"));
    connect(m_pLoadPlrFile, SIGNAL(triggered()), SLOT(onLoadPlrFile()));

    m_pInsertAct = new QAction(tr("Insert project"), this);
    m_pInsertAct->setStatusTip(tr("<p>Insert an existing project in the current project</p>"));
    m_pInsertAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    connect(m_pInsertAct, SIGNAL(triggered()), SLOT(onInsertProject()));

    m_pNoAppAct = new QAction(tr("Close all"), this);
    m_pNoAppAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_9));
    m_pNoAppAct->setStatusTip(tr("<p>Close all modules, but do not unload the active project</p>"));
    connect(m_pNoAppAct, SIGNAL(triggered()), SLOT(onSetNoApp()));

    m_pXDirectAct = new QAction(tr("Foil design"), this);
    m_pXDirectAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_5));
    m_pXDirectAct->setStatusTip(tr("<p>Open the Foil direct analysis module</p>"));
    connect(m_pXDirectAct, SIGNAL(triggered()), SLOT(onXDirect()));

    m_pXPlaneAct = new QAction(tr("Plane design"), this);
    m_pXPlaneAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_6));
    m_pXPlaneAct->setStatusTip(tr("<p>Open the Wing/plane design and analysis module</p>"));
    connect(m_pXPlaneAct, SIGNAL(triggered()), SLOT(onXPlane()));

    m_pXSailAct = new QAction(tr("Sail design"), this);
    m_pXSailAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_7));
    m_pXSailAct->setStatusTip(tr("Open the sail design and analysis module"));
    connect(m_pXSailAct, SIGNAL(triggered()), SLOT(onXSail()));

    m_pLoadLastProjectAct = new QAction(tr("Load last project"), this);
    m_pLoadLastProjectAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    m_pLoadLastProjectAct->setStatusTip(tr("Loads the last saved project"));
    connect(m_pLoadLastProjectAct, SIGNAL(triggered()), SLOT(onLoadLastProject()));

    m_pSaveProjectAsAct = new QAction(tr("Save project as"), this);
    m_pSaveProjectAsAct->setShortcut(QKeySequence::SaveAs);
    m_pSaveProjectAsAct->setStatusTip(tr("Save the current project under a new name"));
    connect(m_pSaveProjectAsAct, SIGNAL(triggered()), SLOT(onSaveProjectAs()));

    m_pPreferencesAct = new QAction(tr("Preferences"), this);
    m_pPreferencesAct->setStatusTip(tr("Set default preferences for this application"));
    m_pPreferencesAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_P));
    connect(m_pPreferencesAct, SIGNAL(triggered(bool)), SLOT(onPreferences()));

    m_pRestoreToolbarsAct     = new QAction(tr("Restore toolbars"), this);
    m_pRestoreToolbarsAct->setStatusTip(tr("Restores the toolbars to their original state"));
    connect(m_pRestoreToolbarsAct, SIGNAL(triggered()), SLOT(onRestoreToolbars()));

    m_pSaveViewToImageFileAct = new QAction(tr("Save view to image file"), this);
    m_pSaveViewToImageFileAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_I));
    m_pSaveViewToImageFileAct->setStatusTip(tr("Saves the current view to a file on disk"));
    connect(m_pSaveViewToImageFileAct, SIGNAL(triggered()), SLOT(onSaveViewToImageFile()));

    m_pExecScript     = new QAction(tr("Execute script"), this);
    m_pExecScript->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X));
    m_pExecScript->setStatusTip(tr("Executes a set of foil and plane analysis defined in an xml file"));
    connect(m_pExecScript, SIGNAL(triggered()), SLOT(onExecuteScript()));

    m_pResetSettingsAct = new QAction(tr("Restore default settings"), this);
    m_pResetSettingsAct->setStatusTip(tr("will revert to default settings at the next session"));
    connect(m_pResetSettingsAct, SIGNAL(triggered()), SLOT(onResetSettings()));


    for (int i=0; i<MAXRECENTFILES; ++i)
    {
        m_pRecentFileActs.push_back(new QAction(this));
        m_pRecentFileActs[i]->setVisible(false);
        connect(m_pRecentFileActs[i], SIGNAL(triggered()), SLOT(onLoadRecentFile()));
    }

    m_pExportCurGraphDataToFile = new QAction(tr("to file"), this);
    m_pExportCurGraphDataToFile->setStatusTip(tr("Export the current graph data to a text file"));
    connect(m_pExportCurGraphDataToFile, SIGNAL(triggered()), SLOT(onExportCurGraphDataToFile()));

    m_pExportGraphToSvgFile = new QAction(tr("to SVG file"), this);
    connect(m_pExportGraphToSvgFile, SIGNAL(triggered()), SLOT(onExportCurGraphToSVG()));

    m_pCopyCurGraphDataAct = new QAction(tr("to clipboard"), this);
    m_pCopyCurGraphDataAct->setStatusTip("<p>"+tr("Copies the curve data to the clipboard, for pasting in an external editor or a spreadsheet") + "</p>");
    connect(m_pCopyCurGraphDataAct, SIGNAL(triggered()), SLOT(onCopyCurGraphData()));

    m_pResetCurGraphScales = new QAction(QIcon(":/icons/OnResetGraphScale.png"), tr("Reset scales \tR"), this);
    m_pResetCurGraphScales->setStatusTip(tr("Restores the graph's x and y scales"));
    connect(m_pResetCurGraphScales, SIGNAL(triggered()), SLOT(onResetCurGraphScales()));

    m_pResetGraphSplitter = new QAction(tr("Reset splitter sizes"), this);
    m_pResetGraphSplitter->setStatusTip(tr("Resets the split splizes to their default values"));
    m_pResetGraphSplitter->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    connect(m_pResetGraphSplitter, SIGNAL(triggered()), SLOT(onResetGraphSplitter()));

    m_pShowGraphLegend = new QAction(tr("Show legend"), this);
    m_pShowGraphLegend->setCheckable(true);
    m_pShowGraphLegend->setStatusTip(tr("Toggle the in-graph legend display"));
    connect(m_pShowGraphLegend, SIGNAL(triggered()), SLOT(onShowGraphLegend()));

    m_pCurGraphDlgAct = new QAction(tr("Settings\tG"), this);
    connect(m_pCurGraphDlgAct, SIGNAL(triggered()), SLOT(onCurGraphSettings()));

    m_pOpenGraphInNewWindow = new QAction(tr("Open in new window"), this);
    m_pOpenGraphInNewWindow->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_W));
    connect(m_pOpenGraphInNewWindow, SIGNAL(triggered()), SLOT(onOpenGraphInNewWindow()));

    m_pFastGraphAct = new QAction(tr("Fast graph"), this);
    m_pFastGraphAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    connect(m_pFastGraphAct, SIGNAL(triggered()), SLOT(onFastGraph()));

    m_pShowLogWindow = new QAction(tr("Show log window"), this);
    m_pShowLogWindow->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_M));
    connect(m_pShowLogWindow, SIGNAL(triggered()), SLOT(onShowLogWindow()));

    m_pExitAct = new QAction(tr("Exit"), this);
    m_pExitAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    m_pExitAct->setStatusTip(tr("Exit the application"));
    connect(m_pExitAct, SIGNAL(triggered()), SLOT(close()));

    m_pOpenGLAct = new QAction(tr("OpenGL settings"), this);
    m_pOpenGLAct->setShortcut(QKeySequence(Qt::CTRL |  Qt::ALT | Qt::Key_O));
    connect(m_pOpenGLAct, SIGNAL(triggered()), SLOT(onOpenGLInfo()));

    m_pViewLogFile = new QAction(tr("View last log file"), this);
    m_pViewLogFile->setShortcut(Qt::Key_L);
    connect(m_pViewLogFile, SIGNAL(triggered()), SLOT(onLogFile()));

    m_pViewTraceFile = new QAction(tr("View trace file"), this);
    m_pViewTraceFile->setShortcut(QKeySequence(Qt::ALT | Qt::Key_T));
    connect(m_pViewTraceFile, SIGNAL(triggered()), SLOT(onTraceFile()));
}


void MainFrame::createDockWindows()
{
    setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::North);
    setTabPosition(Qt::RightDockWidgetArea, QTabWidget::North);

    createXDirectDockWindows();
    createXSailDockWindows();
    createXPlaneDockWindows();

    m_pCpViewWt    = new CpViewWt(this);
    m_pCpGraphCtrl = new CpGraphCtrls(this, m_pXPlane, m_pXSail);
    m_pdwCp3d = new QDockWidget(tr("3d Cp graph"), this);
    m_pdwCp3d->setWidget(m_pCpGraphCtrl);
    m_pdwCp3d->setObjectName(m_pdwCp3d->windowTitle());
    addDockWidget(Qt::RightDockWidgetArea, m_pdwCp3d);

    m_pswCentralWidget = new QStackedWidget;
    m_pswCentralWidget->setObjectName("CentralWt");
    m_pswCentralWidget->addWidget(&m_VoidWidget);
    m_pswCentralWidget->addWidget(m_pDFoilWt);
    m_pswCentralWidget->addWidget(m_pBLTiles);
    m_pswCentralWidget->addWidget(m_pPolarTiles);
    m_pswCentralWidget->addWidget(m_pXDirect->m_pOpPointWt);
    m_pswCentralWidget->addWidget(m_pXPlane->m_pgl3dXPlaneView);
    m_pswCentralWidget->addWidget(m_pWPolarTiles);
    m_pswCentralWidget->addWidget(m_pStabPolarTiles);
    m_pswCentralWidget->addWidget(m_pStabTimeTiles);
    m_pswCentralWidget->addWidget(m_pPOppTiles);
    m_pswCentralWidget->addWidget(m_pCpViewWt);
    m_pswCentralWidget->addWidget(m_pXSail->m_pgl3dXSailView);
    m_pswCentralWidget->addWidget(m_pXSailTiles);

    OpPointWt::setMainFrame(this);

    DFoilLegendWt::setXDirect(m_pXDirect);

    FoilExplorer::setMainFrame(this);
    PlaneExplorer::setMainFrame(this);
}


void MainFrame::createXSailDockWindows()
{
    m_pXSailTiles = new GraphTiles(MAXGRAPHS,this);    
    m_pXSail   = new XSail(this);

    for(int igw=0; igw<m_pXSailTiles->nGraphWts(); igw++)
    {
        connect(m_pXSailTiles->graphWt(igw), SIGNAL(graphWindow(Graph*)), SLOT(onOpenGraphInNewWindow(Graph*)));
    }
    m_pXSailTiles->setLegendWt(new XPlaneLegendWt);
    m_pXSailTiles->setGraphControls(new GraphTileCtrls);
    m_pXSailTiles->setMaxGraphs(5);
    m_pXSailTiles->setSingleGraphOrientation(Qt::Horizontal);
    m_pXSailTiles->setupMainLayout();
    m_pXSailTiles->setGraphList(m_pXSail->m_PlrGraph);
    m_pXSailTiles->saveActiveSet();


    m_pdwXSail = new QDockWidget(tr("Boat analysis"), this);
    m_pdwXSail->setObjectName(m_pdwXSail->windowTitle());
    addDockWidget(Qt::RightDockWidgetArea, m_pdwXSail);
    m_pdwXSail->setWidget(m_pXSail->m_pAnalysisCtrls);

    m_pdwXSail3dCtrls = new QDockWidget(tr("Sail 3d controls"), this);
    m_pdwXSail3dCtrls->setObjectName(m_pdwXSail3dCtrls->windowTitle());
    addDockWidget(Qt::RightDockWidgetArea, m_pdwXSail3dCtrls);
    m_pdwXSail3dCtrls->setWidget(m_pXSail->m_pgl3dControls);

    m_pdwBoatTree = new QDockWidget(tr("Boat object explorer"), this);
    m_pdwBoatTree->setObjectName(m_pdwBoatTree->windowTitle());
    addDockWidget(Qt::LeftDockWidgetArea, m_pdwBoatTree);
    m_pdwBoatTree->setWidget(m_pXSail->m_pBoatExplorer);
}


void MainFrame::createXDirectDockWindows()
{
    m_pXDirect = new XDirect(this);
    m_pBLTiles = new GraphTiles(MAXGRAPHS, this);
    for(int igw=0; igw<m_pBLTiles->nGraphWts(); igw++)
    {
        connect(m_pBLTiles->graphWt(igw), SIGNAL(graphWindow(Graph*)), SLOT(onOpenGraphInNewWindow(Graph*)));
    }
    m_pBLTiles->setLegendWt(new XDirectLegendWt);
    m_pBLTiles->setGraphControls(new BLGraphCtrls);
    m_pBLTiles->setMaxGraphs(5);
    m_pBLTiles->setSingleGraphOrientation(Qt::Horizontal);
    m_pBLTiles->setupMainLayout();
    m_pBLTiles->setGraphList(m_pXDirect->m_BLGraph);
    m_pBLTiles->saveActiveSet();

    m_pPolarTiles = new GraphTiles(MAXGRAPHS, this);
    for(int igw=0; igw<m_pPolarTiles->nGraphWts(); igw++)
    {
        connect(m_pPolarTiles->graphWt(igw), SIGNAL(graphWindow(Graph*)), SLOT(onOpenGraphInNewWindow(Graph*)));
    }
    m_pPolarTiles->setLegendWt(new XDirectLegendWt);
    m_pPolarTiles->setGraphControls(new GraphTileCtrls);
    m_pPolarTiles->setMaxGraphs(5);
    m_pPolarTiles->setSingleGraphOrientation(Qt::Horizontal);
    m_pPolarTiles->setupMainLayout();
    m_pPolarTiles->setGraphList(m_pXDirect->m_PlrGraph);
    m_pPolarTiles->saveActiveSet();

    m_pDFoilWt = new DFoilWt(this);
    m_pDFoilWt->setXDirect(m_pXDirect);
    m_pXDirect->m_pDFoilWt = m_pDFoilWt;

    m_pdwXDirect = new QDockWidget(tr("Analysis 2d"), this);
    m_pdwXDirect->setObjectName(m_pdwXDirect->windowTitle());
    m_pdwXDirect->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_pdwXDirect);
    m_pdwXDirect->setWidget(m_pXDirect->m_pAnalysisControls);
    m_pdwXDirect->setVisible(false);
    m_pdwXDirect->setFloating(false);
    m_pdwXDirect->move(960,60);

    m_pdwOpPoint = new QDockWidget(tr("Operating point"), this);
    m_pdwOpPoint->setObjectName(m_pdwOpPoint->windowTitle());
    m_pdwOpPoint->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_pdwOpPoint);
    m_pdwOpPoint->setWidget(m_pXDirect->m_pOpPointControls);
    m_pdwOpPoint->setVisible(true);
    m_pdwOpPoint->setFloating(false);
    m_pdwOpPoint->move(960,60);

    m_pdwFoilTree = new QDockWidget(tr("Foil object explorer"), this);
    m_pdwFoilTree->setObjectName(m_pdwFoilTree->windowTitle());
    m_pdwFoilTree->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_pdwFoilTree);
    m_pdwFoilTree->setVisible(false);
    m_pdwFoilTree->setFloating(false);
    m_pdwFoilTree->resize(100,600);
    m_pdwFoilTree->move(60,60);
    m_pdwFoilTree->setWidget(m_pXDirect->m_pFoilExplorer);

    m_pdwFoilTable = new QDockWidget(tr("Foil object table"), this);
    m_pdwFoilTable->setObjectName(m_pdwFoilTable->windowTitle());
    m_pdwFoilTable->setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, m_pdwFoilTable);
    m_pdwFoilTable->setVisible(false);
    m_pdwFoilTable->setFloating(false);
    m_pdwFoilTable->setWidget(m_pXDirect->m_pFoilTable);

    m_pXDirect->connectSignals();
}


void MainFrame::createXPlaneDockWindows()
{
    m_pXPlane  = new XPlane(this);
    m_pWPolarTiles = new GraphTiles(MAXGRAPHS, this);
    for(int igw=0; igw<m_pWPolarTiles->nGraphWts(); igw++)
    {
        connect(m_pWPolarTiles->graphWt(igw), SIGNAL(graphWindow(Graph*)), SLOT(onOpenGraphInNewWindow(Graph*)));
    }
    m_pWPolarTiles->setLegendWt(new XPlaneLegendWt);
    m_pWPolarTiles->setGraphControls(new GraphTileCtrls);
    m_pWPolarTiles->setMaxGraphs(5);
    m_pWPolarTiles->setSingleGraphOrientation(Qt::Horizontal);
    m_pWPolarTiles->setupMainLayout();
    m_pWPolarTiles->setGraphList(m_pXPlane->m_WPlrGraph);
    m_pWPolarTiles->saveActiveSet();

    m_pPOppTiles = new GraphTiles(MAXGRAPHS, this);
    for(int igw=0; igw<m_pPOppTiles->nGraphWts(); igw++)
    {
        connect(m_pPOppTiles->graphWt(igw), SIGNAL(graphWindow(Graph*)), SLOT(onOpenGraphInNewWindow(Graph*)));
    }
    m_pPOppTiles->setLegendWt(new XPlaneLegendWt);
    m_pXPlane->m_pPOppGraphCtrls = new POppGraphCtrls(m_pPOppTiles);
    m_pPOppTiles->setGraphControls(m_pXPlane->m_pPOppGraphCtrls);
    m_pPOppTiles->setMaxGraphs(5);
    m_pPOppTiles->setSingleGraphOrientation(Qt::Vertical);
    m_pPOppTiles->setupMainLayout();
    m_pPOppTiles->set1SplitterOrientation(Qt::Vertical);
    m_pPOppTiles->setGraphList(m_pXPlane->m_WingGraph);
    m_pPOppTiles->saveActiveSet();

    m_pStabTimeTiles = new GraphTiles(MAXGRAPHS, this);
    for(int igw=0; igw<m_pStabTimeTiles->nGraphWts(); igw++)
    {
        connect(m_pStabTimeTiles->graphWt(igw), SIGNAL(graphWindow(Graph*)), SLOT(onOpenGraphInNewWindow(Graph*)));
    }
    m_pStabTimeTiles->setLegendWt(new XPlaneLegendWt);
    m_pStabTimeTiles->setGraphControls(new GraphTileCtrls);
    m_pStabTimeTiles->setMaxGraphs(4);
    m_pStabTimeTiles->setSingleGraphOrientation(Qt::Horizontal);
    m_pStabTimeTiles->setupMainLayout();
    m_pStabTimeTiles->setGraphList(m_pXPlane->m_TimeGraph);
    m_pStabTimeTiles->saveActiveSet();

    m_pStabPolarTiles = new GraphTiles(MAXGRAPHS, this);
    for(int igw=0; igw<m_pStabPolarTiles->nGraphWts(); igw++)
    {
        connect(m_pStabPolarTiles->graphWt(igw), SIGNAL(graphWindow(Graph*)), SLOT(onOpenGraphInNewWindow(Graph*)));
    }
    m_pStabPolarTiles->setLegendWt(new XPlaneLegendWt);
    m_pStabPolarTiles->setGraphControls(new GraphTileCtrls);
    m_pStabPolarTiles->setMaxGraphs(2);
    m_pStabPolarTiles->setSingleGraphOrientation(Qt::Horizontal);
    m_pStabPolarTiles->setupMainLayout();   
    m_pStabPolarTiles->setGraphList(m_pXPlane->m_StabPlrGraph);
    m_pStabPolarTiles->saveActiveSet();

    m_pdwAnalysis3d = new QDockWidget(tr("Analysis 3d"), this);
    m_pdwAnalysis3d->setObjectName(m_pdwAnalysis3d->windowTitle());
    addDockWidget(Qt::RightDockWidgetArea, m_pdwAnalysis3d);
    m_pdwAnalysis3d->setWidget(m_pXPlane->m_pAnalysisControls);

    m_pdwPlaneTree = new QDockWidget(tr("Plane explorer"), this);
    m_pdwPlaneTree->setWidget(m_pXPlane->m_pPlaneExplorer);
    m_pdwPlaneTree->setObjectName(m_pdwPlaneTree->windowTitle());
    addDockWidget(Qt::LeftDockWidgetArea, m_pdwPlaneTree);

    m_pdwXPlaneResults3d = new QDockWidget(tr("Plane 3d view"), this);
    m_pdwXPlaneResults3d->setObjectName(m_pdwXPlaneResults3d->windowTitle());
    addDockWidget(Qt::RightDockWidgetArea, m_pdwXPlaneResults3d);
    m_pdwXPlaneResults3d->setWidget(m_pXPlane->m_pPOpp3dCtrls);

    m_pdwGraphControls = new QDockWidget(tr("Graphs"), this);
    m_pdwGraphControls->setObjectName(m_pdwGraphControls->windowTitle());
    addDockWidget(Qt::RightDockWidgetArea, m_pdwGraphControls);

    m_pdwStabTime = new QDockWidget(tr("Stability time controls"), this);
    m_pdwStabTime->setWidget(m_pXPlane->m_pStabTimeControls);
    m_pdwStabTime->setObjectName(m_pdwStabTime->windowTitle());
    addDockWidget(Qt::RightDockWidgetArea, m_pdwStabTime);

    m_pXPlane->connectSignals();
}


void MainFrame::createMenus()
{
    m_pFileMenu = menuBar()->addMenu(tr("&File"));
    {
        m_pFileMenu->addAction(m_pNewProjectAct);
        m_pFileMenu->addAction(m_pOpenAct);
        m_pFileMenu->addAction(m_pLoadLastProjectAct);
        m_pFileMenu->addAction(m_pInsertAct);
        m_pFileMenu->addAction(m_pCloseProjectAct);
        m_pFileMenu->addSeparator();
        m_pFileMenu->addAction(m_pLoadFoil);
        m_pFileMenu->addAction(m_pLoadPlrFile);
        m_pFileMenu->addSeparator();
        m_pFileMenu->addAction(m_pSaveAct);
        m_pFileMenu->addAction(m_pSaveProjectAsAct);
        m_pFileMenu->addSeparator();
        m_pFileMenu->addAction(m_pShowLogWindow);
        m_pFileMenu->addAction(m_pViewTraceFile);

        m_pSeparatorAct = m_pFileMenu->addSeparator();
        for (int i=0; i<m_pRecentFileActs.size(); i++)
        {
            m_pFileMenu->addAction(m_pRecentFileActs[i]);
        }

        m_pFileMenu->addSeparator();
        m_pFileMenu->addAction(m_pExitAct);
        m_pFileMenu->addSeparator();
        updateRecentFileActions();
    }

    m_pModuleMenu = menuBar()->addMenu(tr("&Module"));
    {
        m_pModuleMenu->addAction(m_pNoAppAct);
        m_pModuleMenu->addSeparator();
        m_pModuleMenu->addAction(m_pXDirectAct);
        m_pModuleMenu->addAction(m_pXPlaneAct);
        m_pModuleMenu->addAction(m_pXSailAct);
        m_pModuleMenu->addSeparator();
        m_pModuleMenu->addAction(m_pExecScript);
        m_pModuleMenu->addSeparator();
        m_pModuleMenu->addAction(m_pFastGraphAct);
    }

    m_pOptionsMenu = menuBar()->addMenu(tr("&Options"));
    {
        m_pOptionsMenu->addSeparator();
        m_pOptionsMenu->addAction(m_pPreferencesAct);
        m_pOptionsMenu->addAction(m_pOpenGLAct);
        m_pOptionsMenu->addSeparator();
        m_pOptionsMenu->addAction(m_pRestoreToolbarsAct);
        m_pOptionsMenu->addSeparator();
        m_pOptionsMenu->addAction(m_pResetSettingsAct);
        m_pOptionsMenu->addSeparator();
    }

    m_pHelpMenu = menuBar()->addMenu(tr("&?"));
    {
        QAction *pAboutf5Act = new QAction(tr("About flow5"), this);
        pAboutf5Act->setStatusTip(tr("More information about flow5"));
        connect(pAboutf5Act, SIGNAL(triggered()), SLOT(aboutFlow5()));

        QAction *pAboutQtAct = new QAction(tr("About Qt"), this);
        connect(pAboutQtAct, SIGNAL(triggered()), SLOT(aboutQt()));

        QAction *pCreditsAct = new QAction(tr("Credits"), this);
        connect(pCreditsAct, SIGNAL(triggered(bool)), SLOT(onCredits()));

        QAction *pOnlineDoc = new QAction(tr("Online doc."), this);
        connect(pOnlineDoc, SIGNAL(triggered()), SLOT(onOnlineDoc()));

        QAction *pReleaseNotes = new QAction(tr("Release notes"), this);
        connect(pReleaseNotes, SIGNAL(triggered()), SLOT(onReleaseNotes()));

        m_pHelpMenu->addAction(pOnlineDoc);
        m_pHelpMenu->addAction(pReleaseNotes);
        m_pHelpMenu->addSeparator();
        m_pHelpMenu->addAction(pCreditsAct);
        m_pHelpMenu->addAction(pAboutQtAct);
        m_pHelpMenu->addSeparator();
        m_pHelpMenu->addAction(pAboutf5Act);
    }

    //Create module specific menus
    m_pXDirect->m_pMenus->createMenus();
    m_pBLTiles->setContextMenu(m_pXDirect->m_pMenus->m_pBLCtxMenu);
    m_pPolarTiles->setContextMenu(m_pXDirect->m_pMenus->m_pOperPolarCtxMenu);

    m_pXPlane->m_pMenus->createMenus();
    m_pPOppTiles->setContextMenu(m_pXPlane->m_pMenus->m_pWOppCtxMenu);
    m_pWPolarTiles->setContextMenu(m_pXPlane->m_pMenus->m_pWPlrCtxMenu);
    m_pStabPolarTiles->setContextMenu(m_pXPlane->m_pMenus->m_pWPlrCtxMenu);
    m_pStabTimeTiles->setContextMenu(m_pXPlane->m_pMenus->m_pWTimeCtxMenu);

    m_pXSail->m_pMenus->createMenus();
    m_pXSailTiles->setContextMenu(m_pXSail->m_pMenus->m_pBtPlrCtxMenu);
}


void MainFrame::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
    QFont fnt;
    QFontMetrics fm(fnt);
    m_plabProjectName = new QLabel(QString());
    m_plabProjectName->setMinimumWidth(fm.averageCharWidth()*30);
    statusBar()->addPermanentWidget(m_plabProjectName);
}


void MainFrame::createToolbars()
{
    m_ptbMain = addToolBar(tr("MainToolBar"));
    {
        m_ptbMain->setObjectName("MainToolBar");
        m_ptbMain->addAction(m_pNewProjectAct);
        m_ptbMain->addAction(m_pOpenAct);
        m_ptbMain->addAction(m_pSaveAct);
    }
    createXDirectToolbars();
    createXPlaneToolbar();
    createXSailToolbars();

    int minsize = DisplayOptions::iconSize();
    if(m_ptbMain->iconSize().height()<minsize)    m_ptbMain->setIconSize(   QSize(minsize, minsize));
    if(m_ptbXPlane->iconSize().height()<minsize)  m_ptbXPlane->setIconSize( QSize(minsize, minsize));
    if(m_ptbXDirect->iconSize().height()<minsize) m_ptbXDirect->setIconSize(QSize(minsize, minsize));
    if(m_ptbDFoil->iconSize().height()<minsize)   m_ptbDFoil->setIconSize(  QSize(minsize, minsize));
    if(m_ptbXSail->iconSize().height()<minsize)   m_ptbXSail->setIconSize(  QSize(minsize, minsize));
}


void MainFrame::createXSailToolbars()
{
    m_ptbXSail = addToolBar(tr("XSailToolBar"));
    {
        m_ptbXSail->setObjectName("XSailToolBar");
        m_ptbXSail->addAction(m_pXSail->m_pActions->m_p3dView);
        m_ptbXSail->addAction(m_pXSail->m_pActions->m_pPolarView);
    }
}


void MainFrame::createXDirectToolbars()
{
    m_ptbXDirect = addToolBar(tr("XDirect"));
    {
        m_ptbXDirect->setObjectName("XDirect");
        m_ptbXDirect->addAction(m_pLoadFoil);
        m_ptbXDirect->addSeparator();
        m_ptbXDirect->addAction(m_pXDirect->m_pActions->m_pDesignAct);
        m_ptbXDirect->addAction(m_pXDirect->m_pActions->m_pBLAct);
        m_ptbXDirect->addAction(m_pXDirect->m_pActions->m_pOpPointsAct);
        m_ptbXDirect->addAction(m_pXDirect->m_pActions->m_pPolarsAct);
    }

    m_ptbDFoil = addToolBar(tr("Foil"));
    {
        m_ptbDFoil->setObjectName("Foil");
        m_ptbDFoil->addAction(m_pDFoilWt->m_pZoomInAct);
        m_ptbDFoil->addAction(m_pDFoilWt->m_pZoomLessAct);
        m_ptbDFoil->addAction(m_pDFoilWt->m_pResetXYScaleAct);
        m_ptbDFoil->addAction(m_pDFoilWt->m_pResetXScaleAct);
        m_ptbDFoil->addAction(m_pDFoilWt->m_pZoomYAct);
        m_ptbDFoil->addSeparator();
        m_ptbDFoil->addAction(m_pDFoilWt->m_pGridAct);

        m_ptbDFoil->addSeparator();
        m_ptbDFoil->addAction(m_pXDirect->m_pActions->m_pFillFoil);
        m_ptbDFoil->addAction(m_pXDirect->m_pActions->m_pShowLEPosition);
        m_ptbDFoil->addAction(m_pXDirect->m_pActions->m_pShowTEHinge);
    }
}


void MainFrame::createXPlaneToolbar()
{
    XPlaneActions *pActions = m_pXPlane->m_pActions;
    m_ptbXPlane = addToolBar(tr("PlaneToolBar"));
    {
        m_ptbXPlane->setObjectName("PlaneToolBar");
        m_ptbXPlane->addAction(pActions->m_pWOppAct);
        m_ptbXPlane->addAction(pActions->m_pWPolarAct);
        m_ptbXPlane->addAction(pActions->m_pW3dAct);
        m_ptbXPlane->addAction(pActions->m_pCpViewAct);
        m_ptbXPlane->addAction(pActions->m_pRootLocusAct);
        m_ptbXPlane->addAction(pActions->m_pStabTimeAct);
    }
}


void MainFrame::deleteProject()
{
    displayMessage(tr("Deleting current project\n"), false);

    // clear everything
    // make sure that the pointers are set to null before deleting the objects
    // to avoid incorrect memory reads
    displayMessage(tr("   Deleting 2d objects\n"), false);
    XDirect::setCurFoil(nullptr);
    XDirect::setCurPolar(nullptr);
    XDirect::setCurOpp(nullptr);
    Objects2d::deleteObjects();

    m_pXDirect->setFoil(nullptr);
    m_pXDirect->updateFoilExplorers();
    m_pXDirect->resetCurves();
    m_pDFoilWt->resetLegend();

    displayMessage("   Deleting plane objects\n", false);
    m_pXPlane->m_pCurPlane  = nullptr;
    m_pXPlane->m_pCurPOpp   = nullptr;
    m_pXPlane->m_pCurPlPolar = nullptr;
    Objects3d::deleteObjects();
    m_pXPlane->resetCurves();
    m_pXPlane->setPlane(nullptr);
    m_pXPlane->setControls();
    m_pXPlane->m_pPlaneExplorer->setObjectProperties();

    displayMessage("   Deleting sail objects\n", false);
    m_pXSail->m_pCurBoat    = nullptr;
    m_pXSail->m_pCurBtPolar = nullptr;
    m_pXSail->m_pCurBtOpp   = nullptr;
    SailObjects::deleteObjects();
    m_pXSail->setBoat(nullptr);
    m_pXSail->resetCurves();
    m_pXSail->setControls();
    m_pXSail->updateObjectView();


    setProjectName("");
    setSavedState(true);

    updateView();

    displayMessage("   Done deleting objects\n\n", false);
}


void MainFrame::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
    bool bAlt = (pEvent->modifiers() & Qt::AltModifier);
    bool bShift = (pEvent->modifiers() & Qt::ShiftModifier);

    if(s_iApp == xfl::XDIRECT && m_pXDirect)
    {
        m_pXDirect->keyPressEvent(pEvent);
    }
    else if(s_iApp == xfl::XPLANE && m_pXPlane)
    {
        m_pXPlane->keyPressEvent(pEvent);
    }
    else if(s_iApp == xfl::XSAIL && m_pXSail)
    {
        m_pXSail->keyPressEvent(pEvent);
    }
    else
    {
        switch (pEvent->key())
        {
            case Qt::Key_X:
            {
                if(bAlt)
                {
                    onTestRun();
                    pEvent->accept();
                }
                break;
            }
            case Qt::Key_D:
            {
#ifdef QT_DEBUG
                if(bCtrl)
                {
                    resize(2560, 1440);
                    setGeometry(150,0,2560, 1440);
                    qDebug()<<"App device pixel ratio"<<qApp->devicePixelRatio();
                    qDebug()<<"Device pixel ratio"<<devicePixelRatio()<<devicePixelRatioF();
                    qDebug()<<"Screen pixel ratio"<<QGuiApplication::primaryScreen()->devicePixelRatio();
                    qDebug()<<"size="<<size();
                    qDebug()<<"geometry="<<geometry();
                    qDebug()<<"framegeom"<<frameGeometry();
                    pEvent->accept();

                }
#endif
                break;
            }
            case Qt::Key_K:
            {
                if(bCtrl)
                {
                    StreamTestDlg dlg(this);
                    dlg.exec();
                }
                break;
            }
            case Qt::Key_L:
            {
                onLogFile();
                break;
            }
            case Qt::Key_P:
            {
                if(bCtrl)
                {
                    Panel3TestDlg dlg;
                    dlg.exec();
                }
                else if(bAlt)
                {
                    VortonTestDlg dlg(nullptr);
                    dlg.exec();
                }
                else if(bShift)
                {
                    gl3dSingularity *pTestView = new gl3dSingularity;
                    pTestView->setAttribute(Qt::WA_DeleteOnClose);
                    pTestView->show();
                    pTestView->activateWindow();
                }
                break;
            }
            case Qt::Key_T:
            {
                if(bCtrl && bAlt)
                {
                    onTraceFile();
                    pEvent->accept();
                }
                break;
            }
            case Qt::Key_1:
            {
                if(bCtrl)
                {
                    onDFoil();
                    pEvent->accept();
                }
                break;
            }
            case Qt::Key_5:
            {
                if(bCtrl)
                {
                    onXDirect();
                    pEvent->accept();
                }
                break;
            }
            case Qt::Key_6:
            {
                if(bCtrl)
                {
                    onXPlane();
                    pEvent->accept();
                }
                break;
            }
            case Qt::Key_9:
            {
                if(bCtrl)
                {
                    onOpenGLInfo();
                    pEvent->accept();
                }
                break;
            }
            case (Qt::Key_F2):
            {
                gl3dAxesView *pTestView =  new gl3dAxesView();
                pTestView->setAttribute(Qt::WA_DeleteOnClose);
                pTestView->show();
                pTestView->activateWindow();
                break;
            }
            default:
                pEvent->ignore();
                return;
        }
    }
}


void MainFrame::keyReleaseEvent(QKeyEvent *pEvent)
{
    if(s_iApp == xfl::XPLANE && m_pXPlane)
    {
        if (pEvent->key()==Qt::Key_Control)
        {
            updateView();
        }
        else m_pXPlane->keyReleaseEvent(pEvent);
    }

    pEvent->accept();
}


xfl::enumApp MainFrame::loadProjectFile(QString const &PathName)
{
    QString pathname = PathName;

    QFileInfo fi(pathname);
    if(!fi.exists())
    {
        displayMessage("File " + PathName + " not found\n\n", true);
        return xfl::NOAPP;
    }
    pathname.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator
    s_XflProjectPath = fi.canonicalPath();

    if(pathname.endsWith("fl5"))
        m_FilePath = pathname;

    SaveOptions::setLastDirName(fi.absolutePath());

    emit loadFile(PathName);

    return xfl::NOAPP;
}


void MainFrame::handleIOResults(bool bError)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    xfl::enumApp iApp(xfl::NOAPP);
    if     (SailObjects::nBoats()) iApp = xfl::XSAIL;
    else if(Objects3d::nPlanes())  iApp = xfl::XPLANE;
    else                           iApp = xfl::XDIRECT;

    switch (iApp)
    {
        case xfl::XDIRECT:
        {
            m_pXDirect->setFoil();

            setSavedState(false);

            if(iApp==xfl::XDIRECT)
            {
                m_pXDirect->setControls();
            }

            onXDirect();
            break;
        }
        case xfl::XPLANE:
        {
//            for(Plane *pPlane : Objects3d::planes())
//                Objects3d::makePlaneTriangulation(pPlane);

            onXPlane();
            break;
        }
        case xfl::XSAIL:
        {
//            for(Boat *pBoat : SailObjects::boats())
//                Objects3d::makeBoatTriangulation(pBoat);
            onXSail();
            break;
        }
        case xfl::NOAPP: break;
    }

    if(!bError)
    {
        m_pLogMessageDlg->hide();
        addRecentFile(m_FilePath);
        setSavedState(true);
        setProjectName(m_FilePath);
    }
    QApplication::restoreOverrideCursor();
}


void MainFrame::displayMessage(QString const &msg, bool bShowWindow, bool bStatusBar, int duration)
{
    if(bStatusBar) statusBar()->showMessage(msg, duration);
    if(bShowWindow) m_pLogMessageDlg->show();
    m_pLogMessageDlg->onAppendPlainText(msg);
    xfl::trace(msg);
}


void MainFrame::displayHtmlMessage(QString const &msg, bool bShowWindow)
{
    if(bShowWindow) m_pLogMessageDlg->show();
    m_pLogMessageDlg->onAppendHtmlText(msg);
    xfl::trace(msg);
}


void MainFrame::showStabTimeCtrls(bool bVisible)
{
    bool bfloat = m_pdwStabTime->isFloating();
    if(s_iApp==xfl::XPLANE)
    {
        if(!bfloat)
            tabifyDockWidget(m_pdwPlaneTree, m_pdwStabTime);
        m_pdwStabTime->setVisible(bVisible);
        m_pdwStabTime->raise();
    }
}


void MainFrame::onLoadFoilFile()
{
    QStringList const PathNames = QFileDialog::getOpenFileNames(this, "Open file",
                                                          SaveOptions::datFoilDirName(),
                                                          "Foil file (*.dat)");
    if(!PathNames.size()) return;

    for (QString const& PathName : PathNames)
    {
        if (PathName.endsWith(".dat", Qt::CaseInsensitive))
        {
            QFileInfo fi(PathName);
            if (!fi.exists() || !fi.isReadable())
            {
                QString strange = "<p><font color=red>Error:</font>   ...Could not open the file "+ PathName +"<br></p>";
                displayHtmlMessage(strange, true);
                continue;
            }

            int iLineError(0);

            Foil *pFoil = new Foil();
            bool bOK = foil::readFoilFile(PathName.toStdString(), pFoil, iLineError);

            if(bOK)
            {
                pFoil->setLineWidth(Curve::defaultLineWidth());
                Objects2d::insertThisFoil(pFoil);

                displayHtmlMessage("<p><font color=green>Successfully loaded</font> " + QString::fromStdString(pFoil->name())+"<br></p>", false);
            }
            else
            {
                delete pFoil;
                QString strange = "<p><font color=red>Error reading the file: </font>"+ PathName + QString::asprintf(" at line %d<br></p>", iLineError);

                displayHtmlMessage(strange, true);
            }
        }
    }

    displayMessage(EOLch + EOLch, false, false);

    if(s_iApp==xfl::XDIRECT)
    {
        m_pXDirect->updateFoilExplorers();
        m_pXDirect->m_pFoilExplorer->selectFoil(XDirect::curFoil());
        m_pXDirect->setControls();
    }

    setSavedState(false);
}


void MainFrame::onLoadPlrFile()
{
    QStringList PathNames;
    QString PathName;

    PathNames = QFileDialog::getOpenFileNames(this, "Open file",
                                              SaveOptions::plrPolarDirName(),
                                              "Airfoil polar file (*.plr)");
    if(!PathNames.size()) return;

    for (int i=0; i<PathNames.size(); i++)
    {
        PathName = PathNames.at(i);
        if (PathName.endsWith(".plr", Qt::CaseInsensitive))
        {

            QFile plrFile;
            plrFile.setFileName(PathName);
            if (!plrFile.open(QIODevice::ReadOnly))
            {
                QString strange = "   ...Could not open the file "+ PathName + EOLch;
                displayMessage(strange, true);
                continue;
            }

            std::vector<Foil*> foilList;
            std::vector<Polar*> polarList;
            io::readPolarFile(plrFile, foilList, polarList);
            plrFile.close();

            for(uint i=0; i<foilList.size(); i++)
            {
                Foil *pFoil=foilList.at(i);
                if(pFoil)
                {
                    pFoil->setVisible(true); // clean up former mess
                    Objects2d::insertThisFoil(pFoil);
                }
            }

            for(uint i=0; i<polarList.size(); i++)
            {
                Polar*pPolar=polarList.at(i);
                Objects2d::insertPolar(pPolar);
            }

            XDirect::setCurPolar(nullptr);
            XDirect::setCurOpp(nullptr);

            displayMessage("Successfully loaded polar file\n", false);

        }
    }

    if(s_iApp==xfl::XDIRECT)
    {
        m_pXDirect->updateFoilExplorers();
        m_pXDirect->m_pFoilExplorer->selectFoil(XDirect::curFoil());
        m_pXDirect->setControls();
    }

    setSavedState(false);
}


void MainFrame::onLoadProjectFile()
{
    QString PathName;

    if(xfl::dontUseNativeMacDlg())
    {
        QFileDialog fd;
        fd.setOption(QFileDialog::DontUseNativeDialog);
        PathName = fd.getOpenFileName(this, "Open File",
                                      SaveOptions::lastDirName(),
                                      "flow5 file (*.xfl *.fl5)");
    }
    else
    {
        PathName = QFileDialog::getOpenFileName(this, "Open file",
                                                SaveOptions::lastDirName(),
                                                "flow5 file (*.xfl *.fl5)");
    }

    if(PathName.isEmpty()) return;

    if(!onCloseProject()) return;

    onShowLogWindow(true);

    loadProjectFile(PathName);
}


void MainFrame::onLoadLastProject()
{
    if(!m_RecentFiles.size()) return;

    if(!onCloseProject()) return;


    loadRecentProject();
}


void MainFrame::onLoadRecentFile()
{
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    if(!onCloseProject()) return;

    QString pathname = pSenderAction->data().toString();

    loadRecentProject(pathname);
}


void MainFrame::loadRecentProject(QString recentfilename)
{
    onShowLogWindow(true);

    if(recentfilename.isEmpty())
    {
        if(m_RecentFiles.isEmpty()) return;
        recentfilename = m_RecentFiles.front();
    }

    if(recentfilename.endsWith(".xfl", Qt::CaseInsensitive) || recentfilename.endsWith(".fl5", Qt::CaseInsensitive))
    {
        xfl::enumApp iApp = loadProjectFile(recentfilename);

        switch (iApp)
        {
            case xfl::NOAPP:
            {
                // something is wrong with this file - remove it from the list
                break;
            }
            case xfl::XDIRECT:
            {
                onXDirect();
                onShowLogWindow(false);
                break;
            }
            case xfl::XPLANE:
            {
                onXPlane();

                onShowLogWindow(false);
                break;
            }
            case xfl::XSAIL:
            {
                onXSail();
                onShowLogWindow(false);
                break;
            }
        }
    }
}


void MainFrame::onReleaseNotes()
{
    QDesktopServices::openUrl(QUrl("https://flow5.tech/docs/releasenotes.html"));
}


void MainFrame::onOnlineDoc()
{
    QDesktopServices::openUrl(QUrl("https://flow5.tech/docs/flow5_doc/flow5_doc.html"));
}


void MainFrame::onLogFile()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(SaveOptions::lastLogFileName()));
}


void MainFrame::onTraceFile()
{
    if(!xfl::g_pTraceFile) return;
    QFileInfo fi(*xfl::g_pTraceFile);
    QDesktopServices::openUrl(QUrl::fromLocalFile(fi.filePath()));
}


bool MainFrame::onCloseProject()
{
    if(!s_bSaved)
    {
        int resp = QMessageBox::question(this, tr("Question"), tr("Save the current project?"),
                                         QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

        if (QMessageBox::Cancel == resp)
        {
            return false;
        }
        else if (QMessageBox::Yes == resp)
        {
            if(saveProject(m_FilePath))
            {
                deleteProject();
                displayMessage(tr("The project ") + m_ProjectName + tr(" has been saved\n\n"), false);
            }
            else return false; //save failed, don't close
        }
        else if (QMessageBox::No == resp)
        {
            deleteProject();
        }
    }
    else
    {
        deleteProject();
    }
    updateView();

    return true;
}


void MainFrame::onNewProject()
{
    if(onCloseProject())
        onSetNoApp();
}


void MainFrame::onOpenGLInfo()
{
    if(m_pXPlane && m_pXPlane->m_pgl3dXPlaneView)
    {
        m_pXPlane->m_pgl3dXPlaneView->setZAnimation(false);
        m_pXPlane->m_pPOpp3dCtrls->setControls();
    }
    if(m_pXSail && m_pXSail->m_pgl3dXSailView)
    {
        m_pXSail->m_pgl3dXSailView->setZAnimation(false);
        m_pXSail->m_pgl3dControls->setControls();
    }
    OpenGlDlg w(this);
    w.initDialog();
    w.exec();
}


void MainFrame::onResetSettings()
{
    int resp = QMessageBox::question(this, tr("Default Settings"), tr("Are you sure you want to reset the default settings?"),
                                     QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
    if(resp == QMessageBox::Yes)
    {
        QMessageBox::warning(this, tr("Default Settings"), tr("The settings will be reset at the next session"));
#if defined Q_OS_MAC
        QSettings settings(QSettings::IniFormat,QSettings::UserScope,"flow5", "flow5");
#elif defined Q_OS_LINUX
        QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"flow5", "flow5");
#else
        QSettings settings(QSettings::IniFormat,QSettings::UserScope,"flow5");
#endif
        settings.clear();
        SaveOptions::resetDefaultDirNames();
        // do not save on exit
        m_bSaveSettings = false;
    }
}


void MainFrame::onRestoreToolbars()
{
    hideDockWindows();

    bool bFloat=false;
    m_pdwGraphControls->setFloating(bFloat);
    m_pdwGraphControls->restoreGeometry(QByteArray());

    m_pdwAnalysis3d->setFloating(bFloat);
    m_pdwAnalysis3d->restoreGeometry(QByteArray());

    m_pdwXSail->setFloating(bFloat);
    m_pdwXSail->restoreGeometry(QByteArray());

    m_pdwXDirect->setFloating(bFloat);
    m_pdwXDirect->restoreGeometry(QByteArray());

    m_pdwOpPoint->setFloating(bFloat);
    m_pdwOpPoint->restoreGeometry(QByteArray());

    m_pdwXPlaneResults3d->setFloating(bFloat);
    m_pdwXPlaneResults3d->restoreGeometry(QByteArray());

    m_pdwStabTime->setFloating(bFloat);
    m_pdwStabTime->restoreGeometry(QByteArray());

    m_pdwXSail3dCtrls->setFloating(bFloat);
    m_pdwXSail3dCtrls->restoreGeometry(QByteArray());


    m_ptbMain->setVisible(true);
    switch (s_iApp)
    {
        case xfl::XDIRECT:
        {
            m_ptbDFoil->setVisible(m_pXDirect->isDesignView());
            m_ptbXDirect->show();
            m_pdwXDirect->show();
            if(m_pXDirect->isDesignView()) m_pdwFoilTable->show();
            else                           m_pdwFoilTree->show();
            m_pdwGraphControls->show();
            break;
        }
        case xfl::XPLANE:
        {
            m_pdwPlaneTree->show();
            m_pdwAnalysis3d->show();
            m_ptbXPlane->show();
            m_pXPlane->setControls(true);
            break;
        }
        case xfl::XSAIL:
        {
            m_pdwBoatTree->show();
            m_pdwXSail->show();
            m_ptbXSail->show();
            m_pXSail->setControls();
            break;
        }
        default:
            break;
    }
}


void MainFrame::onSaveTimer()
{
    if (!m_ProjectName.length()) return;
    if(m_pXPlane->isAnalysisRunning()  ||  m_pXSail->isAnalysisRunning())
    {
        displayMessage("Skipped auto-save because an analysis is running.\n\n", false);
        return;
    }

    if(saveProject(m_FilePath))
    {
        displayMessage("The project " + m_ProjectName + " has been saved\n\n", false);
    }
}


void MainFrame::onSaveProject()
{
    if (!m_ProjectName.length())
    {
        onSaveProjectAs();
        return;
    }

    if(saveProject(m_FilePath))
    {
        displayMessage("The project " + m_ProjectName + " has been saved\n\n", false);
    }
    m_pXPlane->updateView();
}


void MainFrame::onSaveProjectAs()
{
    QString Filter = "flow5 project file (*.fl5)";

    QString pathname = QFileDialog::getSaveFileName(this, "Save the project file",
                                                    m_FilePath,
                                                    Filter);

    if(!pathname.length()) return; // user changed his mind

    int pos = pathname.indexOf(".fl5", Qt::CaseInsensitive);
    if(pos<0) pathname += ".fl5";

    if(saveProject(pathname))
    {
        setProjectName(m_FilePath);
        displayMessage("The project " + m_ProjectName + " has been saved\n\n", false);
    }
    else
    {
        displayMessage("Error saving project " + m_ProjectName + "\n\n", true);
    }
}


void MainFrame::onInsertProject()
{
    QFileDialog fd;
    QString pathname = fd.getOpenFileName(this, "Open File",
                                    SaveOptions::lastDirName(),
                                    "flow5 file (*.xfl *.fl5)");

     if(!pathname.length()) return;


    QFileInfo fi(pathname);
    SaveOptions::setLastDirName(fi.path());

    QFile XFile(pathname);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        onShowLogWindow(true);
        QString strange = "Could not open the file "+ pathname + "\n\n";
        displayMessage(strange, true);
        return;
    }

    QString strange = "Importing the file "+ pathname + EOLch;
    displayMessage(strange, false);


    QString end = pathname.right(4).toLower();
    FileIO loader;

    if(end==".xfl")
    {

        QDataStream ar(&XFile);
        bool bIsStoring = false;
        PlanePolar wplr; // dummy
        bool  bRead = loader.serializeProjectXfl(ar, bIsStoring, &wplr);
        if(!bRead)
        {
            onShowLogWindow(true);
            QString strange = "Error reading the file: "+pathname+"\n\n";
            displayMessage(strange, true);
        }
    }
    else if(end==".fl5")
    {
        QDataStream ar(&XFile);
        bool bLoadPlanes = loader.serializeProjectFl5(ar, false);
        if(!bLoadPlanes)
        {
            onShowLogWindow(true);
            displayMessage("Error reading the file: "+pathname+"\n\n", false);
        }
        else
        {
            displayHtmlMessage("<p><font color=green>The file "+pathname+" has been read successfully<br><br></p>", false);
        }
    }
    XFile.close();
    setSavedState(false);

    if(s_iApp == xfl::XPLANE)
    {
        m_pXPlane->updateTreeView();
        m_pXPlane->setPlane();
        m_pXPlane->m_pPlaneExplorer->selectPlane(m_pXPlane->curPlane());
        m_pXPlane->resetCurves();
    }
    else if(s_iApp == xfl::XDIRECT)
    {
        m_pXDirect->createCurves();
        m_pXDirect->updateFoilExplorers();
    }
    else if(s_iApp == xfl::XSAIL)
    {
        m_pXSail->updateObjectView();
        m_pXSail->setBoat();
        m_pXSail->m_pBoatExplorer->selectBoat(m_pXSail->curBoat());
        m_pXPlane->resetCurves();
    }
    updateView();
    QApplication::restoreOverrideCursor();
}


void MainFrame::onSaveViewToImageFile()
{
    QString FileName, Filter;
    QStringList filters;
    filters << "Portable Network Graphics (*.png)"<<"JPEG (*.jpg)"<<"Windows Bitmap (*.bmp)";

    switch(m_ImageFormat)
    {
        case xfl::PNG:
        {
            Filter = filters.at(0);
            break;
        }
        case xfl::JPEG :
        {
            Filter = filters.at(1);
            break;
        }
        case xfl::BMP :
        {
            Filter = filters.at(2);
            break;
        }
    }

    FileName = QFileDialog::getSaveFileName(this, "Save image",
                                            SaveOptions::lastExportDirName(),
                                            "Portable Network Graphics (*.png);;JPEG (*.jpg);;Windows Bitmap (*.bmp)",
                                            &Filter);

    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastExportDirName(FileName.left(pos));

    if(Filter == filters.at(0))
    {
        if(FileName.right(4)!=".png") FileName+= ".png";
        m_ImageFormat = xfl::PNG;
    }
    else if(Filter == filters.at(1))
    {
        if(FileName.right(4)!=".jpg") FileName+= ".jpg";
        m_ImageFormat = xfl::JPEG;
    }
    else if(Filter == filters.at(2))
    {
        if(FileName.right(4)!=".bmp") FileName+= ".bmp";
        m_ImageFormat = xfl::BMP;
    }

    switch(s_iApp)
    {
        case xfl::XDIRECT:
        {
            if(m_pXDirect->isDesignView())
            {
                QPixmap pix = m_pXDirect->m_pDFoilWt->grab();
                pix.save(FileName, "PNG");
            }
            else if(m_pXDirect->isBLView())
            {
                QPixmap pix = m_pBLTiles->grab();
                pix.save(FileName, "PNG");
            }
            else if(m_pXDirect->isOppView())
            {
                QPixmap pix = m_pXDirect->m_pOpPointWt->grab();
                pix.save(FileName, "PNG");
            }
            else if(m_pXDirect->isPolarView())
            {
                QPixmap pix = m_pPolarTiles->grab();
                pix.save(FileName, "PNG");
            }
            break;
        }
        case xfl::XSAIL:
        {
            if(m_pXSail->is3dView())
            {
                QPixmap outPix = m_pXSail->m_pgl3dXSailView->grab();
                outPix.save(FileName);
            }
            else
            {
                QPixmap pix = m_pXSailTiles->grab();
                pix.save(FileName, "PNG");
            }
            break;
        }
        case xfl::XPLANE:
        {
            if(m_pXPlane->is3dView())
            {
                QPixmap outPix = m_pXPlane->m_pgl3dXPlaneView->grab();
                outPix.save(FileName);
            }
            else
            {
                if(m_pXPlane->isPOppView())
                {
                    QPixmap pix = m_pPOppTiles->grab();
                    pix.save(FileName, "PNG");
                }
                else if(m_pXPlane->isStabTimeView())
                {
                    QPixmap pix = m_pStabTimeTiles->grab();
                    pix.save(FileName, "PNG");
                }
                else if(m_pXPlane->isStabPolarView())
                {
                    QPixmap pix = m_pStabPolarTiles->grab();
                    pix.save(FileName, "PNG");
                }
                else if(m_pXPlane->isCpView())
                {
                    QPixmap pix = m_pCpViewWt->grab();
                    pix.save(FileName, "PNG");
                }
                else
                {
                    QPixmap pix = m_pWPolarTiles->grab();
                    pix.save(FileName, "PNG");
                }
            }
            break;
        }
         default:
            break;
    }
    QString TempFilePath = QDir::toNativeSeparators(FileName);
    QApplication::clipboard()->setText(TempFilePath);
    QString strange("The image file has been saved to "+ TempFilePath+" and the path has been copied to the clipboard.\n");
    displayMessage(strange, false);
}


void MainFrame::hideDockWindows()
{
    m_ptbXPlane->hide();
    m_ptbXDirect->hide();
    m_ptbDFoil->hide();
    m_ptbXSail->hide();

    m_pdwGraphControls->hide();

    m_pdwXDirect->hide();
    m_pdwFoilTree->hide();
    m_pdwFoilTable->hide();
    m_pdwOpPoint->hide();

    m_pdwCp3d->hide();
    m_pdwAnalysis3d->hide();
    m_pdwPlaneTree->hide();
    m_pdwXPlaneResults3d->hide();
    m_pdwStabTime->hide();

    m_pdwXSail->hide();
    m_pdwXSail3dCtrls->hide();
    m_pdwBoatTree->hide();
}


void MainFrame::stopAnimations()
{
    if(m_pXDirect) m_pXDirect->stopAnimate();
    if(m_pXPlane) m_pXPlane->stopAnimate();
}


void MainFrame::onDFoil()
{
    if(!m_pXDirect) return;

    m_pXDirect->setDesignView();
    if(s_iApp!=xfl::XDIRECT)
    {
        s_iApp = xfl::XDIRECT;

        if(m_pXPlane) m_pXPlane->stopAnimate();

        hideDockWindows();

        m_pdwFoilTable->show();
        m_ptbXDirect->hide();
        m_ptbDFoil->setVisible(true);
        setActiveCentralWidget();
        setMenus();
    }

    m_pXDirect->setFoil();
    m_pDFoilWt->resetLegend();

    m_pXDirect->setControls();
    m_pXDirect->showDockWidgets();
    m_pXDirect->updateView();
}


void MainFrame::onXDirect()
{
    if(!m_pXDirect) return;

    if(s_iApp!=xfl::XDIRECT)
    {
        s_iApp = xfl::XDIRECT;

        if(m_pXPlane) m_pXPlane->stopAnimate();

        hideDockWindows();

        if(m_pXDirect->isDesignView())
        {
            m_pdwFoilTree->hide();
            m_pdwFoilTable->show();
            m_ptbDFoil->setVisible(true);
        }
        else
        {
            m_pdwFoilTree->show();
            m_pdwFoilTable->hide();
            m_ptbDFoil->setVisible(false);
        }

        m_ptbXDirect->show();
        setActiveCentralWidget();
        setMenus();

        if     (m_pXDirect->isBLView())       m_pdwGraphControls->setWidget(m_pBLTiles->graphControls());
        else if(m_pXDirect->isPolarView())    m_pdwGraphControls->setWidget(m_pPolarTiles->graphControls());
    }

    m_pXDirect->setFoil();
//    m_pXDirect->setPolar();
//    m_pXDirect->setOpp();
    m_pXDirect->resetCurves();

    m_pDFoilWt->resetLegend();

    if(m_pXDirect->m_pOpPointWt) m_pXDirect->m_pOpPointWt->setFoilScale(true);

    m_pXDirect->updateFoilExplorers();
    m_pXDirect->m_pFoilExplorer->selectObjects();
    m_pXDirect->m_pFoilExplorer->setObjectProperties();
    m_pXDirect->resetCurves();

    m_pXDirect->setControls();
    m_pXDirect->showDockWidgets();
    m_pXDirect->updateView();
}


void MainFrame::onXPlane()
{
    if(s_iApp!=xfl::XPLANE)
    {
        stopAnimations();

        s_iApp = xfl::XPLANE;

        hideDockWindows();
        m_ptbXPlane->show();
        m_pdwAnalysis3d->show();
        m_pdwPlaneTree->show();

        setMenus();
        setActiveCentralWidget();

        if(m_pXPlane->isPOppView())
            m_pdwGraphControls->setWidget(m_pPOppTiles->graphControls());
        else if(m_pXPlane->isStabPolarView())
            m_pdwGraphControls->setWidget(m_pStabPolarTiles->graphControls());
        else if(m_pXPlane->isStabTimeView())
            m_pdwGraphControls->setWidget(m_pStabTimeTiles->graphControls());
        else if(m_pXPlane->isPolarView())
            m_pdwGraphControls->setWidget(m_pWPolarTiles->graphControls());

        m_pdwGraphControls->setVisible(!m_pXPlane->isCpView());

        Objects2d::cancelTEFlapAngles();
    }

    if(m_pXPlane) // keeping CLang quiet
    {
        displayMessage("Setting planes, polars and operating points...", false);
        m_pXPlane->cancelStreamLines();

        m_pXPlane->setPlane();

        displayMessage(" done\n\n", false);

        m_pXPlane->updateTreeView();
        m_pXPlane->m_pPlaneExplorer->selectObjects();
        m_pXPlane->m_pPlaneExplorer->setObjectProperties();
        m_pXPlane->resetCurves();
        m_pXPlane->m_pgl3dXPlaneView->resetglPOpp();
        m_pXPlane->m_pgl3dXPlaneView->resetglColorMap();

        m_pXPlane->m_pgl3dXPlaneView->reset3dScale();
        m_pXPlane->setControls();
    }

    updateView();
}


void MainFrame::onXSail()
{
    if(s_iApp!=xfl::XSAIL)
    {
        stopAnimations();
        s_iApp = xfl::XSAIL;

        hideDockWindows();
        m_ptbXSail->show();
        m_pdwXSail->show();
        m_pdwBoatTree->show();
        // other docks are set in setControls() depending on the view
        setMenus();
        setActiveCentralWidget();
        m_pdwGraphControls->setWidget(m_pXSailTiles->graphControls());
    }

    m_pXSail->setBoat();

    m_pXSail->m_pgl3dXSailView->reset3dScale();

    m_pXSail->updateObjectView();
    m_pXSail->resetCurves();

    m_pXSail->enableObjectView(true);
    m_pXSail->m_pBoatExplorer->selectObjects();
    m_pXSail->setControls();
    m_pXSail->updateView();
    m_pXSail->m_pgl3dXSailView->resetglBtOpp();
    m_pXSail->m_pgl3dXSailView->resetglColorMap();    
}


bool MainFrame::saveProject(const QString &filepath)
{
    QString PathName(filepath);
    if(PathName.endsWith(".xfl")) PathName = PathName.replace(".xfl", ".fl5");

    QString tempdir = SaveOptions::tempDirName();
    QFileInfo dirinfo(tempdir);
    if (!dirinfo.exists() || !dirinfo.isWritable()) tempdir = QDir::tempPath();

    QString backupFileName = tempdir + QDir::separator() + m_ProjectName + ".bak";

    QFile fp(PathName);
    if (!fp.open(QIODevice::WriteOnly))
    {
        onShowLogWindow(true);
        QString strange = "Could not open the file: "+PathName+" for writing\n\n";
        displayMessage(strange, true);
        onShowLogWindow(true);
        return false;
    }
    fp.close(); // to authorize removal


    // write first to a backup file in the temp directory
    // if successful, request the OS to copy and overwrite in the destination folder.
    QFile fpb(backupFileName);
    if (!fpb.open(QIODevice::WriteOnly))
    {
        onShowLogWindow(true);
        QString strange = "Could not open the temporary file: "+backupFileName+" for writing\n\n";
        displayMessage(strange, true);
        onShowLogWindow(true);
        return false;
    }

    displayMessage(EOLch + "Saving project "+filepath + EOLch, false);

    QDataStream ar(&fpb);
    FileIO saver;
    connect(&saver, SIGNAL(displayMessage(QString)), m_pLogMessageDlg, SLOT(onAppendPlainText(QString)));

    bool bSaved = saver.serializeProjectFl5(ar, true);

    if(!bSaved)
    {
        QString strange = "Error saving the project file " + backupFileName;
        displayMessage(strange, true);
        onShowLogWindow(true);
        return false;
    }
    else
    {
        if(QFile::remove(PathName))
        {
            if(!fpb.copy(PathName))
            {
                QString strange = "Error copying the backup file\n" + backupFileName + " to\n"+PathName;
                displayMessage(strange, true);
                onShowLogWindow(true);
                return false;
            }
            else
            {
                QFile::remove(backupFileName);
            }
        }
        else
        {
            QString strange = "Error when saving the file\n" + PathName;
            strange += "\nA back-up file has been generated in "+backupFileName;
            displayMessage(strange,true);
            onShowLogWindow(true);
            return false;
        }
    }

    m_FilePath = PathName;
    fpb.close();

    saveSettings();

    setSavedState(true);
    addRecentFile(m_FilePath);

    return true;
}


void MainFrame::onSavePlaneAsProject()
{
    QString strong;
    if(m_pXPlane->m_pCurPlane) strong = QString::fromStdString(m_pXPlane->m_pCurPlane->name())+".fl5";

    QString pathname;
    QString Filter = "flow5 project file (*.fl5)";
    QString FileName = strong;

    pathname = QFileDialog::getSaveFileName(this, "Save the project file",
                                            SaveOptions::lastDirName()+"/"+FileName,
                                            Filter,
                                            &Filter);

    if(!pathname.length()) return;//nothing more to do
    int pos = pathname.indexOf(".fl5", Qt::CaseInsensitive);
    if(pos<0) pathname += ".fl5";
    pathname.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator

    QFileInfo fileInfo(pathname);
    SaveOptions::setLastDirName(fileInfo.path());

    QFile fp(pathname);

    if (!fp.open(QIODevice::WriteOnly))
    {
        onShowLogWindow(true);
        QString strange = "Could not open the file: "+pathname+" for writing\n\n";
        displayMessage(strange, true);
        onShowLogWindow(true);
        return;
    }

    QDataStream ar(&fp);

    FileIO saver;
    saver.serializeProjectMetaDataFl5(ar, true);

    //make the list of foils to save in the project file
    std::vector<Foil*> FoilList;
    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pXPlane->m_pCurPlane);
    if(pPlaneXfl)
    {
        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            WingXfl const*pWing = pPlaneXfl->wing(iw);
            for(int is=0; is<pWing->nSections(); is++)
            {
                WingSection const &ws = pWing->section(is);
                Foil *pRFoil = Objects2d::foil(ws.rightFoilName());
                Foil *pLFoil = Objects2d::foil(ws.leftFoilName());
                if(pRFoil)
                {
                    if(std::find(FoilList.begin(), FoilList.end(), pRFoil)==FoilList.end())
                        FoilList.push_back(pRFoil);
                }
                if(pLFoil)
                {
                    if(std::find(FoilList.begin(), FoilList.end(), pLFoil)==FoilList.end())
                        FoilList.push_back(pLFoil);
                }
            }
        }
    }


    FileIO fileio;
    fileio.storeFoilsFl5(FoilList, ar, false);
    fileio.storePlaneFl5(m_pXPlane->m_pCurPlane, ar);
    fileio.serializeBtObjectsFl5(ar, true);

    displayMessage(pathname + " has been saved successfully\n\n", false);

    fp.close();
}


void MainFrame::onSaveBoatAsProject()
{
    QString strong;
    Boat *pBoat = m_pXSail->curBoat();
    if(!pBoat)
    {
        displayMessage("No active boat to save\n", true);
        return;
    }

    strong = QString::fromStdString(pBoat->name()).simplified()+".fl5";

    QString pathname;
    QString Filter = "flow5 project file (*.fl5)";
    QString FileName = strong;

    pathname = QFileDialog::getSaveFileName(this, "Save the project file",
                                            SaveOptions::lastDirName()+"/"+FileName,
                                            "flow5 project file (*.fl5)",
                                            &Filter);

    if(!pathname.length()) return;//nothing more to do
    int pos = pathname.indexOf(".fl5", Qt::CaseInsensitive);
    if(pos<0) pathname += ".fl5";
    pathname.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator

    QFileInfo fileInfo(pathname);
    SaveOptions::setLastDirName(fileInfo.path());

    QFile fp(pathname);

    if (!fp.open(QIODevice::WriteOnly))
    {
        onShowLogWindow(true);
        QString strange = "Could not open the file: "+pathname+" for writing\n\n";
        displayMessage(strange, true);
        onShowLogWindow(true);
        return;
    }
    fp.close();

    if(io::saveBoatAsProject(pBoat, pathname))
        displayMessage(pathname + " has been saved successfully\n\n", false);
    else
        displayMessage("Error exporting the boat to :" + pathname + "_n\n", true, true);
}


bool MainFrame::loadSettings()
{
    int SettingsFormat=0;

#if defined Q_OS_MAC
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"flow5", "flow5");
#elif defined Q_OS_LINUX
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"flow5", "flow5");
#else
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"flow5");
#endif

    settings.beginGroup("MainFrame");
    {
        SettingsFormat = settings.value("SettingsFormat").toInt();
        if(SettingsFormat != SETTINGSFORMAT)
        {
            outputSettingsStatus(settings, true);
            return false;
        }

        PrefsDlg::setStyleName(settings.value("Style", PrefsDlg::styleName()).toString());

        DisplayOptions::setStyleSheetOverride(settings.value("bStyleSheet", DisplayOptions::bStyleSheetOverride()).toBool());

        int k = settings.value("ExportFileType", 0).toInt();
        if (k==0) xfl::setExportFileType(xfl::TXT);
        else      xfl::setExportFileType(xfl::CSV);

        m_GraphExportFilter = settings.value("GraphExportFilter",".csv").toString();

        m_ImageDirName = settings.value("ImageDirName").toString();

        int lengthidx = settings.value("LengthUnit").toInt();
        switch(lengthidx)
        {
            case 0: Units::setLengthUnit(Units::MM);     break;
            case 1: Units::setLengthUnit(Units::CM);     break;
            case 2: Units::setLengthUnit(Units::DM);     break;
            default:
            case 3: Units::setLengthUnit(Units::M);      break;
            case 4: Units::setLengthUnit(Units::INCH);   break;
            case 5: Units::setLengthUnit(Units::FT);   break;
        }

        int speedidx = settings.value("SpeedUnit").toInt();
        switch(speedidx)
        {
            default:
            case 0: Units::setSpeedUnit(Units::MS);     break;
            case 1: Units::setSpeedUnit(Units::KMH);    break;
            case 2: Units::setSpeedUnit(Units::FTS);    break;
            case 3: Units::setSpeedUnit(Units::KT);     break;
            case 4: Units::setSpeedUnit(Units::MPH);    break;
        }


        int areaidx = settings.value("AreaUnit").toInt();
        switch(areaidx)
        {
            case 0: Units::setAreaUnit(Units::MM2);  break;
            case 1: Units::setAreaUnit(Units::CM2);  break;
            case 2: Units::setAreaUnit(Units::DM2);  break;
            default:
            case 3: Units::setAreaUnit(Units::M2);   break;
            case 4: Units::setAreaUnit(Units::IN2);  break;
            case 5: Units::setAreaUnit(Units::FT2);  break;
        }

        int massidx = settings.value("WeightUnit").toInt();
        switch(massidx)
        {
            case 0: Units::setMassUnit(Units::G);      break;
            default:
            case 1: Units::setMassUnit(Units::KG);     break;
            case 2: Units::setMassUnit(Units::OZ);     break;
            case 3: Units::setMassUnit(Units::LB);     break;
        }

        int forceidx = settings.value("ForceUnit").toInt();
        switch(forceidx)
        {
            default:
            case 0: Units::setForceUnit(Units::N);      break;
            case 1: Units::setForceUnit(Units::KN);     break;
            case 2: Units::setForceUnit(Units::TON);    break;
            case 3: Units::setForceUnit(Units::LBF);    break;
        }

        int momentidx = settings.value("MomentUnit").toInt();
        switch(momentidx)
        {
            default:
            case 0: Units::setMomentUnit(Units::NM);        break;
            case 1: Units::setMomentUnit(Units::LBFIN);   break;
            case 2: Units::setMomentUnit(Units::LBFFT);   break;
        }

        int pressureidx = settings.value("PressureUnit").toInt();
        switch(pressureidx)
        {
            default:
            case 0: Units::setPressureUnit(Units::PA);      break;
            case 1: Units::setPressureUnit(Units::HPA);     break;
            case 2: Units::setPressureUnit(Units::KPA);     break;
            case 3: Units::setPressureUnit(Units::MPA);     break;
            case 4: Units::setPressureUnit(Units::BAR);     break;
            case 5: Units::setPressureUnit(Units::PSI);     break;
            case 6: Units::setPressureUnit(Units::KSI);     break;
        }

        int inertiaidx =  settings.value("InertiaUnit").toInt();
        switch(inertiaidx)
        {
            default:
            case 0: Units::setInertiaUnit(Units::KGM2);     break;
            case 1: Units::setInertiaUnit(Units::LBMFT2);   break;
        }

        int fluididx = settings.value("FluidUnits", Units::fluidUnitType()).toInt();

        switch(fluididx)
        {
            default:
            case 0: Units::setFluidUnitType(Units::IS);       break;
            case 1: Units::setFluidUnitType(Units::IMPERIAL); break;
        }

        Units::setUnitConversionFactors();

        PlanePolar::setVariableNames();
        PlaneOpp::setVariableNames();
        BoatPolar::setVariableNames();

        switch(settings.value("ImageFormat").toInt())
        {
            case 0:
                m_ImageFormat = xfl::PNG;
                break;
            case 1:
                m_ImageFormat = xfl::JPEG;
                break;
            case 2:
                m_ImageFormat = xfl::BMP;
                break;
            default:
                m_ImageFormat = xfl::PNG;
                break;
        }

        //        a = settings.value("RecentFileSize").toInt();
        QString RecentF,strange;
        m_RecentFiles.clear();
        int n=0;
        do
        {
            RecentF = QString("RecentFile_%1").arg(n);
            strange = settings.value(RecentF).toString();
            if(strange.length())
            {
                m_RecentFiles.append(strange);

                n++;
            }
            else break;
        }while(n<MAXRECENTFILES);

        s_XflProjectPath = settings.value("XflProjectPath",SaveOptions::lastDirName()).toString();

        m_pLogMessageDlg->restoreGeometry(settings.value("LogMsgGeom").toByteArray());
        ObjectPropsDlg::setWindowGeometry(settings.value("ObjectPropsDlg").toByteArray());
        GraphDlg::setWindowGeometry(settings.value("GraphDlg").toByteArray());

    }
    settings.endGroup();

//    gl3dView::loadSettings(settings); // settings have already been loaded at app launch - do not overwrite in case of forced ogl version

    xfl::loadCoreSettings(settings);
    DisplayOptions::loadSettings(settings);
    SaveOptions::loadSettings(settings);

    Attractor2d::loadSettings(settings);
    CPTableView::loadSettings(settings);
    ColourLegend::loadSettings(settings);
    CpViewWt::loadSettings(settings);
    CrossFlowCtrls::loadSettings(settings);
    FastGraphWt::loadSettings(settings);
    FlowCtrls::loadSettings(settings);
    FoilPlrListDlg::loadSettings(settings);
    FoilSVGWriter::loadSettings(settings);
    FuseMesherDlg::loadSettings(settings);
    GraphOptions::loadSettings(settings);
    LogWt::loadSettings(settings);
    GMesherWt::loadSettings(settings);
    OneVortonTestDlg::loadSettings(settings);
    OpenGlDlg::loadSettings(settings);
    Opp3dScalesCtrls::loadSettings(settings);
    Panel3TestDlg::loadSettings(settings);
    PanelAnalysisTest::loadSettings(settings);
    PrefsDlg::loadSettings(settings);
    RenameDlg::loadSettings(settings);
    Section2dOptions::loadSettings(settings);
    SelectionDlg::loadSettings(settings);
    SeparatorsDlg::loadSettings(settings);
    Stab3dCtrls::loadSettings(settings);
    StreamLineCtrls::loadSettings(settings);
    VortonTestDlg::loadSettings(settings);
    W3dPrefs::loadSettings(settings);
    gl2dFractal::loadSettings(settings);
    gl2dQuat::loadSettings(settings);
    gl2dComplex::loadSettings(settings);
    gl2dNewton::loadSettings(settings);
    gl3dAttractors::loadSettings(settings);
    gl3dAxesView::loadSettings(settings);
    gl3dBoids2::loadSettings(settings);
    gl3dBoids::loadSettings(settings);
    gl3dContourView::loadSettings(settings);
    gl3dFlightView::loadSettings(settings);
    gl3dFlowVtx::loadSettings(settings);
    gl3dHydrogen::loadSettings(settings);
    gl3dLorenz2::loadSettings(settings);
    gl3dLorenz::loadSettings(settings);
    gl3dOptim2d::loadSettings(settings);
    gl3dPanelField::loadSettings(settings);
    gl3dQuadField::loadSettings(settings);
    gl3dQuat::loadSettings(settings);
    gl3dSagittarius::loadSettings(settings);
    gl3dShadow::loadSettings(settings);
    gl3dSolarSys::loadSettings(settings);
    gl3dSpace::loadSettings(settings);
    gl3dTexture::loadSettings(settings);
    gl3dVortonField::loadSettings(settings);
    gl3dSingularity::loadSettings(settings);

    m_pCpViewWt->CpGraph()->loadSettings(settings);

    m_pXDirect->loadSettings(settings);
    m_pXPlane->loadSettings(settings);
    m_pXSail->loadSettings(settings);

    outputSettingsStatus(settings, true);

    return true;
}


void MainFrame::outputSettingsStatus(QSettings const &settings, bool bLoad)
{
    QString filename = settings.fileName();
    QFileInfo fi(filename);
    if(bLoad)
    {
        if(fi.exists())
        {
            displayMessage(QString("Loading settings from the file %1\n").arg(filename), false, false);
            switch (settings.status())
            {
                case QSettings::NoError:
                    displayMessage("Loaded settings successfully\n\n", false, false);
                    break;
                case QSettings::AccessError:
                    displayMessage("Settings access error during load\n\n", true, false);
                    break;
                case QSettings::FormatError:
                    displayMessage("Settings format error during load\n\n", true, false);
                    break;
            }

        }
        else
            displayMessage(QString("Settings file %1 not found\n\n").arg(filename), false, false);
    }
    else
    {
        if(!settings.isWritable())
        {
            QString strange;
            strange = QString("The settings file %1 is not writable").arg(filename);
            QMessageBox::warning(this, tr("Warning"), strange);
        }
        else
        {
            switch (settings.status())
            {
                case QSettings::NoError:
                    break;
                case QSettings::AccessError:
                    QMessageBox::warning(this, tr("Warning"), "Settings access error during save\n\n");
                    break;
                case QSettings::FormatError:
                    QMessageBox::warning(this, tr("Warning"), "Settings format error during save\n\n");
                    break;
            }
        }
    }
}


void MainFrame::saveSettings()
{
    if(!m_bSaveSettings) return;

    displayMessage("Saving settings\n\n", false);

#if defined Q_OS_MAC
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"flow5", "flow5");
#elif defined Q_OS_LINUX
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"flow5", "flow5");
#else
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "flow5");
#endif

    settings.beginGroup("MainFrame");
    {
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        settings.setValue("SettingsFormat", SETTINGSFORMAT);

        settings.setValue("Style",       PrefsDlg::styleName());
        settings.setValue("bStyleSheet", DisplayOptions::bStyleSheetOverride());

        if (xfl::exportFileType()==xfl::TXT) settings.setValue("ExportFileType", 0);
        else                                         settings.setValue("ExportFileType", 1);

        settings.setValue("GraphExportFilter", m_GraphExportFilter);

        settings.setValue("ImageDirName", m_ImageDirName);

        settings.setValue("LengthUnit",   Units::lengthUnit());
        settings.setValue("AreaUnit",     Units::areaUnit());
        settings.setValue("WeightUnit",   Units::massUnit());
        settings.setValue("SpeedUnit",    Units::speedUnit());
        settings.setValue("ForceUnit",    Units::forceUnit());
        settings.setValue("MomentUnit",   Units::momentUnit());
        settings.setValue("PressureUnit", Units::pressureUnit());
        settings.setValue("InertiaUnit",  Units::inertiaUnit());
        settings.setValue("FluidUnits",   Units::fluidUnitType());

        settings.setValue("ImageFormat",    m_ImageFormat);
        settings.setValue("RecentFileSize", m_RecentFiles.size());


        QString RecentF;
        for(int i=0; i<m_RecentFiles.size() && i<MAXRECENTFILES; i++)
        {
            RecentF = QString("RecentFile_%1").arg(i);
            if(m_RecentFiles[i].length()) settings.setValue(RecentF, m_RecentFiles.at(i));
            else                          settings.setValue(RecentF, QString());
        }
        for(int i=m_RecentFiles.size(); i<MAXRECENTFILES; i++)
        {
            RecentF = QString("RecentFile_%1").arg(i);
            settings.setValue(RecentF, "");
        }

        settings.setValue("XflProjectPath",s_XflProjectPath);


        settings.setValue("LogMsgGeom", m_pLogMessageDlg->saveGeometry());
        settings.setValue("ObjectPropsDlg", ObjectPropsDlg::windowGeometry());
        settings.setValue("GraphDlg", GraphDlg::windowGeometry());

    }
    settings.endGroup();


    DisplayOptions::saveSettings(settings);
    SaveOptions::saveSettings(settings);
    xfl::saveCoreSettings(settings);

    Attractor2d::saveSettings(settings);
    CPTableView::saveSettings(settings);
    ColourLegend::saveSettings(settings);
    CpViewWt::saveSettings(settings);
    CrossFlowCtrls::saveSettings(settings);
    FastGraphWt::saveSettings(settings);
    FlowCtrls::saveSettings(settings);
    FoilPlrListDlg::saveSettings(settings);
    FoilSVGWriter::saveSettings(settings);
    FuseMesherDlg::saveSettings(settings);
    GraphOptions::saveSettings(settings);
    LogWt::saveSettings(settings);
    GMesherWt::saveSettings(settings);
    OneVortonTestDlg::saveSettings(settings);
    OpenGlDlg::saveSettings(settings);
    Opp3dScalesCtrls::saveSettings(settings);
    Panel3TestDlg::saveSettings(settings);
    PanelAnalysisTest::saveSettings(settings);
    PrefsDlg::saveSettings(settings);
    RenameDlg::saveSettings(settings);
    Section2dOptions::saveSettings(settings);
    SelectionDlg::saveSettings(settings);
    SeparatorsDlg::saveSettings(settings);
    Stab3dCtrls::saveSettings(settings);
    StreamLineCtrls::saveSettings(settings);
    VortonTestDlg::saveSettings(settings);
    W3dPrefs::saveSettings(settings);
    gl2dFractal::saveSettings(settings);
    gl2dQuat::saveSettings(settings);
    gl2dComplex::saveSettings(settings);
    gl2dNewton::saveSettings(settings);
    gl3dAttractors::saveSettings(settings);
    gl3dAxesView::saveSettings(settings);
    gl3dBoids2::saveSettings(settings);
    gl3dBoids::saveSettings(settings);
    gl3dContourView::saveSettings(settings);
    gl3dFlightView::saveSettings(settings);
    gl3dFlowVtx::saveSettings(settings);
    gl3dHydrogen::saveSettings(settings);
    gl3dLorenz2::saveSettings(settings);
    gl3dLorenz::saveSettings(settings);
    gl3dOptim2d::saveSettings(settings);
    gl3dPanelField::saveSettings(settings);
    gl3dQuadField::saveSettings(settings);
    gl3dQuat::saveSettings(settings);
    gl3dSagittarius::saveSettings(settings);
    gl3dShadow::saveSettings(settings);
    gl3dSolarSys::saveSettings(settings);
    gl3dSpace::saveSettings(settings);
    gl3dTexture::saveSettings(settings);
    gl3dView::saveSettings(settings);
    gl3dVortonField::saveSettings(settings);
    gl3dSingularity::saveSettings(settings);

    m_pCpViewWt->CpGraph()->saveSettings(settings);
    m_pXDirect->saveSettings(settings);
    m_pXPlane->saveSettings(settings);
    m_pXSail->saveSettings(settings);

    outputSettingsStatus(settings, false);

}


void MainFrame::setActiveCentralWidget()
{
    switch(s_iApp)
    {
        case xfl::NOAPP:
        {
            m_pswCentralWidget->setCurrentWidget(&m_VoidWidget);
            break;
        }
        case xfl::XPLANE:
        {
            if (m_pXPlane->is3dView())
            {
                m_pswCentralWidget->setCurrentWidget(m_pXPlane->m_pgl3dXPlaneView);
                m_pXPlane->m_pgl3dXPlaneView->setFocus();
            }
            else if (m_pXPlane->isPOppView())
            {
                m_pswCentralWidget->setCurrentWidget(m_pPOppTiles);
                m_pXPlane->setGraphTiles();
                m_pPOppTiles->setFocus();
            }
            else if(m_pXPlane->isStabPolarView())
            {
                m_pswCentralWidget->setCurrentWidget(m_pStabPolarTiles);
                m_pXPlane->setGraphTiles();
                m_pStabPolarTiles->setFocus();
            }
            else if(m_pXPlane->isStabTimeView())
            {
                m_pswCentralWidget->setCurrentWidget(m_pStabTimeTiles);
                m_pXPlane->setGraphTiles();
                m_pStabTimeTiles->setFocus();
            }
            else if (m_pXPlane->isCpView())
            {
                m_pswCentralWidget->setCurrentWidget(m_pCpViewWt);
                m_pCpViewWt->setFocus();
//                m_pswCentralWidget->setCurrentWidget(m_pFastGraphWt);
            }
            else
            {
                m_pswCentralWidget->setCurrentWidget(m_pWPolarTiles);
                m_pXPlane->setGraphTiles();
                m_pWPolarTiles->setFocus();
            }
            break;
        }
        case xfl::XDIRECT:
        {
            switch(m_pXDirect->eView())
            {
                case XDirect::DESIGNVIEW:
                {
                    m_pswCentralWidget->setCurrentWidget(m_pDFoilWt);
                    m_pDFoilWt->setFocus();
                    break;
                }
                case XDirect::BLVIEW:
                {
                    m_pswCentralWidget->setCurrentWidget(m_pBLTiles);
                    m_pXDirect->setGraphTiles();
                    m_pBLTiles->setFocus();
                    break;
                }
                case XDirect::OPPVIEW:
                {
                    m_pswCentralWidget->setCurrentWidget(m_pXDirect->m_pOpPointWt);
                    m_pXDirect->m_pOpPointWt->setFocus();
                    break;
                }
                 case XDirect::POLARVIEW:
                {
                    m_pswCentralWidget->setCurrentWidget(m_pPolarTiles);
                    m_pXDirect->setGraphTiles();
                    m_pPolarTiles->setFocus();
                    break;
                }
            }
            break;
        }
        case xfl::XSAIL:
        {
            if(m_pXSail->isPolarView())
            {
                m_pswCentralWidget->setCurrentWidget(m_pXSailTiles);
                m_pXSail->setGraphTiles();
                m_pXSailTiles->setFocus();
            }
            else
                m_pswCentralWidget->setCurrentWidget(m_pXSail->m_pgl3dXSailView);
            break;
        }
    }
}


void MainFrame::setMenus()
{
    menuBar()->clear();
    menuBar()->addMenu(m_pFileMenu);
    menuBar()->addMenu(m_pModuleMenu);
    switch(s_iApp)
    {
        case xfl::NOAPP:
        {
            break;
        }
        case xfl::XDIRECT:
        {
            if(m_pXDirect && m_pXDirect->m_pMenus)  // Keep CLang quiet and happy
            {
                menuBar()->addMenu(m_pXDirect->m_pMenus->m_pXDirectViewMenu);
                menuBar()->addMenu(m_pXDirect->m_pMenus->m_pXDirectFoilMenu);
                menuBar()->addMenu(m_pXDirect->m_pMenus->m_pXFoilAnalysisMenu);
                menuBar()->addMenu(m_pXDirect->m_pMenus->m_pPolarMenu);
                menuBar()->addMenu(m_pXDirect->m_pMenus->m_pOpPointMenu);
            }
            break;
        }
        case xfl::XPLANE:
        {
            if(m_pXPlane && m_pXPlane->m_pMenus)  // Keep CLang quiet and happy
            {
                menuBar()->addMenu(m_pXPlane->m_pMenus->m_pXPlaneViewMenu);
                menuBar()->addMenu(m_pXPlane->m_pMenus->m_pPlaneMenu);
                menuBar()->addMenu(m_pXPlane->m_pMenus->m_pXPlaneWPlrMenu);
                menuBar()->addMenu(m_pXPlane->m_pMenus->m_pXPlaneWOppMenu);
                menuBar()->addMenu(m_pXPlane->m_pMenus->m_pXPlaneAnalysisMenu);
            }
            break;
        }
        case xfl::XSAIL:
        {
            if(m_pXSail && m_pXSail->m_pMenus)  // Keep CLang quiet and happy
            {
                menuBar()->addMenu(m_pXSail->m_pMenus->m_pXSailViewMenu);
                menuBar()->addMenu(m_pXSail->m_pMenus->m_pBoatMenu);
                menuBar()->addMenu(m_pXSail->m_pMenus->m_pXSailWBtPlrMenu);
                menuBar()->addMenu(m_pXSail->m_pMenus->m_pXSailBtOppMenu);
                menuBar()->addMenu(m_pXSail->m_pMenus->m_pXSailAnalysisMenu);
            }
            break;
        }
    }
    menuBar()->addMenu(m_pOptionsMenu);
    menuBar()->addMenu(m_pHelpMenu);
}


void MainFrame::setProjectName(const QString &PathName)
{
    m_FilePath = PathName;
    QFileInfo fi(PathName);
    m_ProjectName = fi.completeBaseName();

    m_plabProjectName->setText(m_ProjectName);
}


void MainFrame::setSavedState(bool bSaved)
{
    s_bSaved = bSaved;
    if(bSaved) m_plabProjectName->setText(m_ProjectName);
    else       m_plabProjectName->setText(m_ProjectName+ "*");

    if(bSaved) m_pSaveAct->setIcon(QIcon(":/icons/save.png"));
    else       m_pSaveAct->setIcon(QIcon(":/icons/unsaved.png"));
}


void MainFrame::setRefGraphSettings()
{
    for(int ig=0; ig<m_pXDirect->m_BLGraph.count(); ig++)
        GraphOptions::resetGraphSettings(*m_pXDirect->m_BLGraph[ig]);

    for(int ig=0; ig<m_pXDirect->m_PlrGraph.count(); ig++)
        GraphOptions::resetGraphSettings(*m_pXDirect->m_PlrGraph[ig]);

    GraphOptions::resetGraphSettings(*m_pXDirect->m_pOppGraph);

    GraphOptions::resetGraphSettings(*m_pCpViewWt->CpGraph());

    for(int ig=0; ig<m_pXPlane->m_WingGraph.count(); ig++)
        GraphOptions::resetGraphSettings(*m_pXPlane->m_WingGraph[ig]);

    for(int ig=0; ig<m_pXPlane->m_WPlrGraph.count(); ig++)
            GraphOptions::resetGraphSettings(*m_pXPlane->m_WPlrGraph[ig]);

    for(int ig=0; ig<m_pXPlane->m_TimeGraph.count(); ig++)
            GraphOptions::resetGraphSettings(*m_pXPlane->m_TimeGraph[ig]);

    for(int ig=0; ig<m_pXPlane->m_StabPlrGraph.count(); ig++)
            GraphOptions::resetGraphSettings(*m_pXPlane->m_StabPlrGraph[ig]);

    for(int ig=0; ig<m_pXSail->plrGraphSize(); ig++) GraphOptions::resetGraphSettings(*m_pXSail->plrGraph(ig));

    for(int ig=0; ig<m_GraphWidget.size(); ig++) GraphOptions::resetGraphSettings(*m_GraphWidget[ig]->graph());

    GraphOptions::resetGraphSettings(*m_pFastGraphWt->graph());
}


QString MainFrame::shortenFileName(QString &PathName)
{
    QString strong, strange;
    int maxlength = 60;
    if(PathName.length()>maxlength)
    {
        int pos = PathName.lastIndexOf('/');
        if (pos>0)
        {
            strong = '/'+PathName.right(PathName.length()-pos-1);
            strange = PathName.left(maxlength-strong.length()-6);
            pos = strange.lastIndexOf('/');
            if(pos>0) strange = strange.left(pos)+"/...";
            strong = strange+strong;
        }
        else
        {
            strong = PathName.left(30);
        }
    }
    else strong = PathName;
    return strong;
}


void MainFrame::updateRecentFileActions()
{
    int numRecentFiles = qMin(m_RecentFiles.size(), MAXRECENTFILES);

    QString text;
    for (int i=0; i<numRecentFiles; ++i)
    {
        if(i<m_pRecentFileActs.size())
        {
            text = QString("&%1 %2").arg(i+1).arg(shortenFileName(m_RecentFiles[i]));
            if(i==0) text +="\tCtrl+Shift+O";

            m_pRecentFileActs[i]->setText(text);
            m_pRecentFileActs[i]->setData(m_RecentFiles.at(i));
            m_pRecentFileActs[i]->setVisible(true);
        }
    }
    for (int j=numRecentFiles; j<m_pRecentFileActs.size(); j++)
    {
        m_pRecentFileActs[j]->setVisible(false);
    }

    m_pSeparatorAct->setVisible(numRecentFiles>0);
    for (int i=0; i<m_pRecentFileActs.size(); i++)
    {
        m_pFileMenu->addAction(m_pRecentFileActs.at(i));
    }
}


void MainFrame::updateView()
{
    switch(s_iApp)
    {
        case xfl::XDIRECT:
        {
            m_pXDirect->updateView();
            break;
        }
        case xfl::XPLANE:
        {
            if(m_pXPlane)
                m_pXPlane->updateView();
            break;
        }
        case xfl::XSAIL:
        {
            m_pXSail->updateView();
            break;
        }
        default:
            break;
    }
}


void MainFrame::onProjectModified()
{
    setSavedState(false);
}


void MainFrame::onResetGraphSplitter()
{
    switch(s_iApp)
    {
        case xfl::XPLANE:
        {
            if(m_pXPlane->isPOppView())      m_pPOppTiles->onResetSplitters();
            if(m_pXPlane->isStabPolarView()) m_pStabPolarTiles->onResetSplitters();
            if(m_pXPlane->isStabTimeView())  m_pStabTimeTiles->onResetSplitters();
            else                             m_pWPolarTiles->onResetSplitters();
            break;
        }
        case xfl::XDIRECT:
        {
            if     (m_pXDirect->isBLView())       m_pBLTiles->onResetSplitters();
            else if(m_pXDirect->isPolarView())    m_pPolarTiles->onResetSplitters();
            break;
        }
        case xfl::XSAIL:
        {
            m_pXSailTiles->onResetSplitters();
            break;
        }
        default:
            return;
    }
}


/** only available for the plane opp graphs; */
void MainFrame::onShowGraphLegend()
{
    if(m_pPOppTiles->activeGraph())
        m_pPOppTiles->activeGraph()->setLegendVisible(m_pShowGraphLegend->isChecked());
}

void MainFrame::onShowInGraphLegend(bool bShow)
{
    switch(s_iApp)
    {
        case xfl::XPLANE:
        {
            if     (m_pXPlane->isPOppView())      m_pPOppTiles->showInGraphLegend(bShow);
            else if(m_pXPlane->isStabPolarView()) m_pStabPolarTiles->showInGraphLegend(bShow);
            else if(m_pXPlane->isStabTimeView())  m_pStabTimeTiles->showInGraphLegend(bShow);
            else if(m_pXPlane->isCpView())        m_pCpViewWt->showInGraphLegend(bShow);
            else                                  m_pWPolarTiles->showInGraphLegend(bShow);
            break;
        }
        case xfl::XDIRECT:
        {
            m_pPolarTiles->showInGraphLegend(bShow);
            break;
        }
        case xfl::XSAIL:
        {
            m_pXSailTiles->showInGraphLegend(bShow);
            break;
        }
        default:
            return;
    }
}


void MainFrame::onResetCurGraphScales()
{
    switch(s_iApp)
    {
        case xfl::XPLANE:
        {
            if     (m_pXPlane->isPOppView())      m_pPOppTiles->onResetCurGraphScales();
            else if(m_pXPlane->isStabPolarView()) m_pStabPolarTiles->onResetCurGraphScales();
            else if(m_pXPlane->isStabTimeView())  m_pStabTimeTiles->onResetCurGraphScales();
            else if(m_pXPlane->isCpView())        m_pCpViewWt->onResetCurGraphScales();
            else                                  m_pWPolarTiles->onResetCurGraphScales();
            break;
        }
        case xfl::XDIRECT:
        {
            m_pPolarTiles->onResetCurGraphScales();
            break;
        }
        case xfl::XSAIL:
        {
            m_pXSailTiles->onResetCurGraphScales();
            break;
        }
        default:
            return;
    }
}


void MainFrame::onExportCurGraphDataToFile()
{
    switch(s_iApp)
    {
        case xfl::XPLANE:
        {
            if     (m_pXPlane->isPOppView())      m_pPOppTiles->onExportGraphDataToFile();
            else if(m_pXPlane->isStabPolarView()) m_pStabPolarTiles->onExportGraphDataToFile();
            else if(m_pXPlane->isStabTimeView())  m_pStabTimeTiles->onExportGraphDataToFile();
            else if(m_pXPlane->isCpView())        m_pCpViewWt->onExportGraphDataToFile();
            else                                  m_pWPolarTiles->onExportGraphDataToFile();
            break;
        }
        case xfl::XDIRECT:
        {
            if     (m_pXDirect->isBLView())      m_pBLTiles->onExportGraphDataToFile();
            else if(m_pXDirect->isOppView())     m_pXDirect->CpGraph()->toClipboard();
            else if(m_pXDirect->isPolarView())   m_pPolarTiles->onExportGraphDataToFile();

            break;
        }
        case xfl::XSAIL:
        {
            m_pXSailTiles->onExportGraphDataToFile();
            break;
        }
        default:
            break;
    }
}


void MainFrame::onExportCurGraphToSVG()
{
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    QString TempFilePath;
    switch(s_iApp)
    {
        case xfl::XPLANE:
        {
            if     (m_pXPlane->isPOppView())      m_pPOppTiles->exportGraphToSVG(TempFilePath);
            else if(m_pXPlane->isStabPolarView()) m_pStabPolarTiles->exportGraphToSVG(TempFilePath);
            else if(m_pXPlane->isStabTimeView())  m_pStabTimeTiles->exportGraphToSVG(TempFilePath);
//            else if(m_pXPlane->isCpView())        m_pCpViewWt->onExportGraphToSVG();
            else                                  m_pWPolarTiles->exportGraphToSVG(TempFilePath);
            break;
        }
        case xfl::XDIRECT:
        {
            if(m_pXDirect->isBLView())           m_pBLTiles->exportGraphToSVG(TempFilePath);
//            else if(m_pXDirect->isOppView())     m_pXDirect->onExportGraphToSVG();
            else if(m_pXDirect->isPolarView())   m_pPolarTiles->exportGraphToSVG(TempFilePath);

            break;
        }
        case xfl::XSAIL:
        {
            m_pXSailTiles->exportGraphToSVG(TempFilePath);
            break;
        }
        default:
            break;
    }

    TempFilePath = QDir::toNativeSeparators(TempFilePath);
    QApplication::clipboard()->setText(TempFilePath);
    QString strange("The SVG file has been saved to "+ TempFilePath+" and the path has been copied to the clipboard.\n");
    displayMessage(strange, false);
}


void MainFrame::onCopyCurGraphData()
{
    switch(s_iApp)
    {
        case xfl::XPLANE:
        {
            if     (m_pXPlane->isPOppView())      m_pPOppTiles->onExportGraphDataToClipboard();
            else if(m_pXPlane->isStabPolarView()) m_pStabPolarTiles->onExportGraphDataToClipboard();
            else if(m_pXPlane->isStabTimeView())  m_pStabTimeTiles->onExportGraphDataToClipboard();
            else if(m_pXPlane->isCpView())        m_pCpViewWt->onExportGraphDataToClipboard();
            else                                  m_pWPolarTiles->onExportGraphDataToClipboard();
            break;
        }
        case xfl::XDIRECT:
        {
            if     (m_pXDirect->isBLView())    m_pBLTiles->onExportGraphDataToClipboard();
            else if(m_pXDirect->isOppView())   m_pXDirect->CpGraph()->toClipboard();
            else if(m_pXDirect->isPolarView()) m_pPolarTiles->onExportGraphDataToClipboard();
            break;
        }
        case xfl::XSAIL:
        {
            m_pXSailTiles->onExportGraphDataToClipboard();
            break;
        }
        default:
            break;
    }
    displayMessage("The graph data has been copied to the clipboard", false);
}


void MainFrame::onCurGraphSettings()
{
    switch(s_iApp)
    {
        case xfl::XPLANE:
        {
            if     (m_pXPlane->isPOppView())      m_pPOppTiles->onCurGraphSettings();
            else if(m_pXPlane->isStabPolarView()) m_pStabPolarTiles->onCurGraphSettings();
            else if(m_pXPlane->isCpView())        m_pCpViewWt->onCurGraphSettings();
            else if(m_pXPlane->isStabTimeView())  m_pStabTimeTiles->onCurGraphSettings();
            else                                  m_pWPolarTiles->onCurGraphSettings();
            break;
        }
        case xfl::XDIRECT:
        {
            m_pPolarTiles->onCurGraphSettings();
            break;
        }
        case xfl::XSAIL:
        {
            m_pXSailTiles->onCurGraphSettings();
            break;
        }
        default:
            break;
    }
}


void MainFrame::onSetNoApp()
{
    hideDockWindows();
    s_iApp = xfl::NOAPP;
    setMenus();
    setActiveCentralWidget();
    update();
}


/** Make one .plr file for each foil and save it to the path name*/
void MainFrame::onMakePlrFiles(const QString &pathname) const
{
    QString fileName;

    for(int l=0; l<Objects2d::nFoils(); l++)
    {
        Foil *pFoil = Objects2d::foil(l);
        fileName = pathname + QDir::separator() + QString::fromStdString(pFoil->name()) +".plr";

        io::writeFoilPolars(fileName, pFoil);
    }
}


bool MainFrame::onExportAllPolars(QString const &pathName, bool bCSV) const
{
    return io::exportAllPolars(pathName, bCSV? xfl::CSV : xfl::TXT);
}


void MainFrame::onPreferences()
{
    PrefsDlg dlg(this);
    dlg.initWidgets();
    dlg.exec();

    if(GraphOptions::isGraphModified())
    {
        setRefGraphSettings();
    }

    if(Section2dOptions::isModified())
    {
        FuseXflDefDlg::setFrameGrid(Section2dOptions::grid());
        FuseXflDefDlg::setBodyLineGrid(Section2dOptions::grid());
        m_pDFoilWt->setGrid(Section2dOptions::grid());
        m_pDFoilWt->setAutoUnits();
        FoilDlg::setGrid(Section2dOptions::grid());
        SailDlg::setSectionGrid(Section2dOptions::grid());
        Section2dOptions::setModified(false);
    }

    if(DisplayOptions::theme()==DisplayOptions::DARKTHEME)
    {
        m_pXDirect->m_pOpPointWt->setNeutralLineColor(QColor(190,190,190));
    }
    else
    {
        m_pXDirect->m_pOpPointWt->setNeutralLineColor(QColor(60,60,60));
    }

    m_pXDirect->resetPrefs();

    m_pXPlane->resetPrefs();
    m_pXPlane->updateUnits();
    m_pXPlane->m_pgl3dXPlaneView->resetBackground();


    m_pXSail->resetPrefs();
    m_pXSail->updateUnits();    
    m_pXSail->m_pgl3dXSailView->resetBackground();

    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    palette.setColor(QPalette::Window,     DisplayOptions::backgroundColor());
    m_VoidWidget.setPalette(palette); // to update its palette
    m_VoidWidget.update();

    setActiveCentralWidget();

    saveSettings();

    updateView();
}


void MainFrame::hideEvent(QHideEvent *pEvent)
{
    QMainWindow::hideEvent(pEvent);
    for(int ig=0; ig<m_GraphWidget.size(); ig++)
        m_GraphWidget[ig]->close();
}


void MainFrame::onCurvesUpdated()
{
    for(int ig=0; ig<m_GraphWidget.size(); ig++)
        m_GraphWidget[ig]->update();
}


void MainFrame::onCredits()
{
    CreditsDlg dlg(this);
    dlg.exec();
}


void MainFrame::onShowLogWindow(bool bShow)
{
    m_pLogMessageDlg->setVisible(bShow);
    m_pLogMessageDlg->raise();
    m_pLogMessageDlg->update();
    update();
}


void MainFrame::onOpenGraphInNewWindow()
{
    Graph *pGraph(nullptr);

    switch(s_iApp)
    {
        case xfl::XPLANE:
        {
            if(m_pXPlane->isPOppView())
            {
                if(m_pPOppTiles->activeGraphWt())
                    pGraph = m_pPOppTiles->activeGraphWt()->graph();
            }
            else if(m_pXPlane->isStabPolarView())
            {
                if(m_pStabPolarTiles->activeGraphWt())
                    pGraph = m_pStabPolarTiles->activeGraphWt()->graph();
            }
            else if(m_pXPlane->isStabTimeView())
            {
                if(m_pStabTimeTiles->activeGraphWt())
                    pGraph = m_pStabTimeTiles->activeGraphWt()->graph();
            }
            else if(m_pXPlane->isCpView())
            {
                pGraph = m_pXDirect->CpGraph();
            }
            else
            {
                if(m_pWPolarTiles->activeGraphWt())
                    pGraph = m_pWPolarTiles->activeGraphWt()->graph();
            }
            break;
        }
        case xfl::XDIRECT:
        {
            if(m_pXDirect->isBLView())
            {
                if(m_pBLTiles->activeGraphWt())
                    pGraph = m_pBLTiles->activeGraphWt()->graph();
            }
            else if(m_pXDirect->isOppView())
            {
                pGraph = m_pXDirect->m_pOppGraph;
            }
            else if(m_pXDirect->isPolarView())
            {
                if(m_pPolarTiles->activeGraphWt())
                    pGraph = m_pPolarTiles->activeGraphWt()->graph();
            }
            break;
        }
        case xfl::XSAIL:
        {
            if(m_pXSailTiles->activeGraphWt())
                pGraph = m_pXSailTiles->activeGraphWt()->graph();
            break;
        }
        default:
            break;
    }

    onOpenGraphInNewWindow(pGraph);
}


/** slot may have been called either from the menu action or by the graphWt shortcut */
void MainFrame::onOpenGraphInNewWindow(Graph *pGraph)
{
    if(!pGraph)
    {
        displayMessage("No active graph to open in a separate window", false);
        return;
    }

    GraphWt *pGW = new GraphWt();
    pGW->setWindowFlag(Qt::WindowStaysOnTopHint);

    m_GraphWidget.append(pGW);
    Graph *pNewGraph = new Graph;
    pNewGraph->setCurveModel(pGraph->curveModel());
    pNewGraph->copySettings(pGraph);
    pNewGraph->setXVariableList(pGraph->XVariableList());
    pNewGraph->setYVariableList(pGraph->YVariableList());
    pNewGraph->setVariables(pGraph->xVariable(), pGraph->yVariable(0), pGraph->yVariable(1));
    pNewGraph->setName(pGraph->name());

//    pNewGraph->setLegendVisible(true);
    pNewGraph->setAuto(true);
    pNewGraph->setName(pGraph->name());

    m_GraphWidget.back()->setGraph(pNewGraph);
;
    pGW->enableContextMenu(true);
    pGW->setEditable(false);
    m_GraphWidget.back()->show();
    connect(m_GraphWidget.back(), SIGNAL(widgetClosed(GraphWt*)), SLOT(onGraphWidgetClosed(GraphWt*)));
    //    this->raise();
    this->activateWindow();
}


void MainFrame::onGraphWidgetClosed(GraphWt *pGraphWidget)
{
    for(int ig=0; ig<m_GraphWidget.size(); ig++)
    {
        if(m_GraphWidget.at(ig)==pGraphWidget)
        {
            delete m_GraphWidget[ig]->graph();
            m_GraphWidget[ig]->setNullGraph();
            m_GraphWidget.remove(ig);
            break;
        }
    }
}


void MainFrame::setColorListFromFile()
{
    QString appdir = qApp->applicationDirPath();
    QString ColorPathName = appdir + QDir::separator() +"/colorlist.txt";

    QFileInfo fi(ColorPathName);
    if(!fi.exists()) ColorPathName = ":/textfiles/colorlist.txt";

    QFile ColorFile(ColorPathName);
    if(!ColorFile.open(QIODevice::ReadOnly)) return;

    QStringList LineColorList;
    QStringList LineColorNames;
    QVector<QColor> colors;
    QTextStream stream(&ColorFile);
    while(!stream.atEnd())
    {
        QString colorline = stream.readLine().simplified();
        QStringList colorpair = colorline.split(" ");
        if(colorpair.size()>=2)
        {
            LineColorList.append(colorpair.at(0));
            LineColorNames.append(colorpair.at(1));
            colors.append(LineColorList.back());
        }
    }
    for(int i=LineColorList.size(); i<NCOLORROWS*NCOLORCOLS; i++)
    {
        LineColorList.append(QString("#000000"));
        LineColorNames.append(QString("#000000"));
        colors.append(Qt::black);
    }

    CurveModel::setColorList(colors);
    LinePicker::setColorList(LineColorList, LineColorNames);
}


void MainFrame::setPlainColorsFromFile() const
{
    QFile ColorFile(":/textfiles/colors_google.txt");
    if(!ColorFile.open(QIODevice::ReadOnly)) return;

    QStringList GoogleColorNames;
    QTextStream stream(&ColorFile);
    stream.readLine();

    while(!stream.atEnd())
    {
        QString colorline = stream.readLine().simplified();
#if QT_VERSION >= 0x050F00
        QStringList colorpair = colorline.split(QRegularExpression("[,\\s\t]"), Qt::SkipEmptyParts);
#else
        QStringList colorpair = colorline.split(QRegExp("[,\\s\t]"), QString::SkipEmptyParts);
#endif
        GoogleColorNames.append(colorpair);
    }
}


void MainFrame::on3dAnalysisSettings()
{
    Analysis3dSettings A3dDlg(this);
    A3dDlg.initDialog();
    A3dDlg.exec();
}


void MainFrame::onFastGraph()
{
    if(!m_pFastGraphWt->isVisible())
        m_pFastGraphWt->setVisible(true);
    else
        m_pFastGraphWt->raise();
}


void MainFrame::setDefaultStaticFonts()
{
    //"Qt does not support style hints on X11 since this information is not provided by the window system."
    QFont generalfnt(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    QFont fixedfnt(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    DisplayOptions::setTextFont(fixedfnt);
    DisplayOptions::setTableFont(fixedfnt);
    DisplayOptions::setTreeFont(fixedfnt);
    DisplayOptions::setToolTipFont(generalfnt);

    QToolTip::setFont(DisplayOptions::toolTipFont());

    GraphOptions::setTitleFontStruct(DisplayOptions::treeFontStruct());// the general font to avoid fixed width by default
    GraphOptions::setLabelFontStruct(DisplayOptions::treeFontStruct());
    GraphOptions::setLegendFontStruct(DisplayOptions::treeFontStruct());
}


/** called from the command line */
void MainFrame::executeScript(QString const &XmlScriptName, bool bShowProgressStdIO)
{
    m_pLogWt->hide();
//    m_pScriptExecutor->moveToThread(QApplication::instance()->thread());

    if(!m_pScriptExecutor) m_pScriptExecutor = new XflScriptExec;
    m_pScriptExecutor->setEventDestination(nullptr);

    if(bShowProgressStdIO)
    {
        //output log messages to the console
        m_pScriptExecutor->setStdOutStream(true);
    }

    QFileInfo fi(XmlScriptName);
    if(!fi.exists())
    {
        //try again with script dir path
        fi.setFile(SaveOptions::xmlScriptDirName() + QDir::separator() + XmlScriptName);
    }

    if(!fi.exists())
    {
        QString strange("Script file not found... aborting\n");
        if(bShowProgressStdIO) m_pScriptExecutor->traceLog(strange);
    }
    else
    {
        m_pScriptExecutor->runScript(XmlScriptName);
    }

    handleScriptResults();
    delete m_pScriptExecutor;
}


void MainFrame::onExecuteScript()
{
    QString XmlPathName;
    XmlPathName = QFileDialog::getOpenFileName(this, "Open XML script file",
                                               SaveOptions::xmlScriptDirName(),
                                               "XML script file (*.xml)");
    if(!XmlPathName.length()) return;


    onSetNoApp();
    XDirect::setCurFoil(nullptr);
    XDirect::setCurPolar(nullptr);
    XDirect::setCurOpp(nullptr);

    m_pXPlane->m_pCurPlane  = nullptr;
    m_pXPlane->m_pCurPlPolar = nullptr;
    m_pXPlane->m_pCurPOpp   = nullptr;

    m_pLogWt->show();

    QFileInfo fi(XmlPathName);
    if(!fi.exists())
    {
        //try again with script dir path
        fi.setFile(SaveOptions::xmlScriptDirName() + QDir::separator() + XmlPathName);
    }

    if(!m_pScriptExecutor)
    {
        m_pScriptExecutor = new XflScriptExec;
        m_pScriptExecutor->moveToThread(&m_ScriptThread);
        connect(&m_ScriptThread,   &QThread::finished, m_pScriptExecutor, &QObject::deleteLater);
        connect(this, &MainFrame::runScript, m_pScriptExecutor, &XflScriptExec::runScript);
        connect(m_pScriptExecutor, &XflScriptExec::outputMessage, m_pLogWt, &LogWt::onOutputMessage, Qt::BlockingQueuedConnection);
        connect(m_pScriptExecutor, &XflScriptExec::taskFinished, this, &MainFrame::handleScriptResults);
        m_ScriptThread.start();
    }

    emit runScript(XmlPathName);
}


void MainFrame::handleScriptResults()
{
    if(!m_pScriptExecutor) return;

    bool bCSV = m_pScriptExecutor->bCSVOutput();
    if(m_pScriptExecutor->outputPolarBin())
        onMakePlrFiles(m_pScriptExecutor->foilPolarBinOutputDirPath());
    if(m_pScriptExecutor->outputPolarText())
    {
        if(onExportAllPolars(m_pScriptExecutor->foilPolarTextOutputDirPath(), bCSV))
        {
            m_pLogWt->onOutputMessage("The foil polars have been exported to text files\n");
        }
        else
        {
            m_pLogWt->onOutputMessage("Error exporting the foil polars to text files\n");
        }
    }

    if(m_pScriptExecutor->outputWPolarText())
    {
        if(io::exportAllWPolars(m_pScriptExecutor->outputDirPath(), bCSV))
        {
            m_pLogWt->onOutputMessage("The plane polars have been exported to text files\n");
        }
        else
        {
            m_pLogWt->onOutputMessage("Error exporting the plane polars to text files\n");
        }

        if(io::exportAllBtPolars(m_pScriptExecutor->outputDirPath(), bCSV))
        {
            m_pLogWt->onOutputMessage("The boat polars have been exported to text files\n");
        }
        else
        {
            m_pLogWt->onOutputMessage("Error exporting the boat polars to text files\n");
        }
    }

    if(m_pScriptExecutor->outputPOppText())
    {
        if(io::exportAllPOpps(m_pScriptExecutor->outputDirPath(), bCSV, m_pScriptExecutor->exportPanelCp()))
        {
            m_pLogWt->onOutputMessage("The plane operating points have been exported to text files\n");
        }
        else
        {
            m_pLogWt->onOutputMessage("Error exporting the plane operating points to text files\n");
        }

        if(io::exportAllBtOpps(m_pScriptExecutor->outputDirPath(), bCSV, m_pScriptExecutor->exportPanelCp()))
        {
            m_pLogWt->onOutputMessage("The boat operating points have been exported to text files\n");
        }
        else
        {
            m_pLogWt->onOutputMessage("Error exporting the boat polars to text files\n");
        }
    }

    if(m_pScriptExecutor->exportStlMesh())
    {
        QDir stldir(m_pScriptExecutor->outputDirPath()+QDir::separator()+"STL");
        if(!stldir.exists())
        {
            if(!stldir.mkpath(stldir.path()))
            {
                m_pLogWt->onOutputMessage("Failed to create the STL export directory: "+stldir.path()+EOLch);
            }
        }

        if(stldir.exists())
        {
           m_pLogWt->onOutputMessage("Exporting STL meshes to directory: "+stldir.path()+EOLch);
           io::exportAllStlMesh(stldir.path());
        }
    }

    if(m_pScriptExecutor->makeProjectFile())
    {
        QString FilePath = m_pScriptExecutor->projectFilePathName();
        if(saveProject(FilePath))
        {
            setProjectName(FilePath);
            m_pLogWt->onOutputMessage("The project "+ FilePath+" has been saved\n");
        }
        else
        {
            QString strange = "Error saving the project "+ FilePath+EOLch;
            m_pLogWt->onOutputMessage(strange);
        }
    }

    m_pLogWt->onOutputMessage("\n----- Script completed -----\n");

    m_pScriptExecutor->closeLogFile();

    if(m_pLogWt) m_pLogWt->setFinished(true);

    // duplicate the log file in the temp directory
    QString projectLogFileName = SaveOptions::newLogFileName();
    SaveOptions::setLastLogFileName(projectLogFileName);
    QFile::copy(m_pScriptExecutor->logFileName(), projectLogFileName);
}


Graph *MainFrame::cpGraph()
{
    return m_pCpViewWt->CpGraph();
}


void MainFrame::resetCpCurves()
{
    m_pXPlane->resetCurves();
    m_pXSail->resetCurves();
}


#ifdef WIN32
    #include <Windows.h>
#endif

int MainFrame::onTestRun()
{
    return 0;
}








