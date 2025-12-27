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

#include <QString>



#include <panel4.h>
#include <panel3.h>
#include <vortex.h>
#include <panelprecision.h>

#include <units.h>
#include <utils.h>


double Panel4::s_VortexFracPos = 0.25;
double Panel4::s_CtrlPos   = 0.75;


Panel4::Panel4(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB)
{
    reset();
    setPanelFrame(LA, LB, TA, TB);
}


/**
* Defines the vortex and panel geometrical properties necessary for the VLM and panel calculations.
* Assumes that the member variable node[4] has been preset
*/
void Panel4::setPanelFrame()
{
    //set the boundary conditions from existing nodes
    setPanelFrame(m_Node[0], m_Node[3], m_Node[1], m_Node[2]);
}

/**
* Defines the vortex and panel geometrical properties necessary for the VLM and panel calculations.
* Assumes that the array of nodes has been preset
*/
void Panel4::setPanelFrame(Vector3d *pNode)
{
    //set the boundary conditions from existing nodes
    setPanelFrame(pNode[m_iLA], pNode[m_iLB], pNode[m_iTA], pNode[m_iTB]);
}


/**
* Constructs the vortex and panel properties necessary for the VLM and panel calculations, 
  based on the absolute position of the four corner nodes.
 *
 * @param LA the position of the leading edge left node.
 * @param LB the position of the leading edge right node.
 * @param TA the position of the trailing edge left node.
 * @param TB the position of the trailing edge rightt node.
 *
 *    LA__________LB               |
 *    |           |                |
 *    |           |                | freestream velocity vector
 *    |           |                |
 *    |           |               \/
 *    |           |
 *    TA__________TB
 *
 *
*/
void Panel4::setPanelFrame(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB)
{
    Vector3d TALB, LATB, MidA, MidB;
    Vector3d smp, smq;

    if(fabs(LA.y)<1.e-5 && fabs(TA.y)<1.e-5 && fabs(LB.y)<1.e-5 && fabs(TB.y)<1.e-5)
        m_bIsInSymPlane = true;
    else m_bIsInSymPlane = false;

    m_Node[0].set(LA);
    m_Node[1].set(TA);
    m_Node[2].set(TB);
    m_Node[3].set(LB);

    LATB.x = TB.x - LA.x;
    LATB.y = TB.y - LA.y;
    LATB.z = TB.z - LA.z;
    TALB.x = LB.x - TA.x;
    TALB.y = LB.y - TA.y;
    TALB.z = LB.z - TA.z;

    //assert(TALB.norm()>DISTANCEPRECISION);
    //assert(LATB.norm()>DISTANCEPRECISION);
    //    if(isBotPanel()) Normal = TALB * LATB;
    //    else
    m_Normal = LATB * TALB;
    m_Area = m_Normal.norm()/2.0;
    m_Normal.normalize();

    m_VA.x = LA.x*(1.0-s_VortexFracPos) + TA.x*s_VortexFracPos;
    m_VA.y = LA.y*(1.0-s_VortexFracPos) + TA.y*s_VortexFracPos;
    m_VA.z = LA.z*(1.0-s_VortexFracPos) + TA.z*s_VortexFracPos;

    m_VB.x = LB.x*(1.0-s_VortexFracPos) + TB.x*s_VortexFracPos;
    m_VB.y = LB.y*(1.0-s_VortexFracPos) + TB.y*s_VortexFracPos;
    m_VB.z = LB.z*(1.0-s_VortexFracPos) + TB.z*s_VortexFracPos;

    MidA.x = LA.x*(1.0-s_CtrlPos) + TA.x*s_CtrlPos;
    MidA.y = LA.y*(1.0-s_CtrlPos) + TA.y*s_CtrlPos;
    MidA.z = LA.z*(1.0-s_CtrlPos) + TA.z*s_CtrlPos;

    MidB.x = LB.x*(1.0-s_CtrlPos) + TB.x*s_CtrlPos;
    MidB.y = LB.y*(1.0-s_CtrlPos) + TB.y*s_CtrlPos;
    MidB.z = LB.z*(1.0-s_CtrlPos) + TB.z*s_CtrlPos;

    m_CtrlPt.x = (MidA.x+MidB.x)/2.0;
    m_CtrlPt.y = (MidA.y+MidB.y)/2.0;
    m_CtrlPt.z = (MidA.z+MidB.z)/2.0;

    // the center is the average of the 4 corner points // TN-4023 p24
/*    m_CollPt.x = (LA.x + LB.x + TA.x + TB.x)/4.0;
    m_CollPt.y = (LA.y + LB.y + TA.y + TB.y)/4.0;
    m_CollPt.z = (LA.z + LB.z + TA.z + TB.z)/4.0;*/

    int nv = 1;
    m_CollPt.set(LA);
    if(!LB.isSame(LA,0.001)) {m_CollPt += LB; nv++;}
    if(!TB.isSame(LB,0.001)) {m_CollPt += TB; nv++;}
    if(!TA.isSame(TB,0.001)) {m_CollPt += TA; nv++;}
    m_CollPt *= 1.0/double(nv);

    //Use VSAERO figure 8. p23
    //    if(m_iPos==THINSURFACE || m_Pos==TOPSURFACE || m_Pos==BODYSURFACE)
    //    {
    m_m.x = (LB.x + TB.x) *0.5 - m_CollPt.x;
    m_m.y = (LB.y + TB.y) *0.5 - m_CollPt.y;
    m_m.z = (LB.z + TB.z) *0.5 - m_CollPt.z;
    /*    }
    else
    {
        m.x = (LA.x + TA.x) *0.5 - CollPt.x;
        m.y = (LA.y + TA.y) *0.5 - CollPt.y;
        m.z = (LA.z + TA.z) *0.5 - CollPt.z;
    }*/
    m_m.normalize();

    m_l.x =  m_m.y * m_Normal.z - m_m.z * m_Normal.y;
    m_l.y = -m_m.x * m_Normal.z + m_m.z * m_Normal.x;
    m_l.z =  m_m.x * m_Normal.y - m_m.y * m_Normal.x;

    smq.x  = (LB.x + TB.x) * 0.5 - m_CollPt.x;
    smq.y  = (LB.y + TB.y) * 0.5 - m_CollPt.y;
    smq.z  = (LB.z + TB.z) * 0.5 - m_CollPt.z;
    smp.x  = (TB.x + TA.x) * 0.5 - m_CollPt.x;
    smp.y  = (TB.y + TA.y) * 0.5 - m_CollPt.y;
    smp.z  = (TB.z + TA.z) * 0.5 - m_CollPt.z;

    SMP = smp.norm();
    SMQ = smq.norm();

    m_MaxSize = std::max(SMP, SMQ);

    m_CF.setOrigin(m_CollPt);
    m_CF.setIJK(m_l,m_m,m_Normal);
    double const*lij = m_CF.rotationMatrix();

    if(m_Pos>xfl::MIDSURFACE)
    {
        P1.x = lij[0]*(LA.x-m_CollPt.x) + lij[1]*(LA.y-m_CollPt.y) + lij[2]*(LA.z-m_CollPt.z);
        P1.y = lij[3]*(LA.x-m_CollPt.x) + lij[4]*(LA.y-m_CollPt.y) + lij[5]*(LA.z-m_CollPt.z);
        P1.z = lij[6]*(LA.x-m_CollPt.x) + lij[7]*(LA.y-m_CollPt.y) + lij[8]*(LA.z-m_CollPt.z);
        P2.x = lij[0]*(LB.x-m_CollPt.x) + lij[1]*(LB.y-m_CollPt.y) + lij[2]*(LB.z-m_CollPt.z);
        P2.y = lij[3]*(LB.x-m_CollPt.x) + lij[4]*(LB.y-m_CollPt.y) + lij[5]*(LB.z-m_CollPt.z);
        P2.z = lij[6]*(LB.x-m_CollPt.x) + lij[7]*(LB.y-m_CollPt.y) + lij[8]*(LB.z-m_CollPt.z);
        P3.x = lij[0]*(TB.x-m_CollPt.x) + lij[1]*(TB.y-m_CollPt.y) + lij[2]*(TB.z-m_CollPt.z);
        P3.y = lij[3]*(TB.x-m_CollPt.x) + lij[4]*(TB.y-m_CollPt.y) + lij[5]*(TB.z-m_CollPt.z);
        P3.z = lij[6]*(TB.x-m_CollPt.x) + lij[7]*(TB.y-m_CollPt.y) + lij[8]*(TB.z-m_CollPt.z);
        P4.x = lij[0]*(TA.x-m_CollPt.x) + lij[1]*(TA.y-m_CollPt.y) + lij[2]*(TA.z-m_CollPt.z);
        P4.y = lij[3]*(TA.x-m_CollPt.x) + lij[4]*(TA.y-m_CollPt.y) + lij[5]*(TA.z-m_CollPt.z);
        P4.z = lij[6]*(TA.x-m_CollPt.x) + lij[7]*(TA.y-m_CollPt.y) + lij[8]*(TA.z-m_CollPt.z);
    }
    else
    {
        P1.x = lij[0]*(LB.x-m_CollPt.x) + lij[1]*(LB.y-m_CollPt.y) + lij[2]*(LB.z-m_CollPt.z);
        P1.y = lij[3]*(LB.x-m_CollPt.x) + lij[4]*(LB.y-m_CollPt.y) + lij[5]*(LB.z-m_CollPt.z);
        P1.z = lij[6]*(LB.x-m_CollPt.x) + lij[7]*(LB.y-m_CollPt.y) + lij[8]*(LB.z-m_CollPt.z);
        P2.x = lij[0]*(LA.x-m_CollPt.x) + lij[1]*(LA.y-m_CollPt.y) + lij[2]*(LA.z-m_CollPt.z);
        P2.y = lij[3]*(LA.x-m_CollPt.x) + lij[4]*(LA.y-m_CollPt.y) + lij[5]*(LA.z-m_CollPt.z);
        P2.z = lij[6]*(LA.x-m_CollPt.x) + lij[7]*(LA.y-m_CollPt.y) + lij[8]*(LA.z-m_CollPt.z);
        P3.x = lij[0]*(TA.x-m_CollPt.x) + lij[1]*(TA.y-m_CollPt.y) + lij[2]*(TA.z-m_CollPt.z);
        P3.y = lij[3]*(TA.x-m_CollPt.x) + lij[4]*(TA.y-m_CollPt.y) + lij[5]*(TA.z-m_CollPt.z);
        P3.z = lij[6]*(TA.x-m_CollPt.x) + lij[7]*(TA.y-m_CollPt.y) + lij[8]*(TA.z-m_CollPt.z);
        P4.x = lij[0]*(TB.x-m_CollPt.x) + lij[1]*(TB.y-m_CollPt.y) + lij[2]*(TB.z-m_CollPt.z);
        P4.y = lij[3]*(TB.x-m_CollPt.x) + lij[4]*(TB.y-m_CollPt.y) + lij[5]*(TB.z-m_CollPt.z);
        P4.z = lij[6]*(TB.x-m_CollPt.x) + lij[7]*(TB.y-m_CollPt.y) + lij[8]*(TB.z-m_CollPt.z);
    }
}


