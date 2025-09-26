/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include "xmlplanewriter.h"

#include <xflcore/units.h>
#include <xflobjects/objects3d/fuse/fusexfl.h>
#include <xflobjects/objects3d/fuse/fuseocc.h>
#include <xflobjects/objects3d/inertia/pointmass.h>
#include <xflgeom/geom3d/frame.h>
#include <xflobjects/objects3d/plane/planexfl.h>


XmlPlaneWriter::XmlPlaneWriter(QFile &XFile) : XflXmlWriter(XFile)
{
}


void XmlPlaneWriter::writeHeader()
{
    writeDTD("<!DOCTYPE flow5>");
}


void XmlPlaneWriter::writeXMLPlane(PlaneXfl const &plane)
{
    writeStartDocument();
    {
        writeHeader();

        writeStartElement("xflplane");
        writeAttribute("version", "1.0");
        {
            writeComment("For convenience, all field names are case-insensitive, as an exception to the standard xml specification");
            writeComment("Where applicable, default values will be used for all undefined fields");
            writeUnits();
            writeStartElement("Plane");
            {
                writeTextElement("Name", plane.name());
                if(plane.description().length())
                {
                    writeTextElement("Description", plane.description());
                }

                writeTheStyle(plane.theStyle());

                if(plane.pointMassCount())
                {
                    writeStartElement("Inertia");
                    {
                        for(int ipm=0; ipm<plane.pointMassCount(); ipm++)
                        {
                            writePointMass(plane.pointMassAt(ipm));
                        }
                    }
                    writeEndElement();
                }

                for(int ifuse=0; ifuse<plane.nFuse(); ifuse++)
                {
                    if(plane.fuseAt(ifuse) && plane.fuseAt(ifuse)->isXflType())
                    {
                        FuseXfl const*pFuseXfl = dynamic_cast<FuseXfl const*>(plane.fuseAt(ifuse));
                        writeXflFuse(*pFuseXfl, plane.fusePos(0));
                    }
                }

                for(int iw=0; iw<plane.nWings(); iw++)
                {
                    if(plane.wingAt(iw))
                    {
                        writeWing(*plane.wingAt(iw), plane.wingLE(iw), plane.rxAngle(iw), plane.ryAngle(iw));
                    }
                }
            }
        }
        writeEndElement();
    }
    writeEndDocument();
}




