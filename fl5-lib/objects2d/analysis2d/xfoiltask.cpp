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


#include <QString>



#include <xfoiltask.h>
#include <foil.h>
#include <oppoint.h>
#include <polar.h>
#include <geom_params.h>
#include <constants.h>



bool XFoilTask::s_bCancel   = false;
double XFoilTask::s_CdError = 1.0e-3;

int XFoilTask::s_IterLim=100;


XFoilTask::XFoilTask()
{
    m_pFoil    = nullptr;
    m_pPolar   = nullptr;

    m_bViscous = true; // always true
    m_bAlpha   = true;

    m_bErrors = false;
}


void XFoilTask::setAlphaRange(double vMin, double vMax, double vDelta)
{
    m_bAlpha = true;
    m_AnalysisRange.resize(1);
    AnalysisRange &range = m_AnalysisRange.front();
    range.setActive(true);
    range.m_vMin = vMin;
    range.m_vMax = vMax;
    range.m_vInc = vDelta;
}


void XFoilTask::setClRange(double vMin, double vMax, double vDelta)
{
    m_bAlpha = false;
    m_AnalysisRange.resize(1);
    AnalysisRange &range = m_AnalysisRange.front();
    range.setActive(true);
    range.m_vMin = vMin;
    range.m_vMax = vMax;
    range.m_vInc = vDelta;
}


void XFoilTask::traceLog(QString const &str)
{
    traceStdLog(str.toStdString());
}


void XFoilTask::traceStdLog(std::string const &str)
{
    // Access the Q under the lock:
    std::unique_lock<std::mutex> lck(m_mtx);
    m_theMsgQueue.push(str);
    m_cv.notify_all();

    m_Log.append(str);
}


void XFoilTask::run()
{
    if(s_bCancel || !m_pPolar || !m_pFoil)
    {
        m_AnalysisStatus = xfl::FINISHED;
    }
    else
    {
        m_AnalysisStatus = xfl::RUNNING;

        if(m_pPolar->isFixedaoaPolar())     ReSequence();
        else if(m_pPolar->isControlPolar()) thetaSequence();
        else                                alphaSequence(m_bAlpha);

        m_AnalysisStatus = xfl::FINISHED;

        traceLog("\nDone processing ranges.\n"); // final notification after flag is set to FINISHED so that sender thread may exit

        // leave things as they were
        m_pFoil->setTEFlapAngle(0.0);
        m_pFoil->setFlaps();
    }
}


bool XFoilTask::initialize(FoilAnalysis *pFoilAnalysis, bool bKeepOpps)
{
    return initialize(pFoilAnalysis->pFoil, pFoilAnalysis->pPolar, bKeepOpps);
}


bool XFoilTask::initialize(Foil *pFoil, Polar *pPolar, bool bKeepOpps)
{
    s_bCancel = false;

    m_bKeepOpps = bKeepOpps;

    m_bErrors = false;
    m_pFoil = pFoil;
    m_pPolar = pPolar;

    m_AnalysisStatus = xfl::PENDING;

    XFoil::s_bCancel = false;

    std::vector<double> x(m_pFoil->nNodes()), y(m_pFoil->nNodes()), nx(m_pFoil->nNodes()), ny(m_pFoil->nNodes());
    for(int i=0; i<m_pFoil->nNodes(); i++)
    {
        x[i] = m_pFoil->x(i);
        y[i] = m_pFoil->y(i);
        nx[i] = m_pFoil->normal(i).x;
        ny[i] = m_pFoil->normal(i).y;
    }

    if(!m_XFoilInstance.initXFoilGeometry(m_pFoil->nNodes(), x.data(), y.data(), nx.data(), ny.data()))
        return false;

    bool bViscous = true;
    if(!m_XFoilInstance.initXFoilAnalysis(m_pPolar->Reynolds(), m_pPolar->aoaSpec(), m_pPolar->Mach(),
                                          m_pPolar->NCrit(), m_pPolar->XTripTop(), m_pPolar->XTripBot(),
                                          m_pPolar->ReType(), m_pPolar->MaType(), bViscous))
        return false;
    return true;
}


