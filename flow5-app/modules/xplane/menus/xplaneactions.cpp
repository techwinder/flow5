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


#include  <QDockWidget>

#include "xplaneactions.h"

#include <globals/mainframe.h>
#include <modules/xplane/analysis/planeanalysisdlg.h>
#include <modules/xplane/controls/xplanewt.h>
#include <modules/xplane/glview/gl3dxplaneview.h>
#include <modules/xplane/xplane.h>

#include <api/enums_objects.h>
#include <api/planepolar.h>
#include <api/planexfl.h>

XPlaneActions::XPlaneActions(XPlane *pXPlane) : QObject(pXPlane)
{
    m_pXPlane = pXPlane;
    makeActions();
    makeExportActions();
}


void XPlaneActions::makeActions()
{
    m_pPOppAct = new QAction(QIcon(":/icons/OnWOppView.png"), tr("Operating point view\tF5"), m_pXPlane);
    m_pPOppAct->setShortcut(Qt::Key_F5);
    m_pPOppAct->setCheckable(true);
    m_pPOppAct->setStatusTip(tr("Switch to the operating point view"));
    connect(m_pPOppAct, SIGNAL(triggered()), m_pXPlane, SLOT(onPlaneOppView()));

    m_pPlanePolarAct = new QAction(QIcon(":/icons/OnPolarView.png"), tr("Polar view\tF8"), m_pXPlane);
    m_pPlanePolarAct->setShortcut(Qt::Key_F8);
    m_pPlanePolarAct->setCheckable(true);
    m_pPlanePolarAct->setStatusTip(tr("Switch to the polar view"));
    connect(m_pPlanePolarAct, SIGNAL(triggered()), m_pXPlane, SLOT(onPolarView()));

    m_pStabTimeAct = new QAction(QIcon(":/icons/OnStabView.png"), tr("Time response view"), m_pXPlane);
    m_pStabTimeAct->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_F8));
    m_pStabTimeAct->setCheckable(true);
    m_pStabTimeAct->setStatusTip(tr("Switch to stability analysis post-processing"));
    connect(m_pStabTimeAct, SIGNAL(triggered()), m_pXPlane, SLOT(onStabTimeView()));

    m_pRootLocusAct = new QAction(QIcon(":/icons/OnRootLocus.png"), tr("Root locus view"), m_pXPlane);
    m_pRootLocusAct->setShortcut(QKeySequence(Qt::SHIFT|Qt::Key_F8));
    m_pRootLocusAct->setCheckable(true);
    m_pRootLocusAct->setStatusTip(tr("Switch to root locus view"));
    connect(m_pRootLocusAct, SIGNAL(triggered()), m_pXPlane, SLOT(onRootLocusView()));

    m_pPlane3dAct = new QAction(QIcon(":/icons/On3DView.png"), tr("3d view\tF4"), m_pXPlane);
    m_pPlane3dAct->setShortcut(Qt::Key_F4);
    m_pPlane3dAct->setCheckable(true);
    m_pPlane3dAct->setStatusTip(tr("Switch to the 3D view"));
    connect(m_pPlane3dAct, SIGNAL(triggered()), m_pXPlane, SLOT(on3dView()));

    m_pCpViewAct = new QAction(QIcon(":/icons/OnCpView.png"), tr("Cp view\tF9"), m_pXPlane);
    m_pCpViewAct->setShortcut(Qt::Key_F9);
    m_pCpViewAct->setCheckable(true);
    m_pCpViewAct->setStatusTip(tr("Switch to the Cp view"));
    connect(m_pCpViewAct, SIGNAL(triggered()), m_pXPlane, SLOT(onCpView()));

    m_p3dLightAct = new QAction(QIcon(":/icons/light.png"), tr("Light settings"), m_pXPlane);
    m_p3dLightAct->setStatusTip(tr("<p>Define the light options in 3d views</p>"));
    m_p3dLightAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(m_p3dLightAct, SIGNAL(triggered()), m_pXPlane->m_pgl3dXPlaneView, SLOT(onSetupLight()));

    m_pResetScale = new QAction(tr("Reset view\tR"), this);
    connect(m_pResetScale, SIGNAL(triggered()), m_pXPlane->m_pgl3dXPlaneView, SLOT(on3dReset()));

    m_pDefineDefaultPlaneAct = new QAction(tr("Define a new default plane"), m_pXPlane);
    m_pDefineDefaultPlaneAct->setData(0);
    m_pDefineDefaultPlaneAct->setShortcut(Qt::Key_F3);
    connect(m_pDefineDefaultPlaneAct, SIGNAL(triggered()), m_pXPlane, SLOT(onNewPlane()));

    m_pDefineEmptyPlaneAct = new QAction(tr("Define a new empty plane"), m_pXPlane);
    m_pDefineEmptyPlaneAct->setData(1);
    m_pDefineEmptyPlaneAct->setShortcut(QKeySequence(Qt::ALT | Qt::Key_F3));
    connect(m_pDefineEmptyPlaneAct, SIGNAL(triggered()), m_pXPlane, SLOT(onNewPlane()));

    m_pSTLPlaneAct = new QAction(tr("From an STL file"), m_pXPlane);
    m_pSTLPlaneAct->setStatusTip(tr("Import a complete plane geometry from an STL file"));
    m_pSTLPlaneAct->setShortcut(QKeySequence(Qt::SHIFT|Qt::Key_F3));
    connect(m_pSTLPlaneAct, SIGNAL(triggered()), m_pXPlane, SLOT(onImportSTLPlane()));

    m_pEditPlaneAct = new QAction(tr("Edit"), m_pXPlane);
    m_pEditPlaneAct->setStatusTip(tr("Shows a form to edit the active plane"));
    m_pEditPlaneAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    connect(m_pEditPlaneAct, SIGNAL(triggered()), m_pXPlane, SLOT(onEditCurPlane()));

    m_pEditPlaneDescriptionAct = new QAction(tr("Edit description"), m_pXPlane);
    connect(m_pEditPlaneDescriptionAct, SIGNAL(triggered()), m_pXPlane, SLOT(onEditCurPlaneDescription()));

    m_pScalePlaneAct = new QAction(tr("Scale"), m_pXPlane);
    m_pScalePlaneAct->setStatusTip(tr("Shows a form to edit the active plane"));
    connect(m_pScalePlaneAct, SIGNAL(triggered()), m_pXPlane, SLOT(onScalePlane()));

    m_pTranslatePlaneAct = new QAction(tr("Translate"), m_pXPlane);
    connect(m_pTranslatePlaneAct, SIGNAL(triggered()), m_pXPlane, SLOT(onTranslatePlane()));

    m_pWingSelection = new QAction(tr("Select wing curves to display"), m_pXPlane);
    connect(m_pWingSelection, SIGNAL(triggered()), m_pXPlane, SLOT(onWingCurveSelection()));

    m_pMeshInfo = new QAction(tr("Mesh information"), m_pXPlane);
    connect(m_pMeshInfo, SIGNAL(triggered()), m_pXPlane, SLOT(onMeshInfo()));

    m_pConnectPanels = new QAction(tr("Connect triangles"), m_pXPlane);
    m_pConnectPanels->setShortcut(QKeySequence(Qt::ALT|Qt::Key_C));
    connect(m_pConnectPanels, SIGNAL(triggered()), m_pXPlane, SLOT(onConnectTriangles()));

    m_pCheckPanels = new QAction(tr("Check panels"), m_pXPlane);
    connect(m_pCheckPanels, SIGNAL(triggered()), m_pXPlane, SLOT(onCheckPanels()));

    m_pCheckFreeEdges = new QAction(tr("Check free edges"), m_pXPlane);
    m_pCheckFreeEdges->setShortcut(QKeySequence(Qt::ALT|Qt::Key_G));
    connect(m_pCheckFreeEdges, SIGNAL(triggered()), m_pXPlane, SLOT(onCheckFreeEdges()));

    m_pClearHighlightedPanels = new QAction(tr("Clear highlighted"), m_pXPlane);
    m_pClearHighlightedPanels->setShortcut(QKeySequence(Qt::ALT|Qt::Key_L));
    connect(m_pClearHighlightedPanels, SIGNAL(triggered()), m_pXPlane, SLOT(onClearHighlightSelection()));

    m_pCenterOnPanel = new QAction(tr("Center on panel"), m_pXPlane);
    m_pCenterOnPanel->setToolTip(tr("Centers the 3d view on the panel with the selected index"));
    connect(m_pCenterOnPanel, SIGNAL(triggered()), m_pXPlane, SLOT(onCenterViewOnPanel()));

    m_pShowPanelNormals = new QAction(tr("Show panel normals"), m_pXPlane);
    m_pShowPanelNormals->setCheckable(true);
    connect(m_pShowPanelNormals, SIGNAL(triggered()), m_pXPlane, SLOT(onShowPanelNormals()));

    m_pShowNodeNormals = new QAction(tr("Show node normals"), m_pXPlane);
    m_pShowNodeNormals->setCheckable(true);
    connect(m_pShowNodeNormals, SIGNAL(triggered()), m_pXPlane, SLOT(onShowNodeNormals()));

    m_pShowVortices = new QAction(tr("Show vortices and ctrl points"), m_pXPlane);
    m_pShowVortices->setCheckable(true);
    connect(m_pShowVortices, SIGNAL(triggered()), m_pXPlane, SLOT(onShowVortices()));

    m_pRenameCurPlaneAct = new QAction(tr("Rename\tF2"), m_pXPlane);
    m_pRenameCurPlaneAct->setStatusTip(tr("Rename the active object"));
    connect(m_pRenameCurPlaneAct, SIGNAL(triggered()), m_pXPlane, SLOT(onRenameCurPlane()));

    m_pExporttoAVL = new QAction(tr("to AVL"), m_pXPlane);
    m_pExporttoAVL->setStatusTip(tr("Export the active plane or wing to a text file in the format required by AVL"));
    connect(m_pExporttoAVL, SIGNAL(triggered()), m_pXPlane, SLOT(onExporttoAVL()));

    m_pExporttoSTL = new QAction(tr("tessellation to STL"), m_pXPlane);
    m_pExporttoSTL->setStatusTip(tr("Export the plane's tessellation to a file in the STL format"));
    connect(m_pExporttoSTL, SIGNAL(triggered()), m_pXPlane, SLOT(onExporttoSTL()));

    m_pExportMeshtoSTL = new QAction(tr("mesh to STL"), m_pXPlane);
    m_pExportMeshtoSTL->setStatusTip(tr("Export the plane's mesh to a file in the STL format"));
    connect(m_pExportMeshtoSTL, SIGNAL(triggered()), m_pXPlane, SLOT(onExportMeshToSTLFile()));

    m_pExportPlaneToXML = new QAction(tr("to XML"), m_pXPlane);
    connect(m_pExportPlaneToXML, SIGNAL(triggered()), m_pXPlane, SLOT(onExportPlanetoXML()));

    m_pExportCurPOpp = new QAction(tr("to a text file"), m_pXPlane);
    m_pExportCurPOpp->setStatusTip(tr("Export the current operating point to a text or csv file"));
    connect(m_pExportCurPOpp, SIGNAL(triggered()), m_pXPlane, SLOT(onExportCurPOpp()));

    m_pCopyCurPOppData = new QAction(tr("to clipboard"), m_pXPlane);
    m_pCopyCurPOppData->setShortcut(QKeySequence(Qt::ALT|Qt::SHIFT|Qt::Key_X));
    connect(m_pCopyCurPOppData, SIGNAL(triggered()), m_pXPlane, SLOT(onCopyCurPOppData()));


