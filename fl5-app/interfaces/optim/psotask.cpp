/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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


#include <QtConcurrent/QtConcurrent>

#include <interfaces/optim/psotask.h>
#include <api/constants.h>

int    PSOTask::s_ArchiveSize     = 10;
double PSOTask::s_InertiaWeight   = 0.3;
double PSOTask::s_CognitiveWeight = 0.7;
double PSOTask::s_SocialWeight    = 0.7;
double PSOTask::s_ProbRegenerate  = 0.07;

int  PSOTask::s_PopSize           = 17;
int  PSOTask::s_MaxIter           = 30;
bool PSOTask::s_bMultiThreaded    = false; /** @todo change */


QVector<Vector3d> PSOTask::s_DebugPts;
QVector<Vector3d> PSOTask::s_DebugVecs;


PSOTask::PSOTask()
{
    m_bConverged = false;
    m_Iter = 0;
    m_Status = xfl::PENDING;
}


void PSOTask::checkBounds(Particle &particle) const
{
    for(int i=0; i<particle.dimension(); i++)
    {
        particle.setPos(i, std::max(m_Variable.at(i).m_Min, particle.pos(i)));
        particle.setPos(i, std::min(m_Variable.at(i).m_Max, particle.pos(i)));
    }
}


void PSOTask::listPopulation() const
{
    QString strange;
    for(int i=0; i<m_Swarm.size(); i++)
    {
        Particle const &ind = m_Swarm.at(i);
        strange = QString::asprintf(" part[%d]:", i);

/*        for(int jdim=0; jdim<ind.dimension(); jdim++)
            strange += QString::asprintf("  %9g", ind.pos(jdim));
        strange += " | ";
        for(int jdim=0; jdim<ind.dimension(); jdim++)
            strange += QString::asprintf("  %9g", ind.vel(jdim));
        strange += " | ";
        for(int jobj=0; jobj<ind.nObjectives(); jobj++)
            strange += QString::asprintf("  %9g  %9g", ind.fitness(jobj), ind.error(jobj));*/

        for(int ip=0; ip<ind.nBest(); ip++)
        {
            for(int jdim=0; jdim<ind.dimension(); jdim++)
                strange += QString::asprintf("  %9g", ind.bestPos(ip, jdim));
            for(int jobj=0; jobj<ind.nObjectives(); jobj++)
                strange += QString::asprintf("  %9g  ", ind.bestError(ip, jobj));
            strange += " | ";
        }
        qDebug("%s", strange.toStdString().c_str());
    }
    qDebug();
}


int PSOTask::nActiveVariables() const
{
    int nActive = 0;
    for(uint ivar=0; ivar<m_Variable.size(); ivar++)
    {
        OptVariable const &var = m_Variable.at(ivar);
        if(var.m_Max-var.m_Min>DELTAVAR) nActive++;
    }
    return nActive;
}


void PSOTask::outputMsg(QString const &msg)
{
/*    if(!m_pParent) return;
    MessageEvent * pMsgEvent = new MessageEvent(msg);
    qApp->postEvent(m_pParent, pMsgEvent);*/

    emit  outputMessage(msg);
}


void PSOTask::postOptEndEvent() const
{
    if(!m_pParent) return;
    QEvent *pOptimEvent = new QEvent(OPTIM_END_EVENT);
    qApp->postEvent(m_pParent, pOptimEvent);

}


void PSOTask::postParticleEvent() const
{
    if(!m_pParent) return;
    QEvent *pOptimEvent = new QEvent(OPTIM_PARTICLE_EVENT);
    qApp->postEvent(m_pParent, pOptimEvent);

}

void PSOTask::restoreDefaults()
{
    s_PopSize           = 31;
    s_ArchiveSize       = 10;
    s_MaxIter           = 100;
    s_InertiaWeight     = 0.3;
    s_CognitiveWeight   = 0.7;
    s_SocialWeight      = 0.7;
    s_ProbRegenerate    = 0.05;
}