/** Fallback for 3d OTF calculations at unconverged span spations */
bool XFoilTask::processCl(double Cl, double Re, double &Cd, double &XTrTop, double &XTrBot, bool &bCv)
{
    QString str;

    traceLog("   Initializing BL\n");
    m_XFoilInstance.lblini = false;
    m_XFoilInstance.lipan = false;


    m_XFoilInstance.lalfa = false;
    m_XFoilInstance.alfa = 0.0;
    m_XFoilInstance.qinf = 1.0;
    m_XFoilInstance.clspec = Cl;

    m_XFoilInstance.reinf1 = Re;
    m_XFoilInstance.minf1  = 0.0;


    str = QString::asprintf("   Re=%g  Cl=%g ", Re, Cl);
    traceLog(str);
    if(!m_XFoilInstance.speccl())
    {
        str = "Invalid Analysis Settings\nCpCalc: local speed too large\n Compressibility corrections invalid";
        traceLog(str);
        m_bErrors = true;
        return false;
    }

    m_XFoilInstance.lwake = false;
    m_XFoilInstance.lvconv = false;

    int iterations = loop();

    if(m_XFoilInstance.lvconv)
    {
        str = QString::asprintf("   ...converged after %3d iterations / Cl=%5f  Cd=%5f\n", iterations, m_XFoilInstance.cl, m_XFoilInstance.cd);
        traceLog(str);
        bCv    = m_XFoilInstance.lvconv;
        Cd     = m_XFoilInstance.cd;
        XTrTop = m_XFoilInstance.xoctr[1];
        XTrBot = m_XFoilInstance.xoctr[2];
    }
    else
    {
        str = QString::asprintf("   ...unconverged after %d iterations\n", iterations);
        traceLog(str);     

        // fallback: interpolate

        m_XFoilInstance.lblini = false;
        m_XFoilInstance.lipan  = false;

        m_AnalysisRange.clear();

        // define a wide range
        double ClMax(0);
        if(Cl>0) ClMax = Cl + 0.5;
        else     ClMax = Cl -0.5;
        m_AnalysisRange.push_back({true,0.0, ClMax, 0.05});

        // launch the sequence
        alphaSequence(false);

        // interpolate
        bool bOutCl = false;
        Cd     = m_pPolar->interpolateFromCl(Cl, Polar::CD,     bOutCl);
        XTrTop = m_pPolar->interpolateFromCl(Cl, Polar::XTRTOP, bOutCl);
        XTrBot = m_pPolar->interpolateFromCl(Cl, Polar::XTRBOT, bOutCl);
        bCv = !bOutCl;
    }

    return bCv;
}


