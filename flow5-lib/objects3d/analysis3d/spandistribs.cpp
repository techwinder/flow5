/****************************************************************************
 *
 *     flow5 application
 *
 *     Copyright (C) 2025 André Deperrois 
 *
 *     
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

 *
 *****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include <QDataStream>

#include <spandistribs.h>
#include <wingxfl.h>

SpanDistribs::SpanDistribs()
{

}

void SpanDistribs::clearGeometry()
{
    m_Chord.clear();
    m_Offset.clear();
    m_Twist.clear();
    m_StripArea.clear();
    m_StripPos.clear();
    m_PtC4.clear();
}


void SpanDistribs::resizeGeometry(int NStation)
{
    m_Chord.resize(NStation);
    m_Offset.resize(NStation);
    m_Twist.resize(NStation);
    m_StripArea.resize(NStation);
    m_StripPos.resize(NStation);
    m_PtC4.resize(NStation);

    std::fill(m_Chord.begin(), m_Chord.end(), 0);
    std::fill(m_Offset.begin(), m_Offset.end(), 0);
    std::fill(m_Twist.begin(), m_Twist.end(), 0);
    std::fill(m_StripArea.begin(), m_StripArea.end(), 0);
    std::fill(m_StripPos.begin(), m_StripPos.end(), 0);
    std::fill(m_PtC4.begin(), m_PtC4.end(), Vector3d());
}


void SpanDistribs::setGeometry(WingXfl const*pWing)
{
    m_Chord     = pWing->m_Chord;
    m_Offset    = pWing->m_Offset;
    m_PtC4      = pWing->m_PtC4;
    m_StripArea = pWing->m_StripArea;
    m_StripPos  = pWing->m_StripPos;
    m_Twist     = pWing->m_Twist;
}


void SpanDistribs::resizeResults(int NStation)
{
    m_Ai.resize(NStation);               std::fill(m_Ai.begin(),         m_Ai.end(), 0);
    m_Alpha_0.resize(NStation);          std::fill(m_Alpha_0.begin(),    m_Alpha_0.end(), 0);
    m_Cl.resize(NStation);               std::fill(m_Cl.begin(),         m_Cl.end(), 0);
    m_ICd.resize(NStation);              std::fill(m_ICd.begin(),        m_ICd.end(), 0);
    m_PCd.resize(NStation);              std::fill(m_PCd.begin(),        m_PCd.end(), 0);
    m_CmPressure.resize(NStation);       std::fill(m_CmPressure.begin(), m_CmPressure.end(), 0);
    m_CmViscous.resize(NStation);        std::fill(m_CmViscous.begin(),  m_CmViscous.end(), 0);
    m_CmC4.resize(NStation);             std::fill(m_CmC4.begin(),       m_CmC4.end(), 0);
    m_Re.resize(NStation);               std::fill(m_Re.begin(),         m_Re.end(), 0);
    m_XTrBot.resize(NStation);           std::fill(m_XTrBot.begin(),     m_XTrBot.end(), 0);
    m_XTrTop.resize(NStation);           std::fill(m_XTrTop.begin(),     m_XTrTop.end(), 0);
    m_XCPSpanAbs.resize(NStation);       std::fill(m_XCPSpanAbs.begin(), m_XCPSpanAbs.end(), 0);
    m_XCPSpanRel.resize(NStation);       std::fill(m_XCPSpanRel.begin(), m_XCPSpanRel.end(), 0);
    m_BendingMoment.resize(NStation);    std::fill(m_BendingMoment.begin(), m_BendingMoment.end(), 0);
    m_VTwist.resize(NStation);           std::fill(m_VTwist.begin(),     m_VTwist.end(), 0);
    m_Gamma.resize(NStation);            std::fill(m_Gamma.begin(),      m_Gamma.end(), 0);
    m_bConverged.resize(NStation);       std::fill(m_bConverged.begin(), m_bConverged.end(), false);
    m_F.resize(NStation);                std::fill(m_F.begin(),          m_F.end(), Vector3d());
    m_Vd.resize(NStation);               std::fill(m_Vd.begin(),         m_Vd.end(), Vector3d());
}


void SpanDistribs::initializeToZero()
{

    std::fill(m_Chord.begin(),      m_Chord.end(),     0);
    std::fill(m_Offset.begin(),     m_Offset.end(),    0);
    std::fill(m_Twist.begin(),      m_Twist.end(),     0);
    std::fill(m_StripArea.begin(),  m_StripArea.end(), 0);
    std::fill(m_StripPos.begin(),   m_StripPos.end(),  0);
    std::fill(m_PtC4.begin(),       m_PtC4.end(), Vector3d());

    std::fill(m_Ai.begin(),         m_Ai.end(),         0);
    std::fill(m_Alpha_0.begin(),    m_Alpha_0.end(),    0);
    std::fill(m_Cl.begin(),         m_Cl.end(),         0);
    std::fill(m_ICd.begin(),        m_ICd.end(),        0);
    std::fill(m_PCd.begin(),        m_PCd.end(),        0);
    std::fill(m_CmPressure.begin(), m_CmPressure.end(), 0);
    std::fill(m_CmViscous.begin(),  m_CmViscous.end(),  0);
    std::fill(m_CmC4.begin(),       m_CmC4.end(),       0);
    std::fill(m_Re.begin(),         m_Re.end(),         0);
    std::fill(m_XTrBot.begin(),     m_XTrBot.end(),     0);
    std::fill(m_XTrTop.begin(),     m_XTrTop.end(),     0);
    std::fill(m_XCPSpanAbs.begin(), m_XCPSpanAbs.end(), 0);
    std::fill(m_XCPSpanRel.begin(), m_XCPSpanRel.end(), 0);
    std::fill(m_BendingMoment.begin(), m_BendingMoment.end(), 0);
    std::fill(m_VTwist.begin(),     m_VTwist.end(),     0);
    std::fill(m_Gamma.begin(),      m_Gamma.end(),      0);
    std::fill(m_bConverged.begin(), m_bConverged.end(), false);
    std::fill(m_F.begin(),          m_F.end(),          Vector3d());
    std::fill(m_Vd.begin(),         m_Vd.end(),         Vector3d());
}


bool SpanDistribs::serializeSpanResultsFl5(QDataStream &ar, bool bIsStoring)
{
    double dble = 0.0;;
    float xf=0, yf=0, zf=0;
    // 500001: new fl5 format
    // 500002: added offset and PtC4 properties
    int ArchiveFormat = 500002;
    int NStation = int(m_Re.size());
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << NStation;
        for (int k=0; k<NStation; k++)
        {
            ar << m_Chord.at(k) << m_Twist.at(k) << m_StripPos.at(k) << m_StripArea.at(k);
            ar << m_Re.at(k);
            ar << m_Ai.at(k) << m_Cl.at(k) << m_PCd.at(k) << m_ICd.at(k);
            ar << m_CmPressure.at(k) << m_CmViscous.at(k) << m_CmC4.at(k);
            ar << m_XCPSpanRel.at(k)<< m_XCPSpanAbs.at(k);
            ar << m_XTrTop.at(k) << m_XTrBot.at(k);
            ar << m_VTwist.at(k);
            ar << m_BendingMoment.at(k);
            ar << m_Vd.at(k).x << m_Vd.at(k).y << m_Vd.at(k).z;
            ar << m_F.at(k).x << m_F.at(k).y << m_F.at(k).z;
            ar << m_Alpha_0.at(k) << m_Gamma.at(k);

            ar << m_Offset.at(k);
            ar << m_PtC4.at(k).xf()<< m_PtC4.at(k).yf()<< m_PtC4.at(k).zf();
            dble=0.0;
            for(int i=0; i<8; i++) ar<<dble; // space allocation
        }

        for(int i=0; i<10; i++) ar<<dble; // space allocation
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500001 || ArchiveFormat>500002) return false;

        ar >> NStation;
        resizeGeometry(NStation);
        resizeResults(NStation);
        for (int k=0; k<NStation; k++)
        {
            ar >> m_Chord[k] >> m_Twist[k] >> m_StripPos[k] >> m_StripArea[k];
            ar >> m_Re[k];
            ar >> m_Ai[k] >> m_Cl[k] >> m_PCd[k] >> m_ICd[k];
            ar >> m_CmPressure[k] >> m_CmViscous[k] >> m_CmC4[k];
            ar >> m_XCPSpanRel[k]>> m_XCPSpanAbs[k];
            ar >> m_XTrTop[k] >> m_XTrBot[k];
            ar >> m_VTwist[k];
            ar >> m_BendingMoment[k];
            ar >> m_Vd[k].x >> m_Vd[k].y >> m_Vd[k].z;
            ar >> m_F[k].x >> m_F[k].y >> m_F[k].z;
            ar >> m_Alpha_0[k] >> m_Gamma[k];
            if(ArchiveFormat>=500002)
            {
                ar >> m_Offset[k];
                ar >> xf >> yf >> zf;
                m_PtC4[k].set(xf,yf,zf);
            }
            for(int i=0; i<8; i++) ar>>dble; // space allocation
        }
        for(int i=0; i<10; i++) ar >> dble; // space allocation
    }
    return true;
}


/**
 * Returns the normal lift acting on the strip
 * @param m the strip's index
 * @param qDyn the dynamic pressure */
double SpanDistribs::stripLift(int m, double qDyn) const
{
    if(m<0 || m>int(m_StripArea.size())) return 0.0;
    return qDyn*m_Cl.at(m)*m_StripArea.at(m);
}





