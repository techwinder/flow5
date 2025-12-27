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





#include <TColgp_Array2OfPnt.hxx>
#include <Geom_BSplineSurface.hxx>
#include <GeomAPI_PointsToBSplineSurface.hxx>


#include <fusesections.h>
#include <geom_global.h>
#include <occ_globals.h>


FuseSections::FuseSections() : FuseXfl(Fuse::Sections)
{
    m_iHighlightSection = -1;
    m_iActiveSection = -1;
    m_iHighlightedPoint = -1;
    m_iActivePoint = -1;

    m_FitPrecision = 0.001;
}


void FuseSections::makeNURBS()
{
    if(nSections()<=1) return;
    if(m_Section.front().size()<=1) return;

    TColgp_Array2OfPnt array(1, nSections(), 1, int(m_Section.front().size()));

    for(int i=0; i<nSections(); i++)
    {
        std::vector<Vector3d> const &section = m_Section.at(i);
        for(int j=0; j<int(section.size()); j++)
        {
            Vector3d const &pt = section.at(j);
            array.SetValue(i+1, j+1, {pt.x, pt.y, pt.z});
        }
    }

    Handle (Geom_BSplineSurface) occsurf;
    try
    {
        occsurf = GeomAPI_PointsToBSplineSurface(array, 1, 5, GeomAbs_C2, m_FitPrecision);
    }
    catch (Standard_OutOfRange const &ex)
    {
        qDebug("error making poles %s \n", ex.GetMessageString());
    }
    catch (Standard_ConstructionError const &ex)
    {
        qDebug("error making poles %s \n", ex.GetMessageString());
    }
    catch (Standard_NoSuchObject const &ex)
    {
        qDebug("error making poles %s \n", ex.GetMessageString());
    }
    catch(...)
    {
        qDebug("unknown error making poles\n");
    }

    // convert the OCC nurbs to an xfl nurbs
    occ::makeXflNurbsfromOccNurbs(occsurf, m_nurbs);

    m_nurbs.setKnots();

    m_xPanels.resize(m_nurbs.frameCount());
    m_hPanels.resize(m_nurbs.framePointCount());

    std::fill(m_xPanels.begin(), m_xPanels.end(), 1);
    std::fill(m_hPanels.begin(), m_hPanels.end(), 1);
}


void FuseSections::makeDefaultFuse()
{
    m_theStyle.m_Color.setRgb(89,136,143);
    m_nxNurbsPanels = 19;
    m_nhNurbsPanels = 7;

    m_xPanels.clear();
    m_hPanels.clear();

    m_Section.resize(5);
    for(int ifr=0; ifr<nSections(); ifr++)
    {
        m_Section[ifr].resize(5);
    }

    setPoint(0,0, {0.0, 0.0, -0.0422});
    setPoint(0,1, {0.0, 0.0, -0.0422});
    setPoint(0,2, {0.0, 0.0, -0.0422});
    setPoint(0,3, {0.0, 0.0, -0.0422});
    setPoint(0,4, {0.0, 0.0, -0.0422});

    setPoint(1,0, {0.402, 0.0000,  0.095});
    setPoint(1,1, {0.402, 0.0406,  0.074});
    setPoint(1,2, {0.402, 0.0608, -0.00465});
    setPoint(1,3, {0.402, 0.0406, -0.0842});
    setPoint(1,4, {0.402, 0.0000, -0.117});

    setPoint(2,0, {0.786, 0.0000,  0.0674});
    setPoint(2,1, {0.786, 0.0352,  0.0572});
    setPoint(2,2, {0.786, 0.0571,  0.0129});
    setPoint(2,3, {0.786, 0.0361, -0.0279});
    setPoint(2,4, {0.786, 0.0000, -0.0411});

    setPoint(3,0, {1.535, 0.0000, 0.05949});
    setPoint(3,1, {1.535, 0.01127, 0.05444});
    setPoint(3,2, {1.535, 0.01708, 0.03288});
    setPoint(3,3, {1.535, 0.01239, 0.01024});
    setPoint(3,4, {1.535, 0.0000, 0.0035});

    setPoint(4,0, {1.666, 0.000, 0.04452});
    setPoint(4,1, {1.666, 0.000, 0.04452});
    setPoint(4,2, {1.666, 0.000, 0.04452});
    setPoint(4,3, {1.666, 0.000, 0.04452});
    setPoint(4,4, {1.666, 0.000, 0.04452});

    setPanelPos();

    makeNURBS();
    std::string logmsg;
    makeDefaultTriMesh(logmsg, "");
}


