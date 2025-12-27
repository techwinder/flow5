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



#include <units.h>

std::vector<std::string> Units::g_LengthUnitLabels    = {"mm","cm","dm","m","in","ft"};
std::vector<std::string> Units::g_MassUnitLabels      = {"g","kg","oz","lb"};
std::vector<std::string> Units::g_AreaUnitLabels      = {"mm²", "cm²", "dm²", "m²", "in²", "ft²"};
std::vector<std::string> Units::g_ForceUnitLabels     = {"N", "kN", "ton", "lbf"};
std::vector<std::string> Units::g_SpeedUnitLabels     = {"m/s", "km/h", "ft/s", "kt", "mph"};
std::vector<std::string> Units::g_MomentUnitLabels    = {"N.m","lbf.in","lbf.ft"};
std::vector<std::string> Units::g_PressureUnitLabels  = {"Pa", "hPa", "kPa", "MPa", "bar", "psi", "ksi"};
std::vector<std::string> Units::g_InertiaUnitLabels   = {"kg.m²", "lbm.ft²"};
std::vector<std::string> Units::g_DensityUnitLabels   = {"kg/m³", "slugs/ft³"};
std::vector<std::string> Units::g_ViscosityUnitLabels = {"m²/s",  "ft²/s"};

double Units::g_mtoUnit  = 1.0;
double Units::g_mstoUnit = 1.0;
double Units::g_m2toUnit = 1.0;
double Units::g_kgtoUnit = 1.0;
double Units::g_NtoUnit  = 1.0;
double Units::g_NmtoUnit = 1.0;
double Units::g_PatoUnit = 1.0;
double Units::g_kgm2toUnit = 1.0;
double Units::g_kgm3toUnit = 1.0;
double Units::g_m2stoUnit  = 1.0;


int Units::g_LengthUnitIndex   = 3;
int Units::g_SpeedUnitIndex    = 0;
int Units::g_AreaUnitIndex     = 3;
int Units::g_MassUnitIndex     = 1;
int Units::g_ForceUnitIndex    = 0;
int Units::g_MomentUnitIndex   = 0;
int Units::g_PressureUnitIndex = 0;
int Units::g_InertiaUnitIndex  = 0;

int Units::g_FluidUnitType(0);


double Units::toCustomUnit(int index)
{
    switch(index)
    {
        case 0:  return g_mtoUnit;
        case 1:  return g_mstoUnit;
        case 2:  return g_kgtoUnit;
        case 3:  return g_m2toUnit;
        case 4:  return g_NtoUnit;
        case 5:  return g_NmtoUnit;
        case 6:  return g_PatoUnit;
        case 7:  return g_kgm2toUnit;
        default: return 1.0;
    }
}


std::string Units::lengthUnitLabel(int idx)
{
    if(idx>=0) return g_LengthUnitLabels[idx];
    else       return g_LengthUnitLabels[g_LengthUnitIndex];
}


std::string Units::speedUnitLabel(int idx)
{
    if(idx>=0) return g_SpeedUnitLabels[idx];
    else       return g_SpeedUnitLabels[g_SpeedUnitIndex];
}


std::string Units::massUnitLabel(int idx)
{
    if(idx>=0) return g_MassUnitLabels[idx];
    else       return g_MassUnitLabels[g_MassUnitIndex];
}


std::string Units::areaUnitLabel(int idx)
{
    if(idx>=0) return g_AreaUnitLabels[idx];
    else       return g_AreaUnitLabels[g_AreaUnitIndex];
}


std::string Units::forceUnitLabel(int idx)
{
    if(idx>=0) return g_ForceUnitLabels[idx];
    else       return g_ForceUnitLabels[g_ForceUnitIndex];
}


std::string Units::momentUnitLabel(int idx)
{
    if(idx>=0) return g_MomentUnitLabels[idx];
    else       return g_MomentUnitLabels[g_MomentUnitIndex];
}


std::string Units::pressureUnitLabel(int idx)
{
    if(idx>=0) return g_PressureUnitLabels[idx];
    else       return g_PressureUnitLabels[g_PressureUnitIndex];
}


std::string Units::inertiaUnitLabel(int idx)
{
    if(idx>=0) return g_InertiaUnitLabels[idx];
    else       return g_InertiaUnitLabels[g_InertiaUnitIndex];
}


std::string Units::densityUnitLabel()
{
    return g_DensityUnitLabels.at(g_FluidUnitType);
}


std::string Units::viscosityUnitLabel()
{
    return g_ViscosityUnitLabels.at(g_FluidUnitType);
}


