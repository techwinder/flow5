/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QAction>

class XDirect;
class MainFrame;

class XDirectActions : QObject
{
    Q_OBJECT
    friend class XDirectMenus;
    friend class XDirect;
    friend class MainFrame;
    friend class FoilTableView;

    public:
        XDirectActions(MainFrame *pMainFrame, XDirect *pXDirect);
        void makeActions();

    private:
        QAction *m_pDesignAct, *m_pBLAct, *m_pPolarsAct, *m_pOpPointsAct;
        QAction *m_pDefinePolarAct;
        QAction *m_pEditCurPolar, *m_pEditCurPolarPts;
        QAction *m_pMultiThreadedBatchAct;
        QAction *m_pExportCurPolar, *m_pDeletePolar, *m_pResetCurPolar, *m_pCopyCurPolarData;
        QAction *m_pHideFoilPolars, *m_pShowFoilPolars, *m_pShowFoilPolarsOnly, *m_pSaveFoilPolars,*m_pDeleteFoilPolars;
        QAction *m_pShowAllPolars, *m_pHideAllPolars;
        QAction *m_pScanPolarDir, *m_pExportAllPolars;
        QAction *m_pShowAllOpPoints, *m_pHideAllOpPoints, *m_pExportPolarOpps;
        QAction *m_pHideFoilOpps, *m_pShowFoilOpps, *m_pDeleteFoilOpps;
        QAction *m_pHidePolarOpps, *m_pShowPolarOpps, *m_pDeletePolarOpps;
        QAction *m_pExportCurOpp, *m_pCopyCurOppData, *m_pDeleteCurOpp;
        QAction *m_pGetFoilProps, *m_pGetPolarProps, *m_pGetOppProps;
        QAction *m_pShowNeutralLine, *m_pResetFoilScale, *m_pShowInviscidCurve;
        QAction *m_pExportFoilCpGraphAct, *m_pExportToClipBoard;
        QAction *m_pExportCurFoilDat, *m_pExportCurFoilSVG;
        QAction *m_pDeleteCurFoil, *m_pRenameCurFoil, *m_pDuplicateCurFoil, *m_pFoilDescription;
        QAction *m_pDerotateFoil;
        QAction *m_pRefineGlobalFoil;
        QAction *m_pEditCoordsFoil, *m_pScaleFoil;
        QAction *m_pSetTEGap, *m_pSetLERadius, *m_pSetFlap;
        QAction *m_pInterpolateFoils, *m_pNacaFoils, *m_pFoilFromCoords;
        QAction *m_pFoilFrom1Spline,  *m_pFoilFrom2Splines, *m_pFoilFromCamber;

        QAction *m_pCircleFoil, *m_pSquareFoil;

        QAction *m_pSetQVarGraph, *m_pSetCpVarGraph, *m_pShowCpLegend;
        QAction *m_pRenamePolarAct;
        QAction *m_pDuplicatePolars;
        QAction *m_pImportJavaFoilPolar, *m_pImportXFoilPolar;
        QAction *m_pImportXMLFoilAnalysis, *m_pExportXMLFoilAnalysis;
        QAction *m_pXFoilSettings;
        QAction *m_pShowAnalysisWindow;
        QAction *m_pOptimize;

        // dfoil view actions

        QAction *m_pHideAllFoils, *m_pShowAllFoils, *m_pShowActiveFoilOnly, *m_pShowActivePolarOnly;
        QAction *m_pShowLEPosition, *m_pShowLECircle, *m_pShowLegend;
        QAction *m_pFillFoil, *m_pShowTEHinge;

        XDirect *m_pXDirect;
        MainFrame *m_pMainFrame;
};

