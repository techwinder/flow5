/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QApplication>
#include <QFileDialog>
#include <QClipboard>

#include "graph_globals.h"
#include <xflcore/saveoptions.h>
#include <xflgraph/globals/graphsvgwriter.h>


void exportGraphToSvg(GraphWt const *pGraphWt, Graph const *pGraph, QString &tempfilepath)
{
    tempfilepath.clear();

    QString FileName = pGraph->name()+".svg";
    FileName.replace("/", " ");

    FileName = QFileDialog::getSaveFileName(nullptr, "Export graph to SVG",
                                            SaveOptions::lastExportDirName()+"/"+FileName,
                                            "SVG file (*.svg)");

    if(!FileName.length()) return;

    QFileInfo fi(FileName);
    if(fi.suffix().isEmpty() || fi.suffix().compare("svg", Qt::CaseInsensitive)!=0)
        FileName+=".svg";

    SaveOptions::setLastExportDirName(fi.path());


    tempfilepath = QDir::toNativeSeparators(FileName);

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    GraphSVGWriter svgwriter(XFile);
    svgwriter.writeGraph(pGraphWt, pGraph);

    XFile.close();
}


void exportGraphDataToFile(Graph const*pGraph)
{
    QString FileName = pGraph->name().trimmed();
    FileName = FileName.replace(' ', '_');

    QString m_GraphExportFilter = "Comma Separated Values (*.csv)";


    FileName = QFileDialog::getSaveFileName(nullptr, "Export graph", SaveOptions::lastExportDirName()+"/"+FileName,
                                            "Text File (*.txt);;Comma Separated Values (*.csv)",
                                            &m_GraphExportFilter);

    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastExportDirName(FileName.left(pos));

    bool bCSV = false;
    if(m_GraphExportFilter.indexOf("*.txt")>0)
    {
        SaveOptions::setExportFileType(xfl::TXT);
        if(FileName.indexOf(".txt")<0) FileName +=".txt";
    }
    else if(m_GraphExportFilter.indexOf("*.csv")>0)
    {
        SaveOptions::setExportFileType(xfl::CSV);
        if(FileName.indexOf(".csv")<0) FileName +=".csv";
        bCSV = true;
    }

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    pGraph->toFile(XFile, bCSV);

    FileName = QDir::toNativeSeparators(FileName);
    QApplication::clipboard()->setText(FileName);
}
