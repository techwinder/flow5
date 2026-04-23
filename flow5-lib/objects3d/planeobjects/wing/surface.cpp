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

#include <chrono>
#include <thread>

#include <objects2d.h>
#include <surface.h>
#include <foil.h>
#include <planexfl.h>
#include <planepolar.h>
#include <occ_globals.h>
#include <fusexfl.h>
#include <wingsection.h>
#include <panel3.h>
#include <panel4.h>

#include <geom_global.h>
#include <quaternion.h>
#include <vector3d.h>

#include <bspline3d.h>


std::vector<Vector3d> Surface::s_DebugPts;
std::vector<Vector3d> Surface::s_DebugVecs;


Surface::Surface()
{
    m_Index = -1;

    m_bTEFlap = false;

    m_Length   = 0.0;
    m_TwistA   = 0.0;
    m_TwistB   = 0.0;
    m_posATE   = 1.0;
    m_posBTE   = 1.0;
    m_NXPanels  = 1;
    m_NYPanels  = 2;
    m_NXLead    = 1;
    m_NXFlap    = 0;
    m_XDistType = xfl::COSINE;
    m_YDistType = xfl::UNIFORM;

    m_pLeftSurface = m_pRightSurface = nullptr;
    m_nQuads = 0;

    m_bIsInSymPlane = false;
    m_bIsTipLeft      = m_bIsTipRight      = false;
    m_bIsLeftSurf     = m_bIsRightSurf     = false;
    m_bClosedLeftSide = m_bClosedRightSide = false;

    m_bIsCenterSurf = false;
    m_bJoinRight    = true;

    m_InnerSection = 0;
    m_OuterSection = 0;

    m_FirstStripPanelIndex = m_LastStripPanelIndex = -1;


    tmp_alpha_dA = tmp_alpha_dB = tmp_delta=0;
    tmp_pos=xfl::NOSURFACE;
    tmp_pFoilA = tmp_pFoilB=nullptr;
    tmp_pFuse=nullptr;
}



/**
 * Adds the index of the input panel to the array of flap panel indexes.
 * Adds the indexes of the panel's nodes to the array of node indexes
 * @param pPanel the pointer of the panel to add to the flap panel list.
 */
void Surface::addFlapPanel4(Panel4 const &p4)
{
    //Add Nodes
    if(!isFlapNode(p4.m_iLA)) m_FlapNode4.push_back(p4.m_iLA);
    if(!isFlapNode(p4.m_iLB)) m_FlapNode4.push_back(p4.m_iLB);
    if(!isFlapNode(p4.m_iTA)) m_FlapNode4.push_back(p4.m_iTA);
    if(!isFlapNode(p4.m_iTB)) m_FlapNode4.push_back(p4.m_iTB);

    //Add panels
    if(!hasFlapPanel4(p4.index())) m_FlapPanel4.push_back(p4.index());
}


/**
 * Copy the data from another Surface object to this Surface
 * @param Surface the source Surface from which the data shall be duplicated
 */
void Surface::copy(Surface const &aSurface)
{
    m_LA.copy(aSurface.m_LA);
    m_LB.copy(aSurface.m_LB);
    m_TA.copy(aSurface.m_TA);
    m_TB.copy(aSurface.m_TB);
    m_XDistType = aSurface.m_XDistType;
    m_YDistType = aSurface.m_YDistType;
    m_nQuads = aSurface.m_nQuads;

    m_Length    = aSurface.m_Length;
    m_NXPanels  = aSurface.m_NXPanels;
    m_NYPanels  = aSurface.m_NYPanels;
    m_FoilNameA = aSurface.m_FoilNameA;
    m_FoilNameB = aSurface.m_FoilNameB;
    m_TwistA    = aSurface.m_TwistA;
    m_TwistB    = aSurface.m_TwistB;

    m_Normal  = aSurface.m_Normal;
    m_NormalA = aSurface.m_NormalA;
    m_NormalB = aSurface.m_NormalB;

    m_bIsTipLeft       = aSurface.m_bIsTipLeft;
    m_bIsTipRight      = aSurface.m_bIsTipRight;
    m_bIsLeftSurf      = aSurface.m_bIsLeftSurf;
    m_bIsRightSurf     = aSurface.m_bIsRightSurf;
    m_bIsCenterSurf    = aSurface.m_bIsCenterSurf;
    m_bJoinRight       = aSurface.m_bJoinRight;
    m_bIsInSymPlane    = aSurface.m_bIsInSymPlane;
    m_bClosedLeftSide  = aSurface.m_bClosedLeftSide;
    m_bClosedRightSide = aSurface.m_bClosedRightSide;

    m_InnerSection = aSurface.m_InnerSection;
    m_OuterSection = aSurface.m_OuterSection;

    m_bTEFlap       = aSurface.m_bTEFlap;

    m_HingePoint  = aSurface.m_HingePoint;
    m_HingeVector = aSurface.m_HingeVector;

    m_NXFlap = aSurface.m_NXFlap;
    m_NXLead = aSurface.m_NXLead;

    m_FlapNode4 = aSurface.m_FlapNode4;
    m_FlapPanel4 = aSurface.m_FlapPanel4;
    m_Panel4List = aSurface.m_Panel4List;

    m_FlapPanel3 = aSurface.m_FlapPanel3;
    m_Panel3List = aSurface.m_Panel3List;

    m_xPointA = aSurface.m_xPointA;
    m_xPointB = aSurface.m_xPointB;

    m_SideA = aSurface.m_SideA;
    m_SideB = aSurface.m_SideB;
    m_SideA_Top = aSurface.m_SideA_Top;
    m_SideB_Top = aSurface.m_SideB_Top;
    m_SideA_Bot = aSurface.m_SideA_Bot;
    m_SideB_Bot = aSurface.m_SideB_Bot;

    m_FirstStripPanelIndex = aSurface.m_FirstStripPanelIndex;
    m_LastStripPanelIndex  = aSurface.m_LastStripPanelIndex;
}


/**
 * Returns the twist of the specified strip
 * @param k the 0-based index of the strip for which the leading point shall be returned.
 * @return the strip's twist.
 */
double Surface::stripTwist(int k) const
{
    double y1=0.0, y2=0.0;
    getYDist(k, y1, y2);
    double tau = (y1+y2)/2.0;
    return m_TwistA *(1.0-tau) + m_TwistB*tau;
}


/**
 * Returns the area of the virtual foil at a specified relative span position. Used in Inertia calaculations.
 * @param tau the relative percentage of the Surface's span length
 * @return the cross area at the specified location
 */
double Surface::foilArea(double tau) const
{
    Foil const*pFoilA = foilA();
    Foil const*pFoilB = foilB();
    if(pFoilA && pFoilB)
    {
        return (pFoilA->area() + pFoilB->area())/2.0*chord(tau)*chord(tau);//m2
    }
    else
        return 0.0;
}


/**
 * If required, left and right side points are distinct from top and surface points
 * This is to avoid confusion of the trailing nodes at the surface's tip
 * @param nStrips the number of horzontal panels strips in the tip thickness
 */
int Surface::makeTipNodes(std::vector<Node> &nodes, bool bLeft, int nStrips) const
{
    Node node;
    std::vector<Node> nodetiplist((m_NXPanels+1)*(nStrips+1));
    int n0 = int(nodes.size());
    nStrips = std::max(nStrips, 1);

    int isb=0;
    int ist=nStrips;

    Vector3d C,R;
    // make the left nodes
    if(bLeft)
    {
        Vector3d S =  Vector3d(1,0,0) * m_Normal; // outwards normal to the tip
        // from TE to LE
        int in=0;
        for(int is=isb; is<=ist; is++)
        {
            for(int l=0; l<=m_NXPanels; l++)
            {
                C.set((m_SideA_Bot.at(l)+m_SideA_Top.at(l))/2.0);
                R.set((m_SideA_Top.at(l)-m_SideA_Bot.at(l))/2.0);
                double rad = R.norm();
                R.normalize();
                double theta = double(nStrips-is)/double(nStrips) * PI;
                node.set(C + R*cos(theta)*rad + S*sin(theta)*rad);
                if     (is==isb) node.setSurfacePosition(xfl::BOTSURFACE);
                else if(is==ist) node.setSurfacePosition(xfl::TOPSURFACE);
                else             node.setSurfacePosition(xfl::SIDESURFACE);
                node.setIndex(n0);

                nodetiplist[in] = node;
                n0++;
                in++;
            }
        }
        nodes.insert(nodes.end(), nodetiplist.begin(), nodetiplist.end());
    }
    else
    {
        // make the right tip nodes
        Vector3d S =  m_Normal * Vector3d(1,0,0) ; // outwards normal to the tip
        // from TE to LE
        int in=0;
        for(int is=isb; is<=ist; is++)
        {
            for(int l=0; l<=m_NXPanels; l++)
            {
                C.set((m_SideB_Bot.at(l)+m_SideB_Top.at(l))/2.0);
                R.set((m_SideB_Top.at(l)-m_SideB_Bot.at(l))/2.0);
                double rad = R.norm();
                R.normalize();
                double theta = double(nStrips-is)/double(nStrips) * PI;
                node.set(C + R*cos(theta)*rad + S*sin(theta)*rad);
                if     (is==isb) node.setSurfacePosition(xfl::BOTSURFACE);
                else if(is==ist) node.setSurfacePosition(xfl::TOPSURFACE);
                else             node.setSurfacePosition(xfl::SIDESURFACE);
                node.setIndex(n0);

                nodetiplist[in] = node;
                n0++;
                in++;
            }
        }

        nodes.insert(nodes.end(), nodetiplist.begin(), nodetiplist.end());
    }

    return int(nodetiplist.size());
}


/**
 * If required, left and right side points are distinct from top and surface points
 * This is to avoid confusion of the trailing nodes at the surface's tip
 */
void Surface::makePanelNodes(std::vector<Node> &nodes, bool bMakeLeftNodes, bool bMidSurface) const
{
    double y1=0, y2=0;
    Node node;
    std::vector<Node> lefttiplist;
    int n0 = int(nodes.size());

    // make left side nodes
    getYDist(0,y1,y2);

    // make the left chord points
    if(bMakeLeftNodes)
    {
        if(bMidSurface)
        {
            for(int l=0; l<=m_NXPanels; l++)
            {
                node = m_SideA.at(l);
                node.setSurfacePosition(xfl::MIDSURFACE);
                node.setIndex(n0);

                lefttiplist.push_back(node);
                n0++;
            }
        }
        else
        {
            // start with bottom points, from TE to LE included
            for(int l=0; l<=m_NXPanels; l++)
            {
                node = m_SideA_Bot.at(l)*(1.0-y1) + m_SideB_Bot.at(l)*y1;
                node.setSurfacePosition(xfl::BOTSURFACE);
                node.setIndex(n0);

                lefttiplist.push_back(node);
                n0++;
            }
            // add top surface points, from LE excluded to TE
            for(int l=m_NXPanels-1; l>=0; l--)
            {
                node = m_SideA_Top.at(l)*(1.0-y1) + m_SideB_Top.at(l)*y1;
                node.setSurfacePosition(xfl::TOPSURFACE);
                node.setIndex(n0);


                lefttiplist.push_back(node);
                n0++;
            }
        }

        nodes.insert(nodes.end(), lefttiplist.begin(), lefttiplist.end());
    }

    // make the central nodes including the last right side nodes
    for (int k=0; k<NYPanels(); k++)
    {
        getYDist(k,y1,y2);
        if(bMidSurface)
        {
            for(int l=0; l<=m_NXPanels; l++)
            {
                node = m_SideA.at(l)*(1.0-y2) + m_SideB.at(l)*y2;
                node.setSurfacePosition(xfl::MIDSURFACE);
                node.setIndex(n0);

                nodes.push_back(node);
                n0++;
            }
        }
        else
        {
            //start with bottom points, from TE to LE included
            for(int l=0; l<=m_NXPanels; l++)
            {
                node = m_SideA_Bot.at(l)*(1.0-y2) + m_SideB_Bot.at(l)*y2;
                node.setSurfacePosition(xfl::BOTSURFACE);
                node.setIndex(n0);

                nodes.push_back(node);
                n0++;
            }
            //add top surface points, from LE excluded to TE
            for(int l=m_NXPanels-1; l>=0; l--)
            {
                node = m_SideA_Top.at(l)*(1.0-y2) + m_SideB_Top.at(l)*y2;
                node.setSurfacePosition(xfl::TOPSURFACE);
                node.setIndex(n0);

                nodes.push_back(node);
                n0++;
            }
        }
    }
}


