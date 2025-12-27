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

#include <chrono>
#include <string>
#include <thread>


#include <QString>
#include <QDebug>


#if defined ACCELERATE
  #include <Accelerate/Accelerate.h>
  #define lapack_int int
#elif defined INTEL_MKL
    #include <mkl.h>
#elif defined OPENBLAS
    #include <openblas/lapacke.h>
//    #define lapack_int int
#endif


#include <planetask.h>

#include <geom_params.h>
#include <mesh_globals.h>
#include <objects2d.h>
#include <objects3d.h>
#include <objects_global.h>
#include <p3linanalysis.h>
#include <p3unianalysis.h>
#include <p4analysis.h>
#include <panelanalysis.h>
#include <planeopp.h>
#include <planepolar.h>
#include <planexfl.h>
#include <polar.h>
#include <stabderivatives.h>
#include <units.h>
#include <utils.h>
#include <wingxfl.h>
#include <xfoiltask.h>


bool PlaneTask::s_bViscInitTwist = false;
double PlaneTask::s_ViscRelax = 0.5;
double PlaneTask::s_ViscAlphaPrecision = 0.01;
int PlaneTask::s_ViscMaxIter = 35;

PlaneTask::PlaneTask() : Task3d()
{
    m_pPlane  = nullptr;
    m_pPlPolar = nullptr;

    m_Ctrl = -LARGEVALUE;
    m_Alpha = 0.0;
    m_QInf = m_Beta = m_Phi = 0.0;

    m_bDerivatives = true;

    m_AF.resetAll();
}


PlaneTask::~PlaneTask()
{
}


void PlaneTask::cancelTask()
{
    Task3d::cancelTask();
    XFoilTask::setCancelled(true);
}


void PlaneTask::setViscousLoopSettings(bool bInitVTwist, double relaxfactor, double alphaprec, int maxiters)
{
    s_bViscInitTwist     = bInitVTwist;
    s_ViscRelax          = relaxfactor;
    s_ViscAlphaPrecision = alphaprec;
    s_ViscMaxIter        = maxiters;
}


void PlaneTask::setOppList(std::vector<double> const &opplist)
{
    m_AngleList  = opplist;
    m_nRHS       = int(opplist.size());
}


void PlaneTask::setCtrlOppList(std::vector<double> const &opplist)
{
    m_T6CtrlList = opplist;
    m_nRHS       = int(opplist.size());
}


void PlaneTask::setStabOppList(std::vector<double> const &opplist)
{
    m_T7CtrlList = opplist;
    m_nRHS       = int(opplist.size());
}


void PlaneTask::setT8OppList(std::vector<T8Opp> const &ranges)
{
    m_T8Opps = ranges;
    m_nRHS = int(ranges.size());
}


bool PlaneTask::initializeTask()
{
    QString strange, strong;
    QString lenlab = Units::lengthUnitQLabel();

    m_bWarning = m_bError = false;

    XFoilTask::setCancelled(false);

    traceStdLog("Initializing task\n\n");

    // allocate virtual twist angles
    m_gamma.resize(m_pPlane->nStations());
    std::fill(m_gamma.begin(), m_gamma.end(), 0);

    traceStdLog(m_pPlane->name() + EOLstr);
    traceStdLog(m_pPlPolar->name()+"\n\n");

    // check that reference dims are valid
    if(!checkWPolarData(m_pPlane, m_pPlPolar))
    {
        traceStdLog("Aborting analysis\n");
        return false;
    }

    // check flaps and ViscOTF
    if(m_pPlPolar->hasActiveFlap())
    {
        if(!m_pPlPolar->isViscOnTheFly())
        {
            traceStdLog("Warning: XFoil on the fly calculations should be selected when TE flaps are active\n\n");
        }
    }

    traceStdLog(m_pPolar3d->name() + EOLstr);

    if     (m_pPolar3d->isFixedSpeedPolar())  strange = "Type 1 - Fixed speed polar";
    else if(m_pPolar3d->isFixedLiftPolar())   strange = "Type 2 - Fixed lift polar";
    else if(m_pPolar3d->isFixedaoaPolar())    strange = "Type 4 - Fixed "+ ALPHAch + " polar";
    else if(m_pPolar3d->isBetaPolar())        strange = "Type 5 - " + BETAch + " polar";
    else if(m_pPolar3d->isControlPolar())     strange = "Type 6 - Control polar";
    else if(m_pPolar3d->isStabilityPolar())   strange = "Type 7 - Stability polar";
    else if(m_pPolar3d->isBoatPolar())        strange = "Sail analysis";
    else                                      strange = "Unsupported polar type";
    traceLog(strange+"\n\n");

    strange = RHOch + QString::asprintf(" = %9.5g ", m_pPolar3d->density()*Units::densitytoUnit());
    traceLog(strange+ Units::densityUnitQLabel() + EOLch);
    strange = NUch  + QString::asprintf(" = %9.5g ", m_pPolar3d->viscosity()*Units::viscositytoUnit());
    traceLog(strange+ Units::viscosityUnitQLabel()+"\n\n");

    strange = QString::asprintf("RFF         = %g\n", Panel::RFF());
    traceLog(strange);
    strange = QString::asprintf("Core radius = %g ", Vortex::coreRadius()*Units::mtoUnit());
    strange += Units::lengthUnitQLabel()  + EOLch;

    if(Vortex::coreRadius()<1.e-6)
    {
        strange += "Caution: too small core radiuses can lead to numerical instabilities.\n"
                   "Recommendation: Core radius > 1 µm\n\n";
    }


    traceLog(strange);
    traceStdLog("\n");

    if(PanelAnalysis::s_bMultiThread) traceStdLog("Running in multi-threaded mode\n\n");
    else                              traceStdLog("Running in single-threaded mode\n\n");

    if(PanelAnalysis::s_bDoublePrecision) traceStdLog("Linear system calculations in floating point double precision\n\n");
    else                                  traceStdLog("Linear system calculations in floating point single precision\n\n");

    if(Panel3::usingNintcheuFataMethod())
        traceStdLog("Using S. Nintcheu-Fata's method for off-plane integrals\n\n");
    else
        traceStdLog("Using M. Carley's method for off-plane integrals\n\n");

    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl *>(m_pPlane);

    if(m_pPlPolar->isQuadMethod())
    {
        traceStdLog("Initializing the quad analysis\n");
        if(!pPlaneXfl)
        {
            traceStdLog("Quad analysis not available for STL-type planes\n");
            return false;
        }
        m_pP4A = new P4Analysis;
        m_pPA = m_pP4A;
    }
    else if(m_pPlPolar->isTriangleMethod())
    {
        traceStdLog("Initializing the triangle analysis\n");

        traceStdLog("Checking element sizes\n");
        std::vector<int> criticals;
        std::vector<double> sizes;
        m_pPlane->triMesh().checkElementSize(Vortex::coreRadius(), criticals, sizes);
        if(criticals.size()>0)
        {
            strange = "The element size is incompatible with the vortex core size.\n"
                      "Either increase the element size, or reduce the vortex core size to be at most 1/50 of the smallest element size.\n"
                      "The vortex core size is set in the global analysis settings (menu option analysis/3d analysis settings).\n";
            strange += QString::asprintf("   Core radius = %g ", Vortex::coreRadius()*Units::mtoUnit()) + lenlab + EOLch;
            strange += "      Critical elements:\n";
            for(uint i=0; i<criticals.size(); i++)
            {
                strange += QString::asprintf("      element %5d   width=%11.5g ",
                                             criticals.at(i),
                                             sizes.at(i)*Units::mtoUnit()) + lenlab;
                strange += QString::asprintf("  ratio = 1/%.1f", sizes.at(i)/Vortex::coreRadius()) + EOLch;
            }
            traceLog(strange+"\n\n");
            m_bError = true;
            return false;
        }
        traceStdLog("   ...done\n\n");

        if(m_pPlPolar->isTriUniformMethod())
        {
            m_pP3A = new P3UniAnalysis;
            m_pPA = m_pP3A;
        }
        if(m_pPlPolar->isTriLinearMethod())
        {
            // make the node normals on the fly
            // node normals are required to compute local Cp coefficients at nodes
            m_pP3A = new P3LinAnalysis;
            m_pPA = m_pP3A;
        }
    }

    switch(m_pPlPolar->type())
    {
        case xfl::BOATPOLAR:
        {
            return false;
        }
        case xfl::EXTERNALPOLAR:
        {
            traceStdLog("External polars are not available for analysis\n");
            return false;
        }
        case xfl::T1POLAR:
        case xfl::T2POLAR:
        case xfl::T3POLAR:
        case xfl::T4POLAR:
        case xfl::T5POLAR:
        case xfl::T7POLAR:
        case xfl::T8POLAR:
        {
            m_pPlane->restoreMesh();

            std::string outstring;
            // rotate the flaps in position before connecting to prevent connections at flap edges if deflection is non-zero
            if(pPlaneXfl) pPlaneXfl->setFlaps(m_pPlPolar, outstring);
            traceStdLog(outstring);

            auto start = std::chrono::system_clock::now();

            traceStdLog("Connecting triangular panels...");

            if(!m_pPlane->connectTriMesh(true, false, true))
            {
                strange = "\n   Error making trailing edge connections -- aborting.\n\n";
                traceLog(strange);
                m_bError = true;
                return false;
            }

            auto end = std::chrono::system_clock::now();
            int duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            strange = QString::asprintf("   done in %.3f s\n", double(duration)/1000.0);
            traceLog(strange);


            if(fabs(m_pPlPolar->phi())>AOAPRECISION)
            {
                if(m_pPlPolar->isTriangleMethod())
                    pPlaneXfl->triMesh().rotate(0,0,m_pPlPolar->phi());
                else if(m_pPlPolar->isQuadMethod())
                    pPlaneXfl->quadMesh().rotate(0,0,m_pPlPolar->phi());
            }

            if(pPlaneXfl && m_pPlPolar->isQuadMethod())
            {
                m_pP4A->setQuadMesh(pPlaneXfl->quadMesh());
                m_pP4A->initializeAnalysis(m_pPlPolar, m_nRHS);
            }

            else if(m_pPlPolar->isTriangleMethod())
            {
                m_pP3A->setTriMesh(m_pPlane->triMesh());
                m_pP3A->initializeAnalysis(m_pPlPolar, m_nRHS);
            }

            break;
        }
        case xfl::T6POLAR:
        {
            auto start = std::chrono::system_clock::now();

            traceStdLog("Connecting triangular panels...");

            if(!m_pPlane->connectTriMesh(true, false, true))
            {
                strange = "\n   Error making trailing edge connections -- aborting.\n\n";
                traceLog(strange);
                m_bError = true;
                return false;
            }

            auto end = std::chrono::system_clock::now();
            int duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            strange = QString::asprintf("   done in %.3f s\n", double(duration)/1000.0);
            traceLog(strange);

            if(pPlaneXfl && m_pPlPolar->isQuadMethod())
            {
                m_pP4A->setQuadMesh(pPlaneXfl->refQuadMesh());
                m_pP4A->initializeAnalysis(m_pPlPolar, m_nRHS);
            }
            else if(m_pPlPolar->isTriangleMethod())
            {
                m_pP3A->setTriMesh(m_pPlane->triMesh());
                m_pP3A->initializeAnalysis(m_pPlPolar, m_nRHS);
            }

            if(m_pPlPolar->bViscousLoop())
            {
                strange = "The viscous loop is activated:\n";
                strong = QString::asprintf("   Max. iterations         = %d\n", s_ViscMaxIter);
                strange += strong;
                strong = QString::asprintf("   Virtual twist precision = %g", s_ViscAlphaPrecision);
                strong += DEGch  + EOLch;
                strange += strong;
                strong = QString::asprintf("   Relaxation factor       = %g\n\n", s_ViscRelax);
                strange += strong;
                traceLog(strange);
            }

            if(m_pPlPolar->bVortonWake())
            {
                strange = "Vortex particle wake:\n";
                strong = QString::asprintf("   Max. iterations  = %d\n", m_pPolar3d->VPWIterations());
                strange += strong;
                strong = QString::asprintf("   Discard distance = %g x MAC\n", m_pPolar3d->VPWMaxLength());
                strange += strong;
                strong = QString::asprintf("   Vorton core size = %g x MAC = %g",
                                           m_pPolar3d->vortonCoreSize(), m_pPolar3d->vortonCoreSize()*m_pPolar3d->referenceChordLength()*Units::mtoUnit());
                strong += lenlab  + EOLch;
                strange += strong;
                traceLog(strange);
            }
            break;
        }
    }

    strange = QString::asprintf("Counted %d elements\n", m_pPA->nPanels());
    strange += QString::asprintf("Matrix size = %d\n\n", m_pPA->matSize());
    traceLog(strange);

    if(isCancelled()) return false;

    traceStdLog("Allocating result arrays\n");
    allocatePlaneResultArrays();

    return true;
}


void PlaneTask::allocatePlaneResultArrays()
{
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);

    if(pPlaneXfl)
    {
        m_WingForce.resize(pPlaneXfl->nWings());

        m_SpanDistFF.resize(pPlaneXfl->nWings());
        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            WingXfl const *pWing = pPlaneXfl->wingAt(iw);
            m_SpanDistFF[iw].resizeResults(pWing->nStations());
            m_SpanDistFF[iw].setGeometry(pWing);
        }
    }
    else
    {
        m_WingForce.resize(1); // 1 thing

        m_SpanDistFF.resize(1);

        m_SpanDistFF[0].resizeGeometry(m_pPlane->nStations());
        m_SpanDistFF[0].resizeResults(m_pPlane->nStations());

    }
 }


