/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/


#include <QApplication>
#include <QThread>


#ifdef Q_OS_LINUX
    #  include <unistd.h>
#endif
#ifdef Q_OS_WIN
  #include <windows.h>
//  #include <synchapi.h>
  #define sleep(s) Sleep(s)
#endif
#ifdef Q_OS_MAC
    #  include <unistd.h>
#endif


#include "afmesher2d.h"
#include <xflgeom/geom2d/node2d.h>

#include <xflgeom/geom_globals/geom_global.h>


int AFMesher2d::s_MaxIterations = -1;

bool AFMesher2d::s_bCancel = false;
double AFMesher2d::s_SearchRadiusFactor = 0.7; // best by test (Fischer)
double AFMesher2d::s_GrowthFactor = 1.0;
double AFMesher2d::s_MaxEdgeLength = 0.25;
int AFMesher2d::s_MaxPanelCount = 1000;
bool AFMesher2d::s_bMultiThread = false;

float AFMesher2d::s_NodeMergeDistance(0.0001f);


AFMesher2d::AFMesher2d() : QObject()
{
    m_pParent = nullptr;
    m_bError = false;
}


void AFMesher2d::onMakeTriangles()
{
    QString strange;
    if(!m_PSLG.size())
    {
        postMessageEvent("PSLG has not been defined - aborting\n\n");
        emit meshFinished();
        thread()->exit(0); // exit event loop so that finished() is emitted
        return;
    }
    m_bError = !makeTriangles(strange);

    postMessageEvent(strange);

    emit meshFinished();
    thread()->exit(0); // exit event loop so that finished() is emitted
}


