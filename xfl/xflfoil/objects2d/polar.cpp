/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#include <xflcore/xflcore.h>
#include <xflmath/constants.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/polar.h>
#include <xflfoil/objects2d/oppoint.h>


QStringList Polar::s_VariableNames = {ALPHACHAR + " ("+DEGCHAR + ")", THETACHAR + " ("+DEGCHAR + ")", "Cl", "Cd", "Cdp", "Cm",
                                     "HMom", "Cpmin", "Cl/Cd", "|Cl|^(3/2)/Cd", "1/sqrt(Cl)", "Re", "XCp",
                                     "Xtr_top", "Xtr_bot"};

Polar::Polar() : XflObject()
{
    m_Reynolds = 1.0e6;
    m_aoaSpec  = 0.0;
    m_Type     = xfl::T1POLAR;
    m_BLMethod = BL::XFOIL;
    m_ReType   = 1;
    m_MaType   = 1;

    m_TEFlapAngle = 0.0;

    m_Mach     = 0.0;
    m_ACrit    = 9.0;
    m_XTripTop = 1.0;
    m_XTripBot = 1.0;
    m_FoilName.clear();
    m_Name.clear();

}


Polar::Polar(double Re, double NCrit, double xTrTop, double xTrBot, BL::enumBLMethod blmethod)
{
    m_Reynolds = Re;
    m_aoaSpec = 0.0;
    m_Type = xfl::T1POLAR;
    m_BLMethod = blmethod;
    m_ReType = 1;
    m_MaType = 1;

    m_Mach     = 0.0;
    m_ACrit    = NCrit;
    m_XTripTop = xTrTop;
    m_XTripBot = xTrBot;
    m_FoilName.clear();
    m_Name.clear();

}



Polar::Polar(const Polar &polar)
{
    copy(polar);
}


/**
 * Exports the data of the polar to a text file
 * @param out the instance of output QtextStream
 * @param FileType TXT if the data is separated by spaces, CSV for a comma separator
 * @param bDataOnly true if the analysis parameters should not be output
 */
void Polar::exportPolar(QTextStream &out, QString const &versionName, bool bDataOnly, bool bCSV) const
{
    QString strong, Header;
    if(!bDataOnly)
    {
        strong = versionName;
        strong += "\n\n";
        out << strong;
        strong =(" Calculated polar for: ");
        strong += m_FoilName + "\n\n";
        out << strong;

        strong = QString(" %1 %2").arg(m_ReType).arg(m_MaType);

        if(m_ReType==1)      strong += (" Reynolds number fixed       ");
        else if(m_ReType==2) strong += (" Reynolds number ~ 1/sqrt(CL)");
        else if(m_ReType==3) strong += (" Reynolds number ~ 1/CL      ");

        if(m_MaType==1)      strong += ("   Mach number fixed         ");
        else if(m_MaType==2) strong += ("   Mach number ~ 1/sqrt(CL)  ");
        else if(m_MaType==3) strong += ("   Mach number ~ 1/CL        ");
        strong +="\n\n";
        out << strong;

        strong=QString((" xtrf =   %1 (top)        %2 (bottom)\n"))
                        .arg(m_XTripTop,0,'f',3).arg(m_XTripBot,0,'f',3);
        out << strong;

        strong = QString(" Mach = %1     Re = %2 e 6     Ncrit = %3\n\n")
                 .arg(m_Mach,7,'f',3).arg(Reynolds()/1.e6,9,'f',3).arg(m_ACrit,7,'f',3);
        out << strong;
    }

    if(m_Type!=xfl::T4POLAR)
    {
        if(!bCSV) Header = ("  alpha     CL        CD       CDp       Cm    Top Xtr Bot Xtr   Cpmin    Chinge    XCp    \n");
        else      Header = ("alpha,CL,CD,CDp,Cm,Top Xtr,Bot Xtr,Cpmin,Chinge,XCp\n");
        out << Header;
        if(!bCSV)
        {
            Header=QString(" ------- -------- --------- --------- -------- ------- ------- -------- --------- ---------\n");
            out << Header;
        }
        for (int j=0; j<m_Alpha.size(); j++)
        {
            if(!bCSV) strong = QString(" %1  %2  %3  %4  %5")
                                            .arg(m_Alpha[j],7,'f',3)
                                            .arg(m_Cl[j],7,'f',4)
                                            .arg(m_Cd[j],8,'f',5)
                                            .arg(m_Cdp[j],8,'f',5)
                                            .arg(m_Cm[j],7,'f',4);
            else      strong = QString("%1,%2,%3,%4,%5")
                                            .arg(m_Alpha[j],7,'f',3)
                                            .arg(m_Cl[j],7,'f',4)
                                            .arg(m_Cd[j],8,'f',5)
                                            .arg(m_Cdp[j],8,'f',5)
                                            .arg(m_Cm[j],7,'f',4);

            out << strong;
            if(m_XTrTop[j]<990.0)
            {
                if(!bCSV) strong=QString("  %1  %2").arg(m_XTrTop[j],6,'f',4).arg( m_XTrBot[j],6,'f',4);
                else      strong=QString(",%1,%2").arg(m_XTrTop[j],6,'f',4).arg( m_XTrBot[j],6,'f',4);
                out << strong;
            }
            if(!bCSV) strong=QString("  %1  %2  %3\n").arg(m_Cpmn[j],7,'f',4).arg(m_HMom[j],7,'f',4).arg(m_XCp[j],7,'f',4);
            else      strong=QString(",%1,%2,%3\n").arg(m_Cpmn[j],7,'f',4).arg(m_HMom[j],7,'f',4).arg(m_XCp[j],7,'f',4);
            out << strong;
            }
    }
    else
    {
        if(!bCSV) Header=QString(("  alpha     Re      CL        CD       CDp       Cm    Top Xtr Bot Xtr   Cpmin    Chinge     XCp    \n"));
        else      Header=QString(("alpha,Re,CL,CD,CDp,Cm,Top Xtr,Bot Xtr,Cpmin,Chinge,XCp\n"));
        out << Header;
        if(!bCSV)
        {
            Header=QString(" ------- -------- -------- --------- --------- -------- ------- ------- -------- --------- ---------\n");
            out << Header;
        }
        for (int j=0; j<m_Alpha.size(); j++)
        {
            if(!bCSV) strong=QString(" %1 %2  %3  %4  %5  %6")
                                            .arg(m_Alpha[j],7,'f',3)
                                            .arg( m_Re[j],8,'f',0)
                                            .arg( m_Cl[j],7,'f',4)
                                            .arg( m_Cd[j],8,'f',5)
                                            .arg(m_Cdp[j],8,'f',5)
                                            .arg(m_Cm[j],7,'f',4);
            else      strong=QString(" %1,%2,%3,%4,%5,%6")
                                            .arg(m_Alpha[j],7,'f',3)
                                            .arg( m_Re[j],8,'f',0)
                                            .arg( m_Cl[j],7,'f',4)
                                            .arg( m_Cd[j],8,'f',5)
                                            .arg(m_Cdp[j],8,'f',5)
                                            .arg(m_Cm[j],7,'f',4);
            out << strong;
            if(m_XTrTop[j]<990.0)
            {
                if(!bCSV) strong=QString("  %1  %2").arg(m_XTrTop[j],6,'f',4).arg(m_XTrBot[j],6,'f',4);
                else      strong=QString(",%1,%2").arg(m_XTrTop[j],6,'f',4).arg(m_XTrBot[j],6,'f',4);
                out << strong;
            }
            if(!bCSV) strong=QString("  %1  %2  %3\n").arg(m_Cpmn[j],7,'f',4).arg(m_HMom[j],7,'f',4).arg(m_XCp[j],7,'f',4);
            else      strong=QString(",%1,%2,%3\n").arg(m_Cpmn[j],7,'f',4).arg(m_HMom[j],7,'f',4).arg(m_XCp[j],7,'f',4);
            out << strong;
        }
    }
    out << "\n\n";
}


