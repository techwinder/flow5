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
#include <fl5lib_global.h>


#include <xflxmlreader.h>

class BSpline;
class BoatPolar;

class FL5LIB_EXPORT XmlBtPolarReader : public XflXmlReader
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


