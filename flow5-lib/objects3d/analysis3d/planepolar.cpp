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


#include <planepolar.h>

#include <constants.h>
#include <geom_global.h>
#include <objects_global.h>
#include <plane.h>
#include <planeopp.h>
#include <planestl.h>
#include <planexfl.h>
#include <surface.h>
#include <units.h>
#include <utils.h>
#include <wingopp.h>

std::vector<std::string> PlanePolar::s_VariableNames;


PlanePolar::PlanePolar() : Polar3d()
{
      m_pPlane = nullptr;

      setDefaultSpec(m_pPlane);
}


void PlanePolar::setDefaults()
{
    Polar3d::setDefaults();

    m_pPlane = nullptr;
    m_PlaneName = std::string("");

    m_bAVLDrag          = false;
    m_bAdjustedVelocity = false;
    m_bAutoInertia      = true;
    m_bFuseDrag         = false;
    m_bFuseMi           = false;
    m_bGround           = false;
    m_bOtherWingsArea   = false;
    m_bThinSurfaces     = true;
    m_bTrefftz          = true;
    m_bViscLoop         = false;
    m_bViscous          = true;
    m_bVortonWake       = false;
    m_bWingTipMi        = false;

    m_Type = xfl::T1POLAR;


    m_AlphaSpec = 0.0;
    m_AnalysisMethod = xfl::TRIUNIFORM;
    m_BC = xfl::DIRICHLET;

    m_Viscosity = 1.5e-5;
    m_Density = 1.225;

    m_FuseCf = 0.0;
    m_FuseDragMethod = PlanePolar::KARMANSCHOENHERR;
    m_FuseDragMethod = PlanePolar::MANUALFUSECF;

    m_GroundHeight = 0.0;
    m_QInfSpec  = 10.0;

    m_ReferenceDim   = xfl::PROJECTED;
    m_RefArea  = 0.0;
    m_RefChord = 0.0;
    m_RefSpan  = 0.0;

    m_BufferWakeFactor   = 0.3;  // x MAC
    m_TotalWakeLengthFactor = 30.0;
    m_VPWMaxLength       = 30.0;
    m_VortonL0           = 1.0;  // x MAC
    m_WakePanelFactor = 1.1;
    m_XNeutralPoint = 0.0;
    m_nXWakePanel4 = 5;

    m_AVLSpline.clearControlPoints();
    m_AVLSpline.appendControlPoint(0.035, -0.5);
    m_AVLSpline.appendControlPoint(0.015, -0.3);
    m_AVLSpline.appendControlPoint(0.005, 0.1);
    m_AVLSpline.appendControlPoint(0.015, 0.7);
    m_AVLSpline.appendControlPoint(0.035, 1.3);

    m_FlapControls.clear();
    m_AVLControls.clear();

    m_OperatingRange.clear();
    m_InertiaRange.clear();
    m_AngleRange.clear();
    m_Alpha.clear();
    m_Ctrl.clear();
    m_Beta.clear();
    m_Phi.clear();
    m_QInfinite.clear();
    m_AF.clear();

    m_XNP.clear();
    m_Mass_var.clear();

    m_CoG_x.clear();
    m_CoG_z.clear();
    m_MaxBending.clear();

    m_EV.clear();
}


void PlanePolar::setDefaultSpec(Plane const*pPlane)
{
    setDefaults();

    m_pPlane = pPlane;

    m_ExtraDrag.clear();

    m_OperatingRange.resize(4);  //Vel, alpha, beta, phi
    m_OperatingRange[0].setRange(10,10); // to ensure that the default analysis is runnable
    m_OperatingRange[1].setRange(0,0);   // to ensure that the default analysis is runnable
    m_OperatingRange[2].setRange(0,0);   // to ensure that the default analysis is runnable
    m_OperatingRange[3].setRange(0,0);   // to ensure that the default analysis is runnable
    m_InertiaRange.resize(3);    //mass, CoG.x, CoG.z

    clearAngleRangeList();
    if(pPlane)
    {
        setReferenceArea(m_pPlane->planformArea(m_bOtherWingsArea));
        setReferenceSpanLength(m_pPlane->planformSpan());
        retrieveInertia(pPlane);
        resetAngleRanges(pPlane);
    }
    else
    {
        m_RefArea = m_RefSpan = m_RefChord = 0.0;
    }
}


void PlanePolar::replacePOppDataAt(int pos, PlaneOpp const *pPOpp)
{
    if(pos<0 || pos>= dataSize()) return;
    removeAt(pos);
    insertPOppDataAt(pos, pPOpp);
}


void PlanePolar::insertPOppDataAt(int pos, PlaneOpp const *pPOpp)
{
    if(pos<0 || pos> dataSize()) return; // if(pos==size), then the data is appended

    m_Alpha.insert(m_Alpha.begin()+pos, pPOpp->alpha());
    m_Beta.insert(m_Beta.begin()+pos, pPOpp->beta());
    m_Phi.insert(m_Phi.begin()+pos, pPOpp->phi());
    m_QInfinite.insert(m_QInfinite.begin()+pos, pPOpp->QInf());

    m_AF.insert(m_AF.begin()+pos, pPOpp->m_AF);

    if(pPOpp->nWOpps()) m_MaxBending.insert(m_MaxBending.begin()+pos, pPOpp->WOpp(0).m_MaxBending);
    else                m_MaxBending.insert(m_MaxBending.begin()+pos, 0.0);
    m_Ctrl.insert(m_Ctrl.begin()+pos, pPOpp->ctrl());
    m_XNP.insert(m_XNP.begin()+pos,  pPOpp->m_SD.XNP);

    m_EV.insert(m_EV.begin()+pos, EigenValues());
    m_EV[pos].m_EV[0] = pPOpp->m_EigenValue[0];
    m_EV[pos].m_EV[1] = pPOpp->m_EigenValue[1];
    m_EV[pos].m_EV[2] = pPOpp->m_EigenValue[2];
    m_EV[pos].m_EV[3] = pPOpp->m_EigenValue[3];
    m_EV[pos].m_EV[4] = pPOpp->m_EigenValue[4];
    m_EV[pos].m_EV[5] = pPOpp->m_EigenValue[5];
    m_EV[pos].m_EV[6] = pPOpp->m_EigenValue[6];
    m_EV[pos].m_EV[7] = pPOpp->m_EigenValue[7];

    //make room for computed values

    m_Mass_var.insert(m_Mass_var.begin()+pos, pPOpp->m_Mass);
    m_CoG_x.insert(m_CoG_x.begin()+pos, 0.0);
    m_CoG_z.insert(m_CoG_z.begin()+pos, 0.0);

    calculatePoint(pos);
}


std::complex<double> PlanePolar::eigenValue(int iMode, int index) const
{
    if(index<0 || index>=int(m_EV.size()))
        return std::complex<double>(0.0,0.0);
    if(iMode<0 || iMode>7)
        return std::complex<double>(0.0,0.0);

    return m_EV.at(index).m_EV[iMode];
}