void PlaneTask::setObjects(Plane *pPlane, PlanePolar *pWPolar)
{
    m_pPlane = pPlane;
    m_pPlPolar = pWPolar;
    m_pPolar3d = m_pPlPolar;

    m_AF.resetAll();
    m_AF.setReferenceDims(pWPolar->referenceArea(), pWPolar->referenceChordLength(), pWPolar->referenceSpanLength());

    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pPlane);
    if(pPlaneXfl)
    {
        m_SpanDistFF.resize(pPlaneXfl->nWings());
        m_PartAF.resize(m_pPlane->nParts());

        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            WingXfl *pWing = pPlaneXfl->wing(iw);
            if(m_pPlPolar->bProjectedDim())
                m_PartAF[iw].setReferenceDims(pWing->projectedArea(), pWing->MAC(), pWing->projectedSpan());
            else
                m_PartAF[iw].setReferenceDims(pWing->planformArea(),  pWing->MAC(), pWing->planformSpan());

            m_SpanDistFF[iw].resizeResults(pWing->nStations());
            pWing->resizeSpanDistribs();
        }
    }

    if(m_pPlPolar && m_pPlPolar->bAutoInertia())
    {
        if(pPlane)
        {
            double mass = pPlane->totalMass();
            Vector3d CoG = pPlane->inertia().CoG_t();
            m_pPlPolar->setMass(mass);
            m_pPlPolar->setCoG(CoG);
            m_pPlPolar->setIxx(pPlane->inertia().Ixx_t());
            m_pPlPolar->setIyy(pPlane->inertia().Iyy_t());
            m_pPlPolar->setIzz(pPlane->inertia().Izz_t());
            m_pPlPolar->setIxz(pPlane->inertia().Ixz_t());

            if(m_pPlPolar->isType6())
            {
                m_pPlPolar->retrieveInertia(pPlane);
            }
        }
    }
}


bool PlaneTask::checkWPolarData(Plane const *pPlane, PlanePolar *pWPolar)
{
    traceStdLog("Checking plane and polar data\n");
    if(!pPlane)
    {
        traceStdLog("Error: plane object not found... aborting\n");
        return false;
    }

    if(!pWPolar)
    {
        traceStdLog("Error: polar object not found... aborting\n");
        return false;
    }

    QString strange, logmsg;
    std::string str;
    bool bCheck = true;

    if(m_pPlane->isXflType())
    {
        PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
        if(!pWPolar->checkFlaps(pPlaneXfl, str))
        {
            logmsg += QString::fromStdString(str);
            pWPolar->resizeFlapCtrls(pPlaneXfl);
        }

        if(pPlaneXfl->nWings()>0)
        {
            if(fabs(pPlane->mac())<1.e-3)
            {
                // needed for wake construction
                strange = QString::asprintf("   error: Plane has MAC = %g", pPlane->mac()*Units::mtoUnit());
                strange += Units::lengthUnitQLabel()  + EOLch;
                logmsg += strange;
                bCheck = false;
            }

            if(fabs(pWPolar->referenceChordLength())<1.e-3)
            {
                // needed for coefficient calculations
                strange = QString::asprintf("   error: reference chord length is %g", pWPolar->referenceChordLength()*Units::mtoUnit());
                strange += Units::lengthUnitQLabel()  + EOLch;
                logmsg += strange;
                bCheck = false;
            }
            if(fabs(pWPolar->referenceSpanLength())<1.e-3)
            {
                // needed for coefficient calculations
                strange = QString::asprintf("   error: reference span length is %g", pWPolar->referenceSpanLength()*Units::mtoUnit());
                strange += Units::lengthUnitQLabel()  + EOLch;
                logmsg += strange;
                bCheck = false;
            }
            if(fabs(pWPolar->referenceArea())<1.e-6)
            {
                // needed for coefficient calculations
                strange = QString::asprintf("   error: reference area is %g", pWPolar->referenceArea()*Units::m2toUnit());
                strange += Units::areaUnitQLabel()  + EOLch;
                logmsg += strange;
                bCheck = false;
            }

            if(!bCheck)
            {
                if(!pPlane->hasMainWing())
                {
                    strange = "   warning: no main wing detected in plane:" + QString::fromStdString(pPlane->name());
                }
                logmsg += strange;
                m_bError = true;
            }
        }
    }

    if(bCheck) traceStdLog("   ...done\n\n");
    else       traceLog(logmsg);

    return bCheck;
}


void PlaneTask::loop()
{
    if(m_pPlPolar->isType123458())
    {
        if(m_pPlPolar->isType8())
        {
            // use specified T8 range
        }
        else
        {
            // make equivalent T8 range
            m_T8Opps.clear();

            double alpha(0), beta(0), qinf(0);
            for(uint qrhs=0; qrhs<m_AngleList.size(); qrhs++)
            {
                if(m_pPlPolar->isType1())
                {
                    alpha = m_AngleList.at(qrhs);
                    beta  = m_pPlPolar->betaSpec();
                    qinf  = m_pPlPolar->velocity();
                }
                else if(m_pPlPolar->isFixedLiftPolar() || m_pPlPolar->isGlidePolar())
                {
                    alpha = m_AngleList.at(qrhs);
                    beta  = m_pPlPolar->betaSpec();
                    qinf =  1.0;
                }
                else if(m_pPlPolar->isType4())
                {
                    alpha = m_pPlPolar->alphaSpec();
                    beta  = m_pPlPolar->betaSpec();
                    qinf  = m_AngleList.at(qrhs);
                }
                else if(m_pPlPolar->isType5())
                {
                    alpha = m_pPlPolar->alphaSpec();
                    beta  = m_AngleList.at(qrhs);
                    qinf  = m_pPlPolar->velocity();
                }
                m_T8Opps.push_back({true, alpha, beta, qinf});
            }
        }
        T123458Loop();
    }
    else if(m_pPlPolar->isType6())
    {
        T6Loop();
    }
    else if(m_pPlPolar->isType7())
    {
        T7Loop();
    }

}


bool PlaneTask::T6Loop()
{
    QString log, str, strange;

    double error(0), CL(0);

    Vector3d F;

    traceStdLog("\nSolving the problem... \n\n");

    std::vector<Vector3d> VField(m_pPA->nPanels()), VVPW(m_pPA->nPanels());

    if(m_pPlPolar->isVLM()) m_pPA->m_nStations = m_pPlane->nStations(); // for assertion checks only

    bool bConvergedLast = false;

    // initialize for the first operating point
    std::fill(m_gamma.begin(), m_gamma.end(), 0);

    double AlphaStab(0.0), BetaStab(0.0), QInfStab(1.0);

    for (m_qRHS=0; m_qRHS<m_nRHS; m_qRHS++)
    {
        if(s_bCancel)
        {
            m_AnalysisStatus = xfl::CANCELLED;
            return false;
        }


        log.clear();
        traceStdLog("\n");
        m_bStopVPWIterations = false;

        m_Ctrl = m_T6CtrlList.at(m_qRHS);
        strange = QString::asprintf("    Processing control value= %.3f\n", m_Ctrl);

        if(!m_pPlPolar->isAdjustedVelocity())
        {
            QInfStab = m_pPlPolar->QInfCtrl(m_Ctrl);
            str = "      V" + INFch + QString::asprintf(" = %.3f ", QInfStab*Units::mstoUnit());
            strange += str + Units::speedUnitQLabel()  + EOLch;
            if(fabs(QInfStab)<1.e-6)
            {
                strange +="         null velocity... skipping operating point\n";
                traceLog(strange);
                continue;
            }
        }
        else
        {
            strange += "      V" + INFch + " = adjusted\n";
        }

        m_Alpha = m_pPlPolar->aoaCtrl(m_Ctrl);
        str = "      " + ALPHAch + QString::asprintf("  = %.3f", m_Alpha);
        strange += str + DEGch  + EOLch;

        m_Beta = m_pPlPolar->betaCtrl(m_Ctrl);
        str = "      " + BETAch  + QString::asprintf("  = %.3f", m_Beta);
        strange += str + DEGch  + EOLch;
        BetaStab = -m_Beta;

        m_Phi = m_pPlPolar->phiCtrl(m_Ctrl);
        str = "      " + PHIch   + QString::asprintf("  = %.3f", m_Phi);
        strange += str + DEGch  + EOLch;

        double mass = m_pPlPolar->massCtrl(m_Ctrl);
        str = QString::asprintf("      mass     = %.3f ", mass*Units::kgtoUnit());
        strange += str + Units::massUnitQLabel()  + EOLch;

        Vector3d CoG = m_pPlPolar->CoGCtrl(m_Ctrl);
        str = QString::asprintf("      CoG.x    = %.3f ", CoG.x*Units::mtoUnit());
        strange += str + Units::lengthUnitQLabel()  + EOLch;

        str = QString::asprintf("      CoG.z    = %.3f ", CoG.z*Units::mtoUnit());
        strange += str + Units::lengthUnitQLabel()  + EOLch;
        traceLog(strange);

        //reset the initial geometry before a new angle is processed
        traceStdLog("\n      Restoring the base mesh\n");
        m_pPlane->restoreMesh();
        m_pPA->restorePanels();

        traceStdLog("      Setting control positions\n");

        PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pPlane);
        if(pPlaneXfl)
        {
            std::string str;
            if(m_pP4A && m_pPlane->isXflType())
            {
                pPlaneXfl->setRangePositions4(m_pPlPolar, m_Ctrl, str);
                m_pP4A->setQuadMesh(pPlaneXfl->quadMesh());
            }
            else if(m_pP3A)
            {
                pPlaneXfl->setRangePositions3(m_pPlPolar, m_Ctrl, str);
                m_pP3A->setTriMesh(m_pPlane->triMesh());
            }
            log += QString::fromStdString(str);
        }

        if(m_pP4A && m_pPlane->isXflType())
        {
            pPlaneXfl->quadMesh().rotate(m_Alpha, m_Beta, m_Phi);
            m_pP4A->setQuadMesh(pPlaneXfl->quadMesh());
        }
        else if(m_pP3A)
        {
            m_pPlane->triMesh().rotate(m_Alpha, m_Beta, m_Phi);
            m_pP3A->setTriMesh(m_pPlane->triMesh());
        }


        traceLog(log);

        traceStdLog("      Updating wake panels\n");
        // wake panels aligned with x-axis up to beta 11
        // wake panels aligned the with the freestream direction since beta 12
        // little or no difference to the results as long as the downwash is evaluated on the wake itself
        m_pPA->makeWakePanels(objects::windDirection(AlphaStab, 0.0), m_pPlPolar->bVortonWake());

        if(m_pPlPolar->bVortonWake())
        {
            m_pPA->clearVortons(); // from the previous operating point calculation
            m_pPA->m_VortexNeg.clear();
        }



        auto start = std::chrono::system_clock::now();

        traceStdLog("      Making the influence matrix...");
        m_pPA->makeInfluenceMatrix();

        auto end = std::chrono::system_clock::now();
        int duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        start = end;
        strange = QString::asprintf("     done in %.3f s\n", double(duration)/1000.0);

        traceLog(strange);

        if(m_pPA->m_bMatrixError) return false;
        if (isCancelled()) return true;

        if(!m_pPlPolar->isVLM())
        {
            m_pPA->addWakeContribution();
            if(m_pPA->m_bMatrixError) return false;
        }
        if (isCancelled()) return true;

        traceStdLog("      LAPACK - LU factorization...");

        if (!m_pPA->LUfactorize())
        {
            traceStdLog(" singular matrix, aborting\n");
            m_bError = true;
            return true;
        }

        end = std::chrono::system_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        start = end;
        strange = QString::asprintf("       done in %.3f s\n", double(duration)/1000.0);

        traceLog(strange);

        traceStdLog("      Making source strengths...");
        m_pPA->makeSourceStrengths(objects::windDirection(m_Alpha, 0.0)); // unit source strengths
        traceStdLog("         done\n");

        // go through the loop at least once
        int nWakeIter = 1;
        if(m_pPolar3d->bVortonWake()) nWakeIter = std::max(nWakeIter, m_pPolar3d->VPWIterations());


        bool bViscLoopError = false;
        // initial velocity is 1 m/s, and is scaled at each iteration
        m_QInf = 1.0;
        if(m_pPlPolar->isAdjustedVelocity()) QInfStab = 1.0;

        // start the roll-up iterations
        if(s_bViscInitTwist || !bConvergedLast)     std::fill(m_gamma.begin(), m_gamma.end(), 0);


        traceStdLog("      Starting wake iterations\n");

        for(int ivw=0; ivw<nWakeIter; ivw++)
        {
            strange.clear();

            if(m_pPlPolar->bVortonWake())
                m_pPA->makeRHSVWVelocities(VVPW);

            bViscLoopError = false;
            int nViscIter = 1;
            if(m_pPlPolar->bViscousLoop()) nViscIter = std::max(nViscIter, s_ViscMaxIter);
            for(int inl=0; inl<nViscIter; inl++)
            {
                Vector3d VInf(objects::windDirection(AlphaStab, BetaStab)*m_QInf);
                std::fill(VField.begin(), VField.end(), VInf);
                if(m_pPlane->isXflType())  addTwistedVelField(m_QInf, AlphaStab, VField);

                // add the vorton induced velocities to the velocity field
                for(uint iv=0; iv<VField.size(); iv++) VField[iv] += VVPW.at(iv);

//                m_pPA->makeSourceStrengths(VField);

                m_pPA->makeRHS(VField, m_pPA->m_uRHS, nullptr);

                m_pPA->backSubUnitRHS(m_pPA->m_uRHS.data(), nullptr, nullptr, nullptr, nullptr, nullptr);

/*                if(ivw==nWakeIter-1)
                {
                    for(int ip=0; ip<m_pPA->nPanels(); ip++)
                        qDebug(" %17g   %17g", m_pPA->m_Sigma.at(ip), m_pPA->m_uRHS.at(ip));
                }*/

                if(m_pPlPolar->isQuadMethod())
                    m_pP4A->m_Mu = m_pP4A->m_uRHS;
                else
                {
                    if(m_pPlPolar->isTriLinearMethod())
                        m_pP3A->m_Mu = m_pP3A->m_uRHS;
                    if(m_pPlPolar->isTriUniformMethod())
                        m_pPA->makeVertexDoubletDensities(m_pP3A->m_uRHS, m_pP3A->m_Mu);
                }

                computeInducedForces(0, 0, m_QInf); //  the mesh has been rotated, use the x-aligned wind direction

                if(m_pPlPolar->isAdjustedVelocity())
                {
                    std::string str;
                    double v = computeBalanceSpeeds(0, mass, m_bError, "      ", str);

                    if(v<=0.0)
                    {
                        traceStdLog("   " + str);
                        bViscLoopError = true;
                        break;
                    }
                    QInfStab *= v/m_QInf;
                }
                else
                {
                    strange = "\n";
                }

                scaleResultsToSpeed(m_QInf, QInfStab);

                // scale the calculated densities - required for Cp calc.
                for(uint i=0; i<m_pPA->m_uRHS.size(); i++) m_pPA->m_uRHS[i] *= QInfStab/m_QInf;

                m_QInf = QInfStab; // set the velocity for the next iteration
                F.reset();
                if(m_pPlane->isXflType())
                {
                    for(int iw=0; iw<m_pPlane->nWings(); iw++)
                    {
                        F += m_WingForce.at(iw);
                    }
                }
                else
                {
                    F = m_WingForce.front();
                }
//                CL = F.dot(windNormal(AlphaStab, BetaStab))/m_pWPolar->referenceArea();
                CL = F.dot(objects::windNormal(0, 0))/m_pPlPolar->referenceArea();


                if(m_pPlPolar->isViscous() && m_pPlPolar->bViscousLoop())
                {
                    std::string strerr;
                    if(updateVirtualTwist(QInfStab, error, strerr))
                    {
                        str = QString::asprintf("         iter=%3d   error=%9.5f", inl+1, error);
                        str += DEGch + "   ";
                        str += QString::asprintf(" CL=%9.5f", CL);
                        traceLog(str + strange);
                    }
                    else
                    {
                        traceStdLog(strerr);
                        bViscLoopError = true;
                        break; // break viscous iterations
                    }
                }

                if(error<s_ViscAlphaPrecision) break;


                if (isCancelled()) return true;
            } // end viscous loop

            if(m_pPlPolar->isViscous() && m_pPlPolar->bViscousLoop())
            {
                if(!bViscLoopError)
                {
                    if(error<s_ViscAlphaPrecision)
                    {
                        str = "         --- Converged ---\n\n";
                        traceLog(str);
                        bConvergedLast = true;
                    }
                    else
                    {
                        str = "         Unconverged, max number of iterations exceeded, skipping operating point\n\n";
                        traceLog(str);
                        m_bError = true;
                        bConvergedLast = false;
                        bViscLoopError = true;
                    }
                }
                else
                {
                    str = "         Error in the viscous loop, skipping the operating point\n\n";
                    traceLog(str);
                    bConvergedLast = false;
                    m_bWarning = true;
                    continue;
                }
            }

            if(bViscLoopError) break; // break wake iterations
            if(m_bStopVPWIterations) break; // user requested interruption

            // advect the vortons
//            advectVortons(AlphaStab, 0, QInfStab, 0);
            if(m_pPolar3d->bVortonWake()) advectVortons(0, 0, QInfStab, 0); // v7.24 check

            // add a vorton row and clean inactive vortons
            if(m_pPolar3d->bVortonWake()) makeVortonRow(0);

            if(s_bLiveUpdate )
            {
                traceVPWLog(m_Ctrl);
            }

            if(m_pPlPolar->bVortonWake())
                traceLog(QString::asprintf("      VPW iteration %3d/%3d      CL=%9.5f\n", ivw+1, nWakeIter, CL));

            if(isCancelled()) return true;
        } // end VPW loop

        traceLog(EOLch);

        if(bViscLoopError) continue; // move on to the next operating point calculation

        computeInducedDrag(0, 0, QInfStab);


        m_pPA->makeLocalVelocities(m_pPA->m_uRHS, m_pPA->m_vRHS, m_pPA->m_wRHS,
                                   m_pPA->m_uVLocal, m_pPA->m_vVLocal, m_pPA->m_wVLocal, objects::windDirection(0, 0)*m_QInf); // only using first

        std::fill(VField.begin(), VField.end(), objects::windDirection(0, 0)*QInfStab);
        m_pPA->computeOnBodyCp(VField, m_pPA->m_uVLocal, m_pPA->m_Cp);

        strange = QString::asprintf("      Calculating plane for control parameter=%.3f\n", m_Ctrl);
        traceLog(strange);

        PlaneOpp *pPOpp = computePlane(m_Ctrl, m_Alpha, BetaStab, m_Phi, QInfStab, mass, CoG, true);
        traceStdLog(EOLstr);

        storePOpp(pPOpp);

        if (isCancelled()) return true;
    }

    return true;
}


