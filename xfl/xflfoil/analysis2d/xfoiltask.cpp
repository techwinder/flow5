/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QThread>
#include <QCoreApplication>


#include <xflfoil/analysis2d/xfoiltask.h>
#include <xflcore/xflcore.h>
#include <xflcore/flow5events.h>
#include <xflmath/constants.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/polar.h>

#include <xflfoil/objects2d/objects2d.h>
#include <xflfoil/objects2d/polar.h>
#include <xflfoil/objects2d/oppoint.h>




bool XFoilTask::s_bCancel    = false;
bool XFoilTask::s_bSkipOpp   = false;
bool XFoilTask::s_bSkipPolar = false;
bool XFoilTask::s_bStoreOpp  = true;
bool XFoilTask::s_bViscous = true;
bool XFoilTask::s_bAlpha = true;
bool XFoilTask::s_bInitBL = true;
double XFoilTask::s_CdError = 1.0e-3;

int XFoilTask::s_IterLim=100;
bool XFoilTask::s_bAutoInitBL = true;



XFoilTask::XFoilTask(QObject *pParent)
{
    setAutoDelete(true);
    m_pParent  = pParent;
    m_pFoil    = nullptr;
    m_pPolar   = nullptr;

    m_bErrors = false;

    m_OutMessage.clear();
    m_OutStream.setDevice(nullptr);
}


void XFoilTask::setAlphaRange(double vMin, double vMax, double vDelta)
{
    s_bAlpha = true;
    m_AnalysisRange.resize(1);
    AnalysisRange &range = m_AnalysisRange.front();
    range.setActive(true);
    range.m_vMin = vMin;
    range.m_vMax = vMax;
    range.m_vInc = vDelta;
}


void XFoilTask::setClRange(double vMin, double vMax, double vDelta)
{
    s_bAlpha = false;
    m_AnalysisRange.resize(1);
    AnalysisRange &range = m_AnalysisRange.front();
    range.setActive(true);
    range.m_vMin = vMin;
    range.m_vMax = vMax;
    range.m_vInc = vDelta;
}


void XFoilTask::traceLog(QString const &str)
{
    emit outputMessage(str);


    if(m_OutStream.device() || m_OutStream.string())
    {
        m_OutStream << str;
    }
    m_OutMessage += str;
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
        else                                alphaSequence(s_bAlpha);

        m_AnalysisStatus = xfl::FINISHED;

        // leave things as they were
        m_pFoil->setTEFlapAngle(0.0);
        m_pFoil->setFlaps();
    }

    // post an event to notify the parent window that the task is done
    if(m_pParent)
        qApp->postEvent(m_pParent, new XFoilTaskEvent(this, m_pFoil, m_pPolar));
}


void XFoilTask::initializeBL()
{
    s_bInitBL = true;
}


bool XFoilTask::initializeTask(FoilAnalysis *pFoilAnalysis, bool bStoreOpp, bool bViscous, bool bInitBL)
{
    return initializeTask(pFoilAnalysis->pFoil, pFoilAnalysis->pPolar, bStoreOpp, bViscous, bInitBL);
}


