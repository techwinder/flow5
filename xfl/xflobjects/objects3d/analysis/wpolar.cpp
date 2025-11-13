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

#include "wpolar.h"
#include <xflobjects/objects_globals/objects_global.h>
#include <xflobjects/objects3d/wing/surface.h>
#include <xflobjects/objects3d/plane/plane.h>
#include <xflobjects/objects3d/plane/planestl.h>
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflobjects/objects3d/analysis/wingopp.h>
#include <xflobjects/objects3d/analysis/planeopp.h>
#include <xflgeom/geom_globals/geom_global.h>
#include <xflmath/constants.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>

QStringList WPolar::s_VariableNames;


WPolar::WPolar() : Polar3d()
{
    m_pPlane = nullptr;

    m_AlphaSpec = 0.0;
    m_QInfSpec  = 10.0;

    m_bViscLoop = false;

    m_RefArea  = 0.0;
    m_RefChord = 0.0;
    m_RefSpan  = 0.0;
    m_bOtherWingsArea = false;

    m_XNeutralPoint = 0.0;

    m_bAVLDrag = false;
    m_AVLSpline.clearControlPoints();
    m_AVLSpline.appendControlPoint(0.035, -0.5);
    m_AVLSpline.appendControlPoint(0.015, -0.3);
    m_AVLSpline.appendControlPoint(0.005, 0.1);
    m_AVLSpline.appendControlPoint(0.015, 0.7);
    m_AVLSpline.appendControlPoint(0.035, 1.3);

    setDefaultSpec(nullptr);
}