/*    m_pEditWingObject = new QAction("Edit (Advanced)", m_pXPlane);
    m_pEditWingObject->setStatusTip("Shows a form to edit the wing of the active plane");
    m_pEditWingObject->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_W));
    m_pEditWingObject->setData(xfl::Main);
    connect(m_pEditWingObject, SIGNAL(triggered()), m_pXPlane, SLOT(onEditCurWing()));

    m_pEditStabDef = new QAction("Edit", m_pXPlane);
    m_pEditStabObject = new QAction("Edit (Advanced)", m_pXPlane);
    m_pEditStabObject->setData(xfl::Elevator);
    m_pEditStabObject->setShortcut(Qt::CTRL | Qt::ALT | Qt::Key_E);
    connect(m_pEditStabObject, SIGNAL(triggered()), m_pXPlane, SLOT(onEditCurWing()));

    m_pEditFinObject = new QAction("Edit (Advanced)", m_pXPlane);
    m_pEditFinObject->setData(xfl::Fin);
    m_pEditFinObject->setShortcut(Qt::CTRL | Qt::ALT | Qt::Key_F);
    connect(m_pEditFinObject, SIGNAL(triggered()), m_pXPlane, SLOT(onEditCurWing()));
*/

    m_pEditWingDef = new QAction(tr("Edit"), m_pXPlane);
    m_pEditWingDef->setStatusTip(tr("Shows a form to edit the wing of the active plane"));
    m_pEditWingDef->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));
    m_pEditWingDef->setData(xfl::Main);
    connect(m_pEditWingDef, SIGNAL(triggered()), m_pXPlane, SLOT(onEditCurWing()));

    m_pEditStabDef = new QAction(tr("Edit"), m_pXPlane);
    m_pEditStabDef->setData(xfl::Elevator);
    m_pEditStabDef->setShortcut(Qt::CTRL | Qt::Key_E);
    connect(m_pEditStabDef, SIGNAL(triggered()), m_pXPlane, SLOT(onEditCurWing()));

    m_pEditFinDef = new QAction(tr("Edit"), m_pXPlane);
    m_pEditFinDef->setData(xfl::Fin);
    m_pEditFinDef->setShortcut(Qt::CTRL | Qt::Key_F);
    connect(m_pEditFinDef, SIGNAL(triggered()), m_pXPlane, SLOT(onEditCurWing()));


    m_pEditFuse = new QAction(tr("Edit"), m_pXPlane);
    m_pEditFuse->setStatusTip(tr("Shows a form to edit the body of the active plane"));
    m_pEditFuse->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    connect(m_pEditFuse, SIGNAL(triggered()), m_pXPlane, SLOT(onEditCurFuse()));

    m_pEditFuseObject = new QAction(tr("Edit (Advanced)"), m_pXPlane);
    m_pEditFuseObject->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_B));
    connect(m_pEditFuseObject, SIGNAL(triggered()), m_pXPlane, SLOT(onEditCurFuse()));

    m_pScaleWing = new QAction(tr("Scale"), m_pXPlane);
    m_pScaleWing->setStatusTip(tr("Scale the dimensions of the main wing"));
    m_pScaleWing->setData(xfl::Main);
    connect(m_pScaleWing, SIGNAL(triggered()), m_pXPlane, SLOT(onScaleWing()));

    m_pScaleStab = new QAction(tr("Scale"), m_pXPlane);
    m_pScaleStab->setStatusTip(tr("Scale the dimensions of the elevator"));
    m_pScaleStab->setData(xfl::Elevator);
    connect(m_pScaleStab, SIGNAL(triggered()), m_pXPlane, SLOT(onScaleWing()));

    m_pScaleFin = new QAction(tr("Scale"), m_pXPlane);
    m_pScaleFin->setStatusTip(tr("Scale the dimensions of the fin"));
    m_pScaleFin->setData(xfl::Fin);
    connect(m_pScaleFin, SIGNAL(triggered()), m_pXPlane, SLOT(onScaleWing()));

    m_pScaleFuse = new QAction(tr("Scale"), m_pXPlane);
    m_pScaleFuse->setStatusTip(tr("Scale the dimensions of the active fuse"));
    connect(m_pScaleFuse, SIGNAL(triggered()), m_pXPlane, SLOT(onScaleFuse()));

    m_pTranslateWing = new QAction(tr("Translate"), m_pXPlane);
    m_pTranslateWing->setData(xfl::Main);
    connect(m_pTranslateWing, SIGNAL(triggered()), m_pXPlane, SLOT(onTranslateWing()));

    m_pTranslateStab = new QAction(tr("Translate"), m_pXPlane);
    m_pTranslateStab->setData(xfl::Elevator);
    connect(m_pTranslateStab, SIGNAL(triggered()), m_pXPlane, SLOT(onTranslateWing()));

    m_pTranslateFin = new QAction(tr("Translate"), m_pXPlane);
    m_pTranslateFin->setData(xfl::Fin);
    connect(m_pTranslateFin, SIGNAL(triggered()), m_pXPlane, SLOT(onTranslateWing()));

    m_pTranslateFuse = new QAction(tr("Translate"), m_pXPlane);
    m_pTranslateFuse->setStatusTip(tr("Translate the dimensions of the active fuse"));
    connect(m_pTranslateFuse, SIGNAL(triggered()), m_pXPlane, SLOT(onTranslateFuse()));

    m_pInertiaWing = new QAction(tr("Inertia"), m_pXPlane);
    m_pInertiaWing->setData(xfl::Main);
    connect(m_pInertiaWing, SIGNAL(triggered()), m_pXPlane, SLOT(onWingInertia()));

    m_pInertiaStab= new QAction(tr("Inertia"), m_pXPlane);
    m_pInertiaStab->setData(xfl::Elevator);
    connect(m_pInertiaStab, SIGNAL(triggered()), m_pXPlane, SLOT(onWingInertia()));

    m_pInertiaFin = new QAction(tr("Inertia"), m_pXPlane);
    m_pInertiaFin->setData(xfl::Fin);
    connect(m_pInertiaFin, SIGNAL(triggered()), m_pXPlane, SLOT(onWingInertia()));

    m_pInertiaFuse = new QAction(tr("Inertia"), m_pXPlane);
    connect(m_pInertiaFuse, SIGNAL(triggered()), m_pXPlane, SLOT(onFuseInertia()));

    m_pPropsWing = new QAction(tr("Properties"), m_pXPlane);
    m_pPropsWing->setData(xfl::Main);
    connect(m_pPropsWing, SIGNAL(triggered()), m_pXPlane, SLOT(onWingProps()));

    m_pPropsStab= new QAction(tr("Properties"), m_pXPlane);
    m_pPropsStab->setData(xfl::Elevator);
    connect(m_pPropsStab, SIGNAL(triggered()), m_pXPlane, SLOT(onWingProps()));

    m_pPropsFin = new QAction(tr("Properties"), m_pXPlane);
    m_pPropsFin->setData(xfl::Fin);
    connect(m_pPropsFin, SIGNAL(triggered()), m_pXPlane, SLOT(onWingProps()));

    m_pPropsFuse = new QAction(tr("Properties"), m_pXPlane);
    connect(m_pPropsFuse, SIGNAL(triggered()), m_pXPlane, SLOT(onFuseProps()));

    m_pMakeFuseTriMesh = new QAction(tr("Fuse mesher"), m_pXPlane);
    m_pMakeFuseTriMesh->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_M));
    connect(m_pMakeFuseTriMesh, SIGNAL(triggered()), m_pXPlane, SLOT(onFuseTriMesh()));

    m_pResetFuseMesh = new QAction(tr("Reset triangular mesh"), m_pXPlane);
    m_pResetFuseMesh->setStatusTip(tr("Resets the triangular mesh based on the uncut fuse"));
    connect(m_pResetFuseMesh, SIGNAL(triggered()), m_pXPlane, SLOT(onResetFuseMesh()));

    m_pExportAllPlanePolars = new QAction(tr("Export"), m_pXPlane);
    m_pExportAllPlanePolars->setStatusTip(tr("Export all available polars to text files"));
    connect(m_pExportAllPlanePolars, SIGNAL(triggered()), m_pXPlane, SLOT(onExportAllPlPolars()));

    m_pPlaneInertia = new QAction(tr("Inertia\tF12"), m_pXPlane);
    m_pPlaneInertia->setStatusTip(tr("Define the inertia for the active plane"));
    connect(m_pPlaneInertia, SIGNAL(triggered()), m_pXPlane, SLOT(onPlaneInertia()));

    m_pShowCurPOppOnly = new QAction(tr("Show active only"), m_pXPlane);
    m_pShowCurPOppOnly->setStatusTip(tr("Hide all the curves except for the one corresponding to the active operating point"));
    m_pShowCurPOppOnly->setCheckable(true);
    connect(m_pShowCurPOppOnly, SIGNAL(triggered()), m_pXPlane, SLOT(onCurPOppOnly()));

    m_pShowAllPOpps = new QAction(tr("Show all"), m_pXPlane);
    m_pShowAllPOpps->setStatusTip(tr("Show the graph curves of all operating points"));
    connect(m_pShowAllPOpps, SIGNAL(triggered()), m_pXPlane, SLOT(onShowAllPOpps()));

    m_pHideAllPOpps = new QAction(tr("Hide all"), m_pXPlane);
    m_pHideAllPOpps->setStatusTip(tr("Hide the graph curves of all operating points"));
    connect(m_pHideAllPOpps, SIGNAL(triggered()), m_pXPlane, SLOT(onHideAllPOpps()));

    m_pDeleteAllPOpps = new QAction(tr("Delete all"), m_pXPlane);
    m_pDeleteAllPOpps->setStatusTip(tr("Delete all the operating points of all planes and polars"));
    connect(m_pDeleteAllPOpps, SIGNAL(triggered()), m_pXPlane, SLOT(onDeleteAllPOpps()));

    m_pShowPolarPOpps = new QAction(tr("Show"), m_pXPlane);
    m_pShowPolarPOpps->setStatusTip(tr("Show the curves of all the operating points of the active polar"));
    connect(m_pShowPolarPOpps, SIGNAL(triggered()), m_pXPlane, SLOT(onShowWPlrPOpps()));

    m_pHidePolarPOpps = new QAction(tr("Hide"), m_pXPlane);
    m_pHidePolarPOpps->setStatusTip(tr("Hide the curves of all the operating points of the active polar"));
    connect(m_pHidePolarPOpps, SIGNAL(triggered()), m_pXPlane, SLOT(onHideAllWPlrOpps()));

    m_pDeletePolarPOpps = new QAction(tr("Delete"), m_pXPlane);
    m_pDeletePolarPOpps->setStatusTip(tr("Delete all the operating points of the active polar"));
    connect(m_pDeletePolarPOpps, SIGNAL(triggered()), m_pXPlane, SLOT(onDeleteWPlrPOpps()));

    m_pShowPolarOppsOnly = new QAction(tr("Show only the polar's operating points"), m_pXPlane);
    connect(m_pShowPolarOppsOnly, SIGNAL(triggered()), m_pXPlane, SLOT(onShowWPolarOppsOnly()));

    m_pShowTargetCurve = new QAction(tr("Show target curve"), m_pXPlane);
    m_pShowTargetCurve->setCheckable(false);
    connect(m_pShowTargetCurve, SIGNAL(triggered()), m_pXPlane, SLOT(onShowTargetCurve()));

    m_pDefineT123578Polar = new QAction(tr("Define an analysis"), m_pXPlane);
    m_pDefineT123578Polar->setShortcut(Qt::Key_F6);
    m_pDefineT123578Polar->setStatusTip(tr("Define an analysis for the active plane"));
    connect(m_pDefineT123578Polar, SIGNAL(triggered()), m_pXPlane, SLOT(onDefineT123578Polar()));

    m_pDefineT6Polar = new QAction(tr("Define a control analysis"), m_pXPlane);
    m_pDefineT6Polar->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F6));
    m_pDefineT6Polar->setStatusTip(tr("Shows a form to edit a new control analysis"));
    connect(m_pDefineT6Polar, SIGNAL(triggered()), m_pXPlane, SLOT(onDefineT6Polar()));

    m_pDefineT7Polar = new QAction(tr("Define a stability analysis"), m_pXPlane);
    m_pDefineT7Polar->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F6));
    m_pDefineT7Polar->setStatusTip(tr("Define a stability analysis for the current wing or plane"));
    connect(m_pDefineT7Polar, SIGNAL(triggered()), m_pXPlane, SLOT(onDefineT7Polar()));

    m_pDuplicatePlanePolar = new QAction(tr("Duplicate existing analyses"), m_pXPlane);
    m_pDuplicatePlanePolar->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
    connect(m_pDuplicatePlanePolar, SIGNAL(triggered()), m_pXPlane, SLOT(onDuplicateAnalyses()));

    m_pBatchAnalysis = new QAction(tr("Batch analysis (1)"), m_pXPlane);
    m_pBatchAnalysis->setShortcut(QKeySequence(Qt::Key_F7));
    connect(m_pBatchAnalysis, SIGNAL(triggered()), m_pXPlane, SLOT(onBatchAnalysis()));

    m_pBatchAnalysisOld = new QAction(tr("Batch analysis (2)"), m_pXPlane);
    m_pBatchAnalysisOld->setShortcut(QKeySequence(Qt::ALT|Qt::Key_F7));
    connect(m_pBatchAnalysisOld, SIGNAL(triggered()), m_pXPlane, SLOT(onBatchAnalysis2()));

    m_pEditPlanePolarDef = new QAction(tr("Edit"), m_pXPlane);
    m_pEditPlanePolarDef->setStatusTip(tr("Modify the analysis parameters of this polar"));
    m_pEditPlanePolarDef->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_E));
    connect(m_pEditPlanePolarDef, SIGNAL(triggered()), m_pXPlane, SLOT(onEditCurPlPolar()));

    m_pEditExtraDrag = new QAction(tr("Edit extra drag"), m_pXPlane);
    m_pEditPlanePolarDef->setStatusTip(tr("Modify the extra drag parameters and the AVL-type parabolic drag without recalculating the polar."));
    connect(m_pEditExtraDrag, SIGNAL(triggered()), m_pXPlane, SLOT(onEditExtraDrag()));

    m_pEditPlanePolarPts = new QAction(tr("Edit data points"), m_pXPlane);
    m_pEditPlanePolarPts->setStatusTip(tr("Modify the data points of this polar"));
    connect(m_pEditPlanePolarPts, SIGNAL(triggered()), m_pXPlane, SLOT(onEditCurPlPolarPts()));

    m_pImportXmlAnalyses = new QAction(tr("Import analyses from xml"), m_pXPlane);
    m_pImportXmlAnalyses->setStatusTip(tr("Import one or more analyses defined by xml files."));
    connect(m_pImportXmlAnalyses, SIGNAL(triggered()), m_pXPlane, SLOT(onImportAnalysesFromXML()));

    m_pImportPolarData = new QAction(tr("Import external polar"), m_pXPlane);
    m_pImportPolarData->setStatusTip(tr("Create a polar and import its data from an external source."));
    connect(m_pImportPolarData, SIGNAL(triggered()), m_pXPlane, SLOT(onImportExternalPolar()));

    m_pPlanePolarNameAct = new QAction(tr("Analysis auto name options"), m_pXPlane);
    m_pPlanePolarNameAct->setStatusTip(tr("Set the default options for automatic naming of polars"));
    connect(m_pPlanePolarNameAct, SIGNAL(triggered()), m_pXPlane, SLOT(onAutoWPolarNameOptions()));

    m_pHidePlanePolars = new QAction(tr("Hide"), m_pXPlane);
    m_pHidePlanePolars->setStatusTip(tr("Hide all the polar curves associated to the active wing or plane"));
    connect(m_pHidePlanePolars, SIGNAL(triggered()), m_pXPlane, SLOT(onHidePlaneWPolars()));

    m_pShowPlanePolars = new QAction(tr("Show"), m_pXPlane);
    m_pShowPlanePolars->setStatusTip(tr("Show all the polar curves associated to the active wing or plane"));
    connect(m_pShowPlanePolars, SIGNAL(triggered()), m_pXPlane, SLOT(onShowPlaneWPolars()));

    m_pDeletePlanePolars = new QAction(tr("Delete"), m_pXPlane);
    m_pDeletePlanePolars->setStatusTip(tr("Delete all the polars associated to the active wing or plane"));
    connect(m_pDeletePlanePolars, SIGNAL(triggered()), m_pXPlane, SLOT(onDeletePlaneWPolars()));

    m_pShowPlanePolarsOnly = new QAction(tr("Show only plane polars"), m_pXPlane);
    m_pShowPlanePolarsOnly->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_U));
    connect(m_pShowPlanePolarsOnly, SIGNAL(triggered()), m_pXPlane, SLOT(onShowPlaneWPolarsOnly()));

    m_pHideAllPlanePolars = new QAction(tr("Hide"), m_pXPlane);
    m_pHideAllPlanePolars->setStatusTip(tr("Hide all the polar curves of all wings and planes"));
    connect(m_pHideAllPlanePolars, SIGNAL(triggered()), m_pXPlane, SLOT(onHideAllPlPolars()));

    m_pShowAllPlanePolars = new QAction(tr("Show"), m_pXPlane);
    m_pShowAllPlanePolars->setStatusTip(tr("<p>Show all the polar curves of all wings and planes</p>"));
    connect(m_pShowAllPlanePolars, SIGNAL(triggered()), m_pXPlane, SLOT(onShowAllPlPolars()));

    m_pHidePlanePOpps = new QAction(tr("Hide"), m_pXPlane);
    m_pHidePlanePOpps->setStatusTip(tr("<p>Hide all the operating point curves of the active wing or plane</p>"));
    connect(m_pHidePlanePOpps, SIGNAL(triggered()), m_pXPlane, SLOT(onHidePlaneOpps()));

    m_pShowPlanePOpps = new QAction(tr("Show"), m_pXPlane);
    m_pShowPlanePOpps->setStatusTip(tr("Show all the operating point curves of the active wing or plane"));
    connect(m_pShowPlanePOpps, SIGNAL(triggered()), m_pXPlane, SLOT(onShowPlaneOpps()));

    m_pDeletePlanePOpps = new QAction(tr("Delete"), m_pXPlane);
    m_pDeletePlanePOpps->setStatusTip(tr("<p>Delete all the operating points of the active wing or plane</p>"));
    connect(m_pDeletePlanePOpps, SIGNAL(triggered()), m_pXPlane, SLOT(onDeletePlanePOpps()));

    m_pDeleteCurPlane = new QAction(tr("Delete"), m_pXPlane);
