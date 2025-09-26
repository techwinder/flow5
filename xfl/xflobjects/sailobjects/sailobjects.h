/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QVector>
#include <xflcore/linestyle.h>


class Boat;
class BoatPolar;
class BoatOpp;



namespace SailObjects
{
    extern QVector<Boat*> s_oaBoat;
    extern QVector<BoatPolar*> s_oaBtPolar;
    extern QVector<BoatOpp*> s_oaBtOpp;
    extern BoatOpp *s_pLastBtOpp;
    extern int s_SailDarkFactor;

    void deleteObjects();
    void deleteBoat(Boat *pBoat, bool bDeleteBtPolars);
    void deleteBtPolar(BoatPolar *pBtPolar);
    void deleteBtPolars(Boat *pBoat);
    void deleteBoatBtOpps(Boat *pBoat);
    void deleteBtPolarOpps(BoatPolar *pBtPolar);
    void deleteBoatResults(Boat *pBoat, bool bDeletePolars);
    void deleteAllBtOpps();

    inline Boat *boat(int idx)         {if(idx>=0 && idx<s_oaBoat.size())    return s_oaBoat.at(idx);    else return nullptr;}
    inline BoatPolar *btPolar(int idx) {if(idx>=0 && idx<s_oaBtPolar.size()) return s_oaBtPolar.at(idx); else return nullptr;}
    inline BoatOpp *btOpp(int idx)     {if(idx>=0 && idx<s_oaBtOpp.size())   return s_oaBtOpp.at(idx);   else return nullptr;}
    inline int boatCount()    {return s_oaBoat.size();}
    inline int btPolarCount()   {return s_oaBtPolar.size();}
    inline int btOppCount() {return s_oaBtOpp.size();}
    inline void setLastBtOpp(BoatOpp *pBtOpp) {s_pLastBtOpp=pBtOpp;}
    inline BoatOpp *lastBtOpp()   {return s_pLastBtOpp;}
    BoatOpp* getBoatOppObject(Boat*pBoat, BoatPolar *pBtPolar, double ctrl);
    void storeBtOpps(BoatPolar *pBtPolar, const QVector<BoatOpp *> &BtOppList);

    Boat*      boat(QString const &BoatName);
    BoatPolar* btPolar(const Boat *pBoat, const QString &BPolarName);
    BoatOpp *  getBoatOpp(const Boat *pBoat, const BoatPolar *pBPolar, double x);
    inline Boat* appendBoat(Boat*pBoat) {s_oaBoat.append(pBoat); return pBoat;}
    inline void appendBtPolar(BoatPolar *pBPolar) {s_oaBtPolar.append(pBPolar);}
    inline void appendBtOpp(BoatOpp *pBOpp) {s_oaBtOpp.append(pBOpp);}

    void insertThisBoat(Boat*pBoat);
    inline void insertThisBoat(int idx, Boat*pBoat) {s_oaBoat.insert(idx, pBoat);}
    void insertBtPolar(BoatPolar *pBtPolar);
    inline void insertBtPolar(int idx, BoatPolar *pBPolar) {s_oaBtPolar.insert(idx, pBPolar);}
    void insertBtOpp(BoatOpp *pBtOpp);

    bool boatExists(QString boatname);
    void setBoatStyle(Boat *pBoat, const LineStyle &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints);
    void setBPolarStyle(BoatPolar *pBPolar, LineStyle const &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints);

    void setWPolarColor(Boat *pBoat, BoatPolar *pBPolar);

    void setBoatVisible(Boat *pPlane, bool bVisible);
    void setBPolarVisible(BoatPolar *pWPolar, bool bVisible);

    bool hasResults(const Boat *pBoat);

    QStringList boatNames();
    QStringList polarNames(const Boat *pBoat);

}

