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



#include <vector2d.h>
#include <constants.h>



void Vector2d::rotateZ(Vector2d const &O, double ZTilt)
{
    //Rotate the vector around the Z-axis, by an angle ZTilt
    Vector2d OP;
    OP.x = x-O.x;
    OP.y = y-O.y;

    ZTilt *= PI/180.0;

    x = O.x + OP.x * cos(ZTilt) - OP.y * sin(ZTilt);
    y = O.y + OP.x * sin(ZTilt) + OP.y * cos(ZTilt);
}


void Vector2d::rotateZ(double beta)
{
    beta *= PI/180.0;

    double xo = x;
    double yo = y;
    x =  xo * cos(beta) - yo * sin(beta);
    y =  xo * sin(beta) + yo * cos(beta);
}


/**
 *     Returns the angle of this vector with V, in degrees
 */
double Vector2d::angle(Vector2d const &v) const
{
    double dot = v.x*x + v.y*y; // proportional to cosine
    double det = v.x*y - v.y*x; // proportional to sine
    return atan2(det, dot) * 180.0/PI;
}





