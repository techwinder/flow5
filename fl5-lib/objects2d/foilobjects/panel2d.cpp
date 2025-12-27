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

#include <cassert>

#include <panel2d.h>

#include <constants.h>
#include <geom_params.h>

Panel2d::Panel2d()
{
    A.set(0.0,0.0);
    B.set(1.0,0.0);
    m_index = -1;
    m_BLNodeIndex[0] = m_BLNodeIndex[1] = -1;
    m_bTEPanel = false;
    m_bWakePanel = false;
    setFrame(A, B);
}


Panel2d::Panel2d(Vector2d const &P0, Vector2d const &P1)
{
    m_index = -1;
    m_BLNodeIndex[0] = m_BLNodeIndex[1] = -1;
    m_bTEPanel = false;
    m_bWakePanel = false;
    setFrame({P0.x, P0.y}, {P1.x, P1.y});
}


Panel2d::Panel2d(Node2d const &N0, Node2d const &N1)
{
    A = N0;
    B = N1;
    m_index = -1;
    m_BLNodeIndex[0] = m_BLNodeIndex[1] = -1;
    m_bTEPanel = false;
    m_bWakePanel = false;
    setFrame();
}


Panel2d::Panel2d(Vector2d const &A, Vector2d const &B, int index, bool bTEPanel)
{
    m_index = index;
    m_bTEPanel = bTEPanel;
    m_bWakePanel = false;
    setFrame({A.x, A.y}, {B.x, B.y});
}


void Panel2d::setFrame(Node2d const &A, Node2d const &B, int index, bool bTEPanel)
{
    m_index = index;
    m_BLNodeIndex[0] = m_BLNodeIndex[1] = -1;
    m_bTEPanel = bTEPanel;
    m_bWakePanel = false;
    setFrame(A, B);
}


/**
* Constructs the geometrical panel properties based on the panel's two endpoints.
* @param TA the position of the trailing edge left node.
* @param TB the position of the trailing edge rightt node.
*/
void Panel2d::setFrame(Node2d const &nd0, Node2d const &nd1)
{
    A = nd0;
    B = nd1;
    setFrame();
}


void Panel2d::setFrame()
{
    I.x = (A.x+B.x)/2.0;
    I.y = (A.y+B.y)/2.0;
    I.setNormal(Normal);
    I.setIndex(-1);
    I.setWakeNode(isWakePanel());

    T = A-B;

    m_Length = T.norm();
    T.normalize();
    Normal.set(-T.y, +T.x);

    m_Frame.setOrigin(I);
    m_Frame.setI({-T.x, -T.y});

    if(m_bTEPanel)
    {
        aPanel = atan2( -T.x , T.y ) + PI;
    }
    else
    {
        aPanel = atan2(-Normal.y, -Normal.x);
    }

    m_bIsNull = (m_Length<LENGTHPRECISION);

    if(!m_bIsNull)
    {
        Al = globalToLocalPosition(A); // should be (-length/2, 0.0)
        Bl = globalToLocalPosition(B); // should be (+length/2, 0.0)
    }
    else
    {
        Al.set(0.,0.);
        Bl.set(0.,0.);
    }
}


/**
* Converts the global coordinates of the input position in local panel coordinates.
* @param  V the position defined in global coordinates
* @return The Vector2d holding the local coordinates
*/
Vector2d Panel2d::globalToLocalPosition(Vector2d const &Pos) const
{
    return m_Frame.globalToLocalPosition(Pos);
}


/**
* Converts the global coordinates of the input vector in local panel coordinates.
* @param  V the vector defined in global coordinates
* @return The Vector2d defined in local coordinates
*/
Vector2d Panel2d::globalToLocalVector(Vector2d const &V) const
{
    return m_Frame.globalToLocal(V);
}


/**
* Converts the local coordinates of the input vector in global panel coordinates.
*@param  V the Vector2d defined in local coordinates
*@return The Vector2d defined in global coordinates
*/
Vector2d Panel2d::localToGlobalVector(Vector2d const &Vl) const
{
    return m_Frame.localToGlobal(Vl);
}


