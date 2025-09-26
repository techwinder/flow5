/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xflscript/xml/xsail/xmlxsailreader.h>

class Boat;


class XmlBoatReader : public XmlXSailReader
{
    public:
        XmlBoatReader(QFile &file);

        bool readXMLBoatFile();

        Boat * boat() const {return m_pBoat;}


    private:
        bool readBoat(Boat *pBoat, double lengthunit, double areaunit, double massunit);

        Boat *m_pBoat;
};



