/****************************************************************************

    flow5 application
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


#include <xflxmlwriter.h>

class Sail;
class SailNurbs;
class SailSpline;
class SailWing;

class FL5LIB_EXPORT XmlXSailWriter : public XflXmlWriter
{
    public:
        XmlXSailWriter(QFile &XFile);

    protected:
        virtual void writeHeader() = 0;
        void writeMetaData(Sail const *pSail, Vector3d const &position);
        void writeNURBSSail(const SailNurbs *pNSail, Vector3d const &position);
        void writeSplineSail(const SailSpline *pSSail, Vector3d const &position);
        void writeWingSail(const SailWing *pWSail, Vector3d const &position);
};


