/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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




#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include <sailocc.h>

#include <occ_globals.h>
#include <units.h>
#include <utils.h>

SailOcc::SailOcc() : ExternalSail()
{
    m_bRuledMesh = false;
}


void SailOcc::duplicate(Sail const*pSail)
{
    ExternalSail::duplicate(pSail);
    SailOcc const* pOccSail = dynamic_cast<SailOcc const*>(pSail);
    m_Shape = pOccSail->m_Shape;
    m_BRep  = pOccSail->m_BRep;
}


void SailOcc::makeSurface()
{
    m_Lx = fabs(m_Tack.x-m_Clew.x);
    m_Lx = std::max(m_Lx, fabs(m_Head.x-m_Peak.x));

    m_Ly = fabs(m_Tack.y-m_Clew.y);
    m_Ly = std::max(m_Ly, fabs(m_Head.y-m_Peak.y));

    m_Lz = fabs(m_Head.z-m_Tack.z);
    m_Lz = std::max(m_Lz, fabs(m_Peak.z-m_Clew.z));
}


void SailOcc::shapesToBreps()
{
    occ::shapesToBreps(m_Shape, m_BRep);
}


void SailOcc::makeTriangulation(int , int )
{
    // moved out of fl5-lib to avoid dependency to gmsh
//    int nada = 0;
}


void SailOcc::properties(std::string &properties, const std::string &prefx, bool bFull) const
{
    std::string props;
    std::string frontspacer = prefx;
    std::string strlength = Units::lengthUnitLabel();
    std::string strarea = Units::areaUnitLabel();
    std::string strange;
    Vector3d foot = m_Clew-m_Tack;
    Vector3d gaff = m_Peak-m_Head;
    double bottwist = atan2(foot.y, foot.x)*180.0/PI;
    double toptwist = atan2(gaff.y, gaff.x)*180.0/PI;

    props.clear();
    props += frontspacer + m_Name +"\n";
    if(bFull)
    {
        props += frontspacer + "   CAD type sail\n";
    }
    strange = std::format("   Luff length    = {:7.3g}", luffLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+ EOLstr;
    strange = std::format("   Leech length   = {:7.3g}", leechLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+ EOLstr;
    strange = std::format("   Foot length    = {:7.3g}", footLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+ EOLstr;
    strange = std::format("   Wetted area    = {:7.3g} ", m_WettedArea*Units::m2toUnit());
    props += frontspacer + strange + strarea+"\n";
    strange = std::format("   Head twist     = {:7.3g} ", toptwist);
    props += frontspacer + strange+ DEGstr + "\n";
    strange = std::format("   Foot twist     = {:7.3g} ", bottwist);
    props += frontspacer + strange+ DEGstr + "\n";
    strange = std::format("   Aspect ratio   = {:7.3g} ", aspectRatio());
    props += frontspacer + strange + "\n";
    strange = std::format("   Triangle count = {:d}", int(m_RefTriangles.size()));
    props += frontspacer + strange + "\n";

    if(m_Shape.Size()<=1)
        strange = std::format("   Sail is made of {:d} shape", m_Shape.Size());
    else
        strange = std::format("   Sail is made of {:d} shapes", m_Shape.Size());

    props += frontspacer+strange;
    if(bFull)
    {
        props +="\n";
        std::string str;
        for(TopTools_ListIteratorOfListOfShape shapeit(m_Shape); shapeit.More(); shapeit.Next())
        {
            occ::listShapeContent(shapeit.Value(), str, prefx);
            props += frontspacer+str;
        }
    }
    properties = props;
}


void SailOcc::flipXZ()
{
    ExternalSail::flipXZ();

    occ::flipShapesXZ(m_Shape);
    for(unsigned int it=0; it<m_RefTriangles.size(); it++)
    {
        m_RefTriangles[it].flipXZ();
    }
    makeTriPanels(Vector3d());
}

void SailOcc::scaleAR(double newAR)
{
    double ar = aspectRatio();
    if(ar<0.001)  return;
    if(newAR<0.001) return;

    double ratio = sqrt(newAR/ar);

    occ::scaleShapes(m_Shape, 1.0/ratio, 1.0, ratio);
    occ::shapesToBreps(m_Shape, m_BRep);

    //tack is unchanged
    m_Clew.x = m_Tack.x + (m_Clew.x-m_Tack.x)/ratio;
    m_Clew.z = m_Tack.z + (m_Clew.z-m_Tack.z)*ratio;

    m_Peak.x = m_Head.x + (m_Peak.x-m_Head.x)/ratio;
    m_Peak.z = m_Tack.z + (m_Peak.z-m_Tack.z)*ratio;

    m_Head.z = m_Tack.z + (m_Head.z-m_Tack.z)*ratio;
}


void SailOcc::translate(Vector3d const &T)
{
    Sail::translate(T);

    occ::translateShapes(m_Shape, T);
    occ::shapesToBreps(m_Shape, m_BRep);

    makeTriPanels(Vector3d());

    computeProperties();
}


void SailOcc::rotate(const Vector3d &origin, const Vector3d &axis, double theta)
{
    occ::rotateShapes(m_Shape, origin, axis, theta);
    occ::shapesToBreps(m_Shape, m_BRep);

    m_Triangulation.rotate(origin, axis, theta);
    for(unsigned int it=0; it<m_RefTriangles.size(); it++)
    {
        m_RefTriangles[it].rotate(origin, axis, theta);
    }
    makeTriPanels(Vector3d());

    m_Clew.rotate(origin, axis, theta);
    m_Tack.rotate(origin, axis, theta);
    m_Peak.rotate(origin, axis, theta);
    m_Head.rotate(origin, axis, theta);

    computeProperties();
}


void SailOcc::scale(double XFactor, double YFactor, double ZFactor)
{
    occ::scaleShapes(m_Shape, XFactor, YFactor, ZFactor);
    occ::shapesToBreps(m_Shape, m_BRep);

    m_Triangulation.scale(XFactor, YFactor, ZFactor);
    for(unsigned int it=0; it<m_RefTriangles.size(); it++)
    {
        m_RefTriangles[it].scale(XFactor, YFactor, ZFactor);
    }
    makeTriPanels(Vector3d());

    m_Clew *= XFactor;
    m_Peak *= XFactor;
    m_Head *= XFactor;
    m_Tack *= XFactor;
    computeProperties();
}

