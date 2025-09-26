/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xflscript/xml/xsail/xmlxsailreader.h>


class XmlSailReader : public XmlXSailReader
{
    public:
        XmlSailReader(QFile &file);
        bool readXMLSailFile();
        Sail *sail() {return m_pSail;}

    private:
        Sail *m_pSail;
};


