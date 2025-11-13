/****************************************************************************

    flow5
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


#include <QDir>
#include <QDirIterator>
#include <QTime>

#include "saveoptions.h"
#include <xflcore/trace.h>
#include "saveoptions.h"

xfl::enumTextFileType SaveOptions::s_ExportFileType;  /**< Defines if the list separator for the output text files should be a space or a comma. */
QString SaveOptions::s_LastDirName       = QDir::homePath();
QString SaveOptions::s_datFoilDirName    = QDir::homePath();
QString SaveOptions::s_plrPolarDirName   = QDir::homePath();
QString SaveOptions::s_xmlPolarDirName   = QDir::homePath();
QString SaveOptions::s_xmlPlaneDirName   = QDir::homePath();
QString SaveOptions::s_xmlWPolarDirName  = QDir::homePath();
QString SaveOptions::s_xmlScriptDirName  = QDir::homePath();
QString SaveOptions::s_LastExportDirName = QDir::homePath();
QString SaveOptions::s_CADDirName        = QDir::homePath();
QString SaveOptions::s_STLDirName        = QDir::homePath();
QString SaveOptions::s_TempDirName       = QDir::tempPath();
QString SaveOptions::s_LastLogFileName;

bool SaveOptions::s_bXmlWingFoils(false);

bool SaveOptions::s_bCleanOnExit(true);
bool SaveOptions::s_bSaveOpps=true;
bool SaveOptions::s_bSavePOpps=true;
bool SaveOptions::s_bSaveBtOpps=true;
bool SaveOptions::s_bAutoSave=false;
bool SaveOptions::s_bAutoLoadLast=false;
bool SaveOptions::s_bUseLastDir=true;


int SaveOptions::s_SaveInterval= 17;
QString SaveOptions::s_CsvSeparator = ",";


void SaveOptions::resetDefaultDirNames()
{
    s_LastDirName       = QDir::homePath();
    s_datFoilDirName    = QDir::homePath();
    s_plrPolarDirName   = QDir::homePath();
    s_xmlPolarDirName   = QDir::homePath();
    s_xmlPlaneDirName   = QDir::homePath();
    s_xmlWPolarDirName  = QDir::homePath();
    s_xmlScriptDirName  = QDir::homePath();
    s_LastExportDirName = QDir::homePath();
    s_TempDirName = QDir::tempPath();
}


void SaveOptions::loadSettings(QSettings &settings)
{
    settings.beginGroup("SaveOptions");
    {
        s_LastDirName       = settings.value("LastDirName",             QDir::homePath()).toString();
        s_datFoilDirName    = settings.value("DatFoilDirName",          QDir::homePath()).toString();
        s_plrPolarDirName   = settings.value("PlrPolarDirName",         QDir::homePath()).toString();
        s_xmlPolarDirName   = settings.value("XmlFoilAnalysisDirName",  QDir::homePath()).toString();
        s_xmlPlaneDirName   = settings.value("XmlPlaneDirName",         QDir::homePath()).toString();
        s_xmlWPolarDirName  = settings.value("XmlPlaneAnalysisDirName", QDir::homePath()).toString();
        s_xmlScriptDirName  = settings.value("XmlScriptDirName",        QDir::homePath()).toString();
        s_CADDirName        = settings.value("CADDirName",              QDir::homePath()).toString();
        s_STLDirName        = settings.value("STLDirName",              QDir::homePath()).toString();
        s_TempDirName       = settings.value("TempDirName",             QDir::tempPath()).toString();
        s_LastExportDirName = settings.value("ExportDirName",           QDir::tempPath()).toString();
        s_LastLogFileName   = settings.value("LastLogFileName").toString();

        s_bUseLastDir   = settings.value("UseLastDir",          s_bUseLastDir).toBool();
        s_bAutoLoadLast = settings.value("AutoLoadLastProject", s_bAutoLoadLast).toBool();
        s_bSaveOpps     = settings.value("SaveOpps",            s_bSaveOpps).toBool();
        s_bSavePOpps    = settings.value("SavePOpps",           s_bSavePOpps).toBool();
        s_bSaveBtOpps   = settings.value("SaveBtOpps",          s_bSaveBtOpps).toBool();
        s_bXmlWingFoils = settings.value("XmlWingFoils",        s_bXmlWingFoils).toBool();

        s_bAutoSave     = settings.value("AutoSaveProject", false).toBool();
        s_SaveInterval  = settings.value("AutoSaveInterval", 10).toInt();
        s_bAutoSave = false;

        int k = settings.value("ExportFormat", 0).toInt();
        if(k) s_ExportFileType = xfl::CSV;
        else  s_ExportFileType = xfl::TXT;
        s_CsvSeparator   = settings.value("CSVSeparator", ",").toString();

        s_bCleanOnExit = settings.value("CleanLogOnExit", true).toBool();
    }
    settings.endGroup();
}


