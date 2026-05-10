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


#include <boatpolar.h>
#include <boatopp.h>
#include <boat.h>
#include <sail.h>
#include <sailobjects.h>
#include <utils.h>
#include <units.h>
#include <constants.h>

std::vector<std::string> BoatPolar::s_BtPolarVariableNames={"Ctrl",
                                               "TWS", "TWA ("+DEGstr+")", "AWS", "AWA ("+DEGstr+")",
                                               PHIstr,
                                               "CD", "CDi", "CDv", "CL",
                                               "CX", "CY", "CX_sum", "CY_sum",
                                               "FFFx (N)", "FFFy (N)", "FFFz (N)",
                                               "Fx (N)", "Fy (N)", "Fz (N)",
                                               "L (N.m)", "M (N.m)", "N (N.m)"};


BoatPolar::BoatPolar() : Polar3d()
{
    m_Name.clear();
    m_BoatName.clear();
    setDefaultSpec(nullptr);
    clearData();
}


void BoatPolar::setDefaultSpec(Boat const *pBoat)
{
    m_AnalysisMethod = xfl::QUADS;
    m_Type      = xfl::BOATPOLAR;

    m_bIgnoreBodyPanels = true;

    m_bGround       = true;
    m_GroundHeight = 0.0;

    m_ReferenceDim = xfl::AUTODIMS;
    m_ReferenceArea = 1.0;
    m_ReferenceChord = 1.0;

    m_VBtMin = m_VBtMax = 0.0;
    m_TWSMin = m_TWSMax = 10.0; //wind speed
    m_TWAMin = m_TWAMax = 0.0;  //sideslip = wind angle
    m_PhiMin = m_PhiMax = 0.0;  //heeling angle
    m_RyMin  = m_RyMax = 0.0;   //Rotation around Y (°), special for windsurfs

    m_WindSpline.clearControlPoints();
    m_WindSpline.setDegree(2);
    m_WindSpline.appendControlPoint(0.0,  0.0);
    m_WindSpline.appendControlPoint(0.5,  1.0);
    m_WindSpline.appendControlPoint(1.0, 10.0);
    m_WindSpline.appendControlPoint(1.0, 15.0);
    m_WindSpline.updateSpline();
    m_WindSpline.makeCurve();

    m_bTrefftz  = true ;
    m_AnalysisMethod = xfl::TRIUNIFORM;
    m_bViscous = true;
    m_NCrit = 9;
    m_XTrTop = m_XTrBot = 1.0;
    m_bAutoInertia = true;

    m_Density = 1.225;
    m_Viscosity = 1.5e-5;

    m_nXWakePanel4 = 5;
    m_TotalWakeLengthFactor = 30.0;
    m_WakePanelFactor = 1.1;

    m_CoG.set(0,0,0);

    if(pBoat)
    {
        resizeSailAngles(pBoat->nSails());
        std::fill(m_SailAngleMin.begin(), m_SailAngleMin.end(), 0);
        std::fill(m_SailAngleMax.begin(), m_SailAngleMax.end(), 0);
    }
    else
    {
        m_SailAngleMin.clear();
        m_SailAngleMax.clear();
    }

    m_ExtraDrag.clear();
}


void BoatPolar::duplicateSpec(const Polar3d *pPolar3d)
{
    Polar3d::duplicateSpec(pPolar3d);
    if(!pPolar3d->isBoatPolar()) return;

    BoatPolar const *pBtPolar = dynamic_cast<BoatPolar const*>(pPolar3d);

    m_VBtMin = pBtPolar->m_VBtMin;
    m_VBtMax = pBtPolar->m_VBtMax;
    m_TWSMax     = pBtPolar->m_TWSMax;
    m_TWSMin     = pBtPolar->m_TWSMin;
    m_TWSMax     = pBtPolar->m_TWSMax;
    m_TWAMin     = pBtPolar->m_TWAMin;
    m_TWAMax     = pBtPolar->m_TWAMax;
    m_PhiMin     = pBtPolar->m_PhiMin;
    m_PhiMax     = pBtPolar->m_PhiMax;
    m_RyMin      = pBtPolar->m_RyMin;
    m_RyMax      = pBtPolar->m_RyMax;

    m_SailAngleMin = pBtPolar->m_SailAngleMin;
    m_SailAngleMax = pBtPolar->m_SailAngleMax;

    m_ReferenceArea  = pBtPolar->m_ReferenceArea;
    m_ReferenceChord = pBtPolar->m_ReferenceChord;

    m_WindSpline = pBtPolar->m_WindSpline;
}


