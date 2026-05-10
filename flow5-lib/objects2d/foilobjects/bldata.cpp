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



#include <bldata.h>



void BLData::reset()
{
    BLMethod = BL::NOBLMETHOD;
    Side = BL::NOSIDE;
    bIsConverged = false;
    iLE = 0;
    nTE = -1;

    QInf     = 1.0;
    CL       = 0.0;
    Cm       = 0.0;
    XCP      = 0.0;
    Cd_SY    = 0.0;
    XTr      = 1.0;
    XLamSep  = 1.0;
    XTurbSep = 1.0;

    s.clear();
    Qi.clear();
    Qv.clear();
    CTau.clear();
    CTq.clear();
    Cd.clear();
    Cf.clear();
    tauw.clear();
    H.clear();
    HStar.clear();
    Qv.clear();
    delta3.clear();
    dstar.clear();
    nTS.clear();
    theta.clear();
    delta.clear();
    gamtr.clear();
    bConverged.clear();
    bTurbulent.clear();
    node.clear();
    foilnode.clear();
}


void BLData::resizeData(int N, bool bResultsOnly)
{
    if(!bResultsOnly)
    {
        s.resize(N);

        std::fill(s.begin(), s.end(), 0);
        foilnode.resize(N);
    }

    Qi.resize(N, 0);
    Qv.resize(N, 0);
    CTau.resize(N, 0);
    CTq.resize(N, 0);
    Cd.resize(N, 0);
    Cf.resize(N, 0);
    tauw.resize(N, 0);
    H.resize(N, 0);
    HStar.resize(N, 0);
    delta3.resize(N, 0);
    dstar.resize(N, 0);
    nTS.resize(N, 0);
    theta.resize(N, 0);
    delta.resize(N, 0);
    gamtr.resize(N, 0);
    bConverged.resize(N, false);
    bTurbulent.resize(N, false);

/*    Qi.fill(0);
    Qv.fill(0);
    CTau.fill(0);
    CTq.fill(0);
    Cd.fill(0);
    Cf.fill(0);
    tauw.fill(0);
    H.fill(0);
    HStar.fill(0);
    delta3.fill(0);
    dstar.fill(0);
    nTS.fill(0);
    theta.fill(0);
    delta.fill(0);
    gamtr.fill(0);*/

/*    bConverged.fill(false);
    bTurbulent.fill(false);*/

}


void BLData::listBL() const
{
/*    std::string strange;
    std::cout << "  nx  s  Qi   Qv   d*  theta  H  nTS  gamtr  Cf" <<std::endl;
    for(int in=0; in<nNodes(); in++)
    {
        strange = std::format(" {:3d}  {:11g}  {:11g}  {:11g}  {:11g}  {:11g}  {:11g}  {:11g}  {:11g}  {:11g}  ",
                        in, s[in], Qi[in], Qv[in], dstar[in], theta[in], H[in], nTS[in], gamtr[in], Cf[in]);
        std::cout << std::format("{:s}", strange.toStdString().c_str()) << std::endl;
    }*/
}



