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

#include <QAction>
#include <QMenuBar>

#include "xdirectmenus.h"

#include <globals/mainframe.h>
#include <modules/xdirect/menus/xdirectactions.h>
#include <modules/xdirect/view2d/dfoilwt.h>
#include <modules/xdirect/xdirect.h>

#include <interfaces/widgets/line/linemenu.h>


XDirectMenus::XDirectMenus(MainFrame *pMainFrame, XDirect *pXDirect)
{
    m_pMainFrame = pMainFrame;
    m_pXDirect = pXDirect;
}


void XDirectMenus::createMenus()
{
    createOppMenus();
    createPolarMenus();
    createFoilMenus();
    createOtherMenus();
    createBLMenus();
    createDesignViewMenus();
}


void XDirectMenus::createFoilMenus()
{
    XDirectActions const *pActions = m_pXDirect->m_pActions;
    m_pXDirectFoilMenu = m_pMainFrame->menuBar()->addMenu(tr("&Foil"));
    {
        m_pActiveFoilMenu = m_pXDirectFoilMenu->addMenu(tr("Active foil"));
        {
            m_pActiveFoilMenu->addAction(pActions->m_pGetFoilProps);
            m_pActiveFoilMenu->addSeparator();
            m_pActiveFoilMenu->addAction(pActions->m_pRenameCurFoil);
            m_pActiveFoilMenu->addAction(pActions->m_pFoilDescription);
            m_pActiveFoilMenu->addAction(pActions->m_pDeleteCurFoil);
            m_pActiveFoilMenu->addAction(pActions->m_pDuplicateCurFoil);
            m_pActiveFoilMenu->addSeparator();

//            m_pActiveFoilMenu->addAction(pActions->m_pNormalizeFoil);
            m_pActiveFoilMenu->addAction(pActions->m_pDerotateFoil);
            m_pActiveFoilMenu->addAction(pActions->m_pRefineGlobalFoil);
            m_pActiveFoilMenu->addAction(pActions->m_pEditCoordsFoil);
            m_pActiveFoilMenu->addAction(pActions->m_pScaleFoil);
            m_pActiveFoilMenu->addAction(pActions->m_pSetTEGap);
            m_pActiveFoilMenu->addAction(pActions->m_pSetLERadius);
            m_pActiveFoilMenu->addAction(pActions->m_pSetFlap);
            m_pActiveFoilMenu->addSeparator();
            QMenu *pExportMenu = m_pActiveFoilMenu->addMenu(tr("Export"));
            {
                pExportMenu->addAction(pActions->m_pExportCurFoilDat);
                pExportMenu->addAction(pActions->m_pExportCurFoilSVG);
            }
            m_pActiveFoilMenu->addSeparator();
            m_pActiveFoilMenu->addAction(pActions->m_pDefinePolarAct);
            m_pActiveFoilMenu->addAction(pActions->m_pDuplicatePolars);
            QMenu *pPolarsMenu = m_pActiveFoilMenu->addMenu(tr("Associated polars"));
            {
                pPolarsMenu->addAction(pActions->m_pShowFoilPolars);
                pPolarsMenu->addAction(pActions->m_pHideFoilPolars);
                pPolarsMenu->addAction(pActions->m_pDeleteFoilPolars);
                pPolarsMenu->addSeparator();
                pPolarsMenu->addAction(pActions->m_pShowFoilPolarsOnly);
                pPolarsMenu->addSeparator();
                pPolarsMenu->addAction(pActions->m_pSaveFoilPolars);
            }
            m_pActiveFoilMenu->addSeparator();
            QMenu *pOppMenu = m_pActiveFoilMenu->addMenu(tr("Associated operating points"));
            {
                pOppMenu->addAction(pActions->m_pShowFoilOpps);
                pOppMenu->addAction(pActions->m_pHideFoilOpps);
                pOppMenu->addAction(pActions->m_pDeleteFoilOpps);
            }
        }
        m_pXDirectFoilMenu->addSeparator();
        m_pXDirectFoilMenu->addAction(pActions->m_pShowActiveFoilOnly);
        m_pXDirectFoilMenu->addAction(pActions->m_pShowAllFoils);
        m_pXDirectFoilMenu->addAction(pActions->m_pHideAllFoils);
        m_pXDirectFoilMenu->addSeparator();
        QMenu *pDesignMenu = m_pXDirectFoilMenu->addMenu(tr("Design"));
        {
            pDesignMenu->addAction(pActions->m_pInterpolateFoils);
            pDesignMenu->addAction(pActions->m_pNacaFoils);
            pDesignMenu->addAction(pActions->m_pFoilFromCoords);
            pDesignMenu->addAction(pActions->m_pFoilFrom1Spline);
//            pDesignMenu->addAction(pActions->m_pFoilFrom2Splines);
            pDesignMenu->addAction(pActions->m_pFoilFromCamber);
            QMenu *pTestMenu = pDesignMenu->addMenu(tr("Testing"));
            {
                pTestMenu->addAction(pActions->m_pSquareFoil);
                pTestMenu->addAction(pActions->m_pCircleFoil);
            }
        }
    }
}


