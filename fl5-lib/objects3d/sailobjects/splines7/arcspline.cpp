/****************************************************************************

        sail7 Application
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

//Spline based on the two BSpline3d Arcs

#include <arcspline.h>


#include <constants.h>


ArcSpline::ArcSpline() : s7Spline()
{
    m_SplineType = Spline::ARC;

    m_BSpline1.setTheStyle(m_theStyle);
    m_BSpline2.setTheStyle(m_theStyle);

    m_BSpline1.setDegree(2);
    m_BSpline2.setDegree(2);

    m_CtrlPt.clear();
    m_CtrlPt.push_back(Vector2d(0.0, 0.0));
    m_CtrlPt.push_back(Vector2d(0.3, 0.10));
    m_CtrlPt.push_back(Vector2d(1.0, 0.0));
}


void ArcSpline::duplicate(Spline const &arcspline)
{
    Spline::duplicate(arcspline);

    ArcSpline const *pArcSpline = dynamic_cast<const ArcSpline*>(&arcspline);

    m_BSpline1 = pArcSpline->m_BSpline1;
    m_BSpline2 = pArcSpline->m_BSpline2;
}


double ArcSpline::getY(double xinterp, bool bRelative) const
{
    double x = xinterp;
    if(bRelative) x = m_Output.front().x + xinterp*(m_Output.back().x - m_Output.front().x);

    if(x<=m_CtrlPt.at(1).x) return m_BSpline1.getY(x, true);
    else                    return m_BSpline2.getY(x, true);

}


void ArcSpline::getCamber(double &Camber, double &xc)
{
    xc     = m_CtrlPt[1].x;
    Camber = m_CtrlPt[1].y;
}



void ArcSpline::getSlopes(double &s0, double &s1)
{
    s0 = atan2(m_SlopePoint[0].y, m_SlopePoint[0].x)      * 180/PI;
    s1 = atan2(m_SlopePoint[1].y, (1.-m_SlopePoint[1].x)) * 180/PI;
}



Vector2d ArcSpline::normal(double const &)
{
    return Vector2d(0.0, 1.0);
}


Vector2d ArcSpline::splinePoint(double u) const
{
    if(u<0.5)
    {
        return m_BSpline1.splinePoint(2.0*u);
    }
    else
    {
        return m_BSpline2.splinePoint((u-0.5)*2.0);
    }
}


void ArcSpline::splineDerivative(double u, double &dx, double &dy) const
{
    (void)u;
    dx=dy=0.0;
}


bool ArcSpline::updateSpline()
{
    m_BSpline1.clearControlPoints();
    m_BSpline1.appendControlPoint(m_CtrlPt[0]);
    m_BSpline1.appendControlPoint(m_SlopePoint[0]);
    m_BSpline1.appendControlPoint(m_CtrlPt[1]);

    m_BSpline2.clearControlPoints();
    m_BSpline2.appendControlPoint(m_CtrlPt[1]);
    m_BSpline2.appendControlPoint(m_SlopePoint[1]);
    m_BSpline2.appendControlPoint(m_CtrlPt[2]);

    bool b1 = m_BSpline1.updateSpline();
    bool b2 = m_BSpline2.updateSpline();
    m_bSingular = !(b1&&b2);

    return b1 && b2;
}
