/****************************************************************************

    sail7 Application
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


#define _MATH_DEFINES_DEFINED

#include <sailglobals.h>
#include <vector3d.h>
#include <constants.h>


void setWindAxis(double const Beta, Vector3d &WindDirection, Vector3d &WindNormal, Vector3d &WindSide)
{
    double cosb = cos(Beta*PI/180.0);
    double sinb = sin(Beta*PI/180.0);

    //   Define wind (stability) axis
    WindDirection.set(cosb, sinb, 0.0);
    WindNormal.set(-sinb,cosb,0);
    WindSide      = WindNormal * WindDirection;
}


