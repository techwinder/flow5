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

#include <QVBoxLayout>
#include <QRandomGenerator>

#include "gl3doptim2d.h"

#include <xfl/test/test3d/gl3dsurfaceplot.h>
#include <xfl3d/globals/gl_globals.h>
#include <xfl/controls/w3dprefs.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/plaintextoutput.h>
#include <xflwidgets/wt_globals.h>
#include <xflcore/displayoptions.h>


int    gl3dOptim2d::s_iAlgo           = 0;
bool   gl3dOptim2d::s_bMinimum        = true;
int    gl3dOptim2d::s_PopSize         = 29;
int    gl3dOptim2d::s_Dt              = 100; //ms
double gl3dOptim2d::s_MaxError        = 1.e-4;
int    gl3dOptim2d::s_MaxIter         = 100;

//PSO specific
double gl3dOptim2d::s_InertiaWeight   = 0.3;
double gl3dOptim2d::s_CognitiveWeight = 0.7;
double gl3dOptim2d::s_SocialWeight    = 0.7;
double gl3dOptim2d::s_ProbRegenerate  = 0.05;

//GA specific
double gl3dOptim2d::s_ProbXOver       = 0.5;
double gl3dOptim2d::s_ProbMutation    = 0.15;
double gl3dOptim2d::s_SigmaMutation   = 0.5;

gl3dOptim2d::gl3dOptim2d() : gl3dSurface()
{
    setWindowTitle("2d Optimization");

    m_bglResetTriangle = true;

    m_Error = LARGEVALUE;
    m_BestError = LARGEVALUE;
    m_Iter = 0;
    m_iBest = -1;

    setupLayout();
    connect(&m_Timer, SIGNAL(timeout()), SLOT(onIteration()));

    double length = makeTestSurface();
    setReferenceLength(length);
    reset3dScale();

    onMakeGAPopulation();
    onMakeSwarm();
    onMakeSimplex();
}


gl3dOptim2d::~gl3dOptim2d()
{
}


