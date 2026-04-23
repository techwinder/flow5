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

// Visual studio bug override
//https://developercommunity.visualstudio.com/t/Visual-Studio-17100-Update-leads-to-Pr/10669759?sort=newest
//#define _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR

#include <QCoreApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>

#include <core/xflcore.h>
#include <interfaces/script/xflexecutor.h>

#include <api/flow5events.h>
#include <api/llttask.h>
#include <api/objects3d.h>
#include <api/planeopp.h>
#include <api/planetask.h>
#include <api/planexfl.h>
#include <api/task3d.h>
#include <api/trimesh.h>
#include <api/planepolar.h>
#include <modules/xplane/analysis/plpolarnamemaker.h>

#include <api/xmlplanepolarreader.h>


XflExecutor::XflExecutor(QObject *pParent) : QObject()
{
    m_pEventDest = pParent;
    m_pXFile = nullptr;

    m_OutLogStream.setDevice(nullptr);
    m_AnalysisStatus=xfl::PENDING;
    m_bStdOutStream = false;
    m_nTaskStarted = m_nTaskDone = 0;
    m_bMakePlaneOpps = false;
    m_bCompStabDerivatives = false;
}


XflExecutor::~XflExecutor()
{
    for(int ix=0; ix<m_PlaneExecList.count(); ix++)
    {
        if(m_PlaneExecList.at(ix)) delete m_PlaneExecList.at(ix);
    }
    m_PlaneExecList.clear();
    closeLogFile();
}


/** clear arrays prior to next run */
void XflExecutor::clearArrays()
{
    for(int ix=0; ix<m_PlaneExecList.count(); ix++)
    {
        if(m_PlaneExecList.at(ix)) delete m_PlaneExecList.at(ix);
    }
    m_PlaneExecList.clear();

    m_oaPlane.clear(); // don't delete objects which are owned by Objects3d
    m_oaWPolar.clear();

    m_T12Range.clear();
    m_T3Range.clear();
    m_T5Range.clear();
    m_T6Range.clear();
    m_T7Range.clear();
    m_T8Range.clear();
}



bool XflExecutor::setLogFile(QString const &logfilename, QString const &versionname)
{
    m_LogFileName = logfilename;
    m_pXFile = new QFile(m_LogFileName);
    if(!m_pXFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        traceLog("Could not create the log file\n Aborting\n");
        return false;
    }
    m_OutLogStream.setDevice(m_pXFile);
    m_OutLogStream << "\n";
    m_OutLogStream << versionname;
    m_OutLogStream << "\n";
    QDateTime dt = QDateTime::currentDateTime();
    QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");
    m_OutLogStream << str<<"\n";

    m_OutLogStream.flush();
    return true;
}


void XflExecutor::traceStdLog(std::string const &strMsg)
{
    traceLog(QString::fromStdString(strMsg));
}


void XflExecutor::traceLog(QString const &strMsg)
{
    if(m_bStdOutStream)
    {
        std::cout<<strMsg.toStdString();
    }

    if(m_OutLogStream.device())
    {
        m_OutLogStream << strMsg;
        m_OutLogStream.flush();
    }

    // rebroadcast the event in case we are running from the GUI
/*    if(m_pEventDest)
    {
        MessageEvent *pMsgEvent = new MessageEvent(strMsg.toStdString());
        qApp->postEvent(m_pEventDest, pMsgEvent);
    }*/

    emit outputMessage(strMsg);
}


void XflExecutor::onCancel()
{
    m_AnalysisStatus=xfl::CANCELLED;

//    Task2d::cancelAnalyses();
    Task3d::setCancelled(true);
    TriMesh::setCancelled(true); /// @todo whats-the-point?

    traceLog("\n_____________Analysis cancellation request received_____________\n\n");
    emit cancelTask();
}


