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

class MainFrame;
class XDirect;
class LineMenu;
class XDirectMenus
{
    friend class XDirect;
    friend class MainFrame;
    friend class OpPointWt;
    friend class FoilExplorer;
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

