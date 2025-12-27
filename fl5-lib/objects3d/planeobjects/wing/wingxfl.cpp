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

#include <QString>
#include <QTextStream>


#include <wingxfl.h>

#include <foil.h>
#include <geom_global.h>
#include <objects2d.h>
#include <objects_global.h>
#include <panel3.h>
#include <panel4.h>
#include <planeopp.h>
#include <polar.h>
#include <surface.h>
#include <triangle3d.h>
#include <units.h>
#include <utils.h>
#include <planepolar.h>

double WingXfl::s_MinSurfaceLength = 0.0001;


WingXfl::WingXfl(xfl::enumType type) : Part()
{

    m_nPanel3 = m_nPanel4 = 0;
    m_nNodes = 0;

    m_MAChord    = 0.0;// mean aero chord
    m_ProjectedArea = 0.0;
    m_ProjectedSpan = 0.0;

    m_WingType    = type;
    m_NStation = 0;
    m_nTipStrips = 1;
    m_bTwoSided       = true;
    m_bSymmetric       = true;
    m_bCloseInnerSide = false;

    m_nFlaps =  0;

    switch (m_WingType)
    {
        default:
        case xfl::Main:     makeDefaultWing();  break;
        case xfl::Elevator: makeDefaultStab();  break;
        case xfl::Fin:      makeDefaultFin();   break;
    }
}


/** This path will lead us to destruction */
WingXfl::~WingXfl()
{
    m_Section.clear();
    clearSurfaces();
}



void WingXfl::makeDefaultWing()
{
    m_WingType = xfl::Main;
    m_Section.clear();
    appendWingSection(.300, 0.0, 0.0, 0.0, 0.000, 13, 19, xfl::TANH, xfl::INV_EXP, "", "");
    appendWingSection(.190, 0.0, 1.5, 0.0, 0.085, 13, 19, xfl::TANH, xfl::INV_EXP, "", "");
    m_nTipStrips = 1;
}


void WingXfl::makeDefaultStab()
{
    //make default wing
    m_WingType = xfl::Elevator;
    m_Section.clear();
    appendWingSection(0.200, 0.0, 0.00, 0.0, 0.00, 5, 5, xfl::TANH, xfl::INV_EXP, "", "");
    appendWingSection(0.110, 0.0, 0.32, 0.0, 0.05, 5, 5, xfl::TANH, xfl::INV_EXP, "", "");
    m_nTipStrips = 1;
}


void WingXfl::makeDefaultFin()
{
    m_WingType = xfl::Fin;
    m_bTwoSided = false;
    m_bCloseInnerSide = true;
    m_Section.clear();
    appendWingSection(0.200, 0.0, 0.00, 0.0, 0.00, 5, 5, xfl::TANH, xfl::TANH, "", "");
    appendWingSection(0.120, 0.0, 0.29, 0.0, 0.08, 5, 5, xfl::TANH, xfl::TANH, "", "");
    m_nTipStrips = 1;
}


void WingXfl::computeGeometry()
{
    Foil const*pFoilA=nullptr, *pFoilB=nullptr;
    double MinPanelSize(0);

    double surface = 0.0;
    double xysurface = 0.0;
    setSectionLength(0, 0.0);
    setYProj(0, yPosition(0));
    for (int is=1; is<nSections(); is++)
        setSectionLength(is, yPosition(is) - yPosition(is-1));
    for (int is=1; is<nSections(); is++)
    {
        setYProj(is, yProj(is-1) + sectionLength(is) * cos(dihedral(is-1)*PI/180.0));
    }

    m_PlanformSpan  = tipPos();
    m_ProjectedSpan = 0.0;
    m_MAChord = 0.0;

    for (int is=0; is<nSections()-1; is++)
    {
        pFoilA = Objects2d::foil(rightFoilName(is));
        pFoilB = Objects2d::foil(rightFoilName(is+1));
        surface   += sectionLength(is+1)*(chord(is)+chord(is+1))/2.0;//m2
        xysurface += (sectionLength(is+1)*(chord(is)+chord(is+1))/2.0)*cos(dihedral(is)*PI/180.0);
        m_ProjectedSpan += sectionLength(is+1)*cos(dihedral(is)*PI/180.0);

        m_MAChord += integralC2(yPosition(is), yPosition(is+1), chord(is), chord(is+1));
    }

    if(m_bTwoSided)
    {
        m_PlanformSpan  *=2.0;
        m_ProjectedSpan *=2.0;

        m_PlanformArea  = 2.0 * surface;
        m_ProjectedArea = 2.0 * xysurface;

        m_MAChord = m_MAChord * 2.0 / m_PlanformArea;
    }
    else
    {
        m_PlanformArea  = surface;
        m_ProjectedArea = xysurface;
        m_MAChord = m_MAChord / m_PlanformArea;
    }

    //calculate the number of flaps
    m_nFlaps = 0;
    if(s_MinSurfaceLength>0.0) MinPanelSize = s_MinSurfaceLength;
    else                       MinPanelSize = m_PlanformSpan;

    if(!pFoilA || !pFoilB) return;
    for (int is=1; is<nSections(); is++)
    {
        pFoilA = Objects2d::foil(rightFoilName(is-1));
        pFoilB = Objects2d::foil(rightFoilName(is));
        if(pFoilA && pFoilB && m_bTwoSided)
        {
            if(pFoilA->hasTEFlap() && pFoilB->hasTEFlap() && fabs(yPosition(is)-yPosition(is-1))>MinPanelSize)    m_nFlaps++;
        }
        pFoilA = Objects2d::foil(leftFoilName(is-1));
        pFoilB = Objects2d::foil(leftFoilName(is));
        if(pFoilA && pFoilB)
        {
            if(pFoilA->hasTEFlap() && pFoilB->hasTEFlap() && fabs(yPosition(is)-yPosition(is-1))>MinPanelSize)    m_nFlaps++;
        }
    }
}


int WingXfl::makeTriPanels(int ip3start, int indStart, bool bThickSurfaces)
{
    m_StripStartNodes.clear();

    m_TriMesh.clearMesh();

    setFirstPanel3Index(ip3start);
    setFirstNodeIndex(indStart);

    int nWingPanels=0;
    int leftnodeidx=-1, rightnodeidx=-1;
    for(int jSurf=0; jSurf<nSurfaces(); jSurf++)
    {
        Surface &surf = m_Surface[jSurf];
        surf.makeTriPanels(m_TriMesh.panels(), m_TriMesh.nodes(), ip3start, bThickSurfaces, m_nTipStrips, leftnodeidx, rightnodeidx);

        if(std::find(m_StripStartNodes.begin(), m_StripStartNodes.end(), leftnodeidx) == m_StripStartNodes.end())
            m_StripStartNodes.push_back(leftnodeidx);
        if(std::find(m_StripStartNodes.begin(), m_StripStartNodes.end(), rightnodeidx) == m_StripStartNodes.end())
            m_StripStartNodes.push_back(rightnodeidx);

        nWingPanels += int(surf.panel3list().size());
        ip3start += int(surf.panel3list().size());
    }

    m_nPanel3 = nWingPanels;
    m_nNodes = m_TriMesh.nNodes()-m_FirstNodeIndex;

    return nWingPanels;
}


#define NXSTATIONS 20
#define NYSTATIONS 40

/**
 * Calculates and returns the inertia properties of the structure based on the object's mass
 * and on the existing geometry
 * The mass is assumed to have been set previously.
 */
void WingXfl::computeStructuralInertia(const Vector3d &PartPosition)
{
    if(!m_bAutoInertia) return;

    std::vector<double> ElemVolume(NXSTATIONS*NYSTATIONS*nSurfaces());
    std::vector<Vector3d> PtVolume(NXSTATIONS*NYSTATIONS*nSurfaces());

    double rho{0}, LocalSpan{0};
    double LocalVolume{0};
    double LocalChord{0},  LocalArea{0},  tau{0};
    double LocalChord1{0}, LocalArea1{0}, tau1{0};
    double xrel{0}, xrel1{0}, yrel{0}, ElemArea{0};
    Vector3d ATop, ABot, CTop, CBot, PointNormal, Diag1, Diag2;
    Vector3d PtC4, Pt, Pt1, N;

    Vector3d StructCoG;
    StructCoG.set(0.0, 0.0, 0.0);
    double CoGIxx_vol(0.0), CoGIyy_vol(0.0), CoGIzz_vol(0.0), CoGIxz_vol(0.0);

    //sanity check
    double CoGIxxCheck(0), CoGIyyCheck(0), CoGIzzCheck(0), CoGIxzCheck(0);
    double recalcMass(0.0);
    double recalcVolume(0.0);
    double checkVolume(0.0);


    //the mass density is assumed to be homogeneous
    //the local mass is proportional to the chord x foil area
    //the foil's area is interpolated
    //we consider the whole wing, i.e. all left and right surfaces
    //note : in avl documentation, each side is considered separately
    //first get the CoG - necessary for future application of Huygens/Steiner theorem
    int p = 0;

    for (int j=0; j<nSurfaces(); j++)
    {
        Surface const &surf = m_Surface.at(j);
        LocalSpan = surf.m_Length/double(NYSTATIONS);
        for (int k=0; k<NYSTATIONS; k++)
        {
            tau  = double(k)   / double(NYSTATIONS);
            tau1 = double(k+1) / double(NYSTATIONS);
            yrel = (tau+tau1)/2.0;

            surf.getSection(tau,  LocalChord,  LocalArea,  Pt);
            surf.getSection(tau1, LocalChord1, LocalArea1, Pt1);
//            LocalVolume = (LocalArea+LocalArea1)/2.0 * LocalSpan;
            PtC4.x = (Pt.x + Pt1.x)/2.0;
            PtC4.y = (Pt.y + Pt1.y)/2.0;
            PtC4.z = (Pt.z + Pt1.z)/2.0;

//            CoGCheck += LocalVolume * PtC4;
            for(int l=0; l<NXSTATIONS; l++)
            {
                //browse mid-section
                xrel  = 1.0 - 1.0/2.0 * (1.0-cos(double(l)  *PI /double(NXSTATIONS)));
                xrel1 = 1.0 - 1.0/2.0 * (1.0-cos(double(l+1)*PI /double(NXSTATIONS)));

                surf.getSurfacePoint(xrel,  xrel, yrel,  xfl::TOPSURFACE, ATop, N);
                surf.getSurfacePoint(xrel,  xrel, yrel,  xfl::BOTSURFACE, ABot, N);
                surf.getSurfacePoint(xrel1, xrel1, yrel, xfl::TOPSURFACE, CTop, N);
                surf.getSurfacePoint(xrel1, xrel1, yrel, xfl::BOTSURFACE, CBot, N);
                PtVolume[p] = (ATop+ABot+CTop+CBot)/4.0;
                Diag1 = ATop - CBot;
                Diag2 = ABot - CTop;
                PointNormal = Diag1 * Diag2;

                ElemArea = PointNormal.norm()/2.0;
                if(ElemArea>0.0) ElemVolume[p] = ElemArea * LocalSpan;
                else
                {
                    //no area, means that the foils have not yet been defined for this surface
                    // so just count a unit volume, temporary
                    ElemVolume[p] = 1.0;

                }
                checkVolume += ElemVolume[p];
                StructCoG.x += ElemVolume[p] * PtVolume[p].x;
                StructCoG.y += ElemVolume[p] * PtVolume[p].y;
                StructCoG.z += ElemVolume[p] * PtVolume[p].z;
                p++;
            }
        }
    }

    if(checkVolume>0.0)  rho = m_Inertia.structuralMass()/checkVolume;
    else                 rho = 0.0;

    if(checkVolume>0.0)  StructCoG *= 1.0/ checkVolume;
    else                 StructCoG.set(0.0, 0.0, 0.0);


    // CoG is the new origin for inertia calculation
    p=0;
    for (int j=0; j<nSurfaces(); j++)
    {
        Surface const &surf = m_Surface.at(j);
        LocalSpan = surf.m_Length/double(NYSTATIONS);
        for (int k=0; k<NYSTATIONS; k++)
        {
            tau  = double(k)   / double(NYSTATIONS);
            tau1 = double(k+1) / double(NYSTATIONS);
            surf.getSection(tau,  LocalChord,  LocalArea,  Pt);
            surf.getSection(tau1, LocalChord1, LocalArea1, Pt1);

            LocalVolume = (LocalArea+LocalArea1)/2.0 * LocalSpan;

            PtC4.x = (Pt.x + Pt1.x)/2.0;
            PtC4.y = (Pt.y + Pt1.y)/2.0;
            PtC4.z = (Pt.z + Pt1.z)/2.0;

            CoGIxxCheck += LocalVolume*rho * ( (PtC4.y-StructCoG.y)*(PtC4.y-StructCoG.y) + (PtC4.z-StructCoG.z)*(PtC4.z-StructCoG.z) );
            CoGIyyCheck += LocalVolume*rho * ( (PtC4.x-StructCoG.x)*(PtC4.x-StructCoG.x) + (PtC4.z-StructCoG.z)*(PtC4.z-StructCoG.z) );
            CoGIzzCheck += LocalVolume*rho * ( (PtC4.x-StructCoG.x)*(PtC4.x-StructCoG.x) + (PtC4.y-StructCoG.y)*(PtC4.y-StructCoG.y) );
            CoGIxzCheck += LocalVolume*rho * ( (PtC4.x-StructCoG.x)*(PtC4.z-StructCoG.z) );

            recalcMass   += LocalVolume*rho;
            recalcVolume += LocalVolume;

            for(int l=0; l<NXSTATIONS; l++)
            {
                //browse mid-section
                CoGIxx_vol += ElemVolume[p]*rho * ( (PtVolume[p].y-StructCoG.y)*(PtVolume[p].y-StructCoG.y) + (PtVolume[p].z-StructCoG.z)*(PtVolume[p].z-StructCoG.z));
                CoGIyy_vol += ElemVolume[p]*rho * ( (PtVolume[p].x-StructCoG.x)*(PtVolume[p].x-StructCoG.x) + (PtVolume[p].z-StructCoG.z)*(PtVolume[p].z-StructCoG.z));
                CoGIzz_vol += ElemVolume[p]*rho * ( (PtVolume[p].x-StructCoG.x)*(PtVolume[p].x-StructCoG.x) + (PtVolume[p].y-StructCoG.y)*(PtVolume[p].y-StructCoG.y));
                CoGIxz_vol += ElemVolume[p]*rho * ( (PtVolume[p].x-StructCoG.x)*(PtVolume[p].z-StructCoG.z) );
                p++;
            }
        }
    }

    m_Inertia.setCoG_s(StructCoG-PartPosition);
    m_Inertia.setIxx_s(CoGIxx_vol);
    m_Inertia.setIyy_s(CoGIyy_vol);
    m_Inertia.setIzz_s(CoGIzz_vol);
    m_Inertia.setIxz_s(CoGIxz_vol);
    (void)recalcMass;
    (void)CoGIxxCheck;
    (void)CoGIyyCheck;
    (void)CoGIzzCheck;
    (void)CoGIxzCheck;
    (void)recalcMass;
    (void)recalcVolume;
}


