/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QFile>

#include <xflscript/xml/xflxmlwriter.h>



class WPolar;

class XmlWPolarWriter : public XflXmlWriter
{
    public:
        XmlWPolarWriter(QFile &XFile);
        void writeXMLWPolar(WPolar const *pWPolar);
        void writeHeader();
        void writeWPolarData(const WPolar *pWPolar);
        void writeWPolarInertia(const WPolar *pWPolar);
};


