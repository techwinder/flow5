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

#define _MATH_DEFINES_DEFINED

#include <QApplication>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QFutureSynchronizer>
#include <QElapsedTimer>




#if defined ACCELERATE
  #include <Accelerate/Accelerate.h>
  #define lapack_int int
#elif defined INTEL_MKL
    #include <mkl.h>
#elif defined OPENBLAS
    #include <cblas.h>
    #include <openblas/lapacke.h>
//    #define lapack_int int
#endif


#include "planetask.h"

#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/analysis3d/p3linanalysis.h>
#include <xflobjects/analysis3d/p3unianalysis.h>
#include <xflobjects/analysis3d/p4analysis.h>
#include <xflobjects/analysis3d/panelanalysis.h>
#include <xflpanels/mesh/mesh_globals.h>
#include <xflgeom/geom_globals/geom_params.h>
#include <xflobjects/objects_globals/objects_global.h>
#include <xflobjects/objects3d/analysis/planeopp.h>
#include <xflobjects/objects3d/analysis/stabderivatives.h>
#include <xflobjects/objects3d/analysis/wpolar.h>
#include <xflfoil/objects2d/objects2d.h>
#include <xflfoil/objects2d/polar.h>
#include <xflfoil/analysis2d/xfoiltask.h>
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflobjects/objects3d/wing/wingxfl.h>

bool PlaneTask::s_bViscInitTwist = false;
double PlaneTask::s_ViscRelax = 0.5;
double PlaneTask::s_ViscAlphaPrecision = 0.01;
int PlaneTask::s_ViscMaxIter = 35;


PlaneTask::PlaneTask() : Task3d()
{
    m_pPlane  = nullptr;
    m_pWPolar = nullptr;

    m_Ctrl = -10000.0;
    m_Alpha = 0.0;
    m_QInf = m_Beta = m_Phi = 0.0;

    m_AF.resetAll();

}


PlaneTask::~PlaneTask()
{
}


void PlaneTask::setViscousLoopSettings(bool bInitVTwist, double relaxfactor, double alphaprec, int maxiters)
{
    s_bViscInitTwist     = bInitVTwist;
    s_ViscRelax          = relaxfactor;
    s_ViscAlphaPrecision = alphaprec;
    s_ViscMaxIter        = maxiters;
}


void PlaneTask::setOppList(QVector<double> const &opplist)
{
    m_AngleList  = opplist;
    m_nRHS       = opplist.size();
}


void PlaneTask::setCtrlOppList(QVector<double> const &opplist)
{
    m_T6CtrlList = opplist;
    m_nRHS       = opplist.size();
}


void PlaneTask::setStabOppList(QVector<double> const &opplist)
{
    m_T7CtrlList = opplist;
    m_nRHS       = opplist.size();
}


void PlaneTask::setT8OppList(QVector<T8Opp> const &ranges)
{
    m_T8Opps = ranges;
    m_nRHS = ranges.size();
}


bool PlaneTask::initializeTask()
{
    QString strange, strong;

    m_bWarning = m_bError = false;

    onTraceLog("Initializing task\n\n");

    // allocate virtual twist angles
    m_gamma.resize(m_pPlane->nStations());
    m_gamma.fill(0);

    onTraceLog(m_pPlane->name()+"\n");
    onTraceLog(m_pWPolar->name()+"\n\n");

    // check that reference dims are valid
    if(!checkWPolarData(m_pPlane, m_pWPolar))
    {
        onTraceLog("Aborting analysis\n");
        return false;
    }

    // check flaps and ViscOTF
    if(m_pWPolar->hasActiveFlap())
    {
        if(!m_pWPolar->isViscOnTheFly())
        {
            onTraceLog("Warning: XFoil on the fly calculations should be selected when TE flaps are active\n\n");
        }
    }

    onTraceLog(m_pPolar3d->name()+"\n");

    if     (m_pPolar3d->isFixedSpeedPolar())  strange = "Type 1 - Fixed speed polar";
    else if(m_pPolar3d->isFixedLiftPolar())   strange = "Type 2 - Fixed lift polar";
    else if(m_pPolar3d->isBetaPolar())        strange = "Type 5 - Beta polar";
    else if(m_pPolar3d->isControlPolar())     strange = "Type 6 - Control polar";
    else if(m_pPolar3d->isStabilityPolar())   strange = "Type 7 - Stability polar";
    else if(m_pPolar3d->isBoatPolar())        strange = "Sail analysis";
    else                                      strange = "Unsupported polar type";
    onTraceLog(strange+"\n\n");

    strange = RHOCHAR + QString::asprintf(" = %9.5g ", m_pPolar3d->density()*Units::densitytoUnit());
    onTraceLog(strange+ Units::densityUnitLabel()+"\n");
    strange = NUCHAR  + QString::asprintf(" = %9.5g ", m_pPolar3d->viscosity()*Units::viscositytoUnit());
    onTraceLog(strange+ Units::viscosityUnitLabel()+"\n\n");

    strange = QString::asprintf("RFF         = %g\n", Panel::RFF());
    onTraceLog(strange);
    strange = QString::asprintf("Core radius = %g ", Vortex::coreRadius()*Units::mtoUnit());
    strange += Units::lengthUnitLabel() + "\n";

    if(Vortex::coreRadius()<1.e-6)
    {
        strange += "Caution: too small core radiuses can lead to numerical instabilities.\n"
                   "Recommendation: Core radius > 1 µm\n\n";
    }


    onTraceLog(strange);
    onTraceLog("\n");

    if(PanelAnalysis::s_bMultiThread) onTraceLog("Running in multi-threaded mode\n\n");
    else                              onTraceLog("Running in single-threaded mode\n\n");

    if(PanelAnalysis::s_bDoublePrecision) onTraceLog("Linear system calculations in floating point double precision\n\n");
    else                                  onTraceLog("Linear system calculations in floating point single precision\n\n");

    if(Panel3::usingNintcheuFataMethod())
        onTraceLog("Using S. Nintcheu-Fata's method for off-plane integrals\n\n");
    else
        onTraceLog("Using M. Carley's method for off-plane integrals\n\n");

    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl *>(m_pPlane);

    if(m_pWPolar->isQuadMethod())
    {
        onTraceLog("Initializing the quad analysis\n");
        if(!pPlaneXfl)
        {
            onTraceLog("Quad analysis not available for STL-type planes\n");
            return false;
        }
        m_pP4A = new P4Analysis;
        m_pPA = m_pP4A;
    }
    else if(m_pWPolar->isTriangleMethod())
    {
        onTraceLog("Initializing the triangle analysis\n");

        QElapsedTimer t;   t.start();

        onTraceLog("Checking element sizes\n");
        QVector<int> criticals;
        QVector<double> sizes;
        m_pPlane->triMesh().checkElementSize(Vortex::coreRadius(), criticals, sizes);
        if(criticals.size()>0)
        {
            strange = "The element size is incompatible with the vortex core size.\n"
                      "Either increase the element size, or reduce the vortex core size to be at most 1/50 of the smallest element size.\n"
                      "The vortex core size is set in the global analysis settings (menu option analysis/3d analysis settings).\n";
            strange += QString::asprintf("   Core radius = %g ", Vortex::coreRadius()*Units::mtoUnit()) + Units::lengthUnitLabel()+"\n";
            strange += "      Critical elements:\n";
            for(int i=0; i<criticals.size(); i++)
            {
                strange += QString::asprintf("      element %5d   width=%11.5g ",
                                             criticals.at(i),
                                             sizes.at(i)*Units::mtoUnit()) + Units::lengthUnitLabel();
                strange += QString::asprintf("  ratio = 1/%.1f", sizes.at(i)/Vortex::coreRadius())+ "\n";
            }
            onTraceLog(strange+"\n\n");
            m_bError = true;
            return false;
        }
        onTraceLog("   ...done\n\n");

        onTraceLog("Connecting triangular panels...");

        if(!m_pPlane->connectTriMesh(false, m_pWPolar->bThickSurfaces()))
        {
            strange = "\n   Error making trailing edge connections -- aborting.\n\n";
            onTraceLog(strange);
            m_bError = true;
            return false;
        }

        m_pPlane->restoreMesh();
        onTraceLog("   done\n");

        if(m_pWPolar->isTriUniformMethod())
        {
            m_pP3A = new P3UniAnalysis;
            m_pPA = m_pP3A;
        }
        if(m_pWPolar->isTriLinearMethod())
        {
            // make the node normals on the fly
            // node normals are required to compute local Cp coefficients at nodes
            m_pPlane->refTriMesh().makeNodeNormals(false);
            m_pP3A = new P3LinAnalysis;
            m_pPA = m_pP3A;
        }

        strange = QString::asprintf("   Time to make the triangle connections: %.3f s\n\n", double(t.elapsed())/1000.0);
        onTraceLog(strange);
    }

    switch(m_pWPolar->type())
    {
        case xfl::BOATPOLAR:
        {
            return false;
        }
        case xfl::T4POLAR:
        {
            onTraceLog("T4 analysis are deprecated\n");
            return false;
        }
        case xfl::EXTERNALPOLAR:
        {
            onTraceLog("External polars are not available for analysis\n");
            return false;
        }
        case xfl::T1POLAR:
        case xfl::T2POLAR:
        case xfl::T3POLAR:
        case xfl::T5POLAR:
        case xfl::T7POLAR:
        case xfl::T8POLAR:
        {
            m_pPlane->restoreMesh();

            QString outstring;
            if(pPlaneXfl) pPlaneXfl->setFlaps(m_pWPolar, outstring);

            onTraceLog(outstring);

            if(fabs(m_pWPolar->phi())>AOAPRECISION)
            {
                if(m_pWPolar->isTriangleMethod())
                    pPlaneXfl->triMesh().rotate(0,0,m_pWPolar->phi());
                else if(m_pWPolar->isQuadMethod())
                    pPlaneXfl->quadMesh().rotate(0,0,m_pWPolar->phi());
            }

            if(pPlaneXfl && m_pWPolar->isQuadMethod())
            {
                m_pP4A->setQuadMesh(pPlaneXfl->quadMesh());
                m_pP4A->initializeAnalysis(m_pWPolar, m_nRHS);
            }

            else if(m_pWPolar->isTriangleMethod())
            {
                m_pP3A->setTriMesh(m_pPlane->triMesh());
                m_pP3A->initializeAnalysis(m_pWPolar, m_nRHS);
            }

            break;
        }
        case xfl::T6POLAR:
        {
            if(pPlaneXfl && m_pWPolar->isQuadMethod())
            {
                m_pP4A->setQuadMesh(pPlaneXfl->refQuadMesh());
                m_pP4A->initializeAnalysis(m_pWPolar, m_nRHS);
            }
            else if(m_pWPolar->isTriangleMethod())
            {
                m_pP3A->setTriMesh(m_pPlane->triMesh());
                m_pP3A->initializeAnalysis(m_pWPolar, m_nRHS);
            }

            if(m_pWPolar->bViscousLoop())
            {
                strange = "The viscous loop is activated:\n";
                strong = QString::asprintf("   Max. iterations         = %d\n", s_ViscMaxIter);
                strange += strong;
                strong = QString::asprintf("   Virtual twist precision = %g", s_ViscAlphaPrecision);
                strong += DEGCHAR + "\n";
                strange += strong;
                strong = QString::asprintf("   Relaxation factor       = %g\n\n", s_ViscRelax);
                strange += strong;
                onTraceLog(strange);
            }

            if(m_pWPolar->bVortonWake())
            {
                strange = "Vortex particle wake:\n";
                strong = QString::asprintf("   Max. iterations  = %d\n", m_pPolar3d->VPWIterations());
                strange += strong;
                strong = QString::asprintf("   Discard distance = %g x MAC\n", m_pPolar3d->VPWMaxLength());
                strange += strong;
                strong = QString::asprintf("   Vorton core size = %g x MAC = %g",
                                           m_pPolar3d->vortonCoreSize(), m_pPolar3d->vortonCoreSize()*m_pPolar3d->referenceChordLength()*Units::mtoUnit());
                strong += Units::lengthUnitLabel() + "\n";
                strange += strong;
                onTraceLog(strange);
            }
            break;
        }
    }

    strange = QString::asprintf("Counted %d elements\n", m_pPA->nPanels());
    strange += QString::asprintf("Matrix size = %d\n\n", m_pPA->matSize());
    onTraceLog(strange);

    if(isCancelled()) return false;

    onTraceLog("Allocating result arrays\n");
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


void PlaneTask::setObjects(Plane *pPlane, WPolar *pWPolar)
{
    m_pPlane = pPlane;
    m_pWPolar = pWPolar;
    m_pPolar3d = m_pWPolar;

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
            if(m_pWPolar->bProjectedDim())
                m_PartAF[iw].setReferenceDims(pWing->projectedArea(), pWing->MAC(), pWing->projectedSpan());
            else
                m_PartAF[iw].setReferenceDims(pWing->planformArea(),  pWing->MAC(), pWing->planformSpan());

            m_SpanDistFF[iw].resizeResults(pWing->nStations());
            pWing->resizeSpanDistribs();
        }
    }

    if(m_pWPolar && m_pWPolar->bAutoInertia())
    {
        if(pPlane)
        {
            double mass = pPlane->totalMass();
            Vector3d CoG = pPlane->inertia().CoG_t();
            m_pWPolar->setMass(mass);
            m_pWPolar->setCoG(CoG);
            m_pWPolar->setIxx(pPlane->inertia().Ixx_t());
            m_pWPolar->setIyy(pPlane->inertia().Iyy_t());
            m_pWPolar->setIzz(pPlane->inertia().Izz_t());
            m_pWPolar->setIxz(pPlane->inertia().Ixz_t());

            if(m_pWPolar->isType6())
            {
                m_pWPolar->retrieveInertia(pPlane);
            }
        }
    }
}