void gl3dOptim2d::setupLayout()
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    palette.setColor(QPalette::Text, DisplayOptions::textColor());

    QColor clr = DisplayOptions::backgroundColor();
    clr.setAlpha(0);
    palette.setColor(QPalette::Window, clr);
    palette.setColor(QPalette::Base, clr);

    QFrame *pFrame = new QFrame(this);
    {
        pFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        pFrame->setCursor(Qt::ArrowCursor);
//        pMainFrame->setAttribute(Qt::WA_NoSystemBackground);

        QVBoxLayout *pMainLayout = new QVBoxLayout;
        {
            QFrame *pAlgoFrame = new QFrame;
            {
//                pAlgoFrame->setAttribute(Qt::WA_NoSystemBackground);

                QHBoxLayout *pAlgoLayout = new QHBoxLayout;
                {
                    QLabel *pLabAlgo = new QLabel("Algorithm:");
//                    pLabAlgo->setAttribute(Qt::WA_NoSystemBackground);
                    m_prbPSO = new QRadioButton("PSO");
//                    m_prbPSO->setAttribute(Qt::WA_NoSystemBackground);
                    m_prbPSO->setToolTip("Particle Swarm Optimization");

                    m_prbGA  = new QRadioButton("GA");
//                    m_prbGA->setAttribute(Qt::WA_NoSystemBackground);
                    m_prbGA->setToolTip("Genetic Algorithm");

                    m_prbSimplex = new QRadioButton("Simplex");
//                    m_prbSimplex->setAttribute(Qt::WA_NoSystemBackground);

                    m_prbPSO->setChecked(    s_iAlgo==0);
                    m_prbGA->setChecked(     s_iAlgo==1);
                    m_prbSimplex->setChecked(s_iAlgo==2);

                    connect(m_prbPSO,     SIGNAL(clicked()), SLOT(onAlgorithm()));
                    connect(m_prbGA,      SIGNAL(clicked()), SLOT(onAlgorithm()));
                    connect(m_prbSimplex, SIGNAL(clicked()), SLOT(onAlgorithm()));

                    pAlgoLayout->addStretch();
                    pAlgoLayout->addWidget(pLabAlgo);
                    pAlgoLayout->addWidget(m_prbPSO);
                    pAlgoLayout->addWidget(m_prbGA);
                    pAlgoLayout->addWidget(m_prbSimplex);
                    pAlgoLayout->addStretch();
                }
                pAlgoFrame->setLayout(pAlgoLayout);
            }

            QGridLayout *pCommonCtrlsLayout = new QGridLayout;
            {
                QLabel *pLabNParticles = new QLabel("Population size:");
//                pLabNParticles->setAttribute(Qt::WA_NoSystemBackground);
                m_piePopSize = new IntEdit(s_PopSize);

                QLabel *pLabMaxError = new QLabel("Max. error:");
//                pLabMaxError->setAttribute(Qt::WA_NoSystemBackground);
                m_pdeMaxError = new FloatEdit(s_MaxError);

                QLabel *pLabUpdate = new QLabel("Update interval:");
//                pLabUpdate->setAttribute(Qt::WA_NoSystemBackground);
                m_pieUpdateDt = new IntEdit(s_Dt);
                QLabel *pLabMilliSecs = new QLabel("ms");
//                pLabMilliSecs->setAttribute(Qt::WA_NoSystemBackground);

                QPushButton *ppbMakeSurface = new QPushButton("Make random surface");
                connect(ppbMakeSurface, SIGNAL(clicked()), SLOT(onMakeSurface()));

                pCommonCtrlsLayout->addWidget(pLabNParticles,       1, 1);
                pCommonCtrlsLayout->addWidget(m_piePopSize,         1, 2);

                pCommonCtrlsLayout->addWidget(pLabMaxError,         2, 1);
                pCommonCtrlsLayout->addWidget(m_pdeMaxError,        2, 2);

                pCommonCtrlsLayout->addWidget(pLabUpdate,           3, 1);
                pCommonCtrlsLayout->addWidget(m_pieUpdateDt,        3, 2);
                pCommonCtrlsLayout->addWidget(pLabMilliSecs,        3, 3);

                pCommonCtrlsLayout->addWidget(ppbMakeSurface,       4,1,1,3);
            }

            QFrame *pTargetFrame = new QFrame;
            {
//                pTargetFrame->setAttribute(Qt::WA_NoSystemBackground);
                QHBoxLayout *pTargetLayout = new QHBoxLayout;
                {
                    QLabel *pLabTarget = new QLabel("Converge on:");
//                    pLabTarget->setAttribute(Qt::WA_NoSystemBackground);
                    m_prbMin = new QRadioButton("Minimum");
//                    m_prbMin->setAttribute(Qt::WA_NoSystemBackground);
                    m_prbMax = new QRadioButton("Maximum");
//                    m_prbMax->setAttribute(Qt::WA_NoSystemBackground);
                    m_prbMin->setChecked(s_bMinimum);
                    m_prbMax->setChecked(!s_bMinimum);
                    connect(m_prbMin, SIGNAL(clicked()), SLOT(onTarget()));
                    connect(m_prbMax, SIGNAL(clicked()), SLOT(onTarget()));
                    pTargetLayout->addWidget(pLabTarget);
                    pTargetLayout->addWidget(m_prbMin);
                    pTargetLayout->addWidget(m_prbMax);
                    pTargetLayout->addStretch();
                }
                pTargetFrame->setLayout(pTargetLayout);
            }

            m_pswAlgo = new QStackedWidget;
            {
//                m_pswAlgo->setAttribute(Qt::WA_NoSystemBackground);
                QGroupBox *pPSOBox = new QGroupBox("Particle Swarm Optimization");
                {
//                    pPSOBox->setPalette(palette);
                    pPSOBox->setFlat(true);
                    pPSOBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
//                    pPSOBox->setAttribute(Qt::WA_NoSystemBackground);

                    QVBoxLayout *pPSOLayout = new QVBoxLayout;
                    {
                        QGridLayout *pInputLayout = new QGridLayout;
                        {
                            QLabel *pLabInertia = new QLabel("Inertia weight w:");
                            m_pdeInertiaWeight = new FloatEdit(s_InertiaWeight);
                            m_pdeInertiaWeight->setToolTip("<p>The inertia weight determines the influence of the particle's "
                                                           "current velocity on its updated velocity.<br>"
                                                           "Recommendation: 0.3</p>");

                            QLabel *pLabCognitive = new QLabel("Cognitive weight c1:");
                            m_pdeCognitiveWeight = new FloatEdit(s_CognitiveWeight);
                            m_pdeCognitiveWeight->setToolTip("<p>The cognitive weight determines the influence of the particle's best position.<br>"
                                                             "Recommendation: 0.7</p>");

                            QLabel *pLabSocial = new QLabel("Social weight c2:");
                            m_pdeSocialWeight = new FloatEdit(s_SocialWeight);
                            m_pdeSocialWeight->setToolTip("<p>The social weight determines the influence of the global best-known position.<br>"
                                                          "Recommendation: 0.7</p<");

                            QLabel *pLabRegen = new QLabel("Regeneration probability:");
                            m_pdePropRegenerate = new FloatEdit(s_ProbRegenerate*100.0);
                            m_pdePropRegenerate->setRange(0.0, 100.0);
                            m_pdePropRegenerate->setToolTip("<p>The probability that a particle will be re-created at a random position at each iteration.<br>"
                                                        "Increases the likelyhood that the swarm will not get stuck on a local minimum.<br>"
                                                        "Recommendation: 5% to 25%</p>");
                            QLabel *pLabPercent = new QLabel("%");

                            pInputLayout->addWidget(pLabInertia,          3, 1);
                            pInputLayout->addWidget(m_pdeInertiaWeight,   3, 2);

                            pInputLayout->addWidget(pLabCognitive,        4, 1);
                            pInputLayout->addWidget(m_pdeCognitiveWeight, 4, 2);

                            pInputLayout->addWidget(pLabSocial,           5, 1);
                            pInputLayout->addWidget(m_pdeSocialWeight,    5, 2);

                            pInputLayout->addWidget(pLabRegen,            6, 1);
                            pInputLayout->addWidget(m_pdePropRegenerate,  6, 2);
                            pInputLayout->addWidget(pLabPercent,          6, 3);

                            pInputLayout->setColumnStretch(1,3);
                            pInputLayout->setColumnStretch(2,2);
                        }

                        QPushButton *ppbResetDefaults = new QPushButton("Reset PSO defaults");
                        connect(ppbResetDefaults, SIGNAL(clicked()), SLOT(onResetPSODefaults()));

                        QPushButton *ppbMakeParticles = new QPushButton("Make random swarm");
                        connect(ppbMakeParticles, SIGNAL(clicked()), SLOT(onMakeSwarm()));

                        m_ppbSwarm = new QPushButton("Swarm");
                        connect(m_ppbSwarm, SIGNAL(clicked()), SLOT(onSwarm()));

                        pPSOLayout->addLayout(pInputLayout);
                        pPSOLayout->addWidget(ppbResetDefaults);
                        pPSOLayout->addWidget(ppbMakeParticles);

                        pPSOLayout->addWidget(m_ppbSwarm);
                    }
                    pPSOBox->setLayout(pPSOLayout);
                }

                QGroupBox *pGABox = new QGroupBox("Genetic Algorithm");
                {
                    QVBoxLayout *pGACtrlsLayout = new QVBoxLayout;
                    {
                        QGridLayout *pInputLayout = new QGridLayout;
                        {
                            QLabel *pLabXOver = new QLabel("Cross-over probability:");
                            m_pdeProbXOver = new FloatEdit(s_ProbXOver*100.0);
                            m_pdeProbXOver->setRange(0.0, 100.0);
                            QLabel *plabPercent0 = new QLabel("%");

                            QLabel *pLabProbMute = new QLabel("Mutation probability:");
                            m_pdeProbMutation = new FloatEdit(s_ProbMutation*100.0);
                            m_pdeProbMutation->setRange(0.0, 100.0);
                            QLabel *plabPercent1 = new QLabel("%");

                            QLabel *pLabSigMute = new QLabel("Mutation standard deviation:");
                            m_pdeSigmaMutation = new FloatEdit(s_SigmaMutation);
                            m_pdeSigmaMutation->setRange(0.0, 1.0); // absolute value

                            pInputLayout->addWidget(pLabXOver,            1, 1);
                            pInputLayout->addWidget(m_pdeProbXOver,       1, 2);
                            pInputLayout->addWidget(plabPercent0,         1, 3);

                            pInputLayout->addWidget(pLabProbMute,         2, 1);
                            pInputLayout->addWidget(m_pdeProbMutation,    2, 2);
                            pInputLayout->addWidget(plabPercent1,         2, 3);

                            pInputLayout->addWidget(pLabSigMute,          3, 1);
                            pInputLayout->addWidget(m_pdeSigmaMutation,   3, 2);

                            pInputLayout->setColumnStretch(1,3);
                            pInputLayout->setColumnStretch(2,2);
                        }

                        QPushButton *ppbResetDefaults = new QPushButton("Reset GA defaults");
                        connect(ppbResetDefaults, SIGNAL(clicked()), SLOT(onResetGADefaults()));

                        QPushButton *ppbMakePop = new QPushButton("Make random population");
                        connect(ppbMakePop, SIGNAL(clicked()), SLOT(onMakeGAPopulation()));


                        m_ppbStartGA = new QPushButton("Start evolution");
                        connect(m_ppbStartGA, SIGNAL(clicked()), SLOT(onStartGA()));
                        pGACtrlsLayout->addLayout(pInputLayout);
                        pGACtrlsLayout->addStretch();
                        pGACtrlsLayout->addWidget(ppbResetDefaults);
                        pGACtrlsLayout->addWidget(ppbMakePop);
                        pGACtrlsLayout->addWidget(m_ppbStartGA);
                    }
                    pGABox->setLayout(pGACtrlsLayout);
                }

                QGroupBox *pSimplexBox = new QGroupBox("Simplex");
                {
                    QGridLayout *pSimplexLayout = new QGridLayout;
                    {
                        m_ppbNewSimplex = new QPushButton("Make random simplex");
                        m_ppbSimplex = new QPushButton("Start");
                        connect(m_ppbNewSimplex, SIGNAL(clicked()), SLOT(onMakeSimplex()));
                        connect(m_ppbSimplex,    SIGNAL(clicked()), SLOT(onStartSimplex()));
                        pSimplexLayout->addWidget(m_ppbNewSimplex, 2,1);
                        pSimplexLayout->addWidget(m_ppbSimplex, 3,1);
                        pSimplexLayout->setRowStretch(1,1);
                    }
                    pSimplexBox->setLayout(pSimplexLayout);
                }

                m_pswAlgo->addWidget(pPSOBox);
                m_pswAlgo->addWidget(pGABox);
                m_pswAlgo->addWidget(pSimplexBox);
                if      (s_iAlgo==0) m_pswAlgo->setCurrentWidget(pPSOBox);
                else if (s_iAlgo==1) m_pswAlgo->setCurrentWidget(pGABox);
                else if (s_iAlgo==2) m_pswAlgo->setCurrentWidget(pSimplexBox);
            }

            m_ppt = new PlainTextOutput;
            m_ppt->setCharDimensions(35, 15);

            pMainLayout->addWidget(pAlgoFrame);
            pMainLayout->addLayout(pCommonCtrlsLayout);
            pMainLayout->addWidget(pTargetFrame);
            pMainLayout->addWidget(m_pswAlgo);
            pMainLayout->addWidget(m_ppt);
        }


        pFrame->setLayout(pMainLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        setWidgetStyle(pFrame, palette);
    }
}