/** @todo what about AF and EV? */
void PlanePolar::insertDataAt(int pos, double Alpha, double Beta, double Phi, double QInf, double Ctrl, double Cb, double XNP)
{
    if(pos<0 || pos>dataSize()) return;

    m_Alpha.insert(m_Alpha.begin()+pos, Alpha);
    m_Beta.insert(m_Beta.begin()+pos, Beta);
    m_Phi.insert(m_Phi.begin()+pos, Phi);
    m_QInfinite.insert(m_QInfinite.begin()+pos, QInf);

    m_MaxBending.insert(m_MaxBending.begin()+pos, Cb);
    m_Ctrl.insert(m_Ctrl.begin()+pos, Ctrl);
    if(isStabilityPolar()) m_XNP.insert(m_XNP.begin()+pos, XNP);
    else                   m_XNP.insert(m_XNP.begin()+pos, 0.0);

    m_Mass_var.insert(m_Mass_var.begin()+pos, 0.0);
    m_CoG_x.insert(m_CoG_x.begin()+pos, 0.0);
    m_CoG_z.insert(m_CoG_z.begin()+pos, 0.0);
}


void PlanePolar::resizeData(int newsize)
{
    m_Alpha.resize(newsize);
    m_Beta.resize(newsize);
    m_Phi.resize(newsize);
    m_QInfinite.resize(newsize);

    m_MaxBending.resize(newsize);
    m_Ctrl.resize(newsize);
    m_XNP.resize(newsize);

    m_Mass_var.resize(newsize);
    m_CoG_x.resize(newsize);
    m_CoG_z.resize(newsize);

    m_AF.resize(newsize);
    m_EV.resize(newsize);
}


void PlanePolar::addPlaneOpPointData(PlaneOpp const *pPOpp)
{
    bool bInserted(false);
    double d(0.001);
    int size = dataSize();

    if(size)
    {
        for (int i=0; i<size; i++)
        {
            if(m_Type<xfl::T4POLAR)
            {
                if (fabs(pPOpp->alpha()-m_Alpha.at(i)) < d)
                {
                    replacePOppDataAt(i, pPOpp);
                    bInserted = true;
                    break;
                }
                else if (pPOpp->alpha() < m_Alpha.at(i))
                {
                    insertPOppDataAt(i, pPOpp);
                    bInserted = true;
                    break;
                }
            }
            else if(isFixedaoaPolar())
            {
                // type 4, sort by speed
                if (fabs(pPOpp->m_QInf - m_QInfinite.at(i)) < d)
                {
                    // then erase former result
                    replacePOppDataAt(i, pPOpp);
                    bInserted = true;
                    break;
                }
                else if (pPOpp->m_QInf < m_QInfinite.at(i))
                {
                    // sort by crescending speed
                    insertPOppDataAt(i, pPOpp);
                    bInserted = true;
                    break;
                }
            }
            else if(isBetaPolar())
            {
                // type 5, sort by sideslip angle
                if (fabs(pPOpp->beta() - m_Beta.at(i)) < d)
                {
                    // then erase former result
                    replacePOppDataAt(i, pPOpp);
                    bInserted = true;
                    break;
                }
                else if (pPOpp->beta() < m_Beta.at(i))
                {
                    // sort by crescending speed
                    insertPOppDataAt(i, pPOpp);
                    bInserted = true;
                    break;
                }
            }
            else if(isStabilityPolar() || isControlPolar())
            {
                // Control or stability analysis, sort by control value
                if (fabs(pPOpp->ctrl() - m_Ctrl.at(i))<d)
                {
                    // then erase former result
                    replacePOppDataAt(i, pPOpp);
                    bInserted = true;
                    break;
                }
                else if (pPOpp->ctrl() < m_Ctrl.at(i))
                {
                    // sort by crescending control values
                    insertPOppDataAt(i, pPOpp);
                    bInserted = true;
                    break;
                }
            }
            else if(isType8Polar())
            {
                // Type 8 analysis, sort by alpha then beta then QInf
                if (fabs(pPOpp->alpha()-m_Alpha.at(i))<d)
                {
                    if (fabs(pPOpp->beta() - m_Beta.at(i)) < d)
                    {
                        if (fabs(pPOpp->m_QInf - m_QInfinite.at(i)) < d)
                        {
                            // then erase former result
                            replacePOppDataAt(i, pPOpp);
                            bInserted = true;
                            break;
                        }
                        else if (pPOpp->m_QInf < m_QInfinite.at(i))
                        {
                            // sort by crescending speed
                            insertPOppDataAt(i, pPOpp);
                            bInserted = true;
                            break;
                        }
                    }
                    else if (pPOpp->beta() < m_Beta.at(i))
                    {
                        // sort by crescending speed
                        insertPOppDataAt(i, pPOpp);
                        bInserted = true;
                        break;
                    }
                }
                else if (pPOpp->alpha() < m_Alpha.at(i))
                {
                    insertPOppDataAt(i, pPOpp);
                    bInserted = true;
                    break;
                }
            }
        }
    }

    if(!bInserted)
    {
        // data is appended at the end
        int size = dataSize();
        insertPOppDataAt(size, pPOpp);
    }
}


void PlanePolar::calculatePoint(int iPt)
{
    if(iPt<0 || iPt>=int(m_AF.size())) return;

    double ctrl = m_Ctrl.at(iPt);
    if(m_Type==xfl::T6POLAR)
    {
        m_Mass_var[iPt] = m_InertiaRange.at(0).ctrlVal(ctrl);
        m_CoG_x[iPt]    = m_InertiaRange.at(1).ctrlVal(ctrl);
        m_CoG_z[iPt]    = m_InertiaRange.at(2).ctrlVal(ctrl);
    }
    else
    {
        m_Mass_var[iPt] = m_Mass;
        m_CoG_x[iPt] = m_CoG.x;
        m_CoG_z[iPt] = m_CoG.z;
    }

    /** @todo restore */
    /*    if(m_bFuseDrag)
        {
                double Re = m_FuseLength * m_QInfinite.at(iPt) / m_Viscosity;
                m_FuseCf[iPt] = fuseDragCoef(Re);
        }
    else*/
    {
        //        m_FuseCf[iPt] = 0.0;
    }

//    Vector3d WindD = windDirection(m_Alpha.at(iPt), m_Beta.at(iPt));
//    Vector3d WindN = windNormal(m_Alpha.at(iPt), m_Beta.at(iPt));

    if(m_AF.size()>1 && !isStabilityPolar())
    {
        double XCpClf = m_AF.front().centreOfPressure().x * m_AF.front().CL();
        double XCpClb = m_AF.back().centreOfPressure().x * m_AF.back().CL();
        m_XNeutralPoint = (XCpClb-XCpClf) / (m_AF.back().CL()-m_AF.front().CL());
    }
    else m_XNeutralPoint = 0.0;

    if(isStabilityPolar()) m_EV[iPt].computeModes();
}


