/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QMenu>

class XSail;
class MainFrame;

class XSailMenus
{
    friend class XSail;
    friend class MainFrame;
    friend class gl3dXSailView;
    friend class BoatTreeView;

public:
    XSailMenus(MainFrame *pMainFrame, XSail *pXSail);

    void checkMenus();
    void createMenus();
    void createMainBarMenus();
    void create3dCtxMenus();
    void createPolarCtxMenus();
    void createSubMenus();


private:

    QMenu *m_pXSailViewMenu, *m_pBoatMenu, *m_pXSailAnalysisMenu, *m_pXSailWBtPlrMenu, *m_pXSailBtOppMenu;

    QMenu *m_pSubMainSailMenu, *m_pSubJibMenu, *m_pSubHullMenu;
    QMenu *m_pCurBoatMenu, *m_pCurBtPlrMenu, *m_pCurBtOppMenu;

    QMenu *m_p3dCtxMenu;
    QMenu *m_pBtPlrCtxMenu;

    XSail *m_pXSail;
    MainFrame *m_pMainFrame;

};