void FuseSections::setPoint(int isec, int ipt, Vector3d const &pt)
{
    if(isec<0||isec>=nSections()) return;
    std::vector<Vector3d> &section = m_Section[isec];
    if(ipt<0 ||ipt>=int(section.size())) return;
    section[ipt].set(pt);
}


void FuseSections::setNSections(int nsections)
{
    m_Section.resize(nsections);
}


void FuseSections::setNPoints(int npts)
{
    for(int i=0; i<nSections(); i++)
    {
        m_Section[i].resize(npts);
    }
}


bool FuseSections::serializePartFl5(QDataStream &ar, bool bIsStoring)
{
    FuseXfl::serializePartFl5(ar, bIsStoring);
    int k=0;
    int n=0;
    int nIntSpares=0;
    int nDbleSpares=0;
    double dble=0;

    // 500001: new fl5 format
    // 500002: added fit precision
    int ArchiveFormat = 500002;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << nSections();
        if(nSections()>0)
            ar << int(m_Section.front().size());

        for(int is=0; is<nSections(); is++)
        {
            std::vector<Vector3d> const & section = m_Section.at(is);
            for(int ic=0; ic<int(section.size()); ic++)
            {
                Vector3d const & pt = section.at(ic);
                ar << pt.x << pt.y << pt.z;
            }
        }

        ar << m_FitPrecision;

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;

    }
    else
    {
        ar >> ArchiveFormat;
        if((ArchiveFormat<500001) || (ArchiveFormat>500002)) return false;

        ar >> n;
        m_Section.resize(n);

        if(n>0)
        {
            ar >> k;
            for(int i=0; i<n; i++) m_Section[i].resize(k);
        }

        for(int is=0; is<nSections(); is++)
        {
            std::vector<Vector3d> & section = m_Section[is];
            for(int ic=0; ic<int(section.size()); ic++)
            {
                Vector3d & pt = section[ic];
                ar >> pt.x >> pt.y >> pt.z;
            }
        }

        if(ArchiveFormat>=500002)
            ar >> m_FitPrecision;

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        // make the shapes, shells, and triangulation
        makeFuseGeometry();
        computeStructuralInertia(Vector3d());

        // make the default triangular mesh
        std::string strange;
        makeDefaultTriMesh(strange, "");
    }

    return true;
}


void FuseSections::makeFuseGeometry()
{
    makeNURBS();
    FuseXfl::makeFuseGeometry();
}


void FuseSections::scale(double XFactor, double YFactor, double ZFactor)
{
    for(int is=0; is<nSections(); is++)
    {
        std::vector<Vector3d> &section = m_Section[is];
        for(int ic=0; ic<int(section.size()); ic++)
        {
            section[ic].x *= XFactor;
            section[ic].y *= YFactor;
            section[ic].z *= ZFactor;
        }
    }
    makeFuseGeometry();
}


void FuseSections::scaleFrame(double YFactor, double ZFactor, int FrameID)
{
    if(FrameID<0 || FrameID>=nSections()) return;
    std::vector<Vector3d> &section = m_Section[FrameID];
    for(int ic=0; ic<int(section.size()); ic++)
    {
        section[ic].y *= YFactor;
        section[ic].z *= ZFactor;
    }
    makeFuseGeometry();
}


void FuseSections::translateFrame(Vector3d T, int FrameID)
{
    if(FrameID<0 || FrameID>=nSections()) return;
    std::vector<Vector3d> &section = m_Section[FrameID];
    for(int ic=0; ic<int(section.size()); ic++)
    {
        section[ic].translate(T);
    }
    makeFuseGeometry();
}


void FuseSections::translate(Vector3d const &T)
{
    for(int is=0; is<nSections(); is++)
    {
        std::vector<Vector3d> &section = m_Section[is];
        for(int ic=0; ic<int(section.size()); ic++)
        {
            section[ic].translate(T);
        }
    }
    makeFuseGeometry();
}


void FuseSections::duplicateFuse(const Fuse &aFuse)
{
    FuseSections const&fusepts = dynamic_cast<const FuseSections&>(aFuse);

    m_Section = fusepts.m_Section;
    FuseXfl::duplicateFuse(aFuse);
}