void Units::setUnitConversionFactors()
{
    switch(g_LengthUnitIndex)
    {
        case 0:{//mm
            g_mtoUnit  = 1000.0;
            break;
        }
        case 1:{//cm
            g_mtoUnit  = 100.0;
            break;
        }
        case 2:{//dm
            g_mtoUnit  = 10.0;
            break;
        }
        case 3:{//m
            g_mtoUnit  = 1.0;
            break;
        }
        case 4:{//in
            g_mtoUnit  = 1000.0/25.4;
            break;
        }
        case 5:{//ft
            g_mtoUnit  = 1000.0/25.4/12.0;
            break;
        }
        default:{//m
            g_mtoUnit  = 1.0;
            break;
        }
    }
    switch(g_AreaUnitIndex)
    {
        case 0:{//mm²
            g_m2toUnit = 1000000.0;
            break;
        }
        case 1:{//cm²
            g_m2toUnit = 10000.0;
            break;
        }
        case 2:{//dm²
            g_m2toUnit = 100.0;
            break;
        }
        case 3:{//m²
            g_m2toUnit = 1.0;
            break;
        }
        case 4:{//in²
            g_m2toUnit = 1./0.254/0.254*100.0;
            break;
        }
        case 5:{//ft²
            g_m2toUnit = 1./0.254/0.254/144.0*100.0;
            break;
        }
        default:{
            g_m2toUnit = 1.0;
            break;
        }
    }

    switch(g_MassUnitIndex){
        case 0:{///g
            g_kgtoUnit = 1000.0;
            break;
        }
        case 1:{//kg
            g_kgtoUnit = 1.0;

            break;
        }
        case 2:{//oz
            g_kgtoUnit = 1./ 2.83495e-2;
            break;
        }
        case 3:{//lb
            g_kgtoUnit = 1.0/0.45359237;
            break;
        }
        default:{
            g_kgtoUnit = 1.0;
            break;
        }
    }

    switch(g_SpeedUnitIndex){
        case 0:{// m/s
            g_mstoUnit = 1.0;
            break;
        }
        case 1:{// km/h
            g_mstoUnit = 3600.0/1000.0;
            break;
        }
        case 2:{// ft/s
            g_mstoUnit = 100.0/2.54/12.0;
            break;
        }
        case 3:{// kt (int.)
            g_mstoUnit = 1.0/0.514444;
            break;
        }
        case 4:{// mph
            g_mstoUnit = 3600.0/1609.344;
            break;
        }
        default:{
            g_mstoUnit = 1.0;
            break;
        }
    }

    switch(g_ForceUnitIndex){
        case 0:{//N
            g_NtoUnit = 1.0;
            break;
        }
        case 1:{//kN
            g_NtoUnit = 0.001;
            break;
        }
        case 2:{//ton
            g_NtoUnit = 1.0/1000.0/9.81;
            break;
        }
        case 3:{//lbf
            g_NtoUnit = 1.0/4.44822;
            break;
        }
        default:{
            g_NtoUnit = 1.0;
            break;
        }
    }

    switch(g_MomentUnitIndex)
    {
        case 0:{//N.m
            g_NmtoUnit = 1.0;
            break;
        }
        case 1:{//lbf.in
            g_NmtoUnit = 1.0/4.44822/0.0254;
            break;
        }
        case 2:{//lbf.0t
            g_NmtoUnit = 1.0/4.44822/12.0/0.0254;
            break;
        }
        default:{
            g_NmtoUnit = 1.0;
            break;
        }
    }

    switch(g_PressureUnitIndex)
    {
        case 0:{//Pa
            g_PatoUnit = 1.0;
            break;
        }
        case 1:{//hPa
            g_PatoUnit = 1.0/100.0;
            break;
        }
        case 2:{//kPa
            g_PatoUnit = 1.0/1000.0;
            break;
        }
        case 3:{//MPa
            g_PatoUnit = 1.0/1000000;
            break;
        }
        case 4:{//bar
            g_PatoUnit = 1.0/100000;
            break;
        }
        case 5:{//psi
            g_PatoUnit = 0.000145038;
            break;
        }
        case 6:{//ksi
            g_PatoUnit = 0.000000145038;
            break;
        }
        default:{
            g_PatoUnit = 1.0;
            break;
        }
    }

    switch(g_InertiaUnitIndex)
    {
        case 0:{//Pa
            g_kgm2toUnit = 1.0;
            break;
        }
        case 1:{//hPa
            g_kgm2toUnit = 1.0/0.45359237 * 1000.0/25.4/12.0 * 1000.0/25.4/12.0;
            break;
        }
    }

    switch (g_FluidUnitType)
    {
        case 0:
        { //INTERNATIONAL
            g_kgm3toUnit = 1.0;
            g_m2stoUnit = 1.0;
            break;
        }
        case 1:
        { //IMPERIAL
            g_kgm3toUnit = 0.00194122;
            g_m2stoUnit  = 10.7182881;
            break;
        }
    }
}



