/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xflscript/xml/xflxmlreader.h>

class PlaneXfl;

class XmlFuseReader : public XflXmlReader
{
    public:
        XmlFuseReader(QFile &file);

        bool readXMLFuseFile();

        FuseXfl const *fuseXfl() const {return m_pFuseXfl;}
        FuseXfl *fuseXfl() {return m_pFuseXfl;}

    private:
        FuseXfl *m_pFuseXfl;
};





