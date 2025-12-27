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


#include <nurbssurface.h>

#include <constants.h>
#include <frame.h>
#include <geom_global.h>
#include <node.h>
#include <quad3d.h>
#include <triangle3d.h>



/**
 * The public constructor.
 * @param iAxis defines direction of the u parameter; v is in the y-direction
 */
NURBSSurface::NURBSSurface(int uAxis, int vAxis)
{
    m_Color.setRgb(255,0,0);

    m_Frame.clear();
    m_Frame.reserve(10);

    m_uAxis = uAxis; //directed in x direction, mainly
    m_vAxis = vAxis;     //directed in z direction, mainly

    m_iuDegree = 2;
    m_ivDegree = 2;

    m_iOutput = 31;

    m_BunchAmp = 0.0;
    m_BunchDist = 0.5;

    m_EdgeWeightu = 1.0;
    m_EdgeWeightv = 1.0;

    m_iActiveFrame = -1;
}


/** the copy constructor - make a deep copy of the frames */
NURBSSurface::NURBSSurface(NURBSSurface const &nurbs)
{
    copy(nurbs);
}


void NURBSSurface::copy(NURBSSurface const & aSurface)
{
    m_iuDegree    = aSurface.m_iuDegree;
    m_ivDegree    = aSurface.m_ivDegree;
    m_iOutput     = aSurface.m_iOutput;
    m_BunchAmp    = aSurface.m_BunchAmp;
    m_BunchDist   = aSurface.m_BunchDist;
    m_EdgeWeightu = aSurface.m_EdgeWeightu;
    m_EdgeWeightv = aSurface.m_EdgeWeightv;

    m_uAxis = aSurface.m_uAxis;
    m_vAxis = aSurface.m_vAxis;

    m_Color = aSurface.m_Color;

    clearFrames();
    for(int i=0; i<aSurface.frameCount(); i++)
    {
        m_Frame.push_back(Frame());
        m_Frame[i] = aSurface.m_Frame[i];
    }
    setKnots();
}


NURBSSurface::~NURBSSurface()
{
}


/**
 * Returns the u-parameter for a given position along the u-axis and a given v parameter
 * Proceeds by iteration - time consuming,
 * @param pos the point coordinate for which the parameter u is requested
 * @param v the specified value of the v-parameter
 * @return the value of the u-parameter
 */
double NURBSSurface::getu(double pos, double v) const
{
    if(m_Frame.size()==0) return 0.0;

    if(pos<=m_Frame.front().position().dir(m_uAxis))
        return 0.0;
    if(pos>=m_Frame.back().position().dir(m_uAxis))
        return 1.0;
    if(fabs(m_Frame.back().position().dir(m_uAxis) - m_Frame.front().position().dir(m_uAxis))<0.0000001)
        return 0.0;

    int iter=0;

    double u1 = 0.0, u2 = 1.0;

//    v = 0.0;//use top line, but doesn't matter
    while(fabs(u2-u1)>1.0e-6 && iter<100)
    {
        double u=(u1+u2)/2.0;
        double zz = 0.0;
        for(int iu=0; iu<frameCount(); iu++) //browse all points
        {
            double zh = 0.0;
            for(int jv=0; jv<framePointCount(); jv++)
            {
                double c =  geom::basis(jv, m_ivDegree, v, m_vKnot.data());
                zh += m_Frame[iu].position().dir(m_uAxis) * c;
            }
            double b = geom::basis(iu, m_iuDegree, u, m_uKnot.data());
            zz += zh * b;
        }
        if(zz>pos) u2 = u;
        else       u1 = u;
        iter++;
    }
    return (u1+u2)/2.0;
}


/**
 * Returns the v-parameter for a given position along the v-axis and a given u parameter
 * Proceeds by iteration - time consuming,
 * @param pos the point coordinate for which the parameter u is requested
 * @param v the specified value of the v-parameter
 * @return the value of the u-parameter
 */
