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

#include <vector>
#include <string>

#include <fl5lib_global.h>
#include <geom_params.h>

class FL5LIB_EXPORT AngleControl
{
    public:
        AngleControl() {m_Name.clear();}
        AngleControl(std::string const &name, std::vector<double> const &values) {m_Name=name; m_Value=values;}

        inline void resizeValues(int n) {m_Value.resize(n);}
        inline void resetValues() {std::fill(m_Value.begin(), m_Value.end(), 0);}

        inline std::string const &name() const {return m_Name;}
        inline void setName(std::string const &controlname) {m_Name=controlname;}

        inline int nValues() const {return int(m_Value.size());}

        inline double value(  int iCtrl) const {if (iCtrl<0 || iCtrl>=nValues())   return 0.0; else return m_Value.at(iCtrl);}
        inline void setValue( int iCtrl, double g) {if (iCtrl>=0 && iCtrl<nValues())   m_Value[iCtrl]=g;}
        inline void addValue(double g) {m_Value.push_back(g);}

        inline bool hasActiveAngle() const {for(int ig=0; ig<nValues(); ig++) {if(fabs(m_Value.at(ig))>FLAPANGLEPRECISION) return true;} return false;}

    private:

        std::string m_Name;     /**< The control's name which serves as its unique identifier */
        std::vector<double> m_Value; /**< The flap angles in degrees or the gains in degrees/ctrl unit depending on the use.*/
};

