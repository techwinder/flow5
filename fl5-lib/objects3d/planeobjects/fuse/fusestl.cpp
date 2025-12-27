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


#include <QDataStream>

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
    m_WettedArea = m_BaseTriangulation.wettedArea();
}


void FuseStl::computeSurfaceProperties(std::string &msg, const std::string &prefx)
{
//    computeWettedArea();

    m_Triangulation.computeSurfaceProperties(m_Length, m_MaxWidth, m_MaxHeight, m_WettedArea);

    QString strong, logmsg;
    QString prefix = QString::fromStdString(prefx);

    strong = QString::asprintf("Length          = %9.5g ", length()*Units::mtoUnit());
    strong += Units::lengthUnitQLabel() + "\n";
    logmsg += prefix + strong;

    strong = QString::asprintf("Max. width      = %9.5g ", m_MaxWidth*Units::mtoUnit());
    strong += Units::lengthUnitQLabel() + "\n";
    logmsg += prefix + strong;

    strong = QString::asprintf("Max. height     = %9.5g ", m_MaxHeight*Units::mtoUnit());
    strong += Units::lengthUnitQLabel() + "\n";
    logmsg += prefix + strong;

    strong = QString::asprintf("Wetted area     = %9.5g ", m_WettedArea*Units::m2toUnit());
    strong += Units::areaUnitQLabel();
    logmsg += prefix + strong;

    msg = logmsg.toStdString();
}


bool FuseStl::serializePartFl5(QDataStream &ar, bool bIsStoring)
{
    Fuse::serializePartFl5(ar, bIsStoring);
    int k(0);
    float xf(0),yf(0),zf(0);
    double x(0),y(0),z(0);
    int nIntSpares(0);
    int nDbleSpares(0);

    int n(0);
    double dble(0);

    Vector3d V0,V1,V2;

    // 500001: new fl5 format
    // 500002: storing variables as floats rather than doubles
    int ArchiveFormat = 500002;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << nTriangles();
        for(int i=0; i<nTriangles(); i++)
        {
            Triangle3d const &t3d = triangleAt(i);
            xf = t3d.vertexAt(0).xf(); yf = t3d.vertexAt(0).yf(); zf = t3d.vertexAt(0).zf();
            ar << xf <<yf << zf;
//            ar << t3d.vertex(0).xf() << t3d.vertex(0).yf() << t3d.vertex(0).zf();
            ar << t3d.vertexAt(1).xf() << t3d.vertexAt(1).yf() << t3d.vertexAt(1).zf();
            ar << t3d.vertexAt(2).xf() << t3d.vertexAt(2).yf() << t3d.vertexAt(2).zf();
        }

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        dble=0.0;
        for (int i=0; i<nDbleSpares; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500001 || ArchiveFormat>500010) return false;

        clearTriangles();
        ar >> k;
        for(int i=0; i<k; i++)
        {
            if(ArchiveFormat<500002) { ar >> x  >> y  >> z;    V0.set(x,y,z);}
            else
            {
                ar >> xf >> yf >> zf;
                V0.set(double(xf), double(yf), double(zf));
            }
            if(ArchiveFormat<500002) { ar >> x  >> y  >> z;    V1.set(x,y,z);}
            else
            {
                ar >> xf >> yf >> zf;
                V1.set(double(xf), double(yf), double(zf));
            }
            if(ArchiveFormat<500002) { ar >> x  >> y  >> z;    V2.set(x,y,z);}
            else
            {
                ar >> xf >> yf >> zf;
                V2.set(double(xf), double(yf), double(zf));
            }
            Triangle3d t3(V0,V1,V2);
            m_Triangulation.appendTriangle(t3);
        }


        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        makeFuseGeometry();
        makeTriangleNodes();
        makeNodeNormals();

        std::string logmsg;
        makeDefaultTriMesh(logmsg, "");
        m_BaseTriangulation.setTriangles(m_Triangulation.triangles());
    }
    return true;
}


// Intersects the triangulation, but not precise enough if the triangulation is coarse
bool FuseStl::intersectFuse(const Vector3d &A, const Vector3d &B, Vector3d &I, bool bMultiThreaded) const
{
    Node nd;
    bool b = geom::intersectTriangles(m_BaseTriangulation.triangles(), A, B, nd, bMultiThreaded);
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
    objects::computeSurfaceInertia(m_Inertia, m_BaseTriangulation.triangles(), PartPosition);
}


