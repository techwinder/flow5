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


#include <QDataStream>

#include <sail.h>
#include <boatpolar.h>
#include <triangle3d.h>
#include <units.h>
#include <utils.h>
#include <trimesh.h>
#include <panel3.h>
#include <panel4.h>
#include <geom_global.h>

int Sail::s_iXRes = 37;
int Sail::s_iZRes = 31;


Sail::Sail() : Part()
{
    m_Name = "Sail Name";
    m_theStyle.m_Color.setRgba(155, 168, 174, 155);

    m_RefArea = m_RefChord = 0.0;
    m_LuffAngle = 0.0;

    m_LE.reset();

    m_bRuledMesh = true;
    m_NXPanels      = 7;
    m_XDistrib      = xfl::TANH;
    m_NZPanels      = 11;
    m_ZDistrib      = xfl::TANH;

    m_bThinSurface = true;

    m_MaxElementSize = 1.0;

}


Sail::~Sail()
{
}


void Sail::duplicate(Sail const*pSail)
{
    Part::duplicatePart(*pSail);

    m_RefArea       = pSail->m_RefArea;
    m_RefChord      = pSail->m_RefChord;

    m_NXPanels      = pSail->m_NXPanels;
    m_XDistrib         = pSail->m_XDistrib;

    m_LuffAngle     = pSail->m_LuffAngle;

    m_Tack          = pSail->m_Tack;
    m_Head          = pSail->m_Head;
    m_Clew          = pSail->m_Clew;
    m_Peak          = pSail->m_Peak;

    m_Triangulation = pSail->m_Triangulation;

    m_bRuledMesh      = pSail->m_bRuledMesh;
    m_bThinSurface    = pSail->m_bThinSurface;
    m_TopTEIndexes    = pSail->m_TopTEIndexes;
    m_BotMidTEIndexes = pSail->m_BotMidTEIndexes;
    m_RefTriangles    = pSail->m_RefTriangles;

    m_MaxElementSize  = pSail->m_MaxElementSize;

    m_TriMesh         = pSail->m_TriMesh;

    m_SpanResFF       = pSail->m_SpanResFF;
    m_SpanResSum      = pSail->m_SpanResSum;

    m_EdgeSplit   = pSail->m_EdgeSplit;
}


