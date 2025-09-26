/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QVector>

#include <xflgeom/geom2d/node2d.h>



namespace BL
{
    typedef enum {XFOIL, NOBLMETHOD} enumBLMethod;
    typedef enum {TOP, BOTTOM, WAKE, NOSIDE} enumBLSide;
}


struct BLData
{
    BLData()
    {
        reset();
    }

    void reset();

    void listBL() const;

    /** Returns the size of the node array */
    int nNodes() const {return s.size();}

    /** Returns the viscous pressure coefficient at node n */
    double Cpv(int nx) const
    {
        if(nx<0 || nx>=Qv.size()) return 0.0;
        return 1.0-Qv.at(nx)*Qv.at(nx)/QInf/QInf;
    }

    /** Returns the inviscid pressure coefficient at node n */
    double Cpi(int nx) const
    {
        if(nx<0 || nx>=Qi.size()) return 0.0;
        return 1.0-Qi.at(nx)*Qi.at(nx)/QInf/QInf;
    }

    /** Returns the mass defect at station nx */
    double massDefect(int nx) const
    {
        if(nx<0 || nx>=Qv.size()) return 0.0;
        return Qv.at(nx)*dstar.at(nx);
    }

    /** Returns the source strength associated with the mass defects at station nx and nx+1*/
    double sigma(int np) const
    {
        if(np<0 || np>=nNodes()-1) return 0.0;
        if(np==0) return 0.0; // first panel is messed up by enforcement of stagnation point
        double ds = s[np+1]-s[np];
        return (massDefect(np+1)-massDefect(np))/ds;
    }

    /** Returns the length of the surface, wake length not included */
    double length() const
    {
        if(nTE>0 && nTE<nNodes()) return s[nTE];
        return 1.0; // need to return something
    }

    bool bTop()           const {return Side==BL::TOP;}
    bool bBottom()        const {return Side==BL::BOTTOM;}
    bool bWake()          const {return Side==BL::WAKE;}
    void setSide(BL::enumBLSide side) {Side = side;}

    bool isTurbulent(int nx) const {return s.at(nx)>=XTr;}
    bool isSeparated(int nx) const {return (s.at(nx)>=XLamSep || s.at(nx)>=XTurbSep);}

    /**<Resizes the arrays */
    void resizeData(int N, bool bResultsOnly);
    void serializeFl5(QDataStream &ar, bool bIsStoring);


    BL::enumBLMethod BLMethod;

    BL::enumBLSide Side;                  /**< true if is top side BL data, false if is bottom side */
    bool bIsConverged;          /**< true if the BL calculation was successfull, false otherwise */

    double QInf;                /**< the freestream velocity */
    double Cd_SY;                /**< Squire-Young drag */
    double XTr;                 /**< The transition point from laminar to turbulent in this boundary layer */
    double XLamSep;             /**< The laminar separation point in this boundary layer */
    double XTurbSep;            /**< The turbulent separation point in this boundary layer */
    double CL;                  /**< The lift coefficient CL = Sum(F.n)/(1/2 rho V²) */
    double Cm;                  /**< The moment coefficient Cm = Sum(F.n)/(1/2 rho V² chord) */
    double XCP;                 /**< The position of the center of pressure = Sum(lever_arm F.n)/Sum(F.n) */
    int iLE;                    /**< The index of the first node in the foil's array of nodes */
    int nTE;                    /**< The index of the trailing edge node in the s[]  array */

    QVector<double> Qi;         /**< the distribution of stream velocity for an inviscid analysis */
    QVector<double> Qv;         /**< the distribution of BL edge velocities; initialized with Qi and updated in downstream marching order */

    QVector<double> CTau;       /**< Shear stress coefficient */
    QVector<double> CTq;        /**< Equilibrium shear stress coefficient */
    QVector<double> Cd;         /**< Dissipation coefficient */
    QVector<double> Cf;         /**< Skin friction coefficient */
    QVector<double> tauw;       /**< Wall shear; added to compare to XFoil's results */
    QVector<double> H;          /**< Shape parameter (= H12 = dstar/theta) */
    QVector<double> HStar;      /**< Kinetic energy shape parameter; used only in integral method (= H32 = theta/delta3) */
    QVector<double> delta3;     /**< Energy thickness; used exclusively in Eppler's model */
    QVector<double> dstar;      /**  <Displacement thickness (=delta1) */
    QVector<double> nTS;        /**< Amplification parameter oF TS waves */
    QVector<double> s;          /**< Spline coordinates */
    QVector<double> theta;      /**< Momentum thickness (=delta2) */
    QVector<double> delta;      /**< Boundary layer thickness */
    QVector<double> gamtr;      /**< The intermittency factor from transition point to fully turbulent flow */

    QVector<bool> bConverged;   /**< true if the station is converged, false otherwise */
    QVector<bool> bTurbulent;   /**< true if the flow is turbulent at the station; equivalent to s>xtr, not stored */
    QVector<Node2d> node;       /**< the array of BL nodes, aligned on the x-axis */
    QVector<Node2d> foilnode;   /**< the array of BL nodes on the foil's surface; used for BL display*/
};