void Panel4::translate(double tx, double ty, double tz)
{
    m_Node[0].translate(tx, ty, tz);
    m_Node[1].translate(tx, ty, tz);
    m_Node[2].translate(tx, ty, tz);
    m_Node[3].translate(tx, ty, tz);
    setPanelFrame();
}


/**
 * Finds the intersection point of a ray with the panel.
 * The ray is defined by a point and a direction vector.
 * @param A the ray's origin
 * @param U the ray's direction
 * @param I the intersection point
 * @param dist the distance of A to the panel in the direction of the panel's normal
 */
bool Panel4::intersect(Vector3d const &A, Vector3d const &U, Vector3d &I) const
{
    Triangle3d t30(m_Node[0], m_Node[1], m_Node[2]);
    if(t30.intersectRayInside(A, U, I)) return true;

    Triangle3d t31(m_Node[0], m_Node[2], m_Node[3]);
    if(t31.intersectRayInside(A, U, I)) return true;

    return false;
}


void Panel4::rotate(double alpha, double beta, double phi)
{
    Vector3d Origin;
    for(int i=0; i<4; i++)
    {
        Node &nd = m_Node[i];
        if(fabs(phi)>ANGLEPRECISION)            nd.rotateX(Origin, phi);
        if(fabs(beta)>ANGLEPRECISION)           nd.rotateZ(Origin, beta);
        if(fabs(alpha)>ANGLEPRECISION)          nd.rotateY(Origin, alpha);
    }
    setPanelFrame();
}


