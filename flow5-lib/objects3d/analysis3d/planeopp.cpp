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
#include <iomanip>

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

    double qDyn = 0.5*pWPolar->density()*m_QInf*m_QInf;

    if(pPlane && pWPolar)
    {

        Vector3d Force  = m_AF.Fff()+m_AF.viscousDragForce();
        Vector3d Moment = m_AF.Mi()+m_AF.Mv();

        Force  *= qDyn * Units::NtoUnit();
        Moment *= qDyn * Units::NmtoUnit();

        Vector3d Force_w = m_AF.toWindAxes(Force);
        Vector3d Moment_w = m_AF.toWindAxes(Moment);

        props += "Total force: Geom. axes   Wind axes\n";
        strong = std::format("      Fx={:11.3f}  {:11.3f} ", Force.x, Force_w.x) + Units::forceUnitLabel();
        props += strong + "\n";

        strong = std::format("      Fy={:11.3f}  {:11.3f} ", Force.y, Force_w.y) + Units::forceUnitLabel();
        props += strong +"\n";

        strong = std::format("      Fz={:11.3f}  {:11.3f} ", Force.z,  Force_w.z) + Units::forceUnitLabel();
        props += strong + "\n";

        strong = std::format("      Mx={:11.3f}  {:11.3f} ", Moment.x, Moment_w.x) + Units::momentUnitLabel();
        props += strong + "\n";

        strong = std::format("      My={:11.3f}  {:11.3f} ", Moment.y, Moment_w.y) + Units::momentUnitLabel();
        props += strong + "\n";

        strong = std::format("      Mz={:11.3f}  {:11.3f} ", Moment.z, Moment_w.z) + Units::momentUnitLabel();
        props += strong + "\n\n";
    }


    PlaneXfl const* pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
    if(pPlaneXfl && pWPolar)
    {

        props += "Parts:       Geom. axes   Wind axes\n";
        for(unsigned int iw=0; iw<m_WingOpp.size(); iw++)
        {
            WingXfl const *pWing = pPlaneXfl->wingAt(iw);
            WingOpp const*pWOpp = &m_WingOpp.at(iw);

            if(pWing && pWOpp)
            {
                props += "  " + pWing->name() + ":\n";

                Vector3d DragForce = pWOpp->m_AF.viscousDragForce();
                Vector3d Force  = pWOpp->m_AF.Fff()+DragForce;
                Vector3d Moment = pWOpp->m_AF.Mi()+pWOpp->m_AF.Mv();

                Force  *= qDyn * Units::NtoUnit();
                Moment *= qDyn * Units::NmtoUnit();

                Vector3d Force_w = m_AF.toWindAxes(Force);
                Vector3d Moment_w = m_AF.toWindAxes(Moment);

                strong = std::format("      Fx={:11.3f}  {:11.3f} ", Force.x,  Force_w.x) + Units::forceUnitLabel();
                props += strong + "\n";

                strong = std::format("      Fy={:11.3f}  {:11.3f} ", Force.y,  Force_w.y) + Units::forceUnitLabel();
                props += strong +"\n";

                strong = std::format("      Fz={:11.3f}  {:11.3f} ", Force.z,  Force_w.z) + Units::forceUnitLabel();
                props += strong + "\n";

                strong = std::format("      Mx={:11.3f}  {:11.3f} ", Moment.x, Moment_w.x) + Units::momentUnitLabel();
                props += strong + "\n";

                strong = std::format("      My={:11.3f}  {:11.3f} ", Moment.y, Moment_w.y) + Units::momentUnitLabel();
                props += strong + "\n";

                strong = std::format("      Mz={:11.3f}  {:11.3f} ", Moment.z, Moment_w.z) + Units::momentUnitLabel();
                props += strong + "\n\n";
            }
        }

        for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++)
        {
            if(int(m_FuseAF.size())<=ifuse) break;  // FuseAF not defined if LLT

            Fuse const *pFuse = pPlaneXfl->fuseAt(ifuse);           
            props += "  " + pFuse->name() + ":\n";

            AeroForces const &AF = m_FuseAF.at(ifuse);
            Vector3d DragForce = WindD * AF.fuseDrag();
            Vector3d Force  = AF.Fff()+DragForce;
            Vector3d Moment = AF.Mi()+AF.Mv();

            Force  *= qDyn * Units::NtoUnit();
            Moment *= qDyn * Units::NmtoUnit();

            Vector3d Force_w = m_AF.toWindAxes(Force);
            Vector3d Moment_w = m_AF.toWindAxes(Moment);

            strong = std::format("      Fx={:11.3f}  {:11.3f} ", Force.x, Force_w.x) + Units::forceUnitLabel();
            props += strong + "\n";

            strong = std::format("      Fy={:11.3f}  {:11.3f} ", Force.y, Force_w.y) + Units::forceUnitLabel();
            props += strong +"\n";

            strong = std::format("      Fz={:11.3f}  {:11.3f} ", Force.z,  Force_w.z) + Units::forceUnitLabel();
            props += strong + "\n";

            strong = std::format("      Mx={:11.3f}  {:11.3f} ", Moment.x, Moment_w.x) + Units::momentUnitLabel();
            props += strong + "\n";

            strong = std::format("      My={:11.3f}  {:11.3f} ", Moment.y, Moment_w.y) + Units::momentUnitLabel();
            props += strong + "\n";

            strong = std::format("      Mz={:11.3f}  {:11.3f} ", Moment.z, Moment_w.z) + Units::momentUnitLabel();
            props += strong + "\n\n";
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
                strange = std::format("    Flap_{:d} = {:8.4f} ", i+1, WOpp(iwo).m_FlapMoment[i]*Units::NmtoUnit());
                props += strange + Units::momentUnitLabel() + EOLstr;
            }
        }
    }


    props += "\n";

    if(isType12358() || isType7())
    {
        props += "\n";
        props += "Non-dim. stability derivatives in stability axes:\n";
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
            props += "Non-dimensional control derivatives in stability axes:\n";
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
            strange  = std::format("{:.2f}", alpha()) + DEGstr + " ";
            strange += std::format("{:.2f}", beta())  + DEGstr + " ";
            strange += std::format("{:.2f}", QInf()*Units::mstoUnit()) + " " + Units::speedUnitLabel();
            break;
        case xfl::T6POLAR:
        case xfl::T7POLAR:
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


void PlaneOpp::exportMainDataToString(Plane const*pPlane, std::string &poppdata, xfl::enumTextFileType filetype, const std::string &textsep) const
{
    std::stringstream sstr;

    std::string strange;
    std::string title;
    std::string len = Units::lengthUnitLabel();
    std::string inertia = Units::inertiaUnitLabel();

    std::string sep = "  ";
    if(filetype==xfl::CSV) sep = textsep+ " ";

    sstr << std::string(planeName())+"\n";
    sstr << std::string(polarName())+"\n\n";
    sstr << std::setw(17) << ALPHAstr;
    sstr << sep;
    sstr << std::setw(17) << BETAstr;
    sstr << sep;
    sstr << std::setw(17) << PHIstr;
    sstr << sep;
    sstr << std::setw(17)<< std::string("ctrl") + sep;
    sstr << sep;
    sstr << std::setw(17)<< std::string("VInf("+Units::speedUnitLabel()+")") +"\n";

    strange = std::format("{:17g}", m_Alpha);                     sstr << strange+sep;
    strange = std::format("{:17g}", m_Beta);                      sstr << strange+sep;
    strange = std::format("{:17g}", m_Phi);                       sstr << strange+sep;
    strange = std::format("{:17g}", m_Ctrl);                      sstr << strange+sep;
    strange = std::format("{:17g}", m_QInf*Units::mstoUnit());    sstr << strange;

    sstr << "\n\n";

    sstr << std::setw(17) <<    std::string("CL")          ;   sstr << sep;
    sstr << std::setw(17) <<    std::string("CX")          ;   sstr << sep;
    sstr << std::setw(17) <<    std::string("CY")          ;   sstr << sep;
    sstr << std::setw(17) <<    std::string("CD_inviscid") ;   sstr << sep;
    sstr << std::setw(17) <<    std::string("CD_viscous")  ;   sstr << sep;
    sstr << std::setw(17) <<    std::string("Cl")          ;   sstr << sep;
    sstr << std::setw(17) <<    std::string("Cm_inviscid") ;   sstr << sep;
    sstr << std::setw(17) <<    std::string("Cm_viscous")  ;   sstr << sep;
    sstr << std::setw(17) <<    std::string("Cn_inviscid") ;   sstr << sep;
    sstr << std::setw(17) <<    std::string("Cn_viscous")  + "\n";


    strange = std::format("{:17g}", m_AF.CL());     sstr << strange+sep;
    strange = std::format("{:17g}", m_AF.CD());     sstr << strange+sep;
    strange = std::format("{:17g}", m_AF.Cy());     sstr << strange+sep;
    strange = std::format("{:17g}", m_AF.CDi());    sstr << strange+sep;
    strange = std::format("{:17g}", m_AF.CDv());    sstr << strange+sep;
    strange = std::format("{:17g}", m_AF.Cli());    sstr << strange+sep;
    strange = std::format("{:17g}", m_AF.Cmi());    sstr << strange+sep;
    strange = std::format("{:17g}", m_AF.Cmv());    sstr << strange+sep;
    strange = std::format("{:17g}", m_AF.Cni());    sstr << strange+sep;
    strange = std::format("{:17g}", m_AF.Cnv());    sstr << strange;

    sstr << "\n\n";


    sstr << std::setw(17) <<  std::string("CP.x("+len+")");         sstr << sep;
    sstr << std::setw(17) <<  std::string("CP.y("+len+")");         sstr << sep;
    sstr << std::setw(17) <<  std::string("CP.z("+len+")");         sstr << sep;
    sstr << std::setw(17) <<  std::string("NP.x("+len+")");         sstr << sep;
    sstr << "\n";

    strange = std::format("{:17g}", m_AF.centreOfPressure().x*Units::mtoUnit());    sstr << strange+sep;
    strange = std::format("{:17g}", m_AF.centreOfPressure().y*Units::mtoUnit());    sstr << strange+sep;
    strange = std::format("{:17g}", m_AF.centreOfPressure().z*Units::mtoUnit());    sstr << strange+sep;
    strange = std::format("{:17g}", m_SD.XNP   *Units::mtoUnit());    sstr << strange+sep;
    sstr << "\n\n";

    sstr << std::setw(17) <<  std::string("mass("+Units::massUnitLabel()+")");     sstr << sep;
    sstr << std::setw(17) <<  std::string("CoG.x("+len+")");                       sstr << sep;
    sstr << std::setw(17) <<  std::string("CoG.y("+len+")");                       sstr << sep;
    sstr << std::setw(17) <<  std::string("CoG.z("+len+")");                       sstr << sep;
    sstr << std::setw(17) <<  std::string("CoG_Ixx("+inertia+")");                 sstr << sep;
    sstr << std::setw(17) <<  std::string("CoG_Iyy("+inertia+")");                 sstr << sep;
    sstr << std::setw(17) <<  std::string("CoG_Izz("+inertia+")");                 sstr << sep;
    sstr << std::setw(17) <<  std::string("CoG_Ixz("+inertia+")");                 sstr << sep;
    poppdata +="\n";

    strange = std::format("{:17g}", m_Mass*Units::kgtoUnit());            sstr << strange+sep;
    strange = std::format("{:17g}", m_CoG.x*Units::mtoUnit());            sstr << strange+sep;
    strange = std::format("{:17g}", m_CoG.y*Units::mtoUnit());            sstr << strange+sep;
    strange = std::format("{:17g}", m_CoG.z*Units::mtoUnit());            sstr << strange+sep;
    strange = std::format("{:17g}", m_Inertia[0]*Units::kgm2toUnit());    sstr << strange+sep;
    strange = std::format("{:17g}", m_Inertia[1]*Units::kgm2toUnit());    sstr << strange+sep;
    strange = std::format("{:17g}", m_Inertia[2]*Units::kgm2toUnit());    sstr << strange+sep;
    strange = std::format("{:17g}", m_Inertia[3]*Units::kgm2toUnit());    sstr << strange + "\n\n";


    if(isType12358() || isType7())
    {
        StabDerivatives const &SD = m_SD;

        sstr << std::setw(17) << std::string("CXu");   sstr << sep;
        sstr << std::setw(17) << std::string("CZu");   sstr << sep;
        sstr << std::setw(17) << std::string("Cmu");   sstr << sep;
        sstr << std::setw(17) << std::string("CXa");   sstr << sep;
        sstr << std::setw(17) << std::string("CZa");   sstr << sep;
        sstr << std::setw(17) << std::string("Cma");   sstr << sep;
        sstr << std::setw(17) << std::string("CXq");   sstr << sep;
        sstr << std::setw(17) << std::string("CZq");   sstr << sep;
        sstr << std::setw(17) << std::string("Cmq");   sstr << sep;
        sstr << EOLstr;
        sstr << std::format("{:17g}", SD.CXu);   sstr << sep;
        sstr << std::format("{:17g}", SD.CZu);   sstr << sep;
        sstr << std::format("{:17g}", SD.Cmu);   sstr << sep;
        sstr << std::format("{:17g}", SD.CXa);   sstr << sep;
        sstr << std::format("{:17g}", SD.CZa);   sstr << sep;
        sstr << std::format("{:17g}", SD.Cma);   sstr << sep;
        sstr << std::format("{:17g}", SD.CXq);   sstr << sep;
        sstr << std::format("{:17g}", SD.CZq);   sstr << sep;
        sstr << std::format("{:17g}", SD.Cmq);   sstr << sep;
        sstr << EOLstr + EOLstr;

        sstr << std::setw(17) << std::string("Cyb");   sstr << sep;
        sstr << std::setw(17) << std::string("Clb");   sstr << sep;
        sstr << std::setw(17) << std::string("Cnb");   sstr << sep;
        sstr << std::setw(17) << std::string("Cyp");   sstr << sep;
        sstr << std::setw(17) << std::string("Clp");   sstr << sep;
        sstr << std::setw(17) << std::string("Cnp");   sstr << sep;
        sstr << std::setw(17) << std::string("Cyr");   sstr << sep;
        sstr << std::setw(17) << std::string("Clr");   sstr << sep;
        sstr << std::setw(17) << std::string("Cnr");   sstr << sep;
        sstr << EOLstr;
        sstr << std::format("{:17g}", SD.CYb);   sstr << sep;
        sstr << std::format("{:17g}", SD.Clb);   sstr << sep;
        sstr << std::format("{:17g}", SD.Cnb);   sstr << sep;
        sstr << std::format("{:17g}", SD.CYp);   sstr << sep;
        sstr << std::format("{:17g}", SD.Clp);   sstr << sep;
        sstr << std::format("{:17g}", SD.Cnp);   sstr << sep;
        sstr << std::format("{:17g}", SD.CYr);   sstr << sep;
        sstr << std::format("{:17g}", SD.Clr);   sstr << sep;
        sstr << std::format("{:17g}", SD.Cnr);   sstr << sep;
        sstr << EOLstr + EOLstr;

        for(unsigned int i=0; i<SD.ControlNames.size(); i++)
        {
            sstr << "  " + std::string(SD.ControlNames.at(i)) + EOLstr;

            sstr << std::setw(17) << std::string("CXd");   sstr << sep;
            sstr << std::setw(17) << std::string("CYd");   sstr << sep;
            sstr << std::setw(17) << std::string("CZd");   sstr << sep;
            sstr << std::setw(17) << std::string("Cld");   sstr << sep;
            sstr << std::setw(17) << std::string("Cmd");   sstr << sep;
            sstr << std::setw(17) << std::string("Cnd");   sstr << sep;
            sstr << EOLstr;
            sstr << std::format("{:17g}", SD.CXe.at(i));   sstr << sep;
            sstr << std::format("{:17g}", SD.CYe.at(i));   sstr << sep;
            sstr << std::format("{:17g}", SD.CZe.at(i));   sstr << sep;
            sstr << std::format("{:17g}", SD.CLe.at(i));   sstr << sep;
            sstr << std::format("{:17g}", SD.CMe.at(i));   sstr << sep;
            sstr << std::format("{:17g}", SD.CNe.at(i));   sstr << sep;
            sstr << EOLstr + EOLstr;
        }


    }
    sstr << "\n\n";


    if(pPlane->isXflType())
    {
        PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);


        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            if(iw<nWOpps())
            {
                //if there are any flaps
                if(WOpp(iw).m_FlapMoment.size())
                {
                    sstr << std::string(pPlaneXfl->wingAt(iw)->name()) +" - flap moments ("+ Units::momentUnitLabel() +")\n";
                    for (int l=0; l<WOpp(iw).m_nFlaps; l++)
                    {
                        sstr << std::setw(17) << std::format("Flap_{:d}", l+1);
                        sstr << sep;

                    }
                    sstr << EOLstr;
                    for (int l=0; l<WOpp(iw).m_nFlaps; l++)
                    {
                        strange = std::format("{:17g}", WOpp(iw).m_FlapMoment.at(l)*Units::NmtoUnit()) + sep;
                        sstr << strange;
                    }
                    sstr << EOLstr;
                }
            }
        }
    }
    sstr << "\n\n";

    std::stringstream sstitle;

    sstitle << std::string("y("+Units::lengthUnitLabel()+")");   sstitle << sep;;
    sstitle << std::string("Re");          sstitle << sep;;
    sstitle << std::string("Ai");          sstitle << sep;;
    sstitle << std::string("Cd_i");        sstitle << sep;;
    sstitle << std::string("Cd_v");        sstitle << sep;;
    sstitle << std::string("Cl");          sstitle << sep;;
    sstitle << std::string("CP.x(%)");     sstitle << sep;;
    sstitle << std::string("Trans.top");   sstitle << sep;;
    sstitle << std::string("Trans.bot");   sstitle << sep;;
    sstitle << std::string("Cm_i");        sstitle << sep;;
    sstitle << std::string("Cm_v");        sstitle << sep;;
    sstitle << std::string("Bending.mom"); sstitle << sep;;
    sstitle << std::string("Vd.x");        sstitle << sep;;
    sstitle << std::string("Vd.y");        sstitle << sep;;
    sstitle << std::string("Vd.z");        sstitle << sep;;
    sstitle << std::string("F.x");         sstitle << sep;;
    sstitle << std::string("F.y");         sstitle << sep;;
    sstitle << std::string("F.z");         sstitle << sep;;
    sstitle << GAMMAstr;
    sstitle << '\n';

    title = sstitle.str();

    if(pPlane->isXflType())
    {
        PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            if(iw>=nWOpps()) break; // error somewhere

            WingOpp const &aWOpp = WOpp(iw);

            sstr << std::string(pPlaneXfl->wingAt(iw)->name())+'\n';
            sstr << title;
            for(int i=0; i<aWOpp.m_NStation; i++)
            {
                strange = std::format("{:17g}", aWOpp.spanResults().m_StripPos.at(i)*Units::mtoUnit());     sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_Re.at(i));             sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_Ai.at(i));             sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_ICd.at(i));            sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_PCd.at(i));            sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_Cl.at(i));             sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_XCPSpanRel.at(i));     sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_XTrTop.at(i));         sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_XTrBot.at(i));         sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_CmC4.at(i));           sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_CmViscous.at(i));      sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_BendingMoment.at(i));  sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_Vd.at(i).x);           sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_Vd.at(i).y);           sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_Vd.at(i).z);           sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_F.at(i).x);            sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_F.at(i).y);            sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_F.at(i).z);            sstr << strange+sep;
                strange = std::format("{:17g}", aWOpp.spanResults().m_Gamma.at(i));          sstr << strange+sep;

                sstr << '\n';
            }
            sstr << '\n';
        }
    }

    poppdata = sstr.str();
}



