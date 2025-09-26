/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QXmlStreamReader>
#include <QFile>
#include <xflscript/xml/xflxmlreader.h>


class Polar;

class XmlPolarReader : public XflXmlReader
{
    public:
        XmlPolarReader(QFile &file, Polar *pPolar);
        bool readXMLPolarFile();

    private:
        bool readPolar(Polar *pPolar);
        Polar *m_pPolar;
};


