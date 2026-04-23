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



#include <splinefoil.h>
#include <geom_global.h>
#include <foil.h>


SplineFoil::SplineFoil()
{
    m_theStyle = {true, Line::SOLID, 2, fl5Color(119, 183, 83), Line::NOSYMBOL};
    m_bOutPoints   = false;
    m_bModified    = false;
    m_bCenterLine  = false;
    m_Intrados.setTheStyle(m_theStyle);

    m_bForceCloseLE = true;
    m_bForceCloseTE = true;

    m_bSymmetric = false;

    initSplineFoil();
}


SplineFoil::SplineFoil(SplineFoil const *pSF)
{
    copy(pSF);
}


/**
 * Sets the display style from the input parameters.
 * @param style the index of the style.
 * @param width the curve's width.
 * @param color te curve's color.
 */
void SplineFoil::setCurveParams(LineStyle const &ls)
{
    m_theStyle = ls;
    m_Intrados.setTheStyle(m_theStyle);
    m_Extrados.setTheStyle(m_theStyle);
}


/**
 * Initializes the SplineFoil object with stock data.
 */
void SplineFoil::initSplineFoil()
{
    m_bModified   = false;
    
    setCurveParams(m_theStyle);

    m_Extrados.clearControlPoints();
    m_Extrados.appendControlPoint(0.0 , 0.0);
    m_Extrados.appendControlPoint(0.0 , 0.00774066);
    m_Extrados.appendControlPoint(0.0306026, 0.0343829);
    m_Extrados.appendControlPoint(0.289036 , 0.0504014);
    m_Extrados.appendControlPoint(0.576000,  0.0350933);
    m_Extrados.appendControlPoint(0.736139 , 0.0269428);
    m_Extrados.appendControlPoint(1. , 0.);

    m_Intrados.clearControlPoints();
    m_Intrados.appendControlPoint(0. , 0.);
    m_Intrados.appendControlPoint(0. , -0.00774066);
    m_Intrados.appendControlPoint(0.0306026, -0.0343829);
    m_Intrados.appendControlPoint(0.289036 , -0.0504014);
    m_Intrados.appendControlPoint(0.576000,  -0.0350933);
    m_Intrados.appendControlPoint(0.736139 , -0.0269428);
    m_Intrados.appendControlPoint(1. , 0.);

    makeSplineFoil();
}


void SplineFoil::makeSplineFoil()
{
    m_Extrados.updateSpline();
    m_Extrados.makeCurve();
    m_Intrados.updateSpline();
    m_Intrados.makeCurve();
    compMidLine();
}


/**
 * Calculates the SplineFoil's mid-camber line and stores the resutls in the memeber array.
 * @return
 */
void SplineFoil::compMidLine()
{
    m_fThickness = 0.0;
    m_fCamber    = 0.0;
    m_fxCambMax  = 0.0;
    m_fxThickMax = 0.0;

    m_rpMid.resize(100);
    m_rpMid.front().x   = 0.0;
    m_rpMid.front().y   = 0.0;

    double step = 1.0/double(m_rpMid.size());

    for(uint k=0; k<m_rpMid.size(); k++)
    {
        double x = k*step;
        double yex = m_Extrados.getY(x, true);
        double yin = m_Intrados.getY(x, true);
        m_rpMid[k].x = x;
        m_rpMid[k].y = (yex+yin)/2.0;
        if(fabs(yex-yin)>m_fThickness)
        {
            m_fThickness = fabs(yex-yin);
            m_fxThickMax = x;
        }
        if(fabs(m_rpMid.at(k).y)>fabs(m_fCamber))
        {
            m_fCamber = m_rpMid.at(k).y;
            m_fxCambMax = x;
        }    
    }
    m_rpMid.back().x = 1.0;
    m_rpMid.back().y = 0.0;
}


/**
 * Initializes this SplineFoil object with the data from another.
 * @param pSF a pointer to the source SplineFoil object.
 */
void SplineFoil::copy(SplineFoil const* pSF)
{
    m_theStyle = pSF->m_theStyle;
    m_Extrados.duplicate(pSF->m_Extrados);
    m_Intrados.duplicate(pSF->m_Intrados);
    m_Extrados.updateSpline();
    m_Extrados.makeCurve();
    m_Intrados.updateSpline();
    m_Intrados.makeCurve();

    m_fCamber    = pSF->m_fCamber;
    m_fThickness = pSF->m_fThickness;
    m_fxCambMax  = pSF->m_fxCambMax; 
    m_fxThickMax = pSF->m_fxThickMax;
    m_bSymmetric  = pSF->m_bSymmetric;
}


/**
 * Exports the current SplineFoil to a Foil object.
 * @param pFoil a pointer to the existing Foil object to be loaded with the SplineFoil points.
 */