/**
 * Makes the field of freestream velocity vectors on panels, taking into account virtual twist.
 * Used to make the RHS in the control loop.
*/
void PlaneTask::addTwistedVelField(double QInf, double alpha, std::vector<Vector3d> &VField) const
{
    if(!m_pPlane->isXflType()) return;
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);

    int iStrip = 0;
    int iPanel = 0;

    int coef = m_pPlPolar->bThinSurfaces() ? 1 : 2; // top and bottom surfaces
    if(m_pPlPolar->isTriangleMethod()) coef *= 2; // 2 triangles/quad
    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = pPlaneXfl->wingAt(iw);
        int nWingPanels = m_pPlPolar->isQuadMethod() ? pWing->nPanel4() : pWing->nPanel3();
        int iWingPanel0=iPanel;
        for(int is=0; is<pWing->nSurfaces(); is++)
        {
            Surface const &surf = pWing->surfaceAt(is);

            // left tip patch
            if(m_pPlPolar->bThickSurfaces() && surf.isClosedLeftSide())
            {
                do
                {
                    assert(m_pPA->panelAt(iPanel)->isSidePanel());
//                    VField[iPanel] = VInf; // no virtual twist corrections for tip patches
                    iPanel++;
                }
                while (m_pPA->panelAt(iPanel)->isSidePanel());
            }

            //strips
            for(int iy=0; iy<surf.NYPanels(); iy++)
            {
                double alfa = alpha + m_gamma.at(iStrip);
                Vector3d VStrip(QInf * cos(alfa*PI/180.0), 0.0, QInf * sin(alfa*PI/180.0));
                for(int ix=0; ix<surf.NXPanels()*coef; ix++)
                {
                    if(m_pPA->panelAt(iPanel)->isSidePanel()) continue; // case of a middle gap with thick surfaces
                    VField[iPanel] = VStrip;
                    iPanel++;
                }
                iStrip++;
            }

            // right tip patch
            if(m_pPlPolar->bThickSurfaces() && surf.isClosedRightSide())
            {
                // there is a right tip patch in two cases
                //  - the wing is one-sided and closed at its inner side
                //  - the wing is two-side and the surface is the right tip surface
                if ( (!pWing->isTwoSided() && pWing->isClosedInnerSide()) ||
                      (pWing->isTwoSided() && surf.isTipRight()))
                do
                {
                    assert(m_pPA->panelAt(iPanel)->isSidePanel());
//                    VField[iPanel] = VInf; // no virtual twist corrections for tip patches
                    iPanel++;
                }
                while ((iPanel-iWingPanel0)<nWingPanels && (iPanel<m_pPA->nPanels()) && (m_pPA->panelAt(iPanel)->isSidePanel()));
            }
        }
    }
//    assert(iPanel==nPanels); not true if there is a fuselage
    assert(iStrip==m_pPlane->nStations());
}


/**
 Computationally Efficient Transonic and Viscous Potential Flow Aero-Structural Method for Rapid Multidisciplinary Design Optimization of Aeroelastic Wing Shaping Control, by Eric Ting and Daniel Chaparro,
  Advanced Modeling and Simulation (AMS) Seminar Series, NASA Ames Research Center, June 28, 2017
 */
bool PlaneTask::updateVirtualTwist(double QInf, double &error, std::string &logmsg)
{
    if(!m_pPlane->isXflType()) return false;

    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pPlane);

    QString logg;
    QString strange, strong;

    Vector3d CoG; // dummy argument, unused
    Foil *pFoil0=nullptr;
    Foil *pFoil1=nullptr;

    double *Cp3Vtx=nullptr;
    int qrhs = 0;
    if(m_pPlPolar->isTriangleMethod())
    {
        Cp3Vtx = m_pP3A->m_Cp.data() + qrhs*3*m_pP3A->nPanels();
    }
    double *mu4=nullptr,*cp4=nullptr;
    if(m_pPlPolar->isQuadMethod())
    {
        mu4    = m_pP4A->m_Mu.data()    + qrhs*m_pP4A->m_Panel4.size();
        cp4    = m_pP4A->m_Cp.data()    + qrhs*m_pP4A->m_Panel4.size();
    }

    error = 0.0;

    int iStation = 0;
    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl *pWing = pPlaneXfl->wing(iw);

        // compute the lift coefficient on strips
        if(m_pPlPolar->isTriangleMethod())
        {
            pWing->panel3ComputeStrips(m_pP3A->m_Panel3, m_pPlPolar, CoG, m_Alpha, m_Beta, QInf, Cp3Vtx, m_SpanDistFF[iw]);
        }
        else if(m_pPlPolar->isQuadMethod())
        {
            // Compute aero data on strips
            pWing->panel4ComputeStrips(m_pP4A->m_Panel4, m_pPlPolar, CoG, m_Alpha, m_Beta, QInf, cp4, mu4, m_SpanDistFF[iw]);
        }

        SpanDistribs const&sd = m_SpanDistFF.at(iw);

        int m=0;
        for(int jSurf=0; jSurf<pWing->nSurfaces(); jSurf++)
        {
            Surface const &surf = pWing->surface(jSurf);
            for(int k=0; k<surf.NYPanels(); k++)
            {
                double Re = sd.m_Chord.at(m) * QInf / m_pPlPolar->viscosity();

                double Cl_vlm = sd.m_Cl.at(m);

                double tau=0;
                pWing->getFoils(&pFoil0, &pFoil1, sd.m_StripPos.at(m), tau);
                double alpha_0 = Objects2d::getZeroLiftAngle(pFoil0, pFoil1, Re, tau);

                double aoa_effective = alpha_0 + (Cl_vlm/2.0/PI) *180.0/PI - m_gamma[iStation];
                bool bOutRe=false, bOutAlpha = false;
                double Cl_visc = Objects2d::getPlrPointFromAlpha(Polar::CL, pFoil0, pFoil1, Re, aoa_effective, tau, bOutRe, bOutAlpha);

                if(bOutRe || bOutAlpha)
                {
                    strange = "       " + QString::fromStdString(pWing->name()) + QString::asprintf("  Span pos[%d]=%11g ", m, sd.m_StripPos.at(m)*Units::mtoUnit());
                    strange += Units::lengthUnitQLabel();
                    strange += ",  Re = ";
                    strong = QString::asprintf("%.0f", Re);
                    strange += strong;
                    logg += strange;
                    // interpolation error, makes no sense to continue
                    if(bOutAlpha)
                    {
                        strong = QString::asprintf(", aoa_effective=%7.3g", aoa_effective) + DEGch;
                        logg +=strong;
                    }
                    else if(bOutRe)
                    {
                        strong = QString::asprintf(", aoa_effective=%7.3g", aoa_effective) + DEGch;
                        logg += strong;
                    }
                    return false;
                }
                double delta = (Cl_visc-Cl_vlm)/2.0/PI *180.0/PI;

                error = std::max(error, fabs(delta));

                m_gamma[iStation] += delta * s_ViscRelax;
                m++; // wing station counter
                iStation++; // plane station counter
            }
        }
    }

    logmsg = logg.toStdString();
    return true;
}


void PlaneTask::outputStateMatrices(PlaneOpp const *pPOpp)
{
    QString strange, log;

    //____________________Longitudinal stability_____________

    strange = "      _____State matrices__________\n";
    log += strange;
    strange = "       Longitudinal state matrix\n";
    log += strange;
    for (int i=0; i<4; i++)
    {
        strange = QString::asprintf("        %13g      %13g      %13g      %13g\n", pPOpp->m_ALong[i][0], pPOpp->m_ALong[i][1], pPOpp->m_ALong[i][2], pPOpp->m_ALong[i][3]);
        log += strange;
    }


    //____________________Lateral stability_____________

    strange = "       Lateral state matrix\n";
    log += strange;

    for (int i=0; i<4; i++)
    {
        strange = QString::asprintf("        %13g      %13g      %13g      %13g\n", pPOpp->m_ALat[i][0], pPOpp->m_ALat[i][1], pPOpp->m_ALat[i][2], pPOpp->m_ALat[i][3]);
        log += strange;
    }

    strange ="\n";
    log += strange;

    //output control derivatives

    //build the control matrix
    for(int ie=0; ie<m_pPlPolar->nAVLCtrls(); ie++)
    {
        strange = "      _____Control Matrices for set " + QString::fromStdString(m_pPlPolar->AVLCtrl(ie).name())+" __________\n";
        log += strange;

        strange = "       Longitudinal control matrix\n";
        log += strange;

        strange = QString::asprintf("      %13g\n      %13g\n      %13g\n      %13g\n\n",  pPOpp->m_BLong[ie][0],  pPOpp->m_BLong[ie][1],  pPOpp->m_BLong[ie][2],  pPOpp->m_BLong[ie][3]);
        log += strange;

        strange = "       Lateral control matrix\n";
        log += strange;

        strange = QString::asprintf("      %13g\n      %13g\n      %13g\n      %13g\n\n", pPOpp->m_BLat[ie][0], pPOpp->m_BLat[ie][1], pPOpp->m_BLat[ie][2], pPOpp->m_BLat[ie][3]);
        log += strange;
    }

    traceLog(log);
}


