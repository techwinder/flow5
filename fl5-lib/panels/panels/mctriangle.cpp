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




#include <mctriangle.h>

#include <constants.h>
#include <matrix.h>
#include <panel.h>
#include <panelprecision.h>

long MCTriangle::s_NullCount=0;

MCTriangle::MCTriangle() : Triangle3d()
{
}

/** 
 * @brief The constructor with initialization of vertices.
 * The input vertices are given in the parent panel's local coordinate system.
 * In this class, the "global" coordinate system is the "local" coordinate system of the parent panel.
 * The local coordinate system is defined i.a.w. figure 1.
 *
 * @param V0 the first vertex, which is the projection of the field point on the parent panel's plane.
 * @param V1 the second vertex, which is a vertex of the parent panel's
 * @param V2 the third vertex, which is a vertex of the parent panel's
 * @param N the parent panel's normal vector, used to define the sign the triangle's orientation.
 */
MCTriangle::MCTriangle(Vector3d const& V0, Vector3d const& V1, Vector3d const& V2, Vector3d const& N) : Triangle3d(V0, V1, V2)
{
    setTriangle(V0,V1,V2,N);
}


void MCTriangle::setTriangle(Vector3d const &V0, Vector3d const &V1, Vector3d const &V2, Vector3d const &N)
{
    theta = sinT = sinT2 = sinT3 = cosT = cosT2 = cosT3 =
            beta = beta2 = beta3 = alfa = alfa2 = alfa3 = alfa4 =
            alfap = alfap2 = delta = delta2 = fz = z2 = 0.0;
    m_z = orientation = phi = 0.0;

    m_bNullTriangle = false;
    m_a = 0.0;
    r1 = r2 = thetaMax = 0.0;

    ptEval = V0;
    if(fabs(ptEval.z)<1.e-6)
        ptEval.z = 0.0;

    m_S[0].set(V0.x, V0.y, 0.0);  //i.e. the field point, in global coordinates
    m_S[1] = V1;
    m_S[2] = V2;

    //We use the parent panel's normal to define the triangle's orientation
    m_Normal = N;
    setTriangle();
}


