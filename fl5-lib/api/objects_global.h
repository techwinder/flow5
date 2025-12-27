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


#include <enums_objects.h>
#include <geom_enums.h>
#include <vector2d.h>
#include <vector3d.h>
#include <mathelem.h>


class Inertia;
class Node;
class PlaneOpp;
class PlaneXfl;
class Triangle3d;
class Boat;
class BoatOpp;

namespace objects
{

    FL5LIB_EXPORT std::string surfacePosition(xfl::enumSurfacePosition pos);

    FL5LIB_EXPORT void modeProperties(std::complex<double> lambda, double &omegaN, double &omega1, double &dsi);

    FL5LIB_EXPORT void computeSurfaceInertia(Inertia &inertia, const std::vector<Triangle3d> &triangles, const Vector3d &PartPosition);

    FL5LIB_EXPORT Vector3d windDirection(double alpha, double beta);
    FL5LIB_EXPORT Vector3d windSide(double alpha, double beta);
    FL5LIB_EXPORT Vector3d windNormal(double alpha, double beta);
    FL5LIB_EXPORT Vector3d windToGeomAxes(Vector3d const &Vw, double alpha, double beta);


    FL5LIB_EXPORT int AVLSpacing(xfl::enumDistribution distrib);


    FL5LIB_EXPORT void VLMCmnVelocity(Vector3d const &A, Vector3d const &B, Vector3d const &C, Vector3d &V, bool bAll, double fardist);
    FL5LIB_EXPORT void VLMQmnVelocity(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB, Vector3d const &C, Vector3d &V);
    FL5LIB_EXPORT void VLMQmnPotential(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB, Vector3d const &C, double &phi);
}