bool AFMesher2d::makeTriangles(QString &logmsg)
{   
    m_Triangles.clear();
    PSLG2d &front = m_PSLG;
    int seg_index = front.last().index()+1;

    QVector<Node2d> frontnodes;
    front.makeNodes(frontnodes);
    Triangle2d triangle;

    int iseg=0;
    int iter=0;
    if(s_MaxIterations==0)
    {
        m_PSLG = front;
        return true;
    }

    while(front.size()>0)
    {
        iter++;

        if(s_MaxIterations>0 && iter>=s_MaxIterations)
        {
            int nada=0; (void)nada;
        }

        // process the segment
        Segment2d baseseg = front.at(iseg); // don't take a reference from a changing array

        // find the previous adjacent segment which makes the minimal angle with this segment
        int iprevious = -1;
        double theta_prev = 2.0*PI;
        front.previous(iseg, theta_prev, iprevious);

        // find the next adjacent segment which makes the minimal angle with this segment
        int inext = -1;
        double theta_next = 2.0*PI;
        front.next(iseg, theta_next, inext);


        if(iprevious>=0 && inext>=0)
        {
            // address the case where {previous.seg.next} form a closed triangle
            // this eliminates some special cases at the outset
            Segment2d previous = front.at(iprevious);  // don't take a reference on a changing array
            Segment2d next     = front.at(inext);      // don't take a reference on a changing array

            if(previous.vertexAt(0).isSame(next.vertexAt(1), 1.e-6))
            {
                triangle.setTriangle(baseseg.vertexAt(0), baseseg.vertexAt(1), next.vertexAt(1));
                triangle.setIndex(m_Triangles.size());
                m_Triangles.push_back(triangle);
                //remove the three segments
                front.removeSegments(next);
                front.removeSegments(previous);
                front.removeSegments(baseseg);
                // move on
                iseg++;
                if(front.size()) iseg = iseg%front.size();

                if(s_MaxIterations>=0 && iter>=s_MaxIterations) break;

                continue;
            }
        }

        // To avoid a flat panel at the nose, don't build the first triangle using the last segment.

        bool bCheckCloseNodes = true;

        // If the angle with the adjacent segment is less than 90°
        if(theta_prev<PI/2 && theta_next<PI/2)
        {
            // join the closest node
            Segment2d previous = front.at(iprevious);  // don't take a reference on a changing array
            Segment2d next     = front.at(inext);      // don't take a reference on a changing array
            if(previous.length()<next.length())
            {
                triangle.setTriangle(baseseg.vertexAt(0), baseseg.vertexAt(1), previous.vertexAt(0));
            }
            else
            {
                triangle.setTriangle(baseseg.vertexAt(0), baseseg.vertexAt(1), next.vertexAt(1));
            }
            bCheckCloseNodes = false;
        }
        else if(theta_prev<PI/2 && theta_prev<theta_next)
        {
            // use the previous segment to build the triangle
            Segment2d previous = front.at(iprevious);  // don't take a reference on a changing array
            triangle.setTriangle(baseseg.vertexAt(0), baseseg.vertexAt(1), previous.vertexAt(0));
            bCheckCloseNodes = false;
        }
        //  next segment can be used if the angle is less than PI/2
        else if(theta_next<PI/2)
        {
            // use the next segment to build the triangle
            Segment2d next     = front.at(inext);      // don't take a reference on a changing array
            triangle.setTriangle(baseseg.vertexAt(0), baseseg.vertexAt(1), next.vertexAt(1));
            bCheckCloseNodes = false;
        }
        else
        {
            // no success with the previous and next segments
            // make the ideal triangle
            if(!makeEquiTriangle(baseseg, s_MaxEdgeLength, s_GrowthFactor, m_Nodes.size(), triangle))
            {
                //face is unmeshable
                QString strange;
                strange = QString::asprintf("***Unconverged: Could not build triangle at iteration %d\n", iter);
                logmsg += strange;
                m_PSLG = front;
                return false;
            }
            m_Nodes.append(triangle.vertexAt(2));
            bCheckCloseNodes = true; // there may be some more suitable nodes nearby
        }

        // make the new segments
        Segment2d newsegs[] = {{triangle.vertexAt(0), triangle.vertexAt(2)}, {triangle.vertexAt(2), triangle.vertexAt(1)}};

        // Before adding the triangle,
        //   - check that there are no internal nodes
        //   - check that the new segments do not intersect existing ones
        //   - check if there are available nodes within the search radius

        QVector<Node2d> insidenodes, closenodes;
        front.nodesInTriangle(triangle, insidenodes);

        if(bCheckCloseNodes)
        {
            // the search radius shouldn't be too large to maintain a smooth progression
            // of triangle sizes, and not too small to avoid filling gaps with small triangles
            double searchradius = qMin(s_MaxEdgeLength, baseseg.length() * s_SearchRadiusFactor);
            front.nodesAroundCenter(triangle.vertexAt(2), searchradius, closenodes);
        }

        QVector<int> intersected;
        QVector<Vector2d> I;
        front.intersect(newsegs[0], intersected, I, false, s_NodeMergeDistance);
        front.intersect(newsegs[1], intersected, I, false, s_NodeMergeDistance);
//        front.intersect(newsegs[0], intersected, I, 0.005);
//        front.intersect(newsegs[1], intersected, I, 0.005);

        // add the vertices of the intersected segments to the close nodes
        for(int i=0; i<intersected.size(); i++)
        {
            Segment2d const &seg = front.at(intersected.at(i));
            // the vertices may already be in the close nodes
            bool b0Exists(false), b1Exists(false);
            for(int ic=0; ic<closenodes.size(); ic++)
            {
                if(seg.vertexAt(0).isSame(closenodes.at(ic), s_NodeMergeDistance)) b0Exists = true;
                if(seg.vertexAt(1).isSame(closenodes.at(ic), s_NodeMergeDistance)) b1Exists = true;
            }
            if(!b0Exists) closenodes.append(seg.vertexAt(0));
            if(!b1Exists) closenodes.append(seg.vertexAt(1));
        }

        // link preferably to the inside nodes
        if(insidenodes.size())
        {
            // pick the one which minimizes the distances to the base segment's vertices?
            double dmax = 1e10;
            for(int in=0; in<insidenodes.size(); in++)
            {
                Node2d const &nd = insidenodes.at(in);
                double d0 = nd.distanceTo(baseseg.vertexAt(0));
                double d1 = nd.distanceTo(baseseg.vertexAt(1));
                // rule out the base segment's nodes
                if(d0<1.e-6 || d1<1.e-6) continue;

                double dist = d0 + d1;
                if(dist<dmax)
                {
                    triangle = {baseseg.vertexAt(0), baseseg.vertexAt(1), nd};
                    dmax = dist;
                }
            }
        }
        else if(closenodes.size())
        {
            Triangle2d t3d = triangle;
            double dmax = 1.e10;
            for(int in=0; in<closenodes.size(); in++)
            {
                Node2d const &nd = closenodes.at(in);
                if(nd.isSame(baseseg.vertexAt(0), s_NodeMergeDistance) || nd.isSame(baseseg.vertexAt(1), s_NodeMergeDistance))
                    continue;

                double d0 = nd.distanceTo(baseseg.vertexAt(0));
                double d1 = nd.distanceTo(baseseg.vertexAt(1));
                // reject the base segment's nodes
                if(d0<s_NodeMergeDistance || d1<s_NodeMergeDistance) continue;

                // reject if intersecting adjacent segments
                Segment2d seg1(baseseg.vertexAt(0), nd);
                Segment2d seg2(baseseg.vertexAt(1), nd);
                Vector2d In;
                if(iprevious>=0)
                {
                    Segment2d const &previous = front.at(iprevious);
                    if(seg2.intersects(previous, In, false, s_NodeMergeDistance)) continue;
                }
                if(inext>=0)
                {
                    Segment2d const &next = front.at(inext);
                    if(seg1.intersects(next, In, false, s_NodeMergeDistance)) continue;
                }

                Triangle2d t3d_test = {baseseg.vertexAt(0), baseseg.vertexAt(1), nd};
                double dist = d0 + d1;
                // reject inverted
                if(dist<dmax /* && baseseg.averageNormal().dot(t3d_test.normal())>0 */)
                {
                    t3d = t3d_test;
                    dmax = dist;
                }
            }

            // It can occur that new nodes have been included in the modified triangle,
            // so check again
            insidenodes.clear();
            front.nodesInTriangle(t3d, insidenodes);

            if(insidenodes.size())
            {
                // pick the one which minimizes the distances to the base segment's vertices?
                double dmax = 1e10;
                for(int in=0; in<insidenodes.size(); in++)
                {
                    Node2d const &nd = insidenodes.at(in);
                    double d0 = nd.distanceTo(baseseg.vertexAt(0));
                    double d1 = nd.distanceTo(baseseg.vertexAt(1));
                    // rule out the base segment's nodes
                    if(d0>1.e-6 && d1>1.e-6)
                    {
                        double dist = d0 + d1;
                        if(dist<dmax)
                        {
                            t3d = {baseseg.vertexAt(0), baseseg.vertexAt(1), nd};
                            dmax = dist;
                        }
                    }
                }
            }
            // if everything has been checked, keep this triangle
            triangle = t3d;
        }

        if(!triangle.isNull())
        {
            triangle.setIndex(m_Triangles.size());
            m_Triangles.push_back({triangle});
        }

        // update segments
        newsegs[0] = {triangle.vertexAt(0), triangle.vertexAt(2)};
        newsegs[1] = {triangle.vertexAt(2), triangle.vertexAt(1)};

        // the new triangle has been inserted
        // remove the base segment
        front.removeAt(iseg); // same as triangle.edge(0)

        // check if the other two edges need to be added if non existent,
        // or removed if the front has merged

        for(int is=0; is<2; is++)
        {
            int iExists(-1);
            front.isSegment(newsegs[is], iExists,1.e-6);
            if(iExists>=0)
            {
                // the front has merged, delete the existing segment, and don't add the new one
                front.removeAt(iExists);
                if(iExists<iseg) iseg--;
            }
            else
            {
                newsegs[is].setIndex(seg_index++);
                front.insert(iseg, newsegs[is]);
                iseg++;
            }
        }

        if(front.size()) iseg = iseg%front.size();
        if(m_Triangles.size()>s_MaxPanelCount) break;

        if(s_MaxIterations>=0 && iter>=s_MaxIterations) break;
        if(s_bCancel) break;
    }


    logmsg.clear();
    if(m_Triangles.size()>=s_MaxPanelCount)
    {
        logmsg = "   The maximum number of triangles has been reached\n";
    }

/*    if(front.size()==0)
    {
        QString strange;
        strange = QString::asprintf("   Made %d triangles in %d iterations\n", facetriangles.size(), iter);
        logmsg += strange;
    }*/
    if(front.size()!=0)
    {
        QString strange;
        strange = QString::asprintf("   ***Unconverged: made %d triangles in %d iterations***\n"
                                    "                   remains %d unprocessed segments\n",
                                    int(m_Triangles.size()), iter, int(front.size()));
        logmsg += strange;
    }

    m_PSLG = front;
    return true;
}


void AFMesher2d::postMessageEvent(QString const & msg) const
{
    MessageEvent * pMsgEvent = new MessageEvent(msg);
    qApp->postEvent(m_pParent, pMsgEvent);

}


bool AFMesher2d::makeEquiTriangle(Segment2d const &baseseg, double maxedgelength, double growthfactor, int nodeindex,
                                  Triangle2d &triangle)
{
    if(baseseg.length()<LENGTHPRECISION) return false;

    Node2d S = baseseg.CoG();
    Vector2d U = baseseg.unitDir();
    double h = qMin(maxedgelength, baseseg.length()*growthfactor);
    Vector2d Height(-U.y*h, U.x*h);
    S += Height;
    S.setIndex(nodeindex);
    triangle.setTriangle(baseseg.vertexAt(0), baseseg.vertexAt(1), S);
    return true;
}
