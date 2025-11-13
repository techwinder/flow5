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

#define _MATH_DEFINES_DEFINED


#include <QRandomGenerator>

#include <xflobjects/objects3d/plane/plane.h>
#include <xflobjects/objects3d/plane/planestl.h>
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflobjects/objects3d/wing/wingxfl.h>
#include <xflobjects/objects3d/wing/surface.h>

#include <xflobjects/objects3d/wing/surface.h>
#include <xflobjects/objects_globals/objects_global.h>

#include <xflobjects/objects3d/analysis/wpolar.h>
#include <xflobjects/objects3d/analysis/planeopp.h>
#include <xflobjects/objects3d/analysis/wingopp.h>

#include <xflmath/constants.h>
#include <xflmath/mathelem.h>
#include <xflcore/xflcore.h>
#include <xflcore/units.h>

QStringList PlaneOpp::s_POppVariables = { "Local lift coef.", "Local lift C.Cl/M.A.C.",
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


PlaneOpp::PlaneOpp(Plane const *pPlane, WPolar const *pWPolar, int panel4ArraySize, int panel3ArraySize) : Opp3d()
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
        m_WPlrName        = pWPolar->name();
        m_bThinSurface    = pWPolar->bThinSurfaces();
//        m_bTiltedGeom     = pWPolar->isTilted();
        m_PolarType      = pWPolar->type();
        m_AnalysisMethod  = pWPolar->analysisMethod();

        m_theStyle = pWPolar->theStyle();
    }

    memset(m_Is, 0, 9*sizeof(double));

    allocateMemory(panel4ArraySize, panel3ArraySize);
}


void PlaneOpp::setVariableNames(QString const &strForce, QString const &strMoment)
{
    s_POppVariables.clear();
    s_POppVariables << "Local lift coef." << "Local lift C.Cl/M.A.C." <<
                       "Airfoil viscous drag coef." <<"Induced drag coef." <<"Total drag coef." <<
                       "Local drag C.Cd/M.A.C." <<"1/4 chord pitching moment coef." <<
                       "CoG visc. pitching moment coef." <<"CoG pressure pitching moment coef." <<
                       "Reynolds" <<"Top transition x%" <<"Bot. transition x%" <<
                       "Centre of pressure x%" <<
                       "Strip lift (" +strForce+")" <<
                       "Bending moment (" +strMoment+")" <<
                       "aoa+ai+twist" << "Effective aoa" << "Induced angle" <<
                       "Virtual twist" << "Circulation";
}


void PlaneOpp::initialize()
{
    m_PlaneName   = "";
    m_WPlrName    = "";

    m_nPanel4 = m_nPanel3 = 0;

    m_NodeValMin = m_NodeValMax = 0.0;

    m_PolarType     = xfl::T1POLAR;
    m_AnalysisMethod = Polar3d::VLM2;

    m_Mass = 0.0;
    m_CoG.set(0.0,0.0,0.0);
    m_Inertia[0] = m_Inertia[1] = m_Inertia[2] = m_Inertia[3] = 0.0;

    m_theStyle.m_Stipple     = Line::SOLID;
    m_theStyle.m_Width       = 1;
    m_theStyle.m_Symbol  = Line::NOSYMBOL;
    m_theStyle.m_bIsVisible  = true;


    int h = QRandomGenerator::global()->bounded(360);
    int s = QRandomGenerator::global()->bounded(155)+100;
    int v = QRandomGenerator::global()->bounded(155)+100;
    m_theStyle.m_Color.setHsv(h,s,v,255);

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
    memset(m_gamma.data(),     0, uint(m_gamma.size())     * sizeof(double));
    memset(m_sigma.data(), 0, uint(m_sigma.size()) * sizeof(double));
    memset(m_Cp.data(),    0, uint(m_Cp.size())    * sizeof(double));
}


