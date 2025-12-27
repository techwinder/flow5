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

#define _MATH_DEFINES_DEFINED

#include <QDir>
#include <QTime>

#include "xflscriptreader.h"

#include <api/objects_global.h>
#include <api/planetask.h>
#include <api/vorton.h>
#include <api/xml_globals.h>

#include <core/xflcore.h>
#include <modules/xobjects.h>


XflScriptReader::XflScriptReader() : QXmlStreamReader()
{
    m_ProjectFileName =  "script_"+QDateTime::currentDateTime().toString("yyMMdd_hhmmss")+".fl5";

    m_PreLoadProjectFilePath.clear();

    //foil Analysis Data
    m_FoilPolarType = xfl::T1POLAR;

    m_bOutputPolarsText = m_bOutputPolarsBin = false;
    m_bMakeOpps = false;
    m_bLoadAllFoils = false;
    m_bRunAllFoilAnalyses = false;
    m_bRepanelFoils = false;
    m_NFoilPanels = 100;
    m_bAlphaSpec = m_bFromZero = true;
    m_XtrBot = m_XtrTop = 0.0;
    m_MaxXFoilIterations = 100;


    m_PlaneFileList.clear();
    m_WPolarFileList.clear();

    // Viscous loop
    m_bViscousLoop = false;
    m_bViscInitVTwist    = PlaneTask::bViscInitVTwist();
    m_ViscRelaxFactor    = PlaneTask::viscRelaxFactor();
    m_ViscAlphaPrecision = PlaneTask::maxViscError();
    m_ViscMaxIterations  = PlaneTask::maxViscIter();

    //VPW
    m_VPWCoreSize      = 0.5;
    m_VPWDiscardDist   = 30.0;
    m_VPWIterations    = 35;

    m_bLoadAllPlanes       = false;
    m_bRunAllPlaneAnalyses = false;

    m_bLoadAllBoats = false;
    m_bRunAllBoatAnalyses = false;
    m_bOutputBtPolarsText = false;
    m_bMakeBtOpps = false;

    m_bMakeProjectFile = true;
    m_bMultiThreading = false;
    m_bMakePOpps = m_bOutputPOppsText = m_bExportPanelCp = m_bExportStlMesh = false;
    m_bCsvOutput = false;
    m_bOutputWPolarsText = false;
    m_nMaxThreads = 1;
    m_bDoublePrecision = true;
    m_bRecursiveDirScan = false;

    /*    m_xmlPlaneDirPath =      SaveOptions::xmlPlaneDirName();
    m_xmlWPolarDirPath =     SaveOptions::xmlWPolarDirName();
    m_datFoilDirPath =       SaveOptions::datFoilDirName();*/
    m_OutputDirPath =        QDir::home().absolutePath();// default which will be overwritten later
    m_PolarBinDirPath.clear(); // will be read later, or set to the output dir if not

    m_ThreadPriority = QThread::NormalPriority;
}


bool XflScriptReader::setFileName(QString scriptFileName)
{
    m_ScriptFileName = scriptFileName;

    QFile xmlFile(scriptFileName);
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        //        QString strange = "   ...could not open the file: "+scriptFileName;
        return false;
    }

    setDevice(&xmlFile);

    if(!readScript())
    {
        QString strange;
        strange = QString::asprintf("\nline %d column %d", int(lineNumber()), int(columnNumber()));
        //        QString errorMsg = errorString() + strange;
        return false;
    }
    return true;
}


bool XflScriptReader::readScript()
{
    // level 0
    if (readNextStartElement())
    {
        if (name().compare(QString("xflscript"), Qt::CaseInsensitive)!=0 || attributes().value("version").toString() < "1.0")
        {
            raiseError("The file is not an xml readable script");
            return false;
        }
    }

    // level 1
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Metadata"), Qt::CaseInsensitive)==0)
        {
            readMetaData();
        }
        else if (name().compare(QString("foil_analysis"), Qt::CaseInsensitive)==0)
        {
            readFoilData();
        }
        else if (name().compare(QString("Plane_analysis"), Qt::CaseInsensitive)==0)
        {
            readPlaneData();
        }
        else if (name().compare(QString("Boat_analysis"), Qt::CaseInsensitive)==0)
        {
            readBoatData();
        }
        else
            skipCurrentElement();
    }

    return(!hasError());
}


