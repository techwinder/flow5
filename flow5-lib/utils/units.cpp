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


Units::enumLengthUnit   Units::g_LengthUnit   = Units::M;
Units::enumSpeedUnit    Units::g_SpeedUnit    = Units::MS;
Units::enumAreaUnit     Units::g_AreaUnit     = Units::M2;
Units::enumMassUnit     Units::g_MassUnit     = Units::KG;
Units::enumForceUnit    Units::g_ForceUnit    = Units::N;
Units::enumMomentUnit   Units::g_MomentUnit   = Units::NM;
Units::enumPressureUnit Units::g_PressureUnit = Units::PA;
Units::enumInertiaUnit  Units::g_InertiaUnit  = Units::KGM2;

Units::enumFluidUnit Units::g_FluidUnitType(Units::IS);


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
    else       return g_LengthUnitLabels[g_LengthUnit];
}


std::string Units::speedUnitLabel(int idx)
{
    if(idx>=0) return g_SpeedUnitLabels[idx];
    else       return g_SpeedUnitLabels[g_SpeedUnit];
}


std::string Units::massUnitLabel(int idx)
{
    if(idx>=0) return g_MassUnitLabels[idx];
    else       return g_MassUnitLabels[g_MassUnit];
}


std::string Units::areaUnitLabel(int idx)
{
    if(idx>=0) return g_AreaUnitLabels[idx];
    else       return g_AreaUnitLabels[g_AreaUnit];
}


std::string Units::forceUnitLabel(int idx)
{
    if(idx>=0) return g_ForceUnitLabels[idx];
    else       return g_ForceUnitLabels[g_ForceUnit];
}


std::string Units::momentUnitLabel(int idx)
{
    if(idx>=0) return g_MomentUnitLabels[idx];
    else       return g_MomentUnitLabels[g_MomentUnit];
}


std::string Units::pressureUnitLabel(int idx)
{
    if(idx>=0) return g_PressureUnitLabels[idx];
    else       return g_PressureUnitLabels[g_PressureUnit];
}


std::string Units::inertiaUnitLabel(int idx)
{
    if(idx>=0) return g_InertiaUnitLabels[idx];
    else       return g_InertiaUnitLabels[g_InertiaUnit];
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
    switch(g_LengthUnit)
    {
        case MM:   g_mtoUnit  = 1000.0;                break;
        case CM:   g_mtoUnit  = 100.0;                 break;
        case DM:   g_mtoUnit  = 10.0;                  break;
        default:
        case M:    g_mtoUnit  = 1.0;                   break;
        case INCH: g_mtoUnit  = 1000.0/25.4;           break;
        case FT:   g_mtoUnit  = 1000.0/25.4/12.0;      break;
    }


    switch(g_AreaUnit)
    {
        case MM2:  g_m2toUnit = 1000000.0;                     break;
        case CM2:  g_m2toUnit = 10000.0;                       break;
        case DM2:  g_m2toUnit = 100.0;                         break;
        default:
        case M2:   g_m2toUnit = 1.0;                           break;
        case IN2:  g_m2toUnit = 1./0.254/0.254*100.0;          break;
        case FT2:  g_m2toUnit = 1./0.254/0.254/144.0*100.0;    break;
    }


    switch(g_SpeedUnit)
    {
        default:
        case MS:    g_mstoUnit = 1.0;                          break;
        case KMH:   g_mstoUnit = 3600.0/1000.0;                break;
        case FTS:   g_mstoUnit = 100.0/2.54/12.0;              break;
        case KT:    g_mstoUnit = 1.0/0.514444;                 break;
        case MPH:   g_mstoUnit = 3600.0/1609.344;              break;
    }


    switch(g_MassUnit)
    {
        case G:          g_kgtoUnit = 1000.0;            break;
        default:
        case KG:         g_kgtoUnit = 1.0;               break;
        case OZ:         g_kgtoUnit = 1./ 2.83495e-2;    break;
        case LB:         g_kgtoUnit = 1.0/0.45359237;    break;
    }

    switch(g_ForceUnit)
    {
        default:
        case N:     g_NtoUnit = 1.0;                           break;
        case KN:    g_NtoUnit = 0.001;                         break;
        case TON:   g_NtoUnit = 1.0/1000.0/9.81;               break;
        case LBF:   g_NtoUnit = 1.0/4.44822;                   break;
    }

    switch(g_MomentUnit)
    {
        default:
        case NM:     g_NmtoUnit = 1.0;                       break;
        case LBFIN:  g_NmtoUnit = 1.0/4.44822/0.0254;        break;
        case LBFFT:  g_NmtoUnit = 1.0/4.44822/12.0/0.0254;   break;
    }

    switch(g_PressureUnit)
    {
        default:
        case PA:     g_PatoUnit = 1.0;             break;
        case HPA:    g_PatoUnit = 1.0/100.0;       break;
        case KPA:    g_PatoUnit = 1.0/1000.0;      break;
        case MPA:    g_PatoUnit = 1.0/1000000;     break;
        case BAR:    g_PatoUnit = 1.0/100000;      break;
        case PSI:    g_PatoUnit = 0.000145038;     break;
        case KSI:    g_PatoUnit = 0.000000145038;  break;
    }

    switch(g_InertiaUnit)
    {
        default:
        case KGM2:   g_kgm2toUnit = 1.0;                                                   break;
        case LBMFT2: g_kgm2toUnit = 1.0/0.45359237 * 1000.0/25.4/12.0 * 1000.0/25.4/12.0;  break;
    }

    switch (g_FluidUnitType)
    {
        case Units::IS:
        {
            g_kgm3toUnit = 1.0;
            g_m2stoUnit = 1.0;
            break;
        }
        case Units::IMPERIAL:
        {
            g_kgm3toUnit = 0.00194122;
            g_m2stoUnit  = 10.7182881;
            break;
        }
    }
}



