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
                                    "Cm_pressure","Cl","Cn","Cn_viscous","Cn_pressure","CL/CD", "CL^(3/2)/CD", "1/sqrt(CL)",
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
        qDebug(" %17g", getVariable(iVar, index));
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
void PlanePolar::clearWPolarData()
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

    clearWPolarData();

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


std::string PlanePolar::flapCtrlsSetName() const
{
    std::string aname("Flap set name");
    if(nFlapCtrls())
    {
        if(m_FlapControls.front().name().length()==0)  return aname;

        return m_FlapControls.front().name(); // repurposing
    }
    return aname;
}


bool PlanePolar::checkFlaps(PlaneXfl const*pPlaneXfl, std::string &logmsg) const
{
    if(!pPlaneXfl) return false;

    QString log;

    bool bMatch = true;
    if(nFlapCtrls() != pPlaneXfl->nWings())
    {
        log = QString::asprintf("The number of flap controls sets is %d "
                          "and the plane's number of wings is %d\n", nFlapCtrls(), pPlaneXfl->nWings());
        return false;
    }

    if(bMatch)
    {
        for(int ic=0; ic<nFlapCtrls(); ic++)
        {
            if(flapCtrls(ic).nValues() != pPlaneXfl->wingAt(ic)->nFlaps())
            {
                QString strange = QString::asprintf("The number of flap controls for wing %d "
                                              "does not match the wing's number of flaps\n", ic+1);
                log += strange;
                bMatch = false;
            }
        }
    }

    logmsg = log.toStdString();
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
    for(uint iw=0; iw<m_AngleRange.size(); iw++)
    {
        for(uint c=0; c<m_AngleRange.at(iw).size();  c++)
        {
            m_AngleRange[iw].clear();
        }
        m_AngleRange.clear();
    }
}


