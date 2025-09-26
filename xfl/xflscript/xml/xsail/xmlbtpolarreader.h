/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <xflscript/xml/xflxmlreader.h>

class BSpline;
class BoatPolar;

class XmlBtPolarReader : public XflXmlReader
{
    public:
        XmlBtPolarReader(QFile &file);
        bool readXMLPolarFile();

        BoatPolar *btPolar() {return m_pBtPolar;}

    private:
        bool readBtPolar(BoatPolar *pBtPolar, double lengthunit, double areaunit, double velocityunit);
        bool readSailAngles();
        void readReferenceDimensions(double lengthunit, double areaunit);
        bool readWindSpline(BSpline &spline);

        BoatPolar *m_pBtPolar;

};