/** @brief Sets the vertices of the triangle and defines the local variables required in the integration. */
void MCTriangle::setTriangle()
{
    //define the local coordinate system - Figure 1

    // The origin is the projection of the first vertex on the triangle's plane (fig. 2)
    O.set(m_S[0]);

    //define the three sides, in global coordinates
    S01.x = m_S[1].x-O.x;
    S01.y = m_S[1].y-O.y;
    S01.z = m_S[1].z-O.z;
    S02.x = m_S[2].x-O.x;
    S02.y = m_S[2].y-O.y;
    S02.z = m_S[2].z-O.z;
    S12.x = m_S[2].x-m_S[1].x;
    S12.y = m_S[2].y-m_S[1].y;
    S12.z = m_S[2].z-m_S[1].z;

    m_CoG_g.x = (m_S[0].x+m_S[1].x+m_S[2].x)/3.0;
    m_CoG_g.y = (m_S[0].y+m_S[1].y+m_S[2].y)/3.0;
    m_CoG_g.z = (m_S[0].z+m_S[1].z+m_S[2].z)/3.0;


    Vector3d cross;
    cross.x = S01.y*S02.z-S01.z*S02.y;
    cross.y = S01.z*S02.x-S01.x*S02.z;
    cross.z = S01.x*S02.y-S01.y*S02.x;

    area = cross.norm()/2.0;

    r1 = S01.norm();
    r2 = S02.norm();

    if(r1<SIDELENGTHPRECISION || r2<SIDELENGTHPRECISION || S12.norm()<SIDELENGTHPRECISION)
    {
        s_NullCount++;
        m_bNullTriangle = true;
        area = 0.0;
        return;
    }

    // check if the three vertices are aligned
    /*    Vector3d n1 = S01.normalized();
    Vector3d n2 = S02.normalized();
    if((n1*n2).norm()<1.e-15)
    {
        m_bNullTriangle = true;
        area = 0.0;
        return;
    }*/

    m.set(S01.normalized()); // local x-axis
    l.set(m_Normal * m);  // local y-axis

    m_CF.setOrigin(O);
    m_CF.setIJK(m,l,m_Normal);

    Ol.set(0.0,0.0,0.0);
    m_Sl[0].set(Ol + globalToLocal(m_S[0]-O));
    m_Sl[1].set(Ol + globalToLocal(m_S[1]-O));
    m_Sl[2].set(Ol + globalToLocal(m_S[2]-O));
    m_z = ptEval.z;
    
    m_CoG_l.set(Ol + globalToLocal(m_CoG_g-O));

    m_S01l.set(globalToLocal(S01));       // = Sl[1]-Sl[0];
    m_S02l.set(globalToLocal(S02));       // = Sl[2]-Sl[0];
    m_S12l.set(globalToLocal(S12));       // = Sl[2]-Sl[1];

    //set orientation
    double crossdot = cross.dot(m_Normal);
    orientation = crossdot>0 ? 1.0 : -1.0;

    //create the variables necessary for the integration

    thetaMax = atan2(m_Sl[2].y, m_Sl[2].x); //either, more precise
    //    thetaMax = orientation * acos(S01.dot(S02)/r1/r2);//either
    if(fabs(thetaMax)<VERTEXANGLEPRECISION || (fabs(thetaMax-PI))<VERTEXANGLEPRECISION || (fabs(thetaMax+PI))<VERTEXANGLEPRECISION)
    {
        s_NullCount++;
        m_bNullTriangle = true;
        area = 0.0;
        return;
    }
    assert(fabs(sin(thetaMax))>1.e-10);

    m_a = (r2*cos(thetaMax)-r1)/r2/sin(thetaMax);
    if(std::isnan(m_a))
        assert(std::isnan(m_a));

    // angle offset used in the change of variables in eq. 12
    phi = atan(m_a);
}



/**
 * @brief Sets the variables required in the integration
 * cf. MCarley Eq. (12)
 * @param thet the angle theta defined by MCarley figure (1)
 */
void MCTriangle::setIntermediateVariables(double thet)
{
    theta = thet;

    sinT  = sin(thet);

    sinT2 = sinT*sinT;
    sinT3 = sinT2*sinT;
    cosT  = cos(thet);
    if(fabs(cosT)<INTEGRALPRECISION) {cosT=cosT2=cosT3=0.0;}
    else
    {
        cosT2 = cosT*cosT;
        cosT3 = cosT*cosT*cosT;
    }

    fz    = fabs(m_z);
    z2    = m_z*m_z;

    beta2 = (r1*r1 + m_z*m_z*(1.0+m_a*m_a)) / (1.0+m_a*m_a);
    beta  = sqrt(beta2);
    beta3 = beta*beta2;

    alfa = m_z/beta;
    if(alfa<-1.0) alfa = -1.0; // alfa can happen to be 1+epsilon due to rounding errors
    if(alfa> 1.0) alfa =  1.0;
    alfa2 = alfa*alfa;
    alfa3 = alfa*alfa2;
    alfa4 = alfa2*alfa2;
    alfa5 = alfa2*alfa3;

    alfap = sqrt(1.0-alfa2);
    alfap2 = alfap*alfap;

    if(1.0 - alfa2 * sinT*sinT<0.0) delta2 = 0.0;  // same issue with rounding errors
    else                            delta2 = 1.0 - alfa2 * sinT*sinT;
    delta = sqrt(delta2);
}