int PlanePolar::nAngleRangeCtrls() const
{
    int total=0;
    for(uint iw=0; iw<m_AngleRange.size(); iw++)
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
            strong = pWing->name() + " " + QString::asprintf("Flap %d ", iFlapCtrl).toStdString();
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


bool PlanePolar::serializeWPlrXFL(QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    int i(0), k(0), n(0);
    double dble(0);
    double r0(0), r1(0), r2(0), r3(0), r4(0), r5(0), r6(0), r7(0);
    double i0(0), i1(0), i2(0), i3(0), i4(0), i5(0), i6(0), i7(0);
    QString strange;

    m_PolarFormat = 200013;
    // 200013: v0.00
    // 200014:  added array of control values for stability polars
    // 200015:  added array of control values for controls polars

    if(bIsStoring)
    {
        // STORING REQUIRED TO EXPORT .xfl temp file for foil analysis
        //output the variables to the stream
        ar << m_PolarFormat;

        ar << QString::fromStdString(m_PlaneName);
        ar << QString::fromStdString(m_Name);

        ar << dble << dble << dble;
        ar << 0 << 1;

        xfl::writeColor(ar, 255, 0, 0, 255);
        ar << true << false;

        ar<<1;

        ar<<1;

        ar << false;
        ar << m_bThinSurfaces;
        ar << boolean; // m_bTiltedGeom;
        ar << true;
        ar << m_bViscous;
        ar << m_bIgnoreBodyPanels;

        ar << m_bGround;
        ar << 0.0;

        ar << m_Density << m_Viscosity;

        ar << 2;

        ar << m_bAutoInertia;
        ar << m_Mass;


        ar << dble  << dble  << dble; // formerly CoG moved to Polar3d
        ar << dble << dble << dble << dble;// formerly Inertia tensor moved to Polar3d

        ar << k;

        ar << m_nXWakePanel4 << double(30.0) << double(1.15);

        ar << m_QInfSpec;
        ar << m_AlphaSpec;
        ar << m_BetaSpec;

        // Last store the array data
        ar <<0;


        // space allocation for the future storage of more data, without need to change the format
        for (int i=0; i<20; i++) ar << 0;
        for (int i=0; i<35; i++) ar << dble;
        for (int ix=0; ix<4; ix++) ar << dble;
        for (int ix=0; ix<4; ix++) ar << dble;
        for (int i=0; i<7; i++) ar << dble;

        return true;
    }
    else
    {
        //input the variables from the stream
        ar >> m_PolarFormat;
        if(m_PolarFormat<200000 || m_PolarFormat>205000) return false;

        ar >> strange;   m_PlaneName = strange.toStdString();;
        ar >> strange;   m_Name = strange.toStdString();;

        ar >> m_RefArea >> m_RefChord >> m_RefSpan;
        if(m_PolarFormat<200014)
        {
            ar >> k;
            m_theStyle.setStipple(k);
            ar >> m_theStyle.m_Width;

            m_theStyle.m_Color.serialize(ar, false);

            ar >> m_theStyle.m_bIsVisible >> boolean;
        }
        else
            m_theStyle.serializeXfl(ar, bIsStoring);

        ar >> n;
        if     (n==1) m_AnalysisMethod=xfl::LLT;
        else if(n==2) m_AnalysisMethod=xfl::VLM1;
        else if(n==3) m_AnalysisMethod=xfl::QUADS;
        else if(n==4) m_AnalysisMethod=xfl::TRILINEAR;
        else if(n==5) m_AnalysisMethod=xfl::TRIUNIFORM;

        ar >> n;
        if     (n==1) m_Type=xfl::T1POLAR;
        else if(n==2) m_Type=xfl::T2POLAR;
        else if(n==4) m_Type=xfl::T4POLAR;
        else if(n==5) m_Type=xfl::T5POLAR;
        else if(n==6) m_Type=xfl::T6POLAR;
        else if(n==7) m_Type=xfl::T7POLAR;

        ar >> boolean;
        if(isVLM1())
        {
            if(!boolean) m_AnalysisMethod=xfl::VLM2;
        }

        ar >> m_bThinSurfaces;
        if(m_bThinSurfaces&&!isLLTMethod())
        {
            if(boolean) m_AnalysisMethod=xfl::VLM1;
            else        m_AnalysisMethod=xfl::VLM2;
        }

        if(isTriangleMethod()) m_bThinSurfaces = false;  // cleaning up incorrectly constructed polars

        ar >> boolean; // m_bTiltedGeom;
        ar >> boolean;
        m_BC = boolean? xfl::DIRICHLET : xfl::NEUMANN;

        ar >> m_bViscous;        m_bViscFromCl = true;

        ar >> m_bIgnoreBodyPanels;
        if(isVLM()) m_bIgnoreBodyPanels = true; //clean up override

        ar >> m_bGround;
        ar >> m_GroundHeight;

        ar >> m_Density >> m_Viscosity;

        ar >> k;
        if     (k==1) m_ReferenceDim = xfl::PLANFORM;
        else if(k==2) m_ReferenceDim = xfl::PROJECTED;
        else if(k==3) m_ReferenceDim = xfl::CUSTOM;
        else          m_ReferenceDim = xfl::PLANFORM;

        ar >> m_bAutoInertia;
        ar >> m_Mass;

        ar >> dble >> dble >> dble; // CoGxyz moved to Polar3d
        ar >> dble >> dble >> dble >> dble; //Inertia tensor moved to polar3d

/*        ar >> m_CoG.x >> m_CoG.y >> m_CoG.z;
        ar >> m_Inertia[0] >> m_Inertia[1] >> m_Inertia[2] >> m_Inertia[3];*/

        ar >> k;

        for(int icg=0; icg<k; icg++)
        {
            ar >> dble;
//            m_ControlGain[icg] = dble;
        }

        ar >> m_nXWakePanel4 >> m_TotalWakeLengthFactor >> m_WakePanelFactor;
        ar >> m_QInfSpec;
        ar >> m_AlphaSpec;
        ar >> m_BetaSpec;

        // Last store the array data
        // assumes the arrays have been cleared previously
        double d[20];
        clearWPolarData();

        ar >> n;
        if(abs(n)>10000) return false;

        for (i=0; i<n; i++)
        {
            for(int j=0; j<20; j++)
            {
                ar >> d[j];
            }
            //            insertDataAt(i, d[0],  d[1],  d[2],  d[3],  d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13],
            //                            d[14], d[15], d[16], d[17], d[18], d[19]);
            /*            void WPolar::insertDataAt(int pos, double Alpha, double Beta, double QInf, double Ctrl, double CL, double CY,
 *                                    double ICd, double VCd,
                                      double GCm, double ICm, double VCm, double GRm, double GYm, double IYm, double VYm,
                                      double XCP, double YCP, double ZCP,
                                      double Cb, double XNP)*/
            //            double alpha = d[0];
            //            double beta = d[1];
            //            double QInf = d[2];
            //            double ctrl = d[3];
            double CL   = d[4];
            double CY   = d[5];
            double ICd  = d[6];
            double VCd  = d[7];
            //            double GCm  = d[8];
            double ICm  = d[9];
            double VCm  = d[10];
            double GRm = d[11];
            //            double GYm = d[12];
            double IYm = d[13];
            double VYm = d[14];
/*            double XCP = d[15];
            double YCP = d[16];
            double ZCP = d[17];*/

            //rebuild aeroforces
            double cosa = cos(d[0]*PI/180.0);
            double sina = sin(d[0]*PI/180.0);

            AeroForces AF;
            AF.setReferenceArea(m_RefArea);
            AF.setReferenceChord(m_RefChord);
            AF.setReferenceSpan(m_RefSpan);
            AF.setFff({(ICd*cosa-CL*sina)*m_RefArea, CY*m_RefArea, (ICd*sina+CL*cosa)*m_RefArea}); // N/q
            AF.setProfileDrag(VCd*m_RefArea);
            AF.setMi({GRm*m_RefSpan*m_RefArea, ICm*m_RefChord*m_RefArea, IYm*m_RefSpan*m_RefArea});
            AF.setMv({0.0,                     VCm*m_RefChord*m_RefArea, VYm*m_RefSpan*m_RefArea});
//            AF.setCP({XCP, YCP, ZCP});
            m_AF.push_back(AF);

            insertDataAt(i, d[0],d[1],0.0,d[2],d[3],d[4],d[5]);

            ar >> r0 >> r1 >>r2 >> r3;
            ar >> i0 >> i1 >>i2 >> i3;
            ar >> r4 >> r5 >>r6 >> r7;
            ar >> i4 >> i5 >>i6 >> i7;

            m_EV.push_back(EigenValues());
            m_EV[i].m_EV[0] = std::complex<double>(r0, i0);
            m_EV[i].m_EV[1] = std::complex<double>(r1, i1);
            m_EV[i].m_EV[2] = std::complex<double>(r2, i2);
            m_EV[i].m_EV[3] = std::complex<double>(r3, i3);
            m_EV[i].m_EV[4] = std::complex<double>(r4, i4);
            m_EV[i].m_EV[5] = std::complex<double>(r5, i5);
            m_EV[i].m_EV[6] = std::complex<double>(r6, i6);
            m_EV[i].m_EV[7] = std::complex<double>(r7, i7);

        }

        // space allocation
        // integers
        for (int i=0; i<15; i++) ar >> k;

        m_bVortonWake=false;
        ar >> k; //m_nWakeIterations; m_nWakeIterations=std::max(m_nWakeIterations, 1);
        ar >> k;
        ar >> k;
        ar >> k;    m_bAdjustedVelocity = (k ? true : false);
        ar >> k;    m_theStyle.m_Symbol=LineStyle::convertSymbol(k);

        // double
        for (int i=0; i<29; i++) ar >> dble;
        ar >> dble >> dble >> dble;
        ar >> m_NCrit >> m_XTrTop >> m_XTrBot;
        if(fabs(m_NCrit)<PRECISION)
        {
            m_NCrit=9.0;   m_XTrTop=1.0;   m_XTrBot=1.0;
        }

        m_ExtraDrag.clear();
        m_ExtraDrag.resize(4);
        for (int ix=0; ix<4; ix++)
        {
            std::string strong;
            strong = QString::asprintf("Extra drag %d", ix).toStdString();
            m_ExtraDrag[ix].setName(strong);
        }
        for (int ix=0; ix<4; ix++) {ar>>dble; m_ExtraDrag[ix].setArea(dble);}
        for (int ix=0; ix<4; ix++) {ar>>dble; m_ExtraDrag[ix].setCoef(dble);}
        //clear the null extradrag
        for (int ix=3; ix>=0; ix--)
        {
            ExtraDrag &xd = m_ExtraDrag[ix];
            if(fabs(xd.area())<PRECISION && fabs(xd.coef())<PRECISION) m_ExtraDrag.erase(m_ExtraDrag.begin()+ix);
        }

        if(m_PolarFormat<200013)
        {
/*            for (int ix=0; ix<MAXEXTRADRAG; ix++)
            {
                    m_ExtraDragArea[ix] = 0.0;
                    m_ExtraDragCoef[ix] = 0.0;
            }*/
        }

        for (int i=0; i<7; i++)
        {
            ar >> dble; // m_InertiaGain
        }

        for(int iPt=0; iPt<dataSize(); iPt++)    calculatePoint(iPt);

        return true;
    }
}


bool PlanePolar::serializeFl5v726(QDataStream &ar, bool bIsStoring)
{
    if(!Polar3d::serializeFl5v726(ar, bIsStoring)) return false;

    int k(0), n(0);
    int nIntSpares(0);
    int nDbleSpares(0);

    double dble(0), dmin(0), dmax(0);

    QString strange;

    if(bIsStoring)
    {
        assert(false);
    }
    else
    {
        //input the variables from the stream

        // METADATA
        if ((m_PolarFormat<500001) || (m_PolarFormat>500100))
            return false;

        ar >> strange;  m_PlaneName = strange.toStdString();;

        ar >> m_QInfSpec;
        ar >> m_AlphaSpec;

        ar >> m_bThinSurfaces;

        // REFERENCE DIMENSIONS
        ar >> k;
        if     (k==1) m_ReferenceDim = xfl::PLANFORM;
        else if(k==2) m_ReferenceDim = xfl::PROJECTED;
        else if(k==3) m_ReferenceDim = xfl::CUSTOM;
        else          m_ReferenceDim = xfl::PLANFORM;
        ar >> m_RefArea >> m_RefChord >> m_RefSpan;

        if(m_PolarFormat<500020) m_VortonCoreSize /= m_RefChord;

        // AVL type control ranges
        if(m_PolarFormat>=500021)
        {
            ar >> n;
            m_AVLControls.resize(n);
            for(int ic=0; ic<nAVLCtrls(); ic++) m_AVLControls[ic].serializeFl5(ar, bIsStoring);

//            if(m_Type!=xfl::T6POLAR) m_AVLControls.clear(); // cleaning up
        }

        //STABILITY POLAR ANGLE AND INERTIA GAINS
        int nCtrls;
        ar>>nCtrls; // formerly m_AngleCoef.size() - deprecated in v713
        if(nCtrls<0 || n>10000) return false;
        for(int iw=0; iw<nCtrls; iw++)
        {
            ar>>k;
            for(int ic=0; ic<k; ic++)
            {
                if(m_PolarFormat>=500007)
                {
                    ar>>dble;
                }
                ar>>dble;  // m_AngleGain[iw].push_back(dble);
            }
        }

        for (int i=0; i<7; i++) ar >> dble; // m_InertiaGain

        ar >> k;    m_bAdjustedVelocity = (k ? true : false);

        //CONTROL POLAR RANGES
        clearAngleRangeList();
        ar>>n;
        m_AngleRange.resize(n);
        for(uint iw=0; iw<m_AngleRange.size(); iw++)
        {
            ar>>n;
            for(int c=0; c<n; c++)
            {
                ar >> strange;
                ar >> dmin>> dmax;
                m_AngleRange[iw].push_back({strange.toStdString(), dmin, dmax});
            }
        }

        if(m_PolarFormat>=500002)
        {
            // formerly LE range
            ar>>n;
            for(int c=0; c<n; c++)
            {
                ar >> strange;
                ar >> dmin>> dmax;
            }
        }

        if(m_PolarFormat>=500003)
        {
            // formerly Wing Shape ranges
            ar>>n;
            for(int iw=0; iw<n; iw++)
            {
                ar>>k;
                for(int c=0; c<k; c++)
                {
                    ar >> strange;
                    ar >> dmin>> dmax;
                }
            }
        }

        m_OperatingRange.clear();
        ar>>n;
        for(int c=0; c<n; c++)
        {
            ar >> strange;
            ar >> dmin>> dmax;
            m_OperatingRange.push_back({strange.toStdString(), dmin, dmax});
        }
        if(m_OperatingRange.size()<4)
        {
            m_OperatingRange.resize(4);
            m_OperatingRange[3].set("phi", 0.0, 0.0);
        }

        // update old formats
        m_OperatingRange[0].setName("Vinf");
        m_OperatingRange[1].setName("alpha");
        m_OperatingRange[2].setName("beta");
        m_OperatingRange[3].setName("phi");

        m_InertiaRange.clear();
        ar>>n;
        for(int c=0; c<n; c++)
        {
            ar >> strange;
            ar >> dmin>> dmax;
            m_InertiaRange.push_back({strange.toStdString(), dmin, dmax});
        }

        // FUSE DATA
        if(m_PolarFormat>=500004)
        {
            ar >> k;    m_bFuseMi = (k ? true : false);
        }
        ar >> k;        m_bFuseDrag = (k ? true : false);
        ar >> k;
        if     (k==1) m_FuseDragMethod=PlanePolar::KARMANSCHOENHERR;
        else if(k==2) m_FuseDragMethod=PlanePolar::PRANDTLSCHLICHTING;
        else          m_FuseDragMethod=PlanePolar::MANUALFUSECF;

        // Last store the array data
        // assumes the arrays have been cleared previously
        double d[20];
        clearWPolarData();

        int nSpares(0);
        ar >> nSpares;
        ar >> n;
        if(abs(n)>10000) return false;

        for (int i=0; i<n; i++)
        {
            AeroForces AC;
            if(m_PolarFormat<500022) AC.serializeFl5_b17(ar, bIsStoring);
            else
            {
                 if(!AC.serializeFl5(ar, bIsStoring))
                     return false;
            }
            m_AF.push_back(AC);
            for(int j=0; j<6; j++) ar >> d[j];

            insertDataAt(i, d[0], d[1], 0.0, d[2], d[3], d[4], d[5]);

            EigenValues EV;
            EV.serializeFl5(ar, bIsStoring);
            m_EV.push_back(EV);

            if(nSpares>=1)
                ar >> m_Phi[i];
            for(int i=1; i<nSpares; i++) ar >> dble;

            m_AF.back().setOpp(m_Alpha.at(i), m_Beta.at(i), m_Phi.at(i), m_QInfinite.at(i));
        }

        if(m_PolarFormat>=500017)
        {
            ar >> m_bAVLDrag;
            m_AVLSpline.serializeFl5(ar, bIsStoring);
        }

        // space allocation
        ar >> nIntSpares; // 500005: nIntSpares=1
        if(nIntSpares>=1)
        {
            if(nIntSpares>=1)
            {
                 ar >> n;
                 m_bOtherWingsArea = n==1 ? true : false;
            }

            m_bWingTipMi = m_bThinSurfaces ? false : true; // default for polars saved prior to v7.24
            if(nIntSpares>=2)
            {
                 ar >> n;
                 m_bWingTipMi = n==1 ? true : false;
            }
        }

        ar >> nDbleSpares;
        if(nDbleSpares>=1)
                ar >> m_FuseCf;

        for(int iPt=0; iPt<dataSize(); iPt++) calculatePoint(iPt);
    }
    return true;
}


bool PlanePolar::serializeFl5v750(QDataStream &ar, bool bIsStoring)
{
    if(!Polar3d::serializeFl5v750(ar, bIsStoring)) return false;

    int k(0), n(0);
    bool boolean(false);
    int integer(0);
    double dble(0);
    double dmin(0), dmax(0);

    QString strange;

    if(bIsStoring)
    {
        //METADATA
        ar << QString::fromStdString(m_PlaneName);

        ar << m_QInfSpec;
        ar << m_AlphaSpec;

        ar << m_bThinSurfaces;

        ar << m_bViscLoop;

        // REFERENCE DIMENSIONS
        if     (m_ReferenceDim == xfl::PLANFORM)  ar << 1;
        else if(m_ReferenceDim == xfl::PROJECTED) ar << 2;
        else if(m_ReferenceDim == xfl::CUSTOM)    ar << 3;
        ar << m_RefArea << m_RefChord << m_RefSpan;

        // TE flap angles
        ar << nFlapCtrls();
        for(int ic=0; ic<nFlapCtrls(); ic++) m_FlapControls[ic].serializeFl5(ar, bIsStoring);

        // AVL type control ranges
        ar << int(nAVLCtrls());
        for(int ic=0; ic<nAVLCtrls(); ic++) m_AVLControls[ic].serializeFl5(ar, bIsStoring);

        ar << m_bAdjustedVelocity;

        ar << int(m_AngleRange.size());
        for(uint icg=0; icg<m_AngleRange.size(); icg++)
        {
            ar << int(m_AngleRange.at(icg).size());
            for(uint jcg=0; jcg<m_AngleRange.at(icg).size(); jcg++)
            {
                ar << QString::fromStdString(m_AngleRange.at(icg).at(jcg).name());
                ar << m_AngleRange.at(icg).at(jcg).ctrlMin() << m_AngleRange.at(icg).at(jcg).ctrlMax();
            }
        }

        //Operating point range
        ar << int(m_OperatingRange.size());
        for(uint jcg=0; jcg<m_OperatingRange.size(); jcg++)
        {
            ar << QString::fromStdString(m_OperatingRange.at(jcg).name());
            ar << m_OperatingRange.at(jcg).ctrlMin() << m_OperatingRange.at(jcg).ctrlMax();
        }

        //Inertia range
        ar << int(m_InertiaRange.size());
        for(uint jcg=0; jcg<m_InertiaRange.size(); jcg++)
        {
            ar << QString::fromStdString(m_InertiaRange.at(jcg).name());
            ar << m_InertiaRange.at(jcg).ctrlMin() << m_InertiaRange.at(jcg).ctrlMax();
        }

        // FUSE DATA
        ar << m_bFuseMi;
        ar << m_bFuseDrag;

        switch(m_FuseDragMethod)
        {
            case PlanePolar::MANUALFUSECF:         ar<<0;  break;
            case PlanePolar::KARMANSCHOENHERR:     ar<<1;  break;
            case PlanePolar::PRANDTLSCHLICHTING:   ar<<2;  break;
        }

        // Last store the array data
        int nSpares=0;
        ar << nSpares;
        ar << dataSize();

        for (int i=0; i<dataSize(); i++)
        {
            m_AF[i].serializeFl5(ar, bIsStoring);

            ar << m_Alpha.at(i) << m_Beta.at(i) << m_Phi.at(i) << m_QInfinite.at(i) << m_Ctrl.at(i);

            ar << m_MaxBending.at(i);
            ar << m_XNP.at(i);

            m_EV[i].serializeFl5(ar, bIsStoring);

            for(int js=0; js<nSpares; js++) ar<<dble;  // nSpares=0 anyway in v750
        }


        ar << m_bAVLDrag;
        m_AVLSpline.serializeFl5(ar, bIsStoring);

        ar << m_bOtherWingsArea;
        ar << m_bWingTipMi;
        ar << m_FuseCf;

        // provisions for future variable saves
        for(int i=0; i<10; i++) ar <<boolean;
        for(int i=0; i<20; i++) ar <<integer;
        for(int i=0; i<20; i++) ar <<dble;

        return true;
    }
    else
    {
        // METADATA
        if ((m_PolarFormat<500750) || (m_PolarFormat>501000)) // failsafe
            return false;

        ar >> strange;    m_PlaneName = strange.toStdString();;

        ar >> m_QInfSpec;
        ar >> m_AlphaSpec;

        ar >> m_bThinSurfaces;
        ar >> m_bViscLoop;


        // REFERENCE DIMENSIONS
        ar >> k;
        if     (k==1) m_ReferenceDim = xfl::PLANFORM;
        else if(k==2) m_ReferenceDim = xfl::PROJECTED;
        else if(k==3) m_ReferenceDim = xfl::CUSTOM;
        else          m_ReferenceDim = xfl::PLANFORM;
        ar >> m_RefArea >> m_RefChord >> m_RefSpan;

        // TE flap angles
        ar >> n;
        m_FlapControls.resize(n);
        for(int ic=0; ic<nFlapCtrls(); ic++) m_FlapControls[ic].serializeFl5(ar, bIsStoring);

        // AVL type control ranges
        ar >> n;
        m_AVLControls.resize(n);
        for(int ic=0; ic<nAVLCtrls(); ic++) m_AVLControls[ic].serializeFl5(ar, bIsStoring);


        ar >> m_bAdjustedVelocity;

        //CONTROL POLAR RANGES
        clearAngleRangeList();
        ar>>n;
        m_AngleRange.resize(n);
        for(uint iw=0; iw<m_AngleRange.size(); iw++)
        {
            ar>>n;
            for(int c=0; c<n; c++)
            {
                ar >> strange;
                ar >> dmin>> dmax;
                m_AngleRange[iw].push_back({strange.toStdString(), dmin, dmax});
            }
        }

        m_OperatingRange.clear();
        ar>>n;
        for(int c=0; c<n; c++)
        {
            ar >> strange;
            ar >> dmin>> dmax;

            m_OperatingRange.push_back({strange.toStdString(), dmin, dmax});
        }


        m_InertiaRange.clear();
        ar>>n;
        for(int c=0; c<n; c++)
        {
            ar >> strange;
            ar >> dmin>> dmax;
            m_InertiaRange.push_back({strange.toStdString(), dmin, dmax});
        }

        // FUSE DATA

        ar >> m_bFuseMi;
        ar >> m_bFuseDrag;

        ar >> k;
        if     (k==1) m_FuseDragMethod=PlanePolar::KARMANSCHOENHERR;
        else if(k==2) m_FuseDragMethod=PlanePolar::PRANDTLSCHLICHTING;
        else          m_FuseDragMethod=PlanePolar::MANUALFUSECF;

        // Last store the array data
        // assumes the arrays have been cleared previously
        double d[20];
        clearWPolarData();

        int nSpares(0);
        ar >> nSpares;
        ar >> n;
        if(abs(n)>10000) return false;

        for (int i=0; i<n; i++)
        {
            AeroForces AC;

            if(!AC.serializeFl5(ar, bIsStoring))
                     return false;

            m_AF.push_back(AC);
            for(int j=0; j<7; j++) ar >> d[j];

            insertDataAt(i, d[0], d[1], d[2], d[3], d[4], d[5], d[6]);

            EigenValues EV;
            EV.serializeFl5(ar, bIsStoring);
            m_EV.push_back(EV);


            for(int i=0; i<nSpares; i++) ar >> dble;

            m_AF.back().setOpp(m_Alpha.at(i), m_Beta.at(i), m_Phi.at(i), m_QInfinite.at(i));
        }

        ar >> m_bAVLDrag;
        m_AVLSpline.serializeFl5(ar, bIsStoring);


        ar >> m_bOtherWingsArea;
        ar >> m_bWingTipMi;
        ar >> m_FuseCf;

        // provisions for future variable saves
        for(int i=0; i<10; i++) ar >> boolean;
        for(int i=0; i<20; i++) ar >> integer;
        for(int i=0; i<20; i++) ar >> dble;


        for(int iPt=0; iPt<dataSize(); iPt++) calculatePoint(iPt);
        return true;
    }
}


void PlanePolar::getProperties(std::string &props, Plane const *pPlane) const
{
    QString PolarProps;
    QString strong, strange;
    QString frontspacer("   ");

    double lenunit   = Units::mtoUnit();
    double massunit  = Units::kgtoUnit();
    double speedunit = Units::mstoUnit();
    double areaunit  = Units::m2toUnit();
    QString lenlab   = Units::lengthUnitQLabel();
    QString masslab  = Units::massUnitQLabel();
    QString speedlab = Units::speedUnitQLabel();
    QString arealab  = Units::areaUnitQLabel();

    QString inertiaunit = masslab+"."+lenlab+ SQUAREch;

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
        strong = QString::asprintf("Nbr. of data points = %d",dataSize());
        PolarProps += strong;

        props = PolarProps.toStdString();
        return;
    }
    PolarProps += strong + EOLch;

    if(isFixedSpeedPolar())
    {
        strong  = "V" + INFch + " =" + QString::asprintf(" %.2g", velocity()*speedunit);
        PolarProps += strong + speedlab+ EOLch;
    }
    else if(isFixedaoaPolar())
    {
        strong  = ALPHAch + " =" + QString::asprintf(" %.2f", alphaSpec());
        PolarProps += strong +DEGch+ EOLch;
    }
    else if(isBetaPolar())
    {
        strong  = ALPHAch + " =" + QString::asprintf(" %7.2f", alphaSpec());
        PolarProps += strong +DEGch+ EOLch;
        strong  = "V" + INFch + "   =" + QString::asprintf(" %9.2g", velocity()*speedunit);
        PolarProps += strong + speedlab+ EOLch;
    }

    if(!isControlPolar() && !isBetaPolar() && fabs(betaSpec())>ANGLEPRECISION)
    {
        if(fabs(betaSpec())>AOAPRECISION)
            PolarProps += BETAch + "  = " + QString::asprintf(" %7.2f", betaSpec()) + DEGch+ EOLch;

    }

    if(isType123458() && fabs(m_BankAngle)>ANGLEPRECISION)
    {
        if(fabs(m_BankAngle)>AOAPRECISION)
            PolarProps += PHIch + "  = " + QString::asprintf(" %7.2f", m_BankAngle) + DEGch + EOLch;
    }


    if((isType123458() || isType7()) && hasActiveFlap() && pPlaneXfl)
    {
        PolarProps += "Flap settings: " + QString::fromStdString(flapCtrlsSetName()) + EOLch;
        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            WingXfl const*pWing = pPlaneXfl->wingAt(iw);
            PolarProps += "   " + QString::fromStdString(pWing->name()) +":\n";

            if(iw<nFlapCtrls())
            {
                AngleControl const &avlc = m_FlapControls.at(iw);
                for(int iflap=0; iflap<avlc.nValues(); iflap++)
                {
                    strange = QString::asprintf("      flap %d: %7.2f", iflap+1, avlc.value(iflap)) + DEGch + EOLch;
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
            strange =  "   " + QString::fromStdString(m_OperatingRange.at(0).name()) + ": ";
            strange.resize(17, ' ');
            strong =  QString::asprintf("   %7.3f, %7.3f ", m_OperatingRange.at(0).ctrlMin()*speedunit, m_OperatingRange.at(0).ctrlMax()*speedunit);
            PolarProps += strange + strong + speedlab + EOLch;
        }

        for(uint i=1; i<m_OperatingRange.size(); i++)
        {
            strange = "   " + QString::fromStdString(m_OperatingRange.at(i).name()) + ": ";
            strange.resize(17, ' ');
            strong =  QString::asprintf("  %7.3f, %7.3f", m_OperatingRange.at(i).ctrlMin(), m_OperatingRange.at(i).ctrlMax());
            PolarProps += strange + strong + DEGch + EOLch;
        }

        //inertia
        strange = "   " + QString::fromStdString(m_InertiaRange.at(0).name()) + ": ";
        strange.resize(17, ' ');
        strong =  QString::asprintf(" %7.3f, %7.3f ", m_InertiaRange.at(0).ctrlMin()*massunit, m_InertiaRange.at(0).ctrlMax()*massunit);
        PolarProps += strange + strong + masslab + EOLch;

        strange = "   " + QString::fromStdString(m_InertiaRange.at(1).name()) + ": ";
        strange.resize(17, ' ');
        strong =  QString::asprintf(" %7.3f, %7.3f ", m_InertiaRange.at(1).ctrlMin()*lenunit, m_InertiaRange.at(1).ctrlMax()*lenunit);
        PolarProps += strange + strong + lenlab + EOLch;

        strange = "   " + QString::fromStdString(m_InertiaRange.at(2).name()) + ": ";
        strange.resize(17, ' ');
        strong =  QString::asprintf(" %7.3f, %7.3f ", m_InertiaRange.at(2).ctrlMin()*lenunit, m_InertiaRange.at(2).ctrlMax()*lenunit);
        PolarProps += strange + strong + lenlab + EOLch;

        //Angles
        for(uint j=0; j<m_AngleRange.size(); j++)
        {
            for(uint i=0; i<m_AngleRange.at(j).size(); i++)
            {
                strong =  QString::asprintf(" %7.3f, %7.3f", m_AngleRange.at(j).at(i).ctrlMin(), m_AngleRange.at(j).at(i).ctrlMax());
                strange = "   " + QString::fromStdString(m_AngleRange.at(j).at(i).name()) + ": ";
                strange.resize(17, ' ');
                PolarProps += strange + strong +DEGch + EOLch;
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
    PolarProps += strong + EOLch;

    if(isViscous())
    {
        if(m_bViscOnTheFly)
        {
            PolarProps += "Viscous drag: XFoil on the fly\n";
            PolarProps += QString::asprintf("   NCrit  = %g\n", m_NCrit);
            PolarProps += QString::asprintf("   XTrTop = %g%% chord\n", m_XTrTop*100.0);
            PolarProps += QString::asprintf("   XTrBot = %g%% chord\n", m_XTrBot*100.0);
            if(m_bTransAtHinge)
                PolarProps += "   Forcing transitions at hinge position\n";
        }
        else
        {
            PolarProps += "Viscous drag: interpolated";
            if(m_bViscFromCl) PolarProps += " from Cl\n";
            else              PolarProps += " from " + ALPHAch+ EOLch;
        }

        if(m_bViscLoop) PolarProps += "Viscous loop: enabled\n";
        else            PolarProps += "Viscous loop: disabled\n";

    }
    else  PolarProps += "Inviscid analysis\n";

    if     (referenceDim()==xfl::PLANFORM)  PolarProps += "Ref. dimensions = Planform\n";
    else if(referenceDim()==xfl::PROJECTED) PolarProps += "Ref. dimensions = Projected\n";
    else if(referenceDim()==xfl::CUSTOM)    PolarProps += "Ref. dimensions = Custom\n";

    PolarProps += frontspacer + "Area  =" + QString::asprintf("%9.3f ", referenceArea()       *areaunit) + arealab+ EOLch;
    PolarProps += frontspacer + "Span  =" + QString::asprintf("%9.3f ", referenceSpanLength() *lenunit)  + lenlab + EOLch;
    PolarProps += frontspacer + "Chord =" + QString::asprintf("%9.3f ", referenceChordLength()*lenunit)  + lenlab + EOLch;

    if(!m_bThinSurfaces)
    {
        if(m_bWingTipMi) PolarProps += "Contribution of wing tips to moments: included\n";
        else             PolarProps += "Contribution of wing tips to moments: ignored\n";
    }


    PolarProps += "Fluid properties:\n";

    strong  = frontspacer + RHOch + " = "+QString::asprintf("%9.5g ", density()*Units::densitytoUnit());
    strong += Units::densityUnitQLabel() + EOLch;
    PolarProps += strong;

    strong  = frontspacer + NUch  + " = "+QString::asprintf("%9.5g ", viscosity()*Units::viscositytoUnit());
    strong += Units::viscosityUnitQLabel() + EOLch;
    PolarProps += strong;


    if(bGroundEffect())
    {
        strong = "Ground height = " + QString::asprintf(" %7.2f ", m_GroundHeight*lenunit)+lenlab+ EOLch;
        PolarProps += strong;
    }
    else if(bFreeSurfaceEffect())
    {
        strong = "Free surface height = " + QString::asprintf(" %7.2f ", m_GroundHeight*lenunit)+lenlab+ EOLch;
        PolarProps += strong;
    }

    //Control data
    //Mass and inertia controls
    QString strLen, strMass, strInertia;

    strInertia = strMass+"."+strLen+SQUAREch;

    PolarProps += "Inertia:\n";

    if(bAutoInertia())
    {
        PolarProps += frontspacer + "Using plane inertia\n";
    }

    strong  = "Mass = " + QString::asprintf(" %.3f ", mass()*massunit);
    PolarProps += frontspacer + strong + masslab + EOLch;

    strong = frontspacer+"CoG = (";
    strong += QString::asprintf("%.3f", CoG().x*lenunit);
    strong += QString::asprintf(", %.3f", CoG().y*lenunit);
    strong += QString::asprintf(", %.3f)", CoG().z*lenunit);
    PolarProps += strong + lenlab + EOLch;

    if(isStabilityPolar())
    {
        strong  = frontspacer + "Ixx = "+QString::asprintf("%7.4g ",  Ixx()*lenunit*lenunit*massunit);
        PolarProps += strong + inertiaunit + EOLch;

        strong  = frontspacer + "Iyy = "+QString::asprintf("%7.4g ", Iyy()*lenunit*lenunit*massunit);
        PolarProps += strong + inertiaunit + EOLch;

        strong  = frontspacer + "Izz = "+QString::asprintf("%7.4g ", Izz()*lenunit*lenunit*massunit);
        PolarProps += strong + inertiaunit + EOLch;

        strong  = frontspacer + "Ixz = "+QString::asprintf("%7.4g ", Ixz()*lenunit*lenunit*massunit);
        PolarProps += strong + inertiaunit + EOLch;
    }



    if((isType123458() || isType7()) && pPlaneXfl)
    {
        if(nAVLCtrls())
        {
            PolarProps += "AVL type controls:\n";

            for(int ic=0; ic<nAVLCtrls(); ic++)
            {
                AngleControl const& avlc = m_AVLControls.at(ic);
                PolarProps += "   " + QString::fromStdString(avlc.name())+ EOLch;

                for(int ig=0; ig<avlc.nValues(); ig++)
                {
                    strange = "      " +  QString::fromStdString(pPlaneXfl->controlSurfaceName(ig)) + ":";
                    strange.resize(30, ' ');
                    strange += QString::asprintf(" %7.2g", avlc.value(ig)) + DEGch+ EOLch;

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
                    PolarProps += QString::asprintf("   drag: Cf=%g\n", m_FuseCf);
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
                strong = "   area= " + QString::asprintf(" %7.2f", m_ExtraDrag.at(ix).area()*areaunit) + " ";
                strong += arealab + ",  ";
                PolarProps += strong;
                strong = "coeff.= " + QString::asprintf(" %7.2f", m_ExtraDrag.at(ix).coef());
                PolarProps += strong+ EOLch;
            }
        }
    }

    if(!bVortonWake())
    {
        strong = "Flat panel wake:\n";
        PolarProps += strong;
        strong = QString::asprintf("Nb. of wake panels = %d\n",NXWakePanel4());
        PolarProps += frontspacer + strong;
        strong = QString::asprintf("Length             = %g x MAC\n", totalWakeLengthFactor());
        PolarProps += frontspacer + strong;
        strong = QString::asprintf("Progression factor = %7.2f", wakePanelFactor()) + EOLch;
        PolarProps += frontspacer + strong;
    }
    else
    {
        strong = "Vorton wake:\n";
        PolarProps += strong;
        strong = QString::asprintf("Buffer wake length = %g x MAC\n", m_BufferWakeFactor);
        PolarProps += frontspacer + strong;
        strong = QString::asprintf("Streamwise step    = %g x MAC\n", m_VortonL0);
        PolarProps += frontspacer + strong;
        strong = QString::asprintf("Discard distance   = %g x MAC\n", m_VPWMaxLength);
        PolarProps += frontspacer + strong;
        strong = QString::asprintf("Vorton core size   = %g x MAC = %g", m_VortonCoreSize, m_VortonCoreSize*m_RefChord*Units::mtoUnit());
        strong += lenlab + EOLch;
        PolarProps += frontspacer + strong;
        strong = QString::asprintf("VPW iterations     = %d", m_VPWIterations) + EOLch;
        PolarProps += frontspacer + strong;
    }

    if(dataSize()>1)
    {
        PolarProps += "\n";
        strong = QString::asprintf("XNP = d(XCp.Cl)/dCl =  %7.2f", m_XNeutralPoint * lenunit);
        PolarProps += strong + " " + lenlab + EOLch;

        strong = QString::asprintf("Static margin       = %g", (m_XNeutralPoint-CoG().x)/m_RefChord*100.0);
        PolarProps += strong + EOLch;
    }

    strong = QString::asprintf("Nbr. of data points = %d",dataSize()) + EOLch;
    PolarProps += strong;

    props = PolarProps.toStdString();
}


std::string PlanePolar::exportToString(const std::string &separator) const
{
    QString polardata;
    QString sep = QString::fromStdString(separator);
    QString strong, strange, str;

    strong = QString::fromStdString(planeName()) + EOLch;
    polardata += strong;

    strong = QString::fromStdString(m_Name) + EOLch;
    polardata += strong;

    str = Units::speedUnitQLabel() + EOLch + EOLch;

    if(isFixedSpeedPolar())
    {
        strong = QString::asprintf("Freestream speed = %.3f ", velocity()*Units::mstoUnit());
        strong += str+ EOLch;
    }
    else if(isFixedaoaPolar())
    {
        strong = QString::asprintf("Alpha = %.3f",alphaSpec());
        strong += DEGch+ EOLch;
    }
    else strong = EOLch + EOLch;
    polardata += strong;


    for(int in=0; in<PlanePolar::variableCount(); in++)
    {
        strange =  QString::fromStdString(PlanePolar::variableName(in));
        if(in==0) strange = " "+strange;// start with a blank space for consistency with polar data
        for(int il=int(strange.length()); il<11; il++) strange+=" ";
        polardata += strange+sep;
    }
    polardata += "\n";


    for(int i=0; i<dataSize(); i++)
    {
        for(int iVar=0; iVar<PlanePolar::variableCount(); iVar++)
        {
            strange = QString::asprintf("%11.5g", variable(iVar, i));
            polardata += strange+sep;
        }
        polardata += "\n";
    }

    return polardata.toStdString();
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
            qDebug("KarmanSchoenherrCoef isnan %2d %.0f", iter, Re);
            return 0.0;
        }
        err = Cf-Cf0;
        Cf0 = Cf;
        iter++;
    }
    while(fabs(err)>1.0e-5 && iter<100);

    //    double err2 = 0.242/sqrt(Cf) - log10(Re*Cf);
    //    qDebug("iter =%3d   Cf=%11.5g   err=%9.5g   errEq=%9.5g", iter, Cf, err, err2);

    if(iter<100) return Cf;
    else         return 0.0;
}


double PlanePolar::PrandtlSchlichtingCoef(double Re) const
{
    return 0.455*pow(log10(Re), -2.58);
}



