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

#include <QString>

#include <boattask.h>

#include <boat.h>
#include <boatopp.h>
#include <boatpolar.h>
#include <fuse.h>
#include <p3linanalysis.h>
#include <p3unianalysis.h>
#include <p4analysis.h>
#include <panelanalysis.h>
#include <polar3d.h>
#include <sail.h>
#include <vector3d.h>


BoatTask::BoatTask() : Task3d()
{
    m_pBoat    = nullptr;
    m_pBtPolar = nullptr;
    m_pLiveBtOpp = nullptr;

    m_nRHS = 0;

    m_Ctrl = 0.0;
}


/**
 * Sets the active polar
 * Builds the array of panels depending on the polar type
 * @param bCurrent if true, the active polar is set anew
 * @param WPlrName the name of the polar to set for the active wing or plane
 */
void BoatTask::setObjects(Boat *pBoat, BoatPolar *pBtPolar)
{
    m_pBoat = pBoat;
    m_pBtPolar = pBtPolar;
    m_pPolar3d = pBtPolar;
    m_AF.resetAll();

    if(m_pPolar3d->isQuadMethod())
    {
        m_pP4A = new P4Analysis;
        m_pPA = m_pP4A;
    }
    else if(m_pPolar3d->isTriangleMethod())
    {
        if(m_pPolar3d->isTriUniformMethod())
        {
            m_pP3A = new P3UniAnalysis;
            m_pPA = m_pP3A;
        }
        else if(m_pPolar3d->isTriLinearMethod())
        {
            m_pP3A = new P3LinAnalysis;
            m_pPA = m_pP3A;
        }
    }
}


bool BoatTask::initializeTask(QObject *)
{
    if(!m_pBoat || !m_pBtPolar) return false;

    m_bWarning = false;

    if(m_pPolar3d->isQuadMethod())
    {
        return false; // not activated
    }
    else if(m_pPolar3d->isTriangleMethod())
    {
        initializeTriangleAnalysis();
    }

    allocateSailResultsArrays();

    return true;
}


void BoatTask::setAnalysisRange(std::vector<double> const&opplist)
{
    m_OppList = opplist;
    m_nRHS = int(opplist.size());
}


bool BoatTask::initializeTriangleAnalysis()
{
    for(int is=0; is<m_pBoat->nSails(); is++)
    {
        m_pBoat->sail(is)->updateStations();
    }

    TriMesh &refmesh = m_pBoat->refTriMesh();
    // expensive operation: connect just-in-time only

//    refmesh.makeConnectionsFromNodePosition(false, PanelAnalysis::s_bMultiThread);
    m_pBoat->makeConnections(); // avoids the connection of one sail to another

    refmesh.connectNodes();
    refmesh.makeNodeNormals();

    m_pBoat->triMesh().copyConnections(refmesh); // for later display of information when picking panels on screen
    m_pP3A->setTriMesh(refmesh);

    m_pP3A->initializeAnalysis(m_pPolar3d, 1); // only one point at a time in the case of a boat analysis

//    m_pP3Analysis->makeConnections();
//    std::string strange;
//    strange = QString::asprintf("      Time to make connections: %2f s\n", (double)t.elapsed()/1000);
//    traceStdLog(strange);

    return true;
}


