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


#include <api/planepolarnamemaker.h>
#include <api/planexfl.h>
#include <api/units.h>
#include <api/utils.h>
#include <api/planepolar.h>

bool PlanePolarNameMaker::s_bBC=false;
bool PlanePolarNameMaker::s_bControls=true;
bool PlanePolarNameMaker::s_bExtraDrag=false;
bool PlanePolarNameMaker::s_bFuseDrag=false;
bool PlanePolarNameMaker::s_bGround=true;
bool PlanePolarNameMaker::s_bInertia=true;
bool PlanePolarNameMaker::s_bMethod=true;
bool PlanePolarNameMaker::s_bSurfaces=true;
bool PlanePolarNameMaker::s_bType=true;
bool PlanePolarNameMaker::s_bViscosity=true;


PlanePolarNameMaker::PlanePolarNameMaker()
{
}


std::string PlanePolarNameMaker::makeName(Plane const *pPlane, PlanePolar const *pPlPolar)
{
    std::string plrname;
    if(!pPlPolar) return std::string();

    std::string str, strong;
    std::string strSpeedUnit = Units::speedUnitLabel();

    if(s_bType)
    {
        switch(pPlPolar->type())
        {
            case xfl::T1POLAR:
            {
                if(fabs(pPlPolar->betaSpec())<ANGLEPRECISION)
                    plrname = std::format("-T1-{:.1f} ", pPlPolar->velocity() * Units::mstoUnit());
                else
                {
                    plrname = "-T1-" + BETAstr + std::format("{:.1f}", pPlPolar->betaSpec())+DEGstr;
                    plrname += std::format("-{:.1f}",pPlPolar->velocity() * Units::mstoUnit());
                }
                plrname += strSpeedUnit;
                break;
            }
            case xfl::T2POLAR:
            {
                plrname = "-T2";
                break;
            }
            case xfl::T3POLAR:
            {
                plrname = "-T3";
                break;
            }
            case xfl::T4POLAR: // deprecated, unused
            {
                plrname = "-T4-" + ALPHAstr + std::format("{:.1f}",pPlPolar->alphaSpec()) + DEGstr;
                break;
            }
            case xfl::T5POLAR:
            {
                plrname = "-T5-" + ALPHAstr + std::format("{:.1f}", pPlPolar->alphaSpec())+DEGstr;
                plrname += std::format("-{:.1f}",pPlPolar->velocity() * Units::mstoUnit());
                plrname += strSpeedUnit;
                break;
            }
            case xfl::T6POLAR:
            {
                plrname = "-T6";
                if(pPlPolar->isAdjustedVelocity()) plrname+="/2";
                else                              plrname+="/1";
                break;
            }
            case xfl::T7POLAR:
            {
                plrname = "-T7";
                break;
            }
            case xfl::T8POLAR:
            {
                plrname = "-T8";
                break;
            }
            default:
            {
                plrname = "-Tx";
                break;
            }
        }
    }

    if(fabs(pPlPolar->phi())>AOAPRECISION)
    {
        plrname += "-" + PHIstr + std::format("{:.1f}", pPlPolar->phi()) + DEGstr;
    }

    if(s_bMethod)
    {
        switch(pPlPolar->analysisMethod())
        {
            case xfl::LLT:
            {
                plrname += "-LLT";
                break;
            }
            case xfl::VLM1:
            {
                plrname += "-VLM1";
                break;
            }
            case xfl::VLM2:
            {
                plrname += "-VLM2";
                break;
            }
            case xfl::QUADS:
            {
                plrname += "-Quads";
                break;
            }
            case xfl::TRILINEAR:
            {
                plrname += "-TriLinear";
                break;
            }
            case xfl::TRIUNIFORM:
            {
                plrname += "-TriUniform";
                break;
            }
            case xfl::NOMETHOD:
            {
                plrname += "-NoMethod";
                break;
            }
        }
    }

    if(s_bSurfaces && pPlane && pPlane->isXflType())
    {
        if(pPlPolar->isLLTMethod() || pPlPolar->isVLM())
        {
        }
        else
        {
            if(pPlPolar->bThinSurfaces()) plrname += "-ThinSurf";
            else                         plrname += "-ThickSurf";
        }
    }

    if(s_bBC)
    {
        if(pPlPolar->bDirichlet()) plrname += "-Dirichlet";
        else                      plrname += "-Neumann";
    }

    if(pPlPolar->bTrefftz())
    {
    }
    else
    {
        plrname += "-ForceSum";
    }


    if(s_bInertia)
    {
        str.clear();
        if(pPlPolar->bAutoInertia())
        {
        }
        else
        {
            if(pPlPolar->isControlPolar())
            {
            }
            else  if(pPlPolar->isStabilityPolar())
            {
            }
            else
            {
                if(pPlPolar->isFixedLiftPolar())
                {
                    strong = std::format("-{:.1f}", pPlPolar->mass()*Units::kgtoUnit());
                    plrname += strong + Units::massUnitLabel();
                }

                strong = std::format("-x{:.1f}", pPlPolar->CoG().x*Units::mtoUnit());
                plrname += strong + Units::lengthUnitLabel();

                if(fabs(pPlPolar->CoG().z)>=LENGTHPRECISION)
                {
                    strong = std::format("-z{:.1f}", pPlPolar->CoG().z*Units::mtoUnit());
                    plrname += strong + Units::lengthUnitLabel();
                }
            }
        }
    }

    if(s_bViscosity)
    {
        if(!pPlPolar->isViscous())
        {
            plrname += "-Inviscid";
        }
        else
        {
            if(!pPlPolar->isLLTMethod())
            {
                if(pPlPolar->isViscOnTheFly())
                    plrname += "-ViscOTF";
                if(pPlPolar->isViscInterpolated() && pPlPolar->bViscousLoop())
                    plrname += "-ViscLoop";
            }
        }
    }

    if(s_bControls && pPlane && pPlane->isXflType())
    {
        PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

        switch(pPlPolar->type())
        {
            case xfl::T1POLAR:
            case xfl::T2POLAR:
            case xfl::T3POLAR:
            case xfl::T5POLAR:
            case xfl::T7POLAR:
            case xfl::T8POLAR:
                if(pPlPolar->hasActiveFlap())
                {
                    if(pPlPolar->flapCtrlsName().length()!=0)
                        plrname += "-" + pPlPolar->flapCtrlsName();
                }
                break;
            case xfl::T6POLAR:
                plrname += rangeControlNames(pPlaneXfl, pPlPolar);
                break;
            default:
                break;
        }
    }

    if(s_bExtraDrag)
    {
        for(int i=0; i<pPlPolar->extraDragCount(); i++)
        {
            if(fabs(pPlPolar->extraDrag(i).coef())>PRECISION && fabs(pPlPolar->extraDrag(i).area())>PRECISION)
            {
                plrname+="-ExtraDrag";
                break;
            }
        }
    }

    if(s_bFuseDrag)
    {
        if(pPlPolar->hasFuseDrag())
            plrname += "-FuseDrag";
    }

    if(s_bGround)
    {
        if(pPlPolar->bGroundEffect())
        {
            strong = std::format("-G{:.1f}", pPlPolar->groundHeight()*Units::mtoUnit());
            plrname += strong +Units::lengthUnitLabel();
        }
        else if(pPlPolar->bFreeSurfaceEffect())
        {
            strong = std::format("-FS{:.1f}", pPlPolar->groundHeight()*Units::mtoUnit());
            plrname += strong +Units::lengthUnitLabel();
        }
    }


/*    if(fabs(pPlPolar->betaSpec()) > ANGLEPRECISION  && !pPlPolar->isBetaPolar())
    {
        strong = "-" + BETAstr + std::format("{:.1f}", pPlPolar->betaSpec()) + DEGstr;
        plrname += strong;
    }

    if(fabs(pPlPolar->phi()) > ANGLEPRECISION)
    {
        strong = "-" + PHIstr + std::format("{:.1f}", pPlPolar->phi()) + DEGstr;
        plrname += strong;
    }*/

    //    if(pPlPolar->referenceDim()==Xfl::PROJECTEDREFDIM) plrname += "-proj_area";

    //    if(pPlPolar->isTilted()) plrname += "-TG";

    //    if(pPlPolar->bWakeRollUp()) plrname += "-rollup";

    if(pPlPolar->bVortonWake()) plrname += "-VPW";

    plrname = plrname.substr(1); //remove first character

    return plrname;
}


