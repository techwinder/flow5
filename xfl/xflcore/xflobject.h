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

#include <QColor>
#include <xflcore/linestyle.h>

class Curve;

class XflObject
{
    public:
        XflObject() = default;
        virtual ~XflObject() = default;

    public:
        virtual QString name() const {return m_Name;}
        void setName(QString const & name) {m_Name=name;}

        QColor const &lineColor() const {return m_theStyle.m_Color;}
        void setLineColor(QColor clr) {m_theStyle.m_Color=clr;}

        Line::enumLineStipple lineStipple() const {return m_theStyle.m_Stipple;}
        void setLineStipple(Line::enumLineStipple iStipple) {m_theStyle.m_Stipple=iStipple;}

        int lineWidth() const {return m_theStyle.m_Width;}
        void setLineWidth(int iWidth) {m_theStyle.m_Width=iWidth;}

        bool isVisible() const {return m_theStyle.m_bIsVisible;}
        void setVisible(bool bVisible) {m_theStyle.m_bIsVisible = bVisible;}
        void show() {m_theStyle.m_bIsVisible=true;}
        void hide() {m_theStyle.m_bIsVisible=false;}

        Line::enumPointStyle pointStyle() const {return m_theStyle.m_Symbol;}
        void setPointStyle(Line::enumPointStyle iPointStyle) {m_theStyle.m_Symbol=iPointStyle;}

        void setTheStyle(LineStyle const &ls) {m_theStyle=ls;}
        LineStyle const &theStyle() const {return m_theStyle;}
        LineStyle &theStyle() {return m_theStyle;}

        int curveCount() const {return m_curve.size();}
        void clearCurves() {m_curve.clear();}
        void appendCurve(Curve *pCurve) {m_curve.append(pCurve);}
        Curve *curve(int iCurve) {return m_curve[iCurve];}

    protected:

        QString m_Name;
        LineStyle m_theStyle;
        QList <Curve*> m_curve;
};