void BoatTask::loop()
{
    QString strange;
    strange = "\n   Solving the problem...\n";
    traceLog(strange);

    std::vector<Vector3d> AWS(m_pPA->nPanels());
    std::vector<Vector3d> VField(m_pPA->nPanels());

    // one operating point at a time
    // linear combinations are not possible due to geometry changes for each control value
//    int nStations = 0;
//    for(int is=0; is<m_pBoat->sailCount(); is++)        nStations += m_pBoat->sail(is)->nStations();

    for (m_qRHS=0; m_qRHS<int(m_OppList.size()); m_qRHS++)
    {
        for(uint i=0; i<m_SailForceFF.size();  i++) m_SailForceFF[i].reset();
        for(uint i=0; i<m_SailForceSum.size(); i++) m_SailForceSum[i].reset();
        for(uint i=0; i<m_HullForce.size();    i++) m_HullForce[i].reset();
        for(uint i=0; i<m_SpanDist.size();     i++) m_SpanDist[i].initializeToZero();

        traceStdLog(EOLstr);
        m_bStopVPWIterations = false;

        m_Ctrl = m_OppList.at(m_qRHS);
        strange = QString::asprintf("    Processing control value= %.3f\n", m_Ctrl);
        traceLog(strange);

        double alpha = 0.0;
        double phi   = m_pBtPolar->phi(m_Ctrl);
        double Ry    = m_pBtPolar->Ry(m_Ctrl);
        double beta  = -m_pBtPolar->AWAInf(m_Ctrl);
        double qinf  = m_pBtPolar->AWSInf(m_Ctrl);

        if(fabs(qinf)<1.0e-3)
        {
            traceStdLog("      Wind speed is 0 - skipping point\n\n");
            m_bError = true;
            continue;
        }

        Vector3d winddir = objects::windDirection(alpha, beta);

        //reset the initial geometry before a new angle is processed
        m_pPA->restorePanels();
        if(m_pBtPolar->bVortonWake())
        {
            m_pPA->clearVortons(); // from the previous operating point calculation
            m_pPA->m_VortexNeg.clear();
        }

        if(m_pP3A)
        {
            m_pBoat->rotateMesh(m_pBtPolar, phi, Ry, m_Ctrl, m_pP3A->m_Panel3);
        }

        std::string OutString;
        if(!m_pPolar3d->isVLM()) m_pPA->makeWakePanels(winddir, m_pBtPolar->bVortonWake());

        traceStdLog(OutString+"\n");

        Vector3d VFree = winddir*qinf;

        if (isCancelled()) return;

        m_pPA->makeInfluenceMatrix();
        if(m_pPA->m_bMatrixError) return;
        if (isCancelled()) return;
#ifdef QT_DEBUG
//display_mat(m_pPA->m_aijd.data(), m_pPA->nPanels());
#endif

        if(!m_pPolar3d->isVLM())
        {
            m_pPA->makeSourceStrengths(VFree);
            //compute wake contribution
            m_pPA->addWakeContribution();
        }
#ifdef QT_DEBUG
//display_mat(m_pPA->m_aijd.data(), m_pPA->nPanels());
#endif
        if (isCancelled()) return;

        if (!m_pPA->LUfactorize())
        {
            m_bError = true;
            return;
        }

        // make the array of velocity vectors
#ifdef QT_DEBUG
//            PanelAnalysis::s_DebugPts.clear();
//            PanelAnalysis::s_DebugVecs.clear();
#endif
        for(uint i=0; i<VField.size(); i++)
        {
            m_pBtPolar->apparentWind(m_Ctrl, m_pPA->panelAt(i)->CoG().z, AWS[i]);
#ifdef QT_DEBUG
//            PanelAnalysis::s_DebugPts.append(m_pPA->panelAt(i)->CoG());
//            PanelAnalysis::s_DebugVecs.append(AWS);
#endif
        }

        // go through the loop at least once
        int nWakeIter = 1;
        if(m_pPolar3d->bVortonWake()) nWakeIter = std::max(nWakeIter, m_pPolar3d->VPWIterations());
        if(nWakeIter>1) traceStdLog("      Starting vorton loop\n");
        for(int ivw=0; ivw<nWakeIter; ivw++)
        {
            if(m_pPolar3d->bVortonWake()) traceLog(QString::asprintf("        VPW iteration %3d/%d\n", ivw+1, nWakeIter));

            if(m_pPolar3d->bVortonWake())
                m_pPA->makeRHSVWVelocities(VField);
            else
                for(uint i=0; i<VField.size(); i++) VField[i].reset();

            for(uint i=0; i<VField.size(); i++)
            {
                VField[i] += AWS.at(i);
            }

            m_pPA->makeRHS(VField, m_pPA->m_uRHS, nullptr);
#ifdef QT_DEBUG
//      displayArray(m_pPA->m_uRHS);
#endif
            m_pPA->backSubUnitRHS(m_pPA->m_uRHS.data(), nullptr, nullptr, nullptr, nullptr, nullptr);
#ifdef QT_DEBUG
//      displayArray(m_pPA->m_uRHS);
#endif

            if(m_pBtPolar->isQuadMethod())
                m_pP4A->m_Mu = m_pP4A->m_uRHS;
            else
            {
                if(m_pBtPolar->isTriLinearMethod() && m_pP3A)
                    m_pP3A->m_Mu = m_pP3A->m_uRHS;
                if(m_pBtPolar->isTriUniformMethod() && m_pP3A)
                    m_pPA->makeVertexDoubletDensities(m_pP3A->m_uRHS, m_pP3A->m_Mu);
            }

            if(m_pBtPolar->bVortonWake())
            {
                advectVortons(alpha, beta, qinf, 0);
                makeVortonRow(0);
                if(s_bLiveUpdate)
                {
                    traceVPWLog(m_Ctrl);
                }
            }

            if(m_bStopVPWIterations)
                break; // user requested interruption

            if(isCancelled()) return;
        } // end VPW loop


        traceStdLog("      Making local velocities...");
        m_pPA->makeLocalVelocities(m_pPA->m_uRHS, m_pPA->m_vRHS, m_pPA->m_wRHS, m_pPA->m_uVLocal, m_pPA->m_vVLocal, m_pPA->m_wVLocal, VFree);
        if (isCancelled()) return;
        traceStdLog(" done\n");

        traceStdLog("      Computing on body Cp...");
        if(!m_pPolar3d->isVLM())
        {
            m_pPA->computeOnBodyCp(VField, m_pPA->m_uVLocal, m_pPA->m_Cp);
        }
        if (isCancelled()) return;
        traceStdLog(" done\n");

        traceStdLog("      Calculating far field forces...");
        computeInducedForces(alpha, beta, qinf);
        computeInducedDrag(alpha, beta, qinf, 0, m_SailForceFF, m_SpanDist);
        if (isCancelled()) return;
        traceStdLog(" done\n");

        strange = QString::asprintf("      Computing boat for control parameter=%.3f\n", m_Ctrl);
        traceLog(strange);
        BoatOpp *pBtOpp = computeBoat(0);
        m_BtOppList.push_back(pBtOpp);

        if (isCancelled()) return;
    }

    if(m_AnalysisStatus!=xfl::CANCELLED) m_AnalysisStatus = xfl::FINISHED; // finish the analysis before sending the final condition_variable
    traceStdLog("\nDone plane task.\n"); // final notification after flag is set to FINISHED so that sender thread may exit
}