void SaveOptions::saveSettings(QSettings &settings)
{
    settings.beginGroup("SaveOptions");
    {
        settings.setValue("LastDirName",             s_LastDirName);
        settings.setValue("DatFoilDirName",          s_datFoilDirName);
        settings.setValue("PlrPolarDirName",         s_plrPolarDirName);
        settings.setValue("XmlFoilAnalysisDirName",  s_xmlPolarDirName);
        settings.setValue("XmlPlaneDirName",         s_xmlPlaneDirName);
        settings.setValue("XmlPlaneAnalysisDirName", s_xmlWPolarDirName);
        settings.setValue("XmlScriptDirName",        s_xmlScriptDirName);
        settings.setValue("CADDirName",              s_CADDirName);
        settings.setValue("STLDirName",              s_STLDirName);
        settings.setValue("TempDirName",             s_TempDirName);
        settings.setValue("LastLogFileName",         s_LastLogFileName);
        settings.setValue("ExportDirName",           s_LastExportDirName);

        settings.setValue("UseLastDir",         s_bUseLastDir);
        settings.setValue("AutoSaveProject",    s_bAutoSave);
        settings.setValue("AutoSaveInterval",   s_SaveInterval);
        settings.setValue("AutoLoadLastProject",s_bAutoLoadLast);
        settings.setValue("SaveOpps",           s_bSaveOpps);
        settings.setValue("SavePOpps",          s_bSavePOpps);
        settings.setValue("SaveBtOpps",         s_bSaveBtOpps);
        settings.setValue("XmlWingFoils",       s_bXmlWingFoils);

        if(s_ExportFileType==xfl::TXT) settings.setValue("ExportFormat", 0);
        else                           settings.setValue("ExportFormat", 1);
        settings.setValue("CSVSeparator",   s_CsvSeparator);
        settings.setValue("CleanLogOnExit", s_bCleanOnExit);
    }
    settings.endGroup();
}


void SaveOptions::cleanLogFiles()
{
    if(!s_bCleanOnExit) return;
    if(s_TempDirName.isEmpty()) s_TempDirName = QDir::tempPath();
    QDirIterator it(s_TempDirName, QDirIterator::NoIteratorFlags);
    while (it.hasNext())
    {
        QString fileName =     it.next();
        if(fileName.contains("fl5_", Qt::CaseInsensitive) && fileName.endsWith(".log"))
        {
            QFileInfo fi(fileName);
            bool bOwnerWrite = fi.permission(QFileDevice::WriteOwner);
            if(bOwnerWrite)
            {
                trace ("   Removing "+fileName + " with owner write permission");
                QFile logFile(fileName);
                logFile.remove();
            }
        }
    }
}


QString SaveOptions::newLogFileName()
{
    return s_TempDirName + "/fl5_"+QTime::currentTime().toString("hhmmss")+".log";
}


QString SaveOptions::textSeparator()
{
    if(s_ExportFileType==xfl::TXT) return " ";
    else                           return s_CsvSeparator;
}

void SaveOptions::setLastDirName(QString const &lastDirName)
{
    if(s_bUseLastDir)
        s_LastDirName       = lastDirName;
}

