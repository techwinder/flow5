/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

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



