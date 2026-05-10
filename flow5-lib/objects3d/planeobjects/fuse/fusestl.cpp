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


#include <fusestl.h>
#include <units.h>
#include <geom_global.h>
#include <objects_global.h>


FuseStl::FuseStl() : Fuse()
{
    m_Name = "STL type fuse";
    m_FuseType = Fuse::Stl;
}


void FuseStl::makeFuseGeometry()
{
    std::string logmsg;
    computeSurfaceProperties(logmsg, "");
}


void FuseStl::rotate(const Vector3d &origin, const Vector3d &axis, double theta)
{
    Fuse::rotate(origin, axis, theta);
    std::string strange;
    makeDefaultTriMesh(strange, "");
}


void FuseStl::scale(double XFactor, double YFactor, double ZFactor)
{
    Fuse::scale(XFactor, YFactor, ZFactor);
    std::string strange;
    makeDefaultTriMesh(strange, "");
}


void FuseStl::translate(Vector3d const &T)
{
    Fuse::translate(T);
    std::string strange;
    makeDefaultTriMesh(strange, "");
}


void FuseStl::computeWettedArea()
{
    m_WettedArea = m_Triangulation.wettedArea();
}


void FuseStl::computeSurfaceProperties(std::string &msg, const std::string &prefx)
{
//    computeWettedArea();

    m_Triangulation.computeSurfaceProperties(m_Length, m_MaxWidth, m_MaxHeight, m_WettedArea);

    std::string strong, logmsg;
    std::string prefix = prefx;

    strong = std::format("Length          = {:9.5g} ", length()*Units::mtoUnit());
    strong += Units::lengthUnitLabel() + "\n";
    logmsg += prefix + strong;

    strong = std::format("Max. width      = {:9.5g} ", m_MaxWidth*Units::mtoUnit());
    strong += Units::lengthUnitLabel() + "\n";
    logmsg += prefix + strong;

    strong = std::format("Max. height     = {:9.5g} ", m_MaxHeight*Units::mtoUnit());
    strong += Units::lengthUnitLabel() + "\n";
    logmsg += prefix + strong;

    strong = std::format("Wetted area     = {:9.5g} ", m_WettedArea*Units::m2toUnit());
    strong += Units::areaUnitLabel();
    logmsg += prefix + strong;

    msg = logmsg;
}


// Intersects the triangulation, but not precise enough if the triangulation is coarse
bool FuseStl::intersectFuse(const Vector3d &A, const Vector3d &B, Vector3d &I, bool bMultiThreaded) const
{
    Node nd;
    bool b = geom::intersectTriangles(m_Triangulation.triangles(), A, B, nd, bMultiThreaded);
    I = nd;
    return b;
}


int FuseStl::makeDefaultTriMesh(std::string &logmsg, std::string const &prefix)
{
    m_TriMesh.makeMeshFromTriangles(m_Triangulation.triangles(), 0, xfl::FUSESURFACE, logmsg, prefix);
    for(int i3=0; i3<m_TriMesh.nPanels(); i3++) m_TriMesh.panel(i3).setSurfacePosition(xfl::FUSESURFACE);
    for(int in=0; in<m_TriMesh.nNodes(); in++) m_TriMesh.node(in).setSurfacePosition(xfl::FUSESURFACE);

    return m_TriMesh.panelCount();
}



void FuseStl::computeStructuralInertia(Vector3d const &PartPosition)
{
    if(!m_bAutoInertia) return;
    objects::computeSurfaceInertia(m_Inertia, m_Triangulation.triangles(), PartPosition);
}


