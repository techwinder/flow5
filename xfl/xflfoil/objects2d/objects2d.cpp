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

#include <QApplication>
#include <QDebug>

#include "objects2d.h"

#include <xflmath/constants.h>
#include <xflcore/trace.h>
#include <xflcore/xflcore.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/polar.h>
#include <xflfoil/objects2d/oppoint.h>
#include <xflfoil/objects2d/polarnamemaker.h>
#include <xflgeom/geom_globals/geom_params.h>

QVector<Foil*> Objects2d::s_oaFoil;


QVector<Polar*>   Objects2d::s_oaPolar;
QVector<OpPoint*> Objects2d::s_oaOpp;


Foil *   Objects2d::s_pCurFoil(nullptr);
Polar*   Objects2d::s_pCurPolar(nullptr);
OpPoint* Objects2d::s_pCurOpp(nullptr);



void Objects2d::deleteObjects()
{
    for (int i=s_oaFoil.size()-1; i>=0; i--)
    {
        delete s_oaFoil.at(i);
    }
    s_oaFoil.clear();

    for (int i=s_oaPolar.size()-1; i>=0; i--)
    {
        Polar *pPolar = s_oaPolar.at(i);
        s_oaPolar.removeAt(i);
        delete pPolar;
    }
    for (int i=Objects2d::nOpPoints()-1; i>=0; i--)
    {
        OpPoint *pOpp = s_oaOpp.at(i);
        s_oaOpp.removeAt(i);
        delete pOpp;
    }
}


Foil* Objects2d::foil(const QString &strFoilName)
{
    for (int i=0; i<s_oaFoil.size(); i++)
        if(s_oaFoil.at(i)->name()==strFoilName)
            return s_oaFoil.at(i);
    return nullptr;
}


Polar* Objects2d::polar(QString const &foilname, QString const &polarname)
{
    if(!polarname.length()) return nullptr;
    for(int i=0; i<s_oaPolar.size(); i++)
    {
        if(foilname.compare(s_oaPolar.at(i)->foilName())==0)
        {
            if(polarname.compare(s_oaPolar.at(i)->name())==0)
                return s_oaPolar.at(i);
        }
    }
    return nullptr;
}


Foil* Objects2d::foil(int index)
{
    if(index<0 || index>=s_oaFoil.size()) return nullptr;
    return s_oaFoil.at(index);
}


Foil const* Objects2d::foilAt(int index)
{
    if(index<0 || index>=s_oaFoil.size()) return nullptr;
    return s_oaFoil.at(index);
}


void Objects2d::insertThisFoil(Foil *pFoil)
{
    if(!pFoil) return;

    QString oldFoilName = pFoil->name();

    //check if it's an overwrite
    for (int i=0; i<s_oaFoil.size(); i++)
    {
        Foil*pOldFoil = s_oaFoil.at(i);
        if(pOldFoil && pOldFoil->name()==oldFoilName && pOldFoil!=pFoil)
        {
            // delete the old foil and its children objects
            deleteFoil(pOldFoil);
            break;
        }
    }

    bool bInserted(false);
    for (int i=0; i<s_oaFoil.size(); i++)
    {
        if(pFoil->name().compare(s_oaFoil.at(i)->name(), Qt::CaseInsensitive)<0)
        {
            s_oaFoil.insert(i, pFoil);
            bInserted = true;
            break;
        }
    }
    if(!bInserted) s_oaFoil.append(pFoil);
}


OpPoint* Objects2d::addOpPoint(Foil const *pFoil, Polar *pPolar, OpPoint *pNewOpp, bool bStoreOpp)
{
    if(!pFoil || !pPolar) return nullptr;

    if(pNewOpp ==nullptr) return nullptr;

    pNewOpp->setTheStyle(pPolar->theStyle());
    pNewOpp->setFoilName(pFoil->name());
    pNewOpp->setPolarName(pPolar->name());


    // insert the OpPoint in the current Polar object
    if(pPolar->isFixedLiftPolar() || pPolar->isRubberChordPolar())
    {
        if(pNewOpp)
        {
            pPolar->addOpPointData(pNewOpp);
        }
    }
    else
    {
        pPolar->addOpPointData(pNewOpp);
    }

    //insert the OpPoint in the Operating points array
    if(bStoreOpp)
    {
        return Objects2d::insertOpPoint(pNewOpp);
    }
    else
    {
        delete pNewOpp;
        return nullptr;
    }
}

Foil * Objects2d::deleteFoil(Foil const *pFoil)
{
    if(!pFoil || !pFoil->name().length()) return nullptr;

    for (int j=s_oaOpp.size()-1; j>=0; j--)
    {
        OpPoint *pOpPoint = s_oaOpp.at(j);
        if(pOpPoint->foilName() == pFoil->name())
        {
            if(pOpPoint==s_pCurOpp) s_pCurOpp = nullptr;
            s_oaOpp.removeAt(j);
            delete pOpPoint;
        }
    }

    for (int j=s_oaPolar.size()-1; j>=0; j--)
    {
        Polar* pPolar = s_oaPolar.at(j);
        if(pPolar->foilName() == pFoil->name())
        {
            if(pPolar==s_pCurPolar) s_pCurPolar=nullptr;
            s_oaPolar.removeAt(j);
            delete pPolar;
        }
    }

    Foil *pNewCurFoil(nullptr);
    for (int i=0; i<s_oaFoil.size(); i++)
    {
        if(s_oaFoil.at(i)==pFoil)
            s_oaFoil.remove(i);
    }
    if(pFoil==s_pCurFoil) s_pCurFoil=nullptr;
    delete pFoil;

    if(!pNewCurFoil && s_oaFoil.size()>0)
    {
        pNewCurFoil = s_oaFoil.first();
    }
    return pNewCurFoil;
}