void Surface::getSideNode(double xRel, bool bRight, xfl::enumSurfacePosition pos, Node &node) const
{
    Vector2d foilPt(xRel,0.0);
    Foil const *pFoilA = foilA();
    Foil const *pFoilB = foilB();
    Vector2d N;

    if(!bRight)
    {
        if     (pos==xfl::MIDSURFACE && pFoilA) foilPt = pFoilA->midYRel( xRel,  N);
        else if(pos==xfl::TOPSURFACE && pFoilA) foilPt = pFoilA->upperYRel(xRel, N);
        else if(pos==xfl::BOTSURFACE && pFoilA) foilPt = pFoilA->lowerYRel(xRel, N);
        node.setPosition(m_LA * (1.0-foilPt.x) + m_TA * foilPt.x);
        node.translate(m_Normal * foilPt.y*chord(0.0));
    }
    else
    {
        if     (pos==xfl::MIDSURFACE && pFoilB) foilPt = pFoilB->midYRel( xRel,  N);
        else if(pos==xfl::TOPSURFACE && pFoilB) foilPt = pFoilB->upperYRel(xRel, N);
        else if(pos==xfl::BOTSURFACE && pFoilB) foilPt = pFoilB->lowerYRel(xRel, N);

        node.setPosition(m_LB * (1.0-foilPt.x) + m_TB * foilPt.x);
        node.translate(m_Normal * foilPt.y*chord(1.0));
    }
    node.setNormal(N.x, 0.0, N.y);
}



void Surface::getSidePoints_1(xfl::enumSurfacePosition pos,
                              Fuse const *pFuse,
                              std::vector<Node> &PtA, std::vector<Node> &PtB,
                              int nPoints, xfl::enumDistribution ) const
{
    PtA.resize(nPoints);
    PtB.resize(nPoints);


    Vector3d V = m_Normal * m_NormalA;
    Vector3d U = (m_TA - m_LA).normalized();
    //    double sindA = -V.dot(Vector3d(1.0,0.0,0.0));
    double sindA = -V.dot(U);
    if(sindA> 1.0) sindA = 1.0;
    if(sindA<-1.0) sindA = -1.0;
    tmp_alpha_dA = asin(sindA);

    V = m_Normal * m_NormalB;
    U = (m_TB-m_LB).normalized();
    //    double sindB = -V.dot(Vector3d(1.0,0.0,0.0));
    double sindB = -V.dot(U);
    if(sindB> 1.0) sindB = 1.0;
    if(sindB<-1.0) sindB = -1.0;
    tmp_alpha_dB = asin(sindB);


    tmp_delta = -atan(m_Normal.y / m_Normal.z);
    //    double delta = -atan2(Normal.y, Normal.z)*180.0/PI;

    tmp_pos = pos;
    tmp_pFuse = pFuse;

    tmp_pFoilA = foilA();
    tmp_pFoilB = foilB();

    bool bMultithread = true;

//    int nThreads=std::thread::hardware_concurrency();

    if(bMultithread)
    {
        std::vector<std::thread> threads;

        for(int i=0; i<nPoints; i++)
        {
            threads.push_back(std::thread(&Surface::getSidePoints1_task, this, i, nPoints, &PtA[i], &PtB[i]));
        }

        for(int i=0; i<nPoints; i++)
        {
            threads[i].join();
        }
//        std::cout << "getSidePoints_1 joined all " << m_NXPanels << " threads" <<std::endl;
    }
    else
    {
        for(int i=0; i<nPoints; i++)
        {
            getSidePoints1_task(i, nPoints, &PtA[i], &PtB[i]);
        }
    }
}


//Note: passing references to nodeA and nodeB is not compatible with multithreading
void Surface::getSidePoints1_task(int i, int tmp_nPoints, Node *nodeA, Node *nodeB) const
{
    double cosdA = cos(tmp_alpha_dA);
    double cosdB = cos(tmp_alpha_dB);

    Vector3d I;
    double xRelA=0, xRelB=0;
    if(tmp_pFoilA && tmp_pFoilB && tmp_pFoilA->hasTEFlap() && tmp_pFoilB->hasTEFlap())
    {
        int nPtsLe = tmp_nPoints*3/4;
        int nPtsTr = tmp_nPoints-nPtsLe;

        if(i<nPtsLe)
        {
            xRelA  = 1.0/2.0*(1.0-cos(PI * double(i)/double(nPtsLe-1)))* (tmp_pFoilA->TEXHinge());
            xRelB  = 1.0/2.0*(1.0-cos(PI * double(i)/double(nPtsLe-1)))* (tmp_pFoilB->TEXHinge());
        }
        else
        {
            int j = i-nPtsLe;
            xRelA  = tmp_pFoilA->TEXHinge() + 1.0/2.0*(1.0-cos(PI* double(j)/double(nPtsTr-1))) * (1.-tmp_pFoilA->TEXHinge());
            xRelB  = tmp_pFoilB->TEXHinge() + 1.0/2.0*(1.0-cos(PI* double(j)/double(nPtsTr-1))) * (1.-tmp_pFoilB->TEXHinge());
        }
    }
    else
    {
        xRelA  = 1.0/2.0*(1.0-cos(PI * double(i)/double(tmp_nPoints-1)));
        xRelB  = xRelA;
    }

    nodeA->reset();
    getSideNode(xRelA, false, tmp_pos, *nodeA);
    //scale the thickness
    double Ox = xRelA;
    double Oy = m_LA.y * (1.0-Ox) +  m_TA.y * Ox;
    double Oz = m_LA.z * (1.0-Ox) +  m_TA.z * Ox;
    nodeA->y   = Oy +(nodeA->y - Oy)/cosdA;
    nodeA->z   = Oz +(nodeA->z - Oz)/cosdA;
    nodeA->rotate(m_LA, (m_LA-m_TA).normalized(), +tmp_alpha_dA*180.0/PI);
    nodeA->normal().rotate(Vector3d(1.0,0.0,0.0), tmp_delta*180.0/PI);


    nodeB->reset();
    getSideNode(xRelB, true,  tmp_pos, *nodeB);
    Ox = xRelB;
    Oy = m_LB.y * (1.0-Ox) +  m_TB.y * Ox;
    Oz = m_LB.z * (1.0-Ox) +  m_TB.z * Ox;
    nodeB->y   = Oy +(nodeB->y - Oy)/cosdB;
    nodeB->z   = Oz +(nodeB->z - Oz)/cosdB;
    nodeB->rotate(m_LB, (m_LB-m_TB).normalized(), +tmp_alpha_dB*180.0/PI);
    nodeB->normal().rotate(Vector3d(1.0,0.0,0.0), tmp_delta*180.0/PI);

    if(tmp_pFuse && m_bIsCenterSurf && m_bIsLeftSurf)
    {
        if(tmp_pFuse->intersectFuse(*nodeA, *nodeB, I, false))
        {
            if(I.distanceTo(*nodeA) < nodeB->distanceTo(*nodeA))
                nodeB->setPosition(I);
        }
    }
    else if(tmp_pFuse && m_bIsCenterSurf && m_bIsRightSurf)
    {
        if(tmp_pFuse->intersectFuse(*nodeB, *nodeA, I, true))
        {
            if(I.distanceTo(*nodeB) < nodeA->distanceTo(*nodeB))
            {
                nodeA->setPosition(I);
            }
        }
    }
/*    qDebug("AB  %13g  %13g  %13g  %13g  %13g  %13g",
           nodeA->normal().x, nodeA->normal().y, nodeA->normal().z,
           nodeB->normal().x, nodeB->normal().y, nodeB->normal().z);*/
//    qDebug("normals  %13g  %13g", nodeA->normal().norm(), nodeB->normal().norm());
}


/** Intersects exactly the TOPO_DS_SHELL so that wing and fuse and wing meshes connect */
void Surface::getSidePoints_2(xfl::enumSurfacePosition pos,
                              const Fuse *pFuse,
                              std::vector<Vector3d> &PtA, std::vector<Vector3d> &PtB, std::vector<Vector3d> &NA, std::vector<Vector3d> &NB,
                              std::vector<double> const &xPointsA, std::vector<double> const &xPointsB) const
{
    assert(xPointsA.size()==xPointsB.size());

    tmp_pos = pos;

    Vector3d V = m_Normal * m_NormalA;
    Vector3d U = (m_TA - m_LA).normalized();
    //    double sindA = -V.dot(Vector3d(1.0,0.0,0.0));
    double sindA = -V.dot(U);
    if(sindA> 1.0) sindA = 1.0;
    if(sindA<-1.0) sindA = -1.0;
    tmp_alpha_dA = asin(sindA);

    V = m_Normal * m_NormalB;
    U = (m_TB-m_LB).normalized();
    //    double sindB = -V.dot(Vector3d(1.0,0.0,0.0));
    double sindB = -V.dot(U);
    if(sindB> 1.0) sindB = 1.0;
    if(sindB<-1.0) sindB = -1.0;
    tmp_alpha_dB = asin(sindB);

    tmp_delta = -atan(m_Normal.y / m_Normal.z)*180.0/PI;
    //    double delta = -atan2(Normal.y, Normal.z)*180.0/PI;

    tmp_pFuse = pFuse;

//    auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<Node> nodeA(xPointsA.size());
    std::vector<Node> nodeB(xPointsB.size());
    for(uint i=0; i<xPointsA.size(); i++)
    {
        double xRelA = xPointsA.at(i);
        double xRelB = xPointsB.at(i);
        getSidePoints2_task(xRelA, xRelB, nodeA[i], nodeB[i]);
    }

/*    auto t1 = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    qDebug("Surface::getSidePoints_2 %gms", double(duration)/1000.0);
*/
    for(uint i=0; i<xPointsA.size(); i++)
    {
        PtA[i].set(nodeA.at(i));
        PtB[i].set(nodeB.at(i));
        NA[i].set(nodeA.at(i).normal());
        NB[i].set(nodeB.at(i).normal());
    }
}