void PlaneOpp::getProperties(Plane const *pPlane, WPolar const *pWPolar, QString &props) const
{
    QString strong, strange;
    QString format = xfl::isLocalized() ? "%L1" : "%1";
    Vector3d WindD = windDirection(alpha(), beta());
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

    strong = QString("Mass  = "+format+ " ").arg(m_Mass*Units::kgtoUnit(), 9, 'f', 3);
    props += strong + Units::massUnitLabel() + "\n";

    strong = QString("CoG_x = "+format+ " ").arg(m_CoG.x*Units::mtoUnit(), 9, 'f', 3);
    strong += Units::lengthUnitLabel() + "\n";
    props += strong;

    strong = QString("CoG_z = "+format+ " ").arg(m_CoG.z*Units::mtoUnit(), 9, 'f', 3);
    strong += Units::lengthUnitLabel() + "\n";
    props += strong + "\n";

    strong = "V" + INFCHAR + QString("    = "+format+ " ").arg(m_QInf*Units::mstoUnit(), 9, 'f', 3);
    props += strong + Units::speedUnitLabel()+"\n";

    strong = ALPHACHAR + QString("     = "+format).arg(m_Alpha, 9, 'f', 3);
    props += strong +  DEGCHAR +"\n";

    if(fabs(m_Beta)>ANGLEPRECISION)
    {
        strong = QString("Beta  = "+format).arg(m_Beta, 9,'f',3);
        props += strong + DEGCHAR +"\n";
    }
    props += "\n";

    if(isType7())
    {
        strong  = QString("Control value = " + format).arg(m_Ctrl, 9, 'f', 3);
        props += strong +"\n";
        strong  = QString("XNP = "+format).arg(m_SD.XNP*Units::mtoUnit());
        props += "\n"+strong +" " +Units::lengthUnitLabel()+"\n";

        strong = QString("Static margin = "+format+"%").arg((m_SD.XNP-m_CoG.x)/pWPolar->referenceChordLength()*100.0);
        props += strong + "\n";
    }

    if(xfl::isLocalized())
        strong = QString("CP = (%L1; %L2; %L3) ").arg(m_AF.centreOfPressure().x*Units::mtoUnit(), 0, 'g', 3)
                                                 .arg(m_AF.centreOfPressure().y*Units::mtoUnit(), 0, 'g', 3)
                                                 .arg(m_AF.centreOfPressure().z*Units::mtoUnit(), 0, 'g', 3);
    else
        strong = QString::asprintf("CP = (%.3g; %.3g; %.3g) ", m_AF.centreOfPressure().x*Units::mtoUnit(),
                                                               m_AF.centreOfPressure().y*Units::mtoUnit(),
                                                               m_AF.centreOfPressure().z*Units::mtoUnit());

    props += strong +Units::lengthUnitLabel()+"\n\n";

    strong  = QString("CL  = " + format).arg(m_AF.CL(), 13,'f',7);
    props += strong +"\n";
    strong  = QString("CD  = " + format).arg(m_AF.CD(), 13,'f',7);
    props += strong +"\n";
    strong  = QString("VCD = " + format).arg(m_AF.CDv(),13,'f',7);
    props += strong +"\n";
    strong  = QString("ICD = " + format).arg(m_AF.CDi(),13,'f',7);
    props += strong +"\n";

    strong  = QString("CY  = " + format).arg(m_AF.Cy(), 13,'f',7);
    props += strong +"\n";

    strong  = QString("Cl  = " + format).arg(m_AF.Cli(),13,'f',7);
    props += strong +"\n";

    strong  = QString("Cm  = " + format).arg(m_AF.Cm(), 13,'f',7);
    props += strong +"\n";
    strong  = QString("Cmi = " + format).arg(m_AF.Cmi(),13,'f',7);
    props += strong +"\n";
    strong  = QString("Cmv = " + format).arg(m_AF.Cmv(),13,'f',7);
    props += strong +"\n";

    strong  = QString("Cn  = " + format).arg(m_AF.Cn(), 13,'f',7);
    props += strong +"\n";
    strong  = QString("Cni = " + format).arg(m_AF.Cni(),13,'f',7);
    props += strong +"\n";
    strong  = QString("Cnv = " + format).arg(m_AF.Cnv(),13,'f',7);
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
        if(xfl::isLocalized())
            strong = QString("   Fx=%L1 %2  Mx =%L3 %4")
                             .arg(fx,13,'f',7)
                             .arg(Units::forceUnitLabel())
                             .arg(mx,13,'f',7)
                             .arg(Units::momentUnitLabel());
        else
            strong = QString::asprintf("   Fx=%9.3g %s  Mx =%9.3g %s", fx, Units::forceUnitLabel().toStdString().c_str(), mx, Units::momentUnitLabel().toStdString().c_str());
        props += strong + "\n";

        double fy = m_AF.fffy() + drag.y;
        double my = m_AF.Mi().y + m_AF.Mv().y;
        fy *= qDyn * Units::NtoUnit();
        my *= qDyn * Units::NmtoUnit();
        if(xfl::isLocalized())
            strong = QString("   Fy=%L1 %2  My =%L3 %4")
                             .arg(fy,13,'f',7)
                             .arg(Units::forceUnitLabel())
                             .arg(my,13,'f',7)
                             .arg(Units::momentUnitLabel());
        else
            strong = QString::asprintf("   Fy=%9.3g %s  My =%9.3g %s", fy, Units::forceUnitLabel().toStdString().c_str(), my, Units::momentUnitLabel().toStdString().c_str());
        props += strong + "\n";

        double fz = m_AF.fffz() + drag.z;
        double mz = m_AF.Mi().z + m_AF.Mv().z;
        fz *= qDyn * Units::NtoUnit();
        mz *= qDyn * Units::NmtoUnit();
        if(xfl::isLocalized())
            strong = QString("   Fz=%L1 %2  Mz =%L3 %4")
                             .arg(fz,13,'f',7)
                             .arg(Units::forceUnitLabel())
                             .arg(mz,13,'f',7)
                             .arg(Units::momentUnitLabel());
        else
            strong = QString::asprintf("   Fz=%9.3g %s  Mz =%9.3g %s", fz, Units::forceUnitLabel().toStdString().c_str(), mz, Units::momentUnitLabel().toStdString().c_str());
        props += strong + "\n\n";

        props += "Forces on parts, body axes:\n";
        for(int iw=0; iw<m_WingOpp.size(); iw++)
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
                if(xfl::isLocalized())
                    strong = QString("   Fx=%L1 %2  Mx =%L3 %4")
                                     .arg(fx,13,'f',7)
                                     .arg(Units::forceUnitLabel())
                                     .arg(mx,13,'f',7)
                                     .arg(Units::momentUnitLabel());
                else
                    strong = QString::asprintf("   Fx=%9.3g %s  Mx =%9.3g %s", fx, Units::forceUnitLabel().toStdString().c_str(), mx, Units::momentUnitLabel().toStdString().c_str());
                props += strong + "\n";

                double fy = pWOpp->m_AF.fffy() + drag.y;
                double my = pWOpp->m_AF.Mi().y + pWOpp->m_AF.Mv().y;
                fy *= qDyn * Units::NtoUnit();
                my *= qDyn * Units::NmtoUnit();
                if(xfl::isLocalized())
                    strong = QString("   Fy=%L1 %2  My =%L3 %4")
                                     .arg(fy,13,'f',7)
                                     .arg(Units::forceUnitLabel())
                                     .arg(my,13,'f',7)
                                     .arg(Units::momentUnitLabel());
                else
                    strong = QString::asprintf("   Fy=%9.3g %s  My =%9.3g %s", fy, Units::forceUnitLabel().toStdString().c_str(), my, Units::momentUnitLabel().toStdString().c_str());
                props += strong + "\n";

                double fz = pWOpp->m_AF.fffz() + drag.z;
                double mz = pWOpp->m_AF.Mi().z + pWOpp->m_AF.Mv().z;
                fz *= qDyn * Units::NtoUnit();
                mz *= qDyn * Units::NmtoUnit();
                if(xfl::isLocalized())
                    strong = QString("   Fz=%L1 %2  Mz =%L3 %4")
                                     .arg(fz,13,'f',7)
                                     .arg(Units::forceUnitLabel())
                                     .arg(mz,13,'f',7)
                                     .arg(Units::momentUnitLabel());
                else
                    strong = QString::asprintf("   Fz=%9.3g %s  Mz =%9.3g %s", fz, Units::forceUnitLabel().toStdString().c_str(), mz, Units::momentUnitLabel().toStdString().c_str());
                props += strong + "\n";
            }
        }

        for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++)
        {
            if(m_FuseAF.size()<=ifuse) break;  // FuseAF not defined if LLT

            Vector3d drag = WindD * m_FuseAF.at(ifuse).viscousDrag();
            Fuse const *pFuse = pPlaneXfl->fuseAt(ifuse);
            props += "  " + pFuse->name() + ":\n";
            double fx = m_FuseAF.at(ifuse).fffx() +drag.x;
            double mx = m_FuseAF.at(ifuse).Mi().x + m_FuseAF.at(ifuse).Mv().x;
            fx *= qDyn * Units::NtoUnit();
            mx *= qDyn * Units::NmtoUnit();
            if(xfl::isLocalized())
                strong = QString("   Fx=%L1 %2  Mx =%L3 %4")
                                 .arg(fx,13,'f',7)
                                 .arg(Units::forceUnitLabel())
                                 .arg(mx,13,'f',7)
                                 .arg(Units::momentUnitLabel());
            else
                strong = QString::asprintf("   Fx=%9.3g %s  Mx =%9.3g %s", fx, Units::forceUnitLabel().toStdString().c_str(), mx, Units::momentUnitLabel().toStdString().c_str());
            props += strong + "\n";

            double fy = m_FuseAF.at(ifuse).fffx() + drag.y;
            double my = m_FuseAF.at(ifuse).Mi().y + m_FuseAF.at(ifuse).Mv().y;
            fy *= qDyn * Units::NtoUnit();
            my *= qDyn * Units::NmtoUnit();
            if(xfl::isLocalized())
                strong = QString("   Fy=%L1 %2  My =%L3 %4")
                                 .arg(fy,13,'f',7)
                                 .arg(Units::forceUnitLabel())
                                 .arg(my,13,'f',7)
                                 .arg(Units::momentUnitLabel());
            else
                strong = QString::asprintf("   Fy=%9.3g %s  My =%9.3g %s", fy, Units::forceUnitLabel().toStdString().c_str(), my, Units::momentUnitLabel().toStdString().c_str());
            props += strong + "\n";

            double fz = m_FuseAF.at(ifuse).fffx() + drag.z;
            double mz = m_FuseAF.at(ifuse).Mi().z + m_FuseAF.at(ifuse).Mv().z;
            fz *= qDyn * Units::NtoUnit();
            mz *= qDyn * Units::NmtoUnit();
            if(xfl::isLocalized())
                strong = QString("   Fz=%L1 %2  Mz =%L3 %4")
                                 .arg(fz,13,'f',7)
                                 .arg(Units::forceUnitLabel())
                                 .arg(mz,13,'f',7)
                                 .arg(Units::momentUnitLabel());
            else
                strong = QString::asprintf("   Fz=%9.3g %s  Mz =%9.3g %s", fz, Units::forceUnitLabel().toStdString().c_str(), mz, Units::momentUnitLabel().toStdString().c_str());
            props += strong + "\n";
        }
    }

    bool bFlaps=0;
    for(int iw=0; iw<m_WingOpp.size(); iw++)
    {
        if(m_WingOpp.at(iw).m_FlapMoment.size())    bFlaps=1;
    }

    if(bFlaps)
    {
        props += "\nFlap Moments\n";
        QString str;
        Units::getMomentUnitLabel(str);
        for(int iwo=0; iwo<m_WingOpp.size(); iwo++)
        {
            props += "  " + WOpp(iwo).wingName() +"\n";
            for(int i=0; i<WOpp(iwo).m_nFlaps; i++)
            {
                strange = QString::asprintf("    Flap_%d = %8.4f ", i+1, WOpp(iwo).m_FlapMoment[i]*Units::NmtoUnit());
                props += strange + Units::momentUnitLabel() + "\n";
            }
        }
    }


    props += "\n";

    if(isType12358() || isType7())
    {
        props += "\n";
        props += "Non-dimensional stability derivatives:\n";
        props += QString::asprintf("  CXu = %11g\n", m_SD.CXu);
        props += QString::asprintf("  CZu = %11g\n", m_SD.CZu);
        props += QString::asprintf("  Cmu = %11g\n", m_SD.Cmu);
        props += QString::asprintf("  CXa = %11g\n", m_SD.CXa);
        props += QString::asprintf("  CZa = %11g\n", m_SD.CZa);
        props += QString::asprintf("  Cma = %11g\n", m_SD.Cma);
        props += QString::asprintf("  CXq = %11g\n", m_SD.CXq);
        props += QString::asprintf("  CZq = %11g\n", m_SD.CZq);
        props += QString::asprintf("  Cmq = %11g\n", m_SD.Cmq);
        props += QString::asprintf("  CYb = %11g\n", m_SD.CYb);
        props += QString::asprintf("  Clb = %11g\n", m_SD.Clb);
        props += QString::asprintf("  Cnb = %11g\n", m_SD.Cnb);
        props += QString::asprintf("  CYp = %11g\n", m_SD.CYp);
        props += QString::asprintf("  Clp = %11g\n", m_SD.Clp);
        props += QString::asprintf("  Cnp = %11g\n", m_SD.Cnp);
        props += QString::asprintf("  CYr = %11g\n", m_SD.CYr);
        props += QString::asprintf("  Clr = %11g\n", m_SD.Clr);
        props += QString::asprintf("  Cnr = %11g\n", m_SD.Cnr);
        props += "\n";

        if(m_SD.ControlNames.size())
        {
            props += "Non-dimensional control derivatives:\n";
            for(int i=0; i<m_SD.ControlNames.size(); i++)
            {
                props += "  " + m_SD.ControlNames.at(i) + "\n";
                props += QString::asprintf("    CXd = %11g\n", m_SD.CXe.at(i));
                props += QString::asprintf("    CYd = %11g\n", m_SD.CYe.at(i));
                props += QString::asprintf("    CZd = %11g\n", m_SD.CZe.at(i));
                props += QString::asprintf("    Cld = %11g\n", m_SD.CLe.at(i));
                props += QString::asprintf("    Cmd = %11g\n", m_SD.CMe.at(i));
                props += QString::asprintf("    Cnd = %11g\n", m_SD.CNe.at(i));
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
            modeProperties(c, OmegaN, Omega1, Dsi);

            if(c.imag()>=0.0) strange = "  " + LAMBDACHAR + QString::asprintf(" = %9.4g + %9.4gi", c.real(), c.imag());
            else              strange = "  " + LAMBDACHAR + QString::asprintf(" = %9.4g - %9.4gi", c.real(), qAbs(c.imag()));
            props += strange +"\n";

/*            QString str = QString::asprintf("  Eigenvector: %9.4g + %9.4gi\n"
                                            "               %9.4g + %9.4gi\n"
                                            "               %9.4g + %9.4gi\n"
                                            "               %9.4g + %9.4gi\n",
                    m_EigenVector[im][0].real(),  m_EigenVector[im][0].imag(),
                    m_EigenVector[im][1].real(),  m_EigenVector[im][1].imag(),
                    m_EigenVector[im][2].real(),  m_EigenVector[im][2].imag(),
                    m_EigenVector[im][3].real(),  m_EigenVector[im][3].imag());
            props += str + EOLCHAR;*/

            strange = QString::asprintf("  F (natural)  = %9.3f Hz", OmegaN/2.0/PI);
            props += strange +"\n";

            strange = QString::asprintf("  F (damped)   = %9.3f Hz", Omega1/2.0/PI);
            props += strange +"\n";

            strange = "  " + XICHAR + QString::asprintf("            = %9.3f ", Dsi);
            props += strange +"\n";

            props += "  Normalized eigenvector:\n";
            angle = m_EigenVector[im][3];
            c = m_EigenVector[im][0]/u0;
            if(c.imag()>=0.0) strange = QString::asprintf("    u/u0          = %9.4g + %9.4gi", c.real(), c.imag());
            else              strange = QString::asprintf("    u/u0          = %9.4g - %9.4g", c.real(), qAbs(c.imag()));
            props += strange +"\n";

            c = m_EigenVector[im][1]/u0;
            if(c.imag()>=0.0) strange = QString::asprintf("    w/u0          = %9.4g + %9.4gi",c.real(),c.imag());
            else              strange = QString::asprintf("    w/u0          = %9.4g - %9.4gi",c.real(),qAbs(c.imag()));
            props += strange +"\n";

            c = m_EigenVector[im][2]/(2.0*u0/mac);
            if(c.imag()>=0.0) strange = QString::asprintf("    q/(2.u0.MAC)  = %9.4g + %9.4gi", c.real(), c.imag());
            else              strange = QString::asprintf("    q/(2.u0.MAC)  = %9.4g - %9.4gi", c.real(), qAbs(c.imag()));
            props += strange +"\n";

            c = m_EigenVector[im][3]/angle;
            if(c.imag()>=0.0) strange = "    " + THETACHAR + QString::asprintf(" (rad)       = %9.4g + %9.4gi", c.real(), c.imag());
            else              strange = "    " + THETACHAR + QString::asprintf(" (rad)       = %9.4g - %9.4gi", c.real(), qAbs(c.imag()));
            props += strange +"\n\n";
        }

        props += "\nLateral modes:\n";
        for(int im=4; im<8; im++)
        {
            c = m_EigenValue[im];
            modeProperties(c, OmegaN, Omega1, Dsi);

            if(c.imag()>=0.0) strange = "  " + LAMBDACHAR + QString::asprintf(" = %9.4g + %9.4gi", c.real(), c.imag());
            else              strange = "  " + LAMBDACHAR + QString::asprintf(" = %9.4g - %9.4gi", c.real(), qAbs(c.imag()));
            props += strange +"\n";
/*            QString str = QString::asprintf("  Eigenvector: %9.4g + %9.4gi\n"
                                            "               %9.4g + %9.4gi\n"
                                            "               %9.4g + %9.4gi\n"
                                            "               %9.4g + %9.4gi\n",
                    m_EigenVector[im][0].real(),  m_EigenVector[im][0].imag(),
                    m_EigenVector[im][1].real(),  m_EigenVector[im][1].imag(),
                    m_EigenVector[im][2].real(),  m_EigenVector[im][2].imag(),
                    m_EigenVector[im][3].real(),  m_EigenVector[im][3].imag());
            props += str + EOLCHAR; */

            strange = QString::asprintf("  F (natural)  = %9.3f Hz", OmegaN/2.0/PI);
            props += strange +"\n";

            strange = QString::asprintf("  F (damped)   = %9.3f Hz", Omega1/2.0/PI);
            props += strange +"\n";

            strange = "  " + XICHAR + QString::asprintf("            = %9.3f ", Dsi);
            props += strange +"\n";

            if(fabs(c.real())>PRECISION && fabs(c.imag())<PRECISION)
            {
                strange = QString::asprintf(    "  Time to double = %8.3f s", log(2)/fabs(c.real()));
                props += strange +"\n";
                if(c.real()<0.0)
                {
                    strange = QString::asprintf("  Time constant  = %8.3f", -1.0/c.real());
                    props += strange +"\n";
                }
            }

            props += "  Normalized Eigenvector:\n";

            angle = m_EigenVector[im][3];

            c = m_EigenVector[im][0]/u0;
            if(c.imag()>=0.0) strange = QString::asprintf("    v/u0          = %9.4g + %9.4gi", c.real(), c.imag());
            else              strange = QString::asprintf("    v/u0          = %9.4g - %9.4g",  c.real(), qAbs(c.imag()));
            props += strange +"\n";

            c = m_EigenVector[im][1]/(2.0*u0/span);
            if(c.imag()>=0.0) strange = QString::asprintf("    p/(2.u0.Span) = %9.4g + %9.4gi", c.real(), c.imag());
            else              strange = QString::asprintf("    p/(2.u0.Span) = %9.4g - %9.4g",  c.real(), qAbs(c.imag()));
            props += strange +"\n";

            c = m_EigenVector[im][2]/(2.0*u0/span);
            if(c.imag()>=0.0) strange = QString::asprintf("    r/(2.u0.Span) = %9.4g + %9.4gi", c.real(), c.imag());
            else              strange = QString::asprintf("    r/(2.u0.Span) = %9.4g - %9.4g",  c.real(), qAbs(c.imag()));
            props += strange +"\n";

            c = m_EigenVector[im][3]/angle;
            if(c.imag()>=0.0) strange = "    " + PHICHAR + QString::asprintf(" (rad)       = %9.4g + %9.4gi", c.real(), c.imag());
            else              strange = "    " + PHICHAR + QString::asprintf(" (rad)       = %9.4g - %9.4g",  c.real(), qAbs(c.imag()));
            props += strange +"\n\n";
        }
    }

    if(isTriLinearMethod())
    {
        strange = QString::asprintf("Nodes values = %d", int(m_NodeValue.size()));
        props += strange;
    }
    else if(isTriUniformMethod())
    {
        strange = QString::asprintf("Panel values = %d", m_nPanel3);
        props += strange;
    }
    else if(isQuadMethod())
    {
        strange = QString::asprintf("Panel values = %d", m_nPanel4);
        props += strange;
    }

    if(m_Vorton.size())
    {
        strange = QString::asprintf("Vortons: %d rows x %d columns", int(m_Vorton.size()), int(m_Vorton.first().size()));
        props += "\n" + strange;
    }
}