double NURBSSurface::getv(double pos, double u) const
{
    if(m_Frame.size()==0) return 0.0;

    if(pos<=m_Frame.front().position().dir(m_vAxis))
        return 0.0;
    if(pos>=m_Frame.back().position().dir(m_vAxis))
        return 1.0;
    if(fabs(m_Frame.back().position().dir(m_vAxis) - m_Frame.front().position().dir(m_vAxis))<0.0000001)
        return 0.0;

    int iter=0;
    double  v=0;
    double v1=0.0, v2=1.0;

    Vector3d pt;
//    v = 0.0;//use top line, but doesn't matter
    while(fabs(v2-v1)>1.0e-6 && iter<100)
    {
        v=(v1+v2)/2.0;
        getPoint(u,v, pt);
        if(fabs(pt.dir(m_vAxis)-pos)<0.0001) return v;
        if(pt.dir(m_vAxis)>pos) v2 = v;
        else                    v1 = v;
        iter++;
    }
    return (v1+v2)/2.0;
}


/**
 * Returns the v-parameter for a given value  of u and a geometrical point
 * Proceeds by iteration - time consuming,
 * @param u the specified value of the u-parameter
 * @param r the point for which v is requested
 * @return the value of the v-parameter
 */

double NURBSSurface::getvIntersect(double u, Vector3d r) const
{
    Vector3d t_R;
    double sine = 10000.0;

    if(u<-PRECISION)    return 0.0;
    if(u>1.0+PRECISION) return 0.0;
    if(r.norm()<1.0e-5) return 0.0;

    int iter=0;
    double v, v1, v2;

    r.normalize();
    v1 = 0.0; v2 = 1.0;

    while(fabs(sine)>1.0e-4 && iter<100)
    {
        v=(v1+v2)/2.0;
        getPoint(u, v, t_R);
        t_R.x = 0.0;
        t_R.normalize();//t_R is the unit radial vector for u,v

        sine = (r.y*t_R.z - r.z*t_R.y);

        if(sine>0.0) v1 = v;
        else         v2 = v;
        iter++;
    }

    return (v1+v2)/2.0;
}



/**
 * Returns the point corresponding to the pair of parameters (u,v)
 * Assumes that the knots have been set previously
 *
 * Scans the u-direction first, then v-direction
 * @param u the specified u-parameter
 * @param v the specified v-parameter
 * @param Pt a reference to the point defined by the pair (u,v)
*/
void NURBSSurface::getPoint(double u, double v, Vector3d &Pt) const
{
    Vector3d V, Vv, rpt;
    double wx=0, totalweight=0, cs=0, bs=0;
    if(u<0.0)  u=0.0;
    if(v<0.0)  v=0.0;
    if(u>=1.0) u=0.99999999999;
    if(v>=1.0) v=0.99999999999;

    totalweight = 0.0;
    for(int iu=0; iu<frameCount(); iu++)
    {
        Frame const &uframe = m_Frame.at(iu);
        Vv.set(0.0,0.0,0.0);
        wx = 0.0;
        for(int jv=0; jv<framePointCount(); jv++)
        {
            double w = weight(m_EdgeWeightv, jv, framePointCount());
            cs = geom::basis(jv, m_ivDegree, v, m_vKnot.data()) * w;

            rpt.x = uframe.ctrlPointAt(jv).x;
            rpt.y = uframe.ctrlPointAt(jv).y;
            rpt.z = uframe.ctrlPointAt(jv).z;
            if(fabs(uframe.angle())>ANGLEPRECISION) //degrees
                rpt.rotateY(uframe.position(), uframe.angle());

            Vv.x += rpt.x * cs;
            Vv.y += rpt.y * cs;
            Vv.z += rpt.z * cs;

            wx += cs;
        }
        double w = weight(m_EdgeWeightu, iu, frameCount());
        bs = geom::basis(iu, m_iuDegree, u, m_uKnot.data()) *w;

        V.x += Vv.x * bs;
        V.y += Vv.y * bs;
        V.z += Vv.z * bs;

        totalweight += wx * bs;
    }

    Pt.x = V.x / totalweight;
    Pt.y = V.y / totalweight;
    Pt.z = V.z / totalweight;
}