/** K&P type formulation*/
void Panel2d::linearVortexKP(Vector2d const &pt, double &psiC, double &psiL) const
{
    if(m_bIsNull)
    {
        psiC= 0.0;
        return;
    }

    Vector2d Pl = globalToLocalPosition(pt);

    psiC=psiL=0.0;

    double beta1=0.0, beta2=0.0;
    double lnr1=0.0, lnr2=0.0;

    double xa = Al.x;
    double xb = Bl.x;

    double x = Pl.x;
    double y = Pl.y;

    trig(Pl.x, Pl.y, beta1, beta2, lnr1, lnr2);

    double theta1 = PI/2.0-beta1;
    double theta2 = PI/2.0-beta2;

    psiC = (Pl.x-xa)*lnr1 - (Pl.x-xb)*lnr2 + Pl.y *(theta2-theta1) -xb+xa;
    psiC *= 1.0/2.0/PI;

    psiL  = -(x*x -xb*xb - y*y)*lnr2 -xb*xb/2.0 -x*xb + 2.0*x*y*theta2;
    psiL -= -(x*x -xa*xa - y*y)*lnr1 -xa*xa/2.0 -x*xa + 2.0*x*y*theta1;
    psiL *= 1.0/4.0/PI;
}


void Panel2d::uniformVortex(const Vector2d &pt, Vector2d *vel) const
{
    if(!vel) return;
    if(m_bIsNull)
    {
        vel->set(0.0,0.0);
        return;
    }

    Vector2d Pl = globalToLocalPosition(pt);
    Vector2d vell;

    double theta1=0, theta2=0;
    double lnr1=0, lnr2=0;

    trig(Pl.x, Pl.y, theta1, theta2, lnr1, lnr2);

    double beta1 = PI/2.0-theta1;
    double beta2 = PI/2.0-theta2;
    double d = Bl.x-Al.x;

    double uC = +1.0/2.0/PI * (beta2-beta1);
    vell.x = +(Bl.x*uC)/d - (Al.x*uC)/d;

    double vC =  1.0/2.0/PI * (lnr2-lnr1);
    vell.y = +(Bl.x*vC)/d - (Al.x*vC)/d;

    *vel = localToGlobalVector(vell);
}


/**
 * @brief Returns the velocity at a point, induced by the linear vorticity of the panel.
 * Does not include the freestream component of the velocity.
 * @param P the point where the velocity should be evaluated
 * @param dvdg1 dV/dGamma1, where Gamma1 is the vorticity at node 1
 * @param dvdg2 dV/dGamma2, where Gamma2 is the vorticity at node 2
 */
