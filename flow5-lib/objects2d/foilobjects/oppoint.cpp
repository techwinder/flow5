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

#include <format>
#include <sstream>
#include <iomanip>

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
    std::string outstring;
    std::string strong;
    std::string line, sep;

    outstring = Version+EOLstr;

    strong = m_FoilName + EOLstr;
    outstring += strong;
    strong = m_PlrName + EOLstr;
    outstring += strong;

    if(bCSV) sep = textseparator;
    else     sep = " ";

    strong = std::format("Alpha{:.3f}", m_Alpha);
    line +=  strong + sep + " ";
    strong = std::format("Re={:.0f}", m_Reynolds);
    line +=  strong + sep + " ";
    strong = std::format("Ma{:.3f}", m_Mach);
    line +=  strong + sep + " ";
    strong = std::format("NCrit{:.3f}", m_NCrit);
    line +=  strong;
    outstring += line + EOLstr;

    out = outstring;
}


std::string OpPoint::fullName() const
{
    std::string name = m_FoilName + std::format("-Re={:g}-", m_Reynolds) + ALPHAstr +  std::format("=%.2f", m_Alpha) + DEGstr;
    return name;
}


std::string OpPoint::name() const
{
    std::string strange;

    std::stringstream ss;

    if      (isType6())
    {
        strange = std::format("{:.3f}", m_Theta) +DEGstr;
        ss << std::right << std::setw(9) << strange;
    }
    else if (isType4())
    {
        strange = std::format("{:g}", m_Reynolds);
        ss << std::right << std::setw(9) << strange;
    }
    else
    {
        strange = std::format("{:.3f}", m_Alpha) + DEGstr;
        ss << std::right << std::setw(9) << strange;
    }
    return ss.str();
}


std::string OpPoint::properties(std::string const & textseparator, bool bData) const
{
    std::string props;
    std::string strong;
    props.clear();

    props += THETAstr + std::format("     = {:g}", m_Theta) + DEGstr +EOLstr;
    props += std::format("Re    = {:g}\n",     m_Reynolds);
    props += ALPHAstr + std::format("     = {:g}", m_Alpha) + DEGstr +EOLstr;
    props += std::format("Mach  = {:g}\n",     m_Mach);
    props += std::format("NCrit = {:g}\n",     m_NCrit);
    props += std::format("Cl    = {:11.5f}\n", m_Cl);
    props += std::format("Cd    = {:11.5f}\n", m_Cd);
    props += std::format("Cl/Cd = {:11.5f}\n", m_Cl/m_Cd);
    props += std::format("Cm    = {:11.5f}\n", m_Cm);
    props += std::format("Cdp   = {:11.5f}\n", m_Cdp);
    props += std::format("Cpmn  = {:11.5f}\n", m_Cpmn);
    props += std::format("XCP   = {:11.5f}\n", m_XCP);

    props += EOLstr;
    props += "Transition locations:\n";
    strong += std::format("   Top side     = {:11.5f}\n", m_XTrTop);
    strong += std::format("   Bottom side  = {:11.5f}\n", m_XTrBot);
    props += strong + EOLstr;

    props += EOLstr;
    if(m_bTEFlap)
    {
        props += std::format("T.E. flap moment = {:g}\n", m_TEHMom);
    }
    if(m_bLEFlap)
    {
        props += std::format("L.E. flap moment = {:g}\n", m_m_LEHMom);
    }

    if(bData)
    {
        std::string str;
        exportOpp(str, "", false, textseparator);
        props += EOLstr+ str;
    }

    return props;
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

