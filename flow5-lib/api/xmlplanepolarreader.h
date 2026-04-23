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


#include <xflxmlreader.h>


class PlanePolar;

class FL5LIB_EXPORT XmlPlanePolarReader : public XflXmlReader
{
    public:
        XmlPlanePolarReader(QFile &file);

        bool readXMLPolarFile();

        PlanePolar *wpolar() const {return  m_pPlPolar;}

    private:
        void readWPolar(PlanePolar *pPlPolar, double lengthunit, double areaunit, double massunit, double velocityunit, double inertiaunit);
        void readReferenceDimensions(double lengthunit, double areaunit);

        void readViscosity();

        void readFlapSettings();
        void readAVLControls();
        bool readWPolarInertia(PlanePolar *pPlPolar, double lengthunit, double massunit, double inertiaunit);
        void readInertiaRange(double lengthunit, double massunit);
        void readAngleRange();
        void readOperatingRange(double velocityunit);
        void readFuselageDrag();

        PlanePolar *m_pPlPolar;

};

