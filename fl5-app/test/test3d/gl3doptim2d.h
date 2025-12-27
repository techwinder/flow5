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

#pragma once

#include <QDialog>
#include <QRadioButton>
#include <QStackedWidget>

#include <api/vector3d.h>

#include <interfaces/opengl/testgl/gl3dsurface.h>
#include <interfaces/optim/particle.h>



class IntEdit;
class FloatEdit;
class PlainTextOutput;

class gl3dOptim2d : public gl3dSurface
{
    Q_OBJECT
    public:
        gl3dOptim2d();
        ~gl3dOptim2d();

        void glRenderView() override;
        void glMake3dObjects() override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();

        double PSO_error(double z) const;
        void resetParticles();
        void readData();

        void bound(double &val) const {val = std::min(m_HalfSide, val);val = std::max(-m_HalfSide, val);}

        //PSO specific
        void moveSwarm();

        //GA specific
        double GA_error(double z) const;
        int leastFit();
        int popSize() const {return m_Swarm.size();}
        void calculateFitness();
        void GA_crossOver();
        void evaluatePopulation();
        void listGAPopulation(QString &log) const;
        void makeNewGen();
        void makeSelection();
        void mutateGaussian();
        void mutatePopulation();
        void selection();

        //Simplex specific
        void moveSimplex();

    private slots:
        void onAlgorithm();
        void onMakeSurface();
        void onTarget();
        void onIteration();

        //PSO specific
        void onMakeSwarm();
        void onResetPSODefaults();
        void onSwarm();

        //GA specific
        void onMakeGAPopulation();
        void onResetGADefaults();
        void onStartGA();

        //Simplex specific
        void onMakeSimplex();
        void onStartSimplex();

    private:       
        //common
        double m_Error;
        double m_BestError;
        int m_Iter;
        int m_iBest;
        Vector2d m_BestPosition; /**< best solution found by any particle in the swarm */
        QVector<Particle> m_Swarm; /**< the swarm or the population in the case of the GA*/

        //Simplex specific
        Vector3d m_S[3];
        bool m_bglResetTriangle;

        //Common
        QRadioButton *m_prbPSO, *m_prbGA, *m_prbSimplex;
        QStackedWidget *m_pswAlgo;
        PlainTextOutput *m_ppt;
        IntEdit *m_piePopSize;
        IntEdit *m_pieUpdateDt;
        FloatEdit *m_pdeMaxError;
        QTimer m_Timer;
        QRadioButton *m_prbMin, *m_prbMax;

        //PSO specific
        FloatEdit *m_pdeInertiaWeight;
        FloatEdit *m_pdeCognitiveWeight;
        FloatEdit *m_pdeSocialWeight;
        FloatEdit *m_pdePropRegenerate;
        QPushButton *m_ppbSwarm;

        // GA specific
        FloatEdit *m_pdeProbXOver, *m_pdeProbMutation, *m_pdeSigmaMutation;
        QPushButton *m_ppbStartGA;

        // Simplex specific
        QPushButton *m_ppbNewSimplex, *m_ppbSimplex;

        QOpenGLBuffer m_vboTriangle;

        static int s_iAlgo;
        static bool s_bMinimum;
        static int s_PopSize;
        static int s_Dt;
        static double s_MaxError;
        static double s_InertiaWeight;
        static double s_CognitiveWeight;
        static double s_SocialWeight;
        static double s_ProbRegenerate;

        static int s_MaxIter;
        static double s_ProbXOver;       /** probability of crossover */
        static double s_ProbMutation;    /** probability of mutation */
        static double s_SigmaMutation;   /** standard deviation of the gaussian mutation */
};