/**
 * Remove all data from the polar object
 */
void Polar::reset()
{
    m_Alpha.clear();
    m_Control.clear();
    m_Cl.clear();
    m_Cd.clear();
    m_Cdp.clear();
    m_Cm.clear();
    m_HMom.clear();
    m_Cpmn.clear();
    m_ClCd.clear();
    m_RtCl.clear();
    m_Cl32Cd.clear();
    m_Re.clear();
    m_XCp.clear();

    m_XTrTop.clear();
    m_XTrBot.clear();
    m_XLamSepTop.clear();
    m_XLamSepBot.clear();
    m_XTurbSepTop.clear();
    m_XTurbSepBot.clear();
}


void Polar::addOpPointData(OpPoint *pOpPoint)
{
    if(!pOpPoint->bViscResults()) return;

    bool bInserted = false;
    int size = m_Alpha.size();

    if(size)
    {
        for (int i=0; i<size; i++)
        {
            if(isControlPolar())
            {
                // type 5, sort by control value
                if (fabs(pOpPoint->theta() - m_Control.at(i)) < 0.0005)
                {
                    // then erase former result
                    replaceOppDataAt(i, pOpPoint);
                    bInserted = true;
                    break;
                }
                else if (pOpPoint->theta() < m_Control.at(i))
                {
                    // sort by crescending control parameter
                    insertOppDataAt(i, pOpPoint);
                    bInserted = true;
                    break;
                }
            }
            else if(isFixedaoaPolar())
            {
                // type 4, sort by speed
                if (fabs(pOpPoint->Reynolds() - m_Re.at(i)) < 0.1)
                {
                    // then erase former result
                    replaceOppDataAt(i, pOpPoint);
                    bInserted = true;
                    break;
                }
                else if (pOpPoint->Reynolds() < m_Re.at(i))
                {
                    // sort by crescending speed
                    insertOppDataAt(i, pOpPoint);
                    bInserted = true;
                    break;
                }
            }
            else
            {
                if (fabs(pOpPoint->aoa()-m_Alpha.at(i)) < 0.0005)
                {
                    replaceOppDataAt(i, pOpPoint);
                    bInserted = true;
                    break;
                }
                else if (pOpPoint->aoa() < m_Alpha.at(i))
                {
                    insertOppDataAt(i, pOpPoint);
                    bInserted = true;
                    break;
                }
            }
        }
    }

    if(!bInserted)
    {
        // data is appended at the end
        int size = m_Alpha.size();
        insertOppDataAt(size, pOpPoint);
    }
}


void Polar::replaceOppDataAt(int pos, OpPoint const *pOpp)
{
    if(pos<0 || pos>= m_Alpha.size()) return;

    m_Alpha[pos]   =  pOpp->aoa();
    m_Control[pos] =  pOpp->theta();
    m_Cd[pos]      =  pOpp->Cd;
    m_Cdp[pos]     =  pOpp->Cdp;
    m_Cl[pos]      =  pOpp->Cl;
    m_Cm[pos]      =  pOpp->Cm;
    m_HMom[pos]    =  pOpp->m_TEHMom;
    m_Cpmn[pos]    =  pOpp->Cpmn;
    m_ClCd[pos]    =  pOpp->Cl/pOpp->Cd;
    m_XCp[pos]     =  pOpp->XCP;

    m_XTrTop[pos]      =  pOpp->XTrTop;
    m_XTrBot[pos]      =  pOpp->XTrBot;
    m_XLamSepTop[pos]  =  pOpp->XLamSepTop;
    m_XLamSepBot[pos]  =  pOpp->XLamSepBot;
    m_XTurbSepTop[pos] =  pOpp->XTurbSepTop;
    m_XTurbSepBot[pos] =  pOpp->XTurbSepBot;

    if(pOpp->Cl>0.0) m_RtCl[pos] = 1.0/sqrt(pOpp->Cl);
    else             m_RtCl[pos] = 0.0;
    if (pOpp->Cl>=0.0) m_Cl32Cd[pos] =  pow( pOpp->Cl, 1.5)/ pOpp->Cd;
    else               m_Cl32Cd[pos] = -pow(-pOpp->Cl, 1.5)/ pOpp->Cd;

    if(m_Type==xfl::T1POLAR)  m_Re[pos] =  pOpp->Reynolds();
    else if (m_Type==xfl::T2POLAR)
    {
        if(pOpp->Cl>0.0) m_Re[pos] =  pOpp->Reynolds()/ sqrt(pOpp->Cl);
        else             m_Re[pos] = 0.0;
    }
    else if (m_Type==xfl::T3POLAR)
    {
        if(pOpp->Cl>0.0) m_Re[pos] =  pOpp->Reynolds()/(pOpp->Cl);
        else             m_Re[pos] = 0.0;
    }
}