bool Objects2d::deleteOpp(OpPoint *pOpp)
{
    if(!pOpp) return false;

    for (int iOpp=0; iOpp<s_oaOpp.size(); iOpp++)
    {
        OpPoint* pOldOpp =s_oaOpp.at(iOpp);
        if (pOpp == pOldOpp)
        {
            s_oaOpp.removeAt(iOpp);
            delete pOpp;
            return true;
        }
    }
    return false;
}


void Objects2d::deletePolar(Polar *pPolar)
{
    if(!pPolar) return;

    // start by removing all OpPoints
    for (int l=s_oaOpp.size()-1; l>=0; l--)
    {
        OpPoint *pOpPoint = s_oaOpp.at(l);
        if (pOpPoint->polarName()  == pPolar->name() &&
            pOpPoint->foilName() == pPolar->foilName())
        {
            s_oaOpp.removeAt(l);
            delete pOpPoint;
        }
    }


    for (int iPolar=0; iPolar<s_oaPolar.size(); iPolar++)
    {
        Polar* pOldPolar =s_oaPolar.at(iPolar);
        if (pPolar == pOldPolar)
        {
            s_oaPolar.removeAt(iPolar);
            delete pOldPolar;
            break;
        }
    }
}


OpPoint* Objects2d::insertOpPoint(OpPoint *pNewPoint)
{
    if(!pNewPoint) return nullptr;

    Polar *pPolar = polar(pNewPoint->foilName(), pNewPoint->polarName());

    if(!pPolar)
    {
        delete pNewPoint;
        return nullptr;
    }

    // first add the OpPoint to the OpPoint Array for the current FoilName
    for (int i=0; i<s_oaOpp.size(); i++)
    {
        OpPoint const*pOpPoint = s_oaOpp.at(i);
        if (pNewPoint->foilName().compare(pOpPoint->foilName())<0)
        {
            //insert point
            s_oaOpp.insert(i, pNewPoint);
            return pNewPoint;
        }
        else if (pNewPoint->foilName() == pOpPoint->foilName() && pNewPoint->polarName()==pOpPoint->polarName())
        {
            if(pPolar->isControlPolar())
            {
                if(fabs(pNewPoint->theta()-pOpPoint->theta())<0.0005)
                {
                    //replace the existing point
                    pNewPoint->setTheStyle(pOpPoint->theStyle());

                    s_oaOpp.removeAt(i);
                    delete pOpPoint;
                    s_oaOpp.insert(i, pNewPoint);
                    return pNewPoint;
                }
                else if (pNewPoint->theta() < pOpPoint->theta())
                {
                    //insert point
                    s_oaOpp.insert(i, pNewPoint);
                    return pNewPoint;
                }
            }
            else if(pPolar->isFixedaoaPolar())
            {
                if(fabs(pNewPoint->Reynolds()-pOpPoint->Reynolds())<0.1)
                {
                    //replace the existing point
                    pNewPoint->setTheStyle(pOpPoint->theStyle());

                    s_oaOpp.removeAt(i);
                    delete pOpPoint;
                    s_oaOpp.insert(i, pNewPoint);
                    return pNewPoint;
                }
                else if (pNewPoint->Reynolds() < pOpPoint->Reynolds())
                {
                    //insert point
                    s_oaOpp.insert(i, pNewPoint);
                    return pNewPoint;
                }
            }
            else
            {
                if(fabs(pNewPoint->aoa() - pOpPoint->aoa())<0.0005)
                {
                    //replace the existing point
                    pNewPoint->setTheStyle(pOpPoint->theStyle());

                    s_oaOpp.removeAt(i);
                    delete pOpPoint;
                    s_oaOpp.insert(i, pNewPoint);
                    return pNewPoint;
                }
                else if (pNewPoint->aoa() < pOpPoint->aoa())
                {
                    //insert point
                    s_oaOpp.insert(i, pNewPoint);
                    return pNewPoint;
                }
            }

        }
    }

    s_oaOpp.append(pNewPoint);
    return pNewPoint;
}


void Objects2d::renamePolar(Polar *pPolar, const QString &newplrname)
{
    pPolar->setName(newplrname);
    for(int i=0; i<s_oaPolar.size(); i++)
    {
        if(s_oaPolar.at(i)==pPolar)
        {
            s_oaPolar.removeAt(i);
            insertPolar(pPolar);
        }
    }
}


void Objects2d::insertPolar(Polar *pPolar)
{
    if(!pPolar) return;

    bool bInserted = false;

    for (int ip=0; ip<s_oaPolar.size(); ip++)
    {
        Polar *pOldPlr = s_oaPolar.at(ip);
        if (pOldPlr->name().compare(pPolar->name())==0 &&
            pOldPlr->foilName().compare(pPolar->foilName())==0)
        {
            deletePolar(pOldPlr);
            s_oaPolar.insert(ip, pPolar);
            return;
        }
    }

    for (int j=0; j<s_oaPolar.size(); j++)
    {
        Polar const*pOldPlr = s_oaPolar.at(j);

        //first index is the parent foil name
        if (pPolar->foilName().compare(pOldPlr->foilName(), Qt::CaseInsensitive)<0)
        {
            s_oaPolar.insert(j, pPolar);
            bInserted = true;
            break;
        }
        else if (pPolar->foilName() == pOldPlr->foilName())
        {
            //next index is the BL method
            if(pPolar->BLMethod() < pOldPlr->BLMethod())
            {
                s_oaPolar.insert(j, pPolar);
                bInserted = true;
                break;
            }
            else if(pPolar->BLMethod() == pOldPlr->BLMethod())
            {
                //next index is the polar type
                if(pPolar->type() < pOldPlr->type())
                {
                    s_oaPolar.insert(j, pPolar);
                    bInserted = true;
                    break;
                }
                else if(pPolar->type() == pOldPlr->type())
                {
                    if (pPolar->isFixedaoaPolar())
                    {
                        //Type 4, sort by Alphas
                        if(pPolar->m_aoaSpec < pOldPlr->m_aoaSpec)
                        {
                            s_oaPolar.insert(j, pPolar);
                            bInserted = true;
                            break;
                        }
                    }
                    else
                    {
                        //sort by ReNbr
                        if(pPolar->Reynolds() < pOldPlr->Reynolds())
                        {
                            s_oaPolar.insert(j, pPolar);
                            bInserted = true;
                            break;
                        }
                        //sort by Name
    /*                    if(pPolar->name().compare(pOldPlr->name(), Qt::CaseInsensitive)<0)
                        {
                            s_oaPolar.insert(j, pPolar);
                            bInserted = true;
                            break;
                        }*/
                    }
                }
            }
        }
    }
    if(!bInserted)
    {
        s_oaPolar.append(pPolar);
    }
}


