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

#define _MATH_DEFINES_DEFINED


#include <api/xmlfusewriter.h>

#include <api/frame.h>
#include <api/fusexfl.h>
#include <api/units.h>
#include <api/utils.h>

XmlFuseWriter::XmlFuseWriter(QFile &XFile) : XflXmlWriter(XFile)
{
}


void XmlFuseWriter::writeHeader()
{
    writeDTD("<!DOCTYPE flow5>");
    writeStartElement("xflfuse");
    writeAttribute("version", "1.0");

    writeStartElement("Units");
    {
        writeTextElement("length_unit_to_meter", QString("%1").arg(1./Units::mtoUnit()));
        writeTextElement("mass_unit_to_kg", QString("%1").arg(1./Units::kgtoUnit()));
    }
    writeEndElement();
}


void XmlFuseWriter::writeXMLFuse(FuseXfl const *pFuse)
{
    writeStartDocument();

    writeHeader();

    writeStartElement("xflfuse");
    {
        writeTextElement("Name", pFuse->name());
        writeTextElement("Description", pFuse->description());

        writeComment("If the field AUTOINERTIA is set to TRUE, the fields COG and COG_I** will be ignored");
        writeTextElement("AutoInertia", xfl::boolToString(pFuse->bAutoInertia()));
        writeInertia(pFuse->inertia());

        writeEndElement();

        writeXflFuse(*pFuse, Vector3d());

    }
    writeEndElement();
    writeEndDocument();
}