PlaneOpp* PlaneTask::computePlane(double ctrl, double alpha, double beta, double phi, double QInf, double mass,
                                  Vector3d const &CoG, bool bInGeomAxes)
{
    if(QInf<PRECISION)
    {
        return nullptr; // <=0.0
    }

    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl *>(m_pPlane);

    m_QInf = QInf;
    Vector3d windD,windN;
    if(bInGeomAxes)
    {
        windD.set(1.0,0.0,0.0);
        windN.set(0.0,0.0,1.0);
    }
    else
    {
        windD.set(objects::windDirection(alpha, beta));
        windN.set(objects::windNormal(alpha, beta));
    }

    double const *mu3=(nullptr), *sigma3(nullptr), *Cp3Vtx(nullptr);
    if(m_pP3A && m_pPlPolar->isTriangleMethod())
    {
        mu3    = m_pP3A->m_Mu.data();
        sigma3 = m_pP3A->m_Sigma.data();
        Cp3Vtx = m_pP3A->m_Cp.data();
    }
    double const *mu4(nullptr), *sigma4(nullptr);
    double *cp4(nullptr);

    if(m_pPlPolar->isQuadMethod())
    {
        mu4    = m_pP4A->m_Mu.data();
        sigma4 = m_pP4A->m_Sigma.data();
        cp4    = m_pP4A->m_Cp.data();
    }

    m_AF.resetResults();
//    m_PartAF.resize(m_pPlane->nParts());

    if(m_pPlPolar->isType6())
    {
        m_AF.setOpp(alpha, m_Beta, m_Phi, QInf);
        for(int i=0; i<int(m_PartAF.size()); i++)  m_PartAF[i].setOpp(alpha, beta, m_Phi, QInf);
    }
    else
    {
        m_AF.setOpp(alpha, beta, m_pPlPolar->phi(), QInf);
        for(int i=0; i<int(m_PartAF.size()); i++)  m_PartAF[i].setOpp(alpha, beta, m_pPlPolar->phi(), QInf);
    }

    Vector3d Force, PlaneCP, Mi;

    if(pPlaneXfl)
    {
        int nWings = m_pPlane->nWings();
        Vector3d partforce;
        for(int iw=0; iw<nWings; iw++)
        {

            if(bInGeomAxes) partforce = objects::windToGeomAxes(m_WingForce.at(iw), alpha, beta);
            else            partforce = m_WingForce[iw];

//            if(m_pWPolar->isT6Polar()) partforce.y *= -1.0; // removed in v7.24 to be consistent with AVL

            Force += partforce;                           // (N/q), body axes
            m_PartAF[iw].setFff(partforce);
        }
    }


    if(pPlaneXfl)
    {
        const int nWings = m_pPlane->nWings();
        int iStation = 0;
        Vector3d partforce, partmi;
        for(int iw=0; iw<nWings; iw++)
        {
            WingXfl *pWing = pPlaneXfl->wing(iw);

            //Compute aero forces and span distributions
            if(m_pPlPolar->isQuadMethod())
            {
                pWing->panel4ComputeInviscidForces(m_pP4A->m_Panel4, m_pPlPolar, CoG, alpha, beta, QInf, cp4, mu4, m_PartAF[iw]);
                pWing->panel4ComputeStrips(m_pP4A->m_Panel4, m_pPlPolar, CoG, alpha, beta, QInf, cp4, mu4, m_SpanDistFF[iw]);
            }
            else if(m_pPlPolar->isTriangleMethod())
            {
                pWing->panel3ComputeInviscidForces(m_pP3A->m_Panel3, m_pPlPolar, CoG, alpha, beta, Cp3Vtx, m_PartAF[iw]);
                pWing->panel3ComputeStrips(m_pP3A->m_Panel3, m_pPlPolar, CoG, alpha, beta, QInf, Cp3Vtx, m_SpanDistFF[iw]);
            }

            if(bInGeomAxes)
            {
                partforce = objects::windToGeomAxes(m_PartAF.at(iw).Fsum(), alpha, beta);
                partmi    = objects::windToGeomAxes(m_PartAF.at(iw).Mi(), alpha, beta);
            }
            else
            {
                partforce = m_PartAF.at(iw).Fsum();
                partmi    = m_PartAF.at(iw).Mi();
            }

            if(m_pPlPolar->isType6()) partforce.y *= -1.0;

            m_AF.addFsum(partforce);
            Mi += partmi;           //N.m/q
            m_PartAF[iw].setFsum(partforce);
            m_PartAF[iw].setMi(partmi);           //N.m/q


            if     (m_pPlPolar->isQuadMethod())     pWing->panelComputeBending(m_pP4A->m_Panel4, m_pPlPolar->bThinSurfaces(), m_SpanDistFF[iw]);
            else if(m_pPlPolar->isTriangleMethod()) pWing->panelComputeBending(m_pP3A->m_Panel3, m_pPlPolar->bThinSurfaces(), m_SpanDistFF[iw]);

            iStation += pWing->nStations();
            if(s_bCancel) return nullptr;
        }

        if(s_bCancel) return nullptr;

        if(m_pPlPolar->isViscInterpolated())
            traceStdLog("          Adding interpolated viscous drag...\n");
        else
            traceStdLog("          Calculating XFoil viscous drag on the fly...\n");

        iStation = 0;

        if(m_pPlPolar->isViscous())
        {
            for(int iw=0; iw<nWings; iw++)
            {
                WingXfl *pWing = pPlaneXfl->wing(iw);
                std::string logmsg;

                bool bViscOK = true;

                if(m_pPlPolar->isViscInterpolated())
                {
                    traceStdLog("             Processing "+ pWing->name() + EOLstr);
                    bViscOK = computeViscousDrag(pWing, alpha, beta, QInf, m_pPlPolar, CoG, iStation, m_SpanDistFF[iw], logmsg);
                    if(logmsg.length()!=0)
                    {
                        traceStdLog("                ...Viscous interpolation failures:\n");
                        traceStdLog(logmsg);
                    }
                }
                else
                {
                    traceLog("             Processing "+
                             QString::fromStdString(pWing->name()) +": " +
                             QString::asprintf("%d surfaces", pWing->nSurfaces()) +
                             EOLch);
                    bViscOK = computeViscousDragOTF(pWing, alpha, beta, QInf, m_pPlPolar, CoG, m_pPlPolar->flapCtrls(iw), m_SpanDistFF[iw], logmsg);
                    if(logmsg.length()!=0)
                    {
                        traceStdLog("                ...XFoil OTF failures:\n");
                        traceStdLog(logmsg);
                    }
                }

                if(!bViscOK)
                {
                    m_bError = true;
                    return nullptr; // no point in calculating the rest
                }
                pWing->computeViscousForces(m_pPlPolar, alpha, beta, m_SpanDistFF[iw], m_PartAF[iw]);
                m_AF.addProfileDrag(m_PartAF.at(iw).profileDrag());           //N/q
                m_AF.addMv(m_PartAF.at(iw).Mv());                             //N.m/q

//                m_PartAF[iw].setProfileDrag(pWing->AF().profileDrag());           //N/q
//                m_PartAF[iw].setMv(pWing->AF().Mv());                             //N.m/q
            }
        }
        traceStdLog("             ...done.\n");
        if(s_bCancel) return nullptr;

        // add fuse contribution to Centre of Pressure position, to pressure moments and to viscous properties
        if(m_pPlane->hasFuse())
        {
            Fuse *pFuse = pPlaneXfl->fuse(0);
            int ipart = m_pPlane->nWings();
            if(!m_pPlPolar->bIgnoreBodyPanels())
            {
                Vector3d Force, Moment;
                if(m_pPlPolar->isTriangleMethod())
                    pFuse->computeAero(m_pP3A->m_Panel3, Cp3Vtx, m_pPlPolar, alpha, PlaneCP, Force, Moment);
                else if(m_pPlPolar->isQuadMethod())
                    pFuse->computeAero(m_pP4A->m_Panel4, cp4,    m_pPlPolar, alpha, PlaneCP, Force, Moment);

                if(bInGeomAxes)
                {
                    partforce = objects::windToGeomAxes(Force,  alpha, beta);
                    partmi    = objects::windToGeomAxes(Moment, alpha, beta);
                }
                else
                {
                    partforce = Force;
                    partmi    = Moment;
                }

                m_AF.addFsum(partforce);           // N/q
                if(m_pPlPolar->bFuseMi())
                {
                    Mi += partmi;               // N.m/q
                    m_PartAF[ipart].setFsum(partforce);                             // N/q
                    m_PartAF[ipart].setMi(partmi);                                 // N.m/q
                }
            }

            if(m_pPlPolar->hasFuseDrag())
            {
                Vector3d Force, Moment;
                pFuse->computeViscousForces(m_pPlPolar, alpha, QInf, Force, Moment);
                m_AF.addFuseDrag(Force.dot(windD));
                m_PartAF[ipart].setFuseDrag(Force.dot(windD));

//                Vector3d drag(windD*pFuse->AF().viscousDrag());

                Vector3d leverarm = (pPlaneXfl->fusePos(0)+pFuse->inertia().CoG_t())-CoG;
                m_AF.addMv(leverarm * Force);
                m_PartAF[ipart].setMv(leverarm * Force);
            }
        }
    }
    if(s_bCancel) return nullptr;

    if(m_pPlane->isSTLType())
    {
        //PlaneSTL
        if(bInGeomAxes)
            Force += objects::windToGeomAxes(m_WingForce.at(0), alpha, beta);
        else
            Force += m_WingForce.at(0);                           // (N/q), body axes

        AeroForces AF;
        if(m_pPlPolar && Cp3Vtx)
            computeInviscidAero(m_pP3A->m_Panel3, Cp3Vtx, m_pPlPolar, alpha, AF);
        m_AF.addFsum(AF.Fsum());
        Mi = AF.Mi();
    }

    m_AF.setFff(Force);          // N/q, body axes
    m_AF.setMi(Mi);

    // add all extra drag
    if(m_pPlPolar && m_pPlPolar->hasExtraDrag())
    {
        m_AF.setExtraDrag(m_pPlPolar->extraDragTotal(m_AF.CL()));
    }

    Vector3d M0;
    for(int iw=0; iw<int(m_PartAF.size()); iw++) // only one if STL
    {
        M0 += m_PartAF.at(iw).M0();
    }
    m_AF.setM0(M0);

    if(s_bCancel) return nullptr;

    PlaneOpp *pPOpp(nullptr);
    if(m_pPlPolar)
    {
        if(m_pPlPolar->isType6())     beta = -beta;

        if(m_pPlPolar->isTriangleMethod())
        {
            pPOpp = createPlaneOpp(ctrl, alpha, beta, phi, QInf, mass, CoG, Cp3Vtx, mu3, sigma3);
        }
        else if (m_pPlPolar->isQuadMethod())
        {
            pPOpp = createPlaneOpp(ctrl, alpha, beta, phi, QInf, mass, CoG, cp4, mu4, sigma4);
        }
    }
    return pPOpp;
}


void PlaneTask::computeInviscidAero(std::vector<Panel3> const &panel3, const double *Cp3Vtx,
                                    PlanePolar const *pWPolar, double Alpha,
                                    AeroForces &AF) const
{
    double panellift(0);
    Vector3d PanelForce, LeverArm;
    AF.resetResults();

    //   Define wind axes
    Vector3d WindNormal = objects::windNormal(Alpha, 0.0);

    // Inviscid pressure forces
    Vector3d FiBodyAxes(0,0,0);
    Vector3d MiBodyAxes(0,0,0);
    Vector3d M0;
    for (uint p=0; p<panel3.size(); p++)
    {
        Panel3 const &p3 = panel3.at(p);
        int index = p3.index();
        PanelForce = p3.normal() * (-Cp3Vtx[3*index]) * p3.area();     // N/q
        FiBodyAxes += PanelForce;                            // N/q

        panellift = PanelForce.dot(WindNormal);              // N/q
        M0       += p3.CoG() * panellift;                    // N.m/q

        LeverArm = p3.CoG() - pWPolar->CoG();                // m
        MiBodyAxes += LeverArm * PanelForce;                 // N.m/q
    }

    AF.setM0(M0);                           // N.m/q

    // project inviscid force on wind axes
    AF.setFsum(FiBodyAxes);
    // project inviscid moment on wind axes
    AF.setMi(MiBodyAxes);
}


void PlaneTask::scaleResultsToSpeed(double vOld, double vNew)
{
    //scale the circulation, strip force and downwash fields
    double ratio = vNew/vOld;

    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(pPlaneXfl)
    {
        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            WingXfl const*pWing = pPlaneXfl->wingAt(iw);

            for(int m=0; m<pWing->nStations(); m++)
            {
                m_SpanDistFF[iw].m_F[m]     *= ratio*ratio;
                m_SpanDistFF[iw].m_Vd[m]    *= ratio;
                m_SpanDistFF[iw].m_Gamma[m] *= ratio;
            }
        }
    }

    m_pPA->scaleResultsToSpeed(ratio);
}


double PlaneTask::computeBalanceSpeeds(double Alpha, double mass, bool &bWarning, std::string const &prefix, std::string &log)
{
    Vector3d Force, WindN;
    WindN = objects::windNormal(Alpha, m_Beta);
    Force.set(0.0,0.0,0.0);

    if(m_pPlane->isXflType())
    {
        for(int iw=0; iw<m_pPlane->nWings(); iw++)
        {
            Force += m_WingForce.at(iw);
        }
    }
    else
    {
        Force = m_WingForce.front();
    }

    if(mass<1.e-6) traceStdLog(prefix + "Warning: zero mass - not possible to set a balance speed\n");

    double Lift =  Force.dot(WindN) ;      //N/q, for 1/ms
    if(Lift<=0.0)
    {
        log = prefix + "negative lift .... skipping the angle\n";

        bWarning  = true;
        return -100.0;
    }
    double v = sqrt(2.0* 9.81 * mass/m_pPlPolar->density()/Lift);
    log = prefix + "V" + INFstr + QString::asprintf("=%7.3f ", v*Units::mstoUnit()).toStdString();
    log += Units::speedUnitLabel();
    return v;
}


double PlaneTask::computeGlideSpeed(double Alpha, double mass, std::string &log)
{
    std::string strange;
    Vector3d Force, WindNormal;

    WindNormal = objects::windNormal(Alpha, m_Beta);
    Force.set(0.0,0.0,0.0);

    for(int iw=0; iw<m_pPlane->nWings(); iw++)
    {
        Force += m_WingForce.at(iw);
    }
    double Lift =  Force.dot(WindNormal) ;      //N/q, for 1/ms
    if(Lift<=0.0)
    {
        log = ": negative lift, skipping the angle\n";

        m_bError  = true;
        return -100.0;
    }

    double CL = Lift/m_pPlPolar->referenceArea();
    double CDi = Force.dot(objects::windDirection(Alpha, m_Beta))/m_pPlPolar->referenceArea();
    double v0=0.1;    // m/s
    double v1=1000.0; // m/s
    double CDv1=0.0;

    int iter = 0;
    do
    {
        v0 = v1;
        int iStation = 0;
        CDv1 = 0.0;
        for(int iw=0; iw<m_pPlane->nWings(); iw++)
        {
            WingXfl *pWing = m_pPlane->wing(iw);
            //restore the saved results
//            pWing->setSpanDistFF(m_SpanDistFF.at(iw));
/*            computeViscousDrag(pWing, Alpha, 0.0, v0, m_pWPolar, Vector3d(), iStation);
            pWing->computeViscousForces(m_pWPolar, Alpha, m_Beta);
            CDv0 += pWing->AF().profileDrag();*/

            if(m_pPlPolar->isViscOnTheFly())
                computeViscousDragOTF(pWing, Alpha, 0.0, v1, m_pPlPolar, Vector3d(), m_pPlPolar->flapCtrls(iw), m_SpanDistFF[iw], strange);
            else
                computeViscousDrag(pWing, Alpha, 0.0, v1, m_pPlPolar, Vector3d(), iStation, m_SpanDistFF[iw], strange);
            pWing->computeViscousForces(m_pPlPolar, Alpha, m_Beta, m_SpanDistFF[iw], m_PartAF[iw]);
            CDv1 += m_PartAF.at(iw).profileDrag(); // N/q

            iStation += pWing->nStations();
        }
//        CDv0 /= m_pWPolar->referenceArea();
        CDv1 /= m_pPlPolar->referenceArea();

//        v0 = sqrt(2*mass*9.81/m_pWPolar->density()/m_pWPolar->referenceArea()/sqrt(CL*CL+(CDi+CDv0)*(CDi+CDv0)));
        v1 = sqrt(2*mass*9.81/m_pPlPolar->density()/m_pPlPolar->referenceArea()/sqrt(CL*CL+(CDi+CDv1)*(CDi+CDv1)));

        iter++;
    }
    while(fabs(v1-v0)>0.01 && iter<100);

    if(fabs(v1-v0)>0.01)
    {
        log = ": could not find a stabilized velocity, skipping the angle";

        m_bError  = true;
        return -100.0;
     }
    else
    {
        log = "V" + INFstr + QString::asprintf("= %g ", v1*Units::mstoUnit()).toStdString() + Units::speedUnitLabel();
        log += QString::asprintf(" converged in %d iterations", iter).toStdString();
    }

    return v1;
}


