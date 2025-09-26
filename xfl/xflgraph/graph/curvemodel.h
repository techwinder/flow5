/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QVector>


#include <xflgraph/graph/curve.h>
#include <xflgraph/graph/axis.h>

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





