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


#include <xmlbtpolarwriter.h>

#include <boatpolar.h>
#include <objects_global.h>
#include <units.h>
#include <xml_globals.h>

XmlBtPolarWriter::XmlBtPolarWriter(QFile &XFile) : XflXmlWriter(XFile)
{
}


void XmlBtPolarWriter::writeHeader()
{
}


void XmlBtPolarWriter::writeXMLBtPolar(BoatPolar *pBtPolar)
{
    if(!pBtPolar) return;
    setAutoFormatting(true);
    writeStartDocument();
    {
        writeDTD("<!DOCTYPE flow5>");
        writeStartElement("XflBoatPolar");
        {
            writeAttribute("version", "1.0");
            writeUnits();
            writeBtPolarData(pBtPolar);
        }
        writeEndElement();
    }
    writeEndDocument();
}


void XmlBtPolarWriter::writeBtPolarData(BoatPolar const *pBtPolar)
{
   QString strange;

   writeStartElement("Polar");
   {
       writeTextElement("Polar_Name", QString::fromStdString(pBtPolar->name()));
       writeComment("For scripts: if the boat's name is left blank, the analysis will be associated to all available boats");
       writeTextElement("Boat_Name", QString::fromStdString(pBtPolar->boatName()));
       writeComment("Boat polars are necessarily of the control type");

       writeTheStyle(pBtPolar->theStyle());

       writeTextElement("Method", xml::analysisMethod(pBtPolar->analysisMethod()));

       writeStartElement("Reference_Dimensions");
       {
           writeComment    ("Available options are AUTO and CUSTOM");
           writeTextElement("Reference_Dimensions",   xml::referenceDimension(pBtPolar->referenceDim()));
           writeComment("The following fields are required only if dimensions are set to CUSTOM");
           writeTextElement("Reference_Area",         QString::asprintf("%11.5f", pBtPolar->referenceArea()*Units::m2toUnit()));
           writeTextElement("Reference_Chord_Length", QString::asprintf("%11.5f", pBtPolar->referenceChordLength()*Units::mtoUnit()));
       }
       writeEndElement();

       writeWakeData(*pBtPolar);

       strange = QString::asprintf("%g, %g, %g", pBtPolar->CoG().x*Units::mtoUnit(), pBtPolar->CoG().y*Units::mtoUnit(), pBtPolar->CoG().z*Units::mtoUnit());
       writeTextElement("CoG", strange);


       writeStartElement("Wind_gradient");
       {
           writeWindSpline(pBtPolar->windSpline());
       }
       writeEndElement();

       writeFluidProperties(pBtPolar->viscosity(), pBtPolar->density());

       if(pBtPolar->hasExtraDrag())
           writeExtraDrag(pBtPolar->extraDragList());

       writeComment("Min and max values separated by a comma ,");
       writeTextElement("Boat_Speed", QString::asprintf(" %g, %g", pBtPolar->VBtMin(),  pBtPolar->VBtMax()));
       writeTextElement("TWS",        QString::asprintf(" %g, %g", pBtPolar->qInfMin(), pBtPolar->qInfMax()));
       writeTextElement("TWA",        QString::asprintf(" %g, %g", pBtPolar->twaMin(),  pBtPolar->twaMax()));
       writeTextElement("Phi",        QString::asprintf(" %g, %g", pBtPolar->phiMin(),  pBtPolar->phiMax()));
       writeTextElement("Ry",         QString::asprintf(" %g, %g", pBtPolar->RyMin(),   pBtPolar->RyMax()));

       writeStartElement("SailAngles");
       {
           for(int is=0; is<pBtPolar->sailAngleSize(); is++)
           {
               QString strange, str;
               strange = QString::asprintf("Sail_%d", is+1);
               str = QString::asprintf("%.3f,  %.3f", pBtPolar->sailAngleMin(is), pBtPolar->sailAngleMax(is));
               writeTextElement(strange, str);
           }
       }
       writeEndElement();
   }
   writeEndElement();
}


void XmlBtPolarWriter::writeWindSpline(BSpline const &spline)
{
    writeTextElement("Degree", QString::asprintf("%d",spline.degree()));
    for(int i=0; i<spline.ctrlPointCount(); i++)
    {
        writeTextElement("Point", QString::asprintf("%g, %g", spline.controlPoint(i).x, spline.controlPoint(i).y)); // meters only
    }
}