bool XflScriptReader::readFoilData()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        //level 2
        if(name().compare(QString("Foil_Files"), Qt::CaseInsensitive)==0)
        {
            readFoilDatNames();
        }
        else if(name().compare(QString("Analysis_Files"), Qt::CaseInsensitive)==0)
        {
            readFoilAnalysisFiles();
        }
        else if(name().compare(QString("Batch_Analysis_Data"), Qt::CaseInsensitive)==0)
        {
            readFoilBatchData();
        }
        else if(name().compare(QString("Output"), Qt::CaseInsensitive)==0)
        {
            readFoilAnalysisOutput();
        }
        else if(name().compare(QString("Options"), Qt::CaseInsensitive)==0)
        {
            readFoilAnalysisOptions();
        }
        else if(name().compare(QString("OpPoint_Range"), Qt::CaseInsensitive)==0)
        {
            readFoilAnalysisRange();
        }
        else
            skipCurrentElement();
    }
    return(!hasError());
}


bool XflScriptReader::readFoilAnalysisFiles()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        //level 2
        if(name().compare(QString("Process_All_Files"), Qt::CaseInsensitive)==0)
        {
            m_bRunAllFoilAnalyses = xfl::stringToBool(readElementText().trimmed());
        }
        else if(name().compare(QString("Analysis_File_Name"), Qt::CaseInsensitive)==0)
        {
            m_XmlFoilAnalysisList.push_back(readElementText().trimmed());
        }

        else
            skipCurrentElement();
    }
    return(!hasError());
}



bool XflScriptReader::readFoilAnalysisRange()
{
    m_AlphaRange.clear();
    m_ClRange.clear();
    m_ReRange.clear();
    m_CtrlRange.clear();

    double vmin(0), vmax(0), vinc(0);
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Alpha"), Qt::CaseInsensitive)==0)
        {
            QStringList AlphaList = readElementText().simplified().split(",");
            if(AlphaList.length()>0) vmin = AlphaList.at(0).toDouble();
            if(AlphaList.length()>1) vmax = AlphaList.at(1).toDouble();
            if(AlphaList.length()>2) vinc = AlphaList.at(2).toDouble();

            m_AlphaRange.push_back({true, vmin, vmax, vinc});
        }
        else if(name().compare(QString("Cl"), Qt::CaseInsensitive)==0)
        {
            QStringList ClList = readElementText().simplified().split(",");
            if(ClList.length()>0) vmin = ClList.at(0).toDouble();
            if(ClList.length()>1) vmax = ClList.at(1).toDouble();
            if(ClList.length()>2) vinc = ClList.at(2).toDouble();

            m_ClRange.push_back({true, vmin, vmax, vinc});
        }
        else if(name().compare(QString("Reynolds"), Qt::CaseInsensitive)==0)
        {
            QStringList ReList = readElementText().simplified().split(",");
            if(ReList.length()>0) vmin = ReList.at(0).toDouble();
            if(ReList.length()>1) vmax = ReList.at(1).toDouble();
            if(ReList.length()>2) vinc = ReList.at(2).toDouble();

            m_ReRange.push_back({true, vmin, vmax, vinc});
        }
        else if(name().compare(QString("Control"), Qt::CaseInsensitive)==0)
        {
            QStringList CtrlList = readElementText().simplified().split(",");
            if(CtrlList.length()>0) vmin = CtrlList.at(0).toDouble();
            if(CtrlList.length()>1) vmax = CtrlList.at(1).toDouble();
            if(CtrlList.length()>2) vinc = CtrlList.at(2).toDouble();
            m_CtrlRange.push_back({true, vmin, vmax, vinc});
        }
        else if(name().compare(QString("Spec_Alpha"), Qt::CaseInsensitive)==0)
        {
            m_bAlphaSpec  = xfl::stringToBool(readElementText().trimmed());
        }
        else if(name().compare(QString("From_Zero"), Qt::CaseInsensitive)==0)
        {
            m_bFromZero  = xfl::stringToBool(readElementText().trimmed());
        }
        else
            skipCurrentElement();
    }
    return(!hasError());
}