void Polar::insertOppDataAt(int i, OpPoint const *pOpp)
{
    m_Alpha.insert(  i, pOpp->aoa());
    m_Control.insert(i, pOpp->theta());
    m_Cd.insert(     i, pOpp->Cd);
    m_Cdp.insert(    i, pOpp->Cdp);
    m_Cl.insert(     i, pOpp->Cl);
    m_Cm.insert(     i, pOpp->Cm);

    m_HMom.insert(   i, pOpp->m_TEHMom);
    m_Cpmn.insert(   i, pOpp->Cpmn);
    m_ClCd.insert(   i, pOpp->Cl/pOpp->Cd);
    m_XCp.insert(    i, pOpp->XCP);


    m_XTrTop.insert(     i, pOpp->XTrTop);
    m_XTrBot.insert(     i, pOpp->XTrBot);
    m_XLamSepTop.insert( i, pOpp->XLamSepTop);
    m_XLamSepBot.insert( i, pOpp->XLamSepBot);
    m_XTurbSepTop.insert(i, pOpp->XTurbSepTop);
    m_XTurbSepBot.insert(i, pOpp->XTurbSepBot);

    if(pOpp->Cl>0.0) m_RtCl.insert(i, 1.0/sqrt(pOpp->Cl));
    else             m_RtCl.insert(i, 0.0);

    if (pOpp->Cl>=0.0) m_Cl32Cd.insert(i, pow( pOpp->Cl, 1.5) / pOpp->Cd);
    else               m_Cl32Cd.insert(i,-pow(-pOpp->Cl, 1.5) / pOpp->Cd);

    if(isFixedSpeedPolar())
    {
        m_Re.insert(i, pOpp->Reynolds());
    }
    else if (isFixedLiftPolar())
    {
        if(pOpp->Cl>0) m_Re.insert(i, pOpp->Reynolds()/sqrt(pOpp->Cl));
        else           m_Re.insert(i, 0.0);
    }
    else if (isRubberChordPolar())
    {
        if(pOpp->Cl>0.0) m_Re.insert(i, pOpp->Reynolds()/pOpp->Cl);
        else             m_Re.insert(i, 0.0);
    }
    else if (isFixedaoaPolar())
    {
        m_Re.insert(i, pOpp->Reynolds());
    }
    else if (isControlPolar())
    {
        m_Re.insert(i, pOpp->Reynolds());
    }
}


/**
 * Adds the parameter data to the data arrays
 * The index used to insert the data is the aoa for type 1, 2 and 3 polars, and the freestream velocity for type 4 polars.
 * If a point with identical index exists, the data is replaced.
 * If not, the data is inserted for this index.
 *
 */
void Polar::addPoint(double Alpha, double Cd,   double Cdp,      double Cl,     double Cm,
                     double HMom,  double Cpmn, double Reynolds, double XCp,    double Ctrl,
                     double Xtr1,  double Xtr2, double XLSTop,   double XLSBot, double XTSTop, double XTSBot)
{
    OpPoint Opp;
    Opp.m_bViscResults = true;
    Opp.m_Alpha     = Alpha;
    Opp.Cd          = Cd;
    Opp.Cdp         = Cdp;
    Opp.Cl          = Cl;
    Opp.Cm          = Cm;
    Opp.m_TEHMom    = HMom;
    Opp.Cpmn        = Cpmn;
    Opp.m_Reynolds  = Reynolds;
    Opp.XCP         = XCp;
    Opp.m_Theta      = Ctrl;

    Opp.XTrTop      = Xtr1;
    Opp.XTrBot      = Xtr2;
    Opp.XLamSepTop  = XLSTop;
    Opp.XLamSepBot  = XLSBot;
    Opp.XTurbSepTop = XTSTop;
    Opp.XTurbSepBot = XTSBot;

    addOpPointData(&Opp);
}


/**
 * Copies the polar's data from an existing polar
 * @param pPolar a pointer to the instance of the reference Polar object from which the data should be copied
 */
void Polar::copy(Polar *pPolar)
{
    copySpecification(pPolar);

    m_Alpha    = pPolar->m_Alpha;
    m_Control  = pPolar->m_Control;
    m_Cd       = pPolar->m_Cd;
    m_Cdp      = pPolar->m_Cdp;
    m_Cl       = pPolar-> m_Cl;
    m_Cm       = pPolar->m_Cm;
    m_HMom     = pPolar->m_HMom;
    m_Cpmn     = pPolar->m_Cpmn;
    m_ClCd     = pPolar->m_ClCd;
    m_RtCl     = pPolar->m_RtCl;
    m_Cl32Cd   = pPolar->m_Cl32Cd;
    m_Re       = pPolar->m_Re;
    m_XCp      = pPolar->m_XCp;

    m_XTrTop      = pPolar->m_XTrTop;
    m_XTrBot      = pPolar->m_XTrBot;
    m_XLamSepTop  = pPolar->m_XLamSepTop;
    m_XLamSepBot  = pPolar->m_XLamSepBot;
    m_XTurbSepTop = pPolar->m_XTurbSepTop;
    m_XTurbSepBot = pPolar->m_XTurbSepBot;
}


