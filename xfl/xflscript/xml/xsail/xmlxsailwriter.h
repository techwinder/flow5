/****************************************************************************

    flow5
    Copyright (C) 2025 André Deperrois 
    
    This file is part of flow5.

    flow5 is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    flow5 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with flow5.
    If not, see <https://www.gnu.org/licenses/>.


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


