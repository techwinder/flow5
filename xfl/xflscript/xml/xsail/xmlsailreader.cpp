/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include "xmlsailreader.h"
#include <xflobjects/sailobjects/sails/sail.h>
#include <xflobjects/sailobjects/sails/nurbssail.h>
#include <xflobjects/sailobjects/sails/splinesail.h>
#include <xflobjects/sailobjects/sails/wingsail.h>


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
                    SplineSail*pSS = new SplineSail;
                    m_pSail = pSS;
                    readSplineSail(pSS, pos, lengthunit, areaunit);
                }
                else if (name().toString().compare(QString("NURBSSail"), Qt::CaseInsensitive)==0)
                {
                    NURBSSail*pNS = new NURBSSail;
                    m_pSail = pNS;
                    readNURBSSail(pNS, pos, lengthunit, areaunit);
                }
                else if (name().toString().compare(QString("WingSail"), Qt::CaseInsensitive)==0)
                {
                    WingSail*pWS = new WingSail;
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