void PlanePolar::duplicateSpec(Polar3d const *pPolar3d)
{
    Polar3d::duplicateSpec(pPolar3d);

    if(!pPolar3d->isPlanePolar()) return;
    PlanePolar const *pWPolar = dynamic_cast<PlanePolar const *>(pPolar3d);

    m_bThinSurfaces = pWPolar->bThinSurfaces();


    m_Type  = pWPolar->m_Type;

    m_QInfSpec    = pWPolar->m_QInfSpec;
    m_AlphaSpec   = pWPolar->m_AlphaSpec;
    m_BankAngle   = pWPolar->m_BankAngle;

    if(pWPolar->isBetaPolar()) m_BetaSpec = 0.0;
    else                       m_BetaSpec = pWPolar->m_BetaSpec;

    m_theStyle = pWPolar->theStyle();

    m_bViscLoop = pWPolar->m_bViscLoop;

    // general aerodynamic data - specific to a polar
    m_bAdjustedVelocity = pWPolar->m_bAdjustedVelocity;

    m_OperatingRange  = pWPolar->m_OperatingRange;
    m_InertiaRange    = pWPolar->m_InertiaRange;
    m_AngleRange      = pWPolar->m_AngleRange;

    m_RefArea         = pWPolar->m_RefArea;//for lift and drag calculations
    m_RefChord        = pWPolar->m_RefChord;// for moment calculations
    m_RefSpan         = pWPolar->m_RefSpan;//for moment calculations
    m_bOtherWingsArea = pWPolar->m_bOtherWingsArea;

    //Inertia properties
    m_bAutoInertia = pWPolar->m_bAutoInertia;

    m_FlapControls   = pWPolar->m_FlapControls;
    m_AVLControls = pWPolar->m_AVLControls;

    m_bWingTipMi  = pWPolar->m_bWingTipMi;
    m_bFuseMi     = pWPolar->m_bFuseMi;
    m_bFuseDrag   = pWPolar->m_bFuseDrag;

    m_FuseDragMethod = pWPolar->m_FuseDragMethod;
    m_FuseCf         = pWPolar->m_FuseCf;

    m_ExtraDrag = pWPolar->m_ExtraDrag;
    m_bAVLDrag  = pWPolar->m_bAVLDrag;
    m_AVLSpline.duplicate(pWPolar->m_AVLSpline);
}


void PlanePolar::setVariableNames()
{
    std::string strLength = Units::lengthUnitLabel();;
    std::string strSpeed  = Units::speedUnitLabel();;
    std::string strMass   = Units::massUnitLabel();
    std::string strForce  = Units::forceUnitLabel();
    std::string strMoment = Units::momentUnitLabel();
    s_VariableNames.clear();

    s_VariableNames = std::vector<std::string>({
                                                "Ctrl", ALPHAstr + " ("+DEGstr+")", BETAstr + " ("+DEGstr+")", PHIstr + " ("+DEGstr+")",
                                                "CL", "CD", "CD_viscous", "CD_induced", "CY", "Cm", "Cm_viscous",
                                                "Cm_pressure","Cl","Cn","Cn_viscous","Cn_pressure","CL/CD", "CL"+EXPstr+"(3/2)/CD", "1/sqrt(CL)",
                                                "Lift ("+strForce+")", "Drag ("+strForce+")",
                                                "Fx_FF ("+strForce+")", "Fy_FF ("+strForce+")", "Fz_FF ("+strForce+")",
                                                "Fx_sum ("+strForce+")", "Fy_sum ("+strForce+")", "Fz_sum ("+strForce+")",
                                                "Extra drag ("+strForce+")", "Fuse drag ("+strForce+")", "Cf_Fuse",
                                                "Vx ("+strSpeed+")","Vz ("+strSpeed+")",
                                                "V ("+strSpeed+")", "Gamma", "L ("+ strMoment+")", "M ("+ strMoment+")",
                                                "N ("+ strMoment+")", "CPx ("+ strLength+")", "CPy ("+ strLength+")", "CPz ("+ strLength+")",
                                                "BM ("+ strMoment+")", "m.g.Vz (W)", "Drag x V (W)", "Efficiency", "XCp.Cl",
                                                "XNP ("+ strLength+")", "Phugoid Freq. (Hz)", "Phugoid Damping", "Short Period Freq. (Hz)",
                                                "Short Period Damping Ratio", "Dutch Roll Freq. (Hz)", "Dutch Roll Damping", "Roll Damping",
                                                "Spiral Damping", "Mass ("+strMass+")","CoG_x ("+ strLength+")", "CoG_z ("+ strLength+")"});
}


void PlanePolar::listVariable(int iVar)
{
    if(iVar<0 || iVar>=variableCount()) return;

    for(int index=0; index<dataSize(); index++)
    {
        std::cout << std::format(" {:17g}", getVariable(iVar, index)) << std::endl;
    }
}


double PlanePolar::variable(int iVariable, int index) const
{
    if(iVariable<0 || iVariable>variableCount()) return 0.0;
    if(index<0 || index>dataSize())  return 0.0;
    return getVariable(iVariable, index);
}


double PlanePolar::getVariable(int iVar, int index) const
{
    Vector3d WindD = objects::windDirection(m_Alpha.at(index), m_Beta.at(index));
    Vector3d WindN = objects::windNormal(m_Alpha.at(index), m_Beta.at(index));

    AeroForces const &AF = m_AF.at(index);
    EigenValues const &EV = m_EV.at(index);

    double q = 0.5 * m_Density * m_QInfinite.at(index) * m_QInfinite.at(index);
    switch (iVar)
    {
        case 0:  return m_Ctrl.at(index);
        case 1:  return m_Alpha.at(index);
        case 2:  return m_Beta.at(index);
        case 3:  return m_Phi.at(index);
        case 4:  return AF.CL();
        case 5:  return AF.CD();
        case 6:  return AF.CDv();
        case 7:  return AF.CDi();
        case 8:  return AF.Cy();
        case 9:  return AF.Cm();
        case 10: return AF.Cmv();
        case 11: return AF.Cmi();
        case 12: return AF.Cli();
        case 13: return AF.Cn();
        case 14: return AF.Cnv();
        case 15: return AF.Cni();
        case 16: if(fabs(AF.CD())>PRECISION) return AF.CL()/AF.CD(); else return 0.0;
        case 17: return Cl32Cd(index);
        case 18:
        {
            if(AF.CL()>PRECISION)
            {
                double CL = AF.CL();
                return double(1./sqrt(CL));
            }
            else return 0.0;
        }
        case 19:
            return AF.Fff().dot(WindN) *q * Units::NtoUnit();
        case 20: return (AF.Fff().dot(WindD) +AF.CDv()*m_RefArea)*q * Units::NtoUnit();
        case 21: return AF.fffx() *q * Units::NtoUnit();
        case 22: return AF.fffy() *q * Units::NtoUnit();
        case 23: return AF.fffz() *q * Units::NtoUnit();
        case 24: return AF.fsumx()*q * Units::NtoUnit();
        case 25: return AF.fsumy()*q * Units::NtoUnit();
        case 26: return AF.fsumz()*q * Units::NtoUnit();
        case 27: return extraDragForce(index) * Units::NtoUnit();
        case 28:
        {
            if(!m_pPlane || !m_bFuseDrag || !m_pPlane->isXflType())
                return 0.0;
            PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);

            double fd = 0.0;
            for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++)
            {
                Fuse const*pFuse = pPlaneXfl->fuseAt(ifuse);
                fd += fuseDrag(pFuse, m_QInfinite.at(index));
            }

            return fd  * Units::NtoUnit() * q;
        }
        case 29:
        {
            // Cf_Fuse
            if(!m_pPlane || !m_bFuseDrag || !m_pPlane->isXflType() || !m_pPlane->hasFuse()) return 0.0;
            PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
            Fuse const*pFuse = pPlaneXfl->fuseAt(0);
            double Re = pFuse->length() *  m_QInfinite.at(index) / m_Viscosity;
            return fuseDragCoef(Re);
        }
        case 30: return Vx(index) * Units::mstoUnit();
        case 31: return Vz(index) * Units::mstoUnit();
        case 32: return m_QInfinite.at(index) * Units::mstoUnit();
        case 33: if(fabs(AF.CL())>PRECISION) return atan(AF.CD()/AF.CL()) * 180.0/PI;  else return 0.0;
        case 34: return AF.Mi().x*q * Units::NmtoUnit();
        case 35: return (AF.Mi().y + AF.Mv().y)*q * Units::NmtoUnit();
        case 36: return (AF.Mi().z + AF.Mv().z)*q * Units::NmtoUnit();
        case 37: return AF.centreOfPressure().x * Units::mtoUnit();
        case 38: return AF.centreOfPressure().y * Units::mtoUnit();
        case 39: return AF.centreOfPressure().z * Units::mtoUnit();
        case 40: return m_MaxBending.at(index) * Units::NmtoUnit();
        case 41: return m_Mass_var.at(index) * 9.81 * Vz(index);
        case 42: return AF.CD()*m_RefArea*q * m_QInfinite.at(index);
        case 43:
        {
            double AR = m_RefSpan*m_RefSpan/m_RefArea;
            if(fabs(AF.CDi())<PRECISION) return 0.0;
            else return AF.CL()*AF.CL()/PI/AF.CDi()/AR;
        }
        case 44: return AF.centreOfPressure().x * AF.CL();
        case 45: return m_XNP.at(index) * Units::mtoUnit();
        case 46: return EV.m_PhugoidFrequency;
        case 47: return EV.m_PhugoidDamping;
        case 48: return EV.m_ShortPeriodFrequency;
        case 49: return EV.m_ShortPeriodDamping;
        case 50: return EV.m_DutchRollFrequency;
        case 51: return EV.m_DutchRollDamping;
        case 52: return EV.m_RollDampingT2;
        case 53: return EV.m_SpiralDampingT2;
        case 54: return m_Mass_var.at(index) * Units::kgtoUnit();
        case 55: return m_CoG_x.at(index) * Units::mtoUnit();
        case 56: return m_CoG_z.at(index) * Units::mtoUnit();
        default:
            break;
    }
    return 0.0;
}


