/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

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