bool XflScriptReader::readMetaData()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("make_project_file"), Qt::CaseInsensitive)==0)
        {
            m_bMakeProjectFile = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("project_file_name"), Qt::CaseInsensitive)==0)
        {
            m_ProjectFileName = readElementText().trimmed();
        }
        else if(name().compare(QString("load_project_file"), Qt::CaseInsensitive)==0)
        {
            m_PreLoadProjectFilePath = readElementText().trimmed();
        }
        else if(name().compare(QString("Directories"), Qt::CaseInsensitive)==0)
        {
            readDirectoryData();
        }
        else if(name().compare(QString("polar_text_output_format"), Qt::CaseInsensitive)==0)
        {
            m_bCsvOutput = readElementText().trimmed().compare(QString("csv"), Qt::CaseInsensitive)==0;
        }
        else if(name().compare(QString("MultiThreading"), Qt::CaseInsensitive)==0)
        {
            readThreadingOptions();
        }
        else if(name().compare(QString("Double_Precision"), Qt::CaseInsensitive)==0)
        {
            m_bDoublePrecision = xfl::stringToBool(readElementText());
        }
        else
            skipCurrentElement();
    }
    return !hasError();
}


bool XflScriptReader::readDirectoryData()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("output_dir"), Qt::CaseInsensitive)==0)
        {
            m_OutputDirPath = readElementText().trimmed();
            if(m_OutputDirPath.endsWith(QDir::separator())) m_OutputDirPath.remove(m_OutputDirPath.lastIndexOf(QDir::separator()), 1);
        }
        else if (name().compare(QString("plane_definition_xml_dir"), Qt::CaseInsensitive)==0)
        {
            m_xmlPlaneDirPath = readElementText().trimmed();
            if(m_xmlPlaneDirPath.endsWith(QDir::separator())) m_xmlPlaneDirPath.remove(m_xmlPlaneDirPath.lastIndexOf(QDir::separator()), 1);
        }
        else if (name().compare(QString("plane_analysis_xml_dir"), Qt::CaseInsensitive)==0)
        {
            m_xmlWPolarDirPath = readElementText().trimmed();
            if(m_xmlWPolarDirPath.endsWith(QDir::separator())) m_xmlWPolarDirPath.remove(m_xmlWPolarDirPath.lastIndexOf(QDir::separator()), 1);
        }
        else if (name().compare(QString("foil_files_dir"), Qt::CaseInsensitive)==0)
        {
            m_datFoilDirPath = readElementText().trimmed();
            if(m_datFoilDirPath.endsWith(QDir::separator())) m_datFoilDirPath.remove(m_datFoilDirPath.lastIndexOf(QDir::separator()), 1);
        }
        else if(name().compare(QString("foil_analysis_xml_dir"), Qt::CaseInsensitive)==0)
        {
            m_xmlPolarDirPath = readElementText().trimmed();
            if(m_xmlPolarDirPath.endsWith(QDir::separator())) m_xmlPolarDirPath.remove(m_xmlPolarDirPath.lastIndexOf(QDir::separator()), 1);
        }
        else if(name().compare(QString("foil_polars_dir"), Qt::CaseInsensitive)==0)
        {
            m_PolarBinDirPath = readElementText().trimmed();
            if(m_PolarBinDirPath.endsWith(QDir::separator())) m_PolarBinDirPath.remove(m_PolarBinDirPath.lastIndexOf(QDir::separator()), 1);
        }
        else if(name().compare(QString("xfoil_polars_dir"), Qt::CaseInsensitive)==0)
        {
            m_XFoilPolarDirPath = readElementText().trimmed();
            if(m_XFoilPolarDirPath.endsWith(QDir::separator())) m_XFoilPolarDirPath.remove(m_XFoilPolarDirPath.lastIndexOf(QDir::separator()), 1);
        }
        else if (name().compare(QString("boat_definition_xml_dir"), Qt::CaseInsensitive)==0)
        {
            m_xmlBoatDirPath = readElementText().trimmed();
            if(m_xmlBoatDirPath.endsWith(QDir::separator())) m_xmlBoatDirPath.remove(m_xmlBoatDirPath.lastIndexOf(QDir::separator()), 1);
        }
        else if (name().compare(QString("boat_analysis_xml_dir"), Qt::CaseInsensitive)==0)
        {
            m_xmlBtPolarDirPath = readElementText().trimmed();
            if(m_xmlBtPolarDirPath.endsWith(QDir::separator())) m_xmlBtPolarDirPath.remove(m_xmlBtPolarDirPath.lastIndexOf(QDir::separator()), 1);
        }
        else if(name().compare(QString("recursive_scan"), Qt::CaseInsensitive)==0)
        {
            m_bRecursiveDirScan = xfl::stringToBool(readElementText().trimmed());
        }
        else
            skipCurrentElement();
    }

    return !hasError();
}


