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

#include <QDataStream>

#include <node2d.h>



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
    int nNodes() const {return int(s.size());}

    /** Returns the viscous pressure coefficient at node n */
    double Cpv(int nx) const
    {
        if(nx<0 || nx>=int(Qv.size())) return 0.0;
        return 1.0-Qv.at(nx)*Qv.at(nx)/QInf/QInf;
    }

    /** Returns the inviscid pressure coefficient at node n */
    double Cpi(int nx) const
    {
        if(nx<0 || nx>=int(Qi.size())) return 0.0;
        return 1.0-Qi.at(nx)*Qi.at(nx)/QInf/QInf;
    }

    /** Returns the mass defect at station nx */
    double massDefect(int nx) const
    {
        if(nx<0 || nx>=int(Qv.size())) return 0.0;
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

    std::vector<double> Qi;         /**< the distribution of stream velocity for an inviscid analysis */
    std::vector<double> Qv;         /**< the distribution of BL edge velocities; initialized with Qi and updated in downstream marching order */

    std::vector<double> CTau;       /**< Shear stress coefficient */
    std::vector<double> CTq;        /**< Equilibrium shear stress coefficient */
    std::vector<double> Cd;         /**< Dissipation coefficient */
    std::vector<double> Cf;         /**< Skin friction coefficient */
    std::vector<double> tauw;       /**< Wall shear; added to compare to XFoil's results */
    std::vector<double> H;          /**< Shape parameter (= H12 = dstar/theta) */
    std::vector<double> HStar;      /**< Kinetic energy shape parameter; used only in integral method (= H32 = theta/delta3) */
    std::vector<double> delta3;     /**< Energy thickness; used exclusively in Eppler's model */
    std::vector<double> dstar;      /**  <Displacement thickness (=delta1) */
    std::vector<double> nTS;        /**< Amplification parameter oF TS waves */
    std::vector<double> s;          /**< Spline coordinates */
    std::vector<double> theta;      /**< Momentum thickness (=delta2) */
    std::vector<double> delta;      /**< Boundary layer thickness */
    std::vector<double> gamtr;      /**< The intermittency factor from transition point to fully turbulent flow */

    std::vector<bool> bConverged;   /**< true if the station is converged, false otherwise */
    std::vector<bool> bTurbulent;   /**< true if the flow is turbulent at the station; equivalent to s>xtr, not stored */
    std::vector<Node2d> node;       /**< the array of BL nodes, aligned on the x-axis */
    std::vector<Node2d> foilnode;   /**< the array of BL nodes on the foil's surface; used for BL display*/
};

