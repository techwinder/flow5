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

#include <vector3d.h>



/**
 * Convention
 *   - first rotation is by an angle phi around the z-axis
 *   - second rotation is by an angle theta in [0,PI] about the former x-axis
 *   - third rotation is by an angle psi about the former z-axis
 * All angles are given in degrees
 */
void Vector3d::Euler(double phi, double theta, double psi)
{
    double cpsi = cos(psi  *PI/180.0);        double spsi = sin(psi*PI/180.0);
    double cthe = cos(theta*PI/180.0);        double sthe = sin(theta*PI/180.0);
    double cphi = cos(phi  *PI/180.0);        double sphi = sin(phi*PI/180.0);

    double a11 = cpsi*cphi - cthe*sphi*spsi;
    double a12 = cpsi*sphi + cthe*cphi*spsi;
    double a13 = spsi*sthe;
    double a21 = -spsi*cphi - cthe*sphi*cpsi;
    double a22 = -spsi*sphi + cthe*cphi*cpsi;
    double a23 = cpsi*sthe;
    double a31 = sthe*sphi;
    double a32 = -sthe*cphi;
    double a33 = cthe;

    double x0=x, y0=y, z0=z;
    x = a11*x0 + a12*y0 + a13*z0;
    y = a21*x0 + a22*y0 + a23*z0;
    z = a31*x0 + a32*y0 + a33*z0;
}

/**
 * Convention
 *   - first rotation is by an angle phi around the z-axis
 *   - second rotation is by an angle theta in [0,PI] about the former x-axis
 * All angles are given in degrees
 */
void Vector3d::Euler(double phi, double theta)
{
    double cthe = cos(theta*PI/180.0);        double sthe = sin(theta*PI/180.0);
    double cphi = cos(phi  *PI/180.0);        double sphi = sin(phi*PI/180.0);

    double a11 = cphi;
    double a12 = sphi;
    double a13 = 0;
    double a21 = -cthe*sphi;
    double a22 =  cthe*cphi;
    double a23 = sthe;
    double a31 = sthe*sphi;
    double a32 = -sthe*cphi;
    double a33 = cthe;

    double x0=x, y0=y, z0=z;
    x = a11*x0 + a12*y0 + a13*z0;
    y = a21*x0 + a22*y0 + a23*z0;
    z = a31*x0 + a32*y0 + a33*z0;
}


/**
 * Computes and returns the signed angle in degrees between two 3d vectors
 * @todo UNUSED - there is no such signed angle
 */
double Vector3d::vectorAngle(Vector3d const &V1, Vector3d const &positivedir) const
{
    Vector3d u = this->normalized();
    Vector3d v = V1.normalized();
    double costh = v.dot(u);
    double sinth = (u*v).dot(positivedir);

    if(costh>=0)
    {
        if(sinth>=0.0) return acos(costh)*180.0/PI;
        else           return 360.0-acos(costh)*180.0/PI;
    }
    else
    {
        if(sinth>=0.0) return acos(costh)*180.0/PI;
        else           return 360.0-acos(costh)*180.0/PI;
    }
}



