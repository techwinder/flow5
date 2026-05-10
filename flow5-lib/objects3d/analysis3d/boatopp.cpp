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




#include <boatopp.h>


#include <boat.h>
#include <boatpolar.h>
#include <sail.h>
#include <sailglobals.h>
#include <units.h>
#include <utils.h>


BoatOpp::BoatOpp()
{
    m_BoatName.clear();
    m_BtPolarName.clear();
    m_nPanel3 = 0;
    m_nPanel4 = 0;

    m_AnalysisMethod = xfl::VLM2;

    m_bGround           = false;
    m_bIgnoreBodyPanels = false;
    m_bThinSurfaces     = false;
    m_bTrefftz          = false;

    m_bGround      = true;
    m_GroundHeight = 0.0;

    m_TWS_inf = m_TWA_inf = 0.0;

    m_NodeValMin = m_NodeValMax = 0.0;

    m_SailAngle.clear();
}


BoatOpp::BoatOpp(Boat *pBoat, BoatPolar *pBtPolar, int nPanel3, int nPanel4)
{
    m_BoatName = pBoat->name();
    m_BtPolarName = pBtPolar->name();
    m_nPanel3 = nPanel3;
    m_nPanel4 = nPanel4;

    m_AnalysisMethod = pBtPolar->analysisMethod();

    m_bGround = pBtPolar->m_bGround;
    m_bIgnoreBodyPanels = pBtPolar->bIgnoreBodyPanels();
//    m_bThinSurfaces = pBtPolar->m_bThinSurfaces;
    m_bTrefftz= pBtPolar->m_bTrefftz;

    m_GroundHeight      = 0.0;
    m_QInf        = 0.0;
    m_Beta        = 0.0;
    m_Phi         = 0.0;
    m_Ctrl        = 0.0;

    m_SailAngle.resize(pBtPolar->sailAngleSize());
    for(unsigned int is=0;is<m_SailAngle.size(); is++) m_SailAngle[is]=0.0;

    m_SailForceFF.resize(pBoat->nSails());
    m_SailForceSum.resize(pBoat->nSails());
}


void BoatOpp::resizeResultsArrays(int N)
{
    m_Cp.resize(N);
    m_gamma.resize(N);
    m_sigma.resize(N);
    std::fill(m_Cp.begin(), m_Cp.end(), 0);
    std::fill(m_gamma.begin(), m_gamma.end(), 0);
    std::fill(m_sigma.begin(), m_sigma.end(), 0);
}


