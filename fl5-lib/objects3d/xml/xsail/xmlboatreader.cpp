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




#include <api/xmlboatreader.h>

#include <api/bezierspline.h>
#include <api/boat.h>
#include <api/bspline.h>
#include <api/cubicspline.h>
#include <api/fusexfl.h>
#include <api/pointmass.h>
#include <api/pointspline.h>
#include <api/sail.h>
#include <api/sailnurbs.h>
#include <api/sailspline.h>
#include <api/sailwing.h>

XmlBoatReader::XmlBoatReader(QFile &file) : XmlXSailReader(file)
{
    m_pBoat = nullptr;
}


bool XmlBoatReader::readXMLBoatFile()
{
    double lengthunit = 1.0;
    double massunit = 1.0;
    double velocityunit = 1.0;
    double areaunit = 1.0;
    double inertiaunit = 1.0;
    Vector3d pos;

    if (readNextStartElement())
    {
        if (name().toString() == "xflboat" && attributes().value("version").toString() == "1.0")
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().toString().compare(QString("units"), Qt::CaseInsensitive)==0)
                {
                    readUnits(lengthunit, massunit, velocityunit, areaunit, inertiaunit);
                }
                else if (name().toString().compare(QString("Boat"), Qt::CaseInsensitive)==0)
                {
                    m_pBoat = new Boat;
                    readBoat(m_pBoat, lengthunit, areaunit, massunit);
                }
                else
                    skipCurrentElement();
            }
        }
        else
            raiseError("The file is not an xml boat v.1.0 definition file.");
    }

    if(hasError())
    {
        if(m_pBoat) delete m_pBoat;
        m_pBoat = nullptr;
    }

    return !hasError();
}


/** */
bool XmlBoatReader::readBoat(Boat *pBoat, double lengthunit, double areaunit, double massunit)
{
    if(!pBoat)
    {
        raiseError("Internal error: no boat defined");
        return false;
    }
    while(!atEnd() && !hasError() && readNextStartElement())
    {
        if (name().toString().compare(QString("Name"),Qt::CaseInsensitive) ==0)
        {
            pBoat->setName(readElementText().trimmed());
        }
        else if (name().toString().compare(QString("description"), Qt::CaseInsensitive)==0)
        {
            pBoat->setDescription(readElementText().toStdString());
        }
        else if(name().toString().compare(QString("The_Style"), Qt::CaseInsensitive)==0)
        {
            LineStyle ls;
            readTheStyle(ls);
            pBoat->setTheStyle(ls);
        }
        else if (name().toString().compare(QString("body"), Qt::CaseInsensitive)==0)
        {
            FuseXfl *pFuseXfl = pBoat->appendNewXflHull();
            if(!readFuseXfl(pFuseXfl, lengthunit, massunit))
            {
                return false;
            }
            pFuseXfl->makeFuseGeometry();
        }
        else if (name().toString().compare(QString("NURBSSail"), Qt::CaseInsensitive)==0)
        {
            SailNurbs *pSail = new SailNurbs;
            Vector3d sailLE;

            readNURBSSail(pSail, sailLE, lengthunit, areaunit);
            if(!hasError())
            {
                pSail->setPosition(sailLE);
                m_pBoat->appendSail(pSail);
            }
            else return false;
        }
        else if (name().toString().compare(QString("SplineSail"), Qt::CaseInsensitive)==0)
        {
            SailSpline *pSail = new SailSpline;
            Vector3d sailLE;

            readSplineSail(pSail, sailLE, lengthunit, areaunit);
            if(!hasError())
            {
                pSail->setPosition(sailLE);
                m_pBoat->appendSail(pSail);
            }
            else return false;
        }
        else if (name().toString().compare(QString("WingSail"), Qt::CaseInsensitive)==0)
        {
            SailWing *pWSail = new SailWing;
            Vector3d sailLE;

            readWingSail(pWSail, sailLE, lengthunit, areaunit);
            if(!hasError())
            {
                pWSail->setPosition(sailLE);
                m_pBoat->appendSail(pWSail);
            }
            else return false;
        }
        else
            skipCurrentElement();
    }

    return(hasError());
}


