/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xflscript/xml/xflxmlreader.h>

#include <xflgeom/geom3d/vector3d.h>


class WingXfl;


class XmlXPlaneReader : public XflXmlReader
{
    public:
        XmlXPlaneReader(QFile &XFile);

    protected:
};
