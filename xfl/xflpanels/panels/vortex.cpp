/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QDataStream>

#include "vortex.h"
#include <xflpanels/panels/panelprecision.h>

Vortex::enumVortex Vortex::s_VortexModel = Vortex::LAMB_OSEEN;
double Vortex::s_CoreRadius = 1.e-6;

Vortex::Vortex() : Segment3d()
{
    m_Circulation = 0.0;
}

Vortex::Vortex(Node const &vtx0, Node const &vtx1, double gamma) : Segment3d(vtx0, vtx1)
{
    m_Circulation = gamma;
}


/**
 * Returns the velocity induced by the vortex defined by the segment AB with unit strength
 * at a field point C
 */
void Vortex::getInducedVelocity(Vector3d const &C, Vector3d &vel,
                             double coreradius, enumVortex vortexmodel) const
{
    vortexInducedVelocity(m_S[0], m_S[1], C, vel, coreradius, vortexmodel);
    vel *= m_Circulation;
}


bool Vortex::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat = 500001;

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << m_S[0].x<< m_S[0].y<< m_S[0].z;
        ar << m_S[1].x<< m_S[1].y<< m_S[1].z;
        ar << m_Circulation;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat!=500001) return false;
        ar >> m_S[0].x>> m_S[0].y>> m_S[0].z;
        ar >> m_S[1].x>> m_S[1].y>> m_S[1].z;
        ar >> m_Circulation;

        setNodes(m_S[0], m_S[1]);
    }
    return true;
}



/**
 * Returns the velocity induced by the vortex defined by the segment AB with unit strength
 * at a field point C
 */
void vortexInducedVelocity(Vector3d const &A, Vector3d const &B, Vector3d const &C,
                             Vector3d &vel, double coreradius, Vortex::enumVortex vortexmodel)
{
    double r0_x = B.x - A.x;
    double r0_y = B.y - A.y;
    double r0_z = B.z - A.z;
    double r0 = sqrt(r0_x*r0_x + r0_y*r0_y + r0_z*r0_z);

    double r1_x = C.x - A.x;
    double r1_y = C.y - A.y;
    double r1_z = C.z - A.z;
    double r1 = sqrt(r1_x*r1_x + r1_y*r1_y + r1_z*r1_z);

    double r2_x = C.x - B.x;
    double r2_y = C.y - B.y;
    double r2_z = C.z - B.z;
    double r2 = sqrt(r2_x*r2_x + r2_y*r2_y + r2_z*r2_z);

    if(r0<DISTANCEPRECISION || r1<DISTANCEPRECISION || r2<DISTANCEPRECISION)
    {
        vel.set(0.0,0.0,0.0);
        return;
    }

    double Psi_x = r1_y*r2_z - r1_z*r2_y;
    double Psi_y =-r1_x*r2_z + r1_z*r2_x;
    double Psi_z = r1_x*r2_y - r1_y*r2_x;

    double ftmp = Psi_x*Psi_x + Psi_y*Psi_y + Psi_z*Psi_z;

    if(fabs(ftmp)<PRECISION)
    {
        vel.set(0,0,0);
        return;
    }

    //get the distance of the field point to the line
    double t_x =  r1_y*r0_z - r1_z*r0_y;
    double t_y = -r1_x*r0_z + r1_z*r0_x;
    double t_z =  r1_x*r0_y - r1_y*r0_x;

    double r = sqrt((t_x*t_x+t_y*t_y+t_z*t_z)/(r0_x*r0_x+r0_y*r0_y+r0_z*r0_z));
    double visc = damp(r, vortexmodel, coreradius);

    Psi_x /= ftmp;
    Psi_y /= ftmp;
    Psi_z /= ftmp;

    double Omega =  (r0_x*r1_x + r0_y*r1_y + r0_z*r1_z)/sqrt((r1_x*r1_x + r1_y*r1_y + r1_z*r1_z))
            -(r0_x*r2_x + r0_y*r2_y + r0_z*r2_z)/sqrt((r2_x*r2_x + r2_y*r2_y + r2_z*r2_z));

    vel.x = Psi_x * Omega/4.0/PI * visc;
    vel.y = Psi_y * Omega/4.0/PI * visc;
    vel.z = Psi_z * Omega/4.0/PI * visc;
}


double damp(double r, Vortex::enumVortex vortexmodel, double core)
{
    core  = std::max(core, 0.0);
    switch(vortexmodel)
    {
        default:
        case Vortex::POTENTIAL:
        {
            if(fabs(r)<DISTANCEPRECISION) return 0.0;
            return 1.0;
        }
        case Vortex::CUT_OFF:
        {
            if(r<=core) return 0.0;
            return 1.0;
        }
        case Vortex::LAMB_OSEEN:
        {
            if(core>0.0)
            {
                // using rc = sqrt(4.nu.t)
                if(r>0.0)
                {
                    return (1.0-exp(-r*r/core/core));
                }
                else return 0.0;
            }
            return 1.0;
        }
        case Vortex::RANKINE:
        {
            if(core>0.0 && r<core) return r*r/core/core;
            return 1.0;
        }
        case Vortex::SCULLY:
        {
            if(core>0.0)  return r*r/(core*core+r*r);
            return 1.0;
        }
        case Vortex::VATISTAS:  // n=2
        {
            if(core>0.0)  return r*r/sqrt(core*core*core*core+r*r*r*r);
            return 1.0;
        }
    }

    return 1.0;
}
