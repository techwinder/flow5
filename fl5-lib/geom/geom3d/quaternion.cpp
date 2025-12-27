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


#include <quaternion.h>
#include <utils.h>

Quaternion::Quaternion(double Angle, Vector3d const &R):
    t2{0}, t3{0}, t4{0}, t5{0}, t6{0}, t7{0}, t8{0}, t9{0}, t10{0}
{
    Vector3d N;
    N = R;
    N.normalize();
    double theta = Angle*PI/180.0;

    a = cos(theta/2.0);
    double sina = sin(theta/2.0);

    qx = N.x*sina;
    qy = N.y*sina;
    qz = N.z*sina;

    setTxx();
}


Quaternion::Quaternion(void) : t2{0}, t3{0}, t4{0}, t5{0}, t6{0}, t7{0}, t8{0}, t9{0}, t10{0},
                                      a{1.0}, qx{0}, qy{0}, qz{0}
{
    setTxx();
}


Quaternion::Quaternion(double t, double x, double y, double z) :
                  t2{0}, t3{0}, t4{0}, t5{0}, t6{0}, t7{0}, t8{0}, t9{0}, t10{0},
                  a{t}, qx{x}, qy{y}, qz{z}
{
    setTxx();
}



void Quaternion::setTxx()
{
    t2 =   a*qx;
    t3 =   a*qy;
    t4 =   a*qz;
    t5 =  -qx*qx;
    t6 =   qx*qy;
    t7 =   qx*qz;
    t8 =  -qy*qy;
    t9 =   qy*qz;
    t10 = -qz*qz;
}



/** from Euler Angles */
Quaternion::Quaternion(double pitch, double roll, double yaw)
{
    pitch *= PI/180.0;
    roll  *= PI/180.0;
    yaw   *= PI/180.0;

    double cy = cos(yaw * 0.5);
    double sy = sin(yaw * 0.5);
    double cr = cos(roll * 0.5);
    double sr = sin(roll * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);

    a = cy*cr*cp + sy*sr*sp;
    qx = cy*sr*cp - sy*cr*sp;
    qy = cy*cr*sp + sy*sr*cp;
    qz = sy*cr*cp - cy*sr*sp;

    setTxx();
}



void Quaternion::set(Quaternion const &qt)
{
    a  = qt.a;
    qx = qt.qx;
    qy = qt.qy;
    qz = qt.qz;

    setTxx();
}


void Quaternion::set(double real, double x, double y, double z)
{
    a = real;
    qx = x;
    qy = y;
    qz = z;

    setTxx();
}


void Quaternion::set(double Angle, Vector3d const &R, bool bNormalizeAxis)
{
    Vector3d N;
    N = R;
    if(bNormalizeAxis) N.normalize();
    double theta = Angle*PI/180.0;

    a = cos(theta/2.0);
    double sina = sin(theta/2.0);

    qx = N.x*sina;
    qy = N.y*sina;
    qz = N.z*sina;
    setTxx();
}


Vector3d Quaternion::axis() const
{
    double theta = 2.0*atan2(sqrt(qx*qx+qy*qy+qz*qz), a);
    double sina = sin(theta/2.0);
    if(fabs(sina)>1.0e-6)
        return Vector3d(qx/sina, qy/sina, qz/sina);
    else
        return Vector3d(qx, qy, qz).normalized();
}


double Quaternion::angle() const
{
    double theta = 2.0*atan2(sqrt(qx*qx+qy*qy+qz*qz), a);
    return theta*180.0/PI;
}


void Quaternion::fromEulerAngles(double roll, double pitch, double yaw)
{
    roll  *= PI/180.0;
    pitch *= PI/180.0;
    yaw   *= PI/180.0;

    double c1 = cos(yaw/2.0);
    double s1 = sin(yaw/2.0);
    double c2 = cos(pitch/2.0);
    double s2 = sin(pitch/2.0);
    double c3 = cos(roll/2.0);
    double s3 = sin(roll/2.0);
    double c1c2 = c1*c2;
    double s1s2 = s1*s2;
    double w = c1c2*c3 - s1s2*s3;
    double x = c1c2*s3 + s1s2*c3;
    double y = s1*c2*c3 + c1*s2*s3;
    double z = c1*s2*c3 - s1*c2*s3;
    set(w,x,y,z);
}


void Quaternion::toEulerAngles(double &roll, double &pitch, double &yaw) const
{
    // roll (x-axis rotation)
    double sinr = +2.0 * (a * qx + qy * qz);
    double cosr = +1.0 - 2.0 * (qx * qx + qy * qy);
    roll = atan2(sinr, cosr);

    // pitch (y-axis rotation)
    double sinp = +2.0 * (a * qy - qz * qx);
    if (fabs(sinp) >= 1)
        pitch = copysign(PI / 2, sinp); // use 90 degrees if out of range
    else
        pitch = asin(sinp);

    // yaw (z-axis rotation)
    double siny = +2.0 * (a * qz + qx * qy);
    double cosy = +1.0 - 2.0 * (qy * qy + qz * qz);
    yaw = atan2(siny, cosy);

    roll  *= 180.0/PI;
    pitch *= 180.0/PI;
    yaw   *= 180.0/PI;
}


/**
 * In computer graphics, Slerp is shorthand for spherical linear interpolation, introduced by Ken Shoemake
 * in the context of quaternion interpolation for the purpose of animating 3D rotation.
 * It refers to constant-speed motion along a unit-radius great circle arc,
 * given the ends and an interpolation parameter between 0 and 1.
 */
void Quaternion::slerp(Quaternion const &qt0, Quaternion const &qt1, double t)
{
    Quaternion q0, q1;
    q0.set(qt0.normalized());
    q1.set(qt1.normalized());
    double dot = q0.axis().dot(q1.axis());

    if (dot < 0.0f)
    {
        q1 *= -1;
        dot = -dot;
    }

    dot = std::min(dot, 1.0);

    double theta_0 = acos(dot);        // theta_0 = angle between input vectors
    double theta = theta_0*t;          // theta = angle between v0 and result
    double sin_theta = sin(theta);     // compute this value only once
    double sin_theta_0 = sin(theta_0); // compute this value only once
    double s0(0), s1(0);
    if(fabs(sin_theta_0)>0.001)
    {
        s0 = cos(theta) - dot * sin_theta / sin_theta_0;
        s1 = sin_theta / sin_theta_0;
    }
    else
    {
        s0 = cos(theta);
        s1 = 1.0;
    }

    q0 *= s0;
    q1 *= s1;
    set(q0 + q1);
    normalize();
}


Quaternion Quaternion::operator *(double d)
{
    Quaternion scaled;
    scaled.a  = a*d;
    scaled.qx = qx*d;
    scaled.qy = qy*d;
    scaled.qz = qz*d;

    scaled.setTxx();

    return scaled;
}


std::string Quaternion::listQuaternion() const
{
    QString strange = QString::asprintf("a=%g  q=(%g, %g, %g)\n", a, qx, qy, qz);
    strange += QString::asprintf("Axis  = %f, %g, %f\n", axis().x, axis().y, axis().z);
    strange += QString::asprintf("Angle = %.3f", angle()) + DEGch + EOLch;
    strange += QString::asprintf("Norm = %g", norm()) + EOLch;
    return strange.toStdString();
}



