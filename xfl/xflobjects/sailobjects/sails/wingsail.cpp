/****************************************************************************

    flow5
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


#include <QTime>

#include "wingsail.h"

#include <xflfoil/objects2d/objects2d.h>
#include <xflfoil/objects2d/foil.h>
#include <xflobjects/sailobjects/sails/nurbssail.h>

#include <xflmath/constants.h>
#include <xflcore/xflcore.h>
#include <xflcore/units.h>
#include <xflpanels/mesh/trimesh.h>
#include <xflpanels/panels/panel4.h>

double WingSail::s_MinSurfaceLength = 0.0001;


WingSail::WingSail() : Sail()
{
    m_Name = "Wing type sail";
    m_Description = "Use this model for wing-type sails and for the mast";
    m_theStyle.m_Color = QColor(205,155,105);
    m_NStation = 0;
    m_bThinSurface = false;
}


void WingSail::makeDefaultSail()
{
    m_SailSection.clear();
    m_SectionPos.clear();
    m_Ry.clear();

    m_SectionPos.push_back(Vector3d(0,0,0));
    m_Ry.push_back(0.0);
    m_SailSection.append(WingSailSection());
    WingSailSection &sstack = m_SailSection.last();
    sstack.setFoilName("NACA_0009");
    sstack.setChord(7);
    sstack.setTwist(0.0);
    sstack.setNXPanels(5);
    sstack.setNZPanels(7);

    m_SectionPos.push_back(Vector3d(3,0,7.5));
    m_Ry.push_back(0.0);
    m_SailSection.append(WingSailSection());
    WingSailSection &sstop = m_SailSection.last();
    sstop.setFoilName("NACA_0009");
    sstop.setChord(1);
    sstop.setTwist(0.0);
    sstop.setNXPanels(5);
    sstop.setNZPanels(7);

    makeSurface();
    makeTriangulation();
    makeTriPanels(Vector3d());
}


void WingSail::resizeSections(int nSections, int )
{
    if(nSections==sectionCount()) return;
    if(sectionCount()<2) makeDefaultSail();

    // leave only bot and top sections
    m_SailSection = {m_SailSection.first(), m_SailSection.last()};
    m_SectionPos    = {m_SectionPos.first(), m_SectionPos.last()};
    m_Ry          = {m_Ry.first(), m_Ry.last()};

    // rebuild as many sections as needed between bot and top sections
    int nNewSecs = nSections-sectionCount();
    if(nNewSecs<=0) return;

    WingSailSection const &ss0 = m_SailSection.first();
    WingSailSection const &ssl = m_SailSection.last();

    for(int ins=0; ins<nNewSecs; ins++)
    {
        double ratio = double(ins+1)/double(nNewSecs+1);
        m_SailSection.insert(ins+1, section(ins));
        WingSailSection &ns = m_SailSection[ins+1];
        ns.setChord(ss0.chord()*(1.0-ratio) + ssl.chord()*ratio);
        ns.setTwist(ss0.twist()*(1.0-ratio) + ssl.twist()*ratio);
        ns.setXPanelDistType(ss0.xDistType());
        ns.setZPanelDistType(xfl::UNIFORM);

        m_SectionPos.insert(ins+1, m_SectionPos.first()*(1-ratio)+m_SectionPos.last()*ratio);
        m_Ry.insert(ins+1, m_Ry.first()*(1-ratio)+m_Ry.last()*ratio);
    }
}


void WingSail::flipXZ()
{
    Sail::flipXZ();

    for(int i=0; i<sectionCount(); i++)
    {
        m_SectionPos[i].y = -m_SectionPos[i].y;
    }

    for(int is=0; is<sectionCount(); is++)
    {
        WingSailSection &ws = section(is);
        ws.setTwist(-ws.twist());
    }
}



void WingSail::scale(double XFactor, double YFactor, double ZFactor)
{
    // scale the sail from the tack
    Vector3d const &tack = m_SectionPos[0];
    for(int i=0; i<sectionCount(); i++)
    {
        m_SectionPos[i].x = tack.x + (m_SectionPos[i].x-tack.x) * XFactor;
        m_SectionPos[i].y = tack.y + (m_SectionPos[i].y-tack.y) * YFactor;
        m_SectionPos[i].z = tack.z + (m_SectionPos[i].z-tack.z) * ZFactor;
    }

    for(int is=0; is<sectionCount(); is++)
    {
        WingSailSection &ws = section(is);
        ws.setChord(ws.chord()*XFactor);
    }
    makeSurface();
}


void WingSail::translate(const Vector3d &T)
{
    Sail::translate(T);

    for(int is=0; is<m_SectionPos.size(); is++)
        m_SectionPos[is].translate(T);
    makeSurface();
}


void WingSail::duplicate(const Sail *pSail)
{
    Sail::duplicate(pSail);
    if(!pSail->isWingSail()) return;

    WingSail const*pWS = dynamic_cast<WingSail const*>(pSail);
    m_SectionPos = pWS->m_SectionPos;
    m_Ry       = pWS->m_Ry;
    m_SailSection = pWS->m_SailSection;

    makeSurface();
}


double WingSail::luffLength() const
{
    if(m_SectionPos.size()<2) return 1.0;
    return (m_SectionPos.back() - m_SectionPos.front()).norm();
}


double WingSail::leechLength() const
{
    Vector3d pt0 = point(1.0,0.0);
    Vector3d pt1 = point(1.0,1.0);
    return (pt0-pt1).norm();
}


double WingSail::footLength() const
{
    Vector3d pt0 = point(0.00001,0.0);
    Vector3d pt1 = point(0.99999,0.0);
    return (pt0-pt1).norm();
}


double WingSail::twist() const
{
    WingSailSection const &stack = m_SailSection.front();
    WingSailSection const &stop = m_SailSection.last();

    return stop.twist()-stack.twist();
}


double WingSail::area() const
{
    double S=0.0;
    for(int is=0; is<sectionCount()-1; is++)
    {
        WingSailSection const &ws0 = sectionAt(is);
        WingSailSection const &ws1 = sectionAt(is+1);
        double dz = zPosition(is+1) - zPosition(is);
        double c = (ws0.chord()+ws1.chord())/2.0;
        S += dz*c;
    }
    return S;
}


bool WingSail::serializeSailFl5(QDataStream &ar, bool bIsStoring)
{
    Sail::serializeSailFl5(ar, bIsStoring);

    int n(0);
    int ArchiveFormat=500001;// identifies the format of the file
    double dble(0);
    int nIntSpares(0);
    int nDbleSpares(0);

    if(bIsStoring)
    {
        // storing code
        ar << ArchiveFormat;

        ar<<int(m_SailSection.size());
        for(int i=0; i<sectionCount(); i++)
        {
            m_SailSection[i].serializeFl5(ar, bIsStoring);
            ar << m_SectionPos[i].x<<m_SectionPos[i].y<<m_SectionPos[i].z;
            ar << m_Ry[i];
        }

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares; n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;

        return true;
    }
    else
    {
        // loading code
        ar >> ArchiveFormat;

        if (ArchiveFormat!=500001)  return false;

        ar>>n;
        m_SailSection.resize(n);
        m_SectionPos.resize(n);
        m_Ry.resize(n);
        for(int i=0; i<n; i++)
        {
            m_SailSection[i].serializeFl5(ar, bIsStoring);
            ar >> m_SectionPos[i].x >> m_SectionPos[i].y >> m_SectionPos[i].z;
            ar >> m_Ry[i];
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        makeSurface();
        makeTriangulation();

        return true;
    }
}


void WingSail::createSection(int iSection)
{
    if(iSection>=sectionCount())
    {
        m_SectionPos.append(m_SectionPos.last());
        m_SectionPos.last().z += 0.5;
        m_Ry.append(m_Ry.last());
        m_SailSection.append(m_SailSection.last());
    }
    else
    {
        if(iSection>0)
        {
            // insert before so that this new section has index iSection - is what the user expects
            m_SectionPos.insert(iSection, Vector3d());
            m_SectionPos[iSection] = (m_SectionPos[iSection-1]+m_SectionPos[iSection+1])/2.0;
            m_Ry.insert(iSection, 0.0);
            m_Ry[iSection] = (m_Ry[iSection-1]+m_Ry[iSection+1])/2.0;

            //make a new averaged section
            m_SailSection.insert(iSection, m_SailSection.at(iSection));
            WingSailSection &ssec = m_SailSection[iSection];
            WingSailSection const &ss0 = m_SailSection.at(iSection-1);
            WingSailSection const &ss1 = m_SailSection.at(iSection+1);
            ssec.setFoilName(ss0.foilName());
            ssec.setNXPanels(ss0.nxPanels());
            ssec.setNZPanels(ss0.nzPanels());
            ssec.setXPanelDistType(ss0.xDistType());
            ssec.setZPanelDistType(ss0.zDistType());
            ssec.setChord((ss0.chord()+ss1.chord())/2.0);
            ssec.setTwist((ss0.twist()+ss1.twist())/2.0);
        }
        else // iSection=0
        {
            m_SectionPos.insert(iSection, Vector3d());
            m_SectionPos[iSection].z = m_SectionPos[iSection+1].z-0.5;
            m_Ry.insert(iSection, 0.0);
            m_Ry[iSection] = m_Ry[iSection+1];
            m_SailSection.insert(iSection, m_SailSection.at(iSection));
        }
    }

    makeSurface();
}


void WingSail::deleteSection(int iSection)
{
    if(iSection<0 || iSection>=sectionCount()) return;

    m_SectionPos.removeAt(iSection);
    m_Ry.removeAt(iSection);

    m_SailSection.removeAt(iSection);
//    splineSurface();
}


void WingSail::insertSection(int is, WingSailSection &section, Vector3d position, double ry)
{
    if(is>=sectionCount())
    {
        m_SailSection.append(section);
        m_SectionPos.append(position);
        m_Ry.append(ry);
    }
    else
    {
        m_SailSection.insert(is, section);
        m_SectionPos.insert(is, position);
        m_Ry.insert(is, ry);
    }
}


void WingSail::appendNewSection()
{
    m_SailSection.append(WingSailSection());
    m_SectionPos.append(Vector3d());
    m_Ry.append(0.0);
}


Vector3d WingSail::leadingEdgeAxis() const
{
    if(m_SectionPos.size()<2) return Vector3d(0,0,1);
    return (m_SectionPos.back()-m_SectionPos.front()).normalized();
}


Vector3d WingSail::point(double xrel, double zrel, xfl::enumSurfacePosition pos) const
{
    if(xrel<0 || xrel>1) return Vector3d();
    if(zrel<0 || zrel>1) return Vector3d();

    double z = m_SectionPos.front().z + zrel * (m_SectionPos.back().z-m_SectionPos.front().z);

    // find the two sections surrounding zrel
    int isec = -1;
    double tau=1.0;
    for(int i=0; i<sectionCount()-1; i++)
    {
        if(m_SectionPos[i].z<=z && z <=m_SectionPos[i+1].z)
        {
            double dz =  (m_SectionPos[i+1].z - m_SectionPos[i].z);
            if(fabs(dz)>0.0) tau = (z-m_SectionPos[i].z) / dz;
            else             tau = 0.0;
            isec = i;
            break;
        }
    }
    if(isec<0) return Vector3d();

    WingSailSection const &ws0 = m_SailSection.at(isec);
    WingSailSection const &ws1 = m_SailSection.at(isec+1);

    double x0=0,y0=0,x1=0,y1=0;
    ws0.sectionPoint(xrel, pos, x0, y0);
    ws1.sectionPoint(xrel, pos, x1, y1);

    Vector3d pt0, pt1;
    pt0.set(m_SectionPos[isec].x+x0,y0,m_SectionPos[isec].z);
    pt0.rotateZ(m_SectionPos[isec], -ws0.twist());
    pt0.rotateY(m_SectionPos[isec], m_Ry[isec]);
    pt1.set(m_SectionPos[isec+1].x+x1,y1,m_SectionPos[isec+1].z);
    pt1.rotateZ(m_SectionPos[isec+1], -ws1.twist());
    pt1.rotateY(m_SectionPos[isec+1], m_Ry[isec+1]);
    return Vector3d(pt0.x*(1.0-tau)+pt1.x*tau, pt0.y*(1.0-tau)+pt1.y*tau, pt0.z*(1.0-tau)+pt1.z*tau);
}


/** UNUSED */
Vector3d WingSail::surfacePoint(int isec, double xrel, double zrel, xfl::enumSurfacePosition pos) const
{
    if(isec<0 || isec>=sectionCount()-1) return Vector3d();
    if(xrel<0 || xrel>1) return Vector3d();
    if(zrel<0 || zrel>1) return Vector3d();

    double tau=1.0;

    double dz =  (m_SectionPos[isec+1].z - m_SectionPos[isec].z);
    if(fabs(dz)>0.0) tau = zrel;
    else             tau = 0.0;

    WingSailSection const &ws0 = m_SailSection.at(isec);
    WingSailSection const &ws1 = m_SailSection.at(isec+1);

    double x0=0,y0=0,x1=0,y1=0;
    ws0.sectionPoint(xrel, pos, x0, y0);
    ws1.sectionPoint(xrel, pos, x1, y1);

    Vector3d pt0, pt1;

    pt0.set(m_SectionPos[isec].x+x0,y0,m_SectionPos[isec].z);
    pt0.rotateZ(m_SectionPos[isec], -ws0.twist());
    pt0.rotateY(m_SectionPos[isec], m_Ry[isec]);

    pt1.set(m_SectionPos[isec+1].x+x1,y1,m_SectionPos[isec+1].z);
    pt1.rotateZ(m_SectionPos[isec+1], -ws1.twist());
    pt1.rotateY(m_SectionPos[isec+1], m_Ry[isec+1]);

    return Vector3d(pt0.x*(1.0-tau)+pt1.x*tau, pt0.y*(1.0-tau)+pt1.y*tau, pt0.z*(1.0-tau)+pt1.z*tau);
}


