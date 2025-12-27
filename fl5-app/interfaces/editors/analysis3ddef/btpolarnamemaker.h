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