int FuseSections::isPoint(const Vector3d &Point, double deltax, double deltay, double deltaz) const
{
    if(m_iActiveSection<0 || m_iActiveSection>=nSections()) return -10;
    std::vector<Vector3d> const &section = m_Section.at(m_iActiveSection);
    for(int l=0; l<int(section.size()); l++)
    {
        if (fabs(Point.x-section.at(l).x)<deltax &&
            fabs(Point.y-section.at(l).y)<deltay &&
            fabs(Point.z-section.at(l).z)<deltaz)
              return l;
    }
    return -10;
}


int FuseSections::isSection(Vector3d const &Real, double deltaX, double deltaZ) const
{
    for (int k=0; k<nSections(); k++)
    {
        double h = (m_Section.at(k).front().z + m_Section.at(k).back().z)/2.0;
        if (fabs(Real.x-m_Section.at(k).front().x) < deltaX &&
            fabs(Real.z-h)                         < deltaZ)
            return k;
    }
    return -10;
}


void FuseSections::setActiveSectionPosition(double x, double z)
{
    if(m_iActiveSection<0 || m_iActiveSection>=nSections()) return;
    std::vector<Vector3d> & section = m_Section[m_iActiveSection];
    if(section.size()==0) return;
    double zpos = (section.front().z+section.back().z)/2.0;;

    for (int ic=0; ic<int(section.size()); ic++)
    {
        section[ic].x  = x;
        section[ic].z += z - zpos;
    }
}


void FuseSections::setSectionXPosition(int isec, double x)
{
    if(isec<0 || isec>=nSections()) return;
    std::vector<Vector3d> & section = m_Section[isec];
    for (int ic=0; ic<int(section.size()); ic++)
    {
        section[ic].x  = x;
    }
}


/**
 * Creates the body panels for this fuse object
 * The panels are created in the following order
 *    - for the port side  first, then for the starboard side
 *    - from bottom to top
 *    - from tip to tail
 * The panels are appended to the existing array of panels
 * @return the number of panels which have been created and appended
 */