void Panel2d::linearVortex(Vector2d const &pt,
                           Vector2d *dVdg1, Vector2d *dVdg2,
                           Vector2d *d2Vdydg1_l, Vector2d *d2Vdydg2_l) const
{
    if(m_bIsNull)
    {
        if(dVdg1) dVdg1->set(0.0,0.0);
        if(dVdg2) dVdg2->set(0.0,0.0);
        if(d2Vdydg1_l) d2Vdydg1_l->set(0.0,0.0);
        if(d2Vdydg2_l) d2Vdydg2_l->set(0.0,0.0);
        return;
    }

    Vector2d Pl = globalToLocalPosition(pt);
    Vector2d dVdg1l, dVdg2l;

    double theta1=0, theta2=0;
    double lnr1=0, lnr2=0;

    trig(Pl.x, Pl.y, theta1, theta2, lnr1, lnr2);

    double beta1 = PI/2.0-theta1;
    double beta2 = PI/2.0-theta2;
    double d = Bl.x-Al.x;

    if(dVdg1 && dVdg2)
    {
        double uC = +1.0/2.0/PI * (beta2-beta1);
        double uL = +1.0/2.0/PI * ( Pl.y*(lnr2-lnr1) + Pl.x*(beta2-beta1) );
        dVdg1l.x = +(Bl.x*uC - uL)/d;
        dVdg2l.x = -(Al.x*uC - uL)/d;

        double vC =  1.0/2.0/PI * (lnr2-lnr1);
        double vL =  1.0/2.0/PI * (Pl.x *(lnr2-lnr1) + (Bl.x-Al.x) - Pl.y*(beta2-beta1)) ;
        dVdg1l.y = +(Bl.x*vC - vL)/d;
        dVdg2l.y = -(Al.x*vC - vL)/d;

        *dVdg1 = localToGlobalVector(dVdg1l);
        *dVdg2 = localToGlobalVector(dVdg2l);
    }

    if(d2Vdydg1_l && d2Vdydg2_l)
    {
        double x = Pl.x;
        double y = Pl.y;
        double bx = Bl.x;
        double ax = Al.x;
        //    d2Vdydg1.x =  (bx /PI * (-(x-ax)/y/y / (1.0+ (x-ax)*(x-ax)/y/y) + (x - bx)/y/y / (1.0+ (x-bx)*(x-bx)/y/y)) /2.0 - 1.0/PI * (log((x-bx)*(x-bx) +y*y) /2.0 - log((x-ax)*(x-ax) +y*y) /2.0 + y * (y / ((x-bx)*(x-bx) +y*y) - y / ((x-ax)*(x-ax) +y*y)) + x * (-(x-ax)/y/y / (1.0+ (x-ax)*(x-ax)/y/y) + (x-bx)/y/y / (1.0+ (x-bx)*(x-bx)/y/y))) /2.0) / (bx-ax);
        //    d2Vdydg2.x = -(ax /PI * (-(x-ax)/y/y / (1.0+ (x-ax)*(x-ax)/y/y) + (x - bx)/y/y / (1.0+ (x-bx)*(x-bx)/y/y)) /2.0 - 1.0/PI * (log((x-bx)*(x-bx) +y*y) /2.0 - log((x-ax)*(x-ax) +y*y) /2.0 + y * (y / ((x-bx)*(x-bx) +y*y) - y / ((x-ax)*(x-ax) +y*y)) + x * (-(x-ax)/y/y / (1.0+ (x-ax)*(x-ax)/y/y) + (x-bx)/y/y / (1.0+ (x-bx)*(x-bx)/y/y))) /2.0) / (bx-ax);
        d2Vdydg1_l->x = ((-1.0/4.0/PI*ax*ax + 1.0/2.0/PI*ax*x - 1.0/4.0/PI*x*x - 1.0/4.0/PI*y*y) * log(ax*ax - 2.0*ax*x + x*x + y*y) + ( 1.0/4.0/PI*ax*ax - 1.0/2.0/PI*ax*x + 1.0/4.0/PI*x*x + 1.0/4.0/PI*y*y) * log(bx*bx - 2.0*bx*x + x*x + y*y) + 1.0/2.0/PI*ax*ax + (-1.0/2.0/PI*bx - 1.0/2.0/PI*x)*ax + 1.0/2.0/PI*bx*x)
                        /(-1.0*bx + ax) / (ax*ax - 2.0*ax*x + x*x + y*y);
        d2Vdydg2_l->x = (( 1.0/4.0/PI*bx*bx - 1.0/2.0/PI*bx*x + 1.0/4.0/PI*x*x + 1.0/4.0/PI*y*y) * log(ax*ax - 2.0*ax*x + x*x + y*y) + (-1.0/4.0/PI*bx*bx + 1.0/2.0/PI*bx*x - 1.0/4.0/PI*x*x - 1.0/4.0/PI*y*y) * log(bx*bx - 2.0*bx*x + x*x + y*y) + 1.0/2.0/PI*bx*bx + (-1.0/2.0/PI*ax - 1.0/2.0/PI*x)*bx + 1.0/2.0/PI*ax*x)
                        /(-1.0*bx + ax) / (bx*bx - 2.0*bx*x + x*x + y*y);
    }
}


/** XFoil type formulation, with factor 1/4.pi included*/
void Panel2d::linearVortex(Vector2d const &pt, double &psi_p, double &psi_m) const
{
    if(m_bIsNull)
    {
        psi_p = psi_m = 0.0;
        return;
    }

    Vector2d Pl = globalToLocalPosition(pt);

    double theta1=0, theta2=0;
    double lnr1=0, lnr2=0;

    //Drela fig.2
    double r1 = (pt-A).norm();
    double r2 = (pt-B).norm();

    double x1 = Pl.x-Al.x;
    double x2 = Pl.x-Bl.x;

    trig(Pl.x, Pl.y, theta1, theta2, lnr1, lnr2);

    psi_p = x1*lnr1-x2*lnr2 + x2-x1 - Pl.y*(theta2-theta1);                                //eq. 4
    psi_m = ((x1+x2)*psi_p + r2*r2*lnr2 - r1*r1*lnr1 + 1.0/2.0*(x1*x1-x2*x2)) / (x1-x2);   //eq. 5

    psi_p *= 1.0/4.0/PI;
    psi_m *= 1.0/4.0/PI;
}


/**
 * Returns the stream function, the potential function and the velocity
 * induced by a panel with uniform source strength.
 * The stream function is discontinuous across the panel' line
 * and singular at its endpoints, so special processing is required */
