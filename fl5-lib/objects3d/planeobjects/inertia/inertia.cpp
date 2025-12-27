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

#include <QString>


#include <QDataStream>
#include <QString>

#include <inertia.h>
#include <units.h>


bool Inertia::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    int nIntSpares=5;
    int nDbleSpares=5;
    int k=0;
    double dble=0.0;
    QString strange;

    //500001 : new fl5 format;
    int ArchiveFormat = 500001;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << m_Mass_s;
        ar << m_CoG_s.x << m_CoG_s.y << m_CoG_s.z;
        ar << m_Ixx_s << m_Ixy_s << m_Ixz_s << m_Iyy_s << m_Iyz_s << m_Izz_s;

        ar << pointMassCount();
        for(int im=0; im<pointMassCount(); im++)
        {
            ar << pointMassAt(im).mass();
            ar << pointMassAt(im).position().x << pointMassAt(im).position().y <<pointMassAt(im).position().z;
            ar << QString::fromStdString(pointMassAt(im).tag());
        }

        k=0;
        ar << nIntSpares;
        for(int i=0; i<nIntSpares; i++) ar<<k;

        ar << nDbleSpares;
        dble = 0;
        for(int i=0; i<nDbleSpares; i++) ar<<dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat!=500001) return false;

        ar >> m_Mass_s;
        ar >> m_CoG_s.x >> m_CoG_s.y >> m_CoG_s.z;
        ar >> m_Ixx_s >> m_Ixy_s >> m_Ixz_s >> m_Iyy_s >> m_Iyz_s >> m_Izz_s;

        clearPointMasses();
        ar >> k;
        double m=0,px=0,py=0,pz=0;
        for(int im=0; im<k; im++)
        {
            ar >> m >> px >> py >> pz;
            ar >> strange;
            appendPointMass(m, Vector3d(px, py, pz), strange.toStdString());
        }

        ar >> nIntSpares;
        for(int i=0; i<nIntSpares; i++) ar>>k;
        ar >> nDbleSpares;
        for(int i=0; i<nDbleSpares; i++) ar>>dble;
    }
    return true;
}


/** Returns the position of the CoG in body axis
 * Includes both the structural and point masses
 */
Vector3d Inertia::CoG_t() const
{
    Vector3d cog = m_CoG_s * m_Mass_s;
    for(uint im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);
        cog += pm.position() * pm.mass();
    }
    double themass = totalMass();
    if(fabs(themass)>0.0) return cog *1.0/themass;
    else                  return Vector3d();
}


/**
 * Returns the xx component of the inertia tensor, in the part's CoG frame, including
 * both structural and point masses */
double Inertia::Ixx_t() const
{
    Vector3d cogt = CoG_t();

    Vector3d const &d = (cogt-m_CoG_s);
    double ixx = m_Ixx_s + m_Mass_s*(d.y*d.y+d.z*d.z);

    for(uint im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);
        Vector3d const &p = pm.position()-cogt;
        ixx += pm.mass() * (p.y*p.y+p.z*p.z);
    }
    return ixx;
}


/**
 * Returns the yy component of the inertia tensor, in the part's CoG frame, including
 * both structural and point masses */
double Inertia::Iyy_t() const
{
    Vector3d cogt = CoG_t();

    Vector3d const &d = (cogt-m_CoG_s);
    double iyy = m_Iyy_s + m_Mass_s*(d.x*d.x+d.z*d.z);

    for(uint im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);
        Vector3d const &p = pm.position()-cogt;
        iyy += pm.mass() * (p.x*p.x+p.z*p.z);
    }
    return iyy;
}


/**
 * Returns the zz component of the inertia tensor, in the part's CoG frame, including
 * both structural and point masses */
double Inertia::Izz_t() const
{
    Vector3d cogt = CoG_t();

    Vector3d const &d = (cogt-m_CoG_s);
    double izz = m_Izz_s + m_Mass_s*(d.x*d.x+d.y*d.y);

    for(uint im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);
        Vector3d const &p = pm.position()-cogt;
        izz += pm.mass() * (p.x*p.x+p.y*p.y);
    }
    return izz;
}


/**
 * Returns the xy component of the inertia tensor, in the part's CoG frame, including
 * both structural and point masses
 *
 * Sign modification of the products of inertia in v7.13 from negative to positive
*/
double Inertia::Ixy_t() const
{
    Vector3d cogt = CoG_t();

    Vector3d const &d = m_CoG_s - cogt;
    double ixy = m_Ixy_s + m_Mass_s* d.x*d.y;

    for(uint im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);
        Vector3d const &p = pm.position()-cogt;
        ixy += pm.mass() * (p.x*p.y);
    }
    return ixy;
}


/**
 * Returns the xz component of the inertia tensor, in the part's CoG frame, including
 * both structural and point masses
 *
 * Sign modification of the products of inertia in v7.13 from negative to positive
*/
double Inertia::Ixz_t() const
{
    Vector3d cogt = CoG_t();

    Vector3d const &d = m_CoG_s - cogt;
    double ixz = m_Ixz_s + m_Mass_s*d.x*d.z;

    for(uint im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);
        Vector3d const &p = pm.position()-cogt;
        ixz += pm.mass() * (p.x*p.z);
    }
    return ixz;
}


/**
 * Returns the yz component of the inertia tensor, in the part's CoG frame, including
 * both structural and point masses
 *
 * Sign modification of the products of inertia in v7.13 from negative to positive
 */
double Inertia::Iyz_t() const
{
    Vector3d cogt = CoG_t();

    Vector3d const &d = m_CoG_s - cogt;
    double iyz = m_Iyz_s + m_Mass_s*d.y*d.z;

    for(uint im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);
        Vector3d const &p = pm.position()-cogt;
        iyz += pm.mass() * (p.y*p.z);
    }
    return iyz;
}



void Inertia::scaleMasses(double scalefactor)
{
    m_Mass_s *= scalefactor;
    for(uint i=0; i<m_PointMass.size(); i++)
        m_PointMass[i].setMass(m_PointMass[i].mass()*scalefactor);
}


void Inertia::scaleMassPositions(double scalefactor)
{
    for(uint i=0; i<m_PointMass.size(); i++)
        m_PointMass[i].setPosition(m_PointMass[i].position()*scalefactor);
}


void Inertia::translateMasses(Vector3d const & t)
{
    for(uint i=0; i<m_PointMass.size(); i++)
        m_PointMass[i].setPosition(m_PointMass[i].position()+t);
}


void Inertia::rotateMasses(Vector3d const &O, Vector3d const &axis, double theta)
{
    for(uint i=0; i<m_PointMass.size(); i++)
    {
        Vector3d pos = m_PointMass[i].position();
        pos.rotate(O, axis, theta);
        m_PointMass[i].setPosition(pos);
    }
}




