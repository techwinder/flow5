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


#include <sailstl.h>

#include <trimesh.h>
#include <units.h>
#include <utils.h>


SailStl::SailStl() : ExternalSail()
{
    m_theStyle.m_Color.setRgba(151,201,249,125);
    m_Name = "STL type sail";
}


bool SailStl::serializeSailFl5(QDataStream &ar, bool bIsStoring)
{
    Sail::serializeSailFl5(ar, bIsStoring);
    int k=0, n=0;
    float xf=0,yf=0,zf=0;
    Vector3d V0, V1, V2;

    // 500001: new fl5 format
    // 500002: added top TE indexes in beta 18
    // 500003: brand new format
    int ArchiveFormat = 500003;

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << m_bThinSurface;
        ar << int(m_RefTriangles.size());
        for(uint i=0; i<m_RefTriangles.size(); i++)
        {
            Triangle3d const &t3d = m_RefTriangles.at(i);
            ar << t3d.vertexAt(0).xf() << t3d.vertexAt(0).yf() << t3d.vertexAt(0).zf();
            ar << t3d.vertexAt(1).xf() << t3d.vertexAt(1).yf() << t3d.vertexAt(1).zf();
            ar << t3d.vertexAt(2).xf() << t3d.vertexAt(2).yf() << t3d.vertexAt(2).zf();
        }

        ar << int(m_BotMidTEIndexes.size());
        for(int idx : m_BotMidTEIndexes)   ar << idx;

        ar << int(m_TopTEIndexes.size());
        for(int idx : m_TopTEIndexes)      ar << idx;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500000 || ArchiveFormat>510000) return false;
        if(ArchiveFormat<=500002)
        {
            // deprecated
            m_Triangulation.serializeFl5(ar, bIsStoring);
            m_Triangulation.makeNodes();
            m_Triangulation.makeNodeNormals();
            m_RefTriangles = m_Triangulation.triangles();

            if(ArchiveFormat>=500002)
                ar >> m_bThinSurface;

            ar >> n;
            for(int i=0; i<n; i++)
            {
                ar >> k;
                m_BotMidTEIndexes.push_back(k);
            }

            if(ArchiveFormat>=500002)
            {
                ar >> n;
                for(int i=0; i<n; i++)
                {
                    ar >> k;
                    m_TopTEIndexes.push_back(k);
                }
            }
        }
        else
        {
            ar >> m_bThinSurface;
            ar >> n;
            m_RefTriangles.resize(n);
            for(int i3=0; i3<n; i3++)
            {
                ar >> xf >> yf >> zf;
                V0.set(double(xf), double(yf), double(zf));

                ar >> xf >> yf >> zf;
                V1.set(double(xf), double(yf), double(zf));

                ar >> xf >> yf >> zf;
                V2.set(double(xf), double(yf), double(zf));

                m_RefTriangles[i3].setTriangle(V0, V1, V2);
            }

            m_Triangulation.setTriangles(m_RefTriangles);
            m_Triangulation.makeNodes();
            m_Triangulation.makeNodeNormals();

            ar >> n;
            for(int i=0; i<n; i++)
            {
                ar >> k;
                m_BotMidTEIndexes.push_back(k);
            }

            ar >> n;
            for(int i=0; i<n; i++)
            {
                ar >> k;
                m_TopTEIndexes.push_back(k);
            }
        }

        computeProperties();
        updateStations();
    }
    return true;
}


void SailStl::makeTriangulation(int , int )
{
    m_Triangulation.makeNodes();
    m_Triangulation.makeNodeNormals();
}


void SailStl::setTriangles(std::vector<Triangle3d> const &triangles)
{
    m_Triangulation.clear();
    m_Triangulation.setTriangles(triangles);
    m_RefTriangles = triangles;
    computeProperties();
}


void SailStl::translate(Vector3d const &T)
{
    Sail::translate(T);

    makeTriPanels(Vector3d());
    computeProperties();
}


void SailStl::rotate(const Vector3d &origin, const Vector3d &axis, double theta)
{
    m_Clew.rotate(origin, axis, theta);
    m_Tack.rotate(origin, axis, theta);
    m_Peak.rotate(origin, axis, theta);
    m_Head.rotate(origin, axis, theta);

    m_Triangulation.rotate(origin, axis, theta);
    for(uint it=0; it<m_RefTriangles.size(); it++)
        m_RefTriangles[it].rotate(origin, axis, theta);

    makeTriPanels(Vector3d());
    computeProperties();
}


void SailStl::flipXZ()
{
    ExternalSail::flipXZ();
    for(uint it=0; it<m_RefTriangles.size(); it++)
    {
        m_RefTriangles[it].flipXZ();
    }

    makeTriPanels(Vector3d());
    computeProperties();
}


