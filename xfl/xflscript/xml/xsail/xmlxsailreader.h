/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xflscript/xml/xflxmlreader.h>


class Sail;
class NURBSSail;
class SplineSail;
class WingSail;

class XmlXSailReader : public XflXmlReader
{
    public:
        XmlXSailReader(QFile &file);

    protected:
        bool readNURBSSail(NURBSSail *pNSail, Vector3d &position, double lengthUnit, double areaunit);
        bool readSplineSail(SplineSail *pSSail, Vector3d &position, double lengthunit, double areaunit);
        bool readWingSail(WingSail *pWSail, Vector3d &position, double lengthunit, double areaunit);

};



