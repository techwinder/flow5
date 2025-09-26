/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#define _MATH_DEFINES_DEFINED

#include "boatopp.h"
#include <xflobjects/sailobjects/boat.h>
#include <xflobjects/sailobjects/sails/sail.h>
#include <xflobjects/objects3d/analysis/boatpolar.h>

#include <xflobjects/sailobjects/sailglobals.h>
#include <xflcore/xflcore.h>
#include <xflcore/units.h>


BoatOpp::BoatOpp()
{
    m_BoatName = QString();
    m_BtPolarName = QString();
    m_nPanel3 = 0;
    m_nPanel4 = 0;

    m_AnalysisMethod = Polar3d::VLM2;

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
    for(int is=0;is<m_SailAngle.size(); is++) m_SailAngle[is]=0.0;

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

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << m_BoatName;
        ar << m_BtPolarName;

        m_theStyle.serializeFl5(ar, bIsStoring);

        //ANALYSIS METHOD
        if     (m_AnalysisMethod==Polar3d::LLT)        ar<<1;
        else if(m_AnalysisMethod==Polar3d::VLM1)       ar<<2;
        else if(m_AnalysisMethod==Polar3d::VLM1)       ar<<3;
        else if(m_AnalysisMethod==Polar3d::QUADS)      ar<<4;
        else if(m_AnalysisMethod==Polar3d::TRILINEAR)  ar<<5;
        else if(m_AnalysisMethod==Polar3d::TRIUNIFORM) ar<<6;
        else                                       ar<<0;

        ar << m_bThinSurfaces;
        ar << m_bTrefftz;

        ar << m_bIgnoreBodyPanels;

        ar << m_GroundHeight << m_QInf << m_Beta << m_Phi << m_Ry <<m_Ctrl;

        ar<<int(m_SailAngle.size());
        for(int is=0;is<m_SailAngle.size(); is++)
        {
            ar<< m_SailAngle[is];
        }

        m_AF.serializeFl5(ar, bIsStoring);

        ar << int(m_SailForceFF.size());
        for(int i=0; i<m_SailForceFF.size(); i++)
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
        for(int ir=0; ir<m_Vorton.size(); ir++)
        {
            ar <<int(m_Vorton.at(ir).size());
            for(int ic=0; ic<m_Vorton.at(ir).size(); ic++)
            {
                m_Vorton[ir][ic].serializeFl5(ar, bIsStoring);
            }
        }

        ar << int(m_VortexNeg.size());
        for(int iv=0; iv<m_VortexNeg.size(); iv++)
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
        ar >> m_BoatName;
        ar >> m_BtPolarName;

        m_theStyle.serializeFl5(ar, bIsStoring);

        // ANALYSIS METHOD
        ar >> n;
        if     (n==1) m_AnalysisMethod=Polar3d::LLT;
        else if(n==2) m_AnalysisMethod=Polar3d::VLM1;
        else if(n==3) m_AnalysisMethod=Polar3d::VLM2;
        else if(n==4) m_AnalysisMethod=Polar3d::QUADS;
        else if(n==5) m_AnalysisMethod=Polar3d::TRILINEAR;
        else if(n==6) m_AnalysisMethod=Polar3d::TRIUNIFORM;

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
            for(int ir=0; ir<m_Vorton.size(); ir++)
            {
                ar >> n;
                m_Vorton[ir].resize(n);
                for(int ic=0; ic<m_Vorton.at(ir).size(); ic++)
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
    m_Cp.fill(0);
    m_gamma.fill(0);
    m_sigma.fill(0);
}


void BoatOpp::getProperties(Boat const *pBoat, double density, QString &BOppProperties, bool bLongOutput) const
{
    QString strong, lenunit, forceunit, momentunit, speedunit;

    Units::getLengthUnitLabel(lenunit);       lenunit = " "+lenunit;
    Units::getSpeedUnitLabel(speedunit);      speedunit = " "+speedunit;
    Units::getForceUnitLabel(forceunit);      forceunit = " "+forceunit;
    Units::getMomentUnitLabel(momentunit);    momentunit = " "+momentunit;
    QString fmt  = xfl::isLocalized() ? "%L1" : "%1";

    double q = 0.5*density*m_QInf*m_QInf;

    BOppProperties.clear();

    if(bLongOutput)
    {
        BOppProperties += "Reference dimensions:\n";
        BOppProperties += QString("  area  = "+fmt).arg(m_AF.refArea()*Units::m2toUnit()) + " "+ Units::areaUnitLabel()+ "\n";
        BOppProperties += QString("  chord = "+fmt).arg(m_AF.refChord()*Units::mtoUnit()) + " "+ Units::lengthUnitLabel()+ "\n";
    }

    strong = QString("Ctrl    = "+fmt).arg(m_Ctrl,7,'f',3);
    BOppProperties += strong + "\n";

    strong = "AWS" + INFCHAR + QString("    = "+fmt).arg(m_QInf,7,'f',3);
    BOppProperties += strong + speedunit+"\n";

    strong = "AWA" + INFCHAR + QString("    = "+fmt).arg(m_Beta,7,'f',3);
    BOppProperties += strong +DEGCHAR+"\n";

    strong = PHICHAR + QString("       = "+fmt).arg(m_Phi,7,'f',3);
    BOppProperties += strong +DEGCHAR+"\n";

    for(int is=0; is<m_SailAngle.size(); is++)
    {
        strong = QString::asprintf("Sail_%d Angle = ",is+1) + QString(fmt).arg(m_SailAngle.at(is));
        BOppProperties += strong + DEGCHAR+"\n";
    }

    BOppProperties += QString("CL           = " + fmt).arg(m_AF.CSide())+"\n";
    BOppProperties += QString("CD           = " + fmt).arg(m_AF.CD())+"\n";
    if(bLongOutput)
    {
        BOppProperties += QString("   CDi       = " + fmt).arg(m_AF.CDi())+"\n";
        BOppProperties += QString("   CDv       = " + fmt).arg(m_AF.CDv())+"\n";
    }
    BOppProperties += QString("Cx           = " + fmt).arg(m_AF.Cx())+"\n";
    BOppProperties += QString("Cy           = " + fmt).arg(m_AF.Cy())+"\n";

    strong = QString("Far Field Fx = " + fmt).arg(m_AF.fffx() * q * Units::NtoUnit(), 9,'g',3);
    BOppProperties += strong +forceunit+"\n";
    strong = QString("Far Field Fy = " + fmt).arg(m_AF.fffy() * q * Units::NtoUnit(), 9,'g',3);
    BOppProperties += strong +forceunit+"\n";
    strong = QString("Far Field Fz = " + fmt).arg(m_AF.fffz() * q * Units::NtoUnit(), 9,'g',3);
    BOppProperties += strong +forceunit+"\n";

    strong = QString("Summed Fx    = " + fmt).arg(m_AF.fsumx() * q * Units::NtoUnit(), 9,'g',3);
    BOppProperties += strong +forceunit+"\n";
    strong = QString("Summed Fy    = " + fmt).arg(m_AF.fsumy() * q * Units::NtoUnit(), 9,'g',3);
    BOppProperties += strong +forceunit+"\n";
    strong = QString("Summed Fz    = " + fmt).arg(m_AF.fsumz() * q * Units::NtoUnit(), 9,'g',3);
    BOppProperties += strong +forceunit+"\n";

    strong  = QString("Mx = "+fmt).arg((m_AF.Mi()+m_AF.Mv()).x * q * Units::NmtoUnit(), 9,'g',3);
    BOppProperties += strong +momentunit+"\n";
    strong  = QString("My = "+fmt).arg((m_AF.Mi()+m_AF.Mv()).y * q * Units::NmtoUnit(), 9,'g',3);
    BOppProperties += strong +momentunit+"\n";
    strong  = QString("Mz = "+fmt).arg((m_AF.Mi()+m_AF.Mv()).z * q * Units::NmtoUnit(), 9,'g',3);
    BOppProperties += strong +momentunit;

    if(!bLongOutput) return;
    BOppProperties +="\n";

    strong  = QString("XCE = Mz/Fy ="+fmt).arg((m_AF.Mi()+m_AF.Mv()).z /m_AF.fsumy() * Units::mtoUnit(), 9,'g',3);
    BOppProperties += strong +lenunit+"\n";
    strong  = QString("ZCE = Mx/Fy = "+fmt).arg((m_AF.Mi()+m_AF.Mv()).x /m_AF.fsumy() * Units::mtoUnit(), 9,'g',3);
    BOppProperties += strong +lenunit +"\n";

    for(int is=0; is<m_SailForceFF.size(); is++)
    {
        BOppProperties += pBoat->sailAt(is)->name() + ": \n";
        strong = QString("   Far Field Fx= " + fmt).arg(m_SailForceFF.at(is).x * q * Units::NtoUnit(), 9,'g',3);
        BOppProperties += strong +forceunit+"\n";
        strong = QString("   Far Field Fy= " + fmt).arg(m_SailForceFF.at(is).y * q * Units::NtoUnit(), 9,'g',3);
        BOppProperties += strong +forceunit+"\n";
        strong = QString("   Far Field Fz= " + fmt).arg(m_SailForceFF.at(is).z * q * Units::NtoUnit(), 9,'g',3);
        BOppProperties += strong +forceunit+"\n";
        strong = QString("   Summed Fx   = " + fmt).arg(m_SailForceSum.at(is).x * q * Units::NtoUnit(), 9,'g',3);
        BOppProperties += strong +forceunit+"\n";
        strong = QString("   Summed Fy   = " + fmt).arg(m_SailForceSum.at(is).y * q * Units::NtoUnit(), 9,'g',3);
        BOppProperties += strong +forceunit+"\n";
        strong = QString("   Summed Fz   = " + fmt).arg(m_SailForceSum.at(is).z * q * Units::NtoUnit(), 9,'g',3);
        BOppProperties += strong +forceunit +"\n";
    }
    BOppProperties.chop(1); //last \n
}


Vector3d BoatOpp::windDir() const
{
    return windDirection(0.0, -m_Beta);
}


QString BoatOpp::title(bool bLong) const
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


    strong += QString("%1-").arg(ctrl(), 5,'f',2);

    if(fabs(beta())>PRECISION) strong += QString::fromUtf8("%1°-").arg(beta(), 5,'f',2);

    strong += QString("%1").arg(QInf()*Units::mstoUnit(), 5, 'f', 2);
    strong +=Units::speedUnitLabel();

    return strong;
}



