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


#include "curvemodel.h"


QVector<QColor> CurveModel::s_CurveColor;

CurveModel::CurveModel()
{
    s_CurveColor.resize(10);
    s_CurveColor[0] = QColor( 85, 170, 255);
    s_CurveColor[1] = QColor(215,  75,  65);
    s_CurveColor[2] = QColor(125, 195, 105);
    s_CurveColor[3] = QColor(215, 135, 135);
    s_CurveColor[4] = QColor( 85,  85, 170);
    s_CurveColor[5] = QColor(255, 70,  200);
    s_CurveColor[6] = QColor(165,  55, 255);
    s_CurveColor[7] = QColor( 70, 125, 255);
    s_CurveColor[8] = QColor(153,  85,  36);
    s_CurveColor[9] = QColor(215, 215,  75);

    m_SelectedCurve = nullptr;
}


CurveModel::~CurveModel()
{
    deleteCurves();
}


Curve* CurveModel::curve(int nIndex) const
{
    if(nIndex<0 || nIndex>=m_oaCurve.size())return nullptr;
    return m_oaCurve.at(nIndex);
}



Curve* CurveModel::curve(QString curveTitle, bool bFromLast) const
{
    QString strong;

    if(bFromLast)
    {
        for(int i=m_oaCurve.size()-1; i>=0; i--)
        {
            Curve *pCurve = m_oaCurve.at(i);
            if(pCurve)
            {
                strong = pCurve->name();
                if(strong.compare(curveTitle, Qt::CaseInsensitive)==0) return pCurve;
            }
        }
    }
    else
    {
        for(int i=0; i<m_oaCurve.size(); i++)
        {
            Curve *pCurve = m_oaCurve.at(i);
            if(pCurve)
            {
                strong = pCurve->name();
                if(strong.compare(curveTitle, Qt::CaseInsensitive)==0) return pCurve;
            }
        }
    }

    return nullptr;
}


Curve *CurveModel::firstCurve() const
{
    if(m_oaCurve.size()) return m_oaCurve.front();
    else return nullptr;
}


Curve *CurveModel::lastCurve() const
{
    if(m_oaCurve.size()) return m_oaCurve.back();
    else return nullptr;
}


Curve* CurveModel::addCurve(AXIS::enumAxis axis, bool bDarkTheme)
{
    return addCurve(QString(), axis, bDarkTheme);
}


Curve* CurveModel::addCurve(QString curveName, AXIS::enumAxis axis, bool bDarkTheme)
{
    Curve *pCurve = new Curve();
    if(pCurve)
    {
        int nIndex = m_oaCurve.size();
        QColor clr = s_CurveColor[nIndex%s_CurveColor.size()];
        if(!bDarkTheme) clr = clr.darker();
        pCurve->setColor(clr);
        pCurve->setName(curveName);

        if(axis==AXIS::RIGHTYAXIS) pCurve->setLeftAxis(false);
        else                       pCurve->setLeftAxis(true);

        m_oaCurve.append(pCurve);
    }
    return pCurve;
}


void CurveModel::resetCurves()
{
    for(int i=0; i<m_oaCurve.size(); i++)
    {
        Curve *pCurve = m_oaCurve.at(i);
        pCurve->clear();
    }
}


void CurveModel::deleteCurves()
{
    while(m_oaCurve.size())
    {
        delete m_oaCurve.back();
        m_oaCurve.pop_back();
    }
}


void CurveModel::deleteCurve(int index)
{
    if(index>=0 && index<m_oaCurve.size())
    {
        delete m_oaCurve[index];
        m_oaCurve.removeAt(index);
    }
}

void CurveModel::deleteCurve(Curve *pCurve)
{
    for(int ic=0; ic<m_oaCurve.size(); ic++)
    {
        if(pCurve==m_oaCurve.at(ic))
        {
            delete m_oaCurve[ic];
            m_oaCurve.removeAt(ic);
            return;
        }
    }
}


void CurveModel::deleteCurve(QString CurveTitle)
{
    for(int ic=0; ic<m_oaCurve.size(); ic++)
    {
        if(m_oaCurve.at(ic)->name().compare(CurveTitle)==0)
        {
            m_oaCurve.removeAt(ic);
            return;
        }
    }
}


