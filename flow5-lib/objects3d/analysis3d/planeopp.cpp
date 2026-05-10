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


#include <plane.h>
#include <planestl.h>
#include <planexfl.h>
#include <wingxfl.h>
#include <surface.h>

#include <surface.h>
#include <objects_global.h>

#include <planepolar.h>
#include <planeopp.h>
#include <wingopp.h>

#include <constants.h>
#include <mathelem.h>
#include <utils.h>
#include <units.h>

std::vector<std::string> PlaneOpp::s_POppVariables = { "Local lift coef.", "Local lift C.Cl/M.A.C.",
                                          "Airfoil viscous drag coef.","Induced drag coef.","Total drag coef.",
                                          "Local drag C.Cd/M.A.C.","1/4 chord pitching moment coef.",
                                          "CoG visc. pitching moment coef.","CoG pressure pitching moment coef.",
                                          "Reynolds","Top transition x%","Bot. transition x%",
                                          "Centre of pressure x%",
                                          "Strip force",
                                          "Bending moment",
                                          "aoa+ai+twist", "Effective aoa", "Induced angle",
                                          "Virtual twist", "Circulation"};



PlaneOpp::PlaneOpp() : Opp3d()
{
    initialize();
}


PlaneOpp::PlaneOpp(Plane const *pPlane, PlanePolar const *pWPolar, int panel4ArraySize, int panel3ArraySize) : Opp3d()
{
    initialize();

    if(pPlane)
    {
        m_PlaneName  = pPlane->name();
        m_MAChord    = pPlane->mac();
        m_Span       = pPlane->span();
    }

    if(pWPolar)
    {
        m_PlrName        = pWPolar->name();
        m_bThinSurface    = pWPolar->bThinSurfaces();
//        m_bTiltedGeom     = pWPolar->isTilted();
        m_PolarType      = pWPolar->type();
        m_AnalysisMethod  = pWPolar->analysisMethod();

        m_theStyle = pWPolar->theStyle();
    }

    memset(m_Is, 0, 9*sizeof(double));

    allocateMemory(panel4ArraySize, panel3ArraySize);
}


void PlaneOpp::setVariableNames()
{
    s_POppVariables = {"Local lift coef.",  "Local lift C.Cl/M.A.C.",
                       "Airfoil viscous drag coef.", "Induced drag coef.", "Total drag coef.",
                       "Local drag C.Cd/M.A.C.", "1/4 chord pitching moment coef.",
                       "CoG visc. pitching moment coef.", "CoG pressure pitching moment coef.",
                       "Reynolds", "Top transition x%", "Bot. transition x%",
                       "Centre of pressure x%",
                       "Strip lift (" +Units::forceUnitLabel()+")",
                       "Bending moment (" +Units::momentUnitLabel()+")",
                       "aoa+ai+twist",  "Effective aoa",  "Induced angle",
                       "Virtual twist",  "Circulation"};
}


void PlaneOpp::initialize()
{
    m_PlaneName   = "";
    m_PlrName    = "";

    m_nPanel4 = m_nPanel3 = 0;

    m_NodeValMin = m_NodeValMax = 0.0;

    m_PolarType     = xfl::T1POLAR;
    m_AnalysisMethod = xfl::VLM2;

    m_Mass = 0.0;
    m_CoG.set(0.0,0.0,0.0);
    m_Inertia[0] = m_Inertia[1] = m_Inertia[2] = m_Inertia[3] = 0.0;

    m_theStyle.m_Stipple     = Line::SOLID;
    m_theStyle.m_Width       = 1;
    m_theStyle.m_Symbol  = Line::NOSYMBOL;
    m_theStyle.m_bIsVisible  = true;


    m_theStyle.m_Color = xfl::BlueViolet;

    m_bThinSurface = true;

    m_Span = m_MAChord = 0.0;

    m_bOut        = false;

    m_Alpha       = 0.0;
    m_Beta        = 0.0;
    m_Phi         = 0.0;
    m_QInf        = 0.0;
    m_Ctrl        = 0.0;

    m_SD.reset();

    for(int i=0; i<8; i++)
    {
        m_EigenValue[i] = std::complex<double>(0.0,0.0);
        for(int j=0; j<4; j++)
            m_EigenVector[i][j] = std::complex<double>(0.0,0.0);
    }

    m_phiPH = std::complex<double>(0.0, 0.0);
    m_phiDR = std::complex<double>(0.0, 0.0);

    memset(m_ALong, 0, 16*sizeof(double));
    memset(m_ALat,  0, 16*sizeof(double));
    m_BLong.clear();
    m_BLat.clear();

    //    m_bWing[0] = true;
    m_WingOpp.clear();
}


void PlaneOpp::addWingOpp(int PanelArraySize)
{
    m_WingOpp.push_back({PanelArraySize});
}


