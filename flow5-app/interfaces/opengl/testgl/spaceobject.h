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

#include <QColor>
#include <QMatrix4x4>

#include <api/vector3d.h>

#define NDIM 4  // dimension of system
#define GRAVITY 6.673e-11 // N.m2/kg2
#define AU 1.4959787e11    // Astronomical unit in meters
#define LIGHTSPEED 299.792e6     // m/s

class Planet
{
    public:
        Planet();

        void rk4_step(double dt, int nsteps);

        double mass() const {return m_mass;}

        Vector3d position() const {return Vector3d(m_var[0], m_var[1], 0.0);}
        double velocity() const {return sqrt(m_var[2]*m_var[2]+m_var[3]*m_var[3]);}
        double distance() const {return sqrt(m_var[0]*m_var[0]+m_var[1]*m_var[1]);}
        double totalEnergy() const;
        double refEnergy() const {return m_RefEnergy;}

//        void setPosition(double x, double y) {f_t[0]=x; f_t[1]=y;}
        void setPosition(double x, double y) {m_var[0]=sqrt(x*x+y*y); m_var[1]=0.0;}

        void setCircularVelocity() {setCircularVelocity(s_CentralMass);}
        void setCircularVelocity(double mass)
        {
            m_var[2] = 0.0;
            double om = sqrt(GRAVITY*mass/distance()/distance()/distance());
            double d = distance();
            m_var[3] = om *d;  // m/s
        }

        void initializeOrbit();
        void setVelocity(double vx, double vy) {m_var[2]=vx; m_var[3]=vy;}
        void setRefEnergy() {m_RefEnergy = totalEnergy();}
        void setOrbit(double a, double e, double i, double O, double o);

        double period() const;// {return 2.0*PI/omega();}// seconds

        double period_circular() const {return 2.0*PI/sqrt(GRAVITY*s_CentralMass/distance()/distance()/distance());}// seconds

        inline QMatrix4x4 orbitMat() const;

        void gravityForce(const double *f, double *f_x, double *f_y);
        void gravity_rhs(double *f, double *rhsf);

        void list(QString &props) const;

        static void setCentralMass(double mass) {s_CentralMass=mass;} // the sun, the black hole
        static double centralMass() {return s_CentralMass;}

    public:
        double m_mass;    // mass
        double m_Radius; // planet radius used for display - meters

        float m_Tau;    // utility variable to store a color in the [0,1] range
        QColor m_Color;
        QString m_Name;

        // RK4 parameters
        double m_var[NDIM]; // x, y, x', y'

        //standard orbital elements https://en.wikipedia.org/wiki/Orbital_elements
        double m_a;        // semi-major axis
        double m_e;        // excentricity
        double m_i;        // inclination
        double m_Omega;    // longitude of the ascending node
        double m_omega;    // argument of periapsis/perihelion

        double m_RefEnergy;

        static double s_CentralMass;
};

/** Describes a star or a galaxy */
struct Star
{
    double m_Magnitude=0.0;
    double m_Ra=0.0;//right ascension
    double m_Da=0.0;//declination

    Vector3d m_Position;
    QColor m_Color;
    QString m_Name;
};


// https://en.wikipedia.org/wiki/Orbital_elements
inline QMatrix4x4 Planet::orbitMat() const
{
    // first rotation is around z by argument of periapsis
    QMatrix4x4 periapsis;
    periapsis.rotate(m_omega, 0,0,1);
    // second rotation is around y by inclination angle
    QMatrix4x4 inclination;
    inclination.rotate(m_i, 0,1,0);
    // third rotation is around z again by longitude of ascending node
    QMatrix4x4 longitude;
    longitude.rotate(m_Omega, 0,0,1);
    return longitude * inclination * periapsis;
}