QString PlaneOpp::name() const
{
    QString strange;
    QString format = xfl::isLocalized() ? "%L1" : "%1";

    if(isType8())
    {
        strange  = QString(format).arg(alpha(), 0, 'f', 2) + DEGCHAR + " ";
        strange += QString(format).arg(beta(),  0, 'f', 2) + DEGCHAR + " ";
        strange += QString(format).arg(QInf()*Units::mstoUnit(), 0, 'f', 2) + " " +Units::speedUnitLabel();
    }
    else if(isType7()) strange = QString(format).arg(ctrl(),  7, 'f', 3);
    else if(isType6()) strange = QString(format).arg(ctrl(),  7, 'f', 3);
    else if(isType5()) strange = QString(format).arg(beta(),  7, 'f', 3) + DEGCHAR;
    else               strange = QString(format).arg(alpha(), 7, 'f', 3) + DEGCHAR;

    return strange;
}


QString PlaneOpp::title(bool bLong) const
{
    QString strange;

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

    if(isType7())  strange += QString("ctrl=%1-").arg(ctrl());

    strange += QString::asprintf("%5.2f", m_Alpha) + QString(DEGCHAR) + "_";
    if(fabs(m_Beta)>ANGLEPRECISION)  strange += QString::asprintf("%5.2f", m_Beta) + QString(DEGCHAR) + "_";
    strange += QString::asprintf("%5.2f", QInf()*Units::mstoUnit()) + Units::speedUnitLabel();

    return strange;
}


