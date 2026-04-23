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

#include "graphtilevariableset.h"

GraphTileVariableSet::GraphTileVariableSet()
{
    for(int ig=0; ig<MAXGRAPHS; ig++)
    {
        m_XVar[ig] = m_YVar[ig] = 0;
        m_bYInverted[ig] = false;
    }
}


void GraphTileVariableSet::loadSettings(QSettings &settings, QString const &groupname)
{
    settings.beginGroup(groupname);
    {
        m_Name = settings.value("Name", m_Name).toString();
        for(int ig=0; ig<MAXGRAPHS; ig++)
        {
            QString strange = QString::asprintf("_%d", ig);
            m_XVar[ig]       = settings.value("XVar"      +strange, m_XVar[ig]).toInt();
            m_YVar[ig]       = settings.value("YVar"      +strange, m_XVar[ig]).toInt();
            m_bYInverted[ig] = settings.value("bYInverted"+strange, m_bYInverted[ig]).toBool();
        }
    }
    settings.endGroup();
}


void GraphTileVariableSet::saveSettings(QSettings &settings, QString const &groupname)
{
    settings.beginGroup(groupname);
    {
        settings.setValue("Name", m_Name);
        for(int ig=0; ig<MAXGRAPHS; ig++)
        {
            QString strange = QString::asprintf("_%d", ig);
            settings.setValue("XVar"+strange,       m_XVar[ig]);
            settings.setValue("YVar"+strange,       m_YVar[ig]);
            settings.setValue("bYInverted"+strange, m_bYInverted[ig]);
        }
    }
    settings.endGroup();
}


void GraphTileVariableSet::setVariables(int iGraph, int iXVar, int iYVar, bool bInverted)
{
    if (iGraph>=0 && iGraph<MAXGRAPHS)
    {
        m_XVar[iGraph]=iXVar;
        m_YVar[iGraph]=iYVar;
        m_bYInverted[iGraph] = bInverted;
    }
}


void GraphTileVariableSet:: setVariables(int *iXVar, int *iYVar)
{
    for(int ig=0; ig<MAXGRAPHS; ig++)
    {
        m_XVar[ig]=iXVar[ig];
        m_YVar[ig]=iYVar[ig];
        m_bYInverted[ig] = false;
    }
}



