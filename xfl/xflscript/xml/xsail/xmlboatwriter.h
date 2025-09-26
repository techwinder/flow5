/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xflscript/xml/xsail/xmlxsailwriter.h>


class Boat;

class XmlBoatWriter : public XmlXSailWriter
{
    public:
        XmlBoatWriter(QFile &XFile);
        void writeXMLBoat(const Boat &boat);

    protected:
        void writeHeader();

};

