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


#include <api/xmlwingwriter.h>
#include <api/wingxfl.h>


XmlWingWriter::XmlWingWriter(QFile &XFile) : XflXmlWriter(XFile)
{
}


void XmlWingWriter::writeHeader()
{
    writeDTD("<!DOCTYPE flow5>");
}


void XmlWingWriter::writeXMLWing(WingXfl const &wing, bool bWriteFoils)
{
    writeStartDocument();

    writeHeader();

    writeStartElement("xflwing");
    writeAttribute("version", "1.0");
    {
        writeUnits();

        Vector3d V;
        writeWing(wing, V, 0, 0, bWriteFoils);
    }
    writeEndElement();

    writeEndDocument();
}

