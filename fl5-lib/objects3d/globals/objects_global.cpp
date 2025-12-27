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

#define _MATH_DEFINES_DEFINED
#define _MATH_DEFINES_DEFINED


#include <utils.h>
#include <segment2d.h>
#include <quaternion.h>
#include <constants.h>
#include <planeopp.h>
#include <planepolar.h>
#include <foil.h>
#include <oppoint.h>
#include <polar.h>
#include <splinefoil.h>
#include <objects3d.h>
#include <planexfl.h>
#include <objects_global.h>
#include <boat.h>
#include <sail.h>
#include <boatopp.h>


void objects::modeProperties(std::complex<double> lambda, double &omegaN, double &omega1, double &zeta)
{
    omega1 = fabs(lambda.imag());

    if(omega1 > PRECISION)
    {
        omegaN = sqrt(lambda.real()*lambda.real()+omega1*omega1);
        zeta = -lambda.real()/omegaN;
    }
    else
    {
        omegaN = 0.0;
        zeta = 0.0;
    }
}


Vector3d objects::windDirection(double alpha, double beta)
{
    //change of beta sign introduced in v7.24 to be consistent with AVL
    Vector3d Vinf;
    Vinf.x =   cos(alpha*PI/180.0) * cos(-beta*PI/180.0);
    Vinf.y =   sin(-beta*PI/180.0);
    Vinf.z =   sin(alpha*PI/180.0) * cos(-beta*PI/180.0);
    return Vinf;
}


Vector3d objects::windSide(double alpha, double beta)
{
     //change of beta sign introduced in v7.24 to be consistent with AVL
    Vector3d Vinf;
    Vinf.x =  -cos(alpha*PI/180.0) * sin(-beta*PI/180.0);
    Vinf.y =   cos(-beta*PI/180.0);
    Vinf.z =  -sin(alpha*PI/180.0) * sin(-beta*PI/180.0);
    return Vinf;
}


Vector3d objects::windNormal(double alpha, double beta)
{
    (void)beta;
    // V=(0,0,1)_wind
    Vector3d Vinf;
    Vinf.x = -sin(alpha*PI/180.0);
    Vinf.y = 0.0;
    Vinf.z = cos(alpha*PI/180.0);
    return Vinf;
}


/** Used in the case of a T6 polars to convert the forces to the rotated body axes*/
Vector3d objects::windToGeomAxes(Vector3d const &Vw, double alpha, double beta)
{
    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);
    double cosb = cos(beta*PI/180.0);
    double sinb = sin(beta*PI/180.0);

    double i[9];
    i[0] = cosb*cosa;    i[1] = -sinb*cosa;     i[2] = -sina;
    i[3] = sinb;         i[4] = cosb;           i[5] = 0;
    i[6] = cosb*sina;    i[7] = -sinb*sina;     i[8] = cosa;

    Vector3d Vg;
    Vg.x = i[0]*Vw.x +i[1]*Vw.y + i[2]*Vw.z;
    Vg.y = i[3]*Vw.x +i[4]*Vw.y + i[5]*Vw.z;
    Vg.z = i[6]*Vw.x +i[7]*Vw.y + i[8]*Vw.z;
    return Vg;
}


void objects::computeSurfaceInertia(Inertia &inertia, std::vector<Triangle3d> const &triangles, Vector3d const &PartPosition)
{
    Vector3d cog_s;
    double CoGIxx=0, CoGIyy=0, CoGIzz=0, CoGIxz=0;
    double Ixx=0.0,Iyy=0.0,Izz=0.0,Ixz=0.0;

    cog_s.set(0.0,0.0,0.0);

    // compute the CoG position = average of all triangle's CoG
    // compute the inertia tensor in the body axis
    // using unit density, i.e. triangle mass = area
    double totalarea = 0.0;
    for(uint it=0; it<triangles.size(); it++)
    {
        const Triangle3d &t3 = triangles.at(it);
        cog_s += t3.CoG_g() * t3.area();

        Ixx +=  t3.area() * (t3.CoG_g().y*t3.CoG_g().y + t3.CoG_g().z*t3.CoG_g().z);
        Iyy +=  t3.area() * (t3.CoG_g().x*t3.CoG_g().x + t3.CoG_g().z*t3.CoG_g().z);
        Izz +=  t3.area() * (t3.CoG_g().x*t3.CoG_g().x + t3.CoG_g().y*t3.CoG_g().y);
        // sign modification in v7.13 from negative to positive
        Ixz += t3.area() * (t3.CoG_g().x*t3.CoG_g().z);

        totalarea += t3.area();
    }
    cog_s *= 1.0/totalarea;
    //    m_Inertia.setCoG(CoG);

    //Apply Huyghens/Steiner theorem to get inertia tensor in CoG reference frame
    double d2 = cog_s.x*cog_s.x + cog_s.y*cog_s.y + cog_s.z*cog_s.z;
    CoGIxx = Ixx - totalarea * (d2 - cog_s.x*cog_s.x);
    CoGIyy = Iyy - totalarea * (d2 - cog_s.y*cog_s.y);
    CoGIzz = Izz - totalarea * (d2 - cog_s.z*cog_s.z);
    CoGIxz = Ixz - totalarea * (   - cog_s.x*cog_s.z);

    double density = inertia.structuralMass()/totalarea;
    CoGIxx *= density;
    CoGIyy *= density;
    CoGIzz *= density;
    CoGIxz *= density;

    cog_s = cog_s-PartPosition;

    inertia.setCoG_s(cog_s);
    inertia.setIxx_s(CoGIxx);
    inertia.setIyy_s(CoGIyy);
    inertia.setIzz_s(CoGIzz);
    inertia.setIxz_s(CoGIxz);
}


