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

struct ExtraDrag
{
	ExtraDrag()
	{
		reset();
	}

    ExtraDrag(std::string tag, double area, double coef)
	{
		m_Tag = tag;
		m_Area = area;
		m_Coef = coef;
	}

	void reset()
	{
		m_Tag.clear();
		m_Area = 0.0;
		m_Coef = 0.0;
	}

    void set(std::string tag, double area, double coef)
	{
		m_Tag = tag;
		m_Area = area;
		m_Coef = coef;
	}

    std::string tag()   const {return m_Tag;}
    std::string name() const {return m_Tag;}
	double area()  const {return m_Area;}
	double coef()  const {return m_Coef;}

    void setTag(std::string tag)   {m_Tag=tag;}
    void setName(std::string name) {m_Tag=name;}
    void setArea(double area)  {m_Area=area;}
	void setCoef(double coef)  {m_Coef=coef;}


    std::string m_Tag; /** The array of extra drag tags */
	double m_Area; /** The array of extra drag area */
	double m_Coef; /** The array of extra drag coefficient */
};

