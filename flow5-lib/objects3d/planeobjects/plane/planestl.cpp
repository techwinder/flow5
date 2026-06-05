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


#include <format>


#include <planestl.h>
#include <objects_global.h>
#include <units.h>
#include <utils.h>
#include <geom_global.h>

PlaneSTL::PlaneSTL() : Plane()
{
    m_SurfaceColor = fl5Color(200,200,200);

    m_Name="STL type plane";
    m_bReversed = false;
    m_WettedArea = m_Span = m_Length = m_Height = 0.0;
    m_ReferenceArea = m_ReferenceSpan = m_ReferenceChord = 1.0;
}


void PlaneSTL::copyMetaData(const Plane *pOtherPlane)
{
    PlaneSTL const *pPlaneSTL = dynamic_cast<PlaneSTL const*>(pOtherPlane);

    m_Description  = pOtherPlane->description();
    m_theStyle     = pOtherPlane->theStyle();
    m_SurfaceColor = pPlaneSTL->surfaceColor();
}


void PlaneSTL::duplicate(Plane const *pPlane)
{
    if(!pPlane->isSTLType()) return;
    PlaneSTL const *pPlaneSTL = dynamic_cast<PlaneSTL const*>(pPlane);
    if(!pPlaneSTL) return;

    Plane::duplicate(pPlane);

    m_SurfaceColor = pPlaneSTL->m_SurfaceColor;
    m_RefTriMesh = pPlaneSTL->m_RefTriMesh;
    m_TriMesh = pPlaneSTL->triMesh();
    m_Triangulation = pPlaneSTL->triangulation();

    m_bReversed = pPlaneSTL->m_bReversed;

    m_ReferenceArea  = pPlaneSTL->m_ReferenceArea;
    m_ReferenceChord = pPlaneSTL->m_ReferenceChord;
    m_ReferenceSpan  = pPlaneSTL->m_ReferenceSpan;

    m_WettedArea = pPlaneSTL->m_WettedArea;
    m_Span       = pPlaneSTL->m_Span;
    m_Length     = pPlaneSTL->m_Length;
    m_Height     = pPlaneSTL->m_Height;
}


void PlaneSTL::duplicatePanels(Plane const *pPlane)
{
    if(!pPlane->isSTLType()) return;
    PlaneSTL const *pPlaneSTL = dynamic_cast<PlaneSTL const*>(pPlane);
    m_RefTriMesh = pPlaneSTL->m_RefTriMesh;
    m_TriMesh    = pPlaneSTL->m_TriMesh;
}


/** Assumes the base triangles have been set, and makers evertyhing else */
void PlaneSTL::makePlane(bool , bool , bool )
{
    makeTriangleNodes();
    makeNodeNormals();
    makeTriMesh();
    computeSurfaceProperties();
    if(m_bAutoInertia)
        computeStructuralInertia();
    m_bIsInitialized = true;
}


int PlaneSTL::nStations() const
{
    int n=0;
    for(int i=0; i<m_RefTriMesh.nPanels(); i++)
    {
        if(m_RefTriMesh.panelAt(i).isTrailing() && m_RefTriMesh.panelAt(i).isBotPanel()) n++;
    }
    return n;
}


void PlaneSTL::computeStructuralInertia()
{
    objects::computeSurfaceInertia(m_Inertia, m_Triangulation.triangles(), Vector3d());
}


void PlaneSTL::computeSurfaceProperties()
{
    m_Triangulation.computeSurfaceProperties(m_Length, m_Span, m_Height, m_WettedArea);
}