void PSOTask::onMakeParticleSwarm()
{
    m_Swarm.resize(s_PopSize);

    // no need to multithread, no fitness calculation
    for (int i=0; i<m_Swarm.size(); ++i)
    {
        Particle &particle = m_Swarm[i];
        makeRandomParticle(&particle);
    }

    if(s_bMultiThreaded)
    {
//        QThreadPool::globalInstance()->setMaxThreadCount(4);
         QFutureSynchronizer<void> futureSync;
        for (int isw=0; isw<m_Swarm.size(); ++isw)
        {
           Particle &particle = m_Swarm[isw];
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
           futureSync.addFuture(QtConcurrent::run(QThreadPool::globalInstance(), this, &PSOTask::calcFitness, &particle, false, true));
#else
           futureSync.addFuture(QtConcurrent::run(&PSOTask::calcFitness, this, &particle, false, true));
#endif
        }
        futureSync.waitForFinished();
    }
    else
    {
        for(int i=0; i<m_Swarm.size(); i++)
        {
            calcFitness(&m_Swarm[i], false, true);
            if(isCancelled()) break;
        }
    }

    for(int i=0; i<m_Swarm.size(); i++)
        for(int iobj=0; iobj<m_Objective.size(); iobj++)
            m_Swarm[i].setError(iobj, error(&m_Swarm.at(i), iobj));

    outputMsg(QString::asprintf("Made %d random particles\n", int(m_Swarm.size())));


    QEvent *pEvent = new QEvent(OPTIM_MAKESWARM_EVENT);
    qApp->postEvent(m_pParent, pEvent);

    //    thread()->exit(0); // exit event loop so that finished() is emitted

    // this task may be resumed, so move it back to the main GUI thread
 //   moveToThread(QApplication::instance()->thread());
}


void PSOTask::onStartIterations()
{
/*    if(m_Swarm.size()==0 || m_Swarm.size()!=s_PopSize)
    {
        m_Status = xfl::PENDING;
        outputMsg("Invalid swarm size\n");
        postPSOEvent(0); // notifiy finished
        moveToThread(QApplication::instance()->thread());
        return;
    }

    m_Iter = 0;
    m_Status = xfl::RUNNING;

    outputMsg("Starting swarm iterations\n");
    do
    {
        onIteration();
    }
    while(m_Status==xfl::RUNNING);*/
}


void PSOTask::onIteration()
{
    if(s_bMultiThreaded)
    {
        //m_Swarm.detach(); //kill detach issues altogether
        Particle *particles = new Particle[m_Swarm.size()];
        for(int ip=0; ip<m_Swarm.size(); ip++)   particles[ip] = m_Swarm.at(ip);

        QFutureSynchronizer<void> futureSync;
        for (int isw=0; isw<m_Swarm.size(); ++isw)
        {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            futureSync.addFuture(QtConcurrent::run(this, &PSOTask::moveParticle, particles+isw));
#else
            futureSync.addFuture(QtConcurrent::run(&PSOTask::moveParticle, this, particles+isw));
#endif
        }
        futureSync.waitForFinished();

        for(int ip=0; ip<m_Swarm.size(); ip++)  m_Swarm[ip] = particles[ip];
        delete [] particles;
    }
    else
    {
        for (int isw=0; isw<m_Swarm.size(); ++isw)
        {
            Particle &particle = m_Swarm[isw];
            moveParticle(&particle);
            if(m_Status==xfl::CANCELLED) break;
        }
    }

    m_Iter++;

    makeParetoFrontier();

    // select the best particle defined as the one which minimizes the normalized Manhattan distance to each objective
    int iBest0 = 0;
    double dist2 = LARGEVALUE;

    Particle bestparticle;
    for(int i=0; i<m_Pareto.size(); i++)
    {
        Particle const &particle = m_Pareto.at(i);
        double dp2=0.0;
        m_bConverged = true;
        for(int iobj=0; iobj<m_Objective.size(); iobj++)
        {
            if(fabs(m_Objective.at(iobj).m_MaxError)>1.0e-6)
                dp2 += (particle.error(iobj)/m_Objective.at(iobj).m_MaxError) * (particle.error(iobj)/m_Objective.at(iobj).m_MaxError); // normalized distance
        }

        if(dp2<dist2)
        {
            iBest0 = i;
            dist2 = dp2;
            bestparticle = particle;
        }
    }
    // check if the best particle has converged
    m_bConverged = true;
    for(int io=0; io<bestparticle.nObjectives(); io++)
    {
        if(m_Objective.at(io).m_Type==xfl::EQUALIZE)
        {
            if(bestparticle.error(io)>m_Objective.at(io).m_MaxError)   m_bConverged = false;
        }
        else  if(bestparticle.error(io)>0.0) m_bConverged = false;
    }

    postIterEvent(iBest0);

    if(m_Iter>=s_MaxIter || m_bConverged || m_Status==xfl::CANCELLED)
    {
        if     (m_bConverged)             outputMsg("   ---Converged---\n");
        else if(m_Status==xfl::CANCELLED) outputMsg("The task has been cancelled\n");
        else if(m_Iter>=s_MaxIter)        outputMsg("The maximum number of iterations has been reached\n");

        m_Status = xfl::FINISHED;

        postPSOEvent(iBest0); // tell the GUI that the task is done

        // this task may be resumed, so move it back to the main GUI thread

        moveToThread(qApp->instance()->thread());
    }
    else
    {
        // regenerate particles with random probability if they are not in the Pareto frontier - keep those
        for (int isw=0; isw<m_Swarm.size(); ++isw)
        {
            Particle &particle = m_Swarm[isw];
            double regen = QRandomGenerator::global()->bounded(1.0);
            if (regen<s_ProbRegenerate)
            {
                bool bIsParetoParticle = false;
                for(int ip=0; ip<m_Pareto.size(); ip++)
                {
                    if(m_Pareto.at(ip).isSame(particle))
                    {
                        bIsParetoParticle=true;
                        break;
                    }
                }
                if(!bIsParetoParticle)
                {
                    makeRandomParticle(&particle);
                }
            }
        }
    }
}


