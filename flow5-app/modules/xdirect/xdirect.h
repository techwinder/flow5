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

#include <QKeyEvent>
#include <QSettings>

#include <core/enums_core.h>
#include <api/linestyle.h>

class MainFrame;
class Analysis2dCtrls;
class OpPointCtrls;
class OpPointWt;
class Graph;
class CurveModel;
class Curve;
class Foil;
class Polar;
class OpPoint;
class FoilAnalysisDlg;
class XFoilAnalysisDlg;
class FoilExplorer;
class DFoilWt;
class FoilTable;
class XDirectActions;
class XDirectMenus;


class XDirect : public QObject
{
    friend class BLGraphCtrls;
    friend class BLGraphTiles;
    friend class DFoilLegendWt;
    friend class FoilTable;
    friend class FoilExplorer;
    friend class MainFrame;
    friend class OpPointWt;
    friend class PlrGraphTiles;
    friend class ProfileTiles;
    friend class XDirectActions;
    friend class XDirectLegendWt;
    friend class XDirectMenus;
    friend class XDirectTileWt;

    Q_OBJECT

    public:
    enum enumViews {DESIGNVIEW, OPPVIEW, POLARVIEW, BLVIEW};

    public:
        XDirect(MainFrame *pMainFrame);
        ~XDirect();

        //    void customEvent(QEvent * pEvent);
        void keyPressEvent(QKeyEvent *pEvent);

        void showDockWidgets();
        void setControls();
        void connectSignals();
        void createCurves();
        void createBLCurves();
        void createOppCurves();
        void createPolarCurves();

        void fillCurve(Curve *pCurve, const Polar *pPolar, int XVar, int YVar, OpPoint const *pHighlightOpp) const;

        void importAnalysisFromXML(QFile &xmlFile);

        void loadSettings(QSettings &settings);
        Foil *addNewFoil(Foil *pFoil);
        QString oppData(const OpPoint *pOpp) const;
        bool renameFoil(Foil *pFoil);
        void resetCurves() {m_bResetCurves=true;}
        void resetPrefs();
        void saveSettings(QSettings &settings);
        void setGraphTiles();
        void setBLView(xfl::enumGraphView eView) {m_iBLView=eView;}
        void stopAnimate();
        void updateFoilExplorers();

        Foil* setFoil(Foil* pFoil=nullptr);
        Polar *setPolar(Polar *pPolar=nullptr);
        void setOpp(OpPoint *pOpPoint);
        OpPoint *setOpp(double OpParameter=-123456789.0);

        XDirect::enumViews eView() const {return m_eView;}
        bool isDesignView()  const {return m_eView==XDirect::DESIGNVIEW;}
        bool isBLView()      const {return m_eView==XDirect::BLVIEW;}
        bool isOppView()     const {return m_eView==XDirect::OPPVIEW;}
        bool isPolarView()   const {return m_eView==XDirect::POLARVIEW;}
        void setDesignView()  {m_eView=XDirect::DESIGNVIEW;}
        void setBLView()      {m_eView=XDirect::BLVIEW;}
        void setOppView()     {m_eView=XDirect::OPPVIEW;}
        void setPolarView()   {m_eView=XDirect::POLARVIEW;}

        int plrGraphSize() const {return m_PlrGraph.count();}

        Graph *CpGraph() {return m_pOppGraph;}
        Graph *BLGraph(int iBLGraph)  {return m_BLGraph.at(iBLGraph);}
        Graph *PlrGraph(int iPlrGraph) {return m_PlrGraph.at(iPlrGraph);}


        bool isOneGraphView() const;
        bool isTwoGraphsView() const;
        bool isFourGraphsView() const;
        bool isAllGraphsView() const;

        xfl::enumGraphView blView() const {return m_iBLView;}

        void writeFoilPolars(QDataStream &ar, Foil *pFoil);
        Polar *insertNewPolar(Polar *pNewPolar);

        bool bShowInviscid() const {return m_bShowInviscid;}

        static bool bAlpha()   {return s_bAlpha;}
        static void setAoAAnalysis(bool b) {s_bAlpha=b;}

        static bool bStoreOpps() {return s_bStoreOpp;}
        static void setStoreOpps(bool b) {s_bStoreOpp=b;}


        static bool bKeepOpenOnErrors() {return s_bKeepOpenErrors;}
        static void setKeepOpenOnErrors(bool bKeepOpen) {s_bKeepOpenErrors=bKeepOpen;}

        static bool curOppOnly() {return s_bCurOppOnly;}

        static void setCurFoil( Foil*pFoil)     {s_pCurFoil=pFoil;}
        static void setCurPolar(Polar*pPolar)   {s_pCurPolar=pPolar;}
        static void setCurOpp(  OpPoint* pOpp)  {s_pCurOpp=pOpp;}

        static Foil *   curFoil()  {return s_pCurFoil;}
        static Polar*   curPolar() {return s_pCurPolar;}
        static OpPoint* curOpp()   {return s_pCurOpp;}

    private:

        void makeOppGraph();
        void makeBLGraphs();
        void makePolarGraphs();
        void fillBLXFoilCurves(OpPoint *pOpp, Graph *pGraph, bool bInviscid=false);
        void fillOppCurves(OpPoint *pOpp, Curve *pViscCurve, Curve *pInviscidCurve, int YVar);

    signals:
        void projectModified();
        void curvesUpdated();

