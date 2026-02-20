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


#include <QMenuBar>

#include "xplanemenus.h"
#include <globals/mainframe.h>
#include <modules/xplane/xplane.h>
#include <modules/xplane//menus/xplaneactions.h>


XPlaneMenus::XPlaneMenus(MainFrame *pMainFrame, XPlane *pXPlane)
{
    m_pMainFrame = pMainFrame;
    m_pXPlane = pXPlane;
}


XPlaneMenus::~XPlaneMenus()
{
    if(!m_pSubWingMenu->parent()) delete m_pSubWingMenu;
    if(!m_pSubStabMenu->parent()) delete m_pSubStabMenu;
    if(!m_pSubFuseMenu->parent()) delete m_pSubFuseMenu;
    if(!m_pSubFinMenu->parent())  delete m_pSubFinMenu;
}


void XPlaneMenus::createMenus()
{
    createPlaneSubMenus();
    createMainBarMenus();
    create3dCtxMenus();
    createWPolarCtxMenus();
    createPOppCtxMenus();
}


void XPlaneMenus::create3dCtxMenus()
{
    XPlaneActions *pActions = m_pXPlane->m_pActions;
    //W3D View Context Menu
    m_p3dCtxMenu = new QMenu(tr("Context Menu"), m_pMainFrame);
    {
        m_p3dCtxMenu->addMenu(m_pCurrentPlaneMenu);
        m_p3dCtxMenu->addSeparator();
        m_p3dCtxMenu->addMenu(m_pCurWPlrMenu);
        m_p3dCtxMenu->addSeparator();
        m_p3dCtxMenu->addMenu(m_pCurPOppMenu);
        m_p3dCtxMenu->addSeparator();
        QMenu *pMeshMenu = m_p3dCtxMenu->addMenu(tr("Mesh"));
        {
            pMeshMenu->addAction(pActions->m_pMeshInfo);
            pMeshMenu->addAction(pActions->m_pConnectPanels);
            pMeshMenu->addAction(pActions->m_pCheckPanels);
            pMeshMenu->addAction(pActions->m_pCheckFreeEdges);
            pMeshMenu->addAction(pActions->m_pClearHighlightedPanels);
            pMeshMenu->addSeparator();
            pMeshMenu->addAction(pActions->m_pCenterOnPanel);
            pMeshMenu->addSeparator();
            pMeshMenu->addAction(pActions->m_pShowPanelNormals);
            pMeshMenu->addAction(pActions->m_pShowNodeNormals);
            pMeshMenu->addAction(pActions->m_pShowVortices);
        }

        m_p3dCtxMenu->addSeparator();
        m_p3dCtxMenu->addAction(pActions->m_pResetScale);
        m_p3dCtxMenu->addAction(pActions->m_p3dLightAct);
        m_p3dCtxMenu->addAction(pActions->m_pOpen3dViewInNewWindow);
        m_p3dCtxMenu->addSeparator();
        m_p3dCtxMenu->addAction(m_pMainFrame->m_pViewLogFile);
        m_p3dCtxMenu->addSeparator();
        QMenu *pImageMenu = m_p3dCtxMenu->addMenu(tr("Background Image"));
        {
            pImageMenu->addAction(pActions->m_pBackImageLoad);
            pImageMenu->addAction(pActions->m_pBackImageClear);
            pImageMenu->addAction(pActions->m_pBackImageSettings);
        }
        m_p3dCtxMenu->addSeparator();
        m_p3dCtxMenu->addAction(m_pMainFrame->m_pSaveViewToImageFileAct);
    }
}