bool XflScriptReader::readPlaneData()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Plane_Analysis_Output"), Qt::CaseInsensitive)==0)
        {
            readPlaneAnalysisOutput();
        }
        else if(name().compare(QString("Foil_Dat_Files"), Qt::CaseInsensitive)==0)
        {
            readFoilDatNames();
        }
        else if(name().compare(QString("Foil_Polar_Files"), Qt::CaseInsensitive)==0)
        {
            readFoilPolars();
        }
        else if(name().compare(QString("Plane_Definition_Files"), Qt::CaseInsensitive)==0)
        {
            readPlanes();
        }
        else if(name().compare(QString("Plane_Analysis_Files"), Qt::CaseInsensitive)==0)
        {
            readPlaneAnalysisFiles();
        }
        else if(name().compare(QString("Plane_Analysis_Data"), Qt::CaseInsensitive)==0)
        {
            readPlaneAnalysisData();
        }
        else
            skipCurrentElement();
    }
    return(!hasError());
}


bool XflScriptReader::readThreadingOptions()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        //level 2
        if(name().compare(QString("Allow_Multithreading"), Qt::CaseInsensitive)==0)
        {
            m_bMultiThreading = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("Thread_Priority"), Qt::CaseInsensitive)==0)
        {
            QString priority = readElementText().trimmed();
            if     (priority.compare("Idle",         Qt::CaseInsensitive)==0) m_ThreadPriority=QThread::IdlePriority;
            else if(priority.compare("Lowest",       Qt::CaseInsensitive)==0) m_ThreadPriority=QThread::LowestPriority;
            else if(priority.compare("Low",          Qt::CaseInsensitive)==0) m_ThreadPriority=QThread::LowPriority;
            else if(priority.compare("Normal",       Qt::CaseInsensitive)==0) m_ThreadPriority=QThread::NormalPriority;
            else if(priority.compare("High",         Qt::CaseInsensitive)==0) m_ThreadPriority=QThread::HighPriority;
            else if(priority.compare("Highest",      Qt::CaseInsensitive)==0) m_ThreadPriority=QThread::HighestPriority;
            else if(priority.compare("TimeCritical", Qt::CaseInsensitive)==0) m_ThreadPriority=QThread::TimeCriticalPriority;
            else                                                              m_ThreadPriority=QThread::NormalPriority;
        }
        else if(name().compare(QString("max_threads"), Qt::CaseInsensitive)==0)
        {
            m_nMaxThreads = readElementText().trimmed().toInt();
        }
        else
            skipCurrentElement();
    }
    return !hasError();
}


