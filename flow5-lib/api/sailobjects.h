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

#include <vector>
#include <linestyle.h>

#include <fl5lib_global.h>


class Boat;
class BoatPolar;
class BoatOpp;



namespace SailObjects
{
    extern FL5LIB_EXPORT std::vector<Boat*> s_oaBoat;
    extern FL5LIB_EXPORT std::vector<BoatPolar*> s_oaBtPolar;
    extern FL5LIB_EXPORT std::vector<BoatOpp*> s_oaBtOpp;
    extern FL5LIB_EXPORT BoatOpp *s_pLastBtOpp;
    extern FL5LIB_EXPORT int s_SailDarkFactor;


    FL5LIB_EXPORT inline int nBoats()    {return int(s_oaBoat.size());}
    FL5LIB_EXPORT inline int nBtPolars() {return int(s_oaBtPolar.size());}
    FL5LIB_EXPORT inline int nBtOpps()   {return int(s_oaBtOpp.size());}

    FL5LIB_EXPORT inline std::vector<Boat*>      const& boats() {return s_oaBoat;}
    FL5LIB_EXPORT inline std::vector<BoatPolar*> const& boatPolars() {return s_oaBtPolar;}
    FL5LIB_EXPORT inline std::vector<BoatOpp*>   const& boatOpps() {return s_oaBtOpp;}

    FL5LIB_EXPORT void deleteObjects();
    FL5LIB_EXPORT void deleteBoat(Boat *pBoat, bool bDeleteBtPolars);
    FL5LIB_EXPORT void deleteBtPolar(BoatPolar *pBtPolar);
    FL5LIB_EXPORT void deleteBtPolars(Boat *pBoat);
    FL5LIB_EXPORT void deleteBoatBtOpps(Boat *pBoat);
    FL5LIB_EXPORT void deleteBtPolarOpps(BoatPolar *pBtPolar);
    FL5LIB_EXPORT void deleteBoatResults(Boat *pBoat, bool bDeletePolars);
    FL5LIB_EXPORT void deleteAllBtOpps();

    FL5LIB_EXPORT void removeBoatAt(int idx);
    FL5LIB_EXPORT void removeBtPolarAt(int idx);
    FL5LIB_EXPORT void removeBtOppAt(int idx);

    FL5LIB_EXPORT inline Boat *boat(int idx)         {if(idx>=0 && idx<nBoats())    return s_oaBoat.at(idx);    else return nullptr;}
    FL5LIB_EXPORT inline BoatPolar *btPolar(int idx) {if(idx>=0 && idx<nBtPolars()) return s_oaBtPolar.at(idx); else return nullptr;}
    FL5LIB_EXPORT inline BoatOpp *btOpp(int idx)     {if(idx>=0 && idx<nBtOpps())   return s_oaBtOpp.at(idx);   else return nullptr;}

    FL5LIB_EXPORT BoatOpp *btOpp(Boat const *pBoat, BoatPolar const*pPolar, std::string const &oppname);

    FL5LIB_EXPORT inline void setLastBtOpp(BoatOpp *pBtOpp) {s_pLastBtOpp=pBtOpp;}
    FL5LIB_EXPORT inline BoatOpp *lastBtOpp()   {return s_pLastBtOpp;}
    FL5LIB_EXPORT BoatOpp* getBoatOppObject(Boat*pBoat, BoatPolar *pBtPolar, double ctrl);
    FL5LIB_EXPORT void storeBtOpps(BoatPolar *pBtPolar, const std::vector<BoatOpp *> &BtOppList);

    FL5LIB_EXPORT Boat*       boat(const std::string &BoatName);
    FL5LIB_EXPORT BoatPolar* btPolar(const Boat *pBoat, const std::string &BPolarName);
    FL5LIB_EXPORT BoatOpp *  getBoatOpp(const Boat *pBoat, const BoatPolar *pBPolar, double x);
    FL5LIB_EXPORT inline Boat* appendBoat(Boat*pBoat) {s_oaBoat.push_back(pBoat); return pBoat;}
    FL5LIB_EXPORT inline void appendBtPolar(BoatPolar *pBPolar) {s_oaBtPolar.push_back(pBPolar);}
    FL5LIB_EXPORT inline void appendBtOpp(BoatOpp *pBOpp) {s_oaBtOpp.push_back(pBOpp);}

    FL5LIB_EXPORT void insertThisBoat(Boat*pBoat);
    FL5LIB_EXPORT inline void insertThisBoat(int idx, Boat*pBoat) {s_oaBoat.insert(s_oaBoat.begin()+idx, pBoat);}
    FL5LIB_EXPORT void insertBtPolar(BoatPolar *pBtPolar);
    FL5LIB_EXPORT inline void insertBtPolar(int idx, BoatPolar *pBPolar) {s_oaBtPolar.insert(s_oaBtPolar.begin()+idx, pBPolar);}
    FL5LIB_EXPORT void insertBtOpp(BoatOpp *pBtOpp);

    FL5LIB_EXPORT bool boatExists(std::string const &boatname);
    FL5LIB_EXPORT void setBoatStyle(Boat *pBoat, const LineStyle &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints);
    FL5LIB_EXPORT void setBPolarStyle(BoatPolar *pBPolar, LineStyle const &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints);

    FL5LIB_EXPORT void setWPolarColor(Boat *pBoat, BoatPolar *pBPolar);

    FL5LIB_EXPORT void setBoatVisible(Boat *pPlane, bool bVisible);
    FL5LIB_EXPORT void setBPolarVisible(BoatPolar *pWPolar, bool bVisible);

    FL5LIB_EXPORT bool hasResults(const Boat *pBoat);

    FL5LIB_EXPORT std::vector<std::string> boatNames();
    FL5LIB_EXPORT std::vector<std::string> polarNames(const Boat *pBoat);

}