void XDirectMenus::createPolarMenus()
{
    XDirectActions const *pActions = m_pXDirect->m_pActions;

    m_pXFoilAnalysisMenu = m_pMainFrame->menuBar()->addMenu(tr("&Analysis"));
    {
        m_pXFoilAnalysisMenu->addAction(pActions->m_pDefinePolarAct);
        m_pXFoilAnalysisMenu->addAction(pActions->m_pBatchXFoilAct);
        m_pXFoilAnalysisMenu->addAction(pActions->m_pBatchAltAct);
        m_pXFoilAnalysisMenu->addSeparator();
        m_pXFoilAnalysisMenu->addAction(pActions->m_pImportXMLFoilAnalysis);
        m_pXFoilAnalysisMenu->addSeparator();
        m_pXFoilAnalysisMenu->addAction(m_pMainFrame->m_pViewLogFile);
        m_pXFoilAnalysisMenu->addAction(pActions->m_pShowAnalysisWindow);

        m_pXFoilAnalysisMenu->addSeparator();
        m_pXFoilAnalysisMenu->addAction(pActions->m_pXFoilSettings);
    }

    m_pPolarMenu = m_pMainFrame->menuBar()->addMenu(tr("&Polars"));
    {
        m_pActivePolarMenu = m_pPolarMenu->addMenu(tr("Active polar"));
        {
            m_pActivePolarMenu->addAction(pActions->m_pEditCurPolar);
            m_pActivePolarMenu->addAction(pActions->m_pEditCurPolarPts);
            m_pActivePolarMenu->addAction(pActions->m_pResetCurPolar);
            m_pActivePolarMenu->addAction(pActions->m_pDeletePolar);
            m_pActivePolarMenu->addAction(pActions->m_pRenamePolarAct);
            m_pActivePolarMenu->addSeparator();
            m_pActivePolarMenu->addAction(pActions->m_pShowActivePolarOnly);
            m_pActivePolarMenu->addSeparator();
            m_pActivePolarMenu->addAction(pActions->m_pGetPolarProps);
            m_pActivePolarMenu->addSeparator();
            QMenu *pExportMenu = m_pActivePolarMenu->addMenu(tr("Export"));
            {
                pExportMenu->addAction(pActions->m_pExportCurPolar);
                pExportMenu->addAction(pActions->m_pCopyCurPolarData);
            }

            m_pActivePolarMenu->addSeparator();
            m_pActivePolarMenu->addAction(pActions->m_pExportXMLFoilAnalysis);
            m_pActivePolarMenu->addSeparator();
            QMenu *pPolarOpps = m_pActivePolarMenu->addMenu(tr("Associated operating points"));
            {
                pPolarOpps->addAction(pActions->m_pShowPolarOpps);
                pPolarOpps->addAction(pActions->m_pHidePolarOpps);
                pPolarOpps->addAction(pActions->m_pDeletePolarOpps);
                pPolarOpps->addAction(pActions->m_pExportPolarOpps);
            }
        }
        m_pPolarMenu->addSeparator();
        m_pPolarMenu->addAction(pActions->m_pImportXFoilPolar);
        m_pPolarMenu->addSeparator();
        m_pPolarMenu->addAction(pActions->m_pScanPolarDir);
        m_pPolarMenu->addAction(pActions->m_pExportAllPolars);
        m_pPolarMenu->addSeparator();
        QMenu *pAllPolarsMenu = m_pPolarMenu->addMenu(tr("Polars"));
        {
            pAllPolarsMenu->addAction(pActions->m_pShowAllPolars);
            pAllPolarsMenu->addAction(pActions->m_pHideAllPolars);
        }
        m_pPolarMenu->addSeparator();
    }

    //XDirect polar Context Menu
    m_pOperPolarCtxMenu = new QMenu(tr("Context menu"));
    {
        m_pOperPolarCtxMenu->addMenu(m_pActivePolarMenu);
        m_pOperPolarCtxMenu->addSeparator();//_______________
        QMenu *pCurGraphCtxMenu = m_pOperPolarCtxMenu->addMenu(tr("Graph"));
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

        m_pOperPolarCtxMenu->addSeparator();//_______________
        QMenu *pAnalysisMenu = m_pOperPolarCtxMenu->addMenu(tr("Analysis"));
        {
            pAnalysisMenu->addAction(pActions->m_pDefinePolarAct);
            pAnalysisMenu->addAction(pActions->m_pBatchXFoilAct);
            pAnalysisMenu->addAction(pActions->m_pBatchAltAct);
        }
        m_pOperPolarCtxMenu->addSeparator();//_______________
        QMenu *pAllPolarsMenu = m_pOperPolarCtxMenu->addMenu(tr("Polars"));
        {
            pAllPolarsMenu->addAction(pActions->m_pShowAllPolars);
            pAllPolarsMenu->addAction(pActions->m_pHideAllPolars);
        }
        m_pOperPolarCtxMenu->addSeparator();//_______________
        m_pOperPolarCtxMenu->addAction(m_pMainFrame->m_pSaveViewToImageFileAct);
    }
    //End XDirect polar Context Menu
}


