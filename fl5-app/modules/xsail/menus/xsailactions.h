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

#include <QAction>

class XSail;

class XSailActions : QObject
{
    Q_OBJECT
    friend class MainFrame;
    friend class XSail;
    friend class XSailMenus;

public:
    XSailActions(XSail *pXSail);


private:
    void checkActions();
    void makeActions();
    void makeBoatActions();
    void makeBtPolarActions();
    void makeBtOppActions();
    void makeSubActions();

private:

    //XSail actions
    QActionGroup * m_pViewActionGroup;
    QAction *m_p3dView, *m_pPolarView;

    //3d view actions
    QAction *m_p3dLightAct, *m_pOpen3dViewInNewWindow;


    // boat actions
    QAction *m_pDefineBoatAct, *m_pEditBoatAct, *m_pOptimizeBoatAct;
    QAction *m_pRenameCurBoatAct, *m_pDuplicateCurBoat, *m_pDeleteCurBoat, *m_pSaveBoatAsProjectAct;
    QAction *m_pExportMainSailToXml, *m_pExportJibToXml;
    QAction *m_pExportMainSailToSTL, *m_pExportJibToSTL;
    QAction *m_pExportMainSailToStep, *m_pExportJibToStep;
    QAction *m_pExportBoatToXML, *m_pExportBoatToSTL;
    QAction *m_pShowBoatBtPlrs, *m_pHideBoatBtPlrs, *m_pDeleteBoatBtPlrs, *m_pShowBoatBtPolarsOnly;
    QAction *m_pShowBoatBtOpps, *m_pHideBoatBtOpps, *m_pDeleteBoatBtOpps;

    QAction *m_pEditMainSail, *m_pScaleMainShape, *m_pScaleMainSize, *m_pTranslateMainSail;
    QAction *m_pEditJib, *m_pScaleJibShape, *m_pScaleJibSize, *m_pTranslateJib;

    QAction *m_pConvertMainSailToNURBS, *m_pConvertJibToNURBS;

    QAction *m_pBackImageLoad, *m_pBackImageClear, *m_pBackImageSettings;

    QAction *m_pEditHull,*m_pScaleHull,*m_pTranslateHull;

    QAction *m_pManageBoats, *m_pImportBoatFromXml;

    // polar actions
    QAction *m_pDefineBtPolar, *m_pDuplicateBtPolar, *m_pDuplicateCurBtPolar, *m_pBtPolarNameAct;
    QAction *m_pShowAllBtPlrs, *m_pHideAllBtPlrs,*m_pExportBtPolars;


    QAction *m_pEditBtPolar, *m_pEditBtPolarPts, *m_pRenameCurBtPolar,*m_pDeleteCurBtPolar,*m_pResetCurBtPolar;
    QAction *m_pShowAllBtPlrOpps,*m_pHideAllBtPlrOpps,*m_pDeleteAllBtPlrOpps,*m_pShowBtPlrOppsOnly;
    QAction *m_pExportCurBtPolar, *m_pCopyCurBtPolarData,*m_pExportAnalysisToXML,*m_pShowBtPolarProps;

    QAction *m_pShowAnalysisWindow, *m_pAnalysis3dSettings;

    QAction *m_pImportAnalysisFromXml;
    QAction *m_pMeshInfo, *m_pConnectTriangles, *m_pCheckPanels, *m_pShowNormals;

    //boatopp actions
    QAction *m_pShowBoatBtOppProps, *m_pExportCurBtOpp, *m_pCopyCurBtOppData, *m_pDeleteCurBtOpp;
    QAction *m_pDeleteAllBtPolarOpps;

    XSail *m_pXSail;
};



