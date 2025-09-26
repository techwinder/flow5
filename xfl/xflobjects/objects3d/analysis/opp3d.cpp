/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include "opp3d.h"

bool Opp3d::s_bStoreOpps3d = true;

Opp3d::Opp3d() : XflObject()
{
    m_nPanel3 = 0;
    m_nPanel4 = 0;

    m_bThinSurface = true;

    m_Alpha       = 0.0;
    m_Beta        = 0.0;
    m_Phi         = 0.0;
    m_Ry          = 0.0;
    m_QInf        = 0.0;
    m_Ctrl        = 0.0;

    m_AnalysisMethod = Polar3d::VLM2;

    m_NodeValMin = m_NodeValMax = 0.0;

    m_bGround = m_bFreeSurface = false;
    m_GroundHeight = 0.0;
}


void Opp3d::getVortonVelocity(Vector3d const &pt, double CoreSize, Vector3d &V) const
{
    Vector3d vel;
    V.set(0.,0.,0.);
    for(int ir=0; ir<m_Vorton.size(); ir++)
    {
        for(int jc=0; jc<m_Vorton.at(ir).size(); jc++)
        {
            Vorton const &vtn = m_Vorton.at(ir).at(jc);
            if(vtn.isActive())
            {
                vtn.inducedVelocity(pt, CoreSize, vel);
                V += vel;

                if(m_bGround)
                {
                    Vector3d VG, CG(pt.x, pt.y, -pt.z-2.0*m_GroundHeight);
                    vtn.inducedVelocity(pt, CoreSize, VG);
                    V.x += VG.x;
                    V.y += VG.y;
                    V.z -= VG.z;
                }
            }
        }
    }
}


/** Returns an array of points between the vorton columns; used for 3d-display */
QVector<Vector3d> Opp3d::vortonLines() const
{
    QVector<Vector3d> seg;
    for(int ir=0; ir<m_Vorton.size()-1; ir++)
    {
        for(int ic=0; ic<m_Vorton.at(ir).size(); ic++)
        {
            seg.append(m_Vorton.at(ir  ).at(ic).position());
            seg.append(m_Vorton.at(ir+1).at(ic).position());
        }
    }
    return seg;
}


int Opp3d::vortonCount() const
{
    if(m_Vorton.isEmpty()) return 0;
    return m_Vorton.count() * m_Vorton.first().count();
}



