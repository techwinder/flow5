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


#include <api/xmlwingreader.h>

#include <api/wingxfl.h>

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