bool XFoilTask::initializeTask(Foil *pFoil, Polar *pPolar, bool bStoreOpp, bool bViscous, bool bInitBL)
{
    s_bCancel = false;
    s_bSkipOpp = s_bSkipPolar = false;
    s_bStoreOpp = bStoreOpp;

    m_bErrors = false;
    m_pFoil = pFoil;
    m_pPolar = pPolar;

    s_bInitBL = bInitBL;

    m_AnalysisStatus = xfl::PENDING;

    XFoil::s_bCancel = false;

    QVector<double> x(m_pFoil->nNodes()), y(m_pFoil->nNodes()), nx(m_pFoil->nNodes()), ny(m_pFoil->nNodes());
    for(int i=0; i<m_pFoil->nNodes(); i++)
    {
        x[i] = m_pFoil->x(i);
        y[i] = m_pFoil->y(i);
        nx[i] = m_pFoil->normal(i).x;
        ny[i] = m_pFoil->normal(i).y;
    }

    m_XFoilStream.setString(&m_XFoilLog);
    if(!m_XFoilInstance.initXFoilGeometry(m_pFoil->nNodes(), x.data(), y.data(), nx.data(), ny.data()))
        return false;
    if(!m_XFoilInstance.initXFoilAnalysis(m_pPolar->Reynolds(), m_pPolar->aoa(), m_pPolar->Mach(),
                                          m_pPolar->NCrit(), m_pPolar->XTripTop(), m_pPolar->XTripBot(),
                                          m_pPolar->ReType(), m_pPolar->MaType(),
                                          bViscous, m_XFoilStream)) return false;
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
        str = QString::asprintf("   ...unconverged after %3d iterations\n", iterations);
        traceLog(str);

        // fallback: interpolate

        m_XFoilInstance.lblini = false;
        m_XFoilInstance.lipan  = false;

        m_AnalysisRange.clear();

        // define a wide range
        double ClMax(0);
        if(Cl>0) ClMax = Cl + 0.5;
        else     ClMax = Cl -0.5;
        m_AnalysisRange.append({true,0.0, ClMax, 0.05});

        // launch the sequence
        alphaSequence(false);

        // interpolate
        bool bOutCl = false;
        Cd     = m_pPolar->interpolateFromCl(Cl, Polar::CD,     bOutCl);
        XTrTop = m_pPolar->interpolateFromCl(Cl, Polar::XTRTOP, bOutCl);
        XTrBot = m_pPolar->interpolateFromCl(Cl, Polar::XTRBOT, bOutCl);
        bCv = !bOutCl;
    }

    return true;
}


