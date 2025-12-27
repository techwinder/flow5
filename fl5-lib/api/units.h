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

#include <QString>

#include <string>
#include <vector>

#include <fl5lib_global.h>

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

    extern std::vector<std::string> g_LengthUnitLabels;
    extern std::vector<std::string> g_MassUnitLabels;
    extern std::vector<std::string> g_AreaUnitLabels;
    extern std::vector<std::string> g_ForceUnitLabels;

    extern std::vector<std::string> g_SpeedUnitLabels;
    extern std::vector<std::string> g_MomentUnitLabels;
    extern std::vector<std::string> g_PressureUnitLabels;
    extern std::vector<std::string> g_InertiaUnitLabels;

    extern int g_FluidUnitType;//0= International, 1= Imperial
    extern std::vector<std::string> g_DensityUnitLabels;
    extern std::vector<std::string> g_ViscosityUnitLabels;

    FL5LIB_EXPORT std::string lengthUnitLabel(int idx=-1);
    FL5LIB_EXPORT std::string speedUnitLabel(int idx=-1);
    FL5LIB_EXPORT std::string massUnitLabel(int idx=-1);
    FL5LIB_EXPORT std::string areaUnitLabel(int idx=-1);
    FL5LIB_EXPORT std::string forceUnitLabel(int idx=-1);
    FL5LIB_EXPORT std::string momentUnitLabel(int idx=-1);
    FL5LIB_EXPORT std::string pressureUnitLabel(int idx=-1);
    FL5LIB_EXPORT std::string inertiaUnitLabel(int idx=-1);

    FL5LIB_EXPORT std::string densityUnitLabel();
    FL5LIB_EXPORT std::string viscosityUnitLabel();

    FL5LIB_EXPORT void setUnitConversionFactors();
    FL5LIB_EXPORT double toCustomUnit(int index);


    FL5LIB_EXPORT inline void getLengthUnitLabel(  std::string &label) {label = g_LengthUnitLabels[g_LengthUnitIndex];}
    FL5LIB_EXPORT inline void getSpeedUnitLabel(   std::string &label) {label = g_SpeedUnitLabels[g_SpeedUnitIndex];}
    FL5LIB_EXPORT inline void getMassUnitLabel(    std::string &label) {label = g_MassUnitLabels[g_MassUnitIndex];}
    FL5LIB_EXPORT inline void getAreaUnitLabel(    std::string &label) {label = g_AreaUnitLabels[g_AreaUnitIndex];}
    FL5LIB_EXPORT inline void getForceUnitLabel(   std::string &label) {label = g_ForceUnitLabels.at(g_ForceUnitIndex);}
    FL5LIB_EXPORT inline void getMomentUnitLabel(  std::string &label) {label = g_MomentUnitLabels[g_MomentUnitIndex];}
    FL5LIB_EXPORT inline void getPressureUnitLabel(std::string &label) {label = g_PressureUnitLabels[g_PressureUnitIndex];}
    FL5LIB_EXPORT inline void getInertiaUnitLabel( std::string &label) {label = g_PressureUnitLabels[g_PressureUnitIndex];}


    FL5LIB_EXPORT inline double mtoUnit()     {return g_mtoUnit;}
    FL5LIB_EXPORT inline double mstoUnit()    {return g_mstoUnit;}
    FL5LIB_EXPORT inline double m2toUnit()    {return g_m2toUnit;}
    FL5LIB_EXPORT inline double kgtoUnit()    {return g_kgtoUnit;}
    FL5LIB_EXPORT inline double NtoUnit()     {return g_NtoUnit;}
    FL5LIB_EXPORT inline double NmtoUnit()    {return g_NmtoUnit;}
    FL5LIB_EXPORT inline double PatoUnit()    {return g_PatoUnit;}
    FL5LIB_EXPORT inline double kgm2toUnit()  {return g_kgm2toUnit;}

    FL5LIB_EXPORT inline double densitytoUnit()   {return g_kgm3toUnit;}
    FL5LIB_EXPORT inline double viscositytoUnit() {return g_m2stoUnit;}

    FL5LIB_EXPORT inline int lengthUnitIndex()   {return g_LengthUnitIndex;}
    FL5LIB_EXPORT inline int areaUnitIndex()     {return g_AreaUnitIndex;}
    FL5LIB_EXPORT inline int weightUnitIndex()   {return g_MassUnitIndex;}
    FL5LIB_EXPORT inline int speedUnitIndex()    {return g_SpeedUnitIndex;}
    FL5LIB_EXPORT inline int forceUnitIndex()    {return g_ForceUnitIndex;}
    FL5LIB_EXPORT inline int momentUnitIndex()   {return g_MomentUnitIndex;}
    FL5LIB_EXPORT inline int pressureUnitIndex() {return g_PressureUnitIndex;}
    FL5LIB_EXPORT inline int inertiaUnitIndex()  {return g_InertiaUnitIndex;}

    FL5LIB_EXPORT inline void setLengthUnitIndex(int index)   {g_LengthUnitIndex    = index;}
    FL5LIB_EXPORT inline void setAreaUnitIndex(int index)     {g_AreaUnitIndex      = index;}
    FL5LIB_EXPORT inline void setWeightUnitIndex(int index)   {g_MassUnitIndex      = index;}
    FL5LIB_EXPORT inline void setSpeedUnitIndex(int index)    {g_SpeedUnitIndex     = index;}
    FL5LIB_EXPORT inline void setForceUnitIndex(int index)    {g_ForceUnitIndex     = index;}
    FL5LIB_EXPORT inline void setMomentUnitIndex(int index)   {g_MomentUnitIndex    = index;}
    FL5LIB_EXPORT inline void setPressureUnitIndex(int index) {g_PressureUnitIndex  = index;}
    FL5LIB_EXPORT inline void setInertiaUnitIndex(int index)  {g_InertiaUnitIndex   = index;}

    FL5LIB_EXPORT inline int fluidUnitType() {return g_FluidUnitType;}
    FL5LIB_EXPORT inline void setFluidUnitType(int type) {g_FluidUnitType=type;}


    // convenience conversion to QString
    inline QString lengthUnitQLabel(int idx=-1)    {return QString::fromStdString(Units::lengthUnitLabel(idx));}
    inline QString speedUnitQLabel(int idx=-1)     {return QString::fromStdString(Units::speedUnitLabel(idx));}
    inline QString massUnitQLabel(int idx=-1)      {return QString::fromStdString(Units::massUnitLabel(idx));}
    inline QString areaUnitQLabel(int idx=-1)      {return QString::fromStdString(Units::areaUnitLabel(idx));}
    inline QString forceUnitQLabel(int idx=-1)     {return QString::fromStdString(Units::forceUnitLabel(idx));}
    inline QString momentUnitQLabel(int idx=-1)    {return QString::fromStdString(Units::momentUnitLabel(idx));}
    inline QString pressureUnitQLabel(int idx=-1)  {return QString::fromStdString(Units::pressureUnitLabel(idx));}
    inline QString inertiaUnitQLabel(int idx=-1)   {return QString::fromStdString(Units::inertiaUnitLabel(idx));}

    inline QString densityUnitQLabel()    {return QString::fromStdString(Units::densityUnitLabel());}
    inline QString viscosityUnitQLabel()  {return QString::fromStdString(Units::viscosityUnitLabel());}

}

