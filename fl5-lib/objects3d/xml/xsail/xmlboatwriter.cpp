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



#include <api/xmlboatwriter.h>

#include <api/boat.h>
#include <api/fuse.h>
#include <api/fusexfl.h>
#include <api/sail.h>
#include <api/sailnurbs.h>
#include <api/sailspline.h>
#include <api/sailwing.h>
#include <api/units.h>


XmlBoatWriter::XmlBoatWriter(QFile &XFile) : XmlXSailWriter (XFile)
{
}


void XmlBoatWriter::writeHeader()
{
    writeDTD("<!DOCTYPE flow5>");

    writeStartElement("xflboat");

    writeAttribute("version", "1.0");

    writeComment("The fields in this section specify the factors to apply to convert the units of this file to IS units."
                 "The section should be the first in the file.");
    writeUnits();
}



void XmlBoatWriter::writeXMLBoat(Boat const &boat)
{
    writeStartDocument();

    writeHeader();

    writeStartElement("Boat");
    {
        writeTextElement("Name", boat.name());
        writeTextElement("Description", boat.description());

        writeTheStyle(boat.theStyle());

        for(int ihull=0; ihull<boat.hullCount();ihull++)
        {
            if(boat.hullAt(ihull)->isXflType())
            {
                FuseXfl const *pHullXfl = dynamic_cast<FuseXfl const*>(boat.hullAt(ihull));
                writeXflFuse(*pHullXfl, boat.fusePos(0));
            }
        }

        for(int iw=0; iw<boat.nSails(); iw++)
        {
            if(boat.sailAt(iw)->isNURBSSail())
            {
                SailNurbs const *pNS = dynamic_cast<SailNurbs const *>(boat.sailAt(iw));
                writeNURBSSail(pNS, pNS->position());
            }
            else if(boat.sailAt(iw)->isSplineSail())
            {
                SailSpline const *pSS = dynamic_cast<SailSpline const *>(boat.sailAt(iw));
                writeSplineSail(pSS, pSS->position());
            }
            else if(boat.sailAt(iw)->isWingSail())
            {
                SailWing const *pWS = dynamic_cast<SailWing const *>(boat.sailAt(iw));
                writeWingSail(pWS, pWS->position());
            }
        }
    }
    writeEndElement();
    writeEndDocument();
}
