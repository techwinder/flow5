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


#include <xmlsailreader.h>

#include <sail.h>
#include <sailnurbs.h>
#include <sailspline.h>
#include <sailwing.h>


XmlSailReader::XmlSailReader(QFile &file) : XmlXSailReader(file)
{
    m_pSail = nullptr;
}


bool XmlSailReader::readXMLSailFile()
{
    double lengthunit = 1.0;
    double massunit = 1.0;
    double velocityunit = 1.0;
    double areaunit = 1.0;
    double inertiaunit = 1.0;
    Vector3d pos;
    if (readNextStartElement())
    {
        if (name().toString() == "xflsail" && attributes().value("version").toString() == "1.0")
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().toString().compare(QString("units"), Qt::CaseInsensitive)==0)
                {
                    readUnits(lengthunit, massunit, velocityunit, areaunit, inertiaunit);
                }
                else if (name().toString().compare(QString("SplineSail"), Qt::CaseInsensitive)==0)
                {
                    SailSpline*pSS = new SailSpline;
                    m_pSail = pSS;
                    readSplineSail(pSS, pos, lengthunit, areaunit);
                }
                else if (name().toString().compare(QString("NURBSSail"), Qt::CaseInsensitive)==0)
                {
                    SailNurbs*pNS = new SailNurbs;
                    m_pSail = pNS;
                    readNURBSSail(pNS, pos, lengthunit, areaunit);
                }
                else if (name().toString().compare(QString("WingSail"), Qt::CaseInsensitive)==0)
                {
                    SailWing*pWS = new SailWing;
                    m_pSail = pWS;
                    readWingSail(pWS, pos, lengthunit, areaunit);
                }
                else
                    skipCurrentElement();
            }
        }
        else
            raiseError("The file is not an xml sail v.1.0 definition file.");
    }

    if(hasError())
    {
        if(m_pSail) delete m_pSail;
        m_pSail = nullptr;
    }
    return !hasError();
}