std::string objects::surfacePosition(xfl::enumSurfacePosition pos)
{
    switch(pos)
    {
        case xfl::BOTSURFACE:
            return "BOTTOM";
            break;
        case xfl::MIDSURFACE:
            return "MID";
            break;
        case xfl::TOPSURFACE:
            return "TOP";
            break;
        case xfl::SIDESURFACE:
            return "SIDE";
            break;
        case xfl::FUSESURFACE:
            return "FUSE";
            break;
        case xfl::WAKESURFACE:
            return "WAKE";
            break;
        default:
        case xfl::NOSURFACE:
            return "NOSURFACE";
            break;
    }
}


int objects::AVLSpacing(xfl::enumDistribution distrib)
{

    /*         AVL spacing
               3.0        equal         |   |   |   |   |   |   |   |   |
               2.0        sine          || |  |   |    |    |     |     |
               1.0        cosine        ||  |    |      |      |    |  ||
               0.0        equal         |   |   |   |   |   |   |   |   |
              -1.0        cosine        ||  |    |      |      |    |  ||
              -2.0       -sine          |     |     |    |    |   |  | ||
              -3.0        equal         |   |   |   |   |   |   |   |   | */
    switch (distrib)
    {
        default:
        case xfl::UNIFORM:
            return 0;
        case xfl::COSINE:
        case xfl::TANH:
            return 1;
        case  xfl::SINE:
        case  xfl::EXP:
            return 2;
        case  xfl::INV_SINE:
        case  xfl::INV_EXP:
            return -2;
    }
    return 0;
}


/**
 * HORSESHOE VORTEX FORMULATION
 *
 *    LA__________LB               |
 *    |           |                |
 *    |           |                | freestream velocity vector
 *    |           |                |
 *    |           |               \/
 *    |           |
 *   \/          \/
 *
 *
 * Returns the velocity greated by a horseshoe vortex with unit circulation at a distant point
 *
 * Notes :
 *  - The geometry has been rotated by the sideslip angle, hence, there is no need to align the trailing vortices with sideslip
 *  - Vectorial operations are written inline to save computing times -->longer code, but 4x more efficient....
 *
 * @param A the left point of the bound vortex
 * @param B the right point of the bound vortex
 * @param C the point where the velocity is calculated
 * @param V the resulting velocity vector at point C
 * @param bAll true if the influence of the bound vortex should be evaluated; false for a distant point in the far field.
 */
void objects::VLMCmnVelocity(Vector3d const &A, Vector3d const &B, Vector3d const &C,
                            Vector3d &V, bool bAll, double fardist)
{
    Vector3d velseg, Far;
    V.x = 0.0;
    V.y = 0.0;
    V.z = 0.0;

    if(bAll)
    {
        velseg = vortexInducedVelocity(A, B, C, Vortex::coreRadius());
        V += velseg;
    }

    // Create Far points to align the trailing vortices with the reference axis
    // The trailing vortex legs are not aligned with the free-stream, i.a.w. the small angle approximation
    // If this approximation is not valid, then the geometry should be tilted in the polar definition

    // calculate left contribution
    Far.x = A.x +  fardist;
    Far.y = A.y ;
    Far.z = A.z;// + (Far_x-A.x) * tan(m_Alpha*PI/180.0);
    velseg = vortexInducedVelocity(Far, A, C, Vortex::coreRadius());
    V += velseg;


    // calculate right vortex contribution
    Far.x = B.x +  fardist;
    Far.y = B.y;
    Far.z = B.z;// + (Far_x-B.x) * tan(m_Alpha*PI/180.0);
    velseg = vortexInducedVelocity(B, Far, C, Vortex::coreRadius());
    V += velseg;
}


/** No explicit formulation of the potential for a vortex filament.
 * Use instead the equivalence of the vortex ring to a uniform doublet strength panel */
void objects::VLMQmnPotential(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB,
                     Vector3d const &C, double &phi)
{
    // make a false quad Panel
    Panel4 p4Ring(LA, LB, TA, TB);

    // get the induced potential
    p4Ring.doubletN4023Potential(C, false, phi, Vortex::coreRadius(), true);
}


void objects::VLMQmnVelocity(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB, Vector3d const &C, Vector3d &V)
{
    Vector3d const m_pR[] = {LB, TB, TA, LA, LB};
    Vector3d velseg;
    V.x = 0.0;
    V.y = 0.0;
    V.z = 0.0;

    for (int i=0; i<4; i++)
    {
        velseg = vortexInducedVelocity(m_pR[i], m_pR[i+1], C, Vortex::coreRadius());
        V+= velseg;
    }
}



