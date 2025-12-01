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



#define _MATH_DEFINES_DEFINED


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


bool Polar3d::serializeFl5v726(QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    int k(0), n(0);

    double dble(0.0);
    int nIntExtra(0);
    int nDbleExtra(0);

    QString strange;

    // 500001:  new fl5 format
    // 500002: changes in WPolar serialization
    // 500003: changes in WPolar serialization
    // 500004: changes in WPolar serialization
    // 500005: changes in WPolar serialization
    // 500006: added vorton wake data
    // 500007: changes in WPolar serialization
    // 500011: marked the change to beta 08
    // 500012: added buffer wake length and vorton total length
    // 500013: beta 13 - added Lift and Drag (N) to variables; not stored, but changes the variable count in WPolarExt
    // 500015: beta 15 - added the vorton core size
    // 500016: beta 15 - added the VPW length
    // 500017: beta 15 - added AVL type parabolic drag
    // 500018: beta 15 - skipped?
    // 500019: beta 15 - added VPW max iterations
    // 500020: beta 16 - stored the Vorton core size as a fraction of the reference chord
    // 500021: beta 17 - stored the AVL type controls for stability analyses
    // 500022: beta 18 - modified the format of AeroForces serialization
    // 500023: beta 18 - added ref. dimensions to BoatPolar
    // 500024: beta 19 - added windgradient spline to BoatPolar
    // 500025: beta 20 - added Ry rotations to BoatPolar
    // 500026: v7.03 - added boat speed to BoatPolar
    // 500027: v7.04 - replaced AW variables by TW variables in boat polars
    // 500028: v7.08 - forced IgnoreHullPanels=true for legacy boat polars
    // 500029: v7.21 - added free surface effect
    // 500030: v7.26 - added phi - changes the variable count in WPolarExt

    m_PolarFormat = 500030;


    if(bIsStoring)
    {
        assert(false);
    }
    else
    {
        //input the variables from the stream
        ar >> m_PolarFormat;
        if(m_PolarFormat < 500000 || m_PolarFormat>500100) return false;

        // METADATA
        ar >> strange; m_Name = strange.toStdString();

        m_theStyle.serializeFl5(ar, bIsStoring);


        // ANALYSIS METHOD
        ar >> n;
        if     (n==1) m_AnalysisMethod=xfl::LLT;
        else if(n==2) m_AnalysisMethod=xfl::VLM1;
        else if(n==3) m_AnalysisMethod=xfl::VLM2;
        else if(n==4) m_AnalysisMethod=xfl::QUADS;
        else if(n==5) m_AnalysisMethod=xfl::TRILINEAR;
        else if(n==6) m_AnalysisMethod=xfl::TRIUNIFORM;

        ar >> boolean; // m_bTiltedGeom;
        ar >> m_bTrefftz;
        ar >> boolean;
        m_BC = boolean? xfl::DIRICHLET : xfl::NEUMANN;
        ar >> m_bIgnoreBodyPanels;

        // POLAR TYPE
        ar >> n;
        if     (n==1)   m_Type=xfl::T1POLAR;
        else if(n==2)   m_Type=xfl::T2POLAR;
        else if(n==3)   m_Type=xfl::T3POLAR;
        else if(n==4)   m_Type=xfl::T4POLAR;
        else if(n==5)   m_Type=xfl::T5POLAR;
        else if(n==6)   m_Type=xfl::T6POLAR;
        else if(n==7)   m_Type=xfl::T7POLAR;
        else if(n==8 || n==100) m_Type=xfl::T8POLAR;

         // EXTRADRAGDATA
        QString strong;
        double area=0.0, coef=0.0;
        int ExtraDragCount = 0;
        ar >> ExtraDragCount;
        m_ExtraDrag.clear();
        for (int ix=0; ix<ExtraDragCount; ix++)
        {
            ar >> strong >> area >> coef;
            m_ExtraDrag.push_back({strong.toStdString(), area, coef});
        }

        //AERO DATA
        ar >> m_bGround;
        if(m_PolarFormat>=500029) ar >> m_bFreeSurface;
        ar >> m_GroundHeight >> dble;
        ar >> m_Density >> m_Viscosity;

        //ATTITUDE
        ar >> m_BetaSpec;
        ar >> m_BankAngle;

        // DEFAULT INERTIA DATA
        ar >> m_bAutoInertia;
        ar >>m_Mass;
        ar >> m_CoG.x>> m_CoG.y >> m_CoG.z;
        ar >> m_Inertia[0] >> m_Inertia[1]  >> m_Inertia[2] >> dble >> m_Inertia[3] >> dble;

        // VISCOSITY DATA
        ar >> m_bViscous;
        ar >> boolean; // formerly bViscousOnTheFly
        ar >> m_NCrit >> m_XTrTop >> m_XTrBot;

        // fix test values from legacy file
        m_NCrit=9.0;   m_XTrTop=1.0;   m_XTrBot=1.0;


        // WAKE DATA
        m_bVortonWake=false;
        ar >> k;    k ? m_bVortonWake=true : m_bVortonWake=false;
        ar >> m_nXWakePanel4 >> m_TotalWakeLengthFactor >> m_WakePanelFactor;
        ar >> k; // m_nWakeIterations; m_nWakeIterations=std::max(m_nWakeIterations, 1);


        // space allocation
        ar >> nIntExtra;
        if(nIntExtra>=1)
        {
            ar >> n;
            m_bViscFromCl = n==1 ? true : false;
            if(m_AnalysisMethod==xfl::LLT) m_bViscFromCl = true;
        }
        if(nIntExtra>=2)
        {
            // 500006 format
            ar >> n;
        }
        for(int n=2; n<nIntExtra; n++) ar >> n;

        ar >> nDbleExtra;
        if(nDbleExtra>=2)
        {
            // 500006 format
            ar >> m_VortonL0 >> dble; // m_VortonXFactor;
        }
        if(nDbleExtra>=4)
        {
            // 500012 format
            ar >> m_BufferWakeFactor >> dble; //m_VortonLengthFactor;
        }

        if(m_PolarFormat>=500015)
        {
            // 500015 format
            ar >> m_VortonCoreSize;
        }

        if(m_PolarFormat>=500016)
        {
            // 500016 format
            ar >> m_VPWMaxLength;
        }
        if(m_PolarFormat>=500019)
        {
            // 500019 format
            ar >> m_VPWIterations;
        }
    }
    return true;
}