void Surface::getSidePoints2_task(double xRelA, double xRelB, Node &nodeA, Node &nodeB) const
{
    double cosdA = cos(tmp_alpha_dA);
    double cosdB = cos(tmp_alpha_dB);

    Vector3d I;

    getSideNode(xRelA, false, tmp_pos, nodeA);

    //scale the thickness
    double Ox = xRelA;
    double Oy = m_LA.y * (1.0-Ox) +  m_TA.y * Ox;
    double Oz = m_LA.z * (1.0-Ox) +  m_TA.z * Ox;
    nodeA.y   = Oy +(nodeA.y - Oy)/cosdA;
    nodeA.z   = Oz +(nodeA.z - Oz)/cosdA;
    nodeA.rotate(m_LA, (m_LA-m_TA).normalized(), +tmp_alpha_dA*180.0/PI);
//    NA[i].rotate(Vector3d(1.0,0.0,0.0), delta);


    getSideNode(xRelB, true, tmp_pos, nodeB);
    Ox = xRelB;
    Oy = m_LB.y * (1.0-Ox) +  m_TB.y * Ox;
    Oz = m_LB.z * (1.0-Ox) +  m_TB.z * Ox;
    nodeB.y   = Oy +(nodeB.y - Oy)/cosdB;
    nodeB.z   = Oz +(nodeB.z - Oz)/cosdB;
    nodeB.rotate(m_LB, (m_LB-m_TB).normalized(), +tmp_alpha_dB*180.0/PI);
//    NB[i].rotate(Vector3d(1.0,0.0,0.0), delta);

    if(tmp_pFuse && m_bIsCenterSurf && m_bIsLeftSurf)
    {
        if(tmp_pFuse->intersectFuse(nodeA, nodeB, I, false))
            nodeB.set(I);
    }
    else if(tmp_pFuse && m_bIsCenterSurf && m_bIsRightSurf)
    {
        if(tmp_pFuse->intersectFuse(nodeB, nodeA, I, true))
            nodeA.set(I);
    }
}


/**
 * Returns the position of a surface point at the position specified by the input parameters.
 * @param xArel the relative position at the left Foil
 * @param xBrel the relative position at the right Foil
 * @param yrel the relative span position
 * @param Point a reference of the requested point's position
 * @param pos defines on which surface (Xfl::BOTSURFACE, Xfl::TOPSURFACE, Xfl::MIDSURFACE) the point is calculated
 */
void Surface::getSurfacePoint(double xArel, double xBrel, double yrel, xfl::enumSurfacePosition pos, Vector3d &Point, Vector3d &PtNormal) const
{
    Vector3d APt, BPt;
    Vector2d foilPt;
    Vector2d N;
    Foil const *pFoilA = foilA();
    Foil const *pFoilB = foilB();

    if(pos==xfl::MIDSURFACE && pFoilA && pFoilB)
    {
        foilPt = pFoilA->midYRel(xArel, N);
        APt = m_LA * (1.0-foilPt.x) + m_TA * foilPt.x;
        APt +=  m_Normal * foilPt.y*chord(0.0);

        foilPt = pFoilB->midYRel(xBrel, N);
        BPt = m_LB * (1.0-foilPt.x) + m_TB * foilPt.x;
        BPt +=  m_Normal * foilPt.y*chord(1.0);
    }
    else if(pos==xfl::TOPSURFACE && pFoilA && pFoilB)
    {
        foilPt = pFoilA->upperYRel(xArel, N);
        APt = m_LA * (1.0-foilPt.x) + m_TA * foilPt.x;
        APt +=  m_Normal * foilPt.y*chord(0.0);

        foilPt = pFoilB->upperYRel(xBrel, N);
        BPt = m_LB * (1.0-foilPt.x) + m_TB * foilPt.x;
        BPt +=  m_Normal * foilPt.y*chord(1.0);
    }
    else if(pos==xfl::BOTSURFACE && pFoilA && pFoilB)
    {
        foilPt = pFoilA->lowerYRel(xArel, N);
        APt = m_LA * (1.0-foilPt.x) + m_TA * foilPt.x;
        APt +=  m_Normal * foilPt.y*chord(0.0);

        foilPt = pFoilB->lowerYRel(xBrel, N);
        BPt = m_LB * (1.0-foilPt.x) + m_TB * foilPt.x;
        BPt +=  m_Normal * foilPt.y*chord(1.0);
    }
    Point = APt * (1.0-yrel)+  BPt * yrel;

    PtNormal.set(N.x, 0.0, N.y);
    PtNormal.rotateY(twistAt(yrel));
    double dihedral = atan2(-m_Normal.y, m_Normal.z) * 180.0/PI;
    PtNormal.rotateX(dihedral);
}


void Surface::getSection(double const &tau, double &Chord, double &Area, Vector3d &PtC4) const
{
    Vector3d LA,TA;

    Foil const *pFoilA = foilA();
    Foil const *pFoilB = foilB();
    //explicit double calculations are much faster than vector algebra
    LA.x = m_LA.x * (1.0-tau) + m_LB.x * tau;
    LA.y = m_LA.y * (1.0-tau) + m_LB.y * tau;
    LA.z = m_LA.z * (1.0-tau) + m_LB.z * tau;
    TA.x = m_TA.x * (1.0-tau) + m_TB.x * tau;
    TA.y = m_TA.y * (1.0-tau) + m_TB.y * tau;
    TA.z = m_TA.z * (1.0-tau) + m_TB.z * tau;
    PtC4.x = .75 * LA.x + .25 * TA.x;
    PtC4.y = .75 * LA.y + .25 * TA.y;
    PtC4.z = .75 * LA.z + .25 * TA.z;

    Chord = sqrt((LA.x-TA.x)*(LA.x-TA.x) + (LA.y-TA.y)*(LA.y-TA.y) + (LA.z-TA.z)*(LA.z-TA.z));

    if(pFoilA && pFoilB)
    {
        Area = (pFoilA->area() * tau + pFoilB->area() * (1.0-tau))*Chord*Chord;//m2
    }
    else
    {
        Area = 0.0;
    }
}


/**
 * Initializes the Surface
 */
void Surface::init()
{
    m_bIsTipLeft   = false;
    m_bIsTipRight  = false;
    m_bIsLeftSurf  = false;
    m_bIsRightSurf = false;

    clearQuadFlapMesh();
    clearTriFlapMesh();

    Foil const *pFoilA = foilA();
    Foil const *pFoilB = foilB();
    if(pFoilA && pFoilB)
    {
        Vector2d bisA = pFoilA->TEbisector();
        Vector2d bisB = pFoilA->TEbisector();

        Vector3d bisectorA(bisA.x, 0.0, bisA.y);
        Vector3d bisectorB(bisB.x, 0.0, bisB.y);

        double dihedral = atan2(-m_Normal.y, m_Normal.z) * 180.0/PI;

        bisectorA.rotate(Vector3d(1.0,0.0,0.0), dihedral);
        bisectorB.rotate(Vector3d(1.0,0.0,0.0), dihedral);

        Vector3d A4 = m_LA *3.0/4.0 + m_TA * 1/4.0;
        Vector3d B4 = m_LB *3.0/4.0 + m_TB * 1/4.0;

        bisectorA.rotate(B4-A4, m_TwistA);
        bisectorB.rotate(B4-A4, m_TwistB);

        m_TEBisectorA = bisectorA;
        m_TEBisectorB = bisectorB;
    }
}


void Surface::setFlap()
{
    Foil const *pFoilA = foilA();
    Foil const *pFoilB = foilB();
    Vector3d N;
    if(pFoilA && pFoilA->hasTEFlap())
    {
        m_posATE = pFoilA->TEXHinge();
        if(m_posATE>1.0) m_posATE = 1.0; else if(m_posATE<0.0) m_posATE = 0.0;
    }
    else m_posATE = 1.0;

    if(pFoilB && pFoilB->hasTEFlap())
    {
        m_posBTE = pFoilB->TEXHinge();
        if(m_posBTE>1.0) m_posBTE = 1.0; else if(m_posBTE<0.0) m_posBTE = 0.0;
    }
    else m_posBTE = 1.0;

    if(pFoilA && pFoilB) m_bTEFlap = pFoilA->hasTEFlap() && pFoilB->hasTEFlap();
    else                 m_bTEFlap = false;


    if(pFoilA && pFoilB && pFoilA->hasTEFlap() && pFoilB->hasTEFlap())
    {
        Vector3d HB;
        //create a hinge unit vector and initialize hinge moment
        getSurfacePoint(m_posATE, m_posBTE, 0.0, xfl::MIDSURFACE, m_HingePoint,N);
        getSurfacePoint(m_posATE, m_posBTE, 1.0, xfl::MIDSURFACE, HB, N);
        m_HingeVector = HB-m_HingePoint;
        m_HingeVector.normalize();
    }
}


/**
 * Creates the master points of the mesh on the left and right ends.
 * @param pTranslatedFuse a pointer to the Fuse object, or NULL if none.
 */
void Surface::makeSideNodes(Fuse const*pTranslatedFuse, bool bDebug)
{
    Vector3d V = m_Normal * m_NormalA;
    Vector3d U = (m_TA - m_LA).normalized();
    double sindA = -V.dot(U);
    if(sindA> 1.0) sindA = 1.0;
    if(sindA<-1.0) sindA = -1.0;
    tmp_alpha_dA = asin(sindA);

    V = m_Normal * m_NormalB;
    U = (m_TB-m_LB).normalized();
    double sindB = -V.dot(U);
    if(sindB> 1.0) sindB = 1.0;
    if(sindB<-1.0) sindB = -1.0;
    tmp_alpha_dB = asin(sindB);

    m_SideA.resize(m_NXPanels+1);
    m_SideA_Bot.resize(m_NXPanels+1);
    m_SideA_Top.resize(m_NXPanels+1);
    m_SideB.resize(m_NXPanels+1);
    m_SideB_Bot.resize(m_NXPanels+1);
    m_SideB_Top.resize(m_NXPanels+1);

    auto t0 = std::chrono::high_resolution_clock::now();
    bool bMultihread = true;

    if(bMultihread)
    {
        std::vector<std::thread> threads;
        for (int l=0; l<=m_NXPanels; l++)
        {

//            futureSync.addFuture(QtConcurrent::run(&Surface::makeSideNodeTask, this, l, pTranslatedFuse, m_xPointA.at(l), m_xPointB.at(l)));
            threads.push_back(std::thread(&Surface::makeSideNodeTask, this, l, pTranslatedFuse, m_xPointA.at(l), m_xPointB.at(l)));

        }

        for (int l=0; l<=m_NXPanels; l++)
        {
            threads[l].join();
        }
//        std::cout << "makeSideNodes joined all " << m_NXPanels << "threads" <<std::endl;
    }
    else
    {
        for (int l=0; l<=m_NXPanels; l++)
        {
            makeSideNodeTask(l, pTranslatedFuse, m_xPointA.at(l), m_xPointB.at(l));
        }
    }
    if(bDebug)
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        qDebug("Surface::makeSideNodes %gms", double(duration)/1000.0);
    }

    //merge trailing edge nodes in case the foil has a T.E. gap
    Vector3d Node;

    Node = (m_SideA_Bot[0] + m_SideA_Top[0])/2.0;
    m_SideA_Bot[0].set(Node);
    m_SideA_Top[0].set(Node);

    Node = (m_SideB_Bot[0] + m_SideB_Top[0])/2.0;
    m_SideB_Bot[0].set(Node);
    m_SideB_Top[0].set(Node);

    if(bDebug)
    {
        s_DebugPts.clear();
        for(uint i=0; i<m_SideA_Top.size(); i++) s_DebugPts.push_back(m_SideA_Top.at(i));
        for(uint i=0; i<m_SideA_Top.size(); i++) s_DebugPts.push_back(m_SideB_Top.at(i));
    }
}