int WingXfl::nYPanels() const
{
    double MinPanelSize=0;

    if(s_MinSurfaceLength>0.0) MinPanelSize = s_MinSurfaceLength;
    else                       MinPanelSize = 0.0;

    int ny = 0;
    for(int is=0; is<nSections()-1;is++)
    {
        double panelLength = fabs(yPosition(is)-yPosition(is+1));

        if (panelLength < MinPanelSize || panelLength<planformSpan()/1000.0/2.0)
        {
        }
        else
        {
            ny += m_Section.at(is).m_NYPanels;
        }
    }
    if(isTwoSided()) return ny*2;
    else             return ny;
}


void WingXfl::createSurfaces(Vector3d const &Toffset, double XTilt, double YTilt)
{
    if(nSections()<2) return; // something went wrong

    Vector3d PLA, PTA, PLB, PTB, T1;
    Vector3d O(0.0,0.0,0.0);

    std::vector<Vector3d> VNormal(nSections());
    std::vector<Vector3d> VNSide(nSections());

    double MinPanelSize=0.0;
    if(s_MinSurfaceLength>0.0) MinPanelSize = s_MinSurfaceLength;
    else                       MinPanelSize = 0.0;

    //define the normal to each surface
    int nSurf=0;
    VNormal[0].set(0.0, 0.0, 1.0);
    VNSide[0].set(0.0, 0.0, 1.0);

    for(int is=0; is<nSections()-1; is++)
    {
        double panelLength = fabs(yPosition(is)-yPosition(is+1));

        if (panelLength < MinPanelSize)
        {
        }
        else
        {
            VNormal[nSurf].set(0.0, 0.0, 1.0);
            VNormal[nSurf].rotateX(O, dihedral(is));
            nSurf++;
        }
    }

    clearSurfaces(); // to make sure that all the data is reset
    if(m_bTwoSided) m_Surface.resize(2*nSurf);
    else            m_Surface.resize(nSurf);

    if(nSurf<=0) return;
    int NSurfaces = nSurf;

    for(uint jsurf=0; jsurf<m_Surface.size(); jsurf++) m_Surface[jsurf].setIndex(jsurf);

    //define the normals at panel junctions: average between the normals of the two connecting sufaces
    for(int jsurf=0; jsurf<nSurf; jsurf++)
    {
        VNSide[jsurf+1] = VNormal[jsurf]+VNormal[jsurf+1];
        VNSide[jsurf+1].normalize();
    }

    //we start with the center panel, moving towards the left wing tip
    //however, the calculations are written for surfaces ordered from left tip to right tip,
    //so we number them the opposite way
    nSurf = 0;
    int iSurf = NSurfaces-1;


    for(int jsurf=0; jsurf<nSections()-1; jsurf++)
    {
        double panelLength = fabs(yPosition(jsurf)-yPosition(jsurf+1));
        if (panelLength < MinPanelSize)
        {
        }
        else
        {
            Surface &surf = m_Surface[iSurf];
            surf.m_FoilNameA = leftFoilName(jsurf+1);
            surf.m_FoilNameB = leftFoilName(jsurf);

            surf.m_Length   =  yPosition(jsurf+1) - yPosition(jsurf);

            PLA.x =  offset(jsurf+1);         PLB.x =  offset(jsurf);
            PLA.y = -yPosition(jsurf+1);      PLB.y = -yPosition(jsurf);
            PLA.z =  0.0;                   PLB.z =  0.0;
            PTA.x =  PLA.x+chord(jsurf+1);    PTB.x = PLB.x+chord(jsurf);
            PTA.y =  PLA.y;                    PTB.y = PLB.y;
            PTA.z =  0.0;                   PTB.z =  0.0;

            surf.setCornerPoints(PLA, PTA, PLB, PTB);
            surf.setNormal(); // is (0,0,1)

            surf.rotateX(surf.m_LB, -dihedral(jsurf));
            surf.m_NormalA.set(VNSide[nSurf+1].x, -VNSide[nSurf+1].y, VNSide[nSurf+1].z);
            surf.m_NormalB.set(VNSide[nSurf].x,   -VNSide[nSurf].y,   VNSide[nSurf].z);

            surf.m_TwistA = twist(jsurf+1);
            surf.m_TwistB = twist(jsurf);
            surf.setTwist(true);

            if(jsurf>0 && iSurf<nSurfaces()-1)
            {
                //translate the surface to the left tip of the previous surface
                T1 = m_Surface[iSurf+1].m_LA - surf.m_LB;
                surf.translate(0.0,T1.y,T1.z);
                //                m_Surface[is].m_LB = m_Surface[is+1].m_LA;
                //                m_Surface[is].m_TB = m_Surface[is+1].m_TA;
            }

            nSurf++;

            surf.setNXPanels(nXPanels(jsurf));
            surf.setNYPanels(nYPanels(jsurf));


            //AVL coding + invert XFLR5::SINE and -sine for left wing
            surf.m_XDistType = xPanelDist(jsurf);
            if     (yPanelDist(jsurf) == xfl::SINE)      surf.m_YDistType = xfl::INV_SINE;
            else if(yPanelDist(jsurf) == xfl::COSINE)    surf.m_YDistType = xfl::COSINE;
            else if(yPanelDist(jsurf) == xfl::INV_SINE)  surf.m_YDistType = xfl::SINE;
            else if(yPanelDist(jsurf) == xfl::INV_SINH)  surf.m_YDistType = xfl::INV_SINH;
            else if(yPanelDist(jsurf) == xfl::TANH)      surf.m_YDistType = xfl::TANH;
            else if(yPanelDist(jsurf) == xfl::EXP)       surf.m_YDistType = xfl::INV_EXP;
            else if(yPanelDist(jsurf) == xfl::INV_EXP)   surf.m_YDistType = xfl::EXP;
            else                                       surf.m_YDistType = xfl::UNIFORM;

//            surf.createXPoints();
            surf.setFlap();
            surf.init();
            surf.setLeftSurf(true);
            surf.setIsInSymPlane(false);
            surf.m_InnerSection = jsurf;
            surf.m_OuterSection = jsurf+1;
            --iSurf;
        }
    }
    m_Surface[NSurfaces-1].setCenterSurf(true);//previous left center surface

    //  if there is a center gap, declare the left center surf as a right tip
//    if(fabs(yPosition(0))>PRECISION) m_Surface[NSurfaces-1].setTipRight(true);

    if(m_bTwoSided)
    {
        m_Surface[NSurfaces].setCenterSurf(true);//next right center surface
        iSurf = nSurf;
        for (int jsurf=0; jsurf<nSections()-1; jsurf++)
        {
            double panelLength = fabs(yPosition(jsurf)-yPosition(jsurf+1));
            if (panelLength < MinPanelSize)
            {
            }
            else
            {
                Surface &surf = m_Surface[iSurf];
                surf.m_FoilNameA = rightFoilName(jsurf);
                surf.m_FoilNameB = rightFoilName(jsurf+1);

                surf.m_Length   =  yPosition(jsurf+1) - yPosition(jsurf);

                PLA.x = offset(jsurf);        PLB.x = offset(jsurf+1);
                PLA.y = yPosition(jsurf);     PLB.y = yPosition(jsurf+1);
                PLA.z = 0.0;                PLB.z = 0.0;
                PTA.x = PLA.x+chord(jsurf);   PTB.x = PLB.x+chord(jsurf+1);
                PTA.y = PLA.y;              PTB.y = PLB.y;
                PTA.z = 0.0;                PTB.z = 0.0;

                surf.setCornerPoints(PLA, PTA, PLB, PTB);
                surf.setNormal(); // is (0,0,1)

                surf.rotateX(m_Surface[iSurf].m_LA, dihedral(jsurf));
                surf.m_NormalA.set(VNSide[iSurf-nSurf].x,   VNSide[iSurf-nSurf].y,   VNSide[iSurf-nSurf].z);
                surf.m_NormalB.set(VNSide[iSurf-nSurf+1].x, VNSide[iSurf-nSurf+1].y, VNSide[iSurf-nSurf+1].z);

                surf.m_TwistA   =  twist(jsurf);
                surf.m_TwistB   =  twist(jsurf+1);
                surf.setTwist(true);

                if(jsurf>0 && iSurf>0)
                {
                    //translate the surface to the left tip of the previous surface and merge points
                    T1 = m_Surface[iSurf-1].m_LB -surf.m_LA;
                    surf.translate(0.0, T1.y, T1.z);
                    //                    m_Surface[is].m_LA = m_Surface[is-1].m_LB;
                    //                    m_Surface[is].m_TA = m_Surface[is-1].m_TB;
                }

                surf.setNXPanels(nXPanels(jsurf));
                surf.setNYPanels(nYPanels(jsurf));

                //AVL coding + invert XFLR5::SINE and -sine for left wing
                surf.m_XDistType = xPanelDist(jsurf);
                surf.m_YDistType = yPanelDist(jsurf);

//                surf.createXPoints();
                surf.setFlap();
                surf.init();
                surf.m_bIsLeftSurf   = false;
                surf.m_bIsRightSurf  = true;
                surf.m_bIsInSymPlane = false;

                surf.m_InnerSection = jsurf;
                surf.m_OuterSection = jsurf+1;

                iSurf++;
            }
        }

        // if there is a center gap, declare the right center surf as a left tip
//        if(fabs(yPosition(0))>PRECISION) m_Surface[NSurfaces].setTipLeft(true);
    }

    Vector3d Or(0.0,0.0,0.0);
    if(m_bTwoSided)
    {
        NSurfaces *= 2;
        for (int jSurf=0; jSurf<NSurfaces; jSurf++)
        {
            Surface &surf = m_Surface[jSurf];
            surf.rotateX(Or, XTilt);
            surf.rotateY(Or, YTilt);
            surf.translate(Toffset);
        }
//        m_Surface[NSurfaces-1].setTipRight(true);
    }
    else
    {
        for (int jSurf=0; jSurf<NSurfaces; jSurf++)
        {
            Surface &surf = m_Surface[jSurf];
            surf.rotateX(Or, XTilt);
            surf.rotateZ(Or, YTilt);
            surf.translate(Toffset);
            surf.setLeftSurf(true);
            surf.setRightSurf(false);
            surf.setIsInSymPlane(true);
        }
    }

    m_Surface.front().setTipLeft(true);
    m_Surface.back().setTipRight(true);
    m_Surface.front().setClosedLeftSide(true);

    if(m_bTwoSided)
    {
        if(m_bCloseInnerSide)
        {
            m_Surface[NSurfaces/2-1].setClosedRightSide(true);
            m_Surface[NSurfaces/2].setClosedLeftSide(true);
        }
        m_Surface.back().setClosedRightSide(true);
    }
    else
    {
        if(m_bCloseInnerSide) m_Surface.back().setClosedRightSide(true);
    }

    if(NSurfaces>1) m_Surface[int(NSurfaces/2)-1].m_bJoinRight = true;
    //check for a center gap greater than 1/10mm

    if(m_bTwoSided && yPosition(0)>0.0001)
    {
        int is = int(NSurfaces/2)-1;
        if(is>=0 && is<nSurfaces())
            m_Surface[is].m_bJoinRight   = false;
    }

    m_Surface.back().m_bJoinRight = false;

    if(!m_bTwoSided) m_Surface.back().m_bJoinRight   = false;

    createXPoints();
}


bool WingXfl::hasCenterGap() const
{
    return fabs(yPosition(0))>0.0001;
}


/** Connects the inner surfaces. This cannot be done when the surfaces are constructed,
 * because the fuselage may or may not separate completely the surfaces */