/** Makes a PlanePolar object from the input file and sets the polar's name to be the file's name*/
PlanePolar* XflExecutor::makeWPolar(QString const &fileName, QString const &xmlWPolarDirPath)
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
        QFileInfo fiRel(xmlWPolarDirPath + QDir::separator() + fileName);
        if(fiRel.exists()) pathName = fiRel.filePath();
    }

    xmlFile.setFileName(pathName);

    if (pathName.isEmpty() || !xmlFile.open(QIODevice::ReadOnly))
    {
        QString strange = "   ...could not open the file: "+fileName;
        traceLog(strange+"\n");
        return nullptr;
    }

    XmlPlanePolarReader xwpReader(xmlFile);
    xwpReader.readXMLPolarFile();

    if(xwpReader.hasError())
    {
        QString strange = "   ...error reading the file: "+fileName;
        traceLog(strange+"\n");

        QString errorMsg;
        errorMsg = xwpReader.errorString() + "\n";
        strange = QString::asprintf("      error on line %d column %d\n", int(xwpReader.lineNumber()), int(xwpReader.columnNumber()));

        traceLog(errorMsg+strange);
        return nullptr;
    }

    // two files never have the same name so use this as the polar's name
    xwpReader.wpolar()->setName(fiAbs.completeBaseName().toStdString());

    return xwpReader.wpolar();
}


/** Making the WPolars for the script */
void XflExecutor::makeWPolarArray(bool bRunAllPlaneAnalyses, QStringList &WPolarFileList,
                                  QString const &xmlWPolarDirPath, bool bRecursiveDirScan, QString &logmsg)
{
    logmsg = "Adding the plane analyses from the xml files\n";

    m_oaWPolar.clear();

    if(bRunAllPlaneAnalyses)
    {
        QFileInfo fi(xmlWPolarDirPath);
        if(!fi.isDir())
        {
            logmsg += "      The directory "+xmlWPolarDirPath+ " does not exist\n";
        }
        else
        {
            QStringList filter = {"*.xml"};
            QStringList xmlFileList = xfl::findFiles(xmlWPolarDirPath, filter, bRecursiveDirScan);
            for(int i=0; i<xmlFileList.size(); i++)
            {
                if(!WPolarFileList.contains(xmlFileList.at(i)))
                    WPolarFileList.append(xmlFileList);
            }
        }
    }

    for(int iwp=0; iwp<WPolarFileList.count(); iwp++)
    {
        PlanePolar *pWPolar = makeWPolar(WPolarFileList.at(iwp), xmlWPolarDirPath);
        if(pWPolar)
        {
            if(pWPolar->planeName().length())
            {
                // this WPolar has been defined for a specific plane

                // make sure that AVL-type controls have the same size as the plane
                PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(Objects3d::planeAt(pWPolar->planeName()));
                if(pPlaneXfl)
                {
                    for(int ie=0; ie<pWPolar->nAVLCtrls(); ie++)
                        pWPolar->AVLCtrl(ie).resizeValues(pPlaneXfl->nAVLGains());
                }

                m_oaWPolar.append(pWPolar);
                Objects3d::addPlPolar(pWPolar);
                logmsg += "   the analysis " + QString::fromStdString(pWPolar->name())+
                          " has been added for " + QString::fromStdString(pWPolar->planeName())+"\n";
            }
            else
            {
                // no plane name has been defined for this polar, so make one copy for each available plane
                for(int ip=0; ip<m_oaPlane.size(); ip++)
                {
                    Plane const *pPlane = m_oaPlane.at(ip);
                    PlanePolar *pNewWPolar = new PlanePolar;
                    pNewWPolar->duplicateSpec(pWPolar);
                    pNewWPolar->setName(pWPolar->name());
                    pNewWPolar->setPlaneName(pPlane->name());
                    if(pNewWPolar->bProjectedDim())
                    {
                        pNewWPolar->setReferenceChordLength(pPlane->mac());
                        pNewWPolar->setReferenceSpanLength(pPlane->projectedSpan());
                        pNewWPolar->setReferenceArea(pPlane->projectedArea(false));
                    }
                    else if(pNewWPolar->bPlanformDim())
                    {
                        pNewWPolar->setReferenceChordLength(pPlane->mac());
                        pNewWPolar->setReferenceSpanLength(pPlane->planformSpan());
                        pNewWPolar->setReferenceArea(pPlane->planformArea(false));
                    }
                    else
                    {
                        //custom dims, leave the values read from xml unchanged
                    }

                    // make sure that AVL-type controls have the same size as the plane
                    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
                    if(pPlaneXfl)
                    {
                        for(int ie=0; ie<pNewWPolar->nAVLCtrls(); ie++)
                            pNewWPolar->AVLCtrl(ie).resizeValues(pPlaneXfl->nAVLGains());
                    }

                    m_oaWPolar.append(pNewWPolar);
                    Objects3d::addPlPolar(pNewWPolar);
                    logmsg += "   the analysis "+QString::fromStdString(pNewWPolar->name())+
                              " has been added for "+ QString::fromStdString(pPlane->name())+"\n";
                }
                delete pWPolar; // no further use
            }
        }
        else
        {
            logmsg += "   error reading the plane analysis file " + WPolarFileList.at(iwp) + "\n";
        }
    }

    for(int i=0; i<m_oaWPolar.size(); i++)
    {
        PlanePolar const *pWPolar = m_oaWPolar.at(i);
        if(pWPolar->planeName().length()!=0)
        {
            bool bPlane(false);
            for(int ip=0; ip<m_oaPlane.size(); ip++)
            {
                if(pWPolar->planeName().compare(m_oaPlane.at(ip)->name())==0)
                {
                    bPlane = true;
                    break;
                }
            }
            if(!bPlane)
            {
                logmsg += "   Warning: the polar "+QString::fromStdString(pWPolar->name())+
                        " is associated to plane "+QString::fromStdString(pWPolar->planeName())+
                        " which does not exist\n";
            }
        }
    }

    logmsg += "\n";
}