void BoatOpp::exportMainDataToString(Boat const*, QString &poppdata, xfl::enumTextFileType filetype, QString const &textsep) const
{
    QString strange;
    QString title;
    QString len = Units::lengthUnitLabel();
    QString inertia = Units::inertiaUnitLabel();

    QString sep = "  ";
    if(filetype==xfl::CSV) sep = textsep+ " ";

    poppdata += boatName()+"\n";
    poppdata += polarName()+"\n\n";
    poppdata += "ctrl       " + sep
              + "beta       " + sep
              + "phi        " + sep
              + "VInf("+Units::speedUnitLabel()+")" +sep
              + "h("+Units::lengthUnitLabel()+")\n";
    strange = QString::asprintf("%11.5g", m_Ctrl);
    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_Beta);
    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_Phi);
    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_QInf*Units::mstoUnit());
    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_GroundHeight*Units::mtoUnit());
    poppdata += strange;

    poppdata += "\n\n";

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
    poppdata += strange;

    strange = QString::asprintf("%11.5g", m_AF.CL());
    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.Cx());
    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.Cy());
    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.CDi());
    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.CDv());
    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.Cli());
    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.Cmi());
    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.Cmv());
    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.Cni());
    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.Cnv());
    poppdata += strange;

    poppdata += "\n";

    strange = "CP.x("+len+")";      for(int i=strange.length(); i<11; i++) strange+=" ";   poppdata += strange+sep;
    strange = "CP.y("+len+")";      for(int i=strange.length(); i<11; i++) strange+=" ";   poppdata += strange+sep;
    strange = "CP.z("+len+")";      for(int i=strange.length(); i<11; i++) strange+=" ";   poppdata += strange+sep;
    strange = "NP.x("+len+")";      for(int i=strange.length(); i<11; i++) strange+=" ";   poppdata += strange+sep;
    poppdata += "\n";


    strange = QString::asprintf("%11.5g", m_AF.centreOfPressure().x*Units::mtoUnit());    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.centreOfPressure().y*Units::mtoUnit());    poppdata += strange+sep;
    strange = QString::asprintf("%11.5g", m_AF.centreOfPressure().z*Units::mtoUnit());    poppdata += strange+sep;
    poppdata += strange + "\n\n";

    strange = "mass("+Units::massUnitLabel()+")";  for(int i=strange.length(); i<11; i++) strange+=" ";   poppdata += strange+sep;
    strange = "CoG.x("+len+")";                    for(int i=strange.length(); i<11; i++) strange+=" ";   poppdata += strange+sep;
    strange = "CoG.y("+len+")";                    for(int i=strange.length(); i<11; i++) strange+=" ";   poppdata += strange+sep;
    strange = "CoG.z("+len+")";                    for(int i=strange.length(); i<11; i++) strange+=" ";   poppdata += strange+sep;
    strange = "CoG_Ixx("+inertia+")";              for(int i=strange.length(); i<11; i++) strange+=" ";   poppdata += strange+sep;
    strange = "CoG_Iyy("+inertia+")";              for(int i=strange.length(); i<11; i++) strange+=" ";   poppdata += strange+sep;
    strange = "CoG_Izz("+inertia+")";              for(int i=strange.length(); i<11; i++) strange+=" ";   poppdata += strange+sep;
    strange = "CoG_Ixz("+inertia+")";              for(int i=strange.length(); i<11; i++) strange+=" ";   poppdata += strange+sep;
    poppdata +="\n";


    poppdata += strange + "\n\n";


    title = "y("+Units::lengthUnitLabel()+")";  for(int i=title.length(); i<11; i++) title+=" ";   title +=sep;
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
}


