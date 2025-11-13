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

/** @class This class defines a polar imported from an external source */

#include "wpolarext.h"

WPolarExt::WPolarExt() : WPolar()
{
    m_Type = xfl::EXTERNALPOLAR;
    m_AnalysisMethod = Polar3d::NOMETHOD;
    m_data.resize(variableCount());
}


int WPolarExt::dataSize() const
{
    if(m_data.size()) return m_data.at(0).size();
    return 0;
}


void WPolarExt::resizeData(int newsize)
{
    for(int iVar=0; iVar<m_data.size(); iVar++)
        m_data[iVar].resize(newsize);
}


double WPolarExt::getVariable(int iVariable, int index) const
{
    if(iVariable<0 || iVariable>variableCount()) return 0.0;
    if(index<0 || index>m_data.at(0).size())  return 0.0;
    return m_data.at(iVariable).at(index);
}


void WPolarExt::setData(int iVariable, int index, double value)
{
    if(iVariable<0 || iVariable>=variableCount()) return;
    if(index<0 || index>=m_data.at(iVariable).size())  return;

    m_data[iVariable][index] = value;
}


void WPolarExt::clearData()
{
    m_data.resize(variableCount());
    for(int ivar=0; ivar<m_data.size(); ivar++)
        m_data[ivar].clear();
}


void WPolarExt::insertDataPointAt(int index, bool bAfter)
{
    if(index<0 || index>=dataSize()) return;
    if(bAfter) index++;

    for(int ivar=0; ivar<m_data.size(); ivar++)
    {
        m_data[ivar].insert(index,0.0);
    }
}


void WPolarExt::removeAt(int index)
{
    if(index<0 || index>=dataSize()) return;
    for(int ivar=0; ivar<m_data.size(); ivar++)
    {
        m_data[ivar].removeAt(index);
    }
}


void WPolarExt::copy(WPolar const *pWPolar)
{
    if(!pWPolar->isExternalPolar()) return;

    duplicateSpec(pWPolar);
    m_PlaneName = pWPolar->planeName();
    m_Name = pWPolar->name();

    WPolarExt const *pWPolarExt = dynamic_cast<WPolarExt const*>(pWPolar);
    if(pWPolarExt)
        m_data = pWPolarExt->m_data;
}


bool WPolarExt::serializeFl5v726(QDataStream &ar, bool bIsStoring)
{
    if(!Polar3d::serializeFl5v726(ar, bIsStoring)) return false;

    int n=0;
    int nDataPoints=0;
    int nIntSpares=0;
    int nDbleSpares=0;

    double dble=0.0;

    if(bIsStoring)
    {
        Q_ASSERT(false);
    }
    else
    {
        // METADATA
        if(m_PolarFormat < 500001 || m_PolarFormat>500100) return false;
        ar >> m_PlaneName;

        // load the array data
        clearData();
        int nSpares=0;
        ar >> nSpares;
        ar >> nDataPoints;
        if(abs(n)>10000) return false;
        for(int ivar=0; ivar<m_data.size(); ivar++)
            m_data[ivar].resize(nDataPoints);

        int nStoredVariables = variableCount();
        if     (m_PolarFormat<500013) nStoredVariables = 54;
        else if(m_PolarFormat<500030) nStoredVariables = 56;
        else                          nStoredVariables = 57;

        int ivar = 0;
        for (int i=0; i<nStoredVariables; i++)
        {
            if(m_PolarFormat<500030)
            {
                if(i==3) ivar++; // skipping phi non existant in < 500030
            }

            for(int ipt=0; ipt<nDataPoints; ipt++)
            {
                ar >> dble;
                if(ivar<m_data.size() && ipt < m_data[ivar].size())
                    m_data[ivar][ipt] = dble;
            }
            ivar++;
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        for(int iPt=0; iPt<dataSize(); iPt++)	calculatePoint(iPt);
    }
    return true;
}



bool WPolarExt::serializeFl5v750(QDataStream &ar, bool bIsStoring)
{
    if(!Polar3d::serializeFl5v750(ar, bIsStoring)) return false;

    int n=0;
    int nDataPoints(0);

    bool boolean(false);
    int integer(0);
    double dble(0.0);

    if(bIsStoring)
    {
        //METADATA
        ar << m_PlaneName;

        // store the array data
        int nSpares=0;
        dble=0.0;
        ar << nSpares;
        ar << dataSize();
        for(int ivar=0; ivar<m_data.size(); ivar++)
        {
            for (int i=0; i<m_data.at(ivar).size(); i++)
            {
                ar<<m_data.at(ivar).at(i);
                for(int js=0; js<nSpares; js++) ar<<dble;
            }
        }

        // provisions for future variable saves
        for(int i=0; i<5; i++)  ar << boolean;
        for(int i=0; i<10; i++) ar << integer;
        for(int i=0; i<10; i++) ar << dble;

        return true;
    }
    else
    {
        // METADATA
        if(m_PolarFormat < 500750 || m_PolarFormat>501000) return false;
        ar >> m_PlaneName;

        // load the array data
        clearData();
        int nSpares=0;
        ar >> nSpares;
        ar >> nDataPoints;
        if(abs(n)>10000) return false;
        for(int ivar=0; ivar<m_data.size(); ivar++)
            m_data[ivar].resize(nDataPoints);

        int nStoredVariables = variableCount();

        int ivar = 0;
        for (int i=0; i<nStoredVariables; i++)
        {
            for(int ipt=0; ipt<nDataPoints; ipt++)
            {
                ar >> dble;
                if(ivar<m_data.size() && ipt < m_data[ivar].size())
                    m_data[ivar][ipt] = dble;
            }
            ivar++;
        }

        // provisions for future variable saves
        for(int i=0; i<5; i++)  ar >> boolean;
        for(int i=0; i<10; i++) ar >> integer;
        for(int i=0; i<10; i++) ar >> dble;


        for(int iPt=0; iPt<dataSize(); iPt++)    calculatePoint(iPt);
        return true;
    }
}