bool PlaneOpp::serializePOppXFL(QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    int k(0), n(0);
    float f0(0), f1(0), f2(0);
    double dble(0), dbl1(0), dbl2(0);

    int ArchiveFormat=200002;

    if(bIsStoring)
    {
        //using fl5 format instead
    }
    else
    {
        ar >> ArchiveFormat;
        if (ArchiveFormat<200000 || ArchiveFormat>200003 ) return false;

        n=4;
        m_WingOpp.clear();
        m_WingOpp.resize(n);

        ar >> m_PlaneName;
        ar >> m_WPlrName;
        if(ArchiveFormat<200002)
        {
            ar >> k; m_theStyle.m_Stipple=LineStyle::convertLineStyle(k);
            ar >> m_theStyle.m_Width;
            ar >> m_theStyle.m_Color;
            ar >> m_theStyle.m_bIsVisible >> boolean;
        }
        else m_theStyle.serializeXfl(ar, bIsStoring);

        ar >> m_bOut;
        ar >> boolean;

        ar >> m_bThinSurface >> boolean; //m_bTiltedGeom;

        ar >> n;
        if(n==1)      m_PolarType=xfl::T1POLAR;
        else if(n==2) m_PolarType=xfl::T2POLAR;
        else if(n==4) m_PolarType=xfl::T4POLAR;
        else if(n==5) m_PolarType=xfl::T5POLAR;
        else if(n==6) m_PolarType=xfl::T6POLAR;
        else if(n==7) m_PolarType=xfl::T7POLAR;

        ar >> n;
        if     (n==1) m_AnalysisMethod=Polar3d::LLT;
        else if(n==2)
        {
            if(boolean)  m_AnalysisMethod=Polar3d::VLM1;
            else         m_AnalysisMethod=Polar3d::VLM2;
        }
        else if(n==2) m_AnalysisMethod=Polar3d::QUADS;
        ar >> k;
        if(isTriangleMethod())   m_nPanel3 = k;
        else if (isQuadMethod()) m_nPanel4 = k;
        ar >> n;
        ar >> m_Alpha >> m_QInf;
        ar >> m_Beta;
        ar >> m_Ctrl;

        ar >> m_Mass;

        /*        if(m_AnalysisMethod!=Polar3d::LLTMETHOD)
                {
                        for (k=0; k<nPanels; k++)
                        {
                                ar >> f0 >> f1 >> f2;
                                m_dCp[k]    = (double)f0;
                                m_dSigma[k] = (double)f1;
                                m_dG[k]     = (double)f2;
                        }
                }*/
        if(isQuadMethod())
        {
            m_Cp.resize(m_nPanel4);
            m_sigma.resize(m_nPanel4);
            m_gamma.resize(m_nPanel4);
            for (k=0; k<m_nPanel4; k++)
            {
                ar >> f0 >> f1 >> f2;
                m_Cp[k]    = double(f0);
                m_sigma[k] = double(f1);
                m_gamma[k]     = double(f2);
            }
        }
        else if (isTriangleMethod())
        {
            int N =  3*m_nPanel3;
            m_Cp.resize(N);
            m_gamma.resize(N);
            m_sigma.resize(m_nPanel3);
            for (k=0; k<N; k++)
            {
                ar >> f0;
                m_Cp[k] = double(f0);
            }
            for (k=0; k<N; k++)
            {
                ar >> f0;
                m_gamma[k] = double(f0);
            }
            for (k=0; k<m_nPanel3; k++)
            {
                ar >> f0;
                m_sigma[k] = double(f0);
            }
        }


        int pos = 0;
        for(int iw=0; iw<m_WingOpp.size(); iw++)
        {
            ar >> n;

            if(n)
            {
                m_WingOpp[iw].serializeWingOppXFL(ar, bIsStoring);

                m_WingOpp[iw].m_dCp    = m_Cp.data()    + pos;
                m_WingOpp[iw].m_dG     = m_gamma.data()     + pos;
                m_WingOpp[iw].m_dSigma = m_sigma.data() + pos;
                pos +=m_WingOpp[iw].m_nPanel4;
            }

        }


        ar >> dble >> dbl1 >> dbl2;
        ar >> dble >> dble >> dble >> dble;
        ar >> dble;
        ar >> dble >> dbl1 >> dbl2;
        ar >> dble >> dbl1 >> dbl2;

        ar >> m_SD.CXa >> m_SD.CXq >> m_SD.CXu >> m_SD.CZu >> m_SD.Cmu;
        ar >> m_SD.CZa >> m_SD.CZq >> m_SD.Cma >> m_SD.Cmq;
        ar >> m_SD.CYb >> m_SD.CYp >> m_SD.CYr >> m_SD.Clb >> m_SD.Clp >> m_SD.Clr >> m_SD.Cnb >> m_SD.Cnp >> m_SD.Cnr;

        ar >> n;
        m_SD.resizeControlDerivatives(1);
        ar >> m_SD.CXe.first() >> m_SD.CYe.first() >> m_SD.CZe.first();
        ar >> m_SD.CLe.first() >> m_SD.CMe.first() >> m_SD.CNe.first();

        m_BLat.resize(1);
        m_BLong.resize(1);
        m_BLat.first().resize(4);
        m_BLong.first().resize(4);
        ar >> m_BLat[0][0] >> m_BLat[0][1] >> m_BLat[0][2] >> m_BLat[0][3];
        ar >> m_BLong[0][0]>> m_BLong[0][1]>> m_BLong[0][2]>> m_BLong[0][3];

        for(k=0; k<4; k++)
        {
            ar >> m_ALong[k][0]>> m_ALong[k][1]>> m_ALong[k][2]>> m_ALong[k][3];
            ar >> m_ALat[k][0] >> m_ALat[k][1] >> m_ALat[k][2] >> m_ALat[k][3];
        }

        ar >> dble; // formerly m_XNP
//        if(m_WPolarType!=Xfl::STABILITYPOLAR) m_XNP = 0.0;

        for(int kv=0; kv<8;kv++)
        {
            ar >> dbl1 >> dbl2;
            m_EigenValue[kv] = std::complex<double>(dbl1, dbl2);

            for(int lv=0; lv<4; lv++)
            {
                ar >> dbl1 >> dbl2;
                m_EigenVector[kv][lv] = std::complex<double>(dbl1, dbl2);
            }
        }

        // space allocation
        for (int i=0; i<17; i++) ar >> k;
        int n3,n4;
        ar >> n3 >> n4;
        if (ArchiveFormat==200002)
        {
            m_nPanel3 = n3;
            m_nPanel4 = n4;
        }

        ar >> k; m_theStyle.m_Symbol=LineStyle::convertSymbol(k);

        ar>>m_MAChord>>m_Span;

        double real=0.0, imag=0.0;
        ar >> real >> imag;
        m_phiPH = std::complex<double>(real, imag);
        ar >> real >> imag;
        m_phiDR = std::complex<double>(real, imag);

        for (int i=6; i<50; i++) ar >> dble;
    }
    return true;
}


