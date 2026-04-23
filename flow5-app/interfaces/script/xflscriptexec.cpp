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


#include <QCoreApplication>
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureSynchronizer>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QThreadPool>


#include "xflscriptexec.h"


#include <api/analysisrange.h>
#include <api/boat.h>
#include <api/boatopp.h>
#include <api/boatpolar.h>
#include <api/boattask.h>
#include <api/fileio.h>
#include <api/fl5core.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <api/objects2d_globals.h>
#include <api/objects3d.h>
#include <api/objects_global.h>
#include <api/oppoint.h>
#include <api/panelanalysis.h>
#include <api/planeopp.h>
#include <api/planetask.h>
#include <api/planexfl.h>
#include <api/polar.h>
#include <api/sailobjects.h>
#include <api/utils.h>
#include <api/planepolar.h>
#include <api/xfoiltask.h>
#include <api/xmlboatreader.h>
#include <api/xmlbtpolarreader.h>
#include <api/xmlplanereader.h>
#include <api/xmlpolarreader.h>
#include <api/xmlplanepolarreader.h>

#include <core/xflcore.h>
#include <globals/mainframe.h>
#include <modules/xdirect/analysis/polarnamemaker.h>
#include <modules/xobjects.h>
#include <modules/xplane/analysis/plpolarnamemaker.h>

QThread::Priority XflScriptExec::s_ThreadPriority = QThread::NormalPriority;


XflScriptExec::XflScriptExec() : XflExecutor()
{
    m_nThreads = 0;
    m_pScriptReader = nullptr;
}


XflScriptExec::~XflScriptExec()
{
    if(m_pScriptReader)
    {
        delete m_pScriptReader;
        m_pScriptReader = nullptr;
    }


    for(int ix=0; ix<m_FoilExecList.size(); ix++)
    {
        if(m_FoilExecList.at(ix)) delete m_FoilExecList.at(ix);
    }
    m_FoilExecList.clear();

    for(int ix=0; ix<m_BoatExecList.count(); ix++)
    {
        if(m_BoatExecList.at(ix)) delete m_BoatExecList.at(ix);
    }
    m_BoatExecList.clear();
    closeLogFile();
}


void XflScriptExec::clearArrays()
{
    XflExecutor::clearArrays();

    for(int ix=0; ix<m_FoilExecList.size(); ix++)
    {
        if(m_FoilExecList.at(ix)) delete m_FoilExecList.at(ix);
    }
    m_FoilExecList.clear();

    for(int ix=0; ix<m_BoatExecList.count(); ix++)
    {
        if(m_BoatExecList.at(ix)) delete m_BoatExecList.at(ix);
    }
    m_BoatExecList.clear();

    // don't delete objects which are owned by Objects23d
    m_oaRawFoil.clear();
    m_oaBoat.clear();
    m_oaBtPolar.clear();

}


/**
 * Creates analysis tasks for the foils which have not been loaded by way of .plr files.
 * The analyses are built using the foil_analysis data in the script file.
 */
