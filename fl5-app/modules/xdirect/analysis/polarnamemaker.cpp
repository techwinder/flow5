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

#

#include "polarnamemaker.h"


#include <api/geom_params.h>
#include <api/polar.h>
#include <api/utils.h>

bool PolarNameMaker::s_bType        = true;
bool PolarNameMaker::s_bBLMethod    = true;
bool PolarNameMaker::s_bReynolds    = true;
bool PolarNameMaker::s_bMach        = false;
bool PolarNameMaker::s_bNCrit       = true;
bool PolarNameMaker::s_bXTrTop      = false;
bool PolarNameMaker::s_bXTrBot      = false;


PolarNameMaker::PolarNameMaker(Polar *pPolar)
{
    m_pPolar = pPolar;
}


QString PolarNameMaker::makeName(Polar const*pPolar)
{
    QString plrname;
    Polar samplepolar;
    if(!pPolar) pPolar = &samplepolar;

    QString strong;

//    if(s_bType)
    {
        switch(pPolar->type())
        {
            case xfl::T1POLAR:
            {
                strong = "T1";
                break;
            }
            case xfl::T2POLAR:
            {
                strong = "T2";
                break;
            }
            case xfl::T3POLAR:
            {
                strong = "T3";
                break;
            }
            case(xfl::T4POLAR):
            {
                strong = "T4";
                break;
            }
            case(xfl::T6POLAR):
            {
                strong = "T6";
                break;
            }
            default:
            {
                strong = "Tx";
                break;
            }
        }
        plrname.append(strong);
    }

    if(s_bReynolds)
    {
        strong = "-" + ALPHAch;
        if(pPolar->isFixedaoaPolar())
        {
            strong.append(QString::asprintf("%.2f", pPolar->aoaSpec())+DEGch);
        }
        else if(pPolar->isType123())
            strong = QString::asprintf("-Re%.3f", pPolar->Reynolds()/1.0e6);
        else if(pPolar->isType6())
        {
            strong.append(QString::asprintf("%.2f",   pPolar->aoaSpec())+DEGch);
            strong.append(QString::asprintf("-Re%.3f", pPolar->Reynolds()/1.0e6));
        }

        plrname.append(strong);
    }

    if(s_bMach)
    {
        strong = QString::asprintf("-Ma%.2f", pPolar->Mach());
        plrname.append(strong);
    }

    if(s_bNCrit && pPolar->isXFoil())
    {
        strong = QString::asprintf("-N%.1f", pPolar->NCrit());
        plrname.append(strong);
    }

    if(s_bXTrTop && pPolar->XTripTop()<1.0)
    {
        strong = QString::asprintf("-TopTr%.2f", pPolar->XTripTop());
        plrname.append(strong);
    }

    if(s_bXTrBot && pPolar->XTripBot()<1.0)
    {
        strong = QString::asprintf("-BotTr%.2f", pPolar->XTripBot());
        plrname.append(strong);
    }

    if(s_bBLMethod)
    {
        switch (pPolar->BLMethod())
        {
            case BL::NOBLMETHOD:      plrname += "-?";      break;
            case BL::XFOIL:
            default: break;
        }
    }

    if(!pPolar->isType6() && fabs(pPolar->TEFlapAngle())>ANGLEPRECISION)
    {
        plrname.append("-"+THETAch);
        plrname.append(QString::asprintf("%.2f°", pPolar->TEFlapAngle()));
    }

//    plrname.remove(0,1); //remove first '-' character

    return plrname;
}
