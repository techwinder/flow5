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



#include <fuseflatfaces.h>
#include <geom_global.h>

FuseFlatFaces::FuseFlatFaces() : FuseXfl()
{
    m_FuseType = Fuse::FlatFace;
}

FuseFlatFaces::FuseFlatFaces(FuseXfl const& fusexfl)
{
    duplicateFuseXfl(fusexfl);
    m_FuseType = Fuse::FlatFace;
}


void FuseFlatFaces::makeDefaultFuse()
{
    m_theStyle.m_Color.setRgb(89,136,143);
    m_nxNurbsPanels = 19;
    m_nhNurbsPanels = 7;

    m_xPanels.clear();
    m_hPanels.clear();


    // flat face type
    m_nurbs.setuDegree(3);
    m_nurbs.setvDegree(3);

    for(int ifr=0; ifr<7; ifr++)
    {
        m_nurbs.appendNewFrame();
        m_nurbs.frame(ifr).clearCtrlPoints();
        m_xPanels.push_back(1);
        m_hPanels.push_back(1);
        for(int is=0; is<4; is++)
        {
            m_nurbs.frame(ifr).appendCtrlPoint(Vector3d(0.0,0.0,0.0));
        }
    }

    frame(0).setCtrlPoint(0, 0.0, 0.0, -0.0585);
    frame(0).setCtrlPoint(1, 0.0, 0.0, -0.0585);
    frame(0).setCtrlPoint(2, 0.0, 0.0, -0.0585);
    frame(0).setCtrlPoint(3, 0.0, 0.0, -0.0585);

    frame(1).setCtrlPoint(0, 0.007, 0.000,  0.0024);
    frame(1).setCtrlPoint(1, 0.007, 0.036, -0.0106);
    frame(1).setCtrlPoint(2, 0.007, 0.036, -0.0811);
    frame(1).setCtrlPoint(3, 0.007, 0.000, -0.0981);

    frame(2).setCtrlPoint(0, 0.210, 0.000,  0.072);
    frame(2).setCtrlPoint(1, 0.210, 0.072,  0.046);
    frame(2).setCtrlPoint(2, 0.210, 0.072, -0.095);
    frame(2).setCtrlPoint(3, 0.210, 0.000, -0.129);

    frame(3).setCtrlPoint(0, 0.499, 0.000,  0.123);
    frame(3).setCtrlPoint(1, 0.499, 0.074,  0.088);
    frame(3).setCtrlPoint(2, 0.499, 0.074, -0.076);
    frame(3).setCtrlPoint(3, 0.499, 0.000, -0.115);

    frame(4).setCtrlPoint(0, 0.802, 0.000,  0.080);
    frame(4).setCtrlPoint(1, 0.802, 0.041,  0.061);
    frame(4).setCtrlPoint(2, 0.802, 0.041, -0.035);
    frame(4).setCtrlPoint(3, 0.802, 0.000, -0.058);

    frame(5).setCtrlPoint(0, 1.586, 0.000,  0.060);
    frame(5).setCtrlPoint(1, 1.586, 0.031,  0.047);
    frame(5).setCtrlPoint(2, 1.586, 0.031, -0.009);
    frame(5).setCtrlPoint(3, 1.586, 0.000, -0.018);

    frame(6).setCtrlPoint(0, 1.630, 0.0, 0.0219);
    frame(6).setCtrlPoint(1, 1.630, 0.0, 0.0219);
    frame(6).setCtrlPoint(2, 1.630, 0.0, 0.0219);
    frame(6).setCtrlPoint(3, 1.630, 0.0, 0.0219);


    frame(0).setuPosition(m_nurbs.uAxis(), 0.000);
    frame(1).setuPosition(m_nurbs.uAxis(), 0.070);
    frame(2).setuPosition(m_nurbs.uAxis(), 0.210);
    frame(3).setuPosition(m_nurbs.uAxis(), 0.499);
    frame(4).setuPosition(m_nurbs.uAxis(), 0.802);
    frame(5).setuPosition(m_nurbs.uAxis(), 1.586);
    frame(6).setuPosition(m_nurbs.uAxis(), 1.630);

    setPanelPos();

    std::string logmsg;
    makeDefaultTriMesh(logmsg, "");
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
int FuseFlatFaces::makeQuadMesh(int idx0, Vector3d const &pos)
{
    m_Panel4.clear();

    double dj(0), dj1(0), dl1(0);

    std::vector<Vector3d> refnode;

    Vector3d LA, LB, TA, TB;
    Vector3d PLA, PTA, PLB, PTB;

    int n0(-1), n1(-1), n2(-1), n3(-1), lnx(-1), lnh(-1);
    int pcount = 0;

    int fullSize =0;

    lnx = 0;

    double dpx=pos.x;
    double dpy=pos.y;
    double dpz=pos.z;


    int nx = 0;
    for(int i=0; i<frameCount()-1; i++) nx += m_xPanels.at(i);
    int nh = 0;
    for(int i=0; i<sideLineCount()-1; i++) nh += m_hPanels.at(i);
    fullSize = idx0 + nx*nh*2;
    m_nxNurbsPanels = nx;
    m_nhNurbsPanels = nh;

    for (int i=0; i<frameCount()-1; i++)
    {
        for (int j=0; j<m_xPanels.at(i); j++)
        {
            dj  = double(j)  /double(m_xPanels.at(i));
            dj1 = double(j+1)/double(m_xPanels.at(i));

            //body left side
            lnh = 0;
            for (int k=0; k<sideLineCount()-1; k++)
            {
                //build the four corner points of the strips
                PLB.x =  (1.0- dj) * framePosition(i)              +  dj * framePosition(i+1)               +dpx;
                PLB.y = -(1.0- dj) * frameAt(i).ctrlPointAt(k).y   -  dj * frameAt(i+1).ctrlPointAt(k).y    +dpy;
                PLB.z =  (1.0- dj) * frameAt(i).ctrlPointAt(k).z   +  dj * frameAt(i+1).ctrlPointAt(k).z    +dpz;

                PTB.x =  (1.0-dj1) * framePosition(i)              + dj1 * framePosition(i+1)               +dpx;
                PTB.y = -(1.0-dj1) * frameAt(i).ctrlPointAt(k).y   - dj1 * frameAt(i+1).ctrlPointAt(k).y    +dpy;
                PTB.z =  (1.0-dj1) * frameAt(i).ctrlPointAt(k).z   + dj1 * frameAt(i+1).ctrlPointAt(k).z    +dpz;

                PLA.x =  (1.0- dj) * framePosition(i)              +  dj * framePosition(i+1)               +dpx;
                PLA.y = -(1.0- dj) * frameAt(i).ctrlPointAt(k+1).y -  dj * frameAt(i+1).ctrlPointAt(k+1).y  +dpy;
                PLA.z =  (1.0- dj) * frameAt(i).ctrlPointAt(k+1).z +  dj * frameAt(i+1).ctrlPointAt(k+1).z  +dpz;

                PTA.x =  (1.0-dj1) * framePosition(i)              + dj1 * framePosition(i+1)               +dpx;
                PTA.y = -(1.0-dj1) * frameAt(i).ctrlPointAt(k+1).y - dj1 * frameAt(i+1).ctrlPointAt(k+1).y  +dpy;
                PTA.z =  (1.0-dj1) * frameAt(i).ctrlPointAt(k+1).z + dj1 * frameAt(i+1).ctrlPointAt(k+1).z  +dpz;

                LB = PLB;
                TB = PTB;

                for (int l=0; l<m_hPanels.at(k); l++)
                {
                    m_Panel4.push_back({});
                    Panel4 &p4 = m_Panel4.back();

                    dl1  = double(l+1) / double(m_hPanels[k]);
                    LA = PLB * (1.0- dl1) + PLA * dl1;
                    TA = PTB * (1.0- dl1) + PTA * dl1;

//                    n0 = refnode.indexOf(LA);
                    n0 = geom::isVector3d(refnode, LA);
                    if(n0>=0) {
                        p4.m_iLA = n0;
                    }
                    else {
                        p4.m_iLA = int(refnode.size());
                        refnode.push_back(LA);
                    }

//                    n1 = refnode.indexOf(TA);
                    n1 = geom::isVector3d(refnode, TA);
                    if(n1>=0) {
                        p4.m_iTA = n1;
                    }
                    else {
                        p4.m_iTA = int(refnode.size());
                        refnode.push_back(TA);
                    }

//                    n2 = refnode.indexOf(LB);
                    n2 = geom::isVector3d(refnode, LB);
                    if(n2>=0) {
                        p4.m_iLB = n2;
                    }
                    else {
                        p4.m_iLB = int(refnode.size());
                        refnode.push_back(LB);
                    }

//                    n3 = refnode.indexOf(TB);
                    n3 = geom::isVector3d(refnode, TB);
                    if(n3 >=0) {
                        p4.m_iTB = n3;
                    }
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

                    if(lnx==0)      p4.m_iPU = -1;// no panel downstream
                    if(lnx==nx-1) p4.m_iPD = -1;// no panel upstream

                    p4.m_iPL = p4.index() + 1;
                    p4.m_iPR = p4.index() - 1;

                    if(lnh==0)     p4.m_iPR = fullSize - pcount - 1;
                    if(lnh==nh-1)  p4.m_iPL = fullSize - pcount - 1;

                    pcount++;
                    LB = LA;
                    TB = TA;
                    lnh++;
                }
            }
            lnx++;
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


int FuseFlatFaces::quadCount() const
{
    int total = 0;
    for(int iFrame=0; iFrame<frameCount()-1; iFrame++)
    {
        for(int i=0; i<sideLineCount()-1; i++)
        {
            total += m_xPanels.at(iFrame)*m_hPanels.at(i);
        }
    }
    return total*2;
}


int FuseFlatFaces::makeShape(std::string &log)
{
    m_Shape.Clear();
    m_Shell.Clear();
    m_RightSideShell.Clear();
//    makeBodyFlatPanelShape_with2Triangles(log);
    makeBodyFlatPanelShape_withSpline(log);
//    makeBodySplineShape_old(log);
    return m_Shape.Extent();
}


int FuseFlatFaces::makeSurfaceTriangulation(int , int )
{
    clearTriangles();
    clearTriangleNodes();
    makeFlatFaceTriangulation();

    makeTriangleNodes();
    makeNodeNormals();

    return nTriangles();
}