void Surface::makeSideNodeTask(int l, Fuse const *pTranslatedFuse, double xRelA, double xRelB)
{
    Vector3d I;
    double cosdA = cos(tmp_alpha_dA);
    double alpha_dA = tmp_alpha_dA * 180.0/PI;
    double cosdB = cos(tmp_alpha_dB);
    double alpha_dB = tmp_alpha_dB *180.0/PI;

    Node node, nodeT, nodeB;

    getSideNode(xRelA, false, xfl::MIDSURFACE, node);
    getSideNode(xRelA, false, xfl::TOPSURFACE, nodeT);
    getSideNode(xRelA, false, xfl::BOTSURFACE, nodeB);
    m_SideA[l].setNode(node);
    m_SideA_Top[l].setNode(nodeT);
    m_SideA_Bot[l].setNode(nodeB);

    //scale the thickness
    double Ox = xRelA;
    double Oy = m_LA.y * (1.0-Ox) +  m_TA.y * Ox;
    double Oz = m_LA.z * (1.0-Ox) +  m_TA.z * Ox;
    m_SideA[l].y   = Oy +(m_SideA[l].y   - Oy)/cosdA;
    m_SideA[l].z   = Oz +(m_SideA[l].z   - Oz)/cosdA;
    m_SideA[l].setNormal(node.normal());

    m_SideA_Top[l].y = Oy +(m_SideA_Top[l].y - Oy)/cosdA;
    m_SideA_Top[l].z = Oz +(m_SideA_Top[l].z - Oz)/cosdA;
    m_SideA_Top[l].setNormal(nodeT.normal());

    m_SideA_Bot[l].y = Oy +(m_SideA_Bot[l].y - Oy)/cosdA;
    m_SideA_Bot[l].z = Oz +(m_SideA_Bot[l].z - Oz)/cosdA;
    m_SideA_Bot[l].setNormal(nodeB.normal());

    //rotate the point about the foil's neutral line to account for dihedral
    Vector3d axis(m_LA-m_TA);
    axis.normalize();
    m_SideA[l].rotate(m_LA, axis, alpha_dA);
    m_SideA_Top[l].rotate(m_LA, axis, alpha_dA);
    m_SideA_Bot[l].rotate(m_LA, axis, alpha_dA);

    getSideNode(xRelB, true, xfl::MIDSURFACE, node);
    getSideNode(xRelB, true, xfl::TOPSURFACE, nodeT);
    getSideNode(xRelB, true, xfl::BOTSURFACE, nodeB);
    m_SideB[l].setNode(node);
    m_SideB_Top[l].setNode(nodeT);
    m_SideB_Bot[l].setNode(nodeB);

    //scale the thickness
    Ox = xRelB;
    Oy = m_LB.y * (1.0-Ox) +  m_TB.y * Ox;
    Oz = m_LB.z * (1.0-Ox) +  m_TB.z * Ox;
    m_SideB[l].y   = Oy +(m_SideB[l].y   - Oy)/cosdB;
    m_SideB[l].z   = Oz +(m_SideB[l].z   - Oz)/cosdB;
    m_SideB[l].setNormal(node.normal());
    m_SideB_Top[l].y = Oy +(m_SideB_Top[l].y - Oy)/cosdB;
    m_SideB_Top[l].z = Oz +(m_SideB_Top[l].z - Oz)/cosdB;
    m_SideB_Top[l].setNormal(nodeT.normal());
    m_SideB_Bot[l].y = Oy +(m_SideB_Bot[l].y - Oy)/cosdB;
    m_SideB_Bot[l].z = Oz +(m_SideB_Bot[l].z - Oz)/cosdB;
    m_SideB_Bot[l].setNormal(nodeB.normal());

    //rotate the point about the foil's neutral line to account for dihedral
    axis.set(m_LB-m_TB);
    axis.normalize();
    m_SideB[l].rotate(m_LB, axis, alpha_dB);
    m_SideB_Top[l].rotate(m_LB, axis, alpha_dB);
    m_SideB_Bot[l].rotate(m_LB, axis, alpha_dB);

    if(pTranslatedFuse && m_bIsCenterSurf && m_bIsLeftSurf)
    {
        if(pTranslatedFuse->intersectFuse(m_SideA.at(l),     m_SideB.at(l),     I, false))
        {
            double d0 = I.distanceTo(m_SideA.at(l));
            double d1 = m_SideB.at(l).distanceTo(m_SideA.at(l));
            if(d0<d1)
            {
                m_SideB[l] = I;
                m_bJoinRight = false;
            }
        }
        if(pTranslatedFuse->intersectFuse(m_SideA_Bot.at(l), m_SideB_Bot.at(l), I, false))
        {
            double d0 = I.distanceTo(m_SideA_Bot.at(l));
            double d1 = m_SideB_Bot.at(l).distanceTo(m_SideA_Bot.at(l));
            if(d0<d1)
            {
                m_SideB_Bot[l] = I;
                m_bJoinRight = false;
            }
        }
        if(pTranslatedFuse->intersectFuse(m_SideA_Top.at(l), m_SideB_Top.at(l), I, false))
        {
            double d0 = I.distanceTo(m_SideA_Top.at(l));
            double d1 = m_SideB_Top.at(l).distanceTo(m_SideA_Top.at(l));
            if(d0<d1)
             {
                m_SideB_Top[l] = I;
                m_bJoinRight = false;
            }
        }
    }
    else if(pTranslatedFuse && m_bIsCenterSurf && m_bIsRightSurf)
    {
        if(pTranslatedFuse->intersectFuse(m_SideB.at(l),     m_SideA.at(l),     I, true))
        {
//                if(sqrt(I.y*I.y+I.z*I.z)>sqrt(m_SideA.at(l).y*m_SideA.at(l).y+m_SideA.at(l).z*m_SideA.at(l).z))
            if(I.distanceTo(m_SideB.at(l))<m_SideA.at(l).distanceTo(m_SideB.at(l)))
            {
                m_SideA[l]=I;
            }
        }
        if(pTranslatedFuse->intersectFuse(m_SideB_Bot.at(l), m_SideA_Bot.at(l), I, true))
        {
//                if(sqrt(I.y*I.y+I.z*I.z)>sqrt(m_SideA_Bot.at(l).y*m_SideA_Bot.at(l).y+m_SideA_Bot.at(l).z*m_SideA_Bot.at(l).z))
            if(I.distanceTo(m_SideB_Bot.at(l))<m_SideA_Bot.at(l).distanceTo(m_SideB_Bot.at(l)))
            {
                m_SideA_Bot[l]=I;
            }
        }
        if(pTranslatedFuse->intersectFuse(m_SideB_Top.at(l), m_SideA_Top.at(l), I, true))
        {
//                if(sqrt(I.y*I.y+I.z*I.z)>sqrt(m_SideA_Top.at(l).y*m_SideA_Top.at(l).y+m_SideA_Top.at(l).z*m_SideA_Top.at(l).z))
            if(I.distanceTo(m_SideB_Top.at(l))<m_SideA_Top.at(l).distanceTo(m_SideB_Top.at(l)))
            {
                m_SideA_Top[l]=I;
            }
        }
    }

    // force nodes into the xz symmetry plane to compensate for construction errors
    if(l==0 || l==m_NXPanels+1)
    {
        if(fabs(m_SideA[l].y)    <SYMMETRYPRECISION) m_SideA[l].y     = 0.0;
        if(fabs(m_SideA_Top[l].y)<SYMMETRYPRECISION) m_SideA_Top[l].y = 0.0;
        if(fabs(m_SideA_Bot[l].y)<SYMMETRYPRECISION) m_SideA_Bot[l].y = 0.0;
        if(fabs(m_SideB[l].y)    <SYMMETRYPRECISION) m_SideB[l].y     = 0.0;
        if(fabs(m_SideB_Top[l].y)<SYMMETRYPRECISION) m_SideB_Top[l].y = 0.0;
        if(fabs(m_SideB_Bot[l].y)<SYMMETRYPRECISION) m_SideB_Bot[l].y = 0.0;
    }
}


/**
 * Translates the entire Surface.
 * @param T the translation vector.
 */
void Surface::translate(Vector3d const &T)
{
    m_LA.translate(T);
    m_LB.translate(T);
    m_TA.translate(T);
    m_TB.translate(T);
    m_HingePoint.translate(T);
}


/**
 * Translates the entire Surface.
 * @param tx the x-component of the translation.
 */
void Surface::translate(double tx, double ty, double tz)
{
    m_LA.translate(tx, ty, tz);
    m_LB.translate(tx, ty, tz);
    m_TA.translate(tx, ty, tz);
    m_TB.translate(tx, ty, tz);
    m_HingePoint.translate(tx, ty, tz);
}


/**
 * Creates relative position of the mesh's master points on the left and right sides of the Surface.
 * The chordwise panel distribution is set i.a.w. with the flap hinges, if any.
 * The positions are stored in the member variables m_xPointA and m_xPointB.
 */
void Surface::createXPoints(int nRefXFlaps)
{
    int NXFlapA=0, NXFlapB=0, NXLeadA=0, NXLeadB=0;


    double xHingeA=0, xHingeB=0;

    m_xPointA.resize(m_NXPanels+1);
    m_xPointB.resize(m_NXPanels+1);

    Foil const *pFoilA = foilA();
    Foil const *pFoilB = foilB();

    if(pFoilA && pFoilA->hasTEFlap()) xHingeA=pFoilA->TEXHinge();
    else                              xHingeA=1.0;
    if(pFoilB && pFoilB->hasTEFlap()) xHingeB=pFoilB->TEXHinge();
    else                              xHingeB=1.0;

    if(nRefXFlaps>0)
    {
        NXFlapA = NXFlapB = nRefXFlaps;
    }
    else
    {
        NXFlapA = int((1.0-xHingeA) * double(m_NXPanels)*(1.000+LENGTHPRECISION));// to avoid numerical errors if exact division
        NXFlapB = int((1.0-xHingeB) * double(m_NXPanels)*(1.000+LENGTHPRECISION));

        if(pFoilA && pFoilA->hasTEFlap()) NXFlapA++;
        if(pFoilB && pFoilB->hasTEFlap()) NXFlapB++;

        // uniformize the number of flap panels if flaps are defined at each end
        if(NXFlapA>0 && NXFlapB>0)
        {
            int n = (NXFlapA+NXFlapB)/2;
            NXFlapA = n;
            NXFlapB = n;
        }
    }

    NXLeadA = m_NXPanels - NXFlapA;
    NXLeadB = m_NXPanels - NXFlapB;

    m_NXFlap  = std::max(NXFlapA, NXFlapB);
    //    if(m_NXFlap>m_NXPanels/2) m_NXFlap=(int)m_NXPanels/2;
    m_NXLead  = m_NXPanels - m_NXFlap;

    std::vector<double>fraclist;
    xfl::getPointDistribution(fraclist, NXFlapA, m_XDistType);
    for(int l=0; l<NXFlapA; l++)
    {
/*        dl =  double(l);
        dl2 = double(NXFlapA);
        if(m_XDistType==Xfl::COSINE) m_xPointA[l] = 1.0 - (1.0-xHingeA)/2.0 * (1.0-cos(dl*PI /dl2));
        else                         m_xPointA[l] = 1.0 - (1.0-xHingeA) * (dl/dl2);*/
        m_xPointA[l] = 1.0 - (1.0-xHingeA) * fraclist.at(l);
    }

    xfl::getPointDistribution(fraclist, NXLeadA, m_XDistType);
    for(int l=0; l<NXLeadA; l++)
    {
/*        dl =  double(l);
        dl2 = double(NXLeadA);
        if(m_XDistType==Xfl::COSINE) m_xPointA[l+NXFlapA] = xHingeA - (xHingeA)/2.0 * (1.0-cos(dl*PI /dl2));
        else                         m_xPointA[l+NXFlapA] = xHingeA - (xHingeA) * (dl/dl2);*/
        m_xPointA[l+NXFlapA] = xHingeA - (xHingeA) * fraclist.at(l);
    }

    xfl::getPointDistribution(fraclist, NXFlapB, m_XDistType);
    for(int l=0; l<NXFlapB; l++)
    {
/*        dl =  double(l);
        dl2 = double(NXFlapB);
        if(m_XDistType==Xfl::COSINE) m_xPointB[l] = 1.0 - (1.0-xHingeB)/2.0 * (1.0-cos(dl*PI /dl2));
        else                         m_xPointB[l] = 1.0 - (1.0-xHingeB) * (dl/dl2);*/
        m_xPointB[l] = 1.0 - (1.0-xHingeB) * fraclist.at(l);
    }

    xfl::getPointDistribution(fraclist, NXLeadB, m_XDistType);
    for(int l=0; l<NXLeadB; l++)
    {
/*        dl =  double(l);
        dl2 = double(NXLeadB);
        if(m_XDistType==Xfl::COSINE) m_xPointB[l+NXFlapB] = xHingeB - (xHingeB)/2.0 * (1.0-cos(dl*PI /dl2));
        else                         m_xPointB[l+NXFlapB] = xHingeB - (xHingeB) * (dl/dl2);*/
        m_xPointB[l+NXFlapB] = xHingeB - (xHingeB) * fraclist.at(l);
    }

    m_xPointA[m_NXPanels] = 0.0;
    m_xPointB[m_NXPanels] = 0.0;
}


