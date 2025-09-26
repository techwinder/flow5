/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QFile>
#include <QSettings>
#include <QTextStream>
#include <QKeyEvent>
#include <QPixmap>
#include <QFrame>
#include <QPointer>


#include <xflcore/enums_core.h>
#include <xflcore/linestyle.h>
#include <xflobjects/objects_globals/optstructures.h>
#include <xflpanels/panels/vorton.h>

class Analysis3dCtrls;
class CpViewWt;
class Curve;
class CurveModel;
class Graph;
class LLTAnalysisDlg;
class MainFrame;
class PanelAnalysisTest;
class Plane;
class PlaneAnalysisDlg;
class POpp3dCtrls;
class POppGraphCtrls;
class PlaneOpp;
class PlaneTreeView;
class PlaneXfl;
class Stab3dCtrls;
class StabTimeCtrls;
class WPolar;
class WingXfl;
class WingOpp;
class WingWt;
class XPlaneActions;
class XPlaneMenus;
class XPlaneWt;
class gl3dXPlaneView;
class Node;


/**
 *@class XPlane
 *@brief This is the class associated to the 3d calculations

  The marshalling yard for object definition, analysis and post-processing.
  The main methods of this class are:
    - Management of wings, planes, polars, and operating points
    - Construction of the panels for 3D calculations
    - Management of the display
    - Management of the Panel Analysis,
    - Mapping of the analyis results to the operating point and polar objects
*/
class XPlane : public QObject
{
    friend class Analysis3dCtrls;
    friend class CpViewWt;
    friend class MainFrame;
    friend class POppGraphCtrls;
    friend class POpp3dCtrls;
    friend class PlaneTreeView;
    friend class Stab3dCtrls;
    friend class StabTimeCtrls;
    friend class StabTimeTiles;
    friend class WingWt;
    friend class XPlaneActions;
    friend class XPlaneGraphCtrls;
    friend class XPlaneLegendWt;
    friend class XPlaneMenus;
    friend class XPlaneWt;
    friend class gl3dScales;
    friend class gl3dView;
    friend class gl3dXPlaneView;
    friend class StabTimeDlg;

    Q_OBJECT

    public:

        enum enumViews {WOPPVIEW, WPOLARVIEW, CPVIEW, W3DVIEW, STABTIMEVIEW, STABPOLARVIEW, OTHERVIEW};

    public:

        XPlane(MainFrame *pMainFrame);
        ~XPlane();

    private:
        void keyPressEvent(QKeyEvent *event);
        void keyReleaseEvent(QKeyEvent *pEvent);

    public:
        //class methods
        WPolar* addWPolar(WPolar* pWPolar);
        void cancelStreamLines();
        void checkActions();
        void connectSignals();
        void createWPolarCurves();
        void createWOppCurves();
        void createStabilityCurves();
        void displayMessage(QString const &msg, bool bShowWindow, bool bStatusBar, int duration=5000);
        void fillWPlrCurve(Curve *pCurve, WPolar const*pWPolar, int XVar, int YVar);
        void fillStabCurve(Curve *pCurve, WPolar const*pWPolar, int iMode);
        void fillWOppCurve(WPolar const *pWPolar, const PlaneOpp *pPOpp, int iw, int iVar, Curve *pCurve);
        PlaneXfl *importPlaneFromXML(QFile &xmlFile);
        WPolar *importAnalysisFromXML(QFile &xmlFile);
        bool loadSettings(QSettings &settings);
        void makeLegend();
        void outputNodeProperties(int nodeindex, double pickedval);
        void outputPanelProperties(int panelindex);
        QString planeOppData();
        void resetPrefs();
        bool saveSettings(QSettings &settings);
        void selectWOppCurves();
        void setAnalysisParams();
        void setControls(bool bVisibilityOnly=false);
        void setGraphTiles();

        PlaneOpp* setPlaneOpp(PlaneOpp *pPOpp);
        Plane *setPlane(Plane *pPlane);
        Plane *setPlane(QString PlaneName=QString());
        Plane *setModPlane(Plane *pModPlane, bool bUsed, bool bAsNew);
        void setStabTimeYVariables(bool bLong);
        void setView(xfl::enumGraphView eView);
        void setWPolar(const QString &WPlrName);
        void setWPolar(WPolar *pWPolar=nullptr);
        void stopAnimate();
        void stopAnimateMode();
        PlaneOpp *storePOpps(const QList<PlaneOpp *> &POppList);
        void updateTreeView();
        void createCpCurves();
        void updateUnits();
        void updateView();