void Panel4::rotate(Vector3d const &HA, Vector3d const &Axis, double angle)
{
    for(int i=0; i<4; i++)
    {
        m_Node[i].rotate(HA, Axis, angle);
    }
    setPanelFrame();
}


void Panel4::sourceN4023Potential(Vector3d const &C, double &phi, double coreradius) const
{
    double RNUM(0), DNOM(0), pjk(0), CJKi(0);
    double PN(0), A, B(0), PA(0), PB(0), SM(0), SL(0), AM(0), AL(0), Al(0);
    double side(0), sign(0), S(0), GL(0);
    Vector3d PJK, a, b, s, h;

    phi = 0.0;

    PJK.x = C.x - m_CollPt.x;
    PJK.y = C.y - m_CollPt.y;
    PJK.z = C.z - m_CollPt.z;

    PN  = PJK.x*m_Normal.x + PJK.y*m_Normal.y + PJK.z*m_Normal.z;
    pjk = sqrt(PJK.x*PJK.x + PJK.y*PJK.y + PJK.z*PJK.z);

    if(pjk>s_RFF*m_MaxSize)
    {
        // use far-field formula
        phi = -m_Area /pjk;
        return;
    }

    for (int i=0; i<4; i++)
    {
        a.x  = C.x - m_Node[i].x;
        a.y  = C.y - m_Node[i].y;
        a.z  = C.z - m_Node[i].z;

        b.x  = C.x - m_Node[(i+1)%4].x;
        b.y  = C.y - m_Node[(i+1)%4].y;
        b.z  = C.z - m_Node[(i+1)%4].z;

        s.x  = m_Node[(i+1)%4].x - m_Node[i].x;
        s.y  = m_Node[(i+1)%4].y - m_Node[i].y;
        s.z  = m_Node[(i+1)%4].z - m_Node[i].z;

        A    = sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
        B    = sqrt(b.x*b.x + b.y*b.y + b.z*b.z);
        S    = sqrt(s.x*s.x + s.y*s.y + s.z*s.z);
        SM   = s.x*m_m.x + s.y*m_m.y + s.z*m_m.z;
        SL   = s.x*m_l.x + s.y*m_l.y + s.z*m_l.z;
        AM   = a.x*m_m.x + a.y*m_m.y + a.z*m_m.z;
        AL   = a.x*m_l.x + a.y*m_l.y + a.z*m_l.z;
        Al   = AM*SL - AL*SM;
        PA   = PN*PN*SL + Al*AM;
        PB   = PA - Al*SM;

        //get the distance of the TestPoint to the panel's side
        h.x =  a.y*s.z - a.z*s.y;
        h.y = -a.x*s.z + a.z*s.x;
        h.z =  a.x*s.y - a.y*s.x;

        if(m_Node[i].isSame(m_Node[(i+1)%4]))
        {
            //no contribution from this side
            CJKi = 0.0;
        }
        else if (((h.x*h.x+h.y*h.y+h.z*h.z)/(s.x*s.x+s.y*s.y+s.z*s.z) <= coreradius*coreradius)
                 && a.x*s.x+a.y*s.y+a.z*s.z>=0.0 && b.x*s.x+b.y*s.y+b.z*s.z<=0.0)
        {
            CJKi = 0.0; // unused, influence is never evaluated on the panel's side
        }
        else
        {
            //first the potential
            if(fabs(A+B-S)>0.0)    GL = 1.0/S * log(fabs((A+B+S)/(A+B-S)));
            else                GL = 0.0;

            RNUM = SM*PN * (B*PA-A*PB);
            DNOM = PA*PB + PN*PN*A*B*SM*SM;

            if(fabs(PN)<INPLANEPRECISION)
            {
                // side is >0 if the point is on the strip's right side
                side = m_Normal.x*h.x + m_Normal.y*h.y + m_Normal.z*h.z;
                if(side >=0.0)
                    sign = +1.0;
                else
                    sign = -1.0;

                if(DNOM<0.0)
                {
                    if(PN>0.0)     CJKi =  PI * sign;
                    else         CJKi = -PI * sign;
                }
                else if(DNOM == 0.0)
                {
                    if(PN>0.0)  CJKi =  PI/2.0 * sign;
                    else        CJKi = -PI/2.0 * sign;
                }
                else
                    CJKi = 0.0;
            }
            else
            {
                CJKi = atan2(RNUM, DNOM);
            }

            phi += Al*GL - PN*CJKi;
        }
    }
    phi = -phi; //opposite sign given by strict application of NASA4023 formulae
}