void PSOTask::moveParticle(Particle *pParticle) const
{
    double newpos=0, vel=0, deltap=0;
    double r1=0, r2=0;

    int igbest=0, ipbest=0;

    // select a random best in the pareto frontier
    igbest = QRandomGenerator::global()->bounded(m_Pareto.size());
    Particle const &globalbest = m_Pareto.at(igbest);

    // select a random best in the personal bests
    ipbest = QRandomGenerator::global()->bounded(pParticle->nBest());

    // update the velocity
    for(int j=0; j<pParticle->dimension(); j++)
    {
        deltap = m_Variable.at(j).m_Max - m_Variable.at(j).m_Min;
        if(fabs(deltap)>1.0e-6 && fabs(pParticle->vel(j))<1.0e-6)
        {
            // this variable is newly activated, so give it a random velocity
            deltap *= 0.5; // start slowly
            vel = -deltap/2.0 + QRandomGenerator::global()->bounded(deltap);
            pParticle->setVel(j, vel);
        }

        r1 = QRandomGenerator::global()->bounded(1.0);
        r2 = QRandomGenerator::global()->bounded(1.0);

        vel = s_InertiaWeight * pParticle->vel(j) +
              s_CognitiveWeight * r1 * (pParticle->bestPos(ipbest, j) - pParticle->pos(j)) +
              s_SocialWeight    * r2 * (globalbest.pos(j)             - pParticle->pos(j));

        //test if the velocity moves the particle off-bound and if true bounce it in the opposite direction
        newpos = pParticle->pos(j) + vel;
        if(newpos<m_Variable.at(j).m_Min || newpos>m_Variable.at(j).m_Max)
            vel = -vel;

        pParticle->setVel(j, vel);

        // update the position
        pParticle->setPos(j, pParticle->pos(j) + pParticle->vel(j));
    }

    checkBounds(*pParticle);

    if(m_Status==xfl::CANCELLED) return;

    calcFitness(pParticle); // note: do not parallelize in derived class
    for(int i=0; i<m_Objective.size(); i++)  pParticle->setError(i, error(pParticle, i));

    pParticle->updateBest();
}


/** Posted when an iteration has ended */
void PSOTask::postIterEvent(int iBest)
{
    OptimEvent pIterEvent(OPTIM_ITER_EVENT, m_Iter, iBest, m_Pareto[iBest]);
//    qApp->postEvent(m_pParent, pIterEvent);
    emit iterEvent(&pIterEvent);
}


/** Posted when the iteration loop has ended */
void PSOTask::postPSOEvent(int iBest)
{
    if(iBest<0||iBest>=m_Pareto.size()) return;
    OptimEvent *pPSOEvent = new OptimEvent(OPTIM_END_EVENT, m_Iter, iBest, m_Pareto.at(iBest));
    qApp->postEvent(m_pParent, pPSOEvent);

}


