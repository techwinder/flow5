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


// Visual studio bug override
//https://developercommunity.visualstudio.com/t/Visual-Studio-17100-Update-leads-to-Pr/10669759?sort=newest
//#define _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR

#include <QString>


#include <llttask.h>

#include <objects2d.h>
#include <planeopp.h>
#include <planepolar.h>
#include <planexfl.h>
#include <polar.h>
#include <trace.h>
#include <units.h>
#include <wingxfl.h>

int LLTTask::s_IterLim = 100;
int LLTTask::s_NLLTStations = 20;
double LLTTask::s_RelaxMax = 20.0;
double LLTTask::s_CvPrec = 0.01;


LLTTask::LLTTask()
{
    m_pWing = nullptr;
    m_pPlPolar = nullptr;

    resetVariables();
}


void LLTTask::resetVariables()
{
    m_AoAList.clear();
    m_bConverged = false;
    m_bWingOut   = false;
    m_bError = m_bWarning = false;

    m_QInf0 = 0.0;

    m_CL = 0.0;
    m_CDi = 0.0;
    m_CDv = 0.0;

    m_VYm = m_IYm = m_GYm = 0.0;
    m_VCm = m_ICm = m_GCm = 0.0;
    m_GRm = 0.0;

    m_CP.set(0.0,0.0,0.0);

    m_Chord.resize(        s_NLLTStations+1);
    m_Offset.resize(       s_NLLTStations+1);
    m_Twist.resize(        s_NLLTStations+1);
    m_SpanPos.resize(      s_NLLTStations+1);
    m_StripArea.resize(    s_NLLTStations+1);

    m_Re.resize(           s_NLLTStations+1);
    m_Cl.resize(           s_NLLTStations+1);
    m_Ai.resize(           s_NLLTStations+1);
    m_ICd.resize(          s_NLLTStations+1);
    m_PCd.resize(          s_NLLTStations+1);
    m_Cm.resize(           s_NLLTStations+1);
    m_CmAirf.resize(       s_NLLTStations+1);
    m_XCPSpanRel.resize(   s_NLLTStations+1);
    m_XCPSpanAbs.resize(   s_NLLTStations+1);
    m_BendingMoment.resize(s_NLLTStations+1);
    m_XTrTop.resize(       s_NLLTStations+1);
    m_XTrBot.resize(       s_NLLTStations+1);

    std::fill(m_Chord.begin(),         m_Chord.end(),         0);
    std::fill(m_Offset.begin(),        m_Offset.end(),        0);
    std::fill(m_Twist.begin(),         m_Twist.end(),         0);
    std::fill(m_SpanPos.begin(),       m_SpanPos.end(),       0);
    std::fill(m_StripArea.begin(),     m_StripArea.end(),     0);
    std::fill(m_Re.begin(),            m_Re.end(),            0);
    std::fill(m_Cl.begin(),            m_Cl.end(),            0);
    std::fill(m_Ai.begin(),            m_Ai.end(),            0);
    std::fill(m_ICd.begin(),           m_ICd.end(),           0);
    std::fill(m_PCd.begin(),           m_PCd.end(),           0);
    std::fill(m_Cm.begin(),            m_Cm.end(),            0);
    std::fill(m_CmAirf.begin(),        m_CmAirf.end(),        0);
    std::fill(m_XCPSpanRel.begin(),    m_XCPSpanRel.end(),    0);
    std::fill(m_XCPSpanAbs.begin(),    m_XCPSpanAbs.end(),    0);
    std::fill(m_BendingMoment.begin(), m_BendingMoment.end(), 0);
    std::fill(m_XTrTop.begin(),        m_XTrTop.end(),        0);
    std::fill(m_XTrBot.begin(),        m_XTrBot.end(),        0);
}


void LLTTask::initializeVelocity(double alpha, double &QInf)
{
    Foil *pFoil0(nullptr), *pFoil1(nullptr);
    double tau(0);

    switch (m_pPlPolar->type())
    {
        case xfl::T1POLAR:
        {
            QInf = m_pPlPolar->velocity();
            break;
        }
        case xfl::T2POLAR:
        {
            double Lift=0.0;// required for Type 2

            for (int k=1; k<s_NLLTStations; k++)
            {
                double yob   = cos(k*PI/s_NLLTStations);
                m_pWing->getFoils(&pFoil0, &pFoil1, yob*m_pWing->planformSpan()/2.0, tau);
                double alpha0 = Objects2d::getZeroLiftAngle(pFoil0, pFoil1, LARGEVALUE, tau);
                double Cl = 2.0*PI*(alpha-alpha0+m_Twist.at(k))*PI/180.0;
                Lift += Eta(k) * Cl * m_Chord.at(k) /m_pWing->planformSpan();
            }
            if(Lift<=0.0) return;
            QInf  = m_QInf0 / sqrt(Lift);
            break;
        }
        default:
        {
            traceStdLog("******* LLT is only compatible with T1 and T2 polars ******\n\n");
            QInf = m_pPlPolar->velocity();
            break;
        }
    }

    for (int k=1; k<s_NLLTStations; k++)
    {
        m_Re[k] = m_Chord.at(k) * QInf /m_pPlPolar->viscosity();
    }
}


