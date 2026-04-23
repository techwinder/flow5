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
#include <QCoreApplication>

class XPlane;
class MainFrame;

class XPlaneMenus
{
    Q_DECLARE_TR_FUNCTIONS(XPlaneMenus)

    friend class XPlane;
    friend class MainFrame;
    friend class PlaneExplorer;
    friend class gl3dXPlaneView;
    friend class CpViewWt;

    public:
        XPlaneMenus(MainFrame *pMainFrame, XPlane *pXPlane);
        ~XPlaneMenus();

    public:
        void createMenus();
        void createMainBarMenus();
        void create3dCtxMenus();
        void createWPolarCtxMenus();
        void createPOppCtxMenus();
        void createPlaneSubMenus();

    private:
        QMenu *m_pXPlaneViewMenu, *m_pPlaneMenu, *m_pXPlaneAnalysisMenu, *m_pXPlaneWPlrMenu, *m_pXPlaneWOppMenu;
        QMenu *m_pSubWingMenu, *m_pSubStabMenu, *m_pSubFinMenu, *m_pSubFuseMenu;

        QMenu *m_pCurrentPlaneMenu, *m_pCurWPlrMenu, *m_pCurPOppMenu;
        QMenu *m_pCurrentPlaneCtxMenu;
        QMenu *m_pCurWPlrCtxMenu;
        QMenu *m_pCurPOppCtxMenu;

        QMenu *m_pWPlrCtxMenu, *m_pWOppCtxMenu, *m_p3dCtxMenu, *m_pWCpCtxMenu, *m_pWTimeCtxMenu;

        XPlane *m_pXPlane;
        MainFrame *m_pMainFrame;
};