bool XflScriptReader::readPlaneAnalysisOutput()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("make_polars_text_file"), Qt::CaseInsensitive)==0)
        {
            m_bOutputWPolarsText = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("make_oppoints"), Qt::CaseInsensitive)==0)
        {
            m_bMakePOpps = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("make_oppoints_text_file"), Qt::CaseInsensitive)==0)
        {
            m_bOutputPOppsText = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("export_oppoint_Cp"), Qt::CaseInsensitive)==0)
        {
            m_bExportPanelCp = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("export_stl_mesh"), Qt::CaseInsensitive)==0)
        {
            m_bExportStlMesh = xfl::stringToBool(readElementText());
        }
        else
            skipCurrentElement();
    }
    return !hasError();
}


bool XflScriptReader::readFoilPolars()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Polar_File_Name"), Qt::CaseInsensitive)==0)
        {
            m_PolarFileList.append(readElementText().trimmed());
        }
        else
            skipCurrentElement();
    }
    return true;
}


bool XflScriptReader::readPlanes()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Plane_File_Name"), Qt::CaseInsensitive)==0)
        {
            QString filename = readElementText().trimmed();
            if(!filename.isEmpty())
                m_PlaneFileList.append(filename);
        }
        else if(name().compare(QString("Process_All_Files"), Qt::CaseInsensitive)==0)
        {
            m_bLoadAllPlanes = xfl::stringToBool(readElementText().trimmed());
        }
        else
            skipCurrentElement();
    }

    return true;
}


bool XflScriptReader::readPlaneAnalysisFiles()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Analysis_File_Name"), Qt::CaseInsensitive)==0)
        {
            m_WPolarFileList.append(readElementText().trimmed());
        }
        else if(name().compare(QString("Process_All_Files"), Qt::CaseInsensitive)==0)
        {
            m_bRunAllPlaneAnalyses = xfl::stringToBool(readElementText().trimmed());
        }
        else
            skipCurrentElement();
    }

    return true;
}


bool XflScriptReader::readPlaneAnalysisData()
{
    m_T12Range.clear();
    m_T3Range.clear();
    m_T8Range.clear();
    m_T6Range.clear();
    m_T7Range.clear();
    m_T8Range.clear();

    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Alpha"), Qt::CaseInsensitive)==0)
        {
            double vmin=0, vmax=0, vinc=0;
            QStringList alphaList = readElementText().simplified().split(",");
            if(alphaList.length()>0) vmin = alphaList.at(0).toDouble();
            if(alphaList.length()>1) vmax = alphaList.at(1).toDouble();
            if(alphaList.length()>2) vinc = alphaList.at(2).toDouble();
            m_T12Range.append({true, vmin, vmax, vinc});
            m_T3Range.append({true, vmin, vmax, vinc});
        }
        else if(name().compare(QString("Control"), Qt::CaseInsensitive)==0)
        {
            double vmin=0, vmax=0, vinc=0;
            QStringList ctrlList = readElementText().simplified().split(",");
            if(ctrlList.length()>0) vmin = ctrlList.at(0).toDouble();
            if(ctrlList.length()>1) vmax = ctrlList.at(1).toDouble();
            if(ctrlList.length()>2) vinc = ctrlList.at(2).toDouble();
            m_T6Range.append({true, vmin, vmax, vinc});
            m_T7Range.append({true, vmin, vmax, vinc});
        }
        else if(name().compare(QString("T12_Range"), Qt::CaseInsensitive)==0)
        {
            double vmin=0, vmax=0, vinc=0;
            QStringList alphaList = readElementText().simplified().split(",");
            if(alphaList.length()>0) vmin = alphaList.at(0).toDouble();
            if(alphaList.length()>1) vmax = alphaList.at(1).toDouble();
            if(alphaList.length()>2) vinc = alphaList.at(2).toDouble();
            m_T12Range.append({true, vmin, vmax, vinc});
        }
        else if(name().compare(QString("T3_Range"), Qt::CaseInsensitive)==0)
        {
            double vmin=0, vmax=0, vinc=0;
            QStringList alphaList = readElementText().simplified().split(",");
            if(alphaList.length()>0) vmin = alphaList.at(0).toDouble();
            if(alphaList.length()>1) vmax = alphaList.at(1).toDouble();
            if(alphaList.length()>2) vinc = alphaList.at(2).toDouble();
            m_T3Range.append({true, vmin, vmax, vinc});
        }
        else if(name().compare(QString("T5_Range"), Qt::CaseInsensitive)==0)
        {
            double vmin=0, vmax=0, vinc=0;
            QStringList betaList = readElementText().simplified().split(",");
            if(betaList.length()>0) vmin = betaList.at(0).toDouble();
            if(betaList.length()>1) vmax = betaList.at(1).toDouble();
            if(betaList.length()>2) vinc = betaList.at(2).toDouble();
            m_T5Range.append({true, vmin, vmax, vinc});
        }
        else if(name().compare(QString("T6_Range"), Qt::CaseInsensitive)==0)
        {
            double vmin=0, vmax=0, vinc=0;
            QStringList alphaList = readElementText().simplified().split(",");
            if(alphaList.length()>0) vmin = alphaList.at(0).toDouble();
            if(alphaList.length()>1) vmax = alphaList.at(1).toDouble();
            if(alphaList.length()>2) vinc = alphaList.at(2).toDouble();
            m_T6Range.append({true, vmin, vmax, vinc});
        }
        else if(name().compare(QString("T7_Range"), Qt::CaseInsensitive)==0)
        {
            double vmin=0, vmax=0, vinc=0;
            QStringList alphaList = readElementText().simplified().split(",");
            if(alphaList.length()>0) vmin = alphaList.at(0).toDouble();
            if(alphaList.length()>1) vmax = alphaList.at(1).toDouble();
            if(alphaList.length()>2) vinc = alphaList.at(2).toDouble();
            m_T7Range.append({true, vmin, vmax, vinc});
        }
        else if(name().compare(QString("T8_Range"), Qt::CaseInsensitive)==0)
        {
            double vmin=0, vmax=0, vinc=0;
            QStringList oppoint = readElementText().simplified().split(",");
            if(oppoint.length()>0) vmin = oppoint.at(0).toDouble();
            if(oppoint.length()>1) vmax = oppoint.at(1).toDouble();
            if(oppoint.length()>2) vinc = oppoint.at(2).toDouble();
            m_T8Range.push_back({true, vmin, vmax, vinc});
        }
        else if(name().compare(QString("Viscous_Loop"), Qt::CaseInsensitive)==0)
        {
            readViscousLoopData();
        }
        else
            skipCurrentElement();
    }

    return true;
}


