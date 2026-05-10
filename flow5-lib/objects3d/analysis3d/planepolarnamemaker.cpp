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


std::string PlanePolarNameMaker::makeName(Plane const *pPlane, PlanePolar const *pWPolar)
{
    std::string plrname;
    if(!pWPolar) return std::string();

    std::string str, strong;
    std::string strSpeedUnit = Units::speedUnitLabel();

    if(s_bType)
    {
        switch(pWPolar->type())
        {
        case xfl::T1POLAR:
        {
            plrname = std::format("-T1-{:.1f} ", pWPolar->velocity() * Units::mstoUnit());
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
            plrname = "-T4-" + ALPHAstr + std::format("{:.1f}",pWPolar->alphaSpec()) + DEGstr;
            break;
        }
        case xfl::T5POLAR:
        {
            plrname = "-T5-" + ALPHAstr + std::format("{:.1f}", pWPolar->alphaSpec())+DEGstr;
            plrname += std::format("-{:.1f}",pWPolar->velocity() * Units::mstoUnit());
            plrname += strSpeedUnit;
            break;
        }
        case xfl::T6POLAR:
        {
            plrname = "-T6";
            if(pWPolar->isAdjustedVelocity()) plrname+="/2";
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

    if(fabs(pWPolar->phi())>AOAPRECISION)
    {
        plrname += "-" + PHIstr + std::format("{:.1f}", pWPolar->phi()) + DEGstr;
    }

    if(s_bMethod)
    {
        switch(pWPolar->analysisMethod())
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
        if(pWPolar->isLLTMethod() || pWPolar->isVLM())
        {
        }
        else
        {
            if(pWPolar->bThinSurfaces()) plrname += "-ThinSurf";
            else                         plrname += "-ThickSurf";
        }
    }

    if(s_bBC)
    {
        if(pWPolar->bDirichlet()) plrname += "-Dirichlet";
        else                      plrname += "-Neumann";
    }

    if(pWPolar->bTrefftz())
    {
    }
    else
    {
        plrname += "-ForceSum";
    }


    if(s_bInertia)
    {
        str.clear();
        if(pWPolar->bAutoInertia())
        {
        }
        else
        {
            if(pWPolar->isControlPolar())
            {
            }
            else  if(pWPolar->isStabilityPolar())
            {
            }
            else
            {
                if(pWPolar->isFixedLiftPolar())
                {
                    strong = std::format("-{:.1f}", pWPolar->mass()*Units::kgtoUnit());
                    plrname += strong + Units::massUnitLabel();
                }

                strong = std::format("-x{:.1f}", pWPolar->CoG().x*Units::mtoUnit());
                plrname += strong + Units::lengthUnitLabel();

                if(fabs(pWPolar->CoG().z)>=LENGTHPRECISION)
                {
                    strong = std::format("-z{:.1f}", pWPolar->CoG().z*Units::mtoUnit());
                    plrname += strong + Units::lengthUnitLabel();
                }
            }
        }
    }

    if(s_bViscosity)
    {
        if(!pWPolar->isViscous())
        {
            plrname += "-Inviscid";
        }
        else
        {
            if(!pWPolar->isLLTMethod())
            {
                if(pWPolar->isViscOnTheFly())
                    plrname += "-ViscOTF";
                if(pWPolar->isViscInterpolated() && pWPolar->bViscousLoop())
                    plrname += "-ViscLoop";
            }
        }
    }

    if(s_bControls && pPlane && pPlane->isXflType())
    {
        PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

        switch(pWPolar->type())
        {
        case xfl::T1POLAR:
        case xfl::T2POLAR:
        case xfl::T3POLAR:
        case xfl::T5POLAR:
        case xfl::T8POLAR:
            if(pWPolar->hasActiveFlap())
            {
                if(pWPolar->flapCtrlsName().length()!=0)
                    plrname += "-" + pWPolar->flapCtrlsName();
            }
            break;
        case xfl::T6POLAR:
            plrname += rangeControlNames(pPlaneXfl, pWPolar);
            break;
        case xfl::T7POLAR:
            plrname += stabilityControlNames(pPlaneXfl, pWPolar);
            break;
        default:
            break;
        }
    }

    if(s_bExtraDrag)
    {
        for(int i=0; i<pWPolar->extraDragCount(); i++)
        {
            if(fabs(pWPolar->extraDrag(i).coef())>PRECISION && fabs(pWPolar->extraDrag(i).area())>PRECISION)
            {
                plrname+="-ExtraDrag";
                break;
            }
        }
    }

    if(s_bFuseDrag)
    {
        if(pWPolar->hasFuseDrag())
            plrname += "-FuseDrag";
    }

    if(s_bGround)
    {
        if(pWPolar->bGroundEffect())
        {
            strong = std::format("-G{:.1f}", pWPolar->groundHeight()*Units::mtoUnit());
            plrname += strong +Units::lengthUnitLabel();
        }
        else if(pWPolar->bFreeSurfaceEffect())
        {
            strong = std::format("-FS{:.1f}", pWPolar->groundHeight()*Units::mtoUnit());
            plrname += strong +Units::lengthUnitLabel();
        }
    }


/*    if(fabs(pWPolar->betaSpec()) > ANGLEPRECISION  && !pWPolar->isBetaPolar())
    {
        strong = "-" + BETAstr + std::format("{:.1f}", pWPolar->betaSpec()) + DEGstr;
        plrname += strong;
    }

    if(fabs(pWPolar->phi()) > ANGLEPRECISION)
    {
        strong = "-" + PHIstr + std::format("{:.1f}", pWPolar->phi()) + DEGstr;
        plrname += strong;
    }*/

    //    if(pWPolar->referenceDim()==Xfl::PROJECTEDREFDIM) plrname += "-proj_area";

    //    if(pWPolar->isTilted()) plrname += "-TG";

    //    if(pWPolar->bWakeRollUp()) plrname += "-rollup";

    if(pWPolar->bVortonWake()) plrname += "-VPW";

    plrname = plrname.substr(1); //remove first character

    return plrname;
}


std::string PlanePolarNameMaker::stabilityControlNames(const PlaneXfl *pPlane, const PlanePolar *pWPolar)
{
    if(!pPlane) return std::string();
    if(!pWPolar || !pWPolar->isStabilityPolar()) return std::string();

    std::string plrname;
    /*
    for(int iw=0; iw<pWPolar->m_AngleGain.size(); iw++)
    {
        bool bHasGain = false;
        for(int ictrl=0; ictrl<pWPolar->m_AngleGain.at(iw).size(); ictrl++)
        {
            if(fabs(pWPolar->m_AngleGain.at(iw).at(ictrl))>ANGLEPRECISION)
            {
                bHasGain = true;
                break;
            }
        }

        if(bHasGain)
        {
            plrname += "-["+pPlane->wingAt(iw)->name();
            if(pWPolar->m_AngleGain.at(iw).size()>0 && fabs(pWPolar->m_AngleGain.at(iw).at(0))>ANGLEPRECISION)
            {
                strong = std::format("(g{:.1f})", pWPolar->angleGain(iw,0));
                plrname += "_"+strong;
            }
            for(int iFlap=1; iFlap<pWPolar->m_AngleGain.at(iw).size(); iFlap++)
            {
                if(fabs(pWPolar->angleGain(iw, iFlap))>ANGLEPRECISION)
                {
                    strong = std::format("F{:d}(g{:.1f})", iFlap, pWPolar->angleGain(iw, iFlap));
                    plrname += "_"+strong;
                }
            }
            plrname +="]";
        }
    }*/
    return plrname;
}


std::string PlanePolarNameMaker::rangeControlNames(PlaneXfl const *pPlane, PlanePolar const *pWPolar)
{
    if(!pPlane) return std::string();
    if(!pWPolar || !pWPolar->isControlPolar()) return std::string();
    std::string strong;
    std::string plrname;

    // Operating range
    if(fabs(pWPolar->m_OperatingRange.at(0).range())>PRECISION)
    {
        strong = "-V" + INFstr + std::format("[{:g},{:g}]", pWPolar->m_OperatingRange.at(0).ctrlMin()*Units::mstoUnit(), pWPolar->m_OperatingRange.at(0).ctrlMax()*Units::mstoUnit());
        plrname += strong;
    }
    if(fabs(pWPolar->m_OperatingRange.at(1).range())>PRECISION)
    {
        strong = "-" + ALPHAstr +std::format("[{:g},{:g}]", pWPolar->m_OperatingRange.at(1).ctrlMin(), pWPolar->m_OperatingRange.at(1).ctrlMax());
        plrname += strong;
    }
    if(fabs(pWPolar->m_OperatingRange.at(2).range())>PRECISION)
    {
        strong = "-" + BETAstr + std::format("[{:g},{:g}]", pWPolar->m_OperatingRange.at(2).ctrlMin(), pWPolar->m_OperatingRange.at(2).ctrlMax());
        plrname += strong;
    }
    if(fabs(pWPolar->m_OperatingRange.at(3).range())>PRECISION)
    {
        strong = "-" + PHIstr + std::format("[{:g},{:g}]", pWPolar->m_OperatingRange.at(3).ctrlMin(), pWPolar->m_OperatingRange.at(3).ctrlMax());
        plrname += strong;
    }

    // Inertia
    if(fabs(pWPolar->m_InertiaRange.at(0).range())>PRECISION)
    {
        strong = std::format("-Mass[{:g},{:g}]", pWPolar->m_InertiaRange.at(0).ctrlMin()*Units::kgtoUnit(), pWPolar->m_InertiaRange.at(0).ctrlMax()*Units::kgtoUnit());
        plrname += strong;
    }
    if(fabs(pWPolar->m_InertiaRange.at(1).range())>PRECISION)
    {
        strong = std::format("-CGx[{:g},{:g}]", pWPolar->m_InertiaRange.at(1).ctrlMin()*Units::mtoUnit(), pWPolar->m_InertiaRange.at(1).ctrlMax()*Units::mtoUnit());
        plrname += strong;
    }
    if(fabs(pWPolar->m_InertiaRange.at(2).range())>PRECISION)
    {
        strong = std::format("-CGz[{:g},{:g}]", pWPolar->m_InertiaRange.at(2).ctrlMin()*Units::mtoUnit(), pWPolar->m_InertiaRange.at(2).ctrlMax()*Units::mtoUnit());
        plrname += strong;
    }

    // Angle Controls
    for(int iw=0; iw<int(pWPolar->m_AngleRange.size()); iw++)
    {
        if(pWPolar->m_AngleRange.at(iw).size()>0)
        {
            if(fabs(pWPolar->m_AngleRange.at(iw).at(0).range())>PRECISION)
            {
                strong = std::format("[{:g},{:g}]", pWPolar->angleRange(iw,0).ctrlMin(), pWPolar->angleRange(iw,0).ctrlMax());
                plrname += "-"+ pPlane->wingAt(iw)->name() + "_"+strong;
            }
        }
        for(unsigned int iFlap=1; iFlap<pWPolar->m_AngleRange.at(iw).size(); iFlap++)
        {
            if(fabs(pWPolar->angleRange(iw, iFlap).range())>PRECISION)
            {
                strong = std::format("-F{:d}[{:g},{:g}]", iFlap, pWPolar->angleRange(iw, iFlap).ctrlMin(), pWPolar->angleRange(iw, iFlap).ctrlMax());
                plrname += strong;
            }
        }
    }
    return plrname;
}

