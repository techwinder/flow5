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



#include "arcball.h"

double ArcBall::s_sphereRadius = 0.7;


ArcBall::ArcBall()
{
    m_Quat.a  = 0.0;
    m_Quat.qx = m_Quat.qy = m_Quat.qz = 0.0;

    // initialize the arcball with a pseudo iso viewpoint
    Quaternion qti;
    qti.fromEulerAngles(ROLL, PITCH, YAW);
    Quaternion qtyaw(-30.0, Vector3d(0.0,0.0,1.0));
    setQuat(qti*qtyaw);

    m_Start.set(0.0,0.0,1.0);
    m_Current.set(0.0,0.0,1.0);
}


void ArcBall::applyRotation(Quaternion const &qtrot, bool bInverted)
{
    if(bInverted)
        m_Quat = m_Quat * qtrot;
    else
        m_Quat = qtrot * m_Quat;
}


/** update current arcball rotation*/
void ArcBall::move(double ax, double ay)
{
    getSpherePoint(ax,ay, m_Current);
    if((m_Current-m_Start).norm()<1.e-6) return;

    // build the incremental quaternion which rotates the former vector position to the new one
    Quaternion qt;
    qt.from2UnitVectors(m_Start.normalized(), m_Current.normalized());

    // update the quat representing the view matrix
    m_Quat = qt * m_QuatStart;
}


void ArcBall::rotateCrossPoint(float &angle, float &xf, float &yf, float &zf)
{
    Vector3d aa(1.0, 0.0, 0.0);

    double cosa   = aa.dot(m_Current);
    double  cosa2  = sqrt((1.0 + cosa)*0.5);
    angle = 2.0f*acosf(float(cosa2))*180.0f/PIf;

    Vector3d V = aa * m_Current;
    V.normalize();
    xf = V.xf();
    yf = V.yf();
    zf = V.zf();
}


void ArcBall::setQuat(double r, double qx, double qy, double qz)
{
    m_Quat.a  = r;

    m_Quat.qx = qx;
    m_Quat.qy = qy;
    m_Quat.qz = qz;
}


void ArcBall::start(double ax, double ay)
{
    getSpherePoint(ax, ay, m_Start);
    m_Current = m_Start;
    m_QuatStart = m_Quat;
}


/** returns a unit vector to the point on the sphere's surface from the input viewport coordinates */
void ArcBall::getSpherePoint(double xv, double yv, Vector3d &Pt)
{
    double r2 = s_sphereRadius*s_sphereRadius;

    if(r2>xv*xv+yv*yv) Pt.set(xv,yv,sqrt(r2-xv*xv-yv*yv));
    else               Pt.set(xv,yv,0.0);

    Pt.normalize();
}