double LLTTask::Eta(int m) const
{
    return PI/2.0/double(s_NLLTStations) * sin(double(m)*PI/double(s_NLLTStations));
}


double LLTTask::Sigma(int m) const
{
    return PI/8.0/double(s_NLLTStations) * sin(2.*double(m)*PI/double(s_NLLTStations)) ;
}


double LLTTask::Beta(int m, int k) const
{
    double b=0;
    double fk = double(k);
    double fm = double(m);
    double fr = double(s_NLLTStations);

    if (m==k) b = 180.0*fr/8.0/PI/sin(fk*PI/fr);
    else if (isEven(m+k)) b=0.0;
    else
    {
        double c1 = 180.0/4.0/PI/fr/sin(fk*PI/fr);
        double c2 =   1.0/(1.0-cos((fk+fm)*PI/fr)) - 1.0/(1.0-cos((fk-fm)*PI/fr));
        b = c1 * c2;
    }
    return b;
}


void LLTTask::computeWing(double QInf, double Alpha, std::string &ErrMessage)
{
    Foil* pFoil0(nullptr);
    Foil* pFoil1(nullptr);

    QString ErrorMessage;

    double yob(0), tau(0), c4(0), zpos(0);
    double Integral0(0), Integral1(0), Integral2(0), Integral3(0);
    double InducedDrag(0), ViscousDrag(0);
    double InducedYawingMoment(0), ViscousYawingMoment(0);
    double PitchingMoment(0);
    double VCm(0), ICm(0), eta(0), sigma(0), Cm0(0);

    bool bOutRe(false), bError(false);
    bool bPointOutRe(false), bPointOutAlpha(false);
    m_bWingOut = false;

    ErrorMessage.clear();

    for (int m=1; m<s_NLLTStations; m++)
    {
        bPointOutRe    = false;
        bPointOutAlpha = false;
        yob   = cos(double(m)*PI/double(s_NLLTStations));
        m_pWing->getFoils(&pFoil0, &pFoil1, yob*m_pWing->planformSpan()/2.0, tau);

        m_Cl[m]     = Objects2d::getPlrPointFromAlpha(Polar::CL, pFoil0, pFoil1, m_Re[m], Alpha+m_Ai[m]+m_Twist[m], tau, bOutRe, bError);
        if(bOutRe) bPointOutRe = true;
        if(bError) bPointOutAlpha = true;

        m_PCd[m]    = Objects2d::getPlrPointFromAlpha(Polar::CD, pFoil0, pFoil1, m_Re[m], Alpha+m_Ai[m]+m_Twist[m], tau, bOutRe, bError);
        if(bOutRe) bPointOutRe = true;
        if(bError) bPointOutAlpha = true;

        m_ICd[m]    = -m_Cl[m] * (m_Ai[m]* PI/180.0);

        m_XTrTop[m] = Objects2d::getPlrPointFromAlpha(Polar::XTRTOP, pFoil0, pFoil1, m_Re[m], Alpha+m_Ai[m] + m_Twist[m], tau, bOutRe, bError);
        if(bOutRe) bPointOutRe = true;
        if(bError) bPointOutAlpha = true;

        m_XTrBot[m] = Objects2d::getPlrPointFromAlpha(Polar::XTRBOT, pFoil0, pFoil1, m_Re[m], Alpha+m_Ai[m]+m_Twist[m], tau, bOutRe, bError);
        if(bOutRe) bPointOutRe = true;
        if(bError) bPointOutAlpha = true;

        m_CmAirf[m] = Objects2d::getPlrPointFromAlpha(Polar::CM, pFoil0, pFoil1, m_Re[m], Alpha+m_Ai[m]+m_Twist[m], tau, bOutRe, bError);
        if(bOutRe) bPointOutRe = true;
        if(bError) bPointOutAlpha = true;

        m_XCPSpanRel[m] = Objects2d::getPlrPointFromAlpha(Polar::XCP, pFoil0, pFoil1, m_Re[m], Alpha+m_Ai[m]+m_Twist[m], tau, bOutRe, bError);

        if(fabs(m_XCPSpanRel[m])<0.000001)
        {
            //plr mesh was generated prior to v3.15, i.e., without XCp calculations
            Cm0 = Objects2d::getCm0(pFoil0, pFoil1, m_Re[m],tau, bOutRe, bError);
            if(m_Cl[m]!=0.0) m_XCPSpanRel[m] = 0.25 - Cm0/m_Cl[m];
            else             m_XCPSpanRel[m] = 0.25;
        }
        if(bOutRe) bPointOutRe = true;
        if(bError) bPointOutAlpha = true;

        // incorrect up to v6.47, moments should be calculated in wind axes, notwithstanding induced angle and twist
        // makes a non-significant difference on results
/*        double arad = (Alpha+m_Ai[m]+m_Twist[m])*PI/180.0;
        double sina = sin(arad);
        double cosa = cos(arad);*/

        // v6.48
        // added wingLE position to the calculation of lever arms - cf. Ticket 147
        c4   = m_pPlane->wingLE(0).x + m_pWing->C4(yob)                                     - m_pPlPolar->CoG().x; //m
        zpos = m_pPlane->wingLE(0).z + m_pWing->ZPosition(yob*m_pWing->m_PlanformSpan/2.0)  - m_pPlPolar->CoG().z; //m
        Vector3d LeverArm(c4, 0, zpos); //m
        Vector3d Finv(m_ICd[m], 0.0, m_Cl[m]); // Inviscid force,  N/qS
        Vector3d Fvisc(m_PCd[m],0,0);          // Viscous force,   N/qS
        Vector3d Minv  = LeverArm * Finv;      // Inviscid moment, Nm/qS
        Vector3d Mvisc = LeverArm * Fvisc;     // Viscousmoment,   Nm/qS

        double Cm_i = m_CmAirf[m] + Minv.y /m_pWing->MAC();  // N.m/qSc
        double Cm_v =               Mvisc.y/m_pWing->MAC();  // N.m/qSc

        m_Cm[m] = Cm_i + Cm_v;                               // N.m/qSc

        eta = Eta(m);
        sigma = Sigma(m);
        Integral0           += eta   * m_Cl[m]  * m_Chord[m];
        Integral1           += sigma * m_Cl[m]  * m_Chord[m];
        Integral2           += eta   * m_Cl[m]  * m_Chord[m] * (m_Offset[m]+m_XCPSpanRel[m]*m_Chord[m]);
        Integral3           += eta   * m_Cl[m]  * m_Chord[m] * (zpos*m_Chord[m]);
        //  Integral3           += eta   * m_Cl[m]  * m_Chord[m] * ((m_XCPSpanRel[m]*m_Chord[m]*cos(-m_Twist[m]*PI/180.0)+m_Offset[m]) * sin(-Alpha*PI/180.0) + (zpos*m_Chord[m]+m_XCPSpanRel[m]*m_Chord[m]*sin(-m_Twist[m]*PI/180.0)) * cos(-Alpha*PI/180.0));
        InducedDrag         += eta   * m_Cl[m]  * m_Chord[m] * (-m_Ai[m]);
        ViscousDrag         += eta   * m_PCd[m] * m_Chord[m];
        InducedYawingMoment += sigma * m_Cl[m]  * m_Chord[m] * (-m_Ai[m]);
        ViscousYawingMoment += sigma * m_PCd[m] * m_Chord[m];
        PitchingMoment      += eta   * m_Cm[m]      * m_Chord[m] * m_Chord[m];
        VCm                 += eta   * Cm_v    * m_Chord[m] * m_Chord[m];
        ICm                 += eta   * Cm_i * m_Chord[m] * m_Chord[m];

        if(bPointOutAlpha)
        {
            ErrorMessage = QString::asprintf("       Span pos = %9.2f ", cos(m*PI/s_NLLTStations)*m_pWing->planformSpan()/2.0*Units::mtoUnit());
            ErrorMessage += Units::lengthUnitQLabel();
            ErrorMessage += ",  Re = ";
            QString str;
            str = QString::asprintf("%.0f", m_Re.at(m));
            ErrorMessage += str;

            str = QString::asprintf(" ,  A+Ai+Twist = %2f could not be interpolated\n", Alpha+m_Ai.at(m) + m_Twist.at(m));
            ErrorMessage+=str;

            m_bWingOut = true;
            m_bConverged = false;
        }
        else if(bPointOutRe)
        {
            ErrorMessage = QString::asprintf("       Span pos = %9.2f ", cos(m*PI/s_NLLTStations)*m_pWing->planformSpan()/2.0*Units::mtoUnit());
            ErrorMessage += Units::lengthUnitQLabel();
            ErrorMessage += ",  Re = ";
            QString str;
            str = QString::asprintf("%.0f", m_Re.at(m));
            ErrorMessage += str;

            str = QString::asprintf(" ,  A+Ai+Twist = %2f is outside the flight envelope\n", Alpha+m_Ai.at(m) + m_Twist.at(m));
            ErrorMessage+=str;

            m_bWingOut = true;
        }
    }

    m_CL    =  Integral0   * m_pWing->aspectRatio() /m_pWing->planformSpan();
    m_CDi   =  InducedDrag * m_pWing->aspectRatio() /m_pWing->planformSpan()  * PI / 180.0;
    m_CDv   =  ViscousDrag / m_pWing->GChord();

    m_VYm = ViscousYawingMoment /m_pWing->GChord();
    m_IYm = InducedYawingMoment /m_pWing->planformSpan() * PI * m_pWing->aspectRatio() /180.0;
    m_GYm = m_VYm + m_IYm;
    // m_GCm = PitchingMoment / m_GChord / m_MAChord;
    m_VCm = VCm / m_pWing->GChord() / m_pWing->MAC();
    m_ICm = ICm / m_pWing->GChord() / m_pWing->MAC();
    m_GCm = m_VCm + m_ICm;

    m_GRm = -Integral1   * m_pWing->aspectRatio() /m_pWing->planformSpan();

    if(m_CL !=0.0)
    {
        m_CP.x = Integral2 * m_pWing->aspectRatio() /m_pWing->planformSpan()/m_CL;
        //        m_ZCP = Integral3 * m_pWing->aspectRatio() /m_pWing->planformSpan()/m_CL;
        m_CP.z=0.0;//the ZCP position may make physical sense in 3D panel analysis, but not in LLT

    }
    else
    {
        m_CP.set(0.0,0.0,0.0);
    }
    if(m_pWing->isSymmetric()) m_CP.y = 0.0;
    else                      m_CP.y = m_pWing->aspectRatio()/m_CL * Integral1;

    setBending(QInf);
    (void)Integral3;
    (void)PitchingMoment;

    ErrMessage = ErrorMessage.toStdString();
}


