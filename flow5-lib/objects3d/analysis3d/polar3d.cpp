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

#include <cstring>




#include <polar3d.h>
#include <constants.h>
#include <inertia.h>

Polar3d::Polar3d()
{
    setDefaults();
}


void Polar3d::setDefaults()
{
    m_PolarFormat = 200015;

    m_bLocked = false;

    m_bTrefftz        = true;
    m_bGround         = false;
    m_bFreeSurface    = false;

    m_bIgnoreBodyPanels = false;

    m_bViscous           = true;
    m_bViscOnTheFly      = false;
    m_bViscFromCl        = true;

    m_NCrit = 9.0;
    m_XTrTop = 1.0;
    m_XTrBot = 1.0;
    m_bTransAtHinge = false;

    m_BC = xfl::DIRICHLET;

    m_bVortonWake        = false;
    m_BufferWakeFactor   = 0.3;   // x MAC
    m_VortonL0           = 1.0;   // x MAC
    m_VortonCoreSize     = 1.0;   // x MAC
    m_VPWMaxLength       = 30.0;  // x MAC
    m_VPWIterations      = 35;
    m_nXWakePanel4    = 5;
    m_TotalWakeLengthFactor = 30.0;
    m_WakePanelFactor = 1.1;

    m_AnalysisMethod = xfl::QUADS;
    m_Type      = xfl::T1POLAR;

    m_Density   = 1.225;
    m_Viscosity = 1.5e-5;//m2/s

    m_bAutoInertia = true;
    m_GroundHeight = 0.0;
    m_BetaSpec  = 0.0;
    m_BankAngle = 0.0;

    m_Mass = 0.0;
    memset(m_Inertia, 0, 4*sizeof(double));
}


void Polar3d::duplicateSpec(const Polar3d *pPolar3d)
{
    m_Type = pPolar3d->m_Type;

    m_theStyle = pPolar3d->theStyle();

    m_ReferenceDim    = pPolar3d->m_ReferenceDim;

    // general aerodynamic data - specific to a polar
    m_Viscosity   = pPolar3d->viscosity();
    m_Density     = pPolar3d->density();

    m_BankAngle   = pPolar3d->m_BankAngle;
    m_BetaSpec    = pPolar3d->m_BetaSpec;

    m_nXWakePanel4          = pPolar3d->m_nXWakePanel4;
    m_TotalWakeLengthFactor = pPolar3d->m_TotalWakeLengthFactor;
    m_WakePanelFactor       = pPolar3d->m_WakePanelFactor;

    m_bVortonWake           = pPolar3d->m_bVortonWake;
    m_BufferWakeFactor      = pPolar3d->m_BufferWakeFactor;
    m_VortonL0              = pPolar3d->m_VortonL0;
    m_VortonCoreSize        = pPolar3d->m_VortonCoreSize;
    m_VPWMaxLength          = pPolar3d->m_VPWMaxLength;
    m_VPWIterations         = pPolar3d->m_VPWIterations;
    m_BC                    = pPolar3d->m_BC;

    m_bGround               = pPolar3d->m_bGround;
    m_bFreeSurface          = pPolar3d->m_bFreeSurface;
    m_GroundHeight          = pPolar3d->m_GroundHeight;

    m_bTrefftz              = pPolar3d->m_bTrefftz;
    m_bIgnoreBodyPanels     = pPolar3d->m_bIgnoreBodyPanels;

    m_AnalysisMethod        = pPolar3d->m_AnalysisMethod;

    m_bViscous              = pPolar3d->m_bViscous;
    m_bViscOnTheFly         = pPolar3d->m_bViscOnTheFly;
    m_bViscFromCl           = pPolar3d->m_bViscFromCl;

    m_NCrit                 = pPolar3d->m_NCrit;
    m_XTrTop                = pPolar3d->m_XTrTop;
    m_XTrBot                = pPolar3d->m_XTrBot;
    m_bTransAtHinge         = pPolar3d->m_bTransAtHinge;

    m_Mass = pPolar3d->m_Mass;
    m_CoG  = pPolar3d->m_CoG;
    m_Inertia[0]  = pPolar3d->m_Inertia[0];
    m_Inertia[1]  = pPolar3d->m_Inertia[1];
    m_Inertia[2]  = pPolar3d->m_Inertia[2];
    m_Inertia[3]  = pPolar3d->m_Inertia[3];

    m_ExtraDrag = pPolar3d->m_ExtraDrag;
}


bool Polar3d:: hasExtraDrag() const
{
    if(m_bAVLDrag) return true;

    for(int iex=0; iex<extraDragCount(); iex++)
    {
        if(fabs(m_ExtraDrag.at(iex).area())>PRECISION && fabs(m_ExtraDrag.at(iex).coef())>PRECISION) return true;
    }
    return false;
}


double Polar3d::extraDragTotal(double) const
{
    double extradrag=0.0;
    for(int iex=0; iex<extraDragCount(); iex++)
    {
        extradrag += m_ExtraDrag.at(iex).area() * m_ExtraDrag.at(iex).coef();
    }
    return extradrag; // N/q
}


void Polar3d::setInertia(Inertia const &inertia)
{
    m_Mass = inertia.totalMass();
    m_CoG  = inertia.CoG_t();
    m_Inertia[0]  = inertia.Ixx_t();
    m_Inertia[1]  = inertia.Iyy_t();
    m_Inertia[2]  = inertia.Izz_t();
    m_Inertia[3]  = inertia.Ixz_t();
}


/** Returns the constant part of the extra drag, in N/q */
double Polar3d::constantDrag() const
{
    double constanttdrag = 0.0;
    for(int iex=0; iex<extraDragCount(); iex++)
    {
        constanttdrag += m_ExtraDrag.at(iex).area() * m_ExtraDrag.at(iex).coef();
    }
    return constanttdrag;
}


/** Returns a profile drag value based on a spline interpolation; similar to the parabolic spline option available in AVl */
double Polar3d::AVLDrag(double CL) const
{
    if(!m_bAVLDrag) return 0.0;

    if(m_AVLSpline.ctrlPointCount()<=2) return 0.0;
    m_AVLSpline.updateSpline();
    m_AVLSpline.makeCurve();
    if(m_AVLSpline.isSingular()) return 0.0;

    if(CL<=m_AVLSpline.firstCtrlPoint().y) return m_AVLSpline.firstCtrlPoint().x;
    if(CL>=m_AVLSpline.lastCtrlPoint().y)  return m_AVLSpline.lastCtrlPoint().x;
    for(int i=1; i<m_AVLSpline.outputSize(); i++)
    {
        Vector2d const &pt0 = m_AVLSpline.outputPt(i-1);
        Vector2d const &pt1 = m_AVLSpline.outputPt(i);
        if(pt0.y<=CL && CL<pt1.y)
        {
            double CD = pt0.x + (CL-pt0.y) * (pt1.x-pt0.x)/(pt1.y-pt0.y);
            return CD;
        }
    }
    return 0.0;
}


int Polar3d::NXBufferWakePanels() const {return 3;}



