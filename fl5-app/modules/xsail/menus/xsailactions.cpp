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


#define _MATH_DEFINES_DEFINED


#include <QActionGroup>


#include "xsailactions.h"
#include <modules/xsail/xsail.h>
#include <modules/xsail/analysis/boatanalysisdlg.h>
#include <modules/xsail/view/gl3dxsailview.h>
#include <globals/mainframe.h>

XSailActions::XSailActions(XSail *pXSail) : QObject()
{
    m_pXSail=pXSail;
    makeActions();
}


void XSailActions::makeActions()
{
    m_pViewActionGroup = new QActionGroup(m_pXSail);
    {
        m_p3dView= new QAction(QIcon(":/sailimages/sailboat_512.png"), "3d", m_pXSail);
        m_p3dView->setStatusTip("Switch to the 3d view");
        m_p3dView->setShortcut(Qt::Key_F4);
        m_p3dView->setCheckable(true);
        connect(m_p3dView,        SIGNAL(triggered()), m_pXSail, SLOT(on3dView()));

        m_pPolarView = new QAction(QIcon(":/icons/OnPolarView.png"), "Polars", m_pXSail);
        m_pPolarView->setStatusTip("Switch to the display of polar graphs");
        m_pPolarView->setShortcut(Qt::Key_F8);
        m_pPolarView->setCheckable(true);
        connect(m_pPolarView,     SIGNAL(triggered()), m_pXSail, SLOT(onPolarView()));

        m_pViewActionGroup->addAction(m_p3dView);
        m_pViewActionGroup->addAction(m_pPolarView);
    }

    m_p3dLightAct = new  QAction(QIcon(":/icons/light.png"), "Light settings", m_pXSail);
    m_p3dLightAct->setStatusTip("<p>Define the light options in 3d views</p>");
    m_p3dLightAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(m_p3dLightAct, SIGNAL(triggered()), m_pXSail->m_pgl3dXSailView, SLOT(onSetupLight()));

    m_pBackImageLoad = new QAction("Load", this);
    connect(m_pBackImageLoad, SIGNAL(triggered()), m_pXSail->m_pgl3dXSailView, SLOT(onLoadBackImage()));

    m_pBackImageClear = new QAction("Clear", this);
    connect(m_pBackImageClear, SIGNAL(triggered()), m_pXSail->m_pgl3dXSailView, SLOT(onClearBackImage()));

    m_pBackImageSettings = new QAction("Settings", this);
    connect(m_pBackImageSettings, SIGNAL(triggered()), m_pXSail->m_pgl3dXSailView, SLOT(onBackImageSettings()));


    m_pOpen3dViewInNewWindow= new QAction("Open 3d view in new window", m_pXSail);
    m_pOpen3dViewInNewWindow->setCheckable(true);
    m_pOpen3dViewInNewWindow->setShortcut(QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_V));
    connect(m_pOpen3dViewInNewWindow, SIGNAL(triggered()), m_pXSail, SLOT(onOpen3dViewInNewWindow()));

    m_pImportAnalysisFromXml = new QAction("Import from xml", m_pXSail);
    connect(m_pImportAnalysisFromXml, SIGNAL(triggered()), m_pXSail, SLOT(onImportBtPolarFromXML()));

    m_pShowAnalysisWindow = new QAction("Open the analysis window", m_pXSail);
    m_pShowAnalysisWindow->setShortcut(QKeySequence(Qt::SHIFT|Qt::Key_A));
    connect(m_pShowAnalysisWindow, SIGNAL(triggered()), m_pXSail->m_pBtAnalysisDlg, SLOT(show()));

    m_pAnalysis3dSettings = new QAction("3d analysis settings", m_pXSail);
    connect(m_pAnalysis3dSettings, SIGNAL(triggered()), m_pXSail->s_pMainFrame, SLOT(on3dAnalysisSettings()));

    m_pMeshInfo = new QAction("Mesh information", m_pXSail);
    connect(m_pMeshInfo, SIGNAL(triggered()), m_pXSail, SLOT(onMeshInfo()));

    m_pConnectTriangles = new QAction("Make triangle connections", m_pXSail);
    m_pConnectTriangles->setShortcut(QKeySequence(Qt::ALT|Qt::Key_C));
    connect(m_pConnectTriangles, SIGNAL(triggered()), m_pXSail, SLOT(onConnectTriangles()));

    m_pCheckPanels = new QAction("Check panels", m_pXSail);
    connect(m_pCheckPanels, SIGNAL(triggered()), m_pXSail, SLOT(onCheckPanels()));

    m_pShowNormals = new QAction("Show panel normals", m_pXSail);
    m_pShowNormals->setCheckable(true);
    connect(m_pShowNormals, SIGNAL(triggered()), m_pXSail, SLOT(onShowNormals()));

    makeSubActions();
    makeBoatActions();
    makeBtPolarActions();
    makeBtOppActions();
}