bool PlaneOpp::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    int nIntSpares(0);
    int nDbleSpares(0);
    bool boolean(false);
    int k(0), n(0);
    float f0(0), f1(0), f2(0);

    double dble(0), dbl1(0), dbl2(0);

    // 500001: new fl5 format
    // 500002: moved StabilityDerivative serialization to separate class
    // 500011: changed WingOpp/spandistrib format
    // 500012: added vorton serialization in beta 12
    // 500013: added ground props in beta 13
    // 500014: beta 18: added multiple control matrices
    // 500015: beta 18: Modified the format of AeroForces serialization
    // 500016: v7.21: Addded free surface effect
    int ArchiveFormat = 500016;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << int(m_WingOpp.size());

        ar << m_PlaneName;
        ar << m_WPlrName;

        ar << LineStyle::convertLineStyle(m_theStyle.m_Stipple);
        ar << m_theStyle.m_Width;
        ar << LineStyle::convertSymbol(m_theStyle.m_Symbol);
        ar << m_theStyle.m_Color;
        ar << m_theStyle.m_bIsVisible << false;

        ar <<m_nPanel3 << m_nPanel4;
        ar << m_bOut;
        ar << boolean;

        ar << m_bThinSurface << boolean; //m_bTiltedGeom;

        if     (m_PolarType==xfl::T1POLAR) ar<<1;
        else if(m_PolarType==xfl::T2POLAR) ar<<2;
        else if(m_PolarType==xfl::T4POLAR) ar<<4;
        else if(m_PolarType==xfl::T5POLAR) ar<<5;
        else if(m_PolarType==xfl::T6POLAR) ar<<6;
        else if(m_PolarType==xfl::T7POLAR) ar<<7;
        else if(m_PolarType==xfl::T8POLAR) ar<<100;
        else                                ar << 1;

        if     (m_AnalysisMethod==Polar3d::LLT)        ar<<1;
        else if(m_AnalysisMethod==Polar3d::VLM1)       ar<<2;
        else if(m_AnalysisMethod==Polar3d::VLM2)       ar<<3;
        else if(m_AnalysisMethod==Polar3d::QUADS)      ar<<4;
        else if(m_AnalysisMethod==Polar3d::TRILINEAR)  ar<<5;
        else if(m_AnalysisMethod==Polar3d::TRIUNIFORM) ar<<6;
        else                                       ar<<0;

        if(isQuadMethod())          ar<<m_nPanel4;
        else if(isTriangleMethod()) ar<<m_nPanel3;
        else                        ar<<0;

        ar << n; // m_NStations
        ar << m_Alpha << m_QInf;
        ar << m_Beta;
        ar << m_Ctrl;

        ar << m_MAChord<<m_Span;
        ar << m_Mass;
        ar << m_CoG.x << m_CoG.z;
        ar << m_Inertia[0] <<m_Inertia[1] << m_Inertia[2] << m_Inertia[3];

        ar << m_bGround << m_bFreeSurface << m_GroundHeight;

        if(isQuadMethod())
        {
            for (k=0; k<m_nPanel4; k++) ar<<float(m_Cp.at(k))<<float(m_sigma.at(k))<<float(m_gamma.at(k));
        }
        else if (isTriangleMethod())
        {
            int N3 = 3*m_nPanel3;
            for (k=0; k<N3; k++) ar<<float(m_Cp.at(k));
            for (k=0; k<N3; k++) ar<<float(m_gamma.at(k));
            for (k=0; k<m_nPanel3; k++) ar<<float(m_sigma.at(k));
        }


        for(int iw=0; iw<m_WingOpp.size(); iw++)
        {
            m_WingOpp[iw].serializeWingOppFl5(ar, bIsStoring);
        }

        m_AF.serializeFl5(ar, bIsStoring);

        ar << int(m_FuseAF.size());
        for(int ifuse=0; ifuse<m_FuseAF.size(); ifuse++)
            m_FuseAF[ifuse].serializeFl5(ar, bIsStoring);
/*
        ar << m_SD.CXa << m_SD.CXq << m_SD.CXu << m_SD.CZu <<m_SD.Cmu;
        ar << m_SD.CLa << m_SD.CLq << m_SD.Cma << m_SD.Cmq;
        ar << m_SD.CYb << m_SD.CYp << m_SD.CYr << m_SD.Clb << m_SD.Clp << m_SD.Clr << m_SD.Cnb << m_SD.Cnp << m_SD.Cnr;
        ar << m_SD.CXe << m_SD.CYe << m_SD.CZe;
        ar << m_SD.CLe << m_SD.CMe << m_SD.CNe;*/

        m_SD.serializeFl5(ar, bIsStoring);

        ar <<int(m_BLat.size());
        for(int ie=0; ie<m_BLat.size(); ie++)
        {
            for(int j=0; j<4; j++)
            {
                ar << m_BLat.at(ie).at(j);
                ar << m_BLong.at(ie).at(j) ;
            }
        }

        for(k=0; k<4; k++)
        {
            ar << m_ALong[k][0]<< m_ALong[k][1]<< m_ALong[k][2]<< m_ALong[k][3];
            ar << m_ALat[k][0] << m_ALat[k][1] << m_ALat[k][2] << m_ALat[k][3];
        }


        ar << m_Phi; // repurposing

        for(int kv=0; kv<8;kv++)
        {
            ar << m_EigenValue[kv].real() << m_EigenValue[kv].imag();
            for(int lv=0; lv<4; lv++)
            {
                ar << m_EigenVector[kv][lv].real() << m_EigenVector[kv][lv].imag();
            }
        }

        ar << m_phiPH.real() << m_phiPH.imag();
        ar << m_phiDR.real() << m_phiDR.imag();

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

        ar << 0;
        ar << 0;
    }
    else
    {
        ar >> ArchiveFormat;
        if (ArchiveFormat<500001 || ArchiveFormat>500030) return false;

        ar >> n;
        m_WingOpp.clear();
        m_WingOpp.resize(n);

        ar >> m_PlaneName;
        ar >> m_WPlrName;

        ar >> k; m_theStyle.m_Stipple=LineStyle::convertLineStyle(k);
        ar >> m_theStyle.m_Width;
        ar >> k; m_theStyle.m_Symbol=LineStyle::convertSymbol(k);
        ar >> m_theStyle.m_Color;
        ar >> m_theStyle.m_bIsVisible >> boolean;

        ar >> m_nPanel3 >> m_nPanel4;
        ar >> m_bOut;
        ar >> boolean;

        ar >> m_bThinSurface >> boolean; //m_bTiltedGeom;

        ar >> n;
        if     (n==1)   m_PolarType=xfl::T1POLAR;
        else if(n==2)   m_PolarType=xfl::T2POLAR;
        else if(n==4)   m_PolarType=xfl::T4POLAR;
        else if(n==5)   m_PolarType=xfl::T5POLAR;
        else if(n==6)   m_PolarType=xfl::T6POLAR;
        else if(n==7)   m_PolarType=xfl::T7POLAR;
        else if(n==100) m_PolarType=xfl::T8POLAR;

        ar >> n;
        if     (n==1) m_AnalysisMethod=Polar3d::LLT;
        else if(n==2) m_AnalysisMethod=Polar3d::VLM1;
        else if(n==3) m_AnalysisMethod=Polar3d::VLM2;
        else if(n==4) m_AnalysisMethod=Polar3d::QUADS;
        else if(n==5) m_AnalysisMethod=Polar3d::TRILINEAR;
        else if(n==6) m_AnalysisMethod=Polar3d::TRIUNIFORM;
        ar >> k;
        if(isTriangleMethod())   m_nPanel3 = k;
        else if (isQuadMethod()) m_nPanel4 = k;
        ar >> k; //m_NStations;
        ar >> m_Alpha >> m_QInf;
        ar >> m_Beta;
        ar >> m_Ctrl;

        ar >> m_MAChord>>m_Span;

        ar >> m_Mass;
        ar >> m_CoG.x >> m_CoG.z;
        ar >> m_Inertia[0] >> m_Inertia[1] >> m_Inertia[2] >> m_Inertia[3];

        if(ArchiveFormat>=500013)
        {
            ar >> m_bGround;
            if(ArchiveFormat>=500016) ar >> m_bFreeSurface;
            ar >> m_GroundHeight;
        }

        if(isQuadMethod())
        {
            m_Cp.resize(m_nPanel4);
            m_sigma.resize(m_nPanel4);
            m_gamma.resize(m_nPanel4);
            for (k=0; k<m_nPanel4; k++)
            {
                ar >> f0 >> f1 >> f2;
                m_Cp[k]    = double(f0);
                m_sigma[k] = double(f1);
                m_gamma[k]     = double(f2);
            }
        }
        else if (isTriangleMethod())
        {
            int N =  3*m_nPanel3;
            m_Cp.resize(N);
            m_gamma.resize(N);
            m_sigma.resize(m_nPanel3);
            for (k=0; k<N; k++)
            {
                ar >> f0;
                m_Cp[k] = double(f0);
            }
            for (k=0; k<N; k++)
            {
                ar >> f0;
                m_gamma[k] = double(f0);
            }
            for (k=0; k<m_nPanel3; k++)
            {
                ar >> f0;
                m_sigma[k] = double(f0);
            }
        }

        int pos = 0;
        for(int iw=0; iw<m_WingOpp.size(); iw++)
        {
            if(!m_WingOpp[iw].serializeWingOppFl5(ar, bIsStoring))
                return false;

            m_WingOpp[iw].m_dCp    = m_Cp.data()    + pos;
            m_WingOpp[iw].m_dG     = m_gamma.data() + pos;
            m_WingOpp[iw].m_dSigma = m_sigma.data() + pos;
            pos += m_WingOpp[iw].m_nPanel4;
        }

        if(ArchiveFormat<500015) m_AF.serializeFl5_b17(ar, bIsStoring);
        else
        {
            if(!m_AF.serializeFl5(ar, bIsStoring))
                return false;
        }
/*        m_AF.setOpp(m_Alpha, m_Beta, m_Phi, m_QInf);
        for(int iw=0; iw<m_WingOpp.size(); iw++)
        {
            m_WingOpp[iw].m_AF.setOpp(m_Alpha, m_Beta, m_Phi, m_QInf);
        }*/

        int nFuse=0;
        ar >> nFuse;
        m_FuseAF.resize(nFuse);
        for(int ifuse=0; ifuse<nFuse; ifuse++)
        {
            if(ArchiveFormat<500015) m_FuseAF[ifuse].serializeFl5_b17(ar, bIsStoring);
            else
            {
                if(!m_FuseAF[ifuse].serializeFl5(ar, bIsStoring))
                    return false;
            }
        }

        if(ArchiveFormat<=500001)
        {
            for(int isd=0; isd<24; isd++) ar>>dble;
        }
        else
            m_SD.serializeFl5(ar, bIsStoring);

        if(ArchiveFormat<=500013)
        {
            m_BLat.resize(1);
            m_BLong.resize(1);
            m_BLat.first().resize(4);
            m_BLong.first().resize(4);
            ar >> m_BLat[0][0] >> m_BLat[0][1] >> m_BLat[0][2] >> m_BLat[0][3];
            ar >> m_BLong[0][0]>> m_BLong[0][1]>> m_BLong[0][2]>> m_BLong[0][3];
        }
        else
        {
            ar >>n;
            m_BLat.resize(n);
            m_BLong.resize(n);
            for(int ie=0; ie<m_BLat.size(); ie++)
            {
                m_BLat[ie].resize(4);
                m_BLong[ie].resize(4);
                for(int j=0; j<4; j++)
                {
                    ar >> m_BLat[ie][j];
                    ar >> m_BLong[ie][j];
                }
            }

        }

        for(k=0; k<4; k++)
        {
            ar >> m_ALong[k][0]>> m_ALong[k][1]>> m_ALong[k][2]>> m_ALong[k][3];
            ar >> m_ALat[k][0] >> m_ALat[k][1] >> m_ALat[k][2] >> m_ALat[k][3];
        }


        ar >> m_Phi;  // repurposing - formerly m_XNP
        m_AF.setOpp(m_Alpha, m_Beta, m_Phi, m_QInf);
        for(int iw=0; iw<m_WingOpp.size(); iw++)
        {
            m_WingOpp[iw].m_AF.setOpp(m_Alpha, m_Beta, m_Phi, m_QInf);
        }

        for(int kv=0; kv<8;kv++)
        {
            ar >> dbl1 >> dbl2;
            m_EigenValue[kv] = std::complex<double>(dbl1, dbl2);

            for(int lv=0; lv<4; lv++)
            {
                ar >> dbl1 >> dbl2;
                m_EigenVector[kv][lv] = std::complex<double>(dbl1, dbl2);
            }
        }

        double real=0.0, imag=0.0;
        ar >> real >> imag;
        m_phiPH = std::complex<double>(real, imag);
        ar >> real >> imag;
        m_phiDR = std::complex<double>(real, imag);

        if(ArchiveFormat<=500001)
        {
            for (int i=0; i<50; i++) ar >> n;
            for (int i=0; i<50; i++) ar >> dble;
        }
        else
        {
            if(ArchiveFormat>=500012)
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

            ar >> nIntSpares;
            ar >> nDbleSpares;
        }
    }
    return true;
}


