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

#include "xsailmenus.h"
#include <globals/mainframe.h>
#include <modules/xsail/xsail.h>
#include <modules/xsail/menus/xsailactions.h>
#include <api/boat.h>
#include <api/sail.h>
#include <api/boatpolar.h>
#include <api/boatopp.h>

XSailMenus::XSailMenus(MainFrame *pMainFrame, XSail *pXSail)
{
    m_pMainFrame = pMainFrame;
    m_pXSail = pXSail;
}


void XSailMenus::createMenus()
{
    createSubMenus();
    createMainBarMenus();
    createPolarCtxMenus();
    create3dCtxMenus();
}


void XSailMenus::createMainBarMenus()
{
    XSailActions const *pActions = m_pXSail->m_pActions;
    m_pXSailViewMenu =  m_pMainFrame->menuBar()->addMenu(tr("&View"));
    {
        m_pXSailViewMenu->addAction(pActions->m_pPolarView);
        m_pXSailViewMenu->addAction(pActions->m_p3dView);
        m_pXSailViewMenu->addSeparator();
        m_pXSailViewMenu->addAction(pActions->m_p3dLightAct);
        m_pXSailViewMenu->addAction(pActions->m_pOpen3dViewInNewWindow);
        m_pXSailViewMenu->addSeparator();
        QMenu *pBackImageMenu = m_pXSailViewMenu->addMenu(tr("Background image"));
        {
            pBackImageMenu->addAction(pActions->m_pBackImageLoad);
            pBackImageMenu->addAction(pActions->m_pBackImageClear);
            pBackImageMenu->addAction(pActions->m_pBackImageSettings);
        }
        m_pXSailViewMenu->addSeparator();
        m_pXSailViewMenu->addAction(m_pMainFrame->m_pSaveViewToImageFileAct);

    }

    m_pBoatMenu = m_pMainFrame->menuBar()->addMenu(tr("&Boat"));
    {
        m_pBoatMenu->addAction(pActions->m_pDefineBoatAct);
        m_pBoatMenu->addAction(pActions->m_pManageBoats);
        m_pCurBoatMenu = m_pBoatMenu->addMenu(tr("Active boat"));
        {
            m_pCurBoatMenu->addSeparator();
            m_pCurBoatMenu->addAction(pActions->m_pEditBoatAct);
//            m_pCurBoatMenu->addAction(pActions->m_pOptimizeBoatAct);
            m_pCurBoatMenu->addMenu(m_pSubMainSailMenu);
            m_pCurBoatMenu->addMenu(m_pSubJibMenu);
            m_pCurBoatMenu->addMenu(m_pSubHullMenu);
            m_pCurBoatMenu->addSeparator();
            m_pCurBoatMenu->addAction(pActions->m_pRenameCurBoatAct);
            m_pCurBoatMenu->addAction(pActions->m_pDuplicateCurBoat);
            m_pCurBoatMenu->addAction(pActions->m_pDeleteCurBoat);
            m_pCurBoatMenu->addAction(pActions->m_pSaveBoatAsProjectAct);
            m_pCurBoatMenu->addSeparator();
            QMenu *pExportMenu = m_pCurBoatMenu->addMenu(tr("Export"));
            {
                pExportMenu->addAction(pActions->m_pExportBoatToXML);
            }
            m_pCurBoatMenu->addSeparator();
            QMenu *pAnalysisMenu = m_pCurBoatMenu->addMenu(tr("Analysis"));
            {
                pAnalysisMenu->addAction(pActions->m_pDefineBtPolar);
                pAnalysisMenu->addAction(pActions->m_pDuplicateBtPolar);
                pAnalysisMenu->addAction(pActions->m_pBtPolarNameAct);
                pAnalysisMenu->addAction(pActions->m_pImportAnalysisFromXml);
                pAnalysisMenu->addSeparator();
                pAnalysisMenu->addAction(pActions->m_pShowAnalysisWindow);
                pAnalysisMenu->addAction(pActions->m_pAnalysis3dSettings);
                pAnalysisMenu->addAction(m_pMainFrame->m_pViewLogFile);
                pAnalysisMenu->addSeparator();
                QMenu *pMeshMenu = pAnalysisMenu->addMenu(tr("Mesh"));
                {
                    pMeshMenu->addAction(pActions->m_pMeshInfo);
                    pMeshMenu->addAction(pActions->m_pConnectTriangles);
                    pMeshMenu->addAction(pActions->m_pCheckPanels);
                    pMeshMenu->addAction(pActions->m_pShowNormals);
                }
            }
            m_pCurBoatMenu->addSeparator();
            QMenu *pPolarMenu = m_pCurBoatMenu->addMenu(tr("Associated polars"));
            {
                pPolarMenu->addAction(pActions->m_pShowBoatBtPlrs);
                pPolarMenu->addAction(pActions->m_pHideBoatBtPlrs);
                pPolarMenu->addAction(pActions->m_pDeleteBoatBtPlrs);
                pPolarMenu->addSeparator();
                pPolarMenu->addAction(pActions->m_pShowBoatBtPolarsOnly);
            }
            m_pCurBoatMenu->addSeparator();
            QMenu *pPOppMenu = m_pCurBoatMenu->addMenu(tr("Associated operating points"));
            {
                pPOppMenu->addAction(pActions->m_pDeleteBoatBtOpps);
            }
        }
        m_pBoatMenu->addAction(pActions->m_pImportBoatFromXml);
    }

    m_pXSailWBtPlrMenu = m_pMainFrame->menuBar()->addMenu(tr("Polars"));
    {
        m_pCurBtPlrMenu = m_pXSailWBtPlrMenu->addMenu(tr("Active polar"));
        {
            m_pCurBtPlrMenu->addAction(pActions->m_pEditBtPolar);
            m_pCurBtPlrMenu->addAction(pActions->m_pEditBtPolarPts);
            m_pCurBtPlrMenu->addAction(pActions->m_pRenameCurBtPolar);
            m_pCurBtPlrMenu->addAction(pActions->m_pDuplicateCurBtPolar);
            m_pCurBtPlrMenu->addAction(pActions->m_pDeleteCurBtPolar);
            m_pCurBtPlrMenu->addAction(pActions->m_pResetCurBtPolar);
            m_pCurBtPlrMenu->addSeparator();
            QMenu *pPOppMenu = m_pCurBtPlrMenu->addMenu(tr("Associated operating points"));
            {
                pPOppMenu->addAction(pActions->m_pDeleteAllBtPolarOpps);
            }
            m_pCurBtPlrMenu->addSeparator();
            QMenu *pExportMenu = m_pCurBtPlrMenu->addMenu(tr("Export data"));
            {
                pExportMenu->addAction(pActions->m_pExportCurBtPolar);
                pExportMenu->addAction(pActions->m_pCopyCurBtPolarData);
            }
            m_pCurBtPlrMenu->addSeparator();
            m_pCurBtPlrMenu->addAction(pActions->m_pExportAnalysisToXML);
            m_pCurBtPlrMenu->addSeparator();
            m_pCurBtPlrMenu->addAction(pActions->m_pShowBtPolarProps);
        }

        m_pXSailWBtPlrMenu->addSeparator();
        QMenu *pAllPolars = m_pXSailWBtPlrMenu->addMenu(tr("All polars"));
        {
            pAllPolars->addAction(pActions->m_pHideAllBtPlrs);
            pAllPolars->addAction(pActions->m_pShowAllBtPlrs);
            pAllPolars->addAction(pActions->m_pExportBtPolars);
        }
    }

    m_pXSailBtOppMenu = m_pMainFrame->menuBar()->addMenu(tr("Operating point"));
    {
        m_pCurBtOppMenu = m_pXSailBtOppMenu->addMenu(tr("Active operating point"));
        {
            m_pCurBtOppMenu->addAction(pActions->m_pShowBoatBtOppProps);
            QMenu *pExportMenu = m_pCurBtOppMenu->addMenu(tr("Export data"));
            {
                pExportMenu->addAction(pActions->m_pExportCurBtOpp);
                pExportMenu->addAction(pActions->m_pCopyCurBtOppData);
            }
            m_pCurBtOppMenu->addAction(pActions->m_pDeleteCurBtOpp);
        }
        m_pXSailBtOppMenu->addSeparator();
        QMenu *pOpMenu = m_pXSailBtOppMenu->addMenu(tr("Operating points"));
        {
            pOpMenu->addAction(pActions->m_pDeleteAllBtPolarOpps);
        }
    }


    m_pXSailAnalysisMenu  = m_pMainFrame->menuBar()->addMenu(tr("&Analysis"));
    {
        m_pXSailAnalysisMenu->addAction(pActions->m_pDefineBtPolar);
        m_pXSailAnalysisMenu->addAction(pActions->m_pDuplicateBtPolar);
        m_pXSailAnalysisMenu->addAction(pActions->m_pBtPolarNameAct);
        m_pXSailAnalysisMenu->addAction(pActions->m_pImportAnalysisFromXml);
        m_pXSailAnalysisMenu->addSeparator();
        m_pXSailAnalysisMenu->addAction(pActions->m_pShowAnalysisWindow);
        m_pXSailAnalysisMenu->addAction(pActions->m_pAnalysis3dSettings);
        m_pXSailAnalysisMenu->addAction(m_pMainFrame->m_pViewLogFile);
        m_pXSailAnalysisMenu->addSeparator();
        QMenu *pMeshMenu = m_pXSailAnalysisMenu->addMenu(tr("Mesh"));
        {
            pMeshMenu->addAction(pActions->m_pMeshInfo);
            pMeshMenu->addAction(pActions->m_pConnectTriangles);
            pMeshMenu->addAction(pActions->m_pCheckPanels);
            pMeshMenu->addAction(pActions->m_pShowNormals);
        }
    }
}