bool XflScriptReader::readViscousLoopData()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Enable"), Qt::CaseInsensitive)==0)
        {
            m_bViscousLoop = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("Relax_Factor"), Qt::CaseInsensitive)==0)
        {
            m_ViscRelaxFactor = readElementText().trimmed().toDouble();
        }
        else if(name().compare(QString("Init_Virtual_Twist"), Qt::CaseInsensitive)==0)
        {
            m_bViscInitVTwist = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("Alpha_Precision"), Qt::CaseInsensitive)==0)
        {
            m_ViscAlphaPrecision = readElementText().trimmed().toDouble();
        }
        else if(name().compare(QString("Max_Iterations"), Qt::CaseInsensitive)==0)
        {
            m_ViscMaxIterations = readElementText().trimmed().toInt();
        }
        else
            skipCurrentElement();
    }

    return true;
}


bool XflScriptReader::readFoilDatNames()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Foil_File_Name"), Qt::CaseInsensitive)==0)
        {
            QString foilName = readElementText().trimmed();
            m_RawFoilList.append(foilName);
            m_FoilDatList.append(foilName);
        }
        else
            skipCurrentElement();
    }

    return true;
}


bool XflScriptReader::readFoilAnalysisOptions()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Max_XFoil_Iterations"), Qt::CaseInsensitive)==0)
        {
            m_MaxXFoilIterations  = readElementText().trimmed().toInt();
        }
        else if(name().compare(QString("Repanel_Foils"), Qt::CaseInsensitive)==0)
        {
            m_bRepanelFoils  = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("Foil_Panels"), Qt::CaseInsensitive)==0)
        {
            m_NFoilPanels = readElementText().trimmed().toInt();
        }
        else
            skipCurrentElement();
    }
    return true;
}