void SplineFoil::exportToFoil(Foil *pFoil) const
{
    if(!pFoil) return;

    int j = m_Extrados.outputSize();

    pFoil->resizePointArrays(m_Extrados.outputSize() + m_Intrados.outputSize()-1);
    for (int i=0; i<m_Extrados.outputSize(); i++)
    {
        pFoil->setBasePoint(j-i-1, m_Extrados.outputPt(i));
    }
    for (int i=1; i<m_Intrados.outputSize();i++)
    {
        pFoil->setBasePoint(i+j-1, m_Intrados.outputPt(i));
    }
    pFoil->applyBase();
}


bool SplineFoil::serializeXfl(QDataStream &ar, bool bIsStoring)
{
    int k(0), m(0), n(0);
    double dble(0), x(0), y(0);
    int ArchiveFormat=200002; // 200002: new LineStyle format
    QString strangename("SplineFoil");

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << strangename;

        m_theStyle.serializeXfl(ar, bIsStoring);

        ar<<m_bCenterLine << m_bOutPoints;

        ar << m_Extrados.degree() << m_Intrados.degree();
        ar << m_Extrados.outputSize() << m_Intrados.outputSize();

        ar << m_Extrados.ctrlPointCount();
        for (k=0; k<m_Extrados.ctrlPointCount(); k++)
        {
            ar << m_Extrados.controlPoint(k).x << m_Extrados.controlPoint(k).y;
        }

        ar << m_Intrados.ctrlPointCount();
        for (k=0; k<m_Intrados.ctrlPointCount(); k++)
        {
            ar << m_Intrados.controlPoint(k).x << m_Intrados.controlPoint(k).y;
        }

        if(m_bForceCloseLE) k=1; else k=0;
        ar << k;
        if(m_bForceCloseTE) k=1; else k=0;
        ar << k;
        // space allocation for the future storage of more data, without need to change the format
        n=0;
        for (int i=0; i<8; i++) ar << n;
        dble=0;
        for (int i=0; i<10; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat < 200000 || ArchiveFormat > 210000) return false;

        ar >> strangename;
        if(ArchiveFormat<200002)
        {
            m_theStyle.m_Color.serialize(ar, false);

            ar >> n; m_theStyle.setStipple(n);
            ar >> m_theStyle.m_Width;
            ar >> m_theStyle.m_bIsVisible;
        }
        else
            m_theStyle.serializeXfl(ar, bIsStoring);

        m_Intrados.setTheStyle(m_theStyle);
        m_Extrados.setTheStyle(m_theStyle);

        ar >> m_bCenterLine >> m_bOutPoints;

        ar >> m >> n;
        m_Extrados.setDegree(m);
        m_Intrados.setDegree(n);
        ar >> m >> n;
        m_Extrados.setOutputSize(m);
        m_Intrados.setOutputSize(n);

        m_Extrados.clearControlPoints();
        ar >> n;
        for (k=0; k<n;k++)
        {
            ar >> x >> y;
            m_Extrados.appendControlPoint({x, y});
        }

        m_Intrados.clearControlPoints();
        ar >> n;
        for (k=0; k<n;k++)
        {
            ar >> x >> y;
            m_Intrados.appendControlPoint({x, y});
        }

        ar >> k;
        if(k>0) m_bForceCloseLE = true; else m_bForceCloseLE=false;
        ar >> k;
        if(k>0) m_bForceCloseTE = true; else m_bForceCloseTE=false;
        // space allocation
        for (int i=0; i<8; i++) ar >> k;
        for (int i=0; i<10; i++) ar >> dble;

        m_Extrados.splineKnots();
        m_Intrados.splineKnots();

        makeSplineFoil();

    }
    m_bModified = false;
    return true;
}



/**
 * Loads or saves the data of this SplineFoil to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool SplineFoil::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    int n=0;
    int ArchiveFormat=500001;
    double dble=0;

    int nIntSpares=0;
    int nDbleSpares=0;

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        m_theStyle.serializeFl5(ar, bIsStoring);
        m_Extrados.serializeFl5(ar, bIsStoring);
        m_Intrados.serializeFl5(ar, bIsStoring);

        ar << m_bCenterLine << m_bOutPoints;
        ar << m_bSymmetric;
        ar << m_bForceCloseLE << m_bForceCloseTE;

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat < 500000 || ArchiveFormat > 510000) return false;

        m_theStyle.serializeFl5(ar, bIsStoring);
        m_Extrados.serializeFl5(ar, bIsStoring);
        m_Intrados.serializeFl5(ar, bIsStoring);

        ar >> m_bCenterLine >> m_bOutPoints;
        ar >> m_bSymmetric;
        ar >> m_bForceCloseLE >> m_bForceCloseTE;

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        makeSplineFoil();
    }
    m_bModified = false;
    return true;
}


