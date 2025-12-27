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

#pragma once

#include <QObject>



#include <api/flow5events.h>
#include <api/vector3d.h>
#include <api/optstructures.h>
#include <interfaces/optim/particle.h>
#include <api/utils.h>



#define DELTAVAR 0.0001     // minimum difference between varmax and varmin


class OptimEvent : public QEvent
{
    public:
        OptimEvent(QEvent::Type type, int iter, int ibest, Particle const &p): QEvent(type)
        {
            m_Iter  = iter;
            m_iBest = ibest;
            m_Particle = p;
        }

        Particle const &particle() const {return m_Particle;}
        int iter() const {return m_Iter;}
        int iBest() const {return m_iBest;}

    private:
        Particle m_Particle;
        int m_Iter=0;
        int m_iBest = 0;
};


class PSOTask : public QObject
{
    Q_OBJECT

//    friend class  Optim2d;
//    friend class  Optim3d;

    public:
        PSOTask();
        void setParent(QObject *pParent) {m_pParent=pParent;}

        void setDimension(int n) {m_Variable.resize(n);}
        int dimension() const {return int(m_Variable.size());}

        QVector<Particle> const &theSwarm() const {return m_Swarm;}
        int swarmSize() const {return m_Swarm.size();}
        Particle const &swarmAt(int i) {return m_Swarm.at(i);}

        int nVariables() const {return int(m_Variable.size());}
        void setVariables(std::vector<OptVariable> const &var) {m_Variable=var;}
        void setVariable(int iDim, const OptVariable&var) {m_Variable[iDim] = var;}
        OptVariable const &variable(int iDim) const {return m_Variable.at(iDim);}
        int nActiveVariables() const;

        Particle const &particle(int ip) const {return m_Swarm.at(ip);}

        void listPopulation() const;

        bool isConverged() const {return m_bConverged;}


        void setAnalysisStatus(xfl::enumAnalysisStatus status) {m_Status=status;}
        void cancelAnalyis() {m_Status = xfl::CANCELLED;}
        bool isCancelled() const {return m_Status==xfl::CANCELLED;}
        bool isRunning()   const {return m_Status==xfl::RUNNING;}
        bool isPending()   const {return m_Status==xfl::PENDING;}
        bool isFinished()  const {return m_Status==xfl::FINISHED || m_Status==xfl::CANCELLED;}

        void restartIterations() {m_Iter=0;}
        virtual void calcFitness(Particle *pParticle, bool bLong=false, bool bTrace=false) const = 0;

        static void setMultithreaded(bool b) {s_bMultiThreaded=b;}

        void makeParetoFrontier();

        void setNObjectives(int nObj);
        int nObjectives() const {return m_Objective.size();}
        OptObjective & objective(int iobj) {return m_Objective[iobj];}
        OptObjective const & objectiveAt(int iobj) const {return m_Objective.at(iobj);}
        void setObjective(int iobj, const OptObjective &obj) {m_Objective[iobj] = obj;}
        void updateErrors();

        void updateFitnesses();

        QVector<Particle> const &thePareto() const {return m_Pareto;}
        int paretoSize() const {return m_Pareto.size();}
        Particle const &pareto(int i) const {return m_Pareto.at(i);}
        void clearPareto() {m_Pareto.clear();}

        static void restoreDefaults();

    protected:
        void postParticleEvent() const;
        void postOptEndEvent() const;
        void checkBounds(Particle &particle) const;
        void outputMsg(QString const &msg);

    private:
        virtual void makeRandomParticle(Particle *pParticle) const;
        void moveParticle(Particle *pParticle) const;

        void postIterEvent(int iBest);
        void postPSOEvent(int iBest);

        virtual double error(Particle const *pParticle, int iObjective) const;

    public slots:
        void onMakeParticleSwarm();
        void onStartIterations();

    private slots:
        void onIteration(); // in case the iteration is triggered by a timer

    signals:
        void iterEvent(OptimEvent*);
        void outputMessage(QString const&);

    protected:
        QVector<Particle> m_Pareto; /**< the particles which make the Pareto front */


        // size = nObjectives
        QVector<OptObjective> m_Objective;


    protected:
        QVector<Particle> m_Swarm; // the swarm

        bool m_bConverged;

        int m_Iter;
        QObject *m_pParent;
        xfl::enumAnalysisStatus m_Status;

        // size = dim
        std::vector<OptVariable> m_Variable;


    public:
        static int  s_PopSize;
        static int  s_MaxIter;
        static bool s_bMultiThreaded;
        static int    s_ArchiveSize;
        static double s_InertiaWeight;
        static double s_CognitiveWeight;
        static double s_SocialWeight;
        static double s_ProbRegenerate;

        static QVector<Vector3d> s_DebugPts;
        static QVector<Vector3d> s_DebugVecs;
};


void listOptVariables(const QVector<OptVariable> &variables);
//void listParticle(Particle const &particle, QString &log, QString const &prefix);