/** returns F/(rho.gamma) */
Vector3d Panel4::vortexForce(Vector3d const &wind) const
{
    Vector3d force;
    Vector3d s;

    for (int i=0; i<4; i++)
    {
        s.x  = m_Node[(i+1)%4].x - m_Node[i].x;
        s.y  = m_Node[(i+1)%4].y - m_Node[i].y;
        s.z  = m_Node[(i+1)%4].z - m_Node[i].z;

        force += wind*s; // vortex = rho.v*Gamma
    }

    return force;
}


void Panel4::sourceN4023Velocity(Vector3d const &C, bool bSelf, Vector3d &Vel, double coreradius) const
{
    double RNUM(0), DNOM(0), pjk(0), CJKi(0);
    double PN(0), A(0), B(0), PA(0), PB(0), SM(0), SL(0), AM(0), AL(0), Al(0);
    double side(0), sign(0), S(0), GL(0);
    Vector3d PJK, a, b, s, T1, T2, h;


    if(bSelf)
    {
        // With the normal pointing outwards,
        //   - exterior limit to Vn =  2.pi.n
        //   - interior limit is Vn = -2.pi.n
        // Exterior Neumann BC:
        Vel = m_Normal *(+2.0*PI);;
        return;
    }

    Vel.x=0.0; Vel.y=0.0; Vel.z=0.0;

    PJK.x = C.x - m_CollPt.x;
    PJK.y = C.y - m_CollPt.y;
    PJK.z = C.z - m_CollPt.z;

    PN  = PJK.x*m_Normal.x + PJK.y*m_Normal.y + PJK.z*m_Normal.z;
    pjk = sqrt(PJK.x*PJK.x + PJK.y*PJK.y + PJK.z*PJK.z);


    if(pjk>s_RFF*m_MaxSize)
    {
        // use far-field formula
        Vel.x = PJK.x * m_Area/pjk/pjk/pjk;
        Vel.y = PJK.y * m_Area/pjk/pjk/pjk;
        Vel.z = PJK.z * m_Area/pjk/pjk/pjk;
        return;
    }

    for (int i=0; i<4; i++)
    {
        a.x  = C.x - m_Node[i].x;
        a.y  = C.y - m_Node[i].y;
        a.z  = C.z - m_Node[i].z;

        b.x  = C.x - m_Node[(i+1)%4].x;
        b.y  = C.y - m_Node[(i+1)%4].y;
        b.z  = C.z - m_Node[(i+1)%4].z;

        s.x  = m_Node[(i+1)%4].x - m_Node[i].x;
        s.y  = m_Node[(i+1)%4].y - m_Node[i].y;
        s.z  = m_Node[(i+1)%4].z - m_Node[i].z;

        A    = sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
        B    = sqrt(b.x*b.x + b.y*b.y + b.z*b.z);
        S    = sqrt(s.x*s.x + s.y*s.y + s.z*s.z);
        SM   = s.x*m_m.x + s.y*m_m.y + s.z*m_m.z;
        SL   = s.x*m_l.x + s.y*m_l.y + s.z*m_l.z;
        AM   = a.x*m_m.x + a.y*m_m.y + a.z*m_m.z;
        AL   = a.x*m_l.x + a.y*m_l.y + a.z*m_l.z;
        Al   = AM*SL - AL*SM;
        PA   = PN*PN*SL + Al*AM;
        PB   = PA - Al*SM;

        //get the distance of the TestPoint to the panel's side
        h.x =  a.y*s.z - a.z*s.y;
        h.y = -a.x*s.z + a.z*s.x;
        h.z =  a.x*s.y - a.y*s.x;

        if(m_Node[i].isSame(m_Node[(i+1)%4]))
        {
            //no contribution from this side
            CJKi = 0.0;
        }
        else if ((((h.x*h.x+h.y*h.y+h.z*h.z)/(s.x*s.x+s.y*s.y+s.z*s.z) <= coreradius*coreradius)
                  && a.x*s.x+a.y*s.y+a.z*s.z>=0.0 && b.x*s.x+b.y*s.y+b.z*s.z<=0.0)
                 || A < coreradius || B < coreradius)
        {
            CJKi = 0.0; // unused, influence is never evaluated on the panel's side
        }
        else
        {
            //first the potential
            if(fabs(A+B-S)>0.0)    GL = 1.0/S * log(fabs((A+B+S)/(A+B-S)));
            else                GL = 0.0;

            RNUM = SM*PN * (B*PA-A*PB);
            DNOM = PA*PB + PN*PN*A*B*SM*SM;

            if(fabs(PN)<INPLANEPRECISION)
            {
                // side is >0 if the point is on the strip's right side
                side = m_Normal.x*h.x + m_Normal.y*h.y + m_Normal.z*h.z;
                if(side >=0.0)
                    sign = +1.0;
                else
                    sign = -1.0;

                if(DNOM<0.0)
                {
                    if(PN>0.0)     CJKi =  PI * sign;
                    else         CJKi = -PI * sign;
                }
                else if(DNOM == 0.0)
                {
                    if(PN>0.0)  CJKi =  PI/2.0 * sign;
                    else        CJKi = -PI/2.0 * sign;
                }
                else
                    CJKi = 0.0;
            }
            else
            {
                CJKi = atan2(RNUM, DNOM);
            }

            T1.x   = m_l.x      * SM*GL;
            T1.y   = m_l.y      * SM*GL;
            T1.z   = m_l.z      * SM*GL;
            T2.x   = m_m.x      * SL*GL;
            T2.y   = m_m.y      * SL*GL;
            T2.z   = m_m.z      * SL*GL;

            // assemble NASA 4023 eq.40
            Vel.x   += m_Normal.x * CJKi + T1.x - T2.x;
            Vel.y   += m_Normal.y * CJKi + T1.y - T2.y;
            Vel.z   += m_Normal.z * CJKi + T1.z - T2.z;
        }
    }

}