void LLTTask::setBending(double QInf)
{
    //dynamic pressure, kg/m³
    double q = 0.5*m_pPlPolar->density() * QInf * QInf;

    for (int j=1; j<s_NLLTStations; j++)
    {
        double y = m_SpanPos[j];
        double bm = 0.0;
        if (y>=0)
        {
            for (int jj=0; jj<j; jj++)
            {
                double yy =  m_SpanPos[jj];
                bm += (yy-y) * m_Cl[jj] * m_StripArea[jj];
            }
        }
        else
        {
            for (int jj=j+1; jj<s_NLLTStations; jj++)
            {
                double yy =  m_SpanPos[jj];
                bm += (y-yy) * m_Cl[jj] * m_StripArea[jj];
            }
        }
        m_BendingMoment[j] = bm*q;
    }
}


/**
 * Calculates the linear solution to the Lifting line problem, for the given wing geometry and angle of attack.
 * This is the starting point for the non-linear iterations.
 * A simplifying assumtion is that the lift slope is Cl = 2.pi (alpha-alpha0+wahshout) at all positions.
 * Numerical experiments have shown however that the non-linear LLT converges in roughly the same amount of iterations
 * whatever the initial state, even if random or asymmetric.
 * @param Alpha the angle of attack, in degrees
 * @return true if a linear solution has been set, false otherwise. Should always be true, unless the user has defined some crazy wing configuration. Who knows what a user can do ?
 */