void WingSail::makeTriangulation(int nx, int )
{
    m_Triangulation.clear();

    QVector<QVector<Vector3d>> sectionpoints;
    sectionpoints.resize(sectionCount());

    // split approximately nz between the sections
    // could keep only 1 since it is a straight extrusion
    // for STL export this may not be satisfactory?
    QVector<int> nzsplit(sectionCount()-1, 1);

    // make the contour points for each section
    for(int is=0; is<sectionCount(); is++)
    {
        WingSailSection const &ssec = section(is);
        ssec.getPoints(sectionpoints[is], nx);
    }

    //make the triangles between the sections
    for(int is=0; is<sectionCount()-1; is++)
    {
        QVector<Vector3d> const &botpts = sectionpoints.at(is);
        QVector<Vector3d> const &toppts = sectionpoints.at(is+1);
        for(int ipt=0; ipt<botpts.size()-1; ipt++)
        {
            Vector3d ptb0 = botpts.at(ipt);
            ptb0.x += xPosition(is);
            ptb0.z = zPosition(is);
            ptb0.rotateZ(sectionPosition(is), -section(is).twist());
            ptb0.rotateY(sectionPosition(is), sectionAngle(is));
            Vector3d ptb1 = botpts.at(ipt+1);
            ptb1.x += xPosition(is);
            ptb1.z = zPosition(is);
            ptb1.rotateZ(sectionPosition(is), -section(is).twist());
            ptb1.rotateY(sectionPosition(is), sectionAngle(is));

            Vector3d ptt0 = toppts.at(ipt);
            ptt0.x += xPosition(is+1);
            ptt0.z = zPosition(is+1);
            ptt0.rotateZ(sectionPosition(is+1), -section(is+1).twist());
            ptt0.rotateY(sectionPosition(is+1), sectionAngle(is+1));
            Vector3d ptt1 = toppts.at(ipt+1);
            ptt1.x += xPosition(is+1);
            ptt1.z = zPosition(is+1);
            ptt1.rotateZ(sectionPosition(is+1), -section(is+1).twist());
            ptt1.rotateY(sectionPosition(is+1), sectionAngle(is+1));

            Vector3d pti_b0 = ptb0;
            Vector3d pti_b1 = ptb1;
            for(int iz=0; iz<nzsplit.at(is); iz++)
            {
                double tau = double(iz+1)/double(nzsplit.at(is));
                Vector3d pti_t0 = ptb0 *(1-tau) + ptt0*tau;
                Vector3d pti_t1 = ptb1 *(1-tau) + ptt1*tau;
                m_Triangulation.appendTriangle({pti_b0, pti_t0, pti_b1});
                m_Triangulation.appendTriangle({pti_b1, pti_t0, pti_t1});
                pti_b0 = pti_t0;
                pti_b1 = pti_t1;
            }
        }
    }

    // make the tip patches
    QVector<double> fraclist;
    xfl::getPointDistribution(fraclist, nx+1);
    for(int is=0; is<sectionCount(); is+=sectionCount()-1) // only the first and last
    {
        WingSailSection const &ws = m_SailSection.at(is);
        Foil const *pFoil = Objects2d::foil(ws.foilName());
        if(pFoil)
        {
            Vector2d N;
            Vector2d pb0, pt0, pb1, pt1;
            Vector3d PL0, PR0, PL1, PR1;
            pb0 = pFoil->lowerYRel(0.0, N);
            pt0 = pFoil->upperYRel(0.0, N);
            PL0.set(xPosition(is)+pb0.x*ws.chord(), pb0.y*ws.chord(), zPosition(is));
            PL0.rotateZ(sectionPosition(is), -ws.twist());
            PL0.rotateY(sectionPosition(is), sectionAngle(is));
            PR0.set(xPosition(is)+pt0.x*ws.chord(), pt0.y*ws.chord(), zPosition(is));
            PR0.rotateZ(sectionPosition(is), -ws.twist());
            PR0.rotateY(sectionPosition(is), sectionAngle(is));
            for (int k=1; k<fraclist.size()-1; k++)
            {
                double xrel = fraclist.at(k);
                pb1 = pFoil->lowerYRel(xrel, N);
                pt1 = pFoil->upperYRel(xrel, N);

                PL1.set(xPosition(is)+pb1.x*ws.chord(), pb1.y*ws.chord(), zPosition(is));
                PL1.rotateZ(sectionPosition(is), -ws.twist());
                PL1.rotateY(sectionPosition(is), sectionAngle(is));
                PR1.set(xPosition(is)+pt1.x*ws.chord(), pt1.y*ws.chord(), zPosition(is));
                PR1.rotateZ(sectionPosition(is), -ws.twist());
                PR1.rotateY(sectionPosition(is), sectionAngle(is));

                if(!PL0.isSame(PR0) && !PL0.isSame(PR0))
                {
                    if(is==0) m_Triangulation.appendTriangle({PL0, PR0, PL1});
                    else      m_Triangulation.appendTriangle({PL0, PL1, PR0});
                }
                if(!PL0.isSame(PR1) && !PL0.isSame(PR1))
                {
                    if(is==0) m_Triangulation.appendTriangle({PL1, PR0, PR1});
                    else      m_Triangulation.appendTriangle({PL1, PR1, PR0});
                }
                PL0 = PL1;
                PR0 = PR1;
            }
        }
    }

    m_Triangulation.makeNodes();
//    m_Triangulation.makeTriangleConnections();
    m_Triangulation.makeNodeNormals();
}


