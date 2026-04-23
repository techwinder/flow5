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


#define _MATH_DEFINES_DEFINED

#include <opp3d.h>


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

    m_AnalysisMethod = xfl::VLM2;

    m_NodeValMin = m_NodeValMax = 0.0;

    m_bGround = m_bFreeSurface = false;
    m_GroundHeight = 0.0;
}


void Opp3d::getVortonVelocity(Vector3d const &pt, double CoreSize, Vector3d &V) const
{
    Vector3d vel;
    V.set(0.,0.,0.);
    for(uint ir=0; ir<m_Vorton.size(); ir++)
    {
        for(uint jc=0; jc<m_Vorton.at(ir).size(); jc++)
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
std::vector<Vector3d> Opp3d::vortonLines() const
{
    std::vector<Vector3d> seg;
    for(int ir=0; ir<int(m_Vorton.size()-1); ir++)
    {
        for(int ic=0; ic<int(m_Vorton.at(ir).size()); ic++)
        {
            seg.push_back(m_Vorton.at(ir  ).at(ic).position());
            seg.push_back(m_Vorton.at(ir+1).at(ic).position());
        }
    }
    return seg;
}


int Opp3d::vortonCount() const
{
    if(m_Vorton.size()==0) return 0;
    return int(m_Vorton.size() * m_Vorton.front().size());
}



