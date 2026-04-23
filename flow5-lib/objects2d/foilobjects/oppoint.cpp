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

#include <QString>


#include <oppoint.h>

#include <foil.h>
#include <polar.h>
#include <utils.h>


OpPoint::OpPoint() : XflObject()
{
    m_PolarType = xfl::T1POLAR;

    m_BLMethod = BL::XFOIL;
    m_bViscResults = false;//not a  viscous point a priori
    m_bBL          = false;// no boundary layer surface either
    m_bTEFlap      = false;
    m_bLEFlap      = false;

    m_Alpha    = 0.0;
    m_Reynolds = 0.0;
    m_Mach     = 0.0;
    m_Theta     = 0.0;
    m_NCrit      = 0.0;
    m_Cd         = 0.0;
    m_Cdp        = 0.0;
    m_Cl         = 0.0;
    m_Cm         = 0.0;

    m_XTrTop      = 1.0;
    m_XTrBot      = 1.0;
    m_XLamSepTop  = 1.0;
    m_XLamSepBot  = 1.0;
    m_XTurbSepTop = 1.0;
    m_XTurbSepBot = 1.0;


    m_XForce = 0.0;
    m_YForce = 0.0;
    m_Cpmn   = 0.0;
    m_XCP  = 0.0;
    m_m_LEHMom   = 0.0; m_TEHMom = 0.0;


    m_theStyle.m_bIsVisible = true;
    m_theStyle.m_Symbol = Line::NOSYMBOL;
    m_theStyle.m_Stipple = Line::SOLID;
    m_theStyle.m_Width = 1;
    m_theStyle.m_Color = xfl::Bisque;
}


void OpPoint::duplicate(OpPoint const &opp)
{
    m_PolarType    = opp.m_PolarType;
    m_FoilName     = opp.m_FoilName;
    m_PlrName      = opp.m_PlrName;
    m_theStyle     = opp.theStyle();

    m_BLMethod     = opp.m_BLMethod;
    m_bViscResults = opp.m_bViscResults;
    m_bBL          = opp.m_bBL;
    m_bTEFlap      = opp.m_bTEFlap;
    m_bLEFlap      = opp.m_bLEFlap;

    m_Alpha        = opp.m_Alpha;
    m_Reynolds     = opp.m_Reynolds;
    m_Mach         = opp.m_Mach;
    m_Theta        = opp.m_Theta;
    m_NCrit        = opp.m_NCrit;
    m_Cd           = opp.m_Cd;
    m_Cdp          = opp.m_Cdp;
    m_Cl           = opp.m_Cl;
    m_Cm           = opp.m_Cm;

    m_XTrTop       = opp.m_XTrTop;
    m_XTrBot       = opp.m_XTrBot;
    m_XLamSepTop   = opp.m_XLamSepTop;
    m_XLamSepBot   = opp.m_XLamSepBot;
    m_XTurbSepTop  = opp.m_XTurbSepTop;
    m_XTurbSepBot  = opp.m_XTurbSepBot;


    m_XForce       = opp.m_XForce;
    m_YForce       = opp.m_YForce;
    m_Cpmn         = opp.m_Cpmn;
    m_XCP          = opp.m_XCP;
    m_m_LEHMom     = opp.m_m_LEHMom;
    m_TEHMom       = opp.m_TEHMom;

    m_Cpi          = opp.m_Cpi;
    m_Cpv          = opp.m_Cpv;
    m_Qi           = opp.m_Qi;
    m_Qv           = opp.m_Qv;
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

                if(m_bViscResults) pmid = 0.5*(m_Cpv.at(i+1) + m_Cpv.at(i));
                else               pmid = 0.5*(m_Cpi.at(i+1) + m_Cpi.at(i));

                hmom += pmid * (xmid*dx + ymid*dy);
                hfx  -= pmid * dy;
                hfy  += pmid * dx;
            }
        }


        //store the results
        m_TEHMom = hmom;
        m_XForce   = hfx;
        m_YForce   = hfy;
    }
}


void OpPoint::exportOpp(std::string &out, const std::string &Version, bool bCSV, std::string const &textseparator) const
{
    QString outstring;
    QString strong;
    QString line, sep;

    outstring = QString::fromStdString(Version)+"\n";

    strong = QString::fromStdString(m_FoilName) + "\n";
    outstring += strong;
    strong = QString::fromStdString(m_PlrName) + "\n";
    outstring += strong;

    if(bCSV) sep = QString::fromStdString(textseparator);
    else     sep = " ";

    strong = QString::asprintf("Alpha%.3f", m_Alpha);
    line +=  strong + sep + " ";
    strong = QString::asprintf("Re=%.0f", m_Reynolds);
    line +=  strong + sep + " ";
    strong = QString::asprintf("Ma%.3f", m_Mach);
    line +=  strong + sep + " ";
    strong = QString::asprintf("NCrit%.3f", m_NCrit);
    line +=  strong;
    outstring += line + "\n";

    out = outstring.toStdString();
}


std::string OpPoint::fullName() const
{
    QString name = QString::fromStdString(m_FoilName) + QString::asprintf("-Re=%g-", m_Reynolds) + ALPHAch +  QString::asprintf("=%.2f", m_Alpha) + DEGch;
    return name.toStdString();
}


std::string OpPoint::name() const
{
    QString strange;

    if      (isType6())
    {
        strange = QString::asprintf("%.3f", m_Theta) +DEGch;
        strange = strange.rightJustified(9);
    }
    else if (isType4())
    {
        strange = QString::asprintf("%g", m_Reynolds);
        strange = strange.rightJustified(9);
    }
    else
    {
        strange = QString::asprintf("%.3f", m_Alpha) + DEGch;
        strange = strange.rightJustified(9);
    }
    return strange.toStdString();
}


