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

#include <QString>
#include <QSettings>

#include <core/enums_core.h>
#include <api/utils.h>


namespace SaveOptions
{
    extern bool s_bUseLastDir;
    extern bool s_bAutoLoadLast;       /**< true if the last project should be loaded on startup */
    extern bool s_bAutoSave;           /**< true if the project should be auto-saved on regular intervals */
    extern int s_SaveInterval;         /**< the time interval in muinutes between two project auto-saves */

    extern bool s_bCleanOnExit;

    extern xfl::enumTextFileType s_ExportFileType;  /**< Defines if the list separator for the output text files should be a space or a comma. */
    extern QString s_LastDirName, s_TempDirName;
    extern QString s_xmlPlaneDirName, s_xmlWPolarDirName, s_xmlScriptDirName;
    extern QString s_datFoilDirName, s_xmlPolarDirName, s_plrPolarDirName;
    extern QString s_LastLogFileName;
    extern QString s_LastExportDirName; /** Directory wher .csv, .svg etc files are exported */
    extern QString s_CADDirName;
    extern QString s_STLDirName;
    extern QString s_CsvSeparator;

    extern bool s_bXmlWingFoils;

    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings);


    QString newLogFileName();
    void cleanLogFiles();
    void setLastDirName(QString const &lastDirName);
    void resetDefaultDirNames();
    QString textSeparator();

    inline QString const &lastDirName()      {return s_LastDirName;}
    inline QString const &datFoilDirName()   {return s_datFoilDirName;}
    inline QString const &plrPolarDirName()  {return s_plrPolarDirName;}
    inline QString const &xmlPolarDirName()  {return s_xmlPolarDirName;}
    inline QString const &xmlPlaneDirName()  {return s_xmlPlaneDirName;}
    inline QString const &xmlWPolarDirName() {return s_xmlWPolarDirName;}
    inline QString const &xmlScriptDirName() {return s_xmlScriptDirName;}
    inline QString const &CADDirName()       {return s_CADDirName;}
    inline QString const &STLDirName()       {return s_STLDirName;}
    inline QString const &tempDirName()      {return s_TempDirName;}
    inline QString const &lastLogFileName()  {return s_LastLogFileName;}
    inline QString const &lastExportDirName(){return s_LastExportDirName;}

    inline void setDatFoilDirName(QString const &datDirName)      {s_datFoilDirName    = datDirName;}
    inline void setPlrPolarDirName(QString const &plrDirName)     {s_plrPolarDirName   = plrDirName;}
    inline void setXmlPolarDirName(QString const &xmlDirName)     {s_xmlPolarDirName   = xmlDirName;}
    inline void setXmlPlaneDirName(QString const &xmlDirName)     {s_xmlPlaneDirName   = xmlDirName;}
    inline void setXmlWPolarDirName(QString const &xmlDirName)    {s_xmlWPolarDirName  = xmlDirName;}
    inline void setXmlScriptDirName(QString const &scriptDirName) {s_xmlScriptDirName  = scriptDirName;}
    inline void setCADDirName(QString const &CADDirName)          {s_CADDirName        = CADDirName;}
    inline void setSTLDirName(QString const &STLDirName)          {s_STLDirName        = STLDirName;}
    inline void setTempDirName(QString const &tmpDirName)         {s_TempDirName       = tmpDirName;}
    inline void setLastLogFileName(QString const &logFileName)    {s_LastLogFileName   = logFileName;}
    inline void setLastExportDirName(QString const &dirName)      {s_LastExportDirName = dirName;}

    inline xfl::enumTextFileType exportFileType() {return s_ExportFileType;}
    inline void setExportFileType(xfl::enumTextFileType exportformat) {s_ExportFileType=exportformat;}

    inline void setAutoLoadLast(bool bAuto) {s_bAutoLoadLast=bAuto;}
    inline void setAutoSave(bool bAuto) {s_bAutoSave=bAuto;}

    inline bool bAutoLoadLast()   {return s_bAutoLoadLast;}
    inline bool bAutoSave()       {return s_bAutoSave;}
    inline bool bXmlWingFoils()   {return s_bXmlWingFoils;}

    inline int saveInterval()     {return s_SaveInterval;}

    inline QString csvSeparator() {return s_CsvSeparator;}



}

