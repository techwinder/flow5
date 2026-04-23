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

#include <cmath>

#include <fl5lib_global.h>

class FL5LIB_EXPORT Vector2d
{
    public:
        double x;
        double y;

        //inline constructors
        Vector2d() : x{0}, y{0}
        {
        }

        Vector2d(double xi, double yi) : x{xi}, y{yi}
        {
        }

        virtual ~Vector2d() = default;

        void reset() {x=y=0.0;}

        //access
        float xf() const {return float(x);}
        float yf() const {return float(y);}

        //inline operators
        double &operator[](int i)
        {
            if(i==0) return x;
            if(i==1) return y;
            return x;
        }


        bool operator ==(Vector2d const &V)
        {
            //used only to compare point positions
            return (V.x-x)*(V.x-x) + (V.y-y)*(V.y-y)<0.000000001;
        }

        void operator+=(Vector2d const &T)
        {
            x += T.x;
            y += T.y;
        }

        void operator-=(Vector2d const &T)
        {
            x -= T.x;
            y -= T.y;
        }

        void operator*=(double const &d)
        {
            x *= d;
            y *= d;
        }

        Vector2d operator *(double const &d) const
        {
            Vector2d T(x*d, y*d);
            return T;
        }


        Vector2d operator /(double const &d) const
        {
            Vector2d T(x/d, y/d);
            return T;
        }

        Vector2d operator +(Vector2d const &V) const
        {
            Vector2d T(x+V.x, y+V.y);
            return T;
        }


        Vector2d operator -(Vector2d const &V) const
        {
            Vector2d T(x-V.x, y-V.y);
            return T;
        }


        //inline methods
        void copy(Vector2d const &V)
        {
            x = V.x;
            y = V.y;
        }

        void set(double x0, double y0)
        {
            x = x0;
            y = y0;
        }

        void set(Vector2d const &V)
        {
            x = V.x;
            y = V.y;
        }

        void normalize()
        {
            double abs = norm();
            if(abs< 1.e-10) return;
            x/=abs;
            y/=abs;
        }

        Vector2d normalized() const
        {
            double l = norm();
            if(fabs(l)<0.000000001) return Vector2d(0.0,0.0);
            else return Vector2d(x/l, y/l);
        }

        Vector2d toUnit() const
        {
            Vector2d unitVector;
            double abs = norm();
            if(abs> 1.e-10)
            {
                unitVector.x/=abs;
                unitVector.y/=abs;
            }
            else
            {
                unitVector.set(x,y);
                unitVector.normalize();
            }
            return unitVector;
        }


        double norm() const
        {
            return sqrt(x*x+y*y);
        }

        float normf() const
        {
            return sqrtf(float(x*x+y*y));
        }

        double dot(Vector2d const &V) const
        {
            return x*V.x + y*V.y ;
        }

        bool isSame(Vector2d const &V, double precision=1.e-9) const
        {
            //used only to compare point positions
            return sqrt((V.x-x)*(V.x-x) + (V.y-y)*(V.y-y)) < precision;
        }

        void translate(Vector2d const &T)
        {
            x += T.x;
            y += T.y;
        }

        void translate(const double &tx, const double &ty)
        {
            x += tx;
            y += ty;
        }


        Vector2d translated(const double &tx, const double &ty) const
        {
            return Vector2d(x+tx, y+ty);
        }

        double angle(Vector2d const &V) const;

        double distanceTo(Vector2d pt) const {return sqrt((pt.x-x)*(pt.x-x) + (pt.y-y)*(pt.y-y));}

        //other methods
        virtual void rotateZ(Vector2d const &O, double ZTilt);
        virtual void rotateZ(double beta);

};



