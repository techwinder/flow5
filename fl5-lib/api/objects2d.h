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

#pragma once

/**
  * @file This file implements the variables and methods used to manage 3D objects
  */

#include <vector>
#include <string>


#include <bldata.h>
#include <enums_objects.h>
#include <linestyle.h>
#include <polar.h>

class Foil;
class OpPoint;

namespace Objects2d
{
    extern FL5LIB_EXPORT std::vector <Foil*>    s_oaFoil;
    extern FL5LIB_EXPORT std::vector <Polar*>   s_oaPolar;  /**< The array of void pointers to the Polar objects. */
    extern FL5LIB_EXPORT std::vector <OpPoint*> s_oaOpp;    /**< The array of void pointers to the OpPoint objects. */


    FL5LIB_EXPORT  inline std::vector<Foil*>    const & foils()           {return s_oaFoil;}
    FL5LIB_EXPORT  inline std::vector<Polar*>   const & polars()          {return s_oaPolar;}
    FL5LIB_EXPORT  inline std::vector<OpPoint*> const & operatingPoints() {return s_oaOpp;}


    FL5LIB_EXPORT inline int nFoils()    {return int(s_oaFoil.size());}
    FL5LIB_EXPORT inline int nPolars()   {return int(s_oaPolar.size());}
    FL5LIB_EXPORT inline int nOpPoints() {return int(s_oaOpp.size());}

    FL5LIB_EXPORT void      deleteObjects();
    FL5LIB_EXPORT void      deleteFoilResults(Foil *pFoil, bool bDeletePolars=false);

    FL5LIB_EXPORT Foil*     foil(std::string const &name);
    FL5LIB_EXPORT Foil*     foil(int index);
    FL5LIB_EXPORT Foil const* foilAt(int index);
    FL5LIB_EXPORT Foil*     deleteFoil(const Foil *pFoil);
    FL5LIB_EXPORT void      insertThisFoil(Foil *pFoil);

    FL5LIB_EXPORT void      renameThisFoil(Foil *pFoil, const std::string &newFoilName);
    FL5LIB_EXPORT Foil*     setModFoil(Foil *pModFoil);


    FL5LIB_EXPORT bool      foilExists(const std::string &FoilName, Qt::CaseSensitivity cs=Qt::CaseInsensitive);

    FL5LIB_EXPORT std::vector<Foil*> sortedFoils();

    FL5LIB_EXPORT void insertPolar(Polar *pPolar);
    FL5LIB_EXPORT Polar *createPolar(Foil const*pFoil, xfl::enumPolarType PolarType, double Spec, double Mach, double NCrit, double XTop, double XBot);


    FL5LIB_EXPORT inline Polar* polarAt(int iPlr) {if(iPlr<0||iPlr>=nPolars()) return nullptr; else return s_oaPolar.at(iPlr);}
    FL5LIB_EXPORT Polar* polar(const std::string &foilname, const std::string &polarname);
    FL5LIB_EXPORT Polar* polar(const Foil *pFoil, const std::string &PolarName);
    FL5LIB_EXPORT Polar* polar(const Foil *pFoil, xfl::enumPolarType type, BL::enumBLMethod method, float Re);
    FL5LIB_EXPORT inline Polar* polar(int i) {if(i>=0 && i<nPolars()) return s_oaPolar.at(i); else return nullptr;}
    FL5LIB_EXPORT void deletePolar(Polar *pPolar);
    FL5LIB_EXPORT inline void removePolarAt(int ipl) {if(ipl>=0 && ipl<nPolars()) s_oaPolar.erase(s_oaPolar.begin()+ipl);}
    FL5LIB_EXPORT inline void appendPolar(Polar*pPolar) {s_oaPolar.push_back(pPolar);}

    FL5LIB_EXPORT inline OpPoint*  opPointAt(int iOpp) {return s_oaOpp.at(iOpp);}
    FL5LIB_EXPORT OpPoint*  opPointAt(Foil const*pFoil, Polar const *pPolar, double OppParam);
    FL5LIB_EXPORT OpPoint *insertOpPoint(OpPoint *pNewPoint);
    FL5LIB_EXPORT bool deleteOpp(OpPoint *pOpp);
    FL5LIB_EXPORT void addOpPoint(OpPoint *pNewOpp, bool bStoreOpp);
    FL5LIB_EXPORT inline void removeOpPointAt(int io) {if(io>=0 && io<nOpPoints()) s_oaOpp.erase(s_oaOpp.begin()+io);}
    FL5LIB_EXPORT inline void appendOpp(OpPoint *pOpp) {s_oaOpp.push_back(pOpp);}

    FL5LIB_EXPORT inline void appendFoil(Foil *pFoil) {s_oaFoil.push_back(pFoil);}

    FL5LIB_EXPORT double getZeroLiftAngle(Foil const*pFoil0, Foil const*pFoil1, double Re, double Tau);
    FL5LIB_EXPORT void   getStallAngles(Foil const*pFoilA, Foil const*pFoilB, double Re, double Tau, double &negative, double &positive);
    FL5LIB_EXPORT void   getLinearizedPolar(Foil const*pFoil0, Foil const*pFoil1, double Re, double Tau, double &Alpha0, double &Slope);

    FL5LIB_EXPORT double getPlrPointFromAlpha(Foil const*pFoil, double Re, double Alpha, Polar::enumPolarVariable PlrVar, bool &bOutRe, bool &bOutCl);
    FL5LIB_EXPORT double getPlrPointFromAlpha(Polar::enumPolarVariable var, Foil const*pFoil0, Foil const*pFoil1, double Re, double Alpha, double Tau, bool &bOutRe, bool &bOutCl);

    FL5LIB_EXPORT double getPlrPointFromCl(Foil const*pFoil, double Re, double Cl, Polar::enumPolarVariable PlrVar, bool &bOutRe, bool &bOutCl);

    FL5LIB_EXPORT double getCm0(Foil const*pFoil0, Foil const*pFoil1, double Re, double Tau, bool &bOutRe, bool &bError);

    FL5LIB_EXPORT void renamePolar(Polar *pPolar, const std::string &newplrname);

    FL5LIB_EXPORT void setFoilStyle(Foil *pFoil, LineStyle const &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints, bool bFlowDown=true, int DarkFactor=100);
    FL5LIB_EXPORT void setPolarStyle(Polar *pPolar, const LineStyle &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints, bool bFlowDown=true, int darkfactor=100);

    FL5LIB_EXPORT void setFoilVisible(Foil *pFoil, bool bVisible, bool bChildrenOnly);
    FL5LIB_EXPORT void setPolarVisible(Polar *pPolar, bool bVisible);

    FL5LIB_EXPORT void cancelTEFlapAngles();

    FL5LIB_EXPORT bool makeNacaFoil(Foil *pNacaFoil, int digits, int nPanels=100);
    bool makeNaca4(Foil *pFoil, int digits);
    bool makeNaca5(Foil *pFoil, int digits);
    void makeNacaThickness(Foil *pFoil, double t);

    FL5LIB_EXPORT std::vector<std::string> foilNames();
    FL5LIB_EXPORT std::vector<std::string> polarList(Foil const*pFoil, BL::enumBLMethod method=BL::NOBLMETHOD);
}




