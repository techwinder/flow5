/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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



class Foil;
class Polar;

#include <fl5lib_global.h>

namespace foil
{
    FL5LIB_EXPORT bool readFoilFile(const std::string &filename, Foil *pFoil, int &iLineError);

    FL5LIB_EXPORT void deRotate(Foil *pFoil);
    FL5LIB_EXPORT void normalize(Foil *pFoil);
    FL5LIB_EXPORT void scaleFoil(Foil *pFoil, double camber, double xCamber, double thickness, double xThickness);
    FL5LIB_EXPORT void interpolateFoils(Foil*pFoil, Foil* const pFoil1, Foil *const pFoil2, double frac);
    FL5LIB_EXPORT void setTEGap(Foil *pFoil, double targetgap, double blendinglength);

}
