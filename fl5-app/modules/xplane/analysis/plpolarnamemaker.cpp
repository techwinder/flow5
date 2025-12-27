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



#include <api/planexfl.h>
#include <api/units.h>
#include <api/utils.h>
#include <api/planepolar.h>
#include <modules/xplane/analysis/plpolarnamemaker.h>

bool PlPolarNameMaker::s_bBC=false;
bool PlPolarNameMaker::s_bControls=true;
bool PlPolarNameMaker::s_bExtraDrag=false;
bool PlPolarNameMaker::s_bFuseDrag=false;
bool PlPolarNameMaker::s_bGround=true;
bool PlPolarNameMaker::s_bInertia=true;
bool PlPolarNameMaker::s_bMethod=true;
bool PlPolarNameMaker::s_bSurfaces=true;
bool PlPolarNameMaker::s_bType=true;
bool PlPolarNameMaker::s_bViscosity=true;


PlPolarNameMaker::PlPolarNameMaker()
{
}

QString PlPolarNameMaker::makeName(Plane const *pPlane, PlanePolar const *pWPolar)
{
    QString plrname;
    if(!pWPolar) return QString();

    QString str, strong;
    QString strSpeedUnit = Units::speedUnitQLabel();

    if(s_bType)
    {
        switch(pWPolar->type())
        {
            case xfl::T1POLAR:
            {
                plrname = QString::asprintf("-T1-%.1f ", pWPolar->velocity() * Units::mstoUnit());
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
                plrname = "-T4-" + ALPHAch + QString::asprintf("%.1f",pWPolar->alphaSpec()) + DEGch;
                break;
            }
            case xfl::T5POLAR:
            {
                plrname = "-T5-" + ALPHAch + QString::asprintf("%.1f", pWPolar->alphaSpec())+DEGch;
                plrname += QString::asprintf("-%.1f",pWPolar->velocity() * Units::mstoUnit());
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
        plrname += "-" + PHIch + QString::asprintf("%.1f", pWPolar->phi()) + DEGch;
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
                    strong = QString::asprintf("-%.1f", pWPolar->mass()*Units::kgtoUnit());
                    plrname += strong + Units::massUnitQLabel();
                }

                strong = QString::asprintf("-x%.1f", pWPolar->CoG().x*Units::mtoUnit());
                plrname += strong + Units::lengthUnitQLabel();

                if(fabs(pWPolar->CoG().z)>=LENGTHPRECISION)
                {
                    strong = QString::asprintf("-z%.1f", pWPolar->CoG().z*Units::mtoUnit());
                    plrname += strong + Units::lengthUnitQLabel();
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
                    if(pWPolar->flapCtrlsSetName().length()!=0)
                        plrname += "-" + QString::fromStdString(pWPolar->flapCtrlsSetName());
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
            strong = QString::asprintf("-G%.1f", pWPolar->groundHeight()*Units::mtoUnit());
            plrname += strong +Units::lengthUnitQLabel();
        }
        else if(pWPolar->bFreeSurfaceEffect())
        {
            strong = QString::asprintf("-FS%.1f", pWPolar->groundHeight()*Units::mtoUnit());
            plrname += strong +Units::lengthUnitQLabel();
        }
    }


/*    if(fabs(pWPolar->beta()) > .001  && !pWPolar->isBetaPolar())
    {
        strong = QString(QString::fromUtf8("-b%1°")).arg(pWPolar->beta(),0,'f',1);
        plrname += strong;
    }

    if(fabs(pWPolar->phi()) > .001)
    {
        strong = QString(QString::fromUtf8("-B%1°")).arg(pWPolar->phi(),0,'f',1);
        plrname += strong;
    }*/

//    if(pWPolar->referenceDim()==Xfl::PROJECTEDREFDIM) plrname += "-proj_area";

//    if(pWPolar->isTilted()) plrname += "-TG";

//    if(pWPolar->bWakeRollUp()) plrname += "-rollup";

    if(pWPolar->bVortonWake()) plrname += "-VPW";

    plrname.remove(0,1); //remove first character

    return plrname;
}


QString PlPolarNameMaker::stabilityControlNames(const PlaneXfl *pPlane, const PlanePolar *pWPolar)
{
    if(!pPlane) return QString();
    if(!pWPolar || !pWPolar->isStabilityPolar()) return QString();

    QString plrname;
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
                strong = QString::asprintf("(g%.1f)", pWPolar->angleGain(iw,0));
                plrname += "_"+strong;
            }
            for(int iFlap=1; iFlap<pWPolar->m_AngleGain.at(iw).size(); iFlap++)
            {
                if(fabs(pWPolar->angleGain(iw, iFlap))>ANGLEPRECISION)
                {
                    strong = QString::asprintf("F%d(g%.1f)", iFlap, pWPolar->angleGain(iw, iFlap));
                    plrname += "_"+strong;
                }
            }
            plrname +="]";
        }
    }*/
    return plrname;
}