void XSailActions::makeBoatActions()
{
    m_pDefineBoatAct = new QAction("Define a new boat", m_pXSail);
    m_pDefineBoatAct->setStatusTip("Shows a dialogbox to create a new boat definition");
    m_pDefineBoatAct->setShortcut(Qt::Key_F3);
    connect(m_pDefineBoatAct, SIGNAL(triggered()), m_pXSail, SLOT(onNewBoat()));

    m_pEditBoatAct = new QAction("Edit", m_pXSail);
    m_pEditBoatAct->setStatusTip("Shows a form to edit the currently selected boat");
    m_pEditBoatAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    connect(m_pEditBoatAct,   SIGNAL(triggered()), m_pXSail, SLOT(onEditCurBoat()));

    m_pRenameCurBoatAct = new QAction("Rename", m_pXSail);
    m_pRenameCurBoatAct->setShortcut(QKeySequence(Qt::Key_F2));
    connect(m_pRenameCurBoatAct,   SIGNAL(triggered()), m_pXSail, SLOT(onRenameCurBoat()));

    m_pDuplicateCurBoat = new QAction("Duplicate", m_pXSail);
    m_pDuplicateCurBoat->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    connect(m_pDuplicateCurBoat,   SIGNAL(triggered()), m_pXSail, SLOT(onDuplicateCurBoat()));

    m_pDeleteCurBoat = new QAction("Delete", m_pXSail);
    connect(m_pDeleteCurBoat,   SIGNAL(triggered()), m_pXSail, SLOT(onDeleteCurBoat()));

    m_pSaveBoatAsProjectAct = new QAction("Save as project", m_pXSail);
    connect(m_pSaveBoatAsProjectAct,   SIGNAL(triggered()), m_pXSail->s_pMainFrame, SLOT(onSaveBoatAsProject()));

    m_pExportBoatToXML = new QAction("to XML", m_pXSail);
    connect(m_pExportBoatToXML,   SIGNAL(triggered()), m_pXSail, SLOT(onExportToXML()));

    m_pManageBoats = new QAction("Manage", m_pXSail);
    connect(m_pManageBoats,   SIGNAL(triggered()), m_pXSail, SLOT(onManageBoats()));

    m_pImportBoatFromXml = new QAction("Import from XML", m_pXSail);
    connect(m_pImportBoatFromXml,   SIGNAL(triggered()), m_pXSail, SLOT(onImportBoatFromXml()));

    m_pShowBoatBtPlrs = new QAction("Show", m_pXSail);
    connect(m_pShowBoatBtPlrs,   SIGNAL(triggered()), m_pXSail, SLOT(onShowBtPolars()));

    m_pHideBoatBtPlrs = new QAction("Hide", m_pXSail);
    connect(m_pHideBoatBtPlrs,   SIGNAL(triggered()), m_pXSail, SLOT(onHideBtPolars()));

    m_pDeleteBoatBtPlrs = new QAction("Delete", m_pXSail);
    connect(m_pDeleteBoatBtPlrs,   SIGNAL(triggered()), m_pXSail, SLOT(onDeleteBtPolars()));

    m_pShowBoatBtPolarsOnly = new QAction("Show only", m_pXSail);
    m_pShowBoatBtPolarsOnly->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_U));
    connect(m_pShowBoatBtPolarsOnly,   SIGNAL(triggered()), m_pXSail, SLOT(onShowOnlyBtPolars()));

    m_pShowBoatBtOpps = new QAction("Show", m_pXSail);
    connect(m_pShowBoatBtOpps,   SIGNAL(triggered()), m_pXSail, SLOT(onShowBtOpps()));

    m_pHideBoatBtOpps = new QAction("Hide", m_pXSail);
    connect(m_pHideBoatBtOpps,   SIGNAL(triggered()), m_pXSail, SLOT(onHideBtOpps()));

    m_pDeleteBoatBtOpps = new QAction("Delete all", m_pXSail);
    connect(m_pDeleteBoatBtOpps,   SIGNAL(triggered()), m_pXSail, SLOT(onDeleteBoatBtOpps()));
}