/** Tailored for 3d OTF calculations */
bool XFoilTask::processClList(QVector<double> const& ClList, QVector<double> const& ReList,
                              QVector<double> &CdList, QVector<double> &XTrTopList, QVector<double> &XTrBotList,
                              QVector<bool> &CvList)
{
    QString str;

    CdList.resize(ClList.size());
    XTrTopList.resize(ClList.size());
    XTrBotList.resize(ClList.size());
    CvList.resize(ClList.size());

    CdList.fill(0);
    XTrTopList.fill(0);
    XTrBotList.fill(0);
    CvList.fill(true);


//    if(s_bInitBL)
    {
        m_XFoilInstance.lblini = false;
        m_XFoilInstance.lipan = false;
        traceLog("   Initializing BL\n");
    }

    for(int icl=0; icl<ClList.size(); icl++)
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
            str = QString::asprintf("   ...converged after %3d iterations / Cl=%5f  Cd=%5f\n", iterations, m_XFoilInstance.cl, m_XFoilInstance.cd);
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

/** aoa or Cl ranges */
bool XFoilTask::alphaSequence(bool bAlpha)
{
    QString str;

    double SpMin(0), SpMax(0), SpInc(0);

    for (int iSeries=0; iSeries<m_AnalysisRange.size(); iSeries++)
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

        if(s_bInitBL)
        {
            m_XFoilInstance.lblini = false;
            m_XFoilInstance.lipan = false;
            traceLog("   Initializing BL\n");
        }

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
            if(s_bSkipPolar)
            {
                m_XFoilInstance.lblini = false;
                m_XFoilInstance.lipan = false;
                s_bSkipPolar = false;
                traceLog("    .......skipping polar \n");
                return false;
            }

            if(bAlpha)
            {
                m_XFoilInstance.alfa = alphadeg * PI/180.0;
                m_XFoilInstance.lalfa = true;
                m_XFoilInstance.qinf = 1.0;
                str = QString("   ")+ ALPHACHAR + QString::asprintf(" = %7.3f", alphadeg) + DEGCHAR;
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
                str = QString::asprintf("   ...converged after %3d iterations / Cl=%5f  Cd=%5f\n", iterations, m_XFoilInstance.cl, m_XFoilInstance.cd);
                traceLog(str);
                if(m_pParent)
                {
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

                        qApp->postEvent(m_pParent, new XFoilOppEvent(m_pFoil, m_pPolar, pOpPoint));
                    }
                }
            }
            else
            {
                str = QString::asprintf("   ...unconverged after %3d iterations\n", iterations);
                traceLog(str);
                m_bErrors = true;
            }

            if(XFoil::s_bFullReport)
            {
                m_XFoilStream.flush();
                traceLog(m_XFoilLog);
                m_XFoilLog.clear();
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

    double SpMin(0), SpMax(0), SpInc(0);
    double alphadeg = m_pPolar->aoa();

    str = QString("   ")+ ALPHACHAR + QString::asprintf(" = %7.3f", alphadeg) + DEGCHAR;
    traceLog(str);

    for (int iSeries=0; iSeries<m_AnalysisRange.size(); iSeries++)
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

        if(s_bInitBL)
        {
            m_XFoilInstance.lblini = false;
            m_XFoilInstance.lipan = false;
            traceLog("   Initializing BL\n");
        }


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
            if(s_bSkipPolar)
            {
                m_XFoilInstance.lblini = false;
                m_XFoilInstance.lipan = false;
                s_bSkipPolar = false;
                traceLog("    .......skipping polar \n");
                return false;
            }

            m_XFoilInstance.alfa = alphadeg * PI/180.0;
            m_XFoilInstance.lalfa = true;
            m_XFoilInstance.qinf = 1.0;

            theta = SpMin + iter * SpInc;
            str = QString("   ")+ THETACHAR + QString::asprintf(" = %7.3f", theta) + DEGCHAR;
            traceLog(str);

            m_pFoil->setTEFlapAngle(theta);
            m_pFoil->setFlaps();
            int npts = m_pFoil->nNodes();

            QVector<double> x(npts), y(npts), nx(npts), ny(npts);
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
                str = QString::asprintf("   ...converged after %3d iterations / Cl=%5f  Cd=%5f\n", iterations, m_XFoilInstance.cl, m_XFoilInstance.cd);
                traceLog(str);
                if(m_pParent)
                {
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

                        qApp->postEvent(m_pParent, new XFoilOppEvent(m_pFoil, m_pPolar, pOpPoint));
                    }
                }
            }
            else
            {
                str = QString::asprintf("   ...unconverged after %3d iterations\n", iterations);
                traceLog(str);
                m_bErrors = true;
            }

            if(XFoil::s_bFullReport)
            {
                m_XFoilStream.flush();
                traceLog(m_XFoilLog);
                m_XFoilLog.clear();
            }


            if(fabs(SpInc)<FLAPANGLEPRECISION)
            {
                str += THETACHAR + " increment is null - aborting\n";
                traceLog(str);
                break;
            }

        }
    }

    return true;
}


