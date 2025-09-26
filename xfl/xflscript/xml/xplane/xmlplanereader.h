/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xflscript/xml/xplane/xmlxplanereader.h>

class PlaneXfl;

class XmlPlaneReader : public XmlXPlaneReader
{
    public:
        XmlPlaneReader(QFile &file);

        bool readFile();

        PlaneXfl *plane() const {return m_pPlane;}

    private:
        bool readPlane(PlaneXfl *pPlane, double lengthUnit, double massUnit, double inertiaUnit);
        PlaneXfl *m_pPlane;
};