bool WingXfl::connectInnerSurfaces(std::vector<Panel3> &panels, bool bThickSurfaces)
{
    int coef = bThickSurfaces ? 2 : 1;
    for(int is=0; is<nSurfaces()-1; is++)
    {
        Surface const &sl = m_Surface.at(is);
        Surface const &sr = m_Surface.at(is+1);
        if(!sl.isCenterSurf() || !sl.isLeftSurf() || !sr.isCenterSurf() || !sr.isRightSurf())
            continue;

        if(sl.isClosedRightSide() || sr.isClosedLeftSide())
        {
            // case of a center gap, do not connect anything
            return true;
        }

        // connect the two surfaces;
        // this is done by setting the right side panels' node indexes to those of the left side panels;
        // 1 or two vertices are assigned for each right side panel
        // leaves some nodes of the right surface dangling, but they can be kept unused in the array
        for(int ir=0; ir<sr.NXPanels()*2*coef; ir++)  // top and bottom * 2 triangles
        {
            int nr = sr.m_FirstStripPanelIndex + ir;
            if(nr<0 || nr>=int(panels.size()))
            {
                return false;
            }
            Panel3 &pr = panels[nr];
            for(int il=0; il<sl.NXPanels()*2*coef; il++)  // top and bottom * 2 triangles
            {
                int nl = sl.m_LastStripPanelIndex + il;
                Panel3 &pl = panels[nl];
                if(nl<0 || nl>=int(panels.size()))
                {
                    return false;
                }
                if(pl.surfacePosition()==pr.surfacePosition())
                {
                    //connect only same top bot mid positions
                    int nMerged = 0;
                    int vl[]{-1,-1}; // identify the vertices being merged
                    int vr[]{-1,-1};

                    for(int ivr=0; ivr<3; ivr++)
                    {
                        for(int ivl=0; ivl<3; ivl++)
                        {
                            if(pr.vertexAt(ivr).isSame(pl.vertexAt(ivl), 0.0005))
                            {
                                pr.setVertex(ivr, pl.vertexAt(ivl));// could also only set the index
                                vl[nMerged]=ivl;
                                vr[nMerged]=ivr;
                                nMerged++;
                            }
                        }
                    }

                    if(ir==0 && bThickSurfaces)
                    {
                        m_StripStartNodes[is+2] = pr.vertexAt(2).index();
                    }

                    if(nMerged==2)
                    {
                        // two common vertices, the panels are neighbours
                        // identify the indexes of the common edges to assign neighbours
                        int iEdgeL = 3-vl[0]-vl[1];
                        int iEdgeR = 3-vr[0]-vr[1];

                        pl.setNeighbour(nr, iEdgeL);
                        pr.setNeighbour(nl, iEdgeR);
                        break; // done; don't try to merge pr again, for instance with top or bot surface
                    }
                }
            }
            return true;
        }
    }
    return true;
}


bool WingXfl::connectSurfaceToNext(int iSurf, std::vector<Panel3> &panels, bool bConnectFlaps, bool bThickSurfaces)
{
    int coef = bThickSurfaces ? 2 : 1;

    if(iSurf<0 || iSurf+1>=nSurfaces()) return false; // error

    Surface const &sl = m_Surface.at(iSurf);
    Surface const &sr = m_Surface.at(iSurf +1);
    if(sl.isClosedRightSide() || sr.isClosedLeftSide())
    {
        // case of a center gap, do not connect anything
        return true;
    }

    // connect the two surfaces;
    // this is done by setting the right side panels' node indexes to those of the left side panels;
    // 1 or two vertices are assigned for each right side panel
    // leaves some nodes of the right surface dangling, but they can be kept unused in the array
    for(int ir=0; ir<sr.NXPanels()*2*coef; ir++)  // top and bottom * 2 triangles
    {
        int nr = sr.m_FirstStripPanelIndex + ir;

        if(nr<0 || nr>=int(panels.size())) return false; // error

        Panel3 &pr = panels[nr];

        if(pr.isFlapPanel())
            if(!bConnectFlaps) continue;

        for(int il=0; il<sl.NXPanels()*2*coef; il++)  // top and bottom * 2 triangles
        {
            int nl = sl.m_LastStripPanelIndex + il;
            Panel3 &pl = panels[nl];
            if(pl.isFlapPanel() && !bConnectFlaps)
                continue;

            if(nl<0 || nl>=int(panels.size())) return false; // error

            if(pl.surfacePosition()==pr.surfacePosition())
            {
                //connect only same top bot mid positions
                int nMerged = 0;
                int vl[]{-1,-1}; // identify the vertices being merged
                int vr[]{-1,-1};

                for(int ivr=0; ivr<3; ivr++)
                {
                    for(int ivl=0; ivl<3; ivl++)
                    {
                        if(pr.vertexAt(ivr).isSame(pl.vertexAt(ivl), 0.0001))
                        {
                            pr.setVertex(ivr, pl.vertexAt(ivl));// could also only set the index
                            vl[nMerged]=ivl;
                            vr[nMerged]=ivr;
                            nMerged++;
                        }
                        if(nMerged>=2) break; // prevent further merges if small panels
                    }
                    if(nMerged>=2) break; // prevent further merges if small panels
                }

                if(ir==0 && bThickSurfaces)
                {
                    m_StripStartNodes[iSurf+2] = pr.vertexAt(2).index();
                }

                if(nMerged==2)
                {
                    // two common vertices, the panels are neighbours
                    // identify the indexes of the common edges to assign neighbours
                    int iEdgeL = 3-vl[0]-vl[1];
                    int iEdgeR = 3-vr[0]-vr[1];

                    pl.setNeighbour(nr, iEdgeL);
                    pr.setNeighbour(nl, iEdgeR);
                    break; // done; don't try to merge pr again, for instance with top or bot surface
                }
            }
        }
    }
    return true;
}


/**
 * Create x-dir panel distributions at surface junctions
 */
void WingXfl::createXPoints()
{
    m_Surface.front().createXPoints();

    for(int is=1; is<nSurfaces(); is++)
    {
        Surface &surfA = surface(is-1);
        Surface &surfB = surface(is);
        surfB.createXPoints(); // left side nodes will be modified next to match those of surfA

        if(!surfA.hasTEFlap() && !surfB.hasTEFlap())
        {
            // no flap either side
            // do nothing
        }
        else if(surfA.hasTEFlap() && !surfB.hasTEFlap())
        {
            // flap on the left surface but not on the right
            // adapt left side nodes of surfB
            surfB.setXDistribA(surfA.xDistribB());
        }
        else if(!surfA.hasTEFlap() && surfB.hasTEFlap())
        {
            // flap on the right surface but not on the left
            // adapt right side nodes of surfA
            surfA.setXDistribB(surfB.xDistribA());
        }
        else if(surfA.hasTEFlap() && surfB.hasTEFlap())
        {
            // flap on both surfaces
            // it is assumed that the hinges are positioned at the same chordwise length
            // this is almost always the case on standard wing designs
            surfB.setXDistribA(surfA.xDistribB()); // this shouldn't change anything
        }
    }
}


/**
* Copies the geometrical data from an existing Wing
*@param pWing a pointer to the instance of the source Wing object
*/
void WingXfl::duplicate(WingXfl const *pWing)
{
    duplicate(*pWing);
}


/**
* Copies the geometrical data from an existing Wing
*@param pWing a pointer to the instance of the source Wing object
*/
void WingXfl::duplicate(WingXfl const &aWing)
{
    duplicatePart(aWing);
    m_bLocked = false;

    m_WingType        = aWing.m_WingType;
    m_NStation        = aWing.m_NStation;
    m_PlanformSpan    = aWing.m_PlanformSpan;
    m_ProjectedSpan   = aWing.m_ProjectedSpan;
    m_PlanformArea    = aWing.m_PlanformArea;
    m_ProjectedArea   = aWing.m_ProjectedArea;
    m_MAChord         = aWing.m_MAChord;

    m_nTipStrips      = aWing.m_nTipStrips;
    m_bSymmetric      = aWing.m_bSymmetric;
    m_bTwoSided       = aWing.m_bTwoSided;
    m_bCloseInnerSide = aWing.m_bCloseInnerSide;

    m_Section = aWing.m_Section;
//    m_Section.detach(); // forces deep copy?
//    for (int is=0; is<aWing.nSections(); is++)   m_Section.push_back(aWing.m_Section.value(is));

    m_Surface = aWing.m_Surface;
//    m_Surface.detach(); // forces deep copy?

//    computeGeometry();
    m_nFlaps = aWing.m_nFlaps;
    copyInertia(aWing);

    m_LE = aWing.m_LE;
    m_rx = aWing.m_rx;
    m_ry = aWing.m_ry;
    m_rz = aWing.m_rz;

    m_FirstPanel3Index = aWing.m_FirstPanel3Index;
    m_FirstPanel4Index = aWing.m_FirstPanel4Index;
    m_nPanel3 = aWing.m_nPanel3;
    m_nPanel4 = aWing.m_nPanel4;
    m_nNodes  = aWing.m_nNodes;

    m_StripStartNodes = aWing.m_StripStartNodes;
}


/**
* Returns the wing's average sweep from root to tip measured at the quarter chord
* The sweep is calulated as the arctangent of the root and tip quarter-chord points
*@return the value of the average sweep, in degrees
*/
double WingXfl::averageSweep() const
{
    double xroot = rootOffset() + chord(0)/4.0;
    double xtip  = tipOffset()  + tipChord()/4.0;
    double sweep = (atan2(xtip-xroot, m_PlanformSpan/2.0)) * 180.0/PI;
    return sweep;
}


/**
 * Returns the x-position of the quarter-chord point at a given span position
 *@param yob the span position where the quarter-chord point will be calculated
 *@return the quarter-chord position
 */
double WingXfl::C4(double yob) const
{
    double C4 = 0.0;
    double y = fabs(yob*m_PlanformSpan/2.0);
    for(int is=0; is<nSections()-1; is++)
    {
        if(yPosition(is)<= y && y <=yPosition(is+1))
        {
            double tau = (y - yPosition(is))/(yPosition(is+1)-yPosition(is));
            double cord  = chord(is)  + tau * (chord(is+1) - chord(is));
            double off = offset(is) + tau * (offset(is+1) - offset(is));
            C4 = off + cord/4.0;
            return C4;
        }
    }
    return C4;
}

/**
 * Calculates and returns the chord length at a given relative span position
 * @param yob the relative span position in %, where the chord length will be calculated
 * @return the chord length
 */
double WingXfl::getChord(double yob) const
{
    double cord = 0.0;
    double tau;
    double y;

    y= fabs(yob*m_PlanformSpan/2.0);//geometry is symmetric
    for(int is=0; is<nSections()-1; is++)
    {
        if(yPosition(is)<=y && y <=yPosition(is+1))
        {
            tau = (y - yPosition(is))/(yPosition(is+1)-yPosition(is));
            cord = chord(is) + tau * (chord(is+1) - chord(is));
            return cord;
        }
    }
    return -1.0;
}


/**
 * Returns the x-position of the Leading Edge at the specified absolute span position.
 */
double WingXfl::xLE(double spanpos) const
{
    double yob = spanpos/m_ProjectedSpan*2.0;
    //    double chord = getChord(yRel);
    double offset = getOffset(yob);
    return offset;
}


/**
 * Returns the x-position of the Trailing Edge at the specified absolute span position.
 */
double WingXfl::xTE(double spanpos) const
{
    double yob = spanpos/m_ProjectedSpan*2.0;
    double chord = getChord(yob);
    double offset = getOffset(yob);
    return offset+chord;
}


/**
 * Calculates and returns the offste value at a given relative span position
 * @param yob the relative span position in %, where the offset will be calculated
 * @return the offset value
 */
double WingXfl::getOffset(double yob) const
{
    double tau, y;
    double off = 0.0;

    y= fabs(yob*m_PlanformSpan/2.0);
    for(int is=0; is<nSections()-1; is++)
    {
        if(yPosition(is)<= y && y <=yPosition(is+1))
        {
            tau = (y - yPosition(is))/(yPosition(is+1)-yPosition(is));
            off = offset(is) + tau * (offset(is+1) - offset(is));
            return off;
        }
    }

    return -1.0;
}


/**
 * Calculates and returns the twist angle at a given relative span position
 * @param yob the relative position where the twist angle will be calculated
 * @return the twist angle in degrees
 */
double WingXfl::getTwist(double yob) const
{
    double y = fabs(yob*m_PlanformSpan/2.0);//geometry is symmetric

    // calculate twist at each station
    if (y>=0.0)
    {
        //right wing
        for (int is=0; is<nSections()-1; is++)
        {
            if(yPosition(is) <= y && y <=yPosition(is+1))
            {
                double tau = (y-yPosition(is))/(yPosition(is+1)-yPosition(is));
                return twist(is)+(twist(is+1)-twist(is)) * tau;
            }
        }
    }
    return 0.0;
}


/**
 * Calculates and returns the dihedral angle at a given relative span position
 * @param yob the relative position where the dihedral angle will be calculated
 * @return the dihedral angle in degrees
 */
double WingXfl::getDihedral(double yob) const
{
    double y= fabs(yob*m_PlanformSpan/2.0);//geometry is symmetric
    for(int is=0; is<nSections()-1; is++)
    {
        if(yPosition(is)<= y && y <=yPosition(is+1))
        {
            if(yob>=0) return dihedral(is);
            else  return -dihedral(is);
        }
    }
    return 0.0;
}


/**
 * Returns pointers to the left and right foils of a given span position, and the relative position of the span position between these two foils
 * @param pFoil0 the pointer to the pointer of the left foil
 * @param pFoil1 the pointer to the pointer of the right foil
 * @param y the reference span position
 * @param t the ratio between the position of the two foils
 */