/**
 * Sets the surface twist
 */
void Surface::setTwist(bool bQuarterChord)
{
    Vector3d A4, B4, U, T;
    if(bQuarterChord)
    {
        A4 = m_LA *3.0/4.0 + m_TA * 1/4.0;
        B4 = m_LB *3.0/4.0 + m_TB * 1/4.0;
    }
    else
    {
        A4 = m_LA;
        B4 = m_LB;
    }

    // create a vector perpendicular to NormalA and x-axis
    T.x = 0.0;
    T.y = +m_NormalA.z;
    T.z = -m_NormalA.y;
    //rotate around this axis
    U = m_LA-A4;
    U.rotate(T, m_TwistA);
    m_LA = A4+ U;

    U = m_TA-A4;
    U.rotate(T, m_TwistA);
    m_TA = A4 + U;

    m_NormalA.rotate(T, m_TwistA);

    // create a vector perpendicular to NormalB and x-axis
    T.x = 0.0;
    T.y = +m_NormalB.z;
    T.z = -m_NormalB.y;

    U = m_LB-B4;
    U.rotate(T, m_TwistB);
    m_LB = B4+ U;

    U = m_TB-B4;
    U.rotate(T, m_TwistB);
    m_TB = B4 + U;

    m_NormalB.rotate(T, m_TwistB);
}


