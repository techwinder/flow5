/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    
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

#include <api/xmlpolarwriter.h>

#include <api/polar.h>
#include <api/objects_global.h>


XmlPolarWriter::XmlPolarWriter(QFile &XFile) : XflXmlWriter(XFile)
{
}


void XmlPolarWriter::writeXMLPolar(Polar *pPolar)
{
    if(!pPolar) return;

    QString strange;

    writeHeader();
    writeStartElement("Polar");
    {
        writeTextElement("Foil_Name",  pPolar->foilName());
        writeTextElement("Polar_Name", pPolar->name());
        writeTextElement("Type",       objects::polarType(pPolar->type()));
        switch(pPolar->BLMethod())
        {
            default:
            case BL::XFOIL:        writeTextElement("Method", "XFoil");        break;
        }


        if(!pPolar->isControlPolar())
        {
            writeTextElement("Fixed_Reynolds",           QString("%1").arg(pPolar->Reynolds(),11,'f',0));
            writeTextElement("Fixed_AOA",                QString("%1").arg(pPolar->aoaSpec(),11,'f',3));
            writeTextElement("Mach",                     QString("%1").arg(pPolar->Mach(),7,'f', 2));
            writeTextElement("ReType",                   QString("%1").arg(pPolar->ReType()));
            writeTextElement("MaType",                   QString("%1").arg(pPolar->MaType()));
            writeTextElement("NCrit",                    QString("%1").arg(pPolar->NCrit(),7,'f', 1));
            writeTextElement("Forced_Top_Transition",    QString("%1").arg(pPolar->XTripTop(),7,'f', 2));
            writeTextElement("Forced_Bottom_Transition", QString("%1").arg(pPolar->XTripBot(),7,'f', 2));
        }
    }
    writeEndElement();
    writeEndDocument();
}


void XmlPolarWriter::writeHeader()
{
    setAutoFormatting(true);

    writeStartDocument();
    writeDTD("<!DOCTYPE flow5>");
    writeStartElement("Foil_Polar");
    writeAttribute("version", "1.0");
}