bool PlaneTask::checkWPolarData(Plane const *pPlane, WPolar *pWPolar)
{
    onTraceLog("Checking plane and polar data\n");
    if(!pPlane)
    {
        onTraceLog("Error: plane object not found... aborting\n");
        return false;
    }

    if(!pWPolar)
    {
        onTraceLog("Error: polar object not found... aborting\n");
        return false;
    }

    QString strange;
    QString logmsg;
    bool bCheck = true;

    if(m_pPlane->isXflType())
    {
        PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
        if(!pWPolar->checkFlaps(pPlaneXfl, logmsg))
        {
            pWPolar->resizeFlapCtrls(pPlaneXfl);
        }

        if(pPlaneXfl->nWings()>0)
        {
            if(fabs(pPlane->mac())<1.e-3)
            {
                // needed for wake construction
                strange = QString::asprintf("   error: Plane has MAC = %g", pPlane->mac()*Units::mtoUnit());
                strange += Units::lengthUnitLabel() + "\n";
                logmsg += strange;
                bCheck = false;
            }

            if(fabs(pWPolar->referenceChordLength())<1.e-3)
            {
                // needed for coefficient calculations
                strange = QString::asprintf("   error: reference chord length is %g", pWPolar->referenceChordLength()*Units::mtoUnit());
                strange += Units::lengthUnitLabel() + "\n";
                logmsg += strange;
                bCheck = false;
            }
            if(fabs(pWPolar->referenceSpanLength())<1.e-3)
            {
                // needed for coefficient calculations
                strange = QString::asprintf("   error: reference span length is %g", pWPolar->referenceSpanLength()*Units::mtoUnit());
                strange += Units::lengthUnitLabel() + "\n";
                logmsg += strange;
                bCheck = false;
            }
            if(fabs(pWPolar->referenceArea())<1.e-6)
            {
                // needed for coefficient calculations
                strange = QString::asprintf("   error: reference area is %g", pWPolar->referenceArea()*Units::m2toUnit());
                strange += Units::areaUnitLabel() + "\n";
                logmsg += strange;
                bCheck = false;
            }

            if(!bCheck)
            {
                if(!pPlane->hasMainWing())
                {
                    strange = QString::asprintf("   warning: no main wing detected in plane %s\n", pPlane->name().toStdString().c_str());
                }
                logmsg += strange;
                m_bError = true;
            }
        }
    }

    if(bCheck) onTraceLog("   ...done\n\n");
    else       onTraceLog(logmsg);

    return bCheck;
}


void PlaneTask::clearPOppList()
{
    for(int ip=m_PlaneOppList.size()-1; ip>=0; ip--)
    {
        delete m_PlaneOppList.at(ip);
        m_PlaneOppList.removeAt(ip);
    }
}


void PlaneTask::loop()
{
    if(m_pWPolar && m_pWPolar->isFixedaoaPolar())
    {
        onTraceLog("Fixed aoa polars are deprecated and not supported in flow5");
        return;
    }

    if(m_pWPolar->isType12358())
    {
        if(m_pWPolar->isType8())
        {
            // use specified T8 range
        }
        else
        {
            // make equivalent T8 range
            m_T8Opps.clear();

            double alpha(0), beta(0), qinf(0);
            for(int qrhs=0; qrhs<m_AngleList.size(); qrhs++)
            {
                if(m_pWPolar->isType1())
                {
                    alpha = m_AngleList.at(qrhs);
                    beta  = m_pWPolar->betaSpec();
                    qinf  = m_pWPolar->velocity();
                }
                else if(m_pWPolar->isFixedLiftPolar() || m_pWPolar->isGlidePolar())
                {
                    alpha = m_AngleList.at(qrhs);
                    beta  = m_pWPolar->betaSpec();
                    qinf =  1.0;
                }
                else if(m_pWPolar->isType5())
                {
                    alpha = m_pWPolar->alphaSpec();
                    beta  = m_AngleList.at(qrhs);
                    qinf  = m_pWPolar->velocity();
                }
                m_T8Opps.append({true, alpha, beta, qinf});
            }
        }
        T123458Loop();
    }
    else if(m_pWPolar->isType6())
    {
        T6Loop();
    }
    else if(m_pWPolar->isType7())
    {
        T7Loop();
    }
}