void WingXfl::getFoils(Foil **pFoil0, Foil **pFoil1, double y, double &t)
{
    if  (y>0.0)
    {
        //search Right wing
        for (int is=0; is<nSections()-1; is++)
        {
            if (yPosition(is)<=y && y<=yPosition(is+1))
            {
                *pFoil0 = Objects2d::foil(rightFoilName(is));
                *pFoil1 = Objects2d::foil(rightFoilName(is+1));
                t = (y-yPosition(is))/(yPosition(is+1) - yPosition(is));
                return;
            }
        }
    }
    else
    {
        //search left wing
        y = -y;
        for (int is=0; is<nSections()-1; is++)
        {
            if (yPosition(is)<=y && y<yPosition(is+1))
            {
                *pFoil0 = Objects2d::foil(leftFoilName(is));
                *pFoil1 = Objects2d::foil(leftFoilName(is+1));
                t = (y-yPosition(is))/(yPosition(is+1) - yPosition(is));
                return;
            }
        }
    }
    t = 0;
    pFoil0 = nullptr;// use linear
    pFoil1 = nullptr;// use linear
}


void WingXfl::surfacePoint(double xRel, double ypos, xfl::enumSurfacePosition pos, Vector3d &Point, Vector3d &PtNormal) const
{
    double fy = fabs(ypos);

    int iSurf = nSurfaces()/2;

    double yl = 0.0;

    for (int is=0; is<nSections()-1; is++)
    {
        if(fabs(yPosition(is+1)-yPosition(is))>s_MinSurfaceLength)
        {
            if(yPosition(is)< (fy+PRECISION) && fy<=yPosition(is+1))
            {
                break;
            }
            yl += m_Surface.at(iSurf).spanLength();
            iSurf++;
        }
    }
    if(iSurf<nSurfaces())
    {
        Surface const &surf = m_Surface.at(iSurf);
        double yRel = (fabs(ypos)-yl)/surf.spanLength();
        surf.getSurfacePoint(xRel, xRel, yRel, pos, Point, PtNormal);

        if(ypos<0)
        {
            Point.y = -Point.y;
            PtNormal.y = -PtNormal.y;
        }
    }
    else
    {
        Point.set(0.0,0.0,0.0);
        PtNormal.set(0.0,0.0,0.0);
    }
}


/**
 * Returns the relative position in % of a given absolute span position
 * @param SpanPos the absolute span position
 * @return the relative position, in %
 */
double WingXfl::ySectionRel(double SpanPos) const
{
    double y = fabs(SpanPos);
    for(int is=0; is<nSections()-1; is++)
    {
        if(yPosition(is)<=y && y <yPosition(is+1))
        {
            if(SpanPos>0) return  (y-yPosition(is))/(yPosition(is+1)-yPosition(is));
            else          return  (y-yPosition(is+1))/(yPosition(is)-yPosition(is+1));
        }
    }
    return 1.0;
}

/**
 * The z-position of a specified absolute span position.
 * Used for moment evaluations in LLT, where the wing is defined as a 2D planform
 * @param y the abolute span position
 * @return the absolute z-position
 */
double WingXfl::ZPosition(double y) const
{
    double tau;
    double ZPos =0.0;

    y= fabs(y);
    if(y<=0.0) return 0.0;
    for (int is=0; is<nSections()-1; is++)
    {
        if(yPosition(is)< y && y<=yPosition(is+1))
        {
            for (int iss=0; iss<is; iss++)
            {
                ZPos+=sectionLength(iss+1) * sin(dihedral(iss)*PI/180.0);
            }
            tau = (y - yPosition(is))/(yPosition(is+1)-yPosition(is));
            ZPos += tau * sectionLength(is+1) * sin(dihedral(is)*PI/180.0);
            return ZPos;
        }
    }
    return 0.0;
}


/**
 * Computes the bending moment at each span position based on the results of the panel analysis
 * Assumes the array of force vectors has been calculated previously
 * @param bThinSurface true if the calculation has been performed on thin VLM surfaces, false in the case of a 3D-panelanalysis
 */
void WingXfl::panelComputeBending(std::vector<Panel4> const &panel4list, bool bThinSurface, SpanDistribs &SpanResFF)
{
    std::vector<double> ypos, zpos;
    Vector3d Dist(0.0,0.0,0.0);
    Vector3d Moment;

    int coef=0, p=0;
    double bm=0;

    if(bThinSurface) coef = 1;
    else             coef = 2;

    while(panel4list.at(m_FirstPanel4Index+p).isSidePanel()) p++;

    int NSurfaces = nSurfaces();

    for (int j=0; j<NSurfaces; j++)
    {
        Surface const &surf = m_Surface.at(j);
        for (int k=0; k<surf.NYPanels(); k++)
        {
            Panel4 const &p4 = panel4list.at(m_FirstPanel4Index+p);

            if(!bThinSurface)
            {
                ypos.push_back(p4.m_CollPt.y);
                zpos.push_back(p4.m_CollPt.z);
            }
            else
            {
                ypos.push_back(p4.vortexPosition().y);
                zpos.push_back(p4.trailingVortex().z);
            }

            p += coef * surf.NXPanels();
        }
    }

    for (int j=0; j<m_NStation; j++)
    {
        bm = 0.0;
        if (ypos[j]<=0)
        {
            for (int jj=0; jj<j; jj++)
            {
                Dist.y =  -ypos[jj]+ypos[j];
                Dist.z =  -zpos[jj]+zpos[j];
                Moment = Dist * SpanResFF.m_F.at(jj);
                bm += Moment.x;
            }
        }
        else
        {
            for (int jj=j+1; jj<m_NStation; jj++)
            {
                Dist.y =  ypos[jj]-ypos[j];
                Dist.z =  zpos[jj]-zpos[j];
                Moment = Dist * SpanResFF.m_F.at(jj);
                bm += Moment.x;
            }
        }
        SpanResFF.m_BendingMoment[j] = bm;
    }
}


/**
 * Computes the bending moment at each span position based on the results of the panel analysis
 * Assumes the array of force vectors has been calculated previously
 * @param bThinSurface true if the calculation has been performed on thin surfaces
 */
void WingXfl::panelComputeBending(const std::vector<Panel3> &panel3list, bool bThinSurface, SpanDistribs &SpanResFF)
{
    std::vector<double> ypos, zpos;
    Vector3d Dist(0.0,0.0,0.0);
    Vector3d Moment;

    int coef=0, p=0;
    double bm=0;

    if(bThinSurface) coef = 1;
    else             coef = 2;

    while(panel3list.at(m_FirstPanel3Index+p).isSidePanel()) p++;

    int NSurfaces = nSurfaces();

    for (int j=0; j<NSurfaces; j++)
    {
        Surface const &surf = m_Surface.at(j);
        for (int k=0; k<surf.NYPanels(); k++)
        {
            Panel3 const &p3 = panel3list.at(m_FirstPanel3Index+p);
            ypos.push_back(p3.CoG().y);
            zpos.push_back(p3.CoG().z);

            p += coef*surf.NXPanels() *2;
        }
    }

    for (int j=0; j<m_NStation; j++)
    {
        bm = 0.0;
        if (ypos[j]<=0)
        {
            for (int jj=0; jj<j; jj++)
            {
                Dist = SpanResFF.m_PtC4[jj]-SpanResFF.m_PtC4[j];
                Moment = Dist * SpanResFF.m_F.at(jj);
                bm += -Moment.x;
            }
        }
        else
        {
            for (int jj=j+1; jj<m_NStation; jj++)
            {
                Dist = SpanResFF.m_PtC4[jj]-SpanResFF.m_PtC4[j];
                Moment = Dist * SpanResFF.m_F.at(jj);
                bm += Moment.x;
            }
        }
        SpanResFF.m_BendingMoment[j] = bm;
    }
}


/**
* Scales the wing chord-wise so that the root chord is set to the NewChord value
*@param NewChord the new value of the root chord
*/
void WingXfl::scaleChord(double NewChord)
{
    double ratio = NewChord/chord(0);
    for (int is=0; is<nSections(); is++)
    {
        setChord(is, chord(is)*ratio);
        setOffset(is, offset(is)*ratio);
    }
    computeGeometry();
}


/**
* Scales the wing span-wise so that the span is set to the NewSpan value
*@param NewSpan the new value of the span
*/
void WingXfl::scaleSpan(double NewSpan)
{
    for (int is=0; is<nSections(); is++)
    {
        setYPosition(is, yPosition(is)* NewSpan/m_PlanformSpan);
        setSectionLength(is, sectionLength(is)* NewSpan/m_PlanformSpan);
    }
    computeGeometry();
}


/**
* Scales the wing's sweep so that the sweep is set to the NewSweep value
* @param newSweep the new value of the average quarter-chord sweep, in degrees
*/
void WingXfl::scaleSweep(double newSweep)
{
    double rootOffset = m_Section.front().m_Offset;
    double rootchord4 = rootOffset + chord(0)/4.0;

    //scale each panel's offset
    for(int is=1; is<nSections(); is++)
    {
        double chord4Offset = rootchord4 + tan(newSweep*PI/180.0) * m_Section.at(is).m_YPosition;
        setOffset(is, chord4Offset - chord(is)/4.0);
    }
    computeGeometry();
}


/**
* Scales the wing's twist angles so that the tip twist is set to the NewTwist value.
*@param NewTwist the new value of the average quarter-chord twist, in degrees
*/
void WingXfl::scaleTwist(double NewTwist)
{
    if(fabs(tipTwist())>0.0001)
    {
        //scale each panel's twist
        double ratio = NewTwist/tipTwist();

        for(int is=1; is<nSections(); is++)
        {
            setTwist(is, twist(is)*ratio);
        }
    }
    else
    {
        //Set each panel's twist in the ratio of the span position
        for(int is=1; is<nSections(); is++)
        {
            setTwist(is, NewTwist*yPosition(is)/(m_PlanformSpan/2.0));
        }
    }
    computeGeometry();
}


/**
* Scales the wing's area.
* All dimensions scaled proportionally sqrt(2).
* Useful e.g. when the weight is changed or when playing with Cl.
* @param newArea the new value of the wing's area.
*/
void WingXfl::scaleArea(double newArea)
{
    if(fabs(m_PlanformArea)<PRECISION) return;
    if(newArea<PRECISION) return;

    double ratio = sqrt(newArea/m_PlanformArea);

    for (int is=0; is<nSections(); is++)
    {
        setYPosition(is, yPosition(is)*ratio);
        setChord(is, chord(is)*ratio);
    }
    computeGeometry();
}


/**
 * Returns the number of mesh panels defined on this Wing's surfaces; the number is given for a double-side mesh of the wing
 * @return the total number of panels
 */
int WingXfl::quadTotal(bool bThinSurface) const
{
    double MinPanelSize;

    if(s_MinSurfaceLength>0.0) MinPanelSize = s_MinSurfaceLength;
    else                       MinPanelSize = m_PlanformSpan/1000.0;
    int total = 0;
    for (int is=0; is<nSections()-1; is++)
    {
        //do not create a surface if its length is less than the critical size
        if (fabs(yPosition(is)-yPosition(is+1)) > MinPanelSize)    total += nXPanels(is)*nYPanels(is);
    }
    if(!isFin()) total *=2;

    if(!bThinSurface)
    {
        total *= 2;

        //add tip patch count
        total += 2*m_nTipStrips*m_Section.front().nXPanels();
    }

    return total;
}


int WingXfl::nTriangles() const
{
    int total = quadTotal(false);
    total *=2; // two triangles/quad
    total -= m_nTipStrips*2*2; // the leading and trailingh quads at the tip patches are degenerate.

    return total;
}



void WingXfl::panel4ComputeInviscidForces(std::vector<Panel4> const &panel4list,
                                          PlanePolar const *pWPolar, Vector3d const &cog, double alpha, double beta, double QInf,
                                          double *Cp4, double const*gamma,
                                          AeroForces &AF) const
{
    Vector3d leverArmPanelCoG;
    Vector3d ForcePt, PanelForce;

    // Define the wind axes
    Vector3d WindDirection = objects::windDirection(alpha, beta);
    Vector3d WindNormal    = objects::windNormal(alpha, beta);

    Vector3d FiBodyAxes(0.0,0.0,0.0); // inviscid pressure forces, body axes
    Vector3d MiBodyAxes(0.0,0.0,0.0); // inviscid moment of pressure forces, body axes

    Vector3d PartM0;

    for(int i4=0; i4<m_nPanel4; i4++)
    {
        Panel4 const & p4 = panel4list.at(i4+m_FirstPanel4Index);
        if(!pWPolar->bWingTipMi() && p4.isSidePanel()) continue;

        int index = p4.index();
        assert(index==i4+m_FirstPanel4Index);
        ForcePt = p4.CoG();

        if(!pWPolar->isVLM())
        {
            ForcePt = p4.m_CollPt;
            PanelForce = p4.normal() * (-Cp4[index]) * p4.area();      // Newtons/q
        }
        else  // if(pWPolar->isVLMMethod() && p4.isMidPanel())  // is mid panel
        {
            // for each panel along the chord, add the lift coef
            ForcePt = p4.vortexPosition();
            PanelForce  = WindDirection * p4.trailingVortex();
            PanelForce *= 2.0 * gamma[index] /QInf;                                 //Newtons/q

            if(pWPolar->isVLM2() && !p4.isLeading())
            {
                Vector3d Force = WindDirection * p4.trailingVortex();
                Force      *= 2.0 * gamma[index+1] /QInf;                          //Newtons/q
                PanelForce -= Force;
            }
            Cp4[index] = -PanelForce.dot(p4.normal())/p4.area();    //
        }

        PartM0 += ForcePt * PanelForce.dot(WindNormal);

        leverArmPanelCoG = ForcePt - cog;                                // m

        FiBodyAxes += PanelForce;                             // N
        MiBodyAxes += leverArmPanelCoG * PanelForce;          // N.m/q
    }

    AF.setFsum(FiBodyAxes);                   // N/q
    AF.setMi(MiBodyAxes);                     // N.m/q
    AF.setM0(PartM0);
}


