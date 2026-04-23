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

#include <iostream>
#include <thread>

#include <task3d.h>

#include <polar3d.h>
#include <panelanalysis.h>
#include <p4analysis.h>
#include <p3unianalysis.h>
#include <p3linanalysis.h>


bool Task3d::s_bCancel = false;

int Task3d::s_MaxNRHS = 100;


bool Task3d::s_bVortonStretch = true;
bool Task3d::s_bVortonRedist = true;

bool Task3d::s_bLiveUpdate = false;


Task3d::Task3d()
{
    m_pPolar3d = nullptr;
    m_pPA      = nullptr;
    m_pP3A     = nullptr;
    m_pP4A     = nullptr;

    m_bError = m_bWarning = false;
    m_bStopVPWIterations = false;

    m_bKeepOpps = false;
    m_bStdOut   = false;

    m_qRHS = -1;
    m_nRHS = 0;

    m_AnalysisStatus = xfl::PENDING;
}


Task3d::~Task3d()
{
    if(m_pPA) delete m_pPA;
}


void Task3d::traceLog(const QString &str)
{
    traceStdLog(str.toStdString());
}


void Task3d::traceStdLog(std::string const &str)
{
    // notify the parent thread
    VPWReport report;
    report.setMsg(str);
    // Access the Q under the lock:
    std::unique_lock<std::mutex> lck(m_mtx);
    m_theMsgQueue.push(report);
    m_cv.notify_all();

    // output to the terminal
    if(m_bStdOut)
    {
        std::cout << str.c_str();
    }
}


void Task3d::traceVPWLog(double ctrl)
{
    VPWReport report;
    report.m_Ctrl = ctrl;
    report.m_Vortons = m_pPA->m_Vorton;

    // Access the Q under the lock:
    std::unique_lock<std::mutex> lck(m_mtx);
    m_theMsgQueue.push(report);
    m_cv.notify_all();
}


void Task3d::cancelTask()
{
    traceStdLog("Cancelling the panel analysis\n");
    if(m_pPA) m_pPA->cancelAnalysis();
    s_bCancel = true;
    m_AnalysisStatus = xfl::CANCELLED;
}


void Task3d::run()
{
    m_AnalysisStatus = xfl::RUNNING;

    if(s_bCancel || !m_pPolar3d)
    {
        m_AnalysisStatus = xfl::CANCELLED;
        return;
    }

    traceStdLog("Analyzing\n");

    loop();

    if(m_AnalysisStatus!=xfl::CANCELLED) m_AnalysisStatus = xfl::FINISHED;
}


void Task3d::setAnalysisStatus(xfl::enumAnalysisStatus status)
{
    m_AnalysisStatus = status;
    if(m_pPA) m_pPA->setAnalysisStatus(status);
}


void Task3d::advectVortons(double alpha, double beta, double QInf, int qrhs)
{
    if(!m_pPolar3d->bVortonWake()) return;

    if(m_pP4A)
    {
        tmp_Mu    = m_pP4A->m_Mu.data()    + qrhs*m_pP4A->nPanels();
        tmp_Sigma = m_pP4A->m_Sigma.data() + qrhs*m_pP4A->nPanels();
    }
    else if(m_pP3A)
    {
        tmp_Mu    = m_pP3A->m_Mu.data()    + qrhs*3*m_pP3A->nPanels();
        tmp_Sigma = m_pP3A->m_Sigma.data() + qrhs  *m_pP3A->nPanels();
    }
    else return;

    // update positions and vorticities
    // duplicate the existing vortons which will be replaced all at once at the end of the procedure
    std::vector<std::vector<Vorton>> newvortons = m_pPA->m_Vorton;
    double dl = m_pPolar3d->vortonL0() * m_pPolar3d->referenceChordLength(); //m
    tmp_dt = dl/QInf;
    tmp_VInf = objects::windDirection(alpha, beta)*QInf;
    tmp_vortonwakelength = m_pPolar3d->VPWMaxLength()*m_pPolar3d->referenceChordLength();


    if(PanelAnalysis::s_bMultiThread)
    {
        std::vector<std::thread> threads;

        for(uint irow=0; irow<newvortons.size(); irow++)
        {
            threads.push_back(std::thread(&Task3d::advectVortonRow, this, &newvortons[irow]));
        }

        for(uint irow=0; irow<newvortons.size(); irow++)
        {
            threads[irow].join();
        }
//        std::cout << "Task3d::advectVortons joined all " << newvortons.size() << " threads" <<std::endl;

    }
    else
    {
        for(uint irow=0; irow<newvortons.size(); irow++)
        {
            advectVortonRow(&newvortons[irow]);
        }
    }

    // save the new vortons
    m_pPA->m_Vorton = newvortons;

//    qDebug("Vorton advect %2d elapsed: %9.3f s", m_pPA->m_Vorton.size(), double(t.elapsed())/1000.0);
}


void Task3d::advectVortonRow(std::vector<Vorton> *thisrow)
{
    Vector3d VT1, VT2, translation, P1;

    for(uint iv=0; iv<thisrow->size(); iv++)
    {
        // convect-translate the vorton
        Vorton &vtn = (*thisrow)[iv];
        if(vtn.isActive())
        {
            Vector3d const &P0 = vtn.position();

            //RK2
            if(m_pPA)  m_pPA->getVelocityVector(P0, tmp_Mu, tmp_Sigma, VT1, Vortex::coreRadius(), false, false);
            translation.set((tmp_VInf + VT1)*tmp_dt);
            P1.set(P0 + translation*tmp_dt/2.0);

            if(m_pPA)  m_pPA->getVelocityVector(P1, tmp_Mu, tmp_Sigma, VT2, Vortex::coreRadius(), false, false);
            translation.set((tmp_VInf+VT2)*tmp_dt);
            vtn.translate(translation);

            if(vtn.position().norm()>tmp_vortonwakelength)
                vtn.setActive(false);

/*                if(s_bVortonStretch)
            {
                //update the vorton's vorticity
                // Youjiang Wang eq.13
                // d(omega)/dt = grad(U).omega + nu.Laplace(w)
                // second term of the equation is not present in the Willis paper;
                // use the straightforward Euler
                // include the velocity gradient induced by the bound vortices?
                m_pPA->getVortonVelocityGradient(P0, G);

                omega.set(vtn.vortex());

                domx = (G[0]*omega.x + G[1]*omega.y + G[2]*omega.z)*dt;
                domy = (G[3]*omega.x + G[4]*omega.y + G[5]*omega.z)*dt;
                domz = (G[6]*omega.x + G[7]*omega.y + G[8]*omega.z)*dt;
                omega.x += domx;
                omega.y += domy;
                omega.z += domz;
                vtn.setVortex(omega);
            }*/
        }
    }
}