int FuseSections::makeQuadMesh(int idx0, const Vector3d &pos)
{
    m_Panel4.clear();

    double uk=0.0, uk1=0.0, v=0.0;

    std::vector<Vector3d> refnode;

    Vector3d LA, LB, TA, TB;

    int n0=-1, n1=-1, n2=-1, n3=-1;
    int nx = nxNurbsPanels();
    int nh = nhNurbsPanels();
    int pcount = 0;

    int fullSize =0;

    double dpx=pos.x;
    double dpy=pos.y;
    double dpz=pos.z;

    fullSize = idx0 + 2*nx*nh;
    //start with left side... same as for wings
    for (int k=0; k<nx; k++)
    {
        uk  = m_XPanelPos.at(k);
        uk1 = m_XPanelPos.at(k+1);

        getPoint(uk,  0, false, LB);
        getPoint(uk1, 0, false, TB);

        LB.x += dpx;
        LB.y += dpy;
        LB.z += dpz;
        TB.x += dpx;
        TB.y += dpy;
        TB.z += dpz;

        for (int l=0; l<nh; l++)
        {
            m_Panel4.push_back({});
            Panel4 &p4 = m_Panel4.back();
            //start with left side... same as for wings
            v = double(l+1) / double(nh);
            getPoint(uk,  v, false, LA);
            getPoint(uk1, v, false, TA);

            LA.x += dpx;
            LA.y += dpy;
            LA.z += dpz;
            TA.x += dpx;
            TA.y += dpy;
            TA.z += dpz;

//            n0 = refnode.indexOf(LA);
            n0 = geom::isVector3d(refnode, LA);
            if(n0>=0)  p4.m_iLA = n0;
            else {
                p4.m_iLA = int(refnode.size());
                refnode.push_back(LA);
            }

//            n1 = refnode.indexOf(TA);
            n1 = geom::isVector3d(refnode, TA);
            if(n1>=0)  p4.m_iTA = n1;
            else {
                p4.m_iTA = int(refnode.size());
                refnode.push_back(TA);
            }

//            n2 = refnode.indexOf(LB);
            n2 = geom::isVector3d(refnode, LB);
            if(n2>=0) p4.m_iLB = n2;
            else {
                p4.m_iLB = int(refnode.size());
                refnode.push_back(LB);
            }

//            n3 = refnode.indexOf(TB);
            n3 = geom::isVector3d(refnode, TB);
            if(n3 >=0) p4.m_iTB = n3;
            else {
                p4.m_iTB = int(refnode.size());
                refnode.push_back(TB);
            }

            p4.m_bIsInSymPlane  = false;
            p4.m_bIsLeading     = false;
            p4.m_bIsTrailing    = false;
            p4.m_Pos = xfl::FUSESURFACE;
            p4.setIndex(idx0 + nPanel4()-1);
            p4.m_bIsLeftWingPanel  = true;
            p4.setPanelFrame(LA, LB, TA, TB);

            // set neighbour panels

            p4.m_iPD = p4.index() + nh;
            p4.m_iPU = p4.index() - nh;

            if(k==0)    p4.m_iPU = -1;// no panel downstream
            if(k==nx-1) p4.m_iPD = -1;// no panel upstream

            p4.m_iPL = p4.index() + 1;
            p4.m_iPR = p4.index() - 1;

            if(l==0)     p4.m_iPR = fullSize - pcount - 1;
            if(l==nh-1)  p4.m_iPL = fullSize - pcount - 1;

            LB = LA;
            TB = TA;
            pcount++;
        }
    }


    //right side next
    int i4 = nPanel4();

    for (int k=nx-1; k>=0; k--)
    {
        for (int l=nh-1; l>=0; l--)
        {
            i4--;
            m_Panel4.push_back({});
            Panel4 &p4 = m_Panel4.back();

            LA = refnode[m_Panel4[i4].m_iLB];
            TA = refnode[m_Panel4[i4].m_iTB];
            LB = refnode[m_Panel4[i4].m_iLA];
            TB = refnode[m_Panel4[i4].m_iTA];

            LA.y = -LA.y + 2*dpy;
            LB.y = -LB.y + 2*dpy;
            TA.y = -TA.y + 2*dpy;
            TB.y = -TB.y + 2*dpy;

//            n0 = refnode.indexOf(LA);
            n0 = geom::isVector3d(refnode, LA);
            if(n0>=0)  p4.m_iLA = n0;
            else {
                p4.m_iLA = int(refnode.size());
                refnode.push_back(LA);
            }

//            n1 = refnode.indexOf(TA);
            n1 = geom::isVector3d(refnode, TA);
            if(n1>=0) p4.m_iTA = n1;
            else {
                p4.m_iTA = int(refnode.size());
                refnode.push_back(TA);
            }

//            n2 = refnode.indexOf(LB);
            n2 = geom::isVector3d(refnode, LB);
            if(n2>=0)  p4.m_iLB = n2;
            else {
                p4.m_iLB = int(refnode.size());
                refnode.push_back(LB);
            }

//            n3 = refnode.indexOf(TB);
            n3 = geom::isVector3d(refnode, TB);
            if(n3 >=0)  p4.m_iTB = n3;
            else {
                p4.m_iTB = int(refnode.size());
                refnode.push_back(TB);
            }

            p4.m_bIsInSymPlane  = false;
            p4.m_bIsLeading     = false;
            p4.m_bIsTrailing    = false;
            p4.m_Pos = xfl::FUSESURFACE;
            p4.setIndex(idx0 + nPanel4()-1);
            p4.m_bIsLeftWingPanel  = false;
            p4.setPanelFrame(LA, LB, TA, TB);

            // set neighbour panels

            p4.m_iPD = idx0 + nPanel4() - nh -1;
            p4.m_iPU = idx0 + nPanel4() + nh -1;

            if(k==0) p4.m_iPU = -1;// no panel downstream
            if(k==nx-1) p4.m_iPD = -1;// no panel upstream

            p4.m_iPL = p4.index() + 1;
            p4.m_iPR = p4.index() - 1;

            if(l==0)     p4.m_iPL = fullSize - pcount - 1;
            if(l==nh-1)  p4.m_iPR = fullSize - pcount - 1;

            LB = LA;
            TB = TA;

            pcount++;
        }
    }
    int nQuads = nPanel4()-idx0;

    return nQuads;
}