void XflScriptExec::makeFoilAnalysisList()
{
    traceLog("Making foil analysis list\n");

    m_FoilExecList.clear();

    if(m_pScriptReader->m_bRunAllFoilAnalyses)
    {
        QFileInfo fi(m_pScriptReader->xmlPolarDirPath());
        if(!fi.isDir())
        {
            traceLog("      The directory "+m_pScriptReader->xmlPolarDirPath()+ " does not exist\n");
        }
        else
        {
            // list file
            QStringList filter = {"*.xml"};
            QStringList xmlFileList = xfl::findFiles(m_pScriptReader->xmlPolarDirPath(), filter, m_pScriptReader->bRecursiveDirScan());
            m_pScriptReader->m_XmlFoilAnalysisList.append(xmlFileList);
        }
    }

    // run the analysis only for the "raw" foils
    for(int ip=0; ip<m_oaRawFoil.size(); ip++)
    {
        Foil *pFoil = m_oaRawFoil.at(ip);
        if(!pFoil) continue;

        // make batch analysis pairs

        // type 1 only
        for(int iPolar=0; iPolar<m_pScriptReader->m_Reynolds.size(); iPolar++)
        {
            Q_ASSERT(m_pScriptReader->m_Reynolds.size()==m_pScriptReader->m_Mach.size());
            Q_ASSERT(m_pScriptReader->m_Reynolds.size()==m_pScriptReader->m_NCrit.size());

            FoilAnalysis *pFoilAnalysis = new FoilAnalysis;
            Polar *pPolar = createPolar(pFoil,
                                        m_pScriptReader->m_Reynolds.at(iPolar),
                                        m_pScriptReader->m_Mach.at(iPolar),
                                        m_pScriptReader->m_NCrit.at(iPolar),
                                        m_pScriptReader->m_XtrTop,
                                        m_pScriptReader->m_XtrBot,
                                        xfl::T1POLAR);

            pPolar->setTheStyle(pFoil->theStyle());

            pFoilAnalysis->m_Foil.copy(pFoil);
            pFoilAnalysis->m_pPolar = pPolar;


            pFoilAnalysis->range = m_pScriptReader->m_AlphaRange;


            Objects2d::insertPolar(pPolar);
            m_FoilExecList.append(pFoilAnalysis);
            traceLog("   added analysis task for  ("+QString::fromStdString(pFoil->name())+", "+QString::fromStdString(pPolar->name())+")\n");
        }

        // make analysis pairs from xml analysis files
        for(int ip=0; ip< m_pScriptReader->m_XmlFoilAnalysisList.size(); ip++)
        {
            QString filename = m_pScriptReader->m_XmlFoilAnalysisList.at(ip);
            QFileInfo fi(filename);

            // is it an absolute path?
            bool bFound = fi.exists();
            if(!bFound)
            {
                // try in the specified directory path
                fi.setFile(QDir(m_pScriptReader->m_xmlPolarDirPath), filename);
                if(!fi.exists())
                {
                    QString strange = "Could not find the file: " + filename + "\n";
                    traceLog(strange);
                }
                else bFound = true;
            }
            if(bFound)
            {
                QFile xmlfile(fi.absoluteFilePath());
                if (!xmlfile.open(QIODevice::ReadOnly))
                {
                    QString strange = "Could not open the file: " + filename + "\n";
                    traceLog(strange);
                }
                else
                {
                    FoilAnalysis *pFoilAnalysis = new FoilAnalysis;
                    Polar *pPolar = new Polar;
                    XmlPolarReader polarReader(xmlfile, pPolar);
                    polarReader.readXMLPolarFile();

                    if(!pPolar->foilName().length())
                    {
                        // if no foil name has been defined in the analysis file, attach it to all foils
                        pPolar->setFoilName(pFoil->name());
                    }

                    if(polarReader.hasError())
                    {
                        QString errorMsg = polarReader.errorString() + QString("on line %1 column %2\n").arg(polarReader.lineNumber()).arg(polarReader.columnNumber());
                        errorMsg = filename + ": " + errorMsg;
                        traceLog(errorMsg);
                        delete pPolar;
                    }
                    else
                    {
                        if(pFoil->name().compare(pPolar->foilName())==0)
                        {
                            pFoilAnalysis->m_Foil.copy(pFoil);
                            pFoilAnalysis->m_pPolar = pPolar;
                            if(pPolar->type()<xfl::T4POLAR)
                            {
                                pFoilAnalysis->range = m_pScriptReader->m_AlphaRange;
                            }
                            else if(pPolar->isFixedLiftPolar())
                            {
/*                                pFoilAnalysis->vMin = m_pScriptReader->m_FoilReynoldsMin;
                                pFoilAnalysis->vMax = m_pScriptReader->m_FoilReynoldsMax;
                                pFoilAnalysis->vInc = m_pScriptReader->m_FoilReynoldsInc;*/
                            }
                            else if(pPolar->isControlPolar())
                            {
/*                                pFoilAnalysis->vMin = m_pScriptReader->m_FoilCtrlMin;
                                pFoilAnalysis->vMax = m_pScriptReader->m_FoilCtrlMax;
                                pFoilAnalysis->vInc = m_pScriptReader->m_FoilCtrlInc;*/
                            }

                            Objects2d::insertPolar(pPolar);
                            pPolar->setTheStyle(pFoil->theStyle());
                            m_FoilExecList.append(pFoilAnalysis);
                            traceLog("   added analysis task for  ("+QString::fromStdString(pFoil->name())+", "+QString::fromStdString(pPolar->name())+")\n");
                        }
                        else
                        {
                            delete pPolar;
                        }
                    }
                }
            }
        }
    }
}


bool XflScriptExec::loadFoilPolarFiles()
{
    if(m_pScriptReader->m_PolarFileList.count())
        traceLog("Adding foils and their polars from the .plr files\n");

    FileIO fileio;

    for(int ifo=0; ifo<m_pScriptReader->m_PolarFileList.count(); ifo++)
    {
        QString polarPathName;
        QFile plrFile;

        QFileInfo fiAbs(m_pScriptReader->m_PolarFileList.at(ifo));

        if(fiAbs.exists())
        {
            //try the absolute path
            polarPathName = fiAbs.filePath();
        }
        else
        {
            //try the relative path

            QStringList filter = {"*.plr"};
            bool bFound = xfl::findFile(m_pScriptReader->m_PolarFileList.at(ifo), m_pScriptReader->binPolarDirPath(), filter, true, polarPathName);
            if(!bFound) polarPathName.clear();

        }

        QFileInfo fi(polarPathName);
        if(!fi.exists())
        {
            QString strange = "   ...could not find the file: "+m_pScriptReader->m_PolarFileList.at(ifo);
            traceLog(strange+"\n");
            continue;
        }

        plrFile.setFileName(polarPathName);


        if (!plrFile.open(QIODevice::ReadOnly))
        {
            QString strange = "   ...Could not open the file: "+m_pScriptReader->m_PolarFileList.at(ifo);
            traceLog(strange+"\n");
        }
        else
        {
            std::vector<Foil*>foilList;
            std::vector<Polar*> polarList;

            objects::readPolarFile(plrFile, foilList, polarList);

            for(uint ifoil=0;ifoil<foilList.size(); ifoil++)
            {
                Foil *pFoil = foilList.at(ifoil);
                if(pFoil)
                {
                    traceLog("   added foil and polars for: "+QString::fromStdString(pFoil->name())+"\n");
                    Objects2d::insertThisFoil(pFoil);
                }
                else
                {
                    traceLog("   ...failed to add the foil from: "+polarPathName+"\n");
                }
            }

            for(uint ipolar=0; ipolar<polarList.size(); ipolar++)
            {
                Polar*pPolar = polarList.at(ipolar);
                Objects2d::insertPolar(pPolar);
            }
        }
    }
    traceLog("\n");
    return true;
}