void Surface::makeTriPanels(std::vector<Panel3> &panel3list, std::vector<Node> &nodes, int ip3start, bool bThickSurfaces, int nTipStrips, int &leftnodeidx, int &rightnodeidx)
{
    int iLA=0, iLB=0, iTA=0, iTB=0;
    m_Panel3List.clear();
    clearTriFlapMesh();

    int nx = m_NXPanels+1;  // number of nodes in the chordwise direction for a thin surface
    int nx2 = 2*m_NXPanels+1;  // number of nodes in the chordwise direction for a thick surface

    // add this surface's points to the array of nodes
    // always duplicate the nodes in the xz plane, in case there is a fuse in between
    bool bMakeLeftSideNodes = m_bIsTipLeft || (m_bIsCenterSurf&&m_bIsRightSurf);

    bMakeLeftSideNodes = true;

    int n0 = int(nodes.size());
    if(!bMakeLeftSideNodes)
    {
        // use last node column of adjacent left surface
        if(bThickSurfaces) n0 -= nx2;
        else               n0 -= nx;
    }

    bool bMakeLeftPatch  = bThickSurfaces && m_bClosedLeftSide;
    bool bMakeRightPatch = bThickSurfaces && m_bClosedRightSide;
    nTipStrips = std::max(1, nTipStrips);

    int nsn = 0;// index of the first bottom surface node

    if(bMakeLeftPatch) nsn = makeTipNodes(nodes, true, nTipStrips);
    // Identify the left rear tip node which will be used to connect all left rear tip triangles
    // from bot, side and top surface
    int nLeftTipRearNode = n0;

    makePanelNodes(nodes, bMakeLeftSideNodes, !bThickSurfaces);

    // Identify the right rear tip node which will be used to connect all left rear tip triangles
    // from bot, side and top surface
    int nRightTipRearNode = int(nodes.size()-1);

    if(bMakeRightPatch) makeTipNodes(nodes, false, nTipStrips);

    // the two tip nodes do not belong to any surface;
    // this is used in the analysis to set Cp=0 at these singular locations
    if(bMakeLeftPatch)
    {
        nodes[nLeftTipRearNode].setSurfacePosition(xfl::NOSURFACE);
        nodes[nLeftTipRearNode].setSurfaceIndex(m_Index);
    }
    if(bMakeRightPatch)
    {
        nodes[nRightTipRearNode].setSurfacePosition(xfl::NOSURFACE);
        nodes[nRightTipRearNode].setSurfaceIndex(m_Index);
    }

    int nNullTriangles=0;
    int nLA(0), nLB(0), nTA(0), nTB(0);


    if(bMakeLeftPatch)
    {
        // the top and bottom node rows are not used; the surface nodes are used instead
        // these nodes are left unconnected and unused in the node array
        int in = 0;

/*        s_DebugPts.clear();
        for(int i=0; i<m_SideA_Bot.size(); i++)  s_DebugPts.push_back(m_SideA_Bot.at(i));
        for(int i=m_SideA_Top.size()-2; i>=0; i--)  s_DebugPts.push_back(m_SideA_Top.at(i));
*/

        for(int is=0; is<nTipStrips; is++)
        {
            in = is * nx;
            // make a strip
            for (int l=0; l<m_NXPanels; l++)
            {
                nTA = n0+in;
                nLA = n0+in+1;
                nTB = n0+in+nx;
                nLB = n0+in+nx+1;

                // reuse top/bot nodes where applicable
                if(is==0)
                {
                    //use the bottom central surface nodes instead for LA and TA
                    nLA = n0 + nsn + l +1;
                    nTA = n0 + nsn + l;
                }
                if(is==nTipStrips-1)
                {
                    //use the top central surface nodes instead for LB and TB
                    nLB = n0 + nsn + nx2 - l -2;
                    nTB = n0 + nsn + nx2 - l -1;

                }

                // special treatment at the TE for all strips: connect to the one tip rear node
                if(l==0)
                {
                    nTA = nTB = nLeftTipRearNode;
                }

                if(l==m_NXPanels-1)
                {
                    // at the LE, use the surface leading node instead for all the strips
                    nLA = nLB = n0 + nsn + l +1;
                }

                // make the upper triangle
                Panel3 p3tup;
                p3tup.setFrame(nodes, nTA, nTB, nLB);
                if(!p3tup.isNullTriangle()) /** @todo any use? */
                {
                    panel3list.push_back(p3tup);
                    Panel3 &p3T = panel3list.back();
                    p3T.setSurfacePosition(xfl::SIDESURFACE);
                    p3T.setSurfaceIndex(m_Index);
                    p3T.setIndex(ip3start++);
                    p3T.m_bIsLeftWingPanel  = true;
                    p3T.m_iPD = int(panel3list.size())-2;
                    p3T.m_iPU = (l==m_NXPanels-1) ? -1 : int(panel3list.size());
                    if(m_bTEFlap && l<m_NXFlap)
                    {
                        addFlapPanel3Index(p3T.index());
                        p3T.setFlapPanel(true);
                    }
                    p3T.setSurfaceNormal(m_Normal);
                    addPanel3Index(p3T.index());
                }
                else nNullTriangles++;

                // make the lower triangle
                Panel3 p3tlw;
                p3tlw.setFrame(nodes, nTA, nLB, nLA);
                if(!p3tlw.isNullTriangle())
                {
                    panel3list.push_back(p3tlw);
                    Panel3 &p3B = panel3list.back();
                    p3B.setSurfacePosition(xfl::SIDESURFACE);
                    p3B.setSurfaceIndex(m_Index);
                    p3B.setIndex(ip3start++);
                    p3B.m_bIsLeftWingPanel  = true;
                    p3B.m_iPD = int(panel3list.size())-2;
                    p3B.m_iPU = int(panel3list.size());
                    if(m_bTEFlap && l<NXFlap())
                    {
                        addFlapPanel3Index(p3B.index());
                        p3B.setFlapPanel(true);
                    }
                    p3B.setSurfaceNormal(m_Normal);
                    addPanel3Index(p3B.index());
                }
                else nNullTriangles++;

                in++;
            }
        }
        // move right one column of nodes, to not re-use the tip nodes
        n0 += (nTipStrips+1)*nx;
    }

    m_FirstStripPanelIndex = int(panel3list.size());
    xfl::enumSurfacePosition side(xfl::NOSURFACE);
    for (int k=0; k<m_NYPanels; k++)
    {
        //from T.E. to L.E.
        for (int l=0; l<2*m_NXPanels; l++)
        {
            Panel3 p3Dtmp, p3Utmp;
            if(!bThickSurfaces)
            {
                side = xfl::MIDSURFACE;
                if(l>=m_NXPanels) break;
            }
            else
            {
                if(l<m_NXPanels) side = xfl::BOTSURFACE;
                else             side = xfl::TOPSURFACE;
            }

            // Bottom surface:
            //     Left wing             Right wing
            //   0           2         0           2
            //   _____________         _____________
            //   |          /|0       0|\          |
            //   | left   /  |         |  \  right |
            //   |      /    |         |    \      |
            //   |    /      |         |      \    |
            //   |  /  right |         | left   \  |
            //   |/          |         |          \| 1
            // 1 _____________         _____________
            //   1           2         1           2
            //

            if(side==xfl::MIDSURFACE)
            {
                iTA = n0 +  k   *nx + l;
                iLA = n0 +  k   *nx + l+1;
                iTB = n0 + (k+1)*nx + l;
                iLB = n0 + (k+1)*nx + l+1;
            }
            else
            {
                iTA = n0 +  k   *nx2 + l;
                iLA = n0 +  k   *nx2 + l+1;
                iTB = n0 + (k+1)*nx2 + l;
                iLB = n0 + (k+1)*nx2 + l+1;

                // in the case of thick surfaces, make sure that top, bot and side panels are connected
                // at the rear tips
                if(m_bClosedLeftSide)
                {
                    if(k==0)
                    {
                        if(l==0)                   iTA = nLeftTipRearNode;
                        else if(l==2*m_NXPanels-1) iLA = nLeftTipRearNode;
                    }
                }
                if(m_bClosedRightSide)
                {
                    if(k==m_NYPanels-1)
                    {
                        if(l==0)
                            iTB = nRightTipRearNode;
                        else if(l==2*m_NXPanels-1)
                            iLB = nRightTipRearNode;
                    }
                }

                // note the strip starting nodes for later use
                if(l==0)
                {
                    if(k==0)
                        leftnodeidx = iTA;
                    if(k==m_NYPanels-1)
                        rightnodeidx = iTB;
                }
            }

            if(m_bIsLeftSurf)
            {
                if(side==xfl::MIDSURFACE)
                {
                    p3Dtmp.setFrame(nodes, iLB, iTA, iTB);
                    p3Utmp.setFrame(nodes, iLA, iTA, iLB);
                    p3Utmp.setLeftSidePanel(true);
                }
                else if(side==xfl::BOTSURFACE)
                {
                    p3Dtmp.setFrame(nodes, iLB, iTB, iTA);
                    p3Utmp.setFrame(nodes, iLA, iLB, iTA);
                    p3Utmp.setLeftSidePanel(true);
                }
                else
                {
                    p3Dtmp.setFrame(nodes, iTB, iTA, iLA);
                    p3Utmp.setFrame(nodes, iTB, iLA, iLB);
                    p3Dtmp.setLeftSidePanel(true);
                }

                if(l==0)
                {
                    p3Dtmp.m_bIsTrailing = true;
                    if (side==xfl::BOTSURFACE || side==xfl::MIDSURFACE)
                    {
                        p3Dtmp.m_TELeftBisector  = TEbisector(p3Dtmp.vertexAt(1));
                        p3Dtmp.m_TERightBisector = TEbisector(p3Dtmp.vertexAt(2));
                    }
                }
                if(l==m_NXPanels-1)   p3Utmp.m_bIsLeading  = true;
                if(l==m_NXPanels)     p3Dtmp.m_bIsLeading  = true;
                if(l==2*m_NXPanels-1) p3Utmp.m_bIsTrailing = true;
            }
            else
            {
                if(side==xfl::MIDSURFACE)
                {
                    p3Dtmp.setFrame(nodes, iLA, iTA, iTB);
                    p3Utmp.setFrame(nodes, iTB, iLB, iLA);// so that (l,m) is same as opposite panel on right surface
                    p3Dtmp.setLeftSidePanel(true);
                }
                else if(side==xfl::BOTSURFACE)
                {
                    p3Dtmp.setFrame(nodes, iLA, iTB, iTA);
                    p3Utmp.setFrame(nodes, iLA, iLB, iTB);
                    p3Dtmp.setLeftSidePanel(true);
                }
                else
                {
                    p3Dtmp.setFrame(nodes, iTB, iTA, iLB);
                    p3Utmp.setFrame(nodes, iTA, iLA, iLB);
                    p3Utmp.setLeftSidePanel(true);
                }
                if(l==0)
                {
                    p3Dtmp.m_bIsTrailing = true;
                    if (side==xfl::BOTSURFACE || side==xfl::MIDSURFACE)
                    {
                        p3Dtmp.m_TELeftBisector  = TEbisector(p3Dtmp.vertexAt(1));
                        p3Dtmp.m_TERightBisector = TEbisector(p3Dtmp.vertexAt(2));
                    }
                }
                if(l==NXPanels()-1)   p3Utmp.m_bIsLeading  = true;
                if(l==NXPanels())     p3Dtmp.m_bIsLeading  = true;
                if(l==2*NXPanels()-1) p3Utmp.m_bIsTrailing = true;
            }

            if(!p3Dtmp.isNullTriangle())
            {
                panel3list.push_back(p3Dtmp);
                Panel3 &p3D = panel3list.back();
                p3D.setSurfacePosition(side);
                p3D.setSurfaceIndex(m_Index);
                p3D.setIndex(ip3start++);
                p3D.m_bIsInSymPlane  = m_bIsInSymPlane;
                p3D.m_bIsLeftWingPanel  = m_bIsLeftSurf;
                if(p3D.isTrailing())
                {
                    if     (side==xfl::BOTSURFACE)
                        p3D.setOppositeIndex(p3D.index()+4*m_NXPanels-1);
                    else if(side==xfl::TOPSURFACE)
                        p3D.setOppositeIndex(p3D.index()-4*m_NXPanels+1); // should not ne reached
                }

                // note: UP and DOWN are defined on the developed surface
                // i.e. UP direction is towards the TE on the top surface
                p3D.m_iPD = int(panel3list.size())-2;
                p3D.m_iPU = int(panel3list.size());
                if(l==0)              p3D.m_iPD = -1;// no panel downstream
                if(m_bTEFlap &&  (l<NXFlap() || l>2*m_NXPanels-NXFlap()-1))
                {
                    p3D.setFlapPanel(true);
                    addFlapPanel3Index(p3D.index());
                }
                p3D.setSurfaceNormal(m_Normal);
                addPanel3Index(p3D.index());
            }
            else nNullTriangles++;

            if(!p3Utmp.isNullTriangle())
            {
                panel3list.push_back(p3Utmp);
                Panel3 &p3U = panel3list.back();
                p3U.setSurfacePosition(side);
                p3U.setSurfaceIndex(m_Index);
                p3U.setIndex(ip3start++);
                p3U.m_bIsInSymPlane  = m_bIsInSymPlane;
                p3U.m_bIsLeftWingPanel  = m_bIsLeftSurf;
                if(p3U.isTrailing())
                {
                    if     (side==xfl::BOTSURFACE)
                        p3U.setOppositeIndex(p3U.index()+4*m_NXPanels-1);// should not ne reached
                    else if(side==xfl::TOPSURFACE)
                        p3U.setOppositeIndex(p3U.index()-4*m_NXPanels+1);
                }

                // note: UP and DOWN are defined on the developed surface - NO
                // i.e. UP direction is towards the TE on the top surface - NO
                p3U.m_iPD = int(panel3list.size())-2;
                p3U.m_iPU = int(panel3list.size());

                if(bThickSurfaces)
                {
                    if(l==2*m_NXPanels-1) p3U.m_iPU = -1;// no panel upstream
                }
                else
                {
                    if(l==m_NXPanels-1) p3U.m_iPU = -1;// no panel upstream
                }

                if(m_bTEFlap &&  (l<NXFlap() || l>2*m_NXPanels-NXFlap()-1))
                {
                    p3U.setFlapPanel(true);
                    addFlapPanel3Index(p3U.index());
                }

                p3U.setSurfaceNormal(m_Normal);
                addPanel3Index(p3U.index());
            }
            else nNullTriangles++;
        }
    }

    if(bThickSurfaces)
        m_LastStripPanelIndex = int(panel3list.size())-2*2*m_NXPanels;
    else
        m_LastStripPanelIndex = int(panel3list.size())-  2*m_NXPanels;

    n0 += (2*m_NXPanels+1)*(m_NYPanels+1);
    if(bMakeRightPatch)
    {
        int in = 0;

        for(int is=0; is<nTipStrips; is++)
        {
            in = is * nx;
            // make a strip
            for (int l=0; l<m_NXPanels; l++)
            {
                nLA = n0+in+1;
                nTA = n0+in;
                nLB = n0+nx+in+1;
                nTB = n0+nx+in;

                // reuse top/bot nodes where applicable
                if(is==0)
                {
                    nLA = n0 -nx2 + l +1;
                    nTA = n0 -nx2 + l;
                    //special treatment at the LE

               }               
                if(is==nTipStrips-1)
                {
                    nLB = n0 - l - 2;
                    nTB = n0 - l - 1;

                    //special treatment at the TE
//                    if(l==0) nTA = nTB;
                }

                // special treatment at the TE for all strips: connect to the one tip rear node
                if(l==0) nTA = nTB = nRightTipRearNode;

                if(l==m_NXPanels-1)
                {
                    nLB = nLA = n0 -nx2 + l +1;
                }

                // make the upper triangle
                Panel3 p3tup;
                p3tup.setFrame(nodes, nLB, nTB, nTA);
                if(p3tup.isNullTriangle())
                {
                    nNullTriangles++;
                }
                else
                {
                    panel3list.push_back(p3tup);
                    Panel3 &p3T = panel3list.back();
                    p3T.setSurfacePosition(xfl::SIDESURFACE);
                    p3T.setSurfaceIndex(m_Index);
                    p3T.setIndex(ip3start++);
                    p3T.m_bIsLeftWingPanel  = true;
                    p3T.m_iPD = int(panel3list.size())-2;
                    p3T.m_iPU = (l==m_NXPanels-1) ? -1 : int(panel3list.size());
                    if(m_bTEFlap && l<m_NXFlap)
                    {
                        addFlapPanel3Index(p3T.index());
                        p3T.setFlapPanel(true);
                    }
                    p3T.setSurfaceNormal(m_Normal);
                    addPanel3Index(p3T.index());
                }

                //make the lower triangle
                Panel3 p3tlw;
                p3tlw.setFrame(nodes, nLA, nLB, nTA);
                if(p3tlw.isNullTriangle())
                {
                    nNullTriangles++;
                }
                else
                {
                    panel3list.push_back(p3tlw);
                    Panel3 &p3B = panel3list.back();
                    p3B.setSurfacePosition(xfl::SIDESURFACE);
                    p3B.setSurfaceIndex(m_Index);
                    p3B.setIndex(ip3start++);
                    p3B.m_bIsLeftWingPanel  = true;
                    p3B.m_iPD = int(panel3list.size())-2;
                    p3B.m_iPU = int(panel3list.size());
                    if(m_bTEFlap && l<m_NXFlap)
                    {
                        addFlapPanel3Index(p3B.index());
                        p3B.setFlapPanel(true);
                    }
                    p3B.setSurfaceNormal(m_Normal);
                    addPanel3Index(p3B.index());
                }

                in++;
            }
        }
    }

    std::sort(m_Panel3List.begin(), m_Panel3List.end());

    (void)nNullTriangles;
}