void WingSail::makeSurface()
{
    Vector3d PLA, PTA, PLB, PTB;

    double MinPanelSize = 0.0;
    if(s_MinSurfaceLength>0.0) MinPanelSize = s_MinSurfaceLength;

    //define the normal to each surface
    int nSurf=0;


    for(int jss=m_SailSection.size()-1; jss>=1; jss--)
    {
        double panelLength = fabs(zPosition(jss)-zPosition(jss-1));
        if (panelLength < MinPanelSize)
        {
        }
        else
        {
            nSurf++;
        }
    }

    m_Surface.clear(); // reset the default data
    m_Surface.resize(nSurf);

    if(nSurf<=0) return;

    int NSurfaces = nSurf;

    int iSurf = 0;

    // build from top to bottom so that "top" surface is on starboard side
    for(int jss=m_SailSection.size()-1; jss>=1; jss--)
    {
        double panelLength = fabs(zPosition(jss)-zPosition(jss-1));
        if (panelLength < MinPanelSize)
        {
        }
        else
        {
            WingSailSection const &ws0 = section(jss);
            WingSailSection const &ws1 = section(jss-1);
            Surface &surf = m_Surface[iSurf];
            surf.setFoilNames(ws0.foilName(), ws1.foilName());

            surf.setPlanformLength(panelLength);

            PLA.x = xPosition(jss);                          PLB.x = xPosition(jss-1);
            PLA.y = 0.0;                                     PLB.y = 0.0;
            PLA.z = zPosition(jss);                          PLB.z = zPosition(jss-1);
            PTA.x = xPosition(jss) +ws0.chord();             PTB.x = xPosition(jss-1) +ws1.chord();
            PTA.y = 0.0;                                     PTB.y = 0.0;
            PTA.z = zPosition(jss);                          PTB.z = zPosition(jss-1);

            PTA.rotateY(PLA, m_Ry.at(jss));
            PTB.rotateY(PLB, m_Ry.at(jss-1));

            surf.setCornerPoints(PLA, PTA, PLB, PTB);
            surf.setNormal(); // is (0,0,1)

            surf.setNormals({0,1,0}, {0,1,0});

            surf.setTwist(ws0.twist(), ws1.twist());
            surf.setTwist(false);

            surf.setNXPanels(ws1.nxPanels());
            surf.setNYPanels(ws1.nzPanels());

            surf.setXDistType(ws1.xDistType());
            surf.setYDistType(ws1.zDistType());

            surf.createXPoints();
            surf.setFlap();
            surf.init();
            surf.setLeftSurf(false);
            surf.setRightSurf(false);
            surf.setClosedLeftSide(false);
            surf.setClosedRightSide(false);
            surf.setIsInSymPlane(false);
            surf.setInnerSection(jss);
            surf.setOuterSection(jss-1);

            surf.makeSideNodes(nullptr);

            iSurf++;
        }
    }
    m_Surface[NSurfaces-1].setCenterSurf(true);//previous left center surface

    m_Surface.first().setTipLeft(true);
    m_Surface.first().setClosedLeftSide(true);
    m_Surface.last().setTipRight(true);
    m_Surface.last().setClosedRightSide(true);
    m_Surface.first().setJoinRight(false);

    m_Head = m_Surface.first().LA();
    m_Peak = m_Surface.first().TA();
    m_Tack = m_Surface.last().LB();
    m_Clew = m_Surface.last().TB();

    computeChords();
}