bool XFoilTask::ReSequence()
{
    QString str;
    QString strange;

    for (int iSeries=0; iSeries<m_AnalysisRange.size(); iSeries++)
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

        if(s_bSkipPolar)
        {
            m_XFoilInstance.lblini = false;
            m_XFoilInstance.lipan = false;
            s_bSkipPolar = false;
            traceLog("    .......skipping polar \n");
            return false;
        }

        int iter=0;
        double SpMin = range.m_vMin;
        double SpMax = range.m_vMax;
        double SpInc = fabs(range.m_vInc);
        if(SpMax<SpMin) SpInc = -SpInc;

        double Re = SpMin;
        do
        {
            strange =QString::asprintf("Re = %7.0f ", Re);
            traceLog(strange);
            m_XFoilInstance.reinf1 = Re;
            m_XFoilInstance.lalfa = true;
            m_XFoilInstance.qinf = 1.0;

            // here we go !
            if (!m_XFoilInstance.specal())
            {
                QString str;
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
                m_bErrors = true;
            }

            if(m_pParent)
            {
                OpPoint *pOpPoint = new OpPoint;
                pOpPoint->setFoilName(m_pFoil->name());
                pOpPoint->setPolarName(m_pPolar->name());
                pOpPoint->setTheStyle(m_pPolar->theStyle());
                addXFoilData(pOpPoint, m_XFoilInstance, m_pFoil);
                pOpPoint->setTheta(m_pPolar->TEFlapAngle());
                qApp->postEvent(m_pParent, new XFoilOppEvent(m_pFoil, m_pPolar, pOpPoint));
            }

            if(XFoil::s_bFullReport)
            {
                m_XFoilStream.flush();
                traceLog(m_XFoilLog);
                m_XFoilLog.clear();
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

        if(s_bSkipOpp || s_bSkipPolar)
        {
            m_XFoilInstance.lblini = false;
            m_XFoilInstance.lipan = false;
            s_bSkipOpp = false;
            return iterations;
        }
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
        if(s_bAutoInitBL)
        {
            m_XFoilInstance.lblini = false;
            m_XFoilInstance.lipan = false;
        }
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


/**
* Adds the results of the XFoil Calculation to the OpPoint object
* @param pOpPoint a pointer to the instance of the OpPoint to be filled with the data from the XFoil object.
*/
void XFoilTask::addXFoilData(OpPoint *pOpp, XFoil &xfoil, Foil const *pFoil)
{
    int i(0), j(0), ibl(0), is(0);

    pOpp->m_Alpha      = xfoil.alfa*180.0/PI;
    pOpp->Cd           = xfoil.cd;
    pOpp->Cdp          = xfoil.cdp;
    pOpp->Cl           = xfoil.cl;
    pOpp->XCP          = xfoil.xcp;
    pOpp->Cm           = xfoil.cm;
    pOpp->m_Reynolds   = xfoil.reinf;
    pOpp->m_Mach       = xfoil.minf;
    pOpp->NCrit        = xfoil.acrit;

    pOpp->m_bTEFlap    = pFoil->hasTEFlap();
    pOpp->m_bLEFlap    = pFoil->hasLEFlap();

    pOpp->Cpmn   = xfoil.cpmn;

    pOpp->resizeSurfacePoints(xfoil.n);
    for (int k=0; k<xfoil.n; k++)
    {
        pOpp->Cpi[k] = xfoil.cpi[k+1];
        pOpp->Qi[k]  = xfoil.qgamm[k+1];
    }

    if(xfoil.lvisc && xfoil.lvconv)
    {
        pOpp->XTrTop =xfoil.xoctr[1];
        pOpp->XTrBot =xfoil.xoctr[2];
        pOpp->m_bViscResults = true;
        pOpp->m_bBL = true;

        for (int k=0; k<xfoil.n; k++)
        {
            pOpp->Cpv[k] = xfoil.cpv[k+1];
            pOpp->Qv[k]  = xfoil.qvis[k+1];
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
//            qDebug(" is=%d  %11g  %11g  ", is, pXFoil.x[i], pXFoil.dstr[ibl][is] );
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


void XFoilTask::loadSettings(QSettings &settings)
{
    settings.beginGroup("XFoilTask");
    {
        s_bAutoInitBL    = settings.value("AutoInitBL",  s_bAutoInitBL).toBool();
        s_IterLim        = settings.value("IterLim",     s_IterLim).toInt();
        s_bStoreOpp      = settings.value("StoreOpp",    s_bStoreOpp).toBool();
        s_bAlpha         = settings.value("AlphaSpec",   s_bAlpha).toBool();
        s_bViscous       = settings.value("Viscous",     s_bViscous).toBool();
        s_bInitBL        = settings.value("InitBL",      s_bInitBL).toBool();

        XFoil::setVAccel(settings.value("VAccel").toDouble());

        XFoil::setFullReport(settings.value("FullReport").toBool());
    }
    settings.endGroup();
}


void XFoilTask::saveSettings(QSettings &settings)
{
    settings.beginGroup("XFoilTask");
    {
        settings.setValue("AutoInitBL",   s_bAutoInitBL);
        settings.setValue("IterLim",      s_IterLim);
        settings.setValue("AlphaSpec",    s_bAlpha);
        settings.setValue("StoreOpp",     s_bStoreOpp);
        settings.setValue("Viscous",      s_bViscous);
        settings.setValue("InitBL",       s_bInitBL);

        settings.setValue("FullReport",   XFoil::fullReport());
        settings.setValue("VAccel",       XFoil::VAccel());
    }
    settings.endGroup();
}