int Surface::makeQuadPanels(std::vector<Panel4> &panel4list, int &nWakeColumn, bool bThickSurfaces, int nTipStrips)
{
    int k=0;
    int n4_0 = int(panel4list.size());

    int n4 = 0;
    xfl::enumSurfacePosition side = xfl::NOSURFACE;
    Vector3d LA, LB, TA, TB;

    bool bLeftPatch  = bThickSurfaces && m_bClosedLeftSide;
    bool bRightPatch = bThickSurfaces && m_bClosedRightSide;
    nTipStrips = std::max(1, nTipStrips);
    int nStripNodes = m_NXPanels+1;

    m_Panel4List.clear();
    clearQuadFlapMesh();

    if (bLeftPatch)
    {
        int nLA=0, nLB=0, nTA=0, nTB=0;
        std::vector<Node> nodes;
        makeTipNodes(nodes, true, nTipStrips);
        int in = 0;
        int nsp = nTipStrips * m_NXPanels;

        for(int is=0; is<nTipStrips; is++)
        {
            in = is * nStripNodes;
            // make a strip
            for (int l=0; l<m_NXPanels; l++)
            {
                Panel4 p4tmp;
                nTB = in + l + nStripNodes;
                nTA = in + l + nStripNodes + 1;
                nLB = in + l;
                nLA = in + l + 1;
                p4tmp.setPanelFrame(nodes.at(nLA), nodes.at(nTA), nodes.at(nLB), nodes.at(nTB));

                panel4list.push_back(p4tmp);
                Panel4 &p4T = panel4list.back();
                p4T.setSurfacePosition(xfl::SIDESURFACE);
                p4T.setSurfaceIndex(m_Index);
                p4T.setIndex(int(panel4list.size())-1);
                p4T.m_bIsLeftWingPanel  = true;
                if(l>0)             p4T.m_iPD = int(panel4list.size())-2;
                if(l<m_NXPanels-1)  p4T.m_iPU = int(panel4list.size());

                if(is==0)           p4T.m_iPL = n4_0 + nsp+l;
                else                p4T.m_iPL = p4T.index() - m_NXPanels;

                if(is<nTipStrips-1) p4T.m_iPR = p4T.index() + m_NXPanels;
                else                p4T.m_iPR = n4_0 + nsp + 2*m_NXPanels-1-l;

                if(m_bTEFlap && l<m_NXFlap)
                {
                    addFlapPanel4(p4T);
                    p4T.setFlapPanel(true);
                }
                p4T.setSurfaceNormal(m_Normal);
                addPanel4Index(p4T.index());
                n4++;
            }
            in++;
        }
    }

    int nb0 = int(panel4list.size()); // the index of the first bottom surface panel

    for (k=0; k<NYPanels(); k++)
    {
        //add "horizontal" panels, mid side, or following a strip from bot to top if 3D Panel
        if(bThickSurfaces)  side = xfl::BOTSURFACE;  //start with lower surf,
        else                side = xfl::MIDSURFACE;
        //from T.E. to L.E.
        for (int l=0; l<NXPanels(); l++)
        {
            panel4list.push_back({});
            Panel4 &p4 = panel4list.back();
            getPanel(k, l, side, LA, LB, TA, TB);

            if(l==0)            p4.setTrailing(true);
            if(l==NXPanels()-1) p4.setLeading(true);

            p4.m_bIsInSymPlane  = isInSymPlane();

            p4.setSurfacePosition(side);
            p4.setSurfaceIndex(m_Index);
            p4.setIndex(n4_0 + n4);
            p4.m_bIsLeftWingPanel  = isLeftSurf();

            if(side==xfl::MIDSURFACE)
                p4.setPanelFrame(LA, LB, TA, TB);
            else if (side==xfl::BOTSURFACE)
                p4.setPanelFrame(LB, LA, TB, TA);

            // set neighbour panels
            // valid only for Panel 2-sided Analysis
            // we are on the bottom or middle surface
            p4.m_iPD = n4_0 + n4-1;
            p4.m_iPU = n4_0 + n4+1;
            if(l==0)                                     p4.m_iPD = -1;// no panel downstream
            if(l==NXPanels()-1 && side==xfl::MIDSURFACE) p4.m_iPU = -1;// no panel upstream

            if(side==xfl::MIDSURFACE)
            {
                //wings are modelled as thin surfaces
                p4.m_iPR = n4_0 + n4 + NXPanels();
                p4.m_iPL = n4_0 + n4 - NXPanels();
                if(k==0            && isTipLeft())  p4.m_iPL = -1;
                if(k==NYPanels()-1 && isTipRight()) p4.m_iPR = -1;
            }
            else
            {
                //wings are modelled as thick surfaces
                p4.m_iPL = n4_0 + n4+2*NXPanels();
                p4.m_iPR = n4_0 + n4-2*NXPanels();
                if(k==0            && m_bClosedLeftSide)  p4.m_iPR = -1; // for simplicity connect once all surface panels are built
                if(k==NYPanels()-1 && m_bClosedRightSide) p4.m_iPL = -1;
            }

            if(side==xfl::MIDSURFACE)
            {
                if(k==0)            p4.m_iPL = -1;
                if(k==NYPanels()-1) p4.m_iPR = -1;

            }
            else
            {
                if(k==0)            p4.m_iPR = -1;
                if(k==NYPanels()-1) p4.m_iPL = -1;
            }

            if(p4.isTrailing())
            {
                p4.setWakeColumn(nWakeColumn);
                if(!bThickSurfaces)
                {
                }
                if (p4.isBotPanel() || p4.isMidPanel())
                {
                    p4.m_TELeftBisector  = TEbisector(TA);
                    p4.m_TERightBisector = TEbisector(TB);
                }
            }

            if(m_bTEFlap && l<NXFlap())
            {
                addFlapPanel4(p4);
                p4.setFlapPanel(true);
            }
            p4.setSurfaceNormal(m_Normal);
            addPanel4Index(p4.index());

            n4++;
        }

        if (bThickSurfaces)
        {
            side = xfl::TOPSURFACE;
            //from L.E. to T.E.
            for (int l=NXPanels()-1;l>=0; l--)
            {
                panel4list.push_back({});
                Panel4 &p4 = panel4list.back();
                getPanel(k,l,side, LA, LB, TA, TB);

                if(l==0)            p4.setTrailing(true);
                if(l==NXPanels()-1) p4.setLeading(true);

                p4.m_bIsInSymPlane  = isInSymPlane();


                p4.setSurfacePosition(side);
                p4.setSurfaceIndex(m_Index);
                p4.setIndex(n4_0 + n4);
                p4.m_bIsLeftWingPanel  = isLeftSurf();

                p4.setPanelFrame(LA, LB, TA, TB);

                // set neighbour panels
                // valid only for Panel 2-sided Analysis
                // we are on the top surface
                p4.m_iPD = n4_0 + n4-1;
                p4.m_iPU = n4_0 + n4+1;
                if(l==0)  p4.m_iPU = -1;// no panel downstream - changed the connection dir in v7

                p4.m_iPL = n4_0 + n4-2*NXPanels();//assuming all wing panels have same chordwise distribution
                p4.m_iPR = n4_0 + n4+2*NXPanels();//assuming all wing panels have same chordwise distribution

                if(k==0            && m_bClosedLeftSide)  p4.m_iPL = -1; // for simplicity connect once all surface panels are built
                if(k==NYPanels()-1 && m_bClosedRightSide) p4.m_iPR = -1;

                //do not link to next surfaces... will be done in JoinSurfaces() if surfaces are continuous
                if(k==0)            p4.m_iPL = -1;
                if(k==NYPanels()-1) p4.m_iPR = -1;


                if(p4.isTrailing())
                {
                    //                    p4.m_iWake = m_nWakePanel4;//next wake element
                    //                    p4.m_iWakeColumn = nWakeColumn;
                    //                    makeWakeElems(p4, pPlane, pWPolar);
                }

                if(m_bTEFlap && l<NXFlap())
                {
                    addFlapPanel4(p4);
                    p4.setFlapPanel(true);
                }
                p4.setSurfaceNormal(m_Normal);
                addPanel4Index(p4.index());
                n4++;
            }
            nWakeColumn++;
        }
    }

    if (bRightPatch)
    {
        int nLA=0, nLB=0, nTA=0, nTB=0;
        std::vector<Node> nodes;
        makeTipNodes(nodes, false, nTipStrips);
        int in = 0;
        int nsp = int(panel4list.size())-1; // the index of the last top surface panel
        for(int is=0; is<nTipStrips; is++)
        {
            in = is * nStripNodes;
            // make a strip
            for (int l=0; l<m_NXPanels; l++)
            {
                Panel4 p4tmp;
                nLA = in + l + nStripNodes + 1;
                nLB = in + l + 1;
                nTA = in + l + nStripNodes;
                nTB = in + l;
                p4tmp.setPanelFrame(nodes.at(nLA), nodes.at(nLB), nodes.at(nTA), nodes.at(nTB));

                panel4list.push_back(p4tmp); // avoids potential problems with side trailing panels in the fin if in sym plane
                Panel4 &p4T = panel4list.back();
                p4T.setSurfacePosition(xfl::SIDESURFACE);
                p4T.setSurfaceIndex(m_Index);
                p4T.setIndex(int(panel4list.size())-1);
                p4T.m_bIsLeftWingPanel  = true;

                if(l>0)            p4T.m_iPD = int(panel4list.size())-2;
                if(l<m_NXPanels-1) p4T.m_iPU = int(panel4list.size());

                if(is==0)           p4T.m_iPR = nsp - 2*m_NXPanels + l + 1;
                else                p4T.m_iPR = p4T.index() - m_NXPanels;

                if(is<nTipStrips-1) p4T.m_iPL = p4T.index() + m_NXPanels;
                else                p4T.m_iPL = nsp-l;

                if(m_bTEFlap && l<m_NXFlap)
                {
                    addFlapPanel4(p4T);
                    p4T.setFlapPanel(true);
                }
                p4T.setSurfaceNormal(m_Normal);
                addPanel4Index(p4T.index());
                n4++;
            }
            in++;
        }
    }

    // connect closed tips
    if(bLeftPatch)
    {
        // connect first strip
        for(int l=0; l<2*m_NXPanels; l++)
        {
            Panel4 &p4 = panel4list[nb0+l];
            if(p4.isBotPanel())
            {
                for(int i4=0; i4<m_NXPanels; i4++)
                {
                    int index = n4_0+i4;
                    Panel4 &p4tip = panel4list[index];
                    if(p4.LB().isSame(p4tip.LA(), 1.e-4) && p4.TB().isSame(p4tip.TA(), 1.0e-4))
                    {
                        p4.m_iPR = p4tip.index();
                        p4tip.m_iPL = p4.index();
                    }
                }
            }
            else if(p4.isTopPanel())
            {
                for(int i4=0; i4<m_NXPanels; i4++)
                {
                    int index = n4_0+(nTipStrips-1)*m_NXPanels+i4;
                    Panel4 &p4tip = panel4list[index];
                    if(p4.LA().isSame(p4tip.LB(), 1.e-4) && p4.TA().isSame(p4tip.TB(), 1.0e-4))
                    {
                        p4.m_iPL = p4tip.index();
                        p4tip.m_iPR = p4.index();
                    }
                }
            }
        }
    }
    if(bRightPatch)
    {
        nb0 += (m_NYPanels-1)*2*m_NXPanels;
        int ntip0 = nb0 + 2*m_NXPanels;
        // connect last strip
        for(int l=0; l<2*m_NXPanels; l++)
        {
            Panel4 &p4 = panel4list[nb0+l];
            if(p4.isBotPanel())
            {
                for(int i4=0; i4<m_NXPanels; i4++)
                {
                    int index = ntip0+i4;
                    Panel4 &p4tip = panel4list[index];
                    if(p4.LA().isSame(p4tip.LB(), 1.e-4) && p4.TA().isSame(p4tip.TB(), 1.0e-4))
                    {
                        p4.m_iPL = p4tip.index();
                        p4tip.m_iPR = p4.index();
                    }
                }
            }
            else if(p4.isTopPanel())
            {
                for(int i4=0; i4<m_NXPanels; i4++)
                {
                    int index = ntip0+(nTipStrips-1)*m_NXPanels+i4;
                    Panel4 &p4tip = panel4list[index];
                    if(p4.LB().isSame(p4tip.LA(), 1.e-4) && p4.TB().isSame(p4tip.TA(), 1.0e-4))
                    {
                        p4.m_iPR = p4tip.index();
                        p4tip.m_iPL = p4.index();
                    }
                }
            }
        }
    }

    setNQuads(n4);

    return nQuads();
}


Foil const *Surface::foilA() const
{
    return Objects2d::foil(m_FoilNameA);
}


Foil const *Surface::foilB() const
{
    return Objects2d::foil(m_FoilNameB);
}


Foil *Surface::foilA()
{
    return Objects2d::foil(m_FoilNameA);
}


Foil *Surface::foilB()
{
    return Objects2d::foil(m_FoilNameB);
}


//UNUSED
bool Surface::makeSectionSplines(BSpline3d &leftspline, BSpline3d &rightspline) const
{
    int nCtrlPoints = 11;
    int nPoints = 2*nCtrlPoints; // minimum to get a good approximation
    std::vector<Vector3d> PtA_T(nPoints),  PtA_B(nPoints), PtB_T(nPoints), PtB_B(nPoints);
    std::vector<Vector3d> NA(nPoints), NB(nPoints);
    std::vector<double> xdistrib;
    xfl::getPointDistribution(xdistrib, nPoints-1, xfl::COSINE);

    getSidePoints_2(xfl::TOPSURFACE, nullptr, PtA_T, PtB_T, NA, NB, xdistrib, xdistrib);
    getSidePoints_2(xfl::BOTSURFACE, nullptr, PtA_B, PtB_B, NA, NB, xdistrib, xdistrib);
    //Left Spline
    std::vector<Vector3d> points;
    for(int i=int(PtA_B.size()-1); i>=0; i--) points.push_back(PtA_B.at(i));
    for(int i=0; i<int(PtA_T.size()); i++) points.push_back(PtA_T.at(i));
    if(!leftspline.approximate(3, nCtrlPoints, points)) return false;

    //Right spline
    points.clear();
    for(int i=int(PtB_B.size()-1); i>=0; i--) points.push_back(PtB_B.at(i));
    for(int i=1; i<int(PtB_T.size()); i++) points.push_back(PtB_T.at(i));
    if(!rightspline.approximate(3, nCtrlPoints, points)) return false;

    return true;
}