void WingSail::computeChords()
{
    double y1=0, y2=0;
    Vector3d C;

    int NSurfaces = m_Surface.size();
    if(NSurfaces<=0) return;

    m_SpanResFF.clearGeometry();

    // as many stations as there are strips
    QVector<double> SpanPosition;
    m_NStation = 0;


    int n2 = NSurfaces;
    double x0 = m_Surface[n2-1].LB().x;
    double y0 = m_Surface[n2-1].LB().y;

    // list the stations of the left side surfaces, from root chord to left tip
    for (int j=n2-1; j>=0; j--)
    {
        Surface const & surf = m_Surface.at(j);
        for (int k=0; k<surf.NYPanels(); k++)
        {
            //calculate span positions at each station
            surf.getYDist(k, y1, y2);
            SpanPosition.append(y0 - (y1+y2)/2.0*surf.planformLength());
            m_NStation++;
        }
        y0 -= surf.planformLength();
    }

    //make the span position array
    for(int m=0; m<SpanPosition.size(); m++)
    {
        int idx = m_NStation-m-1;
        m_SpanResFF.m_StripPos.push_back(SpanPosition.at(idx));
    }

    m_NStation = m_SpanResFF.m_StripPos.size();

    double tau;
    for (int j=0; j<NSurfaces; j++)
    {
        Surface const & surf = m_Surface.at(j);
        for (int k=0; k<surf.NYPanels(); k++)
        {
            //calculate chords and offsets at each station
            double stripchord = surf.chord(k);
            m_SpanResFF.m_Chord.push_back(stripchord);

            Vector3d PtC4;
            surf.getC4(k, PtC4, tau);
            m_SpanResFF.m_PtC4.push_back(PtC4);

            surf.getLeadingPt(k, C);
            m_SpanResFF.m_Offset.push_back(C.x-x0);
            double tw = surf.stripTwist(k);
            m_SpanResFF.m_Twist.push_back(tw);

//            double stripwidth = sqrt((surf.LB.y-surf.LA.y)*(surf.LB.y-surf.LA.y) + (surf.LB.z-surf.LA.z)*(surf.LB.z-surf.LA.z));
            double stripwidth = surf.stripWidth(k);
            m_SpanResFF.m_StripArea.push_back((stripwidth*stripchord));
        }
    }

    //    for(int i=0; i<m_SpanResFF.m_StripArea.size(); i++) qDebug(" %3d %15.7f", i, m_SpanResFF.m_StripArea.at(i));
    // prepare results arrays
    m_SpanResFF.resizeResults(m_NStation);

    m_SpanResSum = m_SpanResFF;
}