bool XflScriptExec::loadXFoilPolarFiles()
{
    if(m_pScriptReader->xfoilPolarDirPath().isEmpty()) return true;

    traceLog("Adding XFoil polars from the .txt files\n");

    QString logmsg;
    QString strange;
    QStringList XFoilPolarList;

    QFileInfo fi(m_pScriptReader->xfoilPolarDirPath());
    if(!fi.isDir())
    {
        traceLog("      The directory "+m_pScriptReader->xfoilPolarDirPath()+ " does not exist\n");
    }
    else
    {
        QStringList filter = {"*.txt"};
        QStringList XFoilFileList = xfl::findFiles(m_pScriptReader->xfoilPolarDirPath(), filter, true);
        for(int i=0; i<XFoilFileList.size(); i++)
        {
            if(!XFoilPolarList.contains(XFoilFileList.at(i)))
                XFoilPolarList.append(XFoilFileList);
        }
    }

    for(int ifo=0; ifo<XFoilPolarList.count(); ifo++)
    {
        QFile XFoilFile;

        XFoilFile.setFileName(XFoilPolarList.at(ifo));

        Polar *pPolar = Objects3d::importXFoilPolar(XFoilFile, logmsg);
        if(pPolar)
        {
            pPolar->setLineWidth(Curve::defaultLineWidth());
            QString foilname = QString::fromStdString(pPolar->foilName());
            Foil *pFoil = Objects2d::foil(foilname.toStdString());
            if(!pFoil)
            {
                strange = "   ...no foil with the name "+foilname+" --> discarding "+XFoilFile.fileName();
                delete pPolar;
                logmsg += strange + "\n";
                continue;
            }

            pPolar->setTheStyle(pFoil->theStyle());

            Objects2d::insertPolar(pPolar);
            logmsg += ("   added the XFoil polar: "+ foilname +" / "+QString::fromStdString(pPolar->name()) +"\n");
        }
        else
        {
            logmsg += ("   ...failed to add the polar from "+XFoilPolarList.at(ifo)+"\n");
            continue;
        }
    }
    traceLog(logmsg + "\n");

    return true;
}


bool XflScriptExec::makeFoils()
{
    traceLog("Reading foil .dat files\n");

    if(m_pScriptReader->m_bLoadAllFoils)
    {
        QFileInfo fi(m_pScriptReader->datFoilDirPath());
        if(!fi.isDir())
        {
            traceLog("      The directory "+m_pScriptReader->datFoilDirPath()+ " does not exist\n");
        }
        else
        {
            QStringList filter = {"*.dat"};
            QStringList datFileList = xfl::findFiles(m_pScriptReader->datFoilDirPath(), filter, true);
            for(int i=0; i<datFileList.size(); i++)
            {
                if(!m_pScriptReader->m_FoilDatList.contains(datFileList.at(i)))
                    m_pScriptReader->m_FoilDatList.append(datFileList);
            }
        }
    }

    for(int ifo=0; ifo<m_pScriptReader->m_FoilDatList.count(); ifo++)
    {
        QString datPathName;
        QFileInfo fiAbs(m_pScriptReader->m_FoilDatList.at(ifo));

        if(fiAbs.exists())
        {
            //try the absolute path
            datPathName = fiAbs.filePath();
        }
        else
        {
            //try the relative path
            QStringList filter = {"*.dat"};
            bool bFound = xfl::findFile(m_pScriptReader->m_FoilDatList.at(ifo), m_pScriptReader->m_datFoilDirPath, filter, true, datPathName);
            if(!bFound) datPathName.clear();
        }


        QFileInfo fi(datPathName);
        if (!fi.exists() || !fi.isReadable())
        {
            QString strange = "   ...could not open the file "+m_pScriptReader->m_FoilDatList.at(ifo);
            traceLog(strange+"\n");
            continue;
        }
        else
        {
            int iLineError(0);

            Foil *pFoil = new Foil();
            if(objects::readFoilFile(datPathName.toStdString(), pFoil, iLineError))
            {
                pFoil->setLineWidth(Curve::defaultLineWidth());
                pFoil->setLineColor(xfl::randomfl5Color());
                if(m_pScriptReader->m_bRepanelFoils)
                {
                    rePanelFoil(pFoil);
                }

                Objects2d::insertThisFoil(pFoil);
                if(m_pScriptReader->m_RawFoilList.contains(m_pScriptReader->m_FoilDatList.at(ifo)))
                    m_oaRawFoil.append(pFoil);
                traceLog("   added foil: "+QString::fromStdString(pFoil->name())+"\n");
            }
            else
            {
                QString strange = "   ...failed to add the foil from file "+m_pScriptReader->m_FoilDatList.at(ifo)+"\n";
                strange += QString::asprintf("         read error on line %d\n", iLineError);
                traceLog(strange);
                delete pFoil;
                continue;
            }

        }
    }
    traceLog("\n");

    return true;
}


