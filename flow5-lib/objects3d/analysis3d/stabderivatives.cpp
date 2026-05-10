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

#include <cmath>


#include <stabderivatives.h>


StabDerivatives::StabDerivatives()
{
    reset();
    m_Span=m_Area=m_MAC=0;
    m_QInf  = 0.0;
    m_rho  = 0.0;
    m_CoG_x = 0.0;
    m_Mass  = 0.0;
}


void StabDerivatives::setMetaData(double span, double mac, double area, double u0, double mass, double CoG_x, double rho)
{
    m_Span  = span;
    m_MAC   = mac;
    m_Area  = area;
    m_rho   = rho;
    m_Mass  = mass;
    m_CoG_x = CoG_x;
    m_QInf  = u0;
}


void StabDerivatives::reset()
{
    //Dimensional stability derivatives
    Xu = Xw = Xq = Zu = Zw = Zq = Mu = Mw = Mq = Zwp = Mwp = 0.0;
    Yv = Yp = Yr = Lv = Lp = Lr = Nv = Np = Nr = 0.0;

    ControlNames.clear();
    Xde.clear();
    Yde.clear();
    Zde.clear();
    Lde.clear();
    Mde.clear();
    Nde.clear();


    CXu = CZu = Cmu = 0.0;
    CXa = CZa = Cma = CXq = CZq = Cmq = CYb = CYp = CYr = Clb = Clp = Clr = Cnb = Cnp = Cnr = 0.0;
    CXe.clear();
    CYe.clear();
    CZe.clear();
    CLe.clear();
    CMe.clear();
    CNe.clear();

    XNP = 0.0;
}


void StabDerivatives::resizeControlDerivatives(int n)
{
    ControlNames.resize(n);
    Xde.resize(n);
    Yde.resize(n);
    Zde.resize(n);
    Lde.resize(n);
    Mde.resize(n);
    Nde.resize(n);

    CXe.resize(n);
    CYe.resize(n);
    CZe.resize(n);
    CLe.resize(n);
    CMe.resize(n);
    CNe.resize(n);
}


void StabDerivatives::computeNDStabDerivatives()
{
    double u0 = m_QInf;
    double mac = m_MAC;
    double q = 1./2. * m_rho * u0 * u0;
    double b = m_Span;
    double S = m_Area;

    double theta0 = 0.0;//steady level flight only?

    double Cw0 = m_Mass * 9.81/q/S; //E&R p.127
    //    Cx0 =  Cw0 * sin(theta0); //E&R p.119
    //    Cz0 = -Cw0 * cos(theta0); //E&R p.118

    //E&R p. 118, table 4.4
    CXu = (Xu - m_rho * u0*S*Cw0*sin(theta0))/(0.5*m_rho*u0*S);
    CZu = (Zu + m_rho * u0*S*Cw0*cos(theta0))/(0.5*m_rho*u0*S);
//    qDebug("u0={:11g}  Zu={:11g}  Cw0={:11g}  CZu={:11g}", u0, Zu, Cw0, CZu);
    Cmu = Mu /(0.5*m_rho*u0*mac*S);
    CXa = Xw /(0.5*m_rho*u0*S);
    CZa = Zw /(0.5*m_rho*u0*S);
    Cma = Mw /(0.5*m_rho*u0*mac*S);
    CXq = Xq /(.25*m_rho*u0*mac*S);
    CZq = Zq /(.25*m_rho*u0*mac*S);
    Cmq = Mq /(.25*m_rho*u0*mac*mac*S);

    XNP = m_CoG_x + Cma/CZa * mac; //E&R (eq. 2.3.5 p.29)

    CYb = Yv*    u0     /(q*S);
    CYp = Yp* 2.*u0     /(q*S*b);
    CYr = Yr* 2.*u0     /(q*S*b);
    Clb = Lv*    u0     /(q*S*b);
    Clp = Lp*(2.*u0/b)  /(q*S*b);
    Clr = Lr*(2.*u0/b)  /(q*S*b);
    Cnb = Nv*    u0     /(q*S*b);
    Cnp = Np*(2.*u0/b)  /(q*S*b);
    Cnr = Nr*(2.*u0/b)  /(q*S*b);

    CXe.resize(Xde.size());
    CYe.resize(Xde.size());
    CZe.resize(Xde.size());
    CLe.resize(Xde.size());
    CMe.resize(Xde.size());
    CNe.resize(Xde.size());
    for(unsigned int i=0; i<Xde.size(); i++)
    {
        CXe[i] = Xde.at(i)/(q*S);
        CYe[i] = Yde.at(i)/(q*S);
        CZe[i] = Zde.at(i)/(q*S);
        CLe[i] = Lde.at(i)/(q*S*b);
        CMe[i] = Mde.at(i)/(q*S*mac);
        CNe[i] = Nde.at(i)/(q*S*b);
    }
}