/** Tailored for 3d OTF calculations */
bool XFoilTask::processClList(std::vector<double> const& ClList, std::vector<double> const& ReList,
                              std::vector<double> &CdList, std::vector<double> &XTrTopList, std::vector<double> &XTrBotList,
                              std::vector<bool> &CvList)
{
    QString str;

    CdList.resize(ClList.size());
    XTrTopList.resize(ClList.size());
    XTrBotList.resize(ClList.size());
    CvList.resize(ClList.size());

    std::fill(CdList.begin(),     CdList.end(),     0);
    std::fill(XTrTopList.begin(), XTrTopList.end(), 0);
    std::fill(XTrBotList.begin(), XTrBotList.end(), 0);
    std::fill(CvList.begin(),     CvList.end(),     true);


//    if(s_bInitBL)
    {
        m_XFoilInstance.lblini = false;
        m_XFoilInstance.lipan = false;
        traceLog("   Initializing BL\n");
    }

    for(uint icl=0; icl<ClList.size(); icl++)
    {
        if(s_bCancel) break;

        double Cl = ClList.at(icl);

        m_XFoilInstance.lalfa = false;
        m_XFoilInstance.alfa = 0.0;
        m_XFoilInstance.qinf = 1.0;
        m_XFoilInstance.clspec = Cl;

        m_XFoilInstance.reinf1 = ReList.at(icl);
        m_XFoilInstance.minf1  = 0.0;

        str = QString::asprintf("   Re=%g  Cl=%g ", ReList.at(icl), Cl);
        traceLog(str);
        if(!m_XFoilInstance.speccl())
        {
            str = "Invalid Analysis Settings\nCpCalc: local speed too large\n Compressibility corrections invalid";
            traceLog(str);
            m_bErrors = true;
            return false;
        }

        m_XFoilInstance.lwake = false;
        m_XFoilInstance.lvconv = false;

        int iterations = loop();

        CvList[icl] = m_XFoilInstance.lvconv;
        if(m_XFoilInstance.lvconv)
        {
            str = QString::asprintf("   ...converged after %d iterations / Cl=%5f  Cd=%5f\n", iterations, m_XFoilInstance.cl, m_XFoilInstance.cd);
            traceLog(str);
            CdList[icl]     = m_XFoilInstance.cd;
            XTrTopList[icl] = m_XFoilInstance.xoctr[1];
            XTrBotList[icl] = m_XFoilInstance.xoctr[2];
        }
        else
        {
            str = QString::asprintf("   ...unconverged after %3d iterations\n", iterations);
            traceLog(str);
            m_bErrors = true;
            m_XFoilInstance.lblini = false;
            m_XFoilInstance.lipan  = false;
        }
    }

    return true;
}


void XFoilTask::initializeBL()
{
    traceLog("   Initializing B.L.\n");
    m_XFoilInstance.lblini = false;
    m_XFoilInstance.lipan = false;
}


