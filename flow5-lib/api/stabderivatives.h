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

class FL5LIB_EXPORT StabDerivatives
{
    public:
        StabDerivatives();

        void setMetaData(double span, double mac, double area, double u0, double mass, double CoG_x, double rho);

        void reset();

        void resizeControlDerivatives(int n);

        void computeNDStabDerivatives();
        bool serializeFl5(QDataStream &ar, bool bIsStoring);

        //--------------- VARIABLES --------------

        // Operating point data
        double m_Span, m_Area, m_MAC;
        double m_QInf;
        double m_Mass;
        double m_CoG_x;
        double m_rho;


        // DIMENSIONAL derivatives
        // longitudinal
        double Xu, Xw, Zu, Zw, Xq, Zq, Mu, Mw, Mq;//first order
        double Zwp, Mwp;                          //second order derivatives, cannot be calculated by a panel method, set to zero.
        // lateral
        double Yv, Yp, Yr, Lv, Lp, Lr, Nv, Np, Nr;//first order

        // control derivatives

        std::vector<std::string> ControlNames;
        std::vector<double> Xde, Yde, Zde, Lde, Mde, Nde;

        // NON DIMENSIONAL derivatives
        // stability
        double CXu, CZu, Cmu, CXa, CZa, Cma, CXq, CZq, Cmq, CYb, CYp, CYr, Clb, Clp, Clr, Cnb, Cnp, Cnr;
        // control
        std::vector<double>  CXe,CYe,CZe;
        std::vector<double>  CLe,CMe,CNe;

        // result
        double XNP;                 /**< Neutral point x-position resulting from stability analysis */

};