/** Making the WPolars for the batch analysis*/
void XflExecutor::makeWPolars(QMap<QString, bool> const&Analyses, QString const &xmlWPolarDirPath, QString &logmsg)
{
    logmsg = "Adding the plane analyses from the xml files\n";

    m_oaWPolar.clear();

    QFileInfo fiDir(xmlWPolarDirPath);
    if(!fiDir.isDir())
    {
        logmsg += "      The directory "+xmlWPolarDirPath+ " does not exist\n";
        return;
    }

    QMapIterator<QString, bool> it(Analyses);
    while (it.hasNext())
    {
        it.next();
        if(!it.value()) continue; //inactive

        PlanePolar *pWPolar = makeWPolar(it.key(), xmlWPolarDirPath);

        if(pWPolar)
        {
            if(pWPolar->planeName().length())
            {
                // this WPolar has been defined for a specific plane
                m_oaWPolar.append(pWPolar);
                Objects3d::addPlPolar(pWPolar);
                logmsg += "   the analysis " + QString::fromStdString(pWPolar->name())+
                          " has been added for "+ QString::fromStdString(pWPolar->planeName())+"\n";
                Plane const*pPlane = Objects3d::planeAt(pWPolar->planeName());
                if(pPlane)
                {
                    pWPolar->setPlane(pPlane);
                    if(pWPolar->bProjectedDim())
                    {
                        pWPolar->setReferenceChordLength(pPlane->mac());
                        pWPolar->setReferenceSpanLength(pPlane->projectedSpan());
                        pWPolar->setReferenceArea(pPlane->projectedArea(false));
                    }
                    else if(pWPolar->bPlanformDim())
                    {
                        pWPolar->setReferenceChordLength(pPlane->mac());
                        pWPolar->setReferenceSpanLength(pPlane->planformSpan());
                        pWPolar->setReferenceArea(pPlane->planformArea(false));
                    }
                    else
                    {
                        //custom dims, leave the values read from xml unchanged
                    }

                    // make sure that AVL-type controls have the same size as the plane
                    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
                    if(pPlaneXfl)
                    {
                        for(int ie=0; ie<pWPolar->nAVLCtrls(); ie++)
                            pWPolar->AVLCtrl(ie).resizeValues(pPlaneXfl->nAVLGains());
                    }
                }
            }
            else
            {
                // no plane name has been defined for this polar, so make one copy for each available plane
                for(int ip=0; ip<m_oaPlane.size(); ip++)
                {
                    Plane const *pPlane = m_oaPlane.at(ip);
                    PlanePolar *pNewWPolar = new PlanePolar;
                    pNewWPolar->duplicateSpec(pWPolar);
//                    pNewWPolar->setTheStyle(pPlane->theStyle());
                    pNewWPolar->setVisible(true);
                    pNewWPolar->setName(pWPolar->name());
                    pNewWPolar->setPlaneName(pPlane->name());
                    if(pNewWPolar->bProjectedDim())
                    {
                        pNewWPolar->setReferenceChordLength(pPlane->mac());
                        pNewWPolar->setReferenceSpanLength(pPlane->projectedSpan());
                        pNewWPolar->setReferenceArea(pPlane->projectedArea(false));
                    }
                    else if(pNewWPolar->bPlanformDim())
                    {
                        pNewWPolar->setReferenceChordLength(pPlane->mac());
                        pNewWPolar->setReferenceSpanLength(pPlane->planformSpan());
                        pNewWPolar->setReferenceArea(pPlane->planformArea(false));
                    }
                    else
                    {
                        //custom dims, leave the values read from xml unchanged
                    }

                    // make sure that AVL-type controls have the same size as the plane
                    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
                    if(pPlaneXfl)
                    {
                        for(int ie=0; ie<pNewWPolar->nAVLCtrls(); ie++)
                            pNewWPolar->AVLCtrl(ie).resizeValues(pPlaneXfl->nAVLGains());
                    }

                    m_oaWPolar.append(pNewWPolar);
                    Objects3d::addPlPolar(pNewWPolar);
                    logmsg += "   the analysis "+QString::fromStdString(pNewWPolar->name())+
                              " has been added for "+ QString::fromStdString(pPlane->name())+"\n";
                }
                delete pWPolar; // no further use
            }
        }
        else
        {
            logmsg += "   error reading the plane analysis file "+it.key()+ "\n";
        }
    }
    logmsg += "\n";
}