void XflScriptExec::makePlanes()
{
    if(m_pScriptReader->m_bLoadAllPlanes)
    {
        QFileInfo fi(m_pScriptReader->xmlPlaneDirPath());
        if(!fi.isDir())
        {
            traceLog("      The directory "+m_pScriptReader->xmlPlaneDirPath()+ " does not exist\n");
        }
        else
        {
            QStringList filter = {"*.xml"};
            QStringList xmlFileList = xfl::findFiles(m_pScriptReader->xmlPlaneDirPath(), filter, m_pScriptReader->bRecursiveDirScan());
            for(int i=0; i<xmlFileList.size(); i++)
            {
                if(!m_pScriptReader->m_PlaneFileList.contains(xmlFileList.at(i)))
                    m_pScriptReader->m_PlaneFileList.append(xmlFileList);
            }
        }
    }

    if(m_pScriptReader->m_PlaneFileList.count())
    {
        traceLog("Adding planes from the .xml files\n");
    }
    else
    {
        traceLog("No xml plane files to load\n");
    }

    m_oaPlane.clear();

    for(int ip=0; ip<m_pScriptReader->m_PlaneFileList.count(); ip++)
    {
        QString planePathName;
        QFile xmlFile;

        QFileInfo fiAbs(m_pScriptReader->m_PlaneFileList.at(ip));
        if(fiAbs.exists())
        {
            //try the absolute path
            planePathName = fiAbs.filePath();
        }
        else
        {
            //try the relative path
            QFileInfo fiRel(m_pScriptReader->xmlPlaneDirPath() + QDir::separator() + m_pScriptReader->m_PlaneFileList.at(ip));
            if(fiRel.exists()) planePathName = fiRel.filePath();
        }
        xmlFile.setFileName(planePathName);

        if (!xmlFile.open(QIODevice::ReadOnly))
        {
            QString strange = "   ...could not open the file " +xmlFile.fileName();
            traceLog(strange+"\n");
            continue;
        }

        QFileInfo fi(xmlFile);

        XmlPlaneReader planereader(xmlFile);

        if(planereader.readFile())
        {
            PlaneXfl *pPlane = planereader.plane();

            if(checkPlaneFoils(pPlane))
            {
                pPlane->makePlane(true, false, true);
                // if the plane's name is already used, discard the plane and emit a warning
                bool bExists = false;
                for(int ip=0; ip<m_oaPlane.size(); ip++)
                {
                    if(pPlane->name().compare(m_oaPlane.at(ip)->name())==0)
                    {
                        bExists = true;
                        traceLog("   The plane: "+QString::fromStdString(pPlane->name()) +" is defined multiple times... discarding redundant occurences\n");
                    }
                }
                if(!bExists)
                {
                    m_oaPlane.append(pPlane);
                    Objects3d::addPlane(pPlane);
                    traceLog("   added the plane: "+QString::fromStdString(pPlane->name())+" from file "+fi.fileName()+"\n");
                }
            }
            else
            {
                traceLog("      foils not found ...discarding this plane\n");
                delete pPlane;
            }
        }
        else
        {
            QString strange = "      ...failed to load the file "+xmlFile.fileName()+"\n";
            strange += QString("      ")+planereader.errorString() + "\n";
            QString errorMsg;
            errorMsg = QString::asprintf("      error on line %d column %d", int(planereader.lineNumber()), int(planereader.columnNumber()));
            strange += errorMsg + "\n";
            traceLog(strange);
        }
        traceLog("\n");
    }
    traceLog("\n");
}


bool XflScriptExec::readScript(QString const &xmlScriptPathName)
{
    traceLog("Reading script "+ xmlScriptPathName+"\n");

    QFile xmlFile(xmlScriptPathName);
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        QString strange = "   ...Could not open the file "+xmlScriptPathName;
        traceLog(strange+"\n");
        return false;
    }

    if(m_pScriptReader)
    {
        delete m_pScriptReader; // to reset all arrays if script is run from the UI
        m_pScriptReader = nullptr;
    }
    m_pScriptReader = new XflScriptReader;

    m_pScriptReader->setDevice(&xmlFile);

    if(!m_pScriptReader->readScript())
    {
        QString strange;
        strange = QString::asprintf("\nline %d column %d", int(m_pScriptReader->lineNumber()), int(m_pScriptReader->columnNumber()));
        QString errorMsg = m_pScriptReader->errorString() + strange;
        traceLog(errorMsg+"\n");
        return false;
    }
    else
    {
        traceLog("Script imported, no parsing error\n\n");
    }
    return true;
}


bool XflScriptExec::preLoadProject()
{
    if(!m_pScriptReader->m_PreLoadProjectFilePath.length()) return false;

    QFileInfo fi(m_pScriptReader->m_PreLoadProjectFilePath);
    QFile XFile(m_pScriptReader->m_PreLoadProjectFilePath);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange = "Could not open the project file: "+m_pScriptReader->m_PreLoadProjectFilePath+"\n\n";
        traceLog(strange);
        return false;
    }

    QDataStream ar(&XFile);
    bool bRead =  false;
    //        QString end = m_pScriptReader->m_PreLoadProjectFilePath.right(4).toLower();

    // dummy arguments;
    Polar plr;
    PlanePolar wplr;

    FileIO loader;

    if(fi.suffix().toLower()=="fl5")
    {
        bRead = loader.serializeProjectFl5(ar, false);
    }
    else if (fi.suffix().toLower()=="xfl")
    {
        bRead = loader.serializeProjectXfl(ar, false, &wplr);
    }
    else
    {
        bRead = false;
    }

    if(!bRead)
    {
        QString strange = "Error reading the project file: "+m_pScriptReader->m_PreLoadProjectFilePath+"\n\n";
        traceLog(strange);
        return false;
    }

    QString strange = "Pre-loaded the project file: "+m_pScriptReader->m_PreLoadProjectFilePath+"\n\n";
    traceLog(strange);

    return true;
}


/**
 * The script has been loaded, the objects have been built
 * The analyses have been defined
 * The consistency and completeness of the data has been checked
 * What are we waiting for? */