void BoatOpp::exportPanel3DataToString(Boat const*pBoat,
                                      xfl::enumTextFileType exporttype,
                                      QString &paneldata) const
{
    QString strong, Format;
    QTextStream out;
    out.setString(&paneldata);

    if(exporttype==xfl::TXT) out << " Panel        CtrlPt.x        CtrlPt.y        CtrlPt.z           Nx               Ny             Nz            Area             Cp\n";
    else                     out << "Panel,CtrlPt.x,CtrlPt.y,CtrlPt.z,Nx,Ny,Nz,Area,Cp\n";

    if(exporttype==xfl::TXT) Format = "%1     %2     %3     %4     %5     %6     %7     %8     %9\n";
    else                     Format = "%1, %2, %3, %4, %5, %6, %7, %8, %9\n";


    for(int iw=0; iw<pBoat->nSails(); iw++)
    {
        Sail const *pSail = pBoat->sailAt(iw);

        out << pSail->name() + "_Cp Coefficients"+"\n";
        int p=pSail->m_FirstPanel3Index;

        for(int i3=0; i3<pSail->nPanel3(); i3++)
        {
            Panel3 const &p3 = pBoat->triMesh().panelAt(p);

            double cp=0;
            for(int in=0; in<3; in++) cp += m_Cp.at(p3.index()*3+in);
            cp /= 3.0;

            strong = QString(Format).arg(p, 6)
                                    .arg(p3.CoG().x,    11)
                                    .arg(p3.CoG().y,    11)
                                    .arg(p3.CoG().z,    11)
                                    .arg(p3.normal().x, 11)
                                    .arg(p3.normal().y, 11)
                                    .arg(p3.normal().z, 11)
                                    .arg(p3.area(),     11)
                                    .arg(cp,            11);

            out << strong;
            p++;
        }

        out << ("\n\n");
    }
}
