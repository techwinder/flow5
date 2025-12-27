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

#include <string>

#include <fl5lib_global.h>

class FL5LIB_EXPORT CtrlRange
{
    public:
        CtrlRange() : m_CtrlMin{0.0}, m_CtrlMax{0.0}
        {
        }

        CtrlRange(std::string const &name, double cmin, double cmax) {m_CtrlName=name; m_CtrlMin=cmin; m_CtrlMax=cmax;}

        double ctrlVal(double t) const {return (1.0-t)*m_CtrlMin + t*m_CtrlMax;}
        double ctrlMin() const {return m_CtrlMin;}
        double ctrlMax() const {return m_CtrlMax;}
        double range() const {return m_CtrlMax-m_CtrlMin;}
        std::string const &name() const {return m_CtrlName;}

        void set(std::string const & name, double cmin, double cmax) {m_CtrlName=name;m_CtrlMin=cmin; m_CtrlMax=cmax;}
        void setName(std::string const &controlname) {m_CtrlName=controlname;}
        void setRange(double cmin, double cmax) {m_CtrlMin=cmin; m_CtrlMax=cmax;}
        void setCtrlMin(double cmin) {m_CtrlMin=cmin;}
        void setCtrlMax(double cmax) {m_CtrlMax=cmax;}

    private:
        double m_CtrlMin;
        double m_CtrlMax;
        std::string m_CtrlName; //optional
};