void BoatPolar::addPoint(BoatOpp const *pBtOpp)
{
    bool bInserted = false;
    int i=0;
    int size = dataSize();
    if(size)
    {
        for (i=0; i<size; i++)
        {
            if (fabs(pBtOpp->ctrl() - m_Ctrl.at(i)) < 0.001)
            {
                // then erase former result
                m_Ctrl[i]    =  pBtOpp->ctrl();
                m_VInf[i]    =  pBtOpp->QInf();
                m_Beta[i]    =  pBtOpp->beta();
                m_Phi[i]     =  pBtOpp->phi();
                m_AF[i]      =  pBtOpp->aeroForces();
                bInserted = true;
                break;
            }
            else if (pBtOpp->ctrl() < m_Ctrl.at(i))
            {
                // sort by crescending control values
                m_Ctrl.insert(m_Ctrl.begin()+i, pBtOpp->ctrl());
                m_VInf.insert(m_VInf.begin()+i, pBtOpp->QInf());
                m_Beta.insert(m_Beta.begin()+i, pBtOpp->beta());
                m_Phi.insert( m_Phi.begin()+i, pBtOpp->phi());
                m_AF.insert(  m_AF.begin()+i, pBtOpp->aeroForces());
                bInserted = true;
                break;
            }
        }
    }
    if(!bInserted)
    {
        // data is appended at the end
        m_Ctrl.push_back(pBtOpp->ctrl());
        m_VInf.push_back(pBtOpp->QInf());
        m_Beta.push_back(pBtOpp->beta());
        m_Phi.push_back(pBtOpp->phi());
        m_AF.push_back(pBtOpp->aeroForces());
    }
}


double BoatPolar::variable(int iVariable, int index) const
{
    if(iVariable<0 || iVariable>variableCount()) return 0.0;
    if(index<0 || index>dataSize())  return 0.0;
    return getVariable(iVariable, index);
}


double BoatPolar::getVariable(int iVar, int iPoint) const
{
    double q = qDyn(m_Ctrl.at(iPoint));
    switch (iVar)
    {
        default:
        case 0:  return m_Ctrl.at(iPoint);
        case 1:
        {
            double ctrl = m_Ctrl.at(iPoint);
            return TWSInf(ctrl);
        }
        case 2:
        {
            double ctrl = m_Ctrl.at(iPoint);
            return TWAInf(ctrl);
        }
        case 3:   return m_VInf.at(iPoint);
        case 4:   return m_Beta.at(iPoint);
        case 5:   return m_Phi.at(iPoint);
        case 6:   return m_AF.at(iPoint).CD();
        case 7:   return m_AF.at(iPoint).CDi();
        case 8:   return m_AF.at(iPoint).CDv();
        case 9:   return m_AF.at(iPoint).CSide();
        case 10:  return m_AF.at(iPoint).Cx();
        case 11:  return m_AF.at(iPoint).Cy();
        case 12:  return m_AF.at(iPoint).Cx_sum();
        case 13:  return m_AF.at(iPoint).Cy_sum();
        case 14:  return m_AF.at(iPoint).fffx()  * q * Units::NtoUnit();
        case 15:  return m_AF.at(iPoint).fffy()  * q * Units::NtoUnit();
        case 16:  return m_AF.at(iPoint).fffz()  * q * Units::NtoUnit();
        case 17:  return m_AF.at(iPoint).fsumx() * q * Units::NtoUnit();
        case 18:  return m_AF.at(iPoint).fsumy() * q * Units::NtoUnit();
        case 19:  return m_AF.at(iPoint).fsumz() * q * Units::NtoUnit();
        case 20:  return (m_AF.at(iPoint).Mi()+m_AF.at(iPoint).Mv()).x * q * Units::NmtoUnit();
        case 21:  return (m_AF.at(iPoint).Mi()+m_AF.at(iPoint).Mv()).y * q * Units::NmtoUnit();
        case 22:  return (m_AF.at(iPoint).Mi()+m_AF.at(iPoint).Mv()).z * q * Units::NmtoUnit();
    }
    return m_Ctrl.at(iPoint);
}


