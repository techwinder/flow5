/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

/**
  * @file This file implements the variables and methods used to manage 3D objects
  */

#include <QStringList>
#include <QMap>

#include <xflcore/enums_objects.h>
#include <xflcore/linestyle.h>
#include <xflfoil/objects2d/bldata.h>
#include <xflfoil/objects2d/polar.h>

class Foil;
class OpPoint;

namespace Objects2d
{
    extern QVector<Foil*> s_oaFoil;
    extern QVector <Polar *> s_oaPolar;  /**< The array of void pointers to the Polar objects. */
    extern QVector <OpPoint *> s_oaOpp;    /**< The array of void pointers to the OpPoint objects. */

    extern Foil *s_pCurFoil;
    extern Polar *s_pCurPolar;
    extern OpPoint *s_pCurOpp;

    void      deleteObjects();
    void      deleteFoilResults(Foil *pFoil, bool bDeletePolars=false);
    Foil*     foil(QString const &strFoilName);
    Foil*     foil(int index);
    Foil const* foilAt(int index);
    Foil*     deleteFoil(const Foil *pFoil);
    void      insertThisFoil(Foil *pFoil);

    void      renameThisFoil(Foil *pFoil, QString const &newFoilName);
    Foil*     setModFoil(Foil *pModFoil);


    bool      foilExists(QString const &FoilName, Qt::CaseSensitivity cs=Qt::CaseInsensitive);

    QVector<Foil*> sortedFoils();

    void insertPolar(Polar *pPolar);
    Polar *createPolar(Foil const*pFoil, xfl::enumPolarType PolarType, double Spec, double Mach, double NCrit, double XTop, double XBot);


    inline Polar* polarAt(int iPlr) {if(iPlr<0||iPlr>=s_oaPolar.size()) return nullptr; else return s_oaPolar.at(iPlr);}
    Polar* polar(const QString &foilname, QString const &polarname);
    Polar* polar(const Foil *pFoil, const QString &PolarName);
    Polar* polar(const Foil *pFoil, xfl::enumPolarType type, BL::enumBLMethod method, float Re);
    inline Polar* polar(int i) {if(i>=0 && i<s_oaPolar.size()) return s_oaPolar.at(i); else return nullptr;}
    void deletePolar(Polar *pPolar);
    inline void removePolarAt(int ipl) {if(ipl>=0 && ipl<s_oaPolar.size()) s_oaPolar.removeAt(ipl);}
    inline void appendPolar(Polar*pPolar) {s_oaPolar.append(pPolar);}

    inline OpPoint*  opPointAt(int iOpp) {return s_oaOpp.at(iOpp);}
    OpPoint*  opPointAt(Foil const*pFoil, Polar const *pPolar, double OppParam);
    OpPoint *insertOpPoint(OpPoint *pNewPoint);
    bool deleteOpp(OpPoint *pOpp);
    OpPoint*  addOpPoint(Foil const *pFoil, Polar *pPolar, OpPoint *pNewOpp, bool bStoreOpp);
    inline void removeOpPointAt(int io) {if(io>=0 && io<s_oaOpp.size()) s_oaOpp.removeAt(io);}
    inline void appendOpp(OpPoint *pOpp) {s_oaOpp.append(pOpp);}

    inline void appendFoil(Foil *pFoil) {s_oaFoil.append(pFoil);}

    inline int nFoils() {return s_oaFoil.size();}
    inline int nPolars() {return s_oaPolar.count();}
    inline int nOpPoints() {return s_oaOpp.size();}

    double getZeroLiftAngle(Foil const*pFoil0, Foil const*pFoil1, double Re, double Tau);
    void   getStallAngles(Foil const*pFoilA, Foil const*pFoilB, double Re, double Tau, double &negative, double &positive);
    void   getLinearizedPolar(Foil const*pFoil0, Foil const*pFoil1, double Re, double Tau, double &Alpha0, double &Slope);

    double getPlrPointFromAlpha(Foil const*pFoil, double Re, double Alpha, Polar::enumPolarVariable PlrVar, bool &bOutRe, bool &bOutCl);
    double getPlrPointFromAlpha(Polar::enumPolarVariable var, Foil const*pFoil0, Foil const*pFoil1, double Re, double Alpha, double Tau, bool &bOutRe, bool &bOutCl);

    double getPlrPointFromCl(Foil const*pFoil, double Re, double Cl, Polar::enumPolarVariable PlrVar, bool &bOutRe, bool &bOutCl);

    double getCm0(Foil const*pFoil0, Foil const*pFoil1, double Re, double Tau, bool &bOutRe, bool &bError);

    void renamePolar(Polar *pPolar, QString const &newplrname);

    void setFoilStyle(Foil *pFoil, LineStyle const &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints, bool bFlowDown=true);
    void setPolarStyle(Polar *pPolar, const LineStyle &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints, bool bFlowDown=true);

    void setFoilVisible(Foil *pFoil, bool bVisible, bool bChildrenOnly);
    void setPolarVisible(Polar *pPolar, bool bVisible);

    void cancelTEFlapAngles();

    inline Foil*    curFoil()  {return s_pCurFoil;}
    inline Polar*   curPolar() {return s_pCurPolar;}
    inline OpPoint* curOpp()   {return s_pCurOpp;}

    inline void setCurFoil(Foil *pFoil) {s_pCurFoil=pFoil;}
    inline void setCurPolar(Polar *pPolar) {s_pCurPolar=pPolar;}
    inline void setCurOpp(OpPoint*pOpp) {s_pCurOpp=pOpp;}


    QStringList foilNames();
    QStringList polarList(Foil const*pFoil, BL::enumBLMethod method=BL::NOBLMETHOD);
}