bool PlaneTask::T6Loop()
{
    QString log, str, strange;

    QElapsedTimer t;

    double error(0), CL(0);

    Vector3d F;

    onTraceLog("\nSolving the problem... \n\n");

    QVector<Vector3d> VField(m_pPA->nPanels()), VVPW(m_pPA->nPanels());

    if(m_pWPolar->isVLM()) m_pPA->m_nStations = m_pPlane->nStations(); // for assertion checks only

    bool bConvergedLast = false;
    m_gamma.fill(0); // initialize for the first operating point

    double AlphaStab(0.0), BetaStab(0.0), QInfStab(1.0);

    for (m_qRHS=0; m_qRHS<m_nRHS; m_qRHS++)
    {
        log.clear();
        onTraceLog("\n");
        m_bStopVPWIterations = false;

        m_Ctrl = m_T6CtrlList.at(m_qRHS);
        strange = QString::asprintf("    Processing control value= %.3f\n", m_Ctrl);

        if(!m_pWPolar->isAdjustedVelocity())
        {
            QInfStab = m_pWPolar->QInfCtrl(m_Ctrl);
            str = "      V" + INFCHAR + QString::asprintf(" = %.3f ", QInfStab*Units::mstoUnit());
            strange += str + Units::speedUnitLabel() + "\n";
            if(fabs(QInfStab)<1.e-6)
            {
                strange +="         null velocity... skipping operating point\n";
                onTraceLog(strange);
                continue;
            }
        }
        else
        {
            strange += "      V" + INFCHAR + " = adjusted\n";
        }

        m_Alpha = m_pWPolar->aoaCtrl(m_Ctrl);
        str = "      " + ALPHACHAR + QString::asprintf("  = %.3f", m_Alpha);
        strange += str + DEGCHAR + "\n";

        m_Beta = m_pWPolar->betaCtrl(m_Ctrl);
        str = "      " + BETACHAR  + QString::asprintf("  = %.3f", m_Beta);
        strange += str + DEGCHAR + "\n";
        BetaStab = -m_Beta;

        m_Phi = m_pWPolar->phiCtrl(m_Ctrl);
        str = "      " + PHICHAR   + QString::asprintf("  = %.3f", m_Phi);
        strange += str + DEGCHAR + "\n";

        double mass = m_pWPolar->massCtrl(m_Ctrl);
        str = QString::asprintf("      mass     = %.3f ", mass*Units::kgtoUnit());
        strange += str + Units::massUnitLabel() + "\n";

        Vector3d CoG = m_pWPolar->CoGCtrl(m_Ctrl);
        str = QString::asprintf("      CoG.x    = %.3f ", CoG.x*Units::mtoUnit());
        strange += str + Units::lengthUnitLabel() + "\n";

        str = QString::asprintf("      CoG.z    = %.3f ", CoG.z*Units::mtoUnit());
        strange += str + Units::lengthUnitLabel() + "\n";
        onTraceLog(strange);

        //reset the initial geometry before a new angle is processed
        onTraceLog("\n      Restoring the base mesh\n");
        m_pPlane->restoreMesh();
        m_pPA->restorePanels();

        onTraceLog("      Setting control positions\n");

        PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pPlane);
        if(pPlaneXfl)
        {
            if(m_pP4A && m_pPlane->isXflType())
            {
                pPlaneXfl->setRangePositions4(m_pWPolar, m_Ctrl, log);
                m_pP4A->setQuadMesh(pPlaneXfl->quadMesh());
            }
            else if(m_pP3A)
            {
                pPlaneXfl->setRangePositions3(m_pWPolar, m_Ctrl, log);
                m_pP3A->setTriMesh(m_pPlane->triMesh());
            }
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


        onTraceLog(log);

        onTraceLog("      Updating wake panels\n");
        // wake panels aligned with x-axis up to beta 11
        // wake panels aligned the with the freestream direction since beta 12
        // little or no difference to the results as long as the downwash is evaluated on the wake itself
        m_pPA->makeWakePanels(windDirection(AlphaStab, 0.0), m_pWPolar->bVortonWake());

        if(m_pWPolar->bVortonWake())
        {
            m_pPA->clearVortons(); // from the previous operating point calculation
            m_pPA->m_VortexNeg.clear();
        }

        t.start();

        onTraceLog("      Making the influence matrix...");
        m_pPA->makeInfluenceMatrix();

        strange = QString::asprintf("     done in %.3f s\n", double(t.restart())/1000.0);
        onTraceLog(strange);

        if(m_pPA->m_bMatrixError) return false;
        if (isCancelled()) return true;

        if(!m_pWPolar->isVLM())
        {
            m_pPA->addWakeContribution();
            if(m_pPA->m_bMatrixError) return false;
        }
        if (isCancelled()) return true;

        onTraceLog("      LAPACK - LU factorization...");

        if (!m_pPA->LUfactorize())
        {
            onTraceLog(" singular matrix, aborting\n");
            m_bError = true;
            return true;
        }

        strange = QString::asprintf("       done in %.3f s\n", double(t.restart())/1000.0);
        onTraceLog(strange);

        onTraceLog("      Making source strengths...");
        m_pPA->makeSourceStrengths(windDirection(m_Alpha, 0.0)); // unit source strengths
        onTraceLog("         done\n");

        // go through the loop at least once
        int nWakeIter = 1;
        if(m_pPolar3d->bVortonWake()) nWakeIter = std::max(nWakeIter, m_pPolar3d->VPWIterations());


        bool bViscLoopError = false;
        // initial velocity is 1 m/s, and is scaled at each iteration
        m_QInf = 1.0;
        if(m_pWPolar->isAdjustedVelocity()) QInfStab = 1.0;

        // start the roll-up iterations
        if(s_bViscInitTwist || !bConvergedLast) m_gamma.fill(0);

        onTraceLog("      Starting wake iterations\n");

        for(int ivw=0; ivw<nWakeIter; ivw++)
        {
            strange.clear();

            if(m_pWPolar->bVortonWake())
                m_pPA->makeRHSVWVelocities(VVPW);

            bViscLoopError = false;
            int nViscIter = 1;
            if(m_pWPolar->bViscousLoop()) nViscIter = std::max(nViscIter, s_ViscMaxIter);
            for(int inl=0; inl<nViscIter; inl++)
            {
                Vector3d VInf(windDirection(AlphaStab, BetaStab)*m_QInf);
                VField.fill(VInf);
                if(m_pPlane->isXflType())  addTwistedVelField(m_QInf, AlphaStab, VField);

                // add the vorton induced velocities to the velocity field
                for(int iv=0; iv<VField.size(); iv++) VField[iv] += VVPW.at(iv);

//                m_pPA->makeSourceStrengths(VField);

                m_pPA->makeRHS(VField, m_pPA->m_uRHS, nullptr);

                m_pPA->backSubUnitRHS(m_pPA->m_uRHS.data(), nullptr, nullptr, nullptr, nullptr, nullptr);

/*                if(ivw==nWakeIter-1)
                {
                    for(int ip=0; ip<m_pPA->nPanels(); ip++)
                        qDebug(" %17g   %17g", m_pPA->m_Sigma.at(ip), m_pPA->m_uRHS.at(ip));
                }*/

                if(m_pWPolar->isQuadMethod())
                    m_pP4A->m_Mu = m_pP4A->m_uRHS;
                else
                {
                    if(m_pWPolar->isTriLinearMethod())
                        m_pP3A->m_Mu = m_pP3A->m_uRHS;
                    if(m_pWPolar->isTriUniformMethod())
                        m_pPA->makeVertexDoubletDensities(m_pP3A->m_uRHS, m_pP3A->m_Mu);
                }

                computeInducedForces(0, 0, m_QInf); //  the mesh has been rotated, use the x-aligned wind direction

                if(m_pWPolar->isAdjustedVelocity())
                {
                    double v = computeBalanceSpeeds(0, mass, m_bError, QString("      "), strange);

                    if(v<=0.0)
                    {
                        onTraceLog("   " + strange);
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
                for(int i=0; i<m_pPA->m_uRHS.size(); i++) m_pPA->m_uRHS[i] *= QInfStab/m_QInf;

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
                    F = m_WingForce.first();
                }
//                CL = F.dot(windNormal(AlphaStab, BetaStab))/m_pWPolar->referenceArea();
                CL = F.dot(windNormal(0, 0))/m_pWPolar->referenceArea();


                if(m_pWPolar->isViscous() && m_pWPolar->bViscousLoop())
                {
                    log.clear();
                    if(updateVirtualTwist(QInfStab, error, log))
                    {
                        str = QString::asprintf("         iter=%3d   error=%9.5f", inl+1, error);
                        str += DEGCHAR + "   ";
                        str += QString::asprintf(" CL=%9.5f", CL);
                        onTraceLog(str + strange);
                    }
                    else
                    {
                        onTraceLog(log);
                        bViscLoopError = true;
                        break; // break viscous iterations
                    }
                }

                if(error<s_ViscAlphaPrecision) break;


                if (isCancelled()) return true;
            } // end viscous loop

            if(m_pWPolar->isViscous() && m_pWPolar->bViscousLoop())
            {
                if(!bViscLoopError)
                {
                    if(error<s_ViscAlphaPrecision)
                    {
                        str = "         --- Converged ---\n\n";
                        onTraceLog(str);
                        bConvergedLast = true;
                    }
                    else
                    {
                        str = "         Unconverged, max number of iterations exceeded, skipping operating point\n\n";
                        onTraceLog(str);
                        m_bError = true;
                        bConvergedLast = false;
                        bViscLoopError = true;
                    }
                }
                else
                {
                    str = "         Error in the viscous loop, skipping the operating point\n\n";
                    onTraceLog(str);
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

            if(s_bLiveUpdate && m_pEventDestination)
            {
                VPWUpdateEvent *pVPWEvent = new VPWUpdateEvent;
                pVPWEvent->m_Ctrl = m_Alpha;
                pVPWEvent->m_Vortons = m_pPA->m_Vorton;
                qApp->postEvent(m_pEventDestination, pVPWEvent);
            }

            if(m_pWPolar->bVortonWake())
                onTraceLog(QString::asprintf("      VPW iteration %3d/%3d      CL=%9.5f\n", ivw+1, nWakeIter, CL));

            if(isCancelled()) return true;
        } // end VPW loop

        onTraceLog("\n");

        if(bViscLoopError) continue; // move on to the next operating point calculation

        computeInducedDrag(0, 0, QInfStab);


        m_pPA->makeLocalVelocities(m_pPA->m_uRHS, m_pPA->m_vRHS, m_pPA->m_wRHS,
                                   m_pPA->m_uVLocal, m_pPA->m_vVLocal, m_pPA->m_wVLocal, windDirection(0, 0)*m_QInf); // only using first

        VField.fill(windDirection(0, 0)*QInfStab);
        m_pPA->computeOnBodyCp(VField, m_pPA->m_uVLocal, m_pPA->m_Cp);

        str = QString::asprintf("      Calculating plane for control parameter=%.3f\n", m_Ctrl);
        onTraceLog(str);

        PlaneOpp *pPOpp = computePlane(m_Ctrl, m_Alpha, BetaStab, m_Phi, QInfStab, mass, CoG, true);
        onTraceLog("\n");

        if(pPOpp)
        {
            //add the data to the polar object
            if(!pPOpp->isOut())
            {
                m_pWPolar->addPlaneOpPointData(pPOpp);
                m_PlaneOppList.append(pPOpp);
            }
        }

        if (isCancelled()) return true;
    }

    return true;
}


/**
 * Makes the field of freestream velocity vectors on panels, taking into account virtual twist.
 * Used to make the RHS in the control loop.
*/
void PlaneTask::addTwistedVelField(double QInf, double alpha, QVector<Vector3d> &VField) const
{
    if(!m_pPlane->isXflType()) return;
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);

    int iStrip = 0;
    int iPanel = 0;

    int coef = m_pWPolar->bThinSurfaces() ? 1 : 2; // top and bottom surfaces
    if(m_pWPolar->isTriangleMethod()) coef *= 2; // 2 triangles/quad
    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = pPlaneXfl->wingAt(iw);
        int nWingPanels = m_pWPolar->isQuadMethod() ? pWing->nPanel4() : pWing->nPanel3();
        int iWingPanel0=iPanel;
        for(int is=0; is<pWing->nSurfaces(); is++)
        {
            Surface const &surf = pWing->surfaceAt(is);

            // left tip patch
            if(m_pWPolar->bThickSurfaces() && surf.isClosedLeftSide())
            {
                do
                {
                    Q_ASSERT(m_pPA->panelAt(iPanel)->isSidePanel());
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
            if(m_pWPolar->bThickSurfaces() && surf.isClosedRightSide())
            {
                // there is a right tip patch in two cases
                //  - the wing is one-sided and closed at its inner side
                //  - the wing is two-side and the surface is the right tip surface
                if ( (!pWing->isTwoSided() && pWing->isClosedInnerSide()) ||
                      (pWing->isTwoSided() && surf.isTipRight()))
                do
                {
                    Q_ASSERT(m_pPA->panelAt(iPanel)->isSidePanel());
//                    VField[iPanel] = VInf; // no virtual twist corrections for tip patches
                    iPanel++;
                }
                while ((iPanel-iWingPanel0)<nWingPanels && (iPanel<m_pPA->nPanels()) && (m_pPA->panelAt(iPanel)->isSidePanel()));
            }
        }
    }
//    Q_ASSERT(iPanel==nPanels); not true if there is a fuselage
    Q_ASSERT(iStrip==m_pPlane->nStations());
}


/**
 Computationally Efficient Transonic and Viscous Potential Flow Aero-Structural Method for Rapid Multidisciplinary Design Optimization of Aeroelastic Wing Shaping Control, by Eric Ting and Daniel Chaparro,
  Advanced Modeling and Simulation (AMS) Seminar Series, NASA Ames Research Center, June 28, 2017
 */
bool PlaneTask::updateVirtualTwist(double QInf, double &error, QString &log)
{
    if(!m_pPlane->isXflType()) return false;
    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pPlane);

    QString strange, strong;

    Vector3d CoG; // dummy argument, unused
    Foil *pFoil0=nullptr;
    Foil *pFoil1=nullptr;

    double *Cp3Vtx=nullptr;
    int qrhs = 0;
    if(m_pWPolar->isTriangleMethod())
    {
        Cp3Vtx = m_pP3A->m_Cp.data() + qrhs*3*m_pP3A->nPanels();
    }
    double *mu4=nullptr,*cp4=nullptr;
    if(m_pWPolar->isQuadMethod())
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
        if(m_pWPolar->isTriangleMethod())
        {
            pWing->panel3ComputeStrips(m_pP3A->m_Panel3, m_pWPolar, CoG, m_Alpha, m_Beta, QInf, Cp3Vtx, m_SpanDistFF[iw]);
        }
        else if(m_pWPolar->isQuadMethod())
        {
            // Compute aero data on strips
            pWing->panel4ComputeStrips(m_pP4A->m_Panel4, m_pWPolar, CoG, m_Alpha, m_Beta, QInf, cp4, mu4, m_SpanDistFF[iw]);
        }

        SpanDistribs const&sd = m_SpanDistFF.at(iw);

        int m=0;
        for(int jSurf=0; jSurf<pWing->nSurfaces(); jSurf++)
        {
            Surface const &surf = pWing->surface(jSurf);
            for(int k=0; k<surf.NYPanels(); k++)
            {
                double Re = sd.m_Chord.at(m) * QInf / m_pWPolar->viscosity();

                double Cl_vlm = sd.m_Cl.at(m);

                double tau=0;
                pWing->getFoils(&pFoil0, &pFoil1, sd.m_StripPos.at(m), tau);
                double alpha_0 = Objects2d::getZeroLiftAngle(pFoil0, pFoil1, Re, tau);

                double aoa_effective = alpha_0 + (Cl_vlm/2.0/PI) *180.0/PI - m_gamma[iStation];
                bool bOutRe=false, bOutAlpha = false;
                double Cl_visc = Objects2d::getPlrPointFromAlpha(Polar::CL, pFoil0, pFoil1, Re, aoa_effective, tau, bOutRe, bOutAlpha);

                if(bOutRe || bOutAlpha)
                {
                    strange = QString("       ") + pWing->name() + QString::asprintf("  Span pos[%d]=%11g ", m, sd.m_StripPos.at(m)*Units::mtoUnit());
                    strange += Units::lengthUnitLabel();
                    strange += ",  Re = ";
                    strong = QString::asprintf("%.0f", Re);
                    strange += strong;
                    log += strange;
                    // interpolation error, makes no sense to continue
                    if(bOutAlpha)
                    {
                        strong = QString::asprintf(", aoa_effective=%7.3g", aoa_effective) + DEGCHAR;
                        log +=strong;
                    }
                    else if(bOutRe)
                    {
                        strong = QString::asprintf(", aoa_effective=%7.3g", aoa_effective) + DEGCHAR;
                        log += strong;
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
    for(int ie=0; ie<m_pWPolar->nAVLControls(); ie++)
    {
        strange = "      _____Control Matrices for set " + m_pWPolar->AVLCtrl(ie).name()+" __________\n";
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

    onTraceLog(log);
}



PlaneOpp* PlaneTask::computePlane(double ctrl, double alpha, double beta, double phi, double QInf, double mass, Vector3d const &CoG, bool bInGeomAxes)
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
        windD.set(windDirection(alpha, beta));
        windN.set(windNormal(alpha, beta));
    }

    double const *mu3=(nullptr), *sigma3(nullptr), *Cp3Vtx(nullptr);
    if(m_pP3A && m_pWPolar->isTriangleMethod())
    {
        mu3    = m_pP3A->m_Mu.data();
        sigma3 = m_pP3A->m_Sigma.data();
        Cp3Vtx = m_pP3A->m_Cp.data();
    }
    double const *mu4(nullptr), *sigma4(nullptr);
    double *cp4(nullptr);

    if(m_pWPolar->isQuadMethod())
    {
        mu4    = m_pP4A->m_Mu.data();
        sigma4 = m_pP4A->m_Sigma.data();
        cp4    = m_pP4A->m_Cp.data();
    }

    m_AF.resetResults();
//    m_PartAF.resize(m_pPlane->nParts());

    if(m_pWPolar->isType6())
    {
        m_AF.setOpp(alpha, m_Beta, m_Phi, QInf);
        for(int i=0; i<m_PartAF.size(); i++)  m_PartAF[i].setOpp(alpha, beta, m_Phi, QInf);
    }
    else
    {
        m_AF.setOpp(alpha, beta, m_pWPolar->phi(), QInf);
        for(int i=0; i<m_PartAF.size(); i++)  m_PartAF[i].setOpp(alpha, beta, m_pWPolar->phi(), QInf);
    }

    Vector3d Force, PlaneCP, Mi;

    if(pPlaneXfl)
    {
        int nWings = m_pPlane->nWings();
        Vector3d partforce;
        for(int iw=0; iw<nWings; iw++)
        {

            if(bInGeomAxes) partforce = windToGeomAxes(m_WingForce.at(iw), alpha, beta);
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
            if(m_pWPolar->isQuadMethod())
            {
                pWing->panel4ComputeInviscidForces(m_pP4A->m_Panel4, m_pWPolar, CoG, alpha, beta, QInf, cp4, mu4, m_PartAF[iw]);
                pWing->panel4ComputeStrips(m_pP4A->m_Panel4, m_pWPolar, CoG, alpha, beta, QInf, cp4, mu4, m_SpanDistFF[iw]);
            }
            else if(m_pWPolar->isTriangleMethod())
            {
                pWing->panel3ComputeInviscidForces(m_pP3A->m_Panel3, m_pWPolar, CoG, alpha, beta, Cp3Vtx, m_PartAF[iw]);
                pWing->panel3ComputeStrips(m_pP3A->m_Panel3, m_pWPolar, CoG, alpha, beta, QInf, Cp3Vtx, m_SpanDistFF[iw]);
            }

            if(bInGeomAxes)
            {
                partforce = windToGeomAxes(m_PartAF.at(iw).Fsum(), alpha, beta);
                partmi    = windToGeomAxes(m_PartAF.at(iw).Mi(), alpha, beta);
            }
            else
            {
                partforce = m_PartAF.at(iw).Fsum();
                partmi    = m_PartAF.at(iw).Mi();
            }

            if(m_pWPolar->isType6()) partforce.y *= -1.0;

            m_AF.addFsum(partforce);
            Mi += partmi;           //N.m/q
            m_PartAF[iw].setFsum(partforce);
            m_PartAF[iw].setMi(partmi);           //N.m/q


            if     (m_pWPolar->isQuadMethod())     pWing->panelComputeBending(m_pP4A->m_Panel4, m_pWPolar->bThinSurfaces(), m_SpanDistFF[iw]);
            else if(m_pWPolar->isTriangleMethod()) pWing->panelComputeBending(m_pP3A->m_Panel3, m_pWPolar->bThinSurfaces(), m_SpanDistFF[iw]);

            iStation += pWing->nStations();
            if(s_bCancel) break;
        }

        if(m_pWPolar->isViscInterpolated())
            onTraceLog("          Adding interpolated viscous drag...\n");
        else
            onTraceLog("          Calculating XFoil viscous drag on the fly...\n");

        iStation = 0;

        for(int iw=0; iw<nWings; iw++)
        {
            WingXfl *pWing = pPlaneXfl->wing(iw);
            if(m_pWPolar->isViscous())
            {
                onTraceLog("             Processing "+ pWing->name() + EOLCHAR);

                QString logmsg;

                bool bViscOK = true;

                if(m_pWPolar->isViscInterpolated())
                {
                    bViscOK = computeViscousDrag(pWing, alpha, beta, QInf, m_pWPolar, CoG, iStation, m_SpanDistFF[iw], logmsg);
                    if(!logmsg.isEmpty())
                    {
                        onTraceLog("             ...Viscous interpolation failures: \n");
                        onTraceLog(logmsg);
                    }
                }
                else
                {
                    bViscOK = computeViscousDragOTF(pWing, alpha, beta, QInf, m_pWPolar, CoG, m_pWPolar->flapCtrls(iw), m_SpanDistFF[iw], logmsg);
                    if(!logmsg.isEmpty())
                    {
                        onTraceLog("             ...XFoil OTF failures: ");
                        onTraceLog(logmsg);
                    }
                }

                if(!bViscOK)
                {
                    m_bError = true;
                    return nullptr; // no point in calculating the rest
                }
                pWing->computeViscousForces(m_pWPolar, alpha, beta, m_SpanDistFF[iw], m_PartAF[iw]);
                m_AF.addProfileDrag(m_PartAF.at(iw).profileDrag());           //N/q
                m_AF.addMv(m_PartAF.at(iw).Mv());                             //N.m/q

//                m_PartAF[iw].setProfileDrag(pWing->AF().profileDrag());           //N/q
//                m_PartAF[iw].setMv(pWing->AF().Mv());                             //N.m/q
            }
        }
        onTraceLog("             ...done.\n");

        // add fuse contribution to Centre of Pressure position, to pressure moments and to viscous properties
        if(m_pPlane->hasFuse())
        {
            Fuse *pFuse = pPlaneXfl->fuse(0);
            int ipart = m_pPlane->nWings();
            if(!m_pWPolar->bIgnoreBodyPanels())
            {
                Vector3d Force, Moment;
                if(m_pWPolar->isTriangleMethod())
                    pFuse->computeAero(m_pP3A->m_Panel3, Cp3Vtx, m_pWPolar, alpha, PlaneCP, Force, Moment);
                else if(m_pWPolar->isQuadMethod())
                    pFuse->computeAero(m_pP4A->m_Panel4, cp4,    m_pWPolar, alpha, PlaneCP, Force, Moment);

                if(bInGeomAxes)
                {
                    partforce = windToGeomAxes(Force,  alpha, beta);
                    partmi    = windToGeomAxes(Moment, alpha, beta);
                }
                else
                {
                    partforce = Force;
                    partmi    = Moment;
                }

                m_AF.addFsum(partforce);           // N/q
                if(m_pWPolar->bFuseMi())
                {
                    Mi += partmi;               // N.m/q
                    m_PartAF[ipart].setFsum(partforce);                             // N/q
                    m_PartAF[ipart].setMi(partmi);                                 // N.m/q
                }
            }

            if(m_pWPolar->hasFuseDrag())
            {
                Vector3d Force, Moment;
                pFuse->computeViscousForces(m_pWPolar, alpha, QInf, Force, Moment);
                m_AF.addFuseDrag(Force.dot(windD));
                m_PartAF[ipart].setFuseDrag(Force.dot(windD));

//                Vector3d drag(windD*pFuse->AF().viscousDrag());

                Vector3d leverarm = (pPlaneXfl->fusePos(0)+pFuse->inertia().CoG_t())-CoG;
                m_AF.addMv(leverarm * Force);
                m_PartAF[ipart].setMv(leverarm * Force);
            }
        }
    }

    if(m_pPlane->isSTLType())
    {
        //PlaneSTL
        if(bInGeomAxes)
            Force += windToGeomAxes(m_WingForce.at(0), alpha, beta);
        else
            Force += m_WingForce.at(0);                           // (N/q), body axes

        AeroForces AF;
        if(m_pWPolar && Cp3Vtx)
            computeInviscidAero(m_pP3A->m_Panel3, Cp3Vtx, m_pWPolar, alpha, AF);
        m_AF.addFsum(AF.Fsum());
        Mi = AF.Mi();
    }

    m_AF.setFff(Force);          // N/q, body axes
    m_AF.setMi(Mi);

    // add all extra drag
    if(m_pWPolar && m_pWPolar->hasExtraDrag())
    {
        m_AF.setExtraDrag(m_pWPolar->extraDragTotal(m_AF.CL()));
    }

    Vector3d M0;
    for(int iw=0; iw<m_PartAF.size(); iw++) // only one if STL
    {
        M0 += m_PartAF.at(iw).M0();
    }
    m_AF.setM0(M0);

    PlaneOpp *pPOpp(nullptr);
    if(m_pWPolar)
    {
        if(m_pWPolar->isType6())     beta = -beta;

        if(m_pWPolar->isTriangleMethod())
        {
            pPOpp = createPlaneOpp(ctrl, alpha, beta, phi, QInf, mass, CoG, Cp3Vtx, mu3, sigma3);
        }
        else if (m_pWPolar->isQuadMethod())
        {
            pPOpp = createPlaneOpp(ctrl, alpha, beta, phi, QInf, mass, CoG, cp4, mu4, sigma4);
        }
    }
    return pPOpp;
}


void PlaneTask::computeInviscidAero(QVector<Panel3> const &panel3, const double *Cp3Vtx,
                                    WPolar const *pWPolar, double Alpha,
                                    AeroForces &AF) const
{
    double panellift(0);
    Vector3d PanelForce, LeverArm;
    AF.resetResults();

    //   Define wind axes
    Vector3d WindNormal = windNormal(Alpha, 0.0);

    // Inviscid pressure forces
    Vector3d FiBodyAxes(0,0,0);
    Vector3d MiBodyAxes(0,0,0);
    Vector3d M0;
    for (int p=0; p<panel3.size(); p++)
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


double PlaneTask::computeBalanceSpeeds(double Alpha, double mass, bool &bWarning, QString const &prefix, QString &log) const
{
    Vector3d Force, WindN;
    WindN = windNormal(Alpha, m_Beta);
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
        Force = m_WingForce.first();
    }

    if(mass<1.e-6) onTraceLog(prefix + "Warning: zero mass - not possible to set a balance speed\n");

    double Lift =  Force.dot(WindN) ;      //N/q, for 1/ms
    if(Lift<=0.0)
    {
        log = prefix + "negative lift .... skipping the angle\n";

        bWarning  = true;
        return -100.0;
    }
    double v = sqrt(2.0* 9.81 * mass/m_pWPolar->density()/Lift);
    log = prefix + "V" + INFCHAR + QString::asprintf("=%7.3f ", v*Units::mstoUnit());
    log += Units::speedUnitLabel();
    return v;
}


double PlaneTask::computeGlideSpeed(double Alpha, double mass, QString &log)
{
    QString strange;
    Vector3d Force, WindNormal;

    WindNormal = windNormal(Alpha, m_Beta);
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

    double CL = Lift/m_pWPolar->referenceArea();
    double CDi = Force.dot(windDirection(Alpha, m_Beta))/m_pWPolar->referenceArea();
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

            if(m_pWPolar->isViscOnTheFly())
                computeViscousDragOTF(pWing, Alpha, 0.0, v1, m_pWPolar, Vector3d(), m_pWPolar->flapCtrls(iw), m_SpanDistFF[iw], strange);
            else
                computeViscousDrag(pWing, Alpha, 0.0, v1, m_pWPolar, Vector3d(), iStation, m_SpanDistFF[iw], strange);
            pWing->computeViscousForces(m_pWPolar, Alpha, m_Beta, m_SpanDistFF[iw], m_PartAF[iw]);
            CDv1 += m_PartAF.at(iw).profileDrag(); // N/q

            iStation += pWing->nStations();
        }
//        CDv0 /= m_pWPolar->referenceArea();
        CDv1 /= m_pWPolar->referenceArea();

//        v0 = sqrt(2*mass*9.81/m_pWPolar->density()/m_pWPolar->referenceArea()/sqrt(CL*CL+(CDi+CDv0)*(CDi+CDv0)));
        v1 = sqrt(2*mass*9.81/m_pWPolar->density()/m_pWPolar->referenceArea()/sqrt(CL*CL+(CDi+CDv1)*(CDi+CDv1)));

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
        log = "V" + INFCHAR + QString::asprintf("= %g ", v1*Units::mstoUnit()) + Units::speedUnitLabel();
        log += QString::asprintf(" converged in %d iterations", iter);
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
            if(m_pWPolar->isQuadMethod())
                m_pP4A->inducedForce(pWing->nPanel4(), QInf, alpha, beta, pos, forcebodyaxes, m_SpanDistFF[iw]);
            else if(m_pP3A && m_pWPolar->isTriangleMethod())
                m_pP3A->inducedForce(pWing->nPanel3(), QInf, alpha, beta, pos, forcebodyaxes, m_SpanDistFF[iw]);

            //save the results... will save another FF calculation when computing the operating points
            m_WingForce[iw] = forcebodyaxes;     // N/q, body axes
//            m_SpanDistFF[iw] = pWing->spanDistFF();

            if      (m_pWPolar->isTriangleMethod()) pos += pWing->nPanel3();
            else if (m_pWPolar->isQuadMethod())     pos += pWing->nPanel4();

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
                if(m_pWPolar->isQuadMethod())
                    m_pP4A->trefftzDrag(pWing->nPanel4(), QInf, alpha, beta, pos, FFForceBodyAxes, m_SpanDistFF[iw]);
                else if(m_pWPolar->isTriangleMethod())
                    m_pP3A->trefftzDrag(pWing->nPanel3(), QInf, alpha, beta, pos, FFForceBodyAxes, m_SpanDistFF[iw]);
            }
            m_WingForce[iw] += FFForceBodyAxes;     // N/q, body axes

            if      (m_pWPolar->isTriangleMethod()) pos += pWing->nPanel3();
            else if (m_pWPolar->isQuadMethod())     pos += pWing->nPanel4();

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
    if(m_pWPolar->isTriangleMethod()) nPanel3=m_pP3A->nPanels();
    else                              nPanel4=m_pP4A->nPanels();

    PlaneOpp *pPOpp = new PlaneOpp(m_pPlane, m_pWPolar, nPanel4, nPanel3);
    if(!pPOpp) return nullptr;

    pPOpp->setTheStyle(m_pWPolar->theStyle());
    pPOpp->setVisible(true);

    PlaneOpp *pLastPOpp(nullptr);
    if(m_PlaneOppList.size()) pLastPOpp = m_PlaneOppList.back();
    if(pLastPOpp) pPOpp->setLineColor(pLastPOpp->lineColor().darker(xfl::darkFactor()));

    int iStation = 0;
    if(pPlaneXfl)
    {
        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            WingXfl const*pWing = pPlaneXfl->wingAt(iw);

            pPOpp->addWingOpp(pWing->nPanel4());
            pPOpp->m_WingOpp[iw].createWOpp(pWing, m_pWPolar, m_SpanDistFF.at(iw), m_PartAF.at(iw));

            if(m_pWPolar->isViscous() && m_pWPolar->bViscousLoop())
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

    if(m_pWPolar->isTriangleMethod())
    {
        int N = m_pP3A->nPanels();
        int N3 = N*3;
        if(Cp)    memcpy(pPOpp->m_Cp.data(),    Cp,      ulong(N3)*sizeof(double));
        if(Gamma) memcpy(pPOpp->m_gamma.data(), Gamma,   ulong(N3)*sizeof(double));
        if(Sigma) memcpy(pPOpp->m_sigma.data(), Sigma,   ulong(N)*sizeof(double));
    }
    else if (m_pWPolar->isQuadMethod())
    {
        int N = m_pP4A->nPanels();
        if(Cp)    memcpy(pPOpp->m_Cp.data(),    Cp,    ulong(N)*sizeof(double));
        if(Gamma) memcpy(pPOpp->m_gamma.data(), Gamma, ulong(N)*sizeof(double));
        if(Sigma) memcpy(pPOpp->m_sigma.data(), Sigma, ulong(N)*sizeof(double));
    }


    pPOpp->m_bOut   = false; // v7.50: only keeping successful viscous calculations

    pPOpp->m_Mass   = mass;
    pPOpp->m_CoG    = CoG;
    pPOpp->m_Inertia[0] = m_pWPolar->Ixx();
    pPOpp->m_Inertia[1] = m_pWPolar->Iyy();
    pPOpp->m_Inertia[2] = m_pWPolar->Izz();
    pPOpp->m_Inertia[3] = m_pWPolar->Ixz();


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
                    pPOpp->m_WingOpp[iw].m_FlapMoment.append(pWing->flapMomentAt(i));
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

    if(m_pWPolar->bVortonWake())
    {
        pPOpp->m_Vorton = m_pPA->m_Vorton;
        pPOpp->m_VortexNeg = m_pPA->m_VortexNeg;
    }
    return pPOpp;
}


bool PlaneTask::T7Loop()
{
    QString str, outstring;

    onTraceLog("\nSolving the problem... \n\n");

    m_pPA->m_nStations = m_pPlane->nStations();// for assertion checks only?

    m_nRHS = 1;
    m_qRHS = 0;

    m_Ctrl = 0.0;
    m_Beta = 0.0;
    m_Phi  = m_pWPolar->phi();

    m_pPA->makeWakePanels(windDirection(0,0), false);

    setLinearSolution();

    onTraceLog("   Making unit panel velocities...");
    m_pPA->makeLocalVelocities(m_pPA->m_uRHS, m_pPA->m_vRHS, m_pPA->m_wRHS, m_pPA->m_uVLocal, m_pPA->m_vVLocal, m_pPA->m_wVLocal, windDirection(0,0));
    onTraceLog("   done\n");

    Vector3d CoG = m_pWPolar->CoGCtrl(m_Ctrl);
    double mass = m_pWPolar->massCtrl(m_Ctrl);

    outstring.clear();

    onTraceLog(outstring);
    if(isCancelled()) return true;

    // next find the balanced and trimmed conditions
    double AlphaEq(0), u0(0);

    onTraceLog("      Computing trimmed conditions\n");

    if(!m_pPA->computeTrimmedConditions(mass, CoG, AlphaEq, u0, m_pWPolar->bFuseMi()))
    {
        if(isCancelled()) return true;
        //no zero moment alpha
        str = QString::asprintf("      Unsuccessful attempt to trim the model for control position=%.2f - skipping.\n\n\n", m_Ctrl);
        onTraceLog(str);
        m_bError = true;
    }
    else
    {
        m_QInf = u0;

        onTraceLog("      Calculating far field forces...\n");
        computeInducedForces(AlphaEq, m_Beta, 1.0);
        computeInducedDrag(AlphaEq, m_Beta, 1.0);
        scaleResultsToSpeed(1.0, u0);

        if (isCancelled()) return true;

        str = QString::asprintf("      Calculating Plane for alpha=%.2f", AlphaEq) + DEGCHAR + EOLCHAR;
        onTraceLog(str);
        PlaneOpp *pPOpp = computePlane(m_Ctrl, AlphaEq, m_Beta, m_Phi, u0, mass, CoG, false);
        if(!pPOpp)
        {
            onTraceLog("           Error generating the operating point... discarding\n\n");
            return true;
        }

        if (isCancelled()) return true;

        m_PlaneOppList.append(pPOpp);
        if(computeStability(pPOpp, true))
        {
            //add the data to the polar object
            if(pPOpp && !pPOpp->isOut())
                m_pWPolar->addPlaneOpPointData(pPOpp);
        }
        onTraceLog("       Done operating point\n\n");
    }

    return true;
}


bool PlaneTask::computeStability(PlaneOpp *pPOpp, bool bOutput)
{
    QString str;
    // Compute stability and control derivatives in stability axes
    m_pPA->computeStabilityDerivatives(pPOpp->alpha(), pPOpp->QInf(), pPOpp->cog(), m_pWPolar->bFuseMi(), pPOpp->m_SD, m_Force0, m_Moment0);
    if(isCancelled()) return true;

    pPOpp->m_SD.resizeControlDerivatives(m_pWPolar->nAVLControls());

    if(m_pWPolar->hasActiveAVLControl())
        computeControlDerivatives(m_Ctrl, pPOpp->alpha(), pPOpp->QInf(), pPOpp->m_SD); //single derivative, w.r.t. the polar's control variable
    if(isCancelled()) return true;



    pPOpp->m_SD.setMetaData(m_pWPolar->referenceSpanLength(), m_pWPolar->referenceChordLength(), m_pWPolar->referenceArea(),
                            pPOpp->QInf(), pPOpp->mass(), pPOpp->cog().x, m_pWPolar->density());
    pPOpp->m_SD.computeNDStabDerivatives();
    if (bOutput) outputNDStabDerivatives(pPOpp->QInf(), pPOpp->m_SD);


    // build the state matrices
    if(pPOpp->mass()<PRECISION)
    {
        onTraceLog("          Null mass - skipping calculation of eigenthings\n");
    }
    else
    {
        // Compute inertia in stability axes
        m_pWPolar->Ixx();

        pPOpp->computeStabilityInertia(m_pWPolar->inertia());

        pPOpp->buildStateMatrices(m_pWPolar->nAVLControls());
        if(bOutput) outputStateMatrices(pPOpp);

        // Solve for eigenvalues

        if(!pPOpp->solveEigenvalues(str))
        {
            onTraceLog("             Unsuccessful attempt to compute eigenvalues\n");
            if(m_pWPolar->isType7()) m_bError = true;
            else m_bWarning = true;
        }
        else
        {
            // output the eigenvalues
            onTraceLog("          Eigenthings computed successfully\n");
            if (bOutput)
            {
                pPOpp->outputEigen(str);
                onTraceLog(str+EOLCHAR);
            }
        }
    }
    return true;
}


bool PlaneTask::T123458Loop()
{
    QString strange, str, outstring;

    onTraceLog("\nSolving the problem... \n\n");

    m_pPA->m_nStations = m_pPlane->nStations();// for assertion checks only?

    m_pPA->makeWakePanels(windDirection(0,0), false);
    m_pPA->savePanels();

    setLinearSolution();

    onTraceLog("   Making unit panel velocities... ");
    int N = m_pPA->m_uVLocal.size();
    QVector<Vector3d> uVLocal(N), vVLocal(N), wVLocal(N); // keep a copy since these velocity arrays are modified during the calculation of derivatives
    m_pPA->makeLocalVelocities(m_pPA->m_uRHS, m_pPA->m_vRHS, m_pPA->m_wRHS, uVLocal, vVLocal, wVLocal, windDirection(0,0));

    onTraceLog("   done\n");

    Vector3d CoG = m_pWPolar->CoGCtrl(m_Ctrl);
    double mass = m_pWPolar->massCtrl(m_Ctrl);

    if(isCancelled()) return true;

    m_nRHS = m_T8Opps.size();
    strange = QString::asprintf("   Calculating %d operating point", m_nRHS);
    if(m_nRHS>1) strange +="s";
    onTraceLog(EOLCHAR+strange+EOLCHAR);

    for(int io=0; io<m_T8Opps.size(); io++)
    {
        T8Opp const &t8opp = m_T8Opps.at(io);

        if(!t8opp.isActive()) continue;
        m_qRHS = io;
        m_Alpha = t8opp.alpha();
        m_Beta  = t8opp.beta();
        m_Phi   = m_pWPolar->phi();
        m_QInf  = t8opp.Vinf();
        m_Ctrl  = 0.0;

        outstring = QString("     ");
        outstring += ALPHACHAR + QString::asprintf("=%g", m_Alpha) + DEGCHAR + ", ";
        outstring += BETACHAR  + QString::asprintf("=%g", m_Beta)  + DEGCHAR + ", ";
        outstring += PHICHAR   + QString::asprintf("=%g", m_Phi)   + DEGCHAR + ", ";

        if(m_pWPolar->isType1() || m_pWPolar->isType5() || m_pWPolar->isType8())
            outstring += "V" + INFCHAR + QString::asprintf("=%g ", m_QInf*Units::mstoUnit()) + Units::speedUnitLabel() + EOLCHAR;
        else
            outstring += "V" + INFCHAR + ": adjusted" + EOLCHAR;

        onTraceLog(outstring);

        onTraceLog("       Creating source strengths...\n");
        m_pPA->makeSourceStrengths(windDirection(m_Alpha, m_Beta));

        onTraceLog("       Calculating doublet strengths...\n");
        m_pPA->makeUnitDoubletStrengths(m_Alpha, m_Beta);

        onTraceLog("       Calculating far field forces...\n");

        computeInducedForces(m_Alpha, m_Beta, 1.0);
        computeInducedDrag(  m_Alpha, m_Beta, 1.0);

        if(m_pWPolar->isType1() || m_pWPolar->isType5())
        {
            m_QInf = m_pWPolar->velocity();
        }
        else if(m_pWPolar->isType2() || m_pWPolar->isType3())
        {
            onTraceLog("       Calculating balance speeds...\n");
            if (m_pWPolar->isFixedLiftPolar())
                m_QInf = computeBalanceSpeeds(m_Alpha, m_pWPolar->mass(), m_bError, QString(), strange);
            else if(m_pWPolar->isGlidePolar())
                m_QInf = computeGlideSpeed(m_Alpha, m_pWPolar->mass(), strange);
            strange = QString("             ") + strange + EOLCHAR;
            onTraceLog(strange);
        }
        else if(m_pWPolar->isType8())
        {
            m_QInf = t8opp.m_Vinf;
        }
        else
        {
            Q_ASSERT(false);
        }

        scaleResultsToSpeed(1.0, m_QInf);

        if (isCancelled()) return true;

        onTraceLog("       Calculating on-body pressure coefficients...\n");
        QVector<Vector3d> VLocal;
        QVector<Vector3d> VInf(m_pPA->nPanels());
        VInf.fill(windDirection(m_Alpha, m_Beta));

        // Save a little time by restoring the unit velocty fields instead of recalculating them
        m_pPA->m_uVLocal = uVLocal;
        m_pPA->m_vVLocal = vVLocal;
        m_pPA->m_wVLocal = wVLocal;

        m_pPA->combineLocalVelocities(m_Alpha, m_Beta, VLocal);
        m_pPA->computeOnBodyCp(VInf, VLocal, m_pPA->m_Cp);
        if (isCancelled()) return true;

        str = QString::asprintf("       Calculating plane\n");
        onTraceLog(str);
        PlaneOpp *pPOpp = computePlane(m_Ctrl, m_Alpha, m_Beta, m_pWPolar->phi(), m_QInf, mass, CoG, false);
        if(!pPOpp)
        {
            onTraceLog("\n          Error generating the operating point... discarding\n\n");
            continue;
        }

        if (isCancelled()) return true;

        computeStability(pPOpp, true);

        //add the data to the polar object
        if(pPOpp && !pPOpp->isOut())
            m_pWPolar->addPlaneOpPointData(pPOpp);

        m_PlaneOppList.append(pPOpp);
        onTraceLog("          Done operating point\n\n");
    }

    return true;
}


void PlaneTask::outputNDStabDerivatives(double u0, StabDerivatives const &SD)
{
    QString str;

    double q = 1./2. * m_pWPolar->density() * u0 * u0;
    double b   = m_pWPolar->referenceSpanLength();
    double S   = m_pWPolar->referenceArea();
    double mac = m_pPlane->mac();


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

    str = QString::asprintf("Neutral Point position = %g ", SD.XNP*Units::mtoUnit()) + Units::lengthUnitLabel();
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
    logmsg += prefix + str + EOLCHAR;

    //output control derivatives
    if(!m_pWPolar->hasActiveAVLControl())
    {
        onTraceLog(logmsg);
        return;
    }

    str = QString("             Control derivatives\n");
    logmsg += str;
    for(int ie=0; ie<m_pWPolar->nAVLControls(); ie++)
    {
        if(SD.ControlNames.size()>ie)
        {
            logmsg += prefix + SD.ControlNames.at(ie) + "\n";
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

    onTraceLog(logmsg);
}


void PlaneTask::computeControlDerivatives(double t7ctrl, double alphaeq, double u0, StabDerivatives &SD)
{
    Vector3d WindDirection, Force, Moment, V0, is, js, ks;

    int N = 0;
    if      (m_pWPolar->isQuadMethod())     N = m_pP4A->nPanels();
    else if (m_pWPolar->isTriangleMethod()) N = m_pP3A->nPanels();

    double const *sigma = nullptr;
    if      (m_pWPolar->isQuadMethod())     sigma = m_pP4A->m_Sigma.data();
    else if (m_pWPolar->isTriangleMethod()) sigma = m_pP3A->m_Sigma.data();

    // Define the stability axes and the freestream velocity field
    double cosa = cos(alphaeq*PI/180.0);
    double sina = sin(alphaeq*PI/180.0);
    V0.set(u0*cosa, 0.0, u0*sina);
    WindDirection.set(cosa, 0.0, sina);
    is.set(-cosa, 0.0, -sina);
    js.set(  0.0, 1.0,   0.0);
    ks.set( sina, 0.0, -cosa);

    double DeltaCtrl = 0.001;

    for(int ie=0; ie<m_pWPolar->nAVLControls(); ie++)
    {
        onTraceLog("      Processing control set " + m_pWPolar->AVLCtrl(ie).name()+"\n");
        SD.ControlNames[ie] = m_pWPolar->AVLCtrl(ie).name();
        if(!m_pWPolar->AVLCtrl(ie).hasActiveAngle())
        {
            SD.Xde[ie] = SD.Yde[ie] = SD.Zde[ie] = SD.Lde[ie] = SD.Mde[ie] = SD.Nde[ie] = 0.0;
            onTraceLog("        No active gain... skipping\n\n");
            continue;
        }

        QString outstring;
        m_pPA->restorePanels();
        if(m_pPlane->isXflType())
        {
            PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(m_pPlane);
            if(m_pP4A)
            {
                setControlPositions(pPlaneXfl, m_pWPolar, m_pP4A->m_Panel4, DeltaCtrl, ie, outstring);
            }
            else if(m_pP3A)
            {
                setControlPositions(pPlaneXfl, m_pWPolar, m_pP3A->m_Panel3,
                                    m_pP3A->m_pRefTriMesh->nodes(), DeltaCtrl, ie, outstring);
            }
        }
        m_pPA->makeWakePanels(windDirection(0,0), false);

        //create the RHS
        onTraceLog("         Making the unit RHS vectors...\n");
        QVector<Vector3d> VField(N, V0);
//        Vector3d Omega;
//        m_pPA->makeUnitRHSVectors(); // could also use one RHS
//        m_pPA->combineUnitRHS(m_pPA->m_cRHS, V0, Omega);

        m_pPA->makeRHS(VField, m_pPA->m_cRHS, nullptr);

        QString strong = "         Calculating the control derivatives\n\n";
        onTraceLog(strong);

        //LU solve
        m_pPA->backSubRHS(m_pPA->m_cRHS);

        // make node vertex array
        // only needed for triuniform method to align with TriLinAnalysis
        QVector<double> cRHSVertex;
        double *muc = nullptr;
        Vector3d CoG = m_pWPolar->CoGCtrl(t7ctrl);
        Vector3d VInf = windDirection(alphaeq, 0.0) * u0;

        if(m_pWPolar->isQuadMethod())
        {
            muc = m_pPA->m_cRHS.data();
            m_pPA->forces(muc, sigma, alphaeq, 0.0, CoG, m_pWPolar->bFuseMi(), VField, Force, Moment);
        }
        else if(m_pWPolar->isTriangleMethod())
        {
            QVector<double> notanrhs(m_pPA->m_cRHS.size());
            m_pPA->makeLocalVelocities(m_pPA->m_cRHS, notanrhs, notanrhs, m_pPA->m_uVLocal, m_pPA->m_vVLocal, m_pPA->m_wVLocal, windDirection(alphaeq, 0.0));
            if(m_pWPolar->isTriUniformMethod() && m_pP3A)
            {
                cRHSVertex.resize(3*N);
                m_pP3A->makeVertexDoubletDensities(m_pPA->m_cRHS, cRHSVertex);
                muc = cRHSVertex.data();
            }
            else if(m_pWPolar->isTriLinearMethod())
            {
                muc = m_pPA->m_cRHS.data();
            }

            VField.fill(VInf);
            m_pPA->computeOnBodyCp(VField, m_pPA->m_uVLocal, m_pPA->m_Cp);
            m_pPA->forces(muc, sigma, alphaeq, 0.0, CoG, m_pWPolar->bFuseMi(), VField, Force, Moment);
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
void PlaneTask::setControlPositions(PlaneXfl const*pPlaneXfl, WPolar const*pWPolar,
                                    QVector<Panel4> &panel4, double deltactrl,
                                    int iAVLCtrl,
                                    QString &outstring)
{
    Vector3d H, Origin;
    Vector3d YVector(0.0, 1.0, 0.0);
    QString strange, strong;
    double totalAngle=0.0, deltaangle = 0.0;
    double gain = 0.0;

    int nCtrl = 0;

    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = pPlaneXfl->wingAt(iw);
        int iCtrl=0;
        if(iAVLCtrl>=0 && iAVLCtrl<pWPolar->nAVLControls()) gain = pWPolar->AVLGain(iAVLCtrl, nCtrl);
        else                                                gain = 0.0;
        deltaangle = deltactrl * gain; // degrees

        // first control value is the overall Y rotation
        if (fabs(deltaangle)>ANGLEPRECISION)
        {
            //rotate the normals and control point positions
            H.set(0.0, 1.0, 0.0);

            totalAngle = pPlaneXfl->ryAngle(iw) + deltaangle;
            strange = "   Rotating " + pWing->name();
            strong = QString::asprintf(" by %.f°, total angle is %.f°\n", deltaangle, totalAngle);
            outstring += strange + strong;

            Origin = pPlaneXfl->wingLE(iw);

            for(int i=0; i<pWing->nPanel4(); i++)
            {
                int p = pWing->firstPanel4Index() + i;
                panel4[p].rotate(Origin, YVector, deltaangle);
            }
        }

        // following control values are for the flaps
        iCtrl++;
        nCtrl++;
        for (int jSurf=0; jSurf<pWing->surfaceCount(); jSurf++)
        {
            Surface const &surf = pWing->surfaceAt(jSurf);
            if(surf.hasTEFlap())
            {
                if(iAVLCtrl>=0 && iAVLCtrl<pWPolar->nAVLControls()) gain = pWPolar->AVLGain(iAVLCtrl, nCtrl);
                else                                                gain = 0.0;
                deltaangle = deltactrl * gain; // degrees

                if (fabs(deltaangle)>ANGLEPRECISION)
                {
                    //Add delta rotations to initial control setting and to wing or flap delta rotation
                    if(fabs(surf.foilA()->TEFlapAngle())>0.0 && fabs(surf.foilB()->TEFlapAngle())>0.0)
                        totalAngle = deltaangle + (surf.foilA()->TEFlapAngle() + surf.foilB()->TEFlapAngle())/2.0;
                    else
                        totalAngle = deltaangle;

                    strange = QString::asprintf("- rotating flap %d by %g°, total flap angle is %g°", iCtrl, deltaangle, totalAngle);

                    strange = "      " + pWing->name()+strange+"\n";
                    outstring +=strange;

                    for(int i4=0; i4<panel4.size(); i4++)
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

    outstring  +="\n";
}


/** Sets the angle positions of wings and flap for a stability analysis
 * @param  iAVLCtrl the index of the AVL-type control, e.g. aileron, rudder, elevator
 * @param deltactrtl the amplitude of the control
 */
void PlaneTask::makeControlBC(PlaneXfl const*pPlaneXfl, WPolar const*pWPolar,
                              Vector3d *normals, double deltactrl, int iAVLCtrl,
                              QString &outstring)
{
    Vector3d H, Origin;
    Vector3d YVector(0.0, 1.0, 0.0);
    QString strange, strong;
    double totalangle(0.0), deltaangle(0.0);
    double gain(0.0);

    int nCtrl = 0;

    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = pPlaneXfl->wingAt(iw);
        int iCtrl=0;
        if(iAVLCtrl>=0 && iAVLCtrl<pWPolar->nAVLControls()) gain = pWPolar->AVLGain(iAVLCtrl, nCtrl);
        else                                                gain = 0.0;
        deltaangle = deltactrl * gain; // degrees

        // first control value is the overall Y rotation
        if (fabs(deltaangle)>ANGLEPRECISION)
        {
            //rotate the normals and control point positions
            H.set(0.0, 1.0, 0.0);

            totalangle = pPlaneXfl->ryAngle(iw) + deltaangle;
            strange = "   Rotating " + pWing->name();
            strong = QString::asprintf(" by %.f°, total angle is %.f°\n", deltaangle, totalangle);
            outstring += strange + strong;

            Origin = pPlaneXfl->wingLE(iw);

            Quaternion qt(deltaangle, H);

            if(pWPolar->isQuadMethod())
            {
                for(int i=0; i<pWing->nPanel4(); i++)
                {
                    int p = pWing->firstPanel4Index() + i;
                    qt.conjugate(normals[p]);
                }
            }
            else if(pWPolar->isTriangleMethod())
            {
                for(int i=0; i<pWing->nPanel3(); i++)
                {
                    int p = pWing->firstPanel3Index() + i;
                    qt.conjugate(normals[p]);
                }
            }
        }

        // following control values are for the flaps
        iCtrl++;
        nCtrl++;
        for (int jSurf=0; jSurf<pWing->surfaceCount(); jSurf++)
        {
            Surface const &surf = pWing->surfaceAt(jSurf);
            if(surf.hasTEFlap())
            {
                if(iAVLCtrl>=0 && iAVLCtrl<pWPolar->nAVLControls()) gain = pWPolar->AVLGain(iAVLCtrl, nCtrl);
                else                                                gain = 0.0;
                deltaangle = deltactrl * gain; // degrees

                if (fabs(deltaangle)>ANGLEPRECISION)
                {
                    //Add delta rotations to initial control setting and to wing or flap delta rotation
                    if(fabs(surf.foilA()->TEFlapAngle())>0.0 && fabs(surf.foilB()->TEFlapAngle())>0.0)
                        totalangle = deltaangle + (surf.foilA()->TEFlapAngle() + surf.foilB()->TEFlapAngle())/2.0;
                    else
                        totalangle = deltaangle;

//                    strange = QString::fromUtf8("- rotating flap %1 by %2°, total angle is %3°").arg(iCtrl).arg(deltaAngle, 5, 'f',2).arg(totalAngle, 5, 'f',2);
                    strange = QString::asprintf("- rotating flap %d by %g°, total flap angle is %g°", iCtrl, deltaangle, totalangle);

                    strange = "      " + pWing->name()+strange+"\n";
                    outstring +=strange;

                    Quaternion qt(deltaangle, surf.hingeVector());
                    for(int ip=0; ip<m_pPA->nPanels(); ip++)
                    {
                        if(pWPolar->isQuadMethod())
                        {
                            if(surf.hasFlapPanel4(ip))
                            {
                                qt.conjugate(normals[ip]);
                            }
                        }
                        else if(pWPolar->isTriangleMethod())
                        {
                            if(surf.hasFlapPanel3(ip))
                            {
                                qt.conjugate(normals[ip]);
                            }
                        }
                    }
                }
                iCtrl++;
                nCtrl++;
            }
        }
    }

    outstring  +="\n";
}


/** Sets the angle positions of wings and flap for a stability analysis */
void PlaneTask::setControlPositions(PlaneXfl const *pPlaneXfl, WPolar const *pWPolar,
                                    QVector<Panel3> &panel3, QVector<Node> const &refnodes, double deltactrl, int iAVLCtrl,
                                    QString &outstring)
{
    Vector3d H, Origin;
    Vector3d YVector(0.0, 1.0, 0.0);
    QString strange, strong;
    double totalAngle(0.0), deltaangle(0);
    double gain(0);
    QVector<Node> node = refnodes;

    int nCtrl = 0;
    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = pPlaneXfl->wingAt(iw);
        int iCtrl=0;

        if(iAVLCtrl>=0 && iAVLCtrl<pWPolar->nAVLControls()) gain = pWPolar->AVLGain(iAVLCtrl, nCtrl);
        else                                                gain = 0;
        deltaangle = gain * deltactrl; // degrees

        // first control value is the overall Y rotation
        if(fabs(deltaangle)>ANGLEPRECISION)
        {
            //rotate the normals and control point positions
            H.set(0.0, 1.0, 0.0);

            totalAngle = pPlaneXfl->ryAngle(iw) + deltaangle;
            strange = "   Rotating " + pWing->name();
            strong = QString::asprintf(" by %.f°, total angle is %.f°\n", deltaangle, totalAngle);
            outstring += strange + strong;

            Origin = pPlaneXfl->wingLE(iw);

            pPlaneXfl->rotateWingNodes(panel3, node, pWing, Origin, YVector, deltaangle);
        }

        // following control values are for the flaps
        iCtrl++;
        nCtrl++;
        for (int jSurf=0; jSurf<pWing->surfaceCount(); jSurf++)
        {
            Surface const &surf = pWing->surfaceAt(jSurf);
            if(surf.hasTEFlap())
            {
                if(iAVLCtrl>=0 && iAVLCtrl<pWPolar->nAVLControls()) gain = pWPolar->AVLGain(iAVLCtrl, nCtrl);
                else                                                gain = 0;
                deltaangle = gain * deltactrl; // degrees

                if(fabs(deltaangle)>ANGLEPRECISION)
                 {
                    //Add delta rotations to initial control setting and to wing or flap delta rotation
                    if(fabs(surf.foilA()->TEFlapAngle())>0.0 && fabs(surf.foilB()->TEFlapAngle())>0.0)
                        totalAngle = deltaangle + (surf.foilA()->TEFlapAngle() + surf.foilB()->TEFlapAngle())/2.0;
                    else
                        totalAngle = deltaangle;

                    strange = QString::asprintf("- rotating flap %d by %g°, total flap angle is %g°", iCtrl, deltaangle, totalAngle);
                    strange = "      " + pWing->name()+strange+"\n";
                    outstring +=strange;

                    pPlaneXfl->rotateFlapNodes(panel3, node, surf, surf.hingePoint(), surf.hingeVector(), deltaangle);
                }
                iCtrl++;
                nCtrl++;
            }
        }
    }


    TriMesh::rebuildPanelsFromNodes(panel3, node);

    outstring  +="\n";
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
        emit taskFinished();
        return;
    }

    m_AnalysisStatus = xfl::RUNNING;

    if(s_bCancel || !m_pWPolar)
    {
        m_AnalysisStatus = xfl::CANCELLED;
        return;
    }

    loop();

    m_bWarning = m_bWarning || m_pPA->m_bWarning;
    m_bError = m_bError || m_pPA->m_bWarning;

    if(m_AnalysisStatus!=xfl::CANCELLED) m_AnalysisStatus = xfl::FINISHED;

    emit taskFinished();
}


bool PlaneTask::computeViscousDrag(WingXfl *pWing, double alpha, double beta, double QInf,
                                   WPolar const *pWPolar, Vector3d const &cog, int iStation0, SpanDistribs &SpanResFF, QString &logmsg) const
{
    QString strong, strange, strOut;

    // Define the wind axes
    Vector3d winddirection = windDirection(alpha, beta);
    Vector3d windside      = windSide(alpha, beta);


    bool bViscOK = true;

    SpanDistribs &sd = SpanResFF;

    sd.m_Re.clear();
    for (int m=0; m<pWing->nStations(); m++)  sd.m_Re.push_back(SpanResFF.m_Chord.at(m) * QInf /pWPolar->viscosity());

    int m=0;// wing station counter
    for (int j=0; j<pWing->surfaceCount(); j++)
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
                    strOut = QString::asprintf(",  AoA_effective = %7.2f", aoa_effective) + DEGCHAR + EOLCHAR;
                    bViscOK = false;
                }
            }

            strong = QString("           ") + QString::asprintf("     Span position %9.2f ", sd.m_StripPos.at(m)*Units::mtoUnit());
            strong += Units::lengthUnitLabel();
            strong += QString::asprintf(",  Re = %9.0f", sd.m_Re.at(m));

            if(bOutVar)
            {
                logmsg += strong + strOut;
            }
            else if(bOutRe)
            {
                strange = QString::asprintf(",  Cl = %7.2f\n", sd.m_Cl.at(m));
                logmsg += strong + strange;
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

    return bViscOK;
}


bool PlaneTask::computeViscousDragOTF(WingXfl *pWing, double alpha, double beta, double QInf,
                                      WPolar const *pWPolar, Vector3d const &cog, AngleControl const &TEFlapAngles, SpanDistribs &SpanResFF, QString &logmsg)
{
    // on the fly viscous drag calculation
    // for each surface, calulate the drag at each end foil for each lift and reynolds at each span station
    // then interpolate
    QString strong, strange;

    Q_ASSERT(pWing->nFlaps()==TEFlapAngles.nValues());

    // Define the wind axes
    Vector3d winddirection = windDirection(alpha, beta);
    Vector3d windside      = windSide(alpha, beta);

    SpanDistribs &sd = SpanResFF;
    // Calculate the Reynolds number on each strip

    sd.m_Re.clear();
    for (int m=0; m<pWing->nStations(); m++)  sd.m_Re.push_back(SpanResFF.m_Chord.at(m) * QInf /pWPolar->viscosity());

    Polar a2dPolar;
    a2dPolar.setType(xfl::T1POLAR);
    a2dPolar.setReType(1);
    a2dPolar.setMaType(1);
    a2dPolar.setMach(0.0);
    a2dPolar.setNCrit(m_pPolar3d->NCrit());
    a2dPolar.setXTripTop(m_pPolar3d->XTrTop());
    a2dPolar.setXTripBot(m_pPolar3d->XTrBot());

    int iCtrl = 0;
    int m=0;// wing station counter
    for (int jsurf=0; jsurf<pWing->surfaceCount(); jsurf++)
    {
        onTraceLog(QString::asprintf("                Processing surface %d\n", jsurf+1));
        Surface const &surf = pWing->surface(jsurf);

        Foil foilA, foilB;
        foilA.copy(surf.foilA(), true);
        foilB.copy(surf.foilB(), true);


        if(surf.hasTEFlap())
        {
            double theta = TEFlapAngles.value(iCtrl);
            foilA.setTEFlapAngle(theta);
            foilA.setFlaps();
            foilB.setTEFlapAngle(theta);
            foilB.setFlaps();
            iCtrl++;
        }
        else
        {
            foilA.applyBase();
            foilB.applyBase();
        }


        QVector<double> ClList, ReList;

        for(int k=0; k<surf.NYPanels(); k++)
        {
            ClList.append(sd.m_Cl.at(m+k));
            ReList.append(sd.m_Re.at(m+k));
        }

        QVector<double> CdAList, XTrTopAList, XTrBotAList;
        QVector<double> CdBList, XTrTopBList, XTrBotBList;
        QVector<bool> CvAList, CvBList;

        XFoilTask *pTask = new XFoilTask(this); // watch the stack
//        pTask->setStreamText(&strange);

        // Process span stations incrementally and in sequence, assuming that convergence
        // is optimized because 2d BL is similar from one station to the next

        pTask->initializeTask(&foilA, &a2dPolar, false, true, true);
        pTask->processClList(ClList, ReList, CdAList, XTrTopAList, XTrBotAList, CvAList);

        int cnt = 0;
        for(int ic=0; ic<CvAList.size(); ic++) cnt += CvAList.at(ic);
        strange = QString::asprintf("                  %d/%d converged left span stations\n", cnt, int(CvAList.size()));
        onTraceLog(strange);

        // fall back: re-try unconverged points with BL initialization
        for(int k=0; k<CvAList.size(); k++)
        {
            if(!CvAList.at(k))
            {
                QString str = QString::asprintf("                    Fallback left side,  station %2d, Cl=%7.3f... ", k+1,  ClList.at(k));
                onTraceLog(str);

                pTask->processCl(ClList.at(k), ReList.at(k), CdAList[k], XTrTopAList[k], XTrBotAList[k], CvAList[k]);

                if(CvAList.at(k)) onTraceLog("converged\n");
                else              onTraceLog("failed\n");

            }

            if(!CvAList.at(k))
            {
                strong = surf.leftFoilName();
                strong +=  QString::asprintf(", Span pos. %.3f ", sd.m_StripPos.at(m+k)*Units::mtoUnit());
                strong += Units::lengthUnitLabel();
                strong += QString::asprintf(",  Cl=%.3f, Re=%.0f", ClList.at(k), ReList.at(k));

                strong += EOLCHAR + "                   ...discarding the operating point" + EOLCHAR;

                logmsg += strong;

                delete pTask;
                return false; // XFoil OTF fail, operating point will be discarded
            }

            if(isCancelled())
            {
                delete pTask;
                return false;
            }
        }

        pTask->initializeTask(&foilB, &a2dPolar, false, true, true);
        pTask->processClList(ClList, ReList, CdBList, XTrTopBList, XTrBotBList, CvBList);

        cnt = 0;
        for(int ic=0; ic<CvBList.size(); ic++) cnt += CvBList.at(ic);
        strange = QString::asprintf("                  %d/%d converged right span stations\n", cnt, int(CvBList.size()));
        onTraceLog(strange);

        // fall back: re-try unconverged points with BL initialization
        for(int k=0; k<CvBList.size(); k++)
        {
            if(!CvBList.at(k))
            {
                QString str = QString::asprintf("                    Fallback right side, station %2d, Cl=%7.3f... ", k+1,  ClList.at(k));
                onTraceLog(str);

                pTask->processCl(ClList.at(k), ReList.at(k), CdBList[k], XTrTopBList[k], XTrBotBList[k], CvBList[k]);

                if(CvAList.at(k)) onTraceLog("converged\n");
                else              onTraceLog("failed\n");
            }

            if(!CvBList.at(k))
            {
                strong = surf.rightFoilName();
                strong += QString::asprintf(", Span pos. %.3f ", sd.m_StripPos.at(m+k)*Units::mtoUnit());
                strong += Units::lengthUnitLabel();
                strong += QString::asprintf(",  Cl=%.3f, Re=%.0f", ClList.at(k), ReList.at(k));

                strong += EOLCHAR + "                   ...discarding the operating point" + EOLCHAR;

                logmsg += strong;

                delete pTask;
                return false; // XFoil OTF fail, operating point will be discarded
            }

            if(isCancelled())
            {
                delete pTask;
                return false;
            }
        }

        delete pTask;

        for(int k=0; k<surf.NYPanels(); k++)
        {
            double tau = 0.0;
            Vector3d PtC4;
            surf.getC4(k, PtC4, tau);
            if (tau<0.0) tau = 0.0; // redundant
            if (tau>1.0) tau = 1.0; // redundant

            sd.m_Alpha_0[m] = Objects2d::getZeroLiftAngle(surf.foilA(), surf.foilB(), sd.m_Re.at(m), tau); // what's the use + wrong if flap

            // interpolate
            Q_ASSERT(CvAList.at(k) && CvBList.at(k));

            sd.m_PCd[m]    = CdAList.at(k)     * (1.0-tau) + CdBList.at(k)     * tau;
            sd.m_XTrTop[m] = XTrTopAList.at(k) * (1.0-tau) + XTrTopBList.at(k) * tau;
            sd.m_XTrBot[m] = XTrBotAList.at(k) * (1.0-tau) + XTrBotBList.at(k) * tau;

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

    if(!s_bCancel)
        Q_ASSERT(iCtrl == TEFlapAngles.nValues());

    return true;
}


void PlaneTask::makeVortonRow(int qrhs)
{
    if(!m_pPolar3d->bVortonWake()) return;

    double const *mu    = nullptr;
    if     (m_pP4A) mu = m_pP4A->m_Mu.constData() + qrhs*m_pP4A->nPanels();
    else if(m_pP3A) mu = m_pP3A->m_Mu.constData() + qrhs*3*m_pP3A->nPanels();
    else return;

    // update positions and vorticities
    // duplicate the existing vortons which will be replaced all at once at the end of the procedure
    QVector<QVector<Vorton>> newvortons = m_pPA->m_Vorton;

    // convert the doublet sheet into the first row of vortons
    // and prepend the row to the array
    QVector<Vorton> vortonrow; // a row includes the vorton rows from all the wings
    QVector<Vortex> wingvortexneg;
    m_pPA->m_VortexNeg.clear(); // update the trailing negating vortices

    // reset the first step
    double dl = m_pPolar3d->vortonL0() * m_pPolar3d->referenceChordLength(); //m

    if(m_pPlane->isXflType())
    {
        QVector<Vorton> wingvortons;
        int pos = 0;
        for(int iw=0; iw<m_pPlane->nWings(); iw++)
        {
            WingXfl const *pWing = m_pPlane->wingAt(iw);
            if(m_pP4A)
            {
                m_pP4A->makeVortons(dl, mu, pos, pWing->nPanel4(), pWing->nStations(), vortonrow.size(), wingvortons, wingvortexneg);
                pos += pWing->nPanel4();
            }
            else if(m_pP3A)
            {
                m_pP3A->makeVortons(dl, mu, pos, pWing->nPanel3(), pWing->nStations(), vortonrow.size(), wingvortons, wingvortexneg);
                pos += pWing->nPanel3();
            }

            vortonrow.append(wingvortons);
            m_pPA->m_VortexNeg.append(wingvortexneg);
        }
    }
    else
    {
        if(m_pP3A)
        {
            m_pP3A->makeVortons(dl, mu, 0, m_pPlane->nPanel3(), m_pPlane->nStations(), 0, vortonrow, wingvortexneg);
            m_pPA->m_VortexNeg.append(wingvortexneg);
        }
    }


    // merge the vortons and update indexes
    int iv=0;
    while(iv<vortonrow.size())
    {
        for(int jv=vortonrow.size()-1; jv>iv; jv--)
        {
            if(vortonrow.at(iv).position().isSame(vortonrow.at(jv).position(), 1.e-4))
            {
                vortonrow[iv].setVortex(vortonrow[iv].vortex()+vortonrow[jv].vortex());
                vortonrow.removeAt(jv);
                for(int i=0; i<m_pPA->m_VortexNeg.size(); i++)
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

    newvortons.prepend(vortonrow);


    // check if the last row is still active
    bool bActiveLastRow = false;
    QVector<Vorton> &lastrow = newvortons.last();
    for(int i=0; i<lastrow.size(); i++)
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
    m_pPA->getVelocityVector(C, m_pPA->m_Mu.constData(), m_pPA->m_Sigma.constData(), velocity, coreradius, bWakeOnly, bMultiThread);
}


bool PlaneTask::setLinearSolution()
{
    QElapsedTimer t; t.start();
    QString strange;

    onTraceLog("   Making the unit RHS vectors...");
    m_pPA->makeUnitRHSVectors();

    strange = QString::asprintf("     done in %.3f s\n", double(t.restart())/1000.0);
    onTraceLog(strange);

    if (isCancelled()) return true;

    onTraceLog("   Making the influence matrix...");
    m_pPA->makeInfluenceMatrix();

    strange = QString::asprintf("     done in %.3f s\n", double(t.restart())/1000.0);
    onTraceLog(strange);

    if(m_pPA->m_bMatrixError)
    {
        m_bError = true;
        return false;
    }
    if (isCancelled()) return true;

    if(!m_pWPolar->isVLM())
    {
        onTraceLog("   Adding the wake's contribution...");
        m_pPA->addWakeContribution();

        strange = QString::asprintf("  done in %.3f s\n", double(t.restart())/1000.0);
        onTraceLog(strange);

        if(m_pPA->m_bMatrixError) return false;
    }
    if (isCancelled()) return true;

    onTraceLog("   LAPACK - LU factorization...");
    if (!m_pPA->LUfactorize())
    {
        onTraceLog(" singular matrix, aborting\n");

        m_bError = true;
        return false;
    }
    strange = QString::asprintf("       done in %.3f s\n", double(t.restart())/1000.0);
    onTraceLog(strange);
    if (isCancelled()) return true;

    onTraceLog("   Back-substituting RHS...");
    m_pPA->backSubUnitRHS(m_pPA->m_uRHS.data(), m_pPA->m_vRHS.data(), m_pPA->m_wRHS.data(), m_pPA->m_pRHS.data(), m_pPA->m_qRHS.data(), m_pPA->m_rRHS.data());

    strange = QString::asprintf("           done in %.3f s\n", double(t.restart())/1000.0);
    onTraceLog(strange);

//    listArrays(m_pPA->m_uRHS, m_pPA->m_wRHS);

    return true;
}