Polar* Objects2d::polar(const Foil *pFoil, xfl::enumPolarType type, BL::enumBLMethod method, float Re)
{
    for (int i=0; i<s_oaPolar.size(); i++)
    {
        Polar *pPolar =  s_oaPolar.at(i);
        if (    pPolar->foilName() == pFoil->name() &&
                pPolar->BLMethod() == method &&
                pPolar->type() == type &&
                fabs(pPolar->Reynolds()-Re)<1)
        {
            return pPolar;
        }
    }
    return nullptr;
}


Polar *Objects2d::polar(Foil const*pFoil, QString const &PolarName)
{
    if (!PolarName.length()) return nullptr;
    if (!pFoil)        return nullptr;

    for (int i=0; i<s_oaPolar.size(); i++)
    {
        Polar *pPolar =  s_oaPolar.at(i);
        if (pPolar->foilName() == pFoil->name() &&  pPolar->name() == PolarName)
        {
            return pPolar;
        }
    }
    return nullptr;
}


void Objects2d::deleteFoilResults(Foil *pFoil, bool bDeletePolars)
{
    for (int j=s_oaOpp.size()-1; j>=0; j--)
    {
        OpPoint *pOpPoint = s_oaOpp[j];
        if(pOpPoint->foilName() == pFoil->name())
        {
            s_oaOpp.removeAt(j);
            delete pOpPoint;
        }
    }

    for (int j=s_oaPolar.size()-1; j>=0; j--)
    {
        Polar *pPolar = s_oaPolar.at(j);
        if(pPolar->foilName() == pFoil->name())
        {
            if(bDeletePolars)
            {
                s_oaPolar.removeAt(j);
                delete pPolar;
            }
            else
            {
                pPolar->reset();
            }
        }
    }
}


/**
* Returns the value of an aero coefficient, interpolated on a polar mesh, and based on the value of the Reynolds Number and of the lift coefficient.
* Proceeds by identifiying the two polars surronding Re, then interpolating both with the value of Alpha,
* last by interpolating the requested variable between the values measured on the two polars.
*@param m_poaPolar the pointer to the array of polars.
*@param pFoil the pointer to the foil
*@param Re the Reynolds number .
*@param Cl the lift coefficient, used as the input parameter for interpolation.
*@param PlrVar the index of the variable to interpolate.
*@param  bError true if Cl is outside the min or max Cl of the polar mesh.
*@param  bOutRe true if Re is outside the min or max Reynolds number of the polar mesh.
*@return the interpolated value.
*/
double Objects2d::getPlrPointFromCl(Foil const*pFoil, double Re, double Cl, Polar::enumPolarVariable PlrVar, bool &bOutRe, bool &bOutCl)
{
    double Var1=0, Var2=0;
    double Clmin=0, Clmax=0;

    if(!pFoil)
    {
        bOutRe = true;
        bOutCl = true;
        return 0.000;
    }

    // make an array of relevant polars
    QVector<Polar const*>polars;

    for (int i=0; i<s_oaPolar.size(); i++)
    {
       Polar const *pPolar = s_oaPolar.at(i);
       if(pPolar->isFixedSpeedPolar() && pPolar->foilName().compare(pFoil->name())==0 && pPolar->hasPoints())
       {
            polars.append(pPolar);
       }
    }

    if(polars.size()==0)
    {
        // can't interpolate anything
        bOutRe = true;
        return 0;
    }
    else if(polars.size()==1)
    {
        //interpolate Cl on this polar
        bOutRe = true;
        return polars.first()->interpolateFromCl(Cl, PlrVar, bOutCl);
    }

    //more than one polar - interpolate between  - tough job

    //First Find the two polars with Reynolds number surrounding the specified Re
    Polar const * pPolar1 = nullptr;
    Polar const * pPolar2 = nullptr;

    //Type 1 Polars are sorted by crescending Re Number

    //if Re is less than that of the first polar, use this one
    for (int i=0; i< polars.size(); i++)
    {
        Polar const *pPolar = polars.at(i);

        if (Re < pPolar->Reynolds())
        {
            bOutRe = true;
            return pPolar->interpolateFromCl(Cl, PlrVar, bOutCl);
            break;
        }
        break;

    }

    // if not Find the two polars
    for (int i=0; i<polars.size(); i++)
    {
        Polar const *pPolar = polars.at(i);

        // we have found the first type 1 polar for this foil
        pPolar->getClLimits(Clmin, Clmax);
        if (pPolar->Reynolds() <= Re)
        {
            if(Clmin <= Cl && Cl <= Clmax)
            {
                pPolar1 = pPolar;
            }
        }
        else
        {
            if(Clmin <= Cl && Cl <= Clmax)
            {
                pPolar2 = pPolar;
                break;
            }
        }

    }

    if (!pPolar2)
    {
        //then Re is greater than that of any polar
        // so use the last polar and interpolate Cls on this polar
        bOutRe = true;
        if(!pPolar1)
        {
            bOutRe = true;
            bOutCl = true;
            return 0.000;
        }
        return pPolar1->interpolateFromCl(Cl, PlrVar, bOutCl);
    }
    else
    {
        // Re is between that of polars 1 and 2
        // so interpolate Cls for each
        if(!pPolar1)
        {
            bOutRe = true;
            bOutCl = true;
            return 0.000;
        }

        if(!pPolar1->m_Cl.size())
        {
            bOutRe = true;
            bOutCl = true;
            return 0.000;
        }
        Var1 = pPolar1->interpolateFromCl(Cl, PlrVar, bOutCl);

        if(!pPolar2->m_Cl.size())
        {
            bOutRe = true;
            bOutCl = true;
            return 0.000;
        }
        Var2 = pPolar2->interpolateFromCl(Cl, PlrVar, bOutCl);

        // then interpolate Variable

        double v =   (Re - pPolar1->Reynolds()) / (pPolar2->Reynolds() - pPolar1->Reynolds());
        double Var = Var1 + v * (Var2-Var1);
        return Var;
    }
}