void BoatPolar::getProperties(std::string &props, xfl::enumTextFileType filetype, bool bData) const
{
    std::string PolarProps;

    std::string strong, lenlab, masslab, speedlab, arealab;
    std::string frontspacer("  ");

    lenlab   = " "+ Units::lengthUnitLabel();
    masslab  = " "+ Units::massUnitLabel();
    speedlab = " "+ Units::speedUnitLabel();
    arealab  = " "+ Units::areaUnitLabel();

//    if     (isPanel4Method() && bThickSurfaces()) PolarProps += "Quad Panels/thick surfaces";
//    if     (isPanel4Method() && bThinSurfaces())  PolarProps += "Quad Panels/thin surfaces";
    if     (isPanel4Method())             PolarProps += "Quad Panels/thin surfaces";
    else if(isVLM() && isVLM1())          PolarProps += "Quad Panels/VLM1";
    else if(isVLM() && !isVLM1())         PolarProps += "Quad Panels/VLM2";
    else if(isTriUniformMethod())         PolarProps += "Triangular Panels\nUniform doublet densities";
    else if(isTriLinearMethod())          PolarProps += "Triangular Panels\nLinear doublet densities";
    PolarProps +="\n";

//    if(bThinSurfaces()) PolarProps +="Wings as thin surfaces\n";
//    else                PolarProps +="Wings as thick surfaces\n";

    if(boundaryCondition()==xfl::DIRICHLET)  PolarProps += "B.C. = Dirichlet\n";
    else                                     PolarProps += "B.C. = Neumann\n";

    PolarProps += "Reference dimensions:\n";
    PolarProps += "  area  = " + std::format("{:g}", m_ReferenceArea*Units::m2toUnit()) + arealab+ "\n";
    PolarProps += "  chord = " + std::format("{:g}", m_ReferenceChord*Units::mtoUnit()) + lenlab+ "\n";

    if(isViscous()) PolarProps +="Viscous analysis\n";
    else            PolarProps += "Inviscid analysis\n";
    PolarProps +="\n";

    strong = "Analysis variables:";
    PolarProps += strong + "\n";

    strong = "V_boat:  "+ std::format("%5.1f, %5.1f", m_VBtMin*Units::mstoUnit(), m_VBtMax*Units::mstoUnit());
    PolarProps += frontspacer + strong + speedlab+ "\n";

    strong = "TWS_inf: "+std::format("%5.1f, %5.1f", m_TWSMin*Units::mstoUnit(), m_TWSMax*Units::mstoUnit());
    PolarProps += frontspacer + strong + speedlab+ "\n";

    strong = "TWA:     "+std::format("%5.1f, %5.1f", m_TWAMin, m_TWAMax);
    PolarProps += frontspacer + strong + DEGstr + EOLstr;

    strong = "Phi:     "+std::format("%5.1f, %5.1f", m_PhiMin, m_PhiMax);
    PolarProps += frontspacer + strong + DEGstr + EOLstr;

    strong = "Ry:      "+std::format("%5.1f, %5.1f", m_RyMin, m_RyMax);
    PolarProps += frontspacer + strong + DEGstr + EOLstr;

    for(unsigned int is=0; is<m_SailAngleMin.size();is++)
    {
        Boat *pBoat = SailObjects::boat(boatName());
        if(!pBoat) break;
        Sail *pSail = pBoat->sail(is);
        if(!pSail) break;
        strong = " = "+std::format("%5.1f, %5.1f", m_SailAngleMin[is], m_SailAngleMax[is]);
        PolarProps += frontspacer+ pSail->name() + strong + " "+ DEGstr + EOLstr;
    }

    PolarProps +="\n";

    strong  = "CoG = ("+std::format("{:7.2f}", m_CoG.x*Units::mtoUnit());
    PolarProps += strong + lenlab;

    strong  = "; "+std::format("{:7.2f}", m_CoG.z*Units::mtoUnit());
    PolarProps += strong + lenlab + ")\n";


    PolarProps += "Fluid properties:\n";

    strong  = frontspacer + RHOstr + " = " + std::format("{:9.5g}", density()*Units::densitytoUnit());
    strong += Units::densityUnitLabel() + "\n";
    PolarProps += strong;

    strong  = frontspacer + NUstr  + " = " + std::format("{:9.5g}", viscosity()*Units::viscositytoUnit());
    strong += Units::viscosityUnitLabel() + "\n";
    PolarProps += strong;

    if(extraDragCount())
    {
        strong = "Extra drag:\n";
        PolarProps += strong;
        for(int ix=0; ix<extraDragCount(); ix++)
        {
            if(fabs(m_ExtraDrag[ix].area())>PRECISION && fabs(m_ExtraDrag[ix].coef())>PRECISION)
            {
                PolarProps += frontspacer+ m_ExtraDrag.at(ix).tag()+":";
                strong = " area=" + std::format("{:g}", m_ExtraDrag.at(ix).area()*Units::m2toUnit());
                strong += arealab + ",  ";
                PolarProps += strong;
                strong = "coeff.=" + std::format("{:g}", m_ExtraDrag.at(ix).coef());
                PolarProps += strong + "\n";
            }
        }
        PolarProps += "\n";
    }

    if(!isVLM())
    {
        if(!bVortonWake())
        {
            strong = "Flat panel wake:\n";
            PolarProps += strong;
            strong = "Nb. of wake panels = " + std::format("{:d}", NXWakePanel4());
            PolarProps += frontspacer + strong;
            strong = "Length             = "+std::format("{:g}", totalWakeLengthFactor());
            PolarProps += frontspacer + strong + " x MAC\n";
            strong = "Progression factor = "+std::format("{:g}", wakePanelFactor());
            PolarProps += frontspacer + strong;
        }
        else
        {
            strong = "Vorton wake:\n";
            double refchord = referenceChordLength();
            PolarProps += strong;
            strong = "Buffer wake length = " + std::format("{:9g}", m_BufferWakeFactor*refchord*Units::mtoUnit());
            PolarProps += frontspacer + strong + lenlab + EOLstr;
            strong = "Streamwise step    = " + std::format("{:9g}", m_VortonL0*refchord*Units::mtoUnit());
            PolarProps += frontspacer + strong + lenlab + EOLstr;
            strong = "Discard distance   = " + std::format("{:9g}", m_VPWMaxLength*refchord*Units::mtoUnit());
            PolarProps += frontspacer + strong + lenlab + EOLstr;
            strong = "Vorton core size   = " + std::format("{:9g}", m_VortonCoreSize*refchord*Units::mtoUnit());
            PolarProps += frontspacer + strong + lenlab + EOLstr;
            strong = "VPW iterations     = " + std::format("{:d}", m_VPWIterations);
            PolarProps += frontspacer + strong;
        }
    }

    strong = std::format("Data points = {:d}\n", int(m_Ctrl.size()));
    PolarProps += "\n"+strong;

    if(!bData) return;
    std::string outstring;
    exportBtPlr(outstring, filetype, true);
    PolarProps += "\n"+strong;

    props = PolarProps;
}


