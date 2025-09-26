/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <xflscript/xml/xflxmlwriter.h>

class Sail;
class NURBSSail;
class SplineSail;
class WingSail;

class XmlXSailWriter : public XflXmlWriter
{
    public:
        XmlXSailWriter(QFile &XFile);

    protected:
        virtual void writeHeader() = 0;
        void writeMetaData(Sail const *pSail, Vector3d const &position);
        void writeNURBSSail(const NURBSSail *pNSail, Vector3d const &position);
        void writeSplineSail(const SplineSail *pSSail, Vector3d const &position);
        void writeWingSail(const WingSail *pWSail, Vector3d const &position);
};