double Objects2d::getPlrPointFromAlpha(Polar::enumPolarVariable var, Foil const*pFoil0, Foil const*pFoil1,
                                       double Re, double Alpha, double Tau, bool &bOutRe, bool &bOutCl)
{
    if(!pFoil0 || !pFoil1)
    {
        bOutRe = true;
        bOutCl = true;
        return 0.0;
    }

    bool IsOutRe = false;
    bool IsOutCl  = false;
    bOutRe = false;
    bOutCl = false;

    double var0 = getPlrPointFromAlpha(pFoil0, Re, Alpha, var, IsOutRe, IsOutCl);
    if(IsOutRe) bOutRe = true;
    if(IsOutCl) bOutCl = true;
    double var1 = getPlrPointFromAlpha(pFoil1, Re, Alpha, var, IsOutRe, IsOutCl);
    if(IsOutRe) bOutRe = true;
    if(IsOutCl) bOutCl = true;

    if (Tau<0.0) Tau = 0.0;
    if (Tau>1.0) Tau = 1.0;
    return ((1-Tau) * var0 + Tau * var1);
}


/**
* Returns the value of an aero coefficient, interpolated on a polar mesh, and based on the value of the Reynolds Number and of the aoa.
* Proceeds by identifiying the two polars surronding Re, then interpolating both with the value of Alpha,
* last by interpolating the requested variable between the values measured on the two polars.
*@param pFoil the pointer to the foil
*@param Re the Reynolds number .
*@param Alpha the angle of attack.
*@param PlrVar the index of the variable to interpolate.
*@param bOutRe true if Cl is outside the min or max Cl of the polar mesh.
*@param bError if Re is outside the min or max Reynolds number of the polar mesh.
*@return the interpolated value.
*/
double Objects2d::getPlrPointFromAlpha(Foil const*pFoil, double Re, double Alpha, Polar::enumPolarVariable PlrVar, bool &bOutRe, bool &bOutAlpha)
{
    double Var1=0, Var2=0;

    bOutRe = false;

    if(!pFoil)
    {
        bOutRe = true;
        bOutAlpha = true;
        return 0.000;
    }

    QVector<Polar const*>polars;

    for (int i=0; i<s_oaPolar.size(); i++)
    {
        Polar const *pPolar = polarAt(i);
        if(pPolar->isFixedSpeedPolar() && pPolar->foilName().compare(pFoil->name())==0)
        {
            polars.append(pPolar);
        }
    }
    if(!polars.size())
    {
        bOutRe=true;
        return 0.0;
    }

    //more than one polar - interpolate between
    //First Find the two polars with Reynolds number surrounding wanted Re
    Polar const *pPolar1 = nullptr;
    Polar const *pPolar2 = nullptr;

    //Type 1 Polars are sorted by crescending Re Number

    //if Re is less than that of the first polar, use this one
    for (int i=0; i<polars.size(); i++)
    {
        Polar const *pPolar = polars.at(i);
        if(pPolar->m_Alpha.size()>0)
        {
            // we have found the first type 1 polar for this foil
            if (Re<pPolar->Reynolds())
            {
                bOutRe = true;
                return pPolar->interpolateFromAlpha(Alpha, PlrVar, bOutAlpha);
                break;
            }
            break;
        }
    }

    // if not find the two polars surrounding the Re number
    for (int i=0; i<polars.size(); i++)
    {
        Polar const *pPolar = polars.at(i);
        if(pPolar->m_Alpha.size()>0)
        {
            if (pPolar->Reynolds() <= Re)
            {
                pPolar1 = pPolar;
            }
            else
            {
                pPolar2 = pPolar;
                break;
            }
        }
    }

    if (!pPolar2)
    {
        //then Re is greater than that of any polar
        // so use last polar and interpolate alphas on this polar
        bOutRe = true;
        if(!pPolar1 || !pPolar1->m_Alpha.size())
        {
            bOutAlpha = true;
            return 0.000;
        }

        return pPolar1->interpolateFromAlpha(Alpha, PlrVar, bOutAlpha);
    }
    else
    {
        // Re is between that of polars 1 and 2
        // so interpolate alphas for each

        if(!pPolar1 || !pPolar1->m_Alpha.size())
        {
            bOutRe = true;
            bOutAlpha = true;
            return 0.000;
        }
        Var1 = pPolar1->interpolateFromAlpha(Alpha, PlrVar, bOutAlpha);


        if(!pPolar2->m_Alpha.size())
        {
            bOutRe = true;
            bOutAlpha = true;
            return 0.000;
        }

        Var2 = pPolar2->interpolateFromAlpha(Alpha, PlrVar, bOutAlpha);
        // then interpolate Variable

        double v = (Re - pPolar1->Reynolds()) / (pPolar2->Reynolds() - pPolar1->Reynolds());
        double Var = Var1 + v * (Var2-Var1);
        return Var;
    }

    Q_ASSERT(false);
    bOutRe = true;
    bOutAlpha = true;
    return 0.000;// we missed something somewhere...
}


/**
 * Returns the coefficient of an approximate linearized Cl=f(aoa) curve, based on the geometrical position of a point between two sections on a wing.
 * @param pFoil0 the pointer to the left foil  of the wing's section.
 * @param pFoil1 the pointer to the left foil  of the wing's section.
 * @param Re the Reynolds number at the point's position.
 * @param Tau the relative position of the point between the two foils.
 * @param Alpha0 the zero-lift angle; if the interpolation fails, returns Alpha0 = 0
 * @param Slope the slope of the lift curve; if the interpolation fails, returns Slope = 2 PI
 */