void NURBSSurface::getNormal(double u, double v, Vector3d &N) const
{
    Vector3d Su, Sv, Vv, rpt;
    double cs=0, bs=0;

    u=std::max(u, 1e-4);
    v=std::max(v, 1e-4);
    u=std::min(u, 1.0-1.e-4);
    v=std::min(v, 1.0-1.e-4);

    // calculate the u derivative
    for(int iu=0; iu<frameCount(); iu++)
    {
        Frame const &uframe = m_Frame.at(iu);
        Vv.reset();

        for(int jv=0; jv<framePointCount(); jv++)
        {
             cs = geom::basis(jv, m_ivDegree, v, m_vKnot.data());

            rpt.x = uframe.ctrlPointAt(jv).x;
            rpt.y = uframe.ctrlPointAt(jv).y;
            rpt.z = uframe.ctrlPointAt(jv).z;
            if(fabs(uframe.angle())>0.01) //degrees
            {
                 rpt.rotateY(uframe.position(), uframe.angle());
            }

            Vv.x += rpt.x * cs;
            Vv.y += rpt.y * cs;
            Vv.z += rpt.z * cs;

        }

        bs = geom::basisDerivative(iu, m_iuDegree, u, m_uKnot.data());

        Su.x += Vv.x * bs;
        Su.y += Vv.y * bs;
        Su.z += Vv.z * bs;
    }

    // calculate the u derivative
    for(int iu=0; iu<frameCount(); iu++)
    {
        Frame const &uframe = m_Frame.at(iu);
        Vv.reset();

        for(int jv=0; jv<framePointCount(); jv++)
        {
             cs = geom::basisDerivative(jv, m_ivDegree, v, m_vKnot.data());

            rpt.x = uframe.ctrlPointAt(jv).x;
            rpt.y = uframe.ctrlPointAt(jv).y;
            rpt.z = uframe.ctrlPointAt(jv).z;
            if(fabs(uframe.angle())>0.01) //degrees
            {
                 rpt.rotateY(uframe.position(), uframe.angle());
            }

            Vv.x += rpt.x * cs;
            Vv.y += rpt.y * cs;
            Vv.z += rpt.z * cs;

        }

        bs = geom::basis(iu, m_iuDegree, u, m_uKnot.data());

        Sv.x += Vv.x * bs;
        Sv.y += Vv.y * bs;
        Sv.z += Vv.z * bs;
    }

    N = (Su * Sv).normalized();

}


/**
 * Returns the weight of the control point
 * @param i the index of the point along the edge
 * @param N the total number of points along the edge
 * @return the point's weight
 **/
double NURBSSurface::weight(double const &d, const int &i, int const &N) const
{
    if(fabs(d-1.0)<PRECISION) return 1.0;
    if(i<(N+1)/2)             return pow(d, i);
    else                      return pow(d, N-i-1);
}


/*
double NURBSSurface::Weight(int i, int N)
{
    // returns the weight of the control point
    // i is the index of the point along the edge
    // N is total number of points along the edge

    if(fabs(m_EdgeWeight-1.0)<PRECISION) return 1.0;
    if(i<N/2)                            return 1./pow(m_EdgeWeight, (int)((N-1)/2-i));
    else                                 return 1./pow(m_EdgeWeight, i-(int)(N/2));
}*/



Vector3d const &NURBSSurface::ctrlPoint(int iframe, int ipt) const
{
    return m_Frame.at(iframe).ctrlPointAt(ipt);
}


/**
 * Creates the knot array for the two directions
 * Assumes consistency of degrees and point/frame counts
 */
