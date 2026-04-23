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

#include <vector2d.h>


class FL5LIB_EXPORT GQTriangle
{
    public:
        GQTriangle(int order=3);
        double testIntegral();
        int nPoints() {return int(m_point.size());}

        void makeCoeffs(int order);

        std::vector<Vector2d> const &points() const {return m_point;}
        std::vector<double> const &weights() const {return m_weight;}

    private:

        int m_iOrder;
        std::vector<Vector2d> m_point;
        std::vector<double> m_weight;
};

