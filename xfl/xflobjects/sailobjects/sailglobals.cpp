/****************************************************************************

    sail7 Application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <xflobjects/sailobjects/sailglobals.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflmath/constants.h>


void setWindAxis(double const Beta, Vector3d &WindDirection, Vector3d &WindNormal, Vector3d &WindSide)
{
    double cosb = cos(Beta*PI/180.0);
    double sinb = sin(Beta*PI/180.0);

    //   Define wind (stability) axis
    WindDirection.set(cosb, sinb, 0.0);
    WindNormal.set(-sinb,cosb,0);
    WindSide      = WindNormal * WindDirection;
}