void PlaneTask::computeInducedForces(double alpha, double beta, double QInf)
{
    if(m_pPlane->isXflType())
    {
        int nWings = m_pPlane->nWings();
        int pos = 0;
        for(int iw=0; iw<nWings; iw++)
        {
            WingXfl *pWing = m_pPlane->wing(iw);
            Vector3d forcebodyaxes;
            if(m_pPlPolar->isQuadMethod())
                m_pP4A->inducedForce(pWing->nPanel4(), QInf, alpha, beta, pos, forcebodyaxes, m_SpanDistFF[iw]);
            else if(m_pP3A && m_pPlPolar->isTriangleMethod())
                m_pP3A->inducedForce(pWing->nPanel3(), QInf, alpha, beta, pos, forcebodyaxes, m_SpanDistFF[iw]);

            //save the results... will save another FF calculation when computing the operating points
            m_WingForce[iw] = forcebodyaxes;     // N/q, body axes
//            m_SpanDistFF[iw] = pWing->spanDistFF();

            if      (m_pPlPolar->isTriangleMethod()) pos += pWing->nPanel3();
            else if (m_pPlPolar->isQuadMethod())     pos += pWing->nPanel4();

            if(isCancelled())return;
        }
    }
    else
    {
        m_WingForce[0].reset();
        m_pP3A->inducedForce(m_pPlane->nPanel3(), QInf, alpha, beta, 0, m_WingForce[0], m_SpanDistFF[0]);
    }
}


void PlaneTask::computeInducedDrag(double alpha, double beta, double QInf)
{
    Vector3d FFForceBodyAxes;

    PanelAnalysis::clearDebugPts();

    if(m_pPlane->isXflType())
    {
        int nWings = m_pPlane->nWings();

        int m0 = 0;
        int pos = 0;
        for(int iw=0; iw<nWings; iw++)
        {
            WingXfl *pWing = m_pPlane->wing(iw);
            if(m_pPolar3d->bVortonWake())
            {
                m_pPA->vortonDrag(alpha, beta, QInf, m0, pWing->nStations(), FFForceBodyAxes, m_SpanDistFF[iw]);
                m0 += pWing->nStations();
            }
            else
            {
                if(m_pPlPolar->isQuadMethod())
                    m_pP4A->trefftzDrag(pWing->nPanel4(), QInf, alpha, beta, pos, FFForceBodyAxes, m_SpanDistFF[iw]);
                else if(m_pPlPolar->isTriangleMethod())
                    m_pP3A->trefftzDrag(pWing->nPanel3(), QInf, alpha, beta, pos, FFForceBodyAxes, m_SpanDistFF[iw]);
            }
            m_WingForce[iw] += FFForceBodyAxes;     // N/q, body axes

            if      (m_pPlPolar->isTriangleMethod()) pos += pWing->nPanel3();
            else if (m_pPlPolar->isQuadMethod())     pos += pWing->nPanel4();

            if(isCancelled())return;
        }
    }
    else
    {
        // STL case
        if(m_pPolar3d->bVortonWake())
            m_pPA->vortonDrag(alpha, beta, QInf, 0, m_pPlane->nStations(), FFForceBodyAxes, m_SpanDistFF[0]);
        else
            m_pP3A->trefftzDrag(m_pPlane->nPanel3(), QInf, alpha, beta, 0, FFForceBodyAxes, m_SpanDistFF[0]);
        m_WingForce[0] += FFForceBodyAxes;     // N/q, body axes
    }
}


PlaneOpp *PlaneTask::createPlaneOpp(double ctrl, double alpha, double beta, double phi, double QInf, double mass, Vector3d const &CoG,
                                    double const *Cp, double const *Gamma, double const *Sigma, bool bCpOnly) const
{
    PlaneXfl const *pPlaneXfl = nullptr;
    if(m_pPlane->isXflType()) pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);

    int nPanel3=0, nPanel4=0;
    if(m_pPlPolar->isTriangleMethod()) nPanel3=m_pP3A->nPanels();
    else                              nPanel4=m_pP4A->nPanels();

    PlaneOpp *pPOpp = new PlaneOpp(m_pPlane, m_pPlPolar, nPanel4, nPanel3);
    if(!pPOpp) return nullptr;

    pPOpp->setTheStyle(m_pPlPolar->theStyle());
    pPOpp->setVisible(true);

    PlaneOpp *pLastPOpp(nullptr);
    if(m_PlaneOppList.size()) pLastPOpp = m_PlaneOppList.back();
    if(pLastPOpp) pPOpp->setLineColor(pLastPOpp->lineColor().darker(100));

    int iStation = 0;
    if(pPlaneXfl)
    {
        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            WingXfl const*pWing = pPlaneXfl->wingAt(iw);

            pPOpp->addWingOpp(pWing->nPanel4());
            pPOpp->m_WingOpp[iw].createWOpp(pWing, m_pPlPolar, m_SpanDistFF.at(iw), m_PartAF.at(iw));

            if(m_pPlPolar->isViscous() && m_pPlPolar->bViscousLoop())
            {
                //save the virtual twist for the operating point display
                SpanDistribs &sd = pPOpp->m_WingOpp[iw].m_SpanDistrib;
                for(int is=0; is<pWing->nStations(); is++)
                {
                    sd.m_VTwist[is] = m_gamma.at(iStation);
                    iStation++;
                }
            }
        }
    }

    WingOpp *pWOpp(nullptr);
    if(pPOpp->hasWOpp())
        pWOpp = &pPOpp->m_WingOpp[0];

    if(m_pPlPolar->isTriangleMethod())
    {
        int N = m_pP3A->nPanels();
        int N3 = N*3;
        if(Cp)    memcpy(pPOpp->m_Cp.data(),    Cp,      ulong(N3)*sizeof(double));
        if(Gamma) memcpy(pPOpp->m_gamma.data(), Gamma,   ulong(N3)*sizeof(double));
        if(Sigma) memcpy(pPOpp->m_sigma.data(), Sigma,   ulong(N)*sizeof(double));
    }
    else if (m_pPlPolar->isQuadMethod())
    {
        int N = m_pP4A->nPanels();
        if(Cp)    memcpy(pPOpp->m_Cp.data(),    Cp,    ulong(N)*sizeof(double));
        if(Gamma) memcpy(pPOpp->m_gamma.data(), Gamma, ulong(N)*sizeof(double));
        if(Sigma) memcpy(pPOpp->m_sigma.data(), Sigma, ulong(N)*sizeof(double));
    }


    pPOpp->m_bOut   = false; // v7.50: only keeping successful viscous calculations

    pPOpp->m_Mass   = mass;
    pPOpp->m_CoG    = CoG;
    pPOpp->m_Inertia[0] = m_pPlPolar->Ixx();
    pPOpp->m_Inertia[1] = m_pPlPolar->Iyy();
    pPOpp->m_Inertia[2] = m_pPlPolar->Izz();
    pPOpp->m_Inertia[3] = m_pPlPolar->Ixz();


    pPOpp->setAlpha(alpha);
    pPOpp->setBeta(beta);
    pPOpp->setPhi(phi);
    pPOpp->setQInf(QInf);
    pPOpp->setCtrl(ctrl);

    pPOpp->m_bGround      = m_pPolar3d->bGroundEffect();
    pPOpp->m_bFreeSurface = m_pPolar3d->bFreeSurfaceEffect();
    pPOpp->m_GroundHeight = m_pPolar3d->groundHeight();

    pPOpp->m_AF = m_AF;

    if(bCpOnly) return pPOpp;

    int pos = 0;

    if(pPlaneXfl)
    {
        for(int iw=0;iw<pPlaneXfl->nWings(); iw++)
        {
            WingXfl const*pWing = pPlaneXfl->wingAt(iw);
            if(pWing && pPOpp->nWOpps()>iw)
            {
                pPOpp->m_WingOpp[iw].m_AF = m_PartAF.at(iw);

                pPOpp->m_WingOpp[iw].m_FlapMoment.clear();
                for (int i=0; i<pWing->nFlaps(); i++)
                {
                    pPOpp->m_WingOpp[iw].m_FlapMoment.push_back(pWing->flapMomentAt(i));
                }

                pPOpp->m_WingOpp[iw].m_dCp    = pPOpp->m_Cp.data()    + pos;
                pPOpp->m_WingOpp[iw].m_dG     = pPOpp->m_gamma.data() + pos;
                pPOpp->m_WingOpp[iw].m_dSigma = pPOpp->m_sigma.data() + pos;
                pos += pWing->nPanel3();
            }
        }
    }

    if(pPlaneXfl)
    {
        if(pPlaneXfl->hasMainWing())
        {
            double Cb = 0.0;
            for (int l=0; l<pPlaneXfl->wingAt(0)->nStations(); l++)
            {
                if(fabs(m_SpanDistFF[0].m_BendingMoment[l])>fabs(Cb)) Cb = m_SpanDistFF[0].m_BendingMoment[l];
            }
            if(pWOpp) pWOpp->m_MaxBending = Cb;
        }
    }

    pPOpp->m_FuseAF.clear();
    if(pPlaneXfl)
    {
        for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++)
        {
    //        pPOpp->m_FuseAF.push_back(m_pPlane->fuse(ifuse)->AF());
            pPOpp->m_FuseAF.push_back(m_PartAF.at(m_pPlane->nWings()+ifuse));
        }
    }

    if(m_pPlPolar->bVortonWake())
    {
        pPOpp->m_Vorton = m_pPA->m_Vorton;
        pPOpp->m_VortexNeg = m_pPA->m_VortexNeg;
    }
    return pPOpp;
}


bool PlaneTask::T7Loop()
{
    QString str, outstring;

    traceStdLog("\nSolving the problem... \n\n");

    m_pPA->m_nStations = m_pPlane->nStations();// for assertion checks only?

    m_nRHS = 1;
    m_qRHS = 0;

    m_Ctrl = 0.0;
    m_Beta = 0.0;
    m_Phi  = m_pPlPolar->phi();

    m_pPA->makeWakePanels(objects::windDirection(0,0), false);

    setLinearSolution();

    traceStdLog("   Making unit panel velocities...");
    m_pPA->makeLocalVelocities(m_pPA->m_uRHS, m_pPA->m_vRHS, m_pPA->m_wRHS, m_pPA->m_uVLocal, m_pPA->m_vVLocal, m_pPA->m_wVLocal, objects::windDirection(0,0));
    traceStdLog("   done\n");

    Vector3d CoG = m_pPlPolar->CoGCtrl(m_Ctrl);
    double mass = m_pPlPolar->massCtrl(m_Ctrl);

    outstring.clear();

    traceLog(outstring);
    if(isCancelled()) return true;

    // next find the balanced and trimmed conditions
    double AlphaEq(0), u0(0);

    traceStdLog("      Calculating trimmed conditions\n");

    if(!m_pPA->computeTrimmedConditions(mass, CoG, AlphaEq, u0, m_pPlPolar->bFuseMi()))
    {
        if(isCancelled()) return true;
        //no zero moment alpha
        str = QString::asprintf("      Unsuccessful attempt to trim the model for control position = %.2f - skipping.\n\n\n", m_Ctrl);
        traceLog(str);
        m_bError = true;
    }
    else
    {
        m_QInf = u0;

        traceStdLog("      Calculating far field forces...\n");
        computeInducedForces(AlphaEq, m_Beta, 1.0);
        computeInducedDrag(AlphaEq, m_Beta, 1.0);
        scaleResultsToSpeed(1.0, u0);

        if (isCancelled()) return true;

        str = "      Calculating Plane for "+ALPHAch + QString::asprintf("=%.2f", AlphaEq) + DEGch + EOLch;
        traceLog(str);
        PlaneOpp *pPOpp = computePlane(m_Ctrl, AlphaEq, m_Beta, m_Phi, u0, mass, CoG, false);
        if(!pPOpp)
        {
            traceStdLog("           Error generating the operating point... discarding\n\n");
            return true;
        }

        if (isCancelled()) return true;

        if(computeStability(pPOpp, true))
        {          
            storePOpp(pPOpp);
        }
        traceStdLog("       Done operating point\n\n");
    }

    return true;
}


