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

#pragma once

#include <string>

#include <QDataStream>

/*
 * Implements a Vortex Particle Wake, i.a.w.:
 * D. J. Willis and J. Peraire,
 * A Combined pFFT-Multipole Tree Code, Unsteady Panel Method with Vortex Particle Wakes,
 * 43rd AIAA Aerospace Sciences Meeting and Exhibit, 10-13 January 2005, Reno. N.V
 *
 * Youjiang Wang , Moustafa Abdel-Maksoud, Baowei Song
 * A boundary element-vortex particle hybrid method with inviscid shedding scheme
 * Computers and Fluids 168 (2018) 73–86
 */

#include <vector3d.h>


class FL5LIB_EXPORT Vorton
{
    public:
        Vorton();
        Vorton(Vector3d const &position, Vector3d const &u, double g);

        void setVorton(Vector3d const &position, Vector3d const &u, double g);
        void setPosition(Vector3d const &pos) {m_Position=pos;}
        void setPosition(double px, double py, double pz) {m_Position.x=px; m_Position.y=py; m_Position.z=pz;}

        void setVortex(Vector3d const &omega) {m_Omega.set(omega);}
        void setVortex(double ux, double uy, double uz, double omega);
        void setVortex(Vector3d const &U, double omega) {m_Omega.set(U*omega);}

        bool isActive() const {return m_bActive;}
        void setActive(bool b) {m_bActive=b;}

        Vector3d const &position() const {return m_Position;}
        Vector3d unitDir() const {return m_Omega.normalized();}
        Vector3d const &vortex() const {return m_Omega;}

        double circulation() const {return m_Omega.norm();}
        void setCirculation(double gamma);

        Vector3d vorticity(Vector3d const &pos) const;
        inline void inducedVelocity(Vector3d const &R, double CoreSize, Vector3d &V) const;
        void velocityGradient(Vector3d const &R, double CoreSize, double *G) const;

        /** Wang eq. 15 */
        inline double mollifiedInt(double lambda) const
        {
            double f = (lambda*lambda+2.5) * lambda*lambda*lambda;
            double d =  sqrt((1+lambda*lambda)*(1+lambda*lambda)*(1+lambda*lambda)*(1+lambda*lambda)*(1+lambda*lambda));
            return f/d; // moved factor 1./4 Pi into the Kernel
        }

        double zeta(double lambda) const;

        void translate(Vector3d const &T) {m_Position += T;}

        bool serializeFl5(QDataStream &ar, bool bIsStoring);

        float xf() const {return m_Position.xf();}
        float yf() const {return m_Position.yf();}
        float zf() const {return m_Position.zf();}

        void listVorton(const std::string &prefix) const;


    private:
        Vector3d m_Position;    /**< the vorton's position */
        Vector3d m_Omega;         /**< the vorton's vorticity */
        double m_Volume;        /**< the volume associated to this vorton */

        bool m_bActive;         /**< false if the vorton is further away from the plane than the max distance defined in the WPolar */


    public:
        static bool s_bMollify;
};




inline void Vorton::inducedVelocity(Vector3d const &R, double CoreSize, Vector3d &V) const
{
    if(!m_bActive)
    {
        V.reset();
        return;
    }
    //fast inline implementation
    double RpRx = R.x - m_Position.x;
    double RpRy = R.y - m_Position.y;
    double RpRz = R.z - m_Position.z;
    double r = sqrt(RpRx*RpRx + RpRy*RpRy + RpRz*RpRz);

    if(r<1.e-6)
    {
        V.set(0,0,0);
        return;
    }

    double r3 = r*r*r;
    double Kx = -RpRx/r3;
    double Ky = -RpRy/r3;
    double Kz = -RpRz/r3;

    double f = 1.0;
    if(CoreSize>0.0)
    {
        f = mollifiedInt(r/CoreSize);
    }

    // Willis Eq. 21 + mollification
    V.x = ( Ky*m_Omega.z-Kz*m_Omega.y) * 1.0/4.0/PI*f;
    V.y = (-Kx*m_Omega.z+Kz*m_Omega.x) * 1.0/4.0/PI*f;
    V.z = ( Kx*m_Omega.y-Ky*m_Omega.x) * 1.0/4.0/PI*f;
}