/** aoa or Cl ranges */
bool XFoilTask::alphaSequence(bool bAlpha)
{
    QString str;

    double SpMin(0), SpMax(0), SpInc(0);

    for (uint iSeries=0; iSeries<m_AnalysisRange.size(); iSeries++)
    {
        if(s_bCancel) break;
        AnalysisRange const &range = m_AnalysisRange.at(iSeries);
        if(range.isActive())
        {
            traceLog(QString::asprintf("\nProcessing active range [%7.3f  %7.3f  %7.3f]\n", range.m_vMin, range.m_vMax, range.m_vInc));
        }
        else
        {
            traceLog(QString::asprintf("\nSkipping inactive range [%7.3f  %7.3f  %7.3f]\n", range.m_vMin, range.m_vMax, range.m_vInc));
            continue;
        }

        initializeBL();

        int iter=0;
        SpMin = range.m_vMin;
        SpMax = range.m_vMax;
        SpInc = fabs(range.m_vInc);
        if(SpMax<SpMin) SpInc = -SpInc;

        double alphadeg = SpMin;
        double Cl       = SpMin; // could use alphadeg instead

        do
        {
            if(s_bCancel) break;

            if(bAlpha)
            {
                m_XFoilInstance.alfa = alphadeg * PI/180.0;
                m_XFoilInstance.lalfa = true;
                m_XFoilInstance.qinf = 1.0;
                str = "   " + ALPHAch;
                str.append(QString::asprintf(" = %7.3f°", alphadeg));
                traceLog(str);


                // here we go!
                if (!m_XFoilInstance.specal())
                {
                    str = "Invalid Analysis Settings\nCpCalc: local speed too large\n Compressibility corrections invalid";
                    traceLog(str);
                    m_bErrors = true;
                    return false;
                }
            }
            else
            {
                m_XFoilInstance.lalfa = false;
                m_XFoilInstance.alfa = 0.0;
                m_XFoilInstance.qinf = 1.0;
                m_XFoilInstance.clspec = Cl;
                str = QString::asprintf("   Cl = %7.3f", Cl);
                traceLog(str);
                if(!m_XFoilInstance.speccl())
                {
                    str = "Invalid Analysis Settings\nCpCalc: local speed too large\n Compressibility corrections invalid";
                    traceLog(str);
                    m_bErrors = true;
                    return false;
                }
            }

            m_XFoilInstance.lwake = false;
            m_XFoilInstance.lvconv = false;

            int iterations = loop();

            if(m_XFoilInstance.lvconv)
            {
                str = QString::asprintf("   ...converged after %3d iterations / Cl=%9.5f  Cd=%9.5f\n", iterations, m_XFoilInstance.cl, m_XFoilInstance.cd);
                traceLog(str);

                if(m_XFoilInstance.cd<s_CdError)
                {
                    str = QString::asprintf("      ...discarding operating point with spurious Cd=%g\n", m_XFoilInstance.cd);
                    traceLog(str);
                }
                else
                {
                    OpPoint *pOpPoint = new OpPoint;
                    pOpPoint->setFoilName(m_pFoil->name());
                    pOpPoint->setPolarName(m_pPolar->name());
                    pOpPoint->setTheStyle(m_pPolar->theStyle());
                    addXFoilData(pOpPoint, m_XFoilInstance, m_pFoil);
                    m_pPolar->addOpPointData(pOpPoint); // store the data on the fly; a polar is only used by one task at a time
                    pOpPoint->setTheta(m_pPolar->TEFlapAngle());

                    if(m_bKeepOpps) m_OpPoints.push_back(pOpPoint);
                    else delete pOpPoint;
                }
            }
            else
            {
                str = QString::asprintf("   ...unconverged after %3d iterations\n", iterations);
                traceLog(str);
                traceLog("      ...initializing BL\n");
                m_XFoilInstance.lblini = false;
                m_XFoilInstance.lipan = false;

                m_bErrors = true;
            }

            if(XFoil::s_bFullReport)
            {
                traceStdLog(m_XFoilInstance.report());
            }

            alphadeg += SpInc;
            Cl       += SpInc;

            if(SpMin<=SpMax)
            {
                if(bAlpha)
                {
                    if(alphadeg>SpMax+ANGLEPRECISION) break;
                }
                else
                    if(Cl>SpMax+ANGLEPRECISION) break;
            }
            else
            {
                if(bAlpha)
                {
                    if(alphadeg<SpMax-ANGLEPRECISION) break;
                }
                else
                    if(Cl<SpMax-ANGLEPRECISION) break;
            }

            if(fabs(SpInc)<ANGLEPRECISION) break;
        } while(iter++<1000); // failsafe limit
    }

    return true;
}


