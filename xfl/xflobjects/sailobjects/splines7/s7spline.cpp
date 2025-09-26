/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#include "s7spline.h"


s7Spline::s7Spline() : Spline()
{
    m_SplineType = Spline::BSPLINE;
    m_SlopePoint[0].set(0.1,0.1);
    m_SlopePoint[1].set(0.8,0.1);
}


void s7Spline::makeCurve()
{
    for(int i=0; i<m_Output.size(); i++)
    {
        double di = double(i)/double(m_Output.size()-1);
        splinePoint(di, m_Output[i].x, m_Output[i].y);
    }
}
