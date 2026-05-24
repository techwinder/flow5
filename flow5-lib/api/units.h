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

#include <format>


#include <string>
#include <vector>

#include <fl5lib_global.h>

namespace Units
{
    typedef enum {MM, CM, DM, M, IN, FT}            enumLengthUnit;
    typedef enum {MS, KMH, FTS, KT, MPH}            enumSpeedUnit;
    typedef enum {MM2, CM2, DM2, M2, IN2, FT2}      enumAreaUnit;
    typedef enum {G, KG, OZ, LB}                    enumMassUnit;
    typedef enum {N, KN, TON, LBF}                  enumForceUnit;
    typedef enum {NM, LBFIN, LBFFT}                 enumMomentUnit;
    typedef enum {PA, HPA, KPA, MPA, BAR, PSI, KSI} enumPressureUnit;
    typedef enum {KGM2, LBMFT2}                     enumInertiaUnit;
    typedef enum {IS, IMPERIAL}                     enumFluidUnit;

    extern FL5LIB_EXPORT enumLengthUnit   g_LengthUnit;
    extern FL5LIB_EXPORT enumSpeedUnit    g_SpeedUnit;
    extern FL5LIB_EXPORT enumAreaUnit     g_AreaUnit;
    extern FL5LIB_EXPORT enumMassUnit     g_MassUnit;
    extern FL5LIB_EXPORT enumForceUnit    g_ForceUnit;
    extern FL5LIB_EXPORT enumMomentUnit   g_MomentUnit;
    extern FL5LIB_EXPORT enumPressureUnit g_PressureUnit;
    extern FL5LIB_EXPORT enumInertiaUnit  g_InertiaUnit;

    extern FL5LIB_EXPORT double g_mtoUnit;    /**< Conversion factor from meters to the custom length unit. */
    extern FL5LIB_EXPORT double g_mstoUnit;   /**< Conversion factor from m/s to the custom speed unit. */
    extern FL5LIB_EXPORT double g_m2toUnit;   /**< Conversion factor from square meters to the custom area unit. */
    extern FL5LIB_EXPORT double g_kgtoUnit;   /**< Conversion factor from kg to the custom mass unit. */
    extern FL5LIB_EXPORT double g_NtoUnit;    /**< Conversion factor from Newtons to the custom force unit. */
    extern FL5LIB_EXPORT double g_NmtoUnit;   /**< Conversion factor from N.m to the custom unit for moments. */
    extern FL5LIB_EXPORT double g_PatoUnit;   /**< Conversion factor from Pascal to the custom unit for pressures. */
    extern FL5LIB_EXPORT double g_kgm2toUnit; /**< Conversion factor from kg.m² to custom unit for inertias */

    extern FL5LIB_EXPORT double g_kgm3toUnit; /**< Conversion factor from kg.m³ to imperial unit for densities */
    extern FL5LIB_EXPORT double g_m2stoUnit;  /**< Conversion factor from m²/s to imperial unit for kinematic viscosities */

    extern FL5LIB_EXPORT std::vector<std::string> g_LengthUnitLabels;
    extern FL5LIB_EXPORT std::vector<std::string> g_MassUnitLabels;
    extern FL5LIB_EXPORT std::vector<std::string> g_AreaUnitLabels;
    extern FL5LIB_EXPORT std::vector<std::string> g_ForceUnitLabels;

    extern FL5LIB_EXPORT std::vector<std::string> g_SpeedUnitLabels;
    extern FL5LIB_EXPORT std::vector<std::string> g_MomentUnitLabels;
    extern FL5LIB_EXPORT std::vector<std::string> g_PressureUnitLabels;
    extern FL5LIB_EXPORT std::vector<std::string> g_InertiaUnitLabels;

    extern FL5LIB_EXPORT enumFluidUnit g_FluidUnitType;
    extern FL5LIB_EXPORT std::vector<std::string> g_DensityUnitLabels;
    extern FL5LIB_EXPORT std::vector<std::string> g_ViscosityUnitLabels;

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


    FL5LIB_EXPORT inline void getLengthUnitLabel(  std::string &label) {label = g_LengthUnitLabels.at(g_LengthUnit);}
    FL5LIB_EXPORT inline void getSpeedUnitLabel(   std::string &label) {label = g_SpeedUnitLabels.at(g_SpeedUnit);}
    FL5LIB_EXPORT inline void getMassUnitLabel(    std::string &label) {label = g_MassUnitLabels.at(g_MassUnit);}
    FL5LIB_EXPORT inline void getAreaUnitLabel(    std::string &label) {label = g_AreaUnitLabels.at(g_AreaUnit);}
    FL5LIB_EXPORT inline void getForceUnitLabel(   std::string &label) {label = g_ForceUnitLabels.at(g_ForceUnit);}
    FL5LIB_EXPORT inline void getMomentUnitLabel(  std::string &label) {label = g_MomentUnitLabels.at(g_MomentUnit);}
    FL5LIB_EXPORT inline void getPressureUnitLabel(std::string &label) {label = g_PressureUnitLabels.at(g_PressureUnit);}
    FL5LIB_EXPORT inline void getInertiaUnitLabel( std::string &label) {label = g_PressureUnitLabels.at(g_PressureUnit);}


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

    FL5LIB_EXPORT inline enumLengthUnit   lengthUnitIndex()   {return g_LengthUnit;}
    FL5LIB_EXPORT inline enumSpeedUnit    speedUnitIndex()    {return g_SpeedUnit;}
    FL5LIB_EXPORT inline enumAreaUnit     areaUnitIndex()     {return g_AreaUnit;}
    FL5LIB_EXPORT inline enumMassUnit     massUnitIndex()     {return g_MassUnit;}
    FL5LIB_EXPORT inline enumForceUnit    forceUnitIndex()    {return g_ForceUnit;}
    FL5LIB_EXPORT inline enumMomentUnit   momentUnitIndex()   {return g_MomentUnit;}
    FL5LIB_EXPORT inline enumPressureUnit pressureUnitIndex() {return g_PressureUnit;}
    FL5LIB_EXPORT inline enumInertiaUnit  inertiaUnitIndex()  {return g_InertiaUnit;}

    FL5LIB_EXPORT inline void setLengthUnit(  enumLengthUnit   index) {g_LengthUnit   = index;}
    FL5LIB_EXPORT inline void setSpeedUnit(   enumSpeedUnit    index) {g_SpeedUnit    = index;}
    FL5LIB_EXPORT inline void setAreaUnit(    enumAreaUnit     index) {g_AreaUnit     = index;}
    FL5LIB_EXPORT inline void setMassUnit(    enumMassUnit     index) {g_MassUnit     = index;}
    FL5LIB_EXPORT inline void setForceUnit(   enumForceUnit    index) {g_ForceUnit    = index;}
    FL5LIB_EXPORT inline void setMomentUnit(  enumMomentUnit   index) {g_MomentUnit   = index;}
    FL5LIB_EXPORT inline void setPressureUnit(enumPressureUnit index) {g_PressureUnit = index;}
    FL5LIB_EXPORT inline void setInertiaUnit( enumInertiaUnit  index) {g_InertiaUnit  = index;}

    FL5LIB_EXPORT inline enumFluidUnit fluidUnitType() {return g_FluidUnitType;}
    FL5LIB_EXPORT inline void setFluidUnitType(enumFluidUnit type) {g_FluidUnitType=type;}


}

