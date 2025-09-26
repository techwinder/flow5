/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QMenu>

class XPlane;
class MainFrame;

class XPlaneMenus
{
    friend class XPlane;
    friend class MainFrame;
    friend class PlaneTreeView;
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