void XPlaneMenus::createWPolarCtxMenus()
{
    XPlaneActions *pActions = m_pXPlane->m_pActions;
    //Polar View Context Menu
    m_pWPlrCtxMenu = new QMenu(tr("Context Menu"), m_pMainFrame);
    {
        m_pWPlrCtxMenu->addMenu(m_pCurrentPlaneMenu);
        m_pWPlrCtxMenu->addSeparator();
        m_pWPlrCtxMenu->addMenu(m_pCurWPlrMenu);
        m_pWPlrCtxMenu->addSeparator();
        QMenu *pAllPolars = m_pWPlrCtxMenu->addMenu(tr("All polars"));
        {
            pAllPolars->addAction(pActions->m_pHideAllWPlrs);
            pAllPolars->addAction(pActions->m_pShowAllWPlrs);
            pAllPolars->addAction(pActions->m_pExportAllWPolars);
        }
        QMenu *pCurGraphCtxMenu = m_pWPlrCtxMenu->addMenu(tr("Graph"));
        {
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pResetGraphSplitter);
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pResetCurGraphScales);
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pCurGraphDlgAct);
            pCurGraphCtxMenu->addSeparator();
            QMenu *pExportMenu = pCurGraphCtxMenu->addMenu(tr("Export"));
            {
                pExportMenu->addAction(m_pMainFrame->m_pExportCurGraphDataToFile);
                pExportMenu->addAction(m_pMainFrame->m_pCopyCurGraphDataAct);
                pExportMenu->addAction(m_pMainFrame->m_pExportGraphToSvgFile);
            }

            pCurGraphCtxMenu->addSeparator();
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pOpenGraphInNewWindow);
        }
        m_pWPlrCtxMenu->addSeparator();
        m_pWPlrCtxMenu->addAction(m_pMainFrame->m_pViewLogFile);
        m_pWPlrCtxMenu->addAction(m_pMainFrame->m_pSaveViewToImageFileAct);
    }
}