void WingXfl::panel3ComputeInviscidForces(const std::vector<Panel3> &panel3list,
                                          PlanePolar const *pWPolar, Vector3d const &cog,
                                          double alpha, double beta, double const *Cp3Vtx,
                                          AeroForces &AF) const
{
    Vector3d leverArmPanelCoG;
    Vector3d ForcePt, PanelForce;

    // Define the wind axes
    Vector3d WindNormal = objects::windNormal(alpha, beta);

    Vector3d FiBodyAxes(0.0,0.0,0.0); // inviscid pressure forces, body axes
    Vector3d MiBodyAxes(0.0,0.0,0.0); // inviscid moment of pressure forces, body axes

    Vector3d PartM0;

    for(int i3=0; i3<m_nPanel3; i3++)
    {
        Panel3 const & p3 = panel3list.at(i3+m_FirstPanel3Index);

        if(!pWPolar->bWingTipMi() && p3.isSidePanel())
            continue;

        int idx = p3.index() *3;
        ForcePt = p3.CoG();
        if(pWPolar->isTriUniformMethod())
        {
            double CpAverage = (Cp3Vtx[idx]+Cp3Vtx[idx+1]+Cp3Vtx[idx+2])/3.0;
            PanelForce = p3.normal() * (-CpAverage) * p3.area();      // Newtons/q
        }
        else if(pWPolar->isTriLinearMethod())
        {
            PanelForce.set(0,0,0);
            for(int iv=0; iv<3; iv++)
            {
                Node const &nd = p3.vertexAt(iv);
                PanelForce += nd.normal() * (-Cp3Vtx[idx+iv]) * p3.area()/3.0;      // Newtons/q
            }
        }

        PartM0 += ForcePt * PanelForce.dot(WindNormal);

        leverArmPanelCoG = ForcePt - cog;                                // m

        FiBodyAxes += PanelForce;                             // N
        MiBodyAxes += leverArmPanelCoG * PanelForce;          // N.m/q
    }

    AF.setFsum(FiBodyAxes);                   // N/q
    AF.setMi(MiBodyAxes);                   // N.m/q
    AF.setM0(PartM0);
}


void WingXfl::panel4ComputeStrips(std::vector<Panel4> const &panel4list, PlanePolar const *pWPolar, Vector3d const &CoG,
                                  double alpha, double beta, double QInf, double const *Cp4, double const *Gamma,
                                  SpanDistribs &SpanResSum)
{
    int p(0), iStrip(0), nFlap(0), coef(1);
    double CPStrip(0.0), NForce(0.0);
    Vector3d HingeLeverArm,  PtC4Strip, PtLEStrip, SurfaceNormal, LeverArmC4CoG, LeverArmPanelC4, LeverArmPanelCoG;
    Vector3d ForcePt, PanelForce;
    Vector3d Force, stripforce, panelmoment, HingeMoment, stripmoment;

    // Define the number of panels to consider on each strip
    if(pWPolar->bThinSurfaces()) coef = 1;    // only mid-surface
    else                         coef = 2;    // top and bottom surfaces

    // Define the wind axis
    Vector3d WindDirection = objects::windDirection(alpha, beta);
//    Vector3d WindNormal    = objects::windNormal(alpha, beta);
    Vector3d WindSide      = objects::windSide(alpha, beta);

    // Calculate the Reynolds number on each strip
    SpanResSum.m_Re.clear();
    for (iStrip=0; iStrip<m_NStation; iStrip++)
    {
        SpanResSum.m_Re.push_back(SpanResSum.m_Chord.at(iStrip) * QInf /pWPolar->viscosity());
        SpanResSum.m_CmC4[iStrip]=SpanResSum.m_CmPressure[iStrip]=0.0;
    }

    iStrip = p = nFlap = 0;
    m_FlapMoment.clear();

    // Calculate the coefficients on each strip
    for (int j=0; j<nSurfaces(); j++)
    {
        Surface const &surf = m_Surface.at(j);
        if(surf.hasTEFlap()) m_FlapMoment.push_back(0.0);
        SurfaceNormal = surf.normal();

        // skip the tip patch
        while (panel4list.at(m_FirstPanel4Index + p).isSidePanel()) p++;

        // consider each strip in turn
        for (int k=0; k<surf.NYPanels(); k++)
        {
            //initialize strip forces and moments
            stripforce.set(0.0,0.0,0.0);
            stripmoment.set(0.0,0.0,0.0);

            CPStrip = 0.0;

            surf.getLeadingPt(k, PtLEStrip);

            PtC4Strip = SpanResSum.m_PtC4.at(iStrip);

            LeverArmC4CoG = PtC4Strip - CoG;

            for (int l=0; l<coef*surf.NXPanels(); l++)
            {
                if(m_FirstPanel4Index+p>=int(panel4list.size()))
                {
                    return;
                }

                Panel4 const &p4 = panel4list.at(m_FirstPanel4Index + p);
                int index = p4.index();
                // Get the force acting on the panel
                if(!p4.isMidPanel() || !pWPolar->isVLM())
                {
                    ForcePt = p4.m_CollPt;
                    PanelForce = p4.normal() * (-Cp4[index]) * p4.area();      // Newtons/q
                }
                else  // if(pWPolar->isVLMMethod() && p4.isMidPanel())  // is mid panel
                {
                    // for each panel along the chord, add the lift coef
                    ForcePt = p4.vortexPosition();
                    PanelForce  = WindDirection * p4.trailingVortex();
                    PanelForce *= 2.0 * Gamma[index] /QInf;                                 //Newtons/q

                    if(pWPolar->isVLM() && pWPolar->isVLM2() && !p4.isLeading())
                    {
                        Force       = WindDirection * p4.trailingVortex();
                        Force      *= 2.0 * Gamma[index+1] /QInf;                          //Newtons/q
                        PanelForce -= Force;
                    }
                }

                stripforce += PanelForce;                                           // Newtons/q
                NForce = PanelForce.dot(SurfaceNormal);                             // Newtons/q

                LeverArmPanelC4    = ForcePt - PtC4Strip;                           // m
                LeverArmPanelCoG   = ForcePt - CoG;                                 // m

                panelmoment = LeverArmPanelC4 * PanelForce;                         // N.m/q
                SpanResSum.m_CmC4[iStrip]  += panelmoment.dot(WindSide);          // N.m/q

                stripmoment += LeverArmPanelCoG * PanelForce;                       // N.m/q
                CPStrip += ForcePt.x * NForce;                                      // N.m
                if(surf.hasTEFlap())
                {
                    if(surf.hasFlapPanel4(p4))
                    {
                        //then p is on the flap, so add its contribution
                        HingeLeverArm = ForcePt - surf.hingePoint();
                        HingeMoment = HingeLeverArm * PanelForce;                   // N.m/q
                        m_FlapMoment[nFlap] += HingeMoment.dot(surf.m_HingeVector)* pWPolar->density() * QInf * QInf/2.0;  //N.m
                    }
                }
                p++;
            }

            // calculate the position of the centre of pressure
            NForce = stripforce.dot(SurfaceNormal);                             // Newtons/q
            SpanResSum.m_XCPSpanRel[iStrip]    = (CPStrip/NForce - PtLEStrip.x)/SpanResSum.m_Chord.at(iStrip);
            SpanResSum.m_XCPSpanAbs[iStrip]    =  CPStrip/NForce;

            SpanResSum.m_CmC4[iStrip] *= 1.0 /SpanResSum.m_Chord.at(iStrip)/SpanResSum.m_StripArea.at(iStrip);
            SpanResSum.m_CmPressure[iStrip] = stripmoment.dot(WindSide)/SpanResSum.m_Chord.at(iStrip)/SpanResSum.m_StripArea.at(iStrip);

            // strip sum results
/*            double wdforce = stripforce.dot(WindDirection);
            //            double wsforce = StripForce.dot(WindSide);
            double wnforce = stripforce.dot(WindNormal);

            SpanResSum.m_Cl[iStrip]  = wnforce/SpanResSum.m_StripArea.at(iStrip);          // adimensional
            SpanResSum.m_ICd[iStrip] = wdforce/SpanResSum.m_StripArea.at(iStrip);          // adimensional
*/
            iStrip++;
        }

        if(surf.m_bTEFlap) nFlap++;
        if(surf.isTipRight() && pWPolar->bThickSurfaces())
            p+= surf.NXPanels(); // surface loop ends here anyway

    }
}


void WingXfl::panel3ComputeStrips(std::vector<Panel3> const &panel3list, PlanePolar const*pWPolar, Vector3d const &CoG,
                                  double alpha, double beta, double QInf, double const*Cp3Vtx,
                                  SpanDistribs &SpanResSum)
{
    int nFlap(0), idx(0);
    double CPStrip(0.0), NForce(0.0);
    Vector3d HingeLeverArm,  PtC4Strip, PtLEStrip, surfaceNormal;
    Vector3d leverArmC4CoG, leverArmPanelC4, leverArmPanelCoG;

    Vector3d StripForce, panelmoment, hingemoment, stripmoment;

    //    Vector3d Origin(0.0,0.0,0.0);
    Vector3d ForcePt, PanelForce;

    // Define the wind axes
//    Vector3d WindDirection = objects::windDirection(alpha, beta);
//    Vector3d WindNormal    = objects::windNormal(alpha, beta);
    Vector3d WindSide      = objects::windSide(alpha, beta);

    // Calculate the Reynolds number on each strip
    SpanResSum.m_Re.clear();
    for (int m=0; m<m_NStation; m++) SpanResSum.m_Re.push_back(SpanResSum.m_Chord.at(m) * QInf /pWPolar->viscosity());

    int iStrip = 0;

    m_FlapMoment.clear();

    // calculate the coefficients on each strip
    // Tip patches are ignored

    for(int isurf=0; isurf<nSurfaces(); isurf++)
    {
        Surface const &surf = m_Surface.at(isurf);
        surfaceNormal = surf.normal();
        if(surf.hasTEFlap()) m_FlapMoment.push_back(0.0);
        int ksurf = 0;

        int i3=0;
        do
        {
            Panel3 const &p3i = panel3list.at(surf.m_Panel3List.at(i3));
            if(!p3i.isWingPanel()) break;

            // consider each strip in turn
            if(p3i.isTrailing() && (p3i.isBotPanel() || p3i.isMidPanel()))
            {
                //initialize
                StripForce.set(0.0,0.0,0.0);
                stripmoment.set(0.0,0.0,0.0);

                SpanResSum.m_CmC4[iStrip] = 0.0;
                CPStrip        = 0.0;

                surf.getLeadingPt(ksurf, PtLEStrip);

                PtC4Strip = SpanResSum.m_PtC4.at(iStrip);

                leverArmC4CoG = PtC4Strip - CoG;

                do
                {
                    idx =  surf.m_Panel3List.at(i3);
                    Panel3 const &p3strip = panel3list.at(idx);

                    ForcePt = p3strip.CoG();
                    double CpAverage = (Cp3Vtx[3*idx]+Cp3Vtx[3*idx+1]+Cp3Vtx[3*idx+2])/3.0;
                    PanelForce = p3strip.normal() * (-CpAverage) * p3strip.area();      // Newtons/q

                    StripForce += PanelForce;                                           // Newtons/q
                    NForce = PanelForce.dot(surfaceNormal);                             // Newtons/q

                    leverArmPanelC4  = ForcePt - PtC4Strip;                             // m
                    panelmoment = leverArmPanelC4 * PanelForce;                         // N.m/q
                    SpanResSum.m_CmC4[iStrip]  += panelmoment.y;                      // N.m/q

                    leverArmPanelCoG = ForcePt - CoG;                                   // m
                    stripmoment += leverArmPanelCoG * PanelForce;                       // N.m/q

                    CPStrip   += ForcePt.x * NForce;

                    if(surf.hasTEFlap())
                    {
                        if(surf.hasFlapPanel3(idx))
                        {
                            //then p is on the flap, so add its contribution
                            HingeLeverArm = ForcePt - surf.hingePoint();
                            hingemoment = HingeLeverArm * PanelForce;                   //N.m/q
                            m_FlapMoment[nFlap] += hingemoment.dot(surf.hingeVector())* pWPolar->density() * QInf * QInf/2.0;  //N.m
                        }
                    }
                    i3++;

                    if(p3strip.iPU()<0) break; // break when no panel upstream = no panel downstream on top surface
                    if(p3strip.isSidePanel()) break; // not interested in sides panels, right tip has been reached
                } while(i3<nPanel3()); // just a safety limit

                // calculate center of pressure position
                NForce = StripForce.dot(surfaceNormal);
                SpanResSum.m_XCPSpanRel[iStrip] = (CPStrip/NForce- PtLEStrip.x)/SpanResSum.m_Chord.at(iStrip);
                SpanResSum.m_XCPSpanAbs[iStrip] =  CPStrip/NForce;

                SpanResSum.m_CmC4[iStrip] *= 1.0  /SpanResSum.m_Chord.at(iStrip)/SpanResSum.m_StripArea.at(iStrip);
                SpanResSum.m_CmPressure[iStrip] = stripmoment.dot(WindSide)/SpanResSum.m_Chord.at(iStrip)/SpanResSum.m_StripArea.at(iStrip);

                // strip sum results
//                double wdforce = StripForce.dot(WindDirection);                          // Newtons/q
                //            double wsforce = StripForce.dot(WindSide);                               // Newtons/q
//                double wnforce = StripForce.dot(WindNormal);                             // Newtons/q
//                SpanResSum.m_Cl[iStrip]  = wnforce/SpanResSum.m_StripArea.at(iStrip);           // adimensional
//                SpanResSum.m_ICd[iStrip] = wdforce/SpanResSum.m_StripArea.at(iStrip);           // adimensional

                ksurf++;
                iStrip++;
            }
            else i3++;
        }
        while(i3<surf.nPanel3());

        if(surf.hasTEFlap()) nFlap++;
    }

}


