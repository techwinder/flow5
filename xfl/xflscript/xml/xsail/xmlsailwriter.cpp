/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include "xmlsailwriter.h"
#include <xflobjects/sailobjects/sails/sail.h>
#include <xflobjects/sailobjects/sails/nurbssail.h>
#include <xflobjects/sailobjects/sails/splinesail.h>
#include <xflobjects/sailobjects/sails/wingsail.h>

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
        NURBSSail *pNS = dynamic_cast<NURBSSail*>(pSail);
        writeNURBSSail(pNS, Vector3d());
    }
    else if(pSail->isSplineSail())
    {
        SplineSail const *pSSail = dynamic_cast<const SplineSail*>(pSail);
        writeSplineSail(pSSail, Vector3d());
    }
    else if(pSail->isWingSail())
    {
        WingSail const *pWSail = dynamic_cast<const WingSail*>(pSail);
        writeWingSail(pWSail, Vector3d());
    }

    writeEndElement();
    writeEndDocument();
}