void XDirectMenus::createOppMenus()
{
    XDirectActions const *pActions = m_pXDirect->m_pActions;
    m_pOpPointMenu = m_pMainFrame->menuBar()->addMenu(tr("&Operating points"));
    {
        m_pActiveOppMenu = m_pOpPointMenu->addMenu(tr("Active operating point"));
        {
            QMenu *pExportMenu = m_pActiveOppMenu->addMenu(tr("Export"));
            {
                pExportMenu->addAction(pActions->m_pExportCurOpp);
                pExportMenu->addAction(pActions->m_pCopyCurOppData);
            }
            m_pActiveOppMenu->addSeparator();
            m_pActiveOppMenu->addAction(pActions->m_pDeleteCurOpp);
            m_pActiveOppMenu->addAction(pActions->m_pGetOppProps);
        }
        m_pOpPointMenu->addSeparator();
        m_pXDirectCpGraphMenu = m_pOpPointMenu->addMenu(tr("Cp graph"));
        {
            m_pXDirectCpGraphMenu->addAction(pActions->m_pSetCpVarGraph);
            m_pXDirectCpGraphMenu->addAction(pActions->m_pSetQVarGraph);
            m_pXDirectCpGraphMenu->addAction(pActions->m_pShowInviscidCurve);
            m_pXDirectCpGraphMenu->addSeparator();
            m_pXDirectCpGraphMenu->addAction(pActions->m_pShowCpLegend);
            m_pXDirectCpGraphMenu->addAction(m_pMainFrame->m_pResetGraphSplitter);
            m_pXDirectCpGraphMenu->addAction(m_pMainFrame->m_pResetCurGraphScales);
            m_pXDirectCpGraphMenu->addSeparator();
            QMenu *pExportMenu = m_pXDirectCpGraphMenu->addMenu(tr("Export"));
            {
                pExportMenu->addAction(pActions->m_pExportFoilCpGraphAct);
                pExportMenu->addAction(pActions->m_pExportToClipBoard);
            }

            m_pXDirectCpGraphMenu->addSeparator();
            m_pXDirectCpGraphMenu->addAction(m_pMainFrame->m_pOpenGraphInNewWindow);
        }
        m_pOpPointMenu->addSeparator();
        QMenu *pAllOppsMenu = m_pOpPointMenu->addMenu(tr("Operating points"));
        {
//            pAllOppsMenu->addAction(pActions->m_pShowCurOppOnly);
            pAllOppsMenu->addAction(pActions->m_pHideAllOpPoints);
            pAllOppsMenu->addAction(pActions->m_pShowAllOpPoints);
        }
    }
}