void Panel4::doubletN4023Velocity(Vector3d const &C, Vector3d &V, double coreradius, bool bUseRFF) const
{
    Vector3d PJK, a, b, s, T1, h;
    double pjk(0);
    double PN(0), A(0), B(0), GL(0);

    V.reset();

    PJK.x = C.x - m_CollPt.x;
    PJK.y = C.y - m_CollPt.y;
    PJK.z = C.z - m_CollPt.z;

    PN  = PJK.x*m_Normal.x + PJK.y*m_Normal.y + PJK.z*m_Normal.z;
    pjk = sqrt(PJK.x*PJK.x + PJK.y*PJK.y + PJK.z*PJK.z);

    if(bUseRFF && pjk> s_RFF*m_MaxSize)
    {
        // use far-field formula
        T1.x = PJK.x*3.0*PN - m_Normal.x*pjk*pjk;
        T1.y = PJK.y*3.0*PN - m_Normal.y*pjk*pjk;
        T1.z = PJK.z*3.0*PN - m_Normal.z*pjk*pjk;
        double pjk5 = pjk*pjk*pjk*pjk*pjk;
        V.x  = T1.x * m_Area /pjk5;
        V.y  = T1.y * m_Area /pjk5;
        V.z  = T1.z * m_Area /pjk5;
        return;
    }

/*
    Vector3d const *pR[5];
    pR[0] = m_Node + 0;
    pR[1] = m_Node + 1;
    pR[2] = m_Node + 2;
    pR[3] = m_Node + 3;
    pR[4] = m_Node + 0; */

    for (int i=0; i<4; i++)
    {
        a.x  = C.x - m_Node[i].x;
        a.y  = C.y - m_Node[i].y;
        a.z  = C.z - m_Node[i].z;
        b.x  = C.x - m_Node[(i+1)%4].x;
        b.y  = C.y - m_Node[(i+1)%4].y;
        b.z  = C.z - m_Node[(i+1)%4].z;
        s.x  = m_Node[(i+1)%4].x - m_Node[i].x;
        s.y  = m_Node[(i+1)%4].y - m_Node[i].y;
        s.z  = m_Node[(i+1)%4].z - m_Node[i].z;
        A    = sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
        B    = sqrt(b.x*b.x + b.y*b.y + b.z*b.z);

        //get the distance of the field point to the panel's side
        h.x =  a.y*s.z - a.z*s.y;
        h.y = -a.x*s.z + a.z*s.x;
        h.z =  a.x*s.y - a.y*s.x;

        if(m_Node[i].isSame(m_Node[(i+1)%4]))
        {
        }
        else if(A<coreradius || B<coreradius) // v7: added additional check that the point is not on a vertex
        {
            // the point is on the vertex
        }
        else if ((sqrt(h.x*h.x+h.y*h.y+h.z*h.z)<coreradius) // v7: no normalization by the segment's length
                 //        else if (((h.x*h.x+h.y*h.y+h.z*h.z)/(s.x*s.x+s.y*s.y+s.z*s.z) <= coreradius*coreradius)
                 && a.x*s.x+a.y*s.y+a.z*s.z>=0.0  && b.x*s.x+b.y*s.y+b.z*s.z<=0.0)
        {
            // the point is on the segment, within the core radius
        }
        else
        {
            // next the induced velocity
            h.x =  a.y*b.z - a.z*b.y;
            h.y = -a.x*b.z + a.z*b.x;
            h.z =  a.x*b.y - a.y*b.x;
            GL = ((A+B) /A/B/ (A*B + a.x*b.x+a.y*b.y+a.z*b.z));
            V.x += h.x * GL;
            V.y += h.y * GL;
            V.z += h.z * GL;
        }
    }
}