bool PlaneTask::computeStability(PlaneOpp *pPOpp, bool bOutput)
{
    std::string str;
    // Compute stability and control derivatives in stability axes
    traceStdLog("             Calculating stability derivatives\n");
    m_pPA->computeStabilityDerivatives(pPOpp->alpha(), pPOpp->QInf(), pPOpp->cog(), m_pPlPolar->bFuseMi(), pPOpp->m_SD, m_Force0, m_Moment0);
    if(isCancelled()) return true;

    pPOpp->m_SD.resizeControlDerivatives(m_pPlPolar->nAVLCtrls());

    if(m_pPlPolar->hasActiveAVLControl())
    {
        computeControlDerivatives(m_Ctrl, pPOpp->alpha(), pPOpp->QInf(), pPOpp->m_SD); //single derivative, w.r.t. the polar's control variable
    }
    if(isCancelled()) return true;


    pPOpp->m_SD.setMetaData(m_pPlPolar->referenceSpanLength(), m_pPlPolar->referenceChordLength(), m_pPlPolar->referenceArea(),
                            pPOpp->QInf(), pPOpp->mass(), pPOpp->cog().x, m_pPlPolar->density());
    pPOpp->m_SD.computeNDStabDerivatives();
    if (bOutput) outputNDStabDerivatives(pPOpp->QInf(), pPOpp->m_SD);


    // build the state matrices
    if(pPOpp->mass()<PRECISION)
    {
        traceStdLog("          Null mass - skipping calculation of eigenthings\n");
    }
    else
    {
        traceStdLog("          Calculating eigenthings\n");
        // Compute inertia in stability axes
        m_pPlPolar->Ixx();

        pPOpp->computeStabilityInertia(m_pPlPolar->inertia());

        pPOpp->buildStateMatrices(m_pPlPolar->nAVLCtrls());
        if(bOutput) outputStateMatrices(pPOpp);

        // Solve for eigenvalues

        if(!pPOpp->solveEigenvalues(str))
        {
            traceStdLog("             Unsuccessful attempt to compute eigenvalues\n");
            if(m_pPlPolar->isType7()) m_bError = true;
            else m_bWarning = true;
        }
        else
        {
            // output the eigenvalues
            traceStdLog("          Eigenthings computed successfully\n");
            if (bOutput)
            {
                pPOpp->outputEigen(str);
                traceStdLog(str+EOLstr);
            }
        }
    }
    return true;
}


bool PlaneTask::T123458Loop()
{
    QString strange, str, outstring;

    traceStdLog("\nSolving the problem... \n\n");

    m_pPA->m_nStations = m_pPlane->nStations();// for assertion checks only?

    m_pPA->makeWakePanels(objects::windDirection(0,0), false);
    m_pPA->savePanels();

    setLinearSolution();

    traceStdLog("   Making unit panel velocities... ");
    int N = int(m_pPA->m_uVLocal.size());
    std::vector<Vector3d> uVLocal(N), vVLocal(N), wVLocal(N); // keep a copy since these velocity arrays are modified during the calculation of derivatives
    m_pPA->makeLocalVelocities(m_pPA->m_uRHS, m_pPA->m_vRHS, m_pPA->m_wRHS, uVLocal, vVLocal, wVLocal, objects::windDirection(0,0));

    traceStdLog("     done\n");

    Vector3d CoG = m_pPlPolar->CoGCtrl(m_Ctrl);
    double mass = m_pPlPolar->massCtrl(m_Ctrl);

    if(isCancelled()) return true;

    m_nRHS = int(m_T8Opps.size());
    strange = QString::asprintf("   Calculating %d operating point", m_nRHS);
    if(m_nRHS>1) strange +="s";
    traceLog(EOLch+strange+EOLch);

    for(uint io=0; io<m_T8Opps.size(); io++)
    {
        if(s_bCancel)
        {
            m_AnalysisStatus = xfl::CANCELLED;
            return false;
        }

        T8Opp const &t8opp = m_T8Opps.at(io);

        if(!t8opp.isActive()) continue;
        m_qRHS = io;
        m_Alpha = t8opp.alpha();
        m_Beta  = t8opp.beta();
        m_Phi   = m_pPlPolar->phi();
        m_QInf  = t8opp.Vinf();
        m_Ctrl  = 0.0;

        outstring = "     ";
        outstring += ALPHAch + QString::asprintf("=%g", m_Alpha) + DEGch + ", ";
        outstring += BETAch  + QString::asprintf("=%g", m_Beta)  + DEGch + ", ";
        outstring += PHIch   + QString::asprintf("=%g", m_Phi)   + DEGch + ", ";

        if(m_pPlPolar->isType1() || m_pPlPolar->isType4() || m_pPlPolar->isType5() || m_pPlPolar->isType8())
            outstring += "V" + INFch + QString::asprintf("=%g ", m_QInf*Units::mstoUnit()) + Units::speedUnitQLabel() + EOLch;
        else
            outstring += "V" + INFch + ": adjusted" + EOLch;

        traceLog(outstring);

        traceStdLog("       Creating source strengths...\n");
        m_pPA->makeSourceStrengths(objects::windDirection(m_Alpha, m_Beta));

        traceStdLog("       Calculating doublet strengths...\n");
        m_pPA->makeUnitDoubletStrengths(m_Alpha, m_Beta);

        traceStdLog("       Calculating far field forces...\n");

        computeInducedForces(m_Alpha, m_Beta, 1.0);
        computeInducedDrag(  m_Alpha, m_Beta, 1.0);

        if(m_pPlPolar->isType1() || m_pPlPolar->isType5())
        {
            m_QInf = m_pPlPolar->velocity();
        }
        else if(m_pPlPolar->isType2() || m_pPlPolar->isType3())
        {
            std::string str;
            traceStdLog("       Calculating balance speeds...\n");
            if (m_pPlPolar->isFixedLiftPolar())
                m_QInf = computeBalanceSpeeds(m_Alpha, m_pPlPolar->mass(), m_bError, "", str);
            else if(m_pPlPolar->isGlidePolar())
                m_QInf = computeGlideSpeed(m_Alpha, m_pPlPolar->mass(), str);
            strange = "             " + QString::fromStdString(str) + EOLch;
            traceLog(strange);
        }
        else if(m_pPlPolar->isType4())
        {
            m_QInf = t8opp.m_Vinf;
        }
        else if(m_pPlPolar->isType8())
        {
            m_QInf = t8opp.m_Vinf;
        }
        else
        {
            assert(false);
        }

        if(m_QInf<0) continue;

        scaleResultsToSpeed(1.0, m_QInf);

        if (isCancelled()) return true;

        traceStdLog("       Calculating on-body pressure coefficients...\n");
        std::vector<Vector3d> VLocal;
        std::vector<Vector3d> VInf(m_pPA->nPanels());
        std::fill(VInf.begin(), VInf.end(), objects::windDirection(m_Alpha, m_Beta));

        // Save a little time by restoring the unit velocty fields instead of recalculating them
        m_pPA->m_uVLocal = uVLocal;
        m_pPA->m_vVLocal = vVLocal;
        m_pPA->m_wVLocal = wVLocal;

        m_pPA->combineLocalVelocities(m_Alpha, m_Beta, VLocal);
        m_pPA->computeOnBodyCp(VInf, VLocal, m_pPA->m_Cp);
        if (isCancelled()) return true;

        str = "       Calculating plane\n";
        traceLog(str);
        PlaneOpp *pPOpp = computePlane(m_Ctrl, m_Alpha, m_Beta, m_pPlPolar->phi(), m_QInf, mass, CoG, false);
        if(!pPOpp)
        {
            traceStdLog("\n          Error generating the operating point... discarding\n\n");
            continue;
        }

        if (isCancelled()) return true;

        if(m_bDerivatives)
        {
            traceStdLog("          Calculating derivatives and eigenthings\n");
            computeStability(pPOpp, true);
        }
        else
        {
            traceStdLog("          Skipping derivatives and eigenthings\n");
        }

        storePOpp(pPOpp);

        traceStdLog("          Done operating point\n\n");
    }

    return true;
}


void PlaneTask::storePOpp(PlaneOpp *pPOpp)
{
    if(!pPOpp) return;

    if(!pPOpp->isOut()) // discard failed visc interpolated opps
        m_pPlPolar->addPlaneOpPointData(pPOpp);

    if(m_bKeepOpps)
    {
        m_PlaneOppList.push_back(pPOpp);
        Objects3d::insertPlaneOpp(pPOpp);
    }
    else            delete pPOpp;

}


void PlaneTask::outputNDStabDerivatives(double u0, StabDerivatives const &SD)
{

    double q = 1./2. * m_pPlPolar->density() * u0 * u0;
    double b   = m_pPlPolar->referenceSpanLength();
    double S   = m_pPlPolar->referenceArea();
    double mac = m_pPlane->mac();

    QString str;
    QString prefix("                ");
    QString logmsg;

    // no OpPoint, we output the data to the log file
    str = "             Longitudinal derivatives\n";
    logmsg += str;
    str = QString::asprintf("Xu  = %11.5g         Cxu  = %11.5g\n", SD.Xu,SD.CXu);
    logmsg += prefix + str;
    str = QString::asprintf("Xw  = %11.5g         Cxa  = %11.5g\n", SD.Xw,SD.CXa);
    logmsg += prefix + str;
    str = QString::asprintf("Xq  = %11.5g         Cxq  = %11.5g\n", SD.Xq, SD.CXq);
    logmsg += prefix + str;
    str = QString::asprintf("Zu  = %11.5g         Czu  = %11.5g\n", SD.Zu, SD.CZu);
    logmsg += prefix + str;
    str = QString::asprintf("Zw  = %11.5g         CZa  = %11.5g\n", SD.Zw, SD.CZa);
    logmsg += prefix + str;
    str = QString::asprintf("Zq  = %11.5g         CZq  = %11.5g\n", SD.Zq, SD.CZq);
    logmsg += prefix + str;
    str = QString::asprintf("Mu  = %11.5g         Cmu  = %11.5g\n", SD.Mu, SD.Cmu);
    logmsg += prefix + str;
    str = QString::asprintf("Mw  = %11.5g         Cma  = %11.5g\n", SD.Mw, SD.Cma);
    logmsg += prefix + str;
    str = QString::asprintf("Mq  = %11.5g         Cmq  = %11.5g\n", SD.Mq, SD.Cmq);
    logmsg += prefix + str;

    str = QString::asprintf("Neutral Point position = %g ", SD.XNP*Units::mtoUnit()) + Units::lengthUnitQLabel();
    str +="\n\n";
    logmsg += prefix + str;

    str = "             Lateral derivatives\n";
    logmsg += str;
    str = QString::asprintf("Yv  = %11.5g         CYb  = %11.5g\n", SD.Yv, SD.CYb);
    logmsg += prefix + str;
    str = QString::asprintf("Yp  = %11.5g         CYp  = %11.5g\n", SD.Yp, SD.CYp);
    logmsg += prefix + str;
    str = QString::asprintf("Yr  = %11.5g         CYr  = %11.5g\n", SD.Yr, SD.CYr);
    logmsg += prefix + str;
    str = QString::asprintf("Lv  = %11.5g         Clb  = %11.5g\n", SD.Lv, SD.Clb);
    logmsg += prefix + str;
    str = QString::asprintf("Lp  = %11.5g         Clp  = %11.5g\n", SD.Lp, SD.Clp);
    logmsg += prefix + str;
    str = QString::asprintf("Lr  = %11.5g         Clr  = %11.5g\n", SD.Lr, SD.Clr);
    logmsg += prefix + str;
    str = QString::asprintf("Nv  = %11.5g         Cnb  = %11.5g\n", SD.Nv, SD.Cnb);
    logmsg += prefix + str;
    str = QString::asprintf("Np  = %11.5g         Cnp  = %11.5g\n", SD.Np, SD.Cnp);
    logmsg += prefix + str;
    str = QString::asprintf("Nr  = %11.5g         Cnr  = %11.5g\n", SD.Nr, SD.Cnr);
    logmsg += prefix + str + EOLch;

    //output control derivatives
    if(!m_pPlPolar->hasActiveAVLControl())
    {
        traceLog(logmsg);
        return;
    }

    str = "             Control derivatives\n";
    logmsg += str;
    for(int ie=0; ie<m_pPlPolar->nAVLCtrls(); ie++)
    {
        if(int(SD.ControlNames.size())>ie)
        {
            logmsg += prefix + QString::fromStdString(SD.ControlNames.at(ie))  + EOLch;
            logmsg += prefix + QString::asprintf("Xde = %11.5g         CXde = %11.5g\n", SD.Xde.at(ie), SD.Xde.at(ie)/(q*S));
            logmsg += prefix + QString::asprintf("Yde = %11.5g         CYde = %11.5g\n", SD.Yde.at(ie), SD.Yde.at(ie)/(q*S));
            logmsg += prefix + QString::asprintf("Zde = %11.5g         CZde = %11.5g\n", SD.Zde.at(ie), SD.Zde.at(ie)/(q*S));
            logmsg += prefix + QString::asprintf("Lde = %11.5g         CLde = %11.5g\n", SD.Lde.at(ie), SD.Lde.at(ie)/(q*S*b));
            logmsg += prefix + QString::asprintf("Mde = %11.5g         CMde = %11.5g\n", SD.Mde.at(ie), SD.Mde.at(ie)/(q*S*mac));
            logmsg += prefix + QString::asprintf("Nde = %11.5g         CNde = %11.5g\n", SD.Nde.at(ie), SD.Nde.at(ie)/(q*S*b));
        }
    }
    str ="\n";
    logmsg += str;

    traceLog(logmsg);
}