void BoatTask::scaleResultsToSpeed(int qrhs)
{
    double qinf = m_pBtPolar->AWSInf(m_Ctrl);
    for(int is=0; is<m_pBoat->nSails(); is++)
    {
        Sail const *pSail = m_pBoat->sail(is);
        for(int m=0; m<pSail->nStations(); m++)
        {
            m_SpanDist[qrhs*m_pBoat->nSails()+is].m_F[m]  *= qinf*qinf;
            m_SpanDist[qrhs*m_pBoat->nSails()+is].m_Vd[m] *= qinf;
        }
    }

    m_pPA->scaleResultsToSpeed(qinf);
}


BoatOpp* BoatTask::computeBoat(int qrhs)
{
    Vector3d BoatCP;

//    double QInf = m_pBtPolar->QInf(m_Ctrl);
    double beta = m_pBtPolar->AWAInf(m_Ctrl);
//    double phi  = m_pBtPolar->phi(m_CtrlParam);

    double *mu3=nullptr, *sigma3=nullptr, *Cp3Vtx=nullptr;
    if(m_pBtPolar->isTriangleMethod() && m_pP3A)
    {
        mu3    = m_pP3A->m_Mu.data()    + qrhs*3*m_pP3A->nPanels();
        sigma3 = m_pP3A->m_Sigma.data() + qrhs*  m_pP3A->nPanels();
        Cp3Vtx = m_pP3A->m_Cp.data()    + qrhs*3*m_pP3A->nPanels();
    }
    double *mu4=nullptr, *sigma4=nullptr, *cp4=nullptr;
    if(m_pBtPolar->isQuadMethod() && m_pP4A)
    {
        mu4    = m_pP4A->m_Mu.data()    + qrhs*m_pP4A->m_Panel4.size();
        sigma4 = m_pP4A->m_Sigma.data() + qrhs*m_pP4A->m_Panel4.size();
        cp4    = m_pP4A->m_Cp.data()    + qrhs*m_pP4A->m_Panel4.size();
    }


    traceStdLog("       Calculating aerodynamic coefficients...\n");

    m_AF.resetResults();

    Vector3d Fff;
    Vector3d Fsum;
//    double extradrag=0.0;

    Vector3d Mi;
    Vector3d Mv;

    Vector3d Force, Moment;

    int nSails = m_pBoat->nSails();
    for(int is=0; is<nSails; is++)
    {
        Sail *pSail = m_pBoat->sail(is);
        traceStdLog("         Calculating " + pSail->name()+EOLstr);

        //restore the saved unit inviscid results
        pSail->m_SpanResFF = m_SpanDist[is];
        Fff       += m_SailForceFF.at(is);          // N/q

        //Compute forces and moment
        if(m_pPolar3d->isQuadMethod())
        {
            // not activated
        }
        else if(m_pPolar3d->isTriangleMethod())
        {
            pSail->panel3ComputeInviscidForces(m_pP3A->panels(), m_pBtPolar, m_pBtPolar->CoG(), beta, Cp3Vtx, BoatCP, Force, Moment);
        }

        m_SailForceSum[is] = Force;

        // forces and moments in body axis
        Fsum      += Force;         // N/q
        Mi        += Moment;           // N.m/q
//        Mv        += pSail->m_AF.Mv();           // N.m/q
//        extradrag += pSail->m_AF.extraDrag();    // N/q
    }
    m_AF.setFff(Fff);
    m_AF.setFsum(Fsum);
    m_AF.setMi(Mi);
    m_AF.setMv(Mv);
    m_AF.setExtraDrag(m_pBtPolar->constantDrag()); // N/q

    m_AF.setM0(Vector3d()); // not sure what is the center of pressure of a boat

    BoatOpp *pBtOpp=nullptr;
    if(m_pPolar3d->isTriangleMethod() && Cp3Vtx)
    {
        pBtOpp = createBtOpp(Cp3Vtx, mu3, sigma3);
    }
    else if (m_pPolar3d->isQuadMethod() && cp4)
    {
        pBtOpp  = createBtOpp(cp4, mu4, sigma4);
    }

    traceStdLog("\n");

    return pBtOpp;
}