#define PARTICLEBESTSIZE 3
void PSOTask::makeRandomParticle(Particle *pParticle) const
{
    int nBest = std::min(int(m_Objective.size()), PARTICLEBESTSIZE);
    pParticle->resizeArrays(int(m_Variable.size()), int(m_Objective.size()), nBest);
    double pos=0, vel=0;
    double deltap = 0.0;
    double deltav = 0.0;

    for(int i=0; i<pParticle->dimension(); i++)
    {
        deltap = m_Variable.at(i).m_Max - m_Variable.at(i).m_Min;
        pos = m_Variable.at(i).m_Min + QRandomGenerator::global()->bounded(deltap);

        pParticle->setPos(i, pos);
        pParticle->initializeBest();

        deltav = deltap/2.0; // start slowly
        vel = -deltav/2.0 + QRandomGenerator::global()->bounded(deltav);
        pParticle->setVel(i, vel);
    }
}


void PSOTask::setNObjectives(int nObj)
{
    if(m_Objective.size()!=nObj)
    {
        m_Objective.resize(nObj);

        int nBest = std::min(int(m_Objective.size()), PARTICLEBESTSIZE);
        for(int i=0; i<m_Swarm.size(); i++)
            m_Swarm[i].resizeArrays(int(m_Variable.size()), int(m_Objective.size()), nBest);
        for(int i=0; i<m_Pareto.size(); i++)
            m_Pareto[i].resizeArrays(int(m_Variable.size()), int(m_Objective.size()), nBest);
    }
}


void PSOTask::updateErrors()
{
    double err=LARGEVALUE;
    for(int j=0; j<m_Swarm.size(); j++)
    {
        Particle &particle = m_Swarm[j];
        for(int iobj=0; iobj<m_Objective.size(); iobj++)
        {
            err = error(&particle, iobj);
            particle.setError(iobj, err);
        }
        particle.initializeBest();
    }

/*    for(int j=0; j<m_Pareto.size(); j++)
    {
        Particle &particle = m_Pareto[j];
        for(int iobj=0; iobj<m_Objective.size(); iobj++)
        {
            err = error(&particle, iobj);
            particle.setError(iobj, err);
        }
//        particle.initializeBest();
    }*/
}


void PSOTask::makeParetoFrontier()
{
    for(int j=0; j<m_Swarm.size(); j++)
    {
        Particle const &pj = m_Swarm.at(j);
        bool bIsDominated=false;
        for(int ip=0; ip<m_Pareto.size(); ip++)
        {
            Particle const &ppar = m_Pareto.at(ip);
            if(ppar.dominates(&pj))
            {
                // this particle does not belong to the Pareto front, discard it
                bIsDominated=true;
                break;
            }
        }

        if(!bIsDominated)
        {
            // this particle is worthy
            // check if it dominates any of the existing particles in the Pareto frontier
            for(int ip=m_Pareto.size()-1; ip>=0; ip--)
            {
                Particle const &ppar = m_Pareto.at(ip);
                if(pj.dominates(&ppar))
                {
                    m_Pareto.removeAt(ip);
                }
            }
            m_Pareto.append(pj);
        }
        if(m_Status==xfl::CANCELLED) break;
    }

    /** @todo target removal to ensure max dispersion of the Pareto frontier */
    if(s_ArchiveSize>1)
    {
        while(m_Pareto.size()>s_ArchiveSize)
        {
            m_Pareto.removeAt(QRandomGenerator::global()->bounded(m_Pareto.size()));
        }
    }
}


double PSOTask::error(const Particle *pParticle, int iObjective) const
{
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



void listOptVariables(QVector<OptVariable> const &variables)
{
    for(int iv=0; iv<variables.size(); iv++)
    {
            OptVariable const &var = variables.at(iv);
            QString strange = QString::fromStdString(var.m_Name).leftJustified(13, ' ') + QString::asprintf("[%11.5f - %11.5f]", var.m_Min, var.m_Max);
            qDebug()<<strange;
    }
}


void PSOTask::updateFitnesses()
{
    if(s_bMultiThreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for (int isw=0; isw<m_Swarm.size(); ++isw)
        {
           Particle &particle = m_Swarm[isw];
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
           futureSync.addFuture(QtConcurrent::run(QThreadPool::globalInstance(), this, &PSOTask::calcFitness, &particle, false, true));
#else
           futureSync.addFuture(QtConcurrent::run(&PSOTask::calcFitness, this, &particle, false, true));
#endif
        }
        futureSync.waitForFinished();
    }
    else
    {
        for(int i=0; i<m_Swarm.size(); i++)
            calcFitness(&m_Swarm[i]);
    }

    for(int i=0; i<m_Swarm.size(); i++)
    {
        for(int iobj=0; iobj<m_Objective.size(); iobj++)
            m_Swarm[i].setError(iobj, error(&m_Swarm.at(i), iobj));
    }
}