void FuseSections::setSelectedPoint(Vector3d const &pt)
{
    if(m_iActiveSection<0 || m_iActiveSection>=nSections()) return;
    std::vector<Vector3d> & section = m_Section[m_iActiveSection];
    if(m_iActivePoint<0 || m_iActivePoint>=int(section.size())) return;
    section[m_iActivePoint] = pt;
}


bool FuseSections::hasActivePoint() const
{
    if(!m_iActiveSection) return false;
    if(m_iActiveSection>=nSections()) return false;
    return m_iActivePoint>=0 && m_iActivePoint<int(m_Section.at(m_iActiveSection).size());
}



int FuseSections::insertFrame(Vector3d const &Real)
{
    m_iHighlightSection= -1;
    if(m_Section.size()==0)
    {
        m_Section.resize(1);
        m_Section.front().resize(3);
        for(int i=0; i<nSections(); i++)
            m_Section.front()[i].x = Real.x;
        return 1;
    }

    if(m_Section.front().size()==0)
    {
        for(int i=0; i<nSections(); i++)
        {
            m_Section[i].resize(3);
        }
    }

    if(Real.x<m_Section.front().front().x)
    {
        m_Section.insert(m_Section.begin(), std::vector<Vector3d>());
        m_xPanels.insert(m_xPanels.begin(), 1);
        std::vector<Vector3d> &first = m_Section.front();
        first.resize(m_Section[1].size());
        for (int k=0; k<int(first.size()); k++)
        {
            first[k].set(Real.x,0.0,Real.z);
        }
        return 0;
    }
    else if(Real.x>m_Section.back().front().x)
    {
        m_Section.push_back(std::vector<Vector3d>());
        m_xPanels.push_back(1);
        std::vector<Vector3d> &last = m_Section.back();
        last.resize(m_Section.front().size());
        for (int k=0; k<int(last.size()); k++)
        {
            last[k].set(Real.x,0.0,Real.z);
        }
        return nSections()-1;
    }
    else
    {
        for (int n=0; n<nSections()-1; n++)
        {
            std::vector<Vector3d> &secn  = m_Section[n];
            std::vector<Vector3d> &secn1 = m_Section[n+1];
            if(secn.front().x<=Real.x  &&  Real.x<secn1.front().x)
            {
                m_Section.insert(m_Section.begin()+n+1,  std::vector<Vector3d>(secn.size()));
                m_xPanels.insert(m_xPanels.begin()+n+1, 1);
                // update references after resize
                std::vector<Vector3d> &secn  = m_Section[n];
                std::vector<Vector3d> &secn1 = m_Section[n+1];
                std::vector<Vector3d> &secn2 = m_Section[n+2];

                for (uint k=0; k<secn1.size(); k++)
                {
                    secn1[k].x = (secn[k].x + secn2[k].x)/2.0;
                    secn1[k].y = (secn[k].y + secn2[k].y)/2.0;
                    secn1[k].z = (secn[k].z + secn2[k].z)/2.0;
                }
                m_iActiveSection = n+1;
                return n+1;
            }
        }
    }

    return -1;
}


void FuseSections::insertPoint(int iPt)
{
    if(!nSections()) return;
    if(iPt<0||iPt>int(m_Section.front().size())) return;

    Vector3d pt;
    for(int isec=0; isec<nSections(); isec++)
    {
        std::vector<Vector3d> section = m_Section[isec];
        if(iPt==0)
        {
            pt.set(section.front());
            pt.z += 0.1;
        }
        else if (iPt==nSections()-1)
        {
            pt.set(section.back());
            pt.z -= 0.1;
        }

        pt = (section.at(iPt-1)+section.at(iPt))/2.0;
        section.insert(section.begin() + iPt, pt);
    }

    m_hPanels.insert(m_hPanels.begin()+iPt, 1);

    makeNURBS();
}


/**
 * Inserts a control point in the selected Frame.
 * @param Real the Vector3d which defines the point to insert
 * @return the index of the control point in the array; control points are indexed from bottom to top on the left side.
 */
void FuseSections::insertPoint(Vector3d const &Real)
{
    if(!nSections()) return;
    if(!hasActiveSection()) return;

    std::vector<Vector3d> &section = activeSection();

    int n=0;
    double areal = atan2(Real.y, Real.z) *180/PI;

    for(n=0; n<int(section.size()); n++)
    {
        double actrlPt = atan2(section.at(n).y, section.at(n).z) *180/PI;
        if(areal<=actrlPt) break;
    }
    n--;

    m_iActivePoint = n+1;

    insertPoint(n+1);
    section[n+1].set(Real);

    m_hPanels.insert(m_hPanels.begin()+n, 1);

}