QString PlPolarNameMaker::rangeControlNames(PlaneXfl const *pPlane, PlanePolar const *pWPolar)
{
    if(!pPlane) return QString();
    if(!pWPolar || !pWPolar->isControlPolar()) return QString();
    QString strong;
    QString plrname;

    // Operating range
    if(fabs(pWPolar->m_OperatingRange.at(0).range())>PRECISION)
    {
        strong = "-V" + INFch + QString::asprintf("[%g,%g]", pWPolar->m_OperatingRange.at(0).ctrlMin()*Units::mstoUnit(), pWPolar->m_OperatingRange.at(0).ctrlMax()*Units::mstoUnit());
        plrname += strong;
    }
    if(fabs(pWPolar->m_OperatingRange.at(1).range())>PRECISION)
    {
        strong = "-" + ALPHAch +QString::asprintf("[%g,%g]", pWPolar->m_OperatingRange.at(1).ctrlMin(), pWPolar->m_OperatingRange.at(1).ctrlMax());
        plrname += strong;
    }
    if(fabs(pWPolar->m_OperatingRange.at(2).range())>PRECISION)
    {
        strong = "-" + BETAch + QString::asprintf("[%g,%g]", pWPolar->m_OperatingRange.at(2).ctrlMin(), pWPolar->m_OperatingRange.at(2).ctrlMax());
        plrname += strong;
    }
    if(fabs(pWPolar->m_OperatingRange.at(3).range())>PRECISION)
    {
        strong = "-" + PHIch + QString::asprintf("[%g,%g]", pWPolar->m_OperatingRange.at(3).ctrlMin(), pWPolar->m_OperatingRange.at(3).ctrlMax());
        plrname += strong;
    }

    // Inertia
    if(fabs(pWPolar->m_InertiaRange.at(0).range())>PRECISION)
    {
        strong = QString::asprintf("-Mass[%g,%g]", pWPolar->m_InertiaRange.at(0).ctrlMin()*Units::kgtoUnit(), pWPolar->m_InertiaRange.at(0).ctrlMax()*Units::kgtoUnit());
        plrname += strong;
    }
    if(fabs(pWPolar->m_InertiaRange.at(1).range())>PRECISION)
    {
        strong = QString::asprintf("-CGx[%g,%g]", pWPolar->m_InertiaRange.at(1).ctrlMin()*Units::mtoUnit(), pWPolar->m_InertiaRange.at(1).ctrlMax()*Units::mtoUnit());
        plrname += strong;
    }
    if(fabs(pWPolar->m_InertiaRange.at(2).range())>PRECISION)
    {
        strong = QString::asprintf("-CGz[%g,%g]", pWPolar->m_InertiaRange.at(2).ctrlMin()*Units::mtoUnit(), pWPolar->m_InertiaRange.at(2).ctrlMax()*Units::mtoUnit());
        plrname += strong;
    }

    // Angle Controls
    for(int iw=0; iw<int(pWPolar->m_AngleRange.size()); iw++)
    {
        if(pWPolar->m_AngleRange.at(iw).size()>0)
        {
            if(fabs(pWPolar->m_AngleRange.at(iw).at(0).range())>PRECISION)
            {
                strong = QString::asprintf("[%g,%g]", pWPolar->angleRange(iw,0).ctrlMin(), pWPolar->angleRange(iw,0).ctrlMax());
                plrname += "-"+QString::fromStdString(pPlane->wingAt(iw)->name())+"_"+strong;
            }
        }
        for(uint iFlap=1; iFlap<pWPolar->m_AngleRange.at(iw).size(); iFlap++)
        {
            if(fabs(pWPolar->angleRange(iw, iFlap).range())>PRECISION)
            {
                strong = QString::asprintf("-F%d[%g,%g]", iFlap, pWPolar->angleRange(iw, iFlap).ctrlMin(), pWPolar->angleRange(iw, iFlap).ctrlMax());
                plrname += strong;
            }
        }
    }
    return plrname;
}