/** compute resultant viscous force and moments from strip drag */
void WingXfl::computeViscousForces(PlanePolar const *pWPolar, double alpha, double beta, SpanDistribs &SpanResFF, AeroForces &AF) const
{
    // Define the wind axis
    Vector3d winddirection = objects::windDirection(alpha, beta);

    Vector3d Fv;
    Vector3d Mv;
    for (int m=0; m<m_NStation; m++)
    {
        Vector3d dragvector =  winddirection * SpanResFF.m_PCd.at(m) * SpanResFF.m_StripArea.at(m);    // N/q
        Fv += dragvector;
        Vector3d stripleverarm = SpanResFF.m_PtC4.at(m)-pWPolar->CoG();    // N.m/q
        Mv += stripleverarm * dragvector;
    }

    //project on wind axis
    AF.setProfileDrag(Fv.dot(winddirection));    // N/q
    AF.setMv(Mv);    // N.m/q
}



bool WingXfl::isWingPanel4(int nPanel) const
{
    return m_FirstPanel4Index<=nPanel && nPanel<m_FirstPanel4Index+m_nPanel4;
}


/**
 * Removes the section identified by its index
 * @param iSection the index of the section
 */
void WingXfl::removeWingSection(int const iSection)
{
    if(iSection<0 || iSection>=nSections()) return;
    m_Section.erase(m_Section.begin()+iSection);
}


/**
 * Inserts a section at a postion identified by its index
 * @param iSection the index of the section
 */
void WingXfl::insertSection(int iSection)
{
    if(iSection==0)
        m_Section.push_back(WingSection());
    else if(iSection>=nSections())
        m_Section.push_back(WingSection());
    else
        m_Section.insert(m_Section.begin()+iSection, WingSection());
}


/**
 * Appends a new section at the tip of the wing, with default values
 */
bool WingXfl::appendWingSection()
{
    m_Section.push_back(WingSection());
    return true;
}


bool WingXfl::appendWingSection(WingSection const &ws)
{
    m_Section.push_back(ws);
    return true;
}


/**
 * Appends a new section at the tip of the wing, with values specified as input parameters
 */
bool WingXfl::appendWingSection(double Chord, double Twist, double Pos, double Dihedral, double Offset,
                             int NXPanels, int NYPanels, xfl::enumDistribution XPanelDist, xfl::enumDistribution YPanelDist,
                             std::string const &RightFoilName, std::string const &LeftFoilName)
{
    m_Section.push_back({});
    WingSection &ws = m_Section.back();
    ws.m_Chord      = Chord;
    ws.m_Twist      = Twist;
    ws.m_YPosition  = Pos;
    ws.m_Dihedral   = Dihedral;
    ws.m_Offset     = Offset;

    ws.m_NXPanels   = NXPanels;
    ws.m_NYPanels   = NYPanels;
    ws.m_XPanelDist = XPanelDist;
    ws.m_YPanelDist = YPanelDist;

    ws.m_RightFoilName  = RightFoilName;
    ws.m_LeftFoilName   = LeftFoilName;

    return true;
}


bool WingXfl::isWingFoil(Foil const *pFoil) const
{
    if(!pFoil) return false;

    for (int iws=0; iws<nSections(); iws++)
    {
        if(pFoil->name() == m_Section.at(iws).m_RightFoilName)
        {
            return true;
        }
    }

    if(!m_bSymmetric)
    {
        for (int iws=0; iws<nSections(); iws++)
        {
            if(pFoil->name() == m_Section.at(iws).m_LeftFoilName)
            {
                return true;
            }
        }
    }
    return false;
}



/** Finds the intersection point of a line originating at point O and with unit vector U
 * @param O the origin of the line
 * @param U the unit vector on the lie
 * @param I the intersection point, if any, otherwise returns an unchanged value
 * @return true if an intersection point was found, false otherwise
 */
bool WingXfl::intersectWing(Vector3d const &O,  Vector3d const &U, Vector3d &I, int &idxSurf, double &dist, bool bDirOnly) const
{
    for(idxSurf=0; idxSurf<nSurfaces(); idxSurf++)
    {
        if(geom::intersectQuad3d(m_Surface.at(idxSurf).m_LA, m_Surface.at(idxSurf).m_LB,
                                 m_Surface.at(idxSurf).m_TA, m_Surface.at(idxSurf).m_TB,
                                 O, U, I, bDirOnly))
            return true;
    }

    idxSurf = -1;
    dist=1000.0;

    return false;
}


/**
* Scales the wing's Aspect Ratio.
* Chords and span are scaled accordingly but the wing area remains unchanged.
* Good for general optimisation.
* @param newAR the new value of the aspect ratio.
*/
void WingXfl::scaleAR(double newAR)
{
    double AR = aspectRatio();
    if(AR<0.001)  return;
    if(newAR<0.001) return;

    double ratio = sqrt(newAR/AR);

    for (int is=0; is<nSections(); is++)
    {
        setYPosition(is, yPosition(is)*ratio);
        setChord(    is, chord(is)/ratio);
        setOffset(   is, offset(is)/ratio);
    }
    computeGeometry();
}


/**
* Scales the wing's Taper Ratio.
* Root chord is unchanged, all other chords are scale proportionnally to their span position.
* @param newTR the new value of the taper ratio.
*/
void WingXfl::scaleTR(double newTR)
{
    double TR = taperRatio();
    if(TR<0.001)  return;
    if(newTR<0.001) return;

    double Ratio = TR/newTR;
    for (int is=0; is<nSections(); is++)
    {
        double yRel = yPosition(is)/m_PlanformSpan *2.0;
        double cRatio = 1.0 +  yRel * (Ratio-1.0);
        setChord(is, chord(is) * cRatio);
    }
    computeGeometry();
}


/**
 Auxiliary integral used in LLT and MAC calculations
*/
double WingXfl::integralC2(double y1, double y2, double c1, double c2) const
{
    double res = 0.0;

    if (fabs(y2-y1)<1.e-5) return 0.0;
    double g = (c2-c1)/(y2-y1);

    res = (c1-g*y1)*(c1-g*y1)*(y2-y1) +
            g * (c1-g*y1)      *(y2*y2-y1*y1)+
            g*g/3.0            *(y2*y2*y2-y1*y1*y1);

    return res;
}


bool WingXfl::hasPanel3(int idx3) const
{
/*    for(int jSurf=0; jSurf<nSurfaces(); jSurf++)
    {
        if(m_Surface.at(jSurf).hasPanel3(idx3)) return true;
    }
    return false;*/

    return firstPanel3Index()<=idx3 && idx3<firstPanel3Index()+nPanel3();
}


bool WingXfl::hasPanel4(int idx4) const
{
    for(int jSurf=0; jSurf<nSurfaces(); jSurf++)
    {
        if(m_Surface.at(jSurf).hasPanel4(idx4)) return true;
    }
    return false;

}


WingSection *WingXfl::pSection(int iSec)
{
    if(iSec<0 || iSec>=nSections())
    {
        return nullptr;
    }
    return &m_Section[iSec];
}


bool WingXfl::checkFoils(std::string &log) const
{
    for(int iws =0; iws<nSections(); iws++)
    {
        if(!Objects2d::foil(section(iws).leftFoilName()))
        {
            std::string strong = "Missing foil: " + section(iws).leftFoilName();
            log += strong +"\n";
            return false;
        }
        if(!Objects2d::foil(section(iws).rightFoilName()))
        {
            std::string strong = "Missing foil: " + section(iws).leftFoilName();
            log += strong +"\n";
            return false;
        }
    }
    return true;
}


/**
* Calculates the chord lengths at each position of the NStation defined by the LLT or the Panel analysis
* Makes the array of interpolated foils at each span station
*/
void WingXfl::computeStations()
{
    double y1=0, y2=0;
    Vector3d C;

    int NSurfaces = nSurfaces();
    if(NSurfaces<=0) return;

    m_StripArea.clear();
    m_StripPos.clear();
    m_Chord.clear();
    m_PtC4.clear();
    m_Offset.clear();
    m_Twist.clear();

    // as many stations as there are strips
    std::vector<double> SpanPosition;
    int nStations = 0;


    int n2 = m_bTwoSided ? NSurfaces/2 : NSurfaces;

    // list the stations of the left side surfaces, from root chord to left tip
    /** @todo use the wing sections and not the surfaces to build the span positions */
    double y0 = 0;
    for (int j=n2-1; j>=0; j--)
    {
        Surface const & surf = m_Surface.at(j);
        for (int k=surf.NYPanels()-1; k>=0; k--)
        {
            //calculate span positions at each station
            surf.getYDist(k, y1, y2);
            SpanPosition.push_back(y0 - (1.0-(y1+y2)/2.0)*surf.planformLength());
            nStations++;
        }
        y0 -= surf.planformLength();
    }

    //make the span position array
    for(int m=int(SpanPosition.size())-1; m>=0; m--)
    {
        m_StripPos.push_back(SpanPosition.at(m));
    }

    if(isTwoSided())
    {
        for (int m=0; m<nStations; m++) // from root chord to left tip
        {
            m_StripPos.push_back(-SpanPosition.at(m));
        }
    }

    m_NStation = int(m_StripPos.size());

    double tau=0;
    for (int j=0; j<NSurfaces; j++)
    {
        Surface const & surf = m_Surface.at(j);
        for (int k=0; k<surf.NYPanels(); k++)
        {
            //calculate chords and offsets at each station
            double stripchord = surf.chord(k);
            m_Chord.push_back(stripchord);

            Vector3d PtC4;
            surf.getC4(k, PtC4, tau);
            m_PtC4.push_back(PtC4);

            surf.getLeadingPt(k, C);
            m_Offset.push_back(C.x);

            double tw = surf.stripTwist(k);
            m_Twist.push_back(tw);

            double stripwidth = surf.stripWidth(k);
            m_StripArea.push_back((stripwidth*stripchord));
        }
    }
}


void WingXfl::resizeSpanDistribs(int nStations)
{
    if(nStations==-1) nStations=m_NStation;
    m_StripArea.resize(nStations);
}


/**
 * Loads or Saves the data of this Wing to a binary file.
 * @param ar a refernce to the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool WingXfl::serializePartXFL(QDataStream &ar, bool bIsStoring)
{
    QString tag;
    QString rightfoil, leftfoil;
    int nx(0), ny(0);
    int k(0), n(0);
    int ArchiveFormat(0);// identifies the format of the file
    double dble(0), dm(0), px(0), py(0), pz(0);
    double cord(0), tw(0), pos(0), dih(0), off(0);
    QString strange;
    xfl::enumDistribution xDist=xfl::COSINE, yDist=xfl::UNIFORM;

    if(bIsStoring)
    {
        // using flow5 format instead
        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<100000 || ArchiveFormat>100003) return false;

        ar >> strange;  m_Name=strange.toStdString();
        ar >> strange;  m_Description=strange.toStdString();

        m_theStyle.m_Color.serialize(ar, false);

        ar >> m_bSymmetric;

        m_Section.clear();
        ar >> n;
        for (int i=0; i<n; i++)
        {
            ar >> rightfoil;
            ar >> leftfoil;
            ar >> cord;
            ar >> pos;
            ar >> off;
            ar >> dih;
            ar >> tw;
            ar >> nx;
            ar >> ny;

            ar >> k;
            if(k==1)       xDist = xfl::COSINE;
            else if(k== 2) xDist = xfl::SINE;
            else if(k==-2) xDist = xfl::INV_SINE;
            else           xDist = xfl::UNIFORM;

            ar >> k;
            if(k==1)       yDist = xfl::COSINE;
            else if(k== 2) yDist = xfl::SINE;
            else if(k==-2) yDist = xfl::INV_SINE;
            else           yDist = xfl::UNIFORM;

            appendWingSection(cord, tw, pos, dih, off, nx, ny, xDist, yDist, rightfoil.toStdString(), leftfoil.toStdString());
        }

        // improve uniformity of nx panel numbers
        if(n>=2)
        {
            int nx = m_Section.at(n-2).nXPanels();
            m_Section.back().setNX(nx);
        }

        ar >> dble;
        m_Inertia.setStructuralMass(dble);

        clearPointMasses();
        ar >> n;
        for(int i=0; i<n; i++)
        {
            ar >> dm >> px >> py >> pz;
            ar >> tag;
            m_Inertia.appendPointMass(PointMass(dm, Vector3d(px, py, pz), tag.toStdString()));
        }

        ar>>k; // if(k) m_bTextures=true; else m_bTextures=false;


        // space allocation
        for (int i=1; i<15; i++) ar >> k;
        ar>>k;  // m_bIsFin     = k? true : false; deprecated
        ar>>k;  m_bTwoSided = k? true : false;
        ar>>k; // m_bSymFin    = k? true : false;
        ar>>k;
        switch (k) {
            case 0:
                m_WingType=xfl::Main;
                if(ArchiveFormat<100002) m_bTwoSided = true;
                break;
            case 1:
                m_WingType=xfl::OtherWing;
                if(ArchiveFormat<100002) m_bTwoSided = true;
                break;
            case 2:
                m_WingType=xfl::Elevator;
                if(ArchiveFormat<100002) m_bTwoSided = true;
                break;
            case 3:
                m_WingType=xfl::Fin;
                m_bCloseInnerSide = true;
                if(ArchiveFormat<100002) m_bTwoSided = false;
                break;
            case 4:
                m_WingType=xfl::OtherWing;
                if(ArchiveFormat<100002) m_bTwoSided = true;
                break;
            default:
                break;
        }
        ar >>k; /*m_bReversed = k? true : false;*/

        for (int i=0; i<50; i++) ar >> dble;

        m_nTipStrips = 1;

        computeGeometry();

        return true;
    }
}


