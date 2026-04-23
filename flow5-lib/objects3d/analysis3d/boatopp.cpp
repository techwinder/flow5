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

#define _MATH_DEFINES_DEFINED


#include <QString>



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
    for(uint is=0;is<m_SailAngle.size(); is++) m_SailAngle[is]=0.0;

    m_SailForceFF.resize(pBoat->nSails());
    m_SailForceSum.resize(pBoat->nSails());
}


bool BoatOpp::serializeBoatOppFl5(QDataStream &ar, bool bIsStoring)
{
    // 100001: first file format
    // 100002: added lift and drag
    // 100003: added vortons and negating vortices
    // 100004: Modified the format of AeroForces serialization
    // 100005: beta20 - Added the roration about Ry

    int ArchiveFormat=100005;

    float f(0),g(0),h(0);

    int n(0);
    double dble(0);
    int nIntSpares(0);
    int nDbleSpares(0);
    QString strange;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << QString::fromStdString(m_BoatName);
        ar << QString::fromStdString(m_BtPolarName);

        m_theStyle.serializeFl5(ar, bIsStoring);

        //ANALYSIS METHOD
        if     (m_AnalysisMethod==xfl::LLT)        ar<<1;
        else if(m_AnalysisMethod==xfl::VLM1)       ar<<2;
        else if(m_AnalysisMethod==xfl::VLM1)       ar<<3;
        else if(m_AnalysisMethod==xfl::QUADS)      ar<<4;
        else if(m_AnalysisMethod==xfl::TRILINEAR)  ar<<5;
        else if(m_AnalysisMethod==xfl::TRIUNIFORM) ar<<6;
        else                                       ar<<0;

        ar << m_bThinSurfaces;
        ar << m_bTrefftz;

        ar << m_bIgnoreBodyPanels;

        ar << m_GroundHeight << m_QInf << m_Beta << m_Phi << m_Ry <<m_Ctrl;

        ar<<int(m_SailAngle.size());
        for(uint is=0;is<m_SailAngle.size(); is++)
        {
            ar<< m_SailAngle[is];
        }

        m_AF.serializeFl5(ar, bIsStoring);

        ar << int(m_SailForceFF.size());
        for(uint i=0; i<m_SailForceFF.size(); i++)
        {
            ar << m_SailForceFF.at(i).xf()  << m_SailForceFF.at(i).yf()  << m_SailForceFF.at(i).zf();
            ar << m_SailForceSum.at(i).xf() << m_SailForceSum.at(i).yf() << m_SailForceSum.at(i).zf();
        }

        int N=0;
        if(isQuadMethod())           N = m_nPanel4;
        else if(isTriangleMethod())  N = 3*m_nPanel3;
        ar << N;
        for (int p=0; p<N; p++) ar << float(m_Cp[p]) << float(m_gamma[p]) << float(m_sigma[p]);


        ar << int(m_Vorton.size());
        for(uint ir=0; ir<m_Vorton.size(); ir++)
        {
            ar <<int(m_Vorton.at(ir).size());
            for(uint ic=0; ic<m_Vorton.at(ir).size(); ic++)
            {
                m_Vorton[ir][ic].serializeFl5(ar, bIsStoring);
            }
        }

        ar << int(m_VortexNeg.size());
        for(uint iv=0; iv<m_VortexNeg.size(); iv++)
        {
            m_VortexNeg[iv].serializeFl5(ar, bIsStoring);
        }


        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        dble=0.0;
        for (int i=0; i<nDbleSpares; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<100000 || ArchiveFormat>120000) return false;
        //read variables
        ar >> strange;   m_BoatName = strange.toStdString();;
        ar >> strange;   m_BtPolarName = strange.toStdString();;

        m_theStyle.serializeFl5(ar, bIsStoring);

        // ANALYSIS METHOD
        ar >> n;
        if     (n==1) m_AnalysisMethod=xfl::LLT;
        else if(n==2) m_AnalysisMethod=xfl::VLM1;
        else if(n==3) m_AnalysisMethod=xfl::VLM2;
        else if(n==4) m_AnalysisMethod=xfl::QUADS;
        else if(n==5) m_AnalysisMethod=xfl::TRILINEAR;
        else if(n==6) m_AnalysisMethod=xfl::TRIUNIFORM;

        ar >> m_bThinSurfaces;
        ar >> m_bTrefftz;

        ar >> m_bIgnoreBodyPanels;

        ar >> m_GroundHeight >> m_QInf >> m_Beta >> m_Phi;
        if(ArchiveFormat>=100005) ar >> m_Ry;
        ar >> m_Ctrl;

        ar>>n;
        m_SailAngle.resize(n);
        for(int is=0;is<n; is++)
        {
            ar>> m_SailAngle[is];
        }

        if(ArchiveFormat<100004) m_AF.serializeFl5_b17(ar, bIsStoring);
        else
        {
            if(!m_AF.serializeFl5(ar, bIsStoring)) return false;
        }
        m_AF.setOpp(0.0, m_Beta, m_Phi, m_QInf);

        ar >> n;
        for(int i=0; i<n; i++)
        {
            ar >> f >> g >> h;
            m_SailForceFF.push_back({double(f), double(g), double (h)});
            ar >> f >> g >> h;
            m_SailForceSum.push_back({double(f), double(g), double (h)});
        }

        int N=0;
        ar >> N;

        m_Cp.resize(N);
        m_gamma.resize(N);
        m_sigma.resize(N);
        for (int p=0; p<N; p++)
        {
            ar >> f >> g >> h;
            m_Cp[p]    = double(f);
            m_gamma[p]     = double(g);
            m_sigma[p] = double(h);
        }

        m_nPanel4 = m_nPanel3 = 0;
        if(isQuadMethod())           m_nPanel4 = N;
        else if(isTriangleMethod())  m_nPanel3 = N/3;

        if(ArchiveFormat>=100003)
        {
            ar >> n;
            m_Vorton.resize(n);
            for(uint ir=0; ir<m_Vorton.size(); ir++)
            {
                ar >> n;
                m_Vorton[ir].resize(n);
                for(uint ic=0; ic<m_Vorton.at(ir).size(); ic++)
                {
                    m_Vorton[ir][ic].serializeFl5(ar, bIsStoring);
                }
            }

            ar >> n;
            m_VortexNeg.resize(n);
            for(int iv=0; iv<n; iv++)
            {
                m_VortexNeg[iv].serializeFl5(ar, bIsStoring);
            }
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
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
    QString strong;
    QString lenunit, areaunit, forceunit, momentunit, speedunit;
    lenunit    = QString::fromStdString(" " + Units::lengthUnitLabel());
    speedunit  = QString::fromStdString(" " + Units::speedUnitLabel());
    forceunit  = QString::fromStdString(" " + Units::forceUnitLabel());
    momentunit = QString::fromStdString(" " + Units::momentUnitLabel());
    areaunit   = QString::fromStdString(" " + Units::areaUnitLabel());

    double q = 0.5*density*m_QInf*m_QInf;

    QString BOppProperties;

    if(bLongOutput)
    {
        BOppProperties += "Reference dimensions:\n";
        BOppProperties += "  area  = " + QString::asprintf("%9.5g", m_AF.refArea()*Units::m2toUnit()) + areaunit + "\n";
        BOppProperties += "  chord = " + QString::asprintf("%9.5g", m_AF.refChord()*Units::mtoUnit()) + lenunit+ "\n";
    }

    strong = "Ctrl    = " + QString::asprintf("%7.3f", m_Ctrl);
    BOppProperties += strong + "\n";

    strong = "AWS" + INFch + "    = " + QString::asprintf("%7.3f", m_QInf);
    BOppProperties += strong + speedunit+"\n";

    strong = "AWA" + INFch + "    = " + QString::asprintf("%7.3f", m_Beta);
    BOppProperties += strong +DEGch+"\n";

    strong = PHIch + "       = " + QString::asprintf("%7.3f", m_Phi);
    BOppProperties += strong +DEGch+"\n";

    for(uint is=0; is<m_SailAngle.size(); is++)
    {
        strong = QString::asprintf("Sail_%d Angle = ", is+1) + QString::asprintf("%9.3g", m_SailAngle.at(is));
        BOppProperties += strong + DEGch+"\n";
    }

    BOppProperties += "CL           = " + QString::asprintf("%9.5g", m_AF.CSide())+"\n";
    BOppProperties += "CD           = " + QString::asprintf("%9.5g", m_AF.CD())+"\n";
    if(bLongOutput)
    {
        BOppProperties += "   CDi       = " + QString::asprintf("%9.5g", m_AF.CDi())+"\n";
        BOppProperties += "   CDv       = " + QString::asprintf("%9.5g", m_AF.CDv())+"\n";
    }
    BOppProperties += "Cx           = " + QString::asprintf("%9.5g", m_AF.Cx())+"\n";
    BOppProperties += "Cy           = " + QString::asprintf("%9.5g", m_AF.Cy())+"\n";

    strong = "Far Field Fx = " + QString::asprintf("%9.5g", m_AF.fffx() * q * Units::NtoUnit());
    BOppProperties += strong +forceunit+"\n";
    strong = "Far Field Fy = " + QString::asprintf("%9.5g", m_AF.fffy() * q * Units::NtoUnit());
    BOppProperties += strong +forceunit+"\n";
    strong = "Far Field Fz = " + QString::asprintf("%9.5g", m_AF.fffz() * q * Units::NtoUnit());
    BOppProperties += strong +forceunit+"\n";

    strong = "Summed Fx    = " + QString::asprintf("%9.5g", m_AF.fsumx() * q * Units::NtoUnit());
    BOppProperties += strong +forceunit+"\n";
    strong = "Summed Fy    = " + QString::asprintf("%9.5g", m_AF.fsumy() * q * Units::NtoUnit());
    BOppProperties += strong +forceunit+"\n";
    strong = "Summed Fz    = " + QString::asprintf("%9.5g", m_AF.fsumz() * q * Units::NtoUnit());
    BOppProperties += strong +forceunit+"\n";

    strong  = "Mx = " + QString::asprintf("%9.5g", (m_AF.Mi()+m_AF.Mv()).x * q * Units::NmtoUnit());
    BOppProperties += strong +momentunit+"\n";
    strong  = "My = " + QString::asprintf("%9.5g", (m_AF.Mi()+m_AF.Mv()).y * q * Units::NmtoUnit());
    BOppProperties += strong +momentunit+"\n";
    strong  = "Mz = " + QString::asprintf("%9.5g", (m_AF.Mi()+m_AF.Mv()).z * q * Units::NmtoUnit());
    BOppProperties += strong +momentunit;

    if(!bLongOutput) return;
    BOppProperties +="\n";

    strong  = "XCE = Mz/Fy =" + QString::asprintf("%9.5g", (m_AF.Mi()+m_AF.Mv()).z /m_AF.fsumy() * Units::mtoUnit());
    BOppProperties += strong +lenunit+"\n";
    strong  = "ZCE = Mx/Fy = " + QString::asprintf("%9.5g", (m_AF.Mi()+m_AF.Mv()).x /m_AF.fsumy() * Units::mtoUnit());
    BOppProperties += strong +lenunit +"\n";

    for(uint is=0; is<m_SailForceFF.size(); is++)
    {
        BOppProperties += QString::fromStdString(pBoat->sailAt(is)->name()) + ": \n";
        strong = "   Far Field Fx= " + QString::asprintf("%9.5g", m_SailForceFF.at(is).x * q * Units::NtoUnit());
        BOppProperties += strong +forceunit+"\n";
        strong = "   Far Field Fy= " + QString::asprintf("%9.5g", m_SailForceFF.at(is).y * q * Units::NtoUnit());
        BOppProperties += strong +forceunit+"\n";
        strong = "   Far Field Fz= " + QString::asprintf("%9.5g", m_SailForceFF.at(is).z * q * Units::NtoUnit());
        BOppProperties += strong +forceunit+"\n";
        strong = "   Summed Fx   = " + QString::asprintf("%9.5g", m_SailForceSum.at(is).x * q * Units::NtoUnit());
        BOppProperties += strong +forceunit+"\n";
        strong = "   Summed Fy   = " + QString::asprintf("%9.5g", m_SailForceSum.at(is).y * q * Units::NtoUnit());
        BOppProperties += strong +forceunit+"\n";
        strong = "   Summed Fz   = " + QString::asprintf("%9.5g", m_SailForceSum.at(is).z * q * Units::NtoUnit());
        BOppProperties += strong +forceunit +"\n";
    }
//    BOppProperties.pop_back(); //last \n

    props = BOppProperties.toStdString();
}


Vector3d BoatOpp::windDir() const
{
    return objects::windDirection(0.0, -m_Beta);
}


std::string BoatOpp::title(bool bLong) const
{
    QString strong;

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

    strong += QString::asprintf("%5.2f-", ctrl());

    if(fabs(beta())>PRECISION) strong += QString::asprintf("%5.2f°-", beta());

    return strong.toStdString();
}



void BoatOpp::exportMainDataToString(Boat const*, std::string &data, xfl::enumTextFileType filetype, std::string const &textsep) const
{
    QString btoppdata;
    QString strange;
    QString title;
    QString lengthlab  = Units::lengthUnitQLabel();
    QString inertialab = Units::inertiaUnitQLabel();
    QString speedlab   = Units::speedUnitQLabel();
    QString masslab    = Units::massUnitQLabel();

    QString sep = "  ";
    if(filetype==xfl::CSV) sep = QString::fromStdString(textsep+ " ");

    btoppdata += QString::fromStdString(boatName())+"\n";
    btoppdata += QString::fromStdString(polarName())+"\n\n";
    btoppdata += "ctrl       " + sep
              + "beta       " + sep
              + "phi        " + sep
              + "VInf("+speedlab+")" +sep
              + "h("+lengthlab+")\n";
    strange = QString::asprintf("%11.5g", m_Ctrl);
    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_Beta);
    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_Phi);
    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_QInf*Units::mstoUnit());
    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_GroundHeight*Units::mtoUnit());
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

    strange = QString::asprintf("%11.5g", m_AF.CL());
    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.Cx());
    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.Cy());
    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.CDi());
    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.CDv());
    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.Cli());
    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.Cmi());
    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.Cmv());
    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.Cni());
    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.Cnv());
    btoppdata += strange;

    btoppdata += "\n";

    strange = "CP.x("+lengthlab+")";      for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    strange = "CP.y("+lengthlab+")";      for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    strange = "CP.z("+lengthlab+")";      for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    strange = "NP.x("+lengthlab+")";      for(int i=int(strange.length()); i<11; i++) strange+=" ";   btoppdata += strange+sep;
    btoppdata += "\n";


    strange = QString::asprintf("%11.5g", m_AF.centreOfPressure().x*Units::mtoUnit());    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.centreOfPressure().y*Units::mtoUnit());    btoppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.centreOfPressure().z*Units::mtoUnit());    btoppdata += strange+sep;
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

    data = btoppdata.toStdString();
}