bool XflScriptExec::runScript(QString const &scriptpath)
{
    Task3d::setCancelled(false);
    QString logmsg, strange;

    if(!readScript(scriptpath))
    {
        traceLog("Error reading script...aborting\n");
        emit taskFinished();
        return false;

    }

    // clean up previous UI run
    clearArrays();


    if(!makeExportDirectories())
    {
        traceLog("Error making directories ...aborting\n");
        emit taskFinished();
        return false;
    }

    QFileInfo fi(m_pScriptReader->projectFileName());
    QString logfilename(m_OutputPath + QDir::separator() + fi.baseName() + ".log");
    if(!setLogFile(logfilename,  QString::fromStdString(fl5::versionName(true)))) return false;

    preLoadProject();
    if(isCancelled()) return false;

    makeFoils();
    if(isCancelled()) return false;

    makeFoilAnalysisList();
    if(isCancelled()) return false;

    if(m_FoilExecList.size())
    {
        runFoilAnalyses();
    }
    else traceLog("No foil analysis requested\n\n");


    loadFoilPolarFiles();
    if(isCancelled()) return false;

    // list available foils
    strange = "Available foils:\n";
    for(int i=0; i<Objects2d::nFoils(); i++)
    {
        strange += "   " + QString::fromStdString(Objects2d::foilAt(i)->name()) + "\n";
    }
    traceLog(strange+"\n");

    loadXFoilPolarFiles();

    if(isCancelled()) return false;
    traceLog("\n\n-----Starting plane analyses-----\n\n");

    makePlanes();
    if(isCancelled()) return false;

    makeWPolarArray(m_pScriptReader->m_bRunAllPlaneAnalyses, m_pScriptReader->m_WPolarFileList,
                    m_pScriptReader->m_xmlWPolarDirPath, m_pScriptReader->m_bRecursiveDirScan,
                    logmsg);
    traceLog(logmsg);
    if(isCancelled()) return false;

    PlaneTask::setViscousLoopSettings(m_pScriptReader->bViscInitVTwist(), m_pScriptReader->viscRelaxFactor(),
                                      m_pScriptReader->viscAlphapRecision(), m_pScriptReader->viscMaxIterations());

    setT12Range(m_pScriptReader->m_T12Range);
    setT3Range( m_pScriptReader->m_T3Range);
    setT5Range( m_pScriptReader->m_T5Range);
    setT6Range( m_pScriptReader->m_T6Range);
    setT7Range( m_pScriptReader->m_T7Range);
    setT8Range( m_pScriptReader->m_T8Range);

    makePlaneTasks(logmsg);
    traceLog(logmsg);
    if(isCancelled()) return false;

    PanelAnalysis::setMultiThread(    m_pScriptReader->m_bMultiThreading);
    PanelAnalysis::setMaxThreadCount( m_pScriptReader->m_nMaxThreads);
    PanelAnalysis::setDoublePrecision(m_pScriptReader->m_bDoublePrecision);

    m_bMakePlaneOpps = m_pScriptReader->bMakePlaneOpps();
    m_bCompStabDerivatives = m_pScriptReader->bCompStabDerivatives();

    runPlaneAnalyses();
    if(isCancelled()) return false;

    traceLog("_____Plane analyses completed_____\n\n");

    traceLog("\n\n-----Starting boat analyses-----\n\n");

    makeBoats();
    if(isCancelled()) return false;

    makeBtPolarArray();
    if(isCancelled()) return false;

    makeBoatAnalysisList();
    strange = QString::asprintf("Made %d valid analysis pairs (boat, polar) to run\n\n", int(m_BoatExecList.size()));
    traceLog(strange);
    if(isCancelled()) return false;

    runBoatAnalyses();
    if(isCancelled()) return false;

    traceLog("_____ Boat analyses completed_____\n\n");

    traceLog("\n");

    emit taskFinished();
    return true;
}


/** checks that all the foils specified in the plane definition files
 *  are loaded and available */
bool XflScriptExec::checkPlaneFoils(PlaneXfl *pPlane)
{
//    traceLog("Checking foil availability...\n");

    QStringList foilList, missingFoilList;
    for(std::string const &name : Objects2d::foilNames()) foilList.push_back(QString::fromStdString(name));

    for(int iw=0; iw<pPlane->nWings(); iw++)
    {
        WingXfl *pWing = pPlane->wing(iw);
        if(pWing)
        {
            for(int iSec=0; iSec<pWing->nSections(); iSec++)
            {
                QString rightFoilName = QString::fromStdString(pWing->section(iSec).rightFoilName());
                if(!foilList.contains(rightFoilName))
                {
                    if(!missingFoilList.contains(rightFoilName)) missingFoilList.append(rightFoilName);
                }
                if(!pWing->isSymmetric())
                {
                    QString leftFoilName = QString::fromStdString(pWing->section(iSec).leftFoilName());
                    if(!foilList.contains(leftFoilName))
                    {
                        if(!missingFoilList.contains(leftFoilName)) missingFoilList.append(leftFoilName);
                    }
                }
            }
        }
    }

    if(missingFoilList.size())
    {
        for(int iFoil=0; iFoil<missingFoilList.size(); iFoil++)
            traceLog("      ...missing foil "+missingFoilList.at(iFoil)+" for plane "+QString::fromStdString(pPlane->name())+"\n");
    }

    return missingFoilList.size()==0;
}