void XflExecutor::makePlaneTasks(QString &logmsg)
{
    logmsg = "Making the analysis pairs (plane, analysis)\n";

    m_PlaneExecList.clear();

    for(int ip=0; ip<m_oaPlane.size(); ip++)
    {
        Plane *pPlane = m_oaPlane.at(ip);
        if(pPlane)
        {
            for(int iwp=0; iwp<m_oaWPolar.count(); iwp++)
            {
                PlanePolar *pWPolar = m_oaWPolar.at(iwp);

                if(pWPolar->name().length()==0)
                {
                    pWPolar->setName(PlPolarNameMaker::makeName(pPlane, pWPolar).toStdString());
                }

                if(pWPolar && pWPolar->planeName().compare(pPlane->name())==0)
                {
                    if(pWPolar->isLLTMethod())
                    {
                        PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(pPlane);
                        if(!pPlaneXfl)
                        {
                            traceLog("***LLT is only available for xfl-type planes***\n\n");
                        }
                        else
                        {
                            std::vector<double> values, sorteduniquevalues;
                            for(int ia=0; ia<m_T12Range.size(); ia++)
                            {
                                std::vector<double> vals = m_T12Range.at(ia).values();
                                if(m_T12Range.at(ia).isActive())
                                    values.insert(values.end(), vals.begin(), vals.end());
                            }

                            //sort and remove duplicates
                            std::sort(values.begin(), values.end());
                            double dlast = -1.e10;
                            for(uint i=0; i<values.size(); i++)
                            {
                                if(fabs(values.at(i)-dlast)>1.e-6) sorteduniquevalues.push_back(values.at(i));
                                dlast = values.at(i);
                            }

                             bool bValidRange = sorteduniquevalues.size()>0 ? true : false;

                            if(!bValidRange)
                            {
                                traceLog("   no operating points defined for polar "+QString::fromStdString(pWPolar->name())+"... skipping\n\n");
                            }
                            else
                            {
                                LLTTask *pLLTTask = new LLTTask;
                                pLLTTask->setObjects(pPlaneXfl, pWPolar);
                                pLLTTask->setLLTRange(sorteduniquevalues);
                                // pLLTTask->initializeGeom(); done when running the analysis
                                m_PlaneExecList.append(pLLTTask);
                            }
                        }
                    }
                    else
                    {
                        PlaneTask *pPlaneTask = new PlaneTask;
                        pPlaneTask->setObjects(pPlane, pWPolar);
                        pPlaneTask->setComputeDerivatives(m_bCompStabDerivatives);

                        bool bValidRange(false);
                        std::vector<double> values, sorteduniquevalues;
                        std::vector<T8Opp> xrange;
                        switch(pWPolar->type())
                        {
                            case xfl::T2POLAR:
                            case xfl::T1POLAR:
                            {
                                for(int ia=0; ia<m_T12Range.size(); ia++)
                                {
                                    if(m_T12Range.at(ia).isActive())
                                    {
                                        std::vector<double> vals = m_T12Range.at(ia).values();
                                        values.insert(values.begin(), vals.begin(), vals.end());
                                    }
                                }

                                //sort and remove duplicates
                                std::sort(values.begin(), values.end());
                                double dlast = -1.e10;
                                for(uint i=0; i<values.size(); i++)
                                {
                                    if(fabs(values.at(i)-dlast)>1.e-6) sorteduniquevalues.push_back(values.at(i));
                                    dlast = values.at(i);
                                }

                                pPlaneTask->setOppList(sorteduniquevalues);
                                bValidRange = sorteduniquevalues.size()>0 ? true : false;
                                break;
                            }
                            case xfl::T3POLAR:
                            {
                                for(int ia=0; ia<m_T3Range.size(); ia++)
                                {
                                    if(m_T3Range.at(ia).isActive())
                                    {
                                        std::vector<double> vals = m_T3Range.at(ia).values();
                                        values.insert(values.begin(), vals.begin(), vals.end());
                                    }
                                }

                                //sort and remove duplicates
                                std::sort(values.begin(), values.end());
                                double dlast = -1.e10;
                                for(uint i=0; i<values.size(); i++)
                                {
                                    if(fabs(values.at(i)-dlast)>1.e-6) sorteduniquevalues.push_back(values.at(i));
                                    dlast = values.at(i);
                                }

                                pPlaneTask->setOppList(sorteduniquevalues);
                                bValidRange = sorteduniquevalues.size()>0 ? true : false;
                                break;
                            }
                            case xfl::T5POLAR:
                            {
                                for(int ia=0; ia<m_T5Range.size(); ia++)
                                {
                                    if(m_T5Range.at(ia).isActive())
                                    {
                                        std::vector<double> vals = m_T5Range.at(ia).values();
                                        values.insert(values.begin(), vals.begin(), vals.end());
                                    }
                                }

                                //sort and remove duplicates
                                std::sort(values.begin(), values.end());
                                double dlast = -1.e10;
                                for(uint i=0; i<values.size(); i++)
                                {
                                    if(fabs(values.at(i)-dlast)>1.e-6) sorteduniquevalues.push_back(values.at(i));
                                    dlast = values.at(i);
                                }

                                pPlaneTask->setOppList(sorteduniquevalues);
                                bValidRange = sorteduniquevalues.size()>0 ? true : false;
                                break;
                            }
                            case xfl::T6POLAR:
                            {
                                for(int ia=0; ia<m_T6Range.size(); ia++)
                                {
                                    if(m_T6Range.at(ia).isActive())
                                    {
                                        std::vector<double> vals = m_T6Range.at(ia).values();
                                        values.insert(values.begin(), vals.begin(), vals.end());
                                    }
                                }

                                //sort and remove duplicates
                                std::sort(values.begin(), values.end());
                                double dlast = -1.e10;
                                for(uint i=0; i<values.size(); i++)
                                {
                                    if(fabs(values.at(i)-dlast)>1.e-6) sorteduniquevalues.push_back(values.at(i));
                                    dlast = values.at(i);
                                }

                                pPlaneTask->setCtrlOppList(sorteduniquevalues);

                                pWPolar->resetAngleRanges(pPlane);

                                bValidRange = sorteduniquevalues.size()>0 ? true : false;

                                break;
                            }
                            case xfl::T7POLAR:
                            {
                                for(int ia=0; ia<m_T7Range.size(); ia++)
                                {
                                    if(m_T7Range.at(ia).isActive())
                                    {
                                        std::vector<double> vals = m_T7Range.at(ia).values();
                                        values.insert(values.begin(), vals.begin(), vals.end());
                                    }
                                }

                                //sort and remove duplicates
                                std::sort(values.begin(), values.end());
                                double dlast = -1.e10;
                                for(uint i=0; i<values.size(); i++)
                                {
                                    if(fabs(values.at(i)-dlast)>1.e-6) sorteduniquevalues.push_back(values.at(i));
                                    dlast = values.at(i);
                                }

                                pPlaneTask->setStabOppList(sorteduniquevalues);
                                bValidRange = sorteduniquevalues.size()>0 ? true : false;
                                break;
                            }
                            case xfl::T8POLAR:
                            {
                                for(uint ia=0; ia<m_T8Range.size(); ia++)
                                {
                                    if(m_T8Range.at(ia).isActive()) xrange.push_back(m_T8Range.at(ia));
                                }
                                pPlaneTask->setT8OppList(xrange);
                                bValidRange = xrange.size()>0 ? true : false;
                                break;
                            }
                            default:
                            {
                                break;
                            }
                        }

                        if(!bValidRange)
                        {
                            traceLog(QString("   no operating points defined for polar "+QString::fromStdString(pWPolar->name())+"... skipping\n\n"));
                            delete pPlaneTask;
                        }
                        else
                        {
                            m_PlaneExecList.append(pPlaneTask);
                        }
                    }

                    logmsg += "   added analysis for ("+QString::fromStdString(pPlane->name())+
                            ", "+QString::fromStdString(pWPolar->name())+")\n";
                }
            }
        }
    }

    QString strange;
    strange = QString::asprintf("   Made %d valid analysis pairs (plane, polar) to run\n\n", int(m_PlaneExecList.size()));
    logmsg += strange;
}