void BoatTask::setAngles(std::vector<Panel3> &panels, double phi)
{
    Vector3d O(0.0,0.0,0.0);

    //rotate the sail panels around their leading edge axis
    Vector3d LEaxis;
    for(int is=0; is<m_pBoat->nSails(); is++)
    {
        Sail const *pSail = m_pBoat->sail(is);
        LEaxis = pSail->leadingEdgeAxis().normalized();
        Vector3d tack = pSail->m_Tack + pSail->position();

        double angle = m_pBtPolar->sailAngle(is, m_Ctrl);

        if(fabs(angle)>ANGLEPRECISION)
        {
            if(m_pPolar3d->isQuadMethod())
            {
            }
            else if(m_pPolar3d->isTriangleMethod())
            {
                for(int ip=pSail->firstPanel3Index(); ip<pSail->firstPanel3Index()+pSail->nPanel3(); ip++)
                {
                    m_pPA->panel(ip)->rotate(tack, LEaxis, angle);
                }
            }
        }
    }

    // rotate the panels around the x-axis by the bank angle
    for(uint i3=0; i3<panels.size(); i3++)
    {
        panels[i3].rotate(O, Vector3d(1,0,0), phi);
    };
}


int BoatTask::allocateSailResultsArrays()
{
    m_SailForceFF.resize(m_pBoat->nSails());
    m_SailForceSum.resize(m_pBoat->nSails());
    for(int is=0; is<m_pBoat->nSails(); is++)
    {
        m_SailForceFF[is].set(0,0,0);
        m_SailForceSum[is].set(0,0,0);
    }

    m_SpanDist.resize(m_pBoat->nSails());

    for(int iw=0; iw<m_pBoat->nSails(); iw++)
    {
        m_SpanDist[iw] = m_pBoat->sail(iw)->spanDistFF();
    }

    m_HullForce.resize(m_pBoat->nHulls());
    for(int ih=0; ih<m_pBoat->nHulls(); ih++) m_HullForce[ih].set(0,0,0);
    return 0;
}