void NURBSSurface::setKnots()
{
    if(!frameCount())      return;
    if(!framePointCount()) return;

    // assumes consistency of degrees and point/frame counts!

    int nuKnots  = m_iuDegree + frameCount() + 1;
    m_uKnot.resize(nuKnots);

    double b = double(nuKnots-2*m_iuDegree-1);

    for (int j=0; j<nuKnots; j++)
    {
        if (j<m_iuDegree+1)  m_uKnot[j] = 0.0;
        else
        {
            if (j<frameCount())
            {
                if(fabs(b)>0.0) m_uKnot[j] = double(j-m_iuDegree)/b;
                else            m_uKnot[j] = 1.0;
            }
            else m_uKnot[j] = 1.0;
        }
    }


    int nvKnots  = m_ivDegree + framePointCount() + 1;

    m_vKnot.resize(nvKnots);
    b = double(nvKnots-2*m_ivDegree-1);

    for (int j=0; j<nvKnots; j++)
    {
        if (j<m_ivDegree+1)  m_vKnot[j] = 0.0;
        else
        {
            if (j<framePointCount())
            {
                if(fabs(b)>0.0) m_vKnot[j] = double(j-m_ivDegree)/b;
                else            m_vKnot[j] = 1.0;
            }
            else m_vKnot[j] = 1.0;
        }
    }
}


/**
 * Removes a Frame from the array
 * @param iFrame the index of the frame to remove
 */
int NURBSSurface::removeFrame(int iFrame)
{
    if(iFrame>=0 && iFrame<int(m_Frame.size()))
    {
        m_Frame.erase(m_Frame.begin()+iFrame);
    }

    m_iActiveFrame = std::min(iFrame, frameCount());

    setKnots();
    return m_iActiveFrame;
}


/**
 * Removes all the Frame objects from the array
 */
void NURBSSurface::clearFrames()
{
    if(!frameCount()) return;
    for(int ifr=frameCount()-1; ifr>=0; ifr--)
    {
        removeFrame(ifr);
    }
}

/**
 * Inserts a Frame in the array. The Frame is positioned in crescending position along the u-axis
 * @param pNewFrame a pointer to the Frame object to insert.
 */
void NURBSSurface::insertFrame(Frame const &pNewFrame)
{
    for(int ifr=0; ifr<frameCount(); ifr++)
    {
        if(pNewFrame.position().dir(m_uAxis) < m_Frame.at(ifr).position().dir(m_uAxis))
        {
                m_Frame.insert(m_Frame.begin()+ifr, pNewFrame);
                return;
        }
    }

    m_Frame.push_back(pNewFrame); //either the first if none, either the last...
}


/**
 * Appends a new Frame at the end of the array
 * @return a pointer to the Frame which has been created.
 */
Frame &NURBSSurface::appendNewFrame()
{
    m_Frame.push_back(Frame(framePointCount()));
    return m_Frame.back();
}


/**
 * returns the common number of control points in the Frames
 * @return
 */
int NURBSSurface::framePointCount() const
{
    if(m_Frame.size())    return m_Frame.front().nCtrlPoints();
    else return 0;
}


/**
 * Returns the approximate developed length of the frame in the hoop direction
 * @return the developed length
 */
double NURBSSurface::hoopLength(double u, int nDiscretization)
{
    double l=0;

    Vector3d pt0, pt1;
    getPoint(u, 0.0, pt0);
    for(int iv=1; iv<nDiscretization; iv++)
    {
        double v = double(iv)/(double(nDiscretization)-1);
        getPoint(u, v, pt1);
        l += (pt1-pt0).norm();
        pt0 = pt1;
    }
    return l;
}


bool NURBSSurface::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    int nIntSpares=0;
    int nDbleSpares=0;
    int n=0;
    double dble=0;

    // 500001 : new fl5 format
    int ArchiveFormat = 500001;
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        m_Color.serialize(ar, true);
        ar << m_iuDegree<<m_ivDegree;
        ar << m_BunchAmp << m_BunchDist;
        ar << m_EdgeWeightu << m_EdgeWeightv;
        ar << m_uAxis << m_vAxis;
        ar << m_iOutput;


        ar << frameCount();
        for(int k=0; k<frameCount(); k++)
        {
            frame(k).serializeFrameXf7(ar, bIsStoring);
        }

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
        if(ArchiveFormat!=500001) return false;
        m_Color.serialize(ar, false);
        ar >> m_iuDegree >> m_ivDegree;
        ar >> m_BunchAmp >> m_BunchDist;
        ar >> m_EdgeWeightu >> m_EdgeWeightv;
        ar >> m_uAxis >> m_vAxis;
        ar >> m_iOutput;


        clearFrames();
        int nFrames=0;
        ar >> nFrames;
        for(int k=0; k<nFrames; k++)
        {
            appendNewFrame();
            Frame &pFrame = m_Frame.back();
            pFrame.serializeFrameXf7(ar, bIsStoring);
            pFrame.setuPosition(m_uAxis);
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}


