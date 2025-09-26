/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#include "quadmesh.h"
#include <xflgeom/geom_globals/geom_global.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>


QuadMesh::QuadMesh() : XflMesh()
{

}


void QuadMesh::meshInfo(QString &info) const
{
    info = QString();
}


void QuadMesh::cleanMesh(QString &logmsg)
{
    logmsg = QString();
}

int QuadMesh::nWakeColumns() const
{
    return 0;
}


void QuadMesh::rotate(double alpha, double beta, double phi)
{
    for(int i=0; i<nPanels(); i++)
    {
        m_Panel4[i].rotate(alpha, beta, phi);
    }
}


void QuadMesh::getMeshInfo(QString &log) const
{
    QString strong;
    double minarea(0), maxarea(0);
    int minareaindex(0), maxareaindex(0);

    strong = QString::asprintf("    Nbr of quads = %d, Nbr of nodes = %d", nPanels(), nNodes());
    log += strong +"\n";
    minarea = 1.e10;
    maxarea = 0.0;
    minareaindex = maxareaindex = -1;
    for(int i4=0; i4<nPanels(); i4++)
    {
        Panel4 const &p4 = panelAt(i4);
        if(p4.area()<minarea)
        {
            minareaindex=i4;
            minarea = p4.area();
        }
        if(p4.area()>maxarea)
        {
            maxareaindex=i4;
            maxarea = p4.area();
        }
    }
    strong = QString::asprintf("    min. panel area = %9.3g ", minarea*Units::m2toUnit());
    strong += Units::areaUnitLabel();
    log += strong;
    strong = QString::asprintf(" for panel %d\n", minareaindex);
    log += strong;
    strong = QString::asprintf("    max. panel area = %9.3g ", maxarea*Units::m2toUnit());
    strong += Units::areaUnitLabel();
    log += strong;
    strong = QString::asprintf(" for panel %d\n", maxareaindex);
    log += strong;
}


void QuadMesh::checkPanels(QString &log, bool bMinAngle, bool bMinArea, bool bWarp,
                           QVector<int> &minanglelist, QVector<int>&minarealist, QVector<int>&warplist,
                           double minangle, double minarea, double maxquadwarp)
{
    QString strong;
    int count = 0;

    if(bMinAngle)
    {
        //check min angles
        count=0;

        for(int i4=0; i4<nPanels(); i4++)
        {
            double minangle = panel(i4).minAngle();
            if(minangle<minangle)
            {
                minanglelist.push_back(i4);
                count++;
                strong = QString::asprintf("   Panel %4d has min angle %5.1f",i4, minangle);
                strong += DEGCHAR + "\n";
                log += strong;
            }
        }

        if(!minanglelist.size())
        {
            strong = QString::asprintf("No panel with vertex angle less than %.2f", minangle);
            strong += DEGCHAR + "found\n";
        }
        else
        {
            strong = QString::asprintf("Found %d panels with a vertex angle less than %.2f", count, minangle);
            strong += DEGCHAR;
            log += strong + "\n\n";
        }
    }

    if(bMinArea)
    {
        // chek Min area
        count=0;
        for(int i4=0; i4<nPanels(); i4++)
        {
            Panel4 const &p4 = panel(i4);
            if(fabs(p4.area())<minarea)
            {
                minarealist.push_back(i4);
                count++;
                strong = QString::asprintf("   Panel %4d has area %9.3g ", i4, panel(i4).area()*Units::m2toUnit());
                log += strong + Units::areaUnitLabel() +"\n";
            }
    //                else m_PanelHightlight.insert(i4, false);
        }

        if(!minarealist.size())
        {
            log = "No quad panel with low area found\n";
        }
        else
        {
            strong = QString::asprintf("Found %d panels with area less than %.3g", count, minarea*Units::m2toUnit());
            log += strong + Units::areaUnitLabel() + "\n";
        }
    }

    if(bWarp)
    {
        // check Quad Warp
        count=0;
        for(int i4=0; i4<nPanels(); i4++)
        {
            Panel4 const &p4 = panel(i4);
            double warp = p4.warpAngle();
            if(fabs(warp)>maxquadwarp)
            {
                warplist.push_back(i4);
                count++;
                strong = QString::asprintf("   Panel %4d has warp = %7.2g", i4, warp);
                log += strong + DEGCHAR +"\n";
            }
    //                else m_PanelHightlight.insert(i4, false);
        }
        if(!warplist.size())
        {
            strong = QString::asprintf("No panel with warp greater than %.2f", maxquadwarp);
            strong += DEGCHAR + "found\n";
            log += strong;
        }
        else
        {
            strong = QString::asprintf("Found %d panels with warp greater than %.2f", count, maxquadwarp);
            log += strong + DEGCHAR + "\n";
        }
    }
}


