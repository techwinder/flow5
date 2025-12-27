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



//Spline based on the mid line of the NACA 4 digit airfoil series

#include <QDataStream>

#include <naca4spline.h>

#include <constants.h>


Naca4Spline::Naca4Spline() : s7Spline()
{
    m_SplineType  = Spline::NACA4;

    p=.3;
    m = .1;

    m_CtrlPt.clear();
    m_CtrlPt.push_back({0.0,0.0});
    m_CtrlPt.push_back({0.3,0.1});
    m_CtrlPt.push_back({1.0,0.0});
}


void Naca4Spline::getSlopes(double &s0, double &s1)
{
    s0 = atan(m/p/p * 2.0*p)*180/PI;
    s1 = atan(m/(1.0-p)/(1.0-p)  * (2.0*p-2))*180/PI;
}


void Naca4Spline::duplicate(const Spline &n4spline)
{
    s7Spline::duplicate(n4spline);

    Naca4Spline const *pN4Spline = dynamic_cast<const Naca4Spline*>(&n4spline);

    m = pN4Spline->m;
    p = pN4Spline->p;
}


double Naca4Spline::getY(double xinterp, bool bRelative) const
{
    double x = xinterp;
    if(bRelative) x = m_Output.front().x + xinterp*(m_Output.back().x - m_Output.front().x);

    double y=0;
    if(x<p) y = m/p/p* (2.0*p*x - x*x);
    else    y = m/(1.0-p)/(1.0-p) * (1.0-2.0*p + 2.0*p*x-x*x);
    return y;
}


void Naca4Spline::getCamber(double &Camber, double &xc)
{
    Camber = m;
    xc = p;
}


bool Naca4Spline::serialize(QDataStream &ar, bool bIsStoring)
{
    if(bIsStoring)
    {
        ar << m << p;
    }
    else
    {
        ar >> m >> p;
        updateSpline();
        makeCurve();
    }
    return true;
}


bool Naca4Spline::updateSpline()
{
    p = m_CtrlPt.at(1).x;
    m = m_CtrlPt.at(1).y;
    return true;
}


void Naca4Spline::makeCurve()
{
    for(uint i=0; i<m_Output.size(); i++)
    {
        double di = double(i)/double(m_Output.size()-1);
        m_Output[i] = splinePoint(di);
    }
}


Vector2d Naca4Spline::splinePoint(double u) const
{
    double x(0), y(0);
    x = u;

    if(x<p) y = m/p/p* (2.0*p*x - x*x);
    else    y = m/(1.0-p)/(1.0-p) * (1.0-2.0*p + 2.0*p*x-x*x);

    return Vector2d(x,y);
}


void Naca4Spline::splineDerivative(double u, double &dx, double &dy) const
{
    Q_UNUSED(u);
    dx = dy =0.0;
}


Vector2d Naca4Spline::normal(double const &)
{
    return Vector2d(0.0, 1.0);
}