void Panel2d::uniformSource(Vector2d const &pt, double *phi, double *psi, Vector2d *vel) const
{
    if(m_bIsNull)
    {
        if(phi) *phi = 0.0;
        if(psi) *psi = 0.0;
        if(vel) vel->set(0.0,0.0);
        return;
    }

    Vector2d vLocal;
    Vector2d Pl = globalToLocalPosition(pt);

    //Drela fig.2
    double x1  = Pl.x-Al.x;
    double x2  = Pl.x-Bl.x;
//    double Ply = Pl.y-Al.y;

    assert(fabs(Al.y)<LENGTHPRECISION);
    assert(fabs(Bl.y)<LENGTHPRECISION);

    // special case where the point is one of the two extremities
    // velocity is singular
    // stream function is singular but has a limit

    double lnr1(0), lnr2(0), theta1(0), theta2(0), beta1(0), beta2(0);

    if(A.isSame(pt, LENGTHPRECISION))
    {
        lnr1   = 0.0;
        theta1 = 0.0;
        theta2 = PI; // undefined

        theta1 += PI/2.0; // add PI/2 to stream function to be homogeneous with XFoil
        theta2 += PI/2.0;

        if(psi)
        {
            // psi is discontinuous in the direction normal to the panel
            // and psi is undefined at the end points
            // using XFoil formulation at endpoints
            *psi = x1*theta1 - x2*theta2;
            *psi *= 1.0/2.0/PI;
        }

        if(phi)
        {
            *phi = -2.0*(x2*lnr1)/4.0/PI;
        }
        if(vel) vel->set(0.0,0.0);
        return;
    }
    else if(B.isSame(pt, LENGTHPRECISION))
    {
        lnr2   = 0.0;

        theta1 = 0.0; // undefined
        theta2 = 0.0;
        theta1 += PI/2.0; // add PI/2 to stream function to be homogeneous with XFoil
        theta2 += PI/2.0;

        if(psi)
        {
            // psi is discontinuous in the direction normal to the panel
            // hence psi is undefined at the end points
            // using XFoil formulation at endpoints to get correct influence of BL source
            *psi = x1*theta1 - x2*theta2;
            *psi *= 1.0/2.0/PI;
        }

        if(phi)
        {
            *phi = 2.0*(x1*lnr2)/4.0/PI;
        }
        if(vel) vel->set(0.0,0.0);
        return;
    }

    // to reduce the number occurrences of stream function discontinuities
    // if the point lies on the panels line, move it slightly outwards
    // for instance Clark Y bottom points jump either side of the foils bottom panels

    if(fabs(Pl.y)<0.001)
    {
        Pl.y =  LENGTHPRECISION;
    }

    // get the angles
    trig(Pl.x, Pl.y, beta1, beta2, lnr1, lnr2);

    theta1 = PI/2.0-beta1;
    theta2 = PI/2.0-beta2;

    theta1 += PI/2.0; // add PI/2 to stream function to be homogeneous with XFoil
    theta2 += PI/2.0;

    if(psi)
    {
        *psi = x1*theta1 - x2*theta2;
        *psi += Pl.y*(lnr1-lnr2);
        *psi *= 1.0/2.0/PI;
    }

    if(phi)
    {
        *phi = 2.0*(x1*lnr1-x2*lnr2) + 2.0*Pl.y*(theta2-theta1);
        *phi *= 1.0/4.0/PI;
    }

    if(vel)
    {
        vLocal.x = +1.0/2.0/PI * (-lnr2+lnr1);
        if(fabs(Pl.y)<LENGTHPRECISION)
            vLocal.y = 0.0;
        else
            vLocal.y = +1.0/2.0/PI * (theta2-theta1);
        *vel = localToGlobalVector(vLocal);
    }
}