void BoatOpp::exportPanel3DataToString(Boat const*pBoat,
                                       xfl::enumTextFileType exporttype,
                                       std::string &data) const
{
    QString strong, paneldata;

    if(exporttype==xfl::TXT) paneldata += " Panel        CtrlPt.x        CtrlPt.y        CtrlPt.z           Nx               Ny             Nz            Area             Cp\n";
    else                     paneldata += "Panel,CtrlPt.x,CtrlPt.y,CtrlPt.z,Nx,Ny,Nz,Area,Cp\n";


    for(int iw=0; iw<pBoat->nSails(); iw++)
    {
        Sail const *pSail = pBoat->sailAt(iw);

        paneldata += QString::fromStdString(pSail->name())+ "_Cp Coefficients"+"\n";
        int p=pSail->m_FirstPanel3Index;

        for(int i3=0; i3<pSail->nPanel3(); i3++)
        {
            Panel3 const &p3 = pBoat->triMesh().panelAt(p);

            double cp=0;
            for(int in=0; in<3; in++) cp += m_Cp.at(p3.index()*3+in);
            cp /= 3.0;

            if(exporttype==xfl::TXT)
                strong = QString::asprintf("%d     %11g     %11g     %11g     %11g     %11g     %11g     %11g     %11g\n",
                                     p, p3.CoG().x,  p3.CoG().y, p3.CoG().z,  p3.normal().x, p3.normal().y, p3.normal().z, p3.area(), cp);
            else
                strong = QString::asprintf("%d, %11g, %11g, %11g, %11g, %11g, %11g, %11g, %11g\n",
                                     p, p3.CoG().x,  p3.CoG().y, p3.CoG().z,  p3.normal().x, p3.normal().y, p3.normal().z, p3.area(), cp);

            paneldata += strong;
            p++;
        }

        paneldata += ("\n\n");
    }

    data = paneldata.toStdString();
}