void XSailMenus::createSubMenus()
{
    XSailActions const *pActions = m_pXSail->m_pActions;
    m_pSubMainSailMenu = new QMenu(tr("Main sail"));
    {
        m_pSubMainSailMenu->addAction(pActions->m_pEditMainSail);
        m_pSubMainSailMenu->addAction(pActions->m_pTranslateMainSail);
        m_pSubMainSailMenu->addAction(pActions->m_pScaleMainShape);
        m_pSubMainSailMenu->addAction(pActions->m_pScaleMainSize);
        m_pSubMainSailMenu->addSeparator();
        QMenu *pExportMenu = m_pSubMainSailMenu->addMenu(tr("Export"));
        {
            pExportMenu->addAction(pActions->m_pExportMainSailToXml);
            pExportMenu->addAction(pActions->m_pExportMainSailToStep);
            pExportMenu->addAction(pActions->m_pExportMainSailToSTL);
        }
        m_pSubMainSailMenu->addAction(pActions->m_pConvertMainSailToNURBS);
    }

    m_pSubJibMenu = new QMenu(tr("Jib"));
    {
        m_pSubJibMenu->addAction(pActions->m_pEditJib);
        m_pSubJibMenu->addAction(pActions->m_pTranslateJib);
        m_pSubJibMenu->addAction(pActions->m_pScaleJibShape);
        m_pSubJibMenu->addAction(pActions->m_pScaleJibSize);
        m_pSubJibMenu->addSeparator();
        QMenu *pExportMenu = m_pSubJibMenu->addMenu(tr("Export"));
        {
            pExportMenu->addAction(pActions->m_pExportJibToXml);
            pExportMenu->addAction(pActions->m_pExportJibToStep);
            pExportMenu->addAction(pActions->m_pExportJibToSTL);
        }
        m_pSubJibMenu->addAction(pActions->m_pConvertJibToNURBS);
    }

    m_pSubHullMenu = new QMenu(tr("Hull"));
    {
        m_pSubHullMenu->addAction(pActions->m_pEditHull);
        m_pSubHullMenu->addAction(pActions->m_pScaleHull);
        m_pSubHullMenu->addAction(pActions->m_pTranslateHull);
    }
}