Vector3d NURBSSurface::leadingEdgeAxis() const
{
    return (m_Frame.back().firstControlPoint() - m_Frame.front().firstControlPoint());
}


void NURBSSurface::makeDefaultNurbs()
{
    m_Color.setRgb(89,136,143);

    for(int ifr=0; ifr<7; ifr++)
    {
        appendNewFrame();
        frame(ifr).clearCtrlPoints();
        for(int is=0; is<5; is++)
        {
            frame(ifr).appendCtrlPoint(Vector3d(0.0,0.0,0.0));
        }
    }

    frame(0).setCtrlPoint(0, -0.243, 0.0, -0.0172);
    frame(0).setCtrlPoint(1, -0.243, 0.0, -0.0172);
    frame(0).setCtrlPoint(2, -0.243, 0.0, -0.0172);
    frame(0).setCtrlPoint(3, -0.243, 0.0, -0.0172);
    frame(0).setCtrlPoint(4, -0.243, 0.0, -0.0172);

    frame(1).setCtrlPoint(0, -0.228, 0.000,  0.005);
    frame(1).setCtrlPoint(1, -0.228, 0.011,  0.004);
    frame(1).setCtrlPoint(2, -0.228, 0.013, -0.018);
    frame(1).setCtrlPoint(3, -0.228, 0.011, -0.030);
    frame(1).setCtrlPoint(4, -0.228, 0.000, -0.031);

    frame(2).setCtrlPoint(0, -0.051, 0.000,  0.033);
    frame(2).setCtrlPoint(1, -0.051, 0.028,  0.036);
    frame(2).setCtrlPoint(2, -0.051, 0.037, -0.003);
    frame(2).setCtrlPoint(3, -0.051, 0.034, -0.045);
    frame(2).setCtrlPoint(4, -0.051, 0.000, -0.049);

    frame(3).setCtrlPoint(0, 0.094, 0.000,  0.025);
    frame(3).setCtrlPoint(1, 0.094, 0.012,  0.019);
    frame(3).setCtrlPoint(2, 0.094, 0.018,  0.001);
    frame(3).setCtrlPoint(3, 0.094, 0.012, -0.017);
    frame(3).setCtrlPoint(4, 0.094, 0.000, -0.023);

    frame(4).setCtrlPoint(0, 0.279, 0.000,  0.007);
    frame(4).setCtrlPoint(1, 0.279, 0.006,  0.008);
    frame(4).setCtrlPoint(2, 0.279, 0.009,  0.000);
    frame(4).setCtrlPoint(3, 0.279, 0.007, -0.006);
    frame(4).setCtrlPoint(4, 0.279, 0.000, -0.005);

    frame(5).setCtrlPoint(0, 0.705, 0.000,  0.0124);
    frame(5).setCtrlPoint(1, 0.705, 0.010,  0.0118);
    frame(5).setCtrlPoint(2, 0.705, 0.012, -0.0015);
    frame(5).setCtrlPoint(3, 0.705, 0.010, -0.0116);
    frame(5).setCtrlPoint(4, 0.705, 0.000, -0.012);

    frame(6).setCtrlPoint(0, 0.719, 0.00,  0.0);
    frame(6).setCtrlPoint(1, 0.719, 0.00,  0.0);
    frame(6).setCtrlPoint(2, 0.719, 0.00,  0.0);
    frame(6).setCtrlPoint(3, 0.719, 0.00, -0.0);
    frame(6).setCtrlPoint(4, 0.719, 0.00, -0.0);

    frame(0).setuPosition(m_uAxis, -0.243);
    frame(1).setuPosition(m_uAxis, -0.228);
    frame(2).setuPosition(m_uAxis, -0.051);
    frame(3).setuPosition(m_uAxis,  0.094);
    frame(4).setuPosition(m_uAxis,  0.279);
    frame(5).setuPosition(m_uAxis,  0.705);
    frame(6).setuPosition(m_uAxis,  0.719);

    setuDegree(3);
    setvDegree(3);
    setKnots();
}


