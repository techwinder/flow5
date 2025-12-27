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

/**
  @file This file implements the Vector3d class
  */
#pragma once

#include <cmath>

#include <fl5lib_global.h>


#include <geom_params.h>
#include <constants.h>

class FL5LIB_EXPORT Vector3d
{
    public:
        double x;
        double y;
        double z;

        //constructors
        Vector3d() : x{0},y{0},z{0}
        {
        }

        Vector3d(double xi, double yi, double zi) : x{xi}, y{yi}, z{zi}
        {
        }

        Vector3d(double const*coords) : x{coords[0]}, y{coords[1]}, z{coords[2]}
        {
        }

        virtual ~Vector3d() = default;

        double dir(int i) const
        {
            if(i==0) return x;
            if(i==1) return y;
            if(i==2) return z;
            return 0.0;
        }

        double &operator[](int i)
        {
            if(i==0) return x;
            if(i==1) return y;
            if(i==2) return z;
            return x;
        }

        float xf() const {return float(x);}
        float yf() const {return float(y);}
        float zf() const {return float(z);}

        bool operator ==(const Vector3d& V)
        {
            //used only to compare point positions
            //		return (V.x-x)*(V.x-x) + (V.y-y)*(V.y-y) + (V.z-z)*(V.z-z)<0.0001*0.0001;

            if(fabs(V.x-x)<0.0001)
            {
                if(fabs(V.y-y)<0.0001)
                {
                    if(fabs(V.z-z)<0.0001) return true;
                }
            }
            return false;
        }

        void crossP(Vector3d const &T, Vector3d &crossproduct) const
        {
            crossproduct.x =  y*T.z - z*T.y;
            crossproduct.y = -x*T.z + z*T.x;
            crossproduct.z =  x*T.y - y*T.x;
        }

        void operator+=(Vector3d const &T)
        {
            x += T.x;
            y += T.y;
            z += T.z;
        }

        void operator-=(Vector3d const &T)
        {
            x -= T.x;
            y -= T.y;
            z -= T.z;
        }

        void operator*=(double d)
        {
            x *= d;
            y *= d;
            z *= d;
        }

        Vector3d operator +(Vector3d const &V) const {return Vector3d(x+V.x, y+V.y, z+V.z);}
        Vector3d operator -(Vector3d const &V) const {return Vector3d(x-V.x, y-V.y, z-V.z);}
        Vector3d operator *(double const &d)   const {return Vector3d(x*d, y*d, z*d);}
        Vector3d operator /(double const &d)   const {return Vector3d(x/d, y/d, z/d);}

        Vector3d operator *(Vector3d const &T) const
        {
            Vector3d C;
            C.x =  y*T.z - z*T.y;
            C.y = -x*T.z + z*T.x;
            C.z =  x*T.y - y*T.x;
            return C;
        }

        void copy(Vector3d const &V)
        {
            x = V.x;
            y = V.y;
            z = V.z;
        }

        void reset() {x=y=z=0.0;}

        void set(double x0, double y0, double z0)
        {
            x = x0;
            y = y0;
            z = z0;
        }

        void set(Vector3d const &V)
        {
            x = V.x;
            y = V.y;
            z = V.z;
        }

        void normalize()
        {
            double abs = norm();
            if(abs< 1.e-15) return;
            x/=abs;
            y/=abs;
            z/=abs;
        }

        double norm() const{return sqrt(x*x+y*y+z*z);}

        float normf() const{return sqrtf(float(x*x+y*y+z*z));}

        double dot(Vector3d const &V) const{return x*V.x + y*V.y + z*V.z;}

        float dotf(Vector3d const &V) const{return float(x*V.x + y*V.y + z*V.z);}

        bool isSame(Vector3d const &V, double precision=LENGTHPRECISION) const
        {
            if(fabs(V.x-x)<precision)
            {
                if(fabs(V.y-y)<precision)
                {
                    if(fabs(V.z-z)<precision) return true;
                }
            }
            return false;
        }

        void translate(Vector3d const &T)
        {
            x += T.x;
            y += T.y;
            z += T.z;
        }

        void translate(double tx, double ty, double tz)
        {
            x += tx;
            y += ty;
            z += tz;
        }

        Vector3d normalized() const
        {
            double l = norm();
            if(fabs(l)<0.000000001) return Vector3d(0.0,0.0,0.0);
            else return Vector3d(x/l, y/l, z/l);
        }

        Vector3d translated(double tx, double ty, double tz) const
        {
            return Vector3d(x+tx, y+ty, z+tz);
        }

        Vector3d translated(Vector3d const T) const
        {
            return *this + T;
        }

        void reverse()
        {
            x=-x;
            y=-y;
            z=-z;
        }

        Vector3d reversed() const{return Vector3d(-x, -y, -z);}

        int size() const{return 3;}//dimension

        double distanceTo(Vector3d const &pt) const {return sqrt((pt.x-x)*(pt.x-x) + (pt.y-y)*(pt.y-y) + (pt.z-z)*(pt.z-z));}
        double distanceTo(double X, double Y, double Z) const {return sqrt((X-x)*(X-x) + (Y-y)*(Y-y) + (Z-z)*(Z-z));}

