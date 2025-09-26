/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include "xmlfusewriter.h"

#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects3d/fuse/fusexfl.h>
#include <xflgeom/geom3d/frame.h>


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




