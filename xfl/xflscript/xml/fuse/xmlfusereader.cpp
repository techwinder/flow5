/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#define _MATH_DEFINES_DEFINED


#include "xmlfusereader.h"
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflobjects/objects3d/fuse/fusexfl.h>
#include <xflobjects/objects3d/fuse/fusenurbs.h>
#include <xflgeom/geom3d/frame.h>

XmlFuseReader::XmlFuseReader(QFile &file) : XflXmlReader(file)
{

}


bool XmlFuseReader::readXMLFuseFile()
{
    double lengthunit = 1.0;
    double massunit = 1.0;

    if (readNextStartElement())
    {
        if (name().toString().compare("xflfuse", Qt::CaseInsensitive)==0 && attributes().value("version").toString() == "1.0")
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().toString().compare(QString("units"), Qt::CaseInsensitive)==0)
                {
                    while(!atEnd() && !hasError() && readNextStartElement() )
                    {
                        if (name().compare(QString("length_unit_to_meter"),      Qt::CaseInsensitive)==0)
                        {
                            lengthunit = readElementText().trimmed().toDouble();
                        }
                        else if (name().compare(QString("mass_unit_to_kg"),      Qt::CaseInsensitive)==0)
                        {
                            massunit = readElementText().trimmed().toDouble();
                        }
                        else
                            skipCurrentElement();
                    }
                }
                else if (name().toString().compare(QString("body"), Qt::CaseInsensitive)==0)
                {
                    m_pFuseXfl = new FuseNurbs;
                    readFuseXfl(m_pFuseXfl, lengthunit, massunit);
                }
            }
        }
        else
            raiseError("The file is not an xml fuse v.1.0 definition file.");
    }

    if(!m_pFuseXfl) return false;
    return(!hasError());
}