void Polar::copy(Polar const &polar)
{
    copySpecification(polar);

    m_Alpha   =  polar.m_Alpha;
    m_Control =  polar.m_Control;
    m_Cd      =  polar.m_Cd;
    m_Cdp     =  polar.m_Cdp;
    m_Cl      =  polar.m_Cl;
    m_Cm      =  polar.m_Cm;
    m_HMom    =  polar.m_HMom;
    m_Cpmn    =  polar.m_Cpmn;
    m_ClCd    =  polar.m_ClCd;
    m_RtCl    =  polar.m_RtCl;
    m_Cl32Cd  =  polar.m_Cl32Cd;
    m_Re      =  polar.m_Re;
    m_XCp     =  polar.m_XCp;

    m_XTrTop      = polar.m_XTrTop;
    m_XTrBot      = polar.m_XTrBot;
    m_XLamSepTop  = polar.m_XLamSepTop;
    m_XLamSepBot  = polar.m_XLamSepBot;
    m_XTurbSepTop = polar.m_XTurbSepTop;
    m_XTurbSepBot = polar.m_XTurbSepBot;
}


void Polar::copySpecification(Polar const*pPolar)
{
    copySpecification(*pPolar);
}


void Polar::copySpecification(Polar const &polar)
{
    m_Name        = polar.m_Name;
    m_FoilName    = polar.m_FoilName;

    m_TEFlapAngle = polar.m_TEFlapAngle;

    m_theStyle    = polar.m_theStyle;

    m_Type   = polar.m_Type;
    m_BLMethod    = polar.m_BLMethod;
    m_ReType      = polar.m_ReType;
    m_MaType      = polar.m_MaType;

    m_Reynolds    = polar.m_Reynolds;

    m_aoaSpec     = polar.m_aoaSpec;
    m_Mach        = polar.m_Mach;
    m_ACrit       = polar.m_ACrit;
    m_XTripTop    = polar.m_XTripTop;
    m_XTripBot    = polar.m_XTripBot;
}


void Polar::removePoint(int i)
{
    m_Alpha.removeAt(i);
    m_Control.removeAt(i);
    m_Cl.removeAt(i);
    m_Cd.removeAt(i);
    m_Cdp.removeAt(i);
    m_Cm.removeAt(i);
    m_HMom.removeAt(i);
    m_Cpmn.removeAt(i);
    m_ClCd.removeAt(i);
    m_RtCl.removeAt(i);
    m_Cl32Cd.removeAt(i);
    m_Re.removeAt(i);
    m_XCp.removeAt(i);

    m_XTrTop.removeAt(i);
    m_XTrBot.removeAt(i);
    m_XLamSepTop.removeAt(i);
    m_XLamSepBot.removeAt(i);
    m_XTurbSepTop.removeAt(i);
    m_XTurbSepBot.removeAt(i);
}

void Polar::insertPoint(int i)
{
    m_Alpha.insert(i, 0.0);
    m_Control.insert(i, 0.0);
    m_Cl.insert(i, 0.0);
    m_Cd.insert(i, 0.0);
    m_Cdp.insert(i, 0.0);
    m_Cm.insert(i, 0.0);
    m_HMom.insert(i, 0.0);
    m_Cpmn.insert(i, 0.0);
    m_ClCd.insert(i, 0.0);
    m_RtCl.insert(i, 0.0);
    m_Cl32Cd.insert(i, 0.0);
    m_Re.insert(i, 0.0);
    m_XCp.insert(i, 0.0);

    m_XTrTop.insert(i, 0.0);
    m_XTrBot.insert(i, 0.0);
    m_XLamSepTop.insert(i, 0.0);
    m_XLamSepBot.insert(i, 0.0);
    m_XTurbSepTop.insert(i, 0.0);
    m_XTurbSepBot.insert(i, 0.0);
}


/**
* Returns the minimum and maximum angles of attack stored in the polar.
* Since the data is sorted by crescending aoa, this is a matter of returning the first and last values of the array.
*@param &amin the miminum aoa
*@param &amax the maximum aoa
*/
void Polar::getAlphaLimits(double &amin, double &amax) const
{
    if(!m_Alpha.size())
    {
        amin = amax = 0.0;
    }
    else
    {
        amin = 100.0;
        amax =  -100.0;
        for(int i=0; i<m_Alpha.size(); i++)
        {
            amin = std::min(amin, m_Alpha.at(i));
            amax = std::max(amax, m_Alpha.at(i));
        }
    }
}


/**
* Returns the minimum and maximum lift coefficients stored in the polar.
*@param &Clmin the miminum lift coefficient
*@param &Clmax the maximum lift coefficient
*/
void Polar::getClLimits(double &Clmin, double &Clmax) const
{
    if(!m_Cl.size())
    {
        Clmin = 0.0;
        Clmax = 0.0;
    }
    else
    {
        Clmin = 10000.0;
        Clmax =-10000.0;
        double Cl;
        for (int i=0;i<m_Cl.size(); i++)
        {
            Cl = m_Cl[i];
            if(Clmin>Cl) Clmin = Cl;
            if(Clmax<Cl) Clmax = Cl;
        }
    }
}

/**
* Returns the moment coefficient at zero-lift.
* Cm0 is interpolated between the two points in the array such that Cl[i]<0 and Cl[i+1]>0.
* If no such pair is found, the method returns 0.
*@return Cm0
*/
double Polar::getCm0() const
{
    double Clmin =  1000.0;
    double Clmax = -1000.0;
    for (int i=0; i<m_Cl.size(); i++)
    {
        Clmin = qMin(Clmin, m_Cl[i]);
        Clmax = qMax(Clmax, m_Cl[i]);
    }
    if(!(Clmin<0.0) || !(Clmax>0.0)) return 0.0;
    int k=0;
//    double rr  = m_Cl[k];
//    double rr1 = m_Cl[k+1];

    while (m_Cl[k+1]<0.0)
    {
//        rr  = m_Cl[k];
//        rr1 = m_Cl[k+1];
        k++;
    }
    if(k+1>=m_Cm.size()) return 0.0;
    double Cm0 = m_Cm[k] + (m_Cm[k+1]-m_Cm[k])*(0.0-m_Cl[k])/(m_Cl[k+1]-m_Cl[k]);
    return Cm0;
}


