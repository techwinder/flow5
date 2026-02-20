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

#include <QActionGroup>

#include "xdirectactions.h"
#include <modules/xdirect/xdirect.h>
#include <globals/mainframe.h>

#include <interfaces/graphs/graph/graph.h>


XDirectActions::XDirectActions(MainFrame *pMainFrame, XDirect *pXDirect)
{
    m_pMainFrame = pMainFrame;
    m_pXDirect = pXDirect;
    makeActions();
}


void XDirectActions::makeActions()
{
    QActionGroup *pViewActionGroup = new QActionGroup(m_pMainFrame);
    {
        m_pDesignAct = new QAction(QIcon(":/icons/OnDFoil.png"), tr("Foil design view\tF3"), this);
        m_pDesignAct->setShortcut(Qt::Key_F3);
        m_pDesignAct->setCheckable(true);
        m_pDesignAct->setToolTip(tr("<p>Switch to the foil design view</p>"));
        connect(m_pDesignAct, SIGNAL(triggered()), m_pXDirect, SLOT(onDesignView()));

        m_pBLAct = new QAction(QIcon(":/icons/OnBLView.png"), tr("Boundary layer view\tF4"), this);
        m_pBLAct->setShortcut(Qt::Key_F4);
        m_pBLAct->setCheckable(true);
        m_pBLAct->setToolTip(tr("Switch to the boundary layer view"));
        connect(m_pBLAct, SIGNAL(triggered()), m_pXDirect, SLOT(onBLView()));

        m_pOpPointsAct = new QAction(QIcon(":/icons/OnCpView.png"), tr("Operating point view\tF5"), this);
        m_pOpPointsAct->setShortcut(Qt::Key_F5);
        m_pOpPointsAct->setCheckable(true);
        m_pOpPointsAct->setToolTip(tr("<p>Switch to the operating point view</p>"));
        connect(m_pOpPointsAct, SIGNAL(triggered()), m_pXDirect, SLOT(onOpPointView()));

        m_pPolarsAct = new QAction(QIcon(":/icons/OnPolarView.png"), tr("Polar view\tF8"), this);
        m_pPolarsAct->setShortcut(Qt::Key_F8);
        m_pPolarsAct->setCheckable(true);
        m_pPolarsAct->setToolTip(tr("<p>Switch to the polar view<p>"));
        connect(m_pPolarsAct, SIGNAL(triggered()), m_pXDirect, SLOT(onPolarView()));

        pViewActionGroup->addAction(m_pDesignAct);
        pViewActionGroup->addAction(m_pBLAct);
        pViewActionGroup->addAction(m_pOpPointsAct);
        pViewActionGroup->addAction(m_pPolarsAct);
    }


    m_pDeleteCurFoil = new QAction(tr("Delete"), this);
    m_pDeleteCurFoil->setShortcut(Qt::Key_Delete);
    connect(m_pDeleteCurFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onDeleteCurFoil()));

    m_pRenameCurFoil = new QAction(tr("Rename\tF2"), this);
    connect(m_pRenameCurFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onRenameCurFoil()));

    m_pFoilDescription = new QAction(tr("Edit description"), this);
    connect(m_pFoilDescription, SIGNAL(triggered()), m_pXDirect, SLOT(onSetFoilDescription()));

    m_pExportCurFoilDat = new QAction(tr("to .dat file"), this);
    connect(m_pExportCurFoilDat, SIGNAL(triggered()), m_pXDirect, SLOT(onExportCurFoilToDat()));

    m_pExportCurFoilSVG = new QAction(tr("to SVG"), this);
    connect(m_pExportCurFoilSVG, SIGNAL(triggered()), m_pXDirect, SLOT(onExportCurFoilToSVG()));

    m_pDuplicateCurFoil = new QAction(tr("Duplicate"), this);
    m_pDuplicateCurFoil->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    connect(m_pDuplicateCurFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onDuplicateFoil()));

    //    m_pSetCurFoilStyle = new QAction("Set Style"), this);
    //    connect(m_pSetCurFoilStyle, SIGNAL(triggered()), this, SLOT(onCurFoilStyle()));

    m_pDeleteFoilPolars = new QAction(tr("Delete"), this);
    m_pDeleteFoilPolars->setToolTip(tr("<p>Delete all the polars associated to this foil</p>"));
    connect(m_pDeleteFoilPolars, SIGNAL(triggered()), m_pXDirect, SLOT(onDeleteFoilPolars()));

    m_pShowFoilPolarsOnly = new QAction(tr("Show only foil polars"), this);
    m_pShowFoilPolarsOnly->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_U));
    connect(m_pShowFoilPolarsOnly, SIGNAL(triggered()), m_pXDirect, SLOT(onShowFoilPolarsOnly()));

    m_pShowFoilPolars = new QAction(tr("Show"), this);
    connect(m_pShowFoilPolars, SIGNAL(triggered()), m_pXDirect, SLOT(onShowFoilPolars()));

    m_pHideFoilPolars = new QAction(tr("Hide"), this);
    connect(m_pHideFoilPolars, SIGNAL(triggered()), m_pXDirect, SLOT(onHideFoilPolars()));

    m_pSaveFoilPolars = new QAction(tr("Save"), this);
    connect(m_pSaveFoilPolars, SIGNAL(triggered()), m_pXDirect, SLOT(onSavePolars()));

    m_pHidePolarOpps = new QAction(tr("Hide"), this);
    connect(m_pHidePolarOpps, SIGNAL(triggered()), m_pXDirect, SLOT(onHidePolarOpps()));

    m_pShowPolarOpps = new QAction(tr("Show"), this);
    connect(m_pShowPolarOpps, SIGNAL(triggered()), m_pXDirect, SLOT(onShowPolarOpps()));

    m_pDeletePolarOpps = new QAction(tr("Delete"), this);
    connect(m_pDeletePolarOpps, SIGNAL(triggered()), m_pXDirect, SLOT(onDeletePolarOpps()));

    m_pExportPolarOpps = new QAction(tr("Export"), this);
    connect(m_pExportPolarOpps, SIGNAL(triggered()), m_pXDirect, SLOT(onExportPolarOpps()));

    m_pHideFoilOpps = new QAction(tr("Hide"), this);
    connect(m_pHideFoilOpps, SIGNAL(triggered()), m_pXDirect, SLOT(onHideFoilOpps()));

    m_pShowFoilOpps = new QAction(tr("Show"), this);
    connect(m_pShowFoilOpps, SIGNAL(triggered()), m_pXDirect, SLOT(onShowFoilOpps()));

    m_pDeleteFoilOpps = new QAction(tr("Delete"), this);
    connect(m_pDeleteFoilOpps, SIGNAL(triggered()), m_pXDirect, SLOT(onDeleteFoilOpps()));

    m_pDefinePolarAct = new QAction(tr("Define an analysis"), this);
    m_pDefinePolarAct->setShortcut(Qt::Key_F6);
    m_pDefinePolarAct->setToolTip(tr("<p>Define a single analysis/polar</p>"));
    connect(m_pDefinePolarAct, SIGNAL(triggered()), m_pXDirect, SLOT(onDefineAnalysis()));

    m_pBatchXFoilAct = new QAction(tr("Batch analysis (legacy)"), this);
    m_pBatchXFoilAct->setShortcut(QKeySequence(Qt::ALT | Qt::Key_F7));
    m_pBatchXFoilAct->setToolTip(tr("<p>Launches a batch of analyses using all available computer CPU cores</p>"));
    connect(m_pBatchXFoilAct, SIGNAL(triggered()), m_pXDirect, SLOT(onBatchAnalysis()));

    m_pBatchAltAct = new QAction(tr("Batch analysis (new)"), this);
    m_pBatchAltAct->setShortcut(QKeySequence(Qt::Key_F7));
    m_pBatchAltAct->setToolTip(tr("<p>Launches a batch of selected analyses</p>"));
    connect(m_pBatchAltAct, SIGNAL(triggered()), m_pXDirect, SLOT(onBatchAltAnalysis()));

    m_pDeletePolar = new QAction(tr("Delete"), this);
    m_pDeletePolar->setToolTip(tr("Deletes the currently selected polar"));
    connect(m_pDeletePolar, SIGNAL(triggered()), m_pXDirect, SLOT(onDeleteCurPolar()));

    m_pResetCurPolar = new QAction(tr("Reset"), this);
    m_pResetCurPolar->setToolTip(tr("<p>Deletes the contents of the currently selected polar</p>"));
    connect(m_pResetCurPolar, SIGNAL(triggered()), m_pXDirect, SLOT(onResetCurPolar()));

    m_pEditCurPolar = new QAction(tr("Edit"), this);
    m_pEditCurPolar->setToolTip(tr("<p>Edit and modify the analysis parameters</p>"));
    connect(m_pEditCurPolar, SIGNAL(triggered()), m_pXDirect, SLOT(onEditCurPolar()));

    m_pEditCurPolarPts = new QAction(tr("Edit data points"), this);
    m_pEditCurPolarPts->setToolTip(tr("<p>Remove the unconverged or erroneous points of the currently selected polar</p>"));
    connect(m_pEditCurPolarPts, SIGNAL(triggered()), m_pXDirect, SLOT(onEditCurPolarPts()));

    m_pExportCurPolar = new QAction(tr("to file"), this);
    connect(m_pExportCurPolar, SIGNAL(triggered()), m_pXDirect, SLOT(onExportCurPolar()));

    m_pCopyCurPolarData = new QAction(tr("to clipboard"), this);
    m_pCopyCurPolarData->setShortcut(QKeySequence::Copy);
    connect(m_pCopyCurPolarData, SIGNAL(triggered()), m_pXDirect, SLOT(onCopyCurPolarData()));

    m_pExportAllPolars = new QAction(tr("Export all polars"), this);
    connect(m_pExportAllPolars, SIGNAL(triggered()), m_pXDirect, SLOT(onExportAllPolars()));

    m_pScanPolarDir = new QAction(tr("Scan *.plr files"), this);
    connect(m_pScanPolarDir, SIGNAL(triggered()), m_pXDirect, SLOT(onScanPolarFiles()));

    m_pResetFoilScale = new QAction(tr("Reset foil scale"), this);
    m_pResetFoilScale->setToolTip(tr("<p>Resets the foil's scale to original size</p>"));

    m_pRenamePolarAct = new QAction(tr("Rename"), this);
    connect(m_pRenamePolarAct, SIGNAL(triggered()), m_pXDirect, SLOT(onRenameCurPolar()));

    m_pShowInviscidCurve = new QAction(tr("Show inviscid curve"), this);
    m_pShowInviscidCurve->setCheckable(true);
    m_pShowInviscidCurve->setToolTip(tr("<p>Display the operating point's inviscid curve</p>"));
    connect(m_pShowInviscidCurve, SIGNAL(triggered(bool)), m_pXDirect, SLOT(onCpi(bool)));

    m_pExportFoilCpGraphAct = new QAction(tr("to file"), this);
    connect(m_pExportFoilCpGraphAct, SIGNAL(triggered(bool)), m_pXDirect, SLOT(onExportCpGraph()));

    m_pExportToClipBoard = new QAction(tr("to clipboard"), this);
    connect(m_pExportToClipBoard, SIGNAL(triggered(bool)), m_pMainFrame, SLOT(onCopyCurGraphData()));

    m_pShowAllPolars = new QAction(tr("Show all"), this);
    connect(m_pShowAllPolars, SIGNAL(triggered()), m_pXDirect, SLOT(onShowAllPolars()));

    m_pHideAllPolars = new QAction(tr("Hide all"), this);
    connect(m_pHideAllPolars, SIGNAL(triggered()), m_pXDirect, SLOT(onHideAllPolars()));

    m_pShowAllOpPoints = new QAction(tr("Show all"), this);
    connect(m_pShowAllOpPoints, SIGNAL(triggered()), m_pXDirect, SLOT(onShowAllOpps()));

    m_pHideAllOpPoints = new QAction(tr("Hide all"), this);
    connect(m_pHideAllOpPoints, SIGNAL(triggered()), m_pXDirect, SLOT(onHideAllOpps()));

    m_pExportCurOpp = new QAction(tr("to file"), this);
    connect(m_pExportCurOpp, SIGNAL(triggered()), m_pXDirect, SLOT(onExportCurOpp()));

    m_pCopyCurOppData = new QAction(tr("to clipboard"), this);
    m_pCopyCurOppData->setShortcut(QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_C));
    connect(m_pCopyCurOppData, SIGNAL(triggered()), m_pXDirect, SLOT(onCopyCurOppData()));

    m_pDeleteCurOpp = new QAction(tr("Delete"), this);
    connect(m_pDeleteCurOpp, SIGNAL(triggered()), m_pXDirect, SLOT(onDeleteCurOpp()));

    m_pGetFoilProps = new QAction(tr("Properties"), this);
    connect(m_pGetFoilProps, SIGNAL(triggered()), m_pXDirect, SLOT(onFoilProps()));

    m_pGetPolarProps = new QAction(tr("Properties"), this);
    connect(m_pGetPolarProps, SIGNAL(triggered()), m_pXDirect, SLOT(onPolarProps()));

    m_pGetOppProps = new QAction(tr("Properties"), this);
    connect(m_pGetOppProps, SIGNAL(triggered()), m_pXDirect, SLOT(onOpPointProps()));

    m_pDerotateFoil = new QAction(tr("De-rotate and normalize"), this);
    m_pDerotateFoil->setShortcut(QKeySequence(Qt::ALT | Qt::Key_D));
    connect(m_pDerotateFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onDerotateFoil()));

