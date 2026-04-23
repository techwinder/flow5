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

// implements the Savitzky-Golay smoothing filter

#include <vector>

namespace sgsmooth
{
    void makeCoefficients();

    bool smooth_nonuniform(int deg, int w, const std::vector<double> &x, const std::vector<double> &y, std::vector<double> &ysm);

    void smooth(int order, const std::vector<double> &fin, std::vector<double> &fout);

    void testSmooth(std::vector<double> &f, std::vector<double> &g5, std::vector<double> &g7, std::vector<double> &g9);
    void testSmoothNonUniform();

}

