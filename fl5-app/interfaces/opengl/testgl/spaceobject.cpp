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

#include <QRandomGenerator>


#include "spaceobject.h"

double Planet::s_CentralMass = 1.989e30; // sun's mass in kg

Planet::Planet()
{
    memset(m_var, 0, NDIM*sizeof(double));
    m_mass = 1.0;      // kg
    m_Radius = 1.0;    // meters

    m_RefEnergy = 0.0;

    int h = QRandomGenerator::global()->bounded(360);
    int s = QRandomGenerator::global()->bounded(155)+100;
    int v = QRandomGenerator::global()->bounded(80)+120;
    m_Color.setHsv(h,s,v,255);

    m_a      = 0;        // semi-major axis
    m_e      = 0;        // excentricity
    m_i      = 0;        // inclination
    m_Omega  = 0;        // longitude of the ascending node
    m_omega  = 0;        // argument of periapsis
}


// in seconds
double Planet::period() const
{
    return 2.0*PI*sqrt(m_a*m_a*m_a/(GRAVITY*s_CentralMass));
}


/** total energy */
double Planet::totalEnergy() const
{
    double Ec =  0.5 * m_mass * (m_var[2]*m_var[2] + m_var[3]*m_var[3]);
    double Ep =  -GRAVITY*s_CentralMass*m_mass/sqrt(m_var[0]*m_var[0]+m_var[1]*m_var[1]);
    return Ec + Ep;
}


void Planet::gravityForce(double const*f, double *f_x, double *f_y)
{
    double GM = GRAVITY*s_CentralMass;
    double xk = f[0];
    double yk = f[1];
    double rk2 = xk*xk + yk*yk;

    *f_x = - GM * m_mass * xk / sqrt(rk2*rk2*rk2);
    *f_y = - GM * m_mass * yk / sqrt(rk2*rk2*rk2);
}


void Planet::gravity_rhs(double *f, double *rhsf)
{
    double f_x=0, f_y=0;

    gravityForce(f, &f_x, &f_y);

    rhsf[0] = f[2];
    rhsf[1] = f[3];
    rhsf[2] = f_x / m_mass;
    rhsf[3] = f_y / m_mass;
}


void Planet::rk4_step(double dt, int nsteps)
{
    double k1[NDIM], k2[NDIM], k3[NDIM], k4, yt[NDIM], rhsf[NDIM];

    for(int i=0; i<nsteps; i++)
    {
        gravity_rhs(m_var, rhsf);
        for (int i=0; i<NDIM; i++)
        {
            k1[i] = dt * rhsf[i];
            yt[i] = m_var[i] + 0.5*k1[i];
        }
        gravity_rhs(yt, rhsf);
        for (int i=0; i<NDIM; i++)
        {
            k2[i] = dt * rhsf[i];
            yt[i] = m_var[i] + 0.5*k2[i];
        }
        gravity_rhs(yt, rhsf);
        for (int i=0; i<NDIM; i++)
        {
            k3[i] = dt * rhsf[i];
            yt[i] = m_var[i] + k3[i];
        }
        gravity_rhs(yt, rhsf);
        for (int i=0; i<NDIM; i++)
        {
            k4 = dt * rhsf[i];
            m_var[i] += (0.5*k1[i] + k2[i] + k3[i] + 0.5*k4) / 3.0;
        }
    }
}


/** Case of Sgr A* stars */
void Planet::setOrbit(double a, double e, double i, double O, double o)
{
    double dist = 26673; //light-years
    dist *= 9.46e15; // meters

    // a is in arcseconds
    m_a     = a * 1./3600; //degrees
    m_a    *= PI/180.0;    //radians
    m_a    *= dist;        //meters
    m_e     = e;
    m_i     = i;
    m_Omega = O;
    m_omega = o;

    initializeOrbit();
}


void Planet::initializeOrbit()
{
    float Distance = m_a*(m_e+1.0); // apogee distance to Sun
    setPosition(Distance, 0.0);
    float v = sqrt(GRAVITY*Planet::s_CentralMass*(2.0/distance()-1.0/m_a)); // vis-viva equation
    setVelocity(0, v);
}


void Planet::list(QString &props) const
{
    props = m_Name +":\n";
    props += QString::asprintf(  "   period   = %7.2f years", period()/365/24/3600);
    props += QString::asprintf("\n   distance = %7.2f a.u.",  distance()/AU);
    props += QString::asprintf("\n   velocity = %7.2f%% c",   velocity()*100.0/LIGHTSPEED);
    props += QString::asprintf("\n   RK4 energy loss = %5.2f%%",     (totalEnergy()/m_RefEnergy-1.0)*100.0);
}