bool XFoilTask::thetaSequence()
{
    QString str;

    if(!m_pFoil->hasTEFlap())
    {
        str = "The foil "+QString::fromStdString(m_pFoil->name()) + " has no T.E. flap\n"
              "     ...Skipping the T6 polar " + QString::fromStdString(m_pPolar->name()) + EOLch;
        traceLog(str);

        m_bErrors = true;
        return false;
    }


    double SpMin(0), SpMax(0), SpInc(0);
    double alphadeg = m_pPolar->aoaSpec();

    str = QString::asprintf("   alpha = %7.3f°", alphadeg);
    traceLog(str);

    for (uint iSeries=0; iSeries<m_AnalysisRange.size(); iSeries++)
    {
        if(s_bCancel) break;
        AnalysisRange const &range = m_AnalysisRange.at(iSeries);
        if(range.isActive())
        {
            traceLog(QString::asprintf("\nProcessing active range [%7.3f  %7.3f  %7.3f]\n", range.m_vMin, range.m_vMax, range.m_vInc));
        }
        else
        {
            traceLog(QString::asprintf("\nSkipping inactive range [%7.3f  %7.3f  %7.3f]\n", range.m_vMin, range.m_vMax, range.m_vInc));
            continue;
        }

        initializeBL();

        SpMin = range.m_vMin;
        SpMax = range.m_vMax;
        SpInc = fabs(range.m_vInc);
        if(SpMax<SpMin) SpInc = -SpInc;

        double theta = SpMin;
        int nTheta = 0;
        if(fabs(SpInc)>FLAPANGLEPRECISION) nTheta = fabs((SpMax-SpMin)/SpInc) + 1;
        nTheta = std::max(1, nTheta);
        nTheta = std::min(100, nTheta); // failsafe limit

        for(int iter=0; iter<nTheta; iter++)
        {
            if(s_bCancel) break;

            m_XFoilInstance.alfa = alphadeg * PI/180.0;
            m_XFoilInstance.lalfa = true;
            m_XFoilInstance.qinf = 1.0;

            theta = SpMin + iter * SpInc;
            str = QString::asprintf("   theta = %7.3f°", theta);
            traceLog(str);

            m_pFoil->setTEFlapAngle(theta);
            m_pFoil->setFlaps();
            int npts = m_pFoil->nNodes();

            std::vector<double> x(npts), y(npts), nx(npts), ny(npts);
            for(int i=0; i<npts; i++)
            {
                x[i] = m_pFoil->x(i);
                y[i] = m_pFoil->y(i);
                nx[i] = m_pFoil->normal(i).x;
                ny[i] = m_pFoil->normal(i).y;
            }

            if(!m_XFoilInstance.initXFoilGeometry(npts, x.data(), y.data(), nx.data(), ny.data()))
                break;

            m_XFoilInstance.lvisc=true;

            // here we go!
            if (!m_XFoilInstance.specal())
            {
                str = "Invalid Analysis Settings\nCpCalc: local speed too large\n Compressibility corrections invalid";
                traceLog(str);
                m_bErrors = true;
                return false;
            }


            m_XFoilInstance.lwake = false;
            m_XFoilInstance.lvconv = false;

            int iterations = loop();

            if(m_XFoilInstance.lvconv)
            {
                str = QString::asprintf("   ...converged after %3d iterations / Cl=%9.5f  Cd=%9.5f\n", iterations, m_XFoilInstance.cl, m_XFoilInstance.cd);
                traceLog(str);

                if(m_XFoilInstance.cd<s_CdError)
                {
                    str = QString::asprintf("      ...discarding operating point with spurious Cd=%g\n", m_XFoilInstance.cd);
                    traceLog(str);
                }
                else
                {
                    OpPoint *pOpPoint = new OpPoint;
                    pOpPoint->setFoilName(m_pFoil->name());
                    pOpPoint->setPolarName(m_pPolar->name());
                    pOpPoint->setTheStyle(m_pPolar->theStyle());
                    pOpPoint->setTheta(theta);
                    addXFoilData(pOpPoint, m_XFoilInstance, m_pFoil);
                    m_pPolar->addOpPointData(pOpPoint); // store the data on the fly; a polar is only used by one task at a time

                    if(m_bKeepOpps) m_OpPoints.push_back(pOpPoint);
                    else delete pOpPoint;
                }
            }
            else
            {
                str = QString::asprintf("   ...unconverged after %3d iterations\n", iterations);
                traceLog(str);
                traceLog("      ...initializing BL\n");
                m_XFoilInstance.lblini = false;
                m_XFoilInstance.lipan = false;

                m_bErrors = true;
            }

            if(XFoil::s_bFullReport)
            {
                traceStdLog(m_XFoilInstance.report());
            }

            if(fabs(SpInc)<FLAPANGLEPRECISION)
            {
                str.append("theta increment is null - aborting\n");
                traceLog(str);
                break;
            }
        }
    }

    return true;
}