double PlanePolar::Vx(int index) const
{
    double gamma = atan(m_AF.at(index).CD()/m_AF.at(index).CL());
    return m_QInfinite.at(index) * cos(gamma);
}


double PlanePolar::Vz(int index) const
{
    double gamma = atan(m_AF.at(index).CD()/m_AF.at(index).CL());
    double Vh = sqrt(2*m_Mass_var.at(index)*9.81/m_Density/m_RefArea)/Cl32Cd(index);
    double Vg =  m_QInfinite.at(index) * sin(gamma);

    if(!isGlidePolar()) return Vh;
    return Vg;
}


double PlanePolar::Cl32Cd(int index) const
{
    double CL = m_AF.at(index).CL();
    double CD = m_AF.at(index).CD();
    if(fabs(CD)<PRECISION) return 0.0;

    if(m_AF.at(index).CL()>0.0)
    {
        return sqrt(CL*CL*CL)/CD;
    }
    else
    {
        return -sqrt(-CL*CL*CL)/CD;
    }
}


void PlanePolar::removeAoA(double alpha)
{
    for(int index=0; index<dataSize(); index++)
    {
        if(fabs(m_Alpha.at(index)-alpha)<1.e-6)
        {
            removeAt(index);
            break;
        }
    }
}


void PlanePolar::removeAt(int index)
{
    if(index<0 || index>=dataSize()) return;

    m_Alpha.erase(m_Alpha.begin()+index);
    m_Beta.erase(m_Beta.begin()+index);
    m_Phi.erase(m_Phi.begin()+index);

    m_AF.erase(m_AF.begin()+index);

    m_MaxBending.erase(m_MaxBending.begin()+index);

    m_Mass_var.erase(m_Mass_var.begin()+index);
    m_CoG_x.erase(m_CoG_x.begin()+index);
    m_CoG_z.erase(m_CoG_z.begin()+index);
    m_Ctrl.erase(m_Ctrl.begin()+index);
    m_XNP.erase(m_XNP.begin()+index);
    m_EV.erase(m_EV.begin()+index);

    m_QInfinite.erase(m_QInfinite.begin()+index);
}


void PlanePolar::insertDataPointAt(int index, bool bAfter)
{
    if(bAfter) index++;
    m_Alpha.insert(m_Alpha.begin()+index, 0);
    m_Beta.insert(m_Beta.begin()+index, 0);
    m_Phi.insert(m_Phi.begin()+index, 0);

    m_AF.insert(m_AF.begin()+index, AeroForces());

    m_MaxBending.insert(m_MaxBending.begin()+index, 0);

    m_Mass_var.insert(m_Mass_var.begin()+index, 0);
    m_CoG_x.insert(m_CoG_x.begin()+index, 0);
    m_CoG_z.insert(m_CoG_z.begin()+index, 0);
    m_Ctrl.insert(m_Ctrl.begin()+index, 0);
    m_XNP.insert(m_XNP.begin()+index, 0);
    m_EV.insert(m_EV.begin()+index, EigenValues());
    m_QInfinite.insert(m_QInfinite.begin()+index, 0);
}


/**
 *Clears the content of the data arrays
*/
void PlanePolar::clearPolarData()
{
    m_Alpha.clear();
    m_Beta.clear();
    m_Phi.clear();

    m_AF.clear();

    m_MaxBending.clear();

    m_Mass_var.clear();
    m_CoG_x.clear();
    m_CoG_z.clear();
    m_Ctrl.clear();
    m_XNP.clear();
    m_EV.clear();

    m_QInfinite.clear();
}


/**
 * Maps the inertia data from the parameter object to the polar's variables
 * @param ptr a void pointer to the reference wing or plane instance
 * @param bPlane true if the reference object is a plane, false if it is a wing
 */
void PlanePolar::retrieveInertia(const Plane *pPlane)
{
    if(!pPlane) return;
    m_Mass = pPlane->inertia().totalMass();
    m_CoG = pPlane->inertia().CoG_t();
    m_Inertia[0] = pPlane->inertia().Ixx_t();
    m_Inertia[1] = pPlane->inertia().Iyy_t();
    m_Inertia[2] = pPlane->inertia().Izz_t();
    m_Inertia[3] = pPlane->inertia().Ixz_t();

    m_InertiaRange.resize(3);
    m_InertiaRange[0].set("Mass",  pPlane->totalMass(), pPlane->totalMass());
    m_InertiaRange[1].set("CoG_x", pPlane->CoG_t().x,   pPlane->CoG_t().x);
    m_InertiaRange[2].set("CoG_z", pPlane->CoG_t().z,   pPlane->CoG_t().z);
}


void PlanePolar::copy(const PlanePolar *pWPolar)
{
    duplicateSpec(pWPolar);
    m_PlaneName = pWPolar->planeName();
    m_Name = pWPolar->name();

    clearPolarData();

    m_AF = pWPolar->m_AF;
    m_EV = pWPolar->m_EV;

    for(int i=0; i<pWPolar->dataSize(); i++)
    {
        m_Alpha.push_back(     pWPolar->m_Alpha.at(i));
        m_Beta.push_back(      pWPolar->m_Beta.at(i));
        m_Phi.push_back(       pWPolar->m_Phi.at(i));

        m_QInfinite.push_back( pWPolar->m_QInfinite.at(i));

        m_MaxBending.push_back(pWPolar-> m_MaxBending.at(i));
        m_Ctrl.push_back(      pWPolar-> m_Ctrl.at(i));
        m_XNP.push_back(       pWPolar-> m_XNP.at(i));

        m_Mass_var.push_back(  pWPolar-> m_Mass_var.at(i));
        m_CoG_x.push_back(     pWPolar-> m_CoG_x.at(i));
        m_CoG_z.push_back(     pWPolar-> m_CoG_z.at(i));

    }
}