void gl3dOptim2d::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dOptim2d");
    {
        s_iAlgo           = settings.value("Algo",            s_iAlgo).toInt();
        s_bMinimum        = settings.value("Minimum",         s_bMinimum).toBool();
        s_Dt              = settings.value("Dt",              s_Dt).toInt();
        s_PopSize         = settings.value("PopSize",         s_PopSize).toInt();
        s_MaxError        = settings.value("MaxError",        s_MaxError).toDouble();

        s_InertiaWeight   = settings.value("InertiaWeight",   s_InertiaWeight).toDouble();
        s_CognitiveWeight = settings.value("CognitiveWeight", s_CognitiveWeight).toDouble();
        s_SocialWeight    = settings.value("SocialWeight",    s_SocialWeight).toDouble();
        s_ProbRegenerate  = settings.value("PropRegenerate",  s_ProbRegenerate).toDouble();

        s_ProbXOver       = settings.value("CrossOver",       s_ProbXOver).toDouble();
        s_ProbMutation    = settings.value("ProbMutation",    s_ProbMutation).toDouble();
        s_SigmaMutation   = settings.value("SigMutation",     s_SigmaMutation).toDouble();
    }
    settings.endGroup();
}


void gl3dOptim2d::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dOptim2d");
    {
        settings.setValue("Algo",            s_iAlgo);
        settings.setValue("Minimum",         s_bMinimum);
        settings.setValue("Dt",              s_Dt);
        settings.setValue("PopSize",         s_PopSize);
        settings.setValue("MaxError",        s_MaxError);

        settings.setValue("InertiaWeight",   s_InertiaWeight);
        settings.setValue("CognitiveWeight", s_CognitiveWeight);
        settings.setValue("SocialWeight",    s_SocialWeight);
        settings.setValue("PropRegenerate",  s_ProbRegenerate);

        settings.setValue("CrossOver",       s_ProbXOver);
        settings.setValue("ProbMutation",    s_ProbMutation);
        settings.setValue("SigMutation",     s_SigmaMutation);
    }
    settings.endGroup();
}


