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

#include <QString>

#include <iostream>
#include <cstring>

#include <QDataStream>

#include <vorton.h>
#include <constants.h>


bool Vorton::s_bMollify=true; /** @todo debug only, remove */

Vorton::Vorton()
{
    m_Omega.set(0,0,0);
    m_Volume = 0;
    m_bActive = true;
}


Vorton::Vorton(Vector3d const &position, Vector3d const &u, double g)
{
    m_Position.set(position);
    m_Omega.set(u*g);
    m_Volume = 0.0;
    m_bActive = true;
}


void Vorton::setVorton(Vector3d const &position, Vector3d const &u, double g)
{
    m_Position.set(position);
    m_Omega.set(u*g);
    m_Volume = 0.0;
}

void Vorton::setVortex(double ux, double uy, double uz, double omega)
{
    m_Omega.x = ux*omega;
    m_Omega.y = uy*omega;
    m_Omega.z = uz*omega;
    m_Volume = 0.0;
}


void Vorton::setCirculation(double gamma)
{
    m_Omega.normalize();
    m_Omega *= gamma;
}


/**
 * Returns the velocity gradient G of the velocity vector
 * G is a 3x3 tensor such that g_ij = dV_j/dx_i.
 * cf. Willis Eq. 22 */
void Vorton::velocityGradient(Vector3d const &R, double CoreSize, double *G) const
{
    if(!m_bActive)
    {
        memset(G, 0, 9*sizeof(double));
        return;
    }
    double x = R.x - m_Position.x;
    double y = R.y - m_Position.y;
    double z = R.z - m_Position.z;

    double r = sqrt(x*x + y*y + z*z);

    if(fabs(r)<1.e-6)
    {
        memset(G,0,9*sizeof(double));
        return;
    }

    double r3 = r*r*r;
    double r5 = r*r*r*r*r;

    Vector3d const &alpha = vortex();

    G[0] =       0     + 3*x/r5*(y*alpha.z-z*alpha.y);  //  dVx/dx
    G[1] =  alpha.z/r3 + 3*x/r5*(z*alpha.x-x*alpha.z);  //  dVy/dx
    G[2] = -alpha.y/r3 + 3*x/r5*(x*alpha.y-y*alpha.x);  //  dVz/dx

    G[3] = -alpha.z/r3 + 3*y/r5*(y*alpha.z-z*alpha.y);  //  dVx/dy
    G[4] =       0     + 3*y/r5*(z*alpha.x-x*alpha.z);  //  dVy/dy
    G[5] =  alpha.x/r3 + 3*y/r5*(x*alpha.y-y*alpha.x);  //  dVz/dy

    G[6] =  alpha.y/r3 + 3*z/r5*(y*alpha.z-z*alpha.y);  //  dVx/dz
    G[7] = -alpha.x/r3 + 3*z/r5*(z*alpha.x-x*alpha.z);  //  dVy/dz
    G[8] =       0     + 3*z/r5*(x*alpha.y-y*alpha.x);  //  dVz/dz

    double f = 1.0;
    if(s_bMollify)
    {
        double lambda = r/CoreSize;
        f = mollifiedInt(lambda);
    }
//qDebug("core=%13g, r=%13g f=%13g",s_CoreSize,r,f);

    for(int i=0; i<9; i++) G[i] *= 1.0/4.0/PI*f;
}


/** Wang eq. 11 */
double Vorton::zeta(double lambda) const
{
    double d = (1.0+lambda*lambda);
    return 15.0/8.0/PI / sqrt(d*d*d*d*d*d*d*d*d);
}


Vector3d Vorton::vorticity(Vector3d const &pos) const
{
    double r = (pos-m_Position).norm();
    return m_Omega * zeta(r);
}


void Vorton::listVorton(std::string const &prefix) const
{
    QString strange = QString::asprintf("x=%13g y=%13g z=%13g  //  om.x=%13g om.y=%13g om.z=%13g",
                                        m_Position.x, m_Position.y, m_Position.z,
                                        m_Omega.x, m_Omega.y, m_Omega.z);
    std::cout << prefix +" "+strange.toStdString() << std::endl;
}


bool Vorton::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    // 500001: first format

    int ArchiveFormat = 500001;
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << m_Position.x << m_Position.y << m_Position.z;
        ar << m_Omega.x    << m_Omega.y    << m_Omega.z;
        ar << m_Volume;
        ar << m_bActive;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500001 || ArchiveFormat>=500002) return false;

        ar >> m_Position.x >> m_Position.y >> m_Position.z;
        ar >> m_Omega.x    >> m_Omega.y    >> m_Omega.z;
        ar >> m_Volume;
        ar >> m_bActive;
    }
    return true;
}
