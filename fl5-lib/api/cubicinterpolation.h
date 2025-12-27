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

#include <vector>

#include <fl5lib_global.h>


class FL5LIB_EXPORT CubicInterpolation
{
    public:
        CubicInterpolation();
        ~CubicInterpolation();

        bool makeSpline();

        void appendPoint(float xp, float yp);
        void setPoints(const std::vector<float> &xin, const std::vector<float> &yin);
        void setPoints(const float *xin, const float *yin, int n);
        void clearPoints();

        float splineValue(float t) const;
        float splineDerivative(float t) const;
        bool isSingular() const {return m_bSingularSpline;}

        /** if true then the BC is f"=0, if false, the derivative is specified at the endpoints*/
        void setBC2ndType(bool bBC2nd) {m_b2ndDerivativeLeft=m_b2ndDerivativeRight=bBC2nd;}
        /** if true then f"(0)=0, if false, then f'(0) is specified */
        void setBC2ndTypeLeft(bool bBC2nd) {m_b2ndDerivativeLeft=bBC2nd;}
        /** if true then f".back()=0, if false, then f'.back() is specified */
        void setBC2ndTypeRight(bool bBC2nd) {m_b2ndDerivativeRight=bBC2nd;}
        void setEndPointDerivatives(float fp0, float fp1) {m_fp0=fp0; m_fp1=fp1;}
        void setFrontDerivative(float fp0) {m_fp0=fp0;}
        void setBackDerivative(float fp1)  {m_fp1=fp1;}

        void sortPoints();

        // debug access
        std::vector<float> const &x_() const {return x;}
        std::vector<float> const &y_() const {return y;}
        double x_(int k) const {return x.at(k);}
        double y_(int k) const {return y.at(k);}
        double a_(int k) const {return a.at(k);}
        double b_(int k) const {return b.at(k);}
        double c_(int k) const {return c.at(k);}
        double d_(int k) const {return d.at(k);}

    private:

        std::vector<float> a,b,c,d;
        std::vector<float> x, y;
        float m_fp0, m_fp1;

        bool m_bSingularSpline;
        bool m_b2ndDerivativeLeft; /** if true, the BC at end points is f"=0, if false, the BC is f'(front)=d1 and f'(back)=d2 */
        bool m_b2ndDerivativeRight; /** if true, the BC at end points is f"=0, if false, the BC is f'(front)=d1 and f'(back)=d2 */

};

void testCubicInterpolation();

void spline2nd(const float x[], const float y[], int n, float yp1, float ypn, float y2[]);
float c3Recipe(const float xa[], const float ya[], const float *y2a, int n, float x);


float other(const float *X, const float *Y, int n, float u);
void testRecipe();
