/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/




#include <QDataStream>

#include <xflmath/constants.h>

#include "pointspline.h"


PointSpline::PointSpline() : Spline()
{
    m_SplineType = Spline::POINT;
    m_bSingular = false;
    m_Length = 0.0;
}


void PointSpline::makeCurve()
{
    for(int i=0; i<m_Output.size(); i++)
    {
        double di = double(i)/double(m_Output.size()-1);
        splinePoint(di, m_Output[i].x, m_Output[i].y);
    }
}


double PointSpline::getY(double xinterp, bool bRel) const
{
    double y=0.0;
    double x = xinterp;
    if(bRel) x = m_Output.front().x + xinterp*(m_Output.back().x - m_Output.front().x);

    for (int i=0; i<m_CtrlPt.size()-1; i++)
    {
        if (m_CtrlPt[i].x <m_CtrlPt[i+1].x  &&   m_CtrlPt[i].x <= x && x<=m_CtrlPt[i+1].x )
        {
            y = (m_CtrlPt[i].y     + (m_CtrlPt[i+1].y-m_CtrlPt[i].y)
                    /(m_CtrlPt[i+1].x-m_CtrlPt[i].x)*(x-m_CtrlPt[i].x));
            return y;
        }
    }
    return 0.0; //we'll have to do better...
}


void PointSpline::getCamber(double &Camber, double &xc)
{
    Camber = xc =0.0;

    for (int i=0; i<m_CtrlPt.size()-1; i++)
    {
        if(fabs(m_CtrlPt.at(i).y)>fabs(Camber))
        {
            xc = m_CtrlPt.at(i).x;
            Camber = m_CtrlPt.at(i).y;
        }
    }
}


void PointSpline::getSlopes(double &s0, double &s1)
{
    s0 = atan2(m_CtrlPt[1].y, m_CtrlPt[1].x)      * 180/PI;
    int i = m_CtrlPt.size()-2;
    s1 = atan2(m_CtrlPt[i].y, (1.-m_CtrlPt[i].x)) * 180/PI;
}


void PointSpline::splinePoint(double u, double &x, double &y) const
{
    if(u<=0.0 || fabs(m_Length)<LENGTHPRECISION)
    {
        if(m_CtrlPt.size())
        {
            x = m_CtrlPt.first().x;
            y = m_CtrlPt.first().y;
        }
        else x = y = 0.0;
        return;
    }
    if(u>=1.0)
    {
        x = m_CtrlPt.last().x;
        y = m_CtrlPt.last().y;
        return;
    }

    double l0 = 0;
    double l1 = 0;
    for(int i=1; i<m_CtrlPt.size(); i++)
    {
        double deltaL = (m_CtrlPt[i]-m_CtrlPt[i-1]).norm();
        l1 = l0 + deltaL;
        if(u<l1/m_Length)
        {
            double tau = (u - l0/m_Length) / (deltaL/m_Length);
            x = (1.0-tau) * m_CtrlPt[i-1].x + tau *m_CtrlPt[i].x;
            y = (1.0-tau) * m_CtrlPt[i-1].y + tau *m_CtrlPt[i].y;
            return;
        }
        l0 = l1;
    }
}


/** Untested */
void PointSpline::splineDerivative(double u, double &dx, double &dy) const
{
    if(m_CtrlPt.size()<2 || fabs(m_Length)<LENGTHPRECISION)
    {
        dx = dy = 0.0;
        return;
    }

    if(u<=0.0)
    {
        double dl = m_CtrlPt.at(1).distanceTo(m_CtrlPt.at(0));
        if(dl<LENGTHPRECISION)
        {
            dx = dy = 0.0;
            return;
        }
        dx = (m_CtrlPt.at(1).x-m_CtrlPt.at(0).x)/dl;
        dy = (m_CtrlPt.at(1).y-m_CtrlPt.at(0).y)/dl;
    }

    if(u>=1.0)
    {
        int nl = m_CtrlPt.size()-1;
        Vector2d const&last = m_CtrlPt.last();
        Vector2d const&last1 = m_CtrlPt.at(nl-1);
        double dl = last.distanceTo(last1);
        if(dl<LENGTHPRECISION)
        {
            dx = dy = 0.0;
            return;
        }
        dx = (last.x-last1.x)/dl;
        dy = (last.y-last1.y)/dl;
    }

    double l0{0};
    double l1{0};
    for(int i=1; i<m_CtrlPt.size(); i++)
    {
        Vector2d const&pt = m_CtrlPt.at(i);
        Vector2d const&pt1 = m_CtrlPt.at(i-1);

        double dl = pt1.distanceTo(pt);
        l1 = l0 + dl;
        if(u<l1/m_Length)
        {
            dx = (pt.x-pt1.x)/dl;
            dy = (pt.y-pt1.y)/dl;

            return;
        }
        l0 = l1;
    }
}


bool PointSpline::updateSpline()
{
    // the spline's length is needed for point interpolation
    m_Length=0;
    for(int i=1; i<ctrlPointCount(); i++)
    {
        m_Length += (m_CtrlPt[i]-m_CtrlPt[i-1]).norm();
    }

    return true;
}

bool PointSpline::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    if(!Spline::serializeFl5(ar, bIsStoring)) return false;

    if(!bIsStoring)
    {
        updateSpline();
        makeCurve();
    }
    return true;
}
