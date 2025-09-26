/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#define LENGTHPRECISION 1.e-6  /**< PRECISION to compare the proximity of two points or lines */
#define ANGLEPRECISION 1.e-6   /**< PRECISION to compare the value of two angles in degrees - used for aoa, sideslip, angle controls and panel integrals*/
#define KNOTPRECISION  1.e-6   /**< PRECISION to compare spline knots */
#define SYMMETRYPRECISION 1.e-4 /**< if |y| is less than than this value, then the point is considered to be in the xz plane of symmetry and y will be set to zero */

#define AOAPRECISION        1.e-3   /**< PRECISION used to compare the value of two angles in degrees - used for aoa, sideslip, and panel integrals*/
#define CTRLPRECISION       1.e-3   /**< PRECISION used to compare the value of two control parameters*/
#define FLAPANGLEPRECISION  0.001   /**< PRECISION used to compare the value of two flap angles in degrees. Two angles with difference less than this value are considered equal */
#define REYNOLDSPRECISION   0.1     /**< PRECISION used to compare the value of two Reynolds numbers */
