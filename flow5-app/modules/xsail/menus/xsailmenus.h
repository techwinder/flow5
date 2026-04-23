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

class XSail;
class MainFrame;

class XSailMenus
{
    Q_DECLARE_TR_FUNCTIONS(XSailMenus)

    friend class XSail;
    friend class MainFrame;
    friend class gl3dXSailView;
    friend class BoatExplorer;

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

