/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include "polarnamemaker.h"
#include <xflfoil/objects2d/polar.h>
#include <xflcore/xflcore.h>
#include <xflgeom/geom_globals/geom_params.h>

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

    if(s_bType)
    {
        switch(pPolar->type())
        {
            case xfl::T1POLAR:
            {
                strong = "-T1";
                break;
            }
            case xfl::T2POLAR:
            {
                strong = "-T2";
                break;
            }
            case xfl::T3POLAR:
            {
                strong = "-T3";
                break;
            }
            case(xfl::T4POLAR):
            {
                strong = "-T4";
                break;
            }
            case(xfl::T6POLAR):
            {
                strong = "-T6";
                break;
            }
            default:
            {
                strong = "-Tx";
                break;
            }
        }
        plrname += strong;
    }

    if(s_bReynolds)
    {
        if(pPolar->isFixedaoaPolar())
            strong = "-" + ALPHACHAR + QString::asprintf("%.2f", pPolar->aoa()) + DEGCHAR;
        else if(pPolar->isType123())
            strong = QString::asprintf("-Re%.3f", pPolar->Reynolds()/1.0e6);
        else if(pPolar->isType6())
        {
            strong = "-" + ALPHACHAR + QString::asprintf("%.2f", pPolar->aoa()) + DEGCHAR;
            strong += QString::asprintf("-Re%.3f", pPolar->Reynolds()/1.0e6);
        }

        plrname += strong;
    }

    if(s_bMach)
    {
        strong = QString::asprintf("-Ma%.2f", pPolar->Mach());
        plrname += strong;
    }

    if(s_bNCrit && pPolar->isXFoil())
    {
        strong = QString::asprintf("-N%.1f", pPolar->NCrit());
        plrname += strong;
    }

    if(s_bXTrTop && pPolar->XTripTop()<1.0)
    {
        strong = QString::asprintf("-TopTr%.2f", pPolar->XTripTop());
        plrname += strong;
    }

    if(s_bXTrBot && pPolar->XTripBot()<1.0)
    {
        strong = QString::asprintf("-BotTr%.2f", pPolar->XTripBot());
        plrname += strong;
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
        plrname += "-" + THETACHAR + QString::asprintf("%g", pPolar->TEFlapAngle()) + DEGCHAR;
    }

    plrname.remove(0,1); //remove first '-' character

    return plrname;
}