void BoatPolar::exportBtPlr(std::string & outstr, xfl::enumTextFileType filetype, bool bDataOnly) const
{
    std::string Header, strong;
    std::string outstring;

    if (filetype==xfl::TXT)
    {
        if(!bDataOnly)
        {
            strong = "sail7 v.xxxx\n\n";
            outstring += strong;

            outstring += m_BoatName + EOLstr;
            outstring += m_Name + EOLstr;

            outstring += EOLstr;
        }


        Header = "     Ctrl        Fx         Fy         Fz         Mx         My         Mz    \n";
        outstring += Header;
        for (unsigned int j=0; j<m_Ctrl.size(); j++)
        {
            Vector3d drag = objects::windDirection(0, m_Beta.at(j)) * m_AF.at(j).viscousDrag();
            strong = std::format(" {:9.3f}  {:9.3f}  {:9.3f}  {:9.3f}  {:9.3f}  {:9.3f}  {:9.3f} \n",
                                       m_Ctrl.at(j),
                                       m_AF.at(j).fffx()+drag.x,
                                       m_AF.at(j).fffy()+drag.y,
                                       m_AF.at(j).fffz()+drag.z,
                                       (m_AF.at(j).Mi()+m_AF.at(j).Mi()).x,
                                       (m_AF.at(j).Mi()+m_AF.at(j).Mi()).y,
                                       (m_AF.at(j).Mi()+m_AF.at(j).Mi()).z);

            outstring += strong;
        }
    }
    else if(filetype==xfl::CSV)
    {
        if(!bDataOnly)
        {
            strong ="version xxxxxxxxx\n\n";
            outstring += strong;
            outstring += m_BoatName + EOLstr;
            outstring += m_Name + EOLstr;
        }

        Header = "Ctrl, FFFx, FFFy, FFFz, Fx, Fy, Fz, Mx, My, Mz\n";
        outstring += Header;
        for(unsigned int j=0; j<m_Ctrl.size(); j++)
        {
            Vector3d drag = objects::windDirection(0, m_Beta.at(j)) * m_AF.at(j).viscousDrag();

            strong = std::format(" {:9.3f},  {:9.3f},  {:9.3f},  {:9.3f},  {:9.3f},  {:9.3f},  {:9.3f}\n",
                                       m_Ctrl.at(j),
                                       m_AF.at(j).fffx()+drag.x,
                                       m_AF.at(j).fffy()+drag.y,
                                       m_AF.at(j).fffz()+drag.z,
                                       (m_AF.at(j).Mi()+m_AF.at(j).Mi()).x,
                                       (m_AF.at(j).Mi()+m_AF.at(j).Mi()).y,
                                       (m_AF.at(j).Mi()+m_AF.at(j).Mi()).z);

            outstring += strong;

        }
    }
    outstring += "\n\n";

    outstr = outstring;
}