bool LLTTask::setLinearSolution(double Alpha)
{
    xfl::trace("LLTTask::setLinearSolution\n");

    std::vector<double> aij(s_NLLTStations*s_NLLTStations, 0);
    std::vector<double> rhs(s_NLLTStations+1, 0);

    Foil *pFoil0=nullptr, *pFoil1=nullptr;

    int size = s_NLLTStations-1;
    double dn  = double(s_NLLTStations);
    double di(0), dj(0), t0(0), st0(0), snt0(0), ch(0), a0(0), slope(0), tau(0), yob(0), twist(0);
    double cs = m_pWing->rootChord();
    double b  = m_pWing->planformSpan();
    traceStdLog("Initializing linear solution\n\n");

    for (int i=1; i<s_NLLTStations; i++)
    {
        di  = double(i);
        t0  = di * PI/dn;
        yob = cos(t0);
        ch = m_pWing->getChord(yob);      //or m_Chord[i], same
        twist = m_pWing->getTwist(yob);   //or m_Twist[i], same

        st0 = sin(t0);

        for (int j=1; j<s_NLLTStations; j++)
        {
            dj   = double(j);
            snt0 = sin(dj*t0);

            int p = (i-1)*size + (j-1);
            aij[p] = snt0 + ch*PI/b/2.0* dj*snt0/st0;
        }

        m_pWing->getFoils(&pFoil0, &pFoil1, yob*b/2.0, tau);
        a0 = Objects2d::getZeroLiftAngle(pFoil0, pFoil1, m_Re[i], tau);
        rhs[i] = ch/cs * (Alpha-a0+twist)/180.0*PI;
    }

    bool bCancel = false;
    if (!matrix::Gauss(aij.data(), s_NLLTStations-1, rhs.data()+1, 1, bCancel))
    {
        return false;
    }

    for (int i=1; i<s_NLLTStations; i++)
    {
        di  = double(i);
        t0  = di * PI/dn;
        yob = cos(t0);

        m_Cl[i] = 0.0;
        for (int j=1; j<s_NLLTStations; j++)
        {
            dj = double(j);
            snt0 = sin(dj*t0);
            m_Cl[i] += rhs[j]* snt0;
        }
        m_pWing->getFoils(&pFoil0, &pFoil1, yob*b/2.0, tau);
        Objects2d::getLinearizedPolar(pFoil0, pFoil1, m_Re[i], tau, a0, slope);
        a0 = Objects2d::getZeroLiftAngle(pFoil0, pFoil1, m_Re[i], tau); //better approximation ?

        m_Cl[i] *= slope*cs/m_pWing->getChord(yob);
        m_Ai[i]  = -(Alpha-a0+m_pWing->getTwist(yob)) + m_Cl[i]/slope*180.0/PI;
    }
    xfl::trace("LLTTask::setLinearSolution - done\n");

    return true;
}