/** Allocate memory for the arrays */
void PlaneOpp::allocateMemory(int panel4ArraySize, int panel3ArraySize)
{
    m_nPanel4 = panel4ArraySize;
    m_nPanel3 = panel3ArraySize;
    if(isTriangleMethod())
    {
        int N3 = 3*panel3ArraySize;
        m_gamma.resize(N3);
        m_Cp.resize(N3);
        m_sigma.resize(m_nPanel3);
    }
    else
    {
        m_gamma.resize(panel4ArraySize);
        m_Cp.resize(panel4ArraySize);
        m_sigma.resize(panel4ArraySize);
    }
    memset(m_gamma.data(),     0, unsigned(m_gamma.size())     * sizeof(double));
    memset(m_sigma.data(), 0, unsigned(m_sigma.size()) * sizeof(double));
    memset(m_Cp.data(),    0, unsigned(m_Cp.size())    * sizeof(double));
}


void PlaneOpp::getProperties(Plane const *pPlane, PlanePolar const *pWPolar, std::string &properties) const
{
    std::string props;
    std::string strong, strange;
    std::string lenlab = Units::lengthUnitLabel();

    Vector3d WindD = objects::windDirection(alpha(), beta());
//    Vector3d WindN = windNormal(alpha(), beta());

    props.clear();

    if     (isType1()) strong += "Type 1 (Fixed speed)\n";
    else if(isType2()) strong += "Type 2 (Fixed lift)\n";
    else if(isType3()) strong += "Type 3 (Speed polar)\n";
    else if(isType5()) strong += "Type 5 (Beta polar)\n";
    else if(isType6()) strong += "Type 6 (Control analysis)\n";
    else if(isType7()) strong += "Type 7 (Stability analysis)\n";
    else if(isType8()) strong += "Type 8\n";
    else               strong += "Type unknown\n";
    props += strong;

    if     (isLLTMethod())          props += "LLT";
    else if(isPanel4Method())       props += "Quads";
    else if(isVLM1())               props += "VLM1";
    else if(isVLM2())               props += "VLM2";
    else if(isTriUniformMethod())   props += "Triangles - Uniform doublet density ";
    else if(isTriLinearMethod())    props += "Triangles - linear doublet density";
    props +="\n\n";

    if(m_bOut) props += "Point is out of the flight envelope\n";

    strong = "Mass  = "+ std::format(" {:9.3f} ", m_Mass*Units::kgtoUnit());
    props += strong + Units::massUnitLabel() + EOLstr;

    strong = "CoG_x = "+ std::format(" {:9.3f} ", m_CoG.x*Units::mtoUnit());
    strong += lenlab + EOLstr;
    props += strong;

    strong = "CoG_z = "+ std::format(" {:9.3f} ", m_CoG.z*Units::mtoUnit());
    strong += lenlab + EOLstr;
    props += strong + EOLstr;

    strong = "V" + INFstr + "    = "+ std::format(" {:9.3f} ", m_QInf*Units::mstoUnit());
    props += strong + Units::speedUnitLabel()+"\n";

    strong = ALPHAstr + "     = "+ std::format(" {:9.3f}", m_Alpha);
    props += strong +  DEGstr +"\n";

    if(fabs(m_Beta)>ANGLEPRECISION)
    {
        strong = "Beta  = "+ std::format(" {:9.3f}", m_Beta);
        props += strong + DEGstr +"\n";
    }
    props += "\n";

    if(isType6() || isType7())
    {
        strong = "Ctrl  = " +  std::format(" {:9.3f}", m_Ctrl);
        props += strong +"\n";

    }
    if(isType7())
    {
        strong  = "XNP          = "+ std::format(" {:9.3f}", m_SD.XNP*Units::mtoUnit());
        props += "\n"+strong +" " + lenlab +"\n";

        strong = "Static margin = "+std::format(" {:9.3f}", (m_SD.XNP-m_CoG.x)/pWPolar->referenceChordLength()*100.0);
        props += strong + EOLstr;
    }

    strong = std::format("CP    = ({:.3g}; {:.3g}; {:.3g}) ",
                         m_AF.centreOfPressure().x*Units::mtoUnit(),
                         m_AF.centreOfPressure().y*Units::mtoUnit(),
                         m_AF.centreOfPressure().z*Units::mtoUnit());

    props += strong + lenlab + EOLstr + EOLstr;

    strong  = "CL  = " +  std::format(" {:13.7f}", m_AF.CL());
    props += strong +"\n";
    strong  = "CD  = " +  std::format(" {:13.7f}", m_AF.CD());
    props += strong +"\n";
    strong  = "VCD = " +  std::format(" {:13.7f}", m_AF.CDv());
    props += strong +"\n";
    strong  = "ICD = " +  std::format(" {:13.7f}", m_AF.CDi());
    props += strong +"\n";

    strong  = "CY  = " +  std::format(" {:13.7f}", m_AF.Cy());
    props += strong +"\n";

    strong  = "Cl  = " +  std::format(" {:13.7f}", m_AF.Cli());
    props += strong +"\n";

    strong  = "Cm  = " +  std::format(" {:13.7f}", m_AF.Cm());
    props += strong +"\n";
    strong  = "Cmi = " +  std::format(" {:13.7f}", m_AF.Cmi());
    props += strong +"\n";
    strong  = "Cmv = " +  std::format(" {:13.7f}", m_AF.Cmv());
    props += strong +"\n";

    strong  = "Cn  = " +  std::format(" {:13.7f}", m_AF.Cn());
    props += strong +"\n";
    strong  = "Cni = " +  std::format(" {:13.7f}", m_AF.Cni());
    props += strong +"\n";
    strong  = "Cnv = " +  std::format(" {:13.7f}", m_AF.Cnv());
    props += strong +"\n";

    props += "\n";

    if(pPlane && pPlane->isXflType() && pWPolar)
    {
        PlaneXfl const* pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
        double qDyn = 0.5*pWPolar->density()*m_QInf*m_QInf;

        props += "Total force, body axes:\n";
        Vector3d drag = WindD * m_AF.viscousDrag();

        double fx = m_AF.fffx() + drag.x;
        double mx = m_AF.Mi().x + m_AF.Mv().x;
        fx *= qDyn * Units::NtoUnit();
        mx *= qDyn * Units::NmtoUnit();
        strong = std::format("   Fx={:9.3g} {:s}  Mx ={:9.3g} {:s}", fx, Units::forceUnitLabel().c_str(), mx, Units::momentUnitLabel().c_str());
        props += strong + EOLstr;

        double fy = m_AF.fffy() + drag.y;
        double my = m_AF.Mi().y + m_AF.Mv().y;
        fy *= qDyn * Units::NtoUnit();
        my *= qDyn * Units::NmtoUnit();

        strong = std::format("   Fy={:9.3g} {:s}  My ={:9.3g} {:s}", fy, Units::forceUnitLabel().c_str(), my, Units::momentUnitLabel().c_str());
        props += strong + EOLstr;

        double fz = m_AF.fffz() + drag.z;
        double mz = m_AF.Mi().z + m_AF.Mv().z;
        fz *= qDyn * Units::NtoUnit();
        mz *= qDyn * Units::NmtoUnit();

        strong = std::format("   Fz={:9.3g} {:s}  Mz ={:9.3g} {:s}", fz, Units::forceUnitLabel().c_str(), mz, Units::momentUnitLabel().c_str());
        props += strong + "\n\n";

        props += "Forces on parts, body axes:\n";
        for(unsigned int iw=0; iw<m_WingOpp.size(); iw++)
        {
            WingXfl const *pWing = pPlaneXfl->wingAt(iw);
            WingOpp const*pWOpp = &m_WingOpp.at(iw);

            if(pWing && pWOpp)
            {
                Vector3d drag = WindD * pWOpp->m_AF.viscousDrag();
                props += "  " + pWing->name() + ":\n";
                double fx = pWOpp->m_AF.fffx() + drag.x;
                double mx = pWOpp->m_AF.Mi().x + pWOpp->m_AF.Mv().x;
                fx *= qDyn * Units::NtoUnit();
                mx *= qDyn * Units::NmtoUnit();
                strong = std::format("   Fx={:9.3g} {:s}  Mx ={:9.3g} {:s}", fx, Units::forceUnitLabel().c_str(), mx, Units::momentUnitLabel().c_str());
                props += strong + EOLstr;

                double fy = pWOpp->m_AF.fffy() + drag.y;
                double my = pWOpp->m_AF.Mi().y + pWOpp->m_AF.Mv().y;
                fy *= qDyn * Units::NtoUnit();
                my *= qDyn * Units::NmtoUnit();
                strong = std::format("   Fy={:9.3g} {:s}  My ={:9.3g} {:s}", fy, Units::forceUnitLabel().c_str(), my, Units::momentUnitLabel().c_str());
                props += strong + EOLstr;

                double fz = pWOpp->m_AF.fffz() + drag.z;
                double mz = pWOpp->m_AF.Mi().z + pWOpp->m_AF.Mv().z;
                fz *= qDyn * Units::NtoUnit();
                mz *= qDyn * Units::NmtoUnit();
                strong = std::format("   Fz={:9.3g} {:s}  Mz ={:9.3g} {:s}", fz, Units::forceUnitLabel().c_str(), mz, Units::momentUnitLabel().c_str());
                props += strong + EOLstr;
            }
        }

        for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++)
        {
            if(int(m_FuseAF.size())<=ifuse) break;  // FuseAF not defined if LLT

            Vector3d drag = WindD * m_FuseAF.at(ifuse).viscousDrag();
            Fuse const *pFuse = pPlaneXfl->fuseAt(ifuse);
            props += "  " + pFuse->name() + ":\n";
            double fx = m_FuseAF.at(ifuse).fffx() +drag.x;
            double mx = m_FuseAF.at(ifuse).Mi().x + m_FuseAF.at(ifuse).Mv().x;
            fx *= qDyn * Units::NtoUnit();
            mx *= qDyn * Units::NmtoUnit();
            strong = std::format("   Fx={:9.3g} {:s}  Mx ={:9.3g} {:s}", fx, Units::forceUnitLabel().c_str(), mx, Units::momentUnitLabel().c_str());
            props += strong + EOLstr;

            double fy = m_FuseAF.at(ifuse).fffx() + drag.y;
            double my = m_FuseAF.at(ifuse).Mi().y + m_FuseAF.at(ifuse).Mv().y;
            fy *= qDyn * Units::NtoUnit();
            my *= qDyn * Units::NmtoUnit();
            strong = std::format("   Fy={:9.3g} {:s}  My ={:9.3g} {:s}", fy, Units::forceUnitLabel().c_str(), my, Units::momentUnitLabel().c_str());
            props += strong + EOLstr;

            double fz = m_FuseAF.at(ifuse).fffx() + drag.z;
            double mz = m_FuseAF.at(ifuse).Mi().z + m_FuseAF.at(ifuse).Mv().z;
            fz *= qDyn * Units::NtoUnit();
            mz *= qDyn * Units::NmtoUnit();
            strong = std::format("   Fz={:9.3g} {:s}  Mz ={:9.3g} {:s}", fz, Units::forceUnitLabel().c_str(), mz, Units::momentUnitLabel().c_str());
            props += strong + EOLstr;
        }
    }

    bool bFlaps=0;
    for(unsigned int iw=0; iw<m_WingOpp.size(); iw++)
    {
        if(m_WingOpp.at(iw).m_FlapMoment.size())    bFlaps=1;
    }

    if(bFlaps)
    {
        props += "\nFlap Moments\n";

        for(unsigned int iwo=0; iwo<m_WingOpp.size(); iwo++)
        {
            props += "  " + WOpp(iwo).wingName() +"\n";
            for(int i=0; i<WOpp(iwo).m_nFlaps; i++)
            {
                strange = std::format("    Flap_{:d} = %8.4f ", i+1, WOpp(iwo).m_FlapMoment[i]*Units::NmtoUnit());
                props += strange + Units::momentUnitLabel() + EOLstr;
            }
        }
    }


    props += "\n";

    if(isType12358() || isType7())
    {
        props += "\n";
        props += "Non-dimensional stability derivatives:\n";
        props += std::format("  CXu = {:11g}\n", m_SD.CXu);
        props += std::format("  CZu = {:11g}\n", m_SD.CZu);
        props += std::format("  Cmu = {:11g}\n", m_SD.Cmu);
        props += std::format("  CXa = {:11g}\n", m_SD.CXa);
        props += std::format("  CZa = {:11g}\n", m_SD.CZa);
        props += std::format("  Cma = {:11g}\n", m_SD.Cma);
        props += std::format("  CXq = {:11g}\n", m_SD.CXq);
        props += std::format("  CZq = {:11g}\n", m_SD.CZq);
        props += std::format("  Cmq = {:11g}\n", m_SD.Cmq);
        props += std::format("  CYb = {:11g}\n", m_SD.CYb);
        props += std::format("  Clb = {:11g}\n", m_SD.Clb);
        props += std::format("  Cnb = {:11g}\n", m_SD.Cnb);
        props += std::format("  CYp = {:11g}\n", m_SD.CYp);
        props += std::format("  Clp = {:11g}\n", m_SD.Clp);
        props += std::format("  Cnp = {:11g}\n", m_SD.Cnp);
        props += std::format("  CYr = {:11g}\n", m_SD.CYr);
        props += std::format("  Clr = {:11g}\n", m_SD.Clr);
        props += std::format("  Cnr = {:11g}\n", m_SD.Cnr);
        props += "\n";

        if(m_SD.ControlNames.size())
        {
            props += "Non-dimensional control derivatives:\n";
            for(unsigned int i=0; i<m_SD.ControlNames.size(); i++)
            {
                props += "  " + m_SD.ControlNames.at(i) + EOLstr;
                props += std::format("    CXd = {:11g}\n", m_SD.CXe.at(i));
                props += std::format("    CYd = {:11g}\n", m_SD.CYe.at(i));
                props += std::format("    CZd = {:11g}\n", m_SD.CZe.at(i));
                props += std::format("    Cld = {:11g}\n", m_SD.CLe.at(i));
                props += std::format("    Cmd = {:11g}\n", m_SD.CMe.at(i));
                props += std::format("    Cnd = {:11g}\n", m_SD.CNe.at(i));
            }
        }

        std::complex<double> c(0,0), angle(0,0);
        double OmegaN(0), Omega1(0), Dsi(0);
        double u0   = m_QInf;
        double mac  = m_MAChord;
        double span = m_Span;


        props += "\nLongitudinal modes:\n";
        for(int im=0; im<4; im++)
        {
            c = m_EigenValue[im];
            objects::modeProperties(c, OmegaN, Omega1, Dsi);

            if(c.imag()>=0.0) strange = "  " + LAMBDAstr + std::format(" = {:9.4g} + {:9.4g}i", c.real(), c.imag());
            else              strange = "  " + LAMBDAstr + std::format(" = {:9.4g} - {:9.4g}i", c.real(), std::abs(c.imag()));
            props += strange +"\n";

            strange = std::format("  F (natural)  = {:9.3f} Hz", OmegaN/2.0/PI);
            props += strange +"\n";

            strange = std::format("  F (damped)   = {:9.3f} Hz", Omega1/2.0/PI);
            props += strange +"\n";

            strange = "  " + XIstr + std::format("            = {:9.3f} ", Dsi);
            props += strange +"\n";

            props += "  Normalized eigenvector:\n";
            angle = m_EigenVector[im][3];
            c = m_EigenVector[im][0]/u0;
            if(c.imag()>=0.0) strange = std::format("    u/u0          = {:9.4g} + {:9.4g}i", c.real(), c.imag());
            else              strange = std::format("    u/u0          = {:9.4g} - {:9.4g}", c.real(), std::abs(c.imag()));
            props += strange +"\n";

            c = m_EigenVector[im][1]/u0;
            if(c.imag()>=0.0) strange = std::format("    w/u0          = {:9.4g} + {:9.4g}i",c.real(),c.imag());
            else              strange = std::format("    w/u0          = {:9.4g} - {:9.4g}i",c.real(),std::abs(c.imag()));
            props += strange +"\n";

            c = m_EigenVector[im][2]/(2.0*u0/mac);
            if(c.imag()>=0.0) strange = std::format("    q/(2.u0.MAC)  = {:9.4g} + {:9.4g}i", c.real(), c.imag());
            else              strange = std::format("    q/(2.u0.MAC)  = {:9.4g} - {:9.4g}i", c.real(), std::abs(c.imag()));
            props += strange +"\n";

            c = m_EigenVector[im][3]/angle;
            if(c.imag()>=0.0) strange = "    " + THETAstr + std::format(" (rad)       = {:9.4g} + {:9.4g}i", c.real(), c.imag());
            else              strange = "    " + THETAstr + std::format(" (rad)       = {:9.4g} - {:9.4g}i", c.real(), std::abs(c.imag()));
            props += strange +"\n\n";
        }

        props += "\nLateral modes:\n";
        for(int im=4; im<8; im++)
        {
            c = m_EigenValue[im];
            objects::modeProperties(c, OmegaN, Omega1, Dsi);

            if(c.imag()>=0.0) strange = "  " + LAMBDAstr + std::format(" = {:9.4g} + {:9.4g}i", c.real(), c.imag());
            else              strange = "  " + LAMBDAstr + std::format(" = {:9.4g} - {:9.4g}i", c.real(), std::abs(c.imag()));
            props += strange +"\n";


            strange = std::format("  F (natural)  = {:9.3f} Hz", OmegaN/2.0/PI);
            props += strange +"\n";

            strange = std::format("  F (damped)   = {:9.3f} Hz", Omega1/2.0/PI);
            props += strange +"\n";

            strange = "  " + XIstr + std::format("            = {:9.3f} ", Dsi);
            props += strange +"\n";

            if(fabs(c.real())>PRECISION && fabs(c.imag())<PRECISION)
            {
                strange = std::format(    "  Time to double = {:9.3f} s", log(2)/fabs(c.real()));
                props += strange +"\n";
                if(c.real()<0.0)
                {
                    strange = std::format("  Time constant  = {:9.3f}", -1.0/c.real());
                    props += strange +"\n";
                }
            }

            props += "  Normalized Eigenvector:\n";

            angle = m_EigenVector[im][3];

            c = m_EigenVector[im][0]/u0;
            if(c.imag()>=0.0) strange = std::format("    v/u0          = {:9.4g} + {:9.4g}i", c.real(), c.imag());
            else              strange = std::format("    v/u0          = {:9.4g} - {:9.4g}",  c.real(), std::abs(c.imag()));
            props += strange +"\n";

            c = m_EigenVector[im][1]/(2.0*u0/span);
            if(c.imag()>=0.0) strange = std::format("    p/(2.u0.Span) = {:9.4g} + {:9.4g}i", c.real(), c.imag());
            else              strange = std::format("    p/(2.u0.Span) = {:9.4g} - {:9.4g}",  c.real(), std::abs(c.imag()));
            props += strange +"\n";

            c = m_EigenVector[im][2]/(2.0*u0/span);
            if(c.imag()>=0.0) strange = std::format("    r/(2.u0.Span) = {:9.4g} + {:9.4g}i", c.real(), c.imag());
            else              strange = std::format("    r/(2.u0.Span) = {:9.4g} - {:9.4g}",  c.real(), std::abs(c.imag()));
            props += strange +"\n";

            c = m_EigenVector[im][3]/angle;
            if(c.imag()>=0.0) strange = "    " + PHIstr + std::format(" (rad)       = {:9.4g} + {:9.4g}i", c.real(), c.imag());
            else              strange = "    " + PHIstr + std::format(" (rad)       = {:9.4g} - {:9.4g}",  c.real(), std::abs(c.imag()));
            props += strange +"\n\n";
        }
    }

    if(isTriLinearMethod())
    {
        strange = std::format("Nodes values = {:d}", int(m_NodeValue.size()));
        props += strange;
    }
    else if(isTriUniformMethod())
    {
        strange = std::format("Panel values = {:d}", m_nPanel3);
        props += strange;
    }
    else if(isQuadMethod())
    {
        strange = std::format("Panel values = {:d}", m_nPanel4);
        props += strange;
    }

    if(m_Vorton.size())
    {
        strange = std::format("Vortons: {:d} rows x {:d} columns", int(m_Vorton.size()), int(m_Vorton.front().size()));
        props += "\n" + strange;
    }

    properties = props;
}