/*
void Panel2d::uniformSource(Vector2d const &pt, double *phi, double *psi, Vector2d *vel) const
{
    if(m_bIsNull)
    {
        if(phi) *phi = 0.0;
        if(psi) *psi = 0.0;
        if(vel) vel->set(0.0,0.0);
        return;
    }

    Vector2d vLocal;
    Vector2d Pl = globalToLocalPosition(pt);

    //Drela fig.2
    double x1  = Pl.x-Al.x;
    double x2  = Pl.x-Bl.x;
    double Ply = Pl.y-Al.y;

    // special case where the point is one of the two extremities
    // velocity is singular
    // stream function is singular but has a limit

    double lnr1=0, lnr2=0, theta1=0, theta2=0, beta1=0, beta2=0;

    if(A.isSame(pt, LENGTHPRECISION))
    {
        lnr1   = 0.0;
        theta1 = 0.0;
        theta2 = PI; // undefined

        theta1 += PI/2.0; // add PI/2 to stream function to be homogeneous with XFoil
        theta2 += PI/2.0;

        if(psi)
        {
            // psi is discontinuous in the direction normal to the panel
            // and psi is undefined at the end points
            // using XFoil formulation at endpoints
            *psi = x1*theta1 - x2*theta2;
            *psi *= 1.0/2.0/PI;
        }

        if(phi)
        {
            *phi = -2.0*(x2*lnr2)/4.0/PI;
        }
        if(vel) vel->set(0.0,0.0);
        return;
    }
    else if(B.isSame(pt, LENGTHPRECISION))
    {
        lnr2   = 0.0;

        theta1 = 0.0; // undefined
        theta2 = 0.0;
        theta1 += PI/2.0; // add PI/2 to stream function to be homogeneous with XFoil
        theta2 += PI/2.0;

        if(psi)
        {
            // psi is discontinuous in the direction normal to the panel
            // hence psi is undefined at the end points
            // using XFoil formulation at endpoints to get correct influence of BL source
            *psi = x1*theta1 - x2*theta2;
            *psi *= 1.0/2.0/PI;
        }

        if(phi)
        {
            *phi = 2.0*(x1*lnr1)/4.0/PI;
        }
        if(vel) vel->set(0.0,0.0);
        return;
    }

    // to reduce the number occurrences of stream function discontinuities
    // if the point lies on the panels line, move it slightly outwards
    // for instance Clark Y bottom points jump either side of the foils bottom panels

    if(fabs(Ply)<0.000001)
    {
        if(fabs(Ply<PRECISION)) Ply = 0.0;
        else
        {
            if(Pl.y>PRECISION) Ply =  0.001;
            else               Ply = -0.001;
        }
    }

    // get the angles
    trig(Pl.x, Ply, beta1, beta2, lnr1, lnr2);

    theta1 = PI/2.0-beta1;
    theta2 = PI/2.0-beta2;

    theta1 += PI/2.0; // add PI/2 to stream function to be homogeneous with XFoil
    theta2 += PI/2.0;

    if(psi)
    {
        *psi = x1*theta1 - x2*theta2;
        *psi += Ply*(lnr1-lnr2);
        *psi *= 1.0/2.0/PI;
    }

    if(phi)
    {
        *phi = 2.0*(x1*lnr1-x2*lnr2) + 2.0*Ply*(theta2-theta1);
        *phi *= 1.0/4.0/PI;
    }

    if(vel)
    {
        vLocal.x = +1.0/2.0/PI * (-lnr2+lnr1);
        if(fabs(Pl.y)<LENGTHPRECISION)
            vLocal.y = 0.0;
        else
            vLocal.y = +1.0/2.0/PI * (theta2-theta1);
        *vel = localToGlobalVector(vLocal);
    }
}*/