std::string PlanePolarNameMaker::rangeControlNames(PlaneXfl const *pPlane, PlanePolar const *pPlPolar)
{
    if(!pPlane) return std::string();
    if(!pPlPolar || !pPlPolar->isControlPolar()) return std::string();
    std::string strong;
    std::string plrname;

    // Operating range
    if(fabs(pPlPolar->m_OperatingRange.at(0).range())>PRECISION)
    {
        strong = "-V" + INFstr + std::format("[{:g},{:g}]", pPlPolar->m_OperatingRange.at(0).ctrlMin()*Units::mstoUnit(), pPlPolar->m_OperatingRange.at(0).ctrlMax()*Units::mstoUnit());
        plrname += strong;
    }
    if(fabs(pPlPolar->m_OperatingRange.at(1).range())>PRECISION)
    {
        strong = "-" + ALPHAstr +std::format("[{:g},{:g}]", pPlPolar->m_OperatingRange.at(1).ctrlMin(), pPlPolar->m_OperatingRange.at(1).ctrlMax());
        plrname += strong;
    }
    if(fabs(pPlPolar->m_OperatingRange.at(2).range())>PRECISION)
    {
        strong = "-" + BETAstr + std::format("[{:g},{:g}]", pPlPolar->m_OperatingRange.at(2).ctrlMin(), pPlPolar->m_OperatingRange.at(2).ctrlMax());
        plrname += strong;
    }
    if(fabs(pPlPolar->m_OperatingRange.at(3).range())>PRECISION)
    {
        strong = "-" + PHIstr + std::format("[{:g},{:g}]", pPlPolar->m_OperatingRange.at(3).ctrlMin(), pPlPolar->m_OperatingRange.at(3).ctrlMax());
        plrname += strong;
    }

    // Inertia
    if(fabs(pPlPolar->m_InertiaRange.at(0).range())>PRECISION)
    {
        strong = std::format("-Mass[{:g},{:g}]", pPlPolar->m_InertiaRange.at(0).ctrlMin()*Units::kgtoUnit(), pPlPolar->m_InertiaRange.at(0).ctrlMax()*Units::kgtoUnit());
        plrname += strong;
    }
    if(fabs(pPlPolar->m_InertiaRange.at(1).range())>PRECISION)
    {
        strong = std::format("-CGx[{:g},{:g}]", pPlPolar->m_InertiaRange.at(1).ctrlMin()*Units::mtoUnit(), pPlPolar->m_InertiaRange.at(1).ctrlMax()*Units::mtoUnit());
        plrname += strong;
    }
    if(fabs(pPlPolar->m_InertiaRange.at(2).range())>PRECISION)
    {
        strong = std::format("-CGz[{:g},{:g}]", pPlPolar->m_InertiaRange.at(2).ctrlMin()*Units::mtoUnit(), pPlPolar->m_InertiaRange.at(2).ctrlMax()*Units::mtoUnit());
        plrname += strong;
    }

    // Angle Controls
    for(int iw=0; iw<int(pPlPolar->m_AngleRange.size()); iw++)
    {
        if(pPlPolar->m_AngleRange.at(iw).size()>0)
        {
            if(fabs(pPlPolar->m_AngleRange.at(iw).at(0).range())>PRECISION)
            {
                strong = std::format("[{:g},{:g}]", pPlPolar->angleRange(iw,0).ctrlMin(), pPlPolar->angleRange(iw,0).ctrlMax());
                plrname += "-"+ pPlane->wingAt(iw)->name() + "_"+strong;
            }
        }
        for(unsigned int iFlap=1; iFlap<pPlPolar->m_AngleRange.at(iw).size(); iFlap++)
        {
            if(fabs(pPlPolar->angleRange(iw, iFlap).range())>PRECISION)
            {
                strong = std::format("-F{:d}[{:g},{:g}]", iFlap, pPlPolar->angleRange(iw, iFlap).ctrlMin(), pPlPolar->angleRange(iw, iFlap).ctrlMax());
                plrname += strong;
            }
        }
    }
    return plrname;
}