BoatOpp *BoatTask::createBtOpp(double const*Cp, double const*Mu, double const*Sigma)
{
    BoatOpp *pBtOpp = nullptr;

    int nPanels=0;

    if(m_pPolar3d->isQuadMethod())
    {
        // not activated
        return nullptr;
    }
    else if(m_pPolar3d->isTriangleMethod())
    {
        nPanels = m_pBoat->triMesh().nPanels();
        pBtOpp = new BoatOpp(m_pBoat, m_pBtPolar, nPanels, 0);
        if(!pBtOpp) return nullptr;
        pBtOpp->resizeResultsArrays(3*nPanels);
        memcpy(pBtOpp->Cp().data(),    Cp,    3*ulong(nPanels)*sizeof(double));
        memcpy(pBtOpp->gamma().data(), Mu,    3*ulong(nPanels)*sizeof(double));
        memcpy(pBtOpp->sigma().data(), Sigma, ulong(nPanels)*sizeof(double));
    }

    pBtOpp->setTheStyle(m_pPolar3d->theStyle());
    pBtOpp->setVisible(true);

    pBtOpp->setAnalysisMethod(m_pBtPolar->analysisMethod());

//    if(isTriangleMethod())  TriMesh::makeNodeValues(m_RefTriMesh.nodes(), m_Panel3, pBtOpp->m_dCp, pBtOpp->m_NodeValue, 1.0);
    double h    = m_pBtPolar->groundHeight();
    double QInf = m_pBtPolar->AWSInf(m_Ctrl);
    double beta = m_pBtPolar->AWAInf(m_Ctrl);


    //get the data from the PanelAnalysis class, and from the plane object
    pBtOpp->setCtrl(m_Ctrl);
    pBtOpp->setGroundHeight(h);
    pBtOpp->setQInf(QInf);
    pBtOpp->setBeta(beta);
    pBtOpp->setPhi(m_pBtPolar->phi(m_Ctrl));
    pBtOpp->setRy( m_pBtPolar->Ry(m_Ctrl));

    pBtOpp->setTWInf(m_pBtPolar->TWSInf(m_Ctrl), m_pBtPolar->TWAInf(m_Ctrl));

    for(int is=0; is<m_pBtPolar->sailAngleSize(); is++)
    {
        double angle = m_pBtPolar->sailAngle(is, m_Ctrl);
        pBtOpp->setSailAngle(is, angle);
    }

    pBtOpp->setGroundEffect(m_pPolar3d->bGroundEffect());
    pBtOpp->setGroundHeight(m_pPolar3d->groundHeight());

    // store the forces in N/q and N.m/q
    m_AF.setOpp(0.0, beta, 0.0, QInf);
    m_AF.setReferenceChord(m_pBtPolar->referenceChordLength());
    m_AF.setReferenceArea(m_pBtPolar->referenceArea());

    pBtOpp->setAeroForces(m_AF);
    pBtOpp->setSailForceFF(m_SailForceFF);
    pBtOpp->setSailForceSum(m_SailForceSum);

/*    if(m_pBtPolar->isTriangleMethod())
    {
        TriMesh::makeNodeValues(m_pP3A->m_pRefTriMesh->nodes(), m_pP3A->m_Panel3,
                                pBtOpp->m_Cp, pBtOpp->m_NodeValue,
                                pBtOpp->m_NodeValMin, pBtOpp->m_NodeValMax,
                                1.0);
    }*/

    if(m_pBtPolar->bVortonWake())
    {
        pBtOpp->setVortons(m_pPA->m_Vorton);
        pBtOpp->setVortexNeg(m_pPA->m_VortexNeg);
    }

    //add the data to the polar object
    m_pBtPolar->addPoint(pBtOpp);

    return pBtOpp;
}


/**
 * Prepend a vorton row, and clean the last row if inactive
 */