std::string PlaneOpp::name() const
{
    std::string strange;

    switch(m_PolarType)
    {
        case xfl::T8POLAR:
            strange  = std::format("%.2f", alpha()) + DEGstr + " ";
            strange += std::format("%.2f", beta()) + DEGstr + " ";
            strange += std::format("%.2f", QInf()*Units::mstoUnit()) + " " + Units::speedUnitLabel();
            break;
        case xfl::T7POLAR:
            strange = std::format("{:.3f}", alpha())  + DEGstr;
            break;
        case xfl::T6POLAR:
            strange = std::format("{:.3f}", ctrl());
            break;
        case xfl::T5POLAR:
            strange = std::format("{:.3f}", beta())  + DEGstr;
            break;
        case xfl::T4POLAR:
            strange = std::format("{:.3f}", QInf()*Units::mstoUnit()) + " " + Units::speedUnitLabel();
            break;
        case xfl::T1POLAR:
        case xfl::T2POLAR:
        case xfl::T3POLAR:
            strange = std::format("{:.3f}", alpha()) + DEGstr;
            break;
        default:
            strange.clear();
            break;

    }

    return strange;
}


std::string PlaneOpp::title(bool bLong) const
{
    std::string strange;

    if(bLong)
    {
        strange = planeName() + " / ";
        if     (isLLTMethod())         strange += "LLT";
        else if(isVLM1())              strange += "VLM1";
        else if(isVLM2())              strange += "VLM2";
        else if(isQuadMethod())        strange += "Quads";
        else if(isTriUniformMethod())  strange += "TriUni";
        else if(isTriLinearMethod())   strange += "TriLin";

        strange +=" / ";
    }

    if(isType7())  strange += std::format("ctrl={:g}-", ctrl());

    strange += std::format("{:5.2f}", m_Alpha) + DEGstr + "_";
    if(fabs(m_Beta)>ANGLEPRECISION)  strange += std::format("{:5.2f}", m_Beta) + DEGstr + "_";
    strange += std::format("{:5.2f}", QInf()*Units::mstoUnit()) + Units::speedUnitLabel();

    return strange;
}


