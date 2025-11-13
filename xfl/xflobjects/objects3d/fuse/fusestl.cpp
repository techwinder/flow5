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

#include <QDataStream>
#include <QtConcurrent/QtConcurrent>

#include "fusestl.h"
#include <xflcore/units.h>


FuseStl::FuseStl() : Fuse()
{
    m_Name = "STL type fuse";
    m_FuseType = Fuse::Stl;
}


void FuseStl::makeFuseGeometry()
{
    QString logmsg;
    computeSurfaceProperties(logmsg, QString());
}


void FuseStl::rotate(const Vector3d &origin, const Vector3d &axis, double theta)
{
    Fuse::rotate(origin, axis, theta);
    QString strange;
    makeDefaultTriMesh(strange, QString());
}


void FuseStl::scale(double XFactor, double YFactor, double ZFactor)
{
    Fuse::scale(XFactor, YFactor, ZFactor);
    QString strange;
    makeDefaultTriMesh(strange, QString());
}


void FuseStl::translate(Vector3d const &T)
{
    Fuse::translate(T);
    QString strange;
    makeDefaultTriMesh(strange, QString());
}


void FuseStl::computeWettedArea()
{
    m_WettedArea = m_BaseTriangulation.wettedArea();
}


void FuseStl::computeSurfaceProperties(QString &logmsg, QString const &prefix)
{
//    computeWettedArea();

    m_Triangulation.computeSurfaceProperties(m_Length, m_MaxWidth, m_MaxHeight, m_WettedArea);

    QString strong;

    strong = QString::asprintf("Length          = %9.5g ", length()*Units::mtoUnit());
    strong += Units::lengthUnitLabel() + "\n";
    logmsg += prefix + strong;

    strong = QString::asprintf("Max. width      = %9.5g ", m_MaxWidth*Units::mtoUnit());
    strong += Units::lengthUnitLabel() + "\n";
    logmsg += prefix + strong;

    strong = QString::asprintf("Max. height     = %9.5g ", m_MaxHeight*Units::mtoUnit());
    strong += Units::lengthUnitLabel() + "\n";
    logmsg += prefix + strong;

    strong = QString::asprintf("Wetted area     = %9.5g ", m_WettedArea*Units::m2toUnit());
    strong += Units::areaUnitLabel();
    logmsg += prefix + strong;
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

        ar << triangleCount();
        for(int i=0; i<triangleCount(); i++)
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

        QString logmsg;
        makeDefaultTriMesh(logmsg, QString());
        m_BaseTriangulation.setTriangles(m_Triangulation.triangles());
    }
    return true;
}


// Intersects the triangulation, but not precise enough is the triangulation is coarse
bool FuseStl::intersectFuse(const Vector3d &A, const Vector3d &B, Vector3d &I, bool) const
{
    m_nBlocks = QThread::idealThreadCount();

    QVector<bool> bIntersect(m_nBlocks, 0);
    QVector<Vector3d> S(m_nBlocks);

/*    if(s_bMultiThreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            futureSync.addFuture(QtConcurrent::run(this, &FuseStl::blockIntersect,
                                                   iBlock, A, B, S.data()+iBlock, bIntersect.data()+iBlock));
        }
        futureSync.waitForFinished();
    }
    else */
    {
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            blockIntersect(iBlock, A, B, S.data()+iBlock, bIntersect.data()+iBlock);
        }
    }

    bool bResult = false;
    double dmax = 1.e10;
    for(int i=0; i<m_nBlocks; i++)
    {
        if(bIntersect[i])
        {
            bResult = true;
            double d = sqrt((S.at(i).x-A.x)*(S.at(i).x-A.x)+(S.at(i).y-A.y)*(S.at(i).y-A.y)+(S.at(i).z-A.z)*(S.at(i).z-A.z));
            if(d<dmax)
            {
                I = S[i];
                dmax=d;
            }
        }
    }

    return bResult;
}


/** return the point closest to A among the intersected triangles*/
bool FuseStl::blockIntersect(int iBlock, Vector3d A, Vector3d B, Vector3d *I, bool *bIntersect) const
{
    Vector3d Int;

    double dmax = 1.e100;

    int blocksize = int(m_BaseTriangulation.nTriangles()/m_nBlocks)+1; // add one to compensate for rounding errors
    int iStart = iBlock*blocksize;
    int maxRows = m_BaseTriangulation.nTriangles();
    int iMax = std::min(iStart+blocksize, maxRows);

    *bIntersect = false;

    for(int it=iStart; it<iMax; it++)
    {
        Triangle3d const &t3 = m_BaseTriangulation.triangleAt(it);
        if(t3.intersectSegmentInside(A, B, Int, true))
        {
            double d = sqrt((Int.x-A.x)*(Int.x-A.x)+(Int.y-A.y)*(Int.y-A.y)+(Int.z-A.z)*(Int.z-A.z));
            if(d<dmax)
            {
                *I = Int;
                dmax=d;
            }
            *bIntersect = true;
        }
    }

    return *bIntersect;
}


int FuseStl::makeDefaultTriMesh(QString &logmsg, QString const &prefix)
{
    m_TriMesh.makeMeshFromTriangles(m_Triangulation.triangles(), 0, xfl::FUSESURFACE, logmsg, prefix);
    for(int i3=0; i3<m_TriMesh.nPanels(); i3++) m_TriMesh.panel(i3).setSurfacePosition(xfl::FUSESURFACE);
    for(int in=0; in<m_TriMesh.nNodes(); in++) m_TriMesh.node(in).setSurfacePosition(xfl::FUSESURFACE);

    return m_TriMesh.panelCount();
}