void Panel2d::linearSource(Vector2d const &pt, double *psi1, double *psi2, Vector2d *vel1, Vector2d *vel2) const
{
    if(m_bIsNull)
    {
        if(psi1) *psi1=0.0;
        if(psi2) *psi2=0.0;
        if(vel1) vel1->set(0.0,0.0);
        if(vel2) vel2->set(0.0,0.0);
        return;
    }

    Vector2d vl1, vl2;
    Vector2d Pl = globalToLocalPosition(pt);

    //Drela fig.2
    double lnr1, lnr2, theta1, theta2, beta1, beta2;

    lnr1 = lnr2 = theta1 = theta2 = 0.0;

    double l = Bl.x-Al.x;
    trig(Pl.x, Pl.y, beta1, beta2, lnr1, lnr2);

    theta1 = PI/2.0-beta1;
    theta2 = PI/2.0-beta2;
    theta1 += PI/2.0; // add PI/2 to stream function to be homogeneous with XFoil
    theta2 += PI/2.0;

    if(A.isSame(pt))
    {
        lnr1   = 0.0;
        theta1 = 0.0;
    }
    if(B.isSame(pt))
    {
        lnr2   = 0.0;
        theta2 = 0.0;
    }

    double xb = Bl.x;
    double xa = Al.x;
    double X = Pl.x;
    double Y = Pl.y-Al.y; // but Al.y=0 anyways
    double x1  = X-xa;
    double x2  = X-xb;

    if(psi1 && psi2)
    {
        double psiC, psiL;

        psiC =  x1*theta1 - x2*theta2;
        psiC += -Y*(lnr2-lnr1);
        psiC *= 1.0/2.0/PI;

        psiL  = 2.0*X*Y*(lnr2-lnr1);
        psiL += Y*(xb-xa);
        psiL += (X*X - Y*Y - xb*xb)*theta2 - (X*X - Y*Y - xa*xa)*theta1;
        psiL *= -1.0/4.0/PI;

        *psi1 =  (xb*psiC-psiL)/l;
        *psi2 = -(xa*psiC-psiL)/l;
    }


    if(vel1 && vel2)
    {
        double uC =  1.0/2.0/PI * (-lnr2+lnr1);
        double uL =  1.0/2.0/PI * (-X*(lnr2-lnr1) - (xb-xa) + Y*(theta2-theta1));
        double vC =  1.0/2.0/PI * (theta2-theta1);
        double vL =  1.0/2.0/PI * (Y*(lnr2-lnr1)+X*(theta2-theta1));

        vl1.x = (xb*uC-uL)/l;
        vl1.y = (xb*vC-vL)/l;

        vl2.x = -(xa*uC-uL)/l;
        vl2.y = -(xa*vC-vL)/l;

        *vel1 = localToGlobalVector(vl1);
        *vel2 = localToGlobalVector(vl2);
    }
}


/**
 * @brief Calculates angles \f$\theta_1\f$,\f$\theta_2\f$ and logarithms of distances for a field point with coordinates \f$P(x,y)\f$.
 * @param x the x-coordinate of the field point
 * @param y the y-coordinate of the field point
 * @param theta1 the angle between the panel's normal and the line from point A to P
 * @param theta2 the angle between the panel's normal and the line from point B to P
 * @param logr1 the logarithm of the distance from point A to P
 * @param logr2 the logarithm of the distance from point B to P
 */
void Panel2d::trig(double x, double y, double &beta1, double &beta2, double &logr1, double &logr2) const
{
    double x1 = x-Al.x;
    double x2 = x-Bl.x;

    double r1 = sqrt(x1*x1 + y*y);
    double r2 = sqrt(x2*x2 + y*y);

    logr1 = logr2 = beta1 = beta2 = 0.0;
    double sgn = 1.0;

    if(fabs(y)<LENGTHPRECISION) sgn = 1.0;
    else                        sgn = y>0.0 ? 1.0 : -1.0;

    if(r1>0.0)
    {
        logr1 = log(r1);
        // double atan2(double y, double x)
        beta1 = atan2(sgn*x1, sgn*y) + (0.5-0.5*sgn)*PI;
    }

    if(r2>0.0)
    {
        logr2 = log(r2);
        beta2 = atan2(sgn*x2, sgn*y) + (0.5-0.5*sgn)*PI;
    }
}


/**
 * @brief Calculates angles \f$\theta_1\f$,\f$\theta_2\f$ and logarithms of distances
 * for a field point with coordinates \f$P(x,y)\f$.
 * Using XFoil fig. 2 referential.
 * @param x the x-coordinate of the field point
 * @param y the y-coordinate of the field point
 * @param theta1 the angle between the panel's normal and the line from point A to P
 * @param theta2 the angle between the panel's normal and the line from point B to P
 * @param logr1 the logarithm of the distance from point A to P
 * @param logr2 the logarithm of the distance from point B to P
 */
void Panel2d::trigXFoil(double x, double y, double &beta1, double &beta2, double &logr1, double &logr2) const
{
    double x1 = x-Al.x;
    double x2 = x-Bl.x;

    double r1 = sqrt(x1*x1 + y*y);
    double r2 = sqrt(x2*x2 + y*y);

    logr1 = logr2 = beta1 = beta2 = 0.0;
    double sgn = 1.0;

    if(fabs(y)<PRECISION) sgn = 1.0;
    else                  sgn = y>0.0 ? 1.0 : -1.0;

    if(r1>0.0)
    {
        logr1 = log(r1);
        // double atan2(double y, double x)
        beta1 = atan2(sgn*x1, sgn*y) + (0.5-0.5*sgn)*PI;
    }

    if(r2>0.0)
    {
        logr2 = log(r2);
        beta2 = atan2(sgn*x2, sgn*y) + (0.5-0.5*sgn)*PI;
    }
}




