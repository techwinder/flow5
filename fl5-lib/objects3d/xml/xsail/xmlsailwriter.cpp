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


#include <api/xmlsailwriter.h>

#include <api/sail.h>
#include <api/sailnurbs.h>
#include <api/sailspline.h>
#include <api/sailwing.h>

XmlSailWriter::XmlSailWriter(QFile &XFile) : XmlXSailWriter(XFile)
{
}


void XmlSailWriter::writeHeader()
{
    writeDTD("<!DOCTYPE flow5>");
    writeStartElement("xflsail");

    writeAttribute("version", "1.0");

    writeComment("The fields in this section specify the factors to apply to convert the units of this file to IS units."
                 "The section should be the first in the file.");
    writeUnits();
}


void XmlSailWriter::writeXMLSail(Sail *pSail)
{
    writeStartDocument();
    writeHeader();

    if(pSail->isNURBSSail())
    {
        SailNurbs *pNS = dynamic_cast<SailNurbs*>(pSail);
        writeNURBSSail(pNS, Vector3d());
    }
    else if(pSail->isSplineSail())
    {
        SailSpline const *pSSail = dynamic_cast<const SailSpline*>(pSail);
        writeSplineSail(pSSail, Vector3d());
    }
    else if(pSail->isWingSail())
    {
        SailWing const *pWSail = dynamic_cast<const SailWing*>(pSail);
        writeWingSail(pWSail, Vector3d());
    }

    writeEndElement();
    writeEndDocument();
}