void XflScriptExec::runFoilAnalyses()
{
    QString strong;

    //    QThreadPool::globalInstance()->setExpiryTimeout(60000);//ms
    traceLog("\n\n");

    strong = "_____Starting foil analysis_____\n\n";
    traceLog(strong);

    m_nThreads = m_pScriptReader->m_nMaxThreads;
    QThreadPool::globalInstance()->setMaxThreadCount(m_nThreads);
    strong = QString::asprintf("Running with %d thread(s)\n", m_pScriptReader->m_nMaxThreads);
    traceLog(strong+"\n");

    m_nTaskDone = 0;
    m_nTaskStarted = 0;

    strong = QString::asprintf("Found %d (foil, polar) pairs to analyze.\n", int(m_FoilExecList.size()));
    traceLog(strong+"\n");

    XFoilTask::setCancelled(false);

/*    while(m_FoilExecList.size()>0 && !isCancelled())
    {
        if (QThreadPool::globalInstance()->activeThreadCount()<QThreadPool::globalInstance()->maxThreadCount())
        {
            startXFoilTaskThread(); // analyze a new pair
        }

        QThread::msleep(100);
//            QThreadPool::globalInstance()->start(pXFoilTask);
    }
    QThreadPool::globalInstance()->waitForDone();*/

    QFutureSynchronizer<void> futureSync;

    for(int i=0; i<m_FoilExecList.size(); i++)
    {
        XFoilTask *pXFoilTask = new XFoilTask();
//        pXFoilTask->setEventDestination(this);
        //take the last analysis in the array
        FoilAnalysis *pAnalysis = m_FoilExecList.at(i);

        pAnalysis->m_pPolar->setVisible(true);

        //initiate the task
        if(pAnalysis->m_pPolar->isType12())
            pXFoilTask->setAnalysisRanges(pAnalysis->range);
        pXFoilTask->initialize(pAnalysis, false);

        m_nTaskStarted++;
        std::string str = "Starting "+ pAnalysis->m_Foil.name()+" / "+ pAnalysis->m_pPolar->name()+"\n";
        traceStdLog(str);



#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        futureSync.addFuture(QtConcurrent::run(&XFoilTask::run, pXFoilTask));
#else
        QtConcurrent::run(pXFoilTask, &XFoilTask::run);
#endif
    }

    futureSync.waitForFinished();

    cleanUpFoilAnalyses();
    if(isCancelled()) strong = "\n_____Foil analysis cancelled_____\n";
    else              strong = "\n_____Foil analysis completed_____\n";
    traceLog(strong);
}



/**
 * Clean-up is performed when all the threads are terminated
 */
void XflScriptExec::cleanUpFoilAnalyses()
{
    XFoil::s_bCancel = false;

    for(int ia=m_FoilExecList.count()-1; ia>=0; ia--)
    {
        FoilAnalysis *pAnalysis = m_FoilExecList.at(ia);
        m_FoilExecList.removeAt(ia);
        delete pAnalysis;
    }
    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());
}


QString XflScriptExec::projectFilePathName() const
{
    QString projectFileName = m_pScriptReader->projectFileName();
    QFileInfo fi(projectFileName);

    return m_OutputPath + QDir::separator() + fi.fileName();
}


bool XflScriptExec::makeExportDirectories()
{
    bool bOK = true;
    QString projectFileName = m_pScriptReader->projectFileName();
    QFileInfo fi(projectFileName);

    m_OutputPath = m_pScriptReader->outputDirPath();
    m_OutputPath += QDir::separator() + fi.baseName();


    QDir outputDir(m_OutputPath);
    if(!outputDir.exists())
    {
        if(!outputDir.mkpath(m_OutputPath))
        {
            traceLog("Could not make the directory: "+m_OutputPath+"\n");
            bOK = false;
        }
    }

    m_FoilPolarsTextPath = m_OutputPath+QDir::separator()+"Foil_polars";
    QDir exportFoilPolarsDir(m_FoilPolarsTextPath);
    if(!exportFoilPolarsDir.exists())
    {
        if(!exportFoilPolarsDir.mkpath(m_FoilPolarsTextPath))
        {
            traceLog("Could not make the directory: "+m_FoilPolarsTextPath+"\n");
            bOK = false;
        }
    }

    if(m_pScriptReader->binPolarDirPath().length()) m_FoilPolarsBinPath = m_pScriptReader->binPolarDirPath();
    else                                          m_FoilPolarsBinPath = m_FoilPolarsTextPath;

    return bOK;
}


void XflScriptExec::rePanelFoil(Foil *pFoil)
{
    pFoil->initGeometry();

    traceLog("      done re-panelling foil: "+QString::fromStdString(pFoil->name())+"\n");
}


/**
 * Creates a polar object for a given set of specified input data
 * @param pFoil a pointer to the Foil object to which the Polar will be attached
 * @param Re  the value of the Reynolds number
 * @param Mach  the value of the Mach number
 * @param NCrit the value of the transition criterion
 * @return a pointer to the Polar object which has been created
 */
Polar * XflScriptExec::createPolar(Foil const *pFoil, double Re, double Mach, double NCrit, double XtrTop, double XtrBot, xfl::enumPolarType polarType)
{
    if(!pFoil) return nullptr;

    Polar *pNewPolar = new Polar;
    pNewPolar->setFoilName(pFoil->name());
    pNewPolar->setVisible(true);
    pNewPolar->setType(polarType);
    pNewPolar->setMach(Mach);
    pNewPolar->setNCrit(NCrit);
    pNewPolar->setXTripTop(XtrTop);
    pNewPolar->setXTripBot(XtrBot);

    switch (pNewPolar->type())
    {
        case xfl::T1POLAR:
            pNewPolar->setMaType(1);
            pNewPolar->setReType(1);
            break;
        case xfl::T2POLAR:
            pNewPolar->setMaType(2);
            pNewPolar->setReType(2);
            break;
        case xfl::T3POLAR:
            pNewPolar->setMaType(1);
            pNewPolar->setReType(3);
            break;
        case xfl::T4POLAR:
            pNewPolar->setMaType(1);
            pNewPolar->setReType(1);
            break;
        default:
            pNewPolar->setReType(1);
            pNewPolar->setMaType(1);
            break;
    }
    if(polarType!=xfl::T4POLAR)  pNewPolar->setReynolds(Re);
    else                               pNewPolar->setAoaSpec(0.0);

    pNewPolar->setName(PolarNameMaker::makeName(pNewPolar).toStdString());
    return pNewPolar;
}