std::string PlaneOpp::variableName(int iVar)
{
    if(iVar<0 || iVar>=int(s_POppVariables.size()))
        return s_POppVariables.at(0);
    else
        return s_POppVariables.at(iVar);
}


void PlaneOpp::computeStabilityInertia(double const*Inertia)
{
    double Ib[3][3], tR[3][3], tmp[3][3];
    double R[3][3];

    memset(Ib,  0, 9*sizeof(double));
    memset(R,   0, 9*sizeof(double));
    memset(tR,  0, 9*sizeof(double));
    memset(tmp, 0, 9*sizeof(double));

    R[0][0] = -cos(m_Alpha*PI/180.0);
    R[1][0] =  0.0;
    R[2][0] =  sin(m_Alpha*PI/180.0);
    R[0][1] =  0.0;
    R[1][1] =  1.0;
    R[2][1] =  0.0;
    R[0][2] = -sin(m_Alpha*PI/180.0);
    R[1][2] =  0.0;
    R[2][2] = -cos(m_Alpha*PI/180.0);

    tR[0][0] = R[0][0];
    tR[0][1] = R[1][0];
    tR[0][2] = R[2][0];
    tR[1][0] = R[0][1];
    tR[2][0] = R[0][2];
    tR[1][1] = R[1][1];
    tR[1][2] = R[2][1];
    tR[2][1] = R[1][2];
    tR[2][2] = R[2][2];

    // tmp = Ib.R

    Ib[0][0] = Inertia[0];
    Ib[1][1] = Inertia[1];
    Ib[2][2] = Inertia[2];
    Ib[0][2] = Ib[2][0] = Inertia[3];
    Ib[1][0] = Ib[1][2] = Ib[0][1] = Ib[2][1] = 0.0;

    for(int i=0; i<3; i++)
    {
        for(int j=0; j<3; j++)
        {
            tmp[i][j] = Ib[i][0]*R[0][j] + Ib[i][1]*R[1][j] + Ib[i][2]*R[2][j];
        }
    }

    // Is = tR.tmp
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<3; j++)
        {
            m_Is[i][j] = tR[i][0]*tmp[0][j] + tR[i][1]*tmp[1][j] + tR[i][2]*tmp[2][j];
        }
    }
}


