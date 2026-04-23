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



#include <fusenurbs.h>
#include <geom_global.h>

FuseNurbs::FuseNurbs() : FuseXfl()
{
    m_FuseType = Fuse::NURBS;
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
int FuseNurbs::makeQuadMesh(int idx0, const Vector3d &pos)
{
    m_Panel4.clear();

    double uk(0.0), uk1(0.0), v(0.0);

    std::vector<Vector3d> refnode;

    Vector3d LA, LB, TA, TB;

    int n0(-1), n1(-1), n2(-1), n3(-1);
    int nx = nxNurbsPanels();
    int nh = nhNurbsPanels();
    int pcount = 0;

    int fullSize = 0;

    double dpx=pos.x;
    double dpy=pos.y;
    double dpz=pos.z;

    if(int(m_XPanelPos.size())<nx) return 0;

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

            n0 = geom::isVector3d(refnode, LA);
            if(n0>=0)  p4.m_iLA = n0;
            else {
                p4.m_iLA = int(refnode.size());
                refnode.push_back(LA);
            }

            n1 = geom::isVector3d(refnode, TA);
            if(n1>=0)  p4.m_iTA = n1;
            else {
                p4.m_iTA = int(refnode.size());
                refnode.push_back(TA);
            }

            n2 = geom::isVector3d(refnode, LB);
            if(n2>=0) p4.m_iLB = n2;
            else {
                p4.m_iLB = int(refnode.size());
                refnode.push_back(LB);
            }

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

            n0 = geom::isVector3d(refnode, LA);
            if(n0>=0)  p4.m_iLA = n0;
            else {
                p4.m_iLA = int(refnode.size());
                refnode.push_back(LA);
            }

            n1 = geom::isVector3d(refnode, TA);
            if(n1>=0) p4.m_iTA = n1;
            else {
                p4.m_iTA = int(refnode.size());
                refnode.push_back(TA);
            }

            n2 = geom::isVector3d(refnode, LB);
            if(n2>=0)  p4.m_iLB = n2;
            else {
                p4.m_iLB = int(refnode.size());
                refnode.push_back(LB);
            }

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