void BoatTask::makeVortonRow(int qrhs)
{
    if(!m_pPolar3d->bVortonWake()) return;

    double const *mu    = nullptr;
    if     (m_pP4A) mu = m_pP4A->m_Mu.data() + qrhs*m_pP4A->nPanels();
    else if(m_pP3A) mu = m_pP3A->m_Mu.data() + qrhs*3*m_pP3A->nPanels();
    else return;

    // update positions and vorticities
    // duplicate the existing vortons which will be replaced all at once at the end of the procedure
    std::vector<std::vector<Vorton>> newvortons = m_pPA->m_Vorton;

    // convert the doublet sheet into the first row of vortons
    // and prepend the row to the array
    std::vector<Vorton> vortonrow; // a row includes the vorton rows from all the wings
    std::vector<Vortex> wingvortexneg;
    m_pPA->m_VortexNeg.clear(); // update the trailing negating vortices

    // reset the first step
    double dl = m_pPolar3d->vortonL0() * m_pPolar3d->referenceChordLength(); //m

    std::vector<Vorton> wingvortons;
    int pos = 0;
    for(int iw=0; iw<m_pBoat->nSails(); iw++)
    {
        Sail const *pSail = m_pBoat->sailAt(iw);
        if(m_pP4A)
        {
        }
        else if(m_pP3A)
        {
            m_pP3A->makeVortons(dl, mu, pos, pSail->nPanel3(), pSail->nStations(), int(vortonrow.size()), wingvortons, wingvortexneg);
            pos += pSail->nPanel3();
        }

        vortonrow.insert(vortonrow.end(), wingvortons.begin(), wingvortons.end());
        m_pPA->m_VortexNeg.insert(m_pPA->m_VortexNeg.end(), wingvortexneg.begin(), wingvortexneg.end());
    }


    // merge the vortons and update indexes
    int iv=0;
    while(iv<int(vortonrow.size()))
    {
        for(int jv=int(vortonrow.size())-1; jv>iv; jv--)
        {
            if(vortonrow.at(iv).position().isSame(vortonrow.at(jv).position(), 1.e-4))
            {
                vortonrow[iv].setVortex(vortonrow[iv].vortex()+vortonrow[jv].vortex());
                vortonrow.erase(vortonrow.begin()+jv);
                for(uint i=0; i<m_pPA->m_VortexNeg.size(); i++)
                {
                    Vortex &vortex = m_pPA->m_VortexNeg[i];
                    if(vortex.nodeIndex(0)==jv)
                        vortex.setNodeIndex(0, iv);

                    if(vortex.nodeIndex(1)==jv)
                        vortex.setNodeIndex(1, iv);

                    // decrease indexes above jv by 1
                    if(vortex.nodeIndex(0)>jv) vortex.setNodeIndex(0, vortex.nodeIndex(0)-1);
                    if(vortex.nodeIndex(1)>jv) vortex.setNodeIndex(1, vortex.nodeIndex(1)-1);
                }
            }
        }
        iv++;
    }

//for(int i=0; i<m_pPA->m_VortexNeg.size(); i++)    qDebug("vortexnex%3d:  %2d  %2d", i, m_pPA->m_VortexNeg.at(i).nodeIndex(0), m_pPA->m_VortexNeg.at(i).nodeIndex(1));
//qDebug(" ");

    newvortons.insert(newvortons.begin(), vortonrow);

    // if at the first iteration, extend the first row to a flat VPW
/*    if(newvortons.size()==1)
    {
        double l = m_pWPolar->vortonL0() * m_pWPolar->referenceChordLength() * double(m_pWPolar->VPWIterations());
        l += m_pWPolar->bufferWakeLength() * m_pWPolar->referenceChordLength();
        int nrows = int(m_pWPolar->VPWMaxLength()/m_pWPolar->vortonL0());

        Vector3d T(m_pWPolar->vortonL0()* m_pWPolar->referenceChordLength(),0,0);

        for(int ir=0; ir<nrows; ir++)
        {
            std::vector<Vorton> newrow = vortonrow;
            // translate the new row
            for(int ivtn=0; ivtn<newrow.size(); ivtn++)
            {
                newrow[ivtn].translate(T*double(ir+1));
            }
            // push it on the stack of vorton rows
            newvortons.push_back(newrow);
        }
    }*/


    // check if the last row is still active
    bool bActiveLastRow = false;
    std::vector<Vorton> &lastrow = newvortons.back();
    for(uint i=0; i<lastrow.size(); i++)
    {
        if(lastrow.at(i).isActive())
        {
            bActiveLastRow = true;
            break;
        }
    }
    if(!bActiveLastRow)
        newvortons.pop_back();

    // save the new vortons
    m_pPA->m_Vorton = newvortons;
}