void XPlaneMenus::createPOppCtxMenus()
{
    XPlaneActions *pActions = m_pXPlane->m_pActions;
    //WOpp View Context Menu
    m_pWOppCtxMenu = new QMenu(tr("Context Menu"), m_pMainFrame);
    {
        m_pCurrentPlaneCtxMenu = m_pWOppCtxMenu->addMenu(tr("Active plane"));
        {
            m_pCurrentPlaneCtxMenu->addAction(pActions->m_pEditPlaneAct);
            m_pCurrentPlaneCtxMenu->addAction(pActions->m_pEditPlaneDescriptionAct);
            m_pCurrentPlaneCtxMenu->addSeparator();
            m_pCurrentPlaneCtxMenu->addAction(pActions->m_pRenameCurPlaneAct);
            m_pCurrentPlaneCtxMenu->addAction(pActions->m_pDuplicateCurPlane);
            m_pCurrentPlaneCtxMenu->addAction(pActions->m_pDeleteCurPlane);
            m_pCurrentPlaneCtxMenu->addAction(pActions->m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneCtxMenu->addSeparator();
            m_pCurrentPlaneCtxMenu->addAction(pActions->m_pOptimizeAct);
            m_pCurrentPlaneCtxMenu->addSeparator();
            m_pCurrentPlaneCtxMenu->addAction(pActions->m_pTranslatePlaneAct);
            m_pCurrentPlaneCtxMenu->addAction(pActions->m_pScalePlaneAct);
            m_pCurrentPlaneCtxMenu->addSeparator();
            m_pCurrentPlaneCtxMenu->addMenu(m_pSubWingMenu);
            m_pCurrentPlaneCtxMenu->addMenu(m_pSubStabMenu);
            m_pCurrentPlaneCtxMenu->addMenu(m_pSubFinMenu);
            m_pCurrentPlaneCtxMenu->addMenu(m_pSubFuseMenu);

            m_pCurrentPlaneCtxMenu->addSeparator();
            m_pCurrentPlaneCtxMenu->addAction(pActions->m_pPlaneInertia);
            m_pCurrentPlaneCtxMenu->addSeparator();
            QMenu *pExportMenu = m_pCurrentPlaneCtxMenu->addMenu(tr("Export"));
            {
                pExportMenu->addAction(pActions->m_pExporttoAVL);
                pExportMenu->addAction(pActions->m_pExportPlaneToXML);
                pExportMenu->addSeparator();
                pExportMenu->addAction(pActions->m_pExporttoSTL);
                pExportMenu->addAction(pActions->m_pExportMeshtoSTL);
            }
            m_pCurrentPlaneCtxMenu->addSeparator();
            QMenu *pAnalysisMenu = m_pCurrentPlaneCtxMenu->addMenu(tr("Analyses"));
            {
                pAnalysisMenu->addAction(pActions->m_pDefineT123578Polar);
                pAnalysisMenu->addAction(pActions->m_pDefineT6Polar);
                pAnalysisMenu->addAction(pActions->m_pDefineT7Polar);
                pAnalysisMenu->addAction(pActions->m_pDuplicateWPolar);
                pAnalysisMenu->addSeparator();
                pAnalysisMenu->addAction(pActions->m_pBatchAnalysis);
                pAnalysisMenu->addAction(pActions->m_pBatchAnalysisOld);
                pAnalysisMenu->addSeparator();
                pAnalysisMenu->addAction(pActions->m_pImportXmlAnalyses);
            }
            m_pCurrentPlaneCtxMenu->addSeparator();
            m_pCurrentPlaneCtxMenu->addAction(pActions->m_pImportPolarData);
            m_pCurrentPlaneCtxMenu->addSeparator();
            QMenu *pPolarMenu = m_pCurrentPlaneCtxMenu->addMenu(tr("Associated polars"));
            {
                pPolarMenu->addAction(pActions->m_pShowPlaneWPlrs);
                pPolarMenu->addAction(pActions->m_pHidePlaneWPlrs);
                pPolarMenu->addAction(pActions->m_pDeletePlaneWPlrs);
                pPolarMenu->addSeparator();
                pPolarMenu->addAction(pActions->m_pShowPlaneWPlrsOnly);
            }

            m_pCurrentPlaneCtxMenu->addSeparator();
            QMenu *pPOppMenu = m_pCurrentPlaneCtxMenu->addMenu(tr("Associated operating points"));
            {
                pPOppMenu->addAction(pActions->m_pShowPlaneWOpps);
                pPOppMenu->addAction(pActions->m_pHidePlaneWOpps);
                pPOppMenu->addAction(pActions->m_pDeletePlaneWOpps);
            }
        }
        //m_pWOppCtxMenu->addMenu(m_pCurrentPlaneMenu);
        m_pWOppCtxMenu->addSeparator();
        m_pCurWPlrCtxMenu = m_pWOppCtxMenu->addMenu(tr("Active Polar"));
//        m_pCurWPlrCtxMenu->setWindowFlags(Qt::Tool);
        m_pCurWPlrCtxMenu->setWindowTitle(tr("Active Polar"));
        {
            m_pCurWPlrCtxMenu->addAction(pActions->m_pEditWPolarDef);
            m_pCurWPlrCtxMenu->addAction(pActions->m_pEditExtraDrag);
            m_pCurWPlrCtxMenu->addAction(pActions->m_pEditWPolarPts);
            m_pCurWPlrCtxMenu->addSeparator();
            m_pCurWPlrCtxMenu->addAction(pActions->m_pRenameCurWPolar);
            m_pCurWPlrCtxMenu->addAction(pActions->m_pDuplicateCurWPolar);
            m_pCurWPlrCtxMenu->addAction(pActions->m_pDeleteCurWPolar);
            m_pCurWPlrCtxMenu->addAction(pActions->m_pResetCurWPolar);
            m_pCurWPlrCtxMenu->addAction(pActions->m_pShowOnlyCurPolar);
            m_pCurWPlrCtxMenu->addSeparator();
            QMenu *pPOppMenu = m_pCurWPlrCtxMenu->addMenu(tr("Associated operating points"));
            {
                pPOppMenu->addAction(pActions->m_pShowWPlrPOpps);
                pPOppMenu->addAction(pActions->m_pHideAllWPlrOpps);
                pPOppMenu->addAction(pActions->m_pDeleteWPlrPOpps);
                pPOppMenu->addSeparator();
                pPOppMenu->addAction(pActions->m_pShowWPlrOppsOnly);
            }
            m_pCurWPlrCtxMenu->addSeparator();
            QMenu *pExportMenu = m_pCurWPlrCtxMenu->addMenu(tr("Export data"));
            {
                pExportMenu->addAction(pActions->m_pExportCurWPolar);
                pExportMenu->addAction(pActions->m_pCopyCurWPolarData);
            }
            m_pCurWPlrCtxMenu->addAction(pActions->m_pExportAnalysisToXML);
            m_pCurWPlrCtxMenu->addSeparator();
            m_pCurWPlrCtxMenu->addAction(pActions->m_pShowPolarProps);
        }

        //m_pWOppCtxMenu->addMenu(m_pCurWPlrMenu);
        m_pWOppCtxMenu->addSeparator();
        m_pCurPOppCtxMenu = m_pWOppCtxMenu->addMenu(tr("Active operating point"));
        {
            m_pCurPOppCtxMenu->addAction(pActions->m_pShowWOppProps);
            QMenu *pExportMenu = m_pCurPOppCtxMenu->addMenu(tr("Export data"));
            {
                pExportMenu->addAction(pActions->m_pExportCurPOpp);
                pExportMenu->addAction(pActions->m_pCopyCurPOppData);
            }

            m_pCurPOppCtxMenu->addAction(pActions->m_pDeleteCurWOpp);
        }
        //m_pWOppCtxMenu->addMenu(m_pCurWOppMenu);
        m_pWOppCtxMenu->addSeparator();
        QMenu *pOpMenu = m_pWOppCtxMenu->addMenu(tr("Operating points"));
        {
            pOpMenu->addAction(pActions->m_pShowCurWOppOnly);
            pOpMenu->addAction(pActions->m_pShowAllWOpps);
            pOpMenu->addAction(pActions->m_pHideAllWOpps);
            pOpMenu->addAction(pActions->m_pDeleteAllWOpps);
        }

        m_pWOppCtxMenu->addSeparator();
        QMenu *pCurGraphCtxMenu = m_pWOppCtxMenu->addMenu(tr("Graph"));
        {
            pCurGraphCtxMenu->addAction(pActions->m_pWingSelection);
            pCurGraphCtxMenu->addAction(pActions->m_pShowTargetCurve);
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pShowGraphLegend);
            pCurGraphCtxMenu->addSeparator();
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pResetGraphSplitter);
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pResetCurGraphScales);
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pCurGraphDlgAct);
            pCurGraphCtxMenu->addSeparator();
            QMenu *pExportMenu = pCurGraphCtxMenu->addMenu(tr("Export"));
            {
                pExportMenu->addAction(m_pMainFrame->m_pExportCurGraphDataToFile);
                pExportMenu->addAction(m_pMainFrame->m_pCopyCurGraphDataAct);
                pExportMenu->addAction(m_pMainFrame->m_pExportGraphToSvgFile);
            }

            pCurGraphCtxMenu->addSeparator();
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pOpenGraphInNewWindow);
        }
        m_pWOppCtxMenu->addSeparator();
        m_pWOppCtxMenu->addAction(m_pMainFrame->m_pViewLogFile);
        m_pWOppCtxMenu->addAction(m_pMainFrame->m_pSaveViewToImageFileAct);
    }

    //WOpp View Context Menu
    m_pWCpCtxMenu = new QMenu(tr("Context Menu"), m_pMainFrame);
    {
        m_pWCpCtxMenu->addMenu(m_pCurrentPlaneMenu);
        m_pWCpCtxMenu->addSeparator();
        m_pWCpCtxMenu->addMenu(m_pCurWPlrMenu);
        m_pWCpCtxMenu->addSeparator();
        m_pWCpCtxMenu->addMenu(m_pCurPOppMenu);

        m_pWCpCtxMenu->addSeparator();
        QMenu *pCurGraphCtxMenu = m_pWCpCtxMenu->addMenu(tr("Graph"));
        {
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pResetGraphSplitter);
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pResetCurGraphScales);
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pCurGraphDlgAct);
            pCurGraphCtxMenu->addSeparator();
            QMenu *pExportMenu = pCurGraphCtxMenu->addMenu(tr("Export"));
            {
                pExportMenu->addAction(m_pMainFrame->m_pExportCurGraphDataToFile);
                pExportMenu->addAction(m_pMainFrame->m_pCopyCurGraphDataAct);
                pExportMenu->addAction(m_pMainFrame->m_pExportGraphToSvgFile);
            }

            pCurGraphCtxMenu->addSeparator();
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pOpenGraphInNewWindow);
        }
        m_pWCpCtxMenu->addAction(m_pMainFrame->m_pViewLogFile);
        m_pWCpCtxMenu->addSeparator();
        m_pWCpCtxMenu->addAction(m_pMainFrame->m_pSaveViewToImageFileAct);
    }

    //WTime View Context Menu
    m_pWTimeCtxMenu = new QMenu(tr("Context Menu"), m_pMainFrame);
    {
        m_pWTimeCtxMenu->addMenu(m_pCurrentPlaneMenu);
        m_pWTimeCtxMenu->addSeparator();
        m_pWTimeCtxMenu->addMenu(m_pCurWPlrMenu);
        m_pWTimeCtxMenu->addSeparator();
        m_pWTimeCtxMenu->addMenu(m_pCurPOppMenu);
        m_pWTimeCtxMenu->addSeparator();
        QMenu *pCurGraphCtxMenu = m_pWTimeCtxMenu->addMenu(tr("Graph"));
        {
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pResetGraphSplitter);
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pResetCurGraphScales);
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pCurGraphDlgAct);
            pCurGraphCtxMenu->addSeparator();
            QMenu *pExportMenu = pCurGraphCtxMenu->addMenu(tr("Export"));
            {
                pExportMenu->addAction(m_pMainFrame->m_pExportCurGraphDataToFile);
                pExportMenu->addAction(m_pMainFrame->m_pCopyCurGraphDataAct);
                pExportMenu->addAction(m_pMainFrame->m_pExportGraphToSvgFile);
            }

            pCurGraphCtxMenu->addSeparator();
            pCurGraphCtxMenu->addAction(m_pMainFrame->m_pOpenGraphInNewWindow);
        }
        m_pWTimeCtxMenu->addSeparator();

        m_pWTimeCtxMenu->addAction(m_pMainFrame->m_pViewLogFile);
        m_pWTimeCtxMenu->addAction(m_pMainFrame->m_pSaveViewToImageFileAct);
    }
}