/**
 * Creates the longitudinal and lateral state matrices
 * from the derivatives and inertias calculated previously

 * Creates the control state matrix from the control derivatives
*/
void PlaneOpp::buildStateMatrices(int nAVLCtrls)
{
    StabDerivatives const &SD = m_SD;

    double Theta0 = 0.0;

    //use inertia measured in stability axis, CoG origin
    double Ixx = m_Is[0][0];
    double Iyy = m_Is[1][1];
    double Izz = m_Is[2][2];
    double Izx = m_Is[0][2];

    //____________________Longitudinal stability_____________

    m_ALong[0][0] = SD.Xu/m_Mass;
    m_ALong[0][1] = SD.Xw/m_Mass;
    m_ALong[0][2] = 0.0;
    m_ALong[0][3] = -9.81*cos(Theta0*PI/180.0);

    m_ALong[1][0] =  SD.Zu                          / (m_Mass-SD.Zwp);
    m_ALong[1][1] =  SD.Zw                          / (m_Mass-SD.Zwp);
    m_ALong[1][2] = (SD.Zq+m_Mass*m_QInf)           / (m_Mass-SD.Zwp);
    m_ALong[1][3] = -9.81*m_Mass*sin(Theta0*PI/180.0) / (m_Mass-SD.Zwp);

    m_ALong[2][0] = (SD.Mu + SD.Mwp*SD.Zu/(m_Mass-SD.Zwp))                  /Iyy;
    m_ALong[2][1] = (SD.Mw + SD.Mwp*SD.Zw/(m_Mass-SD.Zwp))                  /Iyy;
    m_ALong[2][2] = (SD.Mq + SD.Mwp*(SD.Zq+m_Mass*m_QInf)/(m_Mass-SD.Zwp))  /Iyy;
    m_ALong[2][3] = (SD.Mwp*(-m_Mass*9.81*sin(Theta0))/(m_Mass-SD.Zwp))     /Iyy;

    m_ALong[3][0] = 0.0;
    m_ALong[3][1] = 0.0;
    m_ALong[3][2] = 1.0;
    m_ALong[3][3] = 0.0;


    //____________________Lateral stability_____________
    double Ipxx = (Ixx * Izz - Izx*Izx)/Izz;
    double Ipzz = (Ixx * Izz - Izx*Izx)/Ixx;
    double Ipzx = Izx/(Ixx * Izz - Izx*Izx);

    m_ALat[0][0] = SD.Yv/m_Mass;
    m_ALat[0][1] = SD.Yp/m_Mass;
    m_ALat[0][2] = SD.Yr/m_Mass - m_QInf;
    m_ALat[0][3] = 9.81 * cos(Theta0*PI/180.0);

    m_ALat[1][0] = SD.Lv/Ipxx+Ipzx*SD.Nv;
    m_ALat[1][1] = SD.Lp/Ipxx+Ipzx*SD.Np;
    m_ALat[1][2] = SD.Lr/Ipxx+Ipzx*SD.Nr;
    m_ALat[1][3] = 0.0;

    m_ALat[2][0] = SD.Lv*Ipzx+ SD.Nv/Ipzz;
    m_ALat[2][1] = SD.Lp*Ipzx+ SD.Np/Ipzz;
    m_ALat[2][2] = SD.Lr*Ipzx+ SD.Nr/Ipzz;
    m_ALat[2][3] = 0.0;

    m_ALat[3][0] = 0.0;
    m_ALat[3][1] = 1.0;
    m_ALat[3][2] = tan(Theta0*PI/180.0);
    m_ALat[3][3] = 0.0;


    //build the control matrix
    assert(nAVLCtrls==int(m_SD.Xde.size()));

    m_BLong.resize(nAVLCtrls);
    m_BLat.resize(nAVLCtrls);
    for(int i=0; i<nAVLCtrls; i++)
    {
        m_BLong[i].resize(4);
        m_BLat[i].resize(4);
    }

    for(unsigned int ie=0; ie<m_BLong.size(); ie++)
    {
        // per radian
        m_BLong[ie][0] = SD.Xde.at(ie)/m_Mass;
        m_BLong[ie][1] = SD.Zde.at(ie)/m_Mass;
        m_BLong[ie][2] = SD.Mde.at(ie)/Iyy;
        m_BLong[ie][3] = 0.0;

        m_BLat[ie][0] = SD.Yde.at(ie)/m_Mass;
        m_BLat[ie][1] = SD.Lde.at(ie)/Ipxx+SD.Nde.at(ie)*Ipzx;
        m_BLat[ie][2] = SD.Lde.at(ie)*Ipzx+SD.Nde.at(ie)/Ipzz;
        m_BLat[ie][3] = 0.0;
    }
}


