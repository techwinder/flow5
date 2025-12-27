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



#include "btpolarnamemaker.h"

#include <api/boatpolar.h>
#include <api/boat.h>
#include <api/sail.h>
#include <api/units.h>


bool BtPolarNameMaker::s_bMethod=true;
bool BtPolarNameMaker::s_bBC=false;
bool BtPolarNameMaker::s_bViscosity=true;
bool BtPolarNameMaker::s_bInertia=true;
bool BtPolarNameMaker::s_bControls=true;
bool BtPolarNameMaker::s_bExtraDrag=false;
bool BtPolarNameMaker::s_bGround=false;

BtPolarNameMaker::BtPolarNameMaker()
{

}


QString BtPolarNameMaker::makeName(Boat const *pBoat, BoatPolar const *pBtPolar)
{
    QString plrname;
    if(!pBtPolar) return QString();
    if(!pBoat) return QString();

    QString strong;
    QString strSpeedUnit;
    strSpeedUnit = Units::speedUnitQLabel();


    if(s_bMethod)
    {
        switch(pBtPolar->analysisMethod())
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
                plrname += "-Quad";
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
            default:
                break;
        }
/*        if(pBtPolar->ignoreBodyPanels())
        {
            plrname += "-IgnoreFuse";
        }*/
    }

    if(s_bBC)
    {
        if(pBtPolar->bDirichlet()) plrname += "-Dirichlet";
        else                       plrname += "-Neumann";
    }

    if(pBtPolar->bTrefftz())
    {
    }
    else
    {
        plrname += "-ForceSum";
    }

    if(s_bInertia)
    {
        if(pBtPolar->bAutoInertia())
        {
        }
        else
        {
            if(pBtPolar->isControlPolar())
            {
            }
        }
    }

    if(s_bViscosity)
    {
/*        if(!pBtPolar->isViscous())             plrname += "-Inviscid";
        else if(pBtPolar->isViscousOnTheFly()) plrname += "-ViscOTF";
        else                                  plrname += "-Viscous";*/
    }

    if(s_bControls)
    {
         plrname += rangeControlNames(pBoat, pBtPolar);
    }

    if(s_bExtraDrag)
    {
        for(int i=0; i<pBtPolar->extraDragCount(); i++)
        {
            if(fabs(pBtPolar->extraDrag(i).coef())>PRECISION && fabs(pBtPolar->extraDrag(i).area())>PRECISION)
            {
                plrname+="-ExtraDrag";
                break;
            }
        }
    }


    if(s_bGround)
    {
        if(pBtPolar->bGroundEffect())
        {
            strong = QString::asprintf("-G%.1f", pBtPolar->groundHeight()*Units::mtoUnit());
            plrname += strong + Units::lengthUnitQLabel();
        }
    }

    if(pBtPolar->bVortonWake()) plrname += "-VPW";



/*    if(fabs(pBtPolar->beta()) > .001  && !pBtPolar->isBetaPolar())
    {
        strong = QString(QString::fromUtf8("-b%1°")).arg(pBtPolar->beta(),0,'f',1);
        plrname += strong;
    }

    if(fabs(pBtPolar->phi()) > .001)
    {
        strong = QString(QString::fromUtf8("-B%1°")).arg(pBtPolar->phi(),0,'f',1);
        plrname += strong;
    }*/

//    if(pBtPolar->referenceDim()==xfl::PROJECTEDREFDIM) plrname += "-proj_area";

//    if(pBtPolar->isTilted()) plrname += "-TG";

//    if(pBtPolar->bWakeRollUp()) plrname += "-rollup";

    plrname.remove(0,1); //remove first character

    return plrname;
}


QString BtPolarNameMaker::rangeControlNames(Boat const*pBoat, BoatPolar const*pBtPolar)
{
    if(!pBoat) return QString();
    if(!pBtPolar) return QString();
    QString strong;
    QString plrname;


    // Operating range
    if(fabs(pBtPolar->m_VBtMax-pBtPolar->m_VBtMin)>PRECISION)
    {
        strong = QString::asprintf("-Vbt(%.1f,%.1f)", pBtPolar->m_VBtMin*Units::mstoUnit(), pBtPolar->m_VBtMax*Units::mstoUnit());
        plrname += strong;
    }

    if(fabs(pBtPolar->m_TWSMax-pBtPolar->m_TWSMin)>PRECISION)
    {
        strong = QString::asprintf("-TWS(%.1f,%.1f)", pBtPolar->m_TWSMin*Units::mstoUnit(), pBtPolar->m_TWSMax*Units::mstoUnit());
        plrname += strong;
    }

    if(fabs(pBtPolar->m_TWAMax-pBtPolar->m_TWAMin)>PRECISION)
    {
        strong = QString::asprintf("-TWA(%.1f,%.1f)", pBtPolar->m_TWAMin, pBtPolar->m_TWAMax);
        plrname += strong;
    }

    if(fabs(pBtPolar->phiMax()-pBtPolar->phiMin())>PRECISION)
    {
        strong = QString::asprintf("-phi(%.1f,%.1f)", pBtPolar->phiMin(), pBtPolar->phiMax());
        plrname += strong;
    }

    if(fabs(pBtPolar->RyMax()-pBtPolar->RyMin())>PRECISION)
    {
        strong = QString::asprintf("-Ry(%.1f,%.1f)", pBtPolar->RyMin(), pBtPolar->RyMax());
        plrname += strong;
    }

    // Angle Controls
    for(int is=0; is<int(pBtPolar->m_SailAngleMax.size()); is++)
    {
        if(fabs(pBtPolar->m_SailAngleMax.at(is)-pBtPolar->m_SailAngleMin.at(is))>PRECISION)
        {
            if(is<pBoat->nSails())
            {
                strong = QString::asprintf("(%.1f,%.1f)", pBtPolar->m_SailAngleMin.at(is), pBtPolar->m_SailAngleMax.at(is));
                plrname += "-"+QString::fromStdString(pBoat->sailAt(is)->name())+"_"+strong;
            }
        }
    }
    return plrname;
}

