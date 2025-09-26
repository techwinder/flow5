/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once
#include <QVector>



class CubicInterpolation
{
    public:
        CubicInterpolation();
        ~CubicInterpolation();

        bool makeSpline();

        void appendPoint(float xp, float yp);
        void setPoints(const QVector<float> &xin, const QVector<float> &yin);
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

    public:

        QVector<float> a,b,c,d;
        QVector<float> x, y;
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
