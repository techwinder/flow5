/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#define _MATH_DEFINES_DEFINED


#include "xmlplanereader.h"
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflobjects/objects3d/fuse/fusexfl.h>
#include <xflobjects/objects3d/fuse/fusenurbs.h>

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
            pPlane->setDescription(readElementText().trimmed());
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