void XDirectMenus::createOtherMenus()
{
    XDirectActions const *pActions = m_pXDirect->m_pActions;
    //MainMenu for XDirect Application
    m_pXDirectViewMenu = m_pMainFrame->menuBar()->addMenu(tr("&View"));
    {
        m_pXDirectViewMenu->addAction(pActions->m_pDesignAct);
        m_pXDirectViewMenu->addAction(pActions->m_pBLAct);
        m_pXDirectViewMenu->addAction(pActions->m_pOpPointsAct);
        m_pXDirectViewMenu->addAction(pActions->m_pPolarsAct);
        m_pXDirectViewMenu->addSeparator();
        m_pXDirectViewMenu->addAction(m_pMainFrame->m_pSaveViewToImageFileAct);

    }

    //XDirect Opp Context Menu
    m_pOperFoilCtxMenu = new QMenu(tr("Context Menu"));
    {
//        m_pOperFoilCtxMenu->setTearOffEnabled(true);
        m_pOperFoilCtxMenu->addMenu(m_pActiveFoilMenu);
        m_pOperFoilCtxMenu->addSeparator();//_______________
        m_pOperFoilCtxMenu->addMenu(m_pActivePolarMenu);
        m_pOperFoilCtxMenu->addSeparator();//_______________
        m_pOperFoilCtxMenu->addMenu(m_pActiveOppMenu);
        m_pOperFoilCtxMenu->addSeparator();//_______________
        QMenu *pAllOppsMenu = m_pOperFoilCtxMenu->addMenu(tr("Operating points"));
        {
//            pAllOppsMenu->addAction(pActions->m_pShowCurOppOnly);
            pAllOppsMenu->addAction(pActions->m_pHideAllOpPoints);
            pAllOppsMenu->addAction(pActions->m_pShowAllOpPoints);
        }
        m_pOperFoilCtxMenu->addSeparator();//_______________
    //    CurGraphCtxMenu = OperFoilCtxMenu->addMenu("Cp graph"));

        m_pOperFoilCtxMenu->addMenu(m_pXDirectCpGraphMenu);

        m_pOperFoilCtxMenu->addSeparator();//_______________
        QMenu *pAnalysisMenu = m_pOperFoilCtxMenu->addMenu(tr("Analysis"));
        {
            pAnalysisMenu->addAction(pActions->m_pDefinePolarAct);
            pAnalysisMenu->addAction(pActions->m_pBatchXFoilAct);
            pAnalysisMenu->addAction(pActions->m_pBatchAltAct);
        }
        m_pOperFoilCtxMenu->addSeparator();//_______________
        QMenu *pViewMenu = m_pOperFoilCtxMenu->addMenu(tr("View"));
        {
            pViewMenu->addAction(pActions->m_pResetFoilScale);
        }
    }
    //End XDirect foil Context Menu
}


