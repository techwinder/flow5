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



struct T8Opp
{
        T8Opp() : m_bActive(true), m_Alpha(0), m_Beta(0), m_Vinf(1.0)
        {
        }

        T8Opp(bool bActive, double alpha, double beta, double vinf) : m_bActive(bActive), m_Alpha(alpha), m_Beta(beta), m_Vinf(vinf)
        {
        }

        bool isActive() const {return m_bActive;}
        double alpha() const {return m_Alpha;}
        double beta() const {return m_Beta;}
        double Vinf() const {return m_Vinf;}

        bool m_bActive;
        double m_Alpha;
        double m_Beta;
        double m_Vinf;
};