void XPlaneMenus::createPlaneSubMenus()
{
    XPlaneActions *pActions = m_pXPlane->m_pActions;
    m_pSubWingMenu = new QMenu(tr("Wing"));
    {
        m_pSubWingMenu->addAction(pActions->m_pEditWingDef);
//        m_pSubWingMenu->addAction(pActions->m_pEditWingObject);
        m_pSubWingMenu->addAction(pActions->m_pScaleWing);
        m_pSubWingMenu->addAction(pActions->m_pTranslateWing);
        m_pSubWingMenu->addAction(pActions->m_pInertiaWing);
        m_pSubWingMenu->addSeparator();
        QMenu *pWingExportMenu = m_pSubWingMenu->addMenu(tr("Export"));
        {
            pWingExportMenu->addAction(pActions->m_pExportWingXml);
            pWingExportMenu->addAction(pActions->m_pExportWingCAD);
            pWingExportMenu->addSeparator();
            pWingExportMenu->addAction(pActions->m_pExportWingTessToStl);
            pWingExportMenu->addAction(pActions->m_pExportWingMeshToStl);
        }
        m_pSubWingMenu->addSeparator();
        m_pSubWingMenu->addAction(pActions->m_pPropsWing);
    }
    m_pSubStabMenu = new QMenu(tr("Elevator"));
    {
        m_pSubStabMenu->addAction(pActions->m_pEditStabDef);
//        m_pSubStabMenu->addAction(pActions->m_pEditStabObject);
        m_pSubStabMenu->addAction(pActions->m_pScaleStab);
        m_pSubStabMenu->addAction(pActions->m_pTranslateStab);
        m_pSubStabMenu->addAction(pActions->m_pInertiaStab);
        m_pSubStabMenu->addSeparator();
        QMenu *pStabExportMenu = m_pSubStabMenu->addMenu(tr("Export"));
        {
            pStabExportMenu->addAction(pActions->m_pExportStabXml);
            pStabExportMenu->addAction(pActions->m_pExportStabCAD);
            pStabExportMenu->addSeparator();
            pStabExportMenu->addAction(pActions->m_pExportStabTessToStl);
            pStabExportMenu->addAction(pActions->m_pExportStabMeshToStl);
        }
        m_pSubStabMenu->addSeparator();
        m_pSubStabMenu->addAction(pActions->m_pPropsStab);
    }
    m_pSubFinMenu = new QMenu(tr("Fin"));
    {
        m_pSubFinMenu->addAction(pActions->m_pEditFinDef);
//        m_pSubFinMenu->addAction(pActions->m_pEditFinObject);
        m_pSubFinMenu->addAction(pActions->m_pScaleFin);
        m_pSubFinMenu->addAction(pActions->m_pTranslateFin);
        m_pSubFinMenu->addAction(pActions->m_pInertiaFin);
        m_pSubFinMenu->addSeparator();
        QMenu *pFinExportMenu = m_pSubFinMenu->addMenu(tr("Export"));
        {
            pFinExportMenu->addAction(pActions->m_pExportFinXml);
            pFinExportMenu->addAction(pActions->m_pExportFinCAD);
            pFinExportMenu->addSeparator();
            pFinExportMenu->addAction(pActions->m_pExportFinTessToStl);
            pFinExportMenu->addAction(pActions->m_pExportFinMeshToStl);
        }
        m_pSubFinMenu->addSeparator();
        m_pSubFinMenu->addAction(pActions->m_pPropsFin);
    }
    m_pSubFuseMenu = new QMenu(tr("Fuselage"));
    {
        m_pSubFuseMenu->addAction(pActions->m_pEditFuse);
        m_pSubFuseMenu->addAction(pActions->m_pEditFuseObject);
        m_pSubFuseMenu->addAction(pActions->m_pScaleFuse);
        m_pSubFuseMenu->addAction(pActions->m_pTranslateFuse);
        m_pSubFuseMenu->addAction(pActions->m_pInertiaFuse);
        m_pSubFuseMenu->addSeparator();
        QMenu *pFuseExportMenu = m_pSubFuseMenu->addMenu(tr("Export"));
        {
            pFuseExportMenu->addAction(pActions->m_pExportFuseXml);
            pFuseExportMenu->addAction(pActions->m_pExportFuseCAD);
            pFuseExportMenu->addSeparator();
            pFuseExportMenu->addAction(pActions->m_pExportFuseTessToStl);
            pFuseExportMenu->addAction(pActions->m_pExportFuseMeshToStl);
        }
        m_pSubFuseMenu->addSeparator();
        m_pSubFuseMenu->addAction(pActions->m_pMakeFuseTriMesh);
        m_pSubFuseMenu->addAction(pActions->m_pResetFuseMesh);
        m_pSubFuseMenu->addSeparator();
        m_pSubFuseMenu->addAction(pActions->m_pPropsFuse);
    }
}