void XflExecutor::onRunExecutor()
{
    runPlaneAnalyses();
    emit taskFinished();
}


void XflExecutor::runPlaneAnalyses()
{
    QString strong;
    m_AnalysisStatus = xfl::RUNNING;

    // since each task makes use of all allowed threads,
    // no point in parallelizing the tasks;
    // each task is run in sequence asynchronously

    for(int ia=0; ia<m_PlaneExecList.size(); ia++)
    {
        Task3d *pTask = m_PlaneExecList.at(ia);
//        pTask->setEventDestination(m_pEventDest); // send messages straight to the parent dialog or window
//        pTask->setEventDestination(nullptr);
//        connect(pTask, &Task3d::outputMessage, this, &XflExecutor::traceLog);
//        connect(this, SIGNAL(cancelTask()), pTask, SLOT(onCancel()));

        if(pTask)
        {
            pTask->setKeepOpps(m_bMakePlaneOpps);
        }

        PlaneTask *pPlaneTask = dynamic_cast<PlaneTask*>(pTask);
        LLTTask *pLLTTask = dynamic_cast<LLTTask*>(pTask);

        if(pPlaneTask)
        {
            strong = "Launching plane analysis: " + QString::fromStdString(pPlaneTask->plane()->name()) + " / " + QString::fromStdString(pPlaneTask->wPolar()->name()) + "\n";
            traceLog(strong);

            emit taskStarted(ia);
            runPanelTask(pPlaneTask);
            cleanUpPlaneTask(pPlaneTask);
        }
        else if(pLLTTask)
        {
            strong = "Launching plane analysis: " + QString::fromStdString(pLLTTask->plane()->name()) + " / " + QString::fromStdString(pLLTTask->wPolar()->name()) + "\n";
            traceLog(strong);

            emit taskStarted(ia);
            runLLTTask(pLLTTask);

            cleanUpLLTTask(pLLTTask);
        }
        if(isCancelled()) break;
    }

    m_AnalysisStatus = xfl::FINISHED;

//    qApp->postEvent(m_pEventDest, new QEvent(TASK3D_END_EVENT));
}