double LLTTask::alphaInduced(int k) const
{
    double ai = 0.0;
    for (int m=1; m<s_NLLTStations; m++)
    {
        double b =  Beta(m,k);
        ai += b * m_Cl.at(m) * m_Chord.at(m)/m_pWing->planformSpan();
    }
    return ai;
}


int LLTTask::iterate(double &QInf, double Alpha)
{
    Foil* pFoil0(nullptr);
    Foil* pFoil1(nullptr);

    bool bOutRe(false), bError(false);
    double tau (0);

    double maxa(0);

    int iter = 0;
    while(iter<s_IterLim)
    {
        maxa = 0.0;

        for (int k=1; k<s_NLLTStations; k++)
        {
            double a        = m_Ai[k];
            double anext    = -alphaInduced(k);
            m_Ai[k]  = a +(anext-a)/s_RelaxMax;
            maxa   = qMax(maxa, qAbs(a-anext));
        }

        double Lift=0.0;// required for Type 2
        for (int k=1; k<s_NLLTStations; k++)
        {
            double yob = cos(double(k)*PI/double(s_NLLTStations));
            m_pWing->getFoils(&pFoil0, &pFoil1, yob*m_pWing->planformSpan()/2.0, tau);
            m_Cl[k] = Objects2d::getPlrPointFromAlpha(Polar::CL, pFoil0, pFoil1, m_Re.at(k), Alpha + m_Ai.at(k)+ m_Twist.at(k), tau, bOutRe, bError);
            if (m_pPlPolar->isFixedLiftPolar())
            {
                Lift += Eta(k) * m_Cl.at(k) * m_Chord.at(k);
            }
        }

        if(m_pPlPolar->isFixedLiftPolar())
        {
            Lift *= m_pWing->aspectRatio() / m_pWing->planformSpan();
            if(Lift<=0.0)  return -1;

            QInf  = m_QInf0 / sqrt(Lift);

            for (int k=1; k<s_NLLTStations; k++)
            {
                m_Re[k] = m_Chord.at(k) * QInf /m_pPlPolar->viscosity();

                double yob = cos(double(k)*PI/double(s_NLLTStations));
                m_pWing->getFoils(&pFoil0, &pFoil1, yob*m_pWing->planformSpan()/2.0, tau);
                m_Cl[k] = Objects2d::getPlrPointFromAlpha(Polar::CL, pFoil0, pFoil1, m_Re.at(k), Alpha + m_Ai.at(k)+ m_Twist.at(k), tau, bOutRe, bError);
            }
        }

        if (maxa<s_CvPrec)
        {
            m_bConverged = true;
            break;
        }

        m_iter.push_back(iter);
        m_Max_a.push_back(maxa);

        iter++;
        if(isCancelled()) return -1;
    }
    return iter;
}