void PlaneOpp::exportPanel4DataToString(Plane const *pPlane, PlanePolar const *pWPolar,
                                        xfl::enumTextFileType exporttype,
                                        std::string &paneldata) const
{
    paneldata.clear();
    if(!pPlane->isXflType())
    {
        return;
    }

    if(!pWPolar->isQuadMethod()) return;

    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

    std::string strong, Format;
    std::stringstream out;

    out << "Main Wing Cp Coefficients\n";

    int coef = 1;

    if(!pWPolar->bThinSurfaces())
    {
        coef = 2;
    }
    if(exporttype==xfl::TXT) out << " Panel     CtrlPt.x        CtrlPt.y        CtrlPt.z       Nx      Ny       Nz        Area       Cp\n";
    else                     out << "Panel,CtrlPt.x,CtrlPt.y,CtrlPt.z,Nx,Ny,Nz,Area,Cp\n";


    std::string sep;
    if(exporttype==xfl::TXT) sep = "     ";
    else                     sep = ", ";


    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        if(pPlaneXfl->wingAt(iw))
        {
            out << pPlaneXfl->wingAt(iw)->name() + "Cp Coefficients"+"\n";
            int p=0;
            int iStrip = 0;
            for (int j=0; j<pPlaneXfl->wingAt(iw)->nSurfaces(); j++)
            {
                Surface const & surf = pPlaneXfl->wingAt(iw)->surfaceAt(j);
                if(surf.isTipLeft() && !pWPolar->bThinSurfaces())
                {
                    while (pPlaneXfl->panel4(pPlaneXfl->wingAt(iw)->firstPanel4Index() + p).isSidePanel())
                        p++;
                }

                for(int k=0; k<surf.NYPanels(); k++)
                {
                    iStrip++;
                    strong = std::format("Strip {:d}\n", iStrip);
                    out << strong;

                    for(int l=0; l<surf.NXPanels() * coef; l++)
                    {
                        Panel4 const &p4 = pPlaneXfl->panel4(pPlaneXfl->wingAt(iw)->firstPanel4Index() + p);

                        out << std::format("{:7d}", p) << sep;
                        out << std::format("{:11f}", p4.ctrlPt(isVLMMethod()).x) << sep;
                        out << std::format("{:11f}", p4.ctrlPt(isVLMMethod()).y) << sep;
                        out << std::format("{:11f}", p4.ctrlPt(isVLMMethod()).z) << sep;

                        out << std::format("{:11f}", p4.normal().x) << sep;
                        out << std::format("{:11f}", p4.normal().y) << sep;
                        out << std::format("{:11f}", p4.normal().z) << sep;

                        out << std::format("{:11f}", p4.area()) << sep;
                        out << std::format("{:11f}", WOpp(iw).m_dCp[p]) << EOLstr;
                        p++;
                    }
                }
            }
        }
        out << ("\n\n");
    }

    paneldata = out.str();
}