bool XflScriptReader::readFoilAnalysisOutput()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        //level 2
        if(name().compare(QString("make_polars_bin_file"), Qt::CaseInsensitive)==0)
        {
            m_bOutputPolarsBin = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("make_polars_text_file"), Qt::CaseInsensitive)==0)
        {
            m_bOutputPolarsText = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("make_oppoints"), Qt::CaseInsensitive)==0)
        {
            m_bMakeOpps = xfl::stringToBool(readElementText());
        }
        else
            skipCurrentElement();
    }
    return !hasError();
}


bool XflScriptReader::readFoilBatchData()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Polar_Type"), Qt::CaseInsensitive)==0)
        {
            m_FoilPolarType  = xml::polarType(readElementText());
        }
        else if(name().compare(QString("Forced_Top_Transition"), Qt::CaseInsensitive)==0)
        {
            m_XtrTop = readElementText().trimmed().toDouble();
        }
        else if(name().compare(QString("Forced_Bottom_Transition"), Qt::CaseInsensitive)==0)
        {
            m_XtrBot = readElementText().trimmed().toDouble();
        }
        else if(name().compare(QString("Batch_Range"), Qt::CaseInsensitive)==0)
        {
            readFoilBatchRange();
        }
        else
            skipCurrentElement();
    }

    for(int ic=m_NCrit.size(); ic<m_Reynolds.size() ;ic++)
    {
        m_NCrit.push_back(9);
    }
    for(int ic=m_Mach.size(); ic<m_Reynolds.size() ;ic++)
    {
        m_Mach.push_back(0.0);
    }

    if(m_NCrit.size()>m_Reynolds.size()) m_NCrit.resize(m_Reynolds.size());
    if(m_Mach.size()>m_Reynolds.size())  m_Mach.resize(m_Reynolds.size());

    return !hasError();
}


bool XflScriptReader::readFoilType1234Polars()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Polar_Type"), Qt::CaseInsensitive)==0)
        {
            m_FoilPolarType  = xml::polarType(readElementText());
        }
        else if(name().compare(QString("Forced_Top_Transition"), Qt::CaseInsensitive)==0)
        {
            m_XtrTop = readElementText().trimmed().toDouble();
        }
        else if(name().compare(QString("Forced_Bottom_Transition"), Qt::CaseInsensitive)==0)
        {
            m_XtrBot = readElementText().trimmed().toDouble();
        }
        else if(name().compare(QString("Batch_Range"), Qt::CaseInsensitive)==0)
        {
            readFoilBatchRange();
        }
        else
            skipCurrentElement();
    }
    for(int ic=m_NCrit.size(); ic<m_Reynolds.size() ;ic++)
    {
        m_NCrit.push_back(9);
    }
    for(int ic=m_Mach.size(); ic<m_Reynolds.size() ;ic++)
    {
        m_Mach.push_back(0.0);
    }

    if(m_NCrit.size()>m_Reynolds.size()) m_NCrit.resize(m_Reynolds.size());
    if(m_Mach.size()>m_Reynolds.size())  m_Mach.resize(m_Reynolds.size());

    return !hasError();
}