void XSailMenus::create3dCtxMenus()
{
    XSailActions const *pActions = m_pXSail->m_pActions;
    m_p3dCtxMenu = new QMenu(tr("Context Menu"), m_pMainFrame);
    m_p3dCtxMenu->addMenu(m_pCurBoatMenu);
    m_p3dCtxMenu->addMenu(m_pCurBtPlrMenu);
    m_p3dCtxMenu->addMenu(m_pCurBtOppMenu);
    m_p3dCtxMenu->addSeparator();
    m_p3dCtxMenu->addAction(pActions->m_p3dLightAct);
    m_p3dCtxMenu->addSeparator();
    QMenu *pMeshMenu = m_p3dCtxMenu->addMenu(tr("Mesh"));
    {
        pMeshMenu->addAction(pActions->m_pMeshInfo);
        pMeshMenu->addAction(pActions->m_pConnectTriangles);
        pMeshMenu->addAction(pActions->m_pCheckPanels);
        pMeshMenu->addAction(pActions->m_pShowNormals);
    }

    m_p3dCtxMenu->addSeparator();
    QMenu *pBackImageMenu = m_p3dCtxMenu->addMenu(tr("Background image"));
    {
        pBackImageMenu->addAction(pActions->m_pBackImageLoad);
        pBackImageMenu->addAction(pActions->m_pBackImageClear);
        pBackImageMenu->addAction(pActions->m_pBackImageSettings);
    }
    m_p3dCtxMenu->addSeparator();
    m_p3dCtxMenu->addAction(m_pMainFrame->m_pViewLogFile);
    m_p3dCtxMenu->addAction(m_pMainFrame->m_pSaveViewToImageFileAct);
}


