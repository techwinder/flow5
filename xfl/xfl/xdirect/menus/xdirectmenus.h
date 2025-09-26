/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QMenu>

class MainFrame;
class XDirect;
class LineMenu;
class XDirectMenus
{
    friend class XDirect;
    friend class MainFrame;
    friend class OpPointWt;
    friend class FoilTreeView;
    friend class BLGraphTiles;
    friend class PlrGraphTiles;
    friend class ProfileTiles;

    public:
        XDirectMenus(MainFrame *pMainFrame, XDirect *pXDirect);


    public:
        void createMenus();
        void createFoilMenus();
        void createPolarMenus();
        void createOppMenus();
        void createDesignViewMenus();
        void createBLMenus();
        void createOtherMenus();
        void checkMenus();

    private:

        QMenu *m_pXDirectViewMenu;
        QMenu *m_pXDirectFoilMenu;
        //    QMenu *m_pDesignViewMenu;
        QMenu *m_pCtxMenu;
        QMenu *m_pActiveFoilMenu;
        QMenu *m_pXFoilAnalysisMenu;
        QMenu *m_pOpPointMenu, *m_pXDirectCpGraphMenu, *m_pActiveOppMenu;
        QMenu *m_pPolarMenu, *m_pActivePolarMenu;
        QMenu *m_pGraphPolarMenu, *CurPolarGraphMenu;
        QMenu *m_pOperFoilCtxMenu, *m_pOperPolarCtxMenu;
        QMenu *m_pBLCtxMenu;

        MainFrame *m_pMainFrame;
        XDirect *m_pXDirect;
};