void Objects2d::getLinearizedPolar(Foil const*pFoil0, Foil const*pFoil1, double Re, double Tau, double &Alpha0, double &Slope)
{
    double Alpha00=0, Alpha01(0);
    double Slope0(0), Slope1(0);
    double AlphaTemp1(0), AlphaTemp2(0), SlopeTemp1(0), SlopeTemp2(0);

    //Find the two polars which enclose the Reynolds number
    int size = 0;
    Polar const *pPolar(nullptr), *pPolar1(nullptr), *pPolar2(nullptr);

    if(!pFoil0)
    {
        Q_ASSERT(false);
    }
    else
    {
        pPolar1 = nullptr;
        pPolar2 = nullptr;
        for (int i=0; i<nPolars(); i++)
        {
            pPolar = polarAt(i);
            if(pPolar->foilName() == pFoil0->name()) size++;
        }
        if(size)
        {
            for (int i=0; i<nPolars(); i++)
            {
                pPolar = polarAt(i);
                if(pPolar->foilName() == pFoil0->name())
                {
                    if(pPolar->Reynolds() < Re) pPolar1 = pPolar;
                }
            }
            for (int i=0; i<nPolars(); i++)
            {
                pPolar = polarAt(i);
                if(pPolar->foilName() == pFoil0->name())
                {
                    if(pPolar->Reynolds() > Re)
                    {
                        pPolar2 = pPolar;
                        break;
                    }
                }
            }
        }
        if(pPolar1 && pPolar2)
        {
            pPolar1->getLinearizedCl(AlphaTemp1, SlopeTemp1);
            pPolar2->getLinearizedCl(AlphaTemp2, SlopeTemp2);
            Alpha00 = AlphaTemp1 +
                    (AlphaTemp2-AlphaTemp1) * (Re-pPolar1->Reynolds())/(pPolar2->Reynolds()-pPolar1->Reynolds());
            Slope0  = SlopeTemp1 +
                    (SlopeTemp2-SlopeTemp1) * (Re-pPolar1->Reynolds())/(pPolar2->Reynolds()-pPolar1->Reynolds());
        }
        else
        {
            Alpha00 = 0.0;
            Slope0  = 2.0 * PI;
        }
    }

    if(!pFoil1)
    {
        Alpha01 = 0.0;
        Slope1 = 2.0*PI ;
    }
    else
    {
        pPolar1 = nullptr;
        pPolar2 = nullptr;
        for (int i=0; i<nPolars(); i++)
        {
            pPolar = polarAt(i);
            if(pPolar->foilName() == pFoil1->name()) size++;
        }
        if(size)
        {
            for (int i=0; i<nPolars(); i++)
            {
                pPolar = polarAt(i);
                if(pPolar->foilName() == pFoil1->name())
                {
                    if(pPolar->Reynolds() < Re) pPolar1 = pPolar;
                }
            }
            for (int i=0; i<nPolars(); i++)
            {
                pPolar = polarAt(i);
                if(pPolar->foilName() == pFoil1->name())
                {
                    if(pPolar->Reynolds() > Re)
                    {
                        pPolar2 = pPolar;
                        break;
                    }
                }
            }
        }
        if(pPolar1 && pPolar2)
        {
            pPolar1->getLinearizedCl(AlphaTemp1, SlopeTemp1);
            pPolar2->getLinearizedCl(AlphaTemp2, SlopeTemp2);
            Alpha01 = AlphaTemp1 + (AlphaTemp2-AlphaTemp1) * (Re-pPolar1->Reynolds())/(pPolar2->Reynolds()-pPolar1->Reynolds());
            Slope1  = SlopeTemp1 + (SlopeTemp2-SlopeTemp1) * (Re-pPolar1->Reynolds())/(pPolar2->Reynolds()-pPolar1->Reynolds());
        }
        else
        {
            Alpha01 = 0.0;
            Slope1 = 2.0*PI;
        }
    }

    Alpha0 = ((1-Tau) * Alpha00 + Tau * Alpha01);
    Slope  = ((1-Tau) * Slope0  + Tau * Slope1);
}


