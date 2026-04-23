/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <string>
#include <vector>


namespace xfl
{
    enum enumObjectiveType {MINIMIZE, EQUALIZE, MAXIMIZE}; // defines whether the objective is to minimize, maximize or make equal
}



struct OptObjective
{
        OptObjective() : m_Index{-1}, m_bActive{true}, m_Target{0.0}, m_MaxError{0.0}, m_Type{xfl::EQUALIZE}
    {}

    OptObjective(std::string const &name, int index, bool bActive, double target, double maxerror, xfl::enumObjectiveType type) :
        m_Name{name}, m_Index{index}, m_bActive{bActive}, m_Target{target}, m_MaxError{maxerror}, m_Type{type}
    {}

    std::string m_Name;
    int m_Index{-1};         /**< the objective's index in the array of possible objectives */
    bool m_bActive{true};    /**< true if this objective is active in the current optimization task */
    double m_Target{0.0};    /**< this objective's target value */
    double m_MaxError{0.0};  /**< this objective's maximum error */
    xfl::enumObjectiveType m_Type = xfl::EQUALIZE;
};


struct OptVariable
{
    OptVariable() : m_Min{0.0}, m_Max{0.0}
    {}

    OptVariable(std::string const &name, double valmin, double valmax) : m_Name{name}, m_Min{valmin}, m_Max{valmax}
    {}

    OptVariable(std::string const &name, double val) : m_Name{name}, m_Min{val}, m_Max{val}
    {}

    std::string m_Name;
    double m_Min{0.0};
    double m_Max{0.0};
};


struct OptCp
{
    OptCp() : m_iMin{-1}, m_iMax{-1}
    {}

    OptCp(int iMin, int iMax, std::vector<double> const &Cp) : m_iMin{iMin}, m_iMax{iMax}, m_Cp{Cp}
    {}

    bool isActive() const {return m_iMin>=0 && m_iMax>=0;}

    int m_iMin{-1};           /**< the index on the curve of the spline's first control point */
    int m_iMax{-1};           /**< the index on the curve of the spline's last control point */
    std::vector<double>m_Cp;  /**< the array of objective Cp values from node m_iMin to node m_iMax */
};

