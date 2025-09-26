/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#define _MATH_DEFINES_DEFINED

#include <QDataStream>

#include <xflobjects/sailobjects/sails/sail.h>
#include <xflobjects/objects3d/analysis/boatpolar.h>
#include <xflgeom/geom3d/triangle3d.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflpanels/mesh/trimesh.h>
#include <xflpanels/panels/panel3.h>
#include <xflpanels/panels/panel4.h>

int Sail::s_iXRes = 37;
int Sail::s_iZRes = 31;


Sail::Sail() : Part()
{
    m_Name = "Sail Name";
    m_theStyle.m_Color = QColor(155, 168, 174, 155);

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


void Sail::properties(QString &props, QString const &frontspacer, bool bFull) const
{
    QString strlength = Units::lengthUnitLabel();
    QString strarea = Units::areaUnitLabel();
    QString strange;
    QString format = xfl::isLocalized() ? "%L1 " : "%1 ";

    props.clear();
    props += frontspacer + m_Name +"\n";
    if(bFull)
    {
        if     (isNURBSSail())  props += frontspacer + "   NURBS type sail\n";
        else if(isSplineSail()) props += frontspacer + "   Spline type sail\n";
        else if(isWingSail())   props += frontspacer + "   Wing type sail\n";
        else if(isStlSail())    props += frontspacer + "   STL type sail\n";
        else if(isOccSail())    props += frontspacer + "   CAD type sail\n";
    }
    strange = QString("   Luff length    = "+format).arg(luffLength()*Units::mtoUnit(), 7, 'g', 3);
    props += frontspacer + strange + strlength+"\n";
    strange = QString("   Leech length   = "+format).arg(leechLength()*Units::mtoUnit(), 7, 'g', 3);
    props += frontspacer + strange + strlength+"\n";
    strange = QString("   Foot length    = "+format).arg(footLength()*Units::mtoUnit(), 7, 'g', 3);
    props += frontspacer + strange + strlength+"\n";
    strange = QString("   Area           = "+format).arg( area()*Units::m2toUnit(), 7, 'g', 3);
    props += frontspacer + strange + strarea+"\n";
    strange = QString("   Aspect ratio   = "+format).arg(aspectRatio(), 7, 'g', 3);
    props += frontspacer + strange + "\n";
    strange = QString("   Top twist      = "+format).arg(twist(), 7, 'g', 3);
    props += frontspacer + strange + DEGCHAR+"\n";
    strange = QString("   Triangle count = "+format).arg(m_RefTriangles.size());
    props += frontspacer + strange;
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
        for(int i=0; i<m_EdgeSplit.size(); i++)
        {
            ar << int(m_EdgeSplit[i].size());
            for(int j=0; j<m_EdgeSplit[i].size(); j++)
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
                m_EdgeSplit.first().resize(4);
            }
            else
            {
                m_EdgeSplit.resize(n);
                for(int i=0; i<m_EdgeSplit.size(); i++)
                {
                    ar >> n;
                    m_EdgeSplit[i].resize(n);
                    for(int j=0; j<m_EdgeSplit[i].size(); j++)
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
    double uk=0, uk1=0, vl=0, vl1=0;

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
    for(int i=0; i<m_RefTriangles.size(); i++)
    {
        m_RefTriangles[i].setNeighbours(m_TriMesh.panelAt(i).neighbours());
    }
}


void Sail::mergeRefNodes(Node const& src, Node const&dest)
{
    for(int i3=0; i3<m_RefTriangles.size(); i3++)
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
    QString log, prefix;

    xfl::enumSurfacePosition pos = m_bThinSurface ? xfl::MIDSURFACE : xfl::NOSURFACE;

    // clean the reference panels with the same criteria as the mesh panel
    for (int i=m_RefTriangles.size()-1; i>=0; i--)
    {
        if(m_RefTriangles.at(i).area()<MINREFAREA)
        {
            m_RefTriangles.removeAt(i);
        }
    }

    m_TriMesh.makeMeshFromTriangles(m_RefTriangles, 0, pos, log, prefix);
    m_TriMesh.translatePanels(Tack);

    for(int i3=0; i3<m_TriMesh.nPanels(); i3++) m_TriMesh.panel(i3).setFromSTL(true);
    setTEfromIndexes();
    QVector<int> errors;
    m_TriMesh.connectTrailingEdges(errors);
}


void Sail::panel3ComputeInviscidForces(QVector<Panel3> const &panel3list,
                                       BoatPolar *, Vector3d const &cog,
                                       double beta, double *Cp3Vtx,
                                       Vector3d &CP, Vector3d &Force, Vector3d &Moment) const
{
    Vector3d leverArmPanelCoG;
    Vector3d ForcePt, PanelForce;

    // Define the wind axis
    double alpha = 0.0;
    //    Vector3d WindDirection = windDirection(alpha, beta);
    Vector3d WindNormal    = windNormal(alpha, beta);
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
    for(int it=0; it<m_RefTriangles.size(); it++)
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
    for(int it=0; it<m_RefTriangles.size(); it++)
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
        if(!m_BotMidTEIndexes.contains(idx)) m_BotMidTEIndexes.append(idx);
        // remove from top indexes
        int itop = m_TopTEIndexes.indexOf(idx);
        if(itop>=0) m_TopTEIndexes.removeAt(itop);
    }
    else
    {
        if(!m_TopTEIndexes.contains(idx)) m_TopTEIndexes.append(idx);
        //remove from bot indexes
        int ibot = m_BotMidTEIndexes.indexOf(idx);
        if(ibot>=0) m_BotMidTEIndexes.removeAt(ibot);
    }
    updateStations();
}


void Sail::clearTEIndexes()
{
    for(int i=0; i<m_TopTEIndexes.size(); i++)
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

    for(int i=0; i<m_BotMidTEIndexes.size(); i++)
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
    int m_NStation = m_BotMidTEIndexes.size(); // redundant
    m_SpanResFF.resizeGeometry(m_NStation);
    m_SpanResFF.resizeResults(m_NStation);
    m_SpanResSum.resizeGeometry(m_NStation);
    m_SpanResSum.resizeResults(m_NStation);
}


void Sail::removeTEindex(int i3, bool bBotMid)
{
    if(bBotMid)
    {
        int idx = m_BotMidTEIndexes.indexOf(i3);
        if(idx>=0) m_BotMidTEIndexes.removeAt(idx);
    }
    else
    {
        int idx = m_TopTEIndexes.indexOf(i3);
        if(idx>=0) m_TopTEIndexes.removeAt(idx);
    }
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
    for(int i=0; i<m_BotMidTEIndexes.size(); i++)
    {
        int i3 = m_BotMidTEIndexes.at(i);
        if(i3>=0 && i3<m_TriMesh.nPanels())
        {
            Panel3 &p3b = m_TriMesh.panel(i3);
            p3b.setTrailing(true);

            if(m_bThinSurface) p3b.setSurfacePosition(xfl::MIDSURFACE);
            else               p3b.setSurfacePosition(xfl::BOTSURFACE);

            // triangles may not have been connected yet, so check the edges instead
            for(int it=0; it<m_TopTEIndexes.size(); it++)
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

    for(int i=0; i<m_TopTEIndexes.size(); i++)
    {
        int i3 = m_TopTEIndexes.at(i);
        if(i3>=0 && i3<m_TriMesh.nPanels())
        {
            Panel3 &p3t = m_TriMesh.panel(i3);
            p3t.setTrailing(true);
            p3t.setSurfacePosition(xfl::TOPSURFACE);

            // triangles may not have been connected yet, so check the edges instead
            for(int it=0; it<m_BotMidTEIndexes.size(); it++)
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

/** Builds an array of segments of equal length on an edge to be used as the SLG for the free mesh.
 Only used for internal thin sails of type NURBS or SPLINE*/
void Sail::edgeSegs(int nSegs, double xrel, double zrel, QVector<Segment3d> & slg)
{
    int nVtx = 300;

    Vector3d start, end;
    Node V0, V1;
    QVector<Node> Vtx(nVtx);
    if(xrel<=LENGTHPRECISION)
    {
        double edgelength = 0.0;
        for(int j=0; j<nVtx; j++)
        {
            double v = double(j)/double(nVtx-1);
            Vtx[j] = edgeNode(xrel, v);
            if(j>0)
                edgelength += Vtx.at(j).distanceTo(Vtx.at(j-1));
        }

        start = edgeNode(xrel, 0.0);
        end   = edgeNode(xrel, 1.0);

        V0 = start;
        for(int j=1; j<nSegs-1; j++)
        {
            double dist = 0.0;
            double dc = double(j)/double(nSegs-1) * edgelength;
            for(int iv=1; iv<nVtx; iv++)
            {
                dist += Vtx.at(iv).distanceTo(Vtx.at(iv-1));
                if(dist>=dc)
                {
                    V1 = (Vtx.at(iv-1)+Vtx.at(iv))/2.0;
                    slg.append({V0, V1});
                    V0 = V1;
                    break;
                }
            }
        }
        slg.append({V0, end});
    }
    else if(xrel>=1.0-LENGTHPRECISION)
    {
        double edgelength = 0.0;
        for(int j=0; j<nVtx; j++)
        {
            double v = double(nVtx-j-1)/double(nVtx-1);
            Vtx[j] = edgeNode(xrel, v);
            if(j>0)
                edgelength += Vtx.at(j).distanceTo(Vtx.at(j-1));
        }

        start = edgeNode(xrel, 1.0);
        end   = edgeNode(xrel, 0.0);

        V0 = start;
        for(int j=1; j<nSegs-1; j++)
        {
            double dist = 0.0;
            double dc = double(j)/double(nSegs-1) * edgelength;
            for(int iv=1; iv<nVtx; iv++)
            {
                dist += Vtx.at(iv).distanceTo(Vtx.at(iv-1));
                if(dist>=dc)
                {
                    V1 = (Vtx.at(iv-1)+Vtx.at(iv))/2.0;
                    slg.append({V0, V1});
                    V0 = V1;
                    break;
                }
            }
        }
        slg.append({V0, end});
    }
    else if(zrel<LENGTHPRECISION)
    {
        double edgelength = 0.0;
        for(int j=0; j<nVtx; j++)
        {
            double v = double(nVtx-j-1)/double(nVtx-1);
            Vtx[j] = edgeNode(v, zrel);
            if(j>0)
                edgelength += Vtx.at(j).distanceTo(Vtx.at(j-1));
        }
        start = edgeNode(1.0, zrel);
        end   = edgeNode(0.0, zrel);

        V0 = start;
        for(int j=1; j<nSegs-1; j++)
        {
            double dist = 0.0;
            double dc = double(j)/double(nSegs-1) * edgelength;
            for(int iv=1; iv<nVtx; iv++)
            {
                dist += Vtx.at(iv).distanceTo(Vtx.at(iv-1));
                if(dist>=dc)
                {
                    V1 = (Vtx.at(iv-1)+Vtx.at(iv))/2.0;
                    slg.append({V0, V1});
                    V0 = V1;
                    break;
                }
            }
        }
        slg.append({V0, end});
    }
    else if(zrel>=1.0-LENGTHPRECISION)
    {
        double edgelength = 0.0;
        for(int j=0; j<nVtx; j++)
        {
            double v = double(j)/double(nVtx-1);
            Vtx[j] = edgeNode(v, zrel);
            if(j>0)
                edgelength += Vtx.at(j).distanceTo(Vtx.at(j-1));
        }
        start = edgeNode(0.0, zrel);
        end   = edgeNode(1.0, zrel);

        V0 = start;
        for(int j=1; j<nSegs-1; j++)
        {
            double dist = 0.0;
            double dc = double(j)/double(nSegs-1) * edgelength;
            for(int iv=1; iv<nVtx; iv++)
            {
                dist += Vtx.at(iv).distanceTo(Vtx.at(iv-1));
                if(dist>=dc)
                {
                    V1 = (Vtx.at(iv-1)+Vtx.at(iv))/2.0;
                    slg.append({V0, V1});
                    V0 = V1;
                    break;
                }
            }
        }
        slg.append({V0, end});
    }
}


double Sail::edgeLength(double umin, double vmin, double umax, double vmax) const
{
    int nVtx = 30;
    QVector<Node> Vtx(nVtx);
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
    QVector<Node> nodes(nnodes);

    QVector<double> xfrac, zfrac;
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

            m_RefTriangles.append(p3d);
            m_RefTriangles.append(p3u);

            if(j==0)
                m_BotMidTEIndexes.append(it3d);

            it3d+=2;
        }
    }
}


/** Builds the edge SLG for the free mesh.
    Only used for thin sails of the NURBS or SPLINE types.*/
void Sail::makeEdgeSLG(SLG3d & slg) const
{
//    int nVtx = (m_NXPanels+1)*2+(m_NZPanels+1)*2 - 3;
    int nVtx = 1;
    QVector<EdgeSplit> const& es = m_EdgeSplit.first();

    for(int i=0; i<4; i++) nVtx += es[i].nSegs();

    slg.resize(nVtx-1);

    QVector<Node> Vtx(nVtx);
    int ivtx=0;
    Vtx[ivtx++].set(edgeNode(0,0));

    Node P0, P1;
    int nSegs(0);
    double umin(0), umax(0),vmin(0), vmax(0);
    double u0(0), u1(0), v0(0), v1(0);
    double tl(0);
    double length(0);
    double l(0);

    double eps = 0.0001;
    QVector<double> psplit;

    int iter=0;

    for(int iSide=0; iSide<4; iSide++)
    {
        nSegs = es[iSide].nSegs();
        xfl::getPointDistribution(psplit, nSegs, es[iSide].distrib());
        if(iSide==0)
        {
            umin=0, umax=0;
            vmin=0, vmax=1;
        }
        else if(iSide==1)
        {
            umin=0, umax=1;
            vmin=1, vmax=1;
        }
        else if(iSide==2)
        {
            umin=1, umax=1;
            vmin=1, vmax=0;
        }
        else if(iSide==3)
        {
            umin=1, umax=0;
            vmin=0, vmax=0;
        }

        l = edgeLength(umin, vmin, umax, vmax);

        // proceed using dichotomy
        for(int i=1; i<nSegs; i++)
        {
            tl = l * psplit.at(i); // target length
            u0 = umin;
            u1 = umax;
            v0 = vmin;
            v1 = vmax;

            P0 = edgeNode(u0, v0);
            P1 = edgeNode(u1, v1);
            iter = 0;
            do
            {
                length = edgeLength(umin, vmin, (u0+u1)/2.0, (v0+v1)/2.0);
                if(length<tl)
                {
                    u0 = (u0+u1)/2.0;
                    v0 = (v0+v1)/2.0;
                }
                else
                {
                    u1 = (u0+u1)/2.0;
                    v1 = (v0+v1)/2.0;
                }
            }
            while(fabs(length-tl)>eps && iter++<20);

            Vtx[ivtx++] = edgeNode((u0+u1)/2.0, (v0+v1)/2.0);
        }
        Vtx[ivtx++] = edgeNode(umax, vmax);
    }

    for(int i=0; i<Vtx.size()-1; i++)
    {
        slg[i].setNodes(Vtx[i], Vtx[i+1]);
    }
}


bool Sail::hasPanel3(int index) const
{
    if(index< m_FirstPanel3Index)           return false;
    if(index>=m_FirstPanel3Index+nPanel3()) return false;
    return true;
}