void PlaneOpp::exportMainDataToString(Plane const*pPlane, QString &poppdata, xfl::enumTextFileType filetype, QString const &textsep) const
{
    QString strange;
    QString title;
    QString len = Units::lengthUnitLabel();
    QString inertia = Units::inertiaUnitLabel();

    QString sep = "  ";
    if(filetype==xfl::CSV) sep = textsep+ " ";


    poppdata += planeName()+"\n";
    poppdata += polarName()+"\n\n";
    poppdata +=   ALPHACHAR.rightJustified(17, ' ') + sep
                + BETACHAR.rightJustified(17, ' ') + sep
                + PHICHAR.rightJustified(17, ' ') + sep
                + QString("ctrl").rightJustified(17, ' ') + sep
                + QString("VInf("+Units::speedUnitLabel()+")").rightJustified(17, ' ') +"\n";

    strange = QString::asprintf("%17g", m_Alpha);                     poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_Beta);                      poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_Phi);                       poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_Ctrl);                      poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_QInf*Units::mstoUnit());    poppdata += strange;

    poppdata += "\n\n";

    strange =   QString("CL").rightJustified(17, ' ')          + sep
              + QString("CX").rightJustified(17, ' ')          + sep
              + QString("CY").rightJustified(17, ' ')          + sep
              + QString("CD_inviscid").rightJustified(17, ' ') + sep
              + QString("CD_viscous").rightJustified(17, ' ')  + sep
              + QString("Cl").rightJustified(17, ' ')          + sep
              + QString("Cm_inviscid").rightJustified(17, ' ') + sep
              + QString("Cm_viscous").rightJustified(17, ' ')  + sep
              + QString("Cn_inviscid").rightJustified(17, ' ') + sep
              + QString("Cn_viscous").rightJustified(17, ' ') + "\n";
    poppdata += strange;

    strange = QString::asprintf("%17g", m_AF.CL());     poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_AF.CD());     poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_AF.Cy());     poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_AF.CDi());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_AF.CDv());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_AF.Cli());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_AF.Cmi());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_AF.Cmv());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_AF.Cni());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_AF.Cnv());    poppdata += strange;

    poppdata += "\n\n";

    strange = QString("CP.x("+len+")").rightJustified(17, ' ');         poppdata += strange+sep;
    strange = QString("CP.y("+len+")").rightJustified(17, ' ');         poppdata += strange+sep;
    strange = QString("CP.z("+len+")").rightJustified(17, ' ');         poppdata += strange+sep;
    strange = QString("NP.x("+len+")").rightJustified(17, ' ');         poppdata += strange+sep;
    poppdata += "\n";

    strange = QString::asprintf("%17g", m_AF.centreOfPressure().x*Units::mtoUnit());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_AF.centreOfPressure().y*Units::mtoUnit());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_AF.centreOfPressure().z*Units::mtoUnit());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_SD.XNP   *Units::mtoUnit());    poppdata += strange+sep;
    poppdata += "\n\n";

    strange = QString("mass("+Units::massUnitLabel()+")").rightJustified(17, ' ');     poppdata += strange+sep;
    strange = QString("CoG.x("+len+")").rightJustified(17, ' ');                       poppdata += strange+sep;
    strange = QString("CoG.y("+len+")").rightJustified(17, ' ');                       poppdata += strange+sep;
    strange = QString("CoG.z("+len+")").rightJustified(17, ' ');                       poppdata += strange+sep;
    strange = QString("CoG_Ixx("+inertia+")").rightJustified(17, ' ');                 poppdata += strange+sep;
    strange = QString("CoG_Iyy("+inertia+")").rightJustified(17, ' ');                 poppdata += strange+sep;
    strange = QString("CoG_Izz("+inertia+")").rightJustified(17, ' ');                 poppdata += strange+sep;
    strange = QString("CoG_Ixz("+inertia+")").rightJustified(17, ' ');                 poppdata += strange+sep;
    poppdata +="\n";

    strange = QString::asprintf("%17g", m_Mass*Units::kgtoUnit());            poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_CoG.x*Units::mtoUnit());            poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_CoG.y*Units::mtoUnit());            poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_CoG.z*Units::mtoUnit());            poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_Inertia[0]*Units::kgm2toUnit());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_Inertia[1]*Units::kgm2toUnit());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_Inertia[2]*Units::kgm2toUnit());    poppdata += strange+sep;
    strange = QString::asprintf("%17g", m_Inertia[3]*Units::kgm2toUnit());    poppdata += strange + "\n\n";

    if(isType7() || isType8())
    {
        //non dimensional stability derivatives
        strange  = QString("CXu").rightJustified(17, ' ') + sep;
        strange += QString("CZu").rightJustified(17, ' ') + sep;
        strange += QString("Cmu").rightJustified(17, ' ') + sep;
        strange += QString("CXa").rightJustified(17, ' ') + sep;
        strange += QString("CLa").rightJustified(17, ' ') + sep;
        strange += QString("Cma").rightJustified(17, ' ') + sep;
        strange += QString("CXq").rightJustified(17, ' ') + sep;
        strange += QString("CLq").rightJustified(17, ' ') + sep;
        strange += QString("Cmq").rightJustified(17, ' ') + sep;
        strange += QString("CYb").rightJustified(17, ' ') + sep;
        strange += QString("CYp").rightJustified(17, ' ') + sep;
        strange += QString("CYr").rightJustified(17, ' ') + sep;
        strange += QString("Clb").rightJustified(17, ' ') + sep;
        strange += QString("Clp").rightJustified(17, ' ') + sep;
        strange += QString("Clr").rightJustified(17, ' ') + sep;
        strange += QString("Cnb").rightJustified(17, ' ') + sep;
        strange += QString("Cnp").rightJustified(17, ' ') + sep;
        strange += QString("Cnr").rightJustified(17, ' ') + "\n";
        poppdata += strange;

        strange = QString::asprintf("%17g", m_SD.CXu);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.CZu);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.Cmu);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.CXa);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.CZa);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.Cma);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.CXq);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.CZq);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.Cmq);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.CYb);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.CYp);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.CYr);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.Clb);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.Clp);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.Clr);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.Cnb);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.Cnp);            poppdata += strange+sep;
        strange = QString::asprintf("%17g", m_SD.Cnr);            poppdata += strange;
        poppdata += "\n";

        //non-dimensional control derivatives
        if(m_SD.ControlNames.size())
        {
            strange  = QString("CXe").rightJustified(17, ' ') + sep;
            strange += QString("CYe").rightJustified(17, ' ') + sep;
            strange += QString("CZe").rightJustified(17, ' ') + sep;
            strange += QString("CLe").rightJustified(17, ' ') + sep;
            strange += QString("CMe").rightJustified(17, ' ') + sep;
            strange += QString("CNe").rightJustified(17, ' ') +"\n";
            poppdata += strange;

            for(int i=0; i<m_SD.ControlNames.size(); i++)
            {
                poppdata += m_SD.ControlNames.at(i) + "\n";
                strange = QString::asprintf("%17g", m_SD.CXe.at(i));            poppdata += strange+sep;
                strange = QString::asprintf("%17g", m_SD.CYe.at(i));            poppdata += strange+sep;
                strange = QString::asprintf("%17g", m_SD.CZe.at(i));            poppdata += strange+sep;
                strange = QString::asprintf("%17g", m_SD.CLe.at(i));            poppdata += strange+sep;
                strange = QString::asprintf("%17g", m_SD.CMe.at(i));            poppdata += strange+sep;
                strange = QString::asprintf("%17g", m_SD.CNe.at(i));            poppdata += strange;
            }
        }
    }
    poppdata += "\n\n";

    if(pPlane->isXflType())
    {
        PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            if(iw<m_WingOpp.size())
            {
                //if there are any flaps
                if(WOpp(iw).m_FlapMoment.size())
                {
                    poppdata += pPlaneXfl->wingAt(iw)->name() +"\n";
                    for (int l=0; l<WOpp(iw).m_nFlaps; l++)
                    {
                        strange = QString::asprintf("Flap_%d_moment=%11.5g ", l+1, WOpp(iw).m_FlapMoment.at(l)*Units::NmtoUnit());
                        strange += Units::momentUnitLabel();
                        poppdata += strange +"\n";
                    }
                    poppdata += "\n";
                }
            }
        }
    }
    poppdata += "\n\n";

    title  = QString("y("+Units::lengthUnitLabel()+")").rightJustified(17, ' ') + sep;
    title += QString("Re").rightJustified(17, ' ') + sep;
    title += QString("Ai").rightJustified(17, ' ') + sep;
    title += QString("Cd_i").rightJustified(17, ' ') + sep;
    title += QString("Cd_v").rightJustified(17, ' ') + sep;
    title += QString("Cl").rightJustified(17, ' ') + sep;
    title += QString("CP.x(%)").rightJustified(17, ' ') + sep;
    title += QString("Trans.top").rightJustified(17, ' ') + sep;
    title += QString("Trans.bot").rightJustified(17, ' ') + sep;
    title += QString("Cm_i").rightJustified(17, ' ') + sep;
    title += QString("Cm_v").rightJustified(17, ' ') + sep;
    title += QString("Bending.mom").rightJustified(17, ' ') + sep;
    title += QString("Vd.x").rightJustified(17, ' ') + sep;
    title += QString("Vd.y").rightJustified(17, ' ') + sep;
    title += QString("Vd.z").rightJustified(17, ' ') + sep;
    title += QString("F.x").rightJustified(17, ' ') + sep;
    title += QString("F.y").rightJustified(17, ' ') + sep;
    title += QString("F.z").rightJustified(17, ' ') + sep;
    title += GAMMACHAR.rightJustified(17, ' ');
    title += '\n';

    if(pPlane->isXflType())
    {
        PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            if(iw>=m_WingOpp.size()) break; // error somewhere

            WingOpp const &aWOpp = WOpp(iw);

            poppdata += pPlaneXfl->wingAt(iw)->name()+'\n';
            poppdata += title;
            for(int i=0; i<aWOpp.m_NStation; i++)
            {
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_StripPos.at(i)*Units::mtoUnit());     poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_Re.at(i));             poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_Ai.at(i));             poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_ICd.at(i));            poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_PCd.at(i));            poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_Cl.at(i));             poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_XCPSpanRel.at(i));     poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_XTrTop.at(i));         poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_XTrBot.at(i));         poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_CmC4.at(i));           poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_CmViscous.at(i));      poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_BendingMoment.at(i));  poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_Vd.at(i).x);           poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_Vd.at(i).y);           poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_Vd.at(i).z);           poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_F.at(i).x);            poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_F.at(i).y);            poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_F.at(i).z);            poppdata += strange+sep;
                strange = QString::asprintf("%17g", aWOpp.spanResults().m_Gamma.at(i));          poppdata += strange+sep;

                poppdata += '\n';
            }
            poppdata += '\n';
        }
    }
}