double Polar::getZeroLiftAngle() const
{
    double Clmin =  1000.0;
    double Clmax = -1000.0;
    for (int i=0; i<m_Cl.size(); i++)
    {
        Clmin = qMin(Clmin, m_Cl.at(i));
        Clmax = qMax(Clmax, m_Cl.at(i));
    }
    if(!(Clmin<0.0) || !(Clmax>0.0)) return 0.0;
    int k=0;

    while (m_Cl.at(k)<0.0)
    {
        if(k+1>=m_Cl.size()) return 0.0; // error
        k++;
    }
    if(k+1>=m_Alpha.size()) return 0.0;
    double Alpha0 = m_Alpha.at(k) + (m_Alpha.at(k+1)-m_Alpha.at(k))*(0.0-m_Cl.at(k))/(m_Cl.at(k+1)-m_Cl.at(k));
    return Alpha0;
}


void Polar::getStallAngles(double &negative, double &positive) const
{
    double alpha0 = getZeroLiftAngle();
    positive = negative = alpha0;

    // determine the positive stall angle
    for(int i=0; i<m_Alpha.size(); i++)
    {
        if(m_Alpha.at(i)<alpha0) continue;
        positive = m_Alpha.at(i);
        if(i==m_Alpha.size()-1) break; //use the polar's last angle
        if(m_Cl.at(i+1)-m_Cl.at(i)<0) break; // stall detected
    }

    // determine the negative stall angle
    for(int i=m_Alpha.size()-1; i>=0; i--)
    {
        if(m_Alpha.at(i)>alpha0) continue;
        negative = m_Alpha.at(i);
        if(i==0) break; //use the polar's first angle
        if(m_Cl.at(i)-m_Cl.at(i-1)<0) break; // stall detected
    }
}


/**
* Linearizes Cl vs. Alpha set of points by least square method
* @param Alpha0 the zero-lift angle, i.e.such that Cl = 0, in degrees
* @param slope the slope of the curve Cl=f(aoa), in units 1/°
*/
void Polar::getLinearizedCl(double &Alpha0, double &slope) const
{
    int n = m_Cl.size();

    if(n<=1)
    {
        Alpha0 = 0.0;
        slope = 2.0*PI;
        return;
    }

    double fn = double(n);
    double sum1 = 0.0;
    double sum2 = 0.0;
    double sum3 = 0.0;
    double sum4 = 0.0;
    double b1, b2;

    for (int k=0; k<n; k++)
    {
        sum1 += m_Cl[k] * m_Alpha[k];
        sum2 += m_Alpha[k];
        sum3 += m_Cl[k];
        sum4 += m_Alpha[k] * m_Alpha[k];
    }
    if(fabs(fn*sum4-sum2*sum2)<PRECISION || fabs(fn*sum1-sum2*sum3)<PRECISION)
    {
        //very improbable...
        Alpha0 = 0.0;
        slope = 2.0*PI;
        return;
    }

    b1 = (fn*sum1 - sum2 * sum3)/(fn*sum4 - sum2*sum2);
    b2 = (sum3 - b1 * sum2)/fn;

    slope  = b1; //in cl/°
    Alpha0 = -b2/b1;
}


void Polar::setType(xfl::enumPolarType type)
{
    m_Type =type;
    switch (m_Type)
    {
        case xfl::T1POLAR:
            m_MaType = 1;
            m_ReType = 1;
            break;
        case xfl::T2POLAR:
            m_MaType = 2;
            m_ReType = 2;
            break;
        case xfl::T3POLAR:
            m_MaType = 1;
            m_ReType = 3;
            break;
        case xfl::T4POLAR:
            m_MaType = 1;
            m_ReType = 1;
            break;
        default:
            m_ReType = 1;
            m_MaType = 1;
            break;
    }
}


void Polar::getProperties(Foil const *pFoil, QString &polarprops)
{
    QString strong;
    polarprops.clear();


    switch(m_BLMethod)
    {
        case BL::XFOIL:         polarprops += "BL Solver: XFoil\n";         break;
        case BL::NOBLMETHOD:    polarprops += "BL Solver: Blasius\n";       break;
    }

    strong = QString::asprintf("Type = %d", m_Type+1);
    if     (m_Type==xfl::T1POLAR) strong += " (Fixed speed)\n";
    else if(m_Type==xfl::T2POLAR) strong += " (Fixed lift)\n";
    else if(m_Type==xfl::T4POLAR) strong += " (Fixed angle of attack)\n";
    else if(m_Type==xfl::T6POLAR) strong += " (Control polar)\n";
    polarprops += strong;

    if( (isType123() || isType4()))
    {
        if(pFoil && pFoil->hasTEFlap())
        {
            strong = "T.E. flap angle: " + THETACHAR + QString::asprintf(" = %g", m_TEFlapAngle) + DEGCHAR + EOLCHAR;
        }
        else
        {
            strong = "No T.E. flap" + EOLCHAR;
        }
        polarprops += strong;
    }


    if(m_Type==xfl::T1POLAR)
    {
        strong = QString::asprintf("Reynolds    = %.0f\n", Reynolds());
        polarprops += strong;
        strong = QString::asprintf("Mach        = %5.2f\n", m_Mach);
        polarprops += strong;
    }
    else if(m_Type==xfl::T2POLAR)
    {
        strong = QString::asprintf("Re.sqrt(Cl) = %.0f\n",Reynolds());
        polarprops += strong;
        strong = QString::asprintf("Ma.sqrt(Cl) = %5.2f\n",m_Mach);
        polarprops += strong;
    }
    else if(m_Type==xfl::T3POLAR)
    {
        strong = QString::asprintf("Re.Cl       = %.0f\n",Reynolds());
        polarprops += strong;
        strong = QString::asprintf("Mach        = %.2f\n",m_Mach);
        polarprops += strong;
    }
    else if(m_Type==xfl::T4POLAR)
    {
        strong = ALPHACHAR + QString::asprintf("           = %5.2f",m_aoaSpec) + DEGCHAR +"\n";
        polarprops += strong;
        strong = QString::asprintf("Mach        = %5.2f\n",m_Mach);
        polarprops += strong;
    }
    else if(m_Type==xfl::T6POLAR)
    {
        strong = ALPHACHAR + QString::asprintf("           = %5.2f",m_aoaSpec) + DEGCHAR +"\n";
        polarprops += strong;

        strong = QString::asprintf("Reynolds    = %.0f\n", Reynolds());
        polarprops += strong;
        strong = QString::asprintf("Mach        = %5.2f\n", m_Mach);
        polarprops += strong;
    }


    strong = QString::asprintf("NCrit       = %5.2f\n", m_ACrit);
    polarprops += strong;

    strong = QString::asprintf("Forced top trans.    = %.2f\n", m_XTripTop);
    polarprops += strong;

    strong = QString::asprintf("Forced bottom trans. = %.2f\n", m_XTripBot);
    polarprops += strong;

    if(isType12())
    {
        strong = ALPHACHAR + QString::asprintf("0 = %5.2f", getZeroLiftAngle());
        polarprops += strong + DEGCHAR + EOLCHAR;

        double positive=0, negative=0;
        getStallAngles(negative, positive);
        strong = QString::asprintf("stall angle- = %5.2f", negative);
        polarprops += strong + DEGCHAR + EOLCHAR;
        strong = QString::asprintf("stall angle+ = %5.2f", positive);
        polarprops += strong + DEGCHAR + EOLCHAR;
    }

    strong = QString::asprintf("Number of data points = %d", int(m_Alpha.size()));
    polarprops += "\n" +strong;
}


