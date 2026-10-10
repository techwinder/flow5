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


#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>

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

        /**
         * Sets the figures of the modes from the eigenvalues. The modes are identified by their character rather than by their
         * index, since the eigenvalues are sorted on their real parts only: the short period is the complex pair of the
         * longitudinal roots with the higher frequency and the phugoid the other pair; the Dutch roll is the complex pair of the
         * lateral roots, the roll mode the most negative real root and the spiral the real root closest to zero. A mode that is not
         * oscillatory (a phugoid split in two real roots, for instance) has zero frequency and damping, and is not replaced by
         * another root.
         */
        void computeModes()
        {
            double omegaN(0), omega1(0), zeta(0);
            double pi = 3.141592654;

            std::vector<std::complex<double>> pairs;
            std::vector<double> reals;

            splitRoots(m_EV, 4, pairs, reals);
            m_ShortPeriodFrequency = m_ShortPeriodDamping = 0.0;
            m_PhugoidFrequency = m_PhugoidDamping = 0.0;
            if(pairs.size()>0)
            {
                objects::modeProperties(pairs.at(0), omegaN, omega1, zeta);
                m_ShortPeriodFrequency = omegaN/2.0/pi;
                m_ShortPeriodDamping   = zeta;
            }
            if(pairs.size()>1)
            {
                objects::modeProperties(pairs.at(1), omegaN, omega1, zeta);
                m_PhugoidFrequency = omegaN/2.0/pi;
                m_PhugoidDamping   = zeta;
            }

            splitRoots(m_EV+4, 4, pairs, reals);
            m_DutchRollFrequency = m_DutchRollDamping = 0.0;
            m_RollDampingT2 = m_SpiralDampingT2 = 0.0;
            if(pairs.size()>0)
            {
                objects::modeProperties(pairs.at(0), omegaN, omega1, zeta);
                m_DutchRollFrequency = omegaN/2.0/pi;
                m_DutchRollDamping   = zeta;
            }
            std::sort(reals.begin(), reals.end());
            if(reals.size()>0) m_RollDampingT2 = log(2.0)/fabs(reals.front());
            if(reals.size()>1)
            {
                double spiral = reals.at(1);
                for(unsigned int i=2; i<reals.size(); i++) if(fabs(reals.at(i))<fabs(spiral)) spiral = reals.at(i);
                m_SpiralDampingT2 = log(2.0)/fabs(spiral);
            }
        }


        /** Splits n eigenvalues into the complex conjugate pairs, one representative with a positive imaginary part for each and sorted by decreasing frequency, and the real roots */
        static void splitRoots(std::complex<double> const *ev, int n, std::vector<std::complex<double>> &pairs, std::vector<double> &reals)
        {
            pairs.clear();
            reals.clear();
            std::vector<bool> used(n, false);
            for(int i=0; i<n; i++)
            {
                if(used.at(i)) continue;
                used[i] = true;
                if(fabs(ev[i].imag())>1.e-15)
                {
                    for(int j=i+1; j<n; j++)
                    {
                        if(!used.at(j) && std::abs(std::conj(ev[j])-ev[i])<1.e-6*std::max(1.0, std::abs(ev[i])))
                        {
                            used[j] = true;
                            break;
                        }
                    }
                    pairs.push_back({ev[i].real(), fabs(ev[i].imag())});
                }
                else reals.push_back(ev[i].real());
            }
            std::sort(pairs.begin(), pairs.end(), [](std::complex<double> const &a, std::complex<double> const &b){return a.imag()>b.imag();});
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