/**
* @brief Implements Table 1 + eq 18 and 19. Returns the integral between 0 and theta.
* @param iLine the index of the line requested in Table 1, or iLine=15 for eq. 18 and iLine=16 for eq. 19.
* @param thet the polar angle, in radians
* @return the value of the integral
*/
double MCTriangle::table1(int iLine, bool bInPlane) const
{
    if(bInPlane)
    {
        switch(iLine)
        {
            //use column 3
            case 0:   return beta * Jmn(0,-1) ;                          //    1/R
            case 1:   return beta2/2.0 * Jmn(0, -1);                     //    x/R
            case 2:   return beta2/2.0 * Jmn(1, -2);                     //    y/R
            case 3:
            {
                if(fabs(beta)<INTEGRALPRECISION) return 0.0;
                return -1.0/beta * Jmn(0,  1);                     //    1/R³
            }
            case 4:
            {
                if(fabs(cosT)<INTEGRALPRECISION || fabs(beta)<INTEGRALPRECISION) return 0.0;
                return    -sinT  * log(cosT/beta)  - Jmn(2, -1);   //    x/R³
            }
            case 5:
            {
                if(fabs(cosT)<INTEGRALPRECISION || fabs(beta)<INTEGRALPRECISION) return 0.0;
                return     cosT  * log(cosT/beta)  + Jmn(1,  0);   //    y/R³
            }
            case 6:
            {
                if(fabs(beta2)<INTEGRALPRECISION) return 0.0;
                return -1.0/3.0/beta2/beta * Jmn(0, 3);            //    1/R⁵
            }
            case 7:
            {
                if(fabs(beta2)<INTEGRALPRECISION) return 0.0;
                return -1.0/2.0/beta2      * Jmn(0, 3);            //    x/R⁵
            }
            case 8:
            {
                if(fabs(beta2)<INTEGRALPRECISION) return 0.0;
                return -1.0/2.0/beta2      * Jmn(1, 2);            //    y/R⁵
            }
            case 9:
            {
                return beta      * Jmn(0,  1);                     //   x²/R³
            }
            case 10:  return beta      * Jmn(1,  0);                     //   xy/R³
            case 11:  return beta      * Jmn(2, -1);                     //   y²/R³
            case 12:
            {
                if(fabs(beta)<INTEGRALPRECISION) return 0.0;
                return -1.0/beta * Jmn(0,  3);                     //   x²/R⁵
            }
            case 13:
            {
                if(fabs(beta)<INTEGRALPRECISION) return 0.0;
                return -1.0/beta * Jmn(1,  2);                     //   xy/R⁵
            }
            case 14:
            {
                if(fabs(beta)<INTEGRALPRECISION) return 0.0;
                return -1.0/beta * Jmn(2,  1);                     //   y²/R⁵
            }
            default: return 0.0;
        }
    }
    else
    {
        switch(iLine)
        {
            //use column 2
            case 0:  return beta* Ipmn(1,0,-1) - fz* Jmn(0,0);
            case 1:
            {
                if(fabs(delta+alfap)<INTEGRALPRECISION || fabs(delta-alfap)<INTEGRALPRECISION) return 0.0;
                return beta2*alfap/2.0 * Ipmn(1,0,-1) + z2*alfap/2.0 * Ipmn(-1,2,-1) - z2/4.0*sinT * log((delta+alfap)/(delta-alfap));
            }
            case 2:
            {
                if(fabs(delta+alfap)<INTEGRALPRECISION || fabs(delta-alfap)<INTEGRALPRECISION) return 0.0;
                return beta2*alfap/2.0 * Ipmn(1,1,-2) - z2*alfap/2.0 * Ipmn(-1,1, 0) + z2/4.0*cosT * log((delta+alfap)/(delta-alfap));
            }
            case 3:  return -1.0/beta*Ipmn(-1, 0, 1) + 1.0/fz * Jmn(0,0);
            case 4:
            {
                if(fabs(delta+alfap)<INTEGRALPRECISION || fabs(delta-alfap)<INTEGRALPRECISION) return 0.0;
                return -alfap * Ipmn(-1,0,1) - alfap * Ipmn(-1,2,-1) + sinT/2.0 * log((delta+alfap)/(delta-alfap));
            }
            case 5:
            {
                //                if(fabs(cosT)<PRECISION) return 0.0;
                if(fabs(delta+alfap)<INTEGRALPRECISION || fabs(delta-alfap)<INTEGRALPRECISION) return 0.0;
                return -cosT/2.0 * log((delta+alfap)/(delta-alfap));
            }
            case 6:
            {
                return -1.0/3.0 * (1.0/beta3*Ipmn(-3,0,3) - 1.0/fz/fz/fz*Jmn(0,0));
            }
            case 7:  return alfap*alfap*alfap/3.0/z2*Ipmn(-3,0,1);
            case 8:  return alfap*alfap*alfap/3.0/z2*Ipmn(-3,1,0);
            case 9:  return beta*Ipmn(1,0, 1)+z2/beta*Ipmn(-1,0,3) - 2.0*fz*Jmn(0,2);
            case 10: return beta*Ipmn(1,1, 0)+z2/beta*Ipmn(-1,1,2) - 2.0*fz*Jmn(1,1);
            case 11:
            {
                return beta*Ipmn(1,2,-1)+z2/beta*Ipmn(-1,2,1) - 2.0*fz*Jmn(2,0);
            }
            case 12: return -1.0/beta *Ipmn(-1,0,3) + z2/3.0/beta3*Ipmn(-3,0,5) + 2.0/3.0/fz*Jmn(0,2);
            case 13: return -1.0/beta *Ipmn(-1,1,2) + z2/3.0/beta3*Ipmn(-3,1,4) + 2.0/3.0/fz*Jmn(1,1);
            case 14: return -1.0/beta *Ipmn(-1,2,1) + z2/3.0/beta3*Ipmn(-3,2,3) + 2.0/3.0/fz*Jmn(2,0);
            default: return 0.0;
        }
    }
}



