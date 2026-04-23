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


#pragma once

#include <QMainWindow>
#include <QDir>
#include <QTimer>
#include <QStringList>
#include <QStackedWidget>
#include <QLabel>
#include <QPointer>
#include <QThread>

#include <QAction>

#include <api/utils.h>
#include <core/enums_core.h>
#include <interfaces/graphs/graph/graph.h>

class Boat;
class CpGraphCtrls;
class CpViewWt;
class DFoil;
class DFoilWt;
class Foil;
class Graph;
class GraphWt;
class FastGraphWt;
class LogWt;
class LogMessageDlg;
class Plane;
class PlaneXfl;
class Polar;
class Vector3d;
class PlanePolar;
class XDirect;
class XPlane;
class GraphTiles;
class XflScriptExec;
class XSail;

/**
*@class MainFrame
*@brief The class associated to the application's main window.

  The class fills many functions:
  - it creates the child windows and toolbars of the application
  - it manages the loading and saving of settings
  - it stores and manages the arrays of data as member variables
  - it manages the load & save operations of project files
  
  This class will remain only partially documented.
*/
class MainFrame : public QMainWindow
{
    friend class Flow5App;
    friend class XDirect;
    friend class XDirectMenus;
    friend class XPlane;
    friend class XPlaneActions;
    friend class XPlaneMenus;
    friend class XSail;
    friend class XSailMenus;

    Q_OBJECT

    public:
        MainFrame(QWidget * parent = nullptr);
        ~MainFrame();

        QString const &projectName() const {return m_ProjectName;}
        xfl::enumApp loadProjectFile(const QString &PathName);


        void displayMessage(QString const & msg, bool bShowWindow, bool bStatusBar=true, int duration=5000);
        void displayHtmlMessage(QString const & msg, bool bShowWindow);
        void setSavedState(bool bSaved);
        void testOpenGL();

        void updateView();

        void showStabTimeCtrls(bool bVisible);

        void loadRecentProject(QString recentfilename=QString());

        bool loadSettings();
        void saveSettings();

        void executeScript(QString const &XmlScriptName, bool bShowProgressStdIO);

        Graph *cpGraph();
        void resetCpCurves();

        static xfl::enumApp xflApp()  {return s_iApp;}


    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void keyReleaseEvent(QKeyEvent *pEvent) override;
        void closeEvent (QCloseEvent * pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(1200,800);}


        QString shortenFileName(QString &PathName);

        bool saveProject(QString const &filepath);

        void addRecentFile(const QString &PathName);
        void connectSignals();
        void createMainFrameActions();
        void createDockWindows();
        void createMenus();
        void createStatusBar();
        void createToolbars();
        void createXDirectDockWindows();
        void createXDirectToolbars();
        void createXPlaneDockWindows();
        void createXPlaneToolbar();
        void createXSailToolbars();
        void createXSailDockWindows();
        void deleteProject();
        bool exportAllPolars(const QString &pathName, xfl::enumTextFileType fileType) const;
        void hideDockWindows();
        void setActiveCentralWidget();
        void setColorListFromFile();
        void setPlainColorsFromFile() const;
        void setRefGraphSettings();
        void setMenus();
        void setProjectName(QString const &PathName);
        void stopAnimations();
        void updateRecentFileActions();

        void setDefaultStaticFonts();
        void startTrace(bool bTrace);

        bool exportAllStlMesh(QString const &pathname);
        bool exportAllWPolars(const QString &pathName, bool bCSV);
        bool exportAllBtPolars(QString const &pathname, bool bCSV);
        bool exportAllPOpps(QString const &pathName, bool bCSV, bool bPanelData);
        bool exportAllBtOpps(const QString &pathname, bool bCSV, bool bPanelData) const;

    signals:
        void loadFile(QString);
        void runScript(QString);

    public slots:

        void handleIOResults(bool bError);
        void handleScriptResults();

        void onDFoil();
        void onXDirect();
        void onXPlane();
        void onXSail();
        void onExecuteScript();
        bool onExportAllPolars(const QString &pathName, bool bCSV) const;
        void onMakePlrFiles(QString const &pathName) const;

    private slots:
        void aboutFlow5();
        void aboutQt();
        void on3dAnalysisSettings();
        bool onCloseProject();
        void onCopyCurGraphData();
        void onCredits();
        void onCurGraphSettings();
        void onCurvesUpdated();
        void onExportCurGraphDataToFile();
        void onExportCurGraphToSVG();
        void onFastGraph();
        void onGraphWidgetClosed(GraphWt*);
        void onInsertProject();
        void onLoadProjectFile();
        void onLoadFoilFile();
        void onLoadPlrFile();
        void onLoadLastProject();
        void onLogFile();
        void onNewProject();
        void onOpenGLInfo();
        void onOpenGraphInNewWindow();
        void onOpenGraphInNewWindow(Graph *pGraph);
        void onLoadRecentFile();
        void onProcessFinished();
        void onPreferences();
        void onProjectModified();
        void onOnlineDoc();
        void onReleaseNotes();
        void onResetCurGraphScales();
        void onResetGraphSplitter();
        void onResetSettings();
        void onRestoreToolbars();
        void onSaveBoatAsProject();
        void onSavePlaneAsProject();
        void onSaveProject();
        void onSaveProjectAs();
        void onSaveTimer();
        void onSaveViewToImageFile();
        void onSetNoApp();
        void onShowInGraphLegend(bool bShow);
        void onShowGraphLegend();
        void onShowLogWindow(bool bShow=true);
        void onTraceFile();
        int onTestRun();

