/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <QDataStream>
#include <QVector>

#include <xfoil_params.h>


struct BLXFoil
{
    public:
        BLXFoil();

        int nd1;                    /**< the number of top side BL points  */
        int nd2;                    /**< the number of bot side BL points  */
        int nd3;                    /**< the number of wake side BL points */
        int nside1, nside2;

        double xd1[IVX];            /**< x-coordinate of the first part of the boundary layer */
        double yd1[IVX];            /**< y-coordinate of the first part of the boundary layer */
        double xd2[IVX];            /**< x-coordinate of the second part of the boundary layer */
        double yd2[IVX];            /**< y-coordinate of the second part of the boundary layer */
        double xd3[IWX];            /**< x-coordinate of the third part of the boundary layer */
        double yd3[IWX];            /**< y-coordinate of the third part of the boundary layer */


        double tklam;               /**< Karman-Tsien parameter minf^2 / [1 + sqrt[1-minf^2]]^2 */
        double qinf;                /**< freestream velocity, usually 1 */
        double dstr[IVX][ISX];      /**< bl displacement thickness array */
        double delt[IVX][ISX];      /**< the boundary layer thickness? */
        double thet[IVX][ISX];      /**< bl momentum thickness array */
        double tau[IVX][ISX];       /**< wall shear stress array                 [for plotting only] */
        double dis[IVX][ISX];       /**< dissipation array                       [for plotting only] */
        double ctau[IVX][ISX];      /**< sqrt[max shear coefficient] array */
        double ctq[IVX][ISX];       /**< sqrt[equilibrium max shear coefficient] array [  "  ] */
        double uedg[IVX][ISX];      /**< bl edge velocity array */
        double xbl[IVX][ISX];       /**< x-coordinate of bl variables */
        double Hk[IVX][ISX];        /**< Kinematic shape parameter */
        double RTheta[IVX][ISX];    /**< Momentum thickness Reynolds number */

        int itran[ISX];                  /**< bl array index of transition interval */

        void serialize(QDataStream &ar, bool bIsStoring);
};