std::string PlanePolar::flapCtrlsName() const
{
    std::string aname("Flap set name");
    if(nFlapCtrls())
    {
        if(m_FlapControls.front().name().length()==0)  return aname;

        return m_FlapControls.front().name(); // repurposing.....
    }
    return aname;
}


void PlanePolar::setFlapCtrlsName(std::string name)
{
    if(nFlapCtrls())
    {
        m_FlapControls.front().setName(name); // repurposing.....
    }

}


bool PlanePolar::checkFlaps(PlaneXfl const*pPlaneXfl, std::string &logmsg) const
{
    if(!pPlaneXfl) return false;

    std::string log;

    bool bMatch = true;
    if(nFlapCtrls() != pPlaneXfl->nWings())
    {
        log = std::format("The number of flap controls sets is {:d} "
                          "and the plane's number of wings is {:d}\n", nFlapCtrls(), pPlaneXfl->nWings());
        return false;
    }

    if(bMatch)
    {
        for(int ic=0; ic<nFlapCtrls(); ic++)
        {
            if(flapCtrls(ic).nValues() != pPlaneXfl->wingAt(ic)->nFlaps())
            {
                std::string strange = std::format("The number of flap controls for wing {:d} "
                                              "does not match the wing's number of flaps\n", ic+1);
                log += strange;
                bMatch = false;
            }
        }
    }

    logmsg = log;
    return bMatch;
}


bool PlanePolar::hasActiveFlap() const
{
    for(int ie=0; ie<nFlapCtrls(); ie++)
    {
        if(m_FlapControls.at(ie).hasActiveAngle()) return true;
    }
    return false;
}


bool PlanePolar::hasActiveAVLControl() const
{
    for(int ie=0; ie<nAVLCtrls(); ie++)
    {
        if(m_AVLControls.at(ie).hasActiveAngle()) return true;
    }
    return false;
}


std::string PlanePolar::AVLCtrlName(int ic) const
{
    if(ic>=0 && ic<int(m_AVLControls.size()))
        return m_AVLControls.at(ic).name(); else return std::string();
}


double PlanePolar::AVLGain(int iAVLCtrl, int iCtrlSurf) const
{
    if(iAVLCtrl>=0 && iAVLCtrl<int(m_AVLControls.size()))
        return m_AVLControls.at(iAVLCtrl).value(iCtrlSurf);
    else return 0.0;
}


void PlanePolar::setGain(int iAVLCtrl, int iCtrlSurf, double g)
{
    if(iAVLCtrl>=0 && iAVLCtrl<int(m_AVLControls.size()))
        m_AVLControls[iAVLCtrl].setValue(iCtrlSurf, g);
}


void PlanePolar::clearAngleRangeList()
{
    for(unsigned int iw=0; iw<m_AngleRange.size(); iw++)
    {
        for(unsigned int c=0; c<m_AngleRange.at(iw).size();  c++)
        {
            m_AngleRange[iw].clear();
        }
        m_AngleRange.clear();
    }
}


int PlanePolar::nAngleRangeCtrls() const
{
    int total=0;
    for(unsigned int iw=0; iw<m_AngleRange.size(); iw++)
    {
        total += int(m_AngleRange.at(iw).size());
    }
    return total;
}


CtrlRange PlanePolar::angleRange(int iWing, int iCtrl) const
{
    if(iWing<int(m_AngleRange.size()))
    {
        if(iCtrl<int(m_AngleRange.at(iWing).size()))
            return m_AngleRange.at(iWing).at(iCtrl);
    }
    return CtrlRange();
}


/**
 * Checks if the number of controls has changed, and if so reset all values to defaults
 */
void PlanePolar::resetAngleRanges(Plane const *pPlane)
{
    if(!pPlane || !pPlane->isXflType()) return;
    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

    int oldcount = nAngleRangeCtrls();
    int newcount = 0;
    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        newcount++;
        for(int ic=0; ic<pPlaneXfl->wingAt(iw)->nFlaps(); ic++)
        {
            newcount++;
        }
    }

    if(oldcount==newcount) return;

    clearAngleRangeList();
    std::string strong;
    int iCtrl = 0;
    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = pPlaneXfl->wingAt(iw);
        strong = pWing->name() + " Tilt";
        m_AngleRange.push_back({});
        m_AngleRange.back().push_back({strong, 0.0, 0.0});

        iCtrl++;

        int iFlapCtrl=1;
        for(int ic=0; ic<pPlaneXfl->wingAt(iw)->nFlaps(); ic++)
        {
            strong = pWing->name() + " " + std::format("Flap {:d} ", iFlapCtrl);
            m_AngleRange.back().push_back({strong, 0.0, 0.0});
            iFlapCtrl++;
            iCtrl++;
        }
    }
    (void)iCtrl;
}


void PlanePolar::resizeFlapCtrls(PlaneXfl const *pPlaneXfl)
{
    if(!pPlaneXfl) return;

    m_FlapControls.resize(pPlaneXfl->nWings());

    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = pPlaneXfl->wingAt(iw);
        resizeFlapCtrls(iw, pWing->nFlaps());
    }

    if(isType123458() || isType7())
    {
        for(int ie=0; ie<nAVLCtrls(); ie++)
            m_AVLControls[ie].resizeValues(pPlaneXfl->nAVLGains());
    }
}


void PlanePolar::resetFlapCtrls()
{
    for(int i=0; i<nFlapCtrls(); i++)
    {
        AngleControl &ctrl = m_FlapControls[i];
        for(int j=0; j<ctrl.nValues(); j++) ctrl.setValue(j,0.0);
    }
}


double PlanePolar::flapAngleValue(int iWing, int iFlap) const
{
    if(iWing>=0 && iWing<nFlapCtrls())
        return m_FlapControls.at(iWing).value(iFlap);
    else return 0.0;
}


void PlanePolar::setFlapAngleValue(int iWing, int iFlap, double g)
{
    if(iWing>=0 && iWing<nFlapCtrls())
        m_FlapControls[iWing].setValue(iFlap, g);
}


double PlanePolar::extraDragForce(int index) const
{
    double QInf = m_QInfinite.at(index);
    return extraDragTotal(m_AF[index].CL()) * 0.5 * m_Density * QInf * QInf; // N
}


