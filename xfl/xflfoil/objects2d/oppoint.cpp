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

#include <QRandomGenerator>


#include "oppoint.h"
#include <xflcore/xflcore.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/polar.h>


OpPoint::OpPoint() : XflObject()
{
    m_BLMethod = BL::XFOIL;
    m_bViscResults = false;//not a  viscous point a priori
    m_bBL          = false;// no boundary layer surface either
    m_bTEFlap      = false;
    m_bLEFlap      = false;

    m_Alpha    = 0.0;
    m_Reynolds = 0.0;
    m_Mach     = 0.0;
    m_Theta     = 0.0;
    NCrit      = 0.0;
    Cd         = 0.0;
    Cdp        = 0.0;
    Cl         = 0.0;
    Cm         = 0.0;

    XTrTop      = 1.0;
    XTrBot      = 1.0;
    XLamSepTop  = 1.0;
    XLamSepBot  = 1.0;
    XTurbSepTop = 1.0;
    XTurbSepBot = 1.0;


    XForce = 0.0;
    YForce = 0.0;
    Cpmn   = 0.0;
    XCP  = 0.0;
    m_LEHMom   = 0.0; m_TEHMom = 0.0;


    m_theStyle.m_bIsVisible = true;
    m_theStyle.m_Symbol = Line::NOSYMBOL;
    m_theStyle.m_Stipple = Line::SOLID;
    m_theStyle.m_Width = 1;
    int h = QRandomGenerator::global()->bounded(360);
    int s = QRandomGenerator::global()->bounded(155)+100;
    int v = QRandomGenerator::global()->bounded(155)+100;
    m_theStyle.m_Color.setHsv(h,s,v,255);
}


void OpPoint::duplicate(OpPoint const &opp)
{
    m_FoilName = opp.m_FoilName;
    m_PlrName  = opp.m_PlrName;
    m_theStyle = opp.theStyle();

    m_BLMethod = opp.m_BLMethod;
    m_bViscResults = opp.m_bViscResults;
    m_bBL          = opp.m_bBL;
    m_bTEFlap      = opp.m_bTEFlap;
    m_bLEFlap      = opp.m_bLEFlap;

    m_Alpha    = opp.m_Alpha;
    m_Reynolds = opp.m_Reynolds;
    m_Mach     = opp.m_Mach;
    m_Theta    = opp.m_Theta;
    NCrit      = opp.NCrit;
    Cd         = opp.Cd;
    Cdp        = opp.Cdp;
    Cl         = opp.Cl;
    Cm         = opp.Cm;

    XTrTop      = opp.XTrTop;
    XTrBot      = opp.XTrBot;
    XLamSepTop  = opp.XLamSepTop;
    XLamSepBot  = opp.XLamSepBot;
    XTurbSepTop = opp.XTurbSepTop;
    XTurbSepBot = opp.XTurbSepBot;


    XForce = opp.XForce;
    YForce = opp.YForce;
    Cpmn   = opp.Cpmn;
    XCP  = opp.XCP;
    m_LEHMom = opp.m_LEHMom;
    m_TEHMom = opp.m_TEHMom;

    Cpi = opp.Cpi;
    Cpv = opp.Cpv;
    Qi = opp.Qi;
    Qv = opp.Qv;
}


/**
 * Calculates the moments acting on the flap hinges
 * @param pOpPoint
 */
void OpPoint::setHingeMoments(Foil const*pFoil)
{
    double dx(0), dy(0), xmid(0), ymid(0), pmid(0);
    double xof = pFoil->TEXHinge();
    double ymin = pFoil->baseLowerY(xof);
    double ymax = pFoil->baseUpperY(xof);
    double yof = ymin + (ymax-ymin) * pFoil->TEYHinge();

    if(pFoil->hasTEFlap())
    {
        double hmom = 0.0;
        double hfx  = 0.0;
        double hfy  = 0.0;

        //---- integrate pressures on top and bottom sides of flap
        for (int i=0;i<pFoil->nNodes()-1;i++)
        {
            if (pFoil->x(i)>xof &&    pFoil->x(i+1)>xof)
            {
                dx = pFoil->x(i+1) - pFoil->x(i);
                dy = pFoil->y(i+1) - pFoil->y(i);
                xmid = 0.5*(pFoil->x(i+1)+pFoil->x(i)) - xof;
                ymid = 0.5*(pFoil->y(i+1)+pFoil->y(i)) - yof;

                if(m_bViscResults) pmid = 0.5*(Cpv.at(i+1) + Cpv.at(i));
                else               pmid = 0.5*(Cpi.at(i+1) + Cpi.at(i));

                hmom += pmid * (xmid*dx + ymid*dy);
                hfx  -= pmid * dy;
                hfy  += pmid * dx;
            }
        }


        //store the results
        m_TEHMom = hmom;
        XForce   = hfx;
        YForce   = hfy;
    }
}