/**
* Interpolates the zero-lift angle on the polar mesh, based on the geometrical position of a point between two sections on a wing.
* @param pFoil0 the pointer to the left foil  of the wing's section.
* @param pFoil1 the pointer to the left foil  of the wing's section.
* @param Re the Reynolds number at the point's position.
* @param Tau the relative position of the point between the two foils.
* @return the interpolated value.
*/
double Objects2d::getZeroLiftAngle(Foil const *pFoil0, Foil const*pFoil1, double Re, double Tau)
{
    //returns the 0-lift angle of the foil, at Reynolds=Re
    //if the polar doesn't reach to 0-lift, returns Alpha0 = 0;
    double a01(0), a02(0);
    double Alpha00(0), Alpha01(0);

    //Find the two polars which enclose Reynolds
    bool bPolar = false;
    Polar const *pPolar1(nullptr);
    Polar const *pPolar2(nullptr);

    if(!pFoil0) Alpha00 = 0.0;
    else
    {
        pPolar1 = nullptr;
        pPolar2 = nullptr;
        bPolar = false;
        for (int i=0; i<nPolars(); i++)
        {
            Polar const *pPolar = polarAt(i);
            if(pPolar->foilName() == pFoil0->name()) {bPolar=true; break;}
        }
        if(bPolar)
        {
            for (int i=0; i<nPolars(); i++)
            {
                Polar const *pPolar = polarAt(i);
                if(pPolar->foilName() == pFoil0->name())
                {
                    if(pPolar->Reynolds() < Re) pPolar1 = pPolar;
                }
            }
            for (int i=0; i<nPolars(); i++)
            {
                Polar const *pPolar = polarAt(i);
                if(pPolar->foilName() == pFoil0->name())
                {
                    if(pPolar->Reynolds() > Re)
                    {
                        pPolar2 = pPolar;
                        break;
                    }
                }
            }
        }
        if(pPolar1 && pPolar2)
        {
            a01 = pPolar1->getZeroLiftAngle();
            a02 = pPolar2->getZeroLiftAngle();
            Alpha00 = a01 + (a02-a01) * (Re-pPolar1->Reynolds())/(pPolar2->Reynolds()-pPolar1->Reynolds());
        }
        else if(pPolar1)
        {
            Alpha00 = pPolar1->getZeroLiftAngle();
        }
        else if(pPolar2)
        {
            Alpha00 = pPolar2->getZeroLiftAngle();
        }
        else Alpha00 = 0.0;
    }

    if(!pFoil1) Alpha01 = 0.0;
    else
    {
        pPolar1 = nullptr;
        pPolar2 = nullptr;
        bPolar = false;
        for (int i=0; i<nPolars(); i++)
        {
            Polar const *pPolar = polarAt(i);
            if(pPolar->foilName() == pFoil1->name())  {bPolar=true; break;}
        }
        if(bPolar)
        {
            for (int i=0; i<nPolars(); i++)
            {
                Polar const *pPolar = polarAt(i);
                if(pPolar->foilName() == pFoil1->name())
                {
                    if(pPolar->Reynolds() < Re) pPolar1 = pPolar;
                }
            }
            for (int i=0; i<nPolars(); i++)
            {
                Polar const *pPolar = polarAt(i);
                if(pPolar->foilName() == pFoil1->name())
                {
                    if(pPolar->Reynolds() > Re)
                    {
                        pPolar2 = pPolar;
                        break;
                    }
                }
            }
        }
        if(pPolar1 && pPolar2)
        {
            a01 = pPolar1->getZeroLiftAngle();
            a02 = pPolar2->getZeroLiftAngle();
            Alpha01 = a01 + (a02-a01) * (Re-pPolar1->Reynolds())/(pPolar2->Reynolds()-pPolar1->Reynolds());
        }
        else if(pPolar1)
        {
            Alpha01 = pPolar1->getZeroLiftAngle();
        }
        else if(pPolar2)
        {
            Alpha01 = pPolar2->getZeroLiftAngle();
        }
        else Alpha01 = 0.0;
    }

    return (1-Tau) * Alpha00 + Tau * Alpha01;
}


/**
* Interpolates the positive and negative stall angles on the polar mesh,
* based on the geometrical position of a point between two sections on a wing.
* @param pFoil0 the pointer to the left foil  of the wing's section.
* @param pFoil1 the pointer to the left foil  of the wing's section.
* @param Re the Reynolds number at the point's position.
* @param Tau the relative position of the point between the two foils.
* @return the interpolated value.
*/
void Objects2d::getStallAngles(Foil const *pFoilA, Foil const *pFoilB, double Re, double Tau, double &negative, double &positive)
{
    double posA=0, negA=0;
    double posB=0, negB=0;

    //Find the two polars which enclose Reynolds
    int size = 0;
    Polar const *pPolar1=nullptr;
    Polar const *pPolar2=nullptr;

    if(!pFoilA) {posA = negA = 0.0;}
    else
    {
        pPolar1 = nullptr;
        pPolar2 = nullptr;
        for (int i=0; i<nPolars(); i++)
        {
            Polar const *pPolar = polarAt(i);
            if(pPolar->foilName() == pFoilA->name()) size++;
        }
        if(size)
        {
            for (int i=0; i<nPolars(); i++)
            {
                Polar const *pPolar = polarAt(i);
                if(pPolar->foilName() == pFoilA->name())
                {
                    if(pPolar->Reynolds() < Re) pPolar1 = pPolar;
                }
            }
            for (int i=0; i<nPolars(); i++)
            {
                Polar const*pPolar = polarAt(i);
                if(pPolar->foilName() == pFoilA->name())
                {
                    if(pPolar->Reynolds() > Re)
                    {
                        pPolar2 = pPolar;
                        break;
                    }
                }
            }
        }
        if(pPolar1 && pPolar2)
        {
            double pos1=0, pos2=0;
            double neg1=0, neg2=0;
            pPolar1->getStallAngles(neg1, pos1);
            pPolar2->getStallAngles(neg2, pos2);
            double frac = (Re-pPolar1->Reynolds())/(pPolar2->Reynolds()-pPolar1->Reynolds());
            posA = pos1*(1-frac) + pos2*frac;
            negA = neg1*(1-frac) + neg2*frac;
        }
        else {posA = negA = 0.0;}
    }

    if(!pFoilB) {posB = negB = 0.0;}
    else
    {
        pPolar1 = nullptr;
        pPolar2 = nullptr;
        for (int i=0; i<nPolars(); i++)
        {
            Polar const *pPolar = polarAt(i);
            if(pPolar->foilName() == pFoilB->name()) size++;
        }
        if(size)
        {
            for (int i=0; i<nPolars(); i++)
            {
                Polar const *pPolar = polarAt(i);
                if(pPolar->foilName() == pFoilB->name())
                {
                    if(pPolar->Reynolds() < Re) pPolar1 = pPolar;
                }
            }
            for (int i=0; i<nPolars(); i++)
            {
                Polar const *pPolar = polarAt(i);
                if(pPolar->foilName() == pFoilB->name())
                {
                    if(pPolar->Reynolds() > Re)
                    {
                        pPolar2 = pPolar;
                        break;
                    }
                }
            }
        }
        if(pPolar1 && pPolar2)
        {
            double pos1=0, pos2=0;
            double neg1=0, neg2=0;
            pPolar1->getStallAngles(neg1, pos1);
            pPolar2->getStallAngles(neg2, pos2);
            double frac = (Re-pPolar1->Reynolds())/(pPolar2->Reynolds()-pPolar1->Reynolds());
            posB = pos1*(1-frac) + pos2*frac;
            negB = neg1*(1-frac) + neg2*frac;
        }
        else {posB = negB = 0.0;}
    }

    negative = (1-Tau) * negA + Tau * negB;
    positive = (1-Tau) * posA + Tau * posB;
}


