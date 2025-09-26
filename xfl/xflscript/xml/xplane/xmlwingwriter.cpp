/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include "xmlwingwriter.h"
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects3d/wing/wingxfl.h>

XmlWingWriter::XmlWingWriter(QFile &XFile) : XflXmlWriter(XFile)
{
}


void XmlWingWriter::writeHeader()
{
    writeDTD("<!DOCTYPE flow5>");
}


void XmlWingWriter::writeXMLWing(WingXfl const &wing)
{
    writeStartDocument();

    writeHeader();

    writeStartElement("xflwing");
    writeAttribute("version", "1.0");
    {
        writeUnits();

        Vector3d V;
        writeWing(wing, V, 0, 0);
    }
    writeEndElement();

    writeEndDocument();
}

