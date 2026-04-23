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

#include <fl5lib_global.h>

#include <linestyle.h>


class FL5LIB_EXPORT XflObject
{
    public:
        XflObject() = default;
        virtual ~XflObject() = default;

    public:
        virtual std::string name() const {return m_Name;}
        void setName(std::string const &name) {m_Name = name;}

        fl5Color const &lineColor() const {return m_theStyle.m_Color;}
        void setLineColor(fl5Color const &clr) {m_theStyle.m_Color=clr;}

        Line::enumLineStipple lineStipple() const {return m_theStyle.m_Stipple;}
        void setLineStipple(Line::enumLineStipple iStipple) {m_theStyle.m_Stipple=iStipple;}

        int lineWidth() const {return m_theStyle.m_Width;}
        void setLineWidth(int iWidth) {m_theStyle.m_Width=iWidth;}

        Line::enumPointStyle pointStyle() const {return m_theStyle.m_Symbol;}
        void setPointStyle(Line::enumPointStyle iPointStyle) {m_theStyle.m_Symbol=iPointStyle;}

        void setTheStyle(LineStyle const &ls) {m_theStyle=ls;}
        LineStyle const &theStyle() const {return m_theStyle;}
        LineStyle &theStyle() {return m_theStyle;}

        bool isVisible() const {return m_theStyle.m_bIsVisible;}
        void setVisible(bool bVisible) {m_theStyle.m_bIsVisible = bVisible;}
        void show() {m_theStyle.m_bIsVisible=true;}
        void hide() {m_theStyle.m_bIsVisible=false;}

        int nCurves() const {return int(m_Curve.size());}
        int curveCount() const {return int(m_Curve.size());}
        void clearCurves() {m_Curve.clear();}
        void appendCurve(void *pCurve) {m_Curve.push_back(pCurve);}
        void *curve(int iCurve) {return m_Curve[iCurve];}
        bool hasCurve(void const*pCurve) {return std::find(m_Curve.begin(), m_Curve.end(), pCurve) != m_Curve.end();}

    protected:

        std::string m_Name;
        LineStyle m_theStyle;
        std::vector <void*> m_Curve;
};

