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
#include <iostream>
#include <format>
#include <sstream>

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



std::string BLXFoil::listBL(int iFormat) const
{
    std::string OutString;
    std::stringstream out;

    double que = 0.5*qinf*qinf;

    int iStart = 1;

    out << "\nTop side\n";
    if(iFormat==0) OutString = "    x         Hk     Ue/Vinf      Cf        Cd      A/A0       D*       Theta      CTq\n";
    else           OutString = "x, Hk, Ue/Vinf, Cf, Cd, A/A0, D*, Theta, CTq\n";
    out << (OutString);
    for (int ibl=iStart; ibl<nside1; ibl++)
    {
        if(iFormat==0)
            OutString = std::format("{:8.5f}  {:8.5f}  {:8.5f}  {:8.5f}  {:8.5f}  {:8.5f}  {:8.5f}  {:8.5f}  {:8.5f}\n",
                                    xbl[ibl][1],
                                    Hk[ibl][1],
                                    uedg[ibl][1],
                                    tau[ibl][1]/que,
                                    dis[ibl][1]/qinf/qinf/qinf,
                                    ctau[ibl][1],
                                    dstr[ibl][1],
                                    thet[ibl][1],
                                    ctq[ibl][1]);
        else
            OutString = std::format("{:8.5f}, {:8.5f}, {:8.5f}, {:8.5f}, {:8.5f}, {:8.5f}, {:8.5f}, {:8.5f}, {:8.5f}\n",
                                    xbl[ibl][1],
                                    Hk[ibl][1],
                                    uedg[ibl][1],
                                    tau[ibl][1]/que,
                                    dis[ibl][1]/qinf/qinf/qinf,
                                    ctau[ibl][1],
                                    dstr[ibl][1],
                                    thet[ibl][1],
                                    ctq[ibl][1]);
        out << (OutString);
    }

    out << "\nBottom side\n";
    if(iFormat==0) OutString = "    x         Hk     Ue/Vinf      Cf        Cd      A/A0       D*       Theta      CTq\n";
    else           OutString = "x, Hk, Ue/Vinf, Cf, Cd, A/A0, D*, Theta, CTq\n";
    out << (OutString);
    for (int ibl=iStart; ibl<nside2; ibl++)
    {
        if(iFormat==0)
            OutString = std::format("{:8.5f}  {:8.5f}  {:8.5f}  {:8.5f}  {:8.5f}  {:8.5f}  {:8.5f}  {:8.5f}  {:8.5f}\n",
                                    xbl[ibl][2],
                                    Hk[ibl][2],
                                    uedg[ibl][2],
                                    tau[ibl][2]/que,
                                    dis[ibl][2]/qinf/qinf/qinf,
                                    ctau[ibl][2],
                                    dstr[ibl][2],
                                    thet[ibl][2],
                                    ctq[ibl][2]);
        else
            OutString = std::format("{:8.5f}, {:8.5f}, {:8.5f}, {:8.5f}, {:8.5f}, {:8.5f}, {:8.5f}, {:8.5f}, {:8.5f}\n",
                                    xbl[ibl][2],
                                    Hk[ibl][2],
                                    uedg[ibl][2],
                                    tau[ibl][2]/que,
                                    dis[ibl][2]/qinf/qinf/qinf,
                                    ctau[ibl][2],
                                    dstr[ibl][2],
                                    thet[ibl][2],
                                    ctq[ibl][2]);
        out << (OutString);
    }
    return out.str();
}
