/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#define MAXGRAPHS      5  /**< The max number of graphs available for display at one time. */


#include <QString>
#include <QSettings>

struct GraphTileVariableSet
{
    public:
    GraphTileVariableSet();

        QString const &name() const {return m_Name;}
        void setName(QString const &name) {m_Name=name;}

        int XVar(int iGraph) const {if (iGraph>=0 && iGraph<MAXGRAPHS) return m_XVar[iGraph]; else return 0;}
        int YVar(int iGraph) const {if (iGraph>=0 && iGraph<MAXGRAPHS) return m_YVar[iGraph]; else return 0;}
        bool bYInverted(int iGraph) const {if (iGraph>=0 && iGraph<MAXGRAPHS) return m_bYInverted[iGraph]; else return false;}

        void setXVar(int iGraph, int iVar) {if (iGraph>=0 && iGraph<MAXGRAPHS) m_XVar[iGraph]=iVar;}
        void setYVar(int iGraph, int iVar) {if (iGraph>=0 && iGraph<MAXGRAPHS) m_YVar[iGraph]=iVar;}
        void setVariables(int iGraph, int iXVar, int iYVar, bool bInverted=false);
        void setVariables(int *iXVar, int *iYVar);

        void loadSettings(QSettings &settings, QString const &groupname);
        void saveSettings(QSettings &settings, QString const &groupname);


    public:
        QString m_Name;
        int m_XVar[MAXGRAPHS];
        int m_YVar[MAXGRAPHS];

        bool m_bYInverted[MAXGRAPHS];
};