void Panel4::doubletN4023Potential(Vector3d const &C, bool bSelf, double &phi, double coreradius, bool bUseRFF) const
{
    Vector3d PJK, a, b, s, h;
    double RNUM(0), DNOM(0), pjk(0), CJKi(0);
    double PN(0), A(0), B(0), PA(0), PB(0), SM(0), SL(0), AM(0), AL(0), Al(0);
    double side(0), sign(0);

    if(bSelf)
    {
        phi  = 2.0*PI;
        return;
    }

    phi = 0.0;

    PJK.x = C.x - m_CollPt.x;
    PJK.y = C.y - m_CollPt.y;
    PJK.z = C.z - m_CollPt.z;

    PN  = PJK.x*m_Normal.x + PJK.y*m_Normal.y + PJK.z*m_Normal.z;
    pjk = sqrt(PJK.x*PJK.x + PJK.y*PJK.y + PJK.z*PJK.z);

    if(bUseRFF && pjk> s_RFF*m_MaxSize)
    {
        // use far-field formula
        phi = -PN * m_Area /pjk/pjk/pjk;
        return;
    }

    for (int i=0; i<4; i++)
    {
        a.x  = C.x - m_Node[i].x;
        a.y  = C.y - m_Node[i].y;
        a.z  = C.z - m_Node[i].z;
        b.x  = C.x - m_Node[(i+1)%4].x;
        b.y  = C.y - m_Node[(i+1)%4].y;
        b.z  = C.z - m_Node[(i+1)%4].z;
        s.x  = m_Node[(i+1)%4].x - m_Node[i].x;
        s.y  = m_Node[(i+1)%4].y - m_Node[i].y;
        s.z  = m_Node[(i+1)%4].z - m_Node[i].z;
        A    = sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
        B    = sqrt(b.x*b.x + b.y*b.y + b.z*b.z);
        SM   = s.x*m_m.x + s.y*m_m.y + s.z*m_m.z;
        SL   = s.x*m_l.x + s.y*m_l.y + s.z*m_l.z;
        AM   = a.x*m_m.x + a.y*m_m.y + a.z*m_m.z;
        AL   = a.x*m_l.x + a.y*m_l.y + a.z*m_l.z;
        Al   = AM*SL - AL*SM;
        PA   = PN*PN*SL + Al*AM;
        PB   = PA - Al*SM;

        //get the distance of the TestPoint to the panel's side
        h.x =  a.y*s.z - a.z*s.y;
        h.y = -a.x*s.z + a.z*s.x;
        h.z =  a.x*s.y - a.y*s.x;

        if(m_Node[i].isSame(m_Node[(i+1)%4]))
        {
            CJKi = 0.0;
        }
        else if(A<coreradius || B<coreradius) // v7: added additional check that the point is not on a vertex
        {
            // the point is on the vertex
            CJKi = 0.0;
        }
        else if ((sqrt(h.x*h.x+h.y*h.y+h.z*h.z)<coreradius) // v7: no normalization by the segment's length
                 //        else if (((h.x*h.x+h.y*h.y+h.z*h.z)/(s.x*s.x+s.y*s.y+s.z*s.z) <= coreradius*coreradius)
                 && a.x*s.x+a.y*s.y+a.z*s.z>=0.0  && b.x*s.x+b.y*s.y+b.z*s.z<=0.0)
        {
            // the point is on the segment, within the core radius
        }
        else
        {
            RNUM = SM*PN * (B*PA-A*PB);
            DNOM = PA*PB + PN*PN*A*B*SM*SM;
            if(fabs(PN)<INPLANEPRECISION)
            {
                // side is >0 if on the panel's right side
                side = m_Normal.x*h.x +m_Normal.y*h.y +m_Normal.z*h.z;

                if(side >=0.0) sign = 1.0; else sign = -1.0;

                if(DNOM<0.0)
                {
                    // Ctrl point lies within the strip
                    if(PN>0.0)  CJKi =  PI * sign;
                    else        CJKi = -PI * sign;
                }
                else if(DNOM == 0.0)
                {
                    // Ctrl point lies on the edge of the strip
                    if(PN>0.0)  CJKi =  PI/2.0 * sign;
                    else        CJKi = -PI/2.0 * sign;
                }
                else //DNOM >0
                {
                    // Ctrl point lies outside the strip
                    CJKi = 0.0;
                }
            }
            else
            {
                CJKi = atan2(RNUM,DNOM);
            }
        }
        phi += CJKi;

    }
    phi = -phi;
}