int NURBSSurface::indexof(Frame *pFrame) const
{
    for(uint i=0; i<m_Frame.size(); i++)
    {
        if(&m_Frame.at(i)==pFrame) return i;
    }
    return -1;
}


void NURBSSurface::setFrameCount(int nFrames)
{
    int nfr = frameCount();

    if(nfr==nFrames) return;

    if(nfr<nFrames)
    {
        do
        {
            Frame &pFrame = appendNewFrame();
            pFrame.setZPosition(frameCount()); // whatever
        }
        while(frameCount()<nFrames);
    }
    else
    {
        if(nfr==2) return;
        nFrames = std::max(nFrames, 2);
        do
        {
            removeFrame(1);
        }
        while(frameCount()>nFrames);
    }
    setKnots();
}


void NURBSSurface::setFramePointCount(int nPoints)
{
    int nctrlpts = framePointCount();
    if(nctrlpts==nPoints) return;

    if(nctrlpts<nPoints)
    {
        do
        {
            for(int ifr=0; ifr<frameCount(); ifr++)
            {
                Frame &pFr = frame(ifr);
                Vector3d newpt;
                if(pFr.nCtrlPoints()>0) newpt = pFr.lastControlPoint();
                newpt.x += 1;
                pFr.appendPoint(newpt);
            }
        }
        while(framePointCount()<nPoints);
    }
    else
    {
        if(nctrlpts==2) return;
        nPoints = std::max(nPoints, 2);
        do
        {
            for(int ifr=0; ifr<frameCount(); ifr++)
            {
                frame(ifr).removePoint(1);
            }
        }
        while(framePointCount()>nPoints);
    }
    setKnots();
}


void NURBSSurface::resizeFrames(int nSections, int nPoints)
{
    if(nSections==frameCount() && nPoints==framePointCount()) return;
    if(frameCount()<2) makeDefaultNurbs();// something went wrong,

    Frame &pFr0 = m_Frame.front();
    Frame &pFrl = m_Frame.back();
    pFr0.resizeCtrlPoints(nPoints);
    pFrl.resizeCtrlPoints(nPoints);

    // leave only bot and top Frames
    m_Frame = {m_Frame.front(), m_Frame.back()};

    // rebuild as many Frames as needed between bot and top Frames
    int nNewSecs = nSections-frameCount();
    if(nNewSecs<=0) return;

    for(int ins=0; ins<nNewSecs; ins++)
    {
        double ratio = double(ins+1)/double(nNewSecs+1);
        m_Frame.insert(m_Frame.begin()+ins+1, pFr0);
        Frame &pNS = m_Frame[ins+1];
        pNS.setPosition(pFr0.position()*(1.0-ratio)+pFrl.position()*ratio);
        pNS.setAngle(pFr0.angle()*(1.0-ratio)+pFrl.angle()*ratio);
        for(int ic=0; ic<pFr0.nCtrlPoints(); ic++)
        {
            pNS.setCtrlPoint(ic, pFr0.ctrlPointAt(ic)*(1-ratio) + pFrl.ctrlPointAt(ic)*ratio);
        }
    }
}


void NURBSSurface::insertPoint(int iSel)
{
    if(iSel<0||iSel>framePointCount()) return;

    for(int iFrame=0; iFrame<frameCount(); iFrame++)
    {
        m_Frame[iFrame].insertPoint(iSel);
    }
}


void NURBSSurface::translate(Vector3d const &T)
{
    for(uint i=0; i<m_Frame.size(); i++)
    {
        m_Frame[i].translate(T);
    }
}

/**
 * Intersects a line segment AB with the NURBS surface. The points are expected to be on each side of the NURBS surface.
 *@param A the first point which defines the ray
 *@param B the second point which defines the ray
 *@param I the intersection point
 *@return true if an intersection point could be determined
 */