void WingSail::properties(QString &props, QString const &frontspacer, bool bFull) const
{
    QString strlength = Units::lengthUnitLabel();
    QString strarea = Units::areaUnitLabel();
    QString strange;

    props.clear();
    props += frontspacer + m_Name +"\n";
    if(bFull)
    {
        props += frontspacer + "   Wing type sail\n";
    }
    strange = QString::asprintf("   Luff length    = %7.3g ", luffLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+"\n";
    strange = QString::asprintf("   Leech length   = %7.3g ", leechLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+"\n";
    strange = QString::asprintf("   Foot length    = %7.3g ", footLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+"\n";
    strange = QString::asprintf("   Area           = %7.3g ", area()*Units::m2toUnit());
    props += frontspacer + strange + strarea+"\n";
    strange = QString::asprintf("   Aspect ratio   = %7.3g ", aspectRatio());
    props += frontspacer + strange + "\n";
    strange = QString::asprintf("   Triangle count = %d", m_TriMesh.nPanels());
    props += frontspacer + strange;
}


void WingSail::updateStations()
{
    m_SpanResFF.resizeGeometry(m_NStation);
    m_SpanResFF.resizeResults(m_NStation);
    m_SpanResSum.resizeGeometry(m_NStation);
    m_SpanResSum.resizeResults(m_NStation);
}


void WingSail::makeTriPanels(Vector3d const &Tack)
{
    m_FirstPanel3Index = 0;
    m_TriMesh.clearMesh();

    int nWingPanels=0;
    int leftnodeidx=-1, rightnodeidx=-1;
    for(int jSurf=0; jSurf<m_Surface.size(); jSurf++)
    {
        Surface surf = m_Surface[jSurf];
        surf.translate(Tack);
        surf.makeSideNodes(nullptr);
        surf.makeTriPanels(m_TriMesh.panels(), m_TriMesh.nodes(), true, 1, leftnodeidx, rightnodeidx);
        nWingPanels += surf.panel3list().size();        
    }
    (void)nWingPanels;
}


double WingSail::length() const
{
    double L=0;
    for(int is=0; is<m_SailSection.size(); is++)
        L = std::max(L, m_SailSection.at(is).chord());
    return L;
}