void XflScriptExec::makeBoats()
{
    if(m_pScriptReader->m_bLoadAllBoats)
    {
        QStringList filter = {"*.xml"};
        QStringList xmlFileList = xfl::findFiles(m_pScriptReader->xmlBoatDirPath(), filter, m_pScriptReader->bRecursiveDirScan());
        for(int i=0; i<xmlFileList.size(); i++)
        {
            if(!m_pScriptReader->m_BoatFileList.contains(xmlFileList.at(i)))
                m_pScriptReader->m_BoatFileList.append(xmlFileList);
        }
    }

    if(m_pScriptReader->m_BoatFileList.count())
        traceLog("Adding Boats from the .xml files\n");
    else
        traceLog("No xml Boat files to load\n");

    m_oaBoat.clear();

    for(int ip=0; ip<m_pScriptReader->m_BoatFileList.count(); ip++)
    {
        QString BoatPathName;
        QFile xmlFile;

        traceLog("   Processing file " + m_pScriptReader->m_BoatFileList.at(ip) + "\n");
        QFileInfo fiAbs(m_pScriptReader->m_BoatFileList.at(ip));
        if(fiAbs.exists())
        {
            //try the absolute path
            BoatPathName = fiAbs.filePath();
        }
        else
        {
            //try the relative path
            QFileInfo fiRel(m_pScriptReader->xmlBoatDirPath() + QDir::separator() + m_pScriptReader->m_BoatFileList.at(ip));
            if(fiRel.exists()) BoatPathName = fiRel.filePath();
        }
        xmlFile.setFileName(BoatPathName);

        if (!xmlFile.open(QIODevice::ReadOnly))
        {
            QString strange = "   ...Could not open the file " +xmlFile.fileName();
            traceLog(strange+"\n");
            return;
        }

//        QFileInfo fi(xmlFile);

        XmlBoatReader BoatReader(xmlFile);

        if(BoatReader.readXMLBoatFile() && BoatReader.boat())
        {
            Boat *pBoat = BoatReader.boat();
            pBoat->makeRefTriMesh(false, m_pScriptReader->bMultiThreading()); /** @todo replace with !bIgnoreBodyPanels */
            m_oaBoat.append(pBoat);
            SailObjects::appendBoat(pBoat);
            traceLog("      The boat "+QString::fromStdString(pBoat->name())+" has been read successfully\n");
        }
        else
        {
            QString strange = "      ...failed to load the file "+xmlFile.fileName()+"\n";
            strange += QString("      ")+BoatReader.errorString() + "\n";
            QString errorMsg;
            if(BoatReader.hasError())
            {
                errorMsg = QString::asprintf("      error on line %d column %d", int(BoatReader.lineNumber()), int(BoatReader.columnNumber()));
            }
            else
            {
                errorMsg = "      no boat definition found in the file";
            }
            strange += errorMsg + "\n";
            traceLog(strange);
        }
        traceLog("\n");
    }
    traceLog("\n");
}


void XflScriptExec::makeBtPolarArray()
{
    m_oaBtPolar.clear();

    if(m_pScriptReader->m_bRunAllBoatAnalyses)
    {
        // list file
        QStringList filter = {"*.xml"};
        QStringList xmlFileList = xfl::findFiles(m_pScriptReader->xmlBtPolarDirPath(), filter, m_pScriptReader->bRecursiveDirScan());
        for(int i=0; i<xmlFileList.size(); i++)
        {
            if(!m_pScriptReader->m_BtPolarFileList.contains(xmlFileList.at(i)))
                m_pScriptReader->m_BtPolarFileList.append(xmlFileList);
        }
    }

    if(m_pScriptReader->m_BtPolarFileList.count())
        traceLog("\nAdding the Boat analyses from the xml files\n");

    for(int iwp=0; iwp<m_pScriptReader->m_BtPolarFileList.count(); iwp++)
    {
        BoatPolar *pBtPolar = makeBtPolar(m_pScriptReader->m_BtPolarFileList.at(iwp));
        if(pBtPolar)
        {
            traceLog("   the analysis file "+QString::fromStdString(pBtPolar->name())+ " has been read successfully.\n");
            if(pBtPolar->boatName().length())
            {
                // this BtPolar has been defined for a specific Boat
                m_oaBtPolar.append(pBtPolar);
                SailObjects::appendBtPolar(pBtPolar);
            }
            else
            {
                // no Boat name has been defined for this polar, so make one copy for each available Boat
                for(int ip=0; ip<m_oaBoat.size(); ip++)
                {
                    Boat const *pBoat = m_oaBoat.at(ip);
                    BoatPolar *pNewBtPolar = new BoatPolar;
                    pNewBtPolar->duplicateSpec(pBtPolar);
                    pNewBtPolar->setName(pBtPolar->name());
                    pNewBtPolar->setBoatName(pBoat->name());
                    m_oaBtPolar.append(pNewBtPolar);
                    SailObjects::appendBtPolar(pNewBtPolar);
                    traceLog("   the analysis "+QString::fromStdString(pNewBtPolar->name())+ " has been added for the Boat "+ QString::fromStdString(pBoat->name())+"\n");
                }
                delete pBtPolar; // no further use
            }
        }
        else
        {
            traceLog("   error reading the Boat analysis file "+m_pScriptReader->m_BtPolarFileList.at(iwp)+ "\n");
        }
    }
}