void XflExecutor::closeLogFile()
{
    m_OutLogStream.setDevice(nullptr);
    if(m_pXFile)
    {
        m_pXFile->close(); delete m_pXFile; m_pXFile=nullptr;
    }
}


void XflExecutor::runLLTTask(LLTTask *pLLTTask)
{
    pLLTTask->initializeAnalysis();
    pLLTTask->run();
}


void XflExecutor::runPanelTask(PlaneTask *pPlaneTask)
{
    // set the appropriate mesh for this task
    bool bThickSurfaces = true;

    PlanePolar *pWPolar = pPlaneTask->wPolar();

    if(pWPolar->isVLM())         bThickSurfaces = false;
    if(pWPolar->bThinSurfaces()) bThickSurfaces = false;

    pPlaneTask->setComputeDerivatives(m_bCompStabDerivatives);

    Plane * pPlane = pPlaneTask->plane();
    //and make the mesh
    if(pPlane->isXflType() && pWPolar->isQuadMethod())
    {
        PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(pPlane);
        pPlaneXfl->makeQuadMesh(bThickSurfaces, pWPolar->bIgnoreBodyPanels());
        if(!pWPolar->bVortonWake())
        {
            pPlaneXfl->refQuadMesh().makeWakePanels(pWPolar->NXWakePanel4(), pWPolar->wakePanelFactor(), pWPolar->wakeLength(),
                                                    Vector3d(1.0,0,0), true);
        }
        else
        {
            pPlaneXfl->refQuadMesh().makeWakePanels(3, 1, pWPolar->bufferWakeLength(), Vector3d(1.0,0,0), false);
        }
    }
    if(isCancelled()) return;

    if(pPlane->isXflType() && pWPolar->isTriangleMethod())
    {
        pPlane->makeTriMesh(bThickSurfaces);
        /** @todo check wake panel alignment with wind axes for t6 polars */
        pPlane->refTriMesh().makeWakePanels(pWPolar->NXWakePanel4(), pWPolar->wakePanelFactor(), pWPolar->wakeLength(), Vector3d(1.0,0,0), true);
    }

    // set the active mesh
    pPlane->restoreMesh();
    if(isCancelled()) return;

    std::thread p(&PlaneTask::run, pPlaneTask);
    while(!pPlaneTask->isFinished())
    {
        // Access the Q under the lock:
        std::unique_lock<std::mutex> lck(pPlaneTask->m_mtx);
        pPlaneTask->m_cv.wait(lck, [pPlaneTask]() {return !pPlaneTask->m_theMsgQueue.empty();} );
        VPWReport report = pPlaneTask->m_theMsgQueue.front();
        pPlaneTask->m_theMsgQueue.pop();

        // forward to the UI thread for user notification
//        qApp->postEvent(m_pEventDest, new MessageEvent(report.message()));
        traceStdLog(report.message());
    }

    p.join();
}