void QuadMesh::makeWakePanels(double nxWakePanels, double wakepanelfactor, double TotalWakeLength,
                              Vector3d const &WindDir, bool bAlignWakeTE)
{
    QVector<Vector3d> WakeNode;
    int nWakeColumns=0;
    makeWakePanels(m_Panel4, nxWakePanels, wakepanelfactor, TotalWakeLength, WindDir,
                   m_WakePanel4, WakeNode, nWakeColumns, bAlignWakeTE);
}


int QuadMesh::makeWakePanels(QVector<Panel4> &Panel4List,
                             double nxWakePanels, double wakepanelfactor, double TotalWakeLength,
                             Vector3d const &WindDir,
                             QVector<Panel4>&WakePanel4, QVector<Vector3d> &WakeNode, int &nWakeColumn, bool bAlignWakeTE)
{
    WakePanel4.clear();
    nWakeColumn = 0;

    double series=0.0, ratio=1.0;
    for(int p=0; p<nxWakePanels; p++)
    {
        series += ratio;
        ratio *=wakepanelfactor;
    }

    Vector3d Tl, Tr, Tl1, Tr1;
    int mw=0;
    double l0l=0.0, l0r=0.0;
    for(int i4=0; i4<Panel4List.size(); i4++)
    {
        Panel4 &p4 = Panel4List[i4];

        if(!p4.isFusePanel() && p4.isTrailing()) // bodies don't shed & wake
        {
            if(p4.isBotPanel() || p4.isMidPanel())
            {
                p4.setWakeColumn(nWakeColumn);

                p4.setiWake(mw);
                //start at the trailing edge
                Tl = p4.leftTrailingNode();
                Tr = p4.rightTrailingNode();

                // step (i)
                if(bAlignWakeTE)
                {
                    l0l = (TotalWakeLength-Tl.x)/series;
                    l0r = (TotalWakeLength-Tr.x)/series;
                }
                else
                {
                    l0l = TotalWakeLength/series;
                    l0r = TotalWakeLength/series;
                }

                // step (ii)
                for(int nx=0; nx<nxWakePanels; nx++)
                {
                    Tl1 = Tl + WindDir * l0l;
                    Tr1 = Tr + WindDir * l0r;
                    l0l *= wakepanelfactor;
                    l0r *= wakepanelfactor;

                    WakePanel4.push_back({});
                    Panel4 &p4w = WakePanel4.back();

                    // step (iii)
                    Vector3d Bl, Br, Bl1, Br1;
                    if(p4.isBotPanel())
                    {
                        Bl  = Tl;  // Bl.rotateZ(Origin, -beta);
                        Br  = Tr;  // Br.rotateZ(Origin, -beta);
                        Bl1 = Tl1; // Bl1.rotateZ(Origin, -beta);
                        Br1 = Tr1; // Br1.rotateZ(Origin, -beta);
                    }
                    else // if (p4.isMidPanel())
                    {
                        Bl  = Tr;  // Bl.rotateZ(Origin, -beta);
                        Br  = Tl;  // Br.rotateZ(Origin, -beta);
                        Bl1 = Tr1; // Bl1.rotateZ(Origin, -beta);
                        Br1 = Tl1; // Br1.rotateZ(Origin, -beta);
                    }
                    p4w.setPanelFrame(Bl, Br, Bl1, Br1);
                    p4w.setIndex(mw);
                    p4w.setLeftWingPanel(true);
                    p4w.setAsWakePanel();
                    p4w.m_iPU = nx>0              ? mw-1 : -1;
                    p4w.m_iPD = nx<nxWakePanels-1 ? mw+1 : -1;

                    Tl = Tl1;
                    Tr = Tr1;
                    mw++;
                }
                nWakeColumn++;
            }
        }
    }

    //connect the top trailing panels to the wake columns

    int mwBot=0;
    int iWakeColumn=0;
    for(int i4=0; i4<Panel4List.size(); i4++)
    {
        Panel4 &p4 = Panel4List[i4];
        if(p4.isTrailing())
        {
            if(p4.isBotPanel())
            {
                mwBot = p4.iWake();
                iWakeColumn = p4.iWakeColumn();
            }
            else if(p4.isTopPanel())
            {
                p4.setiWake(mwBot); //same wake index as last trailing bot panel
                p4.setWakeColumn(iWakeColumn);
            }
        }
    }

    //make wake node array
    WakeNode.clear();
    int idx(0);
    for(int i4w=0; i4w<WakePanel4.size(); i4w++)
    {
        Panel4 *p4w = &WakePanel4[i4w];
//        idx = WakeNode.indexOf(p4w->m_Node[0]);
        idx = isVector3d(WakeNode, p4w->m_Node[0]);
        if(idx<0)
        {
            p4w->m_iLA = WakeNode.size();
            WakeNode.append(p4w->m_Node[0]);
        }
        else p4w->m_iLA = idx;

 //       idx = WakeNode.indexOf(p4w->m_Node[1]);
        idx = isVector3d(WakeNode, p4w->m_Node[1]);
        if(idx<0)
        {
            p4w->m_iTA = WakeNode.size();
            WakeNode.append(p4w->m_Node[1]);
        }
        else p4w->m_iTA = idx;

 //       idx = WakeNode.indexOf(p4w->m_Node[2]);
        idx = isVector3d(WakeNode, p4w->m_Node[2]);
        if(idx<0)
        {
            p4w->m_iTB = WakeNode.size();
            WakeNode.append(p4w->m_Node[2]);
        }
        else p4w->m_iTB = idx;

//        idx = WakeNode.indexOf(p4w->m_Node[3]);
        idx = isVector3d(WakeNode, p4w->m_Node[3]);
        if(idx<0)
        {
            p4w->m_iLB = WakeNode.size();
            WakeNode.append(p4w->m_Node[3]);
        }
        else p4w->m_iLB = idx;
    }

    Q_ASSERT(mw==WakePanel4.size());

    return mw;
}


