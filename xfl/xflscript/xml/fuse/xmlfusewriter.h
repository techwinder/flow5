/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <QFile>

#include <xflscript/xml/xflxmlwriter.h>


class FuseXfl;

class XmlFuseWriter : public XflXmlWriter
{
    public:
        XmlFuseWriter(QFile &XFile);
        void writeXMLFuse(FuseXfl const *pFuseXfl);

    protected:
        void writeHeader();


};


