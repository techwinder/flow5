/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include "particle.h"
#include <xflmath/constants.h>
#include <cmath>


Particle::Particle()
{
    m_bIsConverged = false;
    m_bIsInParetoFront = false;
    resizeArrays(0, 1, 1);
}


void Particle::resizeArrays(int ndim, int nobj, int nbest)
{
    m_Position.resize(ndim);   m_Position.detach();
    m_Velocity.resize(ndim);   m_Velocity.detach();

    m_Fitness.resize(nobj);    m_Fitness.detach();
    m_OptCp.resize(nobj);      m_OptCp.detach();
    m_Error.resize(nobj);      m_Error.detach();
    m_Error.fill(LARGEVALUE);

    m_BestPosition.resize(nbest);  m_BestPosition.detach();
    m_BestError.resize(nbest);     m_BestError.detach();
    for(int i=0; i<nbest; i++)
    {
        m_BestPosition[i].resize(ndim);  m_BestPosition.detach();
        m_BestPosition[i].fill(0);

        m_BestError[i].resize(nobj);     m_BestError.detach();
        m_BestError[i].fill(LARGEVALUE);
    }
}


void Particle::initializeBest()
{
    for(int i=0; i<m_BestPosition.size(); i++)
    {
        m_BestPosition[i] = m_Position;
        m_BestError[i].fill(LARGEVALUE);
    }
}


/** Stores the current best if it is non-dominated by the existing solutions */
void Particle::updateBest()
{
    for(int i=0; i<m_BestPosition.size(); i++)
    {
        QVector<double> const &olderror = m_BestError.at(i);

        for(int j=0; j<m_Error.size(); j++)
        {
            if(olderror.at(j)>m_Error.at(j))
            {
                // this particle dominates the old personal best, so replace it
                m_BestError[i] = m_Error;
                m_BestPosition[i] = m_Position;
                return;
            }
        }
    }
}


bool Particle::dominates(Particle const* pOther) const
{
    for(int j=0; j<m_Error.size(); j++)
    {
        if(m_Error.at(j) > pOther->m_Error.at(j))
        {
            return false;
        }
    }
    return true;
}


bool Particle::isSame(Particle const &p) const
{
    double err = 0.01; // 1%
    for(int i=0; i<m_Position.size(); i++)
    {
        if(fabs(m_Position.at(i))>1.0e-6)
        {
            if(fabs((p.pos(i)-m_Position.at(i))/m_Position.at(i))>err) return false;
        }
    }
    return true;
}

