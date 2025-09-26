/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <QFile>

#include <xflscript/xml/xflxmlwriter.h>


class PlaneXfl;
class WingXfl;
class FuseXfl;
class FuseOcc;
class PointMass;

class XmlPlaneWriter : public XflXmlWriter
{
    public:
        XmlPlaneWriter(QFile &XFile);
        void writeXMLPlane(const PlaneXfl &plane);

    protected:
        void writeHeader();

};