        //other methods
        inline virtual void rotate(Vector3d const &R, double Angle);
        inline virtual void rotate(Vector3d const &O, Vector3d const &R, double Angle);
        inline virtual void rotateX(Vector3d const &O, double rx);
        inline virtual void rotateY(Vector3d const &O, double ry);
        inline virtual void rotateZ(Vector3d const &O, double rz);
        inline virtual void rotateX(double angleDeg);
        inline virtual void rotateY(double angleDeg);
        inline virtual void rotateZ(double angleDeg);


        void Euler(double phi, double theta, double psi);
        void Euler(double phi, double theta);

        double vectorAngle(Vector3d const &V1, const Vector3d &positivedir) const;

};


/** Rotates this vector around the axis defined by the vector R and by the angle Angle
* @param R the axis of rotation, assumed to have unit length
* @param Angle the angle of rotation in degrees
*/
inline void Vector3d::rotate(Vector3d const &R, double Angle)
{
    double ca = cos(Angle *PI/180.0);
    double sa = sin(Angle *PI/180.0);

    double x0 = x;
    double y0 = y;
    double z0 = z;

    double ux = R.x;
    double uy = R.y;
    double uz = R.z;

    x =     (ca+ux*ux*(1-ca))  *x0  +  (ux*uy*(1-ca)-uz*sa) *y0 +  (ux*uz*(1-ca)+uy*sa) *z0;
    y =   (uy*ux*(1-ca)+uz*sa) *x0  +    (ca+uy*uy*(1-ca))  *y0 +  (uy*uz*(1-ca)-ux*sa) *z0;
    z =   (uz*ux*(1-ca)-uy*sa) *x0  +  (uz*uy*(1-ca)+ux*sa) *y0 +    (ca+uz*uz*(1-ca))  *z0;
}


/** Rotates the point defined by this vector around origin O, the rotation axis defined by vector R, and by the angle Angle
* @param O the center of rotation
* @param R the axis of rotation
* @param Angle the angle of rotation in degrees
*/
inline void Vector3d::rotate(Vector3d const &O, Vector3d const &R, double Angle)
{
    if(fabs(R.norm())<0.00001) return;

    Vector3d OP;
    OP.x = x-O.x;
    OP.y = y-O.y;
    OP.z = z-O.z;

    OP.rotate(R, Angle);

    x = O.x + OP.x;
    y = O.y + OP.y;
    z = O.z + OP.z;
}


/**
 * The vector is interpreted as a point.
 * Rotates the point around the axis defined by point O and the x direction, by an angle XTilt
 * @param O The origin of the rotation
 * @param XTilt the angle of rotation in degrees
*/
inline void Vector3d::rotateX(Vector3d const &O, double rx)
{
    Vector3d OP;
    OP.x = x-O.x;
    OP.y = y-O.y;
    OP.z = z-O.z;

    rx *=PI/180.0;
    y = O.y + OP.y * cos(rx) - OP.z * sin(rx);
    z = O.z + OP.y * sin(rx) + OP.z * cos(rx);
}


/**
 * The vector is interpreted as a point.
 * Rotates the point around the axis defined by point O and the y direction, by an angle YTilt
 * @param O The origin of the rotation
 * @param YTilt the angle of rotation in degrees
*/
inline void Vector3d::rotateY(Vector3d const &O, double ry)
{
    Vector3d OP;
    OP.x = x-O.x;
    OP.y = y-O.y;
    OP.z = z-O.z;

    ry *=PI/180.0;

    x = O.x + OP.x * cos(ry) + OP.z * sin(ry);
    z = O.z - OP.x * sin(ry) + OP.z * cos(ry);
}


/**
 * The vector is interpreted as a point.
 * Rotates the point around the axis defined by point O and the z direction, by an angle ZTilt
 * @param O The origin of the rotation
 * @param ZTilt the angle of rotation in degrees
*/
inline void Vector3d::rotateZ(Vector3d const &O, double rz)
{
    //Rotate the vector around the Z-axis, by an angle ZTilt
    Vector3d OP;
    OP.x = x-O.x;
    OP.y = y-O.y;
    OP.z = z-O.z;

    rz *=PI/180.0;

    x = O.x + OP.x * cos(rz) - OP.y * sin(rz);
    y = O.y + OP.x * sin(rz) + OP.y * cos(rz);
}


inline void Vector3d::rotateX(double angleDeg)
{
    angleDeg *=PI/180.0;

    double yo = y;
    double zo = z;
    y =  yo * cos(angleDeg) - zo * sin(angleDeg);
    z =  yo * sin(angleDeg) + zo * cos(angleDeg);
}


inline void Vector3d::rotateY(double angleDeg)
{
    angleDeg *=PI/180.0;

    double xo = x;
    double zo = z;
    x =  xo * cos(angleDeg) + zo * sin(angleDeg);
    z = -xo * sin(angleDeg) + zo * cos(angleDeg);
}


inline void Vector3d::rotateZ(double angleDeg)
{
    angleDeg *=PI/180.0;

    double xo = x;
    double yo = y;
    x =  xo * cos(angleDeg) - yo * sin(angleDeg);
    y =  xo * sin(angleDeg) + yo * cos(angleDeg);
}