bool XFoilTask::ReSequence()
{
    QString str, strange;

    for (uint iSeries=0; iSeries<m_AnalysisRange.size(); iSeries++)
    {
        if(s_bCancel) break;
        AnalysisRange const &range = m_AnalysisRange.at(iSeries);
        if(range.isActive())
        {
            traceLog(QString::asprintf("\nProcessing active range [%g  %g  %g]\n", range.m_vMin, range.m_vMax, range.m_vInc));
        }
        else
        {
            traceLog(QString::asprintf("\nSkipping inactive range [%g  %g  %g]\n", range.m_vMin, range.m_vMax, range.m_vInc));
            continue;
        }

        initializeBL();

        int iter=0;
        double SpMin = range.m_vMin;
        double SpMax = range.m_vMax;
        double SpInc = fabs(range.m_vInc);
        if(SpMax<SpMin) SpInc = -SpInc;

        double Re = SpMin;
        do
        {
            strange =QString::asprintf("Re = %7.0f", Re);
            traceLog(strange);
            m_XFoilInstance.reinf1 = Re;
            m_XFoilInstance.lalfa = true;
            m_XFoilInstance.qinf = 1.0;

            // here we go !
            if (!m_XFoilInstance.specal())
            {
                str = "Invalid Analysis Settings\nCpCalc: local speed too large\n Compressibility corrections invalid ";
                traceLog(str);
                m_bErrors = true;
                return false;
            }

            m_XFoilInstance.lwake = false;
            m_XFoilInstance.lvconv = false;

            int iterations = loop();

            if(m_XFoilInstance.lvconv)
            {
                str = QString::asprintf("   ...converged after %3d iterations\n", iterations);
                traceLog(str);
            }
            else
            {
                str = QString::asprintf("   ...unconverged after %3d iterations\n", iterations);
                traceLog(str);
                traceLog("      ...initializing BL\n");
                m_XFoilInstance.lblini = false;
                m_XFoilInstance.lipan = false;

                m_bErrors = true;
            }


            OpPoint *pOpPoint = new OpPoint;
            pOpPoint->setFoilName(m_pFoil->name());
            pOpPoint->setPolarName(m_pPolar->name());
            pOpPoint->setTheStyle(m_pPolar->theStyle());
            addXFoilData(pOpPoint, m_XFoilInstance, m_pFoil);
            pOpPoint->setTheta(m_pPolar->TEFlapAngle());
            m_pPolar->addOpPointData(pOpPoint);

            if(m_bKeepOpps) m_OpPoints.push_back(pOpPoint);
            else delete pOpPoint;


            if(XFoil::s_bFullReport)
            {
                traceStdLog(m_XFoilInstance.report());
            }

            Re += SpInc;

            if(SpMin<=SpMax)
            {
                if(Re>SpMax+0.1) break;
            }
            else
            {
                if(Re<SpMax-0.1) break;
            }
        }
        while(iter++<1000); // failsafe limit
    }
    return true;
}


int XFoilTask::loop()
{
    int iterations = -1;
    if(!m_XFoilInstance.viscal())
    {
        m_XFoilInstance.lvconv = false;
//        QString str ="CpCalc: local speed too large\n Compressibility corrections invalid";
        return -1;
    }

    while(iterations<s_IterLim && !m_XFoilInstance.lvconv && !s_bCancel)
    {
        if(m_XFoilInstance.ViscousIter())
        {
            iterations++;
        }
        else iterations = s_IterLim;
    }

    if(s_bCancel)  return -1;// to exit loop

    if(!m_XFoilInstance.ViscalEnd())
    {
        m_XFoilInstance.lvconv = false;//point is unconverged

        m_XFoilInstance.lblini = false;
        m_XFoilInstance.lipan  = false;
        m_bErrors = true;
        return iterations;
    }

    if(iterations>=s_IterLim && !m_XFoilInstance.lvconv)
    {
        m_XFoilInstance.fcpmin();// Is it of any use?
        return iterations;
    }

    if(!m_XFoilInstance.lvconv)
    {
        m_bErrors = true;
        m_XFoilInstance.fcpmin();// Is it of any use?
        return -1;
    }
    else
    {
        //converged at last
        m_XFoilInstance.fcpmin();// Is it of any use?
        return iterations;
    }
    return false;
}


