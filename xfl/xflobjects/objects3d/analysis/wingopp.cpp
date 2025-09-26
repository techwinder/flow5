/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED


#include <QTextStream>

#include "wingopp.h"
#include <xflobjects/objects3d/wing/wingxfl.h>

#include <xflgeom/geom_globals/geom_global.h>
#include <xflobjects/objects3d/analysis/wpolar.h>



WingOpp::WingOpp(int PanelArraySize)
{
    m_nPanel4    = PanelArraySize;
    m_dCp = m_dG = m_dSigma = nullptr;

    m_bOut         = false;

    m_Span    = 0.0;
    m_MAChord = 0.0;

    m_NStation     = 0;
    m_nFlaps       = 0;

    m_FlapMoment.clear();
}


double WingOpp::maxLift() const
{
    double maxlift = 0.0;
    for (int i=0; i<m_NStation; i++)
    {
        if(m_SpanDistrib.m_Cl.at(i) * m_SpanDistrib.m_Chord.at(i)/m_MAChord>maxlift)
        {
            maxlift = m_SpanDistrib.m_Cl.at(i) * m_SpanDistrib.m_Chord.at(i)/m_MAChord;
        }
    }
    return maxlift;
}


void WingOpp::createWOpp(WingXfl const *pWing, WPolar const *pWPolar, SpanDistribs const &distribs, AeroForces const &AF)
{
    m_WingType   = pWing->type();
    m_WingName   = pWing->name();
    m_nPanel4    = pWing->nPanel4();
    m_NStation   = pWing->nStations();
    m_nFlaps     = pWing->nFlaps();

    m_Span       = pWPolar->referenceSpanLength();

    m_MAChord    = pWing->MAC();

    /**< @todo check if m_CP !=0 */
    m_AF = AF;

    m_SpanDistrib = distribs;

    m_MaxBending =0.0;
    for(int l=0; l<m_SpanDistrib.m_BendingMoment.size(); l++)
    {
        m_MaxBending = std::max(m_MaxBending, m_SpanDistrib.m_BendingMoment.at(l));
    }
}


bool WingOpp::serializeWingOppXFL(QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat=0;
    int k=0, n=0;
    double dble=0;
    Vector3d V;

    if(bIsStoring)
    {
        // using xf7 format instead
    }
    else
    {

        ar >> ArchiveFormat;

        ar >> m_WingName;

        QString plrname;
        ar >> plrname;

        ar >> n; // analysis method


        ar >> m_bOut;

        ar >> m_NStation;
        ar >> m_nPanel4;

        int m_nWakeNodes, m_NXWakePanels;
        double m_FirstWakePanel, m_WakeFactor;
        ar >> m_nWakeNodes >> m_NXWakePanels;
        ar >> m_FirstWakePanel >> m_WakeFactor;

        ar >> dble >> dble >> dble ; // alpha, beta, Qinf no longer of use
        ar >> dble >> m_Span >> m_MAChord; // mass of no use anymore
        // m_CL etc
        ar >> dble >> dble >> dble;
        ar >> dble >> dble;
        ar >> dble >> dble >> dble;
        ar >> dble >> dble;


        ar >> V.x >> V.y >> V.z;// formerly CP

        m_SpanDistrib.resizeGeometry(m_NStation);
        m_SpanDistrib.resizeResults(m_NStation);
        for (k=0; k<m_NStation; k++)
        {
            ar >> m_SpanDistrib.m_Re[k] >> m_SpanDistrib.m_Chord[k] >> m_SpanDistrib.m_Twist[k];
            ar >> m_SpanDistrib.m_Ai[k] >> m_SpanDistrib.m_Cl[k] >> m_SpanDistrib.m_PCd[k] >> m_SpanDistrib.m_ICd[k];
            ar >> m_SpanDistrib.m_CmViscous[k] >> m_SpanDistrib.m_CmC4[k];
            ar >> m_SpanDistrib.m_XCPSpanRel[k]>> m_SpanDistrib.m_XCPSpanAbs[k];
            ar >> m_SpanDistrib.m_XTrTop[k] >> m_SpanDistrib.m_XTrBot[k];
            ar >> m_SpanDistrib.m_BendingMoment[k];
            ar >> m_SpanDistrib.m_Vd[k].x >> m_SpanDistrib.m_Vd[k].y >> m_SpanDistrib.m_Vd[k].z;
            ar >> m_SpanDistrib.m_F[k].x >> m_SpanDistrib.m_F[k].y >> m_SpanDistrib.m_F[k].z;
            ar >> m_SpanDistrib.m_StripPos[k] >> m_SpanDistrib.m_StripArea[k];
        }

        ar >> m_nFlaps;
        m_FlapMoment.clear();
        for(k=0; k<m_nFlaps; k++)
        {
            ar >> dble;
            m_FlapMoment.append(dble);
        }


        // space allocation
        for (int i=0; i<20; i++) ar >> k;
        for (int i=0; i<50; i++) ar >> dble;
    }
    return true;
}



/**
 * Loads or saves the data of this WingOpp to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool WingOpp::serializeWingOppFl5(QDataStream &ar, bool bIsStoring)
{
    double dble(0);
    int nIntSpares(0);
    int nDbleSpares(0);
    int k(0), n(0);

    // 500001: new fl5 format
    // 500002: changed spandistrib format - beta 08
    // 500003: Modified the format of AeroForces serialization

    int ArchiveFormat = 500003;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << m_WingName;

        switch(m_WingType)
        {
            case WingXfl::Main:
            {
                k=0;
                ar<<k;
                break;
            }
            case WingXfl::Elevator:
            {
                k=1;
                ar<<k;
                break;
            }
            case WingXfl::Fin:
            {
                k=2;
                ar<<k;
                break;
            }
            default:
            case WingXfl::OtherWing:
            {
                k=3;
                ar<<k;
                break;
            }
        }

        ar << m_bOut;

        ar << m_NStation;
        ar << m_nPanel4;

        ar << m_Span << m_MAChord;

        m_AF.serializeFl5(ar, bIsStoring);
        m_SpanDistrib.serializeSpanResultsFl5(ar, bIsStoring);

        ar << int(m_FlapMoment.size());
        for(k=0; k<m_FlapMoment.size(); k++)
        {
            ar << m_FlapMoment.at(k);
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
        if(ArchiveFormat<500001 || ArchiveFormat>500100) return false;

        ar >> m_WingName;

        ar >> k;
        switch(k)
        {
            case 0: m_WingType = WingXfl::Main;       break;
            case 1: m_WingType = WingXfl::Elevator;   break;
            case 2: m_WingType = WingXfl::Fin;        break;
            default:
            case 3: m_WingType = WingXfl::OtherWing;  break;
        }

        ar >> m_bOut;

        ar >> m_NStation;
        ar >> m_nPanel4;

        ar >> m_Span >> m_MAChord;


        if(ArchiveFormat<500003) m_AF.serializeFl5_b17(ar, bIsStoring);
        else
        {
            if(!m_AF.serializeFl5(ar, bIsStoring))
                return false;
        }

        if(!m_SpanDistrib.serializeSpanResultsFl5(ar, bIsStoring)) return false;

        ar >> m_nFlaps;
        m_FlapMoment.clear();
        for(k=0; k<m_nFlaps; k++)
        {
            ar >> dble;
            m_FlapMoment.append(dble);
        }


        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}

