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

#include <panel3.h>
#include <panel4.h>
#include <vortex.h>


void testPanel3_1();
void testPanel3_3();
void testPanel4();
void testPanel2d();
void testVortex();
void testBasisFunctions();
void testPanelNF1(bool bPositive = true);
void testPanel5();



enum PANELMETHOD {NASA4023, BASIS, VORTEX, NOPANELMETHOD};

FL5LIB_EXPORT double potentialP3(const Vector3d &pt, const std::vector<Panel3> &panel3, PANELMETHOD iMethod, bool bSource, double CoreRadius);
FL5LIB_EXPORT double potentialP4(const Vector3d &pt, const std::vector<Panel4> &panel4, PANELMETHOD iMethod, bool bSource, double CoreRadius);
FL5LIB_EXPORT Vector3d velocityP3(Vector3d const &pt, const std::vector<Panel3> &panel3, PANELMETHOD iMethod, bool bSource, double CoreRadius, Vortex::enumVortex VortexModel);
FL5LIB_EXPORT Vector3d velocityP4(Vector3d const &pt, const std::vector<Panel4> &panel4, PANELMETHOD iMethod, bool bSource, double CoreRadius);
