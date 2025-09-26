/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QFile>
#include <QXmlStreamWriter>

#include <xflscript/xml/xflxmlwriter.h>

class Polar;

class XmlPolarWriter : public XflXmlWriter
{
    public:
        XmlPolarWriter(QFile &XFile);
        void writeXMLPolar(Polar *pPolar);
        void writeHeader();
};