    public slots:
        void onAnalyze();
        void updateView();

    private slots:

        QString onCopyCurOppData();
        QString onCopyCurPolarData();

        void onAFoilLECircle();
        void onShowLEPosition(bool bShow);
        void onAnalysisSettings();
        void onAutoPolarNameOptions();
        void onBLView();
        void onCircleFoil();
        void onCpGraph();
        void onCpi(bool bInviscid);
        void onCurOppOnly(bool bCurOppOnly=false);
        void onCurveClicked(Curve*pCurve);
        void onCurveDoubleClicked(Curve*pCurve);
        void onDefineAnalysis();
        void onDeleteCurFoil();
        void onDeleteCurOpp();
        void onDeleteCurPolar();
        void onDeleteFoilOpps();
        void onDeleteFoilPolars();
        void onDeletePolarOpps();
        void onDerotateFoil();
        void onDesignView();
        void onDuplicateFoil();
        void onEditCurPolar();
        void onEditCurPolarPts();
        void onExportAllPolars();
        void onExportCpGraph();
        void onExportCurFoilToDat();
        void onExportCurFoilToSVG();
        void onExportCurOpp();
        void onExportCurPolar();
        void onExportPolarOpps() ;
        void onExportXMLAnalysis();
        void onFillFoil(bool);
        void onFinishAnalysis();
        void onFoilCoordinates();
        void onFoilFrom1Spline();
        void onFoilFrom2Splines();
        void onFoilFromCamber();
        void onFoilFromCoords();
        void onFoilProps();
        void onFoilScale();
        void onGraphChanged(int);
        void onHideAllFoils();
        void onHideAllOpps();
        void onHideAllPolars();
        void onHideFoilOpps();
        void onHideFoilPolars();
        void onHidePolarOpps();
        void onImportJavaFoilPolar();
        void onImportXFoilPolars();
        void onImportXMLAnalysis();
        void onInterpolateFoils();
        void onBatchAltAnalysis();
        void onBatchAnalysis();
        void onNacaFoils();
        void onOpenAnalysisWindow();
        void onDuplicateAnalyses();
        void onOpPointGraphChanged();
        void onOpPointProps();
        void onOpPointView();
        void onOppGraph();
        void onPolarProps();
        void onPolarView();
        void onRefineGlobally();
        void onRenameCurFoil();
        void onRenameCurPolar();
        void onResetAllPolarGraphsScales();
        void onResetCurPolar();
        void onResetPolarCurve();
        void onSavePolars();
        void onScanPolarFiles();
        void onSetFlap();
        void onSetFoilDescription();
        void onSetLERadius();
        void onSetTEGap();
        void onShowActiveFoilOnly();
        void onShowActivePolarOnly();
        void onShowAllFoils();
        void onShowAllOpps();
        void onShowAllPolars();
        void onShowCpLegend();
        void onShowFoilOpps();
        void onShowFoilPolars();
        void onShowFoilPolarsOnly();
        void onShowLegend(bool bShow);
        void onShowPolarOpps();
        void onShowTEHinge(bool bShow);
        void onSquareFoil();
        void onStyleChanged();
        void onVarSetChanged(int index);

    private:

        XFoilAnalysisDlg* m_pXFADlg;
        FoilExplorer *m_pFoilExplorer;

        FoilTable *m_pFoilTable;

        OpPointWt *m_pOpPointWt;

        enumViews m_eView;   /**< the currently selected view*/

        bool m_bTrans;             /**< true if the user is dragging a view */
        bool m_bShowInviscid;      /**< true if the inviscid results should be displayed */
        bool m_bResetCurves;       /**< true if the graph curves need to be redrawn before the next view update */

        int m_iActiveGraph;

        int m_iPlrGraph;           /**< defines which polar graph is selected if m_iPlrView=1 */
        xfl::enumGraphView m_iBLView;  /**< defines the number of graphs to be displayed in the BL view */

        QVector<Graph*> m_BLGraph; /**< the array of pointers to the 5 Operating point graphs */
        QVector<CurveModel*> m_BLCurveModel;

        QVector<Graph*> m_PlrGraph;  /**< the array of pointers to the 5 Polar graphs */
        QVector<CurveModel*> m_PlrCurveModel;

        Graph *m_pOppGraph;
        CurveModel *m_pOppCurveModel;

        Analysis2dCtrls *m_pAnalysisControls;
        OpPointCtrls *m_pOpPointControls;

        DFoilWt *m_pDFoilWt;

        XDirectActions *m_pActions;
        XDirectMenus *m_pMenus;

        MainFrame *s_pMainFrame;  /**< a static pointer to the instance of the application's MainFrame object */

        static bool s_bBLTopSide;  /** true if the top side is to be displayed in the BL graphs, false otherwise */
        static bool s_bBLBotSide;  /** true if the top side is to be displayed in the BL graphs, false otherwise */
        static LineStyle s_lsTopBL;
        static LineStyle s_lsBotBL;

        static bool s_bShowCenterlines;
        static bool s_bCurOppOnly;        /**< true if only the current operating point should be displayed */

        static bool s_bKeepOpenErrors; /**< true if the XfoilAnalysisDlg should be kept open if errors occured in the XFoil calculation */

        static bool s_bAlpha;
        static bool s_bStoreOpp;

        static Foil* s_pCurFoil;
        static Polar* s_pCurPolar;
        static OpPoint* s_pCurOpp;

};


