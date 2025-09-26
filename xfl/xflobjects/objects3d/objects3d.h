/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

/**
  * @file This file implements the variables and methods used to manage 3d objects
  */


#pragma once

#include <QList>

#include <xflcore/linestyle.h>

class Plane;
class PlaneSTL;
class PlaneXfl;
class Part;
class WingXfl;
class WPolar;
class PlaneOpp;


namespace Objects3d
{
    // object variable lists
    extern int s_Index;
    extern QList<Plane*> s_oaPlane;   /**< The array of void pointers to the Plane objects. */
    extern QList<WPolar*> s_oaWPolar;  /**< The array of void pointers to the WPolar objects. */
    extern QList<PlaneOpp*> s_oaPOpp;    /**< The array of void pointers to the PlaneOpp objects. */

    Plane* addPlane(Plane *pPlane);
    Plane* setModifiedPlane(Plane *pModPlane);
    void deleteObjects();
    void deletePlane(Plane *pPlane, bool bDeleteResults=true);
    void deletePlaneResults(const Plane *pPlane, bool bDeletePolars=false);
    void deleteExternalPolars(Plane const*pPlane);
    void deleteWPolarResults(WPolar *pWPolar);
    void deleteWPolar(WPolar *pWPolar);
    void deletePlaneOpp(PlaneOpp *pPOpp);
    Plane* plane(const QString &PlaneName);
    Plane const* planeAt(const QString &PlaneName);
    PlaneOpp* getPlaneOpp(Plane const *pPlane, WPolar const*pWPolar, QString const &oppname);
    WPolar* wPolar(const Plane *pPlane, const QString &WPolarName);
    bool planeExists(const QString &planeName);

    inline void appendPlane(Plane*pPlane) {s_oaPlane.push_back(pPlane);}
    inline void insertPlane(int k, Plane*pPlane) {s_oaPlane.insert(k, pPlane);}
    inline void removePlaneAt(int idx) {if(idx<0 ||idx>=s_oaPlane.size()) return; s_oaPlane.removeAt(idx);}
    void renamePlane(QString const &PlaneName);
    void insertPlane(Plane *pModPlane);

    void addWPolar(WPolar *pWPolar);
    void appendWPolar(WPolar *pWPolar);
    void insertWPolar(WPolar *pNewWPolar);
    WPolar*   insertNewWPolar(WPolar *pNewWPolar, const Plane *pCurPlane);
    inline void removeWPolarAt(int idx) {if(idx<0 ||idx>=s_oaWPolar.size()) return; s_oaWPolar.removeAt(idx);}

    void insertPOpp(PlaneOpp *pPOpp);
    inline bool containsPOpp(PlaneOpp *pPOpp) {return s_oaPOpp.contains(pPOpp);}
    inline void removePOppAt(int idx) {if(idx<0 ||idx>=s_oaPOpp.size()) return; s_oaPOpp.removeAt(idx);}
    inline void appendPOpp(PlaneOpp *pPOpp) {s_oaPOpp.push_back(pPOpp);}

    inline int nPlanes()  {return s_oaPlane.size();}
    inline int nPolars()  {return s_oaWPolar.size();}
    inline int nPOpps()   {return s_oaPOpp.size();}

    inline Plane* planeAt(int ip)    {if(ip>=0 && ip<s_oaPlane.size())  return s_oaPlane.at(ip);  else return nullptr;}
    inline WPolar* wPolarAt(int iw)  {if(iw>=0 && iw<s_oaWPolar.size()) return s_oaWPolar.at(iw); else return nullptr;}
    inline PlaneOpp* POppAt(int io)  {if(io>=0 && io<s_oaPOpp.size())   return s_oaPOpp.at(io);   else return nullptr;}

    int  newUniquePartIndex();

    void setWPolarColor(const Plane *pPlane, WPolar *pWPolar);

    void setPlaneVisible(const Plane *pPlane, bool bVisible, bool bStabilityPolarsOnly);
    void setWPolarVisible(WPolar *pWPolar, bool bVisible);

    void setPlaneStyle(Plane *pPlane, LineStyle const &ls, bool bStipple, bool bWidth, bool bColor, bool bPoints);
    void setWPolarStyle(WPolar *pWPolar, LineStyle const &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints);
    void setWPolarPOppStyle(WPolar const* pWPolar, bool bStipple, bool bWidth, bool bColor, bool bPoints);

    bool hasResults(Plane const*pPlane);
    bool hasPOpps(Plane const *pPlane);
    bool hasPOpps(WPolar const *pWPolar);


    void updateFoilName(const QString &oldName, const QString &newName);

    void renameWPolar(WPolar *pWPolar, const Plane *pPlane);
    QStringList planeNames();
    QStringList polarNames(Plane const*pPlane);


    void cleanObjects(QString &log);
    void updateWPolarstoV750();

}




