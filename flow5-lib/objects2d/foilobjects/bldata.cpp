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

#include <QDataStream>

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
/*    QString strange;
    qDebug("  nx  s  Qi   Qv   d*  theta  H  nTS  gamtr  Cf" );
    for(int in=0; in<nNodes(); in++)
    {
        strange = QString::asprintf(" %3d  %11g  %11g  %11g  %11g  %11g  %11g  %11g  %11g  %11g  ",
                        in, s[in], Qi[in], Qv[in], dstar[in], theta[in], H[in], nTS[in], gamtr[in], Cf[in]);
        qDebug("%s", strange.toStdString().c_str());
    }*/
}


void BLData::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    //500001 : first fl5 format
    int n=0;
    int nVariables = 23;

    std::vector<float> fl(nVariables, 0);

    double dble=0.0;
    int nIntSpares=0;
    int nDbleSpares=0;

    int ArchiveFormat = 500001;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        switch(BLMethod)
        {
            case BL::XFOIL:        n=3;   break;
            case BL::NOBLMETHOD:   n=5;   break;
        }
        ar << n;

        switch(Side)
        {
            default:
            case BL::TOP:    n = 0;  break;
            case BL::BOTTOM: n = 1;  break;
            case BL::WAKE:   n = 2;  break;
        }
        ar << n;

        ar << iLE <<nTE;
        ar << bIsConverged;
        ar << QInf << CL << Cm << XCP << Cd_SY << XTr <<XLamSep << XTurbSep;


        ar << int(s.size()); // number of data points
        ar << nVariables; // adjustable number of variables for future growth
        for(uint in=0; in<s.size(); in++)
        {
            ar << float(s[in]);
            ar << float(Qi[in]);
            ar << float(Qv[in]);
            ar << float(CTau[in]);
            ar << float(CTq[in]);
            ar << float(Cd[in]);
            ar << float(Cf[in]);
            ar << float(tauw[in]);
            ar << float(H[in]);
            ar << float(HStar[in]);
            ar << float(delta3[in]);
            ar << float(dstar[in]);
            ar << float(nTS[in]);
            ar << float(theta[in]);
            ar << float(delta[in]);
            ar << float(gamtr[in]);
            bConverged[in] ? ar<<1.0f : ar<<0.0f;
            ar << float(foilnode[in].index());
            ar << float(foilnode[in].x) << float(foilnode[in].y);
            ar << float(foilnode[in].normal().x) << float(foilnode[in].normal().y);
            ar << (foilnode[in].isWakeNode() ? 1.0f : 0.0f);
        }

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;

        ar >> n;
        switch(n)
        {
            default:
            case 3: BLMethod=BL::XFOIL;          break;
            case 5: BLMethod=BL::NOBLMETHOD;     break;
        }

        ar >> n;
        switch(n)
        {
            case 0: Side=BL::TOP;      break;
            case 1: Side=BL::BOTTOM;   break;
            case 2: Side=BL::WAKE;     break;
        }

        ar >> iLE >>nTE;
        ar >> bIsConverged;
        ar >> QInf >> CL >> Cm >> XCP >> Cd_SY >> XTr >>XLamSep >> XTurbSep;

        ar >> n; // number of data points
        resizeData(n, false);
        ar >> nVariables;
        for(int in=0; in<n; in++)
        {
            for(int iv=0; iv<nVariables; iv++) ar >> fl[iv];
            s[in]      = double(fl[0]);
            Qi[in]     = double(fl[1]);
            Qv[in]     = double(fl[2]);
            CTau[in]   = double(fl[3]);
            CTq[in]    = double(fl[4]);
            Cd[in]     = double(fl[5]);
            Cf[in]     = double(fl[6]);
            tauw[in]   = double(fl[7]);
            H[in]      = double(fl[8]);
            HStar[in]  = double(fl[9]);
            delta3[in] = double(fl[10]);
            dstar[in]  = double(fl[11]);
            nTS[in]    = double(fl[12]);
            theta[in]  = double(fl[13]);
            delta[in]  = double(fl[14]);
            gamtr[in]  = double(fl[15]);
            bConverged[in] = double(fl[16])<0.5 ? false : true;
            foilnode[in].setIndex(int(fl[17]));
            foilnode[in].set(double(fl[18]), double(fl[19]));
            foilnode[in].setNormal(Vector2d(double(fl[20]), double(fl[21])));
            foilnode[in].setWakeNode(double(fl[22])>0.5);
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
}