void XDirectMenus::createDesignViewMenus()
{
    XDirectActions const *pActions = m_pXDirect->m_pActions;

    //AFoil Context Menu
    m_pCtxMenu = new QMenu(tr("Context Menu"), m_pMainFrame);
    {
        m_pCtxMenu->addMenu(m_pActiveFoilMenu);

        m_pCtxMenu->addSeparator();
        m_pCtxMenu->addAction(pActions->m_pShowActiveFoilOnly);
        m_pCtxMenu->addAction(pActions->m_pShowAllFoils);
        m_pCtxMenu->addAction(pActions->m_pHideAllFoils);
        m_pCtxMenu->addSeparator();
        QMenu *pLEMenu = m_pCtxMenu->addMenu(tr("Leading edge"));
        {
            pLEMenu->addAction(pActions->m_pShowLEPosition);
            pLEMenu->addAction(pActions->m_pShowLECircle);
        }
        m_pCtxMenu->addSeparator();
        m_pCtxMenu->addAction(m_pXDirect->m_pDFoilWt->m_pResetXScaleAct);
        m_pCtxMenu->addAction(m_pXDirect->m_pDFoilWt->m_pResetYScaleAct);
        m_pCtxMenu->addAction(m_pXDirect->m_pDFoilWt->m_pResetXYScaleAct);
        m_pCtxMenu->addAction(m_pXDirect->m_pDFoilWt->m_pGridAct);
        m_pCtxMenu->addAction(pActions->m_pShowLegend);

        m_pCtxMenu->addSeparator();
        QMenu *pBackMenu = m_pCtxMenu->addMenu(tr("Background image"));
        {
            pBackMenu->addAction(m_pXDirect->m_pDFoilWt->m_pLoadImage);
            pBackMenu->addAction(m_pXDirect->m_pDFoilWt->m_pClearImage);
            pBackMenu->addAction(m_pXDirect->m_pDFoilWt->m_pImageSettings);
        }
        m_pCtxMenu->addSeparator();
        QMenu *pExportMenu = m_pCtxMenu->addMenu(tr("Export"));
        {
            pExportMenu->addAction(m_pXDirect->m_pDFoilWt->m_pExportToSVG);
            pExportMenu->addAction(m_pMainFrame->m_pSaveViewToImageFileAct);
        }
    }
    m_pXDirect->m_pDFoilWt->setContextMenu(m_pCtxMenu);
}


void XDirectMenus::createBLMenus()
{
    XDirectActions const *pActions = m_pXDirect->m_pActions;
    m_pBLCtxMenu = new QMenu(tr("Boundary layer"));
    {
        //        m_pBLCtxMenu->setTearOffEnabled(true);
        m_pBLCtxMenu->addMenu(m_pActiveFoilMenu);
        m_pBLCtxMenu->addSeparator();//_______________
        m_pBLCtxMenu->addMenu(m_pActivePolarMenu);
        m_pBLCtxMenu->addSeparator();//_______________
        m_pBLCtxMenu->addMenu(m_pActiveOppMenu);
        m_pBLCtxMenu->addSeparator();//_______________
        QMenu *pAllOppsMenu = m_pBLCtxMenu->addMenu(tr("Operating points"));
        {
//            pAllOppsMenu->addAction(pActions->m_pShowCurOppOnly);
            pAllOppsMenu->addAction(pActions->m_pHideAllOpPoints);
            pAllOppsMenu->addAction(pActions->m_pShowAllOpPoints);
        }
        m_pBLCtxMenu->addSeparator();//_______________

        QMenu *pBLGraphMenu = m_pBLCtxMenu->addMenu(tr("Graph"));
        {
            pBLGraphMenu->addAction(m_pMainFrame->m_pResetGraphSplitter);
            pBLGraphMenu->addAction(m_pMainFrame->m_pResetCurGraphScales);
            pBLGraphMenu->addSeparator();
            QMenu *pExportMenu = pBLGraphMenu->addMenu(tr("Export"));
            {
                pExportMenu->addAction(pActions->m_pExportFoilCpGraphAct);
                pExportMenu->addAction(pActions->m_pExportToClipBoard);
            }

            pBLGraphMenu->addSeparator();
            pBLGraphMenu->addAction(m_pMainFrame->m_pOpenGraphInNewWindow);
        }

        m_pBLCtxMenu->addSeparator();//_______________
        QMenu *pAnalysisMenu = m_pBLCtxMenu->addMenu(tr("Analysis"));
        {
            pAnalysisMenu->addAction(pActions->m_pDefinePolarAct);
            pAnalysisMenu->addAction(pActions->m_pBatchXFoilAct);
            pAnalysisMenu->addAction(pActions->m_pBatchAltAct);
        }
        m_pBLCtxMenu->addSeparator();//_______________
        m_pBLCtxMenu->addAction(m_pMainFrame->m_pSaveViewToImageFileAct);
    }
}


void XDirectMenus::checkMenus()
{

}