void QuadMesh::getLastTrailingPoint(Vector3d &pt) const
{
    pt.set(-100,0,0);
    for(int i4=0; i4<m_Panel4.size(); i4++)
    {
        Panel4 const &p4 = m_Panel4.at(i4);
        if(p4.TA().x>pt.x) pt.set(p4.TA());
        if(p4.TB().x>pt.x) pt.set(p4.TB());
    }
}


/** assumes that the connections have been made */
void QuadMesh::getFreeEdges(QVector<Segment3d> &freeedges, QVector<QPair<int,int>> &pairerrors) const
{
    pairerrors.clear();
    freeedges.clear();
    for(int i4=0; i4<m_Panel4.size(); i4++)
    {
        Panel4 const &p4 = m_Panel4.at(i4);
        if(p4.m_iPU<0)
        {
            if(p4.isBotPanel() || p4.isMidPanel() || p4.isSidePanel())
                freeedges.push_back({p4.LA(), p4.LB()});
            else
                freeedges.push_back({p4.TA(), p4.TB()});
        }
        else
        {
            if(p4.m_iPU>=m_Panel4.size()) pairerrors.append({p4.index(), p4.index()}); // you never know
            else {
                Panel4 const &pneigh = m_Panel4.at(p4.m_iPU);
                if(pneigh.m_iPD!=p4.index()) pairerrors.append({p4.index(), pneigh.index()});
            }
        }

        if(p4.m_iPD<0)
        {
            if(p4.isBotPanel() || p4.isMidPanel() || p4.isSidePanel())
                freeedges.push_back({p4.TA(), p4.TB()});
            else
                freeedges.push_back({p4.LA(), p4.LB()});
        }
        else
        {
            if(p4.m_iPD>=m_Panel4.size()) pairerrors.append({p4.index(), p4.index()}); // don't assume anything
            else
            {
                Panel4 const &pneigh = m_Panel4.at(p4.m_iPD);
                if(pneigh.m_iPU!=p4.index()) pairerrors.append({p4.index(), pneigh.index()});
            }
        }

        if(p4.m_iPL<0)
        {
            freeedges.push_back({p4.LA(), p4.TA()});
        }
        else
        {
            if(p4.m_iPL>=m_Panel4.size()) pairerrors.append({p4.index(), p4.index()}); // don't assume anything
            else
            {
                Panel4 const &pneigh = m_Panel4.at(p4.m_iPL);
                if(pneigh.m_iPR!=p4.index()) pairerrors.append({p4.index(), pneigh.index()});
            }
        }

        if(p4.m_iPR<0)
        {
            freeedges.push_back({p4.LB(), p4.TB()});
        }
        else
        {
            if(p4.m_iPR>=m_Panel4.size()) pairerrors.append({p4.index(), p4.index()}); // don't assume anything
            else
            {
                Panel4 const &pneigh = m_Panel4.at(p4.m_iPR);
                if(pneigh.m_iPL!=p4.index()) pairerrors.append({p4.index(), pneigh.index()});
            }
        }
    }
}


QVector<Vector3d> QuadMesh::ctrlPts(bool bVLM, double offset) const
{
    QVector<Vector3d> points(m_Panel4.size());
    for(int i4=0; i4<m_Panel4.size(); i4++)
    {
        Panel4  const &p4 = m_Panel4.at(i4);
        points[i4].set(p4.ctrlPt(bVLM) + p4.normal()*offset);
    }
    return points;
}