bool NURBSSurface::intersectNURBS(Vector3d const &Aa, Vector3d const&Bb, double &u, double &v, Vector3d &I) const
{
    Vector3d  tmp, M0, M1, t_Q, t_r, t_N;
    double dist=0, t=0, tp=0;
    int iter = 0;
    int itermax = 20;
    double dmax = 1.0e-5;
    dist = 1000.0;//m

    Vector3d A(Aa), B(Bb);

    M0.set(0.0, A.y, A.z);
    M1.set(0.0, B.y, B.z);

    if(M0.norm()<M1.norm())
    {
        tmp = A;        A = B;          B = tmp;
    }

    //M0 is the outside Point, M1 is the inside point
    M0 = A; M1 = B;


    I = (M0+M1)/2.0; t=0.5;

    while(dist>dmax && iter<itermax)
    {
        //first we get the u parameter corresponding to point I
        tp = t;
        u = getu(I.z, 0.0);  /** @todo I.z or I.x ? */
        t_Q.set(I.x, 0.0, 0.0);
        t_r = (I-t_Q);
        v = getvIntersect(u, t_r);
        getPoint(u, v, t_N);

        //project t_N on M0M1 line
        t = - ( (M0.x - t_N.x) * (M1.x-M0.x) + (M0.y - t_N.y) * (M1.y-M0.y) + (M0.z - t_N.z)*(M1.z-M0.z))
             /( (M1.x -  M0.x) * (M1.x-M0.x) + (M1.y -  M0.y) * (M1.y-M0.y) + (M1.z -  M0.z)*(M1.z-M0.z));

        I.x = M0.x + t * (M1.x-M0.x);
        I.y = M0.y + t * (M1.y-M0.y);
        I.z = M0.z + t * (M1.z-M0.z);

//        dist = sqrt((t_N.x-I.x)*(t_N.x-I.x) + (t_N.y-I.y)*(t_N.y-I.y) + (t_N.z-I.z)*(t_N.z-I.z));
        dist = fabs(t-tp);
        iter++;
    }

    return dist<dmax;
}

/**
 * @brief NURBSSurface::intersect
 * find the intesection of the ray [A,B] withthe NURBS by successive intersections with quads
 * @return
 */
bool NURBSSurface::intersect_ref(Vector3d const&A, Vector3d const&B, double &u, double &v, Vector3d &I) const
{
    double umin=0, umax=1.0;
    double vmin=0, vmax=1.0;
    Vector3d N;
    double pcrit = 1.e-4;

    int iter =0;
    int itermax=30;
    Quad3d quad[4];
    Vector3d Vtx[9];
    do
    {
        for(int i=0; i<3; i++)
        {
            double di = double(i)/2.0;
            for(int j=0; j<3; j++)
            {
                double dj = double(j)/2.0;
                getPoint((1.0-di)*umin+di*umax, (1.0-dj)*vmin+dj*vmax, Vtx[3*i+j]);
            }
        }

        quad[0].setQuad(Vtx[0], Vtx[1], Vtx[4], Vtx[3]);
        quad[1].setQuad(Vtx[1], Vtx[2], Vtx[5], Vtx[4]);
        quad[2].setQuad(Vtx[3], Vtx[4], Vtx[7], Vtx[6]);
        quad[3].setQuad(Vtx[4], Vtx[5], Vtx[8], Vtx[7]);

        bool bIntersect=false;
        if(quad[0].intersectQuadTriangles(A, B, I))
        {
            umax = umin + (umax-umin)/2.0;
            vmax = vmin + (vmax-vmin)/2.0;
            bIntersect = true;
        }
        else if(quad[1].intersectQuadTriangles(A, B, I))
        {
            umax = umin + (umax-umin)/2.0;
            vmin = vmin + (vmax-vmin)/2.0;
            bIntersect = true;
        }
        else if(quad[2].intersectQuadTriangles(A, B, I))
        {
            umin = umin + (umax-umin)/2.0;
            vmax = vmin + (vmax-vmin)/2.0;
            bIntersect = true;
        }
        else if(quad[3].intersectQuadTriangles(A, B, I))
        {
            umin = umin + (umax-umin)/2.0;
            vmin = vmin + (vmax-vmin)/2.0;
            bIntersect = true;
        }
        if(!bIntersect)
            return false;

        iter++;
        if(iter>=itermax)
            return false;
    }
    while (fabs(umax-umin)>pcrit && fabs(vmax-vmin)>pcrit);

    u = (umin+umax)/2.0;
    v = (vmin+vmax)/2.0;

    return true;
}