double Objects2d::getCm0(const Foil *pFoil0, const Foil *pFoil1, double Re, double Tau, bool &bOutRe, bool &bError)
{
    //Find 0-lift angle for local foil
    double Alpha(0);
    double Cm0(0), Cm1(0);
    double Cl0(0), Cl1(0);

    bOutRe = false;
    bError = false;
    bool IsOutRe = false;
    bool IsError = false;

    bOutRe = false;
    for (int i=-10; i<10; i++)
    {
        Alpha = double(i);
        Cl1 = getPlrPointFromAlpha(Polar::CL, pFoil0, pFoil1, Re, Alpha, Tau, IsOutRe, IsError);
        if(Cl1>0.0)
        {
            if(IsOutRe) bOutRe = true;
            if(IsError) bError = true;
            break;
        }
        Cl0 = Cl1;
    }
    if(Cl0>0.0)
    {
        return 0.0;
    }
    Cm0 = getPlrPointFromAlpha(Polar::CM, pFoil0, pFoil1, Re, Alpha-1.0, Tau, IsOutRe, IsError);
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;
    Cm1 = getPlrPointFromAlpha(Polar::CM, pFoil0, pFoil1, Re, Alpha, Tau, IsOutRe, IsError);
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;

    double Res = Cm0 + (Cm1-Cm0)*(0.0-Cl0)/(Cl1-Cl0);

    return Res;
}


OpPoint *Objects2d::opPointAt(Foil const*pFoil, Polar const*pPolar, double OppParam)
{
    OpPoint* pOpPoint;
    if(!pPolar) return nullptr;

    for (int i=0; i<s_oaOpp.size(); i++)
    {
        if(!pPolar) return nullptr;
        pOpPoint = s_oaOpp.at(i);
        //since alphas are calculated at 1/100th
        if (pOpPoint->foilName() == pFoil->name())
        {
            if (pOpPoint->polarName() == pPolar->name())
            {
                if(pPolar->isControlPolar())
                {
                    if(fabs(pOpPoint->theta() - OppParam) <CTRLPRECISION)
                    {
                        return pOpPoint;
                    }
                }
                else if(pPolar->isFixedaoaPolar())
                {
                    if(fabs(pOpPoint->Reynolds() - OppParam) <REYNOLDSPRECISION)
                    {
                        return pOpPoint;
                    }
                }
                else
                {
                    if(fabs(pOpPoint->aoa() - OppParam) <AOAPRECISION)
                    {
                        return pOpPoint;
                    }
                }
            }
        }
    }
    return nullptr;// if no OpPoint has a matching alpha
}


void Objects2d::setPolarStyle(Polar *pPolar, LineStyle const &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints, bool bFlowDown)
{
    if(!pPolar) return;
    pPolar->setTheStyle(ls);

    if(!bFlowDown) return;

    OpPoint *pLastPOpp = nullptr;
    Foil const*pFoil = foil(pPolar->foilName());
    if(!pFoil) return;

    for(int ipp=s_oaOpp.size()-1; ipp>=0; ipp--)
    {
        OpPoint *pOpp = s_oaOpp.at(ipp);
        if(pOpp->foilName().compare(pFoil->name())==0 && pOpp->polarName().compare(pPolar->name())==0)
        {
            if(bStyle) pOpp->setLineStipple(ls.m_Stipple);
            if(bWidth) pOpp->setLineWidth(ls.m_Width);
            if(bColor)
            {
                if(!pLastPOpp) pOpp->setLineColor(ls.m_Color);
                else           pOpp->setLineColor(pLastPOpp->lineColor().darker(xfl::darkFactor()));
            }
            if(bPoints) pOpp->setPointStyle(ls.m_Symbol);

            pLastPOpp = pOpp;
        }
    }
}


void Objects2d::setFoilStyle(Foil *pFoil, const LineStyle &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints, bool bFlowDown)
{
    if(!pFoil) return;
    pFoil->setTheStyle(ls);

    if(!bFlowDown) return;

    Polar *pLastPolar = nullptr;
    OpPoint *pLastOpp = nullptr;
    for(int iwp=0; iwp<s_oaPolar.size(); iwp++)
    {
        Polar *pPolar = s_oaPolar.at(iwp);
        if(pPolar->foilName().compare(pFoil->name())==0)
        {
            if(bStyle) pPolar->setLineStipple(ls.m_Stipple);
            if(bWidth) pPolar->setLineWidth(ls.m_Width);
            if(bColor)
            {
                if(!pLastPolar) pPolar->setLineColor(ls.m_Color);
                else            pPolar->setLineColor(pLastPolar->lineColor().darker(xfl::darkFactor()));
            }
            if(bPoints) pPolar->setPointStyle(ls.m_Symbol);

            pLastPolar = pPolar;
        }
    }
    for(int ipp=0; ipp<s_oaOpp.size(); ipp++)
    {
        OpPoint *pOpp = s_oaOpp.at(ipp);
        if(pOpp->foilName().compare(pFoil->name())==0)
        {
            if(bStyle)pOpp->setLineStipple(ls.m_Stipple);
            if(bWidth)pOpp->setLineWidth(ls.m_Width);
            if(bColor)
            {
                if(!pLastOpp) pOpp->setLineColor(ls.m_Color);
                else          pOpp->setLineColor(pLastOpp->lineColor().darker(xfl::darkFactor()));
            }
            if(bPoints) pOpp->setPointStyle(ls.m_Symbol);

            pLastOpp = pOpp;
        }
    }
}


