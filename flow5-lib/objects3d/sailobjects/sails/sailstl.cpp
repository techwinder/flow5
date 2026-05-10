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


#include <sailstl.h>

#include <trimesh.h>
#include <units.h>
#include <utils.h>


SailStl::SailStl() : ExternalSail()
{
    m_theStyle.m_Color.setRgba(151,201,249,125);
    m_Name = "STL type sail";
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
    for(unsigned int it=0; it<m_RefTriangles.size(); it++)
        m_RefTriangles[it].rotate(origin, axis, theta);

    makeTriPanels(Vector3d());
    computeProperties();
}


void SailStl::flipXZ()
{
    ExternalSail::flipXZ();
    for(unsigned int it=0; it<m_RefTriangles.size(); it++)
    {
        m_RefTriangles[it].flipXZ();
    }

    makeTriPanels(Vector3d());
    computeProperties();
}


void SailStl::properties(std::string &sailprops, const std::string &prefx, bool bFull) const
{
    std::string props;
    std::string strlength = Units::lengthUnitLabel();
    std::string strarea = Units::areaUnitLabel();
    std::string strange;
    std::string frontspacer = prefx;

    Vector3d foot = (m_Clew-m_Tack);
    Vector3d gaff = m_Peak-m_Head;
    double bottwist = atan2(foot.y, foot.x)*180.0/PI;
    double toptwist = atan2(gaff.y, gaff.x)*180.0/PI;

    props.clear();
    props += frontspacer + m_Name +"\n";
    if(bFull)
    {
        props += frontspacer + "   STL type sail\n";
    }
    strange = std::format("   Luff length  = {:7.3f} ", luffLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+"\n";
    strange = std::format("   Leech length = {:7.3f} ", leechLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+"\n";
    strange = std::format("   Foot length  = {:7.3f} ", footLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+"\n";
    strange = std::format("   Wetted area  = {:7.3f} ", m_WettedArea*Units::m2toUnit());
    props += frontspacer + strange + strarea + "\n";
    strange = std::format("   Head twist   = {:7.3f} ", toptwist);
    props += frontspacer + strange+ DEGstr + "\n";
    strange = std::format("   Foot twist   = {:7.3f} ", bottwist);
    props += frontspacer + strange+ DEGstr + "\n";
    strange = std::format("   Aspect ratio = {:7.3f} ", aspectRatio());
    props += frontspacer + strange+"\n";

    strange = std::format("   Triangle count = {:d}", m_Triangulation.nTriangles());
    props += frontspacer + strange;

    sailprops = props;
}


void SailStl::scaleTwist(double newtwist)
{
    double deltatwist = newtwist-twist();
    Vector3d LE;
    Vector3d Luff = m_Head-m_Tack;
    double zrel=0;
    for(unsigned int it=0; it<m_RefTriangles.size(); it++)
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
    for(unsigned int it=0; it<m_RefTriangles.size(); it++)
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