void LLTTask::initializeGeom()
{
    m_bWingOut = false;
    m_bConverged = false;

    if(m_pPlPolar->isFixedLiftPolar()) m_QInf0 = sqrt(2.*m_pPlPolar->mass()* 9.81 /m_pPlPolar->density()/m_pWing->planformArea());
    else                               m_QInf0 = 0.0;

    computeLLTChords(s_NLLTStations, m_Chord.data(), m_Offset.data(), m_Twist.data());

    for (int k=0; k<=s_NLLTStations; k++)
    {
        //  y   = cos(k*PI/s_NLLTStations)* m_pWing->m_PlanformSpan/2.0;
        m_SpanPos[k] = m_pWing->planformSpan()/2.0 * cos(double(k)*PI/double(s_NLLTStations));
    }

    for (int j=1; j<s_NLLTStations; j++)
    {
        double yjp = m_SpanPos[j-1];
        double yjm = m_SpanPos[j+1];
        double yj  = m_SpanPos[j];

        double dy = (yjp-yj)/2.0 + (yj-yjm)/2.0;

        m_StripArea[j] = m_Chord[j]*dy;//m2
    }
}


void LLTTask::run()
{
    if (!m_pPlPolar->isFixedaoaPolar())
    {
        alphaLoop();
    }
    else
    {
//        QInfLoop();
    }

    m_AnalysisStatus = xfl::FINISHED; // finish the analysis before sending the final condition_variable
    traceStdLog("\nDone processing ranges.\n"); // final notification after flag is set to FINISHED so that sender thread may exit

}


bool LLTTask::alphaLoop()
{
    QString strange;
    double yob(0), tau(0);
    Foil *pFoil0(nullptr), *pFoil1(nullptr);
    bool bOutRe(false), bError(false);

    bool s_bInitCalc = true;

    xfl::trace("LLTTask::alphaloop\n");


    for (int i=0; i<int(m_AoAList.size()); i++)
    {
        m_iter.clear();
        m_Max_a.clear();

        double alpha = m_AoAList.at(i);
        if(isCancelled())
        {
            strange = "Analysis cancelled on user request....\n";
            traceLog(strange);
            break;
        }

        if(s_bInitCalc)
        {
            double V;
            initializeVelocity(alpha, V);
            m_pPlPolar->setVelocity(V);
            setLinearSolution(alpha);
        }
        //initialize first iteration
        for (int k=1; k<s_NLLTStations; k++)
        {
            yob   = cos(k*PI/s_NLLTStations);
            m_pWing->getFoils(&pFoil0, &pFoil1, yob*m_pWing->planformSpan()/2.0, tau);
            m_Cl[k] = Objects2d::getPlrPointFromAlpha(Polar::CL, pFoil0, pFoil1, m_Re[k], alpha + m_Ai[k] + m_Twist[k], tau, bOutRe, bError);
        }


        xfl::trace("LLTTask::alphaloop - 1\n");

        strange = "Calculating " + ALPHAch + QString::asprintf(" = %5.2f", alpha) + DEGch + "...";
        traceLog(strange);

        double QInf = m_pPlPolar->velocity();
        int iter = iterate(QInf, alpha);

        if (iter==-1 && !isCancelled())
        {
            strange= "    ...negative Lift... Aborting\n";
            m_bError = true;
            s_bInitCalc = true;
            traceLog(strange);
        }
        else if (iter<s_IterLim && !isCancelled())
        {
            //converged,
            strange= QString::asprintf("    ...converged after %d iterations\n", iter);
            traceOpp(alpha, m_Max_a, strange.toStdString());

            std::string str;
            computeWing(m_pPlPolar->velocity(), alpha, str);// generates wing results,
            traceStdLog(str);
            if (m_bWingOut) m_bWarning = true;
            PlaneOpp *pPOpp = createPlaneOpp(QInf, alpha, m_bWingOut);// Adds WOpp point and adds result to polar


            xfl::trace("LLTTask::alphaloop -2 \n");

            // store the results
            if(pPOpp)
            {
                if(!pPOpp->isOut()) // discard failed visc interpolated opps
                    m_pPlPolar->addPlaneOpPointData(pPOpp);

                if(m_bKeepOpps)
                {
                    m_PlaneOppList.push_back(pPOpp);
                }
                else
                {
                    delete pPOpp;
                    pPOpp = nullptr; // don't leave a pointer dangling
                }
            }
            s_bInitCalc = false;
        }
        else
        {
            if (m_bWingOut) m_bWarning = true;
            m_bError = true;
            strange= QString::asprintf("    ...unconverged after %d iterations out of %d\n", iter, s_IterLim);
            traceOpp(alpha, m_Max_a, strange.toStdString());
            s_bInitCalc = true;
        }

        strange = ALPHAch + QString::asprintf("=%g", alpha) + DEGch;

        xfl::trace("LLTTask::alphaloop - 3\n");

    }

    return true;
}