//    m_pDeleteCurPlane->setShortcut(Qt::Key_Delete);
    m_pDeleteCurPlane->setStatusTip(tr("<p>Delete the active wing or plane</p>"));
    connect(m_pDeleteCurPlane, SIGNAL(triggered()), m_pXPlane, SLOT(onDeleteCurPlane()));

    m_pDuplicateCurPlane = new QAction(tr("Duplicate"), m_pXPlane);
    m_pDuplicateCurPlane->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    m_pDuplicateCurPlane->setStatusTip(tr("Duplicate the active wing or plane"));
    connect(m_pDuplicateCurPlane, SIGNAL(triggered()), m_pXPlane, SLOT(onDuplicateCurPlane()));

    m_pSavePlaneAsProjectAct = new QAction(tr("Save as project"), m_pXPlane);
    m_pSavePlaneAsProjectAct->setStatusTip(tr("Save the active wing or plane as a new separate project"));
    connect(m_pSavePlaneAsProjectAct, SIGNAL(triggered()), m_pXPlane->s_pMainFrame, SLOT(onSavePlaneAsProject()));

    m_pRenameCurPlanePolar = new QAction(tr("Rename\tShift+F2"), m_pXPlane);
    m_pRenameCurPlanePolar->setStatusTip(tr("Rename the active polar"));
    connect(m_pRenameCurPlanePolar, SIGNAL(triggered()), m_pXPlane, SLOT(onRenameCurPlPolar()));

    m_pDuplicateCurPlanePolar = new QAction(tr("Duplicate"), m_pXPlane);
    m_pDuplicateCurPlanePolar->setShortcut(QKeySequence(Qt::SHIFT|Qt::Key_D));
    connect(m_pDuplicateCurPlanePolar, SIGNAL(triggered()), m_pXPlane, SLOT(onDuplicateCurAnalysis()));

    m_pExportCurPlanePolar = new QAction(tr("to file"), m_pXPlane);
    m_pExportCurPlanePolar->setStatusTip(tr("Export the active polar to a text or csv file"));
    connect(m_pExportCurPlanePolar, SIGNAL(triggered()), m_pXPlane, SLOT(onExportWPolarToFile()));

    m_pCopyCurPlanePolarData = new QAction(tr("to clipboard"), m_pXPlane);
    m_pCopyCurPlanePolarData->setShortcut(QKeySequence(Qt::ALT|Qt::Key_X));
    connect(m_pCopyCurPlanePolarData, SIGNAL(triggered()), m_pXPlane, SLOT(onExportPlPolarToClipboard()));

    m_pResetCurPlanePolar = new QAction(tr("Reset"), m_pXPlane);
    m_pResetCurPlanePolar->setStatusTip(tr("Delete all the points of the active polar, but keep the analysis settings"));
    connect(m_pResetCurPlanePolar, SIGNAL(triggered()), m_pXPlane, SLOT(onResetCurPlPolar()));

    m_pShowOnlyCurPolar = new QAction(tr("Show only active polar"), m_pXPlane);
    m_pShowOnlyCurPolar->setShortcut(QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_U));
    connect(m_pShowOnlyCurPolar, SIGNAL(triggered()), m_pXPlane, SLOT(onShowOnlyCurPlPolar()));

    m_pDeleteCurPlanePolar = new QAction(tr("Delete"), m_pXPlane);
    m_pDeleteCurPlanePolar->setStatusTip(tr("Delete the active polar"));
    connect(m_pDeleteCurPlanePolar, SIGNAL(triggered()), m_pXPlane, SLOT(onDeleteCurPlPolar()));

    m_pDeleteCurPOpp = new QAction(tr("Delete"), m_pXPlane);
    m_pDeleteCurPOpp->setStatusTip(tr("Delete the active operating point"));
    connect(m_pDeleteCurPOpp, SIGNAL(triggered()), m_pXPlane, SLOT(onDeleteCurPOpp()));

    m_pShowAnalysisWindow = new QAction(tr("Open the analysis window"), m_pXPlane);
    m_pShowAnalysisWindow->setShortcut(QKeySequence(Qt::SHIFT|Qt::Key_A));
    connect(m_pShowAnalysisWindow, SIGNAL(triggered()), m_pXPlane, SLOT(onOpenAnalysisWindow()));

    m_pAnalysis3dSettings = new QAction(tr("3d analysis settings"), m_pXPlane);
    m_pAnalysis3dSettings->setShortcut(QKeySequence(Qt::ALT|Qt::Key_S));
    connect(m_pAnalysis3dSettings, SIGNAL(triggered()), m_pXPlane->s_pMainFrame, SLOT(on3dAnalysisSettings()));

    m_pShowPolarProps = new QAction(tr("Properties"), m_pXPlane);
    m_pShowPolarProps->setStatusTip(tr("Show the properties of the active polar"));
    m_pShowPolarProps->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Return));
    connect(m_pShowPolarProps, SIGNAL(triggered()), m_pXPlane, SLOT(onPolarProperties()));

    m_pShowPOppProps = new QAction(tr("Properties"), m_pXPlane);
    m_pShowPOppProps->setStatusTip(tr("Show the properties of the active operating point"));
    m_pShowPOppProps->setShortcut(QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Return));
    connect(m_pShowPOppProps, SIGNAL(triggered()), m_pXPlane, SLOT(onPlaneOppProperties()));

    m_pImportPlaneFromXml= new QAction(tr("From an xml file"), m_pXPlane);
    connect(m_pImportPlaneFromXml, SIGNAL(triggered()), m_pXPlane, SLOT(onImportPlanesfromXML()));

    m_pExportAnalysisToXML = new QAction(tr("Export analysis to xml file"), m_pXPlane);
    connect(m_pExportAnalysisToXML, SIGNAL(triggered()), m_pXPlane, SLOT(onExportAnalysisToXML()));

    m_pOpen3dViewInNewWindow= new QAction(tr("Open 3d view in new window"), m_pXPlane);
    m_pOpen3dViewInNewWindow->setShortcut(QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_V));
    connect(m_pOpen3dViewInNewWindow, SIGNAL(triggered()), m_pXPlane, SLOT(onOpen3dViewInNewWindow()));
}