void gl3dOptim2d::glRenderView()
{
    if(m_bDisplaySurface)
    {
        paintColourMap(m_vboSurface);
        if(m_bGrid) paintGrid();

        if(m_bContour)
            paintSegments(m_vboContourLines, W3dPrefs::s_ContourLineStyle);
    }

    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    if(s_iAlgo<2)
    {
        for(int i=0; i<m_Swarm.size(); i++)
        {
            Particle const &p = m_Swarm.at(i);
            if(p.dimension()==2) // watch out for async repaints
            {
                if(i==m_iBest)
                {
                    paintSphere(p.pos(0), p.pos(1), p.fitness(0), 0.015/m_glScalef, Qt::red, true);
                }
                else
                {
                    paintSphere(p.pos(0), p.pos(1), p.fitness(0), 0.011/m_glScalef, Qt::darkYellow, true);
                }
            }
        }
    }
    if(s_iAlgo==2)
    {
        QMatrix4x4 vmMat, pvmMat;
        vmMat = m_matView*m_matModel;
        pvmMat = m_matProj*vmMat;
        m_shadSurf.bind();
        {
            m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
            m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
        }
        m_shadLine.bind();
        {
            m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*vmMat);
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
            m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(Line::SOLID));
            m_shadLine.setUniformValue(m_locLine.m_Thickness, 2.0f);
        }
        m_shadLine.release();

        paintTriangle(m_vboTriangle, false, true, QColor(205, 75, 155, 155));
        for(int i=0; i<3; i++)
            paintSphere(m_S[i], 0.011/m_glScalef, Qt::darkYellow, true);
    }

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dOptim2d::glMake3dObjects()
{
    if(m_bResetSurface)
    {
        glMakeSurface();
        QVector<double> nodevalues(m_PointArray.size());
        for(int i=0; i<m_PointArray.size(); i++) nodevalues[i] = m_PointArray.at(i).z;
        gl::makeQuadContoursOnGrid( m_vboContourLines, m_Size_x, m_Size_y, m_PointArray, nodevalues, true);
    }

    if(m_bglResetTriangle)
    {
        gl::makeTriangle(m_S, m_vboTriangle);
    }

    m_bResetSurface = false;
    m_bglResetTriangle = false;
}


void gl3dOptim2d::onResetPSODefaults()
{
    s_Dt              = 100;
    s_PopSize         = 29;
    s_MaxError        = 1.e-4;
    s_InertiaWeight   = 0.3;
    s_CognitiveWeight = 0.7;
    s_SocialWeight    = 0.7;
    s_ProbRegenerate  = 0.05;

    m_pieUpdateDt->setValue(s_Dt);
    m_piePopSize->setValue(s_PopSize);
    m_pdeMaxError->setValue(s_MaxError);
    m_pdeInertiaWeight->setValue(s_InertiaWeight);
    m_pdeCognitiveWeight->setValue(s_CognitiveWeight);
    m_pdeSocialWeight->setValue(s_SocialWeight);
    m_pdePropRegenerate->setValue(s_ProbRegenerate*100.0);
}


void gl3dOptim2d::readData()
{
    s_CognitiveWeight = m_pdeCognitiveWeight->value();
    s_Dt              = m_pieUpdateDt->value();
    s_InertiaWeight   = m_pdeInertiaWeight->value();
    s_MaxError        = m_pdeMaxError->value();
    s_PopSize         = m_piePopSize->value();
    s_ProbMutation    = m_pdeProbMutation->value()/100.0;
    s_ProbXOver       = m_pdeProbXOver->value()/100.0;
    s_SigmaMutation   = m_pdeSigmaMutation->value();
    s_SocialWeight    = m_pdeSocialWeight->value();
    s_ProbRegenerate  = m_pdePropRegenerate->value()/100.0;
    s_bMinimum        = m_prbMin->isChecked();
}


void gl3dOptim2d::onAlgorithm()
{
    if     (m_prbPSO->isChecked())     s_iAlgo=0;
    else if(m_prbGA->isChecked())      s_iAlgo=1;
    else if(m_prbSimplex->isChecked()) s_iAlgo=2;

    m_pswAlgo->setCurrentIndex(s_iAlgo);

    resetParticles();

    update();
}


void gl3dOptim2d::onTarget()
{
    bool bCurrent = s_bMinimum;
    s_bMinimum = m_prbMin->isChecked();


    if(bCurrent!=s_bMinimum)
    {
        readData();
        if(s_PopSize!=m_Swarm.size()) onMakeSwarm();

        // the target has changed
        resetParticles();

        m_Timer.stop(); // stop and restart immediately
        if     (s_iAlgo==0)
        {
            onSwarm();
        }
        else if(s_iAlgo==1) onStartGA();
        else if(s_iAlgo==2) onStartSimplex();
    }
}


