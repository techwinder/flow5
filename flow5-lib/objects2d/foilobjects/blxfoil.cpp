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


#include <cstring>

#include <blxfoil.h>

BLXFoil::BLXFoil()
{
    nside1 = nside2 = 0;
    nd1 = 0;
    nd2 = 0;
    nd3 = 0;

    tklam = qinf = 0.0;

    memset(xd1,    0, sizeof(xd1));
    memset(xd2,    0, sizeof(xd2));
    memset(xd3,    0, sizeof(xd3));
    memset(yd1,    0, sizeof(yd1));
    memset(yd2,    0, sizeof(yd2));
    memset(yd3,    0, sizeof(yd3));
    memset(thet,   0, sizeof(thet));
    memset(tau,    0, sizeof(tau));
    memset(ctau,   0, sizeof(ctau));
    memset(ctq,    0, sizeof(ctq));
    memset(dis,    0, sizeof(dis));
    memset(dstr,   0, sizeof(dstr));
    memset(delt,   0, sizeof(delt));
    memset(uedg,   0, sizeof(uedg));
    memset(xbl,    0, sizeof(xbl));
    memset(Hk,     0, sizeof(Hk));
    memset(RTheta, 0, sizeof(RTheta));
    memset(itran,  0, sizeof(itran));
}


