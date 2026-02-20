/****************************************************************************

                flow5 application
                Copyright © 2025 André Deperrois
                
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

/**
  * @file This file implements the variables and methods used to manage 3d objects
  */


#pragma once

#include <vector>

#include <linestyle.h>
#include <fl5lib_global.h>

class Plane;
class PlaneSTL;
class PlaneXfl;
class Part;
class WingXfl;
class PlanePolar;
class PlaneOpp;


namespace Objects3d
{
    // object variable lists
    extern FL5LIB_EXPORT int s_Index;
    extern FL5LIB_EXPORT std::vector<Plane*> s_oaPlane;   /**< The array of void pointers to the Plane objects. */
    extern FL5LIB_EXPORT std::vector<PlanePolar*> s_oaPlanePolar;  /**< The array of void pointers to the WPolar objects. */
    extern FL5LIB_EXPORT std::vector<PlaneOpp*> s_oaPlaneOpp;    /**< The array of void pointers to the PlaneOpp objects. */

    FL5LIB_EXPORT  inline std::vector<Plane*>      const &planes()      {return s_oaPlane;}
    FL5LIB_EXPORT  inline std::vector<PlanePolar*> const &planePolars() {return s_oaPlanePolar;}
    FL5LIB_EXPORT  inline std::vector<PlaneOpp*>   const &planeOpps()   {return s_oaPlaneOpp;}

    FL5LIB_EXPORT  Plane* addPlane(Plane *pPlane);
    FL5LIB_EXPORT  void deleteObjects();
    FL5LIB_EXPORT  void deletePlane(Plane *pPlane, bool bDeleteResults=true);
    FL5LIB_EXPORT  void deletePlaneResults(const Plane *pPlane, bool bDeletePolars=false);
    FL5LIB_EXPORT  void deleteExternalPolars(Plane const*pPlane);
    FL5LIB_EXPORT  void deleteWPolarResults(PlanePolar *pWPolar);
    FL5LIB_EXPORT  void deletePlPolar(PlanePolar *pPlPolar);
    FL5LIB_EXPORT  void deletePlaneOpp(PlaneOpp *pPOpp);
    FL5LIB_EXPORT  Plane* plane(const std::string &PlaneName);
    FL5LIB_EXPORT  Plane const* planeAt(const std::string &PlaneName);
    FL5LIB_EXPORT  PlaneOpp* planeOpp(Plane const *pPlane, PlanePolar const*pWPolar, const std::string &oppname);
    FL5LIB_EXPORT  PlanePolar* wPolar(const Plane *pPlane, const std::string &WPolarName);
    FL5LIB_EXPORT  bool planeExists(const std::string &planeName);

    FL5LIB_EXPORT  inline void appendPlane(Plane*pPlane) {s_oaPlane.push_back(pPlane);}
    FL5LIB_EXPORT  inline void insertPlane(int k, Plane*pPlane) {s_oaPlane.insert(s_oaPlane.begin()+k, pPlane);}
    FL5LIB_EXPORT  inline void removePlaneAt(int idx) {if(idx<0 ||idx>=int(s_oaPlane.size())) return; s_oaPlane.erase(s_oaPlane.begin()+idx);}
    FL5LIB_EXPORT  void insertPlane(Plane *pModPlane);
    FL5LIB_EXPORT  void renamePlane(Plane *pPlane, std::string const &newname);

    FL5LIB_EXPORT  void addPlPolar(PlanePolar *pPPolar);
    FL5LIB_EXPORT  void appendPlPolar(PlanePolar *pPPolar);
    FL5LIB_EXPORT  void insertPlPolar(PlanePolar *pNewPPolar);
    FL5LIB_EXPORT  inline void removePlPolarAt(int idx) {if(idx<0 ||idx>=int(s_oaPlanePolar.size())) return; s_oaPlanePolar.erase(s_oaPlanePolar.begin()+idx);}
    FL5LIB_EXPORT  void renamePlPolar(PlanePolar *pWPolar, std::string const &newname);

    FL5LIB_EXPORT  void insertPlaneOpp(PlaneOpp *pPOpp);
    FL5LIB_EXPORT  bool containsPOpp(PlaneOpp *pPOpp);
    FL5LIB_EXPORT  inline void removePOppAt(int idx) {if(idx<0 ||idx>=int(s_oaPlaneOpp.size())) return; s_oaPlaneOpp.erase(s_oaPlaneOpp.begin()+idx);}
    FL5LIB_EXPORT  inline void appendPOpp(PlaneOpp *pPOpp) {s_oaPlaneOpp.push_back(pPOpp);}

    FL5LIB_EXPORT  inline int nPlanes()  {return int(s_oaPlane.size());}
    FL5LIB_EXPORT  inline int nPolars()  {return int(s_oaPlanePolar.size());}
    FL5LIB_EXPORT  inline int nPOpps()   {return int(s_oaPlaneOpp.size());}

    FL5LIB_EXPORT  inline Plane* planeAt(int ip)    {if(ip>=0 && ip<int(s_oaPlane.size()))  return s_oaPlane.at(ip);  else return nullptr;}
    FL5LIB_EXPORT  inline PlanePolar* plPolarAt(int iw)  {if(iw>=0 && iw<int(s_oaPlanePolar.size())) return s_oaPlanePolar.at(iw); else return nullptr;}
    FL5LIB_EXPORT  inline PlaneOpp* POppAt(int io)  {if(io>=0 && io<int(s_oaPlaneOpp.size()))   return s_oaPlaneOpp.at(io);   else return nullptr;}

    FL5LIB_EXPORT  int  newUniquePartIndex();

    FL5LIB_EXPORT  void setPlPolarColor(const Plane *pPlane, PlanePolar *pWPolar, int darkfactor);

    FL5LIB_EXPORT  void setPlaneVisible(const Plane *pPlane, bool bVisible, bool bStabilityPolarsOnly);
    FL5LIB_EXPORT  void setPlPolarVisible(PlanePolar *pWPolar, bool bVisible);

    FL5LIB_EXPORT  void setPlaneStyle(Plane *pPlane, LineStyle const &ls, bool bStipple, bool bWidth, bool bColor, bool bPoints, int darkfactor);
    FL5LIB_EXPORT  void setPlPolarStyle(PlanePolar *pWPolar, LineStyle const &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints, int darkfactor);
    FL5LIB_EXPORT  void setPlPolarPOppStyle(PlanePolar const* pWPolar, bool bStipple, bool bWidth, bool bColor, bool bPoints, int darkfactor);

    FL5LIB_EXPORT  bool hasResults(Plane const*pPlane);
    FL5LIB_EXPORT  bool hasPOpps(Plane const *pPlane);
    FL5LIB_EXPORT  bool hasPOpps(PlanePolar const *pWPolar);


    FL5LIB_EXPORT  void updateFoilName(const std::string &oldName, const std::string &newName);


    FL5LIB_EXPORT  void cleanObjects(std::string &log);
    FL5LIB_EXPORT  void updatePlPolarstoV750();

    FL5LIB_EXPORT PlaneOpp *storePlaneOpps(QList<PlaneOpp*> const &POppList);
    FL5LIB_EXPORT PlaneOpp *storePlaneOpps(std::vector<PlaneOpp*> const &POppList);

}