void OpPoint::exportOpp(QTextStream &out, const QString &Version, bool bCSV, QString const &textseparator) const
{
    QString strong;
    QString line, sep;

    out << Version+"\n";

    strong = m_FoilName + "\n";
    out<< strong;
    strong = m_PlrName + "\n";
    out<< strong;

    if(bCSV) sep = textseparator;
    else     sep = " ";

    strong = QString::asprintf("Alpha=%.3f", m_Alpha);
    line +=  strong + sep + " ";
    strong = QString::asprintf("Re=%.0f", m_Reynolds);
    line +=  strong + sep + " ";
    strong = QString::asprintf("Ma=%.3f", m_Mach);
    line +=  strong + sep + " ";
    strong = QString::asprintf("NCrit=%.3f", NCrit);
    line +=  strong;
    out << line << "\n";
}


QString OpPoint::name() const
{
    QString name = m_FoilName + QString::asprintf("-Re=%g-", m_Reynolds) + ALPHACHAR +  QString::asprintf("=%.2f", m_Alpha) + DEGCHAR;
    return name;
}


bool OpPoint::serializeOppXFL(QDataStream &ar, bool bIsStoring, int ArchiveFormat)
{
    bool boolean(false);
    int k(0);
    float f0(0), f1(0);
    double dble(0);

    if(bIsStoring)
    {
    }
    else
    {
        ar >> ArchiveFormat;
        //write variables
        ar >> m_FoilName;
        ar >> m_PlrName;

        if(ArchiveFormat<200005)
        {
            ar >> k;
            m_theStyle.setStipple(k);
            ar >> m_theStyle.m_Width;
            m_theStyle.m_Color = xfl::readQColor(ar);
            ar >> m_theStyle.m_bIsVisible >> boolean;
        }
        else
            m_theStyle.serializeXfl(ar, bIsStoring);

        int m_n(0);
        ar >> m_Reynolds >> m_Mach >> m_Alpha;
        ar >> m_n >> m_BLXFoil.nd1 >> m_BLXFoil.nd2 >> m_BLXFoil.nd3;

        ar >> m_bViscResults;
        ar >> m_bBL;

        ar >> Cl >> Cm >> Cd >> Cdp;
        ar >> XTrTop >> XTrBot >> XCP;
        ar >> NCrit >> m_TEHMom >> Cpmn;

        Cpv.resize(m_n);
        Cpi.resize(m_n);
        Qv.resize(m_n);
        Qi.resize(m_n);
        for (k=0; k<m_n; k++)
        {
            ar >> f0 >> f1;
            Cpv[k] = f0;
            Cpi[k] = f1;
        }
        for (k=0; k<m_n; k++)
        {
            ar >> f0 >> f1;
            Qv[k] = double(f0);
            Qi[k] = double(f1);
        }
        for (k=0; k<=m_BLXFoil.nd1; k++)
        {
            ar >> f0 >> f1;
            m_BLXFoil.xd1[k] = double(f0);
            m_BLXFoil.yd1[k] = double(f1);
        }
        for (k=0; k<m_BLXFoil.nd2; k++)
        {
            ar >> f0 >> f1;
            m_BLXFoil.xd2[k] = double(f0);
            m_BLXFoil.yd2[k] = double(f1);
        }
        for (k=0; k<m_BLXFoil.nd3; k++)
        {
            ar >> f0 >> f1;
            m_BLXFoil.xd3[k] = double(f0);
            m_BLXFoil.yd3[k] = double(f1);
        }

        // space allocation
        for (int i=0; i<20; i++) ar >> k;
        for (int i=0; i<50; i++) ar >> dble;
    }
    return true;
}


