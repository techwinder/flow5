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


class FL5LIB_EXPORT Hanning
{
    public:
        Hanning(double frequency, double cutfrequency);

        void setFrequency(double freq) {m_Frequency=freq;}
        void setCutFrequency(double freq) {m_FilterCutFreq=freq;}
        void setFilterCoefs();
        void setHanningWindow();

        void filterSeries(const std::vector<double> &doubleData, std::vector<double> &y);


    private:

        std::vector<double> m_Window;
        std::vector<double> m_FilterCoef;

        double m_FilterCutFreq;

        double m_Frequency;
};

void testHanning(std::vector<double> &xt, std::vector<double> &series, std::vector<double> &yF);