        void resetCurves() {m_bResetCurves = true;}
        void resetGraphScales();

        Plane * curPlane()  const {return m_pCurPlane;}
        WPolar *curWPolar() const {return m_pCurWPolar;}
        PlaneOpp const *curPOpp() const {return m_pCurPOpp;}

        bool curPOppOnly() const {return m_bCurPOppOnly;}

        bool isPOppView()      const {return m_eView==XPlane::WOPPVIEW;}
        bool isPolarView()     const {return m_eView==XPlane::WPOLARVIEW;}
        bool isCpView()        const {return m_eView==XPlane::CPVIEW;}
        bool is3dView()        const {return m_eView==XPlane::W3DVIEW;}
        bool isStabPolarView() const {return m_eView==XPlane::STABPOLARVIEW;}
        bool isStabTimeView()  const {return m_eView==XPlane::STABTIMEVIEW;}
        bool isStabilityView() const {return isStabPolarView() || isStabTimeView();}

        bool isOneGraphView() const;
        bool isTwoGraphsView() const;
        bool isFourGraphsView() const;
        bool isAllGraphsView() const;

        bool isAnalysisRunning() const;

        void updateStabilityDirection(bool bLongitudinal);

        void updateVisiblePanels(Plane const *pPlane=nullptr, WPolar const*pWPolar=nullptr);

        void setLiveVortons(double alpha, const QVector<QVector<Vorton> > &vortons);

        Plane * duplicatePlane(Plane *pPlane) const;


    public slots:
        QString onExportWPolarToClipboard();

        void on3dView();
        void onAddCpSectionCurve();
        void onAdjustWingGraphToWingSpan();
        void onAnalyze();
        void onAnimateModeSingle(bool bStep=true);
        void onAutoWPolarNameOptions();
        void onBatchAnalysis();
        void onBatchAnalysis2();
        void onCenterViewOnPanel();
        void onCheckFreeEdges();
        void onCheckPanels();
        void onClearHighlightSelection();
        void onConnectTriangles();
        void onCopyCurPOppData();
        void onCpView();
        void onCurPOppOnly();
        void onCurveClicked(Curve*pCurve, int ipt);
        void onCurveDoubleClicked(Curve*pCurve);
        void onDefineT6Polar();
        void onDefineT7Polar();
        void onDefineT123578Polar();
        void onDeleteAllPOpps();
        void onDeleteWPlrPOpps();
        void onDeleteCurPOpp();
        void onDeleteCurPlane();
        void onDeleteCurWPolar();
        void onDeletePlanePOpps();
        void onDeletePlaneWPolars();
        void onDuplicateCurPlane();
        void onDuplicateCurAnalysis();
        void onDuplicateAnalyses();
        void onEditCurFuse();
        void onEditCurPlane();
        void onEditCurPlaneDescription();
        void onEditCurWPolar();
        void onEditCurWPolarPts();
        void onEditCurWing();
        void onEditExtraDrag();
        void onExportAllWPolars();
        void onExportAnalysisToXML();
        void onExportCurPOpp();
        void onExportFuseMeshToSTL();
        void onExportFuseToCAD();
        void onExportFuseToSTL();
        void onExportFuseToXML();
        void onExportMeshToSTLFile();
        void onExportPlanetoXML();
        void onExportWPolarToFile();
        void onExportWingMeshToSTL();
        void onExportWingToCAD();
        void onExportWingToSTL();
        void onExportWingToXML();
        void onExporttoAVL();
        void onExporttoSTL();
        void onFinishAnalysis(WPolar *pWPolar);
        void onFuseInertia();
        void onFuseProps();
        void onFuseTriMesh();
        void onGraphChanged(int);
        void onHideAllPOpps();
        void onHideAllWPlrOpps();
        void onHideAllWPolars();
        void onHidePlaneOpps();
        void onHidePlaneWPolars();
        void onImportAnalysesFromXML();
        void onImportExternalPolar();
        void onImportPlanesfromXML();
        void onImportSTLPlane();
        void onMeshInfo();
        void onModalView();
        void onNewPlane();
        void onNodeDistance();
        void onOpen3dViewInNewWindow();
        void onOpenAnalysisWindow();
        void onOptim3d();
        void onPickedNode(int iNode);
        void onPickedNodePair(QPair<int, int> nodepair);
        void onPlaneInertia();
        void onPlaneOppProperties();
        void onPlaneOppView();
        void onRenameCurPlane();
        void onRenameCurWPolar();
        void onResetCurWPolar();
        void onResetFuseMesh();
        void onResetWPolarCurves();
        void onRootLocusView();
        void onScalePlane();
        void onScaleFuse();
        void onScaleWing();
        void onShowAllPOpps();
        void onShowWPlrPOpps();
        void onShowAllWPolars();
        void onShowNodeNormals();
        void onShowOnlyCurWPolar();
        void onShowPanelNormals();
        void onShowPlaneOpps();
        void onShowPlaneWPolars();
        void onShowPlaneWPolarsOnly();
        void onShowTargetCurve();
        void onShowVortices();
        void onShowWPolarOppsOnly();
        void onStabTimeView();
        void onStyleChanged();
        void onTranslateFuse();
        void onTranslatePlane();
        void onTranslateWing();
        void onUpdateMeshDisplay();
        void onVarSetChanged(int);
        void onWPolarProperties();
        void onWPolarView();
        void onWingCurveSelection();
        void onWingInertia();
        void onWingProps();

