/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QString>

class Boat;
class BoatPolar;


class BtPolarNameMaker
{
    friend class BtPolarAutoNameDlg;

    public:
        BtPolarNameMaker();
        static QString makeName(const Boat *pBoat, const BoatPolar *pBtPolar);
        static QString rangeControlNames(const Boat *pBoat, const BoatPolar *pBtPolar);


    private:
        static bool s_bMethod;
        static bool s_bBC;
        static bool s_bViscosity;
        static bool s_bInertia;
        static bool s_bControls;
        static bool s_bExtraDrag;
        static bool s_bGround;
};

