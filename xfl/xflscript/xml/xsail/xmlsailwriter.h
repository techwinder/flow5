/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xflscript/xml/xsail/xmlxsailwriter.h>

class Sail;

class XmlSailWriter : public XmlXSailWriter
{
    public:
        XmlSailWriter(QFile &XFile);
        void writeXMLSail(Sail *pSail);

    protected:
        void writeHeader();
};