void SailStl::properties(std::string &sailprops, const std::string &prefx, bool bFull) const
{
    QString props;
    QString strlength = Units::lengthUnitQLabel();
    QString strarea = Units::areaUnitQLabel();
    QString strange;
    QString frontspacer = QString::fromStdString(prefx);

    Vector3d foot = (m_Clew-m_Tack);
    Vector3d gaff = m_Peak-m_Head;
    double bottwist = atan2(foot.y, foot.x)*180.0/PI;
    double toptwist = atan2(gaff.y, gaff.x)*180.0/PI;

    props.clear();
    props += frontspacer + QString::fromStdString(m_Name) +"\n";
    if(bFull)
    {
        props += frontspacer + "   STL type sail\n";
    }
    strange = QString::asprintf("   Luff length  = %7.3f ", luffLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+"\n";
    strange = QString::asprintf("   Leech length = %7.3f ", leechLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+"\n";
    strange = QString::asprintf("   Foot length  = %7.3f ", footLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+"\n";
    strange = QString::asprintf("   Wetted area  = %7.3f ", m_WettedArea*Units::m2toUnit());
    props += frontspacer + strange + strarea + "\n";
    strange = QString::asprintf("   Head twist   = %7.3f ", toptwist);
    props += frontspacer + strange+ DEGch + "\n";
    strange = QString::asprintf("   Foot twist   = %7.3f ", bottwist);
    props += frontspacer + strange+ DEGch + "\n";
    strange = QString::asprintf("   Aspect ratio = %7.3f ", aspectRatio());
    props += frontspacer + strange+"\n";

    strange = QString::asprintf("   Triangle count = %d", m_Triangulation.nTriangles());
    props += frontspacer + strange;

    sailprops = props.toStdString();
}


void SailStl::scaleTwist(double newtwist)
{
    double deltatwist = newtwist-twist();
    Vector3d LE;
    Vector3d Luff = m_Head-m_Tack;
    double zrel=0;
    for(uint it=0; it<m_RefTriangles.size(); it++)
    {
        Triangle3d &t3d = m_RefTriangles[it];
        for(int ivtx=0; ivtx<3; ivtx++)
        {
            Vector3d &S = t3d.vertex(ivtx);
            zrel = (S.z-m_Tack.z)/(m_Head.z-m_Tack.z);
            LE.set(m_Tack + Luff*zrel);
            S.rotateZ(LE, deltatwist*zrel);
        }
        t3d.setTriangle();
    }

    m_Triangulation.setTriangles(m_RefTriangles);
    m_Triangulation.makeNodes();
    m_Triangulation.makeNodeNormals();

    m_Peak.rotateZ(m_Head, deltatwist);

    makeTriPanels(Vector3d());
    computeProperties();
}


void SailStl::scale(double XFactor, double YFactor, double ZFactor)
{
    Sail::scale(XFactor, YFactor, ZFactor);
    makeTriPanels(Vector3d());
    computeProperties();
}


void SailStl::scaleAR(double newAR)
{
    double ar = aspectRatio();
    if(ar<0.001)  return;
    if(newAR<0.001) return;

    double ratio = sqrt(newAR/ar);

    Vector3d LE;
    Vector3d Luff = m_Head-m_Tack;
    double zrel=0;
    for(uint it=0; it<m_RefTriangles.size(); it++)
    {
        Triangle3d &t3d = m_RefTriangles[it];
        for(int ivtx=0; ivtx<3; ivtx++)
        {
            Vector3d &S = t3d.vertex(ivtx);
            zrel = (S.z-m_Tack.z)/(m_Head.z-m_Tack.z);
            LE.set(m_Tack + Luff*zrel);
            S.x = LE.x + (S.x-LE.x)/ratio;
            S.z = m_Tack.z + (S.z-m_Tack.z)*ratio;
        }
        t3d.setTriangle();
    }

    m_Triangulation.setTriangles(m_RefTriangles);
    m_Triangulation.makeNodes();
    m_Triangulation.makeNodeNormals();

    //tack is unchanged
    m_Clew.x = m_Tack.x + (m_Clew.x-m_Tack.x)/ratio;
    m_Clew.z = m_Tack.z + (m_Clew.z-m_Tack.z)*ratio;

    m_Peak.x = m_Head.x + (m_Peak.x-m_Head.x)/ratio;
    m_Peak.z = m_Tack.z + (m_Peak.z-m_Tack.z)*ratio;

    m_Head.z = m_Tack.z + (m_Head.z-m_Tack.z)*ratio;


    makeTriPanels(Vector3d());
    computeProperties();
}


