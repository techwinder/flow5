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


#include <QCoreApplication>

#include "streamlinemaker.h"



#include <api/p3linanalysis.h>
#include <api/p3unianalysis.h>
#include <api/p4analysis.h>
#include <api/vector3d.h>
#include <api/boatopp.h>
#include <api/planeopp.h>
#include <api/polar3d.h>
#include <api/planexfl.h>
#include <api/boat.h>
#include <api/panel3.h>
#include <api/panel4.h>
#include <api/vortex.h>

bool StreamlineMaker::s_bCancel=false;
/*
int StreamlineMaker::m_NX=10;
double StreamlineMaker::m_L0=0.1;
double StreamlineMaker::m_XFactor=1.01;
*/

StreamlineMaker::StreamlineMaker(QObject *pParent) : QRunnable()
{
    m_pParent = pParent;

    m_Mu    = nullptr;
    m_Sigma = nullptr;

    m_QInf = m_Alpha = m_Beta = 0;
    m_NX = 50;
    m_L0 = 0.1;
    m_XFactor = 1.0;

    m_pP4Analysis    = nullptr;
    m_pP3Analysis = nullptr;
    m_pP3Analysis = nullptr;
    m_Index = 0;
    m_pStreamVertexArray = nullptr;
}


StreamlineMaker::~StreamlineMaker()
{
}


void StreamlineMaker::initializeLineMaker(int index, float *pStreamVertexArray, Vector3d const &C0, Vector3d const &VA, Vector3d const &TC,
                                          int NX, double L0, double XFactor)
{
    m_Index = index;
    m_pStreamVertexArray = pStreamVertexArray;
    m_C0 = C0;
    m_UnitDir0 = VA;
    m_TC = TC;
    m_NX = NX;
    m_L0 = L0;
    m_XFactor = XFactor;
}


void StreamlineMaker::setOpp(Polar3d const*pPolar3d, double QInf, double alpha, double beta, double const*mu, double const*sigma)
{
    m_pPolar3d=pPolar3d;
    m_QInf  = QInf;
    m_Alpha = alpha;
    m_Beta  = beta;
    m_Mu    = mu;
    m_Sigma = sigma;
}


void StreamlineMaker::run()
{
    int iVtx = 0; // make a new variable to prevent multithread overwriting
    Vector3d C;
    Vector3d VT, Vel;

    Vector3d winddir, VInf;
    winddir = objects::windDirection(0.0, m_Beta);// alpha=because the wake panels are rotated by aoa
    VInf = winddir * m_QInf;

    // calculate total wake length and compare it to max. length, i.e. the wake length
    double l0 = m_L0;
/*    double series=0.0, r=1.0;
    for(int p=0; p<m_NX; p++)
    {
        series +=r;
        r*=m_XFactor;
    }
    double length = series*l0;
    if(length>=m_pPolar3d->wakeLength())
    {
        //adjust first segment length
        l0 = m_pPolar3d->wakeLength()/series;
    }*/

    // plot the left trailing point only for the extreme left trailing panel
    // and only right trailing points afterwards
    C.x = m_C0.x;
    C.y = m_C0.y;
    C.z = m_C0.z;


    // make the starting point
    m_pStreamVertexArray[iVtx++] = C.xf() + m_TC.xf();
    m_pStreamVertexArray[iVtx++] = C.yf() + m_TC.yf();
    m_pStreamVertexArray[iVtx++] = C.zf() + m_TC.zf();


    //make the first streamline point from the specified trailing edge velocity
    double ds = l0;
    double XXS = l0;
    VT = m_UnitDir0 * l0;
    C += VT;

    m_C0 = C;


    Vector3d C0;
    // continue the streamline
    for (int i=1; i<m_NX+1; i++)
    {
        int iter = 0;
        do
        {
            C0 = C;

            if(m_pP4Analysis)
                m_pP4Analysis->getVelocityVector(C,  m_Mu, m_Sigma, Vel, Vortex::coreRadius(), false, true);
            else if(m_pP3Analysis)
                m_pP3Analysis->getVelocityVector(C,  m_Mu, m_Sigma, Vel, Vortex::coreRadius(), false, true);


            if(Vel.norm()/VInf.norm()>0.5)
            {
                // if the radial velocity is excessive, the point is probably
                // close to a wake line, so reduce the increment
                VT = Vel + VInf;
                C += VT.normalized()/10.0*ds;
            }
            else
            {
                VT = Vel + VInf;
                C += VT.normalized() * ds;
            }
            iter++;
            if(s_bCancel) break;
        } while((C.x-m_C0.x)<XXS && iter<20);

        // adjust exactly to XXS
        if(C.x-C0.x>0.0)
        {
            Vector3d U = (C-C0).normalized();
            C = C0 + U*(m_C0.x+XXS-C0.x);
        }

        m_pStreamVertexArray[iVtx++] = C.xf() + m_TC.xf();
        m_pStreamVertexArray[iVtx++] = C.yf() + m_TC.yf();
        m_pStreamVertexArray[iVtx++] = C.zf() + m_TC.zf();

//qDebug("  %3d  %13.9f  %13.9f  %13.9f", i, C.x + m_TC.x, C.y + m_TC.y, C.z + m_TC.z);


        ds *= m_XFactor;
        XXS += ds;
        if(s_bCancel) break;
    }
//    qDebug("  last  %13.9f  %13.9f  %13.9f", C.x + m_TC.x, C.y + m_TC.y, C.z + m_TC.z);

    if(m_pParent)
        qApp->postEvent(static_cast<QObject*>(m_pParent), new StreamEndTaskEvent(m_Index));
}