std::string PlaneSTL::planeData(bool) const
{
    std::string strange, strong, prefix;

    std::string lengthlab, surfacelab, masslab, arealab;
    lengthlab   = Units::lengthUnitLabel();
    surfacelab  = Units::areaUnitLabel();
    masslab     = Units::massUnitLabel();
    arealab     = Units::areaUnitLabel();

    strong = std::format("Ref. span length  = {:7g} ", m_ReferenceSpan*Units::mtoUnit());
    strong += lengthlab;
    strange += strong+ EOLstr;

    strong = std::format("Ref. area         = {:7g} ", m_ReferenceArea*Units::m2toUnit());
    strong += Units::areaUnitLabel();
    strange += strong+ EOLstr;

    strong = std::format("Ref. chord length = {:7g} ", m_ReferenceChord*Units::mtoUnit());
    strong += lengthlab;
    strange += strong+ EOLstr;

    strong = std::format("Mass              = {:7g} ", totalMass()*Units::kgtoUnit());
    strong += masslab;
    strange += strong+ EOLstr;

    strong = std::format("CoG = ({:.3f}, {:.3f}, {:.3f}) ", m_Inertia.CoG_t().x*Units::mtoUnit(), m_Inertia.CoG_t().y*Units::mtoUnit(), m_Inertia.CoG_t().z*Units::mtoUnit());
    strong += lengthlab;
    strange += strong+ EOLstr;

    strong = std::format("Wing Load         = {:7g} ", totalMass()*Units::kgtoUnit()/m_ReferenceArea/Units::m2toUnit());
    strong += masslab + "/" + surfacelab;
    strange += strong+ EOLstr;

    strong = std::format("Length            = {:9.5g} ", m_Length*Units::mtoUnit());
    strong += lengthlab+ EOLstr;
    strange += prefix + strong;

    strong = std::format("Max. width        = {:9.5g} ", m_Span*Units::mtoUnit());
    strong += lengthlab+ EOLstr;
    strange += prefix + strong;

    strong = std::format("Max. height       = {:9.5g} ", m_Height*Units::mtoUnit());
    strong += lengthlab+ EOLstr;
    strange += prefix + strong;

    strong = std::format("Wetted area       = {:9.5g} ", m_WettedArea*Units::m2toUnit());
    strong += arealab + EOLstr;
    strange += prefix + strong;

    strong = std::format("Triangulation     = {:d}", m_Triangulation.nTriangles());
    strange += prefix + strong+ EOLstr;

    strong = std::format("Triangular panels = {:d}", m_RefTriMesh.nPanels());
    strange += prefix + strong;

    return strange;
}


void PlaneSTL::rotate(Vector3d const &O, Vector3d const &axis, double theta)
{
    m_Triangulation.rotate(O, axis, theta);
    m_RefTriMesh.rotatePanels(O, axis, theta);
    m_TriMesh = m_RefTriMesh;
    m_Inertia.rotateMasses(O, axis, theta);
    if(m_bAutoInertia)
        computeStructuralInertia();
}


void PlaneSTL::scale(double scalefactor)
{
    m_Triangulation.scale(scalefactor, scalefactor, scalefactor);
    m_RefTriMesh.scale(scalefactor, scalefactor, scalefactor);
    m_TriMesh = m_RefTriMesh;
    m_Inertia.scaleMassPositions(scalefactor);
    if(m_bAutoInertia)
        computeStructuralInertia();
}


void PlaneSTL::translate(Vector3d const &T)
{
    m_Triangulation.translate(T);
    m_RefTriMesh.translatePanels(T.x, T.y, T.z);
    m_TriMesh = m_RefTriMesh;
    m_Inertia.translateMasses(T);
    if(m_bAutoInertia)
        computeStructuralInertia();
}


void PlaneSTL::makeTriMesh(bool)
{
    std::string log, prefix;
    m_RefTriMesh.makeMeshFromTriangles(m_Triangulation.triangles(), 0, xfl::NOSURFACE, log, prefix);
    for(int i3=0; i3<m_RefTriMesh.nPanels(); i3++)
    {
        m_RefTriMesh.panel(i3).setFromSTL(true);
    }
    m_TriMesh = m_RefTriMesh;
}


bool PlaneSTL::intersectTriangles(Vector3d A, Vector3d B, Vector3d &I, bool bMultiThreaded)
{
    Node nd;
    bool b = geom::intersectTriangles(m_Triangulation.triangles(), A, B, nd, bMultiThreaded);
    I = nd;
    return b;
}