void PlaneTask::computeControlDerivatives(double t7ctrl, double alphaeq, double u0, StabDerivatives &SD)
{
    Vector3d WindDirection, Force, Moment, V0, is, js, ks;

    int N = 0;
    if      (m_pPlPolar->isQuadMethod())     N = m_pP4A->nPanels();
    else if (m_pPlPolar->isTriangleMethod()) N = m_pP3A->nPanels();

    double const *sigma = nullptr;
    if      (m_pPlPolar->isQuadMethod())     sigma = m_pP4A->m_Sigma.data();
    else if (m_pPlPolar->isTriangleMethod()) sigma = m_pP3A->m_Sigma.data();

    // Define the stability axes and the freestream velocity field
    double cosa = cos(alphaeq*PI/180.0);
    double sina = sin(alphaeq*PI/180.0);
    V0.set(u0*cosa, 0.0, u0*sina);
    WindDirection.set(cosa, 0.0, sina);
    is.set(-cosa, 0.0, -sina);
    js.set(  0.0, 1.0,   0.0);
    ks.set( sina, 0.0, -cosa);

    double DeltaCtrl = 0.001;

    for(int ie=0; ie<m_pPlPolar->nAVLCtrls(); ie++)
    {
        traceStdLog("             Processing control set " + m_pPlPolar->AVLCtrl(ie).name() + EOLstr);
        SD.ControlNames[ie] = m_pPlPolar->AVLCtrl(ie).name();
        if(!m_pPlPolar->AVLCtrl(ie).hasActiveAngle())
        {
            SD.Xde[ie] = SD.Yde[ie] = SD.Zde[ie] = SD.Lde[ie] = SD.Mde[ie] = SD.Nde[ie] = 0.0;
            traceStdLog("        No active gain... skipping\n\n");
            continue;
        }

        std::string outstring;
        m_pPA->restorePanels();
        if(m_pPlane->isXflType())
        {
            PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pPlane);
            if(m_pP4A)
            {
                setControlPositions(pPlaneXfl, m_pPlPolar, m_pP4A->m_Panel4, DeltaCtrl, ie, outstring);
            }
            else if(m_pP3A)
            {
                setControlPositions(pPlaneXfl, m_pPlPolar, m_pP3A->m_Panel3,
                                    m_pP3A->m_pRefTriMesh->nodes(), DeltaCtrl, ie, outstring);
            }
        }
        m_pPA->makeWakePanels(objects::windDirection(0,0), false);

        //create the RHS
//        traceStdLog("         Making the unit RHS vectors...\n");
        std::vector<Vector3d> VField(N, V0);
//        Vector3d Omega;
//        m_pPA->makeUnitRHSVectors(); // could also use one RHS
//        m_pPA->combineUnitRHS(m_pPA->m_cRHS, V0, Omega);

        m_pPA->makeRHS(VField, m_pPA->m_cRHS, nullptr);

/*        std::string strong = "         Calculating the control derivatives\n\n";
        traceStdLog(strong);*/

        //LU solve
        m_pPA->backSubRHS(m_pPA->m_cRHS);

        // make node vertex array
        // only needed for triuniform method to align with TriLinAnalysis
        std::vector<double> cRHSVertex;
        double *muc = nullptr;
        Vector3d CoG = m_pPlPolar->CoGCtrl(t7ctrl);
        Vector3d VInf = objects::windDirection(alphaeq, 0.0) * u0;

        if(m_pPlPolar->isQuadMethod())
        {
            muc = m_pPA->m_cRHS.data();
            m_pPA->forces(muc, sigma, alphaeq, 0.0, CoG, m_pPlPolar->bFuseMi(), VField, Force, Moment);
        }
        else if(m_pPlPolar->isTriangleMethod())
        {
            std::vector<double> notanrhs(m_pPA->m_cRHS.size());
            m_pPA->makeLocalVelocities(m_pPA->m_cRHS, notanrhs, notanrhs, m_pPA->m_uVLocal, m_pPA->m_vVLocal, m_pPA->m_wVLocal, objects::windDirection(alphaeq, 0.0));
            if(m_pPlPolar->isTriUniformMethod() && m_pP3A)
            {
                cRHSVertex.resize(3*N);
                m_pP3A->makeVertexDoubletDensities(m_pPA->m_cRHS, cRHSVertex);
                muc = cRHSVertex.data();
            }
            else if(m_pPlPolar->isTriLinearMethod())
            {
                muc = m_pPA->m_cRHS.data();
            }

            std::fill(VField.begin(), VField.end(), VInf);
            m_pPA->computeOnBodyCp(VField, m_pPA->m_uVLocal, m_pPA->m_Cp);
            m_pPA->forces(muc, sigma, alphaeq, 0.0, CoG, m_pPlPolar->bFuseMi(), VField, Force, Moment);
        }

        // make the forward difference with nominal results
        // which gives the stability derivative for a rotation equal to deltaCtrl x gain
        SD.Xde[ie] = (Force  - m_Force0 ).dot(is) / DeltaCtrl;
        SD.Yde[ie] = (Force  - m_Force0 ).dot(js) / DeltaCtrl;
        SD.Zde[ie] = (Force  - m_Force0 ).dot(ks) / DeltaCtrl;
        SD.Lde[ie] = (Moment - m_Moment0).dot(is) / DeltaCtrl;  // N.m/ctrl
        SD.Mde[ie] = (Moment - m_Moment0).dot(js) / DeltaCtrl;
        SD.Nde[ie] = (Moment - m_Moment0).dot(ks) / DeltaCtrl;
    }

    m_pPA->restorePanels();
}


/** Sets the angle positions of wings and flap for a stability analysis */
void PlaneTask::setControlPositions(PlaneXfl const*pPlaneXfl, PlanePolar const*pWPolar,
                                    std::vector<Panel4> &panel4, double deltactrl,
                                    int iAVLCtrl,
                                    std::string &outstring)
{
    QString outstr;
    QString strange;
    double totalAngle=0.0, deltaangle = 0.0;
    double gain = 0.0;

    int nCtrl = 0;

    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = pPlaneXfl->wingAt(iw);
        int iCtrl=0;
        for (int jSurf=0; jSurf<pWing->nSurfaces(); jSurf++)
        {
            Surface const &surf = pWing->surfaceAt(jSurf);
            if(surf.hasTEFlap())
            {
                if(iAVLCtrl>=0 && iAVLCtrl<pWPolar->nAVLCtrls()) gain = pWPolar->AVLGain(iAVLCtrl, nCtrl);
                else                                             gain = 0.0;
                deltaangle = deltactrl * gain; // degrees

                if (fabs(deltaangle)>ANGLEPRECISION)
                {
                    //Add delta rotations to initial control setting and to wing or flap delta rotation
                    if(fabs(surf.foilA()->TEFlapAngle())>0.0 && fabs(surf.foilB()->TEFlapAngle())>0.0)
                        totalAngle = deltaangle + (surf.foilA()->TEFlapAngle() + surf.foilB()->TEFlapAngle())/2.0;
                    else
                        totalAngle = deltaangle;

                    strange = QString::asprintf("- rotating flap %d by %g°, total flap angle is %g°", iCtrl, deltaangle, totalAngle);

                    strange = "      " + QString::fromStdString(pWing->name()) + strange + EOLch;
                    outstr +=strange;

                    for(uint i4=0; i4<panel4.size(); i4++)
                    {
                        if(surf.hasFlapPanel4(i4))
                        {
                            panel4[i4].rotate(surf.hingePoint(), surf.hingeVector(), deltaangle);
                        }
                    }
                }
                iCtrl++;
                nCtrl++;
            }
        }
    }

    outstr  +="\n";
    outstring = outstr.toStdString();
}


/** Sets the angle positions of wings and flap for a stability analysis */
void PlaneTask::setControlPositions(PlaneXfl const *pPlaneXfl, PlanePolar const *pWPolar,
                                    std::vector<Panel3> &panel3, std::vector<Node> const &refnodes, double deltactrl, int iAVLCtrl,
                                    std::string &outstring)
{
    QString strange;
    QString outstr;
    double totalAngle(0.0), deltaangle(0);
    double gain(0);
    std::vector<Node> node = refnodes;

    int nCtrl = 0;
    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = pPlaneXfl->wingAt(iw);
        int iCtrl=0;

        for (int jSurf=0; jSurf<pWing->nSurfaces(); jSurf++)
        {
            Surface const &surf = pWing->surfaceAt(jSurf);
            if(surf.hasTEFlap())
            {
                if(iAVLCtrl>=0 && iAVLCtrl<pWPolar->nAVLCtrls()) gain = pWPolar->AVLGain(iAVLCtrl, nCtrl);
                else                                             gain = 0;
                deltaangle = gain * deltactrl; // degrees

                if(fabs(deltaangle)>ANGLEPRECISION)
                 {
                    //Add delta rotations to initial control setting and to wing or flap delta rotation
                    if(fabs(surf.foilA()->TEFlapAngle())>0.0 && fabs(surf.foilB()->TEFlapAngle())>0.0)
                        totalAngle = deltaangle + (surf.foilA()->TEFlapAngle() + surf.foilB()->TEFlapAngle())/2.0;
                    else
                        totalAngle = deltaangle;

                    strange = QString::asprintf("- rotating flap %d by %g°, total flap angle is %g°", iCtrl, deltaangle, totalAngle);
                    strange = "      " + QString::fromStdString(pWing->name()) + strange + EOLch;
                    outstr +=strange;

                    pPlaneXfl->rotateFlapNodes(panel3, node, surf, surf.hingePoint(), surf.hingeVector(), deltaangle);
                }
                iCtrl++;
                nCtrl++;
            }
        }
    }


    TriMesh::rebuildPanelsFromNodes(panel3, node);

    outstr  +="\n";

    outstring = outstr.toStdString();
}


/**
 * The method run() is redefined explicitely in the derived class,
 * because the script will not run asynchronously
 * on the abstract base class's method
 */
void PlaneTask::run()
{
    if(!initializeTask())
    {
        m_bWarning = m_bError = true;
        m_AnalysisStatus = xfl::FINISHED;

        return;
    }

    m_AnalysisStatus = xfl::RUNNING;

    if(s_bCancel || !m_pPlPolar)
    {
        m_AnalysisStatus = xfl::CANCELLED;
        return;
    }

    loop();

    m_bWarning = m_bWarning || m_pPA->m_bWarning;
    m_bError = m_bError || m_pPA->m_bWarning;

    if(m_AnalysisStatus!=xfl::CANCELLED) m_AnalysisStatus = xfl::FINISHED;  // finish the analysis before sending the final condition_variable
    traceStdLog("\nDone plane task.\n"); // final notification after flag is set to FINISHED so that sender thread may exit
}


bool PlaneTask::computeViscousDrag(WingXfl *pWing, double alpha, double beta, double QInf,
                                   PlanePolar const *pWPolar, Vector3d const &cog, int iStation0, SpanDistribs &SpanResFF,
                                   std::string &logmsg) const
{
    QString strong, strange, strOut;
    QString logg;

    // Define the wind axes
    Vector3d winddirection = objects::windDirection(alpha, beta);
    Vector3d windside      = objects::windSide(alpha, beta);


    bool bViscOK = true;

    SpanDistribs &sd = SpanResFF;

    sd.m_Re.clear();
    for (int m=0; m<pWing->nStations(); m++)  sd.m_Re.push_back(SpanResFF.m_Chord.at(m) * QInf /pWPolar->viscosity());

    int m=0;// wing station counter
    for (int j=0; j<pWing->nSurfaces(); j++)
    {
        Surface const &surf = pWing->surfaceAt(j);
        for(int k=0; k<surf.NYPanels(); k++)
        {
            bool bOutRe    = false;
            bool bOutVar   = false;

            double tau = 0.0;
            Vector3d PtC4;
            surf.getC4(k, PtC4, tau);
            if (tau<0.0) tau = 0.0; // redundant
            if (tau>1.0) tau = 1.0; // redundant

            sd.m_Alpha_0[m] = Objects2d::getZeroLiftAngle(surf.foilA(), surf.foilB(), sd.m_Re[m], tau);

            double CdA(0), CdB(0), XTrTopA(0), XTrTopB(0), XTrBotA(0), XTrBotB(0);

            if(pWPolar->isViscFromCl())
            {
                CdA = Objects2d::getPlrPointFromCl(surf.foilA(), sd.m_Re.at(m), sd.m_Cl.at(m), Polar::CD, bOutRe, bOutVar);
                CdB = Objects2d::getPlrPointFromCl(surf.foilB(), sd.m_Re.at(m), sd.m_Cl.at(m), Polar::CD, bOutRe, bOutVar);

                XTrTopA = Objects2d::getPlrPointFromCl(surf.foilA(), sd.m_Re.at(m), sd.m_Cl.at(m), Polar::XTRTOP, bOutRe, bOutVar);
                XTrTopB = Objects2d::getPlrPointFromCl(surf.foilB(), sd.m_Re.at(m), sd.m_Cl.at(m), Polar::XTRTOP, bOutRe, bOutVar);

                XTrBotA = Objects2d::getPlrPointFromCl(surf.foilA(), sd.m_Re.at(m), sd.m_Cl.at(m), Polar::XTRBOT, bOutRe, bOutVar);
                XTrBotB = Objects2d::getPlrPointFromCl(surf.foilB(), sd.m_Re.at(m), sd.m_Cl.at(m), Polar::XTRBOT, bOutRe, bOutVar);

                if(bOutVar)
                {
                    strOut = QString::asprintf(",  Cl = %7.2f\n", sd.m_Cl.at(m));
                    bViscOK = false;
                }
            }
            else
            {
                double aoa_effective = sd.m_Alpha_0.at(m) + (sd.m_Cl.at(m)/2.0/PI) *180.0/PI - m_gamma.at(iStation0+m);

                CdA = Objects2d::getPlrPointFromAlpha(surf.foilA(), sd.m_Re.at(m), aoa_effective, Polar::CD, bOutRe, bOutVar);
                CdB = Objects2d::getPlrPointFromAlpha(surf.foilB(), sd.m_Re.at(m), aoa_effective, Polar::CD, bOutRe, bOutVar);

                XTrTopA = Objects2d::getPlrPointFromAlpha(surf.foilA(), sd.m_Re.at(m), aoa_effective, Polar::XTRTOP, bOutRe, bOutVar);
                XTrTopB = Objects2d::getPlrPointFromAlpha(surf.foilB(), sd.m_Re.at(m), aoa_effective, Polar::XTRTOP, bOutRe, bOutVar);

                XTrBotA = Objects2d::getPlrPointFromAlpha(surf.foilA(), sd.m_Re.at(m), aoa_effective, Polar::XTRBOT, bOutRe, bOutVar);
                XTrBotB = Objects2d::getPlrPointFromAlpha(surf.foilB(), sd.m_Re.at(m), aoa_effective, Polar::XTRBOT, bOutRe, bOutVar);

                if(bOutVar)
                {
                    strOut = QString::asprintf(",  AoA_effective = %7.2f", aoa_effective) + DEGch + EOLch;
                    bViscOK = false;
                }
            }

            strong = "           " + QString::asprintf("     Span position %9.2f ", sd.m_StripPos.at(m)*Units::mtoUnit());
            strong += Units::lengthUnitQLabel();
            strong += QString::asprintf(",  Re = %9.0f", sd.m_Re.at(m));

            if(bOutVar)
            {
                logg += strong + strOut;
            }
            else if(bOutRe)
            {
                strange = QString::asprintf(",  Cl = %7.2f\n", sd.m_Cl.at(m));
                logg += strong + strange;
                bViscOK = false;
            }

            if(bOutVar || bOutRe)
            {
                sd.m_PCd[m]    = 0.0;
                sd.m_XTrTop[m] = 0.0;
                sd.m_XTrBot[m] = 0.0;
            }

            sd.m_PCd[m]    = CdA     * (1.0-tau) + CdB     * tau;
            sd.m_XTrTop[m] = XTrTopA * (1.0-tau) + XTrTopB * tau;
            sd.m_XTrBot[m] = XTrBotA * (1.0-tau) + XTrBotB * tau;

            //add the moment of the strip's viscous drag
            Vector3d dragvector = winddirection * (sd.m_PCd.at(m) * sd.m_StripArea.at(m));          //N/q
            Vector3d leverarmcog = sd.m_PtC4.at(m) - cog;   // m
            sd.m_CmViscous[m] =  (leverarmcog * dragvector).dot(windside);   // N.m/q
            sd.m_CmViscous[m] *= 1.0/sd.m_Chord.at(m)/sd.m_StripArea.at(m);

            m++; // wing station counter
            if(s_bCancel) break;
        }
        if(s_bCancel) break;
    }

    logmsg = logg.toStdString();

    return bViscOK;
}


