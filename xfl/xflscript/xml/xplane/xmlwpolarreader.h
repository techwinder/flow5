/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once


#include <xflscript/xml/xflxmlreader.h>


class WPolar;

class XmlWPolarReader : public XflXmlReader
{
    public:
        XmlWPolarReader(QFile &file);

        bool readXMLPolarFile();

        WPolar *wpolar() const {return  m_pWPolar;}

    private:
        void readWPolar(WPolar *pWPolar, double lengthunit, double areaunit, double massunit, double velocityunit, double inertiaunit);
        void readReferenceDimensions(double lengthunit, double areaunit);

        void readViscosity();

        void readFlapSettings();
        void readAVLControls();
        bool readWPolarInertia(WPolar *pWPolar, double lengthunit, double massunit, double inertiaunit);
        void readInertiaRange(double lengthunit, double massunit);
        void readAngleRange();
        void readOperatingRange(double velocityunit);
        void readFuselageDrag();

        WPolar *m_pWPolar;

};

