/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QString>


class Plane;
class PlaneXfl;
class WPolar;

class WPolarNameMaker
{
    friend class WPolarAutoNameDlg;

    public:
        WPolarNameMaker();
        static QString makeName(const Plane *pPlane, const WPolar *pWPolar);
        static QString rangeControlNames(const PlaneXfl *pPlane, const WPolar *pWPolar);
        static QString stabilityControlNames(PlaneXfl const *pPlane, WPolar const *pWPolar);

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