Curve* CurveModel::getClosestPoint(double x, double y, double &xSel, double &ySel, int &nSel) const
{
    double dist=0, x1=0, y1=0;
    double dmax = 1.e40;
    Curve *pOldCurve, *pCurveSel;
    pCurveSel = nullptr;

    for(int i=0; i<m_oaCurve.size(); i++)
    {
        pOldCurve = m_oaCurve.at(i);
        pOldCurve->closestPoint(x, y, x1, y1, dist);
        if(dist<dmax)
        {
            dmax = dist;
            xSel = x1;
            ySel = y1;
            pCurveSel = pOldCurve;
            nSel = i;
        }
    }
    return pCurveSel;
}


bool CurveModel::selectCurve(QString const & curvename)
{
    for(int ic=0; ic<curveCount(); ic++)
    {
        if(curve(ic)->name()==curvename)
        {
            m_SelectedCurve = curve(ic);

            return true;
        }
    }
    return false;
}


bool CurveModel::selectCurve(Curve const*pCurve)
{
    for(int ic=0; ic<curveCount(); ic++)
    {
        if(curve(ic)==pCurve)
        {
            m_SelectedCurve = pCurve;
            return true;
        }
    }
    return false;
}


bool CurveModel::selectCurve(int ic)
{
    if(ic<0 || ic>=curveCount()) return false;
    m_SelectedCurve = curve(ic);

    return true;
}


bool CurveModel::hasVisibleCurve() const
{
    if (curveCount())
    {
        for (int nc=0; nc<curveCount(); nc++)
        {
            Curve *pCurve = curve(nc);
            if ((pCurve->isVisible() ||pCurve->pointsVisible()) && pCurve->size()>=1)
            {
                return true;
            }
        }
    }
    return false;
}


void CurveModel::getXBounds(double &xmin, double &xmax) const
{
    bool bFound = false;
    xmin = 1.e10;
    xmax = -1.e10;
    for (int nc=0; nc<curveCount(); nc++)
    {
        Curve const* pCurve = curve(nc);
        if ((pCurve->isVisible() || pCurve->pointsVisible()) && pCurve->size()>0)
        {
            xmin = std::min(xmin, pCurve->xMin());
            xmax = std::max(xmax, pCurve->xMax());
            bFound = true;
        }
    }

    if(!bFound) xmin = xmax = 0.0;
}


void CurveModel::getYBounds(double &ymin, double &ymax, AXIS::enumAxis axis) const
{
    ymin = 1.e100;
    ymax = -1.e100;
    int iy = (axis==AXIS::RIGHTYAXIS) ? 1 : 0;

    for (int nc=0; nc<curveCount(); nc++)
    {
        Curve const*pCurve = curve(nc);
        if ((pCurve->isVisible() || pCurve->pointsVisible()) && pCurve->size()>0 && pCurve->isYAxis(iy))
        {
            ymin = std::min(ymin, pCurve->yMin());
            ymax = std::max(ymax, pCurve->yMax());
        }
    }
}



void CurveModel::getXPositiveBounds(double &xmin, double &xmax) const
{
    bool bFound = false;
    xmin = 1.e10;
    xmax = -1.e10;
    for (int nc=0; nc<curveCount(); nc++)
    {
        Curve* pCurve = curve(nc);
        if ((pCurve->isVisible() || pCurve->pointsVisible()))
        {
            for(int ip=0; ip<pCurve->count(); ip++)
            {
                if(pCurve->x(ip)>0.0)
                {
                    xmin = std::min(xmin, pCurve->x(ip));
                    xmax = std::max(xmax, pCurve->x(ip));
                    bFound = true;
                }
            }
        }
    }
    if(!bFound) xmin = xmax = 0.0;
}


void CurveModel::getYPositiveBounds(double &ymin, double &ymax) const
{
    bool bFound = false;
    ymin = 1.e10;
    ymax = -1.e10;
    for (int nc=0; nc<curveCount(); nc++)
    {
        Curve* pCurve = curve(nc);
        if ((pCurve->isVisible() || pCurve->pointsVisible()))
        {
            for(int ip=0; ip<pCurve->count(); ip++)
            {
                if(pCurve->y(ip)>0.0)
                {
                    ymin = std::min(ymin, pCurve->y(ip));
                    ymax = std::max(ymax, pCurve->y(ip));
                    bFound = true;
                }
            }
        }
    }
    if(!bFound) ymin = ymax = 0.0;
}


QColor CurveModel::color(int idx)
{
    if(idx<0 || idx>=s_CurveColor.size()) return Qt::gray;
    return s_CurveColor[idx%s_CurveColor.size()];
}
