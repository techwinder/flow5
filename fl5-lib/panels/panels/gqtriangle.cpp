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


#include <math.h>

#include <gqtriangle.h>

GQTriangle::GQTriangle(int order)
{
    makeCoeffs(order);
}


double func(double x, double y)
{
    return     x+y;
}


double GQTriangle::testIntegral()
{
    Vector2d S[3]; // todo set for testing;

    double area=fabs(S[0].x*(S[1].y-S[2].y) + S[1].x*(S[2].y-S[0].y) + S[2].x*(S[0].y-S[1].y))/2.0;

    double z = 0.0;
    for(size_t j=0; j<m_point.size(); j++)
    {
        double x = S[0].x*(1.0-m_point.at(j).x-m_point.at(j).y) + S[1].x*m_point.at(j).x + S[2].x*m_point.at(j).y;
        double y = S[0].y*(1.0-m_point.at(j).x-m_point.at(j).y) + S[1].y*m_point.at(j).x + S[2].y*m_point.at(j).y;
        z += func(x,y)*m_weight.at(j);
    }
    z *= area;
    return z;
}


void GQTriangle::makeCoeffs(int order)
{
    if(order<1) order = 3;
    if(order>8) order = 8;
    m_iOrder = order;
    m_point.clear();
    m_weight.clear();
    switch(order)
    {
        case 1:
        {
            m_point.push_back({0.33333333333333, 0.33333333333333});  m_weight.push_back(1.00000000000000);
            break;
        }
        case 2:
        {
            m_point.push_back({0.16666666666667, 0.16666666666667});  m_weight.push_back(0.33333333333333);
            m_point.push_back({0.16666666666667, 0.66666666666667});  m_weight.push_back(0.33333333333333);
            m_point.push_back({0.66666666666667, 0.16666666666667});  m_weight.push_back(0.33333333333333);
            break;
        }
        default:
        case 3:
        {
            m_point.push_back({0.33333333333333, 0.33333333333333});  m_weight.push_back(-0.56250000000000);
            m_point.push_back({0.20000000000000, 0.20000000000000});  m_weight.push_back(0.52083333333333);
            m_point.push_back({0.20000000000000, 0.60000000000000});  m_weight.push_back(0.52083333333333);
            m_point.push_back({0.60000000000000, 0.20000000000000});  m_weight.push_back(0.52083333333333);
            break;
        }
        case 4:
        {
            m_point.push_back({0.44594849091597, 0.44594849091597});  m_weight.push_back(0.22338158967801);
            m_point.push_back({0.44594849091597, 0.10810301816807});  m_weight.push_back(0.22338158967801);
            m_point.push_back({0.10810301816807, 0.44594849091597});  m_weight.push_back(0.22338158967801);
            m_point.push_back({0.09157621350977, 0.09157621350977});  m_weight.push_back(0.10995174365532);
            m_point.push_back({0.09157621350977, 0.81684757298046});  m_weight.push_back(0.10995174365532);
            m_point.push_back({0.81684757298046, 0.09157621350977});  m_weight.push_back(0.10995174365532);
            break;
        }
        case 5:
        {

            m_point.push_back({0.33333333333333, 0.33333333333333});  m_weight.push_back(0.22500000000000);
            m_point.push_back({0.47014206410511, 0.47014206410511});  m_weight.push_back(0.13239415278851);
            m_point.push_back({0.47014206410511, 0.05971587178977});  m_weight.push_back(0.13239415278851);
            m_point.push_back({0.05971587178977, 0.47014206410511});  m_weight.push_back(0.13239415278851);
            m_point.push_back({0.10128650732346, 0.10128650732346});  m_weight.push_back(0.12593918054483);
            m_point.push_back({0.10128650732346, 0.79742698535309});  m_weight.push_back(0.12593918054483);
            m_point.push_back({0.79742698535309, 0.10128650732346});  m_weight.push_back(0.12593918054483);
            break;
        }
        case 6:
        {
            m_point.push_back({0.24928674517091, 0.24928674517091});  m_weight.push_back(0.11678627572638);
            m_point.push_back({0.24928674517091, 0.50142650965818});  m_weight.push_back(0.11678627572638);
            m_point.push_back({0.50142650965818, 0.24928674517091});  m_weight.push_back(0.11678627572638);
            m_point.push_back({0.06308901449150, 0.06308901449150});  m_weight.push_back(0.05084490637021);
            m_point.push_back({0.06308901449150, 0.87382197101700});  m_weight.push_back(0.05084490637021);
            m_point.push_back({0.87382197101700, 0.06308901449150});  m_weight.push_back(0.05084490637021);
            m_point.push_back({0.31035245103378, 0.63650249912140});  m_weight.push_back(0.08285107561837);
            m_point.push_back({0.63650249912140, 0.05314504984482});  m_weight.push_back(0.08285107561837);
            m_point.push_back({0.05314504984482, 0.31035245103378});  m_weight.push_back(0.08285107561837);
            m_point.push_back({0.63650249912140, 0.31035245103378});  m_weight.push_back(0.08285107561837);
            m_point.push_back({0.31035245103378, 0.05314504984482});  m_weight.push_back(0.08285107561837);
            m_point.push_back({0.05314504984482, 0.63650249912140});  m_weight.push_back(0.08285107561837);
            break;
        }
        case 7:
        {
            m_point.push_back({0.33333333333333, 0.33333333333333});  m_weight.push_back(-0.14957004446768);
            m_point.push_back({0.26034596607904, 0.26034596607904});  m_weight.push_back(0.17561525743321);
            m_point.push_back({0.26034596607904, 0.47930806784192});  m_weight.push_back(0.17561525743321);
            m_point.push_back({0.47930806784192, 0.26034596607904});  m_weight.push_back(0.17561525743321);
            m_point.push_back({0.06513010290222, 0.06513010290222});  m_weight.push_back(0.05334723560884);
            m_point.push_back({0.06513010290222, 0.86973979419557});  m_weight.push_back(0.05334723560884);
            m_point.push_back({0.86973979419557, 0.06513010290222});  m_weight.push_back(0.05334723560884);
            m_point.push_back({0.31286549600487, 0.63844418856981});  m_weight.push_back(0.07711376089026);
            m_point.push_back({0.63844418856981, 0.04869031542532});  m_weight.push_back(0.07711376089026);
            m_point.push_back({0.04869031542532, 0.31286549600487});  m_weight.push_back(0.07711376089026);
            m_point.push_back({0.63844418856981, 0.31286549600487});  m_weight.push_back(0.07711376089026);
            m_point.push_back({0.31286549600487, 0.04869031542532});  m_weight.push_back(0.07711376089026);
            m_point.push_back({0.04869031542532, 0.63844418856981});  m_weight.push_back(0.07711376089026);
            break;
        }
        case 8:
        {
            m_point.push_back({0.33333333333333, 0.33333333333333});  m_weight.push_back(0.14431560767779);
            m_point.push_back({0.45929258829272, 0.45929258829272});  m_weight.push_back(0.09509163426728);
            m_point.push_back({0.45929258829272, 0.08141482341455});  m_weight.push_back(0.09509163426728);
            m_point.push_back({0.08141482341455, 0.45929258829272});  m_weight.push_back(0.09509163426728);
            m_point.push_back({0.17056930775176, 0.17056930775176});  m_weight.push_back(0.10321737053472);
            m_point.push_back({0.17056930775176, 0.65886138449648});  m_weight.push_back(0.10321737053472);
            m_point.push_back({0.65886138449648, 0.17056930775176});  m_weight.push_back(0.10321737053472);
            m_point.push_back({0.05054722831703, 0.05054722831703});  m_weight.push_back(0.03245849762320);
            m_point.push_back({0.05054722831703, 0.89890554336594});  m_weight.push_back(0.03245849762320);
            m_point.push_back({0.89890554336594, 0.05054722831703});  m_weight.push_back(0.03245849762320);
            m_point.push_back({0.26311282963464, 0.72849239295540});  m_weight.push_back(0.02723031417443);
            m_point.push_back({0.72849239295540, 0.00839477740996});  m_weight.push_back(0.02723031417443);
            m_point.push_back({0.00839477740996, 0.26311282963464});  m_weight.push_back(0.02723031417443);
            m_point.push_back({0.72849239295540, 0.26311282963464});  m_weight.push_back(0.02723031417443);
            m_point.push_back({0.26311282963464, 0.00839477740996});  m_weight.push_back(0.02723031417443);
            m_point.push_back({0.00839477740996, 0.72849239295540});  m_weight.push_back(0.02723031417443);
            break;
        }
    }
}
