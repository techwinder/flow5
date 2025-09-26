/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <xflscript/xml/xplane/xmlxplanereader.h>

class WingXfl;

class XmlWingReader : public XmlXPlaneReader
{
    public:
        XmlWingReader(QFile &file);
        WingXfl *wing() {return m_pWing;}

    public:
        bool readXMLWingFile();

    private:
        WingXfl *m_pWing;
};