void PlanePolar::getProperties(std::string &props, Plane const *pPlane) const
{
    std::string PolarProps;
    std::string strong, strange;
    std::string frontspacer("   ");

    double lenunit   = Units::mtoUnit();
    double massunit  = Units::kgtoUnit();
    double speedunit = Units::mstoUnit();
    double areaunit  = Units::m2toUnit();
    std::string lenlab   = Units::lengthUnitLabel();
    std::string masslab  = Units::massUnitLabel();
    std::string speedlab = Units::speedUnitLabel();
    std::string arealab  = Units::areaUnitLabel();

    std::string inertiaunit = masslab+"."+lenlab+ SQUAREstr;

    PlaneXfl const*pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

    PolarProps.clear();

    if     (isFixedSpeedPolar()) strong = "Type 1: Fixed speed";
    else if(isFixedLiftPolar())  strong = "Type 2: Fixed lift";
    else if(isGlidePolar())      strong = "Type 3: Speed polar";
    else if(isFixedaoaPolar())   strong = "Type 4: Fixed angle of attack" ;
    else if(isBetaPolar())       strong = "Type 5: Sideslip analysis";
    else if(isControlPolar())    strong = "Type 6: Control analysis";
    else if(isStabilityPolar())  strong = "Type 7: Stability analysis";
    else if(isType8())           strong = "Type 8";
    else if(isExternalPolar())
    {
        PolarProps = "External polar\n";
        strong = std::format("Nbr. of data points = {:d}",dataSize());
        PolarProps += strong;

        props = PolarProps;
        return;
    }
    PolarProps += strong + EOLstr;

    if(isFixedSpeedPolar())
    {
        strong  = "V" + INFstr + " =" + std::format(" {:.2g}", velocity()*speedunit);
        PolarProps += strong + speedlab+ EOLstr;
    }
    else if(isFixedaoaPolar())
    {
        strong  = ALPHAstr + " =" + std::format(" %.2f", alphaSpec());
        PolarProps += strong +DEGstr+ EOLstr;
    }
    else if(isBetaPolar())
    {
        strong  = ALPHAstr + " =" + std::format(" {:7.2f}", alphaSpec());
        PolarProps += strong +DEGstr+ EOLstr;
        strong  = "V" + INFstr + "   =" + std::format(" %9.2g", velocity()*speedunit);
        PolarProps += strong + speedlab+ EOLstr;
    }

    if(!isControlPolar() && !isBetaPolar() && fabs(betaSpec())>ANGLEPRECISION)
    {
        if(fabs(betaSpec())>AOAPRECISION)
            PolarProps += BETAstr + "  = " + std::format(" {:7.2f}", betaSpec()) + DEGstr+ EOLstr;

    }

    if(isType123458() && fabs(m_BankAngle)>ANGLEPRECISION)
    {
        if(fabs(m_BankAngle)>AOAPRECISION)
            PolarProps += PHIstr + "  = " + std::format(" {:7.2f}", m_BankAngle) + DEGstr + EOLstr;
    }


    if((isType123458() || isType7()) && hasActiveFlap() && pPlaneXfl)
    {
        PolarProps += "Flap settings: " + flapCtrlsName() + EOLstr;
        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            WingXfl const*pWing = pPlaneXfl->wingAt(iw);
            PolarProps += "   " + pWing->name() +":\n";

            if(iw<nFlapCtrls())
            {
                AngleControl const &avlc = m_FlapControls.at(iw);
                for(int iflap=0; iflap<avlc.nValues(); iflap++)
                {
                    strange = std::format("      flap {:d}: {:7.2f}", iflap+1, avlc.value(iflap)) + DEGstr + EOLstr;
                    PolarProps += strange;
                }
            }
        }
    }


    if(isControlPolar())
    {
        //operating range
        if(m_bAdjustedVelocity)
        {
            PolarProps += "   Velocity:       [  Adjusted   ]\n";
        }
        else
        {
            strange =  "   " + m_OperatingRange.at(0).name() + ": ";
            strange.resize(17, ' ');
            strong =  std::format("   {:7.3f}, {:7.3f} ", m_OperatingRange.at(0).ctrlMin()*speedunit, m_OperatingRange.at(0).ctrlMax()*speedunit);
            PolarProps += strange + strong + speedlab + EOLstr;
        }

        for(unsigned int i=1; i<m_OperatingRange.size(); i++)
        {
            strange = "   " + m_OperatingRange.at(i).name() + ": ";
            strange.resize(17, ' ');
            strong =  std::format("  {:7.3f}, {:7.3f}", m_OperatingRange.at(i).ctrlMin(), m_OperatingRange.at(i).ctrlMax());
            PolarProps += strange + strong + DEGstr + EOLstr;
        }

        //inertia
        strange = "   " + m_InertiaRange.at(0).name() + ": ";
        strange.resize(17, ' ');
        strong =  std::format(" {:7.3f}, {:7.3f} ", m_InertiaRange.at(0).ctrlMin()*massunit, m_InertiaRange.at(0).ctrlMax()*massunit);
        PolarProps += strange + strong + masslab + EOLstr;

        strange = "   " + m_InertiaRange.at(1).name() + ": ";
        strange.resize(17, ' ');
        strong =  std::format(" {:7.3f}, {:7.3f} ", m_InertiaRange.at(1).ctrlMin()*lenunit, m_InertiaRange.at(1).ctrlMax()*lenunit);
        PolarProps += strange + strong + lenlab + EOLstr;

        strange = "   " + m_InertiaRange.at(2).name() + ": ";
        strange.resize(17, ' ');
        strong =  std::format(" {:7.3f}, {:7.3f} ", m_InertiaRange.at(2).ctrlMin()*lenunit, m_InertiaRange.at(2).ctrlMax()*lenunit);
        PolarProps += strange + strong + lenlab + EOLstr;

        //Angles
        for(unsigned int j=0; j<m_AngleRange.size(); j++)
        {
            for(unsigned int i=0; i<m_AngleRange.at(j).size(); i++)
            {
                strong =  std::format(" {:7.3f}, {:7.3f}", m_AngleRange.at(j).at(i).ctrlMin(), m_AngleRange.at(j).at(i).ctrlMax());
                strange = "   " + m_AngleRange.at(j).at(i).name() + ": ";
                strange.resize(17, ' ');
                PolarProps += strange + strong +DEGstr + EOLstr;
            }
        }
    }

    if     (isLLTMethod())         PolarProps += "LLT";
    else if(isVLM1())              PolarProps += "Quads/VLM1";
    else if(isVLM2())              PolarProps += "Quads/VLM2";
    else if (isPanel4Method())     PolarProps += "Quads";
    else if(isTriUniformMethod())  PolarProps += "Triangles/Uniform doublet densities";
    else if(isTriLinearMethod())   PolarProps += "Triangles/Linear doublet densities";
    PolarProps +="\n";

    if(bThinSurfaces()) PolarProps +="Wings as thin surfaces\n";
    else                PolarProps +="Wings as thick surfaces\n";

    if(isVLM()) PolarProps += "B.C. = Neumann\n";
    else
    {
        if(boundaryCondition()==xfl::DIRICHLET)  PolarProps += "B.C.: Dirichlet\n";
        else                                         PolarProps += "B.C.: Neumann\n";
    }

    if(bTrefftz()) strong = "Lift & drag: in the far field plane";
    else           strong = "Lift & drag: summation of pressure forces";
    PolarProps += strong + EOLstr;

    if(isViscous())
    {
        if(m_bViscOnTheFly)
        {
            PolarProps += "Viscous drag: XFoil on the fly\n";
            PolarProps += std::format("   NCrit  = {:g}\n", m_NCrit);
            PolarProps += std::format("   XTrTop = {:g}% chord\n", m_XTrTop*100.0);
            PolarProps += std::format("   XTrBot = {:g}% chord\n", m_XTrBot*100.0);
            if(m_bTransAtHinge)
                PolarProps += "   Forcing transitions at hinge position\n";
        }
        else
        {
            PolarProps += "Viscous drag: interpolated";
            if(m_bViscFromCl) PolarProps += " from Cl\n";
            else              PolarProps += " from " + ALPHAstr+ EOLstr;
        }

        if(m_bViscLoop) PolarProps += "Viscous loop: enabled\n";
        else            PolarProps += "Viscous loop: disabled\n";

    }
    else  PolarProps += "Inviscid analysis\n";

    if     (referenceDim()==xfl::PLANFORM)  PolarProps += "Ref. dimensions = Planform\n";
    else if(referenceDim()==xfl::PROJECTED) PolarProps += "Ref. dimensions = Projected\n";
    else if(referenceDim()==xfl::CUSTOM)    PolarProps += "Ref. dimensions = Custom\n";

    PolarProps += frontspacer + "Area  =" + std::format("{:9.3f} ", referenceArea()       *areaunit) + arealab+ EOLstr;
    PolarProps += frontspacer + "Span  =" + std::format("{:9.3f} ", referenceSpanLength() *lenunit)  + lenlab + EOLstr;
    PolarProps += frontspacer + "Chord =" + std::format("{:9.3f} ", referenceChordLength()*lenunit)  + lenlab + EOLstr;

    if(!m_bThinSurfaces)
    {
        if(m_bWingTipMi) PolarProps += "Contribution of wing tips to moments: included\n";
        else             PolarProps += "Contribution of wing tips to moments: ignored\n";
    }


    PolarProps += "Fluid properties:\n";

    strong  = frontspacer + RHOstr + " = "+std::format("{:9.5g} ", density()*Units::densitytoUnit());
    strong += Units::densityUnitLabel() + EOLstr;
    PolarProps += strong;

    strong  = frontspacer + NUstr  + " = "+std::format("{:9.5g} ", viscosity()*Units::viscositytoUnit());
    strong += Units::viscosityUnitLabel() + EOLstr;
    PolarProps += strong;


    if(bGroundEffect())
    {
        strong = "Ground height = " + std::format(" {:7.2f} ", m_GroundHeight*lenunit)+lenlab+ EOLstr;
        PolarProps += strong;
    }
    else if(bFreeSurfaceEffect())
    {
        strong = "Free surface height = " + std::format(" {:7.2f} ", m_GroundHeight*lenunit)+lenlab+ EOLstr;
        PolarProps += strong;
    }

    //Control data
    //Mass and inertia controls
    std::string strLen, strMass, strInertia;

    strInertia = strMass+"."+strLen+SQUAREstr;

    PolarProps += "Inertia:\n";

    if(bAutoInertia())
    {
        PolarProps += frontspacer + "Using plane inertia\n";
    }

    strong  = "Mass = " + std::format(" {:.3f} ", mass()*massunit);
    PolarProps += frontspacer + strong + masslab + EOLstr;

    strong = frontspacer+"CoG = (";
    strong += std::format("{:.3f}", CoG().x*lenunit);
    strong += std::format(", {:.3f}", CoG().y*lenunit);
    strong += std::format(", {:.3f})", CoG().z*lenunit);
    PolarProps += strong + lenlab + EOLstr;

    if(isStabilityPolar())
    {
        strong  = frontspacer + "Ixx = "+std::format("%7.4g ",  Ixx()*lenunit*lenunit*massunit);
        PolarProps += strong + inertiaunit + EOLstr;

        strong  = frontspacer + "Iyy = "+std::format("%7.4g ", Iyy()*lenunit*lenunit*massunit);
        PolarProps += strong + inertiaunit + EOLstr;

        strong  = frontspacer + "Izz = "+std::format("%7.4g ", Izz()*lenunit*lenunit*massunit);
        PolarProps += strong + inertiaunit + EOLstr;

        strong  = frontspacer + "Ixz = "+std::format("%7.4g ", Ixz()*lenunit*lenunit*massunit);
        PolarProps += strong + inertiaunit + EOLstr;
    }



    if((isType123458() || isType7()) && pPlaneXfl)
    {
        if(nAVLCtrls())
        {
            PolarProps += "AVL type controls:\n";

            for(int ic=0; ic<nAVLCtrls(); ic++)
            {
                AngleControl const& avlc = m_AVLControls.at(ic);
                PolarProps += "   " + avlc.name()+ EOLstr;

                for(int ig=0; ig<avlc.nValues(); ig++)
                {
                    strange = "      " +  pPlaneXfl->controlSurfaceName(ig) + ":";
                    strange.resize(30, ' ');
                    strange += std::format(" %7.2g", avlc.value(ig)) + DEGstr+ EOLstr;

                    PolarProps += strange;
                }
            }
        }
    }


    if(pPlaneXfl && pPlaneXfl->hasFuse())
    {
        PolarProps += "Fuselage:\n";
        PolarProps += "   panels: ";
        if(bIgnoreBodyPanels()) PolarProps += "ignored\n";
        else                    PolarProps += "included\n";
        if(bFuseMi())
            PolarProps += "   contribution to moments: included\n";
        else
            PolarProps += "   contribution to moments: ignored\n";
        if(hasFuseDrag())
        {
            switch(fuseDragMethod())
            {
                case PlanePolar::KARMANSCHOENHERR:
                    PolarProps += "   drag: Karman-Schoenherr\n";
                    break;
                case PlanePolar::PRANDTLSCHLICHTING:
                    PolarProps += "   drag: Prandtl-Schlichting\n";
                    break;
                case PlanePolar::MANUALFUSECF:
                    PolarProps += std::format("   drag: Cf={:g}\n", m_FuseCf);
                    break;
            }
        }
    }

    if(extraDragCount() || m_bAVLDrag)
    {
        strong = "Extra drag:\n";
        PolarProps += strong;

        if(m_bAVLDrag) PolarProps += "   AVL drag: active\n";

        for(int ix=0; ix<extraDragCount(); ix++)
        {
            if(fabs(m_ExtraDrag[ix].area())>PRECISION && fabs(m_ExtraDrag[ix].coef())>PRECISION)
            {
                strong = "   area= " + std::format(" {:7.2f}", m_ExtraDrag.at(ix).area()*areaunit) + " ";
                strong += arealab + ",  ";
                PolarProps += strong;
                strong = "coeff.= " + std::format(" {:7.2f}", m_ExtraDrag.at(ix).coef());
                PolarProps += strong+ EOLstr;
            }
        }
    }

    if(!bVortonWake())
    {
        strong = "Flat panel wake:\n";
        PolarProps += strong;
        strong = std::format("Nb. of wake panels = {:d}\n",NXWakePanel4());
        PolarProps += frontspacer + strong;
        strong = std::format("Length             = {:g} x MAC\n", totalWakeLengthFactor());
        PolarProps += frontspacer + strong;
        strong = std::format("Progression factor = {:7.2f}", wakePanelFactor()) + EOLstr;
        PolarProps += frontspacer + strong;
    }
    else
    {
        strong = "Vorton wake:\n";
        PolarProps += strong;
        strong = std::format("Buffer wake length = {:g} x MAC\n", m_BufferWakeFactor);
        PolarProps += frontspacer + strong;
        strong = std::format("Streamwise step    = {:g} x MAC\n", m_VortonL0);
        PolarProps += frontspacer + strong;
        strong = std::format("Discard distance   = {:g} x MAC\n", m_VPWMaxLength);
        PolarProps += frontspacer + strong;
        strong = std::format("Vorton core size   = {:g} x MAC = {:g}", m_VortonCoreSize, m_VortonCoreSize*m_RefChord*Units::mtoUnit());
        strong += lenlab + EOLstr;
        PolarProps += frontspacer + strong;
        strong = std::format("VPW iterations     = {:d}", m_VPWIterations) + EOLstr;
        PolarProps += frontspacer + strong;
    }

    if(dataSize()>1)
    {
        PolarProps += "\n";
        strong = std::format("XNP = d(XCp.Cl)/dCl =  {:7.2f}", m_XNeutralPoint * lenunit);
        PolarProps += strong + " " + lenlab + EOLstr;

        strong = std::format("Static margin       = {:g}", (m_XNeutralPoint-CoG().x)/m_RefChord*100.0);
        PolarProps += strong + EOLstr;
    }

    strong = std::format("Nbr. of data points = {:d}",dataSize()) + EOLstr;
    PolarProps += strong;

    props = PolarProps;
}


