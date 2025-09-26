/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QDataStream>
#include <xflmath/constants.h>
#include "bezierspline.h"



BezierSpline::BezierSpline() : Spline()
{
    m_SplineType = Spline::BEZIER;
    m_bSingular = false;
}


void BezierSpline::splinePoint(double u, double &x, double &y) const
{
    x=y=0.0;
    int N = m_CtrlPt.size();
    for(int ic=0; ic<N; ic++)
    {
        double b = Bernstein(ic, N-1, u);
        x +=  b * m_CtrlPt.at(ic).x;
        y +=  b * m_CtrlPt.at(ic).y;
    }
}


/** UNTESTED */
void BezierSpline::splineDerivative(double u, double &dx, double &dy) const
{
    dx=dy=0.0;
    int N = m_CtrlPt.size();
    int n = N-1;
    for(int ic=0; ic<n; ic++)
    {
        double b = Bernstein(ic, n-1, u);
        dx +=  b * double(n) * (m_CtrlPt.at(ic+1).x - m_CtrlPt.at(ic).x);
        dy +=  b * double(n) * (m_CtrlPt.at(ic+1).y - m_CtrlPt.at(ic).y);
    }
}


void BezierSpline::makeCurve()
{
    double b, t;
    for(int j=0; j<m_Output.size(); j++)
    {
        t = double(j)/double(m_Output.size()-1);
        m_Output[j].set(0.0,0.0);
        for(int ic=0; ic<m_CtrlPt.size(); ic++)
        {
            b = Bernstein(ic, m_CtrlPt.size()-1, t);
            m_Output[j].x +=  b *m_CtrlPt.at(ic).x;
            m_Output[j].y +=  b *m_CtrlPt.at(ic).y;
        }
    }
}


double BezierSpline::Bernstein(int i, int n, double u) const
{
    int fi  = 1;
    int fni = 1;

    for(int k=2;     k<=i; k++) fi  *=k;
    for(int k=n-i+1; k<=n; k++) fni *=k;

    double pui   = 1.0;
    for(int k=0; k<i;   k++) pui*=u;

    double pu1i1 = 1.0;
    for(int k=0; k<n-i; k++) pu1i1*=1.0-u;

    return pui * pu1i1 * double(fni)/double(fi);
}


double BezierSpline::BezierBlend(int k, int n, double u) const
{
    double blend=1.0;

    int nn = n;
    int kn = k;
    int nkn = n - k;

    while (nn >= 1)
    {
        blend *= nn;
        nn--;
        if (kn > 1)
        {
            blend /= double(kn);
            kn--;
        }
        if (nkn > 1)
        {
            blend /= double(nkn);
            nkn--;
        }
    }

    if (k > 0)   for(kn=0; kn<k;   kn++) blend *= u;
    if (n-k > 0) for(kn=0; kn<n-k; kn++) blend *= 1.0-u;

    return(blend);
}


Vector2d BezierSpline::getNormal(double const &x)
{
    double dx, dy;
    Vector2d Normal;

    if(x<=0.0 || x>=1.0) return Vector2d(0.0, 1.0);

    for (int i=0; i<m_Output.size()-1; i++)
    {
        if (m_Output[i].x <m_Output[i+1].x  &&   m_Output[i].x <= x && x<=m_Output[i+1].x )
        {
            dx = m_Output[i+1].x - m_Output[i].x;
            dy = m_Output[i+1].y - m_Output[i].y;
            Normal.set(-dy, dx);
            Normal.normalize();
            return Normal;
        }
    }
    return Vector2d(0.0, 1.0);
}


void BezierSpline::getCamber(double &Camber, double &xc)
{
    Camber = 0.0;
    xc = 0.0;
    for(int i=0; i<m_Output.size(); i++)
    {
        if(fabs(m_Output[i].y)>fabs(Camber))
        {
            xc = m_Output[i].x;
            Camber = m_Output[i].y;
        }
    }
}




void BezierSpline::getSlopes(double &s0, double &s1)
{
    s0 = atan2(m_CtrlPt[1].y, m_CtrlPt[1].x)      * 180/PI;
    int i=m_CtrlPt.size()-2;
    s1 = atan2(m_CtrlPt[i].y, (1.-m_CtrlPt[i].x)) * 180/PI;
}


bool BezierSpline::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    Spline::serializeFl5(ar, bIsStoring);

    int n=0;
    int nIntSpares=0;
    int nDbleSpares=0;
    double dble=0.0;


    if(bIsStoring)
    {
        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;

        return true;
    }
    else
    {
        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;


        updateSpline();
        makeCurve();
        return true;
    }
}