void XPlaneActions::makeExportActions()
{
    m_pExportWingXml = new QAction(tr("to XML file"), m_pXPlane);
    m_pExportWingXml->setData(xfl::Main);
    connect(m_pExportWingXml, SIGNAL(triggered()), m_pXPlane, SLOT(onExportWingToXML()));

    m_pExportWingCAD = new QAction(tr("to STEP file"), m_pXPlane);
    m_pExportWingCAD->setShortcut(QKeySequence(Qt::ALT|Qt::Key_W));
    m_pExportWingCAD->setData(xfl::Main);
    connect(m_pExportWingCAD, SIGNAL(triggered()), m_pXPlane, SLOT(onExportWingToCAD()));

    m_pExportWingTessToStl = new QAction(tr("tessellation to STL file"), m_pXPlane);
    m_pExportWingTessToStl->setData(xfl::Main);
    connect(m_pExportWingTessToStl, SIGNAL(triggered()), m_pXPlane, SLOT(onExportWingToSTL()));

    m_pExportWingMeshToStl = new QAction(tr("mesh to STL file"), m_pXPlane);
    m_pExportWingMeshToStl->setData(xfl::Main);
    connect(m_pExportWingMeshToStl, SIGNAL(triggered()), m_pXPlane, SLOT(onExportWingMeshToSTL()));

    m_pExportStabXml = new QAction(tr("to XML file"), m_pXPlane);
    m_pExportStabXml->setData(xfl::Elevator);
    connect(m_pExportStabXml, SIGNAL(triggered()), m_pXPlane, SLOT(onExportWingToXML()));
    m_pExportStabCAD = new QAction(tr("to STEP file"), m_pXPlane);
    m_pExportStabCAD->setData(xfl::Elevator);
    connect(m_pExportStabCAD, SIGNAL(triggered()), m_pXPlane, SLOT(onExportWingToCAD()));
    m_pExportStabTessToStl = new QAction(tr("tessellation to STL file"), m_pXPlane);
    m_pExportStabTessToStl->setData(xfl::Elevator);
    connect(m_pExportStabTessToStl, SIGNAL(triggered()), m_pXPlane, SLOT(onExportWingToSTL()));
    m_pExportStabMeshToStl = new QAction(tr("mesh to STL file"), m_pXPlane);
    m_pExportStabMeshToStl->setData(xfl::Elevator);
    connect(m_pExportStabMeshToStl, SIGNAL(triggered()), m_pXPlane, SLOT(onExportWingMeshToSTL()));

    m_pExportFinXml = new QAction(tr("to XML file"), m_pXPlane);
    m_pExportFinXml->setData(xfl::Fin);
    connect(m_pExportFinXml, SIGNAL(triggered()), m_pXPlane, SLOT(onExportWingToXML()));
    m_pExportFinCAD = new QAction(tr("to STEP file"), m_pXPlane);
    m_pExportFinCAD->setData(xfl::Fin);
    connect(m_pExportFinCAD, SIGNAL(triggered()), m_pXPlane, SLOT(onExportWingToCAD()));
    m_pExportFinTessToStl = new QAction(tr("tessellation to STL file"), m_pXPlane);
    m_pExportFinTessToStl->setData(xfl::Fin);
    connect(m_pExportFinTessToStl, SIGNAL(triggered()), m_pXPlane, SLOT(onExportWingToSTL()));
    m_pExportFinMeshToStl = new QAction(tr("mesh to STL file"), m_pXPlane);
    m_pExportFinMeshToStl->setData(xfl::Fin);
    connect(m_pExportFinMeshToStl, SIGNAL(triggered()), m_pXPlane, SLOT(onExportWingMeshToSTL()));

    m_pExportFuseXml = new QAction(tr("to XML file"), m_pXPlane);
    connect(m_pExportFuseXml, SIGNAL(triggered()), m_pXPlane, SLOT(onExportFuseToXML()));
    m_pExportFuseCAD = new QAction(tr("to STEP file"), m_pXPlane);
    connect(m_pExportFuseCAD, SIGNAL(triggered()), m_pXPlane, SLOT(onExportFuseToCAD()));
    m_pExportFuseTessToStl = new QAction(tr("tessellation to STL file"), m_pXPlane);
    connect(m_pExportFuseTessToStl, SIGNAL(triggered()), m_pXPlane, SLOT(onExportFuseToSTL()));
    m_pExportFuseMeshToStl = new QAction(tr("mesh to STL file"), m_pXPlane);
    connect(m_pExportFuseMeshToStl, SIGNAL(triggered()), m_pXPlane, SLOT(onExportFuseMeshToSTL()));
}