void XSailActions::makeBtPolarActions()
{
    m_pDefineBtPolar = new QAction("Define an analysis", m_pXSail);
    m_pDefineBtPolar->setShortcut(Qt::Key_F6);
    m_pDefineBtPolar->setStatusTip("Define an analysis for the current boat");
    connect(m_pDefineBtPolar, SIGNAL(triggered()), m_pXSail, SLOT(onDefinePolar()));

    m_pDuplicateCurBtPolar = new QAction("Duplicate", m_pXSail);
    m_pDuplicateCurBtPolar->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_D));
    connect(m_pDuplicateCurBtPolar, SIGNAL(triggered()), m_pXSail, SLOT(onDuplicateCurAnalysis()));

    m_pDuplicateBtPolar = new QAction("Duplicate an existing analysis", m_pXSail);
    m_pDuplicateBtPolar->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
    connect(m_pDuplicateBtPolar, SIGNAL(triggered()), m_pXSail, SLOT(onDuplicateAnalysis()));

    m_pEditBtPolar = new QAction("Edit", m_pXSail);
    m_pEditBtPolar->setStatusTip("Modify the analysis parameters of this polar");
    m_pEditBtPolar->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_E));
    connect(m_pEditBtPolar,   SIGNAL(triggered()), m_pXSail, SLOT(onEditCurBtPolar()));

    m_pBtPolarNameAct = new QAction("Rename current", m_pXSail);
    m_pBtPolarNameAct->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F2));
    connect(m_pBtPolarNameAct,   SIGNAL(triggered()), m_pXSail, SLOT(onRenameCurBtPolar()));

    m_pRenameCurBtPolar = new QAction("Rename", m_pXSail);
    connect(m_pRenameCurBtPolar, SIGNAL(triggered()), m_pXSail, SLOT(onRenameCurBtPolar()));

    m_pShowAllBtPlrs = new QAction("Show", m_pXSail);
    connect(m_pShowAllBtPlrs,   SIGNAL(triggered()), m_pXSail, SLOT(onShowAllBtPolars()));

    m_pHideAllBtPlrs = new QAction("Hide", m_pXSail);
    connect(m_pHideAllBtPlrs,   SIGNAL(triggered()), m_pXSail, SLOT(onHideAllBtPolars()));

    m_pExportBtPolars = new QAction("Export", m_pXSail);
    connect(m_pExportBtPolars,   SIGNAL(triggered()), m_pXSail, SLOT(onExportAllBtPolars()));

    m_pEditBtPolarPts = new QAction("Edit points", m_pXSail);
    connect(m_pEditBtPolarPts,   SIGNAL(triggered()), m_pXSail, SLOT(onEditBtPolarPts()));

    m_pDeleteCurBtPolar = new QAction("Delete", m_pXSail);
    connect(m_pDeleteCurBtPolar,   SIGNAL(triggered()), m_pXSail, SLOT(onDeleteCurBtPolar()));

    m_pResetCurBtPolar = new QAction("Reset", m_pXSail);
    connect(m_pResetCurBtPolar,   SIGNAL(triggered()), m_pXSail, SLOT(onResetBtPolar()));

    m_pShowAllBtPlrOpps = new QAction("Show", m_pXSail);
    connect(m_pShowAllBtPlrOpps,   SIGNAL(triggered()), m_pXSail, SLOT(onShowBtPolarOpps()));

    m_pHideAllBtPlrOpps = new QAction("Hide", m_pXSail);
    connect(m_pHideAllBtPlrOpps,   SIGNAL(triggered()), m_pXSail, SLOT(onHideBtPolarOpps()));

    m_pDeleteAllBtPolarOpps = new QAction("Delete all", m_pXSail);
    connect(m_pDeleteAllBtPolarOpps,   SIGNAL(triggered()), m_pXSail, SLOT(onDeleteBtPolarOpps()));

    m_pShowBtPlrOppsOnly = new QAction("Show only", m_pXSail);
    connect(m_pShowBtPlrOppsOnly,   SIGNAL(triggered()), m_pXSail, SLOT(onShowOnlyBtPolarOpps()));

    m_pExportCurBtPolar = new QAction("to file", m_pXSail);
    connect(m_pExportCurBtPolar,   SIGNAL(triggered()), m_pXSail, SLOT(onExportBtPolarToFile()));

    m_pCopyCurBtPolarData = new QAction("to clipboard", m_pXSail);
    connect(m_pCopyCurBtPolarData,   SIGNAL(triggered()), m_pXSail, SLOT(onExportBtPolarToClipboard()));

    m_pExportAnalysisToXML = new QAction("Export analysis to xml file", m_pXSail);
    connect(m_pExportAnalysisToXML,   SIGNAL(triggered()), m_pXSail, SLOT(onExportBtPolarToXML()));

    m_pShowBtPolarProps = new QAction("Properties", m_pXSail);
    connect(m_pShowBtPolarProps,   SIGNAL(triggered()), m_pXSail, SLOT(onBtPolarProps()));
}