void BoatOpp::getProperties(Boat const *pBoat, double density, std::string &props, bool bLongOutput) const
{
    std::string strong;
    std::string lenunit, areaunit, forceunit, momentunit, speedunit;
    lenunit    = " " + Units::lengthUnitLabel();
    speedunit  = " " + Units::speedUnitLabel();
    forceunit  = " " + Units::forceUnitLabel();
    momentunit = " " + Units::momentUnitLabel();
    areaunit   = " " + Units::areaUnitLabel();

    double q = 0.5*density*m_QInf*m_QInf;

    std::string BOppProperties;

    if(bLongOutput)
    {
        BOppProperties += "Reference dimensions:\n";
        BOppProperties += "  area  = " + std::format("{:9.5g}", m_AF.refArea()*Units::m2toUnit()) + areaunit + "\n";
        BOppProperties += "  chord = " + std::format("{:9.5g}", m_AF.refChord()*Units::mtoUnit()) + lenunit+ "\n";
    }

    strong = "Ctrl    = " + std::format("{:7.3f}", m_Ctrl);
    BOppProperties += strong + "\n";

    strong = "AWS" + INFstr + "    = " + std::format("{:7.3f}", m_QInf);
    BOppProperties += strong + speedunit+"\n";

    strong = "AWA" + INFstr + "    = " + std::format("{:7.3f}", m_Beta);
    BOppProperties += strong +DEGstr+"\n";

    strong = PHIstr + "       = " + std::format("{:7.3f}", m_Phi);
    BOppProperties += strong +DEGstr+"\n";

    for(unsigned int is=0; is<m_SailAngle.size(); is++)
    {
        strong = std::format("Sail_{:d} Angle = ", is+1) + std::format("{:9.3g}", m_SailAngle.at(is));
        BOppProperties += strong + DEGstr+"\n";
    }

    BOppProperties += "CL           = " + std::format("{:9.5g}", m_AF.CSide())+"\n";
    BOppProperties += "CD           = " + std::format("{:9.5g}", m_AF.CD())+"\n";
    if(bLongOutput)
    {
        BOppProperties += "   CDi       = " + std::format("{:9.5g}", m_AF.CDi())+"\n";
        BOppProperties += "   CDv       = " + std::format("{:9.5g}", m_AF.CDv())+"\n";
    }
    BOppProperties += "Cx           = " + std::format("{:9.5g}", m_AF.Cx())+"\n";
    BOppProperties += "Cy           = " + std::format("{:9.5g}", m_AF.Cy())+"\n";

    strong = "Far Field Fx = " + std::format("{:9.5g}", m_AF.fffx() * q * Units::NtoUnit());
    BOppProperties += strong +forceunit+"\n";
    strong = "Far Field Fy = " + std::format("{:9.5g}", m_AF.fffy() * q * Units::NtoUnit());
    BOppProperties += strong +forceunit+"\n";
    strong = "Far Field Fz = " + std::format("{:9.5g}", m_AF.fffz() * q * Units::NtoUnit());
    BOppProperties += strong +forceunit+"\n";

    strong = "Summed Fx    = " + std::format("{:9.5g}", m_AF.fsumx() * q * Units::NtoUnit());
    BOppProperties += strong +forceunit+"\n";
    strong = "Summed Fy    = " + std::format("{:9.5g}", m_AF.fsumy() * q * Units::NtoUnit());
    BOppProperties += strong +forceunit+"\n";
    strong = "Summed Fz    = " + std::format("{:9.5g}", m_AF.fsumz() * q * Units::NtoUnit());
    BOppProperties += strong +forceunit+"\n";

    strong  = "Mx = " + std::format("{:9.5g}", (m_AF.Mi()+m_AF.Mv()).x * q * Units::NmtoUnit());
    BOppProperties += strong +momentunit+"\n";
    strong  = "My = " + std::format("{:9.5g}", (m_AF.Mi()+m_AF.Mv()).y * q * Units::NmtoUnit());
    BOppProperties += strong +momentunit+"\n";
    strong  = "Mz = " + std::format("{:9.5g}", (m_AF.Mi()+m_AF.Mv()).z * q * Units::NmtoUnit());
    BOppProperties += strong +momentunit;

    if(!bLongOutput) return;
    BOppProperties +="\n";

    strong  = "XCE = Mz/Fy =" + std::format("{:9.5g}", (m_AF.Mi()+m_AF.Mv()).z /m_AF.fsumy() * Units::mtoUnit());
    BOppProperties += strong +lenunit+"\n";
    strong  = "ZCE = Mx/Fy = " + std::format("{:9.5g}", (m_AF.Mi()+m_AF.Mv()).x /m_AF.fsumy() * Units::mtoUnit());
    BOppProperties += strong +lenunit +"\n";

    for(unsigned int is=0; is<m_SailForceFF.size(); is++)
    {
        BOppProperties += pBoat->sailAt(is)->name() + ": \n";
        strong = "   Far Field Fx= " + std::format("{:9.5g}", m_SailForceFF.at(is).x * q * Units::NtoUnit());
        BOppProperties += strong +forceunit+"\n";
        strong = "   Far Field Fy= " + std::format("{:9.5g}", m_SailForceFF.at(is).y * q * Units::NtoUnit());
        BOppProperties += strong +forceunit+"\n";
        strong = "   Far Field Fz= " + std::format("{:9.5g}", m_SailForceFF.at(is).z * q * Units::NtoUnit());
        BOppProperties += strong +forceunit+"\n";
        strong = "   Summed Fx   = " + std::format("{:9.5g}", m_SailForceSum.at(is).x * q * Units::NtoUnit());
        BOppProperties += strong +forceunit+"\n";
        strong = "   Summed Fy   = " + std::format("{:9.5g}", m_SailForceSum.at(is).y * q * Units::NtoUnit());
        BOppProperties += strong +forceunit+"\n";
        strong = "   Summed Fz   = " + std::format("{:9.5g}", m_SailForceSum.at(is).z * q * Units::NtoUnit());
        BOppProperties += strong +forceunit +"\n";
    }
//    BOppProperties.pop_back(); //last \n

    props = BOppProperties;
}


Vector3d BoatOpp::windDir() const
{
    return objects::windDirection(0.0, -m_Beta);
}


std::string BoatOpp::title(bool bLong) const
{
    std::string strong;

    if(bLong)
    {
        if(!isLLTMethod()) //always
        {
            if(isVLM1())
            {
                strong += "VLM1";
            }
            else if(isVLM2())
            {
                strong += "VLM2";
            }
            else if(isQuadMethod())
            {
                strong += "Quads";
            }
            else if(isTriUniformMethod())
            {
                strong += "TriUni";
            }
            else if(isTriLinearMethod())
            {
                strong += "TriLin";
            }
        }

        strong +="-";
    }

    strong += std::format("{:5.2f}-", ctrl());

    if(fabs(beta())>PRECISION) strong += std::format("{:5.2f}°-", beta());

    return strong;
}