void XPlaneActions::checkActions()
{
    m_pCpViewAct->setChecked(   m_pXPlane->isCpView());
    m_pPlane3dAct->setChecked(      m_pXPlane->is3dView());
    m_pPOppAct->setChecked(     m_pXPlane->isPOppView());
    m_pPlanePolarAct->setChecked(   m_pXPlane->isPolarView());
    m_pStabTimeAct->setChecked( m_pXPlane->isStabTimeView());
    m_pRootLocusAct->setChecked(m_pXPlane->isStabPolarView());

    m_pShowCurPOppOnly->setEnabled(m_pXPlane->isPOppView());
    m_pShowAllPOpps->setEnabled(m_pXPlane->isPOppView());
    m_pHideAllPOpps->setEnabled(m_pXPlane->isPOppView());
    m_pShowTargetCurve->setEnabled(m_pXPlane->isPOppView());

    m_pShowPolarPOpps->setEnabled(m_pXPlane->isPOppView());
    m_pHidePolarPOpps->setEnabled(m_pXPlane->isPOppView());
    m_pShowPlanePolarsOnly->setEnabled(m_pXPlane->isPolarView());
    m_pShowPlanePolars->setEnabled(m_pXPlane->isPolarView());
    m_pHidePlanePolars->setEnabled(m_pXPlane->isPolarView());
    m_pShowPlanePOpps->setEnabled(m_pXPlane->isPOppView());
    m_pHidePlanePOpps->setEnabled(m_pXPlane->isPOppView());

    m_pDefineT123578Polar->setEnabled(m_pXPlane->curPlane());
    m_pDefineT6Polar->setEnabled(m_pXPlane->curPlane());
    m_pDuplicatePlanePolar->setEnabled(m_pXPlane->curPlane());

    m_pExporttoAVL->setEnabled(m_pXPlane->curPlane() && m_pXPlane->curPlane()->isXflType());
    m_pExportPlaneToXML->setEnabled(m_pXPlane->curPlane() && m_pXPlane->curPlane()->isXflType());
    m_pExporttoSTL->setEnabled(m_pXPlane->curPlane() && m_pXPlane->curPlane()->isXflType());
    m_pExportMeshtoSTL->setEnabled(m_pXPlane->curPlane() && m_pXPlane->curPlane()->isXflType());

    m_pEditPlaneAct->setEnabled(m_pXPlane->curPlane() && !m_pXPlane->curPlane()->isLocked());

    m_pEditPlanePolarDef->setEnabled(m_pXPlane->curPlPolar() && !m_pXPlane->curPlPolar()->isExternalPolar());
    m_pEditPlanePolarPts->setEnabled(m_pXPlane->curPlane() && !m_pXPlane->curPlane()->isLocked());

    m_pShowCurPOppOnly->setChecked(m_pXPlane->m_bCurPOppOnly);

    m_pCheckFreeEdges->setEnabled(m_pXPlane->curPlPolar());
}



