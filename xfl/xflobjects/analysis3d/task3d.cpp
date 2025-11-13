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

#include "task3d.h"
#include <QApplication>
#include <QFutureSynchronizer>
#include <QtConcurrent/QtConcurrent>

#include <xflcore/flow5events.h>
#include <xflobjects/objects3d/analysis/polar3d.h>
#include <xflobjects/analysis3d/panelanalysis.h>
#include <xflobjects/analysis3d/p4analysis.h>
#include <xflobjects/analysis3d/p3unianalysis.h>
#include <xflobjects/analysis3d/p3linanalysis.h>
/*
double tmp_dt=0.0;
double const *tmp_Mu=nullptr;
double const *tmp_Sigma=nullptr;
double tmp_vortonwakelength=0.0;
Vector3d tmp_VInf;
*/
bool Task3d::s_bCancel = false;

int Task3d::s_MaxNRHS = 100;


bool Task3d::s_bVortonStretch = true;
bool Task3d::s_bVortonRedist = true;

bool Task3d::s_bLiveUpdate = false;


Task3d::Task3d()
{
    m_pEventDestination  = nullptr;

    m_pPolar3d = nullptr;
    m_pPA      = nullptr;
    m_pP3A     = nullptr;
    m_pP4A     = nullptr;

    m_bError = m_bWarning = false;
    m_bStopVPWIterations = false;

    m_qRHS = -1;
    m_nRHS = 0;

    m_AnalysisStatus = xfl::PENDING;
}


Task3d::~Task3d()
{
    if(m_pPA) delete m_pPA;
}

/** receives and redirects from the panel or llt analyses */
void Task3d::onTraceLog(QString const &str) const
{
    emit outputMessage(str);
}


void Task3d::onCancel()
{
    onTraceLog("Cancelling the panel analysis\n");
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

    onTraceLog("Analyzing\n");

    loop();

    if(m_AnalysisStatus!=xfl::CANCELLED) m_AnalysisStatus = xfl::FINISHED;

    emit taskFinished();
//    thread()->exit(0); // exit event loop so that finished() is emitted
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
        tmp_Mu    = m_pP4A->m_Mu.constData()    + qrhs*m_pP4A->nPanels();
        tmp_Sigma = m_pP4A->m_Sigma.constData() + qrhs*m_pP4A->nPanels();
    }
    else if(m_pP3A)
    {
        tmp_Mu    = m_pP3A->m_Mu.constData()    + qrhs*3*m_pP3A->nPanels();
        tmp_Sigma = m_pP3A->m_Sigma.constData() + qrhs  *m_pP3A->nPanels();
    }
    else return;

    // update positions and vorticities
    // duplicate the existing vortons which will be replaced all at once at the end of the procedure
    QVector<QVector<Vorton>> newvortons = m_pPA->m_Vorton;
    double dl = m_pPolar3d->vortonL0() * m_pPolar3d->referenceChordLength(); //m
    tmp_dt = dl/QInf;
    tmp_VInf = windDirection(alpha, beta)*QInf;
    tmp_vortonwakelength = m_pPolar3d->VPWMaxLength()*m_pPolar3d->referenceChordLength();

/*    double domx=0, domy=0, domz=0;
    double G[9];
    memset(G, 0, 9*sizeof(double));
    Vector3d omega;*/

    if(PanelAnalysis::s_bMultiThread)
    {
        QFutureSynchronizer<void> futureSync;

        for(int irow=0; irow<newvortons.size(); irow++)
        {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            futureSync.addFuture(QtConcurrent::run(this, &Task3d::advectVortonRow, &newvortons[irow]));
#else
            futureSync.addFuture(QtConcurrent::run(&Task3d::advectVortonRow, this, &newvortons[irow]));
#endif
        }
        futureSync.waitForFinished();
    }
    else
    {
        for(int irow=0; irow<newvortons.size(); irow++)
        {
            advectVortonRow(&newvortons[irow]);
        }
    }

    // save the new vortons
    m_pPA->m_Vorton = newvortons;

//    qDebug("Vorton advect %2d elapsed: %9.3f s", m_pPA->m_Vorton.size(), double(t.elapsed())/1000.0);
}


void Task3d::advectVortonRow(QVector<Vorton> *thisrow)
{
    Vector3d VT1, VT2, trans, P1;

    for(int iv=0; iv<thisrow->size(); iv++)
    {
        // convect-translate the vorton
        Vorton &vtn = (*thisrow)[iv];
        if(vtn.isActive())
        {
            Vector3d const &P0 = vtn.position();

            //RK2
            if(m_pPA)  m_pPA->getVelocityVector(P0, tmp_Mu, tmp_Sigma, VT1, Vortex::coreRadius(), false, false);
            trans.set((tmp_VInf + VT1)*tmp_dt);
            P1.set(P0 + trans*tmp_dt/2.0);

            if(m_pPA)  m_pPA->getVelocityVector(P1, tmp_Mu, tmp_Sigma, VT2, Vortex::coreRadius(), false, false);
            trans.set((tmp_VInf+VT2)*tmp_dt);
            vtn.translate(trans);

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



