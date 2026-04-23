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

#include <QVector>


#include <interfaces/graphs/graph/curve.h>
#include <interfaces/graphs/graph/axis.h>

class CurveModel
{
    friend class Graph;

    public:
        CurveModel();
        ~CurveModel();

        Curve* curve(int nIndex) const;
        Curve* curve(QString curveTitle, bool bFromLast=false) const;
        Curve* firstCurve() const;
        Curve* lastCurve() const;
        Curve* addCurve(AXIS::enumAxis axis=AXIS::LEFTYAXIS, bool bDarkTheme=true);
        Curve* addCurve(QString curveName="", AXIS::enumAxis axis=AXIS::LEFTYAXIS, bool bDarkTheme=true);

        void deleteCurve(int index);
        void deleteCurve(Curve *pCurve);
        void deleteCurve(QString CurveTitle);
        void deleteCurves();
        void resetCurves();
        int curveCount() const {return m_oaCurve.size();}
        bool hasVisibleCurve() const;

        void getXBounds(double &xmin, double &xmax) const;
        void getYBounds(double &ymin, double &ymax, AXIS::enumAxis axis) const;

        void getXPositiveBounds(double &xmin, double &xmax) const;
        void getYPositiveBounds(double &ymin, double &ymax) const;

        Curve* getClosestPoint(double x, double y, double &xSel, double &ySel, int &nSel) const;

        void clearSelection() {m_SelectedCurve = nullptr;}
        bool selectCurve(int ic);
        bool selectCurve(const Curve *pCurve);
        bool selectCurve(QString const & curvename);
        bool isCurveSelected(Curve const *pCurve) const {return m_SelectedCurve==pCurve;}

        static void setColorList(const QVector<QColor> &colors) {s_CurveColor=colors;}
        static QColor color(int idx);

    private:
        QVector<Curve*> m_oaCurve;
        Curve const* m_SelectedCurve;


        static QVector<QColor> s_CurveColor;
};





