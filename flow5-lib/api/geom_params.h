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

#define LENGTHPRECISION 1.e-6  /**< PRECISION to compare the proximity of two points or lines */
#define ANGLEPRECISION 1.e-6   /**< PRECISION to compare the value of two angles in degrees - used for aoa, sideslip, angle controls and panel integrals*/
#define KNOTPRECISION  1.e-6   /**< PRECISION to compare spline knots */
#define SYMMETRYPRECISION 1.e-4 /**< if |y| is less than than this value, then the point is considered to be in the xz plane of symmetry and y will be set to zero */

#define AOAPRECISION        1.e-3   /**< PRECISION used to compare the value of two angles in degrees - used for aoa, sideslip, and panel integrals*/
#define CLPRECISION         1.e-4   /**< PRECISION used to compare the value of two Lift coefficients*/
#define CTRLPRECISION       1.e-3   /**< PRECISION used to compare the value of two control parameters*/
#define FLAPANGLEPRECISION  0.001   /**< PRECISION used to compare the value of two flap angles in degrees. Two angles with difference less than this value are considered equal */
#define REYNOLDSPRECISION   0.1     /**< PRECISION used to compare the value of two Reynolds numbers */