    signals:
        void projectModified();
        void curvesUpdated();

        //____________________Variables______________________________________
        //

    private:
        MainFrame *s_pMainFrame;       /**< a pointer to the frame class */

        PlaneAnalysisDlg *m_pPanelAnalysisDlg;        /**< the dialog class which manages the 3d VLM and Panel calculations */
        LLTAnalysisDlg *m_pLLTAnalysisDlg;
        PlaneTreeView *m_pPlaneTreeView;

        QPointer<XPlaneWt> m_pXPlaneWt;

        gl3dXPlaneView *m_pgl3dXPlaneView;              /**< a pointer to the widget for 3d rendering */

        Analysis3dCtrls *m_pAnalysisControls;

        POpp3dCtrls *m_pPOpp3dCtrls;
        POppGraphCtrls *m_pPOppGraphCtrls;
        StabTimeCtrls *m_pStabTimeControls;

        XPlaneActions *m_pActions;
        XPlaneMenus *m_pMenus;

        bool m_bCurPOppOnly;                /**< true if only the current WOpp is to be displayed */
        bool m_bShowEllipticCurve;          /**< true if the elliptic loading should be displayed in the local lift graph */
        bool m_bShowBellCurve;              /**< true if the bell distribution loading should be displayed in the local lift graph */

        Plane *m_pCurPlane;          /**< a pointer to the active Plane >*/
        WPolar *m_pCurWPolar;        /**< a pointer to the active WPolar >*/
        PlaneOpp *m_pCurPOpp;        /**< a pointer to the active Plane Operating Point, or NULL if none is active; const because only the style can be changed >*/


        QTimer *m_pTimerMode;         /**< A pointer to the timer which signals the animation of the modes in the 3d view */

        enumViews m_eView;    /**< defines the currently active view */


        double m_BellCurveExp;
        bool m_bMaxCL;
        LineStyle m_TargetCurveStyle;

        QVector<Graph*> m_WingGraph;             /**< the array of pointer to the OpPoint graphs */
        QVector<Graph*> m_WPlrGraph;             /**< the array of pointers to the WPolar graphs */
        QVector<Graph*> m_StabPlrGraph;          /**< the array of pointers to the two root locus graphs */
        QVector<Graph*> m_TimeGraph;             /**< the array of pointers to the time response graphs in stability view */

        QVector<CurveModel*> m_WingCurveModel;
        QVector<CurveModel*> m_WPlrCurveModel;
        QVector<CurveModel*> m_StabPlrCurveModel;
        QVector<CurveModel*> m_TimeCurveModel;


        QPointer<PanelAnalysisTest> m_pPanelResultTest;

        bool m_bResetCurves;               /**< true if the curves of the active view should be regenerated before the next view update >*/



        static QVector<OptObjective> s_Objectives;
};