bool PlaneTask::computeSurfaceDragOTF(Surface const &surf, int iStartStation, double theta, SpanDistribs &spandist)
{
    Foil foilA, foilB;
    foilA.copy(surf.foilA(), true);
    foilB.copy(surf.foilB(), true);

    if(isCancelled()) return false;

    const int m = iStartStation;

    if(surf.hasTEFlap() && fabs(theta)>FLAPANGLEPRECISION)
    {
        foilA.setTEFlapAngle(theta);
        foilA.setFlaps();
        foilB.setTEFlapAngle(theta);
        foilB.setFlaps();
    }
    else
    {
        foilA.applyBase();
        foilB.applyBase();
    }

    // Process right side foil
    Polar LeftSidePolar;
    LeftSidePolar.setType(xfl::T1POLAR);
    LeftSidePolar.setReType(1);
    LeftSidePolar.setMaType(1);
    LeftSidePolar.setMach(0.0);
    LeftSidePolar.setNCrit(m_pPolar3d->NCrit());
    LeftSidePolar.resizeData(surf.NYPanels());
    for(int k=0; k<surf.NYPanels(); k++)
    {
        LeftSidePolar.m_Cl[k] = spandist.m_Cl.at(m+k);
        LeftSidePolar.m_Re[k] = spandist.m_Re.at(m+k);
    }

    double XTrTop = m_pPolar3d->XTrTop();
    double XTrBot = m_pPolar3d->XTrBot();
    if(surf.hasTEFlap() && m_pPlPolar->bTransAtHinge())
    {
        XTrTop = std::min(XTrTop, foilA.TEXHinge());
        XTrBot = std::min(XTrBot, foilA.TEXHinge());
    }
    LeftSidePolar.setXTripTop(XTrTop);
    LeftSidePolar.setXTripBot(XTrBot);

    XFoilTask *pLeftTask = new XFoilTask();
    pLeftTask->initialize(foilA, &LeftSidePolar, false);

    // Process right side foil
    Polar RightSidePolar;
    RightSidePolar.setType(xfl::T1POLAR);
    RightSidePolar.setReType(1);
    RightSidePolar.setMaType(1);
    RightSidePolar.setMach(0.0);
    RightSidePolar.setNCrit(m_pPolar3d->NCrit());
    RightSidePolar.resizeData(surf.NYPanels());
    for(int k=0; k<surf.NYPanels(); k++)
    {
        RightSidePolar.m_Cl[k] = spandist.m_Cl.at(m+k);
        RightSidePolar.m_Re[k] = spandist.m_Re.at(m+k);
    }


    XTrTop = m_pPolar3d->XTrTop();
    XTrBot = m_pPolar3d->XTrBot();
    if(surf.hasTEFlap() && m_pPlPolar->bTransAtHinge())
    {
        XTrTop = std::min(XTrTop, foilB.TEXHinge());
        XTrBot = std::min(XTrBot, foilB.TEXHinge());
    }
    RightSidePolar.setXTripTop(XTrTop);
    RightSidePolar.setXTripBot(XTrBot);


    XFoilTask *pRightTask = new XFoilTask();
    pRightTask->initialize(foilB, &RightSidePolar, false);

//    computeSectionDragOTF(pLeftTask);
//    computeSectionDragOTF(pRightTask);

    std::thread leftthread  = std::thread(&PlaneTask::computeSectionDragOTF, this, pLeftTask);
    std::thread rightthread = std::thread(&PlaneTask::computeSectionDragOTF, this, pRightTask);
    leftthread.join();
    rightthread.join();

    delete pLeftTask;
    delete pRightTask;


    bool bCv = true;
    QString logg;
    int iStation=iStartStation;
    for(int k=0; k<surf.NYPanels(); k++)
    {
        double tau = 0.0;
        Vector3d PtC4;
        surf.getC4(k, PtC4, tau);
        if (tau<0.0) tau = 0.0; // redundant
        if (tau>1.0) tau = 1.0; // redundant

        /** @todo what's the use + wrong if flap*/
        spandist.m_Alpha_0[iStation] = Objects2d::getZeroLiftAngle(surf.foilA(), surf.foilB(), spandist.m_Re.at(iStation), tau);

        // interpolate

        // repurposing control variable to contain convergence result
        spandist.m_bConverged[iStation] = LeftSidePolar.m_Control.at(k)>0.0 && RightSidePolar.m_Control.at(k)>0.0;
        spandist.m_PCd[iStation]    = LeftSidePolar.m_Cd.at(k)     * (1.0-tau) + RightSidePolar.m_Cd.at(k)     * tau;
        spandist.m_XTrTop[iStation] = LeftSidePolar.m_XTrTop.at(k) * (1.0-tau) + RightSidePolar.m_XTrTop.at(k) * tau;
        spandist.m_XTrBot[iStation] = LeftSidePolar.m_XTrBot.at(k) * (1.0-tau) + RightSidePolar.m_XTrBot.at(k) * tau;


        if(!spandist.m_bConverged.at(iStation))
        {
            QString strong = "                     ";
            strong +=  QString::asprintf("span pos. %9.3f ", spandist.m_StripPos.at(iStation)*Units::mtoUnit());
            strong += Units::lengthUnitLabel();
            strong += QString::asprintf(", Cl=%9.5f, Re=%7.0f", spandist.m_Cl.at(iStation), spandist.m_Re.at(iStation));

            logg += strong + EOLch;
            bCv = false;
        }


        iStation++; // wing station counter
        if(s_bCancel) break;
    }

    QString report = QString::asprintf("                 ...done surface %d", surf.index());
    if(bCv)
    {
        report += EOLch;
    }
    else
    {
        report += " - OTF failures:" + EOLch + logg;
    }

    traceLog(report);

    return bCv;
}


bool PlaneTask::computeSectionDragOTF(XFoilTask *pTask) const
{
    pTask->processClList();


    // fall back: re-try unconverged points with BL initialization
    Polar const *pPolar = pTask->polar();
    for(int k=0; k<pPolar->dataSize(); k++)
    {
        if(pPolar->m_Control.at(k)<0.0) // repurposed
        {
            pTask->processCl(k);
        }

        if(pPolar->m_Control.at(k)<0.0) // repurposed
        {
            return false; // XFoil OTF fail, operating point will be discarded
        }

        if(isCancelled())
        {
            return false;
        }
    }

    return true;
}


bool PlaneTask::computeViscousDragOTF(WingXfl *pWing, double alpha, double beta, double QInf,
                                      PlanePolar const *pWPolar, Vector3d const &cog, AngleControl const &TEFlapAngles, SpanDistribs &SpanResFF,
                                      std::string &logmsg)
{
    // on the fly viscous drag calculation
    // for each surface, calulate the drag at each end foil for each lift and reynolds at each span station
    // then interpolate

    QString logg;

    assert(pWing->nFlaps()==TEFlapAngles.nValues());

    // Define the wind axes
    Vector3d winddirection = objects::windDirection(alpha, beta);
    Vector3d windside      = objects::windSide(alpha, beta);

    for (int m=0; m<pWing->nStations(); m++)
    {
        SpanResFF.m_Re[m] = SpanResFF.m_Chord.at(m) * QInf /pWPolar->viscosity();
    }

    double theta(0);
    int iCtrl = 0;
    int m=0;// wing station counter

    std::vector<std::thread> threads;


    for (int jsurf=0; jsurf<pWing->nSurfaces(); jsurf++)
    {
        Surface const &surf = pWing->surface(jsurf);
        if(surf.hasTEFlap()) theta = TEFlapAngles.value(iCtrl++);
        else                 theta = 0.0;

        threads.push_back(std::thread(&PlaneTask::computeSurfaceDragOTF, this, surf, m, theta, std::ref(SpanResFF)));
        //computeSurfaceDragOTF(surf, m, theta, std::ref(SpanResFF));
        m += surf.NYPanels();
    }

    for(int isurf=0; isurf<pWing->nSurfaces(); isurf++)        threads[isurf].join();


    SpanDistribs &sd = SpanResFF;
    bool bCv = true;
    for(int k=0; k<sd.nStations(); k++)
    {
        if(!sd.m_bConverged.at(k))
        {
/*            QString strong = "                  ";
            strong +=  QString::asprintf("span pos. %9.3f ", sd.m_StripPos.at(k)*Units::mtoUnit());
            strong += Units::lengthUnitLabel();
            strong += QString::asprintf(", Cl=%9.5f, Re=%7.0f", sd.m_Cl.at(k), sd.m_Re.at(k));

            logg += strong + EOLch;*/
            bCv = false;
        }
    }

    if(bCv)
    {
        m = 0;
        for (int jsurf=0; jsurf<pWing->nSurfaces(); jsurf++)
        {
            Surface const &surf = pWing->surface(jsurf);
            for(int k=0; k<surf.NYPanels(); k++)
            {
                //add the moment of the strip's viscous drag
                Vector3d dragvector = winddirection * (sd.m_PCd.at(m) * sd.m_StripArea.at(m));          //N/q
                Vector3d leverarmcog = sd.m_PtC4.at(m) - cog;   // m
                sd.m_CmViscous[m] =  (leverarmcog * dragvector).dot(windside);   // N.m/q
                sd.m_CmViscous[m] *= 1.0/sd.m_Chord.at(m)/sd.m_StripArea.at(m);

                m++; // wing station counter
                if(s_bCancel) break;
            }
        }
    }

    if(!s_bCancel)
        assert(iCtrl == TEFlapAngles.nValues());

    logmsg = logg.toStdString();

    return bCv;
}


void PlaneTask::makeVortonRow(int qrhs)
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

    if(m_pPlane->isXflType())
    {
        std::vector<Vorton> wingvortons;
        int pos = 0;
        for(int iw=0; iw<m_pPlane->nWings(); iw++)
        {
            WingXfl const *pWing = m_pPlane->wingAt(iw);
            if(m_pP4A)
            {
                m_pP4A->makeVortons(dl, mu, pos, pWing->nPanel4(), pWing->nStations(), int(vortonrow.size()), wingvortons, wingvortexneg);
                pos += pWing->nPanel4();
            }
            else if(m_pP3A)
            {
                m_pP3A->makeVortons(dl, mu, pos, pWing->nPanel3(), pWing->nStations(), int(vortonrow.size()), wingvortons, wingvortexneg);
                pos += pWing->nPanel3();
            }

            vortonrow.insert(vortonrow.end(), wingvortons.begin(), wingvortons.end());
            m_pPA->m_VortexNeg.insert(m_pPA->m_VortexNeg.end(), wingvortexneg.begin(), wingvortexneg.end());
        }
    }
    else
    {
        if(m_pP3A)
        {
            m_pP3A->makeVortons(dl, mu, 0, m_pPlane->nPanel3(), m_pPlane->nStations(), 0, vortonrow, wingvortexneg);
            m_pPA->m_VortexNeg.insert(m_pPA->m_VortexNeg.end(), wingvortexneg.begin(), wingvortexneg.end());
        }
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

//for(int i=0; i<vortonrow.size(); i++)    qDebug("%17g", vortonrow.at(i).circulation());
//qDebug(" ");

    newvortons.insert(newvortons.begin(), vortonrow);


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
    {
        newvortons.pop_back();
//        qDebug(" popping back new size = %d", int(newvortons.size()));
    }
    else
    {
//        qDebug(" new size = %d", int(newvortons.size()));
    }

    // save the new vortons
    m_pPA->m_Vorton = newvortons;
}


void PlaneTask::getVelocityVector(Vector3d const &C, double coreradius, bool bMultiThread, Vector3d &velocity) const
{
    bool bWakeOnly=false;
    m_pPA->getVelocityVector(C, m_pPA->m_Mu.data(), m_pPA->m_Sigma.data(), velocity, coreradius, bWakeOnly, bMultiThread);
}


bool PlaneTask::setLinearSolution()
{
    QString strange;

    auto start = std::chrono::system_clock::now();

    traceStdLog("   Making the unit RHS vectors...");
    m_pPA->makeUnitRHSVectors();

    auto end = std::chrono::system_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    start = end;
    strange = QString::asprintf("       done in %.3f s\n", double(duration)/1000.0);

    traceLog(strange);

    if (isCancelled()) return true;

    traceStdLog("   Making the influence matrix...");
    m_pPA->makeInfluenceMatrix();

    end = std::chrono::system_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    start = end;
    strange = QString::asprintf("       done in %.3f s\n", double(duration)/1000.0);

    traceLog(strange);

    if(m_pPA->m_bMatrixError)
    {
        m_bError = true;
        return false;
    }
    if (isCancelled()) return true;

    if(!m_pPlPolar->isVLM())
    {
        traceStdLog("   Adding the wake's contribution...");
        m_pPA->addWakeContribution();


        end = std::chrono::system_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        start = end;
        strange = QString::asprintf("    done in %.3f s\n", double(duration)/1000.0);
        traceLog(strange);

        if(m_pPA->m_bMatrixError) return false;
    }
    if (isCancelled()) return true;

    traceStdLog("   LAPACK - LU factorization...");
    if (!m_pPA->LUfactorize())
    {
        traceStdLog(" singular matrix, aborting\n");

        m_bError = true;
        return false;
    }

    end = std::chrono::system_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    start = end;
    strange = QString::asprintf("         done in %.3f s\n", double(duration)/1000.0);
    traceLog(strange);
    if (isCancelled()) return true;

    traceStdLog("   Back-substituting RHS...");
    m_pPA->backSubUnitRHS(m_pPA->m_uRHS.data(), m_pPA->m_vRHS.data(), m_pPA->m_wRHS.data(), m_pPA->m_pRHS.data(), m_pPA->m_qRHS.data(), m_pPA->m_rRHS.data());


    end = std::chrono::system_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    start = end;
    strange = QString::asprintf("             done in %.3f s\n", double(duration)/1000.0);

    traceLog(strange);

//    listArrays(m_pPA->m_uRHS, m_pPA->m_wRHS);

    return true;
}