void XFoilTask::addXFoilData(OpPoint *pOpp, XFoil &xfoil, Foil const *pFoil)
{
    int i(0), j(0), ibl(0), is(0);

    pOpp->m_Alpha      = xfoil.alfa*180.0/PI;
    pOpp->m_Cd         = xfoil.cd;
    pOpp->m_Cdp        = xfoil.cdp;
    pOpp->m_Cl         = xfoil.cl;
    pOpp->m_XCP        = xfoil.xcp;
    pOpp->m_Cm         = xfoil.cm;
    pOpp->m_Reynolds   = xfoil.reinf;
    pOpp->m_Mach       = xfoil.minf;
    pOpp->m_NCrit      = xfoil.acrit;

    pOpp->m_bTEFlap    = pFoil->hasTEFlap();
    pOpp->m_bLEFlap    = pFoil->hasLEFlap();

    pOpp->m_Cpmn   = xfoil.cpmn;

    pOpp->resizeSurfacePoints(xfoil.n);
    for (int k=0; k<xfoil.n; k++)
    {
        pOpp->m_Cpi[k] = xfoil.cpi[k+1];
        pOpp->m_Qi[k]  = xfoil.qgamm[k+1];
    }

    if(xfoil.lvisc && xfoil.lvconv)
    {
        pOpp->m_XTrTop =xfoil.xoctr[1];
        pOpp->m_XTrBot =xfoil.xoctr[2];
        pOpp->m_bViscResults = true;
        pOpp->m_bBL = true;

        for (int k=0; k<xfoil.n; k++)
        {
            pOpp->m_Cpv[k] = xfoil.cpv[k+1];
            pOpp->m_Qv[k]  = xfoil.qvis[k+1];
        }
    }
    else
    {
        pOpp->m_bViscResults = false;
    }

    if(pOpp->m_bTEFlap || pOpp->m_bLEFlap)
    {
        pOpp->setHingeMoments(pFoil);
    }

    if(!xfoil.lvisc || !xfoil.lvconv)    return;

//---- add boundary layer on both sides of airfoil
    pOpp->m_BLXFoil.nd1=0;
    pOpp->m_BLXFoil.nd2=0;
    pOpp->m_BLXFoil.nd3=0;
    for (is=1; is<=2; is++)
    {
        for (ibl=2; ibl<=xfoil.iblte[is];ibl++)
        {
            i = xfoil.ipan[ibl][is];
            pOpp->m_BLXFoil.xd1[i] = xfoil.x[i] + xfoil.nx[i]*xfoil.dstr[ibl][is];
            pOpp->m_BLXFoil.yd1[i] = xfoil.y[i] + xfoil.ny[i]*xfoil.dstr[ibl][is];
            pOpp->m_BLXFoil.nd1++;
        }
    }

//---- set upper and lower wake dstar fractions based on first wake point
    is=2;
    double dstrte = xfoil.dstr[xfoil.iblte[is]+1][is];
    double dsf1, dsf2;
    if(dstrte!=0.0)
    {
        dsf1 = (xfoil.dstr[xfoil.iblte[1]][1] + 0.5*xfoil.ante) / dstrte;
        dsf2 = (xfoil.dstr[xfoil.iblte[2]][2] + 0.5*xfoil.ante) / dstrte;
    }
    else
    {
        dsf1 = 0.5;
        dsf2 = 0.5;
    }

//---- plot upper wake displacement surface
    ibl = xfoil.iblte[1];
    i = xfoil.ipan[ibl][1];
    pOpp->m_BLXFoil.xd2[0] = xfoil.x[i] + xfoil.nx[i]*xfoil.dstr[ibl][1];
    pOpp->m_BLXFoil.yd2[0] = xfoil.y[i] + xfoil.ny[i]*xfoil.dstr[ibl][1];
    pOpp->m_BLXFoil.nd2++;

    j= xfoil.ipan[xfoil.iblte[is]+1][is]  -1;
    for (ibl=xfoil.iblte[is]+1; ibl<=xfoil.nbl[is]; ibl++)
    {
        i = xfoil.ipan[ibl][is];
        pOpp->m_BLXFoil.xd2[i-j] = xfoil.x[i] - xfoil.nx[i]*xfoil.dstr[ibl][is]*dsf1;
        pOpp->m_BLXFoil.yd2[i-j] = xfoil.y[i] - xfoil.ny[i]*xfoil.dstr[ibl][is]*dsf1;
        pOpp->m_BLXFoil.nd2++;
    }

//---- plot lower wake displacement surface
    ibl = xfoil.iblte[2];
    i = xfoil.ipan[ibl][2];
    pOpp->m_BLXFoil.xd3[0] = xfoil.x[i] + xfoil.nx[i]*xfoil.dstr[ibl][2];
    pOpp->m_BLXFoil.yd3[0] = xfoil.y[i] + xfoil.ny[i]*xfoil.dstr[ibl][2];
    pOpp->m_BLXFoil.nd3++;

    j = xfoil.ipan[xfoil.iblte[is]+1][is]  -1;
    for (ibl=xfoil.iblte[is]+1; ibl<=xfoil.nbl[is]; ibl++)
    {
        i = xfoil.ipan[ibl][is];
        pOpp->m_BLXFoil.xd3[i-j] = xfoil.x[i] + xfoil.nx[i]*xfoil.dstr[ibl][is]*dsf2;
        pOpp->m_BLXFoil.yd3[i-j] = xfoil.y[i] + xfoil.ny[i]*xfoil.dstr[ibl][is]*dsf2;
        pOpp->m_BLXFoil.nd3++;
    }
    pOpp->m_BLXFoil.tklam = xfoil.tklam;
    pOpp->m_BLXFoil.qinf = xfoil.qinf;

    memcpy(pOpp->m_BLXFoil.thet, xfoil.thet, IVX * ISX * sizeof(double));
    memcpy(pOpp->m_BLXFoil.tau,  xfoil.tau,  IVX * ISX * sizeof(double));
    memcpy(pOpp->m_BLXFoil.ctau, xfoil.ctau, IVX * ISX * sizeof(double));
    memcpy(pOpp->m_BLXFoil.ctq,  xfoil.ctq,  IVX * ISX * sizeof(double));
    memcpy(pOpp->m_BLXFoil.dis,  xfoil.dis,  IVX * ISX * sizeof(double));
    memcpy(pOpp->m_BLXFoil.uedg, xfoil.uedg, IVX * ISX * sizeof(double));
    memcpy(pOpp->m_BLXFoil.dstr, xfoil.dstr, IVX * ISX * sizeof(double));
    memcpy(pOpp->m_BLXFoil.delt, xfoil.delt, IVX * ISX * sizeof(double));
    memcpy(pOpp->m_BLXFoil.itran, xfoil.itran, 3 * sizeof(int));

    xfoil.createXBL();
    xfoil.fillHk();
    xfoil.fillRTheta();
    memcpy(pOpp->m_BLXFoil.xbl, xfoil.xbl, IVX * ISX * sizeof(double));
    memcpy(pOpp->m_BLXFoil.Hk, xfoil.Hk, IVX * ISX * sizeof(double));
    memcpy(pOpp->m_BLXFoil.RTheta, xfoil.RTheta, IVX * ISX * sizeof(double));
    pOpp->m_BLXFoil.nside1 = xfoil.m_nSide1;
    pOpp->m_BLXFoil.nside2 = xfoil.m_nSide2;
}