void PlaneOpp::exportPanel4DataToString(Plane const *pPlane, WPolar const *pWPolar,
                                       xfl::enumTextFileType exporttype,
                                       QString &paneldata) const
{
    paneldata.clear();
    if(!pPlane->isXflType())
    {
        return;
    }

    if(!pWPolar->isQuadMethod()) return;

    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

    QString strong, Format;
    QTextStream out;
    out.setString(&paneldata);
    out << "Main Wing Cp Coefficients\n";

    int coef = 1;

    if(!pWPolar->bThinSurfaces())
    {
        coef = 2;
    }
    if(exporttype==xfl::TXT) out << " Panel     CtrlPt.x        CtrlPt.y        CtrlPt.z       Nx      Ny       Nz        Area       Cp\n";
    else                     out << "Panel,CtrlPt.x,CtrlPt.y,CtrlPt.z,Nx,Ny,Nz,Area,Cp\n";

    if(exporttype==xfl::TXT) Format = "%1     %2     %3     %4     %5     %6     %7     %8     %9\n";
    else                     Format = "%1, %2, %3, %4, %5, %6, %7, %8, %9\n";


    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        if(pPlaneXfl->wingAt(iw))
        {
            out << pPlaneXfl->wingAt(iw)->name() + "Cp Coefficients"+"\n";
            int p=0;
            int iStrip = 0;
            for (int j=0; j<pPlaneXfl->wingAt(iw)->m_Surface.size(); j++)
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
                    strong = QString("Strip %1\n").arg(iStrip);
                    out << strong;

                    for(int l=0; l<surf.NXPanels() * coef; l++)
                    {
                        Panel4 const &p4 = pPlaneXfl->panel4(pPlaneXfl->wingAt(iw)->firstPanel4Index() + p);
                        strong = QString(Format).arg(p,7)
                                     .arg(p4.ctrlPt(isVLMMethod()).x,11,'f')
                                     .arg(p4.ctrlPt(isVLMMethod()).y,11,'f')
                                     .arg(p4.ctrlPt(isVLMMethod()).z,11,'f')
                                     .arg(p4.normal().x,11,'f')
                                     .arg(p4.normal().y,11,'f')
                                     .arg(p4.normal().z,11,'f')
                                     .arg(p4.area(),11,'f')
                                     .arg(WOpp(iw).m_dCp[p],11,'f');

                        out << strong;
                        p++;
                    }
                }
            }
        }
        out << ("\n\n");
    }
}