/** v7.50 - clean slate serialization*/
bool Polar3d::serializeFl5v750(QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    int integer(0);
    double dble(0.0);
    int n(0);
    QString strange;

    // 500750: v7.50 - clean slate serialization

    m_PolarFormat = 500750;

    if(bIsStoring)
    {
        //output the variables to the stream
        ar << m_PolarFormat;

        //METADATA
        ar << QString::fromStdString(m_Name);

        m_theStyle.serializeFl5(ar, bIsStoring);


        //ANALYSIS METHOD
        if     (m_AnalysisMethod==xfl::LLT)        ar<<1;
        else if(m_AnalysisMethod==xfl::VLM1)       ar<<2;
        else if(m_AnalysisMethod==xfl::VLM2)       ar<<3;
        else if(m_AnalysisMethod==xfl::QUADS)      ar<<4;
        else if(m_AnalysisMethod==xfl::TRILINEAR)  ar<<5;
        else if(m_AnalysisMethod==xfl::TRIUNIFORM) ar<<6;
        else                                           ar<<0;

        ar << m_bTrefftz;
        ar << (m_BC==xfl::DIRICHLET);
        ar << m_bIgnoreBodyPanels;

        //POLAR TYPE
        if     (m_Type==xfl::T1POLAR) ar<<1;
        else if(m_Type==xfl::T2POLAR) ar<<2;
        else if(m_Type==xfl::T3POLAR) ar<<3;
        else if(m_Type==xfl::T4POLAR) ar<<4;
        else if(m_Type==xfl::T5POLAR) ar<<5;
        else if(m_Type==xfl::T6POLAR) ar<<6;
        else if(m_Type==xfl::T7POLAR) ar<<7;
        else if(m_Type==xfl::T8POLAR) ar<<8;
        else ar << 0;

        // EXTRADRAGDATA
        int nDrag = int(extraDragCount());
        ar << nDrag;
        for (int ix=0; ix<nDrag; ix++)
            ar << QString::fromStdString(m_ExtraDrag.at(ix).name()) << m_ExtraDrag.at(ix).area() << m_ExtraDrag.at(ix).coef();


        // AERO DATA
        ar << m_bGround << m_bFreeSurface;
        ar << m_GroundHeight << dble;
        ar << m_Density << m_Viscosity;

        //ATTITUDE
        ar << m_BetaSpec;
        ar << m_BankAngle;


        // DEFAULT INERTIA
        ar << m_bAutoInertia;
        ar << m_Mass;
        ar << m_CoG.x<< m_CoG.y << m_CoG.z;
        ar << m_Inertia[0] << m_Inertia[1]  << m_Inertia[2]  << dble << m_Inertia[3] << dble;

        // VISCOSITY DATA
        ar << m_bViscous;
        ar << m_bViscOnTheFly;
        ar << m_NCrit << m_XTrTop << m_XTrBot;

        // WAKE DATA
        ar << m_bVortonWake;
        ar << m_nXWakePanel4 << m_TotalWakeLengthFactor << m_WakePanelFactor;

        ar << m_bViscFromCl;
        ar << m_VortonL0;
        ar << m_BufferWakeFactor;
        ar << m_VortonCoreSize;
        ar << m_VPWMaxLength;
        ar << m_VPWIterations;

        // converted v7.54
        ar << m_bTransAtHinge;

        // provisions for future variable saves
        for(int i=1; i<10; i++) ar <<boolean;
        for(int i=0; i<20; i++) ar <<integer;
        for(int i=0; i<20; i++) ar <<dble;

        return true;
    }
    else
    {
        //input the variables from the stream
        ar >> m_PolarFormat;
        if(m_PolarFormat < 500750 || m_PolarFormat>501000) return false; // failsafe

        // METADATA
        ar >> strange;   m_Name = strange.toStdString();

        m_theStyle.serializeFl5(ar, bIsStoring);

        // ANALYSIS METHOD
        ar >> n;
        if     (n==1) m_AnalysisMethod=xfl::LLT;
        else if(n==2) m_AnalysisMethod=xfl::VLM1;
        else if(n==3) m_AnalysisMethod=xfl::VLM2;
        else if(n==4) m_AnalysisMethod=xfl::QUADS;
        else if(n==5) m_AnalysisMethod=xfl::TRILINEAR;
        else if(n==6) m_AnalysisMethod=xfl::TRIUNIFORM;


        ar >> m_bTrefftz;

        ar >> boolean;
        m_BC = boolean ? xfl::DIRICHLET : xfl::NEUMANN;

        ar >> m_bIgnoreBodyPanels;

        // POLAR TYPE
        ar >> n;
        if     (n==1)   m_Type=xfl::T1POLAR;
        else if(n==2)   m_Type=xfl::T2POLAR;
        else if(n==3)   m_Type=xfl::T3POLAR;
        else if(n==4)   m_Type=xfl::T4POLAR;
        else if(n==5)   m_Type=xfl::T5POLAR;
        else if(n==6)   m_Type=xfl::T6POLAR;
        else if(n==7)   m_Type=xfl::T7POLAR;
        else if(n==8)   m_Type=xfl::T8POLAR;

        // EXTRADRAGDATA
        int nDrag = int(extraDragCount());
        ar >> nDrag;
        m_ExtraDrag.resize(nDrag);
        for (int ix=0; ix<nDrag; ix++)
        {
            ar >> strange;         m_ExtraDrag[ix].m_Tag = strange.toStdString();
            ar >> m_ExtraDrag[ix].m_Area >> m_ExtraDrag[ix].m_Coef;
        }

        // AERO DATA
        ar >> m_bGround >> m_bFreeSurface;
        ar >> m_GroundHeight >> dble;
        ar >> m_Density >> m_Viscosity;

        //ATTITUDE
        ar >> m_BetaSpec;
        ar >> m_BankAngle;


        // DEFAULT INERTIA
        ar >> m_bAutoInertia;
        ar >> m_Mass;
        ar >> m_CoG.x>> m_CoG.y >> m_CoG.z;
        ar >> m_Inertia[0] >> m_Inertia[1]  >> m_Inertia[2]  >> dble >> m_Inertia[3] >> dble;

        // VISCOSITY DATA
        ar >> m_bViscous;
        ar >> m_bViscOnTheFly;
        ar >> m_NCrit >> m_XTrTop >> m_XTrBot;

        // WAKE DATA
        ar >> m_bVortonWake;
        ar >> m_nXWakePanel4 >> m_TotalWakeLengthFactor >> m_WakePanelFactor;

        ar >> m_bViscFromCl;
        ar >> m_VortonL0;
        ar >> m_BufferWakeFactor;
        ar >> m_VortonCoreSize;
        ar >> m_VPWMaxLength;
        ar >> m_VPWIterations;


        // converted v7.54
        ar >> m_bTransAtHinge;

        // provisions for future variable saves
        for(int i=1; i<10; i++) ar >> boolean;
        for(int i=0; i<20; i++) ar >> integer;
        for(int i=0; i<20; i++) ar >> dble;

        return true;
    }
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



