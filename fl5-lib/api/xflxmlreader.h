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

#include <QXmlStreamReader>
#include <QFile>

#include <fl5lib_global.h>

class PlaneXfl;
class PointMass;
class NURBSSurface;
class Inertia;
class Polar3d;
class FuseXfl;
class Vector3d;
class WingXfl;
struct fl5Color;
struct ExtraDrag;
struct LineStyle;

class FL5LIB_EXPORT XflXmlReader : public QXmlStreamReader
{
    public:
        XflXmlReader(QFile &file);
        virtual ~XflXmlReader();

    protected:
        bool readTheStyle(LineStyle &theStyle);
        bool readColor(fl5Color &color);
        bool readPointMass(PointMass &pm, double massUnit, double lengthUnit);
        bool readNurbs(NURBSSurface &nurbs, std::vector<int> &xpanels, double lengthUnit);
        bool readFluidData(double &viscosity, double &density);
        void readInertia(Inertia &inertia, double lengthunit, double massunit, double inertiaunit);
        bool readExtraDrag(std::vector<ExtraDrag> &extra, double areaunit);
        bool readUnits(double &lengthunit, double &massunit, double &velocityunit, double &areaunit, double &inertiaunit);
        bool readWakeData(Polar3d &polar3d);
        bool readFuseXfl(FuseXfl *pBody, double lengthUnit, double massUnit);
        bool readWing(WingXfl *pWing, Vector3d &WingLE, double &Rx, double &Ry, double lengthUnit, double massUnit, double inertiaUnit);


    protected:
        int m_VMajor, m_VMinor; // version number
        QString m_FileName, m_Path;
};