#define NDIV 5 // fastest
#define NVTX (NDIV+1)
/**
 * @brief NURBSSurface::intersect
 * find the intesection of the ray [A,B] with the NURBS by successive intersections with triangles
 * @return
 */
bool NURBSSurface::intersect(Vector3d const&A, Vector3d const&B, double &u, double &v, Vector3d &I) const
{
//    auto t0 = std::chrono::high_resolution_clock::now();

    double umin=0, umax=1.0;
    double vmin=0, vmax=1.0;
    double umin1=0, umax1=1.0;
    double vmin1=0, vmax1=1.0;
    Vector3d N;
    double pcrit = 1.e-2;

    int iter =0;
    int itermax=30;


    Triangle3d t3d;
    Vector3d Vtx[NVTX*NVTX];
    do
    {
        for(int i=0; i<NVTX; i++)
        {
            double di = double(i)/double(NVTX-1);
            for(int j=0; j<NVTX; j++)
            {
                double dj = double(j)/double(NVTX-1);
                getPoint((1.0-di)*umin+di*umax, (1.0-dj)*vmin+dj*vmax, Vtx[NVTX*i+j]);
            }
        }

        bool bIntersect = false;
        for(int k=0; k<NDIV*NDIV; k++)
        {
            int i = int(double(k)/double(NDIV));
            int j = k-i*NDIV;

            t3d.setTriangle(Vtx[i*NVTX+j], Vtx[i*NVTX+j+1], Vtx[(i+1)*NVTX+j+1]);

            if(t3d.intersectSegmentInside(A, B, I, true))
            {
                double di  = double(i)/double(NVTX-1);
                double dj  = double(j)/double(NVTX-1);
                double di1 = double(i+1)/double(NVTX-1);
                double dj1 = double(j+1)/double(NVTX-1);
                umin1 = (1.0-di) *umin + di *umax;
                umax1 = (1.0-di1)*umin + di1*umax;
                vmin1 = (1.0-dj) *vmin + dj *vmax;
                vmax1 = (1.0-dj1)*vmin + dj1*vmax;

                umin=umin1;      umax=umax1;
                vmin=vmin1;      vmax=vmax1;
                bIntersect = true;
                break;
            }
            else
            {
                t3d.setTriangle(Vtx[i*NVTX+j], Vtx[(i+1)*NVTX+j+1], Vtx[(i+1)*NVTX+j]);

                if(t3d.intersectSegmentInside(A, B, I, true))
                {
                    double di  = double(i)  /double(NVTX-1);
                    double dj  = double(j)  /double(NVTX-1);
                    double di1 = double(i+1)/double(NVTX-1);
                    double dj1 = double(j+1)/double(NVTX-1);
                    umin1 = (1.0-di) *umin + di *umax;
                    umax1 = (1.0-di1)*umin + di1*umax;
                    vmin1 = (1.0-dj) *vmin + dj *vmax;
                    vmax1 = (1.0-dj1)*vmin + dj1*vmax;

//                  extend the next triangles to avoid any boundary precision issue
                    umin=umin1*0.99;      umax=umax1*1.01; // extend the next triangles to avoid any boundary precision issue
                    vmin=vmin1*0.99;      vmax=vmax1*1.01;

                    bIntersect = true;
                    break;
                }
            }
        }

        if(!bIntersect)
            return false;

        iter++;
        if(iter>=itermax)
            return false;
    }
    while (fabs(umax-umin)>pcrit && fabs(vmax-vmin)>pcrit);

    u = (umin+umax)/2.0;
    v = (vmin+vmax)/2.0;
/*
    auto t1 = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    qDebug("Intersection(%d):  u=%9g  v=%9g  %gms", iter, u, v, double(duration)/1000.0);
*/
    return true;
}