/**
 * Computes the influence of a uniform doublet density using the vortex ring equivalence.
 */
void Panel4::doubletVortexVelocity(Vector3d const &C, Vector3d &V, double coreradius, bool bUseRFF) const
{
    Vector3d PJK, T1, velseg;
    V.set(0.0,0.0,0.0);

    PJK.x = C.x - m_CollPt.x;
    PJK.y = C.y - m_CollPt.y;
    PJK.z = C.z - m_CollPt.z;

    double PN  = PJK.x*m_Normal.x + PJK.y*m_Normal.y + PJK.z*m_Normal.z;
    double pjk = sqrt(PJK.x*PJK.x + PJK.y*PJK.y + PJK.z*PJK.z);

    if(bUseRFF && pjk>s_RFF*m_MaxSize)
    {
        // use far-field formula
        T1.x = PJK.x*3.0*PN - m_Normal.x*pjk*pjk;
        T1.y = PJK.y*3.0*PN - m_Normal.y*pjk*pjk;
        T1.z = PJK.z*3.0*PN - m_Normal.z*pjk*pjk;
        double pjk5 = pjk *pjk *pjk *pjk *pjk;
        V.x  = T1.x * m_Area /pjk5;
        V.y  = T1.y * m_Area /pjk5;
        V.z  = T1.z * m_Area /pjk5;
        return;
    }

    velseg = vortexInducedVelocity(m_Node[0], m_Node[1], C, coreradius);
    V += velseg;
    velseg = vortexInducedVelocity(m_Node[1], m_Node[2], C, coreradius);
    V += velseg;
    velseg = vortexInducedVelocity(m_Node[2], m_Node[3], C, coreradius);
    V += velseg;
    velseg = vortexInducedVelocity(m_Node[3], m_Node[0], C, coreradius);
    V += velseg;

    V.x *= 4.0*PI;
    V.y *= 4.0*PI;
    V.z *= 4.0*PI;
}


double Panel4::width() const
{
    double l0 =  sqrt( (LB().y - LA().y)*(LB().y - LA().y) +(LB().z - LA().z)*(LB().z - LA().z));
    double l1 =  sqrt( (TB().y - TA().y)*(TB().y - TA().y) +(TB().z - TA().z)*(TB().z - TA().z));
    return (l0+l1)/2.0;
}


double Panel4::length() const
{
    double l0 =  sqrt( (LA().x-TA().x) * (LA().x-TA().x) + (LA().z-TA().z) * (LA().z-TA().z));
    double l1 =  sqrt( (LB().x-TB().x) * (LB().x-TB().x) + (LB().z-TB().z) * (LB().z-TB().z));
    return (l0+l1)/2.0;
}