void XflScriptExec::makeBoatAnalysisList()
{
    traceLog("Making the analysis pairs (boat, analysis)\n");

    m_BoatExecList.clear();
    double vMin=0, vMax=0, vInc=0;

    for(int ip=0; ip<m_oaBoat.size(); ip++)
    {
        Boat *pBoat = m_oaBoat.at(ip);
        if(pBoat)
        {
            for(int iwp=0; iwp<m_oaBtPolar.count(); iwp++)
            {
                BoatPolar *pBtPolar = m_oaBtPolar.at(iwp);
                if(!pBtPolar) continue;

                if(pBtPolar->boatName().compare(pBoat->name())==0)
                {
                    if(m_pScriptReader->m_T6Range.size())
                    {
                        vMin = m_pScriptReader->m_T6Range.front().m_vStart;
                        vMax = m_pScriptReader->m_T6Range.front().m_vEnd;
                        vInc = m_pScriptReader->m_T6Range.front().m_vInc;
                    }

                    if(fabs(vInc)<0.001) vInc = 1.0;
                    if(vMax<vMin) vInc = -fabs(vInc);
                    std::vector<double> opplist;
                    for(double v=vMin; v<vMax; v+=vInc)
                    {
                        opplist.push_back(v);
                    }

                    BoatTask *pBoatTask = new BoatTask;

                    if(pBoatTask)
                    {
                        pBoatTask->setObjects(pBoat, pBtPolar);
                        pBoatTask->setAnalysisRange(opplist);

                        m_BoatExecList.append(pBoatTask);
                    }
                    traceLog("   added analysis for ("+QString::fromStdString(pBoat->name())+", "+QString::fromStdString(pBtPolar->name())+")\n");

                }
            }
        }
    }
    traceLog("\n");
}


void XflScriptExec::runBoatAnalyses()
{
    QString strong;

    PanelAnalysis::setMultiThread(m_pScriptReader->bMultiThreading());
    PanelAnalysis::setMaxThreadCount(m_pScriptReader->nMaxThreads());
    PanelAnalysis::setDoublePrecision(m_pScriptReader->bDoublePrecision());

    for(int ia=0; ia<m_BoatExecList.size(); ia++)
    {
        BoatTask *pBoatTask = m_BoatExecList.at(ia);
//        connect(this, SIGNAL(cancelTask()), pBoatTask, SLOT(onCancel()),         Qt::QueuedConnection);

        strong = "\n   Launching Boat analysis: " + QString::fromStdString(pBoatTask->boat()->name()) + " / " + QString::fromStdString(pBoatTask->btPolar()->name()) + "\n";
        traceLog(strong);

        launchBoatTask(pBoatTask);

        cleanUpBoatTask(pBoatTask);

//        disconnect(pBoatTask, SIGNAL(outputMsg(QString)), nullptr, nullptr);
        if(isCancelled()) break;
    }
}


void XflScriptExec::cleanUpBoatTask(BoatTask *pBoatTask)
{
    //The BtPolar has been populated with results by the BoatTask
    //Store the BoatOpps if requested
    for(uint iBtOpp=0; iBtOpp<pBoatTask->BtOppList().size(); iBtOpp++)
    {
        //add the data to the polar object
        BoatOpp *pBtOpp = pBoatTask->BtOppList().at(iBtOpp);
        if(m_pScriptReader->bMakeBtOpps())
            SailObjects::insertBtOpp(pBtOpp);
        else
        {
            delete pBtOpp;
            pBtOpp = nullptr;
        }
    }

    QString strong;

    if (!pBoatTask->isCancelled() && !pBoatTask->hasErrors())
        strong = "\nPanel analysis completed successfully\n";
    else if (pBoatTask->hasErrors())
        strong = "\nPanel Analysis completed ... Errors encountered\n";

    traceLog(strong+"\n");

}

void XflScriptExec::launchBoatTask(BoatTask *pBoatTask)
{
    // since boat tasks make use of all authorized threads,
    // each task is run in sequence synchronously

    Boat *pBoat = pBoatTask->boat();

    // set the active mesh
    pBoat->restoreMesh();

    pBoatTask->initializeTask(this);
    pBoatTask->run();
}


BoatPolar* XflScriptExec::makeBtPolar(QString const&fileName)
{
    QString pathName;
    QFile xmlFile;


    QFileInfo fiAbs(fileName);
    if(fiAbs.exists())
    {
        //try the absolute path
        pathName = fiAbs.filePath(); // same as fileName
    }
    else
    {
        //try the relative path
        QFileInfo fiRel( m_pScriptReader->xmlBtPolarDirPath() + QDir::separator() + fileName);
        if(fiRel.exists()) pathName = fiRel.filePath();
    }

    xmlFile.setFileName(pathName);

    if (pathName.isEmpty() || !xmlFile.open(QIODevice::ReadOnly))
    {
        QString strange = "   ...could not open the file: "+fileName;
        traceLog(strange+"\n");
        return nullptr;
    }
    else
    {
        XmlBtPolarReader xBtPlrReader(xmlFile);
        xBtPlrReader.readXMLPolarFile();

        if(xBtPlrReader.hasError())
        {
            QString strange = "   ...error reading the file: "+fileName;
            traceLog(strange+"\n");

            QString errorMsg;
            strange = QString::asprintf("   error on line %d column %d",int(xBtPlrReader.lineNumber()), int(xBtPlrReader.columnNumber()));
            errorMsg = xBtPlrReader.errorString() + " at " + strange + "\n";

            traceLog(errorMsg);
            return nullptr;
        }
        else
        {
            if(!xBtPlrReader.btPolar())  traceLog("   no valid analysis definition found in the file\n");
            return xBtPlrReader.btPolar();
        }
    }
}
