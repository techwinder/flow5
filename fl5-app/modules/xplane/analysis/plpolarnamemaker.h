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

#include <QString>

#include <api/fl5lib_global.h>


class Plane;
class PlaneXfl;
class PlanePolar;

class PlPolarNameMaker
{
    friend class  WPolarAutoNameDlg;

    public:
        PlPolarNameMaker();
        static QString makeName(const Plane *pPlane, const PlanePolar *pWPolar);
        static QString rangeControlNames(const PlaneXfl *pPlane, const PlanePolar *pWPolar);
        static QString stabilityControlNames(PlaneXfl const *pPlane, PlanePolar const *pWPolar);

    private:

        static bool s_bType;
        static bool s_bMethod;
        static bool s_bSurfaces;
        static bool s_bBC;
        static bool s_bViscosity;
        static bool s_bInertia;
        static bool s_bControls;
        static bool s_bExtraDrag;
        static bool s_bFuseDrag;
        static bool s_bGround;
};

