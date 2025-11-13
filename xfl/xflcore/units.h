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


#pragma once

#include <QStringList>



namespace Units
{
        extern int g_LengthUnitIndex;    /**< The index of the custom unit in the array of length units. @todo use an enumeration instead. */
        extern int g_AreaUnitIndex;      /**< The index of the custom unit in the array of area units. */
        extern int g_MassUnitIndex;    /**< The index of the custom unit in the array of mass units. */
        extern int g_MomentUnitIndex;    /**< The index of the custom unit in the array of moment units. */
        extern int g_SpeedUnitIndex;     /**< The index of the custom unit in the array of speed units. */
        extern int g_ForceUnitIndex;     /**< The index of the custom unit in the array of force units. */
        extern int g_PressureUnitIndex;  /**< The index of the custom unit in the array of pressure units. */
        extern int g_InertiaUnitIndex;   /**< The index of the custom unit in the array of inertai units. */

        extern double g_mtoUnit;    /**< Conversion factor from meters to the custom length unit. */
        extern double g_mstoUnit;   /**< Conversion factor from m/s to the custom speed unit. */
        extern double g_m2toUnit;   /**< Conversion factor from square meters to the custom area unit. */
        extern double g_kgtoUnit;   /**< Conversion factor from kg to the custom mass unit. */
        extern double g_NtoUnit;    /**< Conversion factor from Newtons to the custom force unit. */
        extern double g_NmtoUnit;   /**< Conversion factor from N.m to the custom unit for moments. */
        extern double g_PatoUnit;   /**< Conversion factor from Pascal to the custom unit for pressures. */
        extern double g_kgm2toUnit; /**< Conversion factor from kg.m² to custom unit for inertias */

        extern double g_kgm3toUnit; /**< Conversion factor from kg.m³ to imperial unit for densities */
        extern double g_m2stoUnit;  /**< Conversion factor from m²/s to imperial unit for kinematic viscosities */

        extern QStringList g_LengthUnitLabels;
        extern QStringList g_MassUnitLabels;
        extern QStringList g_AreaUnitLabels;
        extern QStringList g_ForceUnitLabels;

        extern QStringList g_SpeedUnitLabels;
        extern QStringList g_MomentUnitLabels;
        extern QStringList g_PressureUnitLabels;
        extern QStringList g_InertiaUnitLabels;

        extern int g_FluidUnitType;//0= International, 1= Imperial
        extern QStringList g_DensityUnitLabels;
        extern QStringList g_ViscosityUnitLabels;

        QString lengthUnitLabel(int idx=-1);
        QString speedUnitLabel(int idx=-1);
        QString massUnitLabel(int idx=-1);
        QString areaUnitLabel(int idx=-1);
        QString forceUnitLabel(int idx=-1);
        QString momentUnitLabel(int idx=-1);
        QString pressureUnitLabel(int idx=-1);
        QString inertiaUnitLabel(int idx=-1);

        QString densityUnitLabel();
        QString viscosityUnitLabel();

        void setUnitConversionFactors();
        double toCustomUnit(int index);


        inline void getLengthUnitLabel(  QString &label) {label = g_LengthUnitLabels[g_LengthUnitIndex];}
        inline void getSpeedUnitLabel(   QString &label) {label = g_SpeedUnitLabels[g_SpeedUnitIndex];}
        inline void getMassUnitLabel(    QString &label) {label = g_MassUnitLabels[g_MassUnitIndex];}
        inline void getAreaUnitLabel(    QString &label) {label = g_AreaUnitLabels[g_AreaUnitIndex];}
        inline void getForceUnitLabel(   QString &label) {label = g_ForceUnitLabels.at(g_ForceUnitIndex);}
        inline void getMomentUnitLabel(  QString &label) {label = g_MomentUnitLabels[g_MomentUnitIndex];}
        inline void getPressureUnitLabel(QString &label) {label = g_PressureUnitLabels[g_PressureUnitIndex];}
        inline void getInertiaUnitLabel( QString &label) {label = g_PressureUnitLabels[g_PressureUnitIndex];}


        inline double mtoUnit()     {return g_mtoUnit;}
        inline double mstoUnit()    {return g_mstoUnit;}
        inline double m2toUnit()    {return g_m2toUnit;}
        inline double kgtoUnit()    {return g_kgtoUnit;}
        inline double NtoUnit()     {return g_NtoUnit;}
        inline double NmtoUnit()    {return g_NmtoUnit;}
        inline double PatoUnit()    {return g_PatoUnit;}
        inline double kgm2toUnit()  {return g_kgm2toUnit;}

        inline double densitytoUnit()   {return g_kgm3toUnit;}
        inline double viscositytoUnit() {return g_m2stoUnit;}

        inline int lengthUnitIndex()   {return g_LengthUnitIndex;}
        inline int areaUnitIndex()     {return g_AreaUnitIndex;}
        inline int weightUnitIndex()   {return g_MassUnitIndex;}
        inline int speedUnitIndex()    {return g_SpeedUnitIndex;}
        inline int forceUnitIndex()    {return g_ForceUnitIndex;}
        inline int momentUnitIndex()   {return g_MomentUnitIndex;}
        inline int pressureUnitIndex() {return g_PressureUnitIndex;}
        inline int inertiaUnitIndex()  {return g_InertiaUnitIndex;}

        inline void setLengthUnitIndex(int index)   {g_LengthUnitIndex    = index;}
        inline void setAreaUnitIndex(int index)     {g_AreaUnitIndex      = index;}
        inline void setWeightUnitIndex(int index)   {g_MassUnitIndex      = index;}
        inline void setSpeedUnitIndex(int index)    {g_SpeedUnitIndex     = index;}
        inline void setForceUnitIndex(int index)    {g_ForceUnitIndex     = index;}
        inline void setMomentUnitIndex(int index)   {g_MomentUnitIndex    = index;}
        inline void setPressureUnitIndex(int index) {g_PressureUnitIndex  = index;}
        inline void setInertiaUnitIndex(int index)  {g_InertiaUnitIndex   = index;}

        inline int fluidUnitType() {return g_FluidUnitType;}
        inline void setFluidUnitType(int type) {g_FluidUnitType=type;}

}


