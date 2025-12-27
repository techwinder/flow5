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

#include <QPointer>
#include <QWidget>
#include <QPushButton>
#include <QSettings>
#include <QToolBar>
#include <QMenu>
#include <QFile>

#include <core/enums_core.h>
#include <api/optstructures.h>

class Boat;
class BoatAnalysisDlg;
class BoatOpp;
class BoatPolar;
class BoatExplorer;
class Curve;
class CurveModel;
class Graph;
class MainFrame;
class SailNurbs;
class PanelAnalysisTest;
class Sail;
class Vorton;
class XSailActions;
class XSailAnalysisCtrls;
class XSailMenus;
class Opp3dScalesCtrls;
class gl3dXSailCtrls;
class gl3dXSailView;

class XSail : public QObject
{
    Q_OBJECT
    friend class BoatExplorer;
    friend class MainFrame;
    friend class XSailActions;
    friend class XSailAnalysisCtrls;
    friend class XSailLegendWt;
    friend class XSailMenus;
    friend class gl3dScales;
    friend class gl3dXSailView;
    friend class XSailDisplayCtrls;

    public:
        enum enumViews {W3DVIEW, POLARVIEW};


    public:
        XSail(MainFrame *pMainFrame);
        ~XSail();

        void keyPressEvent(QKeyEvent *pEvent);

    public:
        void cancelStreamLines();
        void setControls();
        void setGraphTiles();
        void updateObjectView();
        void updateUnits();
        void updateView();

        void setLiveVortons(double ctrlparam, std::vector<std::vector<Vorton>> const &vortons);

        BoatPolar* insertNewBtPolar(BoatPolar *pNewPolar, Boat *pCurBoat);

        Boat *setBoat(Boat *pBoat);
        Boat *setBoat(const QString &BoatName=QString());
        BoatPolar *setBtPolar(BoatPolar *pBoatPolar);
        BoatPolar *setBtPolar(QString BPlrName=QString());
        BoatOpp *setBtOpp(BoatOpp *pBoatOpp);
        BoatOpp *setBtOpp(double ctrl = 0.0);

        int plrGraphSize() const {return m_PlrGraph.count();}
        Graph *plrGraph(int igraph) {return m_PlrGraph[igraph];}

        Boat      *curBoat()    const {return m_pCurBoat;}
        BoatPolar *curBtPolar() const {return m_pCurBtPolar;}
        BoatOpp   *curBtOpp()   const {return m_pCurBtOpp;}

        void resetPrefs();

        void enableObjectView(bool bEnable);

        void loadSettings(QSettings &settings);
        void saveSettings(QSettings &settings);

        bool isAnalysisRunning() const;

        void resetCurves() {m_bResetCurves=true;}

        bool isPolarView()     const {return m_eView==POLARVIEW;}
        bool is3dView()        const {return m_eView==W3DVIEW;}


        static bool bStoreOpps3d()   {return s_bStoreOpps3d;}
        static void setStoreOpps3d(bool bStore) {s_bStoreOpps3d=bStore;}

    private:
        Boat *setModBoat(Boat *pModBoat);
        Boat *setModifiedBoat(Boat *pModBoat);
        void checkActions();
        void connectSignals();
        void createBtPolarCurves();
        void createCpCurves();
        void fillBtPlrCurve(Curve *pCurve, const BoatPolar *pBtPolar, int XVar, int YVar);
        BoatPolar *importBtPolarFromXML(QFile &xmlFile);
        Boat *importBoatFromXML(QFile &xmlFile);
        void makeControlWts();
        void makePlrgraphs();
        void outputPanelProperties(int panelindex);
        void outputNodeProperties(int nodeindex, double pickedval);
        void renameBoat(Boat *pBoat);
        void renameBtPolar(BoatPolar *pBtPolar, const Boat *pBoat);
        void cancelDisplayThreads();

        void displayStdMessage(std::string const &msg, bool bShow);
        void displayMessage(QString const &msg, bool bShow);

    signals:
        void projectModified();
        void curvesUpdated();

    public slots:
        void on3dView();
        void onAnalyze();
        void onBtOppProps();
        void onBtPolarProps();
        void onCheckPanels();
        void onConnectTriangles();
        void onCurveClicked(Curve*, int ipt);
        void onCurveDoubleClicked(Curve*);
        void onDefinePolar();
        void onDuplicateCurAnalysis();
        void onDuplicateAnalysis();
        void onDeleteBtPolarOpps();
        void onDeleteBoatBtOpps();
        void onDeleteBtPolars();
        void onDeleteCurBoat();
        void onDeleteCurBtOpp();
        void onDeleteCurBtPolar();
        void onDuplicateCurBoat();
        void onEditBtPolarPts();
        void onEditCurBoat();
        void onEditCurBtPolar();
        void onEditHull();
        void onEditSail();
        void onExportAllBtPolars();
        void onExportBtOppToClipboard();
        void onExportBtOppToFile();
        void onExportBtPolarToClipboard();
        void onExportBtPolarToFile();
        void onExportBtPolarToXML();
        void onExportToXML();
        void onGraphChanged(int);
        void onFinishAnalysis();
        void onHideAllBtPolars();
        void onHideBtOpps();
        void onHideBtPolarOpps();
        void onHideBtPolars();
        void onImportBoatFromXml();
        void onImportBtPolarFromXML();
        void onManageBoats();
        void onMeshInfo();
        void onNewBoat();
        void onOpen3dViewInNewWindow();
        void onPolarView();
        void onRenameCurBoat();
        void onRenameCurBtPolar();
        void onResetBtPolar();
        void onResetBtPolarCurves();
        void onSail();
        void onScaleHull();
        void onScaleSailShape();
        void onScaleSailSize();
        void onShowAllBtPolars();
        void onShowBtOpps();
        void onShowBtPolarOpps();
        void onShowBtPolars();
        void onShowNormals();
        void onShowOnlyBtPolarOpps();
        void onShowOnlyBtPolars();
        void onTranslateHull();
        void onTranslateSail();
        void onUpdate3dScales();
        void onUpdate3dStreamlines();
        void onUpdateWake();       
        void onVarSetChanged(int);
        void onExportSailToXML() const;
        void onExportSailToSTL() const;
        void onExportSailToStep();
        void onConvertSailToNURBS();

    private:
        MainFrame *s_pMainFrame;

        bool m_bResetCurves;

        BoatExplorer *m_pBoatExplorer;

        XSailActions *m_pActions;
        XSailMenus *m_pMenus;
        gl3dXSailView *m_pgl3dXSailView;
        XSailAnalysisCtrls *m_pAnalysisCtrls;
        gl3dXSailCtrls *m_pgl3dControls;


        QVector<Graph*> m_PlrGraph;  /**< the array of pointers to the 5 Polar graphs */
        QVector<CurveModel*> m_PlrCurveModel;

        Boat *m_pCurBoat;
        BoatPolar *m_pCurBtPolar;
        BoatOpp * m_pCurBtOpp;

        enumViews m_eView;    /**< defines the currently active view */

        double m_LastCtrl;

        BoatAnalysisDlg  *m_pBtAnalysisDlg;

        QPointer<PanelAnalysisTest> m_pPanelResultTest;


        static bool s_bStoreOpps3d;
};