// James McCaffrey 11/25/2013 https://visualstudiomagazine.com/Articles/2013/11/01/Particle-Swarm-Optimization.aspx?Page=1
void gl3dOptim2d::onSwarm()
{
    readData();
    if(m_Swarm.size()==0)
    {
        m_ppt->onAppendThisPlainText("Swarm has not been created\n");
        return;
    }

    if(m_Timer.isActive())
    {
        m_Timer.stop();
        m_ppbSwarm->setText("Swarm");
        return;
    }
    else
        m_ppbSwarm->setText("Stop");

    m_Iter = 0;
    m_iBest = -1;
    m_Error = LARGEVALUE;

    m_Timer.start(s_Dt);

    update();
}


void gl3dOptim2d::onMakeSurface()
{
    readData();
    m_bDoubleDipSurface = false;

    bool bWasActive = false;
    if(m_Timer.isActive())
    {
        bWasActive=true;
        m_Timer.stop();
    }
    c0 = QRandomGenerator::global()->bounded(1.5)-0.75;
    c1 = QRandomGenerator::global()->bounded(2.0)-1.5;
    c2 = QRandomGenerator::global()->bounded(2.0)-0.7;
//    qDebug()<<c0<<c1<<c2;
    makeTestSurface();

    resetParticles();

    update();
    m_iBest = -1;
    m_Error = LARGEVALUE;

    if(bWasActive) m_Timer.start(s_Dt);
}


void gl3dOptim2d::resetParticles()
{
    if(s_iAlgo==0)
    {
        m_BestError = LARGEVALUE;
        for (int i=0; i<m_Swarm.size(); ++i)
        {
            Particle &particle = m_Swarm[i];
            particle.setFitness(0, function(particle.pos(0), particle.pos(1)));
            double err = PSO_error(particle.fitness(0));
            particle.setError(0, err);
            particle.setBestError(0, 0, LARGEVALUE);
        }
    }
    else if(s_iAlgo==1)
    {
        for (int i=0; i<popSize(); ++i)
        {
            Particle &particle = m_Swarm[i];
            double fit = function(particle.pos(0), particle.pos(1));
            particle.setFitness(0, fit);
//            particle.setError(fit);
        }
    }
    else if(s_iAlgo==2)
    {
        for (int i=0; i<3; ++i)  m_S[i].z = function(m_S[i].x, m_S[i].y);
        m_bglResetTriangle = true;
    }

    // the stored best positions have become irrelevant, so renew them to set the swarm going
    m_iBest = -1;
    m_Error = LARGEVALUE;
    m_BestError = LARGEVALUE;
    m_BestPosition.x = QRandomGenerator::global()->bounded(2.0*m_HalfSide)-m_HalfSide;
    m_BestPosition.y = QRandomGenerator::global()->bounded(2.0*m_HalfSide)-m_HalfSide;
    for(int i=0; i<m_Swarm.size(); i++)
    {
        m_Swarm[i].setBestError(0, 0, LARGEVALUE);
        double xp = QRandomGenerator::global()->bounded(2.0*m_HalfSide)-m_HalfSide;
        double yp = QRandomGenerator::global()->bounded(2.0*m_HalfSide)-m_HalfSide;
        m_Swarm[i].setBestPosition(0,0,xp);
        m_Swarm[i].setBestPosition(0,1,yp);
    }
}


void gl3dOptim2d::onMakeSwarm()
{
    readData();
    bool bWasActive = false;
    if(m_Timer.isActive())
    {
        bWasActive=true;
        m_Timer.stop();
    }

    m_Swarm.resize(s_PopSize);
    double xp=0, yp=0, xv=0, yv=0;
    double err=LARGEVALUE;
    double maxerror=LARGEVALUE;

    m_iBest = -1;
    double const velamp = m_HalfSide;
    for (int i=0; i<m_Swarm.size(); i++)
    {
        Particle &particle = m_Swarm[i];
        particle.resizeArrays(2,1,1);
        xp = QRandomGenerator::global()->bounded(2.0*m_HalfSide)-m_HalfSide;
        yp = QRandomGenerator::global()->bounded(2.0*m_HalfSide)-m_HalfSide;
        particle.setPos(0, xp);
        particle.setPos(1, yp);
        particle.setFitness(0, function(xp, yp));

        xv = QRandomGenerator::global()->bounded(velamp)-velamp/2.0;
        yv = QRandomGenerator::global()->bounded(velamp)-velamp/2.0;
        particle.setVel(0, xv);
        particle.setVel(1, yv);

        err = PSO_error(particle.fitness(0));
        particle.setError(0, err);
        if(err<maxerror)
        {
            maxerror = err;
            m_iBest = i;
            m_BestPosition.x = particle.pos(0);
            m_BestPosition.y = particle.pos(1);
        }
    }
    update();

    if(bWasActive) m_Timer.start(s_Dt);
}


