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


#include <api/xmlplanereader.h>

#include <api/fusenurbs.h>
#include <api/fusexfl.h>
#include <api/planexfl.h>

XmlPlaneReader::XmlPlaneReader(QFile &file) : XmlXPlaneReader(file)
{
    m_pPlane = nullptr;
}


bool XmlPlaneReader::readFile()
{
    m_pPlane = nullptr;
    double lengthunit = 1.0;
    double massunit = 1.0;
    double velocityunit=1.0, areaunit=1.0, inertiaunit=1.0;
    if (readNextStartElement())
    {
        if (name().toString() == "xflplane" && attributes().value("version").toString() == "1.0")
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().toString().compare(QString("units"), Qt::CaseInsensitive)==0)
                {
                    readUnits(lengthunit, massunit, velocityunit, areaunit, inertiaunit);
                }
                else if (name().toString().compare(QString("plane"), Qt::CaseInsensitive)==0)
                {
                    m_pPlane = new PlaneXfl();
                    readPlane(m_pPlane, lengthunit, massunit, inertiaunit);
                }
            }
        }
        else
            raiseError("The file is not an xml plane v.1.0 definition file.");
    }

    if(hasError())
    {
        if(m_pPlane) delete m_pPlane;
        m_pPlane = nullptr;
    }

    return !hasError();
}


bool XmlPlaneReader::readPlane(PlaneXfl *pPlane, double lengthUnit, double massUnit, double inertiaUnit)
{
    while(!atEnd() && !hasError() && readNextStartElement())
    {
        if (name().toString().compare(QString("Name"),Qt::CaseInsensitive) ==0)
        {
            pPlane->setName(readElementText().trimmed());
        }
        else if (name().toString().compare(QString("description"), Qt::CaseInsensitive)==0)
        {
            pPlane->setDescription(readElementText().toStdString());
        }
        else if (name().toString().compare(QString("The_Style"), Qt::CaseInsensitive)==0)
        {
            LineStyle ls;
            readTheStyle(ls);
            pPlane->setTheStyle(ls);
        }
        else if (name().compare(QString("inertia"), Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("point_mass"), Qt::CaseInsensitive)==0)
                {
                    PointMass pm;
                    readPointMass(pm, massUnit, lengthUnit);
                    pPlane->appendPointMass(pm);
                }
                else
                    skipCurrentElement();
            }
        }
        else if (name().toString().compare(QString("body"), Qt::CaseInsensitive)==0)
        {
            FuseXfl *pFuseXfl = new FuseNurbs;
            if(!readFuseXfl(pFuseXfl, lengthUnit, massUnit))
            {
                delete pFuseXfl;
                return false;
            }
            pFuseXfl->makeFuseGeometry();
            QString logmsg, prefix;
            pFuseXfl->makeDefaultTriMesh(logmsg, prefix);
            m_pPlane->addFuse(pFuseXfl);
        }
        else if (name().toString().compare(QString("wing"), Qt::CaseInsensitive)==0)
        {
            WingXfl newWing;
            m_pPlane->addWing();
            newWing.clearPointMasses();
            newWing.clearSurfaces();
            newWing.m_Section.clear();
            Vector3d wingLE;
            double Rx(0), Ry(0);

            readWing(&newWing, wingLE, Rx, Ry, lengthUnit, massUnit, inertiaUnit);
            int iWing = pPlane->nWings()-1;
            if(!hasError())
            {
                pPlane->wing(iWing)->duplicate(&newWing);
                pPlane->setWingLE(iWing, wingLE);
                pPlane->setRxAngle(iWing, Rx);
                pPlane->setRyAngle(iWing, Ry);
            }
            else return false;

        }
        else
            skipCurrentElement();
    }

    return !hasError();
}