void Sail::properties(std::string &properties, const std::string &prefix, bool bFull) const
{
    QString strlength = Units::lengthUnitQLabel();
    QString strarea = Units::areaUnitQLabel();
    QString strange;

    QString props;
    QString frontspacer = QString::fromStdString(prefix);

    props.clear();
    props += frontspacer + QString::fromStdString(m_Name) + EOLch;
    if(bFull)
    {
        if     (isNURBSSail())  props += frontspacer + "   NURBS type sail\n";
        else if(isSplineSail()) props += frontspacer + "   Spline type sail\n";
        else if(isWingSail())   props += frontspacer + "   Wing type sail\n";
        else if(isStlSail())    props += frontspacer + "   STL type sail\n";
        else if(isOccSail())    props += frontspacer + "   CAD type sail\n";
    }
    strange = QString::asprintf("   Luff length    = %7.3g", luffLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+ EOLch;
    strange = QString::asprintf("   Leech length   = %7.3g", leechLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+ EOLch;
    strange = QString::asprintf("   Foot length    = %7.3g", footLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+ EOLch;
    strange = QString::asprintf("   Area           = %7.3g",  area()*Units::m2toUnit());
    props += frontspacer + strange + strarea+ EOLch;
    strange = QString::asprintf("   Aspect ratio   = %7.3g", aspectRatio());
    props += frontspacer + strange + "\n";
    strange = QString::asprintf("   Top twist      = %7.3g", twist());
    props += frontspacer + strange + DEGch+ EOLch;
    strange = QString::asprintf("   Triangle count = %d", int(m_RefTriangles.size()));
    props += frontspacer + strange;

    properties = props.toStdString();
}


double Sail::size() const
{
    return std::max(footLength(), luffLength());
}


bool Sail::serializeSailFl5(QDataStream &ar, bool bIsStoring)
{
    Part::serializePartFl5(ar, bIsStoring);
    int n=0;

    // 500001: new fl5 format;
    // 500002: beta 17: added Clew-Peak-Head-Tack
    // 500003: beta 18: added reference the area
    // 500004: beta 18: added reference the reference chord
    // 500005: beta 19: added free mesh parameters
    // 500006: v7.03:   added edge split parameters

    int ArchiveFormat=500006;// identifies the format of the file
    int nIntSpares=0;
    int nDbleSpares=0;

    if(bIsStoring)
    {
        // storing code
        ar << ArchiveFormat;

        ar << m_RefArea << m_RefChord;

        ar << m_bThinSurface;
        ar << m_bRuledMesh;
        ar << m_MaxElementSize;

        ar << m_NXPanels;
        ar << 11; // formerly NZPanels
        switch(m_XDistrib)
        {
            default:
            case xfl::UNIFORM:       n=0;  break;
            case xfl::COSINE:        n=1;  break;
            case xfl::SINE:          n=2;  break;
            case xfl::INV_SINE:      n=3;  break;
            case xfl::INV_SINH:      n=4;  break;
            case xfl::TANH:          n=5;  break;
            case xfl::EXP:           n=6;  break;
            case xfl::INV_EXP:       n=7;  break;
        }
        ar << n;

        n=0;
        ar << n; // formerly m_ZDist

        ar << m_LE.x << m_LE.y << m_LE.z;

        ar << m_Clew.x << m_Clew.y << m_Clew.z;
        ar << m_Peak.x << m_Peak.y << m_Peak.z;
        ar << m_Tack.x << m_Tack.y << m_Tack.z;
        ar << m_Head.x << m_Head.y << m_Head.z;

        ar << int(m_EdgeSplit.size());
        for(uint i=0; i<m_EdgeSplit.size(); i++)
        {
            ar << int(m_EdgeSplit[i].size());
            for(uint j=0; j<m_EdgeSplit[i].size(); j++)
                m_EdgeSplit[i][j].serialize(ar, bIsStoring);
        }

        ar << 0; // nIntSpares
        ar << 0; // nDoubleSpares
        return true;
    }
    else
    {
        // loading code
        ar >> ArchiveFormat;

        if (ArchiveFormat<500000 || ArchiveFormat>510000)  return false;

        if(ArchiveFormat>=500003) ar >> m_RefArea;
        if(ArchiveFormat>=500004) ar >> m_RefChord;

        if(ArchiveFormat>=500005)
        {
            ar >> m_bThinSurface;
            ar >> m_bRuledMesh;
            ar >> m_MaxElementSize;
        }

        ar >> m_NXPanels;
        ar >> n; // m_NZPanels;
        ar >> n;
        switch(n)
        {
            default:
            case 0: m_XDistrib=xfl::UNIFORM;      break;
            case 1: m_XDistrib=xfl::COSINE;       break;
            case 2: m_XDistrib=xfl::SINE;         break;
            case 3: m_XDistrib=xfl::INV_SINE;     break;
            case 4: m_XDistrib=xfl::INV_SINH;     break;
            case 5: m_XDistrib=xfl::TANH;         break;
            case 6: m_XDistrib=xfl::EXP;          break;
            case 7: m_XDistrib=xfl::INV_EXP;      break;
        }

        if(ArchiveFormat<=500006)
        {
            ar >> n;
/*            switch(n)
            {
                default:
                case 0: m_ZDist=Xfl::UNIFORM;      break;
                case 1: m_ZDist=Xfl::COSINE;       break;
                case 2: m_ZDist=Xfl::SINE;         break;
                case 3: m_ZDist=Xfl::INV_SINE;     break;
                case 4: m_ZDist=Xfl::INV_SINH;     break;
                case 5: m_ZDist=Xfl::TANH;         break;
                case 6: m_ZDist=Xfl::EXP;          break;
                case 7: m_ZDist=Xfl::INV_EXP;      break;
            }*/
        }

        ar >> m_LE.x >>m_LE.y >> m_LE.z;

        if(ArchiveFormat>=500002)
        {
            ar >> m_Clew.x >> m_Clew.y >> m_Clew.z;
            ar >> m_Peak.x >> m_Peak.y >> m_Peak.z;
            ar >> m_Tack.x >> m_Tack.y >> m_Tack.z;
            ar >> m_Head.x >> m_Head.y >> m_Head.z;
        }

        if(ArchiveFormat>=500006)
        {
            ar >> n;
            if(n==0)
            {
                // update legacy projects
                n=4;
                m_EdgeSplit.resize(n);
                m_EdgeSplit.front().resize(4);
            }
            else
            {
                m_EdgeSplit.resize(n);
                for(uint i=0; i<m_EdgeSplit.size(); i++)
                {
                    ar >> n;
                    m_EdgeSplit[i].resize(n);
                    for(uint j=0; j<m_EdgeSplit[i].size(); j++)
                        m_EdgeSplit[i][j].serialize(ar, bIsStoring);
                }
            }
        }

        // space allocation
        ar >> nIntSpares;
        ar >> nDbleSpares;

        return true;
    }
}


void Sail::makeTriangulation(int nx, int nh)
{
    Vector3d S00, S01, S10, S11;
    double uk(0), uk1(0), vl(0), vl1(0);

    m_Triangulation.clear();

    for (int k=0; k<nx; k++)
    {
        uk  = double(k)   /double(nx);
        uk1 = double(k+1) /double(nx);

        vl=0.0;
        S00 = point(uk,  vl);
        S10 = point(uk1, vl);

        for (int l=0; l<nh; l++)
        {
            vl1 = double(l+1) / double(nh);
            S01 = point(uk,  vl1);
            S11 = point(uk1, vl1);

            if(!S00.isSame(S01) && !S01.isSame(S11) && !S11.isSame(S00))
                m_Triangulation.appendTriangle(Triangle3d(S00, S01, S11));
            if(!S00.isSame(S11) && !S11.isSame(S10) && !S10.isSame(S00))
                m_Triangulation.appendTriangle(Triangle3d(S00, S11, S10));

            S00 = S01;
            S10 = S11;
        }
    }
    m_Triangulation.makeNodes();
//    m_Triangulation.makeTriangleConnections();
    m_Triangulation.makeNodeNormals(isNURBSSail());
}


void Sail::saveConnections()
{
    for(uint i=0; i<m_RefTriangles.size(); i++)
    {
        m_RefTriangles[i].setNeighbours(m_TriMesh.panelAt(i).neighbours());
    }
}


void Sail::mergeRefNodes(Node const& src, Node const&dest)
{
    for(uint i3=0; i3<m_RefTriangles.size(); i3++)
    {
        Triangle3d &t3d = m_RefTriangles[i3];
        for(int ivtx=0; ivtx<3; ivtx++)
        {
            if(t3d.vertex(ivtx).isSame(src, 1.e-4))
            {
                t3d.setVertex(ivtx, dest);
                t3d.setTriangle();
                break;
            }
        }
    }
}


/**
 * Cleans the array of reference triangles to remove those with low area, then
 * builds a panel3 for each reference triangle */
void Sail::makeTriPanels(Vector3d const &Tack)
{
    std::string log, prefix;

    xfl::enumSurfacePosition pos = m_bThinSurface ? xfl::MIDSURFACE : xfl::NOSURFACE;

    // clean the reference panels with the same criteria as the mesh panel
    for (int i=int(m_RefTriangles.size())-1; i>=0; i--)
    {
        if(m_RefTriangles.at(i).area()<MINREFAREA)
        {
            m_RefTriangles.erase(m_RefTriangles.begin() + i);
        }
    }

    m_TriMesh.makeMeshFromTriangles(m_RefTriangles, 0, pos, log, prefix);
    m_TriMesh.translatePanels(Tack);

    for(int i3=0; i3<m_TriMesh.nPanels(); i3++) m_TriMesh.panel(i3).setFromSTL(true);
    setTEfromIndexes();
    std::vector<int> errors;
    m_TriMesh.connectTrailingEdges(errors);
}


void Sail::panel3ComputeInviscidForces(std::vector<Panel3> const &panel3list,
                                       BoatPolar *, Vector3d const &cog,
                                       double beta, double *Cp3Vtx,
                                       Vector3d &CP, Vector3d &Force, Vector3d &Moment) const
{
    Vector3d leverArmPanelCoG;
    Vector3d ForcePt, PanelForce;

    // Define the wind axis
    double alpha = 0.0;
    //    Vector3d WindDirection = windDirection(alpha, beta);
    Vector3d WindNormal    = objects::windNormal(alpha, beta);
    //    Vector3d WindSide      = windSide(alpha, beta);

    Vector3d FiBodyAxis(0.0,0.0,0.0); // inviscid pressure forces, body axis
    Vector3d MiBodyAxis(0.0,0.0,0.0); // inviscid moment of pressure forces, body axis

    double CpAverage = 0.0;

    // Compute resultant and moment of pressure forces
    // Tip patches are included
    for(int i3=m_FirstPanel3Index; i3<m_FirstPanel3Index+m_TriMesh.nPanels(); i3++)
    {
        Panel3 const & p3 = panel3list.at(i3);
        int idx = p3.index();
        ForcePt = p3.CoG();
        //        if(pWPolar->isTriLinearMethod())
        {
            /** @todo for the time being Cp is uniform on the panel; if this is improved, replace with
             *  the basis function's CoG, or better with a full integration on the panel */
            idx*=3;
            CpAverage = (Cp3Vtx[idx]+Cp3Vtx[idx+1]+Cp3Vtx[idx+2])/3.0;
            PanelForce = p3.normal() * (-CpAverage) * p3.area();      // Newtons/q
        }
        /*        else
        {
            PanelForce = p3->normal() * (-Cp[idx]) * p3->area();         // Newtons/q
        }*/

        CP.x += ForcePt.x * PanelForce.dot(WindNormal); //global center of pressure
        CP.y += ForcePt.y * PanelForce.dot(WindNormal);
        CP.z += ForcePt.z * PanelForce.dot(WindNormal);

        leverArmPanelCoG = ForcePt - cog;                     // m

        FiBodyAxis += PanelForce;                             // N
        MiBodyAxis += leverArmPanelCoG * PanelForce;          // N.m/q

//qDebug("  %d Cp=%11g  N=%11g  %11g  %11g  F=%11g  %11g  %11g", i3, CpAverage, p3.normal().x, p3.normal().y, p3.normal().z, PanelForce.x, PanelForce.y, PanelForce.z);
    }

    // store the results
    Force = FiBodyAxis;                   // N/q
    Moment = MiBodyAxis;                     // N.m/q
}


void Sail::translate(const Vector3d &T)
{    
    for(uint it=0; it<m_RefTriangles.size(); it++)
    {
        m_RefTriangles[it].translate(T);
    }

    m_Clew.translate(T);
    m_Tack.translate(T);
    m_Peak.translate(T);
    m_Head.translate(T);
    m_Triangulation.translate(T);
}


void Sail::scaleArea(double newarea)
{
    double factor = sqrt(newarea/area());
    if(fabs(factor-1.0)>0.001)
        scale(factor, factor, factor);
}


void Sail::scale(double XFactor, double YFactor, double ZFactor)
{
    for(uint it=0; it<m_RefTriangles.size(); it++)
    {
        m_RefTriangles[it].scale(XFactor, YFactor, ZFactor);
    }

    m_Clew.x *= XFactor;
    m_Clew.y *= YFactor;
    m_Clew.z *= ZFactor;

    m_Tack.x *= XFactor;
    m_Tack.y *= YFactor;
    m_Tack.z *= ZFactor;

    m_Peak.x *= XFactor;
    m_Peak.y *= YFactor;
    m_Peak.z *= ZFactor;

    m_Head.x *= XFactor;
    m_Head.y *= YFactor;
    m_Head.z *= ZFactor;

    m_Triangulation.scale(XFactor, YFactor, ZFactor);
}


void Sail::flipXZ()
{
    m_Clew.y = -m_Clew.y;
    m_Tack.y = -m_Tack.y;
    m_Peak.y = -m_Peak.y;
    m_Head.y = -m_Head.y;

    m_Triangulation.flipXZ();
}


/** Approximates the surface's normal witth a local crossproduct of tangent vectors */
Vector3d Sail::normal(double xrel, double zrel, xfl::enumSurfacePosition pos) const
{
    Vector3d Tx, Tz;
    Vector3d P0, P1;

    double dl = 0.0001; // relative units [0, 1}
    double eps = 1.0e-6;
    // x vector
    if(fabs(xrel)<eps)
    {
        P0 = point(0, zrel, pos);
        P1 = point(2.0*dl, zrel, pos);
    }
    else if(fabs(1.0-xrel)<eps)
    {
        P0 = point(1.0-2.0*dl, zrel, pos);
        P1 = point(xrel, zrel, pos);
    }
    else
    {
        P0 = point(xrel-dl, zrel, pos);
        P1 = point(xrel+dl, zrel, pos);
    }
    Tx = P1-P0;

    // z vector
    if(fabs(zrel)<eps)
    {
        P0 = point(xrel, zrel, pos);
        P1 = point(xrel, zrel+2.0*dl, pos);
    }
    else if(fabs(1.0-zrel)<eps)
    {
        P0 = point(xrel, zrel-2.0*dl, pos);
        P1 = point(xrel, zrel, pos);
    }
    else
    {
        P0 = point(xrel, zrel-dl, pos);
        P1 = point(xrel, zrel+dl, pos);
    }
    Tz = P1-P0;

    return (Tx*Tz).normalized();
}


Node Sail::edgeNode(double xrel, double zrel, xfl::enumSurfacePosition pos) const
{
    Vector3d Tx, Tz;
    Vector3d P0, P1;

    double dl = 0.0001; // relative units [0, 1}
    double eps = 1.0e-6;
    // x vector
    if(fabs(xrel)<eps)
    {
        P0 = point(0, zrel, pos);
        P1 = point(2.0*dl, zrel, pos);
    }
    else if(fabs(1.0-xrel)<eps)
    {
        P0 = point(1.0-2.0*dl, zrel, pos);
        P1 = point(xrel, zrel, pos);
    }
    else
    {
        P0 = point(xrel-dl, zrel, pos);
        P1 = point(xrel+dl, zrel, pos);
    }
    Tx = P1-P0;

    // z vector
    if(fabs(zrel)<eps)
    {
        P0 = point(xrel, zrel, pos);
        P1 = point(xrel, zrel+2.0*dl, pos);
    }
    else if(fabs(1.0-zrel)<eps)
    {
        P0 = point(xrel, zrel-2.0*dl, pos);
        P1 = point(xrel, zrel, pos);
    }
    else
    {
        P0 = point(xrel, zrel-dl, pos);
        P1 = point(xrel, zrel+dl, pos);
    }
    Tz = P1-P0;

    Node nd;
    nd.setPosition(point(xrel, zrel, pos));
    nd.setNormal((Tz*Tx).normalized());
    return nd;
}


void Sail::addTEindex(int idx, bool bBotMid)
{
    if(bBotMid)
    {
        if(std::find(m_BotMidTEIndexes.begin(), m_BotMidTEIndexes.end(), idx) == m_BotMidTEIndexes.end())
            m_BotMidTEIndexes.push_back(idx);

        // remove from top indexes
        std::vector<int>::iterator it =  std::find(m_TopTEIndexes.begin(), m_TopTEIndexes.end(), idx);
        if(it!=m_TopTEIndexes.end()) m_TopTEIndexes.erase(it);
    }
    else
    {
        if(std::find(m_TopTEIndexes.begin(), m_TopTEIndexes.end(), idx) == m_TopTEIndexes.end())
            m_TopTEIndexes.push_back(idx);

        //remove from bot indexes
        std::vector<int>::iterator it =  std::find(m_BotMidTEIndexes.begin(), m_BotMidTEIndexes.end(), idx);
        if(it!=m_BotMidTEIndexes.end()) m_BotMidTEIndexes.erase(it);
    }
    updateStations();
}


void Sail::clearTEIndexes()
{
    for(uint i=0; i<m_TopTEIndexes.size(); i++)
    {
        int idx = m_TopTEIndexes.at(i);
        if(idx>=0 && idx<m_TriMesh.nPanels())
        {
            Panel3 &p3 = m_TriMesh.panel(idx);
            p3.setTrailing(false);
            p3.setOppositeIndex(-1);
        }
    }
    m_TopTEIndexes.clear();

    for(uint i=0; i<m_BotMidTEIndexes.size(); i++)
    {
        int idx = m_BotMidTEIndexes.at(i);
        if(idx>=0 && idx<m_TriMesh.nPanels())
        {
            Panel3 &p3 = m_TriMesh.panel(idx);
            p3.setTrailing(false);
            p3.setOppositeIndex(-1);
        }
    }
    m_BotMidTEIndexes.clear();

    updateStations();
}


void Sail::updateStations()
{
    int m_NStation = int(m_BotMidTEIndexes.size()); // redundant
    m_SpanResFF.resizeGeometry(m_NStation);
    m_SpanResFF.resizeResults(m_NStation);
    m_SpanResSum.resizeGeometry(m_NStation);
    m_SpanResSum.resizeResults(m_NStation);
}


bool Sail::removeTEindex(int i3, bool bBotMid)
{
    if(bBotMid)
    {
        std::vector<int>::iterator it =  std::find(m_BotMidTEIndexes.begin(), m_BotMidTEIndexes.end(), i3);
        if(it!=m_BotMidTEIndexes.end())
        {
            m_BotMidTEIndexes.erase(it);
            return true;
        }
    }
    else
    {
        std::vector<int>::iterator it =  std::find(m_TopTEIndexes.begin(), m_TopTEIndexes.end(), i3);
        if(it!=m_TopTEIndexes.end())
        {
            m_TopTEIndexes.erase(it);
            return true;
        }
    }
    return false;
}


void Sail::setTEfromIndexes()
{
    for(int i3=0; i3<m_TriMesh.nPanels(); i3++)
    {
        Panel3 &p3 = m_TriMesh.panel(i3);
        p3.setTrailing(false);
        p3.setOppositeIndex(-1);
        if(m_bThinSurface) p3.setSurfacePosition(xfl::MIDSURFACE);
        else               p3.setSurfacePosition(xfl::NOSURFACE);
    }

    // set opposite side panels
    for(uint i=0; i<m_BotMidTEIndexes.size(); i++)
    {
        int i3 = m_BotMidTEIndexes.at(i);
        if(i3>=0 && i3<m_TriMesh.nPanels())
        {
            Panel3 &p3b = m_TriMesh.panel(i3);
            p3b.setTrailing(true);

            if(m_bThinSurface) p3b.setSurfacePosition(xfl::MIDSURFACE);
            else               p3b.setSurfacePosition(xfl::BOTSURFACE);

            // triangles may not have been connected yet, so check the edges instead
            for(uint it=0; it<m_TopTEIndexes.size(); it++)
            {
                bool bFound = false;
                int idx = m_TopTEIndexes.at(it);
                if(idx>=0 && idx<m_TriMesh.nPanels())
                {
                    Panel3 &p3t = m_TriMesh.panel(idx);
                    for(int ie=0; ie<3; ie++)
                    {
                        if(p3b.edgeIndex(p3t.edge(ie), 0.0001)>=0)
                        {
                            p3b.setOppositeIndex(idx);
                            bFound = true;
                            break;
                        }
                    }
                }
                if(bFound) break;
            }
        }
    }

    for(uint i=0; i<m_TopTEIndexes.size(); i++)
    {
        int i3 = m_TopTEIndexes.at(i);
        if(i3>=0 && i3<m_TriMesh.nPanels())
        {
            Panel3 &p3t = m_TriMesh.panel(i3);
            p3t.setTrailing(true);
            p3t.setSurfacePosition(xfl::TOPSURFACE);

            // triangles may not have been connected yet, so check the edges instead
            for(uint it=0; it<m_BotMidTEIndexes.size(); it++)
            {
                int idx = m_BotMidTEIndexes.at(it);
                bool bFound = false;
                if(idx>=0 && idx<m_TriMesh.nPanels())
                {
                    Panel3 &p3b = m_TriMesh.panel(idx);
                    for(int ie=0; ie<3; ie++)
                    {
                        if(p3t.edgeIndex(p3b.edge(ie), 0.0001)>=0)
                        {
                            p3t.setOppositeIndex(idx);
                            bFound = true;
                            break;
                        }
                    }
                }
                if(bFound) break;
            }
        }
    }
}


double Sail::edgeLength(double umin, double vmin, double umax, double vmax) const
{
    int nVtx = 30;
    std::vector<Node> Vtx(nVtx);
    double edgelength = 0.0;
    Vtx[0] = edgeNode(umin, vmin);
    double u=0, v=0, dj=0;
    for(int j=1; j<nVtx; j++)
    {
        dj = double(j)/double(nVtx-1);
        u = umin + dj * (umax-umin);
        v = vmin + dj * (vmax-vmin);
        Vtx[j] = edgeNode(u, v);
        edgelength += Vtx.at(j).distanceTo(Vtx.at(j-1));
    }
    return edgelength;
}


double Sail::twist() const
{
    Vector3d foot = (m_Clew-m_Tack);
    Vector3d gaff = m_Peak-m_Head;

    double bottwist = atan2(foot.y, foot.x);
    double toptwist = atan2(gaff.y, gaff.x);
    return (toptwist-bottwist)*180.0/PI;
}




void Sail::makeRuledMesh(Vector3d const &LE)
{
    // Sail (mid) Surface:
    //   0           2
    //   _____________
    //   |          /|0
    //   | left   /  |
    //   | P3U  /    |
    //   |    /      |
    //   |  /  right |
    //   |/    P3D   |
    // 1 _____________
    //   1           2
    //
    //
    //   2           0
    //   _____________
    //   |          /|0
    //   | left   /  |
    //   | P3U  /    |
    //   |    /      |
    //   |  /  right |
    //   |/    P3D   |
    // 1 _____________
    //   2           1
    //
    int nx = m_NXPanels; // number of panels on a horizontal line
    int nz = m_NZPanels; // number of horizontal lines

    // make the nodes
    m_RefTriangles.clear();

    int nnodes = (nx+1)*(nz+1); // number of panels + 1
    std::vector<Node> nodes(nnodes);

    std::vector<double> xfrac, zfrac;
    xfl::getPointDistribution(xfrac, nx, m_XDistrib);
    xfl::getPointDistribution(zfrac, nz, m_ZDistrib);

    for(int i=0; i<nz+1; i++) // for each horizontal line
    {
        double zrel = zfrac[i];
        for(int j=0; j<nx+1; j++)  // for each vertical line
        {
            Node &nd = nodes[i*(nx+1) + j];
            double xrel = xfrac[nx-j];
            nd.setPosition(point(xrel,zrel) + LE);
            nd.setSurfacePosition(xfl::MIDSURFACE);
        }
    }

    //make the panels from the nodes

    Vector3d LA, LB, TA, TB;
    m_SpanResFF.resizeGeometry(nz);

    m_TopTEIndexes.clear();
    m_BotMidTEIndexes.clear();

    int it3d = 0;
    for(int i=0; i<nz; i++) // for each horizontal line
    {
        for(int j=0; j<nx; j++)  // for each vertical line
        {
            Triangle3d p3u, p3d;
            // LA, LB, TA, TB;
            int ita =  i   *(nx+1) + j;
            int itb = (i+1)*(nx+1) + j;
            int ila =  i   *(nx+1) + j+1;
            int ilb = (i+1)*(nx+1) + j+1;

            LA = nodes.at(ila); // do not take a potentially moving reference
            LB = nodes.at(ilb); // do not take a potentially moving reference
            TA = nodes.at(ita); // do not take a potentially moving reference
            TB = nodes.at(itb); // do not take a potentially moving reference

            p3u.setTriangle(LA, LB, TA);
            p3d.setTriangle(LB, TB, TA);

            m_RefTriangles.push_back(p3d);
            m_RefTriangles.push_back(p3u);

            if(j==0)
                m_BotMidTEIndexes.push_back(it3d);

            it3d+=2;
        }
    }
}


bool Sail::hasPanel3(int index) const
{
    if(index< m_FirstPanel3Index)           return false;
    if(index>=m_FirstPanel3Index+nPanel3()) return false;
    return true;
}


bool Sail::intersect(const Vector3d &A, const Vector3d &B, Vector3d &I, Vector3d &N) const
{
    Node nd;
    bool b = geom::intersectTriangles(m_Triangulation.triangles(), A, B, nd, true);

    I = nd;
    N = nd.normal();

    return b;
}