/**
 * Calculates the cross-flow forces L an Y for a unit velocity
 * Induced drag is not computed in this method
 */
void BoatTask::computeInducedForces(double alpha, double beta, double QInf)
{
    int nSails = m_pBoat->nSails();
    int pos = 0;
    for(int iw=0; iw<nSails; iw++)
    {
        Sail *pSail = m_pBoat->sail(iw);
        if(!m_pBtPolar || !pSail) continue; // to shut clang up

        Vector3d forcebodyaxes;
        if(m_pBtPolar->isQuadMethod() && m_pP4A)
            m_pP4A->inducedForce(pSail->nPanel4(), QInf, alpha, beta, pos, forcebodyaxes, pSail->spanDistFF());
        else if(m_pBtPolar->isTriangleMethod() && m_pP3A)
            m_pP3A->inducedForce(pSail->nPanel3(), QInf, alpha, beta, pos, forcebodyaxes, pSail->spanDistFF());

        //save the results... will save another FF calculation when computing the operating points
        m_SailForceFF[iw] += forcebodyaxes;     // N/q, body axes
        m_SpanDist[iw] = pSail->spanDistFF();

        if      (m_pBtPolar->isTriangleMethod()) pos += pSail->nPanel3();
        else if (m_pBtPolar->isQuadMethod())     pos += pSail->nPanel4();

        if(isCancelled())return;
    }
}


/**
 * Calculates the induced drag half-way down the wake to avoid end effects.
 * Uses the vorton induced velocity if the vorton wake has been defined,
 * otherwise uses the flat wake panels.
 */
void BoatTask::computeInducedDrag(double alpha, double beta, double QInf, int qrhs,
                                  std::vector<Vector3d> &SailForce, std::vector<SpanDistribs> &SpanDist) const
{
    Vector3d Drag;

    int nSails = m_pBoat->nSails();

    int m0 = 0;
    int pos = 0;
    for(int iw=0; iw<nSails; iw++)
    {
        Sail *pSail = m_pBoat->sail(iw);
        if(m_pPolar3d->bVortonWake())
        {
            m_pPA->vortonDrag(alpha, beta, QInf, m0, pSail->nStations(), Drag, pSail->spanDistFF());
            m0 += pSail->nStations();
        }
        else
        {
            if(m_pBtPolar->isQuadMethod() && m_pP4A)
                m_pP4A->trefftzDrag(pSail->nPanel4(), QInf, alpha, beta, pos, Drag, pSail->spanDistFF());
            else if(m_pBtPolar->isTriangleMethod() && m_pP3A)
                m_pP3A->trefftzDrag(pSail->nPanel3(), QInf, alpha, beta, pos, Drag, pSail->spanDistFF());
        }
        SailForce[qrhs*nSails+iw] += Drag;     // N/q, body axes
        SpanDist[qrhs*nSails+iw].m_ICd = pSail->spanDistFF().m_ICd;
        SpanDist[qrhs*nSails+iw].m_Vd  = pSail->spanDistFF().m_Vd;
        SpanDist[qrhs*nSails+iw].m_Ai  = pSail->spanDistFF().m_Ai;

        if      (m_pBtPolar->isTriangleMethod()) pos += pSail->nPanel3();
        else if (m_pBtPolar->isQuadMethod())     pos += pSail->nPanel4();

//        double qDyn =  0.5*QInf*QInf*m_pWPolar->density();
//(m_SailForce[qrhs*nSails+iw]*qDyn).listCoords("Sailforce (N)");
        if(isCancelled())return;
    }
}

