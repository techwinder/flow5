/****************************************************************************

    flow5
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

#include "llttask.h"

#include <QCoreApplication>
#include <QString>


#include <xflobjects/analysis3d/llttask.h>
#include <xflcore/flow5events.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflfoil/objects2d/objects2d.h>
#include <xflobjects/objects3d/analysis/planeopp.h>
#include <xflobjects/objects3d/analysis/wpolar.h>
#include <xflfoil/objects2d/polar.h>
#include <xflobjects/objects3d/wing/wingxfl.h>
#include <xflobjects/objects3d/plane/planexfl.h>


int LLTTask::s_IterLim = 100;
int LLTTask::s_NLLTStations = 20;
double LLTTask::s_RelaxMax = 20.0;
double LLTTask::s_CvPrec = 0.01;


LLTTask::LLTTask()
{
    m_pEventDestination = nullptr;

    m_pWing = nullptr;
    m_pWPolar = nullptr;

    resetVariables();
}


void LLTTask::resetVariables()
{
    m_AoAList.clear();
    m_bConverged = false;
    m_bWingOut   = false;
    m_bError = m_bWarning = false;

    m_LengthUnit.clear();
    m_mtoUnit = 0.0;

    m_QInf0 = 0.0;
    m_Maxa  = 0.0;

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

    m_Chord.fill(0);
    m_Offset.fill(0);
    m_Twist.fill(0);
    m_SpanPos.fill(0);
    m_StripArea.fill(0);
    m_Re.fill(0);
    m_Cl.fill(0);
    m_Ai.fill(0);
    m_ICd.fill(0);
    m_PCd.fill(0);
    m_Cm.fill(0);
    m_CmAirf.fill(0);
    m_XCPSpanRel.fill(0);
    m_XCPSpanAbs.fill(0);
    m_BendingMoment.fill(0);
    m_XTrTop.fill(0);
    m_XTrBot.fill(0);
}


bool LLTTask::saveSettings(QSettings &settings)
{
    settings.beginGroup("LLTAnalysis");
    {
        settings.setValue("NLLTStations",  s_NLLTStations);
        settings.setValue("RelaxFactor",   s_RelaxMax);
        settings.setValue("CvPrecision",   s_CvPrec);
        settings.setValue("MaxIterations", s_IterLim);
    }
    settings.endGroup();

    return true;
}


bool LLTTask::loadSettings(QSettings &settings)
{
    settings.beginGroup("LLTAnalysis");
    {
        s_NLLTStations = settings.value("NLLTStations",  s_NLLTStations).toInt();
        s_RelaxMax     = settings.value("RelaxFactor",   s_RelaxMax).toDouble();
        s_CvPrec       = settings.value("CvPrecision",   s_CvPrec).toDouble();
        s_IterLim      = settings.value("MaxIterations", s_IterLim).toInt();
    }
    settings.endGroup();
    return true;
}


void LLTTask::initializeVelocity(double alpha, double &QInf)
{
    Foil *pFoil0(nullptr), *pFoil1(nullptr);
    double tau(0);

    switch (m_pWPolar->type())
    {
        case xfl::T1POLAR:
        {
            QInf = m_pWPolar->m_QInfSpec;
            break;
        }
        case xfl::T2POLAR:
        {
            double Lift=0.0;// required for Type 2

            for (int k=1; k<s_NLLTStations; k++)
            {
                double yob   = cos(k*PI/s_NLLTStations);
                m_pWing->getFoils(&pFoil0, &pFoil1, yob*m_pWing->planformSpan()/2.0, tau);
                double alpha0 = Objects2d::getZeroLiftAngle(pFoil0, pFoil1, 1.0e10, tau);
                double Cl = 2.0*PI*(alpha-alpha0+m_Twist.at(k))*PI/180.0;
                Lift += Eta(k) * Cl * m_Chord.at(k) /m_pWing->planformSpan();
            }
            if(Lift<=0.0) return;
            QInf  = m_QInf0 / sqrt(Lift);
            break;
        }
        default:
        {
            onTraceLog("******* LLT is only compatible with T1 and T2 polars ******\n\n");
            QInf = m_pWPolar->m_QInfSpec;
            break;
        }
    }

    for (int k=1; k<s_NLLTStations; k++)
    {
        m_Re[k] = m_Chord.at(k) * QInf /m_pWPolar->viscosity();
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


void LLTTask::computeWing(double QInf, double Alpha, QString &ErrorMessage)
{
    Foil* pFoil0(nullptr);
    Foil* pFoil1(nullptr);

    QString strange;
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
        c4   = m_pPlane->wingLE(0).x + m_pWing->C4(yob)                                     - m_pWPolar->CoG().x; //m
        zpos = m_pPlane->wingLE(0).z + m_pWing->ZPosition(yob*m_pWing->m_PlanformSpan/2.0)  - m_pWPolar->CoG().z; //m
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
            ErrorMessage = QString::asprintf("       Span pos = %9.2f ", cos(m*PI/s_NLLTStations)*m_pWing->planformSpan()/2.0*m_mtoUnit);
            ErrorMessage += m_LengthUnit;
            ErrorMessage += ",  Re = ";
            QString string;
            string = QString::asprintf("%.0f", m_Re.at(m));
            ErrorMessage += strange;

            strange = QString::asprintf(" ,  A+Ai+Twist = %.2f could not be interpolated\n", Alpha+m_Ai.at(m) + m_Twist.at(m));
            ErrorMessage+=strange;

            m_bWingOut = true;
            m_bConverged = false;
        }
        else if(bPointOutRe)
        {
            ErrorMessage = QString::asprintf("       Span pos = %9.2f ", cos(m*PI/s_NLLTStations)*m_pWing->planformSpan()/2.0*m_mtoUnit);
            ErrorMessage += m_LengthUnit;
            ErrorMessage += ",  Re = ";
            QString string;
            string = QString::asprintf("%.0f", m_Re.at(m));
            ErrorMessage += string;

            strange = QString::asprintf(" ,  A+Ai+Twist = %.2f is outside the flight envelope\n", Alpha+m_Ai.at(m) + m_Twist.at(m));
            ErrorMessage+=strange;

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
}


void LLTTask::setBending(double QInf)
{
    //dynamic pressure, kg/m³
    double q = 0.5*m_pWPolar->density() * QInf * QInf;

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
    QVector<double> aij(s_NLLTStations*s_NLLTStations, 0);
    QVector<double> rhs(s_NLLTStations+1, 0);

    Foil *pFoil0=nullptr, *pFoil1=nullptr;

    int size = s_NLLTStations-1;
    double dn  = double(s_NLLTStations);
    double di(0), dj(0), t0(0), st0(0), snt0(0), ch(0), a0(0), slope(0), tau(0), yob(0), twist(0);
    double cs = m_pWing->rootChord();
    double b  = m_pWing->planformSpan();
    onTraceLog("Initializing linear solution\n\n");

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
    if (!Gauss(aij.data(), s_NLLTStations-1, rhs.data()+1, 1, bCancel))
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

    int iter = 0;
    while(iter<s_IterLim)
    {
        m_Maxa = 0.0;

        for (int k=1; k<s_NLLTStations; k++)
        {
            double a        = m_Ai[k];
            double anext    = -alphaInduced(k);
            m_Ai[k]  = a +(anext-a)/s_RelaxMax;
            m_Maxa   = qMax(m_Maxa, qAbs(a-anext));
        }

        double Lift=0.0;// required for Type 2
        for (int k=1; k<s_NLLTStations; k++)
        {
            double yob = cos(double(k)*PI/double(s_NLLTStations));
            m_pWing->getFoils(&pFoil0, &pFoil1, yob*m_pWing->planformSpan()/2.0, tau);
            m_Cl[k] = Objects2d::getPlrPointFromAlpha(Polar::CL, pFoil0, pFoil1, m_Re.at(k), Alpha + m_Ai.at(k)+ m_Twist.at(k), tau, bOutRe, bError);
            if (m_pWPolar->isFixedLiftPolar())
            {
                Lift += Eta(k) * m_Cl.at(k) * m_Chord.at(k);
            }
        }

        if(m_pWPolar->isFixedLiftPolar())
        {
            Lift *= m_pWing->aspectRatio() / m_pWing->planformSpan();
            if(Lift<=0.0)  return -1;

            QInf  = m_QInf0 / sqrt(Lift);

            for (int k=1; k<s_NLLTStations; k++)
            {
                m_Re[k] = m_Chord.at(k) * QInf /m_pWPolar->viscosity();

                double yob = cos(double(k)*PI/double(s_NLLTStations));
                m_pWing->getFoils(&pFoil0, &pFoil1, yob*m_pWing->planformSpan()/2.0, tau);
                m_Cl[k] = Objects2d::getPlrPointFromAlpha(Polar::CL, pFoil0, pFoil1, m_Re.at(k), Alpha + m_Ai.at(k)+ m_Twist.at(k), tau, bOutRe, bError);
            }
        }

        if (m_Maxa<s_CvPrec)
        {
            m_bConverged = true;
            break;
        }

        //  if(m_pCurve) m_pCurve->appendPoint(iter, m_Maxa);

        m_x.append(double(iter));
        m_y.append(m_Maxa);

        iter++;
        if(isCancelled()) return -1;
    }
    return iter;
}


void LLTTask::initializeGeom()
{
    m_bWingOut = false;
    m_bConverged = false;

    if(m_pWPolar->isFixedLiftPolar()) m_QInf0 = sqrt(2.*m_pWPolar->mass()* 9.81 /m_pWPolar->density()/m_pWing->planformArea());
    else                              m_QInf0 = 0.0;

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
    if (!m_pWPolar->isFixedaoaPolar())
    {
        alphaLoop();
    }
    else
    {
//        QInfLoop();
    }
    emit taskFinished();
}


bool LLTTask::alphaLoop()
{
    QString str;
    double Alpha(0), yob(0), tau(0);
    Foil *pFoil0(nullptr), *pFoil1(nullptr);
    bool bOutRe(false), bError(false);

    bool s_bInitCalc = true;

    for (int i=0; i<m_AoAList.size(); i++)
    {
        m_x.clear();
        m_y.clear();

        Alpha = m_AoAList.at(i);
        if(isCancelled())
        {
            str = "Analysis cancelled on user request....\n";
            onTraceLog(str);
            break;
        }

        if(s_bInitCalc)
        {
            initializeVelocity(Alpha, m_pWPolar->m_QInfSpec);
            setLinearSolution(Alpha);
        }
        //initialize first iteration
        for (int k=1; k<s_NLLTStations; k++)
        {
            yob   = cos(k*PI/s_NLLTStations);
            m_pWing->getFoils(&pFoil0, &pFoil1, yob*m_pWing->planformSpan()/2.0, tau);
            m_Cl[k] = Objects2d::getPlrPointFromAlpha(Polar::CL, pFoil0, pFoil1, m_Re[k], Alpha + m_Ai[k] + m_Twist[k], tau, bOutRe, bError);
        }


        str= "Calculating " + ALPHACHAR + QString::asprintf(" = %5.2f", Alpha) + DEGCHAR + "...";
        onTraceLog(str);

        int iter = iterate(m_pWPolar->m_QInfSpec, Alpha);

        if (iter==-1 && !isCancelled())
        {
            str= QString("    ...negative Lift... Aborting\n");
            m_bError = true;
            s_bInitCalc = true;
            onTraceLog(str);
        }
        else if (iter<s_IterLim && !isCancelled())
        {
            //converged,
            str= QString::asprintf("    ...converged after %d iterations\n", iter);
            onTraceLog(str);
            computeWing(m_pWPolar->m_QInfSpec, Alpha, str);// generates wing results,
            onTraceLog(str);
            if (m_bWingOut) m_bWarning = true;
            PlaneOpp *pPOpp = createPlaneOpp(m_pWPolar->m_QInfSpec, Alpha, m_bWingOut);// Adds WOpp point and adds result to polar
            if(pPOpp) m_PlaneOppList.append(pPOpp);
            s_bInitCalc = false;
        }
        else
        {
            if (m_bWingOut) m_bWarning = true;
            m_bError = true;
            str= QString::asprintf("    ...unconverged after %d iterations out of %d\n", iter, s_IterLim);
            onTraceLog(str);
            s_bInitCalc = true;
        }

        str = ALPHACHAR + QString::asprintf("=%g", Alpha) + DEGCHAR;

        if(m_pEventDestination)
        {
            LLTOppEvent *pEvent = new LLTOppEvent(Alpha, m_x, m_y);
            qApp->postEvent(m_pEventDestination, pEvent);
        }
    }

    return true;
}


void LLTTask::setObjects(PlaneXfl *pPlane, WPolar *pWPolar)
{
    m_pPlane   = pPlane;
    m_pWing    = pPlane->mainWing();
    m_pWPolar  = pWPolar;
}


void LLTTask::initializeAnalysis()
{
    m_bWarning = m_bError = false;
    m_PlaneOppList.clear();

    onTraceLog("\nLaunching the LLT Analysis....\n");

    initializeGeom();
}


void LLTTask::clearPOppList()
{
    for(int ip=m_PlaneOppList.count()-1; ip>=0; ip--)
    {
        delete m_PlaneOppList.at(ip);
        m_PlaneOppList.removeAt(ip);
    }
}


void LLTTask::computeLLTChords(int NStation, double *lltchord, double *lltoffset, double *llttwist)
{
    for (int k=0; k<=NStation; k++)
    {
        double yob = cos(k*PI/NStation);
        double y   = fabs(yob * m_pWing->planformSpan()/2);
        for (int is=0; is<m_pWing->sectionCount(); is++)
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
    PlaneOpp *pNewPOpp = new PlaneOpp(m_pPlane, m_pWPolar, 0, 0);

    WingXfl const*pWing = m_pPlane->mainWing();
    pNewPOpp->addWingOpp(0);

    AeroForces AF; // dummy argument, filled a posteriori
    pNewPOpp->m_WingOpp[0].createWOpp(pWing, m_pWPolar, m_SpanDistribs, AF);

    WingOpp &mainwopp = pNewPOpp->WOpp(0);
    SpanDistribs &maindist = mainwopp.m_SpanDistrib;
    AeroForces &af = pNewPOpp->aeroForces();

    af.setReferenceArea(m_pWPolar->referenceArea());
    af.setReferenceSpan(m_pWPolar->referenceSpanLength());
    af.setReferenceChord(m_pWPolar->referenceChordLength());

    af.setOpp(Alpha, 0.0, 0.0, QInf);

    int nStation = s_NLLTStations-1;
    mainwopp.m_NStation = nStation;

    maindist.resizeGeometry(nStation); // don't want to use the wing's panel-based number of stations
    maindist.resizeResults(nStation);


    if(pNewPOpp == nullptr)
    {
        onTraceLog("Not enough memory to store the OpPoint\n");
        return nullptr;
    }


    double q = 0.5*m_pWPolar->density() * QInf * QInf;

    pNewPOpp->m_Alpha = Alpha;
    pNewPOpp->m_QInf  = QInf;
    pNewPOpp->m_bGround = false;
    pNewPOpp->m_GroundHeight = 0.0;

    Vector3d FFF; // wind axis
    FFF.x = m_CDi * m_pWPolar->referenceArea();            // N/q
    FFF.z = m_CL * m_pWPolar->referenceArea();             // N/q
    // store in body axis
    double cosa = cos(Alpha*PI/180.0);
    double sina = sin(Alpha*PI/180.0);
    af.setFff({FFF.x*cosa-FFF.z*sina, 0.0, FFF.x*sina+FFF.z*cosa});

    af.setProfileDrag(m_CDv* m_pWPolar->referenceArea());

    Vector3d Mi;
    Mi.x = m_GRm * m_pWPolar->referenceChordLength() * m_pWPolar->referenceArea();            // N.m/q
    Mi.y = m_ICm * m_pWPolar->referenceChordLength() * m_pWPolar->referenceArea();            // N.m/q
    Mi.z = m_IYm * m_pWPolar->referenceChordLength() * m_pWPolar->referenceArea();            // N.m/q
    af.setMi(Mi);

    Vector3d Mv;
    Mv.x = 0.0;
    Mv.y = m_VCm * m_pWPolar->referenceChordLength() * m_pWPolar->referenceArea();            // N.m/q
    Mv.z = m_VYm * m_pWPolar->referenceChordLength() * m_pWPolar->referenceArea();            // N.m/q
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
        m_pWPolar->addPlaneOpPointData(pNewPOpp);

    return pNewPOpp;
}