bool XflScriptReader::readFoilBatchRange()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Alpha"), Qt::CaseInsensitive)==0)
        {
            m_Alpha.clear();
            QStringList AoaList = readElementText().simplified().split(",");
            for(int ir=0; ir<AoaList.count(); ir++)
            {
                if(AoaList.at(ir).length()>0) m_Alpha.append(AoaList.at(ir).toDouble());
            }
        }
        else if(name().compare(QString("Reynolds"), Qt::CaseInsensitive)==0)
        {
            m_Reynolds.clear();
            QStringList ReList = readElementText().simplified().split(",");
            for(int ir=0; ir<ReList.count(); ir++)
            {
                if(ReList.at(ir).length()>0) m_Reynolds.append(ReList.at(ir).toDouble());
            }
        }
        else if(name().compare(QString("NCrit"), Qt::CaseInsensitive)==0)
        {
            m_NCrit.clear();
            QStringList NCritList = readElementText().simplified().split(",");
            for(int ic=0; ic<NCritList.count(); ic++)
            {
                if(NCritList.at(ic).length()>0) m_NCrit.append(NCritList.at(ic).toDouble());
            }
        }
        else if(name().compare(QString("Mach"), Qt::CaseInsensitive)==0)
        {
            m_Mach.clear();
            QStringList MachList = readElementText().simplified().split(",");
            for(int im=0; im<MachList.count(); im++)
            {
                if(MachList.at(im).length()>0) m_Mach.append(MachList.at(im).toDouble());
            }
        }
        else if(name().compare(QString("Forced_Top_Transition"), Qt::CaseInsensitive)==0)
        {
            m_XtrTop = readElementText().trimmed().toDouble();
        }
        else if(name().compare(QString("Forced_Bottom_Transition"), Qt::CaseInsensitive)==0)
        {
            m_XtrBot = readElementText().trimmed().toDouble();
        }
        else
            skipCurrentElement();
    }
    return !hasError();
}


bool XflScriptReader::readBoatData()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Output"), Qt::CaseInsensitive)==0)
        {
            readBoatAnalysisOutput();
        }
        else if(name().compare(QString("Foil_Dat_Files"), Qt::CaseInsensitive)==0)
        {
            readFoilDatNames();
        }
        else if(name().compare(QString("Definition_Files"), Qt::CaseInsensitive)==0)
        {
            readBoats();
        }
        else if(name().compare(QString("Analysis_Files"), Qt::CaseInsensitive)==0)
        {
            readBoatAnalysisFiles();
        }
        else if(name().compare(QString("Analysis_Data"), Qt::CaseInsensitive)==0)
        {
            readBoatAnalysisData();
        }
        else
            skipCurrentElement();
    }
    return(!hasError());
}


bool XflScriptReader::readBoats()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Boat_File_Name"), Qt::CaseInsensitive)==0)
        {
            m_BoatFileList.append(readElementText().trimmed());
        }
        else if(name().compare(QString("Process_All_Files"), Qt::CaseInsensitive)==0)
        {
            m_bLoadAllBoats = xfl::stringToBool(readElementText().trimmed());
        }
        else
            skipCurrentElement();
    }
    return true;
}


bool XflScriptReader::readBoatAnalysisData()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Control"), Qt::CaseInsensitive)==0)
        {
            double vmin=0, vmax=0, vinc=0;
            QStringList ctrlList = readElementText().simplified().split(",");
            if(ctrlList.length()>0) vmin = ctrlList.at(0).toDouble();
            if(ctrlList.length()>1) vmax = ctrlList.at(1).toDouble();
            if(ctrlList.length()>2) vinc = ctrlList.at(2).toDouble();
            m_T6Range.append({true, vmin, vmax, vinc});
        }
        else
            skipCurrentElement();
    }
    return true;
}


bool XflScriptReader::readBoatAnalysisFiles()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Analysis_File_Name"), Qt::CaseInsensitive)==0)
        {
            m_BtPolarFileList.append(readElementText().trimmed());
        }
        else if(name().compare(QString("Process_All_Files"), Qt::CaseInsensitive)==0)
        {
            m_bRunAllBoatAnalyses = xfl::stringToBool(readElementText().trimmed());
        }
        else
            skipCurrentElement();
    }
    return true;
}


bool XflScriptReader::readBoatAnalysisOutput()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("make_polars_text_file"), Qt::CaseInsensitive)==0)
        {
            m_bOutputWPolarsText = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("make_oppoints"), Qt::CaseInsensitive)==0)
        {
            m_bMakeBtOpps = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("make_oppoints_text_file"), Qt::CaseInsensitive)==0)
        {
            m_bOutputPOppsText = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("export_oppoint_Cp"), Qt::CaseInsensitive)==0)
        {
            m_bExportPanelCp = xfl::stringToBool(readElementText());
        }
        else
            skipCurrentElement();
    }
    return !hasError();
}