void XflExecutor::cleanUpLLTTask(LLTTask *pLLTTask)
{
    //The WPolar has been populated with results by the LLTTask
    //Store the POpps if requested

    for(uint iPOpp=0; iPOpp<pLLTTask->planeOppList().size(); iPOpp++)
    {
        //add the data to the polar object
        PlaneOpp *pPOpp = pLLTTask->planeOppList().at(iPOpp);
        if(m_bMakePlaneOpps && !pPOpp->isOut())
            Objects3d::insertPlaneOpp(pPOpp);
        else
        {
            delete pPOpp;
            pPOpp = nullptr;
        }
    }

    QString strong;

    if (!pLLTTask->isCancelled() && !pLLTTask->hasErrors())
        strong = "\nLLT analysis completed successfully\n";
    else if (pLLTTask->hasErrors())
        strong = "\nLLT analysis completed ... Errors encountered\n";

    traceLog(strong+"\n");
}


void XflExecutor::cleanUpPlaneTask(PlaneTask *pPlaneTask)
{
    QString strong;

    if (!pPlaneTask->isCancelled() && !pPlaneTask->hasErrors())
        strong = "\nPanel analysis completed successfully\n";
    else if (pPlaneTask->hasErrors())
        strong = "\nPanel analysis completed ... Errors encountered\n";

    traceLog(strong+"\n");
}



