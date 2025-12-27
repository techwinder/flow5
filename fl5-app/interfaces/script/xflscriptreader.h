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

#include <QFile>
#include <QXmlStreamReader>
#include <QThread>

#include <api/enums_objects.h>
#include <api/analysisrange.h>
#include <api/t8opp.h>

class XflScriptReader : public QXmlStreamReader
{
    friend class XflScriptExec;

    public:
        XflScriptReader();

        bool setFileName(QString scriptFileName);

        bool readScript();

        bool readFoilData();
        bool readFoilDatNames();
        bool readFoilAnalysisFiles();
        bool readFoilAnalysisRange();
        bool readFoilBatchData();
        bool readFoilAnalysisOutput();
        bool readFoilAnalysisOptions();
        bool readFoilBatchRange();
        bool readFoilType1234Polars();

        bool readPlaneData();
        bool readPlanes();
        bool readPlaneAnalysisOutput();
        bool readPlaneAnalysisFiles();
        bool readPlaneAnalysisData();
        bool readViscousLoopData();
        bool readFoilPolars();

        bool readBoatData();
        bool readBoats();
        bool readBoatAnalysisOutput();
        bool readBoatAnalysisFiles();
        bool readBoatAnalysisData();

        bool readMetaData();
        bool readDirectoryData();
        bool readThreadingOptions();

    public:
        //access functions
        //Common settings

        void setprojectFileName(  QString const &path) {m_ProjectFileName=path;}
        void setoutputDirPath(    QString const &path) {m_OutputDirPath=path;}
        void setxmlPlaneDirPath(  QString const &path) {m_xmlPlaneDirPath=path;}
        void setxmlWPolarDirPath( QString const &path) {m_xmlWPolarDirPath=path;}
        void setdatFoilDirPath(   QString const &path) {m_datFoilDirPath=path;}
        void setxmlPolarDirPath(  QString const &path) {m_xmlPolarDirPath=path;}
        void setbinPolarDirPath(  QString const &path) {m_PolarBinDirPath=path;}
        void setXFoilPolarDirPath(QString const &path) {m_XFoilPolarDirPath=path;}

        QString const &projectFileName()   const {return m_ProjectFileName;}
        QString const &outputDirPath()     const {return m_OutputDirPath;}
        QString const &xmlPlaneDirPath()   const {return m_xmlPlaneDirPath;}
        QString const &xmlWPolarDirPath()  const {return m_xmlWPolarDirPath;}
        QString const &xmlBoatDirPath()    const {return m_xmlBoatDirPath;}
        QString const &xmlBtPolarDirPath() const {return m_xmlBtPolarDirPath;}
        QString const &datFoilDirPath()    const {return m_datFoilDirPath;}
        QString const &xmlPolarDirPath()   const {return m_xmlPolarDirPath;}
        QString const &binPolarDirPath()   const {return m_PolarBinDirPath;}
        QString const &xfoilPolarDirPath() const {return m_XFoilPolarDirPath;}

        bool outputPolarsText()  const {return m_bOutputPolarsText;}
        bool outputWPolarsText() const {return m_bOutputWPolarsText;}
        bool outputPOppsText()   const {return m_bOutputPOppsText;}
        bool exportPanelCp()     const {return m_bExportPanelCp;}
        bool exportStlMesh()     const {return m_bExportStlMesh;}
        bool bCsvTextOutput()    const {return m_bCsvOutput;}

        // Foil access functions
        QStringList const &foilDatList() const {return m_FoilDatList;}

        QVector<double> *ReList()    {return &m_Reynolds;}
        QVector<double> *MachList()  {return &m_Mach;}
        QVector<double> *NCritList() {return &m_NCrit;}

        bool isMultiThreading() const {return m_bMultiThreading;}

        double XtrTop() const {return m_XtrTop;}
        double XtrBot() const {return m_XtrBot;}

        int foilPolarType() {return m_FoilPolarType;}
        int maxXFoilIterations() const {return m_MaxXFoilIterations;}

        bool bFromZero() const {return m_bFromZero;}
        bool bAlphaSpec() const {return m_bAlphaSpec;}

        bool bMakeFoilOpps()  const {return m_bMakeOpps;}
        bool bMakePlaneOpps() const {return m_bMakePOpps;}
        bool bMakeBtOpps()    const {return m_bMakeBtOpps;}

        bool bMakeProjectFile() const {return m_bMakeProjectFile;}