void gl3dOptim2d::onIteration()
{
    m_Iter++;
    if(s_iAlgo==0)
    {
        if(m_Swarm.size()==0)
        {
            m_ppt->onAppendThisPlainText("Swarm has not been created\n");
            m_Timer.stop();
            return;
        }
        moveSwarm();
    }
    else if(s_iAlgo==1)
    {
        if(popSize()==0)
        {
            m_ppt->onAppendThisPlainText("Population has not been created\n");
            m_Timer.stop();
            return;
        }
        makeNewGen();
    }
    else if(s_iAlgo==2)
    {
        moveSimplex();
    }


    if(s_iAlgo==2)
    {
        m_BestPosition.x = (m_S[0].x+m_S[1].x+m_S[2].x)/3.0;
        m_BestPosition.y = (m_S[0].y+m_S[1].y+m_S[2].y)/3.0;
    }
    m_Error = error(m_BestPosition, s_bMinimum);

    m_ppt->onAppendThisPlainText(QString::asprintf("It.%d: err=%7.3g\n", m_Iter, m_Error));

    update();

    if(m_Error<s_MaxError)
    {
        m_Timer.stop();

        m_ppbSwarm->setText("Swarm");
        m_ppbStartGA->setText("Start evolution");
        m_ppbSimplex->setText("Start");
        m_ppt->onAppendThisPlainText(QString::asprintf("\nConverged in %d iterations\n", m_Iter));
        if(s_iAlgo<2) m_ppt->onAppendThisPlainText(QString::asprintf("The winner is particle %d\n", m_iBest));
        m_ppt->onAppendThisPlainText(QString::asprintf("x=%7g y=%7g\n", m_BestPosition.x, m_BestPosition.y));
        m_ppt->onAppendThisPlainText(QString::asprintf("Residual error = %g\n\n", m_Error));
    }
}


void gl3dOptim2d::moveSwarm()
{
    double w = s_InertiaWeight;    // inertia weight. see http://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=00870279
    double c1 = s_CognitiveWeight; // cognitive/local weight
    double c2 = s_SocialWeight;    // social/global weight
    double r1=0, r2=0;             // cognitive and social randomizations

    double vel=0, pos=0;
    double newpos=0, newerror=0;

    m_Error=LARGEVALUE;
    for (int isw=0; isw<m_Swarm.size(); ++isw)
    {
        Particle &particle = m_Swarm[isw];

        for(int j=0; j<particle.dimension(); j++)
        {
            r1 = QRandomGenerator::global()->bounded(1.0);
            r2 = QRandomGenerator::global()->bounded(1.0);

            vel = (w * particle.vel(j)) +
                  (c1 * r1 * (particle.bestPos(0, j) - particle.pos(j))) +
                  (c2 * r2 * (m_BestPosition[j]      - particle.pos(j)));
            //test if the velocity moves the particle off-bound and if true bounce it in the opposite direction
            newpos = particle.pos(j) + vel;
            if(newpos<-m_HalfSide || newpos>m_HalfSide)
                vel = -vel;

            particle.setVel(j, vel);

            // new position
            pos = particle.pos(j) + vel;
            if      (pos<-m_HalfSide) pos = -m_HalfSide;
            else if (pos> m_HalfSide) pos =  m_HalfSide;

            particle.setPos(j, pos);
        }
        particle.setFitness(0, function(particle.pos(0), particle.pos(1)));
        newerror = PSO_error(particle.fitness(0));
        particle.setError(0, newerror);

        if (newerror<particle.bestError(0, 0))
        {
            particle.storeBestPosition(0);
            particle.setBestError(0, 0, newerror);
        }

        if (QRandomGenerator::global()->bounded(1.0)<s_ProbRegenerate)
        {
            // new position, leave velocity, update error
            // tw: any reason to leave velocity?
            for (int j=0; j<particle.dimension(); j++)
                particle.setPos(j, m_HalfSide* (QRandomGenerator::global()->bounded(2.0)-1.0));
            particle.setFitness(0, function(particle.pos(0), particle.pos(1)));
            newerror = PSO_error(particle.fitness(0));
            particle.setError(0, newerror);
            particle.storeBestPosition(0);
            particle.setBestError(0, 0, particle.error(0));

        }

        if(newerror<m_Error)
        {
            m_BestPosition[0] = particle.pos(0);
            m_BestPosition[1] = particle.pos(1);
            m_Error = newerror;
            m_iBest = isw;
        }
    }
}


void gl3dOptim2d::onResetGADefaults()
{
    s_Dt              = 100;
    s_MaxError        = 1.e-4;
    s_ProbXOver       = 0.5;
    s_ProbMutation    = 0.15;
    s_SigmaMutation   = 0.5;
    s_PopSize         = 29;
    m_piePopSize->setValue(s_PopSize);
    m_pdeProbXOver->setValue(s_ProbXOver*100.0);
    m_pdeProbMutation->setValue(s_ProbMutation*100.0);
    m_pdeSigmaMutation->setValue(s_SigmaMutation);
    m_pieUpdateDt->setValue(s_Dt);
    m_pdeMaxError->setValue(s_MaxError);
}


void gl3dOptim2d::onMakeGAPopulation()
{
    readData();

    bool bWasActive = m_Timer.isActive();
    if(bWasActive) m_Timer.stop();

    m_Swarm.resize(s_PopSize);

    m_BestError = s_bMinimum ? LARGEVALUE : -LARGEVALUE;
    for(int j=0; j<m_Swarm.size(); j++)
    {
        Particle &particle = m_Swarm[j];
        particle.resizeArrays(2,1,1);

        particle.setPos(0, -m_HalfSide+QRandomGenerator::global()->bounded(2.0*m_HalfSide));
        particle.setPos(1, -m_HalfSide+QRandomGenerator::global()->bounded(2.0*m_HalfSide));
        particle.setFitness(0, function(particle.pos(0), particle.pos(1)));
        particle.setError(0, particle.fitness(0));
        if(particle.error(0)>m_BestError)
        {
            m_BestPosition.x = particle.pos(0);
            m_BestPosition.y = particle.pos(1);
            m_iBest = j;
        }
    }

    if(bWasActive) m_Timer.start(s_Dt);
    update();
}