void WPolar::setDefaultSpec(Plane const*pPlane)
{
    m_pPlane = pPlane;
    m_Type = xfl::T1POLAR;
    m_QInfSpec = 10;
    m_bThinSurfaces = true;

    m_bTrefftz  = true ;
    m_AnalysisMethod = Polar3d::TRIUNIFORM;
    m_BC = Polar3d::DIRICHLET;
    m_bViscous = true;
    m_NCrit = 9;
    m_XTrTop = m_XTrBot = 1.0;
    m_bAutoInertia = true;

    m_ReferenceDim   = Polar3d::PROJECTED;

    m_Density = 1.225;
    m_Viscosity = 1.5e-5;
    m_bGround = false;
    m_GroundHeight = 0.0;

    m_bWingTipMi = true;

    m_bFuseMi = m_bFuseDrag = false;
    m_FuseDragMethod = WPolar::KARMANSCHOENHERR;
    m_FuseCf = 0;

    m_nXWakePanel4 = 5;
    m_TotalWakeLengthFactor = 30.0;
    m_WakePanelFactor = 1.1;

    m_bVortonWake        = false;
    m_BufferWakeFactor   = 0.3;  // x MAC
    m_VortonL0           = 1.0;  // x MAC
    m_VPWMaxLength       = 30.0;

    m_bAdjustedVelocity = false;

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


void WPolar::replacePOppDataAt(int pos, PlaneOpp const *pPOpp)
{
    if(pos<0 || pos>= dataSize()) return;
    removeAt(pos);
    insertPOppDataAt(pos, pPOpp);
}


void WPolar::insertPOppDataAt(int pos, PlaneOpp const *pPOpp)
{
    if(pos<0 || pos> dataSize()) return; // if(pos==size), then the data is appended

    m_Alpha.insert(pos, pPOpp->alpha());
    m_Beta.insert(pos, pPOpp->beta());
    m_Phi.insert(pos, pPOpp->phi());
    m_QInfinite.insert(pos, pPOpp->QInf());

    m_AF.insert(pos, pPOpp->m_AF);

    if(pPOpp->nWOpps()) m_MaxBending.insert(pos, pPOpp->WOpp(0).m_MaxBending);
    else                m_MaxBending.insert(pos, 0.0);
    m_Ctrl.insert(pos, pPOpp->ctrl());
    m_XNP.insert(pos,  pPOpp->m_SD.XNP);

    m_EV.insert(pos, EigenValues());
    m_EV[pos].m_EV[0] = pPOpp->m_EigenValue[0];
    m_EV[pos].m_EV[1] = pPOpp->m_EigenValue[1];
    m_EV[pos].m_EV[2] = pPOpp->m_EigenValue[2];
    m_EV[pos].m_EV[3] = pPOpp->m_EigenValue[3];
    m_EV[pos].m_EV[4] = pPOpp->m_EigenValue[4];
    m_EV[pos].m_EV[5] = pPOpp->m_EigenValue[5];
    m_EV[pos].m_EV[6] = pPOpp->m_EigenValue[6];
    m_EV[pos].m_EV[7] = pPOpp->m_EigenValue[7];

    //make room for computed values

    m_Mass_var.insert(pos, pPOpp->m_Mass);
    m_CoG_x.insert(pos, 0.0);
    m_CoG_z.insert(pos, 0.0);

    calculatePoint(pos);
}


std::complex<double> WPolar::eigenValue(int iMode, int index) const
{
    if(index<0 || index>m_EV.size()) return std::complex<double>(0.0,0.0);
    if(iMode<0 || iMode>7)           return std::complex<double>(0.0,0.0);
    return m_EV.at(index).m_EV[iMode];
}


/** @todo what about AF and EV? */
void WPolar::insertDataAt(int pos, double Alpha, double Beta, double Phi, double QInf, double Ctrl, double Cb, double XNP)
{
    if(pos<0 || pos>dataSize()) return;

    m_Alpha.insert(pos, Alpha);
    m_Beta.insert(pos, Beta);
    m_Phi.insert(pos, Phi);
    m_QInfinite.insert(pos, QInf);

    m_MaxBending.insert(pos, Cb);
    m_Ctrl.insert(pos, Ctrl);
    if(isStabilityPolar()) m_XNP.insert(pos, XNP);
    else                   m_XNP.insert(pos, 0.0);

    m_Mass_var.insert(pos, 0.0);
    m_CoG_x.insert(pos, 0.0);
    m_CoG_z.insert(pos, 0.0);
}


void WPolar::resizeData(int newsize)
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


void WPolar::addPlaneOpPointData(PlaneOpp const *pPOpp)
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


void WPolar::calculatePoint(int iPt)
{
    if(iPt<0 || iPt>=m_AF.count()) return;

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

    if(m_AF.count()>1 && !isStabilityPolar())
    {
        double XCpClf = m_AF.front().centreOfPressure().x * m_AF.front().CL();
        double XCpClb = m_AF.back().centreOfPressure().x * m_AF.back().CL();
        m_XNeutralPoint = (XCpClb-XCpClf) / (m_AF.back().CL()-m_AF.front().CL());
    }
    else m_XNeutralPoint = 0.0;

    if(isStabilityPolar()) m_EV[iPt].computeModes();
}


void WPolar::duplicateSpec(Polar3d const *pPolar3d)
{
    Polar3d::duplicateSpec(pPolar3d);

    if(!pPolar3d->isPlanePolar()) return;
    WPolar const *pWPolar = dynamic_cast<WPolar const *>(pPolar3d);

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


void WPolar::setVariableNames(QString const &strLength, QString const &strSpeed, QString strMass, QString const &strForce, QString const &strMoment)
{
    s_VariableNames.clear();
    s_VariableNames = QStringList({"Ctrl", ALPHACHAR + " ("+DEGCHAR + ")", BETACHAR + " ("+DEGCHAR + ")", PHICHAR + " ("+DEGCHAR + ")",
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


void WPolar::listVariable(int iVar)
{
    if(iVar<0 || iVar>=variableCount()) return;

    for(int index=0; index<dataSize(); index++)
    {
        qDebug(" %17g", getVariable(iVar, index));
    }
}


double WPolar::variable(int iVariable, int index) const
{
    if(iVariable<0 || iVariable>variableCount()) return 0.0;
    if(index<0 || index>dataSize())  return 0.0;
    return getVariable(iVariable, index);
}


double WPolar::getVariable(int iVar, int index) const
{
    Vector3d WindD = windDirection(m_Alpha.at(index), m_Beta.at(index));
    Vector3d WindN = windNormal(m_Alpha.at(index), m_Beta.at(index));

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
        case 10:  return AF.Cmv();
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


double WPolar::Vx(int index) const
{
    double gamma = atan(m_AF.at(index).CD()/m_AF.at(index).CL());
    return m_QInfinite.at(index) * cos(gamma);
}


double WPolar::Vz(int index) const
{
    double gamma = atan(m_AF.at(index).CD()/m_AF.at(index).CL());
    double Vh = sqrt(2*m_Mass_var.at(index)*9.81/m_Density/m_RefArea)/Cl32Cd(index);
    double Vg =  m_QInfinite.at(index) * sin(gamma);

    if(!isGlidePolar()) return Vh;
    return Vg;
}


double WPolar::Cl32Cd(int index) const
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


void WPolar::removeAoA(double alpha)
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


void WPolar::removeAt(int index)
{
    if(index<0 || index>=dataSize()) return;

    m_Alpha.removeAt(index);
    m_Beta.removeAt(index);
    m_Phi.removeAt(index);

    m_AF.removeAt(index);

    m_MaxBending.removeAt(index);

    m_Mass_var.removeAt(index);
    m_CoG_x.removeAt(index);
    m_CoG_z.removeAt(index);
    m_Ctrl.removeAt(index);
    m_XNP.removeAt(index);
    m_EV.removeAt(index);

    m_QInfinite.removeAt(index);

}


void WPolar::insertDataPointAt(int index, bool bAfter)
{
    if(bAfter) index++;
    m_Alpha.insert(index,0);
    m_Beta.insert(index,0);
    m_Phi.insert(index,0);

    m_AF.insert(index, AeroForces());

    m_MaxBending.insert(index,0);

    m_Mass_var.insert(index,0);
    m_CoG_x.insert(index,0);
    m_CoG_z.insert(index,0);
    m_Ctrl.insert(index,0);
    m_XNP.insert(index,0);
    m_EV.insert(index,EigenValues());
    m_QInfinite.insert(index,0);

}


/**
 *Clears the content of the data arrays
*/
void WPolar::clearWPolarData()
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
void WPolar::retrieveInertia(const Plane *pPlane)
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


void WPolar::copy(const WPolar *pWPolar)
{
    duplicateSpec(pWPolar);
    m_PlaneName = pWPolar->planeName();
    m_Name = pWPolar->name();

    clearWPolarData();

    m_AF = pWPolar->m_AF;
    m_EV = pWPolar->m_EV;

    for(int i=0; i<pWPolar->dataSize(); i++)
    {
        m_Alpha.append(     pWPolar->m_Alpha.at(i));
        m_Beta.append(      pWPolar->m_Beta.at(i));
        m_Phi.append(       pWPolar->m_Phi.at(i));

        m_QInfinite.append( pWPolar->m_QInfinite.at(i));

        m_MaxBending.append(pWPolar-> m_MaxBending.at(i));
        m_Ctrl.append(      pWPolar-> m_Ctrl.at(i));
        m_XNP.append(       pWPolar-> m_XNP.at(i));

        m_Mass_var.append(  pWPolar-> m_Mass_var.at(i));
        m_CoG_x.append(     pWPolar-> m_CoG_x.at(i));
        m_CoG_z.append(     pWPolar-> m_CoG_z.at(i));

    }
}


QString WPolar::flapCtrlsSetName() const
{
    QString aname("Flap set name");
    if(m_FlapControls.size())
    {
        if(m_FlapControls.front().name().isEmpty())     return aname;

        return m_FlapControls.front().name(); // repurposing
    }
    return aname;
}


bool WPolar::checkFlaps(PlaneXfl const*pPlaneXfl, QString &log) const
{
    if(!pPlaneXfl) return false;

    log.clear();

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
    return bMatch;
}


bool WPolar::hasActiveFlap() const
{
    for(int ie=0; ie<m_FlapControls.size(); ie++)
    {
        if(m_FlapControls.at(ie).hasActiveAngle()) return true;
    }
    return false;
}


bool WPolar::hasActiveAVLControl() const
{
    for(int ie=0; ie<m_AVLControls.size(); ie++)
    {
        if(m_AVLControls.at(ie).hasActiveAngle()) return true;
    }
    return false;
}


void WPolar::clearAngleRangeList()
{
    for(int iw=0; iw<m_AngleRange.size(); iw++)
    {
        for(int c=0; c<m_AngleRange.at(iw).size();  c++)
        {
            m_AngleRange[iw].clear();
        }
        m_AngleRange.clear();
    }
}


int WPolar::nAngleRangeCtrls() const
{
    int total=0;
    for(int iw=0; iw<m_AngleRange.size(); iw++)
    {
        total += m_AngleRange.at(iw).size();
    }
    return total;
}


CtrlRange WPolar::angleRange(int iWing, int iCtrl) const
{
    if(iWing<m_AngleRange.size())
    {
        if(iCtrl<m_AngleRange.at(iWing).size())
            return m_AngleRange.at(iWing).at(iCtrl);
    }
    return CtrlRange();
}


/**
 * Checks if the number of controls has changed, and if so reset all values to defaults
 */
void WPolar::resetAngleRanges(const Plane *pPlane)
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
    QString strong;
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
            strong = pWing->name() + " " + QString("Flap %1 ").arg(iFlapCtrl);
            m_AngleRange.back().push_back({strong, 0.0, 0.0});
            iFlapCtrl++;
            iCtrl++;
        }
    }
    (void)iCtrl;
}


void WPolar::resizeFlapCtrls(const PlaneXfl *pPlaneXfl)
{
    if(!pPlaneXfl) return;

    m_FlapControls.resize(pPlaneXfl->nWings());

    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = pPlaneXfl->wingAt(iw);
        resizeFlapCtrls(iw, pWing->nFlaps());
    }

    if(isType12358() || isType7())
    {
        // T7 polar
        for(int ie=0; ie<m_AVLControls.size(); ie++)
            m_AVLControls[ie].resizeValues(pPlaneXfl->nAVLGains());
    }
}


void WPolar::resetFlapCtrls()
{
    for(int i=0; i<m_FlapControls.size(); i++)
    {
        AngleControl &ctrl = m_FlapControls[i];
        for(int j=0; j<ctrl.nValues(); j++) ctrl.setValue(j,0.0);
    }
}


double WPolar::flapAngleValue(int iWing, int iFlap) const
{
    if(iWing>=0 && iWing<m_FlapControls.size())
        return m_FlapControls.at(iWing).value(iFlap);
    else return 0.0;
}


void WPolar::setFlapAngleValue(int iWing, int iFlap, double g)
{
    if(iWing>=0 && iWing<m_FlapControls.size())
        m_FlapControls[iWing].setValue(iFlap, g);
}


double WPolar::extraDragForce(int index) const
{
    double QInf = m_QInfinite.at(index);
    return extraDragTotal(m_AF[index].CL()) * 0.5 * m_Density * QInf * QInf; // N
}


bool WPolar::serializeWPlrXFL(QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    int i(0), k(0), n(0);
    double dble(0);
    double r0(0), r1(0), r2(0), r3(0), r4(0), r5(0), r6(0), r7(0);
    double i0(0), i1(0), i2(0), i3(0), i4(0), i5(0), i6(0), i7(0);

    m_PolarFormat = 200013;
    // 200013: v0.00
    // 200014:  added array of control values for stability polars
    // 200015:  added array of control values for controls polars

    if(bIsStoring)
    {
        // STORING REQUIRED TO EXPORT .xfl temp file for foil analysis
        //output the variables to the stream
        ar << m_PolarFormat;

        ar << m_PlaneName;
        ar << m_Name;

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

        ar >> m_PlaneName;
        ar >> m_Name;

        ar >> m_RefArea >> m_RefChord >> m_RefSpan;
        if(m_PolarFormat<200014)
        {
            ar >> k;
            m_theStyle.setStipple(k);
            ar >> m_theStyle.m_Width;

            ar >> m_theStyle.m_Color;

            ar >> m_theStyle.m_bIsVisible >> boolean;
        }
        else
            m_theStyle.serializeXfl(ar, bIsStoring);

        ar >> n;
        if     (n==1) m_AnalysisMethod=Polar3d::LLT;
        else if(n==2) m_AnalysisMethod=Polar3d::VLM1;
        else if(n==3) m_AnalysisMethod=Polar3d::QUADS;
        else if(n==4) m_AnalysisMethod=Polar3d::TRILINEAR;
        else if(n==5) m_AnalysisMethod=Polar3d::TRIUNIFORM;

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
            if(!boolean) m_AnalysisMethod=Polar3d::VLM2;
        }

        ar >> m_bThinSurfaces;
        if(m_bThinSurfaces&&!isLLTMethod())
        {
            if(boolean) m_AnalysisMethod=Polar3d::VLM1;
            else        m_AnalysisMethod=Polar3d::VLM2;
        }

        if(isTriangleMethod()) m_bThinSurfaces = false;  // cleaning up incorrectly constructed polars

        ar >> boolean; // m_bTiltedGeom;
        ar >> boolean;
        m_BC = boolean? Polar3d::DIRICHLET : Polar3d::NEUMANN;

        ar >> m_bViscous;        m_bViscFromCl = true;

        ar >> m_bIgnoreBodyPanels;
        if(isVLM()) m_bIgnoreBodyPanels = true; //clean up override

        ar >> m_bGround;
        ar >> m_GroundHeight;

        ar >> m_Density >> m_Viscosity;

        ar >> k;
        if     (k==1) m_ReferenceDim = Polar3d::PLANFORM;
        else if(k==2) m_ReferenceDim = Polar3d::PROJECTED;
        else if(k==3) m_ReferenceDim = Polar3d::CUSTOM;
        else          m_ReferenceDim = Polar3d::PLANFORM;

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
            QString strong;
            strong = QString::asprintf("Extra drag %d", ix);
            m_ExtraDrag[ix].setName(strong);
        }
        for (int ix=0; ix<4; ix++) {ar>>dble; m_ExtraDrag[ix].setArea(dble);}
        for (int ix=0; ix<4; ix++) {ar>>dble; m_ExtraDrag[ix].setCoef(dble);}
        //clear the null extradrag
        for (int ix=3; ix>=0; ix--)
        {
            ExtraDrag &xd = m_ExtraDrag[ix];
            if(fabs(xd.area())<PRECISION && fabs(xd.coef())<PRECISION) m_ExtraDrag.remove(ix);
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


bool WPolar::serializeFl5v726(QDataStream &ar, bool bIsStoring)
{
    if(!Polar3d::serializeFl5v726(ar, bIsStoring)) return false;

    int k(0), n(0);
    int nIntSpares(0);
    int nDbleSpares(0);

    double dble(0), dmin(0), dmax(0);

    QString strange;

    if(bIsStoring)
    {
        Q_ASSERT(false);
    }
    else
    {
        //input the variables from the stream

        // METADATA
        if ((m_PolarFormat<500001) || (m_PolarFormat>500100))
            return false;

        ar >> m_PlaneName;

        ar >> m_QInfSpec;
        ar >> m_AlphaSpec;

        ar >> m_bThinSurfaces;

        // REFERENCE DIMENSIONS
        ar >> k;
        if     (k==1) m_ReferenceDim = Polar3d::PLANFORM;
        else if(k==2) m_ReferenceDim = Polar3d::PROJECTED;
        else if(k==3) m_ReferenceDim = Polar3d::CUSTOM;
        else          m_ReferenceDim = Polar3d::PLANFORM;
        ar >> m_RefArea >> m_RefChord >> m_RefSpan;

        if(m_PolarFormat<500020) m_VortonCoreSize /= m_RefChord;

        // AVL type control ranges
        if(m_PolarFormat>=500021)
        {
            ar >> n;
            m_AVLControls.resize(n);
            for(int ic=0; ic<m_AVLControls.size(); ic++) m_AVLControls[ic].serializeFl5(ar, bIsStoring);

            if(m_Type!=xfl::T6POLAR) m_AVLControls.clear(); // cleaning up
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
        for(int iw=0; iw<m_AngleRange.size(); iw++)
        {
            ar>>n;
            for(int c=0; c<n; c++)
            {
                ar >> strange;
                ar >> dmin>> dmax;
                m_AngleRange[iw].push_back({strange, dmin, dmax});
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
            m_OperatingRange.push_back({strange, dmin, dmax});
        }
        if(m_OperatingRange.size()<4)
        {
            m_OperatingRange.resize(4);
            m_OperatingRange[3].set(PHICHAR, 0.0, 0.0);
        }

        // update old formats
        m_OperatingRange[0].setName("V"+INFCHAR);
        m_OperatingRange[1].setName(ALPHACHAR);
        m_OperatingRange[2].setName(BETACHAR);
        m_OperatingRange[3].setName(PHICHAR);

        m_InertiaRange.clear();
        ar>>n;
        for(int c=0; c<n; c++)
        {
            ar >> strange;
            ar >> dmin>> dmax;
            m_InertiaRange.push_back({strange, dmin, dmax});
        }

        // FUSE DATA
        if(m_PolarFormat>=500004)
        {
            ar >> k;    m_bFuseMi = (k ? true : false);
        }
        ar >> k;        m_bFuseDrag = (k ? true : false);
        ar >> k;
        if     (k==1) m_FuseDragMethod=WPolar::KARMANSCHOENHERR;
        else if(k==2) m_FuseDragMethod=WPolar::PRANDTLSCHLICHTING;
        else          m_FuseDragMethod=WPolar::MANUALFUSECF;

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

            m_AF.last().setOpp(m_Alpha.at(i), m_Beta.at(i), m_Phi.at(i), m_QInfinite.at(i));
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


bool WPolar::serializeFl5v750(QDataStream &ar, bool bIsStoring)
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
        ar << m_PlaneName;

        ar << m_QInfSpec;
        ar << m_AlphaSpec;

        ar << m_bThinSurfaces;

        ar << m_bViscLoop;

        // REFERENCE DIMENSIONS
        if     (m_ReferenceDim == Polar3d::PLANFORM)  ar << 1;
        else if(m_ReferenceDim == Polar3d::PROJECTED) ar << 2;
        else if(m_ReferenceDim == Polar3d::CUSTOM)    ar << 3;
        ar << m_RefArea << m_RefChord << m_RefSpan;

        // TE flap angles
        ar << int(m_FlapControls.size());
        for(int ic=0; ic<m_FlapControls.size(); ic++) m_FlapControls[ic].serializeFl5(ar, bIsStoring);

        // AVL type control ranges
        ar << int(m_AVLControls.size());
        for(int ic=0; ic<m_AVLControls.size(); ic++) m_AVLControls[ic].serializeFl5(ar, bIsStoring);

        ar << m_bAdjustedVelocity;

        ar << int(m_AngleRange.size());
        for(int icg=0; icg<m_AngleRange.size(); icg++)
        {
            ar << int(m_AngleRange.at(icg).size());
            for(int jcg=0; jcg<m_AngleRange.at(icg).size(); jcg++)
            {
                ar << m_AngleRange.at(icg).at(jcg).name();
                ar << m_AngleRange.at(icg).at(jcg).ctrlMin() << m_AngleRange.at(icg).at(jcg).ctrlMax();
            }
        }

        //Operating point range
        ar << int(m_OperatingRange.size());
        for(int jcg=0; jcg<m_OperatingRange.size(); jcg++)
        {
            ar << m_OperatingRange.at(jcg).name();
            ar << m_OperatingRange.at(jcg).ctrlMin() << m_OperatingRange.at(jcg).ctrlMax();
        }

        //Inertia range
        ar << int(m_InertiaRange.size());
        for(int jcg=0; jcg<m_InertiaRange.size(); jcg++)
        {
            ar << m_InertiaRange.at(jcg).name();
            ar << m_InertiaRange.at(jcg).ctrlMin() << m_InertiaRange.at(jcg).ctrlMax();
        }

        // FUSE DATA
        ar << m_bFuseMi;
        ar << m_bFuseDrag;

        switch(m_FuseDragMethod)
        {
            case WPolar::MANUALFUSECF:         ar<<0;  break;
            case WPolar::KARMANSCHOENHERR:     ar<<1;  break;
            case WPolar::PRANDTLSCHLICHTING:   ar<<2;  break;
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

        ar >> m_PlaneName;

        ar >> m_QInfSpec;
        ar >> m_AlphaSpec;

        ar >> m_bThinSurfaces;
        ar >> m_bViscLoop;


        // REFERENCE DIMENSIONS
        ar >> k;
        if     (k==1) m_ReferenceDim = Polar3d::PLANFORM;
        else if(k==2) m_ReferenceDim = Polar3d::PROJECTED;
        else if(k==3) m_ReferenceDim = Polar3d::CUSTOM;
        else          m_ReferenceDim = Polar3d::PLANFORM;
        ar >> m_RefArea >> m_RefChord >> m_RefSpan;

        // TE flap angles
        ar >> n;
        m_FlapControls.resize(n);
        for(int ic=0; ic<m_FlapControls.size(); ic++) m_FlapControls[ic].serializeFl5(ar, bIsStoring);

        // AVL type control ranges
        ar >> n;
        m_AVLControls.resize(n);
        for(int ic=0; ic<m_AVLControls.size(); ic++) m_AVLControls[ic].serializeFl5(ar, bIsStoring);


        ar >> m_bAdjustedVelocity;

        //CONTROL POLAR RANGES
        clearAngleRangeList();
        ar>>n;
        m_AngleRange.resize(n);
        for(int iw=0; iw<m_AngleRange.size(); iw++)
        {
            ar>>n;
            for(int c=0; c<n; c++)
            {
                ar >> strange;
                ar >> dmin>> dmax;
                m_AngleRange[iw].push_back({strange, dmin, dmax});
            }
        }

        m_OperatingRange.clear();
        ar>>n;
        for(int c=0; c<n; c++)
        {
            ar >> strange;
            ar >> dmin>> dmax;

            m_OperatingRange.push_back({strange, dmin, dmax});
        }


        m_InertiaRange.clear();
        ar>>n;
        for(int c=0; c<n; c++)
        {
            ar >> strange;
            ar >> dmin>> dmax;
            m_InertiaRange.push_back({strange, dmin, dmax});
        }

        // FUSE DATA

        ar >> m_bFuseMi;
        ar >> m_bFuseDrag;

        ar >> k;
        if     (k==1) m_FuseDragMethod=WPolar::KARMANSCHOENHERR;
        else if(k==2) m_FuseDragMethod=WPolar::PRANDTLSCHLICHTING;
        else          m_FuseDragMethod=WPolar::MANUALFUSECF;

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

            m_AF.last().setOpp(m_Alpha.at(i), m_Beta.at(i), m_Phi.at(i), m_QInfinite.at(i));
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



void WPolar::getProperties(QString &PolarProps, Plane const *pPlane,
                           double lenunit, double massunit, double speedunit, double areaunit,
                           const QString &lenlab, const QString &masslab, const QString &speedlab, const QString &arealab) const
{
    QString strong, strange;
    QString frontspacer("   ");
    QString inertiaunit = masslab+"."+lenlab+QString::fromUtf8("²");

    QString fmt  = xfl::isLocalized() ? "%L1" : "%1";
    QString fmt2 = xfl::isLocalized() ? "[%L1 / %L2] " : "[%1 / %2] ";

    PlaneXfl const*pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

    PolarProps.clear();

    if     (isFixedSpeedPolar()) strong = "Type 1: Fixed speed";
    else if(isFixedLiftPolar())  strong = "Type 2: Fixed lift";
    else if(isGlidePolar())      strong = "Type 3: Speed polar";
    else if(isFixedaoaPolar())   strong = "Type 4: Fixed angle of attack" ;
    else if(isBetaPolar())       strong = "Type 5: Sideslip analysis";
    else if(isControlPolar())    strong = "Type 6: Control analysis";
    else if(isStabilityPolar())  strong = "Type 7: Stability analysis";
    else if(isType8())         strong = "Type 8";
    else if(isExternalPolar())
    {
        PolarProps = "External polar\n";
        strong = QString::asprintf("Nbr. of data points = %d",dataSize());
        PolarProps += strong;
        return;
    }
    PolarProps += strong + EOLCHAR;

    if(isFixedSpeedPolar())
    {
        strong  = "V" + INFCHAR + QString(" ="+fmt+" ").arg(velocity()*speedunit,7,'g',2);
        PolarProps += strong + speedlab+ EOLCHAR;
    }
    else if(isFixedaoaPolar())
    {
        strong  = ALPHACHAR + QString(" ="+fmt).arg(alphaSpec(),7,'f',2);
        PolarProps += strong +DEGCHAR+ EOLCHAR;
    }
    else if(isBetaPolar())
    {
        strong  = ALPHACHAR + QString(" ="+fmt).arg(alphaSpec(),7,'f',2);
        PolarProps += strong +DEGCHAR+ EOLCHAR;
        strong  = "V" + INFCHAR + QString("   ="+fmt+" ").arg(velocity()*speedunit,7,'g',2);
        PolarProps += strong + speedlab+ EOLCHAR;
    }

    if(!isControlPolar() && !isBetaPolar() && fabs(betaSpec())>ANGLEPRECISION)
    {
        if(fabs(betaSpec())>AOAPRECISION)
            PolarProps += BETACHAR + QString("  = "+fmt).arg(betaSpec(),7,'g') + DEGCHAR+ EOLCHAR;

    }

    if(isType12358() && fabs(m_BankAngle)>ANGLEPRECISION)
    {
        if(fabs(m_BankAngle)>AOAPRECISION)
            PolarProps += PHICHAR + QString("  = "+fmt).arg(m_BankAngle,7,'g') + DEGCHAR + EOLCHAR;
    }


    if((isType12358() || isType7()) && hasActiveFlap() && pPlaneXfl)
    {
        PolarProps += "Flap settings: " + flapCtrlsSetName() + EOLCHAR;
        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            WingXfl const*pWing = pPlaneXfl->wingAt(iw);
            PolarProps += "   " + pWing->name() +":\n";

            if(iw<m_FlapControls.size())
            {
                AngleControl const &avlc = m_FlapControls.at(iw);
                for(int iflap=0; iflap<avlc.nValues(); iflap++)
                {
                    strange = QString::asprintf("      flap %d: %+7.2f", iflap+1, avlc.value(iflap)) + DEGCHAR + EOLCHAR;
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
            PolarProps += "Velocity:        [  Adjusted   ]\n";
        }
        else
        {
            strong = QString(fmt2)
                     .arg(m_OperatingRange.at(0).ctrlMin()*speedunit, 5, 'f', 2)
                     .arg(m_OperatingRange.at(0).ctrlMax()*speedunit, 5, 'f', 2);
            strange =  m_OperatingRange.at(0).name() + ": ";
            strange.resize(17, ' ');
            PolarProps += strange + strong + speedlab + EOLCHAR;
        }

        for(int i=1; i<m_OperatingRange.size(); i++)
        {
            strong = QString(fmt2)
                     .arg(m_OperatingRange.at(i).ctrlMin(), 5, 'f', 2)
                     .arg(m_OperatingRange.at(i).ctrlMax(), 5, 'f', 2);
            strange =  m_OperatingRange.at(i).name() + ": ";
            strange.resize(17, ' ');
            PolarProps += strange + strong + DEGCHAR + EOLCHAR;
        }

        //inertia
        strong = QString(fmt2)
                 .arg(m_InertiaRange.at(0).ctrlMin()*massunit, 5, 'f', 2)
                 .arg(m_InertiaRange.at(0).ctrlMax()*massunit, 5, 'f', 2);
        strange =  m_InertiaRange.at(0).name() + ": ";
//        for(int j=strange.length(); j<17; j++) strange = strange+" ";
        strange.resize(17, ' ');
        PolarProps += strange + strong + masslab + EOLCHAR;

        strong = QString(fmt2)
                 .arg(m_InertiaRange.at(1).ctrlMin()*lenunit, 5, 'f', 2)
                 .arg(m_InertiaRange.at(1).ctrlMax()*lenunit, 5, 'f', 2);
        strange =  m_InertiaRange.at(1).name() + ": ";
        strange.resize(17, ' ');
        PolarProps += strange + strong + lenlab + EOLCHAR;

        strong = QString(fmt2)
                 .arg(m_InertiaRange.at(2).ctrlMin()*lenunit, 5, 'f', 2)
                 .arg(m_InertiaRange.at(2).ctrlMax()*lenunit, 5, 'f', 2);
        strange =  m_InertiaRange.at(2).name() + ": ";
        strange.resize(17, ' ');
        PolarProps += strange + strong + lenlab + EOLCHAR;

        //Angles
        for(int j=0; j<m_AngleRange.size(); j++)
        {
            for(int i=0; i<m_AngleRange.at(j).size(); i++)
            {
                strong = QString(fmt2)
                         .arg(m_AngleRange.at(j).at(i).ctrlMin(), 5, 'f', 2)
                         .arg(m_AngleRange.at(j).at(i).ctrlMax(), 5, 'f', 2);
                strange =  m_AngleRange.at(j).at(i).name().trimmed() + ": ";
                strange.resize(17, ' ');
                PolarProps += strange + strong +DEGCHAR + EOLCHAR;
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
        if(boundaryCondition()==Polar3d::DIRICHLET)  PolarProps += "B.C.: Dirichlet\n";
        else                                         PolarProps += "B.C.: Neumann\n";
    }

    if(bTrefftz()) strong = "Lift & drag: in the far field plane";
    else           strong = "Lift & drag: summation of pressure forces";
    PolarProps += strong + EOLCHAR;

    if(isViscous())
    {
        if(m_bViscOnTheFly)
        {
            PolarProps += "Viscous drag: XFoil on the fly\n";
            PolarProps += QString::asprintf("   NCrit  = %g\n", m_NCrit);
            PolarProps += QString::asprintf("   XTrTop = %g%% chord\n", m_XTrTop*100.0);
            PolarProps += QString::asprintf("   XTrBot = %g%% chord\n", m_XTrBot*100.0);
        }
        else
        {
            PolarProps += "Viscous drag: interpolated";
            if(m_bViscFromCl) PolarProps += " from Cl\n";
            else              PolarProps += " from " + ALPHACHAR+ EOLCHAR;
        }

        if(m_bViscLoop) PolarProps += "Viscous loop: enabled\n";
        else            PolarProps += "Viscous loop: disabled\n";

    }
    else  PolarProps += "Inviscid analysis\n";

    if     (referenceDim()==Polar3d::PLANFORM)  PolarProps += "Ref. dimensions = Planform\n";
    else if(referenceDim()==Polar3d::PROJECTED) PolarProps += "Ref. dimensions = Projected\n";
    else if(referenceDim()==Polar3d::CUSTOM)    PolarProps += "Ref. dimensions = Custom\n";

    PolarProps += frontspacer + "Area  =" + QString(fmt+" ").arg(referenceArea()       *areaunit,9,'f',3) + arealab+ EOLCHAR;
    PolarProps += frontspacer + "Span  =" + QString(fmt+" ").arg(referenceSpanLength() *lenunit, 9,'f',3) + lenlab + EOLCHAR;
    PolarProps += frontspacer + "Chord =" + QString(fmt+" ").arg(referenceChordLength()*lenunit, 9,'f',3) + lenlab + EOLCHAR;

    if(!m_bThinSurfaces)
    {
        if(m_bWingTipMi) PolarProps += "Contribution of wing tips to moments: included\n";
        else             PolarProps += "Contribution of wing tips to moments: ignored\n";
    }


    PolarProps += "Fluid properties:\n";

    strong  = frontspacer + RHOCHAR + QString(" = "+fmt+ " ").arg(density()*Units::densitytoUnit(),9,'g', 5);
    strong += Units::densityUnitLabel() + "\n";
    PolarProps += strong;

    strong  = frontspacer + NUCHAR  + QString(" = "+fmt + " ").arg(viscosity()*Units::viscositytoUnit(),9,'g', 5);
    strong += Units::viscosityUnitLabel() + "\n";
    PolarProps += strong;


    if(bGroundEffect())
    {
        strong = QString("Ground height = "+fmt).arg(m_GroundHeight*lenunit)+lenlab+ EOLCHAR;
        PolarProps += strong;
    }
    else if(bFreeSurfaceEffect())
    {
        strong = QString("Free surface height = "+fmt).arg(m_GroundHeight*lenunit)+" "+lenlab+ EOLCHAR;
        PolarProps += strong;
    }

    //Control data
    //Mass and inertia controls
    QString strLen, strMass, strInertia;

    strInertia = strMass+"."+strLen+QString::fromUtf8("²");

    PolarProps += "Inertia:\n";

    if(bAutoInertia())
    {
        PolarProps += frontspacer + "Using plane inertia\n";
    }

    strong  = QString("Mass = "+fmt +" ").arg(mass()*massunit,7,'f',3);
    PolarProps += frontspacer + strong + masslab + EOLCHAR;

    strong = frontspacer+"CoG = (";
    strong += QString(fmt+"; ").arg(CoG().x*lenunit,7,'f',3);
    strong += QString(fmt+"; ").arg(CoG().y*lenunit,7,'f',3);
    strong += QString(fmt+") ").arg(CoG().z*lenunit,7,'f',3);
    PolarProps += strong + lenlab + EOLCHAR;

    if(isStabilityPolar())
    {
        strong  = frontspacer + QString("Ixx = "+fmt + " ").arg(Ixx()*lenunit*lenunit*massunit,7,'g',4);
        PolarProps += strong + inertiaunit+ EOLCHAR;

        strong  = frontspacer + QString("Iyy = "+fmt+ " ").arg(Iyy()*lenunit*lenunit*massunit,7,'g',4);
        PolarProps += strong + inertiaunit+ EOLCHAR;

        strong  = frontspacer + QString("Izz = "+fmt+ " ").arg(Izz()*lenunit*lenunit*massunit,7,'g',4);
        PolarProps += strong + inertiaunit+ EOLCHAR;

        strong  = frontspacer + QString("Ixz = "+fmt+ " ").arg(Ixz()*lenunit*lenunit*massunit,7,'g',4);
        PolarProps += strong + inertiaunit+ EOLCHAR;
    }


/*    if(isType7())
    {
        // list AVL ctrls;
        PolarProps += "AVl-type controls:\n";
        for(int ictrl=0; ictrl<m_AVLControls.size(); ictrl++)
        {
            AngleControl const &avlc = m_AVLControls.at(ictrl);
            PolarProps += "   " + avlc.name() + "\n";
            int ic = 0;
            for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
            {
                WingXfl const*pWing = pPlaneXfl->wingAt(iw);
                PolarProps += "      " + pWing->name() +":\n";

                for(int iflap=0; iflap<pWing->nFlaps(); iflap++)
                {
                    strange = QString::asprintf("         flap %d: %7.2f", iflap+1, avlc.value(ic)) + DEGCHAR + "\n";
                    PolarProps += strange;
                    ic++;
                }
            }
        }
    }*/

    if((isType12358() || isType7()) && pPlaneXfl)
    {
        if(nAVLControls())
        {
            PolarProps += "AVL type controls:\n";

            for(int ic=0; ic<nAVLControls(); ic++)
            {
                AngleControl const& avlc = m_AVLControls.at(ic);
                PolarProps += "   " + avlc.name()+ EOLCHAR;

                for(int ig=0; ig<avlc.nValues(); ig++)
                {
                    strange = "      " +  pPlaneXfl->controlSurfaceName(ig) + ":";
                    strange.resize(30, ' ');
                    strange += QString(fmt).arg(avlc.value(ig),7,'g',2) + DEGCHAR+ EOLCHAR;

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
                case WPolar::KARMANSCHOENHERR:
                    PolarProps += "   drag: Karman-Schoenherr\n";
                    break;
                case WPolar::PRANDTLSCHLICHTING:
                    PolarProps += "   drag: Prandtl-Schlichting\n";
                    break;
                case WPolar::MANUALFUSECF:
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
                strong = QString("   area= "+fmt).arg(m_ExtraDrag.at(ix).area()*areaunit) + " ";
                strong += arealab + ",  ";
                PolarProps += strong;
                strong = QString("coeff.= "+fmt).arg(m_ExtraDrag.at(ix).coef());
                PolarProps += strong+ EOLCHAR;
            }
        }
    }

    if(!bVortonWake())
    {
        strong = "Flat panel wake:\n";
        PolarProps += strong;
        strong = QString("Nb. of wake panels = %1\n").arg(NXWakePanel4());
        PolarProps += frontspacer + strong;
        strong = QString(  "Length             = "+fmt+" x MAC\n").arg(totalWakeLengthFactor());
        PolarProps += frontspacer + strong;
        strong = QString("Progression factor = "+fmt).arg(wakePanelFactor()) + EOLCHAR;
        PolarProps += frontspacer + strong;
    }
    else
    {
        strong = "Vorton wake:\n";
        PolarProps += strong;
        strong = QString("Buffer wake length = "+fmt+" x MAC\n").arg(m_BufferWakeFactor);
        PolarProps += frontspacer + strong;
        strong = QString("Streamwise step    = "+fmt+" x MAC\n").arg(m_VortonL0);
        PolarProps += frontspacer + strong;
        strong = QString("Discard distance   = "+fmt+" x MAC\n").arg(m_VPWMaxLength);
        PolarProps += frontspacer + strong;
        strong = QString("Vorton core size   = "+fmt+" x MAC = %2").arg(m_VortonCoreSize).arg(m_VortonCoreSize*m_RefChord*Units::mtoUnit());
        strong += Units::lengthUnitLabel() + EOLCHAR;
        PolarProps += frontspacer + strong;
        strong = QString("VPW iterations     = "+fmt).arg(m_VPWIterations) + EOLCHAR;
        PolarProps += frontspacer + strong;
    }

    if(dataSize()>1)
    {
        PolarProps += "\n";
        strong = QString("XNP = d(XCp.Cl)/dCl = "+fmt).arg(m_XNeutralPoint * lenunit, 7,'f',3);
        PolarProps += strong + " " + lenlab + EOLCHAR;

        strong = QString("Static margin       = "+fmt+"%").arg((m_XNeutralPoint-CoG().x)/m_RefChord*100.0, 5, 'f', 2);
        PolarProps += strong + EOLCHAR;
    }

    strong = QString("Nb. of data points = %1\n").arg(dataSize());
    PolarProps += strong;
}



void WPolar::getWPolarData(QString &polardata, QString const &sep, double speedunit, QString speedlab) const
{
    QString strong, strange, str;

    strong = planeName()+ EOLCHAR;
    polardata += strong;

    strong = name()+ "\n";
    polardata += strong;

    str = speedlab + "\n\n";

    if(isFixedSpeedPolar())
    {
        strong = QString::asprintf("Freestream speed = %.3f ", velocity()*speedunit);
        strong += str+ EOLCHAR;
    }
    else if(isFixedaoaPolar())
    {
        strong = QString::asprintf("Alpha = %.3f",alphaSpec());
        strong += DEGCHAR+ EOLCHAR;
    }
    else strong = "\n\n";
    polardata += strong;


    for(int in=0; in<WPolar::variableCount(); in++)
    {
        strange =  WPolar::variableName(in).replace(" ","_");
        if(in==0) strange = " "+strange;// start with a blank space for consistency with polar data
        for(int il=strange.length(); il<11; il++) strange+=" ";
        polardata += strange+sep;
    }
    polardata += "\n";


    for(int i=0; i<m_Alpha.count(); i++)
    {
        for(int iVar=0; iVar<WPolar::variableCount(); iVar++)
        {
            strange = QString::asprintf("%11.5g", variable(iVar, i));
            polardata += strange+sep;
        }
        polardata += "\n";
    }
}


bool WPolar::hasPOpp(const PlaneOpp *pPOpp) const
{
    if(!pPOpp) return false;
    return (pPOpp->polarName().compare(m_Name)==0) && (pPOpp->planeName().compare(m_PlaneName)==0);
}


/** only applicable for WPolarExt */
void WPolar::setData(int, int, double)
{
    return;
}


/**
 * @return the extra drag force, in N/q
 */
double WPolar::extraDragTotal(double CL) const
{
    double extradrag = constantDrag();

    if(m_bAVLDrag)
    {
        double CDv = AVLDrag(CL);
        extradrag += CDv * m_RefArea;
    }

    return extradrag;
}


void WPolar::recalcExtraDrag()
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


double WPolar::QInfCtrl(double ctrl) const
{
    if(m_Type==xfl::T6POLAR)
        return m_OperatingRange.at(0).ctrlVal(ctrl);
    else return 0.0;
}


double WPolar::aoaCtrl(double ctrl) const
{
    if(m_Type==xfl::T6POLAR)
        return m_OperatingRange.at(1).ctrlVal(ctrl);
    else return 0.0;
}


double WPolar::betaCtrl(double ctrl) const
{
    if(m_Type==xfl::T6POLAR)
        return m_OperatingRange.at(2).ctrlVal(ctrl);
    else return 0.0;
}


double WPolar::phiCtrl(double ctrl) const
{
    if(m_Type==xfl::T6POLAR)
        return m_OperatingRange.at(3).ctrlVal(ctrl);
    else return 0.0;
}


/**
 * Returns the mass for a value of the control parameter.
  */
double WPolar::massCtrl(double ctrl) const
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
Vector3d WPolar::CoGCtrl(double ctrl) const
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
double WPolar::fuseDrag(Fuse const *pFuse, double QInf) const
{
    double Reynolds = pFuse->length() * QInf / m_Viscosity;
    double Cf = fuseDragCoef(Reynolds);
    double ff = pFuse->formFactor();
    return ff * pFuse->wettedArea() * Cf;
}


double WPolar::fuseDragCoef(double Reynolds) const
{
    switch(m_FuseDragMethod)
    {
        case WPolar::KARMANSCHOENHERR:
            return KarmanSchoenherrCoef(Reynolds);
        case WPolar::PRANDTLSCHLICHTING:
            return PrandtlSchlichtingCoef(Reynolds);
        case WPolar::MANUALFUSECF:
            return m_FuseCf;
    }
    return 0.0;
}


/**
 * Calculates and returns the skin friction coefficient i.a.w. Karman-Schoenherr implicit formula
 * @param Re  the operating Renulods number
 * @return the skin friction coefficient Cf
 */
double WPolar::KarmanSchoenherrCoef(double Re) const
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


double WPolar::PrandtlSchlichtingCoef(double Re) const
{
    return 0.455*pow(log10(Re), -2.58);
}



