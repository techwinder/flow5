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

namespace xfl
{
    /** @enum The different types of polar available for 2D and 3D calculations. */
    enum enumPolarType {T1POLAR, T2POLAR, T3POLAR, T4POLAR, T5POLAR, T6POLAR, T7POLAR, T8POLAR, BOATPOLAR, EXTERNALPOLAR};

    enum enumAnalysisMethod {LLT, VLM1, VLM2, QUADS, TRILINEAR, TRIUNIFORM, NOMETHOD};
    enum enumBC {DIRICHLET, NEUMANN};
    enum enumRefDimension {PLANFORM, PROJECTED, CUSTOM, AUTODIMS}; // AUTO is for sails

    /** @enum The different types of wings available for a PlaneXfl. */
    enum enumType {Main, Elevator, Fin, OtherWing};
}




