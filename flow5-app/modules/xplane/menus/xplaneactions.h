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

#include <QMenu>
#include <QAction>

#include <QObject>

class XPlane;
class XPlaneActions : public QObject
{
    friend class XPlane;
    friend class XPlaneWt;
    friend class MainFrame;
    friend class XPlaneMenus;

    Q_OBJECT

    public:
        XPlaneActions(XPlane *pXPlane);

        void makeActions();
        void checkActions();

        void makeExportActions();

    private:
        XPlane *m_pXPlane;

        //XPlane Actions

        QAction *m_pPlanePolarAct, *m_pPOppAct, *m_pPlane3dAct, *m_pCpViewAct, *m_pStabTimeAct, *m_pRootLocusAct;
        QAction *m_p3dLightAct, *m_pResetScale;
        QAction *m_pDefineDefaultPlaneAct, *m_pDefineEmptyPlaneAct, *m_pSTLPlaneAct;
        QAction *m_pSavePlaneAsProjectAct, *m_pRenameCurPlaneAct, *m_pDeleteCurPlane, *m_pDuplicateCurPlane;
        QAction *m_pEditPlanePolarPts, *m_pExportCurPlanePolar, *m_pCopyCurPlanePolarData, *m_pResetCurPlanePolar;
        QAction *m_pShowOnlyCurPolar;
        QAction *m_pEditPlanePolarDef, *m_pEditExtraDrag;
        QAction *m_pShowPolarProps, *m_pShowPOppProps;
        QAction *m_pRenameCurPlanePolar, *m_pDeleteCurPlanePolar, *m_pDuplicateCurPlanePolar;
        QAction *m_pDeleteCurPOpp;
        QAction *m_pWingSelection;
        QAction *m_pMeshInfo, *m_pConnectPanels, *m_pCheckPanels, *m_pClearHighlightedPanels, *m_pCheckFreeEdges;
        QAction *m_pCenterOnPanel;
        QAction *m_pShowPanelNormals, *m_pShowNodeNormals;
        QAction *m_pShowVortices;


        QAction *m_pImportPlaneFromXml, *m_pExportPlaneToXML, *m_pExportAnalysisToXML;
        QAction *m_pImportXmlAnalyses, *m_pImportPolarData;

        QAction *m_pHideAllPlanePolars, *m_pShowAllPlanePolars;
        QAction *m_pHidePlanePolars, *m_pShowPlanePolars, *m_pShowPlanePolarsOnly, *m_pDeletePlanePolars;
        QAction *m_pHidePlanePOpps, *m_pShowPlanePOpps, *m_pDeletePlanePOpps;
        QAction *m_pExportCurPOpp, *m_pCopyCurPOppData;
        QAction *m_pShowCurPOppOnly, *m_pHideAllPOpps, *m_pShowAllPOpps, *m_pDeleteAllPOpps;
        QAction *m_pShowPolarPOpps, *m_pHidePolarPOpps, * m_pDeletePolarPOpps, *m_pShowPolarOppsOnly;
        QAction *m_pDefineT123578Polar, *m_pDefineT6Polar, *m_pDefineT7Polar;
        QAction *m_pDuplicatePlanePolar;
        QAction *m_pBatchAnalysis, *m_pBatchAnalysisOld;
        QAction *m_pPlanePolarNameAct;

        QAction *m_pShowAnalysisWindow, *m_pAnalysis3dSettings;
        QAction *m_pShowTargetCurve;
        QAction *m_pExporttoAVL;
        QAction *m_pExporttoSTL, *m_pExportMeshtoSTL;
        QAction *m_pExportAllPlanePolars, *m_pPlaneInertia;

        QAction *m_pEditPlaneAct, *m_pEditPlaneDescriptionAct;
        QAction *m_pScalePlaneAct, *m_pTranslatePlaneAct;
//        QAction *m_pEditWingObject, *m_pEditStabObject, *m_pEditFinObject;
        QAction *m_pEditWingDef, *m_pEditStabDef, *m_pEditFinDef;
        QAction *m_pExportWingXml, *m_pExportWingCAD, *m_pExportWingTessToStl, *m_pExportWingMeshToStl;
        QAction *m_pExportStabXml, *m_pExportStabCAD, *m_pExportStabTessToStl, *m_pExportStabMeshToStl;
        QAction *m_pExportFinXml,  *m_pExportFinCAD,  *m_pExportFinTessToStl,  *m_pExportFinMeshToStl;
        QAction *m_pEditFuse, *m_pEditFuseObject;
        QAction *m_pExportFuseXml,  *m_pExportFuseCAD, *m_pExportFuseTessToStl, *m_pExportFuseMeshToStl;
        QAction *m_pScaleWing, *m_pScaleStab, *m_pScaleFin,*m_pScaleFuse;
        QAction *m_pTranslateWing, *m_pTranslateStab, *m_pTranslateFin,*m_pTranslateFuse;
        QAction *m_pPropsWing, *m_pPropsStab, *m_pPropsFin,*m_pPropsFuse;
        QAction *m_pInertiaWing, *m_pInertiaStab, *m_pInertiaFin, *m_pInertiaFuse;
        QAction *m_pMakeFuseTriMesh, *m_pResetFuseMesh;
        QAction *m_pOpen3dViewInNewWindow;

};