void FuseSections::removeSideLine(int SideLine)
{
    if(!hasPoints()) return;
    if(SideLine<0 || SideLine>=int(m_Section.front().size())) return;
    for (int isec=0; isec<nSections(); isec++)
    {
        m_Section[isec].erase(m_Section[isec].begin() + SideLine);
    }
}


double FuseSections::length() const
{
    double Length(0);
    if(nSections())
        Length = fabs(m_Section.front().front().x - m_Section.back().front().x);
    else
        Length = 1.0;
    return Length;
}


double FuseSections::height() const
{
    double h=0;
    for(int isec=0; isec<nSections(); isec++)
    {
        double zmin = 1000.0;
        double zmax = -1000.0;
        std::vector<Vector3d> const &section = m_Section.at(isec);
        for(int ic=0; ic<int(section.size()); ic++)
        {
            zmin = std::min(section.at(ic).z, zmin);
            zmax = std::max(section.at(ic).z, zmax);
            h = std::max(zmax-zmin, h);
        }
    }
    return h;
}


int FuseSections::insertFrameBefore(int iFrame)
{
    if(m_Section.size()==0)
    {
        m_Section.resize(1);
        m_xPanels.insert(m_xPanels.begin()+iFrame, 1);
        return 0;
    }

    if(iFrame<=0)
    {
        m_Section.insert(m_Section.begin(), m_Section.front());
        for(int ic =0; ic<int(m_Section.front().size()); ic++)
        {
            m_Section.front()[ic].x -= 0.3;
        }
        m_xPanels.insert(m_xPanels.begin()+iFrame, 1);
        return 0;
    }

    std::vector<Vector3d> const &prev = m_Section.at(iFrame-1);
    std::vector<Vector3d> const &next = m_Section.at(iFrame);
    int npts = pointCount();
    std::vector<Vector3d> newsec(npts);

    for (int ic=0; ic<npts; ic++)
    {
        newsec[ic].x = (prev.at(ic).x + next.at(ic).x)/2.0;
        newsec[ic].y = (prev.at(ic).y + next.at(ic).y)/2.0;
        newsec[ic].z = (prev.at(ic).z + next.at(ic).z)/2.0;
    }

    m_Section.insert(m_Section.begin()+iFrame, newsec);
    m_xPanels.insert(m_xPanels.begin()+iFrame, 1);

    m_iActiveSection = iFrame;

    return iFrame;
}


/**
 * Inserts a new Frame object in the Body definition
 * @param iFrame the index of the frame after which a new Frame will be inserted
 * @return the index of the Frame which has been inserted; Frame objects are indexed from nose to tail
 */
int FuseSections::insertFrameAfter(int iFrame)
{
    if(m_Section.size()==0)
    {
        m_Section.resize(1);
        m_xPanels.insert(m_xPanels.begin()+iFrame, 1);
        return 0;
    }

    if(iFrame>=nSections()-1)
    {
        m_Section.push_back(m_Section.back());
        for(int ic =0; ic<int(m_Section.back().size()); ic++)
        {
            m_Section.back()[ic].x += 0.3;
        }
        m_xPanels.insert(m_xPanels.begin()+iFrame, 1);
        return 0;
    }

    std::vector<Vector3d> const &prev = m_Section.at(iFrame);
    std::vector<Vector3d> const &next = m_Section.at(iFrame+1);
    int npts = pointCount();
    std::vector<Vector3d> newsec(npts);

    for (int ic=0; ic<npts; ic++)
    {
        newsec[ic].x = (prev.at(ic).x + next.at(ic).x)/2.0;
        newsec[ic].y = (prev.at(ic).y + next.at(ic).y)/2.0;
        newsec[ic].z = (prev.at(ic).z + next.at(ic).z)/2.0;
    }

    m_Section.insert(m_Section.begin()+iFrame+1, newsec);
    m_xPanels.insert(m_xPanels.begin()+iFrame+1, 1);

    m_iActiveSection = iFrame+1;

    return iFrame+1;
}