/**
* @brief Implements Table 3.
* @param p the power of delta = sqrt(1-alfa²sin²(theta)) i.a.w. eq 14
* @param m the power of sin(theta) i.a.w. eq. 22
* @param n the power of cos(theta) i.a.w. eq. 22
* @param theta the polar angle, in radians
* @return the value of the integral between 0 and theta
*/
double MCTriangle::Ipmn(int p, int m, int n) const
{
    if(p==-3)
    {
        if      (m==0 && n==1)
        {
            if(fabs(delta)<INTEGRALPRECISION) return 0.0;
            return sinT/delta;
        }
        else if (m==1 && n==0)
        {
            if(fabs(alfap2)<INTEGRALPRECISION || fabs(delta)<INTEGRALPRECISION) return 0.0;
            return -1.0/alfap2 * cosT / delta;
        }
        else if (m==0 && n==3)
        {
            if(fabs(delta)<INTEGRALPRECISION || fabs(alfa*sinT)>1.0) return 0.0;
            return -alfap2/alfa2 * sinT/delta + 1.0/alfa3 * asin(alfa*sinT);
        }
        else if (m==2 && n==3)
        {
            if(fabs(2.0*alfa5*delta)<INTEGRALPRECISION) return 0.0;
            return -( (2.0*alfa2-3.0)*delta*asin(alfa*sinT) - alfa2*alfa*sinT3 + (3.0-2.0*alfa2)*alfa*sinT) / (2.0*alfa5*delta);  //added
        }
        else if (m==1 && n==4)
        {
            if(fabs(1-alfa2)<INTEGRALPRECISION && fabs(cosT3)<INTEGRALPRECISION) return 0.0;
            double d3 =  3*(1-alfa2)*log(fabs(alfa*sqrt(alfa2*cosT2-alfa2+1)+alfa2*cosT)) / (2*alfa4*alfa)
                    -cosT3 / (2*alfa2*sqrt(alfa2*cosT2-alfa2+1))
                    -3*(1-alfa2)*cosT / (2*alfa4*sqrt(alfa2*cosT2-alfa2+1)); //added
            return d3;
        }
        else if (m==0 && n==5)
        {
            if(fabs(2*alfa5*delta)<INTEGRALPRECISION)
                return 0.0;
            return ((4*alfa2-3)*delta*asin(alfa*sinT) - alfa3*sinT3 +(2*alfa4-4*alfa2+3)*alfa*sinT)
                    /(2*alfa5*delta); // added
        }
        else
        {
            // eq. 39
            double I = 1.0/alfa2/delta*(pow(sinT, m-1)+pow(cosT, n-1));
            I -= double(m-1)/alfa2*Ipmn(-1,m-2,n);
            I += double(n-1)/alfa2*Ipmn(-1,m,n-2);
            return I;
        }
    }
    else if(p==-1)
    {
        if (m==0 && n==1)
        {
            if(fabs(alfa*sinT)>1.0) return 0.0;
            return  1.0/alfa * asin(alfa*sinT);
        }
        //        else if (m==0 && n==3)  return (alfa2-1.0)*log(fabs(alfa*sinT+1.0))/(2.0*alfa3) - (alfa2-1)*log(fabs(alfa*sinT-1.0))/(2.0*alfa3) + sinT/alfa2;
        //        else if (m==0 && n==3)  return ((2*alfa2+1)*asinh(fabs(alfa)*sinT)-fabs(alfa)*sinT*sqrt((alfa2*sinT2+1)))/(2*alfa2*fabs(alfa));
        else if (m==0 && n==3)
        {
            if(fabs(alfa*sinT)>1.0) return 0.0;
            //2.584 10.
            return delta*sinT/2.0/alfa2 + (2.0*alfa2-1)/2.0/alfa3 * asin(alfa*sinT);
        }
        else if (m==1 && n==0)
        {
            if(alfa*cosT + delta<INTEGRALPRECISION) return 0.0;
            return  -1.0/alfa * log(alfa*cosT + delta);
        }
        //        else if (m==1 && n==2)  return -((alfa2-1)*log(fabs(alfa*sqrt(alfa2*cosT2-alfa2+1) + alfa2*cosT)) + alfa*cosT*sqrt(alfa2*cosT2-alfa2+1))/(2*alfa3);
        //        else if (m==1 && n==2)  return -delta*cosT/2.0/alfa2 + alfap2/2.0/alfa3*log(alfa*cosT+delta);
        else if (m==1 && n==2)
        {
            if     (fabs(cosT2)<INTEGRALPRECISION)                              return 0.0;
            else if(fabs(sqrt((1-alfa2)*sinT2/cosT2+1)+alfa)<INTEGRALPRECISION) return 0.0;
            else if(fabs(sqrt((1-alfa2)*sinT2/cosT2+1)-alfa)<INTEGRALPRECISION) return 0.0;
            else if(1-(alfa2-1)*sinT2/cosT2<0.0)                                return 0.0;
            return   -((alfa2-1)*log(fabs(sqrt((1-alfa2)*sinT2/cosT2+1)+alfa))
                       + (1-alfa2)*log(fabs(sqrt((1-alfa2)*sinT2/cosT2+1)-alfa))
                       + sqrt(1-(alfa2-1)*sinT2/cosT2)*(alfa*(cosT2-sinT2)+alfa))/(4*alfa3);
        }
        else if (m==2 && n==1)
        {
            if(fabs(alfa*sinT)>1.0) return 0.0;
            return  (asin(alfa*sinT)-alfa*sinT*delta) / (2.*alfa2*alfa);  // added
        }
        else if (m==2 && n==-1)
        {
            if     (fabs(delta+alfap*sinT)<INTEGRALPRECISION) return 0.0;
            else if(fabs(delta-alfap*sinT)<INTEGRALPRECISION) return 0.0;
            else if(fabs(alfap)<INTEGRALPRECISION)            return 0.0;
            else if(fabs(alfa)<INTEGRALPRECISION)             return 0.0;
            else if(fabs(alfa*sinT)>1.0)                      return 0.0;
            return  1.0/2.0/alfap * log((delta+alfap*sinT)/(delta-alfap*sinT)) - 1.0/alfa * asin(alfa*sinT);
        }
    }
    else if (p==1)
    {
        if      (m==0 && n==-1)
        {
            if(fabs(delta+alfap*sinT)<INTEGRALPRECISION) return 0.0;
            if(fabs(delta-alfap*sinT)<INTEGRALPRECISION) return 0.0;
            return  alfap/2.0 * log((delta+alfap*sinT)/(delta-alfap*sinT)) + alfa * asin(alfa*sinT);
        }
        //        else if (m==0 && n== 1) return  asinh(fabs(alfa)*sinT)/(2.0*fabs(alfa)) + sinT*sqrt(1.0+alfa2*sinT2)/2.0;
        else if (m==0 && n== 1)
        {
            //2.583 3.
            if      (alfa*sinT>1.0)
                return delta*sinT/2 + 1.0/2.0/alfa*PI/2.0;
            else if (alfa*sinT<-1.0)
                return delta*sinT/2 - 1.0/2.0/alfa*PI/2.0;
            return delta*sinT/2 + 1.0/2.0/alfa*asin(alfa*sinT);
        }
        else if (m==2 && n==-1)
        {
            if(fabs(delta+alfap*sinT)<INTEGRALPRECISION) return 0.0;
            if(fabs(delta-alfap*sinT)<INTEGRALPRECISION) return 0.0;
            if(fabs(alfa)<INTEGRALPRECISION) return 0.0;
            return -delta*sinT/2.0 + (2.0*alfa2-1.0)/2.0/alfa * asin(alfa*sinT) + alfap/2.0 * log((delta+alfap*sinT)/(delta-alfap*sinT));
        }
        else if (m==1 && n==-2)
        {
            if       (fabs(cosT)<INTEGRALPRECISION)        return 0.0;
            else if  (alfa*cosT + delta<INTEGRALPRECISION) return 0.0;
            return  delta/cosT - alfa*log(alfa*cosT + delta);
        }
        else if (m==1 && n== 0)
        {
            if       (fabs(alfa)<INTEGRALPRECISION)        return 0.0;
            else if  (alfa*cosT + delta<INTEGRALPRECISION) return 0.0;
            return -delta*cosT/2.0 - 1.0/2.0 * alfap2/alfa*log(alfa*cosT + delta);
        }
    }
    //should never arrive here...
    return 0.0;
}


