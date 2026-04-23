/****************************************************************************

    flow5 application
    Copyright (C) 2025 André Deperrois 
    
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


#include <xmlplanewriter.h>

#include <frame.h>
#include <fuseocc.h>
#include <fusexfl.h>
#include <planexfl.h>
#include <pointmass.h>
#include <units.h>


XmlPlaneWriter::XmlPlaneWriter(QFile &XFile) : XflXmlWriter(XFile)
{
}


void XmlPlaneWriter::writeHeader()
{
    writeDTD("<!DOCTYPE flow5>");
}


void XmlPlaneWriter::writeXMLPlane(PlaneXfl const &plane, bool bWriteFoils)
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
                writeTextElement("Name", QString::fromStdString(plane.name()));
                if(plane.description().length())
                {
                    writeTextElement("Description", QString::fromStdString(plane.description()));
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
                        writeWing(*plane.wingAt(iw), plane.wingLE(iw), plane.rxAngle(iw), plane.ryAngle(iw), bWriteFoils);
                    }
                }
            }
        }
        writeEndElement();
    }
    writeEndDocument();
}




