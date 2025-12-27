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

#include <QTextStream>

#include <interfaces/script/xflexecutor.h>
#include <interfaces/script/xflscriptreader.h>

class MainFrame;
class PlaneXfl;
class PlanePolar;
class Foil;
class Polar;
class PlaneTask;
class Boat;
class BoatPolar;
class BoatTask;

struct FoilAnalysis;

class XflScriptExec : public XflExecutor
{
//    Q_OBJECT

    public:
        XflScriptExec();
        ~XflScriptExec();

        bool makeExportDirectories();
        bool makeFoils();
        bool readScript(const QString &xmlScriptFileName);
        void makeFoilAnalysisList();
        void rePanelFoil(Foil *pFoil);

        bool checkPlaneFoils(PlaneXfl *pPlane);
        bool loadFoilPolarFiles();
        bool loadXFoilPolarFiles();
        bool preLoadProject();


        QString projectFilePathName() const;
        QString const &outputDirPath()               const {return m_OutputPath;}
        QString const &foilPolarTextOutputDirPath()  const {return m_FoilPolarsTextPath;}
        QString const &foilPolarBinOutputDirPath()   const {return m_FoilPolarsBinPath;}

        bool outputPolarBin()   const {return m_pScriptReader->m_bOutputPolarsBin;}
        bool outputPolarText()  const {return m_pScriptReader->outputPolarsText();}
        bool outputWPolarText() const {return m_pScriptReader->outputWPolarsText();}
        bool outputPOppText()   const {return m_pScriptReader->outputPOppsText();}
        bool exportPanelCp()    const {return m_pScriptReader->exportPanelCp();}
        bool exportStlMesh()    const {return m_pScriptReader->exportStlMesh();}
        bool makeProjectFile()  const {return m_pScriptReader->bMakeProjectFile();}
        bool bCSVOutput()       const {return m_pScriptReader->bCsvTextOutput();}

        Polar *createPolar(const Foil *pFoil, double Re, double Mach, double NCrit,
                           double XtrTop=1.0, double XtrBot=1.0, xfl::enumPolarType polarType = xfl::T1POLAR);


    public slots:
        bool runScript(const QString &scriptpath);

    private:
        void clearArrays() override;
        void runFoilAnalyses();
        void cleanUpFoilAnalyses();
        void makePlanes();

        void makeBoats();
        void makeBoatAnalysisList();
        void makeBtPolarArray();
        BoatPolar *makeBtPolar(const QString &pathName);
        void runBoatAnalyses();
        void launchBoatTask(BoatTask *pBoatTask);
        void cleanUpBoatTask(BoatTask *pBoatTask);

    private:
        XflScriptReader *m_pScriptReader;

        QList<FoilAnalysis*> m_FoilExecList;

        QList <Foil*>  m_oaRawFoil;  /**< the list of Foils for which an analysis is requested */

        QList<Boat*> m_oaBoat;
        QList<BoatPolar*> m_oaBtPolar;
        QList<BoatTask*> m_BoatExecList;


        int m_nThreads;

        //export directory paths
        QString m_FoilPolarsBinPath, m_FoilPolarsTextPath, m_OutputPath;


        static QThread::Priority s_ThreadPriority;
};