std::string Panel4::properties(bool bLong) const
{
    QString props, strong;
    QString strange;

    props = QString::asprintf("Quad Panel %d\n", m_index);

    strong = QString::asprintf("  CoG  = (%7.3f, %7.3f, %7.3f)", m_CollPt.x*Units::mtoUnit(), m_CollPt.y*Units::mtoUnit(), m_CollPt.z*Units::mtoUnit());
    strong += Units::lengthUnitQLabel() + "\n";
    props += strong;

    strong = QString::asprintf("  Area = %.3g ", m_Area*Units::m2toUnit());
    props += strong + Units::areaUnitQLabel() + "\n";

    strong = QString::asprintf("  Warp angle = %2f ", warpAngle());
    props += strong + DEGch + "\n";

    strong = QString::asprintf("  Min. internal angle = %2f ", minAngle());
    props += strong + DEGch + "\n";

    props += QString::asprintf("  Width  = %5g ", width() *Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n";
    props += QString::asprintf("  Length = %5g ", length()*Units::mtoUnit()) + Units::lengthUnitQLabel() + "\n";

    if(!bLong) return props.toStdString();

    if(m_bPositiveOrientation) props += "  Positive orientation\n";
    else                       props += "  Negative orientation\n";

    switch(m_Pos)
    {
        case xfl::BOTSURFACE:  strange = "  BOTTOM SURFACE\n";   break;
        case xfl::MIDSURFACE:  strange = "  MIDDLE SURFACE\n";   break;
        case xfl::TOPSURFACE:  strange = "  TOP SURFACE\n";      break;
        case xfl::SIDESURFACE: strange = "  SIDE SURFACE\n";     break;
        case xfl::FUSESURFACE: strange = "  FUSE SURFACE\n";     break;
        case xfl::WAKESURFACE: strange = "  WAKE PANEL\n";       break;
        case xfl::NOSURFACE:   strange = "  NO SURFACE\n";       break;
    }
    props += strange;

    strange = QString::asprintf("  Surface index= %d\n", m_SurfaceIndex);
    props += strange;

    strange = QString::asprintf("  Neighbours: PU%4d  PD%4d  PL%4d  PR%4d\n", m_iPU, m_iPD, m_iPL, m_iPR);
    props += strange;

    strange = QString::asprintf("  Vertices:   LA%4d  LB%4d  TA%4d  TB%4d\n", m_iLA, m_iLB, m_iTA, m_iTB);
    props += strange;

/*    for(int i=0; i<4; i++)
    {
        strange = QString::asprintf("  Node%d:  %13.5g  %13.5g  %13.5g\n", i, m_Node[i].x, m_Node[i].y, m_Node[i].z);
        props += strange;
    }*/
    strange = QString::asprintf("  Nd_LA:  %13.5g  %13.5g  %13.5g\n", LA().x, LA().y, LA().z);
    props += strange;
    strange = QString::asprintf("  Nd_LB:  %13.5g  %13.5g  %13.5g\n", LB().x, LB().y, LB().z);
    props += strange;
    strange = QString::asprintf("  Nd_TA:  %13.5g  %13.5g  %13.5g\n", TA().x, TA().y, TA().z);
    props += strange;
    strange = QString::asprintf("  Nd_TB:  %13.5g  %13.5g  %13.5g\n", TB().x, TB().y, TB().z);
    props += strange;

    strange = QString::asprintf("  Normal: %13.7f  %13.7f  %13.7f\n", m_Normal.x, m_Normal.y, m_Normal.z);
    props += strange;
    strange = QString::asprintf("  CollPt: %13.7f  %13.7f  %13.7f\n", m_CollPt.x, m_CollPt.y, m_CollPt.z);
    props += strange;
    strange = QString::asprintf("  CtrlPt: %13.7f  %13.7f  %13.7f\n", m_CtrlPt.x, m_CtrlPt.y, m_CtrlPt.z);
    props += strange;
    strange = QString::asprintf("  Vortex: %13.7f  %13.7f  %13.7f\n", trailingVortex().x, trailingVortex().y, trailingVortex().z);
    props += strange;

    props += "  Local frame:\n";
    strange = QString::asprintf("    l      = (%7.2f, %7.2f, %7.2f)\n", m_l.x, m_l.y, m_l.z);
    props += strange;
    strange = QString::asprintf("    m      = (%7.2f, %7.2f, %7.2f)\n", m_m.x, m_m.y, m_m.z);
    props += strange;
    strange = QString::asprintf("    Normal = (%7.2f, %7.2f, %7.2f)\n", m_Normal.x, m_Normal.y, m_Normal.z);
    props += strange;

    if(isTrailing())
    {
        props += "  Panel is trailing:\n";
        strange = QString::asprintf("    Downstream wake panel  index = %d\n", m_iWake);
        props += strange;
        strange = QString::asprintf("    Downstream wake column index = %d\n", m_iWakeColumn);
        props += strange;
    }
    else if (isLeading())
    {
        props += "  Panel is leading\n";
    }

    return props.toStdString();
}


/**
 * returns the minimum dihedral angle formed by the planes intersecting in the diagonals
 */
double Panel4::warpAngle() const
{
    // for each diagonal,  make the two half triangles, and take the inverse cos of their normal's dot product
    Triangle3d t0(m_Node[0], m_Node[1], m_Node[2]);
    Triangle3d t1(m_Node[0], m_Node[2], m_Node[3]);
    double cosa0 = t0.normal().dot(t1.normal());
    double a0 = fabs(acos(cosa0)*180.0/PI);

    Triangle3d t2(m_Node[0], m_Node[1], m_Node[3]);
    Triangle3d t3(m_Node[1], m_Node[2], m_Node[3]);
    double cosa1 = t2.normal().dot(t3.normal());
    double a1 = fabs(acos(cosa1)*180.0/PI);

    if(t0.isNull() || t1.isNull() || t2.isNull() || t3.isNull()) return 0.0;

    return std::min(a0,a1);
}


double Panel4::minAngle() const
{
    Vector3d S[6];
    S[0] = m_Node[3];
    S[1] = m_Node[0];
    S[2] = m_Node[1];
    S[3] = m_Node[2];
    S[4] = m_Node[3];
    S[5] = m_Node[0];
    double minangle = 360.0;
    double sum=0;
    for(int in=1; in<5; in++)
    {
        Vector3d edge0 = (S[in-1]-S[in]).normalized();
        Vector3d edge1 = (S[in+1]-S[in]).normalized();
        double cosa = edge0.dot(edge1);
        double acosa = fabs(acos(cosa)*180.0/PI);
        if(acosa<minangle) minangle = acosa;
        sum += acosa;
    }
    (void)sum;
    return minangle;
}


/**
 * Checks if a point lies on a panel's edge, within the CoreRadius length
 */
bool Panel4::isEdgePoint(Vector3d const &PtGlobal) const
{
    for(int i=0; i<4; i++)
    {
        Vector3d E = m_Node[(i+1)%4] - m_Node[i];
        double e = E.norm();
        Vector3d R0 = PtGlobal - m_Node[i];
        Vector3d R1 = PtGlobal - m_Node[i+1];

        //check that the dot product is negative
        if(R0.dot(R1)<=0.0)
        {
            Vector3d u = E/e;
            Vector3d H;
            double projection = R0.dot(u);

            H.x = R0.x - u.x * projection;
            H.y = R0.y - u.y * projection;
            H.z = R0.z - u.z * projection;
            double r = sqrt(H.x*H.x + H.y*H.y + H.z*H.z);
            if(r<Vortex::coreRadius()) return true;
        }
    }
    return false;
}