/**
 * Loads or Saves the data of this Wing to a binary file.
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool WingXfl::serializePartFl5(QDataStream &ar, bool bIsStoring)
{
    Part::serializePartFl5(ar, bIsStoring);
    int i(0), k(0), n(0), is(0);
    double dble(0);
    int nIntSpares = 0;
    int nDbleSpares = 0;

    // 500001 : new fl5 format;
    int ArchiveFormat = 500001;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << m_bSymmetric;

        ar << nSections();

        for (is=0; is<nSections(); is++)
        {
            WingSection &ws = m_Section[is];
            ws.serializeFl5(ar, bIsStoring);
        }

        ar << m_bTwoSided;
        ar << m_bCloseInnerSide;
        bool bReverse = false; // deprecated
        ar << bReverse;

        switch (m_WingType)
        {
            case xfl::Main:      ar<<0; break;
//            case WingXfl::Second:    ar<<1; break;
            case xfl::Elevator:  ar<<2; break;
            case xfl::Fin:       ar<<3; break;
            case xfl::OtherWing: ar<<4; break;
        }

        ar << m_LE.x << m_LE.y<< m_LE.z;
        ar << m_rx << m_ry << m_rz;


        // space allocation for the future storage of more data, without need to change the format

        nIntSpares = 1;
        ar << nIntSpares;
        ar << m_nTipStrips;
        for (int i=0; i<nIntSpares-1; i++) ar << 0;

        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << 0.0;

        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<=500000 || ArchiveFormat>500002) return false;

        ar >> m_bSymmetric;

        m_Section.clear();
        ar >> n;
        for (i=0; i<n; i++)
        {
            m_Section.push_back(WingSection());
            m_Section.back().serializeFl5(ar, bIsStoring);
        }

        ar >> m_bTwoSided;
        ar >> m_bCloseInnerSide;
        bool bReverse = false; // deprecated
        ar >> bReverse;

        ar >> n;
        switch(n)
        {
            case 0: m_WingType = xfl::Main;       break;
            case 1: m_WingType = xfl::OtherWing;  break;
            case 2: m_WingType = xfl::Elevator;   break;
            case 3: m_WingType = xfl::Fin;        break;
            default:
            case 4: m_WingType = xfl::OtherWing;  break;
        }

        ar >> m_LE.x >> m_LE.y >> m_LE.z;
        ar >> m_rx >> m_ry >> m_rz;

        // space allocation

        ar >> nIntSpares;
        ar >> m_nTipStrips;
        m_nTipStrips = std::max(m_nTipStrips, 1);
        for (int i=0; i<nIntSpares-1; i++) ar >> k;

        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
        computeGeometry();

        return true;
    }
}


/** Force a common number of x-panels at all span stations */
int WingXfl::uniformizeXPanelNumber()
{
    bool bUniform=true;
    int n0 = m_Section.at(0).nXPanels();
    double average = n0;
    for(int is=1; is<nSections()-1; is++)
    {
        if(m_Section.at(is).nXPanels()!=n0)
        {
            bUniform = false;
        }
        average += m_Section.at(is).nXPanels();
    }
    if(bUniform) return 0;

    int nx = int(std::round(average/nSections()));
    for(int is=0; is<nSections(); is++)
    {
        m_Section[is].m_NXPanels = nx;
    }
    return nx;
}


void WingXfl::exportToAVL(std::string &avlstring, int index, Vector3d const &T, double ry, double lengthunit) const
{
    QString strong, str;
    QString strange;
    QTextStream out(&strange);

    out << EOLch;

    str = "#================" +   QString::fromStdString(m_Name) + "================\n";
    strong.fill('=', str.length()-2);
    strong = "#"+strong + EOLch;

    out << strong;
    out << str + EOLch;

    //write only right wing surfaces since we provide YDUPLICATE
    Surface aSurface;
    int iFlap = 1;

    int NSurfaces = nSurfaces();

    int startIndex = (isFin()? 0 : int(NSurfaces/2));

    out << "#_______________________________________\n";
    out << "SURFACE\n";
    out << QString::fromStdString(m_Name);
    out << EOLch;
    out << "#Nchord    Cspace   [ Nspan Sspace ]\n";
    out << QString::asprintf("%d        %3.1f\n", nXPanels(0), 1.0);

    out << EOLch;
    out << "ANGLE\n";
    out << QString::asprintf("%9.3f                        | dAinc\n", ry);

    out << EOLch;
    out << "TRANSLATE\n";
    out << QString::asprintf("%9.3f    %9.3f     %9.3f\n", T.x*lengthunit, T.y*lengthunit, T.z*lengthunit);

    if(!isFin())
    {
        out << EOLch;
        out << "YDUPLICATE\n";
        out << "0.0\n";
    }
    else if(isTwoSided())
    {
        out << EOLch;
        out << "YDUPLICATE\n";
        out << "0.0\n";
    }

    out << EOLch;
    out << "COMPONENT\n";
    out << QString::asprintf("%4d                         | Lcomp\n",index);

    for(int j=startIndex; j<NSurfaces; j++)
    {
        aSurface.copy(m_Surface.at(j));

        out << "#______________\nSECTION\n";

        strong = QString::asprintf("%9.4f %9.4f %9.4f %9.4f  %7.3f  %3d  %3d   | Xle Yle Zle   Chord Ainc   [ Nspan Sspace ]\n",
                aSurface.m_LA.x       *lengthunit,
                aSurface.m_LA.y       *lengthunit,
                aSurface.m_LA.z       *lengthunit,
                aSurface.chord(0.0)   *lengthunit,
                aSurface.m_TwistA,
                aSurface.NYPanels(),
                objects::AVLSpacing(aSurface.yDistType()));
        out << strong;
        out << "\n";
        out << "AFIL 0.0 1.0\n";
        if(aSurface.foilA())  out << QString::fromStdString(aSurface.foilA()->name()) +".dat\n";
        out << EOLch;
        if(aSurface.hasTEFlap())
        {
            out << "CONTROL\n";
            strong = QString::asprintf("Flap_%d  ", iFlap);

            str = "1.0   ";
            strong += str;

            assert(aSurface.foilA());
            str = QString::asprintf("%5.3f  %9.4f   %9.4f  %9.4f   -1.0  ",
                    aSurface.foilA()->TEXHinge(),
                    aSurface.hingeVector().x,
                    aSurface.hingeVector().y,
                    aSurface.hingeVector().z);
            strong +=str + "| name, gain,  Xhinge,  XYZhvec,  SgnDup\n";
            out << (strong);
        }


        out << ("\n");

    }

    //write the last section
    out << ("\n#______________\nSECTION\n");

    strong = QString::asprintf("%9.4f  %9.4f  %9.4f  %9.4f  %7.3f   %3d  %3d   | Xle Yle Zle   Chord Ainc   [ Nspan Sspace ]\n",
            aSurface.m_LB.x       *lengthunit,
            aSurface.m_LB.y       *lengthunit,
            aSurface.m_LB.z       *lengthunit,
            aSurface.chord(1.0)   *lengthunit,
            aSurface.m_TwistB,
            aSurface.NYPanels(),
            objects::AVLSpacing(aSurface.yDistType()));
    out << strong;
    out << EOLch;
    out << "AFIL 0.0 1.0\n";
    if(aSurface.foilB())  out << QString::fromStdString(aSurface.foilB()->name()) +".dat\n";
    out << EOLch;

    if(aSurface.hasTEFlap())
    {
        out << "CONTROL\n";
        strong = QString::asprintf("Flap_%d  ", iFlap);

        str = "1.0   ";
        strong += str;

        assert(aSurface.foilB());
        str = QString::asprintf("%5.3f  %9.4f  %9.4f   %9.4f   -1.0  ",
                          aSurface.foilB()->TEXHinge(),
                          aSurface.hingeVector().x,
                          aSurface.hingeVector().y,
                          aSurface.hingeVector().z);
        strong +=str + "| name, gain,  Xhinge,  XYZhvec,  SgnDup\n";
        out << strong;
        out << EOLch;

        iFlap++;
    }
    out << EOLch << EOLch;

    avlstring.append(strange.toStdString());
}


void WingXfl::getProperties(std::string &properties, std::string const &prefx) const
{
    QString props;
    QString strange;
    QString prefix = QString::fromStdString(prefx);

    strange = QString::asprintf("%9.3f", m_PlanformArea*Units::m2toUnit()) + " ";
    props += prefix + "Wing area         ="+strange+Units::areaUnitQLabel() + "\n";

    strange = QString::asprintf("%9.3f", m_PlanformSpan*Units::mtoUnit()) + " ";
    props += prefix + "Wing span         ="+strange+Units::lengthUnitQLabel() + "\n";

    strange = QString::asprintf("%9.3f", m_ProjectedArea*Units::m2toUnit()) + " ";
    props += prefix + "Projected area    ="+strange+Units::areaUnitQLabel() + "\n";

    strange = QString::asprintf("%9.3f", m_ProjectedSpan*Units::mtoUnit()) + " ";
    props += prefix + "Projected span    ="+strange+Units::lengthUnitQLabel() + "\n";

    strange = QString::asprintf("%9.3f", GChord()*Units::mtoUnit()) + " ";
    props += prefix + "Mean geom. chord  ="+strange+Units::lengthUnitQLabel() + "\n";

    strange = QString::asprintf("%9.3f", m_MAChord*Units::mtoUnit()) + " ";
    props += prefix + "Mean aero. chord  ="+strange+Units::lengthUnitQLabel() + "\n";

    strange = QString::asprintf("%9.3f", aspectRatio());
    props += prefix + "Aspect ratio      ="+strange+ "\n";

    if(tipChord()>0.0) strange = QString::asprintf("%9.3f", taperRatio());
    else               strange = "Undefined";
    props += prefix + "Taper ratio       ="+strange+"\n";

    strange = QString::asprintf("%9.3f", averageSweep());
    props += prefix + "Sweep             =" + strange + DEGch + "\n";


    strange = QString::asprintf("VLM panels        =%d\n", quadTotal(true));
    props += prefix + strange;

    strange = QString::asprintf("Quad panels       =%d\n", quadTotal(false));
    props += prefix + strange;

    strange = QString::asprintf("Triangular panels =%d", nTriangles());
    props += prefix + strange;

    properties = props.toStdString();
}