void gl3dOptim2d::onStartGA()
{
    readData();

    if(m_Swarm.isEmpty())
    {
        m_ppt->onAppendThisPlainText("Population has not been created\n");
        return;
    }

    if(m_Timer.isActive())
    {
        m_Timer.stop();
        m_ppbStartGA->setText("Start evolution");
        return;
    }
    else
        m_ppbStartGA->setText("Stop");


    m_ppt->clear();

    evaluatePopulation();
//    selectBest();

    m_Iter = 0;
    m_Timer.start(s_Dt);

    QString log;
    log = QString::asprintf("Final: x=%7g y=%7g err=%7.3g\n\n", m_BestPosition.x, m_BestPosition.y, m_Error);
//    listPopulation(log);
    m_ppt->onAppendThisPlainText(log);
    update();
}


void gl3dOptim2d::makeNewGen()
{
    makeSelection();
    GA_crossOver();
    mutateGaussian();
    evaluatePopulation();

    update();
}


void gl3dOptim2d::listGAPopulation(QString &log) const
{
    log += "\nindiv.      x            y            z\n";
    for(int i=0; i<m_Swarm.size(); i++)
    {
        Particle const &particle = m_Swarm.at(i);
        log += QString::asprintf("i%d   %11g  %11g  %11g\n", i, particle.pos(0), particle.pos(1), particle.error(0));
    }
    log += "\n";
}


/** BLX-alpha crossover*/
void gl3dOptim2d::GA_crossOver()
{
    double const alpha = 0.5;
    double frac=0;
    QVector<Particle> oldpop(m_Swarm);
    m_Swarm.clear();
    Particle parent[2], children[2];

    while (oldpop.size()>=2)
    {
        // extract two parents
        int ifirst = QRandomGenerator::global()->bounded(oldpop.size());
        parent[0] = oldpop.takeAt(ifirst);

        int isecond = QRandomGenerator::global()->bounded(oldpop.size());
        parent[1] = oldpop.takeAt(isecond);

        children[0].resizeArrays(parent[0].dimension(), parent[0].nObjectives(), parent[0].nBest());
        children[1].resizeArrays(parent[1].dimension(), parent[1].nObjectives(), parent[1].nBest());

        double prob = QRandomGenerator::global()->bounded(1.0);
        if(prob<s_ProbXOver)
        {
            // create two random children
            for(int iChild=0; iChild<2; iChild++)
            {
                Particle &child = children[iChild];
                for(int i=0; i<child.dimension(); i++)
                {
                    frac = -alpha + QRandomGenerator::global()->bounded(1.0+alpha);
                    child.setPos(i, frac*parent[0].pos(i)+(1.0-frac)*parent[1].pos(i));
                    child.setPos(i, std::max(-m_HalfSide, child.pos(i)));
                    child.setPos(i, std::min( m_HalfSide, child.pos(i)));
                }
                child.setFitness(0, function(child.pos(0), child.pos(1)));
            }
        }
        else
        {
            children[0] = parent[0];
            children[1] = parent[1];
        }
        m_Swarm.append(children[0]);
        m_Swarm.append(children[1]);
    }

    m_Swarm.append(oldpop); // add the remaining single parent if odd population
}


void gl3dOptim2d::evaluatePopulation()
{
    m_iBest = -1;
    double fit=0, maxfit=0;
    for(int i=0; i<popSize(); i++)
    {
        Particle &ind = m_Swarm[i];
        ind.setError(0, function(ind.pos(0), ind.pos(1)));
        fit = GA_error(ind.error(0));
        if(fit>maxfit)
        {
            maxfit = fit;
            m_BestPosition[0] = ind.pos(0);
            m_BestPosition[1] = ind.pos(1);
            m_iBest = i;
        }
    }
}


/** Gaussian mutation */
void gl3dOptim2d::mutateGaussian()
{
    std::default_random_engine generator;
    std::normal_distribution<double> distribution(0.0, s_SigmaMutation);
    for(int i=0; i<popSize(); i++)
    {
        Particle &particle = m_Swarm[i];
        for(int j=0; j<particle.dimension(); j++) // and y components
        {
            double prob = QRandomGenerator::global()->bounded(1.0);
            if(prob<s_ProbMutation)
            {
                double randomvariation = distribution(generator);
                double newgene = particle.pos(j)+ randomvariation;
                newgene = std::max(-m_HalfSide, newgene);
                newgene = std::min( m_HalfSide, newgene);
                particle.setPos(j, newgene);
            }
        }
        particle.setFitness(0, function(particle.pos(0), particle.pos(1)));
    }
}


/** Roulette selection: selects parents with a probability proportional to their fitness */
void gl3dOptim2d::makeSelection()
{
    QVector<double>fit(m_Swarm.size()), cumul(m_Swarm.size());

    for(int i=0; i<m_Swarm.size(); i++)
        fit[i] = GA_error(m_Swarm.at(i).fitness(0));
//        fit[i] = fitness(m_Swarm.at(i).error());  /** @todo ?????????? */

    cumul.first() = fit.first();
    for(int i=1; i<m_Swarm.size(); i++) cumul[i] = cumul.at(i-1) + fit.at(i);

    QVector<Particle> newpop(m_Swarm);
    for(int i=0; i<m_Swarm.size(); i++)
    {
        double p = QRandomGenerator::global()->bounded(cumul.last());
        bool bFound = false;
        for(int j=1; j<m_Swarm.size(); j++)
        {
            if(p<=cumul.at(j))
            {
                newpop[i] = m_Swarm.at(j);
                bFound = true;
                break;
            }
        }
        Q_ASSERT(bFound);
    }

    m_Swarm = newpop;
}


