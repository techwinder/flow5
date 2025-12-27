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



#include <s7spline.h>


s7Spline::s7Spline() : Spline()
{
    m_SplineType = Spline::BSPLINE;
    m_SlopePoint[0].set(0.1,0.1);
    m_SlopePoint[1].set(0.8,0.1);
}


void s7Spline::makeCurve()
{
    for(uint i=0; i<m_Output.size(); i++)
    {
        double di = double(i)/double(m_Output.size()-1);
        m_Output[i] = splinePoint(di);
    }
}
