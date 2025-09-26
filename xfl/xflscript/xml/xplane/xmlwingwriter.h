/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <QFile>

#include <xflscript/xml/xflxmlwriter.h>


class XmlWingWriter : public XflXmlWriter
{
    public:
        XmlWingWriter(QFile &XFile);
        void writeXMLWing(WingXfl const &wing);


    protected:
        void writeHeader();
};

