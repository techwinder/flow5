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

/** @class This class defines a polar imported from an external source */

#include <planepolarext.h>

PlanePolarExt::PlanePolarExt() : PlanePolar()
{
    m_Type = xfl::EXTERNALPOLAR;
    m_AnalysisMethod = xfl::NOMETHOD;
    m_data.resize(variableCount());
}


int PlanePolarExt::dataSize() const
{
    if(m_data.size()) return int(m_data.at(0).size());
    return 0;
}


void PlanePolarExt::resizeData(int newsize)
{
    for(unsigned int iVar=0; iVar<m_data.size(); iVar++)
        m_data[iVar].resize(newsize);
}


double PlanePolarExt::getVariable(int iVariable, int index) const
{
    if(iVariable<0 || iVariable>variableCount()) return 0.0;
    if((index<0) || (index>int(m_data.at(0).size())))  return 0.0;
    return m_data.at(iVariable).at(index);
}


void PlanePolarExt::setData(int iVariable, int index, double value)
{
    if(iVariable<0 || iVariable>=variableCount()) return;
    if(index<0 || index>=int(m_data.at(iVariable).size()))  return;

    m_data[iVariable][index] = value;
}


void PlanePolarExt::clearData()
{
    m_data.resize(variableCount());
    for(unsigned int ivar=0; ivar<m_data.size(); ivar++)
        m_data[ivar].clear();
}


void PlanePolarExt::insertDataPointAt(int index, bool bAfter)
{
    if(index<0 || index>=dataSize()) return;
    if(bAfter) index++;

    for(unsigned int ivar=0; ivar<m_data.size(); ivar++)
    {
        m_data[ivar].insert(m_data[ivar].begin()+index,0.0);
    }
}


void PlanePolarExt::removeAt(int index)
{
    if(index<0 || index>=dataSize()) return;
    for(unsigned int ivar=0; ivar<m_data.size(); ivar++)
    {
        m_data[ivar].erase(m_data[ivar].begin()+index);
    }
}


void PlanePolarExt::copy(PlanePolar const *pWPolar)
{
    if(!pWPolar->isExternalPolar()) return;

    duplicateSpec(pWPolar);
    m_PlaneName = pWPolar->planeName();
    m_Name = pWPolar->name();

    PlanePolarExt const *pWPolarExt = dynamic_cast<PlanePolarExt const*>(pWPolar);
    if(pWPolarExt)
        m_data = pWPolarExt->m_data;
}