bool Surface::makeSectionHalfSpline(xfl::enumSurfacePosition pos, bool bLeft, int degree, int nCtrlPoints, int nOutPoints, BSpline3d &spline) const
{
    if(pos!=xfl::TOPSURFACE && pos!=xfl::BOTSURFACE && pos!=xfl::MIDSURFACE)
        return false;

//    int degree = 3;
//    int nCtrlPoints = 11;
//    int nPoints = 2*nCtrlPoints; // minimum to get a good approximation
    std::vector<Vector3d> PtA(nOutPoints), PtB(nOutPoints);
    std::vector<Vector3d> NA(nOutPoints), NB(nOutPoints);
    std::vector<double> xdistrib;
    xfl::getPointDistribution(xdistrib, nOutPoints-1, xfl::COSINE); // ensures good resolution at LE and TE

    getSidePoints_2(pos, nullptr, PtA, PtB, NA, NB, xdistrib, xdistrib);

    if(bLeft)
    {
        spline.approximate(degree, nCtrlPoints, PtA);
        return !spline.isSingular();
    }
    else
    {
        spline.approximate(degree, nCtrlPoints, PtB);
        return !spline.isSingular();
    }
}


// UNUSED
bool Surface::makeSectionSplinesOcc(bool bTop, bool bLeft, Handle(Geom_BSplineCurve)& theSpline) const
{
    int nCtrlPoints = 11;
    int nPoints = 2*nCtrlPoints; // minimum to get a good approximation
    std::vector<Vector3d> PtA(nPoints), PtB(nPoints);
    std::vector<Vector3d> NA(nPoints), NB(nPoints);
    std::vector<double> xdistrib;
    xfl::getPointDistribution(xdistrib, nPoints-1, xfl::COSINE); // ensures good resolution at LE and TE

    if(bTop)
        getSidePoints_2(xfl::TOPSURFACE, nullptr, PtA, PtB, NA, NB, xdistrib, xdistrib);
    else
        getSidePoints_2(xfl::BOTSURFACE, nullptr, PtA, PtB, NA, NB, xdistrib, xdistrib);

    std::string strange;
    if(bLeft) return occ::makeOCCSplineFromPoints(PtA, theSpline, strange);
    else      return occ::makeOCCSplineFromPoints(PtB, theSpline, strange);
}


void Surface::rotateX(Vector3d const&O, double XTilt)
{
    m_LA.rotateX(O, XTilt);
    m_LB.rotateX(O, XTilt);
    m_TA.rotateX(O, XTilt);
    m_TB.rotateX(O, XTilt);
    m_HingePoint.rotateX(O, XTilt);

    m_Normal.rotateX(XTilt);
    m_NormalA.rotateX(XTilt);
    m_NormalB.rotateX(XTilt);
    m_HingeVector.rotateX(XTilt);
}


void Surface::rotateY(Vector3d const &O, double YTilt)
{
    m_LA.rotateY(O, YTilt);
    m_LB.rotateY(O, YTilt);
    m_TA.rotateY(O, YTilt);
    m_TB.rotateY(O, YTilt);
    m_HingePoint.rotateY(O, YTilt);

    m_Normal.rotateY(YTilt);
    m_NormalA.rotateY(YTilt);
    m_NormalB.rotateY(YTilt);
    m_HingeVector.rotateY(YTilt);
}


void Surface::rotateZ(Vector3d const &O, double ZTilt)
{
    m_LA.rotateZ(O, ZTilt);
    m_LB.rotateZ(O, ZTilt);
    m_TA.rotateZ(O, ZTilt);
    m_TB.rotateZ(O, ZTilt);
    m_HingePoint.rotateZ(O, ZTilt);

    Vector3d Origin(0.0,0.0,0.0);
    m_Normal.rotateZ(Origin, ZTilt);
    m_NormalA.rotateZ(Origin, ZTilt);
    m_NormalB.rotateZ(Origin, ZTilt);
    m_HingeVector.rotateZ(Origin, ZTilt);
}


/**
 * Calculates the corner points of the panel with index k in the span direction and index l in the chordwise direction.
 * The point coordinates are loaded in the memeber variables LA, LB, TA, TB.
 *
 * Assumes the side points have been set previously
 *
 * @param k the index of the strip 0<=k<m_NYPanels
 * @param l the index of the panel in the chordwise direction. 0<=l<m_NXPanels
 * @param pos defines on which surface (Xfl::BOTSURFACE, Xfl::TOPSURFACE, Xfl::MIDSURFACE) the node positions should be calculated.
 */
void Surface::getPanel(int k, int l, xfl::enumSurfacePosition pos,
                              Vector3d &LA, Vector3d &LB, Vector3d &TA, Vector3d &TB) const
{
    double y1=0, y2=0;
    getYDist(k,y1,y2);
    if(pos==xfl::MIDSURFACE)
    {
        LA.x = m_SideA.at(l+1).x * (1.0-y1) + m_SideB.at(l+1).x* y1;
        LA.y = m_SideA.at(l+1).y * (1.0-y1) + m_SideB.at(l+1).y* y1;
        LA.z = m_SideA.at(l+1).z * (1.0-y1) + m_SideB.at(l+1).z* y1;
        TA.x = m_SideA.at(l).x   * (1.0-y1) + m_SideB.at(l).x  * y1;
        TA.y = m_SideA.at(l).y   * (1.0-y1) + m_SideB.at(l).y  * y1;
        TA.z = m_SideA.at(l).z   * (1.0-y1) + m_SideB.at(l).z  * y1;
        LB.x = m_SideA.at(l+1).x * (1.0-y2) + m_SideB.at(l+1).x* y2;
        LB.y = m_SideA.at(l+1).y * (1.0-y2) + m_SideB.at(l+1).y* y2;
        LB.z = m_SideA.at(l+1).z * (1.0-y2) + m_SideB.at(l+1).z* y2;
        TB.x = m_SideA.at(l).x   * (1.0-y2) + m_SideB.at(l).x  * y2;
        TB.y = m_SideA.at(l).y   * (1.0-y2) + m_SideB.at(l).y  * y2;
        TB.z = m_SideA.at(l).z   * (1.0-y2) + m_SideB.at(l).z  * y2;
    }
    else if (pos==xfl::BOTSURFACE)
    {
        LA.x = m_SideA_Bot.at(l+1).x * (1.0-y1) + m_SideB_Bot.at(l+1).x* y1;
        LA.y = m_SideA_Bot.at(l+1).y * (1.0-y1) + m_SideB_Bot.at(l+1).y* y1;
        LA.z = m_SideA_Bot.at(l+1).z * (1.0-y1) + m_SideB_Bot.at(l+1).z* y1;
        TA.x = m_SideA_Bot.at(l).x   * (1.0-y1) + m_SideB_Bot.at(l).x  * y1;
        TA.y = m_SideA_Bot.at(l).y   * (1.0-y1) + m_SideB_Bot.at(l).y  * y1;
        TA.z = m_SideA_Bot.at(l).z   * (1.0-y1) + m_SideB_Bot.at(l).z  * y1;
        LB.x = m_SideA_Bot.at(l+1).x * (1.0-y2) + m_SideB_Bot.at(l+1).x* y2;
        LB.y = m_SideA_Bot.at(l+1).y * (1.0-y2) + m_SideB_Bot.at(l+1).y* y2;
        LB.z = m_SideA_Bot.at(l+1).z * (1.0-y2) + m_SideB_Bot.at(l+1).z* y2;
        TB.x = m_SideA_Bot.at(l).x   * (1.0-y2) + m_SideB_Bot.at(l).x  * y2;
        TB.y = m_SideA_Bot.at(l).y   * (1.0-y2) + m_SideB_Bot.at(l).y  * y2;
        TB.z = m_SideA_Bot.at(l).z   * (1.0-y2) + m_SideB_Bot.at(l).z  * y2;
    }
    else if (pos==xfl::TOPSURFACE)
    {
        LA.x = m_SideA_Top.at(l+1).x * (1.0-y1) + m_SideB_Top.at(l+1).x* y1;
        LA.y = m_SideA_Top.at(l+1).y * (1.0-y1) + m_SideB_Top.at(l+1).y* y1;
        LA.z = m_SideA_Top.at(l+1).z * (1.0-y1) + m_SideB_Top.at(l+1).z* y1;
        TA.x = m_SideA_Top.at(l).x   * (1.0-y1) + m_SideB_Top.at(l).x  * y1;
        TA.y = m_SideA_Top.at(l).y   * (1.0-y1) + m_SideB_Top.at(l).y  * y1;
        TA.z = m_SideA_Top.at(l).z   * (1.0-y1) + m_SideB_Top.at(l).z  * y1;
        LB.x = m_SideA_Top.at(l+1).x * (1.0-y2) + m_SideB_Top.at(l+1).x* y2;
        LB.y = m_SideA_Top.at(l+1).y * (1.0-y2) + m_SideB_Top.at(l+1).y* y2;
        LB.z = m_SideA_Top.at(l+1).z * (1.0-y2) + m_SideB_Top.at(l+1).z* y2;
        TB.x = m_SideA_Top.at(l).x   * (1.0-y2) + m_SideB_Top.at(l).x  * y2;
        TB.y = m_SideA_Top.at(l).y   * (1.0-y2) + m_SideB_Top.at(l).y  * y2;
        TB.z = m_SideA_Top.at(l).z   * (1.0-y2) + m_SideB_Top.at(l).z  * y2;
    }
}


bool Surface::hasPanel3(int idx3) const
{
    return std::find(m_Panel3List.begin(), m_Panel3List.end(), idx3) != m_Panel3List.end();
}


bool Surface::hasPanel4(int idx4) const
{
    return std::find(m_Panel4List.begin(), m_Panel4List.end(), idx4) != m_Panel4List.end();
}


void Surface::addPanel3Index(int idx)
{
    if(!hasPanel3(idx)) m_Panel3List.push_back(idx);
}


void Surface::addPanel4Index(int idx)
{
    if(!hasPanel4(idx)) m_Panel4List.push_back(idx);
}


bool Surface::hasFlapPanel3(int idx3) const
{
    return std::find(m_FlapPanel3.begin(), m_FlapPanel3.end(), idx3) != m_FlapPanel3.end();
}


bool Surface::hasFlapPanel3(Panel3 const &p3) const
{
    return hasFlapPanel3(p3.index());
}


void Surface::addFlapPanel3Index(int idx)
{
    if(!hasFlapPanel3(idx)) m_FlapPanel3.push_back(idx);
}


bool Surface::hasFlapPanel4(int idx4)   const
{
    return std::find(m_FlapPanel4.begin(), m_FlapPanel4.end(), idx4) != m_FlapPanel4.end();
}


bool Surface::hasFlapPanel4(const Panel4 &p4) const
{
    return hasFlapPanel4(p4.index());
}


bool Surface::isFlapNode(int nNode)     const
{
    return std::find(m_FlapNode4.begin(), m_FlapNode4.end(), nNode) != m_FlapNode4.end();
}























