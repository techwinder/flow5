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

#include <format>
#include <string>



#include <occmeshparams.h>

#include <units.h>
#include <utils.h>

OccMeshParams::OccMeshParams()
{
    setDefaults();
}

void OccMeshParams::setDefaults()
{
    m_bLinDefAbs       = false;
    m_LinDeflectionAbs = 0.005;// absolute, meters
    m_LinDeflectionRel = 0.03;
    m_AngularDeviation = 15.0; // in degrees
    m_MaxElementSize   = 0.05;
}


std::string OccMeshParams::listParams(std::string const &prefix)
{
    std::string list;
    std::string strange;
    if(m_bLinDefAbs)
    {
        strange = std::format("Absolute lin. defl. = {:.3f}", m_LinDeflectionAbs*Units::mtoUnit());
        strange += Units::lengthUnitLabel() + "\n";
    }
    else
    {
        strange = std::format("Relative lin. defl. = {:.1f}", m_LinDeflectionRel*100.0);
        strange += DEGstr + "\n";
    }
    list += prefix + strange;

    strange = std::format("Angular deviation = {:.1f}", m_AngularDeviation);
    strange+= DEGstr+"\n";
    list += prefix + strange;

    return list;
}

void OccMeshParams::duplicate(OccMeshParams const &params)
{
    m_bLinDefAbs       = params.m_bLinDefAbs;
    m_LinDeflectionAbs = params.m_LinDeflectionAbs;// absolute, meters
    m_LinDeflectionRel = params.m_LinDeflectionRel;
    m_AngularDeviation = params.m_AngularDeviation; // in degrees
    m_MaxElementSize   = params.m_MaxElementSize;
}