bool PlaneSTL::connectTriMesh(bool bRefTriMesh, bool bConnectTE, bool bMultiThreaded)
{
    bool bOK = true;
    if(bRefTriMesh)
    {
        m_RefTriMesh.makeConnectionsFromNodePositions(bConnectTE, bMultiThreaded);
        m_RefTriMesh.connectNodes();

        std::vector<int>errorlist;
        bOK = m_RefTriMesh.connectTrailingEdges(errorlist);
        if(errorlist.size()) bOK = false;
        m_TriMesh = m_RefTriMesh;
    }
    else
    {
        m_TriMesh.makeConnectionsFromNodePositions(bConnectTE, bMultiThreaded);
        m_TriMesh.connectNodes();

        std::vector<int>errorlist;
        bOK = m_TriMesh.connectTrailingEdges(errorlist);
        if(errorlist.size()) bOK = false;
    }
    return bOK;
}


double PlaneSTL::maxSize() const
{
    double maxl = m_Length;
    maxl = std::max(maxl, m_Span);
    maxl = std::max(maxl, m_Height);
    return maxl;
}


bool PlaneSTL::guessTEPanels(float TEMaxAngle, std::string &logmsg)
{
    if(nPanel3()<=0)
    {
        logmsg += "Plane has no triangles\n";
        return 0;
    }

    Panel3 &p3t = refTriMesh().panel(0);
    if(p3t.neighbourCount()==0)
    {
        logmsg += "Connect the panels before attempting to guess the T.E.\n";
        return 0;
    }

    double ccrit = cos((180.0-TEMaxAngle)*PI/180.0);

    int iTE=0;
    for(int i3=0; i3<refTriMesh().nPanels(); i3++)
    {
        Panel3 &p3 = refTriMesh().panel(i3);
        p3.setTrailing(false);
        p3.setSurfacePosition(xfl::NOSURFACE);
        p3.setOppositeIndex(-1);
    }

    double zcrit = 0.1;

    for(int i3t=0; i3t<refTriMesh().nPanels(); i3t++)
    {
        Panel3 &p3t = refTriMesh().panel(i3t);

        for(int i3b=0; i3b<p3t.neighbourCount(); i3b++)
        {
            int idx = p3t.neighbour(i3b);
            if(idx>=0 && idx<refTriMesh().nPanels())
            {
                Panel3 &p3b = refTriMesh().panel(idx);
                double cos = p3t.normal().dot(p3b.normal());
                if(p3b.oppositeIndex()<0 && cos<ccrit)
                {
                    // we have a TE
                    p3t.setTrailing(true);
                    p3t.setOppositeIndex(p3b.index());
                    p3b.setTrailing(true);
                    p3b.setOppositeIndex(p3t.index());

                    if(p3t.normal().z>zcrit && p3b.normal().z<-zcrit)
                    {
                        p3t.setSurfacePosition(xfl::TOPSURFACE);
                        p3b.setSurfacePosition(xfl::BOTSURFACE);
                    }
                    else if(p3b.normal().z>zcrit && p3t.normal().z<-zcrit)
                    {
                        p3t.setSurfacePosition(xfl::BOTSURFACE);
                        p3b.setSurfacePosition(xfl::TOPSURFACE);
                    }
                    else if(p3t.normal().y>zcrit && p3b.normal().y<-zcrit)
                    {
                        p3t.setSurfacePosition(xfl::TOPSURFACE);
                        p3b.setSurfacePosition(xfl::BOTSURFACE);
                    }
                    else if(p3b.normal().y>zcrit && p3t.normal().y<-zcrit)
                    {
                        p3t.setSurfacePosition(xfl::BOTSURFACE);
                        p3b.setSurfacePosition(xfl::TOPSURFACE);
                    }
                    else
                    {
                        //anything
                        p3t.setSurfacePosition(xfl::TOPSURFACE);
                        p3b.setSurfacePosition(xfl::BOTSURFACE);
                    }
                    iTE++;
                    break;
                }
            }
        }
    }

    std::vector<int> errorlist;
    refTriMesh().connectTrailingEdges(errorlist); // re-orders vertices

    logmsg += std::format("Found {:d} pairs of TE panels.\n", iTE);
    for(int idx : errorlist)
    {
        logmsg += std::format("   TE error at index = {:d}\n", idx);
    }

    if(errorlist.size()) return false;
    else                 return true;
}