bool Polar::serializePolarXFL(QDataStream &ar, bool bIsStoring)
{
    double dble(0.0);
    bool boolean(false);
    int i(0), k(0), n(0);
    int ArchiveFormat(0);// identifies the format of the file
    // 100005 : added the array of control values
    ArchiveFormat = 100005;

    if(bIsStoring)
    {
        ar << ArchiveFormat; // first format for XFL file

        ar << m_FoilName;
        ar << m_Name;

        m_theStyle.serializeXfl(ar, bIsStoring);

        if     (m_Type==xfl::T1POLAR)          ar<<1;
        else if(m_Type==xfl::T2POLAR)          ar<<2;
        else if(m_Type==xfl::T3POLAR) ar<<3;
        else if(m_Type==xfl::T4POLAR)          ar<<4;
        else                                        ar<<1;

        ar << m_MaType << m_ReType;
        ar << Reynolds() << m_Mach;
        ar << m_aoaSpec;
        ar << m_XTripTop << m_XTripBot;
        ar << m_ACrit;

        ar << int(m_Alpha.size());
        for (i=0; i< m_Alpha.size(); i++)
        {
            ar << float(m_Alpha[i])  << float(m_Cd[i]);
            ar << float(m_Cdp[i])    << float(m_Cl[i]) << float(m_Cm[i]);
            ar << float(m_XTrTop[i]) << float(m_XTrBot[i]);
            ar << float(m_HMom[i])   << float(m_Cpmn[i]);
            ar << float(m_Re[i]);
            ar << float(m_XCp[i]);
        }

//        ar << m_theStyle.m_Symbol;
        // space allocation for the future storage of more data, without need to change the format
        for (int i=0; i<19; i++) ar << 0;
        for (int i=0; i<50; i++) ar << 0.0;

        return true;
    }
    else
    {
        //read variables
        float Alpha(0), Cd(0), Cdp(0), Cl(0), Cm(0), XTr1(0), XTr2(0), HMom(0), Cpmn(0), Re(0), XCp(0), Ctrl(0);

        ar >> ArchiveFormat;
        if (ArchiveFormat <100000 || ArchiveFormat>110000) return false;

        ar >> m_FoilName;
        ar >> m_Name;
        if(ArchiveFormat<100005)
        {
            int s(0);
            int w(0);
            ar >>s>>w;
            setLineStipple(LineStyle::convertLineStyle(s));
            setLineWidth(w);
            int r(0),g(0),b(0),a(0);
            xfl::readColor(ar, r,g,b,a);
            setLineColor(QColor(r,g,b,a));
            ar >> m_theStyle.m_bIsVisible >> boolean;
        }
        else
            m_theStyle.serializeXfl(ar, bIsStoring);

        ar >> n;
        if     (n==2) m_Type=xfl::T2POLAR;
        else if(n==3) m_Type=xfl::T3POLAR;
        else if(n==4) m_Type=xfl::T4POLAR;
        else if(n==5) m_Type=xfl::T6POLAR;
        else m_Type=xfl::T1POLAR;

        ar >> m_MaType >> m_ReType;

        ar >> m_Reynolds >> m_Mach;

        ar >> m_aoaSpec;
        ar >> m_XTripTop >> m_XTripBot;
        ar >> m_ACrit;

        ar >> n;

        for (i=0; i<n; i++)
        {
            ar >> Alpha >> Cd >> Cdp >> Cl >> Cm >> XTr1 >> XTr2 >> HMom >> Cpmn >> Re >> XCp;

            addPoint(double(Alpha), double(Cd), double(Cdp), double(Cl), double(Cm), double(HMom), double(Cpmn),
                     double(Re), double(XCp), double(Ctrl), double(XTr1), double(XTr2),
                     0,0,0,0);
        }
        if(ArchiveFormat<100005)
        {
            ar >> n;
            m_theStyle.m_Symbol=LineStyle::convertSymbol(n);
        }

        // space allocation
        for (int i=0; i<19; i++) ar >> k;
        for (int i=0; i<50; i++) ar >> dble;
    }


    return true;
}