void gl3dOptim2d::onMakeSimplex()
{
    for(int i=0; i<3; i++)
    {
        m_S[i].x = QRandomGenerator::global()->bounded(2.0*m_HalfSide)-m_HalfSide;
        m_S[i].y = QRandomGenerator::global()->bounded(2.0*m_HalfSide)-m_HalfSide;
        m_S[i].z = function(m_S[i].x, m_S[i].y);
    }
    m_bglResetTriangle = true;
    update();
}


void gl3dOptim2d::onStartSimplex()
{
    readData();

    if(m_Timer.isActive())
    {
        m_Timer.stop();
        m_ppbSimplex->setText("Start");
        return;
    }
    else
        m_ppbSimplex->setText("Stop");

    m_Iter = 0;
    m_Error = LARGEVALUE;

//    m_Timer.setSingleShot(true);
    m_Timer.start(s_Dt);

    update();
}


//http://www.scholarpedia.org/article/Nelder-Mead_algorithm
void gl3dOptim2d::moveSimplex()
{
//    clearDebugPoints();
//    for(int i=0; i<3; i++) m_DebugPts.append(m_S[i]);

    double sign = s_bMinimum ? 1.0 : -1.0;

    double alpha=1.0, beta=0.5, gamma=2.0, delta=0.5;
    double xr=0, yr=0, xe=0, ye=0, xc=0, yc=0;
    double fr=0, fe=0, fc=0;
    double fh=0, fs=0, fl=0;
    double f[]{0,0,0};
    for(int i=0; i<3; i++)  f[i] = sign * function(m_S[i].x, m_S[i].y);

    // re-order the vertices in crescending function values
    double func=0;
    Vector3d temp;
    if(f[0]>f[1])
    {
        temp=m_S[1];  m_S[1]=m_S[0];  m_S[0]=temp;
        func=f[1];    f[1]=f[0];      f[0]=func;
    }
    if(f[1]>f[2])
    {
        temp=m_S[2];  m_S[2]=m_S[1];  m_S[1]=temp;
        func=f[2];    f[2]=f[1];      f[1]=func;
    }
    if(f[0]>f[1])
    {
        temp=m_S[1];  m_S[1]=m_S[0];  m_S[0]=temp;
        func=f[1];    f[1]=f[0];      f[0]=func;
    }
    fl = sign * function(m_S[0].x, m_S[0].y); // low
    fs = sign * function(m_S[1].x, m_S[1].y); // second
    fh = sign * function(m_S[2].x, m_S[2].y); // high
    //check
/*    Q_ASSERT(fl<=fs);
    Q_ASSERT(fs<=fh);*/

    // Calc center of the best side
    Vector2d C;
    C.x = (m_S[0].x+m_S[1].x)/2.0;
    C.y = (m_S[0].y+m_S[1].y)/2.0;


    // Transform
    // reflect
    xr = C.x + alpha*(C.x-m_S[2].x);    bound(xr);
    yr = C.y + alpha*(C.y-m_S[2].y);    bound(yr);
    fr = sign * function(xr, yr);
    if(fl<=fr && fr<fs)
    {
        // replace worst
        m_S[2].x = xr;
        m_S[2].y = yr;
        m_S[2].z = function(xr, yr);
        m_bglResetTriangle = true;
        return; // done
    }

    // expand
    if(fr<f[0])
    {
        xe = C.x + gamma*(xr-C.x);    bound(xe);
        ye = C.y + gamma*(yr-C.y);    bound(ye);
        fe = sign * function(xe, ye);
        if(fe<fr)
        {
            m_S[2].x = xe;
            m_S[2].y = ye;
            m_S[2].z = function(xe, ye);
            m_bglResetTriangle = true;
            return; // done
        }
        else
        {
            m_S[2].x = xr;
            m_S[2].y = yr;
            m_S[2].z = function(xr, yr);
            m_bglResetTriangle = true;
            return; // done
        }
    }

    // contract
    if(fr>=fs)
    {
        if(fr<fh)
        {
            //outside
            xc = C.x + beta*(xr-C.x);    bound(xc);
            yc = C.y + beta*(yr-C.y);    bound(yc);
            fc = sign * function(xc, yc);
            if(fc<=fr)
            {
                m_S[2].x = xc;
                m_S[2].y = yc;
                m_S[2].z = function(xc, yc);
                m_bglResetTriangle = true;
                return; // done
            }
        }
        else
        {
            //inside
            xc = C.x + beta*(m_S[2].x-C.x);    bound(xc);
            yc = C.y + beta*(m_S[2].y-C.y);    bound(yc);
            fc = sign * function(xc, yc);
            if(fc<fh)
            {
                m_S[2].x = xc;
                m_S[2].y = yc;
                m_S[2].z = function(xc, yc);
                m_bglResetTriangle = true;
                return; // done
            }
        }
    }

    // shrink
    m_S[1] = m_S[0] + (m_S[1]-m_S[0])*delta;
    m_S[2] = m_S[0] + (m_S[2]-m_S[0])*delta;
    m_S[1].z = function(m_S[1].x, m_S[1].y);
    m_S[2].z = function(m_S[2].x, m_S[2].y);
//    m_ppt->onAppendThisPlainText(QString::asprintf("Iter %3d: Shrink\n", m_Iter));
    m_bglResetTriangle = true;
}


double gl3dOptim2d::GA_error(double z) const
{
    return s_bMinimum ? m_ValMax-z : z-m_ValMin;
}


double gl3dOptim2d::PSO_error(double z) const
{
//    return s_bMinimum ? (z-m_ValMin)*(z-m_ValMin) : (z-m_ValMax)*(z-m_ValMax);
    return s_bMinimum ? (z-m_ValMin) : (m_ValMax-z);
}



