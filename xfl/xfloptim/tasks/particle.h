/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xflobjects/objects_globals/optstructures.h>

/**
 * @class Multi-Objective Particle
 * To use in single objective PSO or GA, set NObjectives=1 and NBest=1
 */
class Particle
{
    public:
        Particle();

        int dimension() const {return m_Position.size();}
        int nObjectives() const {return m_Fitness.size();}
        int nBest() const {return m_BestPosition.size();}

        void resetBestError();
        double error(int iobj) const {return m_Error.at(iobj);}
        void setError(int iobj, double err) {m_Error[iobj]=err;}

        double bestError(int iFront, int iobj) const {return m_BestError.at(iFront).at(iobj);}
        void setBestError(int iFront, int iobj, double err) {m_BestError[iFront][iobj]=err;}
        void setBestPosition(int iFront, int idim, double pos) {m_BestPosition[iFront][idim]=pos;}
        void storeBestPosition(int ifront) {m_BestPosition[ifront]=m_Position;}

        bool isSame(const Particle &p) const;

        QVector<double> const &velocity() const {return m_Velocity;}
        QVector<double> const &position() const {return m_Position;}

        void setPos(int i, double dble) {m_Position[i]=dble;}
        void setVel(int i, double dble) {m_Velocity[i]=dble;}

        double pos(int i) const {return m_Position.at(i);}
        double bestPos(int iFront, int iComponent) const {return m_BestPosition.at(iFront).at(iComponent);}
        double vel(int i) const {return m_Velocity.at(i);}

        void setFitness(int i, double f) {m_Fitness[i]=f;}
        double fitness(int i) const {return m_Fitness.at(i);}

        void initializeBest();
        void updateBest();

        void resizeArrays(int dim, int nobj, int nbest);

        bool dominates(Particle const* pOther) const;

        QVector<double> &Cp(int iobj) {return m_OptCp[iobj].m_Cp;}


        bool isConverged()  const {return m_bIsConverged;}
        void setConverged(bool b) {m_bIsConverged=b;}

        void setInParetoFront(bool b) {m_bIsInParetoFront = b;}
        bool isInParetoFront() const {return m_bIsInParetoFront;}

    private:
        // size = dimension = nVariables
        QVector<double> m_Position;
        QVector<double> m_Velocity;

        //size = NObjectives
        QVector<double> m_Fitness; /** the value of each objective function for this particle */
        QVector<double> m_Error;   /** the error associated to each objective */

        //size = PARETOSIZE
        QVector<QVector<double>> m_BestError;    /** the particle's personal best errors achieved so far; size=nObjectives*/
        QVector<QVector<double>> m_BestPosition; /** the particle's personal best positions achieved so far; size=dimension */

        bool m_bIsInParetoFront;

        //XFoil specific
        bool m_bIsConverged;

    public:
        /** @todo the following fields do not belong here, move to a subclass*/
        QVector<OptCp> m_OptCp; /** Only used in the case of Cp optimization*/
        double m_CL=0, m_CDi=0;

};