std::string PlanePolar::exportToString(const std::string &separator) const
{
    std::string polardata;
    std::string sep = separator;
    std::string strong, strange, str;
    std::stringstream out;

    strong = planeName() + EOLstr;
    out <<  strong;

    strong = m_Name + EOLstr;
    out <<  strong;

    str = Units::speedUnitLabel() + EOLstr + EOLstr;

    if(isFixedSpeedPolar())
    {
        strong = std::format("Freestream speed = {:.3f} ", velocity()*Units::mstoUnit());
        strong += str+ EOLstr;
    }
    else if(isFixedaoaPolar())
    {
        strong = std::format("Alpha = {:.3f}", alphaSpec());
        strong += DEGstr+ EOLstr;
    }
    else strong = EOLstr + EOLstr;
    out <<  strong;

    for(int in=0; in<variableCount(); in++)
    {
        strange =  variableName(in);
        if(in==0) strange = " "+strange;// start with a blank space for consistency with polar data
        for(int il=int(strange.length()); il<11; il++) strange+=" ";
        out <<  strange+sep;
    }

    for(int i=0; i<dataSize(); i++)
    {
        for(int iVar=0; iVar<variableCount(); iVar++)
        {
            strange = std::format("{:11.5g}", variable(iVar, i));
            out <<  strange+sep;
        }
        out <<  "\n";
    }

    return out.str();
}