        //Viscous loop
        bool   bViscousLooop()      const {return m_bViscousLoop;}
        bool   bViscInitVTwist()    const {return m_bViscInitVTwist;}
        double viscRelaxFactor()    const {return m_ViscRelaxFactor;}
        double viscAlphapRecision() const {return m_ViscAlphaPrecision;}
        int    viscMaxIterations()  const {return m_ViscMaxIterations;}

        //VPW
        int    VPWIterations()  const {return m_VPWIterations;}
        double VPWCoreSize()    const {return m_VPWCoreSize;}
        double VPWDiscardDist() const {return m_VPWDiscardDist;}

        bool bMultiThreading() const {return m_bMultiThreading;}
        int nMaxThreads() const {return m_nMaxThreads;}
        QThread::Priority threadPriority() const {return m_ThreadPriority;}

        bool bDoublePrecision() const {return m_bDoublePrecision;}

        bool bRecursiveDirScan() const {return m_bRecursiveDirScan;}

    private:
        QString m_ScriptFileName;

        QStringList m_FoilDatList;                    /**< The list of foils  >*/
        QStringList m_PolarFileList;               /**< The list of .plr files to load >*/
        QStringList m_RawFoilList;                 /**< The list of raw foils to analyze using the supplied analysis parameters >*/

        QStringList m_XmlFoilAnalysisList;         /**< The list of xml foil analysis files to load */

        std::vector<AnalysisRange> m_AlphaRange;
        std::vector<AnalysisRange> m_ClRange;
        std::vector<AnalysisRange> m_ReRange;
        std::vector<AnalysisRange> m_CtrlRange;

        QVector<double> m_Reynolds, m_NCrit, m_Mach; /** Type 123 polars */
        QVector<double> m_Alpha; /** Type 4 polars */
        double m_XtrTop, m_XtrBot;

        xfl::enumPolarType m_FoilPolarType;
        int m_MaxXFoilIterations;
        int m_NFoilPanels;
        bool m_bLoadAllFoils;
        bool m_bRunAllFoilAnalyses;
        bool m_bLoadAllPlanes;
        bool m_bRunAllPlaneAnalyses;
        bool m_bLoadAllBoats;
        bool m_bRunAllBoatAnalyses;
        bool m_bRepanelFoils;
        bool m_bMakeOpps, m_bAlphaSpec, m_bFromZero;
        QString m_PreLoadProjectFilePath;
        QString m_xmlPlaneDirPath, m_xmlWPolarDirPath;
        QString m_xmlBoatDirPath, m_xmlBtPolarDirPath;
        QString m_datFoilDirPath, m_xmlPolarDirPath, m_PolarBinDirPath, m_XFoilPolarDirPath;
        QString m_OutputDirPath;
        QString m_ProjectFileName;
        bool m_bOutputPolarsBin;
        bool m_bOutputPolarsText;
        bool m_bMakeProjectFile;
        int m_nMaxThreads;


        // Plane variables
        QStringList m_PlaneFileList;                   /**< the list of planes >*/
        QStringList m_WPolarFileList;                  /**< the list of plane analyses loaded from xml files >*/
        bool m_bMakePOpps;
        bool m_bCsvOutput;
        bool m_bOutputWPolarsText;
        bool m_bOutputPOppsText;
        bool m_bExportPanelCp;
        bool m_bExportStlMesh;

        // boat variables
        QStringList m_BoatFileList;                   /**< the list of boats >*/
        QStringList m_BtPolarFileList;                  /**< the list of boat analyses loaded from xml files >*/
        bool m_bMakeBtOpps;
        bool m_bOutputBtPolarsText;

        // analysis variables
        QVector<AnalysisRange> m_T12Range;
        QVector<AnalysisRange> m_T3Range;
        QVector<AnalysisRange> m_T5Range;
        QVector<AnalysisRange> m_T6Range;
        QVector<AnalysisRange> m_T7Range;
        std::vector<T8Opp> m_T8Range;

        // viscous loop
        bool m_bViscousLoop;
        bool m_bViscInitVTwist;
        double m_ViscRelaxFactor;
        double m_ViscAlphaPrecision;
        int m_ViscMaxIterations;

        //VPW
        int m_VPWIterations;
        double m_VPWCoreSize;
        double m_VPWDiscardDist;

        // other
        bool m_bRecursiveDirScan;
        bool m_bDoublePrecision;

        bool m_bMultiThreading;
        QThread::Priority m_ThreadPriority;
};

