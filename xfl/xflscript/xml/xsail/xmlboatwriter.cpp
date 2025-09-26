/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#define _MATH_DEFINES_DEFINED


#include "xmlboatwriter.h"
#include <xflcore/units.h>
#include <xflobjects/sailobjects/boat.h>
#include <xflobjects/objects3d/fuse/fuse.h>
#include <xflobjects/objects3d/fuse/fusexfl.h>
#include <xflobjects/sailobjects/sails/sail.h>
#include <xflobjects/sailobjects/sails/splinesail.h>
#include <xflobjects/sailobjects/sails/nurbssail.h>
#include <xflobjects/sailobjects/sails/wingsail.h>


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
                NURBSSail const *pNS = dynamic_cast<NURBSSail const *>(boat.sailAt(iw));
                writeNURBSSail(pNS, pNS->position());
            }
            else if(boat.sailAt(iw)->isSplineSail())
            {
                SplineSail const *pSS = dynamic_cast<SplineSail const *>(boat.sailAt(iw));
                writeSplineSail(pSS, pSS->position());
            }
            else if(boat.sailAt(iw)->isWingSail())
            {
                WingSail const *pWS = dynamic_cast<WingSail const *>(boat.sailAt(iw));
                writeWingSail(pWS, pWS->position());
            }
        }
    }
    writeEndElement();
    writeEndDocument();
}
