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


#include <QDataStream>
#include <complex>

#include <objects_global.h>

struct EigenValues
{
public:
    EigenValues()
    {
        reset();
    }

    void reset()
    {
        m_EV[0] = std::complex<double>(0.0,0.0);
        m_EV[1] = std::complex<double>(0.0,0.0);
        m_EV[2] = std::complex<double>(0.0,0.0);
        m_EV[3] = std::complex<double>(0.0,0.0);
        m_EV[4] = std::complex<double>(0.0,0.0);
        m_EV[5] = std::complex<double>(0.0,0.0);
        m_EV[6] = std::complex<double>(0.0,0.0);
        m_EV[7] = std::complex<double>(0.0,0.0);

        m_PhugoidDamping = 0.0;
        m_PhugoidFrequency = 0.0;
        m_RollDampingT2 = 0.0;
        m_ShortPeriodDamping = 0.0;
        m_ShortPeriodFrequency = 0.0;
        m_DutchRollDamping = 0.0;
        m_DutchRollFrequency = 0.0;
        m_SpiralDampingT2 = 0.0;
    }

    void computeModes()
    {
        double OmegaN, Omega1, Dsi;
        double pi = 3.141592654;
        objects::modeProperties(m_EV[2], Omega1, OmegaN, Dsi);
        m_PhugoidDamping   = Dsi;
        m_PhugoidFrequency = Omega1/2.0/pi;

        objects::modeProperties(m_EV[0], Omega1, OmegaN, Dsi);
        m_ShortPeriodFrequency = Omega1/2.0/pi;
        m_ShortPeriodDamping   = Dsi;

        objects::modeProperties(m_EV[5], Omega1, OmegaN, Dsi);
        m_DutchRollFrequency = Omega1/2.0/pi;
        m_DutchRollDamping   = Dsi;

        m_RollDampingT2    = log(2.0)/fabs(m_EV[4].real());
        m_SpiralDampingT2  = log(2.0)/fabs(m_EV[7].real());
    }

    void serializeFl5(QDataStream &ar, bool bIsStoring)
    {
        if(bIsStoring)
        {
            for(int i=0; i<8; i++)
                ar << m_EV[i].real() << m_EV[i].imag();
        }
        else
        {
            double real=0.0, imag=0.0;
            for(int i=0; i<8; i++)
            {
                ar >> real >> imag;
                m_EV[i] = std::complex<double>(real, imag);
            }
        }
    }


public:
    double m_PhugoidFrequency;        /**< the phugoid's frequency, as a result of stability analysis only */
    double m_PhugoidDamping;          /**< the phugoid's damping factor, as a result of stability analysis only */
    double m_RollDampingT2;           /**< the time to double or half for the damping of the roll-damping mode, as a result of stability analysis only */
    double m_ShortPeriodDamping;      /**< the damping of the short period mode, as a result of stability analysis only */
    double m_ShortPeriodFrequency;    /**< the frequency of the short period mode, as a result of stability analysis only */
    double m_DutchRollDamping;        /**< the damping of the Dutch roll mode, as a result of stability analysis only */
    double m_DutchRollFrequency;      /**< the frequency of the Dutch roll mode, as a result of stability analysis only */
    double m_SpiralDampingT2;         /**< the time to double or half for the damping of the spiral mode, as a result of stability analysis only >*/

    std::complex<double> m_EV[8];

};