bool OpPoint::serializeOppFl5(QDataStream &ar, bool bIsStoring)
{
    double dble=0.0;
    int nIntSpares=0;
    int nDbleSpares=0;

    int n(0), k(0);
    float f0(0), f1(0);
    bool boolean(false);

    // 500001: first fl5 format
    // 500002: restored Cpv and Cpi
    // 500003: added surface nodes
    // 500004: restored BLXFoil save
    // 500750: v750, added theta
    int ArchiveFormat = 500750;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        //write variables
        ar << foilName();
        ar << polarName();

        m_theStyle.serializeFl5(ar, bIsStoring);

        switch(m_BLMethod)
        {
            default:
            case BL::XFOIL:         n=0;  break;
        }
        ar << n;

        ar << m_Reynolds << m_Mach << m_Alpha;

        ar << m_Theta;

        ar << m_bViscResults;
        ar << m_bBL;

        ar << Cl << Cm << Cd << Cdp;
        ar << XTrTop << XTrBot << XCP;
        ar << NCrit << m_TEHMom << Cpmn;

        n = 0;
        ar << n; //int(m_Node.size());
/*        for(int l=0; l<m_Node.size(); l++)
        {
            Node2d const &n2d = m_Node.at(l);
            ar << n2d.index() << n2d.isWakeNode() <<n2d.xf() << n2d.yf() << n2d.normal().xf() << n2d.normal().yf();
        }*/

        ar << int(Qi.size());
        for (int l=0; l<Qi.size(); l++)     ar << float(Cpv[l]) << float(Cpi[l]);
        for (int l=0; l<Qi.size(); l++)     ar << float(Qv[l])  << float(Qi[l]);

        if(m_BLMethod==BL::XFOIL)
        {
            m_BLXFoil.serialize(ar, bIsStoring);
        }
        else
        {
        }

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500000 || ArchiveFormat>550000) return false;

        ar >> m_FoilName;
        ar >> m_PlrName;

        m_theStyle.serializeFl5(ar, bIsStoring);


        ar >>n;
        switch(n)
        {
            default:
            case 0: m_BLMethod=BL::XFOIL;         break;
        }

        ar >> m_Reynolds >> m_Mach >> m_Alpha;

        if(ArchiveFormat>=500750)
            ar >> m_Theta;

        ar >> m_bViscResults;
        ar >> m_bBL;

        ar >> Cl >> Cm >> Cd >> Cdp;
        ar >> XTrTop >> XTrBot >> XCP;
        ar >> NCrit >> m_TEHMom >> Cpmn;

        if(ArchiveFormat>=500003)
        {
            ar >> n;
//            m_Node.resize(n);
            for(int l=0; l<n; l++)
            {
//                Node2d &n2d = m_Node[l];
                ar >> k;        //  n2d.setIndex(k);
                ar >> boolean;  //  n2d.setWakeNode(boolean);
                ar >> f0 >> f1; //  n2d.set(f0, f1);
                ar >> f0 >> f1; //  n2d.setNormal(f0, f1);
            }
        }

        ar >> n;
        resizeSurfacePoints(n);
        if(ArchiveFormat>=500002)
        {
            for (int l=0; l<n; l++)
            {
                ar >> f0 >> f1;
                Cpv[l] = double(f0);
                Cpi[l] = double(f1);
            }
        }

        for (int l=0; l<n; l++)
        {
            ar >> f0 >> f1;
            Qv[l] = double(f0);
            Qi[l] = double(f1);
        }

        if(ArchiveFormat>=500004)
        {
            if(m_BLMethod==BL::XFOIL)
            {
                m_BLXFoil.serialize(ar, bIsStoring);
            }
            else
            {
            }
        }
        else
        {
            BLData bl;
            bl.serializeFl5(ar, bIsStoring); // top
            bl.serializeFl5(ar, bIsStoring); // bot
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}


void OpPoint::getOppProperties(QString &OpPointProperties, QString const & textseparator, bool bData) const
{
    QString strong;
    OpPointProperties.clear();

    OpPointProperties += THETACHAR + QString::asprintf("     = %g", m_Theta) + DEGCHAR +"\n";
    OpPointProperties += QString::asprintf("Re    = %g\n", m_Reynolds);
    OpPointProperties += ALPHACHAR + QString::asprintf("     = %g", m_Alpha) + DEGCHAR +"\n";
    OpPointProperties += QString::asprintf("Mach  = %g\n", m_Mach);
    OpPointProperties += QString::asprintf("NCrit = %g\n", NCrit);
    OpPointProperties += QString::asprintf("Cl    = %11.5f\n", Cl);
    OpPointProperties += QString::asprintf("Cd    = %11.5f\n", Cd);
    OpPointProperties += QString::asprintf("Cl/Cd = %11.5f\n", Cl/Cd);
    OpPointProperties += QString::asprintf("Cm    = %11.5f\n", Cm);
    OpPointProperties += QString::asprintf("Cdp   = %11.5f\n", Cdp);
    OpPointProperties += QString::asprintf("Cpmn  = %11.5f\n", Cpmn);
    OpPointProperties += QString::asprintf("XCP   = %11.5f\n", XCP);

    OpPointProperties += "\n";
    OpPointProperties += "Transition locations:\n";
    strong += QString::asprintf("   Top side     = %11.5f\n", XTrTop);
    strong += QString::asprintf("   Bottom side  = %11.5f\n", XTrBot);
    OpPointProperties += strong + "\n";

    OpPointProperties += "\n";
    if(m_bTEFlap)
    {
        OpPointProperties += QString::asprintf("T.E. flap moment = %g\n", m_TEHMom);
    }
    if(m_bLEFlap)
    {
        OpPointProperties += QString::asprintf("L.E. flap moment = %g\n", m_LEHMom);
    }

    if(!bData) return;
    QTextStream out;
    strong.clear();
    out.setString(&strong);
    exportOpp(out, "", false, textseparator);
    OpPointProperties += "\n"+strong;
}


bool OpPoint::isFoilOpp(Foil const *pFoil) const
{
    return (foilName().compare(pFoil->name())==0);
}


bool OpPoint::isPolarOpp(Polar const *pPolar) const
{
    return (foilName().compare(pPolar->foilName())==0) && (polarName().compare(pPolar->name())==0);
}


void OpPoint::resizeSurfacePoints(int N)
{
    Cpi.resize(N);
    Cpv.resize(N);
    Qi.resize(N);
    Qv.resize(N);
    Cpi.fill(0.0);
    Cpv.fill(0.0);
    Qi.fill(0.0);
    Qv.fill(0.0);
}