bool BoatPolar::hasBtOpp(BoatOpp const*pBOpp) const
{
    return pBOpp->polarName()==m_Name;
}


void BoatPolar::clearData()
{
    m_Ctrl.clear();
    m_VInf.clear();
    m_Beta.clear();
    m_Phi.clear();
    m_AF.clear();
}


void BoatPolar::setVariableNames()
{
    std::string const strSpeed = Units::speedUnitLabel();
    std::string const strForce = Units::forceUnitLabel();
    std::string const strMoment = Units::momentUnitLabel();

    s_BtPolarVariableNames = {"Ctrl", "TWS (" +strSpeed+ ")", "TWA (" +DEGstr+ ")", "AWS (" +strSpeed+ ")", "AWA (" +DEGstr+ ")", PHIstr,
                                          "CD", "CDi", "CDv", "CL",
                                          "CX", "CY", "CX_sum", "CY_sum",
                                          "FFFx ("+strForce+")", "FFFy ("+strForce+")", "FFFz ("+strForce+")",
                                          "Fx ("+strForce+")", "Fy ("+strForce+")", "Fz ("+strForce+")",
                                          "L ("+ strMoment+")", "M ("+ strMoment+")", "N ("+ strMoment+")"};
}


void BoatPolar::setSailAngleRange(int isail, double thetamin, double thetamax)
{
    if(isail>=0 && isail<sailAngleSize())
    {
        m_SailAngleMin[isail] = thetamin;
        m_SailAngleMax[isail] = thetamax;
    }
}


void BoatPolar::makeDefaultArrays() //debug only
{
    int N=0;
    m_Ctrl.resize(N);
    m_VInf.resize(N);
    m_Beta.resize(N);
    m_Phi.resize(N);
    m_AF.resize(N);

    double phi = double(rand())/double(RAND_MAX) *2.0*PI;

    for(int i=0; i<N; i++)
    {
        double di = double(i)/double(N-1)/3.0;

        m_Ctrl[i] = 100.0*cos(di*1.0*PI+phi);
        m_VInf[i] = 1.0*di+phi;
        m_Beta[i] = 32.0*cos(di*1.0*PI+phi);
        m_Phi[i]  = 25.0*cos(di*1.0*PI+phi);
    }
}