bool PlanePolar::hasPOpp(const PlaneOpp *pPOpp) const
{
    if(!pPOpp) return false;
    return (pPOpp->polarName().compare(m_Name)==0) && (pPOpp->planeName().compare(m_PlaneName)==0);
}


/** only applicable for WPolarExt */
void PlanePolar::setData(int, int, double)
{
    return;
}


/**
 * @return the extra drag force, in N/q
 */
double PlanePolar::extraDragTotal(double CL) const
{
    double extradrag = constantDrag();

    if(m_bAVLDrag)
    {
        double CDv = AVLDrag(CL);
        extradrag += CDv * m_RefArea;
    }

    return extradrag;
}


void PlanePolar::recalcExtraDrag()
{
    double staticdrag = constantDrag();

    for(int i=0; i<dataSize(); i++)
    {
        m_AF[i].setExtraDrag(staticdrag);
        if(m_bAVLDrag)
        {
            double CDv = AVLDrag(m_AF[i].CL());
            m_AF[i].addExtraDrag(CDv * m_RefArea);
        }
    }
}


double PlanePolar::QInfCtrl(double ctrl) const
{
    if(m_Type==xfl::T6POLAR)
        return m_OperatingRange.at(0).ctrlVal(ctrl);
    else return 0.0;
}


double PlanePolar::aoaCtrl(double ctrl) const
{
    if(m_Type==xfl::T6POLAR)
        return m_OperatingRange.at(1).ctrlVal(ctrl);
    else return 0.0;
}


double PlanePolar::betaCtrl(double ctrl) const
{
    if(m_Type==xfl::T6POLAR)
        return m_OperatingRange.at(2).ctrlVal(ctrl);
    else return 0.0;
}


double PlanePolar::phiCtrl(double ctrl) const
{
    if(m_Type==xfl::T6POLAR)
        return m_OperatingRange.at(3).ctrlVal(ctrl);
    else return 0.0;
}


/**
 * Returns the mass for a value of the control parameter.
  */
double PlanePolar::massCtrl(double ctrl) const
{
    if(isControlPolar())
    {
        return m_InertiaRange.at(0).ctrlVal(ctrl);
    }
    else return m_Mass;
}


/**
 * Returns the position of the CoG for a value of the control parameter.
  */
Vector3d PlanePolar::CoGCtrl(double ctrl) const
{
    if(isControlPolar())
    {
        double x = m_InertiaRange.at(1).ctrlVal(ctrl);
        double z = m_InertiaRange.at(2).ctrlVal(ctrl);
        return Vector3d(x, m_CoG.y, z);
    }
    else return m_CoG;
}


/** Returns the fuse drag as N/q */
double PlanePolar::fuseDrag(Fuse const *pFuse, double QInf) const
{
    double Reynolds = pFuse->length() * QInf / m_Viscosity;
    double Cf = fuseDragCoef(Reynolds);
    double ff = pFuse->formFactor();
    return ff * pFuse->wettedArea() * Cf;
}


double PlanePolar::fuseDragCoef(double Reynolds) const
{
    switch(m_FuseDragMethod)
    {
        case PlanePolar::KARMANSCHOENHERR:
            return KarmanSchoenherrCoef(Reynolds);
        case PlanePolar::PRANDTLSCHLICHTING:
            return PrandtlSchlichtingCoef(Reynolds);
        case PlanePolar::MANUALFUSECF:
            return m_FuseCf;
    }
    return 0.0;
}


/**
 * Calculates and returns the skin friction coefficient i.a.w. Karman-Schoenherr implicit formula
 * @param Re  the operating Renulods number
 * @return the skin friction coefficient Cf
 */
double PlanePolar::KarmanSchoenherrCoef(double Re) const
{
    if(Re<PRECISION) return 0.0;

    // Using Newton method
    // initial guess
    double Cf0 = 0.074/pow(Re, 0.20);
    double Cf=0.0;
    double err=1000.0;
    int iter = 0;
    do
    {
        double n = 0.242-sqrt(Cf0) * log10(Re*Cf0);
        double d = 0.121+sqrt(Cf0) / log10(10.0);
        Cf = Cf0 * (1+n/d);
        if(std::isnan(Cf))
        {
            std::cout << std::format("KarmanSchoenherrCoef isnan {:2d} {:.0f}", iter, Re)<<EOLstr;
            return 0.0;
        }
        err = Cf-Cf0;
        Cf0 = Cf;
        iter++;
    }
    while(fabs(err)>1.0e-5 && iter<100);

    //    double err2 = 0.242/sqrt(Cf) - log10(Re*Cf);
    //    qDebug("iter ={:3d}   Cf={:11.5g}   err={:9.5g}   errEq={:9.5g}", iter, Cf, err, err2);

    if(iter<100) return Cf;
    else         return 0.0;
}


double PlanePolar::PrandtlSchlichtingCoef(double Re) const
{
    return 0.455*pow(log10(Re), -2.58);
}