        //___________________________________________Variables_______________________________
    private:

        XPlane *m_pXPlane;    /**< A pointer to the instance of the XPlane application. The pointer will be cast to the XPlane type at runtime. This is necessary to prevent loop includes of header files. */
        XDirect *m_pXDirect;   /**< A pointer to the instance of the QXDirect application. The pointer will be cast to the QXDirect type at runtime. This is necessary to prevent loop includes of header files. */
        XSail *m_pXSail;


        QStackedWidget *m_pswCentralWidget;     /** The stacked widget which is loaded at the center of the display area. The stack switches between the widgets depending on the user's request. */

        GraphTiles *m_pWPolarTiles;
        GraphTiles *m_pPOppTiles;
        GraphTiles *m_pStabPolarTiles;
        GraphTiles *m_pStabTimeTiles;

        GraphTiles *m_pBLTiles;
        GraphTiles *m_pPolarTiles;

        GraphTiles  *m_pXSailTiles;


        DFoilWt *m_pDFoilWt;
        CpGraphCtrls *m_pCpGraphCtrl;
        CpViewWt *m_pCpViewWt;

        QWidget m_VoidWidget;

        QDockWidget *m_pdwXDirect;
        QDockWidget *m_pdwOpPoint;
        QDockWidget *m_pdwFoilTree;
        QDockWidget *m_pdwFoilTable;


        QDockWidget *m_pdwAnalysis3d;
        QDockWidget *m_pdwPlaneTree;
        QDockWidget *m_pdwXPlaneResults3d;
        QDockWidget *m_pdwStabTime;
        QDockWidget *m_pdwGraphControls;
        QDockWidget *m_pdwCp3d;

        QDockWidget *m_pdwXSail;
        QDockWidget *m_pdwXSail3dCtrls;
        QDockWidget *m_pdwBoatTree;


        QToolBar *m_ptbMain;
        QToolBar *m_ptbXDirect;   /**< The tool bar container which holds the instance of the QXDirect application  */
        QToolBar *m_ptbXPlane;
        QToolBar *m_ptbDFoil;
        QToolBar *m_ptbXSail;


        //Common Menus
        QMenu *m_pFileMenu, *m_pModuleMenu, *m_pOptionsMenu, *m_pHelpMenu;

        //MainFrame actions
        QAction *m_pNoAppAct, *m_pXDirectAct, *m_pXPlaneAct, *m_pXSailAct;
        QAction *m_pOpenAct, *m_pInsertAct;
        QAction *m_pLoadFoil, *m_pLoadPlrFile;
        QAction *m_pSaveAct, *m_pSaveProjectAsAct, *m_pNewProjectAct, *m_pCloseProjectAct;
        QAction *m_pLoadLastProjectAct;
        QAction *m_pPreferencesAct;
        QAction *m_pExitAct;
        QAction *m_pOpenGLAct;
        QVector<QAction*> m_pRecentFileActs;
        QAction *m_pSeparatorAct;
        QAction *m_pSaveViewToImageFileAct, *m_pResetSettingsAct;
        QAction *m_pShowLogWindow;
        QAction *m_pRestoreToolbarsAct;
        QAction *m_pViewLogFile, *m_pViewTraceFile;

        //Graph Actions
        QAction *m_pCurGraphDlgAct, *m_pResetCurGraphScales;
        QAction *m_pExportCurGraphDataToFile, *m_pCopyCurGraphDataAct;
        QAction *m_pExportGraphToSvgFile;
        QAction *m_pResetGraphSplitter;
        QAction *m_pShowGraphLegend;
        QAction *m_pOpenGraphInNewWindow;
        QAction *m_pFastGraphAct;


        QLabel *m_plabProjectName;

        //Script actions
        QAction *m_pExecScript;

        QStringList m_RecentFiles;

        bool m_bManualCheck;
        bool m_bSaveSettings;       /**< true if user-defined settings should be saved on exit. */

        QString m_ImageDirName;
        QString m_FilePath;         /**< The absolute path to the file of the current project. */

        QString m_GraphExportFilter;

        xfl::enumImageFormat m_ImageFormat;   /**< The index of the type of image file which should be used. */
//        QTimer *m_pSaveTimer;          /**< The timer which triggers the autosaving of the project at given intervals */

        LogMessageDlg *m_pLogMessageDlg;
        QPointer<LogWt> m_pLogWt;

        QVector<GraphWt*> m_GraphWidget;

        FastGraphWt *m_pFastGraphWt;

        QString m_ProjectName;

        QTimer m_SaveTimer;


        QThread m_FileIOThread;    /**< So that the lengthy work is moved to a thread, and the widgets remain in the main thread to keep the UI responsive */
        QThread m_ScriptThread;    /**< So that the lengthy work is moved to a thread, and the widgets remain in the main thread to keep the UI responsive */

        XflScriptExec  *m_pScriptExecutor; /**< used to run a script from the UI */

        static xfl::enumApp s_iApp;                 /**< The identification of the active module */
        static bool s_bSaved;       /**< true if the project has not been modified since the last save operation. */


        static QString s_XflProjectPath; /** the path to the xfl project files */


};