double BoatPolar::sailAngle(int iSail, double ctrl) const
{
    if(iSail<0 || iSail>=int(m_SailAngleMin.size())) return 0.0;
    return m_SailAngleMin.at(iSail) + ctrl * (m_SailAngleMax.at(iSail)-m_SailAngleMin.at(iSail));
}


void BoatPolar::resizeSailAngles(int newsize)
{
    if(int(m_SailAngleMin.size())!= newsize) m_SailAngleMin.resize(newsize);
    if(int(m_SailAngleMax.size())!= newsize) m_SailAngleMax.resize(newsize);
}


void BoatPolar::getBtPolarData(std::string &data, std::string const & separator) const
{
    std::string strong, strange;
    std::string polardata;
    std::string sep = separator;

    strong = boatName() + EOLstr;
    polardata += strong;

    strong = name() + EOLstr;
    polardata += strong;

    for(int in=0; in<BoatPolar::variableCount(); in++)
    {
        strange =  BoatPolar::variableName(in);
        if(in==0) strange = "     "+strange;// start with a blank space for consistency with polar data
        for(int il=int(strange.length()); il<11; il++) strange+=" ";
        polardata += strange+sep;
    }
    polardata += "\n";


    for(unsigned int i=0; i<m_Ctrl.size(); i++)
    {
        for(int iVar=0; iVar<BoatPolar::variableCount(); iVar++)
        {
            double pX = getVariable(iVar, i);
            strange = std::format("{:11.5g}", pX);
            polardata += strange+sep;
        }
        polardata += EOLstr;
    }

    data = polardata;
}


void BoatPolar::copy(const BoatPolar *pBoatPolar)
{
    duplicateSpec(pBoatPolar);
    m_BoatName = pBoatPolar->boatName();
    m_Name = pBoatPolar->name();

    clearData();

    m_Ctrl = pBoatPolar->m_Ctrl;
    m_Beta = pBoatPolar->m_Beta;
    m_VInf = pBoatPolar->m_VInf;
    m_Phi  = pBoatPolar->m_Phi;
    m_AF   = pBoatPolar->m_AF;
}



void BoatPolar::remove(int i)
{
    m_Ctrl.erase(m_Ctrl.begin()+i);
    m_VInf.erase(m_VInf.begin()+i);
    m_Beta.erase(m_Beta.begin()+i);
    m_Phi.erase( m_Phi.begin()+i);
    m_AF.erase(  m_AF.begin()+i);
}


void BoatPolar::insertDataPointAt(int index, bool bAfter)
{
    if(bAfter) index++;
    m_Ctrl.insert(m_Ctrl.begin()+index,0.0);
    m_VInf.insert(m_VInf.begin()+index,0.0);
    m_Beta.insert(m_Beta.begin()+index,0.0);
    m_Phi.insert( m_Phi.begin() +index,0.0);
    m_AF.insert(  m_AF.begin()  +index,AeroForces());
}


double BoatPolar::windForce(double z) const
{
    if(m_WindSpline.ctrlPointCount()<1) return 1.0;

    if(z>m_WindSpline.lastCtrlPoint().y) return 1.0;
    if(z<0.0) return 0.0;

    std::vector<Node2d> const &pts = m_WindSpline.outputPts();

    for (unsigned int k=0; k<pts.size()-1; k++)
    {
        if (pts.at(k).y<pts.at(k+1).y  && pts.at(k).y <= z && z<=pts.at(k+1).y )
        {
            double x = (pts.at(k).x + (pts.at(k+1).x-pts.at(k).x)
                                     /(pts.at(k+1).y-pts.at(k).y)*(z-pts.at(k).y));
            return x;
        }
    }
    return 0.0;
}


