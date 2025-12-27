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


#include <QString>


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


bool PlaneSTL::serializePlaneFl5(QDataStream &ar, bool bIsStoring)
{
    int k(0);
    float x(0),y(0),z(0);
    int nIntSpares(0);
    int nDbleSpares(0);
    QString strange;

    int n(0);
    double dble(0);

    // 500001: new fl5 format
    // 500002: added geom and inertia data; serialized the style properly - beta 14
    // 500003: serialized mesh info instead of triangulation info
    // 500004: surface color
    // 500005: base triangulation in beta20

    int ArchiveFormat = 500005;
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << QString::fromStdString(m_Name);
        ar << QString::fromStdString(m_Description);
        m_SurfaceColor.serialize(ar, bIsStoring);
        m_theStyle.serializeFl5(ar, bIsStoring);

        ar << m_bReversed;
        ar << m_ReferenceArea << m_ReferenceChord << m_ReferenceSpan;
        ar << m_WettedArea << m_Span << m_Length << m_Height;

        m_Inertia.serializeFl5(ar, bIsStoring);

        m_RefTriMesh.serializeMeshFl5(ar, bIsStoring);

        m_Triangulation.serializeFl5(ar, bIsStoring);

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        nDbleSpares=0;
        ar << nDbleSpares;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500000 || ArchiveFormat>500100) return false;

        ar >> strange;   m_Name = strange.trimmed().toStdString();
        ar >> strange;   m_Description = strange.trimmed().toStdString();
        if(ArchiveFormat>=500004) m_SurfaceColor.serialize(ar, false);
        if(ArchiveFormat<500002)
        {
            ar >> k; m_theStyle.m_Stipple = LineStyle::convertLineStyle(k);
            ar >> m_theStyle.m_Width;
            ar >> k; m_theStyle.m_Symbol=LineStyle::convertSymbol(k);
            m_theStyle.m_Color.serialize(ar, false);
        }

        if(ArchiveFormat>=500002)
        {
            m_theStyle.serializeFl5(ar, bIsStoring);
            ar >> m_bReversed;
            ar >> m_ReferenceArea >> m_ReferenceChord >> m_ReferenceSpan;
            ar >> m_WettedArea >> m_Span >> m_Length >> m_Height;
            m_Inertia.serializeFl5(ar, bIsStoring);
        }

        if(ArchiveFormat<=500002)
        {
            clearTriangles();
            ar >> k;
            Vector3d V0, V1, V2;
            for(int i=0; i<k; i++)
            {
                ar >> x >> y >> z;
                V0.set(double(x), double(y), double(z));
                ar >> x >> y >> z;
                V1.set(double(x), double(y), double(z));
                ar >> x >> y >> z;
                V2.set(double(x), double(y), double(z));
                Triangle3d t3(V0,V1,V2);
                m_Triangulation.appendTriangle(t3);
            }

            // space allocation
            ar >> nIntSpares;
            for (int i=0; i<nIntSpares; i++) ar >> n;
            ar >> nDbleSpares;
            for (int i=0; i<nDbleSpares; i++) ar >> dble;

            makeTriangleNodes();
            makeNodeNormals();
            makeTriMesh(false);
            m_RefTriMesh = m_TriMesh;
        }
        else
        {
            m_RefTriMesh.serializeMeshFl5(ar, bIsStoring);
            for(int i3=0; i3<m_RefTriMesh.nPanels(); i3++)
            {
                m_RefTriMesh.panel(i3).setFromSTL(true);
            }
            m_TriMesh = m_RefTriMesh;

            if(ArchiveFormat<500005)
            {
                // make triangulation from mesh
                m_Triangulation.setNodes(m_RefTriMesh.nodes());
                m_Triangulation.setTriangleCount(m_RefTriMesh.nPanels());
                for(int i3=0; i3<m_RefTriMesh.nPanels(); i3++)
                {
                    Panel3 const &p3 = m_RefTriMesh.panelAt(i3);
                    Triangle3d &t3d = m_Triangulation.triangle(i3);
                    t3d.setVertices(p3.vertices());
                    t3d.setTriangle();
                }
            }
            else
            {
                m_Triangulation.serializeFl5(ar, bIsStoring);
                m_Triangulation.makeNodes();
            }
            m_Triangulation.makeNodeNormals();

            // space allocation
            ar >> nIntSpares;
            ar >> nDbleSpares;

            if(m_bAutoInertia)
                computeStructuralInertia();
            m_bIsInitialized = true;
        }
    }
    return true;
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
    QString strange, strong, prefix;

    QString lengthlab, surfacelab, masslab, arealab;
    lengthlab   = Units::lengthUnitQLabel();
    surfacelab  = Units::areaUnitQLabel();
    masslab     = Units::massUnitQLabel();
    arealab     = Units::areaUnitQLabel();

    strong = QString::asprintf("Ref. span length  = %7g ", m_ReferenceSpan*Units::mtoUnit());
    strong += lengthlab;
    strange += strong+ EOLch;

    strong = QString::asprintf("Ref. area         = %7g ", m_ReferenceArea*Units::m2toUnit());
    strong += Units::areaUnitQLabel();
    strange += strong+ EOLch;

    strong = QString::asprintf("Ref. chord length = %7g ", m_ReferenceChord*Units::mtoUnit());
    strong += lengthlab;
    strange += strong+ EOLch;

    strong = QString::asprintf("Mass              = %7g ", totalMass()*Units::kgtoUnit());
    strong += masslab;
    strange += strong+ EOLch;

    strong = QString::asprintf("CoG = (%.3f, %.3f, %.3f) ", m_Inertia.CoG_t().x*Units::mtoUnit(), m_Inertia.CoG_t().y*Units::mtoUnit(), m_Inertia.CoG_t().z*Units::mtoUnit());
    strong += lengthlab;
    strange += strong+ EOLch;

    strong = QString::asprintf("Wing Load         = %7g ", totalMass()*Units::kgtoUnit()/m_ReferenceArea/Units::m2toUnit());
    strong += masslab + "/" + surfacelab;
    strange += strong+ EOLch;

    strong = QString::asprintf("Length            = %9.5g ", m_Length*Units::mtoUnit());
    strong += lengthlab+ EOLch;
    strange += prefix + strong;

    strong = QString::asprintf("Max. width        = %9.5g ", m_Span*Units::mtoUnit());
    strong += lengthlab+ EOLch;
    strange += prefix + strong;

    strong = QString::asprintf("Max. height       = %9.5g ", m_Height*Units::mtoUnit());
    strong += lengthlab+ EOLch;
    strange += prefix + strong;

    strong = QString::asprintf("Wetted area       = %9.5g ", m_WettedArea*Units::m2toUnit());
    strong += arealab + EOLch;
    strange += prefix + strong;

    strong = QString::asprintf("Triangulation     = %d", m_Triangulation.nTriangles());
    strange += prefix + strong+ EOLch;

    strong = QString::asprintf("Triangular panels = %d", m_RefTriMesh.nPanels());
    strange += prefix + strong;

    return strange.toStdString();
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
        m_RefTriMesh.makeConnectionsFromNodePosition(bConnectTE, bMultiThreaded);
        m_RefTriMesh.connectNodes();

        std::vector<int>errorlist;
        bOK = m_RefTriMesh.connectTrailingEdges(errorlist);
        if(errorlist.size()) bOK = false;
        m_TriMesh = m_RefTriMesh;
    }
    else
    {
        m_TriMesh.makeConnectionsFromNodePosition(bConnectTE, bMultiThreaded);
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