bool PlaneOpp::solveEigenvalues(std::string &logmsg)
{
    std::string log;

    std::complex<double> rLong[4];
    std::complex<double> rLat[4];
    std::complex<double> vLong[16];
    std::complex<double> vLat[16];

    for(int i=0; i<4; i++)
    {
        rLong[i] = std::complex<double>(0.0,0.0);
        rLat[i]  = std::complex<double>(0.0,0.0);
    }
    for(int i=0; i<16; i++)
    {
        vLong[i] = std::complex<double>(0.0,0.0);
        vLat[i]  = std::complex<double>(0.0,0.0);
    }


    double pLong[]{0,0,0,0,0};
    double pLat[] {0,0,0,0,0};//the coefficients of the characteristic polynomial
    int i=0;


    characteristicPol(m_ALong, pLong);

    if(!LinBairstow(pLong, rLong, 4))
    {
        log += "       Error extracting longitudinal eigenvalues\n";
        return false;
    }

    //sort them
    sortComplex(rLong, 4);

    for(i=0; i<4; i++)
    {
        if(!matrix::eigenVector(m_ALong, rLong[i], vLong+i*4))
        {
            log += std::format("Error extracting longitudinal eigenvector for mode {:d}\n", i);
            return false;
        }
    }


    characteristicPol(m_ALat, pLat);

    if(!LinBairstow(pLat, rLat, 4))
    {
        log += "       Error extracting lateral eigenvalues\n";
        return false;
    }

    //sort them
    sortComplex(rLat, 4);

    for(i=0; i<4; i++)
    {
        if(!matrix::eigenVector(m_ALat, rLat[i], vLat+i*4))
        {
            log += std::format("Error extracting lateral eigenvector for mode {:d}\n", i);
            return false;
        }
    }

    for(int i=0; i<4; i++)
    {
        m_EigenValue[i]   = rLong[i];
        for(int l=0; l<4; l++)  m_EigenVector[i][l]   = vLong[4*i+l];

        m_EigenValue[i+4] = rLat[i];
        for(int l=0; l<4; l++)  m_EigenVector[i+4][l] = vLat[4*i+l];

    }

    logmsg = log;

    return true;
}