bool OpPoint::serializeOppXFL(QDataStream &ar, bool bIsStoring, int ArchiveFormat)
{
    bool boolean(false);
    int k(0);
    float f0(0), f1(0);
    double dble(0);

    QString strange;

    if(bIsStoring)
    {
    }
    else
    {
        ar >> ArchiveFormat;
        //write variables
        ar >> strange;  m_FoilName = strange.toStdString();
        ar >> strange;  m_PlrName  = strange.toStdString();

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

        ar >> m_Cl >> m_Cm >> m_Cd >> m_Cdp;
        ar >> m_XTrTop >> m_XTrBot >> m_XCP;
        ar >> m_NCrit >> m_TEHMom >> m_Cpmn;

        m_Cpv.resize(m_n);
        m_Cpi.resize(m_n);
        m_Qv.resize(m_n);
        m_Qi.resize(m_n);
        for (k=0; k<m_n; k++)
        {
            ar >> f0 >> f1;
            m_Cpv[k] = f0;
            m_Cpi[k] = f1;
        }
        for (k=0; k<m_n; k++)
        {
            ar >> f0 >> f1;
            m_Qv[k] = double(f0);
            m_Qi[k] = double(f1);
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

    QString strange;

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
        ar << QString::fromStdString(m_FoilName);
        ar << QString::fromStdString(m_PlrName);

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

        ar << m_Cl << m_Cm << m_Cd << m_Cdp;
        ar << m_XTrTop << m_XTrBot << m_XCP;
        ar << m_NCrit << m_TEHMom << m_Cpmn;

        n = 0;
        ar << n; //int(m_Node.size());
/*        for(int l=0; l<m_Node.size(); l++)
        {
            Node2d const &n2d = m_Node.at(l);
            ar << n2d.index() << n2d.isWakeNode() <<n2d.xf() << n2d.yf() << n2d.normal().xf() << n2d.normal().yf();
        }*/

        ar << int(m_Qi.size());
        for (uint l=0; l<m_Qi.size(); l++)     ar << float(m_Cpv[l]) << float(m_Cpi[l]);
        for (uint l=0; l<m_Qi.size(); l++)     ar << float(m_Qv[l])  << float(m_Qi[l]);

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

        ar >> strange; m_FoilName = strange.toStdString();
        ar >> strange; m_PlrName = strange.toStdString();;

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

        ar >> m_Cl >> m_Cm >> m_Cd >> m_Cdp;
        ar >> m_XTrTop >> m_XTrBot >> m_XCP;
        ar >> m_NCrit >> m_TEHMom >> m_Cpmn;

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
                m_Cpv[l] = double(f0);
                m_Cpi[l] = double(f1);
            }
        }

        for (int l=0; l<n; l++)
        {
            ar >> f0 >> f1;
            m_Qv[l] = double(f0);
            m_Qi[l] = double(f1);
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


std::string OpPoint::properties(std::string const & textseparator, bool bData) const
{
    QString props;
    QString strong;
    props.clear();

    props += THETAch + QString::asprintf("     = %g", m_Theta) + DEGch +"\n";
    props += QString::asprintf("Re    = %g\n",     m_Reynolds);
    props += ALPHAch + QString::asprintf("     = %g", m_Alpha) + DEGch +"\n";
    props += QString::asprintf("Mach  = %g\n",     m_Mach);
    props += QString::asprintf("NCrit = %g\n",     m_NCrit);
    props += QString::asprintf("Cl    = %11.5f\n", m_Cl);
    props += QString::asprintf("Cd    = %11.5f\n", m_Cd);
    props += QString::asprintf("Cl/Cd = %11.5f\n", m_Cl/m_Cd);
    props += QString::asprintf("Cm    = %11.5f\n", m_Cm);
    props += QString::asprintf("Cdp   = %11.5f\n", m_Cdp);
    props += QString::asprintf("Cpmn  = %11.5f\n", m_Cpmn);
    props += QString::asprintf("XCP   = %11.5f\n", m_XCP);

    props += "\n";
    props += "Transition locations:\n";
    strong += QString::asprintf("   Top side     = %11.5f\n", m_XTrTop);
    strong += QString::asprintf("   Bottom side  = %11.5f\n", m_XTrBot);
    props += strong + "\n";

    props += "\n";
    if(m_bTEFlap)
    {
        props += QString::asprintf("T.E. flap moment = %g\n", m_TEHMom);
    }
    if(m_bLEFlap)
    {
        props += QString::asprintf("L.E. flap moment = %g\n", m_m_LEHMom);
    }

    if(bData)
    {
        std::string str;
        exportOpp(str, "", false, textseparator);
        props += "\n"+ QString::fromStdString(str);
    }

    return props.toStdString();
}


bool OpPoint::isFoilOpp(Foil const *pFoil) const
{
    return (foilName().compare(pFoil->name())==0);
}


bool OpPoint::isPolarOpp(Polar const *pPolar) const
{
    return (m_FoilName.compare(pPolar->foilName())==0) && (m_PlrName.compare(pPolar->name())==0);
}


void OpPoint::resizeSurfacePoints(int N)
{
    m_Cpi.resize(N);
    m_Cpv.resize(N);
    m_Qi.resize(N);
    m_Qv.resize(N);

    std::fill(m_Cpi.begin(), m_Cpi.end(), 0.0);
    std::fill(m_Cpv.begin(), m_Cpv.end(), 0.0);
    std::fill(m_Qi.begin(), m_Qi.end(), 0.0);
    std::fill(m_Qv.begin(), m_Qv.end(), 0.0);
}