/**
* @brief Implements Table 4.
* @param m the power of sin(theta) i.a.w. eq. 22
* @param n the power of cos(theta) i.a.w. eq. 22
* @param theta the polar angle, in radians
* @return the value of the integral between 0 and theta
*/
double MCTriangle::Jmn(int m, int n) const
{
    if     (m==0 && n== 0) return theta;
    //    else if(m==0 && n==-1) return log((1.0+sinT)/(1.0-sinT));
    //    else if(m==0 && n==-1) return log((1.0+sinT)/(cosT)); //using Maple
    else if(m==0 && n==-1)
    {
        if(fabs(sinT)>1.0-INTEGRALPRECISION) return 0.0;
        return 1.0/2.0*(log(1.0+sinT) - log(1.0-sinT)); // http://www.integral-calculator.com/
    }
    else if(m==0 && n== 1) return sinT;
    else if(m==1 && n== 0) return -cosT;
    else if(m==1 && n== 1) return -cos(2.0*theta)/4.0;             // added
    else if(m==0 && n== 3) return sinT - sinT*sinT*sinT/3.0;
    else if(m==1 && n==-2)
    {
        if(fabs(cosT)<INTEGRALPRECISION) return 0.0;
        return 1.0/cosT;
    }
    else if(m==1 && n== 2) return -cosT*cosT*cosT/3.0;
    else if(m==2 && n== 0) return theta/2.0-sin(2.0*theta)/4.0;    // added
    else if(m==2 && n== 1) return 1/3.0*sinT*sinT*sinT;            // added
    else if(m==0 && n== 2) return theta/2.0+sin(2.0*theta)/4.0;    // added
    //    else if(m==2 && n==-1) return -sinT + log(PI/4.0 + theta/2.0);
    else if(m==2 && n==-1)
    {
        if(fabs(1.0+sinT)<INTEGRALPRECISION || fabs(1.0-sinT)<INTEGRALPRECISION) return 0.0;
        return -sinT + 1.0/2.0*(log(1.0+sinT) - log(1.0-sinT)); // http://www.integral-calculator.com/
    }
    else
    {
        //http://www.sosmath.com/calculus/integration/powerproduct/powerproduct.html
        return 0.0;
    }
}



