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


#include "foilsvgwriter.h"
#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <api/foil.h>

bool FoilSVGWriter::s_bSVGClosedTE=false;
bool FoilSVGWriter::s_bSVGFillFoil=true;
bool FoilSVGWriter::s_bSVGExportStyle=true;
double  FoilSVGWriter::s_SVGScaleFactor=10000.0;
double  FoilSVGWriter::s_SVGMargin=0.001;


FoilSVGWriter::FoilSVGWriter(QFile &XFile) : QXmlStreamWriter()
{
    setDevice(&XFile);
    setAutoFormatting(true);
}


void FoilSVGWriter::writeFoil(Foil const *pFoil)
{
    double xmin =  100.0;
    double xmax = -100.0;
    double ymin =  100.0;
    double ymax = -100.0;
    for(int i=0; i<pFoil->nBaseNodes(); i++)
    {
        xmin = std::min(xmin, pFoil->x(i));
        xmax = std::max(xmax, pFoil->x(i));

        ymin = std::min(ymin, pFoil->y(i));
        ymax = std::max(ymax, pFoil->y(i));
    }
    xmin *= s_SVGScaleFactor;
    xmax *= s_SVGScaleFactor;
    ymin *= s_SVGScaleFactor;
    ymax *= s_SVGScaleFactor;
    double margin = s_SVGMargin * s_SVGScaleFactor;
    double w = xmax-xmin + 2*margin;
    double h = ymax-ymin + 2*margin;


    writeStartDocument();

    writeStartElement("svg");
    {
        writeAttribute("xmlns", "http://www.w3.org/2000/svg");

        writeAttribute("width",   QString::asprintf("%dpt", int(w)));
        writeAttribute("height",  QString::asprintf("%dpt", int (h)));
        writeAttribute("viewbox", QString::asprintf("%d %d %d %d", 0, 0,int(w), int(h)));
        writeTextElement("title", QString::fromStdString(pFoil->name()));

        QString strange, str;

        double y =-pFoil->frontNode().y;
        strange = QString::asprintf("M%g %g ", pFoil->frontNode().x*s_SVGScaleFactor + margin, ymax + y*s_SVGScaleFactor + margin);

        for(int i=1; i<pFoil->nNodes(); i++)
        {
            y = -pFoil->y(i);
            str = QString::asprintf("L%g %g ", pFoil->x(i)*s_SVGScaleFactor + margin, ymax + y*s_SVGScaleFactor + margin);
            strange += str;
        }

        if(s_bSVGClosedTE) strange += "Z";

        writeStartElement("path");
        {
            writeAttribute("d", strange);

            QColor foilclr = xfl::fromfl5Clr(pFoil->lineColor());
            QColor fillclr(foilclr);
            fillclr.setAlpha(75);
            if(bSVGFillFoil()) strange = QString::asprintf("fill:%s; ",fillclr.name().toStdString().c_str());
            else                            strange="fill:none; ";
            str = QString::asprintf("stroke:%s; stroke-width:%d", foilclr.name().toStdString().c_str(), pFoil->lineWidth());
            strange += str;
            if(bSVGExportStyle()) writeAttribute("style", strange);
            else                               writeAttribute("style", QString());
        }
        writeEndElement(); //path
    }
    writeEndElement(); //svg
    writeEndDocument();
}


void FoilSVGWriter::loadSettings(QSettings &settings)
{
    settings.beginGroup("FoilSVGWriter");
    {

        setSVGClosedTE(   settings.value("SVGCloseTE",     bSVGClosedTE()).toBool());
        setSVGFillFoil(   settings.value("SVGFillFoil",    bSVGFillFoil()).toBool());
        setSVGExportStyle(settings.value("SVGExportStyle", bSVGExportStyle()).toBool());
        setSVGScaleFactor(settings.value("SVGScaleFactor", SVGScaleFactor()).toDouble());
        setSVGMargin(     settings.value("SVGMargin",      SVGMargin()).toDouble());

    }
    settings.endGroup();
}


void FoilSVGWriter::saveSettings(QSettings &settings)
{
    settings.beginGroup("FoilSVGWriter");
    {

        settings.setValue("SVGCloseTE",     bSVGClosedTE());
        settings.setValue("SVGFillFoil",    bSVGFillFoil());
        settings.setValue("SVGExportStyle", bSVGExportStyle());
        settings.setValue("SVGScaleFactor", SVGScaleFactor());
        settings.setValue("SVGMargin",      SVGMargin());
    }
    settings.endGroup();
}