void PlaneOpp::exportPanel3DataToString(Plane const *pPlane, PlanePolar const *pWPolar,
                                        xfl::enumTextFileType exporttype, std::string const &textsep,
                                        std::string &paneldata) const
{

    paneldata.clear();
    if(!pPlane || !pWPolar) return;
    if(!pWPolar->isTriangleMethod()) return;

    std::string sep = "  ";
    if(exporttype==xfl::CSV) sep = textsep + " ";

    std::string strong, strange;
    std::stringstream out;

    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

    if(pPlaneXfl)
    {
        int coef = 1;
        if(!pWPolar->bThinSurfaces())  coef = 2;

        out << std::setw(17) <<  "Panel";       out << sep;
        out << std::setw(17) << "CtrlPt.x";     out << sep;
        out << std::setw(17) << "CtrlPt.y";     out << sep;
        out << std::setw(17) << "CtrlPt.z";     out << sep;
        out << std::setw(17) << "N.x";          out << sep;
        out << std::setw(17) << "N.y";          out << sep;
        out << std::setw(17) << "N.z";          out << sep;
        out << std::setw(17) << "Area";         out << sep;
        out << std::setw(17) << "Cp" << "\n";

        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            out << pPlaneXfl->wingAt(iw)->name() + " - Cp Coefficients"+"\n";
            int p=0;
            int iStrip = 0;
            for (int j=0; j<pPlaneXfl->wingAt(iw)->nSurfaces(); j++)
            {
                Surface const & surf = pPlaneXfl->wingAt(iw)->surfaceAt(j);
                if(surf.isTipLeft() && !pWPolar->bThinSurfaces())
                {
                    while (pPlaneXfl->panel3At(pPlaneXfl->wingAt(iw)->firstPanel3Index() + p).isSidePanel())
                        p++;
                }

                for(int k=0; k<surf.NYPanels(); k++)
                {
                    iStrip++;
                    out << std::format("Strip {:d}\n", iStrip);

                    for(int l=0; l<surf.NXPanels() * coef *2; l++)
                    {
                        Panel3 const &p3 = pPlaneXfl->panel3At(pPlaneXfl->wingAt(iw)->firstPanel3Index() + p);

                        double cp=0;
                        for(int in=0; in<3; in++) cp += m_Cp.at(p3.index()*3+in);
                        cp /= 3.0;

                        strong = std::format("{:17d}", p3.index())        +sep;
                        strong += std::format("{:17g}", p3.CoG().x)       +sep;
                        strong += std::format("{:17g}", p3.CoG().y)       +sep;
                        strong += std::format("{:17g}", p3.CoG().z)       +sep;
                        strong += std::format("{:17g}", p3.normal().x)    +sep;
                        strong += std::format("{:17g}", p3.normal().y)    +sep;
                        strong += std::format("{:17g}", p3.normal().z)    +sep;
                        strong += std::format("{:17g}", p3.area())        +sep;
                        strong += std::format("{:17g}", cp)               +"\n";

                        out << strong;
                        p++;
                    }
                }
            }
            out << ("\n\n");
        }

        for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++)
        {
            Fuse const *pFuse = pPlaneXfl->fuseAt(ifuse);
            out << pFuse->name() + " - Cp Coefficients"+"\n";

            for(int p=0; p<pFuse->nPanel3(); p++)
            {
                Panel3 const &p3 = pPlaneXfl->panel3At(pFuse->firstPanel3Index() + p);

                double cp=0;
                for(int in=0; in<3; in++) cp += m_Cp.at(p3.index()*3+in);
                cp /= 3.0;

                strong =  std::format("{:17d}", p3.index())       +sep;
                strong += std::format("{:17g}", p3.CoG().x)       +sep;
                strong += std::format("{:17g}", p3.CoG().y)       +sep;
                strong += std::format("{:17g}", p3.CoG().z)       +sep;
                strong += std::format("{:17g}", p3.normal().x)    +sep;
                strong += std::format("{:17g}", p3.normal().y)    +sep;
                strong += std::format("{:17g}", p3.normal().z)    +sep;
                strong += std::format("{:17g}", p3.area())        +sep;
                strong += std::format("{:17g}", cp)               +"\n";


                out << strong;
            }
            out <<"\n\n";
        }
    }
    else
    {
        PlaneSTL const *pPlaneSTL = dynamic_cast<PlaneSTL const*>(pPlane);
        if(!pPlaneSTL) return;


        out << pPlane->name() + " - Cp Coefficients"+"\n";

        out << std::setw(17) << "Panel";   out << sep;
        out << std::setw(17) << "CtrlPt.x";   out << sep;
        out << std::setw(17) << "CtrlPt.y";   out << sep;
        out << std::setw(17) << "CtrlPt.z";   out << sep;
        out << std::setw(17) << "N.x";        out << sep;
        out << std::setw(17) << "N.y";        out << sep;
        out << std::setw(17) << "N.z";        out << sep;
        out << std::setw(17) << "Area";       out << sep;
        out << std::setw(17) << "Cp" << "\n";



        for(int k=0; k<nPanel3(); k++)
        {
            Panel3 const &p3 = pPlaneSTL->panel3At(k);

            double cp=0;
            for(int in=0; in<3; in++) cp += m_Cp.at(p3.index()*3+in);
            cp /= 3.0;

            strong = std::format("{:17d}", p3.index())        +sep;
            strong += std::format("{:17g}", p3.CoG().x)       +sep;
            strong += std::format("{:17g}", p3.CoG().y)       +sep;
            strong += std::format("{:17g}", p3.CoG().z)       +sep;
            strong += std::format("{:17g}", p3.normal().x)    +sep;
            strong += std::format("{:17g}", p3.normal().y)    +sep;
            strong += std::format("{:17g}", p3.normal().z)    +sep;
            strong += std::format("{:17g}", p3.area())        +sep;
            strong += std::format("{:17g}", cp)               +"\n";


            out << strong;

        }

        out << ("\n\n");
    }
    paneldata = out.str();
}