/**
 * @brief Calculates the integrals used in the evaluation of potentials and velocities.
 * Assumes that the triangle is not degenerate, so check it before calling this function.
 * @param Pt the field point
 * @param bSelf true if the field point lies in the plane of the triangle, false otherwise
 * @param *I1 a pointer to the array of 1st order integral values, in the order 1/R, x/R, y/R
 * @param *I3 a pointer to the array of 3rd order integral values, in the order 1/R³, x/R³, y/R³, x²/R³, xy/R³, y²/R³
 * @param *I5 a pointer to the array of 5fift order integral values, in the order 1/R⁵, x/R⁵, y/R⁵, x²/R⁵, xy/R⁵, y²/R⁵
 */
void MCTriangle::integrate(Vector3d const &Pt, bool bSelf, double *I1, double *I3, double *I5, bool bGradients)
{
    double psi, dzeta, cosdz, sindz, X0, Y0;
    double MC1[3], MC3[6], MC5[6];
    memset(MC1, 0, 3*sizeof(double));
    memset(MC3, 0, 6*sizeof(double));
    memset(MC5, 0, 6*sizeof(double));

    psi = atan2(m_S[1].y-Pt.y, m_S[1].x-Pt.x);
    dzeta = psi-phi;
    cosdz = cos(dzeta);
    sindz = sin(dzeta);
    X0 = Pt.x;
    Y0 = Pt.y;

    if(fabs(m_z)<INPLANEPRECISION)
        bSelf = true;

    //upper bound
    setIntermediateVariables(thetaMax+phi);
    if(I1)  for(int i=0; i<3; i++)
    {
        MC1[i]=table1(i, bSelf);
        assert(!std::isinf(MC1[i]) && !std::isnan(MC1[i]));
    }
    if(I3)
    {
        for(int i=3; i<6;  i++)
        {
            int i0 = i-3;
            MC3[i0]=table1(i, bSelf);
            assert(!std::isinf(MC3[i0]) && !std::isnan(MC3[i0]));
        }
        if(bGradients)
        {
            for(int i=9; i<12; i++)
            {
                int i0 = i-6;
                MC3[i0]=table1(i, bSelf);
                assert(!std::isinf(MC3[i0]) && !std::isnan(MC3[i0]));
            }
        }
    }
    if(I5)
    {
        for(int i=6;  i<9;  i++)
        {
            int i0=i-6;
            MC5[i0]=table1(i, bSelf);
            assert(!std::isinf(MC5[i0]) && !std::isnan(MC5[i0]));
        }
        if(bGradients)
        {
            for(int i=12; i<15; i++)
            {
                int i0=i-9;
                MC5[i0]=table1(i, bSelf);
                assert(!std::isinf(MC5[i0]) && !std::isnan(MC5[i0]));
            }
        }
    }

    //lower bound
    setIntermediateVariables(phi);
    if(I1)
    {
        for(int i=0; i<3; i++)
        {
            MC1[i]-=table1(i, bSelf);
            assert(!std::isinf(MC1[i]) && !std::isnan(MC1[i]));
        }
    }
    if(I3)
    {
        for(int i=3; i<6;  i++)
        {
            int i0 = i-3;
            MC3[i0]-=table1(i, bSelf);
            assert(!std::isinf(MC3[i0]) && !std::isnan(MC3[i0]));
        }
        if(bGradients)
        {
            for(int i=9; i<12; i++)
            {
                int i0 = i-6;
                MC3[i0]-=table1(i, bSelf);
                assert(!std::isinf(MC3[i0]) && !std::isnan(MC3[i0]));
            }
        }
    }
    if(I5)
    {
        if(bGradients)
        {
            for(int i=6;  i<9;  i++)
            {
                int i0 = i-6;
                MC5[i0]-=table1(i, bSelf);
                assert(!std::isinf(MC5[i0]) && !std::isnan(MC5[i0]));
            }
            for(int i=12; i<15; i++)
            {
                int i0=i-9;
                MC5[i0]-=table1(i, bSelf);
                assert(!std::isinf(MC5[i0]) && !std::isnan(MC5[i0]));
            }
        }
    }


    // Convert the integrals into the panel's reference frame
    if(I1)
    {
        I1[0]  += MC1[0];                                      // 1/R
        I1[1]  += Pt.x*MC1[0] + cosdz*MC1[1] - sindz*MC1[2];   // x/R
        I1[2]  += Pt.y*MC1[0] + sindz*MC1[1] + cosdz*MC1[2];   // y/R
    }
    if(I3)
    {
        I3[0]  += MC3[0];                                      // 1/R³
        I3[1]  += Pt.x*MC3[0] + cosdz*MC3[1] - sindz*MC3[2];   // x/R³
        I3[2]  += Pt.y*MC3[0] + sindz*MC3[1] + cosdz*MC3[2];   // y/R³

        //second order: xx/R³, xy/R³, zz/R³
        if(bGradients)
        {
            I3[3] += X0*X0*MC3[0] + cosdz*cosdz*MC3[3] + sindz*sindz*MC3[5] + 2.0*X0*cosdz*MC3[1] - 2.0*X0*sindz*MC3[2] - 2.0*sindz*cosdz*MC3[4];
            I3[4] += X0*Y0*MC3[0] + (X0*sindz+Y0*cosdz)*MC3[1] + (X0*cosdz-Y0*sindz)*MC3[2]
                    + sindz*cosdz*MC3[3]    + (cosdz*cosdz-sindz*sindz)*MC3[4] - sindz*cosdz*MC3[5];
            I3[5] += Y0*Y0*MC3[0] + sindz*sindz*MC3[3] + cosdz*cosdz*MC3[5] + 2.0*Y0*sindz*MC3[1] + 2.0*Y0*cosdz*MC3[2] + 2.0*sindz*cosdz*MC3[4];
        }
    }

    if(I5)
    {
        if(bGradients)
        {
            I5[0]  += MC5[0];                                      // 1/R⁵
            I5[1]  += Pt.x*MC5[0] + cosdz*MC5[1] - sindz*MC5[2];   // x/R⁵
            I5[2]  += Pt.y*MC5[0] + sindz*MC5[1] + cosdz*MC5[2];   // y/R⁵

            //second order: xx/r⁵, xy/r⁵, yy/r⁵
            I5[3] += X0*X0*MC5[0] + cosdz*cosdz*MC5[3] + sindz*sindz*MC5[5] + 2.0*X0*cosdz*MC5[1] - 2.0*X0*sindz*MC5[2] - 2.0*sindz*cosdz*MC5[4];
            I5[4] += X0*Y0*MC5[0] + (X0*sindz+Y0*cosdz)*MC5[1] + (X0*cosdz-Y0*sindz)*MC5[2]
                    + sindz*cosdz*MC5[3]    + (cosdz*cosdz-sindz*sindz)*MC5[4] - sindz*cosdz*MC5[5];
            I5[5] += Y0*Y0*MC5[0] + sindz*sindz*MC5[3] + cosdz*cosdz*MC5[5] + 2.0*Y0*sindz*MC5[1] + 2.0*Y0*cosdz*MC5[2] + 2.0*sindz*cosdz*MC5[4];
        }
    }
}


