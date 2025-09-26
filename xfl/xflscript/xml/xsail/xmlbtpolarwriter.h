/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once



#include <xflscript/xml/xflxmlwriter.h>

class BSpline;
class BoatPolar;

class XmlBtPolarWriter : public XflXmlWriter
{
    public:

        XmlBtPolarWriter(QFile &XFile);
        void writeXMLBtPolar(BoatPolar *pBtPolar);
        void writeHeader();
        void writeBtPolarData(const BoatPolar *pBtPolar);
        void writeWindSpline(BSpline const &spline);
};