void LLTTask::setObjects(PlaneXfl *pPlane, PlanePolar *pWPolar)
{
    m_pPlane   = pPlane;
    m_pWing    = pPlane->mainWing();
    m_pPlPolar  = pWPolar;
}


void LLTTask::initializeAnalysis()
{
    m_bWarning = m_bError = false;
    m_PlaneOppList.clear();

    traceStdLog("\nLaunching the LLT Analysis....\n");

    initializeGeom();
}


void LLTTask::clearPOppList()
{
    for(int ip=int(m_PlaneOppList.size()-1); ip>=0; ip--)
    {
        delete m_PlaneOppList.at(ip);
    }
    m_PlaneOppList.clear();
}


void LLTTask::computeLLTChords(int NStation, double *lltchord, double *lltoffset, double *llttwist)
{
    for (int k=0; k<=NStation; k++)
    {
        double yob = cos(k*PI/NStation);
        double y   = fabs(yob * m_pWing->planformSpan()/2);
        for (int is=0; is<m_pWing->nSections(); is++)
        {
            if(m_pWing->yPosition(is) < y && y <=m_pWing->yPosition(is+1))
            {
                double tau = (y-m_pWing->yPosition(is))/(m_pWing->yPosition(is+1)-m_pWing->yPosition(is));
                lltchord[k]  = m_pWing->chord(is)  + (m_pWing->chord(is+1) -m_pWing->chord(is))  * tau;
                lltoffset[k] = m_pWing->offset(is) + (m_pWing->offset(is+1)-m_pWing->offset(is)) * tau;
                llttwist[k]  = m_pWing->twist(is)  + (m_pWing->twist(is+1) -m_pWing->twist(is))  * tau;;
                break;
            }
        }
    }

//    m_pWing->m_SpanResFF.resizeResults(NStation);
//    m_pWing->m_SpanResSum = m_pWing->m_SpanResFF;
}


