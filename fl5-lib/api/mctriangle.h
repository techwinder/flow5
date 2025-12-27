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

#include <vector3d.h>
#include <triangle3d.h>



/**
 * @brief Implements the method described in
 * "Potential integrals on triangles, Michael Carley, January 25, 2012"
 * This class implements a sub-triangle defined by the projection of the field point and two vertices, i.a.w. fig. 2.
 */
class FL5LIB_EXPORT MCTriangle : public Triangle3d
{
	friend class  Panel3;
public:
	MCTriangle();
	MCTriangle(const Vector3d &V0, const Vector3d &V1, const Vector3d &V2, const Vector3d &N);
	void setTriangle();
	void setTriangle(const Vector3d &V0, const Vector3d &V1, const Vector3d &V2, const Vector3d &N);
	void setIntermediateVariables(double thet);
    void integrate(const Vector3d &Pt, bool bSelf, double *I1, double *I3, double *I5, bool bGradients=false);

	double table1(int iLine, bool bInPlane) const;

	double Ipmn(int p, int m, int n) const;
	double Jmn(int m, int n) const;


	static void resetNullCount() {s_NullCount=0;}
	static long nullCount() {return s_NullCount;}

private:

	Vector3d ptEval;

	double m_z;                 /**< the altitude of the control point, in local coordinates */
	double m_a;                 /**< the slope coefficient of side 3, i.a.w. eq. 4 */
	double orientation;         /**< the triangle's orientation w.r.t. the parent panel's normal, i.a.w. eq. 33 */
	double r1, r2;              /**< the length of sides 1 and 2, i.a.w. fig. 1 */
	double thetaMax;            /**< the angle between sides one and 2, i.a.w. fig. 1 */
	double phi;                 /**< the angle defined by eq. 8 */
	double area;                /**< the triangles absolute (?) area */


    /** temporary variables used in the calculation of integrals */
	double theta;
	double sinT, sinT2, sinT3;
	double cosT, cosT2, cosT3;
	double beta, beta2, beta3;
	double alfa, alfa2, alfa3, alfa4, alfa5;
	double alfap, alfap2;
	double delta, delta2;
	double fz, z2;

	static long s_NullCount;
};