bool Polar::serializePolarFl5(QDataStream &ar, bool bIsStoring)
{
    int nIntSpares(0);
    int nDbleSpares(0);

    int nVariables(0);
    int n(0);

    int ArchiveFormat(0);// identifies the format of the file
    // 500001 : initialization of the new fl5 format
    // 500750 : v7.50

    ArchiveFormat = 500750;

    if(bIsStoring)
    {
        ar << ArchiveFormat; // first format for XFL file

        ar << m_FoilName;
        ar << m_Name;

        m_theStyle.serializeFl5(ar, bIsStoring);


        if     (isFixedSpeedPolar())  ar<<1;
        else if(isFixedLiftPolar())   ar<<2;
        else if(isRubberChordPolar()) ar<<3;
        else if(isFixedaoaPolar())    ar<<4;
        else if(isControlPolar())     ar<<5;
        else                          ar<<1;

        switch(m_BLMethod)
        {
            default:
            case BL::XFOIL:         n=0;  break;
            case BL::NOBLMETHOD:    n=4;  break;
        }
        ar << n;

        ar << m_MaType << m_ReType;

/*        ar << m_Density << m_nu;
        ar << m_QInf;
        ar << m_Chord;*/
        ar << m_Reynolds;
        ar << m_Mach;
        ar << m_aoaSpec;
        ar << m_XTripTop << m_XTripBot;
        ar << m_ACrit;

//        ar << nCtrls();
//        for (int ic=0; ic<Polar::nCtrls(); ic++) ar << dble<<dble;
        ar << nVariables; // formerly nCtrls

        nVariables = 12; // change to add new variables
        ar << nVariables;
        ar << int(m_Alpha.size());
        for (int i=0; i< m_Alpha.size(); i++)
        {
            ar << float(m_Alpha.at(i)) << float(m_Cd.at(i));
            ar << float(m_Cdp.at(i))   << float(m_Cl.at(i)) << float(m_Cm.at(i));
            ar << float(m_HMom.at(i))  << float(m_Cpmn.at(i));
            ar << float(m_Re.at(i));
            ar << float(m_XCp.at(i));
            ar << float(m_Control.at(i));
            ar << float(m_XTrTop.at(i)) << float(m_XTrBot.at(i));
            ar << float(m_XLamSepTop.at(i)) << float(m_XLamSepBot.at(i));
            ar << float(m_XTurbSepTop.at(i)) << float(m_XTurbSepBot.at(i));
        }

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=1;
        ar << nDbleSpares;
        ar << m_TEFlapAngle;

        return true;
    }
    else
    {
        //read variables
        float Alpha, Cd(0), Cdp(0), Cl(0), Cm(0), HMom(0), Cpmn(0), Re(0), XCp(0), Ctrl(0);
        float XTr1(0), XTr2(0), XLSTop(0), XLSBot(0), XTSTop(0), XTSBot(0);

        ar >> ArchiveFormat;
        if (ArchiveFormat < 500000 || ArchiveFormat>501000) return false;

        ar >> m_FoilName;
        ar >> m_Name;

        m_theStyle.serializeFl5(ar, bIsStoring);

        ar >> n;
        if     (n==1) m_Type=xfl::T1POLAR;
        else if(n==2) m_Type=xfl::T2POLAR;
        else if(n==3) m_Type=xfl::T3POLAR;
        else if(n==4) m_Type=xfl::T4POLAR;
        else if(n==5) m_Type=xfl::T6POLAR;

        ar >>n;
        switch(n)
        {
            case 0: m_BLMethod=BL::XFOIL;         break;
            case 4: m_BLMethod=BL::NOBLMETHOD;    break;
        }

        ar >> m_MaType >> m_ReType;

        if(ArchiveFormat<500750)
        {
            double qinf(0), rho(0), nu(0), chord(0);
            ar >> rho >> nu; //m_Density >> m_nu;
            ar >> qinf; // m_QInf;
            ar >> chord; // m_Chord;
            m_Reynolds = qinf*chord/nu;
        }
        else
        {
            ar >> m_Reynolds;
        }
        ar >> m_Mach;
        ar >> m_aoaSpec;
        ar >> m_XTripTop >> m_XTripBot;
        ar >> m_ACrit;

        ar >> n; // formerly nCtrls;
        double d1(0), d2(0);
        for (int ic=0; ic<n; ic++)
        {
            ar>>d1>>d2;
        }

        ar >> nVariables;
        ar >> n;
        for (int i=0; i< n; i++)
        {
            ar >> Alpha >> Cd >> Cdp >> Cl >> Cm >> HMom >> Cpmn >> Re >> XCp >> Ctrl;
            ar >> XTr1 >> XTr2 >> XLSTop >> XLSBot >> XTSTop >>XTSBot;
            addPoint(double(Alpha), double(Cd), double(Cdp), double(Cl), double(Cm), double(HMom), double(Cpmn), double(Re), double(XCp), double(Ctrl),
                     double(XTr1), double(XTr2), double(XLSTop), double(XLSBot), double(XTSTop), double(XTSBot));
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        if(nDbleSpares>0)
            ar >> m_TEFlapAngle;
    }
    return true;
}


bool Polar::hasOpp(OpPoint const *pOpp) const
{
    return (pOpp->foilName()==m_FoilName && pOpp->polarName()==m_Name);
}


/**
* Returns a pointer to a variable array of a polar object, based on the variable's index
* @param pPolar the pointer to the polar object
* @param iVar the index of the variable
* @return a const reference to the array holding the values of the variable
*/
QVector<double> const & Polar::getPlrVariable(Polar::enumPolarVariable var) const
{
    switch (var)
    {
        case Polar::ALPHA:
            return m_Alpha;
        case Polar::CTRL:
            return m_Control;
        case Polar::CL:
            return m_Cl;
        case Polar::CD:
            return m_Cd;
        case Polar::CDP:
            return m_Cdp;
        case Polar::CM:
            return m_Cm;
        case Polar::HMOM:
            return m_HMom;
        case Polar::CPMIN:
            return m_Cpmn;
        case Polar::CLCD:
            return m_ClCd;
        case Polar::CL32CD:
            return m_Cl32Cd;
        case Polar::XCP:
            return m_XCp;
        case Polar::XTRTOP:
            return m_XTrTop;
        case Polar::XTRBOT:
            return m_XTrBot;
        case Polar::XLSTOP:
            return m_XLamSepTop;
        case Polar::XLSBOT:
            return m_XLamSepBot;
        case Polar::XTSTOP:
            return m_XTurbSepTop;
        case Polar::XTSBOT:
            return m_XTurbSepBot;
        default:
            return m_Alpha;
    }
}


/**
 * Returns a void pointer to the array of the specified variable of the input Polar
 * @param iVar the index of the variable for which a pointer is requested
 * @return a const reference to the array of the requested variable
 */
QVector<double> const & Polar::getVariable(int iVar) const
{
    switch (iVar)
    {
        case 0:  return m_Alpha;
        case 1:  return m_Control;
        case 2:  return m_Cl;
        case 3:  return m_Cd;
        case 4:  return m_Cdp;
        case 5:  return m_Cm;
        case 6:  return m_HMom;
        case 7:  return m_Cpmn;
        case 8:  return m_ClCd;
        case 9:  return m_Cl32Cd;
        case 10: return m_Cl;
        case 11: return m_Re;
        case 12: return m_XCp;
        case 13: return m_XTrTop;
        case 14: return m_XTrBot;
        default:
            break;
    }
    return m_Alpha;
}


double Polar::interpolateFromAlpha(double alpha, Polar::enumPolarVariable PlrVar, bool &bOutAlpha) const
{
    QVector<double> const &pX = getPlrVariable(PlrVar);
    double amin=100.0, amax=-100;
    getAlphaLimits(amin, amax);

    if(alpha<amin)
    {
        bOutAlpha = true;
        return pX.first();
    }

    if(alpha>amax)
    {
        bOutAlpha = true;
        return pX.last();
    }

    for (int i=0; i<m_Alpha.size()-1; i++)
    {
        if(m_Alpha.at(i)<=alpha && alpha<m_Alpha.at(i+1))
        {
            //interpolate
            if(m_Alpha.at(i+1)-m_Alpha.at(i)<0.00001)//do not divide by zero
                return pX.at(i);
            else
            {
                double u = (alpha - m_Alpha.at(i)) /(m_Alpha.at(i+1)-m_Alpha.at(i));
                return pX.at(i) + u * (pX.at(i+1)-pX.at(i));
            }
        }
    }

//    Q_ASSERT(false);
    bOutAlpha = true;
    return 0.0;
}


double Polar::interpolateFromCl(double Cl, Polar::enumPolarVariable PlrVar, bool &bOutCl) const
{
    //interpolate Cl on this polar
    QVector <double> const &pX = getPlrVariable(PlrVar);

    double Clmin = 100.0;
    double Clmax = -100.0;
    getClLimits(Clmin, Clmax);
    if(Cl < Clmin)
    {
        bOutCl = true;
        if(pX.size()) return pX.front();
        else          return 0.0;
    }
    else if(Cl > Clmax)
    {
        bOutCl= true;
        if(pX.size()) return pX.back();
        else          return 0.0;
    }
    else
    {
        //Start from the point closest to Cl=0 because of weird shaped polars
        int pt = 0;
        double  dist = fabs(m_Cl.at(0));
        for (int i=1; i<m_Cl.size();i ++)
        {
            if (fabs(m_Cl.at(i))< dist)
            {
                dist = fabs(m_Cl.at(i));
                pt = i;
            }
        }
        if(Cl<m_Cl.at(pt))
        {
            for (int i=pt; i>0; i--)
            {
                if(Cl<= m_Cl.at(i) && Cl > m_Cl.at(i-1))
                {
                    //interpolate
                    if(fabs(m_Cl.at(i)-m_Cl.at(i-1)) < 0.00001)
                    {
                        //do not divide by zero
                        return pX.at(i);
                        break;
                    }
                    else
                    {
                        double u = (Cl - m_Cl.at(i-1))
                                  /(m_Cl.at(i)-m_Cl.at(i-1));
                        return pX.at(i-1) + u * (pX.at(i)-pX.at(i-1));
                        break;
                    }
                }
            }
        }
        else
        {
            for (int i=pt; i<m_Cl.size()-1; i++)
            {
                if(m_Cl.at(i) <=Cl && Cl < m_Cl.at(i+1))
                {
                    //interpolate
                    if(fabs(m_Cl.at(i+1)-m_Cl.at(i)) < 0.00001){//do not divide by zero
                        return pX.at(i);
                        break;
                    }
                    else
                    {
                        double u = (Cl - m_Cl.at(i)) / (m_Cl.at(i+1)-m_Cl.at(i));
                        return pX.at(i) + u * (pX.at(i+1)-pX.at(i));
                        break;
                    }
                }
            }
        }
    }

//    Q_ASSERT(false);

    bOutCl = true;
    return 0.0;
}


void Polar::makeCurve(QPolygonF &pts, int XVar, int YVar)  const
{
    pts.clear();

    QVector <double> const &pX = getVariable(XVar);
    QVector <double> const &pY = getVariable(YVar);
    double fx = 1.0;
    double fy = 1.0;


    for (int i=0; i<m_Alpha.size(); i++)
    {
        if (XVar==12)
        {
            if(pX.at(i)>0.0)
            {
                if (YVar==12)
                {
                    if(pY.at(i)>0.0)
                    {
                        pts.append({1.0/sqrt(pX.at(i)), 1.0/sqrt(pY.at(i))});
                    }
                }
                else
                {
                    pts.append({1.0/sqrt(pX.at(i)), pY.at(i)*fy});
                }
            }
        }
        else{
            if (YVar==12)
            {
                if(pY.at(i)>0.0)
                {
                    pts.append({pX.at(i)*fx, 1.0/sqrt(pY.at(i))});
                }
            }
            else
            {
                pts.append({pX.at(i)*fx, pY.at(i)*fy});
            }
        }
    }
}



