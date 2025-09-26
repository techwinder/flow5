
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QFutureSynchronizer>

#include <xfloptim/tasks/psotaskfoil.h>
#include <xflmath/constants.h>

#include <xfoil.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/polar.h>
#include <xflfoil/analysis2d/xfoiltask.h>
#include <xflmath/mathelem.h>


PSOTaskFoil::PSOTaskFoil() : PSOTask()
{
    m_pFoil = nullptr;

    m_iLE = -1;

    m_HHt1    = 1.0;
    m_HHt2    = 1.0;
    m_HHmax   = 0.02;  // unused, replaced by the variable amplitude

    m_ModType = PSOTaskFoil::HH;
}


void PSOTaskFoil::calcFitness(Particle *pParticle, bool, bool) const
{
    Foil tempfoil;
    makeFoil(*pParticle, &tempfoil);

    bool bViscous  = true;
    bool bInitBL = true;

    QString strange;
    bool bConverged(true);

    int nActiveObj=m_Objective.size()/m_NOpt;

    int dim = pParticle->dimension();
    int geomdim = dim-m_NOpt; //-flap angle x NOpt

    for(int iopt=0; iopt<m_NOpt; iopt++)
    {
        double flapangle = pParticle->pos(geomdim+iopt);
        if(fabs(flapangle)>FLAPANGLEPRECISION)
        {
            tempfoil.setTEFlapAngle(flapangle);
            tempfoil.setFlaps();
        }
        else
        {
            tempfoil.setTEFlapAngle(0.0);
            tempfoil.applyBase();
        }

        Polar *pPolar = m_OptPolar[iopt];
        XFoilTask *pTask = new XFoilTask(m_pParent);
        XFoil const &xfoil = pTask->XFoilInstance();
        pTask->setStreamText(&strange);
        pTask->setAoAAnalysis(true);
        pTask->setAlphaRange(pPolar->aoa(), pPolar->aoa(), 1.0); // repurposing T4 aoa


        pTask->initializeTask(&tempfoil, pPolar, bViscous, bInitBL, false);
        pTask->run();

        bConverged = bConverged && xfoil.lvconv;


        for(int k=0; k<nActiveObj; k++)
        {
            int iobj = iopt*nActiveObj + k;
            OptObjective const &obj = m_Objective.at(iobj);
            if      (obj.m_Name=="Cl")     pParticle->setFitness(iobj, xfoil.cl);
            else if (obj.m_Name=="Cd")     pParticle->setFitness(iobj, xfoil.cd);
            else if (obj.m_Name=="Cl/Cd")  pParticle->setFitness(iobj, xfoil.cl/xfoil.cd);
            else if (obj.m_Name=="Cp_min") pParticle->setFitness(iobj, xfoil.cpmn);
            else if (obj.m_Name=="Cm")     pParticle->setFitness(iobj, xfoil.cm);
            else if (obj.m_Name=="Cm0")
            {
    //            task->initializeTask(&tempfoil, m_pPolar, bViscous, bInitBL, false);
                pTask->run();
                bConverged = bConverged && xfoil.lvconv;
                pParticle->setFitness(iobj, xfoil.cm); // ??
            }
        }
        delete pTask;
    }

    pParticle->setConverged(bConverged);

}


double PSOTaskFoil::error(Particle const *pParticle, int iObjective) const
{
    if(!pParticle->isConverged()) return LARGEVALUE;

    switch (m_Objective.at(iObjective).m_Type)
    {
        case xfl::MINIMIZE:
        {
            if(pParticle->fitness(iObjective) <= m_Objective.at(iObjective).m_Target)
                return 0;
            else
                return fabs(pParticle->fitness(iObjective)-m_Objective.at(iObjective).m_Target);
        }
        case xfl::MAXIMIZE:
            if(pParticle->fitness(iObjective) >= m_Objective.at(iObjective).m_Target)
                return 0;
            else
                return fabs(pParticle->fitness(iObjective)-m_Objective.at(iObjective).m_Target);
        default:
        case xfl::EQUALIZE:
            return fabs(pParticle->fitness(iObjective)-m_Objective.at(iObjective).m_Target);
    }
}


void PSOTaskFoil::makeFoil(Particle const &particle, Foil *pFoil) const
{
    pFoil->copy(m_pFoil, false);

    int nActiveObj=m_Objective.size()/m_NOpt;
    int dim = particle.dimension();
    int geomdim = dim-nActiveObj; //-flap angle x NOpt

    switch (m_ModType)
    {
        case PSOTaskFoil::HH:
        {
            double t1(0), hh(0), x(0);

            int halfdim = geomdim/2;

            for(int i=0; i<m_iLE; i++)
            {
                x = pFoil->xb(i);
                for(int j=0; j<halfdim; j++)
                {
                    t1 = m_HHt1*double(j+1)/double(2*halfdim+1);
                    hh = HicksHenne(x, t1, m_HHt2, 0.0, 1.0) * particle.pos(j);
                    pFoil->setBasePoint(i, pFoil->basePoint(i) + pFoil->normal(i) * hh);
                }
            }

            for(int i=m_iLE+1; i<pFoil->nBaseNodes(); i++)
            {
                x = pFoil->x(i);
                for(int j=0; j<halfdim; j++)
                {
                    t1 = m_HHt1*double(j+1)/double(2*halfdim+1);
                    hh = HicksHenne(x, t1, m_HHt2, 0.0, 1.0) * particle.pos(halfdim+j);
                    pFoil->setBasePoint(i, pFoil->basePoint(i) + pFoil->normal(i) * hh);
                }
            }
            break;
        }
        case PSOTaskFoil::SCALE:
        {
            double thickness  = particle.pos(0);
            double camber     = particle.pos(1);
            double Xthickness = particle.pos(2);
            double Xcamber    = particle.pos(3);

            Xcamber    = pFoil->m_BaseCbLine.front().x + (pFoil->m_BaseCbLine.back().x-pFoil->m_BaseCbLine.front().x) * Xcamber;
            Xthickness = pFoil->m_BaseCbLine.front().x + (pFoil->m_BaseCbLine.back().x-pFoil->m_BaseCbLine.front().x) * Xthickness;

            pFoil->setThickness(Xthickness, thickness);
            pFoil->setCamber(Xcamber, camber);
            pFoil->rebuildPointSequenceFromBase();
            pFoil->makeBaseMidLine();

            break;
        }
    }

    pFoil->initGeometry();
}