void BoatOpp::exportMainDataToString(Boat const*, std::string &data, xfl::enumTextFileType filetype, std::string const &textsep) const
{
    std::string btoppdata;
    std::string strange;
    std::string title;
    std::string lengthlab  = Units::lengthUnitLabel();
    std::string inertialab = Units::inertiaUnitLabel();
    std::string speedlab   = Units::speedUnitLabel();
    std::string masslab    = Units::massUnitLabel();

    std::string sep = "  ";
    if(filetype==xfl::CSV) sep = textsep+ " ";

    btoppdata += boatName()+"\n";
    btoppdata += polarName()+"\n\n";
    btoppdata += "ctrl       " + sep
              + "beta       " + sep
              + "phi        " + sep
              + "VInf("+speedlab+")" +sep
              + "h("+lengthlab+")\n";
    strange = std::format("{:11.5g}", m_Ctrl);
    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_Beta);
    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_Phi);
    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_QInf*Units::mstoUnit());
    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_GroundHeight*Units::mtoUnit());
    btoppdata += strange;

    btoppdata += "\n\n";

    strange = "CL         " + sep
            + "CX         " + sep
            + "CY         " + sep
            + "CD_inviscid" + sep
            + "CD_viscous " + sep
            + "Cl         " + sep
            + "Cm_inviscid" + sep
            + "Cm_viscous " + sep
            + "Cn_inviscid" + sep
            + "Cn_viscous\n";
    btoppdata += strange;

    strange = std::format("{:11.5g}", m_AF.CL());
    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_AF.Cx());
    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_AF.Cy());
    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_AF.CDi());
    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_AF.CDv());
    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_AF.Cli());
    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_AF.Cmi());
    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_AF.Cmv());
    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_AF.Cni());
    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_AF.Cnv());
    btoppdata += strange;

    btoppdata += "\n";

    strange = "CP.x("+lengthlab+")";      for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    strange = "CP.y("+lengthlab+")";      for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    strange = "CP.z("+lengthlab+")";      for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    strange = "NP.x("+lengthlab+")";      for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    btoppdata += "\n";


    strange = std::format("{:11.5g}", m_AF.centreOfPressure().x*Units::mtoUnit());    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_AF.centreOfPressure().y*Units::mtoUnit());    btoppdata += strange+sep;
    strange = std::format("{:11.5g}", m_AF.centreOfPressure().z*Units::mtoUnit());    btoppdata += strange+sep;
    btoppdata += strange + "\n\n";

    strange = "mass("+masslab+")";  for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    strange = "CoG.x("+lengthlab+")";                    for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    strange = "CoG.y("+lengthlab+")";                    for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    strange = "CoG.z("+lengthlab+")";                    for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    strange = "CoG_Ixx("+inertialab+")";              for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    strange = "CoG_Iyy("+inertialab+")";              for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    strange = "CoG_Izz("+inertialab+")";              for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    strange = "CoG_Ixz("+inertialab+")";              for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    btoppdata +="\n";


    btoppdata += strange + "\n\n";


    title = "y("+lengthlab+")";  for(int i=int(title.length()); i<11; i++) title+=" ";   title +=sep;
    title += "Re         " + sep;
    title += "Ai         " + sep;
    title += "Cd_i       " + sep;
    title += "Cd_v       " + sep;
    title += "Cl         " + sep;
    title += "CP.x(%)    " + sep;
    title += "Trans.top  " + sep;
    title += "Trans.bot  " + sep;
    title += "Cm_i       " + sep;
    title += "Cm_v       " + sep;
    title += "Bending.mom" + sep;
    title += "Vd.x       " + sep;
    title += "Vd.y       " + sep;
    title += "Vd.z       " + sep;
    title += "F.x        " + sep;
    title += "F.y        " + sep;
    title += "F.z        ";
    title += '\n';

    data = btoppdata;
}


void BoatOpp::exportPanel3DataToString(Boat const*pBoat,
                                       xfl::enumTextFileType exporttype,
                                       std::string &data) const
{
    std::string strong, paneldata;

    if(exporttype==xfl::TXT) paneldata += " Panel        CtrlPt.x        CtrlPt.y        CtrlPt.z           Nx               Ny             Nz            Area             Cp\n";
    else                     paneldata += "Panel,CtrlPt.x,CtrlPt.y,CtrlPt.z,Nx,Ny,Nz,Area,Cp\n";


    for(int iw=0; iw<pBoat->nSails(); iw++)
    {
        Sail const *pSail = pBoat->sailAt(iw);

        paneldata += pSail->name() + "_Cp Coefficients"+"\n";
        int p=pSail->m_FirstPanel3Index;

        for(int i3=0; i3<pSail->nPanel3(); i3++)
        {
            Panel3 const &p3 = pBoat->triMesh().panelAt(p);

            double cp=0;
            for(int in=0; in<3; in++) cp += m_Cp.at(p3.index()*3+in);
            cp /= 3.0;

            if(exporttype==xfl::TXT)
                strong = std::format("{:d}     {:11g}     {:11g}     {:11g}     {:11g}     {:11g}     {:11g}     {:11g}     {:11g}\n",
                                     p, p3.CoG().x,  p3.CoG().y, p3.CoG().z,  p3.normal().x, p3.normal().y, p3.normal().z, p3.area(), cp);
            else
                strong = std::format("{:d}, {:11g}, {:11g}, {:11g}, {:11g}, {:11g}, {:11g}, {:11g}, {:11g}\n",
                                     p, p3.CoG().x,  p3.CoG().y, p3.CoG().z,  p3.normal().x, p3.normal().y, p3.normal().z, p3.area(), cp);

            paneldata += strong;
            p++;
        }

        paneldata += ("\n\n");
    }

    data = paneldata;
}