void PlaneOpp::exportPanel3DataToString(Plane const *pPlane, WPolar const *pWPolar,
                                        xfl::enumTextFileType exporttype, QString const &textsep,
                                        QString &paneldata) const
{

    paneldata.clear();
    if(!pPlane || !pWPolar) return;
    if(!pWPolar->isTriangleMethod()) return;

    QString sep = "  ";
    if(exporttype==xfl::CSV) sep = textsep + " ";

    QString strong, strange;
    QTextStream out;
    out.setString(&paneldata);

    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

    if(pPlaneXfl)
    {
        int coef = 1;
        if(!pWPolar->bThinSurfaces())  coef = 2;

        strange =   QString("Panel").rightJustified(17, ' ')        + sep
                  + QString("CtrlPt.x").rightJustified(17, ' ')     + sep
                  + QString("CtrlPt.y").rightJustified(17, ' ')     + sep
                  + QString("CtrlPt.z").rightJustified(17, ' ')     + sep
                  + QString("N.x").rightJustified(17, ' ')          + sep
                  + QString("N.y").rightJustified(17, ' ')          + sep
                  + QString("N.z").rightJustified(17, ' ')          + sep
                  + QString("Area").rightJustified(17, ' ')         + sep
                  + QString("Cp").rightJustified(17, ' ') + "\n";
        out << strange;

        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            out << pPlaneXfl->wingAt(iw)->name() + " - Cp Coefficients"+"\n";
            int p=0;
            int iStrip = 0;
            for (int j=0; j<pPlaneXfl->wingAt(iw)->m_Surface.size(); j++)
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
                    strong = QString("Strip %1\n").arg(iStrip);
                    out << strong;

                    for(int l=0; l<surf.NXPanels() * coef *2; l++)
                    {
                        Panel3 const &p3 = pPlaneXfl->panel3At(pPlaneXfl->wingAt(iw)->firstPanel3Index() + p);

                        double cp=0;
                        for(int in=0; in<3; in++) cp += m_Cp.at(p3.index()*3+in);
                        cp /= 3.0;

                        strong = QString::asprintf("%17d", p3.index())        +sep;
                        strong += QString::asprintf("%17g", p3.CoG().x)       +sep;
                        strong += QString::asprintf("%17g", p3.CoG().y)       +sep;
                        strong += QString::asprintf("%17g", p3.CoG().z)       +sep;
                        strong += QString::asprintf("%17g", p3.normal().x)    +sep;
                        strong += QString::asprintf("%17g", p3.normal().y)    +sep;
                        strong += QString::asprintf("%17g", p3.normal().z)    +sep;
                        strong += QString::asprintf("%17g", p3.area())        +sep;
                        strong += QString::asprintf("%17g", cp)               +"\n";

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

                strong = QString::asprintf("%17d", p3.index())        +sep;
                strong += QString::asprintf("%17g", p3.CoG().x)       +sep;
                strong += QString::asprintf("%17g", p3.CoG().y)       +sep;
                strong += QString::asprintf("%17g", p3.CoG().z)       +sep;
                strong += QString::asprintf("%17g", p3.normal().x)    +sep;
                strong += QString::asprintf("%17g", p3.normal().y)    +sep;
                strong += QString::asprintf("%17g", p3.normal().z)    +sep;
                strong += QString::asprintf("%17g", p3.area())        +sep;
                strong += QString::asprintf("%17g", cp)               +"\n";


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

        strange =   QString("Panel").rightJustified(17, ' ')        + sep
                  + QString("CtrlPt.x").rightJustified(17, ' ')     + sep
                  + QString("CtrlPt.y").rightJustified(17, ' ')     + sep
                  + QString("CtrlPt.z").rightJustified(17, ' ')     + sep
                  + QString("N.x").rightJustified(17, ' ')          + sep
                  + QString("N.y").rightJustified(17, ' ')          + sep
                  + QString("N.z").rightJustified(17, ' ')          + sep
                  + QString("Area").rightJustified(17, ' ')         + sep
                  + QString("Cp").rightJustified(17, ' ') + "\n";
        out << strange;



        for(int k=0; k<m_nPanel3; k++)
        {
            Panel3 const &p3 = pPlaneSTL->panel3At(k);

            double cp=0;
            for(int in=0; in<3; in++) cp += m_Cp.at(p3.index()*3+in);
            cp /= 3.0;

            strong = QString::asprintf("%17d", p3.index())        +sep;
            strong += QString::asprintf("%17g", p3.CoG().x)       +sep;
            strong += QString::asprintf("%17g", p3.CoG().y)       +sep;
            strong += QString::asprintf("%17g", p3.CoG().z)       +sep;
            strong += QString::asprintf("%17g", p3.normal().x)    +sep;
            strong += QString::asprintf("%17g", p3.normal().y)    +sep;
            strong += QString::asprintf("%17g", p3.normal().z)    +sep;
            strong += QString::asprintf("%17g", p3.area())        +sep;
            strong += QString::asprintf("%17g", cp)               +"\n";


            out << strong;

        }

        out << ("\n\n");
    }
}


QString PlaneOpp::variableName(int iVar)
{
    if(iVar<0 || iVar>=s_POppVariables.size())
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
    Q_ASSERT(nAVLCtrls==m_SD.Xde.size());

    m_BLong.resize(nAVLCtrls);
    m_BLat.resize(nAVLCtrls);
    for(int i=0; i<nAVLCtrls; i++)
    {
        m_BLong[i].resize(4);
        m_BLat[i].resize(4);
    }

    for(int ie=0; ie<m_BLong.size(); ie++)
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


bool PlaneOpp::solveEigenvalues(QString &log)
{
    log.clear();

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
    QString str;

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
        if(!eigenVector(m_ALong, rLong[i], vLong+i*4))
        {
            log += QString::asprintf("Error extracting longitudinal eigenvector for mode %d\n", i);
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
        if(!eigenVector(m_ALat, rLat[i], vLat+i*4))
        {
            log += QString::asprintf("Error extracting lateral eigenvector for mode %d\n", i);
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
    return true;
}


void PlaneOpp::outputEigen(QString &log)
{
    log.clear();

    QString str;
    str = "      ___Longitudinal modes___\n\n";
    log += str;

    str = QString::asprintf("      Eigenvalue:  %9.4g + %9.4gi   |   %9.4g + %9.4gi   |   %9.4g + %9.4gi   |   %9.4g + %9.4gi\n",
            m_EigenValue[0].real(), m_EigenValue[0].imag(),
            m_EigenValue[1].real(), m_EigenValue[1].imag(),
            m_EigenValue[2].real(), m_EigenValue[2].imag(),
            m_EigenValue[3].real(), m_EigenValue[3].imag());
    log += str;
    log += "                    _____________________________________________________________________________________________________\n";

    str = QString::asprintf("      Eigenvector: %9.4g + %9.4gi   |   %9.4g + %9.4gi   |   %9.4g + %9.4gi   |   %9.4g + %9.4gi\n",
            m_EigenVector[0][0].real(),  m_EigenVector[0][0].imag(),
            m_EigenVector[1][0].real(),  m_EigenVector[1][0].imag(),
            m_EigenVector[2][0].real(),  m_EigenVector[2][0].imag(),
            m_EigenVector[3][0].real(),  m_EigenVector[3][0].imag());
    log += str;

    for (int i=1; i<4; i++)
    {
        str = QString::asprintf("                   %9.4g + %9.4gi   |   %9.4g + %9.4gi   |   %9.4g + %9.4gi   |   %9.4g + %9.4gi\n",
                m_EigenVector[0][i].real(),  m_EigenVector[0][i].imag(),
                m_EigenVector[1][i].real(),  m_EigenVector[1][i].imag(),
                m_EigenVector[2][i].real(),  m_EigenVector[2][i].imag(),
                m_EigenVector[3][i].real(),  m_EigenVector[3][i].imag());
        log += str;
    }

    log += EOLCHAR;
    str = "      ___Lateral modes___\n\n";
    log += str;

    str = QString::asprintf("      Eigenvalue:  %9.4g + %9.4gi   |   %9.4g + %9.4gi   |   %9.4g + %9.4gi   |   %9.4g + %9.4gi\n",
            m_EigenValue[4].real(), m_EigenValue[4].imag(),
            m_EigenValue[5].real(), m_EigenValue[5].imag(),
            m_EigenValue[6].real(), m_EigenValue[6].imag(),
            m_EigenValue[7].real(), m_EigenValue[7].imag());
    log += str;
    log += "                    _____________________________________________________________________________________________________\n";

    str = QString::asprintf("      Eigenvector: %9.4g + %9.4gi   |   %9.4g + %9.4gi   |   %9.4g + %9.4gi   |   %9.4g + %9.4gi\n",
            m_EigenVector[4][0].real(),  m_EigenVector[4][0].imag(),
            m_EigenVector[5][0].real(),  m_EigenVector[5][0].imag(),
            m_EigenVector[6][0].real(),  m_EigenVector[6][0].imag(),
            m_EigenVector[7][0].real(),  m_EigenVector[7][0].imag());
    log += str;

    for (int i=1; i<4; i++)
    {
        str = QString::asprintf("                   %9.4g + %9.4gi   |   %9.4g + %9.4gi   |   %9.4g + %9.4gi   |   %9.4g + %9.4gi\n",
                m_EigenVector[4][i].real(),  m_EigenVector[4][i].imag(),
                m_EigenVector[5][i].real(),  m_EigenVector[5][i].imag(),
                m_EigenVector[6][i].real(),  m_EigenVector[6][i].imag(),
                m_EigenVector[7][i].real(),  m_EigenVector[7][i].imag());
        log += str;
    }

}