void XSailActions::makeBtOppActions()
{
    m_pShowBoatBtOppProps = new QAction("Properties", m_pXSail);
    connect(m_pShowBoatBtOppProps,   SIGNAL(triggered()), m_pXSail, SLOT(onBtOppProps()));

    m_pExportCurBtOpp = new QAction("to file", m_pXSail);
    connect(m_pExportCurBtOpp,   SIGNAL(triggered()), m_pXSail, SLOT(onExportBtOppToFile()));

    m_pCopyCurBtOppData = new QAction("to clipboard", m_pXSail);
    connect(m_pCopyCurBtOppData,   SIGNAL(triggered()), m_pXSail, SLOT(onExportBtOppToClipboard()));

    m_pDeleteCurBtOpp = new QAction("Delete", m_pXSail);
    connect(m_pDeleteCurBtOpp,   SIGNAL(triggered()), m_pXSail, SLOT(onDeleteCurBtOpp()));
}


void XSailActions::makeSubActions()
{
    m_pEditMainSail = new QAction("Edit", m_pXSail);
    m_pEditMainSail->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_M));
    connect(m_pEditMainSail, SIGNAL(triggered(bool)), m_pXSail, SLOT(onEditSail()));

    m_pScaleMainShape = new QAction("Scale shape", m_pXSail);
    m_pScaleMainShape->setShortcut(Qt::Key_F10);
    connect(m_pScaleMainShape, SIGNAL(triggered(bool)), m_pXSail, SLOT(onScaleSailShape()));

    m_pScaleMainSize = new QAction("Scale size", m_pXSail);
    m_pScaleMainSize->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_F10));
    connect(m_pScaleMainSize, SIGNAL(triggered(bool)), m_pXSail, SLOT(onScaleSailSize()));

    m_pTranslateMainSail = new QAction("Translate", m_pXSail);
    connect(m_pTranslateMainSail, SIGNAL(triggered(bool)), m_pXSail, SLOT(onTranslateSail()));

    m_pExportMainSailToXml = new QAction("to XML", m_pXSail);
    connect(m_pExportMainSailToXml,   SIGNAL(triggered()), m_pXSail, SLOT(onExportSailToXML()));

    m_pExportMainSailToSTL = new QAction("mesh to STL", m_pXSail);
    connect(m_pExportMainSailToSTL,   SIGNAL(triggered()), m_pXSail, SLOT(onExportSailToSTL()));

    m_pExportMainSailToStep = new QAction("to STEP", m_pXSail);
    connect(m_pExportMainSailToStep,   SIGNAL(triggered()), m_pXSail, SLOT(onExportSailToStep()));

    m_pEditJib = new QAction("Edit", m_pXSail);
    m_pEditJib->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_J));
    connect(m_pEditJib, SIGNAL(triggered(bool)), m_pXSail, SLOT(onEditSail()));

    m_pScaleJibShape = new QAction("Scale", m_pXSail);
    connect(m_pScaleJibShape, SIGNAL(triggered(bool)), m_pXSail, SLOT(onScaleSailShape()));

    m_pScaleJibSize = new QAction("Scale size", m_pXSail);
    connect(m_pScaleJibSize, SIGNAL(triggered(bool)), m_pXSail, SLOT(onScaleSailSize()));

    m_pTranslateJib = new QAction("Translate", m_pXSail);
    connect(m_pTranslateJib, SIGNAL(triggered(bool)), m_pXSail, SLOT(onTranslateSail()));

    m_pExportJibToXml = new QAction("to XML", m_pXSail);
    connect(m_pExportJibToXml,   SIGNAL(triggered()), m_pXSail, SLOT(onExportSailToXML()));

    m_pExportJibToSTL = new QAction("mesh to STL", m_pXSail);
    connect(m_pExportJibToSTL,   SIGNAL(triggered()), m_pXSail, SLOT(onExportSailToSTL()));

    m_pExportJibToStep = new QAction("to STEP", m_pXSail);
    connect(m_pExportJibToStep,   SIGNAL(triggered()), m_pXSail, SLOT(onExportSailToStep()));

    m_pConvertMainSailToNURBS = new QAction("Convert to NURBS type", m_pXSail);
    connect(m_pConvertMainSailToNURBS,   SIGNAL(triggered()), m_pXSail, SLOT(onConvertSailToNURBS()));

    m_pConvertJibToNURBS = new QAction("Convert to NURBS type", m_pXSail);
    connect(m_pConvertJibToNURBS,   SIGNAL(triggered()), m_pXSail, SLOT(onConvertSailToNURBS()));

    m_pEditHull = new QAction("Edit", m_pXSail);
    m_pEditHull->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
    connect(m_pEditHull, SIGNAL(triggered(bool)), m_pXSail, SLOT(onEditHull()));

    m_pScaleHull = new QAction("Scale", m_pXSail);
    connect(m_pScaleHull, SIGNAL(triggered(bool)), m_pXSail, SLOT(onScaleHull()));

    m_pTranslateHull = new QAction("Translate", m_pXSail);
    connect(m_pTranslateHull, SIGNAL(triggered(bool)), m_pXSail, SLOT(onTranslateHull()));
}


void XSailActions::checkActions()
{

}