//    m_pNormalizeFoil = new QAction("Normalize", this);
//    connect(m_pNormalizeFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onNormalizeFoil()));

    m_pRefineGlobalFoil = new QAction(tr("Refine globally"), this);
    m_pRefineGlobalFoil->setShortcut(Qt::Key_F9);
    connect(m_pRefineGlobalFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onRefineGlobally()));

    m_pEditCoordsFoil = new QAction(tr("Coordinates"), this);
    connect(m_pEditCoordsFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onFoilCoordinates()));

    m_pScaleFoil = new QAction(tr("Scale camber and thickness"), this);
    m_pScaleFoil->setShortcut(QKeySequence(Qt::Key_F12));
    connect(m_pScaleFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onFoilScale()));

    m_pSetTEGap = new QAction(tr("Set T.E. gap"), this);
    m_pSetTEGap->setShortcut(QKeySequence(Qt::ALT | Qt::Key_G));
    connect(m_pSetTEGap, SIGNAL(triggered()), m_pXDirect, SLOT(onSetTEGap()));

    m_pSetLERadius = new QAction(tr("Set L.E. radius"), this);
    connect(m_pSetLERadius, SIGNAL(triggered()), m_pXDirect, SLOT(onSetLERadius()));

    m_pSetFlap = new QAction(tr("Set flap"), this);
    m_pSetFlap->setShortcut(QKeySequence(Qt::Key_F10));
    connect(m_pSetFlap, SIGNAL(triggered()), m_pXDirect, SLOT(onSetFlap()));

    m_pInterpolateFoils = new QAction(tr("Interpolate foils"), this);
    m_pInterpolateFoils->setShortcut(QKeySequence(Qt::ALT | Qt::Key_I));
    connect(m_pInterpolateFoils, SIGNAL(triggered()), m_pXDirect, SLOT(onInterpolateFoils()));

    m_pNacaFoils = new QAction(tr("NACA foils"), this);
    m_pNacaFoils->setShortcut(QKeySequence(Qt::ALT | Qt::Key_N));
    connect(m_pNacaFoils, SIGNAL(triggered()), m_pXDirect, SLOT(onNacaFoils()));

    m_pCircleFoil = new QAction(tr("Circular foil"), this);
    connect(m_pCircleFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onCircleFoil()));

    m_pSquareFoil = new QAction(tr("Square foil"), this);
    connect(m_pSquareFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onSquareFoil()));

    m_pFoilFromCoords = new QAction(tr("From coordinates"), this);
    m_pFoilFromCoords->setShortcut(QKeySequence(Qt::ALT | Qt::Key_C));
    connect(m_pFoilFromCoords, SIGNAL(triggered()), m_pXDirect, SLOT(onFoilFromCoords()));

    m_pFoilFrom1Spline = new QAction(tr("From spline"), this);
    m_pFoilFrom1Spline->setShortcut(QKeySequence(Qt::ALT | Qt::Key_1));
    connect(m_pFoilFrom1Spline, SIGNAL(triggered()), m_pXDirect, SLOT(onFoilFrom1Spline()));

    m_pFoilFrom2Splines = new QAction(tr("From 2 splines"), this);
    connect(m_pFoilFrom2Splines, SIGNAL(triggered()), m_pXDirect, SLOT(onFoilFrom2Splines()));

    m_pFoilFromCamber = new QAction(tr("From camber and thickness"), this);
    m_pFoilFromCamber->setShortcut(QKeySequence(Qt::ALT | Qt::Key_3));
    connect(m_pFoilFromCamber, SIGNAL(triggered()), m_pXDirect, SLOT(onFoilFromCamber()));

    m_pDuplicatePolars = new QAction(tr("Duplicate existing analyses"), m_pXDirect);
    m_pDuplicatePolars->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
    connect(m_pDuplicatePolars, SIGNAL(triggered()), m_pXDirect, SLOT(onDuplicateAnalyses()));

    m_pImportXFoilPolar = new QAction(tr("Import XFoil polar(s)"), this);
    connect(m_pImportXFoilPolar, SIGNAL(triggered()), m_pXDirect, SLOT(onImportXFoilPolars()));

    m_pImportXMLFoilAnalysis = new QAction(tr("Import analysis from xml file"), this);
    connect(m_pImportXMLFoilAnalysis, SIGNAL(triggered()), m_pXDirect, SLOT(onImportXMLAnalysis()));

    m_pExportXMLFoilAnalysis = new QAction(tr("Export analysis to xml file"), this);
    connect(m_pExportXMLFoilAnalysis, SIGNAL(triggered()), m_pXDirect, SLOT(onExportXMLAnalysis()));

    m_pXFoilSettings = new QAction(tr("XFoil settings"), this);
    connect(m_pXFoilSettings, SIGNAL(triggered()), m_pXDirect, SLOT(onAnalysisSettings()));

    m_pShowAnalysisWindow = new QAction(tr("Open the analysis window"), m_pXDirect);
    m_pShowAnalysisWindow->setShortcut(QKeySequence(Qt::SHIFT|Qt::Key_A));
    connect(m_pShowAnalysisWindow, SIGNAL(triggered()), m_pXDirect, SLOT(onOpenAnalysisWindow()));

    m_pShowCpLegend = new QAction(tr("Show legend"), this);
    m_pShowCpLegend->setCheckable(true);
    connect(m_pShowCpLegend, SIGNAL(triggered(bool)), m_pXDirect, SLOT(onShowCpLegend()));

    m_pSetCpVarGraph = new QAction(tr("Cp"), this);
    m_pSetCpVarGraph->setCheckable(true);
    m_pSetCpVarGraph->setToolTip(tr("<p>Sets Cp vs. chord graph</p>"));
    connect(m_pSetCpVarGraph, SIGNAL(triggered()), m_pXDirect, SLOT(onCpGraph()));

    m_pSetQVarGraph = new QAction(tr("Velocity"), this);
    m_pSetQVarGraph->setCheckable(true);
    m_pSetQVarGraph->setToolTip(tr("<p>Sets Speed vs. chord graph</p>"));
    connect(m_pSetQVarGraph, SIGNAL(triggered()), m_pXDirect, SLOT(onOppGraph()));

    // design view actions
    m_pShowAllFoils= new QAction(tr("Show all foils"), this);
    m_pShowAllFoils->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));
    connect(m_pShowAllFoils, SIGNAL(triggered()), m_pXDirect, SLOT(onShowAllFoils()));

    m_pHideAllFoils= new QAction(tr("Hide all foils"), this);
    m_pHideAllFoils->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
    connect(m_pHideAllFoils, SIGNAL(triggered()), m_pXDirect, SLOT(onHideAllFoils()));

    m_pShowActiveFoilOnly = new QAction(tr("Show only active foil"), this);
    m_pShowActiveFoilOnly->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U));
    connect(m_pShowActiveFoilOnly, SIGNAL(triggered()), m_pXDirect, SLOT(onShowActiveFoilOnly()));

    m_pShowActivePolarOnly = new QAction(tr("Show only active polar"), m_pXDirect);
    m_pShowActivePolarOnly->setShortcut(QKeySequence(Qt::CTRL|Qt::ALT|Qt::Key_U));
    connect(m_pShowActivePolarOnly, SIGNAL(triggered()), m_pXDirect, SLOT(onShowActivePolarOnly()));

    m_pFillFoil = new QAction(QIcon(":/icons/fillfoil.png"),tr("Fill selected"), this);
    m_pFillFoil->setToolTip(tr("<p>Fill the interior of the active foil</p>"));
    m_pFillFoil->setCheckable(true);
    connect(m_pFillFoil, SIGNAL(triggered(bool)), m_pXDirect, SLOT(onFillFoil(bool)));

    m_pShowTEHinge = new QAction(QIcon(":/icons/TEHinge.png"), tr("Show T.E. hinge"), this);
    m_pShowTEHinge->setToolTip(tr("<p>Show the position of the trailing edge flap's hinge</p>"));
    m_pShowTEHinge->setCheckable(true);
    connect(m_pShowTEHinge, SIGNAL(triggered(bool)), m_pXDirect, SLOT(onShowTEHinge(bool)));

    m_pShowLEPosition = new QAction(QIcon(":/icons/showLE.png"), tr("Show L.E. position"), this);
    m_pShowLEPosition->setToolTip(tr("<p>Show the position of the leading edge</p>"));
    m_pShowLEPosition->setCheckable(true);
    connect(m_pShowLEPosition, SIGNAL(triggered(bool)), m_pXDirect, SLOT(onShowLEPosition(bool)));

    m_pShowLECircle = new QAction(tr("Show L.E. circle"), this);
    connect(m_pShowLECircle, SIGNAL(triggered(bool)), m_pXDirect, SLOT(onAFoilLECircle()));

    m_pShowLegend = new QAction(tr("Show legend"), this);
    m_pShowLegend->setCheckable(true);
    connect(m_pShowLegend, SIGNAL(triggered(bool)), m_pXDirect, SLOT(onShowLegend(bool)));
}