void PlaneOpp::outputEigen(std::string &logmsg)
{
    std::string log;

    std::string str;
    str = "      ___Longitudinal modes___\n\n";
    log += str;

    str = std::format("      Eigenvalue:  {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i\n",
            m_EigenValue[0].real(), m_EigenValue[0].imag(),
            m_EigenValue[1].real(), m_EigenValue[1].imag(),
            m_EigenValue[2].real(), m_EigenValue[2].imag(),
            m_EigenValue[3].real(), m_EigenValue[3].imag());
    log += str;
    log += "                    _____________________________________________________________________________________________________\n";

    str = std::format("      Eigenvector: {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i\n",
            m_EigenVector[0][0].real(),  m_EigenVector[0][0].imag(),
            m_EigenVector[1][0].real(),  m_EigenVector[1][0].imag(),
            m_EigenVector[2][0].real(),  m_EigenVector[2][0].imag(),
            m_EigenVector[3][0].real(),  m_EigenVector[3][0].imag());
    log += str;

    for (int i=1; i<4; i++)
    {
        str = std::format("                   {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i\n",
                m_EigenVector[0][i].real(),  m_EigenVector[0][i].imag(),
                m_EigenVector[1][i].real(),  m_EigenVector[1][i].imag(),
                m_EigenVector[2][i].real(),  m_EigenVector[2][i].imag(),
                m_EigenVector[3][i].real(),  m_EigenVector[3][i].imag());
        log += str;
    }

    log += EOLstr;
    str = "      ___Lateral modes___\n\n";
    log += str;

    str = std::format("      Eigenvalue:  {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i\n",
            m_EigenValue[4].real(), m_EigenValue[4].imag(),
            m_EigenValue[5].real(), m_EigenValue[5].imag(),
            m_EigenValue[6].real(), m_EigenValue[6].imag(),
            m_EigenValue[7].real(), m_EigenValue[7].imag());
    log += str;
    log += "                    _____________________________________________________________________________________________________\n";

    str = std::format("      Eigenvector: {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i\n",
            m_EigenVector[4][0].real(),  m_EigenVector[4][0].imag(),
            m_EigenVector[5][0].real(),  m_EigenVector[5][0].imag(),
            m_EigenVector[6][0].real(),  m_EigenVector[6][0].imag(),
            m_EigenVector[7][0].real(),  m_EigenVector[7][0].imag());
    log += str;

    for (int i=1; i<4; i++)
    {
        str = std::format("                   {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i   |   {:9.4g} + {:9.4g}i\n",
                m_EigenVector[4][i].real(),  m_EigenVector[4][i].imag(),
                m_EigenVector[5][i].real(),  m_EigenVector[5][i].imag(),
                m_EigenVector[6][i].real(),  m_EigenVector[6][i].imag(),
                m_EigenVector[7][i].real(),  m_EigenVector[7][i].imag());
        log += str;
    }
    logmsg = log;
}