void Objects2d::setFoilVisible(Foil *pFoil, bool bVisible, bool bChildrenOnly)
{
    if(!pFoil) return;

    if(!bChildrenOnly) pFoil->setVisible(bVisible);

    for(int iwp=0; iwp<s_oaPolar.size(); iwp++)
    {
        Polar *pPolar = s_oaPolar.at(iwp);
        if(pPolar->foilName().compare(pFoil->name())==0)
        {
            pPolar->setVisible(bVisible);
        }
    }
    for(int ipp=0; ipp<s_oaOpp.size(); ipp++)
    {
        OpPoint *pPOpp = s_oaOpp.at(ipp);
        if(pPOpp->foilName().compare(pFoil->name())==0)
        {
            pPOpp->setVisible(bVisible);
        }
    }
}


void Objects2d::setPolarVisible(Polar *pPolar, bool bVisible)
{
    if(!pPolar) return;
    pPolar->setVisible(bVisible);
    Foil *pFoil = foil(pPolar->foilName());
    if(!pFoil) return;

    for(int ipp=0; ipp<s_oaOpp.size(); ipp++)
    {
        OpPoint *pOpp = s_oaOpp.at(ipp);
        if(pOpp->foilName().compare(pFoil->name())==0)
        {
            if(pOpp->polarName().compare(pPolar->name())==0)
                pOpp->setVisible(bVisible);
        }
    }
}


QVector<Foil*> Objects2d::sortedFoils()
{
    QVector<Foil *> foils;
    for(int i=0; i<nFoils(); i++)
    {
        Foil *pFoil = foil(i);
        bool bInserted = false;
        for(int j=0; j<foils.size(); j++)
        {
            if(QString::compare(pFoil->name(), foils.at(j)->name(), Qt::CaseInsensitive)<0)
            {
                foils.insert(j, pFoil);
                bInserted=true;
                break;
            }
        }
        if(!bInserted)
            foils.push_back(pFoil);
    }
    Q_ASSERT(foils.size()==nFoils());
    return foils;
}


Polar *Objects2d::createPolar(Foil const*pFoil, xfl::enumPolarType PolarType, double Spec, double Mach, double NCrit, double XTop, double XBot)
{
    if(!pFoil) return nullptr;

    Polar *pNewPolar = new Polar;

    pNewPolar->setTheStyle(pFoil->theStyle());
    pNewPolar->setFoilName(pFoil->name());
    pNewPolar->setVisible(true);
    pNewPolar->setType(PolarType);

    if(PolarType!=xfl::T4POLAR)
    {
        pNewPolar->setReynolds(Spec);
    }
    else
    {
        pNewPolar->setAoa(Spec);
    }
    pNewPolar->setMach(Mach);
    pNewPolar->setNCrit(NCrit);
    pNewPolar->setXTripTop(XTop);
    pNewPolar->setXTripBot(XBot);

    pNewPolar->setName(PolarNameMaker::makeName(pNewPolar));

    Polar *pOldPolar = Objects2d::polar(pFoil, pNewPolar->name());

    if(pOldPolar)
    {
        delete pNewPolar;
        pNewPolar = pOldPolar;
    }
    else Objects2d::insertPolar(pNewPolar);
    return pNewPolar;
}


bool Objects2d::foilExists(QString const &FoilName, Qt::CaseSensitivity cs)
{
    for (int i=0; i<s_oaFoil.size(); i++)
    {
        Foil *pFoil = s_oaFoil.at(i);
        if(pFoil->name().compare(FoilName, cs)==0) return true;
    }

    return false;
}


QStringList Objects2d::foilNames()
{
    QStringList list;
    for (int i=0; i<s_oaFoil.size(); i++)
        list.append(s_oaFoil.at(i)->name());
    return list;
}


QStringList Objects2d::polarList(Foil const*pFoil, BL::enumBLMethod method)
{
    QStringList names;
    if(pFoil)
    {
        for(int i=0; i<nPolars(); i++)
        {
            Polar const *pPolar = s_oaPolar.at(i);
            if(pPolar->foilName()==pFoil->name())
            {
                 if(method==BL::NOBLMETHOD || pPolar->BLMethod()==method)
                names.append(pPolar->name());
            }
        }
        // v7.50:  cancel TE flap angles which have been moved to the polar
    }
    return names;
}


void Objects2d::renameThisFoil(Foil *pFoil, const QString &newFoilName)
{
    QString oldFoilName = pFoil->name();

    int pos = -1;
    for (int i=0; i<s_oaFoil.size(); i++)
    {
        if(s_oaFoil.at(i)==pFoil)
        {
            pos = i;
            break;
        }
    }

    if(pos<0) return; // error somewhere

    //rename it
    pFoil->setName(newFoilName);

    // remove an re-insert in alphabetical order
    s_oaFoil.removeAt(pos);
    bool bInserted = false;
    for(int i=0; i<s_oaFoil.size(); i++)
    {
        if(pFoil->name().compare(s_oaFoil.at(i)->name(), Qt::CaseInsensitive)<0)
        {
            s_oaFoil.insert(i, pFoil);
            bInserted = true;
            break;
        }
    }
    if(!bInserted) s_oaFoil.append(pFoil);

    //rename its children objects
    for (int iPolar=0; iPolar<s_oaPolar.size(); iPolar++)
    {
        Polar* pOldPolar = s_oaPolar.at(iPolar);
        if(pOldPolar->foilName() == oldFoilName)
        {
            pOldPolar->setFoilName(newFoilName);
        }
    }

    for (int iOpp=0; iOpp<s_oaOpp.size(); iOpp++)
    {
        OpPoint *pOpPoint = s_oaOpp.at(iOpp);
        if(pOpPoint->foilName() == oldFoilName)
        {
            pOpPoint->setFoilName(newFoilName);
        }
    }
}


void Objects2d::cancelTEFlapAngles()
{
    // v7.50:  cancel TE flap angles which have been moved to the polar

    for(int ifoil=0; ifoil<s_oaFoil.size(); ifoil++)
    {
        Foil *pFoil = s_oaFoil.at(ifoil);
        if(pFoil->hasTEFlap() && fabs(pFoil->TEFlapAngle())>FLAPANGLEPRECISION)
        {
            pFoil->setTEFlapAngle(0.0);
            pFoil->setFlaps();
        }
    }
}







