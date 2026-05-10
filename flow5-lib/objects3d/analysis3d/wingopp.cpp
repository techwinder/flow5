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



#include <wingopp.h>
#include <wingxfl.h>

#include <geom_global.h>
#include <planepolar.h>



WingOpp::WingOpp(int PanelArraySize)
{
    m_nPanel4    = PanelArraySize;
    m_dCp = m_dG = m_dSigma = nullptr;

    m_bOut         = false;

    m_Span    = 0.0;
    m_MAChord = 0.0;

    m_NStation     = 0;
    m_nFlaps       = 0;

    m_FlapMoment.clear();
}


double WingOpp::maxLift() const
{
    double maxlift = 0.0;
    for (int i=0; i<m_NStation; i++)
    {
        if(m_SpanDistrib.m_Cl.at(i) * m_SpanDistrib.m_Chord.at(i)/m_MAChord>maxlift)
        {
            maxlift = m_SpanDistrib.m_Cl.at(i) * m_SpanDistrib.m_Chord.at(i)/m_MAChord;
        }
    }
    return maxlift;
}


void WingOpp::createWOpp(WingXfl const *pWing, PlanePolar const *pWPolar, SpanDistribs const &distribs, AeroForces const &AF)
{
    m_WingType   = pWing->wingType();
    m_WingName   = pWing->name();
    m_nPanel4    = pWing->nPanel4();
    m_NStation   = pWing->nStations();
    m_nFlaps     = pWing->nFlaps();

    m_Span       = pWPolar->referenceSpanLength();

    m_MAChord    = pWing->MAC();

    /**< @todo check if m_CP !=0 */
    m_AF = AF;

    m_SpanDistrib = distribs;

    m_MaxBending =0.0;
    for(unsigned int l=0; l<m_SpanDistrib.m_BendingMoment.size(); l++)
    {
        m_MaxBending = std::max(m_MaxBending, m_SpanDistrib.m_BendingMoment.at(l));
    }
}