void XSailMenus::createPolarCtxMenus()
{
    XSailActions const *pActions = m_pXSail->m_pActions;
    m_pBtPlrCtxMenu = new QMenu(tr("Context Menu"), m_pMainFrame);
    {
        m_pBtPlrCtxMenu->addMenu(m_pCurBoatMenu);
        m_pBtPlrCtxMenu->addSeparator();
        m_pBtPlrCtxMenu->addMenu(m_pCurBtPlrMenu);
        m_pBtPlrCtxMenu->addSeparator();
        QMenu *pAllPolars = m_pBtPlrCtxMenu->addMenu(tr("All polars"));
        {
            pAllPolars->addAction(pActions->m_pHideAllBtPlrs);
            pAllPolars->addAction(pActions->m_pShowAllBtPlrs);
            pAllPolars->addAction(pActions->m_pExportBtPolars);
        }
        QMenu *pCurGraphCtxMenu = m_pBtPlrCtxMenu->addMenu(tr("Graph"));
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
        m_pBtPlrCtxMenu->addSeparator();
        m_pBtPlrCtxMenu->addAction(m_pMainFrame->m_pViewLogFile);
        m_pBtPlrCtxMenu->addAction(m_pMainFrame->m_pSaveViewToImageFileAct);
    }
}


void XSailMenus::checkMenus()
{
    Boat *pBoat = m_pXSail->curBoat();
    BoatPolar *pBtPolar = m_pXSail->curBtPolar();
    BoatOpp *pBtOpp = m_pXSail->curBtOpp();

    m_pSubMainSailMenu->setEnabled(pBoat && pBoat->hasSail());
    m_pSubJibMenu->setEnabled(pBoat && pBoat->hasJib());
    m_pSubHullMenu->setEnabled(pBoat && pBoat->hasHull());

    m_pCurBoatMenu->setEnabled(pBoat);
    m_pCurBtPlrMenu->setEnabled(pBtPolar);
    m_pCurBtOppMenu->setEnabled(pBtOpp);
}