void XPlaneMenus::createMainBarMenus()
{
    XPlaneActions *pActions = m_pXPlane->m_pActions;

    m_pXPlaneViewMenu = m_pMainFrame->menuBar()->addMenu(tr("&View"));
    {
        m_pXPlaneViewMenu->addAction(pActions->m_pWOppAct);
        m_pXPlaneViewMenu->addAction(pActions->m_pWPolarAct);
        m_pXPlaneViewMenu->addAction(pActions->m_pW3dAct);
        m_pXPlaneViewMenu->addAction(pActions->m_pCpViewAct);
        m_pXPlaneViewMenu->addAction(pActions->m_pStabTimeAct);
        m_pXPlaneViewMenu->addAction(pActions->m_pRootLocusAct);
        m_pXPlaneViewMenu->addSeparator();
        m_pXPlaneViewMenu->addAction(pActions->m_pResetScale);
        m_pXPlaneViewMenu->addAction(pActions->m_p3dLightAct);
        m_pXPlaneViewMenu->addAction(pActions->m_pOpen3dViewInNewWindow);
        m_pXPlaneViewMenu->addSeparator();
        m_pXPlaneViewMenu->addAction(m_pMainFrame->m_pSaveViewToImageFileAct);
    }

    m_pPlaneMenu = m_pMainFrame->menuBar()->addMenu(tr("&Plane"));
    {
        m_pPlaneMenu->addAction(pActions->m_pDefinePlaneAct);
        m_pCurrentPlaneMenu = m_pPlaneMenu->addMenu(tr("Active plane"));
        {
            m_pCurrentPlaneMenu->addAction(pActions->m_pEditPlaneAct);
            m_pCurrentPlaneMenu->addAction(pActions->m_pEditPlaneDescriptionAct);
            m_pCurrentPlaneMenu->addSeparator();
            m_pCurrentPlaneMenu->addAction(pActions->m_pRenameCurPlaneAct);
            m_pCurrentPlaneMenu->addAction(pActions->m_pDuplicateCurPlane);
            m_pCurrentPlaneMenu->addAction(pActions->m_pDeleteCurPlane);
            m_pCurrentPlaneMenu->addAction(pActions->m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneMenu->addSeparator();
            m_pCurrentPlaneMenu->addAction(pActions->m_pOptimizeAct);
            m_pCurrentPlaneMenu->addSeparator();
            m_pCurrentPlaneMenu->addAction(pActions->m_pTranslatePlaneAct);
            m_pCurrentPlaneMenu->addAction(pActions->m_pScalePlaneAct);
            m_pCurrentPlaneMenu->addSeparator();
            m_pCurrentPlaneMenu->addMenu(m_pSubWingMenu);
            m_pCurrentPlaneMenu->addMenu(m_pSubStabMenu);
            m_pCurrentPlaneMenu->addMenu(m_pSubFinMenu);
            m_pCurrentPlaneMenu->addMenu(m_pSubFuseMenu);
            m_pCurrentPlaneMenu->addSeparator();
            m_pCurrentPlaneMenu->addAction(pActions->m_pPlaneInertia);
            m_pCurrentPlaneMenu->addSeparator();
            QMenu *pExportMenu = m_pCurrentPlaneMenu->addMenu(tr("Export"));
            {
                pExportMenu->addAction(pActions->m_pExporttoAVL);
                pExportMenu->addAction(pActions->m_pExportPlaneToXML);
                pExportMenu->addSeparator();
                pExportMenu->addAction(pActions->m_pExporttoSTL);
                pExportMenu->addAction(pActions->m_pExportMeshtoSTL);
            }
            m_pCurrentPlaneMenu->addSeparator();
            QMenu *pAnalysisMenu = m_pCurrentPlaneMenu->addMenu(tr("Analyses"));
            {
                pAnalysisMenu->addAction(pActions->m_pDefineT123578Polar);
                pAnalysisMenu->addAction(pActions->m_pDefineT6Polar);
                pAnalysisMenu->addAction(pActions->m_pDefineT7Polar);
                pAnalysisMenu->addAction(pActions->m_pDuplicateWPolar);
                pAnalysisMenu->addSeparator();
                pAnalysisMenu->addAction(pActions->m_pBatchAnalysis);
                pAnalysisMenu->addAction(pActions->m_pBatchAnalysisOld);
                pAnalysisMenu->addSeparator();
                pAnalysisMenu->addAction(pActions->m_pImportXmlAnalyses);
            }
            m_pCurrentPlaneMenu->addSeparator();
            m_pCurrentPlaneMenu->addAction(pActions->m_pImportPolarData);
            m_pCurrentPlaneMenu->addSeparator();
            QMenu *pPolarMenu = m_pCurrentPlaneMenu->addMenu(tr("Associated polars"));
            {
                pPolarMenu->addAction(pActions->m_pShowPlaneWPlrs);
                pPolarMenu->addAction(pActions->m_pHidePlaneWPlrs);
                pPolarMenu->addAction(pActions->m_pDeletePlaneWPlrs);
                pPolarMenu->addSeparator();
                pPolarMenu->addAction(pActions->m_pShowPlaneWPlrsOnly);
            }
            m_pCurrentPlaneMenu->addSeparator();
            QMenu *pPOppMenu = m_pCurrentPlaneMenu->addMenu(tr("Associated operating points"));
            {
                pPOppMenu->addAction(pActions->m_pShowPlaneWOpps);
                pPOppMenu->addAction(pActions->m_pHidePlaneWOpps);
                pPOppMenu->addAction(pActions->m_pDeletePlaneWOpps);
            }
        }
        m_pPlaneMenu->addSeparator();
        QMenu *pImportMenu = m_pPlaneMenu->addMenu(tr("Import"));
        {
            pImportMenu->addAction(pActions->m_pImportPlaneFromXml);
            pImportMenu->addAction(pActions->m_pSTLPlaneAct);
        }
    }

    m_pXPlaneWPlrMenu = m_pMainFrame->menuBar()->addMenu(tr("&Polars"));
    {
        m_pCurWPlrMenu = m_pXPlaneWPlrMenu->addMenu(tr("Active polar"));
        {
            m_pCurWPlrMenu->addAction(pActions->m_pEditWPolarDef);
            m_pCurWPlrMenu->addAction(pActions->m_pEditExtraDrag);
            m_pCurWPlrMenu->addAction(pActions->m_pEditWPolarPts);
            m_pCurWPlrMenu->addSeparator();
            m_pCurWPlrMenu->addAction(pActions->m_pRenameCurWPolar);
            m_pCurWPlrMenu->addAction(pActions->m_pDuplicateCurWPolar);
            m_pCurWPlrMenu->addAction(pActions->m_pDeleteCurWPolar);
            m_pCurWPlrMenu->addAction(pActions->m_pResetCurWPolar);
            m_pCurWPlrMenu->addAction(pActions->m_pShowOnlyCurPolar);
            m_pCurWPlrMenu->addSeparator();
            QMenu *pPOppMenu = m_pCurWPlrMenu->addMenu(tr("Associated operating points"));
            {
                pPOppMenu->addAction(pActions->m_pShowWPlrPOpps);
                pPOppMenu->addAction(pActions->m_pHideAllWPlrOpps);
                pPOppMenu->addAction(pActions->m_pDeleteWPlrPOpps);
                pPOppMenu->addSeparator();
                pPOppMenu->addAction(pActions->m_pShowWPlrOppsOnly);
            }
            m_pCurWPlrMenu->addSeparator();
            QMenu *pExportMenu = m_pCurWPlrMenu->addMenu(tr("Export data"));
            {
                pExportMenu->addAction(pActions->m_pExportCurWPolar);
                pExportMenu->addAction(pActions->m_pCopyCurWPolarData);
            }
            m_pCurWPlrMenu->addSeparator();
            m_pCurWPlrMenu->addAction(pActions->m_pExportAnalysisToXML);
            m_pCurWPlrMenu->addSeparator();
            m_pCurWPlrMenu->addAction(pActions->m_pShowPolarProps);
        }

        QMenu *pAllPolars = m_pXPlaneWPlrMenu->addMenu(tr("All polars"));
        {
            pAllPolars->addAction(pActions->m_pHideAllWPlrs);
            pAllPolars->addAction(pActions->m_pShowAllWPlrs);
            pAllPolars->addAction(pActions->m_pExportAllWPolars);
        }
        m_pXPlaneWPlrMenu->addSeparator();
        m_pXPlaneWPlrMenu->addAction(pActions->m_pImportXmlAnalyses);
        m_pXPlaneWPlrMenu->addAction(pActions->m_pImportPolarData);
    }

    m_pXPlaneWOppMenu = m_pMainFrame->menuBar()->addMenu(tr("&Operating point"));
    {
        m_pCurPOppMenu = m_pXPlaneWOppMenu->addMenu(tr("Active operating point"));
        {
            m_pCurPOppMenu->addAction(pActions->m_pShowWOppProps);
            QMenu *pExportMenu = m_pCurPOppMenu->addMenu(tr("Export data"));
            {
                pExportMenu->addAction(pActions->m_pExportCurPOpp);
                pExportMenu->addAction(pActions->m_pCopyCurPOppData);
            }
            m_pCurPOppMenu->addAction(pActions->m_pDeleteCurWOpp);
        }
        m_pXPlaneWOppMenu->addSeparator();
        QMenu *pOpMenu = m_pXPlaneWOppMenu->addMenu(tr("Operating points"));
        {
            pOpMenu->addAction(pActions->m_pShowCurWOppOnly);
            pOpMenu->addAction(pActions->m_pShowAllWOpps);
            pOpMenu->addAction(pActions->m_pHideAllWOpps);
            pOpMenu->addAction(pActions->m_pDeleteAllWOpps);
        }
        m_pXPlaneWOppMenu->addSeparator();
        m_pXPlaneWOppMenu->addAction(pActions->m_pWingSelection);
        m_pXPlaneWOppMenu->addAction(pActions->m_pShowTargetCurve);
    }

    //XPlane Analysis Menu
    m_pXPlaneAnalysisMenu  = m_pMainFrame->menuBar()->addMenu(tr("&Analysis"));
    {
        m_pXPlaneAnalysisMenu->addAction(pActions->m_pDefineT123578Polar);
        m_pXPlaneAnalysisMenu->addAction(pActions->m_pDefineT6Polar);
        m_pXPlaneAnalysisMenu->addAction(pActions->m_pDefineT7Polar);
        m_pXPlaneAnalysisMenu->addAction(pActions->m_pDuplicateWPolar);
        m_pXPlaneAnalysisMenu->addSeparator();
        m_pXPlaneAnalysisMenu->addAction(pActions->m_pBatchAnalysis);
        m_pXPlaneAnalysisMenu->addAction(pActions->m_pBatchAnalysisOld);
        m_pXPlaneAnalysisMenu->addSeparator();

        m_pXPlaneAnalysisMenu->addSeparator();
        m_pXPlaneAnalysisMenu->addAction(pActions->m_pWPolarNameAct);
        m_pXPlaneAnalysisMenu->addSeparator();
        m_pXPlaneAnalysisMenu->addAction(pActions->m_pImportXmlAnalyses);
        m_pXPlaneAnalysisMenu->addAction(pActions->m_pShowAnalysisWindow);
        m_pXPlaneAnalysisMenu->addAction(m_pMainFrame->m_pViewLogFile);
        m_pXPlaneAnalysisMenu->addSeparator();
        QMenu *pMeshMenu = m_pXPlaneAnalysisMenu->addMenu(tr("Mesh"));
        {
            pMeshMenu->addAction(pActions->m_pMeshInfo);
            pMeshMenu->addAction(pActions->m_pConnectPanels);
            pMeshMenu->addAction(pActions->m_pCheckPanels);
            pMeshMenu->addAction(pActions->m_pCheckFreeEdges);
            pMeshMenu->addAction(pActions->m_pClearHighlightedPanels);
            pMeshMenu->addSeparator();
            pMeshMenu->addAction(pActions->m_pCenterOnPanel);
            pMeshMenu->addSeparator();
            pMeshMenu->addAction(pActions->m_pShowPanelNormals);
            pMeshMenu->addAction(pActions->m_pShowNodeNormals);
            pMeshMenu->addAction(pActions->m_pShowVortices);
        }
        m_pXPlaneAnalysisMenu->addSeparator();
        m_pXPlaneAnalysisMenu->addAction(pActions->m_pAnalysis3dSettings);
    }
}