void WingXfl::makeTriangulation(Fuse const *pFuse, int CHORDPANELS)
{
    if(nSurfaces()<=0)
    {
        // plane has not yet been initialized
        m_Triangulation.clear();
        m_Outline.clear();
        return;
    }

    std::vector<Triangle3d> &triangles = m_Triangulation.triangles();
    int CHORDPOINTS = CHORDPANELS;

    Vector3d N;

    std::vector<Node> PtBotLeft(CHORDPOINTS);
    std::vector<Node> PtBotRight(CHORDPOINTS);
    std::vector<Node> PtTopLeft(CHORDPOINTS);
    std::vector<Node> PtTopRight(CHORDPOINTS);

    //     top and bottom surfaces:
    //        NSurfaces
    //        x2 top and bottom
    //        x(CHORDPOINTS-1) : quads
    //        x2 : two triangles/quad
    int nTriangles = nSurfaces()*(CHORDPOINTS-1)*2*2;

    int nTipStrips = 11;

    // tip patches triangles
    for (int j=0; j<nSurfaces(); j++)
    {
        if(m_Surface.at(j).isClosedLeftSide())  nTriangles += (CHORDPOINTS-1)*2 * nTipStrips;
        if(m_Surface.at(j).isClosedRightSide()) nTriangles += (CHORDPOINTS-1)*2 * nTipStrips;
    }

    triangles.resize(nTriangles);

    int nSurf = nSurfaces();

    //make OUTLINE
    //vertices array size:
    // top and bot surface contours:
    //     NSurfaces
    //     x(CHORDPOINTS-1)*2 : segments from i to i+1, times two vertices
    //                          so that we can make only one call to GL_LINES later on
    //     x2  for A and B sides
    //     x2  for top and bottom
    int nOutlineSegments = nSurf*(CHORDPOINTS-1)*2*2;

    // tip strips
    //   2 tips
    //   x1 mean camber line
    //   x(CHORDPOINTS-1)*2 : segments from i to i+1
    //
    if(isTwoSided())
    {
        nOutlineSegments +=  2 * (CHORDPOINTS-1); // isClosedOuterSide
        if(isClosedInnerSide())
            nOutlineSegments +=  2 * (CHORDPOINTS-1);
    }
    else
    {
        nOutlineSegments +=  1 * (CHORDPOINTS-1); // isClosedOuterSide
        if(isClosedInnerSide())
            nOutlineSegments +=  1 * (CHORDPOINTS-1);
    }

    // leading and trailing edges
    //     2 points mLA & mLB for leading edge
    //     4 points mTA & mTB for trailing edge
    nOutlineSegments += nSurf*(1+4);

    //TE flap outline....
    for (int j=0; j<nSurf; j++)
    {
        Foil const *pFoilA = surfaceAt(j).foilA();
        Foil const *pFoilB = surfaceAt(j).foilB();

        if(pFoilA && pFoilB && pFoilA->hasTEFlap() && pFoilB->hasTEFlap())
        {
            nOutlineSegments += 2;//two vertices for the top line and two for the bottom line
        }
    }
    //LE flap outline....
    for (int j=0; j<nSurf; j++)
    {
        Foil const *pFoilA = surfaceAt(j).foilA();
        Foil const *pFoilB = surfaceAt(j).foilB();

        if(pFoilA && pFoilB && pFoilA->hasLEFlap() && pFoilB->hasLEFlap())
        {
            nOutlineSegments += 2;//two vertices for the top line and two for the bottom line
        }
    }

    m_Outline.resize(nOutlineSegments);

    N.set(0.0, 0.0, 0.0);
    int it3 = 0; //index of triangles
    int ivo = 0; //index of outline segments

    //SURFACE
    for (int jsurf=0; jsurf<nSurf; jsurf++)
    {
        Surface const &surf = surfaceAt(jsurf);

        surf.getSidePoints_1(xfl::TOPSURFACE, pFuse, PtTopLeft, PtTopRight, CHORDPOINTS, xfl::COSINE);
        surf.getSidePoints_1(xfl::BOTSURFACE, pFuse, PtBotLeft, PtBotRight, CHORDPOINTS, xfl::COSINE);

        //top surface
        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            triangles[it3++].setTriangle(PtTopLeft[l], PtTopLeft[l+1],  PtTopRight[l+1]);
            triangles[it3++].setTriangle(PtTopLeft[l], PtTopRight[l+1], PtTopRight[l]);
        }

        //bottom surface
        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            triangles[it3++].setTriangle(PtBotLeft[l], PtBotRight[l+1], PtBotLeft[l+1]);
            triangles[it3++].setTriangle(PtBotLeft[l], PtBotRight[l],   PtBotRight[l+1]);
        }

        Vector3d C, C1, R, R1;
        Node ptbl, ptbl1, pttl, pttl1; // the 4 corner points
        Vector3d nb, nt, nb1, nt1; // the 4 normals at the four corner points
        // left tip patch
        Vector3d NS = surf.normal();
        NS.rotateX(90.0);
        if(surf.isClosedLeftSide())
        {
            // some of these triangles are NULL, but no need to handle them separately
            for (int l=0; l<CHORDPOINTS-1; l++)
            {
                Node const &pbl  = PtBotLeft.at(l);
                Node const &pbl1 = PtBotLeft.at(l+1);
                Node const &ptl  = PtTopLeft.at(l);
                Node const &ptl1 = PtTopLeft.at(l+1);
                C.set( (pbl +ptl) /2.0);
                C1.set((pbl1+ptl1)/2.0);
                R.set( (ptl -pbl) /2.0);
                R1.set((ptl1-pbl1)/2.0);
                double rad  = R.norm();
                double rad1 = R1.norm();
                R.normalize();
                R1.normalize();

                for(int is=0; is<nTipStrips; is++)
                {
                    double thetab = double(is+1)/double(nTipStrips) * PI;
                    double thetat = double(is)  /double(nTipStrips) * PI;

                    nb.set( R* cos(thetab) + NS*sin(thetab));
                    nb1.set(R1*cos(thetab) + NS*sin(thetab));
                    ptbl.set( C  + nb*rad);
                    ptbl1.set(C1 + nb1*rad1);
                    ptbl.setNormal(nb);
                    ptbl1.setNormal(nb1);

                    nt.set( R* cos(thetat) + NS*sin(thetat));
                    nt1.set(R1*cos(thetat) + NS*sin(thetat));
                    pttl.set( C  + nt*rad);
                    pttl1.set(C1 + nt1*rad1);
                    pttl.setNormal(nt);
                    pttl1.setNormal(nt1);

                    triangles[it3++].setTriangle(ptbl,  ptbl1, pttl);
                    triangles[it3++].setTriangle(ptbl1, pttl1, pttl);
                }
            }
        }

        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            // top and bot outline, whether closed or open left side
            // bot

            m_Outline[ivo++].setNodes(PtBotLeft[l], PtBotLeft[l+1]);
            m_Outline[ivo++].setNodes(PtTopLeft[l], PtTopLeft[l+1]);

            if(surf.isClosedLeftSide())
            {
                // make a mid contour line
                Node const &pbl  = PtBotLeft.at(l);
                Node const &pbl1 = PtBotLeft.at(l+1);
                Node const &ptl  = PtTopLeft.at(l);
                Node const &ptl1 = PtTopLeft.at(l+1);
                C.set( (pbl +ptl) /2.0);
                C1.set((pbl1+ptl1)/2.0);
                R.set( (ptl -pbl) /2.0);
                R1.set((ptl1-pbl1)/2.0);
                double rad  = R.norm();
                double rad1 = R1.norm();
                R.normalize();
                R1.normalize();

                double theta = PI/2.0;
                nt.set( R* cos(theta) + NS*sin(theta));
                nt1.set(R1*cos(theta) + NS*sin(theta));
                pttl.set( C  + nt*rad);
                pttl1.set(C1 + nt1*rad1);

                m_Outline[ivo++].setNodes(pttl, pttl1);
            }
        }


        //right tip patch
        NS = surf.normal();
        NS.rotateX(-90);
        if(surf.isClosedRightSide())
        {
            for (int l=0; l<CHORDPOINTS-1; l++)
            {
                // two of these triangles are NULL, but no need to handle them separately
                Node const &pbl  = PtBotRight.at(l);
                Node const &pbl1 = PtBotRight.at(l+1);
                Node const &ptl  = PtTopRight.at(l);
                Node const &ptl1 = PtTopRight.at(l+1);
                C.set( (pbl +ptl) /2.0);
                C1.set((pbl1+ptl1)/2.0);
                R.set( (ptl -pbl) /2.0);
                R1.set((ptl1-pbl1)/2.0);
                double rad  = R.norm();
                double rad1 = R1.norm();
                R.normalize();
                R1.normalize();

                for(int is=0; is<nTipStrips; is++)
                {
                    double thetab = double(is+1)/double(nTipStrips) * PI;
                    double thetat = double(is)  /double(nTipStrips) * PI;

                    nb.set( R* cos(thetab) + NS*sin(thetab));
                    nb1.set(R1*cos(thetab) + NS*sin(thetab));
                    ptbl.set( C  + nb*rad);
                    ptbl1.set(C1 + nb1*rad1);
                    ptbl.setNormal(nb);
                    ptbl1.setNormal(nb1);

                    nt.set( R* cos(thetat) + NS*sin(thetat));
                    nt1.set(R1*cos(thetat) + NS*sin(thetat));
                    pttl.set( C  + nt*rad);
                    pttl1.set(C1 + nt1*rad1);
                    pttl.setNormal(nt);
                    pttl1.setNormal(nt1);

                    triangles[it3++].setTriangle(ptbl, pttl, ptbl1);
                    triangles[it3++].setTriangle(ptbl1, pttl, pttl1);
                }
            }
        }

        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            // top and bot outline, whether closed or open Right side
            // bot
            m_Outline[ivo++].setNodes(PtBotRight[l], PtBotRight[l+1]);

            //top
            m_Outline[ivo++].setNodes(PtTopRight[l], PtTopRight[l+1]);

            if(surf.isClosedRightSide())
            {
                // make a mid contour line
                Node const &pbl  = PtBotRight.at(l);
                Node const &pbl1 = PtBotRight.at(l+1);
                Node const &ptl  = PtTopRight.at(l);
                Node const &ptl1 = PtTopRight.at(l+1);
                C.set( (pbl +ptl) /2.0);
                C1.set((pbl1+ptl1)/2.0);
                R.set( (ptl -pbl) /2.0);
                R1.set((ptl1-pbl1)/2.0);
                double rad  = R.norm();
                double rad1 = R1.norm();
                R.normalize();
                R1.normalize();

                double theta = PI/2.0;
                nt.set( R* cos(theta) + NS*sin(theta));
                nt1.set(R1*cos(theta) + NS*sin(theta));
                pttl.set( C  + nt*rad);
                pttl1.set(C1 + nt1*rad1);
                m_Outline[ivo++].setNodes(pttl, pttl1);
            }
        }

        //Leading edge outline
        m_Outline[ivo++].setNodes(PtTopLeft.front(), PtTopRight.front());

        //trailing edge outline - four lines needed in case TE is not closed

        m_Outline[ivo++].setNodes(PtTopLeft.back(), PtTopRight.back());
        m_Outline[ivo++].setNodes(PtBotLeft.back(), PtBotRight.back());
        m_Outline[ivo++].setNodes(PtBotLeft.back(), PtTopLeft.back());
        m_Outline[ivo++].setNodes(PtBotRight.back(), PtTopRight.back());

        //TE flap outline....
        Vector3d PtA, PtB;

        Foil const *pFoilA = surf.foilA();
        Foil const *pFoilB = surf.foilB();
        if(pFoilA && pFoilB && pFoilA->hasTEFlap() && pFoilB->hasTEFlap())
        {
            surf.getSurfacePoint(surf.foilA()->TEXHinge(), surf.foilA()->TEXHinge(), 0.0, xfl::TOPSURFACE, PtA, N);
            surf.getSurfacePoint(surf.foilB()->TEXHinge(), surf.foilB()->TEXHinge(), 1.0, xfl::TOPSURFACE, PtB, N);
            m_Outline[ivo++].setNodes(PtA, PtB);


            surf.getSurfacePoint(surf.foilA()->TEXHinge(), surf.foilA()->TEXHinge(), 0.0, xfl::BOTSURFACE, PtA, N);
            surf.getSurfacePoint(surf.foilB()->TEXHinge(), surf.foilB()->TEXHinge(), 1.0, xfl::BOTSURFACE, PtB, N);
            m_Outline[ivo++].setNodes(PtA, PtB);
        }

        //LE flap outline....
        if(pFoilA && pFoilB && pFoilA->hasLEFlap() && pFoilB->hasLEFlap())
        {
            surf.getSurfacePoint(surf.foilA()->LEXHinge(), surf.foilA()->LEXHinge(), 0.0, xfl::TOPSURFACE, PtA, N);
            surf.getSurfacePoint(surf.foilB()->LEXHinge(), surf.foilB()->LEXHinge(), 1.0, xfl::TOPSURFACE, PtB, N);
            m_Outline[ivo++].setNodes(PtA, PtB);

            surf.getSurfacePoint(surf.foilA()->LEXHinge(), surf.foilA()->LEXHinge(), 0.0, xfl::BOTSURFACE, PtA, N);
            surf.getSurfacePoint(surf.foilB()->LEXHinge(), surf.foilB()->LEXHinge(), 1.0, xfl::BOTSURFACE, PtB, N);
            m_Outline[ivo++].setNodes(PtA, PtB);
         }
    }
}


double WingXfl::taperRatio() const
{
    if(rootChord()>0.001)
        return tipChord()/rootChord();
    else
        return 0.0;
}


Surface const &WingXfl::rootLeftSurface() const
{
    int NSurfaces = nSurfaces();
    int leftroot = int(NSurfaces/2);

    if(isTwoSided())
        return m_Surface.at(leftroot-1);
    else
        return m_Surface.back();
}


Surface const &WingXfl::rootRightSurface() const
{
    int NSurfaces = nSurfaces();
    int rightroot = int(NSurfaces/2);

    return isTwoSided() ? m_Surface.at(rightroot) : m_Surface.front();
}


void WingXfl::makeMidWires(std::vector<std::vector<Node>> &midwires) const
{
    for(int jsurf=0; jsurf<nSurfaces(); jsurf++)
    {
        Surface const &surf = surfaceAt(jsurf);
        if(surf.isLeftSurf() && surf.isCenterSurf())
        {
            midwires.push_back(surf.m_SideA);
            midwires.push_back(surf.m_SideB);
        }
        else if(surf.isRightSurf() && surf.isCenterSurf())
            midwires.push_back(surf.m_SideB);

    }
}


void WingXfl::makeTopBotWires(std::vector<std::vector<Node>> &topbotwires) const
{
    for(int jsurf=0; jsurf<nSurfaces(); jsurf++)
    {
        Surface const &surf = surfaceAt(jsurf);
        if(surf.isLeftSurf() && surf.isCenterSurf())
        {
            std::vector<Node> leftwire;
            for(uint k=0; k<surf.m_SideA_Bot.size()-1; k++)
                leftwire.push_back(surf.m_SideA_Bot.at(k));
            for(int k=surf.m_SideA_Top.size()-1; k>=0; k--)
                leftwire.push_back(surf.m_SideA_Top.at(k));

            std::vector<Node> midwire;
            for(uint k=0; k<surf.m_SideB_Bot.size()-1; k++)
                midwire.push_back(surf.m_SideB_Bot.at(k));
            for(int k=surf.m_SideB_Top.size()-1; k>=0; k--)
                midwire.push_back(surf.m_SideB_Top.at(k));


            topbotwires.push_back(leftwire);
            topbotwires.push_back(midwire);

/*            for(uint i=0; i<leftwire.size(); i++)
            {
                Node const &nd = leftwire.at(i);
                qDebug("%13g   %13g   %13g", nd.x, nd.y, nd.z);
            }
            qDebug("__________");*/
        }
        else if(surf.isRightSurf() && surf.isCenterSurf())
        {
            std::vector<Node> rightwire;
            for(uint k=0; k<surf.m_SideB_Bot.size()-1; k++)
                rightwire.push_back(surf.m_SideB_Bot.at(k));
            for(int k=surf.m_SideB_Top.size()-1; k>=0; k--)
                rightwire.push_back(surf.m_SideB_Top.at(k));

            topbotwires.push_back(rightwire);
        }
    }

    // close the wires
    for(uint i=0; i<topbotwires.size(); i++)
    {
        std::vector<Node> &wire = topbotwires[i];
        Node midnode = (wire.front() + wire.back())*0.5;
        wire.front() = midnode;
        wire.back() = midnode;
    }
}






