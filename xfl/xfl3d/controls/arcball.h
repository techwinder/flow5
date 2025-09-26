/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

// default angles for pseudo-iso view
#define YAW   -180.0
#define ROLL  -110.0
#define PITCH   0.0

#include <xflgeom/geom3d/vector3d.h>
#include <xflgeom/geom3d/quaternion.h>


class ArcBall
{
    public:
        ArcBall();

        void setQuat(const Quaternion &qt) { m_Quat = qt;}
        void setQuat(double r, double qx, double qy, double qz);
        void start(double ax, double ay);
        void move(double ax, double ay);
        void getSpherePoint(double xv, double yv, Vector3d &Pt);
        void rotateCrossPoint(float &angle, float &xf, float &yf, float &zf);
        void applyRotation(Quaternion const &qtrot, bool bInverted);

        void getRotationMatrix(double*m, bool bTranspose) const {m_Quat.toMatrix(m, bTranspose);}

        static void setSphereRadius(double radius){s_sphereRadius = radius;}
        static double sphereRadius() {return s_sphereRadius;}


    public:
        Vector3d m_Start;                /**< The vector point on the sphere at the time when the mouse was pressed */
        Vector3d m_Current;              /**< The current active point on the sphere */

        Quaternion m_QuatStart;          /**< the quaternion representing the rotation of the view matrix at the time when the mouse was pressed */
        Quaternion m_Quat;               /**< the quaternion representing the rotation of the view matrix */

        static double s_sphereRadius;
};







