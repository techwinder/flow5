/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#define _MATH_DEFINES_DEFINED

#include "xmlwingreader.h"

#include <xflobjects/objects3d/wing/wingxfl.h>

XmlWingReader::XmlWingReader(QFile &file) : XmlXPlaneReader (file)
{
    m_pWing = nullptr;
}


bool XmlWingReader::readXMLWingFile()
{
    double lengthunit = 1.0;
    double massunit = 1.0;
    double inertiaunit = 1.0;
    double velocityunit=1.0, areaunit=1.0; // unused

    if (readNextStartElement())
    {
        if (name().toString().compare("xflwing", Qt::CaseInsensitive)==0 && attributes().value("version").toString().compare("1.0", Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().toString().compare(QString("units"), Qt::CaseInsensitive)==0)
                {
                    readUnits(lengthunit, massunit, velocityunit, areaunit, inertiaunit);
                }
                else if (name().toString().compare(QString("wing"), Qt::CaseInsensitive)==0)
                {
                    m_pWing = new WingXfl;
                    Vector3d V;
                    double rx(0), ry(0);
                    readWing(m_pWing, V, rx, ry, lengthunit, massunit, inertiaunit);
                }
            }
        }
        else
            raiseError("The file is not a plane version 1.0 file.");
    }

    if(!m_pWing) return false;

    if(hasError())
    {
        if(m_pWing)
        {
            delete m_pWing;
            m_pWing = nullptr;
        }
    }

    return !hasError();
}