void BoatPolar::trueWindSpeed(double ctrl, double z, Vector3d &VT) const
{
    if(m_WindSpline.ctrlPointCount()<1) return;
    z = std::min(m_WindSpline.lastCtrlPoint().y, z);
    z = std::max(z, 0.0);

    double tws_inf = m_TWSMin*(1-ctrl) + m_TWSMax*ctrl;
    double twa = m_TWAMin*(1-ctrl) + m_TWAMax*ctrl;;// true wind angle at high altitude in degrees

    std::vector<Node2d> const &pts = m_WindSpline.outputPts();

    for (unsigned int k=0; k<pts.size()-1; k++)
    {
        if (pts.at(k).y<pts.at(k+1).y  && pts.at(k).y <= z && z<=pts.at(k+1).y )
        {
            double amp = (pts.at(k).x + (pts.at(k+1).x-pts.at(k).x) /(pts.at(k+1).y-pts.at(k).y)*(z-pts.at(k).y));
            double Vh = amp*tws_inf;
            VT.set(Vh*cos(twa*PI/180.0), Vh*sin(twa*PI/180.0), 0.0);
            return;
        }
    }
}


/** Returns the wind angle at high altitude */
double BoatPolar::AWSInf(double ctrl) const
{
    double Vb      = m_VBtMin*(1-ctrl)+ m_VBtMax*ctrl;
    double tws_inf = m_TWSMin*(1-ctrl) + m_TWSMax*ctrl;
    double twa     = m_TWAMin*(1-ctrl) + m_TWAMax*ctrl;// true wind angle at high altitude in degrees
    double Vx = tws_inf * cos(twa*PI/180.0)+Vb;
    double Vy = tws_inf * sin(twa*PI/180.0);
    return sqrt(Vx*Vx+Vy*Vy);
}


/** Returns the wind angle at high altitude */
double BoatPolar::AWAInf(double ctrl) const
{
    double Vb      = m_VBtMin*(1-ctrl)+ m_VBtMax*ctrl;
    double tws_inf = m_TWSMin*(1-ctrl) + m_TWSMax*ctrl;
    double twa     = m_TWAMin*(1-ctrl) + m_TWAMax*ctrl;// true wind angle at high altitude in degrees
    double Vx = tws_inf * cos(twa*PI/180.0)+Vb;
    double Vy = tws_inf * sin(twa*PI/180.0);
    double beta = atan2(Vy, Vx)*180.0/PI;
    return beta;
}


/**
 * @param the control parameter
 * @param z altitude
 * @param AWS the apparent wind speed at altitude z
 */
void BoatPolar::apparentWind(double ctrl, double z, Vector3d &AWS) const
{
    if(m_WindSpline.ctrlPointCount()<1) return;
    z = std::min(m_WindSpline.lastCtrlPoint().y, z);
    z = std::max(z, 0.0);

    double Vb = boatSpeed(ctrl);

    double tws_inf = TWSInf(ctrl); // true wind speed at high altitude
    double twa_inf = TWAInf(ctrl); // true wind angle at high altitude


    Vector3d TWS(tws_inf*cos(twa_inf*PI/180.0), tws_inf*sin(twa_inf*PI/180.0), 0.0); // true wind speed

    std::vector<Node2d> const &pts = m_WindSpline.outputPts();

    for (unsigned int k=0; k<pts.size()-1; k++)
    {
        if (pts.at(k).y<pts.at(k+1).y  && pts.at(k).y<=z && z<=pts.at(k+1).y )
        {
            double amp = (pts.at(k).x + (pts.at(k+1).x-pts.at(k).x) /(pts.at(k+1).y-pts.at(k).y)*(z-pts.at(k).y));

            // determine TWA
            Vector3d TWS_h = TWS * amp;
            AWS.set(TWS_h.x+Vb, TWS_h.y, 0.0);
            return;
        }
    }
    // execution should never get here
    AWS.set(TWS.x+Vb, TWS.y, 0.0);
    assert(false);
}


void BoatPolar::setData(int i, double ctrl, double vinf, double beta, double phi)
{
    if(i>=0 && i<int(m_Ctrl.size()))
    {
        m_Ctrl[i] = ctrl;
        m_VInf[i] = vinf;
        m_Beta[i] = beta;
        m_Phi[i]  = phi;
    }
}



