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

#include <QObject>
#include <QVector>
#include <QFile>
#include <QTextStream>


#include <api/analysisrange.h>
#include <api/t8opp.h>
#include <api/utils.h>
#include <api/fl5lib_global.h>

class Plane;
class PlanePolar;
class LLTTask;
class PlaneTask;
class Task3d;

class XflExecutor : public QObject
{
    Q_OBJECT

    public:
        XflExecutor(QObject *pParent=nullptr);
        ~XflExecutor();

        void setEventDestination(QObject *pParent) {m_pEventDest=pParent;}

        void setAnalysisStatus(xfl::enumAnalysisStatus status) {m_AnalysisStatus=status;}
        bool isCancelled() const {return m_AnalysisStatus==xfl::CANCELLED;}
        bool isRunning()   const {return m_AnalysisStatus==xfl::RUNNING;}
        bool isPending()   const {return m_AnalysisStatus==xfl::PENDING;}
        bool isFinished()  const {return m_AnalysisStatus==xfl::FINISHED || m_AnalysisStatus==xfl::CANCELLED;}

        QString const &logFileName() const {return m_LogFileName;}

        bool setLogFile(QString const &logfilename, const QString &versionname);
        void setStdOutStream(bool bStdOut) {m_bStdOutStream = bStdOut;}
        void traceStdLog(std::string const &strMsg);
        void traceLog(QString const &strMsg);
        void closeLogFile();

        PlanePolar *makeWPolar(const QString &pathName, const QString &xmlWPolarDirPath);
        void makePlaneTasks(QString &logmsg);

        void setPlanes(QList<Plane*> &planes) {m_oaPlane=planes;}
        void makeWPolarArray(bool bRunAllPlaneAnalyses, QStringList &WPolarFileList,
                             QString const &xmlWPolarDirPath, bool bRecursiveDirScan, QString &logmsg);
        void makeWPolars(QMap<QString, bool> const&Analyses, QString const &xmlWPolarDirPath, QString &logmsg);
        void setWPolars(QList<PlanePolar*> &polarlist) {m_oaWPolar=polarlist;}

        void runLLTTask(LLTTask *pLLTTask);
        void runPanelTask(PlaneTask *pPlaneTask);
        void cleanUpLLTTask(LLTTask *pLLTTask);
        void cleanUpPlaneTask(PlaneTask *pPlaneTask);

        void runPlaneAnalyses();

        void setMakePOpps(bool b) {m_bMakePlaneOpps=b;}
        void setStabDerivatives(bool b) {m_bCompStabDerivatives=b;}

        QList<PlanePolar*> const & wPolars() const {return m_oaWPolar;}
        QList<Plane*> const& planes() const {return m_oaPlane;}
        QList<Task3d*> &planeTasks() {return m_PlaneExecList;}

        void setT12Range(QVector<AnalysisRange> const&range) {m_T12Range=range;}
        void setT3Range( QVector<AnalysisRange> const&range) {m_T3Range =range;}
        void setT5Range( QVector<AnalysisRange> const&range) {m_T5Range =range;}
        void setT6Range( QVector<AnalysisRange> const&range) {m_T6Range =range;}
        void setT7Range( QVector<AnalysisRange> const&range) {m_T7Range =range;}
        void setT8Range( std::vector<T8Opp>     const&range) {m_T8Range =range;}

        virtual void clearArrays();

    signals:
        void outputMessage(const QString &msg) const;
        void cancelTask();
        void taskFinished();
        void taskStarted(int);

    public slots:
        void onRunExecutor();
        void onCancel();

    protected:

        QObject *m_pEventDest;

        QFile *m_pXFile;
        QString m_LogFileName;
        QTextStream m_OutLogStream;

        bool m_bStdOutStream;

        bool m_bMakePlaneOpps;
        bool m_bCompStabDerivatives;

        int m_nTaskStarted, m_nTaskDone;

        xfl::enumAnalysisStatus m_AnalysisStatus;

        QList<Plane*> m_oaPlane;
        QList<PlanePolar*> m_oaWPolar;
        QList<Task3d*> m_PlaneExecList;

        // analysis variables
        QVector<AnalysisRange> m_T12Range;
        QVector<AnalysisRange> m_T3Range;
        QVector<AnalysisRange> m_T5Range;
        QVector<AnalysisRange> m_T6Range;
        QVector<AnalysisRange> m_T7Range;
        std::vector<T8Opp>     m_T8Range;

};