PlaneOpp* LLTTask::createPlaneOpp(double QInf, double Alpha, bool bWingOut)
{
    PlaneOpp *pNewPOpp = new PlaneOpp(m_pPlane, m_pPlPolar, 0, 0);

    WingXfl const*pWing = m_pPlane->mainWing();
    pNewPOpp->addWingOpp(0);

    AeroForces AF; // dummy argument, filled a posteriori
    pNewPOpp->m_WingOpp[0].createWOpp(pWing, m_pPlPolar, m_SpanDistribs, AF);

    WingOpp &mainwopp = pNewPOpp->WOpp(0);
    SpanDistribs &maindist = mainwopp.m_SpanDistrib;
    AeroForces &af = pNewPOpp->aeroForces();

    af.setReferenceArea(m_pPlPolar->referenceArea());
    af.setReferenceSpan(m_pPlPolar->referenceSpanLength());
    af.setReferenceChord(m_pPlPolar->referenceChordLength());

    af.setOpp(Alpha, 0.0, 0.0, QInf);

    int nStation = s_NLLTStations-1;
    mainwopp.m_NStation = nStation;

    maindist.resizeGeometry(nStation); // don't want to use the wing's panel-based number of stations
    maindist.resizeResults(nStation);


    if(pNewPOpp == nullptr)
    {
        traceStdLog("Not enough memory to store the OpPoint\n");
        return nullptr;
    }


    double q = 0.5*m_pPlPolar->density() * 1.0 * 1.0;

    pNewPOpp->m_Alpha = Alpha;
    pNewPOpp->m_QInf  = QInf;
    pNewPOpp->m_bGround = false;
    pNewPOpp->m_GroundHeight = 0.0;

    Vector3d FFF; // wind axis
    FFF.x = m_CDi * m_pPlPolar->referenceArea();            // N/q
    FFF.z = m_CL * m_pPlPolar->referenceArea();             // N/q
    // store in body axis
    double cosa = cos(Alpha*PI/180.0);
    double sina = sin(Alpha*PI/180.0);
    af.setFff({FFF.x*cosa-FFF.z*sina, 0.0, FFF.x*sina+FFF.z*cosa});

    af.setProfileDrag(m_CDv* m_pPlPolar->referenceArea());

    Vector3d Mi;
    Mi.x = m_GRm * m_pPlPolar->referenceChordLength() * m_pPlPolar->referenceArea();            // N.m/q
    Mi.y = m_ICm * m_pPlPolar->referenceChordLength() * m_pPlPolar->referenceArea();            // N.m/q
    Mi.z = m_IYm * m_pPlPolar->referenceChordLength() * m_pPlPolar->referenceArea();            // N.m/q
    af.setMi(Mi);

    Vector3d Mv;
    Mv.x = 0.0;
    Mv.y = m_VCm * m_pPlPolar->referenceChordLength() * m_pPlPolar->referenceArea();            // N.m/q
    Mv.z = m_VYm * m_pPlPolar->referenceChordLength() * m_pPlPolar->referenceArea();            // N.m/q
    af.setMv(Mv);

//    af.setCP(m_CP);

    pNewPOpp->m_bOut  = bWingOut;

    mainwopp.m_bOut = m_bWingOut;
    mainwopp.m_AF = af;

    Vector3d N;
    double Cb =0.0;
    for (int l=0; l<nStation; l++)
    {
        int ll = nStation-l;
        maindist.m_StripPos[l]       = -m_SpanPos.at(ll);
        maindist.m_StripArea[l]     =  m_StripArea.at(ll);
        maindist.m_Ai[l]            =  m_Ai.at(ll);
        maindist.m_Cl[l]            =  m_Cl.at(ll);
        maindist.m_PCd[l]           =  m_PCd.at(ll);
        maindist.m_ICd[l]           =  m_ICd.at(ll);
        maindist.m_CmPressure[l]    =  m_Cm.at(ll);
        maindist.m_CmViscous[l]     =  m_CmAirf.at(ll);
        maindist.m_XCPSpanRel[l]    =  m_XCPSpanRel.at(ll);
        maindist.m_XCPSpanAbs[l]    =  m_XCPSpanAbs.at(ll);
        maindist.m_Re[l]            =  m_Re.at(ll);
        maindist.m_Chord[l]         =  m_Chord.at(ll);
        maindist.m_Twist[l]         =  m_Twist.at(ll);
        maindist.m_XTrTop[l]        =  m_XTrTop.at(ll);
        maindist.m_XTrBot[l]        =  m_XTrBot.at(ll);
        maindist.m_BendingMoment[l] =  m_BendingMoment.at(ll);
        maindist.m_F[l].set( 0,0,q*pWing->projectedArea()*m_Cl.at(ll));
        maindist.m_Vd[l].set(0,0,QInf*tan(m_Ai[ll]*PI/180.0));

        m_pWing->surfacePoint(0.25, maindist.m_StripPos[l], xfl::MIDSURFACE, maindist.m_PtC4[l], N);

        if(fabs(m_BendingMoment[l])>fabs(Cb)) Cb = m_BendingMoment[l];
    }
    mainwopp.m_MaxBending = Cb;


    //add the data to the polar object
    if(!pNewPOpp->m_bOut)
        m_pPlPolar->addPlaneOpPointData(pNewPOpp);

    return pNewPOpp;
}


void LLTTask::traceOpp(double alpha, std::vector<double>const &max_a, std::string const &msg)
{
    LLTOppReport oppreport(alpha, max_a, msg);

    // Access the Q under the lock:
    std::unique_lock<std::mutex> lck(m_mtx);
    m_theOppQueue.push(oppreport);
    m_cv.notify_all();
}


void LLTTask::traceLog(QString const &str) {traceStdLog(str.toStdString());}


void LLTTask::traceStdLog(std::string const &str)
{
    LLTOppReport oppreport(0, std::vector<double>(), str); // only interested in the message

    // Access the Q under the lock:
    std::unique_lock<std::mutex> lck(m_mtx);
    m_theOppQueue.push(oppreport);
    m_cv.notify_all();
}
